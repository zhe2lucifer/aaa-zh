#include "aui_common_priv.h"
#include "aui_smc.h"
#include "alipltfretcode.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "alislsmc.h"

AUI_MODULE(SMC)

#define PPS_MAX_LEN (6)
#define PPS_MIN_LEN (3)
#define PPSS ((char)0x0FF)
static unsigned char pps_buf[PPS_MAX_LEN] = {0};

/* shall be available through including kernel driver  ali_smc_common.h */
#define SMC_STATUS_OK		(0)
#define SMC_STATUS_NOT_EXIST	(1)
#define SMC_STATUS_NOT_RESET	(2)

#define P3_IDX (4)
#define HEADER_LEN (5)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct smc
{
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	void *slsmc;
	aui_smc_p_fun_cb p_smc_cb;
	int smc_t[MAX_SMC_SLOT];
	int aui_t1_iso_flag[MAX_SMC_SLOT];
};

#define DEFAULT_F	(372)	/* ISO/IEC 7816-3 (2006) section 8.1 */
#define DEFAULT_D	(1)	/* ISO/IEC 7816-3 (2006) section 8.1 */
#define DEFAULT_N	(0)	/* ISO/IEC 7816-3 (2006) section 8.3 */
#define DEFAULT_WI	(10)	/* ISO/IEC 7816-3 (2006) section 10.2 */
#define TA		(0)
#define TB		(1)
#define TC		(2)
#define TD		(3)
#define BYTE_TS_POS	(0)
#define BYTE_T0_POS	(BYTE_TS_POS + 1)
#define ATR_MIN_LEN	(BYTE_T0_POS + 1)
#define ATR_MAX_LEN	(33)
#define GET_Y_SHIFT	(4)
#define LSB_MASK	(0x01)
#define BYTE_SIZE	(8)
#define RECV_MAX_LENGTH (256)

static int get_byte(unsigned char *atr, int len, int type_target, int i_target)
{
	int i;	/* i as used ISO/IEC 7816-3 (2006) table 6 section 8.2 */
	int j;	/* index inside the atr sequence of bytes */
	unsigned char Y;

	if (len < ATR_MIN_LEN || len > ATR_MAX_LEN)
		return -1;

	i = 0;
	j = BYTE_T0_POS;
	for (;;) {
		if (j >= len)
			return -1;

		Y = atr[j] >> GET_Y_SHIFT;

		if ((i_target == i + 1) && (type_target == TA))
			break;
		j += Y & LSB_MASK;
		Y >>= 1;

		if ((i_target == i + 1) && (type_target == TB))
			break;
		j += Y & LSB_MASK;
		Y >>= 1;
		if ((i_target == i + 1) && (type_target == TC))
			break;
		j += Y & LSB_MASK;
		Y >>= 1;

		if ((i_target == i + 1) && (type_target == TD))
			break;
		j += Y & LSB_MASK;
		Y >>= 1;

		i++;
	}
	j++;

	if (!(Y & LSB_MASK) || j >= len)
		return -1;

	return atr[j];
}

#define T_MASK	(0x0F)
//static int smc_t0_supported(unsigned char *atr, int len)
//{
//	int td1 = get_byte(atr, len, TD, 1);
//
//	if (td1 < 0)
//		return 1;
//	else if ((td1 & T_MASK) == 0)
//		return 1;
//	else
//		return 0;
//}
//
//static int smc_t1_supported(unsigned char *atr, int len)
//{
//	int td1 = get_byte(atr, len, TD, 1);
//
//	if (td1 < 0)
//		return 0;
//
//	return ((td1 & T_MASK) == 1) ? 1 : 0;
//
//}

#define DEFAULT_F_D_MASK	(0x10)
static int default_f_d(unsigned char *atr, int len)
{
	int ta2 = get_byte(atr, len, TA, 2);

	if (ta2 < 0)
		return 1;

	return  (ta2 & DEFAULT_F_D_MASK) ? 1 : 0;
}

static int fi_table[] = {
	372, 372, 558, 744, 1116, 1488, 1860, DEFAULT_F, DEFAULT_F,
	512, 768, 1024, 1536, 2048, DEFAULT_F, DEFAULT_F};

#define GET_F_SHIFT		(4)
int smc_get_f(unsigned char *atr, int len)
{
	int ta1 = get_byte(atr, len, TA, 1);

	if (default_f_d(atr, len))
		return DEFAULT_F;
	else if (ta1 < 0)
		return DEFAULT_F;
	else
		return fi_table[ta1 >> GET_F_SHIFT];
}

static int di_table[] = {
	DEFAULT_D, 1, 2, 4, 8, 16, 32, 64, 12, 20, DEFAULT_D, DEFAULT_D,
	DEFAULT_D, DEFAULT_D, DEFAULT_D, DEFAULT_D};

#define GET_D_MASK		(0x0F)
int smc_get_d(unsigned char *atr, int len)
{
	int ta1 = get_byte(atr, len, TA, 1);

	if (default_f_d(atr, len))
		return DEFAULT_D;
	else if (ta1 < 0)
		return DEFAULT_D;
	else
		return di_table[ta1 & GET_D_MASK];
}

int smc_get_n(unsigned char *atr, int len)
{
	int tc1 = get_byte(atr, len, TC, 1);

	if (tc1 < 0)
		return DEFAULT_N;

	return tc1;
}

int smc_get_wi(unsigned char *atr, int len)
{
	int tc2 = get_byte(atr, len, TC, 2);

	if (tc2 < 0)
		return DEFAULT_WI;

	return tc2;
}

#define SW1SW2_LEN		(2)
#ifndef APDU_SMC_DISABLE
static int p3_incorrect(unsigned char *resp, int len)
{
	if (len != SW1SW2_LEN)
		return 0;

	return (resp[0] == 0x67 && resp[1] == 0x00) ? 1 : 0;
}

static int la_indicated(unsigned char *resp, int len)
{
	if (len != SW1SW2_LEN)
		return 0;

	return (0x6c == resp[0]) ? 1 : 0;
}

#define LA_IDX			(1)
static int get_la(unsigned char *resp)
{
	return resp[LA_IDX];
}

static int smc_t0_tdpu_exchange(struct smc *smc, unsigned char *cmd, int len,
					unsigned char *resp, int buf_len)
{
	int status;
	int resp_len = (buf_len);

	status = alislsmc_iso_transfer(smc->slsmc,
					cmd, len,
					resp, resp_len,
					(size_t *)(&resp_len));

	if (status < 0)
		return -1;

	return resp_len;
}


#define P3_IDX			(4)
#define LE_IDX			(4)
static int case_2_exchange(struct smc *smc, unsigned char *cmd,
					int len, unsigned char *resp, int buf_len)
{
	int resp_len;
	int le = cmd[LE_IDX];

	unsigned char cmd_shadow[len];

	memcpy(cmd_shadow, cmd, len);

	resp_len = smc_t0_tdpu_exchange(smc, cmd, len, resp, buf_len);

	if (resp_len < 0) {
		return -1;

	} else if (resp_len == le + SW1SW2_LEN) { /* case 2S.1 */
		return resp_len;

	} else if (p3_incorrect(resp, resp_len)) { /* case 2S.2 */
		return resp_len;
	} else if (la_indicated(resp, resp_len)) { /* case 2S.3 */
		unsigned char la = get_la(resp);
		memcpy(cmd, cmd_shadow, len);
		cmd[P3_IDX] = la;
		resp_len = smc_t0_tdpu_exchange(smc, cmd, len, resp, buf_len);
		return (resp_len == la + SW1SW2_LEN) ? resp_len : -1;
	} else if (resp_len == SW1SW2_LEN) { /* case 2S.4 */
		return resp_len;
	} else {
		return -1;
	}
}

static int command_not_accepted(unsigned char *resp, int len)
{
	int status;

	if (len != SW1SW2_LEN)
		return 0;

	if (resp[0] == 0x61)
		status = 0;
	else if (resp[0] == 0x90 && resp[1] == 0x00)
		status = 0;
	else if ((resp[0] & 0xF0) == 0x60 || (resp[0] & 0xF0) == 0x60)
		status = 1;
	else
		status = 0;

	return status;
}

static int command_accepted(unsigned char *resp, int len)
{
	if (len != SW1SW2_LEN)
		return 0;

	return (resp[0] == 0x90 && resp[1] == 0x00) ? 1 : 0;
}

static int command_accepted_lx_added(unsigned char *resp, int len)
{
	if (len != SW1SW2_LEN)
		return 0;

	return (resp[0] == 0x61) ? 1 : 0;
}

unsigned char case4s3_le(unsigned char le, unsigned char lx)
{
	if (le != 0x00 && lx != 0x00)
		return MIN(le, lx);
	else
		return MAX(le, lx);
}

#define LX_IDX			(1)
static int case_4_exchange(struct smc *smc, unsigned char *cmd, int len,
					unsigned char *resp, int buf_len)

{
	unsigned char le = cmd[len - 1];
	int resp_len = smc_t0_tdpu_exchange(smc, cmd, len - 1, resp, buf_len);

	if (resp_len < 0) {
		return -1;
	} else if (command_not_accepted(resp, resp_len)) { /* case 4S.1 */
		return resp_len;
	} else if (command_accepted(resp, resp_len)) { /* case 4S.2 */
		unsigned char cmd4s2[HEADER_LEN] = {0x00, 0xC0, 0x00, 0x00, le};
		memcpy(cmd, cmd4s2, HEADER_LEN);
		return case_2_exchange(smc, cmd, HEADER_LEN, resp, buf_len);

	} else if (command_accepted_lx_added(resp, resp_len)) { /* case 4S.3 */
		unsigned char lx = resp[LX_IDX];
		unsigned char cmd4s3[HEADER_LEN] = {0x00, 0xC0, 0x00,
						0x00, case4s3_le(le, lx)};
		memcpy(cmd, cmd4s3, HEADER_LEN);
		return case_2_exchange(smc, cmd, HEADER_LEN, resp, buf_len);

	} else {
		return -1;
	}
}

#define HEADER_LEN_EXT		(HEADER_LEN + 2)
#define LC_EXT_MAX		(65536)
#define LC_IDX			(4)
static int get_N_ext(unsigned char *cmd, int len)
{
	int N;

	if (len < HEADER_LEN_EXT)
		return -1;

	N = (cmd[LC_IDX + 1] << BYTE_SIZE) + cmd[LC_IDX + 2];
	if (N == 0)
		N = LC_EXT_MAX;

	return N;
}

#define NOT_EXT	(256)
static int extended_mandatory(int N)
{
	return N > NOT_EXT;
}

static int iso7816_len(int Ne, int extend)
{
	if (Ne == extend)
		Ne = 0;

	return Ne;
}

static int is_case2e2a(unsigned char *resp, int len)
{
	if (len != 2)
		return 0;
	if (resp[0] != 0x67)
		return 0;
	if (resp[1] != 0x00)
		return 0;

	return 1;
}

static int is_case2e2b(unsigned char *resp, int len)
{
	if (len != 2)
		return 0;
	if (resp[0] != 0x6c)
		return 0;

	return 1;
}

static int is_case2e2c(unsigned char *resp, int len)
{
	if (len != 256 + 2)
		return 0;
	if (resp[256] != 0x90)
		return 0;
	if (resp[257] != 0x00)
		return 0;

	return 1;
}

static int is_case2e2d(unsigned char *resp, int len)
{
	if (len < 2)
		return 0;
	if (resp[len-2] != 0x61)
		return 0;

	return 1;
}

static int get_response_c2e2d(struct smc *smc, unsigned char **resp,
								int *buf_len,
								int Nx, int Nm)
{
	unsigned char cmd[HEADER_LEN] = {0x00, 0xC0, 0x00, 0x00, MIN(Nx, Nm)};

	int resp_len = smc_t0_tdpu_exchange(smc, cmd, HEADER_LEN,
							*resp, *buf_len);

	if (resp_len > -1) {
		*resp += resp_len - SW1SW2_LEN;
		*buf_len -= resp_len - SW1SW2_LEN;
	}

	return resp_len;
}

static int case2e2d_core(struct smc *smc, unsigned char *resp,
						int resp_len, int buf_len,
						int Ne)
{
	unsigned char *resp_wrk = resp;
	int buf_len_wrk = buf_len;
	int Nx =  iso7816_len(resp[resp_len -1], 256);
	int Nm = Ne - Nx;
	int ret;

	while (Nm > 0) {
		Nx = get_response_c2e2d(smc, &resp_wrk,
					&buf_len_wrk, Nx, Nm);
		if (Nx < 0)
			break;
		Nm -= Nx;
	}

	if (Nm == 0)
		ret = 0;
	else
		ret = -1;

	return ret;
}

static int case_2e2_core(struct smc *smc, unsigned char *cmd,
				unsigned char *resp, int buf_len, int Ne)
{
		int resp_len;
		unsigned char cmd_shadow[HEADER_LEN];
		unsigned char cmd_case2e2[HEADER_LEN];

		memcpy(cmd_shadow, cmd, HEADER_LEN);
		memcpy(cmd_case2e2, cmd, HEADER_LEN - 1);
		cmd_case2e2[HEADER_LEN-1] = 0;

		resp_len = smc_t0_tdpu_exchange(smc, cmd_case2e2,
						HEADER_LEN, resp, buf_len);

		if (resp_len <  0) {
			/* nothing to do */
		} else if (is_case2e2a(resp, resp_len)) {
			/* nothing to do */

		} else if (is_case2e2b(resp, resp_len)) {
			unsigned char la = get_la(resp);
			memcpy(cmd, cmd_shadow, HEADER_LEN);
			cmd[P3_IDX] = la;
			resp_len = smc_t0_tdpu_exchange(smc, cmd, HEADER_LEN,
								resp, buf_len);
			resp_len = (resp_len == la + SW1SW2_LEN) ? resp_len : -1;
		} else if (is_case2e2c(resp, resp_len)) {
			/* nothing to do */
		} else if (is_case2e2d(resp, resp_len) && buf_len >=
							Ne + SW1SW2_LEN) {
			resp_len = case2e2d_core(smc, resp, resp_len, buf_len, Ne);

		} else {
			resp_len = -1;
		}

		return resp_len;
}

static int case_2e_exchange(struct smc *smc, unsigned char *cmd, int len,
					unsigned char *resp, int buf_len)
{
	int ret;

	int Ne = get_N_ext(cmd, len);

	if (extended_mandatory(Ne)) { /* case 2E.2*/
		ret = case_2e2_core(smc, cmd, resp, buf_len, Ne);

	} else { /* case 2E.1 */
		unsigned char cmd_case2[HEADER_LEN];
		memcpy(cmd_case2, cmd, HEADER_LEN - 1);
		cmd_case2[HEADER_LEN-1] = iso7816_len(Ne, NOT_EXT);
		ret = case_2_exchange(smc, cmd_case2, HEADER_LEN,
								resp, buf_len);
	}

	return ret;
}

static int envelope_cmd_not_suported(unsigned char *resp, int resp_len)
{
	if (resp[resp_len -2] != 0x6d)
		return 0;
	if (resp[resp_len -1] != 0x00)
		return 0;

	return 1;
}

static int ready_to_receive(unsigned char *resp, int resp_len)
{
	if (resp[resp_len -2] != 0x90)
		return 0;
	if (resp[resp_len -1] != 0x00)
		return 0;

	return 1;
}

#define C6C7_LEN (2)
#define CHUNK_LEN (128)
static int case_3e2_core(struct smc *smc, unsigned char *cmd,
				int len, unsigned char *resp, int buf_len)
{
	int ret = -1;
	unsigned char header[HEADER_LEN] = {0x00, 0xC2, 0x00, 0x00,
								0x00};
	unsigned char *neighbour = cmd + len;

	while (cmd < neighbour) {
		int chunk_len = MIN(neighbour - cmd, CHUNK_LEN);
		header[P3_IDX] = chunk_len;
		int resp_len;
		memcpy(cmd, header, HEADER_LEN);
		resp_len = smc_t0_tdpu_exchange(smc, cmd,
						chunk_len + HEADER_LEN,
						resp, buf_len);

		if (resp_len < 0) {
			ret = -1;
			break;
		} if (envelope_cmd_not_suported(resp, resp_len)) {
			ret = 0;
			break;
		} else if (ready_to_receive(resp, resp_len)) {
			ret = 0;
			cmd += CHUNK_LEN;
		} else {
			ret = 0;
			break;
		}
	}

	return ret;
}

static int case_3e_exchange(struct smc *smc, unsigned char *cmd,
				int len, unsigned char *resp, int buf_len)
{
	int ret;

	int Ne = get_N_ext(cmd, len);

	if (extended_mandatory(Ne)) { /* case 3E.2 */

		ret = case_3e2_core(smc, cmd, len, resp, buf_len);

	} else { /* case 3E.1*/
		cmd[P3_IDX] = (unsigned char)Ne;
		memmove(&cmd[P3_IDX + 1], &cmd[P3_IDX + 1 + C6C7_LEN],
						Ne * sizeof(unsigned char));
		ret = smc_t0_tdpu_exchange(smc, cmd, len - C6C7_LEN,
								resp, buf_len);
	}

	return ret;
}

static int is_case4e1a(unsigned char *resp, int len)
{
	int SW1;

	if (len != 2)
		return 0;

	SW1 = resp[0];

	if (SW1 == 0x61 || SW1 == 0x62 || SW1 == 0x63)
		return 0;
	else if ((SW1 & 0xF0) == 0x60)
		return 1;
	else
		return 0;
}

static int is_case4e1b(unsigned char *resp, int len)
{
	if (len != 2)
		return 0;

	if (resp[0] == 0x90 && resp[1] == 0x00)
		return 1;
	else
		return 0;
}

static int is_case4e1c(unsigned char *resp, int len)
{
	if (len != 2)
		return 0;

	if (resp[0] == 0x61)
		return 1;
	else
		return 0;
}

static int is_case4e1d(unsigned char *resp, int len)
{
	int SW1;
	int SW2;

	if (len != 2)
		return 0;

	SW1 = resp[0];
	SW2 = resp[1];

	if (SW1 == 0x90 && SW2 == 0x00)
		return 0;
	else if (SW1 == 0x90 || SW1 == 0x62 || SW1 == 0x62)
		return 1;
	else
		return 0;
}

#define C5C6C7_LEN (3)
#define C5C6_LEN (2)
static int case_4e_exchange(struct smc *smc, unsigned char *cmd,
					int len, unsigned char *resp, int buf_len)

{
	int resp_len;
	int ret = -1;

	int Ne = get_N_ext(cmd, len);

	if (extended_mandatory(Ne)) { /* case 4E.2 transmit part */
		int len_4e2 = HEADER_LEN + C5C6C7_LEN + get_N_ext(cmd, len);
		resp_len = case_3e2_core(smc, cmd, len_4e2, resp, buf_len);
	} else { /* case 4E.1 transmit part */
		int len_4e1 = Ne + HEADER_LEN;
		unsigned char *cmd_4e1 = cmd + C5C6_LEN;
		memmove(cmd_4e1, cmd, HEADER_LEN);
		resp_len = smc_t0_tdpu_exchange(smc, cmd_4e1, len_4e1,
								resp, buf_len);
	}

	if (resp_len < 0) {
		ret = -1;

	} else if (is_case4e1a(resp, resp_len)) {
		ret = 0;

	} else if (is_case4e1b(resp, resp_len)) {
		if (Ne <= 256) {
			unsigned char cmd[HEADER_LEN] =
						{0x00, 0xC0, 0x00, 0x00, Ne};
			ret = case_2_exchange(smc, cmd, HEADER_LEN, resp, buf_len);
		} else {
			unsigned char cmd[HEADER_LEN] =
						{0x00, 0xC0, 0x00, 0x00,0x00};
			ret = case_2e2_core(smc, cmd, resp, buf_len, Ne);
		}

	} else if (is_case4e1c(resp, resp_len)) {
		ret = case2e2d_core(smc, resp, resp_len, buf_len, Ne);

	} else if (is_case4e1d(resp, resp_len)) {
		ret = 0;
	}

	return ret;
}

#define CASE1_HEADER_LEN	(4)
int smc_t0_exchange(struct smc *smc, unsigned char *cmd, int len,
					unsigned char *resp, int buf_len)
{
	int ret = -1;

	if (len < CASE1_HEADER_LEN) {
		ret = -1;
	} else if (len == CASE1_HEADER_LEN) { /* case 1 */
		unsigned char tdpu[len + 1];

		memcpy(tdpu, cmd, sizeof(unsigned char) * len);
		tdpu[len] = 0;

		ret = smc_t0_tdpu_exchange(smc, tdpu, len + 1, resp, buf_len);

	} else if (len == HEADER_LEN) { /* case 2 */
		ret = case_2_exchange(smc, cmd, len, resp, buf_len);

	} else if (cmd[LC_IDX] == len - HEADER_LEN) { /* case 3 */
		ret = smc_t0_tdpu_exchange(smc, cmd, len, resp, buf_len);

	} else if (cmd[LC_IDX] == len - HEADER_LEN - 1) { /* case 4 */
		ret = case_4_exchange(smc, cmd, len, resp, buf_len);

	} else if (cmd[LC_IDX] == 0x00) { /* extended command */
		if (len == HEADER_LEN_EXT) { /* case 2E */
			ret = case_2e_exchange(smc, cmd, len, resp, buf_len);

		} else if (get_N_ext(cmd, len) == len - HEADER_LEN_EXT) {
								/* case 3E */
			ret = case_3e_exchange(smc, cmd, len, resp, buf_len);

		} else if (get_N_ext(cmd, len) == len - HEADER_LEN_EXT - 1) {
								/* case 4E */
			ret = case_4e_exchange(smc, cmd, len, resp, buf_len);
		} else {
			ret = -1;
		}

	} else {
		ret = -1;
	}

	return ret;
}
#endif

static void smc_status_callback(void *p_user_data, unsigned int param)
{
	struct smc *smc_dev = (struct smc *)p_user_data;

	if (smc_dev->p_smc_cb) {
		smc_dev->p_smc_cb(smc_dev->data.dev_idx, param);
	}
}

#define	AUI_MAX_SMC_DEV	2
aui_smc_device_cfg_t smc_config[AUI_MAX_SMC_DEV];

AUI_RTN_CODE aui_smc_init(p_fun_cb psmc_cb_init)
{
	memset(smc_config, 0, AUI_MAX_SMC_DEV * sizeof(aui_smc_device_cfg_t));


	if (psmc_cb_init) {
		return psmc_cb_init((void *)smc_config);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_de_init(p_fun_cb pnim_call_back_init)
{
	return (pnim_call_back_init) ? pnim_call_back_init(NULL) :
							AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_open(aui_smc_attr *psmc_attr, aui_hdl *pp_smc_handle)
{
	struct smc *smc;
	int status;
	struct smc_device_cfg config_sl;

	if (pp_smc_handle == NULL)
		goto smc_alloc_fail;

	smc = (struct smc *)malloc(sizeof(struct smc));
	if (smc == NULL)
		goto smc_alloc_fail;

	if (psmc_attr == NULL || pp_smc_handle == NULL)
		goto param_fail;

	status = alislsmc_open(&smc->slsmc, psmc_attr->ul_smc_id);
	if (status != 0)
		goto shared_lib_fail;

	aui_smc_device_cfg_t config;
	memset(&config, 0, sizeof(aui_smc_device_cfg_t));
	if (memcmp(&smc_config[psmc_attr->ul_smc_id], &config, sizeof(aui_smc_device_cfg_t))) {
		memcpy(&config, &smc_config[psmc_attr->ul_smc_id], sizeof(aui_smc_device_cfg_t));
		memset(&config_sl, 0, sizeof(config_sl));
		config_sl.init_clk_trigger			= config.init_clk_trigger;
		config_sl.def_etu_trigger			= config.def_etu_trigger;
		config_sl.sys_clk_trigger			= config.sys_clk_trigger;
		config_sl.gpio_cd_trigger			= config.gpio_cd_trigger;
		config_sl.gpio_cs_trigger			= config.gpio_cs_trigger;
		config_sl.force_tx_rx_trigger		= config.force_tx_rx_trigger;
		config_sl.parity_disable_trigger	= config.parity_disable_trigger;
		config_sl.parity_odd_trigger		= config.parity_odd_trigger;
		config_sl.apd_disable_trigger		= config.apd_disable_trigger;
		config_sl.type_chk_trigger			= config.type_chk_trigger;
		config_sl.warm_reset_trigger		= config.warm_reset_trigger;
		config_sl.gpio_vpp_trigger			= config.gpio_vpp_trigger;
		config_sl.disable_pps				= config.disable_pps;
		config_sl.invert_power				= config.invert_power;
		config_sl.invert_detect				= config.invert_detect;
		config_sl.class_selection_supported	= config.class_selection_supported;
		config_sl.board_supported_class		= config.board_supported_class;
		config_sl.init_clk_number			= config.init_clk_number;
		config_sl.init_clk_array			= config.init_clk_array;
		config_sl.default_etu				= config.default_etu;
		config_sl.smc_sys_clk				= config.smc_sys_clk;
		config_sl.gpio_cd_pol				= config.gpio_cd_pol;
		config_sl.gpio_cd_io				= config.gpio_cd_io;
		config_sl.gpio_cd_pos				= config.gpio_cd_pos;
		config_sl.gpio_cs_pol				= config.gpio_cs_pol;
		config_sl.gpio_cs_io				= config.gpio_cs_io;
		config_sl.gpio_cs_pos				= config.gpio_cs_pos;
		config_sl.force_tx_rx_cmd			= config.force_tx_rx_cmd;
		config_sl.force_tx_rx_cmd_len		= config.force_tx_rx_cmd_len;
		config_sl.intf_dev_type				= config.intf_dev_type;
		config_sl.gpio_vpp_pol				= config.gpio_vpp_pol;
		config_sl.gpio_vpp_io				= config.gpio_vpp_io;
		config_sl.gpio_vpp_pos				= config.gpio_vpp_pos;
		config_sl.ext_cfg_tag				= config.ext_cfg_tag;
		config_sl.ext_cfg_pointer			= config.ext_cfg_pointer;
		config_sl.use_default_cfg			= config.use_default_cfg;
		alislsmc_set_cfg(smc->slsmc, &config_sl);
	}

	smc->p_smc_cb = psmc_attr->p_fn_smc_cb;
	if (smc->p_smc_cb) {
 		status = alislsmc_register_callback(smc->slsmc, smc, smc_status_callback);
		if (status != 0)
			goto shared_lib_fail;
	}

	smc->data.dev_idx = psmc_attr->ul_smc_id;
	status = aui_dev_reg(AUI_MODULE_SMC, smc);
	if (status != 0)
		goto aui_dev_reg_fail;

	*pp_smc_handle = smc;

	// The plug event monitor should be ready before active
	// alislsmc_start will emit a plug event,
	// must pull it at the end of the open function
	if (alislsmc_start(smc->slsmc)) {
		aui_smc_close(smc);
		*pp_smc_handle = NULL;
		AUI_ERR("alislsmc_start error\n");
		goto aui_dev_reg_fail;
	}

	//AUI_DBG("aui_smc_open OK\n");
	return AUI_RTN_SUCCESS;

aui_dev_reg_fail:
shared_lib_fail:
param_fail:
	free(smc);
smc_alloc_fail:

	return AUI_RTN_FAIL;

}

AUI_RTN_CODE aui_smc_close(aui_hdl smc_handle)
{
	struct smc *smc = smc_handle;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	alislsmc_close(smc->slsmc);

	aui_dev_unreg(AUI_MODULE_SMC, smc);

	free(smc);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_register_callback(aui_hdl smc_handle,
						aui_smc_p_fun_cb callback)
{
	struct smc *smc = smc_handle;
	int status=AUI_RTN_FAIL;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	smc->p_smc_cb = callback;
	if (smc->p_smc_cb) {
 		status = alislsmc_register_callback(smc->slsmc, smc, smc_status_callback);
	}
	if (status != 0)
		return AUI_RTN_FAIL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_get_atr(aui_hdl smc_handle, unsigned char *puc_atr,
						unsigned short *pn_atr_length)
{
	struct smc *smc = smc_handle;
	int status;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	status = alislsmc_get_atr(smc->slsmc, puc_atr, pn_atr_length);

	if (status != 0)
		return AUI_RTN_FAIL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_reset(aui_hdl smc_handle, unsigned char *puc_atr,
				unsigned short *pn_atr_length, int b_cold_rst)
{
	struct smc *smc = smc_handle;
	int status;
	int ioctl_data;
	int *ioctl_arg = &ioctl_data;
	unsigned int reset_mode = 1;

	if (smc == NULL) {
		AUI_ERR("SMC reset handle is NULL.\n");
		return AUI_RTN_FAIL;
	}

	//if (b_cold_rst)
	//	return AUI_RTN_FAIL;
//	if(b_cold_rst == 1 && 0 != alislsmc_deactive(smc->slsmc)) {
//		AUI_DBG("SMC reset deactive error.\n");
//		return AUI_RTN_FAIL;
//	}

    // cold/warm reset control improve
    // Use SMC_IOCMD_SET_RESET_MODE, same as TDS,
    // and the warm_reset_trigger will be ignore.
	reset_mode = b_cold_rst ? 0 : 1;
	// SMC_IOCMD_SET_RESET_MODE Set reset mode: 0: cold reset; 1: warm reset
	if (alislsmc_ioctl(smc->slsmc, SMC_IOCMD_SET_RESET_MODE, (unsigned long)&reset_mode) < 0) {
	    AUI_ERR("SMC_IOCMD_SET_RESET_MODE set %d fail\n", reset_mode);
	    return AUI_RTN_FAIL;
	}

	status = alislsmc_reset(smc->slsmc);

	if (status != 0)
		return AUI_RTN_FAIL;

	status = alislsmc_get_atr(smc->slsmc, puc_atr, pn_atr_length);

	if (status != 0) {
		return AUI_RTN_FAIL;
	}
	status = alislsmc_ioctl(smc->slsmc, SMC_IOCMD_GET_PROTOCOL,
								(unsigned long)ioctl_arg);
	if (status != 0) {
		AUI_ERR("SMC reset SMC_IOCMD_GET_PROTOCOL fail %d.\n", status);
		return AUI_RTN_FAIL;
	}
	smc->smc_t[smc->data.dev_idx] = ioctl_data;
	smc->aui_t1_iso_flag[smc->data.dev_idx] = 0;
	//AUI_DBG("smc->smc_t[%lu]=%d\n", smc->data.dev_idx, smc->smc_t[smc->data.dev_idx]);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_param_get(aui_hdl smc_handle,
        aui_smc_param_t *p_smc_param)
{
    struct smc *smc = smc_handle;
    int ioctl_data = 0;
    int *ioctl_arg = &ioctl_data;
    unsigned char atr[ATR_MAX_LEN] = {0};
    unsigned short atr_len = 0;
    int ret = 0;
    int reseted = 0;

    if (smc == NULL) {
        AUI_DBG("%s input error\n", __FUNCTION__);
        return AUI_RTN_FAIL;
    }

    p_smc_param->m_nETU = -1;
    p_smc_param->m_n_baud_rate = -1;
    p_smc_param->m_n_frequency = -1;
    p_smc_param->m_e_standard = AUI_SMC_STANDARD_ISO;
    p_smc_param->m_e_protocol = -1;
    p_smc_param->m_e_stop_bit = -1;
    p_smc_param->m_e_check_bit = -1;

    // Some parameters are relate to ATR, some are not.
    reseted = !alislsmc_get_atr(smc->slsmc, atr, &atr_len);
    AUI_DBG("atr_len: %d\n", atr_len);

    if (reseted && atr_len > 0) {
        p_smc_param->m_nETU = smc_get_f(atr, atr_len) /
                smc_get_d(atr, atr_len);

        ret = alislsmc_ioctl(smc->slsmc, SMC_IOCMD_GET_PROTOCOL,
                (unsigned long)ioctl_arg);
        if (ret  < 0) {
            AUI_DBG("SMC_CMD_GET_PROTOCOL error\n");
            return AUI_RTN_FAIL;
        }
        if (ioctl_data == 0){
            p_smc_param->m_e_protocol = AUI_SMC_PROTOCOL_T0;
        } else if (ioctl_data == 1){
            if (smc->aui_t1_iso_flag[smc->data.dev_idx] == AUI_SMC_PROTOCOL_T1_ISO) {
                p_smc_param->m_e_protocol = AUI_SMC_PROTOCOL_T1_ISO;
            } else {
                p_smc_param->m_e_protocol = AUI_SMC_PROTOCOL_T1;
            }
        } else {
            p_smc_param->m_e_protocol = AUI_SMC_PROTOCOL_UNKNOWN;
        }
    }

    // Get clock frequency in Hz: SMC_CMD_GET_WCLK
    if (alislsmc_ioctl(smc->slsmc, SMC_IOCMD_GET_WCLK,
            (unsigned long)&p_smc_param->m_n_frequency)) {
        AUI_DBG("SMC_IOCMD_GET_WCLK error\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_param_set(aui_hdl smc_handle,
					const aui_smc_param_t *p_smc_param)
{
	struct smc *smc = smc_handle;
	unsigned long ioctl_data;
	int ret = 0;
	int iso_flag = 0;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	// Set protocol
	switch (p_smc_param->m_e_protocol) {
	case AUI_SMC_PROTOCOL_T0:
		ioctl_data = 0;
		break;
	case AUI_SMC_PROTOCOL_T1:
		ioctl_data = 1;
		break;
	case AUI_SMC_PROTOCOL_T1_ISO:
		iso_flag = 1;
		ioctl_data = 1;
		break;
	case AUI_SMC_PROTOCOL_T14:
		ioctl_data = 14;
		break;
	default:
		ioctl_data = (unsigned long)-1;
		break;
	}
	// Don't set the protocol if the input illegal
	// 0 ~ 15: Transmission protocol defined in ISO/IEC 7816-3
	if (ioctl_data < 16) {
		smc->aui_t1_iso_flag[smc->data.dev_idx] = iso_flag;
		ret = alislsmc_ioctl(smc->slsmc, SMC_IOCMD_SET_PROTOCOL, ioctl_data);
	}

	// Set ETU
	ioctl_data = (unsigned long)p_smc_param->m_nETU;
	if (ioctl_data > 0) {
		ret |= alislsmc_ioctl(smc->slsmc, SMC_IOCMD_SET_ETU, ioctl_data);
	}

	if (ret < 0)
		return AUI_RTN_FAIL;

	return AUI_RTN_SUCCESS;
}

// IRDETO T=14 smart card transmission
// Smartcard Low-Level Interface Document No. 753354
// 4.4 Message Structure Description
// step 1: SendRaw all data
// step 2: ReadRaw 8 bytes respond header(include payload length)
// step 3: ReadRaw payload and checksum
static int smc_irdeto_t14_exchange(
        aui_hdl smc_handle,
        unsigned char *sendData_p,
        int sendDataLength,
        unsigned char *recvData_p,
        int *recvDataLength_p)
{
    int ret = 0;
    unsigned int LC; // Payload length.
    unsigned int RESPONSE_HEAD_LEN = 8;
    //unsigned int read_head_len = 8;
    short recvLength = 0;

    if (aui_smc_raw_write(smc_handle, sendData_p,
            (short)sendDataLength, &recvLength) != AUI_RTN_SUCCESS) {
        AUI_ERR("T14 write fail\n");
        return 1;
    }

    memset(recvData_p, 0, *recvDataLength_p);
    *recvDataLength_p = 0;
    recvLength = 0;
    if (aui_smc_raw_read(smc_handle, recvData_p,
            RESPONSE_HEAD_LEN, &recvLength)) {
        AUI_ERR("T14 read respond header fail\n");
        return 1;
    }

    LC = *((unsigned char *)recvData_p + RESPONSE_HEAD_LEN - 1);

    recvLength = 0;
    if (aui_smc_raw_read(smc_handle, recvData_p + RESPONSE_HEAD_LEN,
            LC + 1, &recvLength)) {
        AUI_ERR("T14 read payload and checksum fail %d bytes\n", LC + 1);
        return 1;
    }

    *recvDataLength_p = recvLength + RESPONSE_HEAD_LEN;
    return ret;
}

AUI_RTN_CODE aui_smc_transfer(aui_hdl smc_handle,
			unsigned char *puc_write_data, int n_number_to_write,
			unsigned char *pc_response_data, int *pn_number_read)
{
	struct smc *smc = smc_handle;
	unsigned char atr[ATR_MAX_LEN];
	unsigned char recv_buf[RECV_MAX_LENGTH];
	int status;
	unsigned short atr_len;
	unsigned int actlen = 0;
//	aui_smc_param_t smc_param;
	int T = 0;

	if (smc == NULL || (puc_write_data == NULL) || (pc_response_data == NULL) || (NULL == pn_number_read))
		return AUI_RTN_FAIL;

	status = alislsmc_get_atr(smc->slsmc, atr, &atr_len);
	if (status != 0)
		return AUI_RTN_FAIL;

	T = smc->smc_t[smc->data.dev_idx];
	//AUI_DBG("TRANSFER smc->smc_t[%lu]=%d\n",
	//		smc->data.dev_idx, smc->smc_t[smc->data.dev_idx]);

//	if (smc_t0_supported(atr, atr_len)) {
	if (T == 0) {

#ifndef APDU_SMC_DISABLE

		*pn_number_read =  smc_t0_exchange(smc,puc_write_data, n_number_to_write,
			pc_response_data, *pn_number_read);

		status = (*pn_number_read < 0) ? -1 : 0;


#else
		short len = *pn_number_read;
//		// pn_number_read is pc_response_data bytes,
//		// but put pc_response_data + SW1SW2_LEN bytes in it!!!
//		status = alislsmc_iso_transfer(smc->slsmc, puc_write_data, (int)n_number_to_write,
//						pc_response_data, (len+SW1SW2_LEN), &actlen);

		// The pc_response_data is the combination of response data and status bytes
		// User should make sure the response buffer is big enough.
		status = alislsmc_iso_transfer(smc->slsmc, puc_write_data, (int)n_number_to_write,
						pc_response_data, (len), &actlen);
#endif

	}
	//else if (smc_t1_supported(atr, atr_len) && T == 1) {
	else if (T == 1) {
		if (smc->aui_t1_iso_flag[smc->data.dev_idx] == 0) {
			uint8_t dad = 0;
			actlen = alislsmc_t1_transfer(smc->slsmc,
					dad,
					puc_write_data, n_number_to_write,
					pc_response_data, *pn_number_read);
		} else {
			// AUI_SMC_PROTOCOL_T1_ISO is T1 raw data transfer mode.
			// Driver will transfer the data directly.
			alislsmc_iso_transfer_t1(smc->slsmc,
					puc_write_data, n_number_to_write,
					pc_response_data, *pn_number_read, &actlen);
		}
	} else if (T == 14) {
	    status = smc_irdeto_t14_exchange(smc_handle, puc_write_data,
	            n_number_to_write, recv_buf, (int *)&actlen);
	    if (actlen < (unsigned int)*pn_number_read) {
	        *pn_number_read = actlen;
	    }
	    memcpy(pc_response_data, recv_buf, *pn_number_read);
	} else {
		AUI_DBG("not support protocol\n");
		status = -1;
	}

	*pn_number_read = actlen;

	if (status != 0)
		return AUI_RTN_FAIL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_send(aui_hdl smc_handle, unsigned char *puc_hdr_body_buf,
						int n_number_to_write,
						int *pn_number_write,
						unsigned char *puc_status_word,
						int n_timeout)
{
	struct smc *smc = smc_handle;
	int ret;
	(void) n_timeout;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	*pn_number_write = 0;

#ifndef APDU_SMC_DISABLE

	ret = smc_t0_tdpu_exchange(smc, puc_hdr_body_buf, n_number_to_write,
						puc_status_word, SW1SW2_LEN);

	*pn_number_write = n_number_to_write;

#else

	size_t resp_len = 0;

	ret = alislsmc_iso_transfer(smc->slsmc,
			puc_hdr_body_buf, n_number_to_write,
			puc_status_word, SW1SW2_LEN,
						(size_t *)(&resp_len));

	*pn_number_write = (int)resp_len;

#endif

	if (ret < 0)
			return AUI_RTN_FAIL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_receive(aui_hdl smc_handle, unsigned char *puc_readcmd_hdr,
						unsigned char *puc_read_buf,
						int n_number_to_read,
						int *pn_number_read,
						unsigned char *puc_status_word,
						int n_timeout)
{
	struct smc *smc = smc_handle;
	(void)n_timeout;

	int ret;
	if (smc == NULL)
		return AUI_RTN_FAIL;

	*pn_number_read = 0;

#ifndef APDU_SMC_DISABLE

	ret = case_2_exchange(smc, puc_readcmd_hdr, HEADER_LEN,
						puc_read_buf, n_number_to_read);

	if((ret < 2))
		return AUI_RTN_FAIL;

	/* Status words */
	memcpy(puc_status_word,puc_read_buf+(ret-SW1SW2_LEN),SW1SW2_LEN);

	*pn_number_read = ret;


#else
	int actlen = 0;

	memset(puc_read_buf,0,(n_number_to_read));

	// The pc_response_data is the combination of response data and status bytes
	// User should make sure the response buffer is big enough.
	ret = alislsmc_iso_transfer(smc->slsmc,
			puc_readcmd_hdr, HEADER_LEN,
			puc_read_buf, (n_number_to_read), (size_t *)&actlen);

	/* Status words */
	if(puc_status_word && actlen >= SW1SW2_LEN)
		memcpy(puc_status_word, puc_read_buf + actlen - SW1SW2_LEN, SW1SW2_LEN);

	*pn_number_read = actlen;

	if (ret != 0 )
			return AUI_RTN_FAIL;

#endif

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_detect(aui_hdl smc_handle)
{
	struct smc *smc = smc_handle;
	int detect_status;
	int *ioctl_arg = &detect_status;
	int ret;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	ret = alislsmc_ioctl(smc->slsmc, SMC_IOCMD_CHECK_STATUS,
						(unsigned long)ioctl_arg);

	if (ret < 0)
		return -1;
	else if (detect_status == SMC_STATUS_OK)
		return AUI_SMC_ERROR_READY;
	else if (detect_status == SMC_STATUS_NOT_EXIST)
		return AUISMC_ERROR_OUT;
	else if (detect_status == SMC_STATUS_NOT_RESET)
		return AUISMC_ERROR_IN;
	else
		return -2;

}

AUI_RTN_CODE aui_smc_active(aui_hdl smc_handle)
{
	struct smc *smc = smc_handle;
//	int status;

	if (smc == NULL)
		return AUI_RTN_FAIL;

//	status = alislsmc_start(smc->slsmc);
//	if (status != 0)
//		return AUI_RTN_FAIL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_deactive(aui_hdl smc_handle)
{
	struct smc *smc = smc_handle;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	return alislsmc_deactive(smc->slsmc);
}

AUI_RTN_CODE aui_smc_setpps(aui_hdl smc_handle,
			unsigned char *puc_write_data, int number_to_writelen,
			unsigned char *puc_response_data,
			int *pn_response_datalen)
{
	struct smc *smc = smc_handle;
	size_t actlen = *pn_response_datalen;
	size_t write_len = number_to_writelen;
	size_t read_len = *pn_response_datalen;

	if (smc == NULL) {
		AUI_DBG("smc_handle is NULL\n");
		return AUI_RTN_FAIL;
	} else {
		//AUI_DBG("smc_handle is OK\n");
	}
	if (alislsmc_raw_write(smc->slsmc, puc_write_data, write_len, &actlen)) {
		AUI_DBG("smc_raw_write fail\n");
		return AUI_RTN_FAIL;
	} else {
		//AUI_DBG("smc_raw_write is OK\n");
	}
	// In the most common case, the PPS response is identical to the PPS request.
	if (alislsmc_raw_read(smc->slsmc, puc_response_data, read_len, &actlen)) {
		AUI_DBG("smc_raw_read fail\n");
		return AUI_RTN_FAIL;
	} else {
		//AUI_DBG("smc_raw_read is OK\n");
	}
	*pn_response_datalen = actlen;
	if (number_to_writelen == (int)actlen
			&& !memcmp(puc_write_data, puc_response_data, actlen)) {
		smc->smc_t[smc->data.dev_idx] = puc_write_data[1] & 0x0F;
		//AUI_DBG("Set smc->smc_t[%lu]=%d\n",
		//		smc->data.dev_idx, smc->smc_t[smc->data.dev_idx]);
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_isexist(aui_hdl smc_handle)
{
	struct smc *smc = smc_handle;

	if (smc == NULL)
		return AUI_RTN_FAIL;

	if (alislsmc_card_exist(smc->slsmc)){
		return AUI_RTN_SUCCESS;
	} else {
		return AUI_RTN_FAIL;
	}
}

AUI_RTN_CODE aui_smc_set(aui_hdl smc_handle, aui_smc_cmd_t cmd,
		void *pv_param)
{
	struct smc *smc = smc_handle;
	enum smc_io_command iocmd;
	int ret = AUI_RTN_SUCCESS;

	if (smc == NULL) {
		return AUI_RTN_FAIL;
	}

	switch (cmd) {
	case AUI_SMC_CMD_SET_WWT:
		iocmd = SMC_IOCMD_SET_WWT;
		break;
	case AUI_SMC_CMD_DISABLE_PPS_ON_RESET:
		iocmd = SMC_IOCMD_DISABLE_PPS_ON_RESET;
		break;
	default:
		return AUI_RTN_FAIL;
	}

	ret = alislsmc_ioctl(smc->slsmc, iocmd, (unsigned long)pv_param);
	return ret;
}

// Setup the interface device after a successful PPS exchange
static int ifd_pps_set(aui_hdl smc_handle, char *pps)
{
    unsigned char FI, DI;
    int Fi, Di, etu;
    aui_smc_param_t smc_param;

    // ISO/IEC 7813-3:2006
    static int conversion_factor[16] = {
            372, 372, 558, 744, 1116, 1488, 1860, -1,
            -1, 512, 768, 1024, 1536, 2048, -1, -1
    };
    static int adjustment_factor[16] = {
            -1, 1, 2, 4, 8, 16, 32, 64, 12, 20, -1, -1, -1, -1, -1, -1
    };
    // IRDETO T=14
    static int t14_conversion_factor[16] = {
            -1, 416, 620, -1, -1, -1, -1, -1,
            -1, 512, -1, 1152, -1, -1, -1, -1
    };
    static int t14_adjustment_factor[16] = {
            -1, 1, 2, 4, 8, 16, 32, 64,
            -1, -1, -1, -1, -1, -1, -1, -1
    };

    if (pps == NULL) {
        return 1;
    }
    memset(&smc_param, 0, sizeof(smc_param));
    FI = 1; // default
    DI = 1;
    if (pps[1] & 0x10) {
        FI = (pps[2] >> 4) & 0x0F;
        DI = pps[2] & 0x0F;
    }
    switch (pps[1] & 0xF) {
        case 14:
            Fi = t14_conversion_factor[FI];
            Di = t14_adjustment_factor[DI];
            break;
        default:
            Fi = conversion_factor[FI];
            Di = adjustment_factor[DI];
            break;
    }

    // Remove CPPTEST warning. Di will never be 0.
    if (Di == 0) {
        AUI_DBG("[PPS] Di == 0 !!!\n");
        return 1;
    }
    etu = Fi / Di;
    smc_param.m_nETU = etu;
    smc_param.m_e_protocol = pps[1] & 0x0F;
    //AUI_DBG("[PPS]ETU: %d T: %d\n", smc_param.m_nETU, smc_param.m_e_protocol);
    if (aui_smc_param_set(smc_handle, &smc_param)
            != AUI_RTN_SUCCESS) {
        AUI_DBG("[PPS] aui_smc_param_set failed\n");
        return 1;
    }
    return 0;
}

AUI_RTN_CODE aui_smc_raw_read(aui_hdl smc_handle, unsigned char *data,
        short size, short *actlen)
{
    int ret = AUI_RTN_SUCCESS;
    struct smc *smc = smc_handle;
    size_t tmp_size = 0;
    ret = alislsmc_raw_read(smc->slsmc, (void *)data, (size_t)size, &tmp_size);
    *actlen = (short)tmp_size;

    // Setup interface device after a successful PPS exchange
    if (!ret && (*(char *)data == PPSS)
            && (*actlen >= PPS_MIN_LEN)
            && (*actlen <= PPS_MAX_LEN)) {
        // ISO/IEC 7816-3:2006 9.3 Successful PPS exchange
        // The success PPS exchange the first 2 bytes is identical
        if (!memcmp(pps_buf, data, 2)) {
            if (ifd_pps_set(smc_handle, (char *)data)) {
                AUI_DBG("Raw transmission PPS ifd_pps_set fail\n");
            }
        }
    }
    return ret;
}

AUI_RTN_CODE aui_smc_raw_write(aui_hdl smc_handle, unsigned char *data,
        short size, short *actlen)
{
    int ret = AUI_RTN_SUCCESS;
    struct smc *smc = smc_handle;
    size_t tmp_size = 0;
    ret = alislsmc_raw_write(smc->slsmc, (void *)data, (size_t)size, &tmp_size);
    *actlen = (short)tmp_size;
    // Monitoring PPS exchange
    if (!ret && (*(char *)data == PPSS)
            && (*actlen >= PPS_MIN_LEN)
            && (*actlen <= PPS_MAX_LEN)) {
        memcpy(pps_buf, data, *actlen);
    }
    return ret;
}
