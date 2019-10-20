#include <aui_smc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "aui_smc_test.h"
#include "aui_test_app.h"
#include "aui_help_print.h"


#define MAX_ATR_LN (33)
#define MAX_RESP_LEN (260)
#if (defined(BOARD_CFG_M3733) || defined(BOARD_CFG_M3755))
#define ID (1)
#else
#define ID (0)
#endif
#define BASIC_CARD_T0

#ifdef BASIC_CARD_T0
#endif

static int g_plug_in_count = 0;
static int g_plug_out_count = 0;

#ifdef AUI_LINUX
	static unsigned long disable_pps_flag = 0;
#endif


static void printseq(unsigned char *seq, int len)
{
	int i;
	for (i = 0; i < len; i++)
		AUI_PRINTF("%02x ", seq[i]);
	AUI_PRINTF("(len = %i)\n", len);

}

static int checkseq(unsigned char *a, int a_len, unsigned char *b, int b_len)
{
	int i;

	if(a_len != b_len)
		return -1;

	for (i = 0; i < a_len; i++)
		if (a[i] != b[i])
			return -1;

	return 0;
}

static void fun_cb(int x, unsigned int status)
{
	AUI_PRINTF("smart card %d %s (status = %u)\n",
			x, status ? "detected" : "missing",  status);
	if (status) {
		g_plug_in_count++;
	} else {
		g_plug_out_count++;
	}
}

static unsigned char g_atr[32];
static unsigned short g_atr_len = 0;
static void fun_cb_reset(int x, unsigned int status)
{
	aui_hdl smc_handle;
	g_atr_len = 0;
	

	AUI_PRINTF(">>>> smart card %d %s (status = %u)\n",
			x, status ? "plug in" : "plug out",  status);
	if (status) {
		g_plug_in_count++;
		g_atr_len = 0;
		memset(g_atr, 0, sizeof(g_atr));
		do {
			if (aui_find_dev_by_idx(AUI_MODULE_SMC, ID, &smc_handle)) {
				AUI_PRINTF(">>>> aui_find_dev_by_idx AUI_MODULE_SMC %d fail\n", ID);
				break;
			}
			if (aui_smc_active(smc_handle)) {
				AUI_PRINTF(">>>> CARD %d aui_smc_active fail\n", ID);
				break;
			}

			#ifdef AUI_LINUX
				aui_smc_set(smc_handle, AUI_SMC_CMD_DISABLE_PPS_ON_RESET, &disable_pps_flag);
			#endif
			
			if (aui_smc_reset(smc_handle, g_atr, &g_atr_len, 1)) {
				AUI_PRINTF(">>>> CARD %d aui_smc_reset fail\n", ID);
				break;
			}
			if (g_atr_len > 0 && g_atr_len < 32) {
				AUI_PRINTF(">>>> CARD %d RESET OK\n", ID);
				printseq(g_atr, g_atr_len);
			} else {
				AUI_PRINTF(">>>> CARD %d ATR ERROR\n", ID);
				printseq(g_atr, 32);
			}
		} while(0);
	} else {
		g_plug_out_count++;
		do {
			if (aui_find_dev_by_idx(AUI_MODULE_SMC, ID, &smc_handle)) {
				AUI_PRINTF(">>>> aui_find_dev_by_idx AUI_MODULE_SMC %d fail\n", ID);
				break;
			}
			if (aui_smc_deactive(smc_handle)) {
				AUI_PRINTF(">>>> CARD %d aui_smc_deactive fail\n", ID);
				break;
			}
			AUI_PRINTF(">>>> CARD %d aui_smc_deactive OK\n", ID);
		} while(0);
	}
	
	
}

static AUI_RTN_CODE *smc_init_cb(void *para)
{
#ifdef AUI_LINUX
#ifdef BOARD_CFG_M3823 // For M3823_02V01	
	aui_smc_device_cfg_t *pconfig = (aui_smc_device_cfg_t *)para;
	AUI_PRINTF("M3823_02V01 running smc_init_cb\n");
	pconfig->init_clk_trigger = 1;
	pconfig->init_clk_number = 1;
	pconfig->apd_disable_trigger = 1;
	pconfig->def_etu_trigger = 1;
	pconfig->default_etu = 372;
	pconfig->warm_reset_trigger = 1;
	pconfig->init_clk_array[0] = 3600000;
	pconfig->invert_detect = 1;
	pconfig->force_tx_rx_trigger = 1;
	pconfig->force_tx_rx_cmd = 0xdd;
	pconfig->force_tx_rx_cmd_len = 5;
#endif

#if (defined(BOARD_CFG_M3733) || defined(BOARD_CFG_M3755)) // For M3733 M3755
	aui_smc_device_cfg_t *pconfig = (aui_smc_device_cfg_t *)para;
	AUI_PRINTF("M3733 running smc_init_cb\n");
	pconfig++; // 3733 use slot 1
	pconfig->init_clk_trigger = 1;
	pconfig->init_clk_number = 1;
	pconfig->apd_disable_trigger = 1;
	pconfig->def_etu_trigger = 1;
	pconfig->default_etu = 372;
	pconfig->warm_reset_trigger = 1;
	pconfig->init_clk_array[0] = 3600000;
	pconfig->invert_detect = 1;
	pconfig->force_tx_rx_trigger = 1;
	pconfig->force_tx_rx_cmd = 0xdd;
	pconfig->force_tx_rx_cmd_len = 5;
#endif
#endif
	return AUI_RTN_SUCCESS;
}

unsigned long test_conax(unsigned long *argc,char **argv,char *sz_out_put)
{
	(void)argc;
	(void)argv;
	(void)sz_out_put;
	static unsigned char atr_exp[] =
		{0x3b, 0x34, 0x94, 0x00, 0x30, 0x42, 0x30, 0x30};

	static unsigned char cmd1_cmd[] = {0xdd, 0x26, 0x00, 0x00, 0x03, 0x10, 0x01, 0x40};
	static unsigned char cmd1[sizeof(cmd1_cmd) / sizeof(unsigned char)];
	static unsigned char cmd1_exp[] = {0x98, 0x11};

	static unsigned char cmd2_cmd[] = {0xdd, 0xca, 0x00, 0x00, 0x11};
	static unsigned char cmd2[sizeof(cmd2_cmd) / sizeof(unsigned char)];
	static unsigned char cmd2_exp[] = {
		0x20, 0x01, 0x40, 0x28, 0x02, 0x0b, 0x00, 0x2f, 0x02, 0x00,
		0x2c, 0x30, 0x01, 0x01/*different car different value*/, 0x23, 0x01, 0x10, 0x90, 0x00};

	static unsigned char cmd3_cmd[] = {0xdd, 0x26, 0x00, 0x00, 0x04, 0x6c, 0x02, 0x20, 0x01};
	static unsigned char cmd3[sizeof(cmd3_cmd) / sizeof(unsigned char)];
	static unsigned char cmd3_exp[] = {0x90, 0x00};

	int status;
	void *smc_handle;
	aui_smc_attr smc_attr;
	unsigned char atr[MAX_ATR_LN];
	unsigned short atr_len;
	unsigned char resp[MAX_RESP_LEN];
	int resp_len;
	aui_smc_param_t smc_param;

	smc_attr.p_fn_smc_cb = fun_cb;
	smc_attr.ul_smc_id = ID;

	status = aui_smc_init((p_fun_cb)smc_init_cb);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_init fail\n");
		return -1;
	}

	AUI_PRINTF("SMD ID: %ld\n", smc_attr.ul_smc_id);
	status = aui_smc_open(&smc_attr, &smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_open fail\n");
		goto error;
	}

	status = aui_smc_active(smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_active fail\n");
		goto error;
	}

	atr_len = MAX_ATR_LN;
	status = aui_smc_reset(smc_handle, atr, &atr_len, 1);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_reset fail\n");
		goto error;
	} else {
		if (!checkseq(atr, (int)atr_len, atr_exp, sizeof(atr_exp))) {
			AUI_PRINTF("Match the expect ATR.\n");
		} else {
			AUI_PRINTF("Don't match the expect ATR.\n");
			printseq(atr, atr_len);
			goto error;
		}
	}

	resp_len = MAX_RESP_LEN;
	MEMCPY(cmd1, cmd1_cmd, sizeof(cmd1));
	AUI_PRINTF("1 transfer:");
	printseq(cmd1, sizeof(cmd1));
	AUI_PRINTF("expect respond:");
	printseq(cmd1_exp, sizeof(cmd1_exp));
	status = aui_smc_transfer(smc_handle,
			cmd1, sizeof(cmd1) / sizeof(unsigned char),
			resp, &resp_len);
	if (status != AUI_RTN_SUCCESS ||
			checkseq(resp, resp_len, cmd1_exp, sizeof(cmd1_exp))) {
		AUI_PRINTF("aui_smc_transfer FAIL %d\n", status);
		printseq(resp, resp_len);
		goto error;
	} else {
		AUI_PRINTF("cmd OK\n");
	}

	resp_len = MAX_RESP_LEN;
	MEMCPY(cmd2, cmd2_cmd, sizeof(cmd2));
	AUI_PRINTF("2 transfer:");
	printseq(cmd2, sizeof(cmd2));
	AUI_PRINTF("expect respond:");
	printseq(cmd2_exp, sizeof(cmd2_exp));
	status = aui_smc_transfer(smc_handle,
			cmd2, sizeof(cmd2) / sizeof(unsigned char),
			resp, &resp_len);
	if (status != AUI_RTN_SUCCESS ||
			checkseq(resp, resp_len, cmd2_exp, sizeof(cmd2_exp))) {
		if (cmd2_exp[13] != resp[13]) {
			AUI_PRINTF("different parent lock level, please check\n");
			AUI_PRINTF("cmd OK\n");
		} else {
			AUI_PRINTF("aui_smc_transfer FAIL %d\n", status);
			printseq(resp, resp_len);
			goto error;
		}
	} else {
		AUI_PRINTF("cmd OK\n");
	}


	resp_len = MAX_RESP_LEN;
	MEMCPY(cmd3, cmd3_cmd, sizeof(cmd3));
	AUI_PRINTF("3 transfer:");
	printseq(cmd3, sizeof(cmd3));
	AUI_PRINTF("expect respond:");
	printseq(cmd3_exp, sizeof(cmd3_exp));
	status = aui_smc_transfer(smc_handle,
			cmd3, sizeof(cmd3) / sizeof(unsigned char),
			resp, &resp_len);
	if (status != AUI_RTN_SUCCESS ||
			checkseq(resp, resp_len, cmd3_exp, sizeof(cmd3_exp))) {
		AUI_PRINTF("aui_smc_transfer FAIL %d\n", status);
		printseq(resp, resp_len);
		goto error;
	} else {
		AUI_PRINTF("cmd OK\n");
	}

	status = aui_smc_isexist(smc_handle);
	if (status != 0) {
		AUI_PRINTF("aui_smc_isexist fail.\n");
		goto error;
	}

	status = aui_smc_detect(smc_handle);
	AUI_PRINTF("detect status = %i\n", status);
	if (status != AUISMC_ERROR_READY) {
		AUI_PRINTF("aui_smc_detect fail\n");
		goto error;
	}

	memset(&smc_param, 0, sizeof(smc_param));
	status = aui_smc_param_get(smc_handle, &smc_param);
	if (status != 0) {
		AUI_PRINTF("aui_smc_param_get fail\n");
		goto error;
	}

	AUI_PRINTF("m_nETU=%d\n", smc_param.m_nETU);
	AUI_PRINTF("m_n_baud_rate=%d\n", smc_param.m_n_baud_rate);
	AUI_PRINTF("m_n_frequency=%d\n", smc_param.m_n_frequency);
	AUI_PRINTF("m_e_standard=%d\n", smc_param.m_e_standard);
	AUI_PRINTF("m_e_protocol=%d\n", smc_param.m_e_protocol);
	AUI_PRINTF("m_e_stop_bit=%d\n", smc_param.m_e_stop_bit);
	AUI_PRINTF("m_e_check_bit=%d\n", smc_param.m_e_check_bit);
	
	smc_param.m_nETU = 2;
	status = aui_smc_param_set(smc_handle, &smc_param);
	if (status != 0) {
		AUI_PRINTF("aui_smc_param_set fail\n");
		goto error;
	}
		
	status = aui_smc_close(smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_close fail\n");
		return -1;
	}

	status = aui_smc_de_init(NULL);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_de_init fail\n");
		return -1;
	}

	return 0;
error:
	aui_smc_close(smc_handle);
	return -1;
}

/*
 * The main steps:
 * 1. aui_smc_init
 * 2. aui_smc_open
 * 3. aui_smc_active
 * 4. aui_smc_reset
 * 5. aui_smc_setpps
 * 6. aui_smc_param_set
 * 7. aui_smc_transfer
 * 8. aui_smc_close
 * 9. aui_smc_de_init
 */
unsigned long testPPS()
{
	int status;
	void *smc_handle;
	aui_smc_attr smc_attr;
	unsigned char atr[MAX_ATR_LN];
	unsigned short atr_len;
	unsigned char resp[MAX_RESP_LEN];
	int resp_len;
	aui_smc_param_t smc_param;

	static unsigned char atr_exp[] =
		{0x3b, 0xb0, 0x36, 0x0, 0x81, 0x31, 0xfe, 0x5d, 0x95};

	static unsigned char c1_cmd[] = {
			0xd1, 0x28, 0x31, 0x13, 0x28, 0x43, 0x29, 0x20, 0x20, 0x63,
			0x6f, 0x6d, 0x76, 0x65, 0x6e, 0x69, 0x65, 0x6e, 0x74, 0x20,
			0x2c, 0x20, 0x62, 0x65, 0x74, 0x61, 0x63, 0x72, 0x79, 0x70,
			0x74, 0x20, 0x62, 0x79, 0x20, 0x20, 0x63, 0x6f, 0x6d, 0x76,
			0x65, 0x6e, 0x69, 0x65, 0x6e, 0x74, 0x20, 0x20, 0x32, 0x30,
			0x30, 0x33, 0x9, 0x14, 0x3, 0xff, 0x80, 0x0, 0x0, 0x0, 0xd8,
			0x12
	};
	static unsigned char c1[sizeof(c1_cmd) / sizeof(unsigned char)];
	
#if 0
	static unsigned char c1_exp[] = {
			0xd1, 0x28, 0x0b, 0x10, 0x50, 0x38, 0x57, 0x45, 0x35, 0x30,
			0x31, 0x37, 0x2d, 0x31, 0x11, 0x11, 0x42, 0x65, 0x74, 0x61,
			0x43, 0x72, 0x79, 0x70, 0x74, 0x20, 0x49, 0x49, 0x2d, 0x31,
			0x2e, 0x32, 0x0b, 0x12, 0x36, 0x31, 0x30, 0x30, 0x30, 0x30,
			0x31, 0x31, 0x32, 0x37, 0x31, 0x13, 0x28, 0x43, 0x29, 0x20,
			0x42, 0x65, 0x74, 0x61, 0x52, 0x65, 0x73, 0x65, 0x61, 0x72,
			0x63, 0x68, 0x2c, 0x20, 0x62, 0x65, 0x74, 0x61, 0x63, 0x72,
			0x79, 0x70, 0x74, 0x20, 0x62, 0x79, 0x20, 0x42, 0x65, 0x74,
			0x61, 0x52, 0x65, 0x73, 0x65, 0x61, 0x72, 0x63, 0x68, 0x20,
			0x32, 0x30, 0x30, 0x30, 0x1a, 0x0f, 0x31, 0x2e, 0x32, 0x2e,
			0x30, 0x2e, 0x39, 0x38, 0x20, 0x32, 0x30, 0x30, 0x32, 0x2d,
			0x30, 0x32, 0x2d, 0x32, 0x31, 0x20, 0x31, 0x37, 0x3a, 0x33,
			0x38, 0x00, 0x00
	};
#endif

	smc_attr.p_fn_smc_cb = fun_cb;
	smc_attr.ul_smc_id = ID;

	AUI_PRINTF("[testPPS]\n");
	status = aui_smc_init((p_fun_cb)smc_init_cb);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_init fail %d\n", status);
		return -1;
	}

	status = aui_smc_open(&smc_attr, &smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_open fail %d\n", status);
		goto error;
	}

	status = aui_smc_active(smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_active fail %d\n", status);
		goto error;
	}

	status = aui_smc_isexist(smc_handle);
	if (status != 0) {
		AUI_PRINTF("aui_smc_isexist fail %d\n", status);
		goto error;
	}

	atr_len = MAX_ATR_LN;
	AUI_PRINTF("expect atr:");
	printseq(atr_exp, sizeof(atr_exp));
	status = aui_smc_reset(smc_handle, atr, &atr_len, 1);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_reset fail\n");
		goto error;
	} else {
		if (!checkseq(atr, (int)atr_len, atr_exp, sizeof(atr_exp))) {
			AUI_PRINTF("Match the expect ATR.\n");
		} else {
			AUI_PRINTF("Don't match the expect ATR.\n");
			printseq(atr, atr_len);
			goto error;
		}
	}


	AUI_PRINTF("pps_prossing 1 \n");
	unsigned char TS0, TD1, index = 1;
	int loop, max_items;
	unsigned char tx_buffer[4], rx_buffer[256];
	int write_len, read_len;
	static int conversion_factor[] = {
			372, 372, 558, 744, 1116, 1488, 1860, -1, -1, 512, 768, 1024, 1536, 2048, -1, -1
	};
	// Di=64 is valid in ISO/IEC 7816-3:2006
	static int adjustment_factor[] = {
			-1, 1, 2, 4, 8, 16, 32, 64, 12, 20, -1, -1, -1, -1, -1, -1
	};
	// Verimatrix card valid PPS data
	static unsigned char PPS1_Valid_Value_Table[] = {
			0x95, 0xA5, 0x38, 0x94, 0xA8, 0x13, 0x34, 0xA4, 0x93, 0x12, 0x33,
			0xA3, 0x92, 0x22, 0x11, 0x32, 0xA2, 0x91, 0x21, 0x31, 0xA1
	};

	TS0 = atr[index++];
	if (TS0 & 0x10) index++;
	if (TS0 & 0x20) index++;
	if (TS0 & 0x40) index++;
	if (TS0 & 0x80) {
		TD1 = atr[index++];
		if(!(TD1 & 0x10)) {
			AUI_PRINTF("CMV PPS detected\n");
		} else {
			AUI_PRINTF("CMV PPS not detected\n");
			return 0;
		}
	} else {
		return 0;
	}

	AUI_PRINTF("pps_prossing 2 \n");
	tx_buffer[0] = 0xFF;
	tx_buffer[1] = 0x11;
	tx_buffer[2] = 0x00;
	tx_buffer[3] = 0x00;

	max_items = sizeof(PPS1_Valid_Value_Table) / sizeof(unsigned char);
	memset(&smc_param, 0, sizeof(smc_param));
	aui_smc_param_get(smc_handle, &smc_param);
	for (loop = 0; loop < max_items; loop++) {
		int i;

		tx_buffer[2] = PPS1_Valid_Value_Table[loop];
		tx_buffer[3] = 0x00;
		for (i = 0; i < 3; i++) {
			tx_buffer[3] ^= tx_buffer[i];
		}

		memset(rx_buffer, 0x00, sizeof(rx_buffer));
		write_len = 4;
		read_len = 4;

		AUI_PRINTF("TX[%02ld] => ", write_len);
		for (i = 0; i < (int)write_len; i++ )
			AUI_PRINTF("[%02X]", tx_buffer[i]);
		AUI_PRINTF("\n");

		status = aui_smc_setpps(smc_handle, tx_buffer, write_len, rx_buffer, &read_len);

		if (status != 0) {
			AUI_PRINTF("aui_smc_setpps failed[error=%d]\n", status);
		}

		AUI_PRINTF("RX[%02ld] => ", read_len);
		for( i=0; i<(int)read_len && i < 4; i++ )
			AUI_PRINTF("[%02X]", rx_buffer[i]);
		AUI_PRINTF("\n");

		if (!status && (read_len == 4) && memcmp(tx_buffer, rx_buffer, 4) == 0) {
			unsigned char FI, DI;
			int Fi, Di, etu;

			FI = (tx_buffer[2] >> 4) & 0x0F;
			DI = tx_buffer[2] & 0x0F;

			Fi = conversion_factor[FI];
			Di = adjustment_factor[DI];
			etu = Fi / Di;

			AUI_PRINTF("[0x%x] => Fi=%d Di=%d etu=%d\n", tx_buffer[2], Fi, Di, etu);
			smc_param.m_nETU = etu;
			AUI_PRINTF("etu: %d, protocol: %d\n", smc_param.m_nETU, smc_param.m_e_protocol);
			aui_smc_param_set(smc_handle, &smc_param);
			AUI_PRINTF("PPS End!! \n");

			resp_len = MAX_RESP_LEN;
			MEMCPY(c1, c1_cmd, sizeof(c1));
			MEMSET(resp, 0, resp_len);
			AUI_PRINTF("transfer:");
			printseq(c1, sizeof(c1));
			status = aui_smc_transfer(smc_handle,
					c1, sizeof(c1) / sizeof(unsigned char),
					resp, &resp_len);
			if (status != AUI_RTN_SUCCESS) {
				AUI_PRINTF("transfer fail %d\n", status);
				printseq(resp, resp_len);
			} else {
		//		if (!checkseq(resp, resp_len, c1_exp, sizeof(c1_exp))) {
				if (resp_len == 123) { // Just check resp_len
					AUI_PRINTF("Response ok.\n");
				} else {
					AUI_PRINTF("Response not match expected.\n");
				}
				AUI_PRINTF("respond:");
				printseq(resp, resp_len);
			}

			if (loop == max_items - 1) {
				break;
			}
		} else {
			goto error;
		}
	}
	AUI_PRINTF("pps_prossing 3 \n");
	status = aui_smc_close(smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_close fail\n");
		return -1;
	}

	status = aui_smc_de_init(NULL);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_de_init fail\n");
		return -1;
	}
	return 0;

error:
	aui_smc_close(smc_handle);
	return -1;
}

unsigned long test_verimatrix(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = 0;

	(void)argc;
	(void)argv;
	(void)sz_out_put;
	static unsigned char atr_exp[] =
		{0x3b, 0xb0, 0x36, 0x0, 0x81, 0x31, 0xfe, 0x5d, 0x95};

	static unsigned char c1_cmd[] = {
		0xd1, 0x28, 0x31, 0x13, 0x28, 0x43, 0x29, 0x20, 0x20, 0x63,
		0x6f, 0x6d, 0x76, 0x65, 0x6e, 0x69, 0x65, 0x6e, 0x74, 0x20,
		0x2c, 0x20, 0x62, 0x65, 0x74, 0x61, 0x63, 0x72, 0x79, 0x70,
		0x74, 0x20, 0x62, 0x79, 0x20, 0x20, 0x63, 0x6f, 0x6d, 0x76,
		0x65, 0x6e, 0x69, 0x65, 0x6e, 0x74, 0x20, 0x20, 0x32, 0x30,
		0x30, 0x33, 0x09, 0x14, 0x03, 0xff, 0x80, 0x00, 0x00, 0x00,
		0xd8, 0x12
	};
	static unsigned char c1[sizeof(c1_cmd) / sizeof(unsigned char)];
	static unsigned char c1_exp[] = {
		0xd1, 0x28, 0x0b, 0x10, 0x50, 0x38, 0x57, 0x45, 0x35, 0x30,
		0x31, 0x37, 0x2d, 0x31, 0x11, 0x11, 0x42, 0x65, 0x74, 0x61,
		0x43, 0x72, 0x79, 0x70, 0x74, 0x20, 0x49, 0x49, 0x2d, 0x31,
		0x2e, 0x32, 0x0b, 0x12, 0x36, 0x31, 0x30, 0x30, 0x30, 0x30,
		0x31, 0x31, 0x32, 0x37, 0x31, 0x13, 0x28, 0x43, 0x29, 0x20,
		0x42, 0x65, 0x74, 0x61, 0x52, 0x65, 0x73, 0x65, 0x61, 0x72,
		0x63, 0x68, 0x2c, 0x20, 0x62, 0x65, 0x74, 0x61, 0x63, 0x72,
		0x79, 0x70, 0x74, 0x20, 0x62, 0x79, 0x20, 0x42, 0x65, 0x74,
		0x61, 0x52, 0x65, 0x73, 0x65, 0x61, 0x72, 0x63, 0x68, 0x20,
		0x32, 0x30, 0x30, 0x30, 0x1a, 0x0f, 0x31, 0x2e, 0x32, 0x2e,
		0x30, 0x2e, 0x39, 0x38, 0x20, 0x32, 0x30, 0x30, 0x32, 0x2d,
		0x30, 0x32, 0x2d, 0x32, 0x31, 0x20, 0x31, 0x37, 0x3a, 0x33,
		0x38, 0x00, 0x00
	};

	static unsigned char c2_cmd[] = {
		0xd1, 0x28, 0x31, 0x13, 0x28, 0x43, 0x29, 0x20,
		0x42, 0x65, 0x74, 0x61, 0x52, 0x65, 0x73, 0x65,
		0x61, 0x72, 0x63, 0x68, 0x2c, 0x20, 0x62, 0x65,
		0x74, 0x61, 0x63, 0x72, 0x79, 0x70, 0x74, 0x20,
		0x62, 0x79, 0x20, 0x42, 0x65, 0x74, 0x61, 0x52,
		0x65, 0x73, 0x65, 0x61, 0x72, 0x63, 0x68, 0x20,
		0x32, 0x30, 0x30, 0x32, 0x09, 0x14, 0x5a, 0x02,
		0x00, 0x00, 0x00, 0x00, 0x01, 0xf5
	};
	static unsigned char c2[sizeof(c2_cmd) / sizeof(unsigned char)];
	static unsigned char c2_exp[] = {
		0xd1, 0x28, 0x0b, 0x10, 0x50, 0x38, 0x57, 0x45,
		0x35, 0x30, 0x31, 0x37, 0x2d, 0x31, 0x11, 0x11,
		0x42, 0x65, 0x74, 0x61, 0x43, 0x72, 0x79, 0x70,
		0x74, 0x20, 0x49, 0x49, 0x2d, 0x31, 0x2e, 0x32,
		0x0b, 0x12, 0x36, 0x31, 0x30, 0x30, 0x30, 0x30,
		0x31, 0x31, 0x32, 0x37, 0x31, 0x13, 0x28, 0x43,
		0x29, 0x20, 0x42, 0x65, 0x74, 0x61, 0x52, 0x65,
		0x73, 0x65, 0x61, 0x72, 0x63, 0x68, 0x2c, 0x20,
		0x62, 0x65, 0x74, 0x61, 0x63, 0x72, 0x79, 0x70,
		0x74, 0x20, 0x62, 0x79, 0x20, 0x42, 0x65, 0x74,
		0x61, 0x52, 0x65, 0x73, 0x65, 0x61, 0x72, 0x63,
		0x68, 0x20, 0x32, 0x30, 0x30, 0x30, 0x1a, 0x0f,
		0x31, 0x2e, 0x32, 0x2e, 0x30, 0x2e, 0x39, 0x38,
		0x20, 0x32, 0x30, 0x30, 0x32, 0x2d, 0x30, 0x32,
		0x2d, 0x32, 0x31, 0x20, 0x31, 0x37, 0x3a, 0x33,
		0x38, 0x00, 0x00
	};

	static unsigned char c3_cmd[] = {
		0xd1, 0x10
	};
	static unsigned char c3[sizeof(c3_cmd) / sizeof(unsigned char)];
	static unsigned char c3_exp[] = {
		0xd1, 0x10, 0x17, 0x24, 0x00, 0x00
	};

	static unsigned char c4_cmd[] = {
		0xd1, 0x0e, 0x00, 0x89, 0x58, 0xd6, 0xaa, 0xcd,
		0x5f, 0xd2, 0x56, 0xd3, 0x87, 0xb9, 0xa1, 0x8e,
		0xd2, 0x40, 0xc3, 0xde, 0xa3, 0x1a, 0xe6, 0x00,
		0xf4, 0x47, 0xc1, 0xe6, 0x39, 0x65, 0xfa, 0xbf,
		0x2d, 0xee, 0xee, 0xed, 0x32, 0x25, 0x75, 0x6f,
		0x36, 0xa1, 0xd1, 0xa9, 0xbf, 0x8c, 0x61, 0x25,
		0x25, 0xbb, 0x34, 0x3c, 0x94, 0xfa, 0x6e, 0x60,
		0xa3, 0x5b, 0x3f, 0x9e, 0xd9, 0x02, 0x36, 0xe6,
		0x89, 0x8b, 0xa1, 0x0b, 0x74, 0x1a, 0x15, 0x52,
		0x77, 0xa7, 0xe2, 0x07, 0xba, 0xa0, 0x4e
	};
	static unsigned char c4[sizeof(c4_cmd) / sizeof(unsigned char)];
	static unsigned char c4_exp[] = {
		0xd1, 0x0e, 0x7b, 0x26, 0x47, 0x76, 0xa8, 0xb1,
		0xd5, 0x3f, 0x9d, 0xbe, 0xba, 0x97, 0xc8, 0x7b,
		0xd9, 0x5d, 0x6a, 0xc1, 0x36, 0x31, 0x30, 0x30,
		0x30, 0x30, 0x31, 0x31, 0x32, 0x37, 0x00, 0x00
	};

#if 0
	static unsigned char c5_cmd[] = {
		0xd5, 0x7f, 0x4c, 0xa2, 0xfc, 0x96, 0xea, 0x90,
		0xaf, 0xfe, 0x7c, 0x5c
	};
	static unsigned char c5[sizeof(c5_cmd) / sizeof(unsigned char)];
	static unsigned char c5_exp[] = {
		0xd5, 0xfe, 0xc0, 0x6c, 0x9b, 0x0a, 0x4e, 0xba,
		0xce, 0x43, 0x23, 0x39, 0x36, 0x70, 0xa7, 0x07,
		0x35, 0x52, 0x02, 0xa8, 0x26, 0xb3, 0x00
	};

	static unsigned char c6_cmd[] = {
		0xd5, 0x71, 0xd9, 0x05, 0xac, 0x54, 0x37, 0x89,
		0xb4, 0x53, 0xa5, 0x21, 0x5a
	};
	static unsigned char c6[sizeof(c6_cmd) / sizeof(unsigned char)];
#endif

	int status;
	void *smc_handle;
	aui_smc_attr smc_attr;
	unsigned char atr[MAX_ATR_LN];
	unsigned short atr_len;
	unsigned char resp[MAX_RESP_LEN];
	int resp_len;
	//aui_smc_param_t smc_param;

	smc_attr.p_fn_smc_cb = fun_cb;
	smc_attr.ul_smc_id = ID;

	status = aui_smc_init((p_fun_cb)smc_init_cb);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_init fail %d\n", status);
		return -1;
	}

	AUI_PRINTF("SMD ID: %ld\n", smc_attr.ul_smc_id);
	status = aui_smc_open(&smc_attr, &smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_open fail %d\n", status);
		goto error;
	}

	status = aui_smc_active(smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_active fail %d\n", status);
		goto error;
	}

	status = aui_smc_isexist(smc_handle);
	if (status != 0) {
		AUI_PRINTF("aui_smc_isexist fail %d\n", status);
		goto error;
	}

	atr_len = MAX_ATR_LN;
	AUI_PRINTF("expect atr:");
	printseq(atr_exp, sizeof(atr_exp));
	status = aui_smc_reset(smc_handle, atr, &atr_len, 1);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_reset fail\n");
		goto error;
	} else {
		if (!checkseq(atr, (int)atr_len, atr_exp, sizeof(atr_exp))) {
			AUI_PRINTF("Match the expect ATR.\n");
		} else {
			AUI_PRINTF("Don't match the expect ATR.\n");
			printseq(atr, atr_len);
			goto error;
		}
	}

	resp_len = MAX_RESP_LEN;
	MEMCPY(c1, c1_cmd, sizeof(c1));
	MEMSET(resp, 0, resp_len);
	AUI_PRINTF("transfer:");
	printseq(c1, sizeof(c1));
	status = aui_smc_transfer(smc_handle, c1, sizeof(c1) / sizeof(unsigned char), resp, &resp_len);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("transfer fail %d\n", status);
		printseq(resp, resp_len);
		goto error;
	} else {
		if (resp_len != sizeof(c1_exp)) {
			AUI_PRINTF("Response Length not match expected.\n");
		}
		AUI_PRINTF("respond:");
		printseq(resp, resp_len);
	}

	resp_len = MAX_RESP_LEN;
	MEMCPY(c2, c2_cmd, sizeof(c2));
	MEMSET(resp, 0, resp_len);
	AUI_PRINTF("transfer:");
	printseq(c2, sizeof(c2));
	status = aui_smc_transfer(smc_handle, c2, sizeof(c2) / sizeof(unsigned char), resp, &resp_len);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("transfer fail %d\n", status);
		printseq(resp, resp_len);
		goto error;
	} else {
		if (resp_len != sizeof(c2_exp)) {
			AUI_PRINTF("Response Length not match expected.\n");
		}
		AUI_PRINTF("respond:");
		printseq(resp, resp_len);
	}

	resp_len = MAX_RESP_LEN;
	MEMCPY(c3, c3_cmd, sizeof(c3));
	MEMSET(resp, 0, resp_len);
	AUI_PRINTF("transfer:");
	printseq(c3, sizeof(c3));
	status = aui_smc_transfer(smc_handle, c3, sizeof(c3) / sizeof(unsigned char), resp, &resp_len);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("transfer fail %d\n", status);
		printseq(resp, resp_len);
		goto error;
	} else {
		if (resp_len != sizeof(c3_exp)) {
			AUI_PRINTF("Response Length not match expected.\n");
		}
		AUI_PRINTF("respond:");
		printseq(resp, resp_len);
	}

	resp_len = MAX_RESP_LEN;
	MEMCPY(c4, c4_cmd, sizeof(c4));
	MEMSET(resp, 0, resp_len);
	AUI_PRINTF("transfer:");
	printseq(c4, sizeof(c4));
	status = aui_smc_transfer(smc_handle, c4, sizeof(c4) / sizeof(unsigned char), resp, &resp_len);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("transfer fail %d\n", status);
		printseq(resp, resp_len);
		goto error;
	} else {
		if (resp_len != sizeof(c4_exp)) {
			AUI_PRINTF("Response Length not match expected.\n");
		}
		AUI_PRINTF("respond:");
		printseq(resp, resp_len);
	}

	status = aui_smc_close(smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_close fail\n");
		return -1;
	}

	status = aui_smc_de_init(NULL);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_de_init fail\n");
		return -1;
	}

	testPPS();

	return ret;
error:
	aui_smc_close(smc_handle);
	return -1;
}

/*-----------------------------------------------------------------------------
  * Function : check_ATR_bytes
  * Desc     : checking  the received ATR bytes of T=0 and T=14 protocol transactions.
  * Param    : ctx: unit test context.
  *          recvBuffer: contains received bytes after card reset.
  * Return   : None
  * --------------------------------------------------------------------------*/
static int check_irdeto_atr_bytes(void* recvBuffer)
{
    unsigned char *precvBuf = NULL;
    unsigned int loop_var, trans_var, history_length;
    unsigned char expected_arr[] = "IRDETO ACS ";
    int T = -1;
    precvBuf = (unsigned char *)recvBuffer;

    T = (precvBuf[3] & 0x0F);

    if ((precvBuf[3] & 0x0F) == 0)
    {
        precvBuf++;

        trans_var = (unsigned int)(*precvBuf & 0xF0);
        history_length = (unsigned int)(*precvBuf & 0x0F);
        precvBuf++;

        if(trans_var  == 0x90)
        {
            precvBuf++;

            if(0x40 == (unsigned int)(*precvBuf & 0xF0))
            {
                precvBuf++;
            }
            precvBuf++;
        }

        if(history_length != 0)
        {
            for(loop_var=0;loop_var<11;loop_var++)
            {
                if( *precvBuf != expected_arr[loop_var])
                {
                    goto error;
                }
                precvBuf++;
            }
        }
    }
    else if((precvBuf[3] & 0x0F) == 0x0E)
    {
        precvBuf++;

        trans_var = (unsigned int)(*precvBuf & 0xF0);
        history_length = (unsigned int)(*precvBuf & 0x0F);
        precvBuf++;

        if(trans_var  == 0x90)
        {
            precvBuf++;
            precvBuf++;
        }

        if(history_length != 0)
        {
            for(loop_var=0;loop_var<11;loop_var++)
            {
                if( *precvBuf != expected_arr[loop_var])
                {
                    goto error;
                }
                precvBuf++;
            }
        }
    } else {
        goto error;
    }

    return (T);

error:
    return -1;
}

#define RECV_MAX_LENGTH 256

static int tst_T0_transaction(void *smc_handle)
{
  unsigned char recvBuffer[RECV_MAX_LENGTH];
  int recvDataLength=0;

  int T0WriteCmdLen = 6;
  int T0ReadCmdLen = 5;
  unsigned char T0WriteCmdData[] = {0xD2, 0x00, 0x00, 0x00, 0x01, 0x3C};
  unsigned char T0ReadCmdData[] = {0xD2, 0xFE, 0x00, 0x00, 0x1D};
  int T0WriteAnswerLen = 2;
  int T0ReadAnswerLen = 0x1D + 2; // payload + SW1 SW2
  unsigned char T0WriteAnswerData[] = {0x90, 0x1D};
  unsigned char T0ReadAnswerData[] = {0x90, 0x00}; // SW1 & SW2 only, payload depends on the smartcard
  int ret = 0;

  /* T=0 write transaction */
  recvDataLength = RECV_MAX_LENGTH;
  ret = aui_smc_transfer(smc_handle,
          T0WriteCmdData, T0WriteCmdLen,
          recvBuffer, &recvDataLength);

  AUI_PRINTF("Transfer data: ");
  printseq(T0WriteCmdData, T0WriteCmdLen);
  AUI_PRINTF("Received data: ");
  printseq(recvBuffer, recvDataLength);
  AUI_PRINTF("Expected data: ");
  printseq(T0WriteAnswerData, T0WriteAnswerLen);

  if (ret || recvDataLength != T0WriteAnswerLen) {
    AUI_PRINTF("aui_smc_transfer error\n");
    goto error;
  }

  if (memcmp(recvBuffer, T0WriteAnswerData, T0WriteAnswerLen) != 0) {
      AUI_PRINTF("For T=0 write transaction, respond is not as expected\n");
      goto error;
  }

  /* T=0 read transaction */
  recvDataLength = RECV_MAX_LENGTH;
  ret = aui_smc_transfer(smc_handle,
          T0ReadCmdData, T0ReadCmdLen,
          recvBuffer, &recvDataLength);

  AUI_PRINTF("Transfer data: ");
  printseq(T0ReadCmdData, T0ReadCmdLen);
  AUI_PRINTF("Received data: ");
  printseq(recvBuffer,recvDataLength);
  AUI_PRINTF("Expected to end with: ");
  printseq(T0ReadAnswerData, 2);

  /* check transaction answer */
  if (ret || recvDataLength != T0ReadAnswerLen) {
      AUI_PRINTF("For T=0 read transaction, length of received data is not as expected");
      AUI_PRINTF("received length=%u, expected=%u)\n", recvDataLength, T0ReadAnswerLen);
    goto error;
  }
  /* check that received data ends with 0x90 0x00 */
  if (memcmp(((char*)recvBuffer) + (T0ReadAnswerLen-2), T0ReadAnswerData, 2) != 0) {
      AUI_PRINTF("For T=0 read transaction, received data is not as expected");
    goto error;
  }

  AUI_PRINTF("Transfer OK\n");
  return 0;

error:
  return -1;
}

static int tst_T14_transaction(void *smc_handle)
{
    unsigned char recvBuffer[RECV_MAX_LENGTH];
    int recvDataLength=0;
    char ExORofRcvdata=0x0;
    int i=0;
    int ret = 0;

    int T14CmdLen = 7;
    unsigned char T14CmdData[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x3C};
    int T14AnswerHeaderLen = 8;
    int T14AnswerPayloadLen = 20;
    int T14AnswerLen = T14AnswerPayloadLen + T14AnswerHeaderLen + 1;
    unsigned char T14AnswerHeaderData[] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14};
    unsigned char T14AnswerChecksum = 0x3F;

    recvDataLength = RECV_MAX_LENGTH;
    ret = aui_smc_transfer(smc_handle,
            T14CmdData, T14CmdLen,
            recvBuffer, &recvDataLength);

    AUI_PRINTF("Perform a T=14 transaction with the following input data: ");
    printseq(T14CmdData, T14CmdLen);
    AUI_PRINTF("Received data: (length=%d)\n", recvDataLength);
    printseq(recvBuffer, recvDataLength);

    /* check transaction answer */
    if (ret || recvDataLength != T14AnswerLen) {
        AUI_PRINTF("For T=14 transaction, length of received data is not as expected "
                "(received length=%u, expected=%u)", recvDataLength, T14AnswerLen);
        goto error;
    }
    if (memcmp(recvBuffer, T14AnswerHeaderData, T14AnswerHeaderLen) != 0) {
        AUI_PRINTF("For T=14 transaction, header of received data is not as expected");
        AUI_PRINTF("Header of received data: %s");
        printseq(recvBuffer, T14AnswerHeaderLen);
        AUI_PRINTF("Expected header data: ");
        printseq(T14AnswerHeaderData, T14AnswerHeaderLen);
        goto error;
    }

    /* verify checksum */
    for ( i=0;i<recvDataLength;i++) {
        ExORofRcvdata ^= *(((char *)recvBuffer)+i);
    }
    if (ExORofRcvdata != T14AnswerChecksum) {
        AUI_PRINTF("For T=14 transaction, checksum of received data is not as expected: "
                "received=0x%08x, expected=0x%08x", ExORofRcvdata, T14AnswerChecksum);
    }

    AUI_PRINTF("For T=14 transaction, received data is as expected\n");
    AUI_PRINTF("Transfer OK\n");
    return 0;

    error:
    return -1;
}

#define IRDETO_T14_ETU (620) // Irdeto T=14 ETU
unsigned long test_irdeto(unsigned long *argc,char **argv,char *sz_out_put)
{
    int ret = 0;

    (void)argc;
    (void)argv;
    (void)sz_out_put;

    int status;
    void *smc_handle;
    aui_smc_attr smc_attr;
    unsigned char atr[MAX_ATR_LN];
    unsigned short atr_len;
    //unsigned char resp[MAX_RESP_LEN];
    //int resp_len;
    aui_smc_param_t smc_param;
    int t = -1;

    smc_attr.p_fn_smc_cb = fun_cb;
    smc_attr.ul_smc_id = ID;

    status = aui_smc_init((p_fun_cb)smc_init_cb);
    if (status != AUI_RTN_SUCCESS) {
        AUI_PRINTF("aui_smc_init fail\n");
        return -1;
    }

    AUI_PRINTF("SMD ID: %ld\n", smc_attr.ul_smc_id);
    status = aui_smc_open(&smc_attr, &smc_handle);
    if (status != AUI_RTN_SUCCESS) {
        AUI_PRINTF("aui_smc_open fail\n");
        goto exit;
    }

    status = aui_smc_active(smc_handle);
    if (status != AUI_RTN_SUCCESS) {
        AUI_PRINTF("aui_smc_active fail\n");
        goto exit;
    }

    atr_len = MAX_ATR_LN;
    status = aui_smc_reset(smc_handle, atr, &atr_len, 1);
    if (status != AUI_RTN_SUCCESS) {
        AUI_PRINTF("IRDETO T0 aui_smc_reset fail\n");
        memset(atr, 0, MAX_ATR_LN);
        memset(&smc_param, 0, sizeof(aui_smc_param));
        smc_param.m_nETU = IRDETO_T14_ETU;
        status = aui_smc_param_set(smc_handle, &smc_param);
        AUI_PRINTF("Set Protocol: %d ETU %d %s\n", smc_param.m_e_protocol,
                smc_param.m_nETU, status ? "Fail" : "OK");
        AUI_PRINTF("Try to reset the T=14 IRDETO smart card\n");
        status = aui_smc_reset(smc_handle, atr, &atr_len, 1);
        if (status) {
            AUI_PRINTF("IRDETO T14 aui_smc_reset fail\n");
            ret = -1;
            goto exit;
        }
    }

    printseq(atr, atr_len);
    t = check_irdeto_atr_bytes(atr);
    AUI_PRINTF("T=%d\n", t);
    if (t >= 0) {
        AUI_PRINTF("Match the expect ATR.\n");
    } else {
        AUI_PRINTF("Don't match the expect ATR.\n");
        ret = -1;
        goto exit;
    }

    if (t == 0) {
        AUI_PRINTF("tst_T0_transaction\n");
        ret = tst_T0_transaction(smc_handle);
    } else if (t == 14) {
        AUI_PRINTF("tst_T14_transaction\n");
        ret = tst_T14_transaction(smc_handle);
    } else {
        ret = -1;
        AUI_PRINTF("Unknown protocol T=%d\n", t);
    }

exit:
    aui_smc_close(smc_handle);
    aui_smc_de_init(NULL);
    return ret;
}

unsigned long test_reset(unsigned long *argc,char **argv,char *sz_out_put)
{
	int status;
	void *smc_handle;
	aui_smc_attr smc_attr;
//	unsigned char atr[MAX_ATR_LN];
//	unsigned short atr_len;
	int ret = 0;

    #ifdef AUI_LINUX
	    if(*argc == 1) {
           disable_pps_flag = atoi(argv[0]); 
         } 
    #endif

	status = aui_smc_init((p_fun_cb)smc_init_cb);	
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_init fail\n");
		return -1;
	}

	smc_attr.p_fn_smc_cb = fun_cb_reset;
	smc_attr.ul_smc_id = ID;

	g_plug_in_count = 0;
	g_plug_out_count = 0;
	AUI_PRINTF("SMD ID: %ld\n", smc_attr.ul_smc_id);
	status = aui_smc_open(&smc_attr, &smc_handle);
	if (status != AUI_RTN_SUCCESS) {
		AUI_PRINTF("aui_smc_open fail\n");
		goto error;
	}
	AUI_PRINTF("aui_smc_open OK %p\n", smc_handle);

	int i = 0;
	int sleep_interval = 5;
	int sleep_time = 2;
	for (i = 1; i <= sleep_time; i++) {
		AUI_PRINTF("\nWait plug in event, then reset card\n");
		AUI_PRINTF("%d / %ds\n", i * sleep_interval, sleep_time * sleep_interval);
		AUI_SLEEP(sleep_interval * 1000);
	}

	AUI_PRINTF("\n Test done ........................................\n\n");
	AUI_PRINTF("Plug in: %d times, Plug out: %d times\n",
			g_plug_in_count, g_plug_out_count);
	printseq(g_atr, g_atr_len);
	AUI_PRINTF("\n");
	AUI_PRINTF(">>>> Please confirm whether ATR and plug event is correct!\n");

	aui_smc_close(smc_handle);
	return ret;
error:
	aui_smc_close(smc_handle);
	return -1;
}

static unsigned long test_smc_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nSMC Test Help");

	/*SMC_1_HELP*/
	#define 	SMC_1_HELP_PART1		"please using the conax card to do the smc test\n"
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Test the SMC conax card\n");
	aui_print_help_instruction_newline(SMC_1_HELP_PART1);

	/*SMC_2_HELP*/
	#define 	SMC_2_HELP_PART1		"please using the verimatrix card to do the smc test\n"
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("Test the SMC verimatrix card\n");
	aui_print_help_instruction_newline(SMC_2_HELP_PART1);

	/*SMC_3_HELP*/
	#define 	SMC_3_HELP_PART1		"please using the irdeto card to do the smc test\n"
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline("Test the SMC irdeto card\n");
	aui_print_help_instruction_newline(SMC_3_HELP_PART1);
	
	
	/*SMC_r_HELP*/
	#define 	SMC_r_HELP_PART1		"SMC test rest and select disable pps or enable pps in reset stage \n"
	#define 	SMC_r_HELP_PART2		"Example :\n"
	#define 	SMC_r_HELP_PART3		"    disable pps  Enter \"r 1\"\n"
	#define 	SMC_r_HELP_PART4		"    enable pps   Enter \"r 0\"\n"
	aui_print_help_command("\'r\'");
	aui_print_help_instruction_newline("Test the SMC RESET\n");
	aui_print_help_instruction_newline(SMC_r_HELP_PART1);
	aui_print_help_instruction_newline(SMC_r_HELP_PART2);
	aui_print_help_instruction_newline(SMC_r_HELP_PART3);
	aui_print_help_instruction_newline(SMC_r_HELP_PART4);

	return AUI_RTN_HELP;
}
void smc_test_reg()
{
	aui_tu_reg_group("smc", "smc tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_conax, "conax card");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_verimatrix, "verimatrix card");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_irdeto, "irdeto card");
	aui_tu_reg_item(2, "r", AUI_CMD_TYPE_API, test_reset, "Reset test");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_smc_help, "smc test help");
}

