/**@file
 *    @brief     ALi AUI NIM function implementation
 *    @author    romain.baeriswyl
 *    @date      2014-02-04
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <semaphore.h>

#include <aui_common.h>
#include "aui_common_priv.h"
#include <aui_nim.h>
#include <alislnim.h>
#include <aui_gpio.h>


AUI_MODULE(NIM)

/* to be removed when present in aui_nim.h */
#ifndef NIM_HANDLE_MAX
#define NIM_HANDLER_MAX		6
#endif

#define AUTO_SCAN_TIMEOUT (300)

struct nim_sat_param {
	int tone_active;
	int lnb_requested;
};

struct nim_ter_param {
	int std; /* enum aui_ter_std */
};

/* Internal device structure  */
struct nim_handler {
	struct aui_st_dev_priv_data data;

	unsigned int demod_type; /* enum aui_demod_type */
	alisl_handle dev;

	aui_hdl gpio_reset, gpio_lnb;

	union {
		struct nim_sat_param sat;
		struct nim_ter_param ter;
	} u;
};
struct nim_gpio{
	int position;
	int set_cnt;
};

static int g_num_nims = 0;
static struct aui_nim_config *g_nim_config = NULL;
static struct nim_handler *g_nim_handler = NULL;

AUI_RTN_CODE aui_nim_init(p_fun_cb call_back)
{
#ifdef CONFIG_ALI_EMULATOR

    return (call_back) ? AUI_RTN_SUCCESS:AUI_RTN_SUCCESS; // use call_back to avoid compile warning
#else
	/* allocate handlers */
	if (g_nim_handler || g_nim_config){
		AUI_ERR("already initialized!\n");
		return 0;
	}

	aui_gpio_init(NULL, NULL);
	g_num_nims = 0;
	g_nim_handler = calloc(NIM_HANDLER_MAX * sizeof(struct nim_handler), 1);
	if (!g_nim_handler)
		aui_rtn(AUI_RTN_FAIL, "calloc error");

	int i;
	for (i=0; i<NIM_HANDLER_MAX; i++)
		(g_nim_handler + i)->data.dev_idx = -1;

	/* allocate configuration structure */
	g_nim_config = calloc(NIM_HANDLER_MAX * sizeof(struct aui_nim_config), 1);
	if (!g_nim_config)
		aui_rtn(AUI_RTN_FAIL, "calloc error");

	/* fill demod and tuner parameters for both NIM instances */
	if (call_back) {
		if (call_back(g_nim_config))
			aui_rtn(AUI_RTN_FAIL, "g_nim_config error");

		struct aui_nim_config tmp;
		memset(&tmp, 0, sizeof(struct aui_nim_config));

		for (i = 0; i < NIM_HANDLER_MAX; i++)
			if (memcmp(&tmp, g_nim_config + i, sizeof(struct aui_nim_config)) == 0)
				break;
		AUI_DBG("got %d NIMs configuration\n", i);
		g_num_nims = i;
	}
	return AUI_RTN_SUCCESS;
#endif    
}

AUI_RTN_CODE aui_nim_open(struct aui_nim_attr *attr, aui_hdl *handle)
{
	if (!attr || !handle || (int)attr->ul_nim_id >= g_num_nims ||
	    !g_nim_handler || !g_nim_config) {
		aui_rtn(AUI_RTN_FAIL, NULL);
	}

	if (attr->en_dmod_type != AUI_NIM_QPSK &&
			attr->en_dmod_type != AUI_NIM_QAM &&
			attr->en_dmod_type != AUI_NIM_OFDM)
		aui_rtn(AUI_RTN_FAIL, NULL);

	struct nim_handler *hdl = g_nim_handler + attr->ul_nim_id;
	struct aui_nim_config *nim = g_nim_config + attr->ul_nim_id;
	// DVB-S and DVB-T tuner should have the ability of output voltage
	if (!nim->disable_auto_lnb_power_control) {
		if (AUI_GPIO_NONE != nim->lnb_power_gpio.position
			&& (attr->en_dmod_type == AUI_NIM_QPSK
				|| attr->en_dmod_type == AUI_NIM_OFDM))
		{
			aui_gpio_attr gpio_power;
			memset(&gpio_power, 0, sizeof(aui_gpio_attr));
			gpio_power.io = nim->lnb_power_gpio.io;
			gpio_power.value_out = nim->lnb_power_gpio.gpio_val;
			gpio_power.uc_dev_idx = nim->lnb_power_gpio.position;
			aui_gpio_open(&gpio_power, &hdl->gpio_lnb);
			aui_gpio_set_value(hdl->gpio_lnb, nim->lnb_power_gpio.gpio_val);
		}
	}
	if (AUI_GPIO_NONE != nim->nim_reset_gpio.position) {
		int shm_id, sem_val=0;
		key_t shm_key;
		char *shm_addr=NULL;
		sem_t *sem;
		int ret = 0;

		sem = sem_open("/etc", O_CREAT, 0644, 1);
		sem_getvalue(sem, &sem_val);
		AUI_DBG("sem_val = %d\n", sem_val);

		shm_key = ftok("/etc", 222);  //multi_process use same key to trace same memery
		if(-1 == shm_key)
		{
			AUI_ERR("get shm_key error!\n");
			return AUI_RTN_FAIL;
		}
		AUI_DBG("shm_key = %d\n", shm_key);
		shm_id = shmget(shm_key, 1, IPC_CREAT|0644); // alloc 1 byte
		if(-1 == shm_id)
		{
			AUI_ERR("get shm_id error!\n");
			return AUI_RTN_FAIL;
		}
		AUI_DBG("shm_id = %d\n", shm_id);

		shm_addr = (char *)shmat(shm_id, NULL, 0);
		if((void*)-1 == shm_addr)
		{
			AUI_ERR("get shm_addr error!\n");
			return AUI_RTN_FAIL;
		}
		AUI_DBG("shm_addr = 0x%p\n", shm_addr);
		AUI_DBG("before share mem value = %d\n", *shm_addr);

		sem_wait(sem);

		if (0 == (*shm_addr & (1<<attr->ul_nim_id))) {
			aui_gpio_attr gpio_reset;
			memset(&gpio_reset, 0, sizeof(aui_gpio_attr));
			gpio_reset.io = nim->nim_reset_gpio.io;
			gpio_reset.uc_dev_idx = nim->nim_reset_gpio.position;
			if (aui_gpio_open(&gpio_reset, &hdl->gpio_reset) || !hdl->gpio_reset) {
				AUI_ERR("aui_gpio_open error!\n");
				ret = -1;
			} else {
				// set low power to reset demod
				aui_gpio_set_value(hdl->gpio_reset, nim->nim_reset_gpio.gpio_val);

				usleep(200000); /* 200 ms */

				aui_gpio_set_value(hdl->gpio_reset, !(nim->nim_reset_gpio.gpio_val));
				aui_gpio_close(hdl->gpio_reset);

				*shm_addr |= 1 << attr->ul_nim_id; /* save nim id */
			}
		}

		sem_post(sem);

		AUI_DBG("after share mem value = %d\n", *shm_addr);

		if (ret)
			return AUI_RTN_FAIL;

		if (shmdt((void*)shm_addr)) {
			AUI_ERR("shdt faild!\n");
			return AUI_RTN_FAIL;
		}

		sem_close(sem);
	}

	hdl->data.dev_idx = attr->ul_nim_id;
	hdl->demod_type = attr->en_dmod_type;
	if (hdl->demod_type == AUI_NIM_OFDM)
		hdl->u.ter.std = attr->en_ter_std;

	AUI_DBG("device %d configure and open nim id %d\n",
		 (int)attr->ul_nim_id, (int)nim->id);

	if (alislnim_open(&hdl->dev, nim->id)) {
		hdl->dev = NULL;
		aui_nim_close(hdl);
		aui_rtn(AUI_RTN_FAIL, "alislnim_open error");
	}

	/* Set the DVBT standard: ISDB-T, DVB-T, DVB-T2 or DVB-T2 combo */
	if (attr->en_dmod_type == AUI_NIM_OFDM) {
		nim->config.dvbt.tuner.std = attr->en_ter_std;
		AUI_DBG("ter std %d\n", attr->en_ter_std);
	}

	if (attr->en_dmod_type == AUI_NIM_QAM) {
		nim->config.dvbc.demod.qam_mode = attr->en_ter_std;
	}

	if (alislnim_set_cfg(hdl->dev, (void *)nim)) {
		aui_nim_close(hdl);
		aui_rtn(AUI_RTN_FAIL, "set_cfg error");
	}

	*handle = (aui_hdl *)hdl;
	aui_dev_reg(AUI_MODULE_NIM, hdl);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_handle_get_byid(unsigned int id, aui_hdl *handle)
{
	if(!handle || (int)id >= g_num_nims)
		aui_rtn(AUI_RTN_FAIL,NULL);

	return aui_find_dev_by_idx(AUI_MODULE_NIM, (unsigned long)id, handle);
}

AUI_RTN_CODE aui_nimid_get_by_handle(aui_hdl handle, unsigned int *id)
{
	struct nim_handler *hdl;
	aui_hdl aui_nim_hdl = NULL;

	if(!id || !handle)
		aui_rtn(AUI_RTN_FAIL,NULL);

	hdl = (struct nim_handler *)handle;
	if (hdl->dev == NULL) {
		// NIM is not opened
		aui_rtn(AUI_RTN_FAIL,NULL);
	}

	*id = ((struct nim_handler *)handle)->data.dev_idx;

	// Double check if the NIM is opened
	if (aui_find_dev_by_idx(AUI_MODULE_NIM, (unsigned long)*id, &aui_nim_hdl)
			!= AUI_RTN_SUCCESS) {
		// NIM is not opened
		aui_rtn(AUI_RTN_FAIL,NULL);
	}
	if (handle != aui_nim_hdl) {
		AUI_ERR("NIM handle error\n");
		aui_rtn(AUI_RTN_FAIL,NULL);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_connect(aui_hdl handle,
			     struct aui_nim_connect_param *param)
{
	struct nim_handler* hdl = (struct nim_handler *)handle;
	struct aui_sat_param *sat;
	struct slnim_freq_lock nim_param;
	aui_nim_22k_e status_22k;

	if (!hdl || !param)
		aui_rtn(AUI_RTN_FAIL,NULL);

	if (param->en_demod_type != hdl->demod_type)
		aui_rtn(AUI_RTN_FAIL,NULL);

	struct aui_nim_config *config = g_nim_config + hdl->data.dev_idx;
	/* check dvbs frequency */
	if (AUI_NIM_QPSK == param->en_demod_type) {
		struct aui_tuner_dvbs_config *p_tuner = &config->config.dvbs.tuner;
		if (param->ul_freq < p_tuner->freq_low ||
				param->ul_freq > p_tuner->freq_high) {
			AUI_ERR("connect param invalid, %d < freq %d < %d\n",
				p_tuner->freq_low, param->ul_freq, p_tuner->freq_high);
			aui_rtn(AUI_RTN_EINVAL, NULL);
		}
	}

	memset(&nim_param, 0, sizeof(struct slnim_freq_lock));
	nim_param.freq = param->ul_freq;

	switch (hdl->demod_type) {
	case AUI_NIM_QPSK: /* DVB-S/S2 */

		sat = &param->connect_param.sat;
		/* Check parameters */
//		if ((sat->ul_src & ~1) || (sat->ul_polar & ~1) ||  (sat->ul_freq_band & ~1)) {
//			aui_rtn(AUI_RTN_EINVAL, "connect param invalid");
//		}

//		if(NULL != hdl->gpio_lnb)
//			aui_gpio_set_value(hdl->gpio_lnb, config->lnb_power_gpio.gpio_val);
		nim_param.sym = sat->ul_symrate;
		nim_param.fec = sat->fec;

		// If ul_freq_band is invalid do not do the following works.
		if (sat->ul_freq_band == AUI_NIM_LOW_BAND
				|| sat->ul_freq_band == AUI_NIM_HIGH_BAND) {
			/* Set polarity */
			if (!(sat->ul_polar & ~1)) {
				if (aui_nim_set_polar(hdl, sat->ul_polar)) {
					aui_rtn(AUI_RTN_FAIL, "set polar failed");
				}
			}

			AUI_DBG("DVBS freq %d symb %d polar %d band %d\n",
					(int)param->ul_freq, (int)sat->ul_symrate,
					(int)sat->ul_polar, (int)sat->ul_freq_band);
#if 1
			usleep(20 * 1000); // See DiSEqC Bus Spec, > 15ms

			slnim_diseqc_operate_t diseqc_param;
			memset(&diseqc_param, 0, sizeof(diseqc_param));

			/* Select satellite with tone burst */
			diseqc_param.mode = (sat->ul_src == 0) ?
					SLNIM_DISEQC_MODE_BURST0 : SLNIM_DISEQC_MODE_BURST1;
			if (alislnim_diseqc_operate(hdl->dev, &diseqc_param))
				aui_rtn(AUI_RTN_FAIL, "select sat failed");

			usleep(20 * 1000); // See DiSEqC Bus Spec, > 15ms

			/* Set tone: activated for high band */
//			if (sat->ul_freq_band == AUI_NIM_LOW_BAND
//					|| sat->ul_freq_band == AUI_NIM_HIGH_BAND) {
				status_22k = (sat->ul_freq_band == AUI_NIM_HIGH_BAND)
						? AUI_NIM_22K_ON : AUI_NIM_22K_OFF;
				if (aui_nim_set22k_onoff(hdl, status_22k)) {
					aui_rtn(AUI_RTN_FAIL, "set tone failed");
				}
//			}

#else
			/* Send DiSEqC message */
			unsigned char msg[] = { 0xe0 /* without reply */,
					0x10 /* any LNB */,
					0x38 /* write to port group 0 */,
					0xf0, 0x00, 0x00 };
			/* msg[3]
			 * bit 0: 0 = low band, 1 = high band
			 * bit 1: 0 = 13V, 1 = 18V
			 * bit 2: 0 = satellite A, 1 = satellite B
			 */
			msg[3] |= sat->ul_freq_band |
					(sat->ul_polar << 1) |
					(sat->ul_src << 2);
			if (aui_diseqc_oper(hdl, AUI_DISEQC_MODE_BYTES, msg, 6, NULL, NULL))
				AUI_ERR("aui_diseqc_oper 1.0 error\n");
#endif
		}
		break;

	case AUI_NIM_QAM: /* DVB-C */
		nim_param.sym = param->connect_param.cab.ul_symrate;
		nim_param.fec = param->connect_param.cab.en_qam_mode;
		switch (param->connect_param.cab.bandwidth) {
		    // DVB-C bandwidth need to be set in some special situations.
		    // For example, J83A with 6MHz band.
		    case AUI_NIM_BANDWIDTH_6_MHZ:
		        nim_param.bandwidth = 6;
		        break;
		    case AUI_NIM_BANDWIDTH_7_MHZ:
		        nim_param.bandwidth = 7;
		        break;
		    case AUI_NIM_BANDWIDTH_8_MHZ:
		        nim_param.bandwidth = 8;
		        break;
		    default:
		        nim_param.bandwidth = 0;
		}
		switch (param->connect_param.cab.en_qam_mode) {
		case AUI_NIM_NOT_DEFINED: nim_param.modulation = SLNIM_QAM_AUTO; break;
		case AUI_NIM_QAM16: nim_param.modulation = SLNIM_QAM16; break;
		case AUI_NIM_QAM32: nim_param.modulation = SLNIM_QAM32; break;
		case AUI_NIM_QAM64: nim_param.modulation = SLNIM_QAM64; break;
		case AUI_NIM_QAM128: nim_param.modulation = SLNIM_QAM128; break;
		case AUI_NIM_QAM256: nim_param.modulation = SLNIM_QAM256; break;
		default:
			AUI_DBG("unsupported modulation %d\n",
				 param->connect_param.cab.en_qam_mode);
			aui_rtn(AUI_RTN_FAIL,NULL);
		}

		AUI_DBG("DVBC freq %d symb %d qam %d bandwidth %d\n", nim_param.freq,
			 nim_param.sym, param->connect_param.cab.en_qam_mode, nim_param.bandwidth);

		break;

	case AUI_NIM_OFDM: /* DVB-T, ISDB-T, DVB-T2 */
		/* check requested standard */
		switch (hdl->u.ter.std) {
		case AUI_STD_ISDBT:
		case AUI_STD_DVBT:
		case AUI_STD_DVBT2:
			if (param->connect_param.ter.std != hdl->u.ter.std)
				aui_rtn(AUI_RTN_FAIL,"wrong standard");
			break;
		case AUI_STD_DVBT2_COMBO:
			if (param->connect_param.ter.std != AUI_STD_DVBT &&
			    param->connect_param.ter.std != AUI_STD_DVBT2 &&
			    param->connect_param.ter.std != AUI_STD_DVBT2_COMBO)
				aui_rtn(AUI_RTN_FAIL,"wrong standard");
			break;
		}
		nim_param.bandwidth = param->connect_param.ter.bandwidth;
		nim_param.standard = param->connect_param.ter.std;
		nim_param.guard_interval = param->connect_param.ter.guard_interval;
		nim_param.fft_mode = param->connect_param.ter.fft_mode;
		nim_param.modulation = param->connect_param.ter.modulation;
		nim_param.fec = param->connect_param.ter.fec;
		nim_param.inverse = param->connect_param.ter.spectrum;

		if (nim_param.standard == AUI_STD_DVBT2 ||
		    nim_param.standard == AUI_STD_DVBT2_COMBO) {
			// Auto connect the first PLP when plp_index is AUI_NIM_PLP_AUTO
			// Auto connect the next PLP when plp_index is AUI_NIM_PLP_SCAN_NEXT
			// Connect the plp_index PLP when plp_index is 0~255 with M3823
			// Connect the plp_id PLP when plp_id is 0~255 with M3627
			nim_param.plp_index = param->connect_param.ter.u.dvbt2.plp_index;
			nim_param.plp_id = param->connect_param.ter.u.dvbt2.plp_id;
		}

		AUI_DBG("DVBT freq %d bw %d std %d fec %d spect %d\n", nim_param.freq,
			 nim_param.bandwidth, nim_param.standard, nim_param.fec, nim_param.inverse);
		break;

	default:
		AUI_DBG("unsupported demod type %d\n", hdl->demod_type);
		aui_rtn(AUI_RTN_FAIL, NULL);
	}

	return alislnim_freqlock(hdl->dev, &nim_param);
}

AUI_RTN_CODE aui_nim_is_lock(aui_hdl handle, int *lock)
{
	struct nim_handler *hdl = (struct nim_handler *)handle;

	if (!hdl || !lock)
		aui_rtn(AUI_RTN_FAIL, NULL);

	if (alislnim_get_lock(hdl->dev, (unsigned int *)lock))
		aui_rtn(AUI_RTN_FAIL, NULL);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_close(aui_hdl handle)
{
	struct nim_handler *hdl = (struct nim_handler *)handle;
	int err = AUI_RTN_SUCCESS;

	if(!hdl)
		return AUI_RTN_SUCCESS;

	if (hdl->dev) {

		if (hdl->demod_type == AUI_NIM_QPSK) {
			if (hdl->u.sat.tone_active) {
				aui_nim_set22k_onoff(hdl, AUI_NIM_22K_OFF);
			}

		}
#if 0 /* not supported yet by kernel */
		if (alislnim_ioctl(hdl->dev, NIM_IOCMD_TUNER_POWER_CONTROL, 0))
			AUI_ERR("tuner standby failed\n");
#endif

		if (alislnim_close(hdl->dev)) {
			err = AUI_RTN_FAIL;
			AUI_ERR("sl close failed\n");
			return err;
		}
		struct aui_nim_config *nim = g_nim_config + hdl->data.dev_idx;
		if (!nim->disable_auto_lnb_power_control) {
			if(NULL != hdl->gpio_lnb) {
				// Close the output voltage of tuner
				aui_gpio_set_value(hdl->gpio_lnb, !nim->lnb_power_gpio.gpio_val);
				aui_gpio_close(hdl->gpio_lnb);
			}
		}
	}
	if (aui_dev_unreg(AUI_MODULE_NIM, hdl)) {
		err = AUI_RTN_FAIL;
		AUI_ERR("aui_dev_unreg error\n");
	}

	memset(hdl, 0, sizeof(struct nim_handler));
	hdl->data.dev_idx = -1;
	return err;
}

AUI_RTN_CODE aui_nim_de_init(p_fun_cb call_back)
{
	/* check that all instances are closed */
	aui_gpio_deinit();

	if (call_back)
		call_back(g_nim_config);

	if (g_nim_handler) {
		free(g_nim_handler);
		g_nim_handler = NULL;
	}
	if (g_nim_config) {
		free(g_nim_config);
		g_nim_config = NULL;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_signal_info_get(aui_hdl handle, struct aui_signal_status *status)
{
	struct nim_handler *hdl = (struct nim_handler *)handle;
	struct slnim_signal_status info;
	int lock;

	if (!hdl || !status)
		aui_rtn(AUI_RTN_FAIL, "invalid param");

	if (alislnim_get_lock(hdl->dev, (unsigned int *)&lock))
		aui_rtn(AUI_RTN_FAIL, "get lock error");

	memset(status, 0, sizeof(struct aui_signal_status));
	if (lock == AUI_NIM_STATUS_LOCKED) {

		/* get channel info */
		memset(&info, 0, sizeof(struct slnim_signal_status));
		if (alislnim_ioctl(hdl->dev, NIM_IOCMD_GET_INFO, (int)&info))
			return AUI_RTN_FAIL;

		// Assign status with info
		// The AUI struct aui_signal_status NOT NEED to keep the same as
		// struct slnim_signal_status now.
		status->ul_freq = info.ul_freq;
		status->ul_signal_strength = info.ul_signal_strength;
		status->ul_signal_quality = info.ul_signal_quality;
		status->ul_bit_error_rate = info.ul_bit_error_rate;
		status->ul_rf_level = info.ul_rf_level;
		status->ul_signal_cn = info.ul_signal_cn;
		status->ul_mer = info.ul_mer;
		status->ul_per = info.ul_per;
        status->ul_pre_ber = info.ul_pre_ber;
		switch (hdl->demod_type) {
		    case AUI_NIM_QPSK:
				status->std = info.info.std;
		        break;
		    case AUI_NIM_QAM:
		        break;
		    case AUI_NIM_OFDM:
		        status->std = info.info.std;
		        if (status->std == AUI_STD_DVBT2) {
		            status->u.dvbt2.fec = info.info.u.dvbt2.fec;
		            status->u.dvbt2.fft_mode = info.info.u.dvbt2.fft_mode;
		            status->u.dvbt2.modulation = info.info.u.dvbt2.modulation;
		            status->u.dvbt2.guard_inter = info.info.u.dvbt2.guard_inter;
		            status->u.dvbt2.spectrum = info.info.u.dvbt2.spectrum;
		            status->u.dvbt2.plp_num = info.info.u.dvbt2.plp_num;
		            status->u.dvbt2.plp_id = info.info.u.dvbt2.plp_id;
		            status->u.dvbt2.plp_index = info.info.u.dvbt2.plp_index;
		            status->u.dvbt2.system_id = info.info.u.dvbt2.system_id;
		            status->u.dvbt2.network_id = info.info.u.dvbt2.network_id;
		            status->u.dvbt2.trans_stream_id = info.info.u.dvbt2.trans_stream_id;
		        } else {
		            status->u.dvbt_isdbt.fec = info.info.u.dvbt_isdbt.fec;
		            status->u.dvbt_isdbt.fft_mode = info.info.u.dvbt_isdbt.fft_mode;
		            status->u.dvbt_isdbt.modulation = info.info.u.dvbt_isdbt.modulation;
		            status->u.dvbt_isdbt.guard_inter = info.info.u.dvbt_isdbt.guard_inter;
		            status->u.dvbt_isdbt.spectrum = info.info.u.dvbt_isdbt.spectrum;
		        }
		        break;
		    default:
		        break;
		}

		return AUI_RTN_SUCCESS;
	}
	aui_rtn(AUI_RTN_FAIL, "signal not locked");
}

// The function 'aui_nim_disconnect' is uesed to simulate the signal loss case.
// It set the invaild frequence value (=1) and symbol value(=1) to the driver, resulting in the channel unlocked.
AUI_RTN_CODE aui_nim_disconnect(aui_hdl handle)
{
	struct nim_handler* hdl = (struct nim_handler *)handle;
	struct slnim_freq_lock nim_param;

	if(!hdl)
		aui_rtn(AUI_RTN_FAIL, NULL);

	memset(&nim_param, 0, sizeof(struct slnim_freq_lock));

	/* set a invalid freq value to unlock */
	switch(hdl->demod_type) {
	case AUI_NIM_QPSK:
		nim_param.freq = 1;
		nim_param.sym = 1;
		break;
	case AUI_NIM_QAM:
		nim_param.freq = 1;
		nim_param.sym = 1;
		break;
	case AUI_NIM_OFDM:
		nim_param.freq = 42000;
		nim_param.sym = 1;
		nim_param.bandwidth = SLNIM_BANDWIDTH_8_MHZ;
		break;
	default :
		aui_rtn(AUI_RTN_FAIL, NULL);
		break;
	}

	if (alislnim_freqlock(hdl->dev, &nim_param))
		aui_rtn(AUI_RTN_FAIL,"disconnect error");

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_auto_scan_start(aui_hdl handle, struct aui_as_sat *param)
{
	struct nim_handler *hdl = (struct nim_handler *)handle;
	struct aui_nim_config *config;
	struct slnim_auto_scan nim_param;
	aui_nim_22k_e status_22k;

	if (!hdl || !param || hdl->demod_type != AUI_NIM_QPSK)
		aui_rtn(AUI_RTN_FAIL,NULL);

	config = g_nim_config + hdl->data.dev_idx;

	if (alislnim_autoscan_active(hdl->dev))
		aui_rtn(AUI_RTN_FAIL,"autoscan already running");

	if(NULL != hdl->gpio_lnb)
		aui_gpio_set_value(hdl->gpio_lnb, config->lnb_power_gpio.gpio_val); // high power

	memset(&nim_param, 0, sizeof(struct slnim_auto_scan));
	nim_param.usr_data = param->pv_user_data;
	nim_param.sfreq = param->ul_start_freq;
	nim_param.efreq = param->ul_stop_freq;
	nim_param.callback = param->aui_as_cb;

	// If ul_freq_band is invalid do not do the following works.
	if (param->ul_freq_band == AUI_NIM_LOW_BAND
			|| param->ul_freq_band == AUI_NIM_HIGH_BAND) {
		/* Set polarity */
		if (!(param->ul_polar & ~1)) {
			if (aui_nim_set_polar(hdl, param->ul_polar))
				aui_rtn(AUI_RTN_FAIL, "set polar failed");
		}

		slnim_diseqc_operate_t diseqc_param;
		memset(&diseqc_param, 0, sizeof(diseqc_param));

		/* Select satellite with tone burst */
		diseqc_param.mode = (param->ul_src == 0) ?
				SLNIM_DISEQC_MODE_BURST0 : SLNIM_DISEQC_MODE_BURST1;
		if (alislnim_diseqc_operate(hdl->dev, &diseqc_param))
			aui_rtn(AUI_RTN_FAIL, "select sat failed");

		usleep(10000);

		/* Set tone: activated for high band */
//		if (param->ul_freq_band == AUI_NIM_LOW_BAND
//				|| param->ul_freq_band == AUI_NIM_HIGH_BAND) {
			status_22k = (param->ul_freq_band == AUI_NIM_HIGH_BAND)
					? AUI_NIM_22K_ON : AUI_NIM_22K_OFF;
			if (aui_nim_set22k_onoff(hdl, status_22k)) {
				aui_rtn(AUI_RTN_FAIL, "set tone failed");
			}
//		}
	}

	/* launch autoscan */
	if (alislnim_autoscan(hdl->dev, &nim_param))
		aui_rtn(AUI_RTN_FAIL,NULL);

	AUI_DBG("[aui] as done\n");
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_nim_auto_scan_stop(aui_hdl handle)
{
	struct nim_handler* hdl = (struct nim_handler *)handle;
	int timeout = AUTO_SCAN_TIMEOUT; /* ms */

	if (!hdl)
		aui_rtn(AUI_RTN_FAIL,NULL);

	if (alislnim_autoscan_active(hdl->dev)) {
		if (alislnim_autoscan_stop(hdl->dev))
			aui_rtn(AUI_RTN_FAIL,NULL);

		while (alislnim_autoscan_active(hdl->dev) && timeout--)
			usleep(1000);

		// If timeout the variable timeout is -1 here.
		if (timeout <= 0) {
		//	AUI_ERR("Stop auto scan timeout (%dMS).\n",
		//			AUTO_SCAN_TIMEOUT);
			aui_rtn(AUI_RTN_FAIL,"autoscan blocked");
		}

		if (alislnim_ioctl(hdl->dev, NIM_IOCMD_STOP_AUTOSCAN, 0))
			aui_rtn(AUI_RTN_FAIL,NULL);
	}

	if (hdl->u.sat.tone_active) {
		aui_nim_set22k_onoff(hdl, AUI_NIM_22K_OFF);
	}
	//AUI_DBG("Stop auto scan OK (%dMS).\n", AUTO_SCAN_TIMEOUT - timeout);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE  aui_diseqc_oper(aui_hdl handle, unsigned int mode,
			      unsigned char *cmd, unsigned int cnt,
			      unsigned char *resp, unsigned int *resp_cnt)
{
	struct nim_handler *hdl = (struct nim_handler *)handle;
	slnim_diseqc_operate_t param;

	if (!hdl || mode >= AUI_DISEQC_MODE_NUM)
		aui_rtn(AUI_RTN_FAIL,NULL);

	param.mode = mode;
	param.cnt = cnt;
	param.cmd = cmd;
	param.rt_value = resp;

	if (resp) {
		/* DiSEqC 2.x : with answer */
		AUI_DBG("diseqc 2.x");
		if (alislnim_diseqc2x_operate(hdl->dev, &param))
			aui_rtn(AUI_RTN_FAIL, "diseqc error");
		*resp_cnt = param.rt_cnt;
	} else {
		/* DiSEqC 1.0 : no answer */
		AUI_DBG("diseqc 1.0");
		if (alislnim_diseqc_operate(hdl->dev, &param))
			aui_rtn(AUI_RTN_FAIL, "diseqc 2x error");
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_set_polar(aui_hdl handle, aui_nim_polar polar)
{
	struct nim_handler* hdl = (struct nim_handler *)handle;
	if (!hdl || (polar & ~1)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	if (alislnim_set_polar(hdl->dev, (unsigned int)polar)) {
		aui_rtn(AUI_RTN_EINVAL, "set polar failed: %d\n", polar);
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_set22k_onoff(aui_hdl handle, aui_nim_22k_e status_22k)
{
	struct nim_handler* hdl = (struct nim_handler *)handle;
	slnim_diseqc_operate_t diseqc_param;
	if (!hdl || (status_22k & ~1)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	memset(&diseqc_param, 0, sizeof(diseqc_param));
	diseqc_param.mode = (status_22k == AUI_NIM_22K_ON)
						? SLNIM_DISEQC_MODE_22KON : SLNIM_DISEQC_MODE_22KOFF;
	if (alislnim_diseqc_operate(hdl->dev, &diseqc_param)) {
		aui_rtn(AUI_RTN_FAIL, "set tone failed");
	}

	hdl->u.sat.tone_active = (status_22k == AUI_NIM_22K_ON);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_set_state(aui_hdl handle, aui_nim_state state)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct nim_handler* hdl = (struct nim_handler *)handle;

	if(!hdl) {
		aui_rtn(AUI_RTN_FAIL, NULL);
	}
	if (state == AUI_NIM_TUNER_STANDBY) {
		// Put tuner into standby(low power)
		if (alislnim_ioctl(hdl->dev, NIM_IOCMD_TUNER_POWER_CONTROL, 0)) {
			ret = AUI_RTN_FAIL;
			AUI_DBG("Tuner standby failed\n");
		}
	}
	return ret;
}

AUI_RTN_CODE aui_nim_sym_offset_limit_set(aui_hdl handle, unsigned long sym_limit)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct nim_handler* hdl = (struct nim_handler *)handle;

	if(!hdl) {
		aui_rtn(AUI_RTN_FAIL, NULL);
	}
	
	if (sym_limit != 0) {
		if (alislnim_ioctl(hdl->dev, NIM_IOCMD_SYM_LIMIT_RANGE, (int)&sym_limit)) {
			ret = AUI_RTN_FAIL;
			AUI_ERR("NIM_DRIVER_SYM_LIMIT_RANGE failed\n");
		}
	}

	return ret;
}

AUI_RTN_CODE aui_nim_lnb_power_set(aui_hdl handle, aui_nim_lnb_power_status lnb_onoff)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    struct nim_handler* hdl = (struct nim_handler *)handle;
    struct aui_nim_config *nim = g_nim_config + hdl->data.dev_idx;
	
    if(!hdl) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }

    if (AUI_GPIO_NONE != nim->lnb_power_gpio.position) {
        if (lnb_onoff) {
            if (hdl->demod_type == AUI_NIM_QPSK || hdl->demod_type == AUI_NIM_OFDM) {
                aui_gpio_attr gpio_power;
                memset(&gpio_power, 0, sizeof(aui_gpio_attr));
                gpio_power.io = nim->lnb_power_gpio.io;
                gpio_power.value_out = nim->lnb_power_gpio.gpio_val;
                gpio_power.uc_dev_idx = nim->lnb_power_gpio.position;
                aui_gpio_open(&gpio_power, &hdl->gpio_lnb);
                aui_gpio_set_value(hdl->gpio_lnb, nim->lnb_power_gpio.gpio_val);
            }
        } else {
            if(NULL != hdl->gpio_lnb) {
                aui_gpio_set_value(hdl->gpio_lnb, !nim->lnb_power_gpio.gpio_val);
                aui_gpio_close(hdl->gpio_lnb);
            }
        }
    } else {
        if (alislnim_ioctl(hdl->dev, NIM_IOCMD_SET_LNB_POWER_ONOFF, (int)&lnb_onoff)) {
            ret = AUI_RTN_FAIL;
            AUI_ERR("NIM_IOCMD_SET_LNB_POWER_ONOFF failed\n");
        }
    }

    return ret;
}

