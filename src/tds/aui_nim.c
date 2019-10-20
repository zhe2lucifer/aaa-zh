/**  @file		This file is the implementation of ali demodulator driver encapsulation.
*	 @brief 	       Expect to meet kinds of midlayer function implementation code of customers.
*	 @author		Seiya.Cao
*	 @date		2013-8-28
*	 @version	1.0.0
*	 @note		Include DVBT(OFDM),DVBC(Cable),DVBS/S2(Satallite) implementation
*/
#include "aui_common_priv.h"
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>
#include <hld/nim/nim.h>
#include <hal/hal_gpio.h>
#include <aui_nim.h>

AUI_MODULE(NIM)

typedef struct
{
	aui_dev_priv_data  data;
	struct nim_device *nim_device;
	aui_nim_attr       nim_attr;
	int                tone_active;
	aui_signal_status  info;
	aui_autoscan_sat   auto_scan_sat;
} aui_nim_dev, *aui_pnim_dev;

typedef enum
{
	AUI_NIM_POLAR_H = 0,	/**  Horizol polar.*/
	AUI_NIM_POLAR_V = 1,	/**  Veritical polar. */
}aui_nim_polar_e;


//#define AUI_NIM_DEBUG

#ifdef AUI_NIM_DEBUG
	#define AUI_NIM_BUG(x...)		  __asm__  __volatile__("sdbbp;nop")
#else
	#define AUI_NIM_BUG(x...)		  do {}while(0)
#endif

#define NIM_HANDLER_MAX		6

typedef INT32 (*autoscan_callback) (
		UINT8 uc_status,
		UINT8 uc_polar,
		UINT32 u_freq,
		UINT32 u_sym,
		UINT8 uc_fec);

static aui_hdl nim_handle_list[NIM_HANDLER_MAX] = {0};

static struct aui_nim_config *p_nim_config = NULL;

AUI_RTN_CODE aui_nim_init(p_fun_cb pnim_cb)
{
	if (p_nim_config != NULL)
	{
		return AUI_RTN_SUCCESS;
	}

	int cfg_size = NIM_HANDLER_MAX * sizeof(struct aui_nim_config);
	p_nim_config = (struct aui_nim_config *) malloc(cfg_size);

	if(NULL == p_nim_config)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	memset(p_nim_config, 0x00, cfg_size);

	if(pnim_cb != NULL)
	{
		 pnim_cb(p_nim_config);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_open(aui_nim_attr *pnim_attr,aui_hdl *pp_nim_handle)
{
	if(NULL == pnim_attr || NULL == pp_nim_handle)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	int devid = pnim_attr->ul_nim_id;
	if (devid >= NIM_HANDLER_MAX)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "invalid nim device ID");
	}

	aui_nim_dev *p_dev = (aui_nim_dev *) malloc(sizeof(aui_nim_dev));
	if (NULL == p_dev)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	p_dev->nim_device = (struct nim_device *) dev_get_by_id(HLD_DEV_TYPE_NIM, devid % 2);

	if(nim_open(p_dev->nim_device) != 0)
	{
		AUI_NIM_BUG();
		free(p_dev);
		aui_rtn(AUI_RTN_EIO, "error");
	}

	memcpy(&p_dev->nim_attr, pnim_attr, sizeof(aui_nim_attr));
	p_dev->data.dev_idx = devid;
	p_dev->tone_active = 0;

	aui_dev_reg(AUI_MODULE_NIM, p_dev);
	*pp_nim_handle = p_dev;
	nim_handle_list[devid] = p_dev;
	return  AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_handle_get_byid(unsigned int ul_nim_id, aui_hdl *p_nim_handle)
{
	if((ul_nim_id >= NIM_HANDLER_MAX)|| !p_nim_handle)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	return aui_find_dev_by_idx(AUI_MODULE_NIM, ul_nim_id, p_nim_handle);
}

AUI_RTN_CODE aui_nimid_get_by_handle(aui_hdl nim_handle, unsigned int *p_nim_id)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle;
	if(!p_nim_id || !p_dev)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	*p_nim_id = p_dev->data.dev_idx;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_connect(aui_hdl nim_handle, aui_nim_connect_param *param)
{
	struct NIM_CHANNEL_CHANGE cc_param;
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;
	int plp_idx = AUI_NIM_PLP_AUTO;
	struct aui_sat_param *sat;
	aui_nim_22k_e tone;
	unsigned int mode;

	if(p_dev == NULL)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(p_dev->nim_device == NULL)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(p_dev->nim_device->do_ioctl_ext == NULL)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(NULL == param)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if (p_dev->nim_attr.en_dmod_type != param->en_demod_type)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	memset(&cc_param, 0, sizeof(struct NIM_CHANNEL_CHANGE));
	cc_param.freq = param->ul_freq;

	switch (param->en_demod_type)
	{
		case AUI_NIM_QPSK:
			sat = &param->connect_param.sat;
			// If ul_freq_band is invalid do not do the following works.
			//AUI_DBG("sat->ul_freq_band: %d\n", sat->ul_freq_band);
			if (sat->ul_freq_band == AUI_NIM_LOW_BAND
					|| sat->ul_freq_band == AUI_NIM_HIGH_BAND) {
				/* Set polarity */
				if (!(sat->ul_polar & ~1)) {
					if (aui_nim_set_polar(nim_handle, sat->ul_polar)) {
						aui_rtn(AUI_RTN_EIO, "set polar failed");
					}
					osal_task_sleep(20);
				}

				/* Select satellite with tone burst */
				mode = (sat->ul_src == 0) ?
						NIM_DISEQC_MODE_BURST0 : NIM_DISEQC_MODE_BURST1;
				if (aui_diseqc_oper(nim_handle, mode, NULL, 0, NULL, NULL))
					aui_rtn(AUI_RTN_EIO, "select sat failed");

				// The gap between ToneBurst and continuos tone should be more than 15ms
				osal_task_sleep(20); // Can't not sleep too long

				/* Set tone: activated for high band */
//				if (sat->ul_freq_band == AUI_NIM_LOW_BAND
//						|| sat->ul_freq_band == AUI_NIM_HIGH_BAND) {
					tone = (sat->ul_freq_band == AUI_NIM_HIGH_BAND)
							? AUI_NIM_22K_ON : AUI_NIM_22K_OFF;
					if (aui_nim_set22k_onoff(nim_handle, tone)) {
						aui_rtn(AUI_RTN_EIO, "set tone failed");
					}
					osal_task_sleep(20);
//				}
			}

			cc_param.sym = param->connect_param.sat.ul_symrate;
			cc_param.fec = param->connect_param.sat.fec;
			break;

		case AUI_NIM_QAM:
			cc_param.sym = param->connect_param.cab.ul_symrate;
			cc_param.fec = param->connect_param.cab.en_qam_mode;
			switch(param->connect_param.cab.en_qam_mode)
			{
				case AUI_NIM_NOT_DEFINED:
					cc_param.modulation = AUI_NIM_NOT_DEFINED;
					break;
				case AUI_NIM_QAM16:
					cc_param.modulation = QAM16;
					break;

				case AUI_NIM_QAM32:
					cc_param.modulation = QAM32;
					break;

				case AUI_NIM_QAM64:
					cc_param.modulation = QAM64;
					break;

				case AUI_NIM_QAM128:
					cc_param.modulation = QAM128;
					break;

				case AUI_NIM_QAM256:
					cc_param.modulation = QAM256;
					break;

				default:
					AUI_NIM_BUG();
					aui_rtn(AUI_RTN_EIO, "error");
					break;
			}
			switch (param->connect_param.cab.bandwidth) {
			    // DVB-C bandwidth need to be set in some special situations.
			    // For example, J83A with 6MHz band.
			    case AUI_NIM_BANDWIDTH_6_MHZ:
			        cc_param.bandwidth = 6;
			        break;
			    case AUI_NIM_BANDWIDTH_7_MHZ:
			        cc_param.bandwidth = 7;
			        break;
			    case AUI_NIM_BANDWIDTH_8_MHZ:
			        cc_param.bandwidth = 8;
			        break;
			    default:
			        cc_param.bandwidth = 0;
			        break;
			}
			break;

		case AUI_NIM_OFDM:
			// Scan T2 only setting
			// TDS (nim_s3821_dvbt_attach and aui_nim_connect)
			// Linux (aui_nim_open and aui_nim_connect)
			// No need to call NIM_DRIVER_SEARCH_T2_SIGNAL_ONLY

			// t2_signal [I/O]
			// 0: to lock T/T2, auto scan mode, using USAGE_TYPE_AUTOSCAN.
			// 1: To lock T2 only, using USAGE_TYPE_CHANCHG. Had locked T2 signal.
			// 2: To lock T only, using USAGE_TYPE_CHANCHG. Had locked T signal.
			switch (param->connect_param.ter.std) {
			case AUI_STD_ISDBT:
			case AUI_STD_DVBT:
				cc_param.usage_type = USAGE_TYPE_CHANCHG;
				cc_param.t2_signal = 2;
				break;

			case AUI_STD_DVBT2:
			case AUI_STD_DVBT2_COMBO:
				if (param->connect_param.ter.std == AUI_STD_DVBT2) {
					cc_param.t2_signal = 1;
				} else {
					cc_param.t2_signal = 0;
				}
				plp_idx = param->connect_param.ter.u.dvbt2.plp_index;
				if (AUI_NIM_PLP_AUTO == plp_idx) { // PLP_AUTO
					cc_param.usage_type = USAGE_TYPE_AUTOSCAN;
				} else if (AUI_NIM_PLP_SCAN_NEXT == plp_idx) {
					//Try to auto detect the next signal pipe within this channel,
					//such as the next PLP of (MPLP of DVB-T2), or the next priority signal(Hierarchy of DVB-T).
					//Before USAGE_TYPE_NEXT_PIPE_SCAN can be used, you must call USAGE_TYPE_AUTOSCAN or USAGE_TYPE_CHANSCAN first.
					cc_param.usage_type = USAGE_TYPE_NEXT_PIPE_SCAN;
				} else if (p_dev->nim_attr.en_ter_std == AUI_STD_DVBT2_COMBO
					&& param->connect_param.ter.std == AUI_STD_DVBT2_COMBO) {
					// Using the USAGE_TYPE_AUTOSCAN type when
					// NIM was opened with AUI_STD_DVBT2_COMBO
 					// and NIM connected signal with t2_signal(AUI_STD_DVBT2_COMBO).
					cc_param.usage_type = USAGE_TYPE_AUTOSCAN;
 				} else {
					// The USAGE_TYPE_CHANCHG mode need to set t2_signal
 					// with the correct signal mode, T ot T2.
					cc_param.plp_index = plp_idx;
					cc_param.plp_id = param->connect_param.ter.u.dvbt2.plp_id;
					cc_param.usage_type = USAGE_TYPE_CHANCHG;
				}
				break;

			default:
				AUI_NIM_BUG();
				aui_rtn(AUI_RTN_EIO, "error");
				break;
			}

			cc_param.bandwidth =  param->connect_param.ter.bandwidth;
			cc_param.guard_interval = param->connect_param.ter.guard_interval;
			cc_param.fft_mode = param->connect_param.ter.fft_mode;
			cc_param.modulation = param->connect_param.ter.modulation;
			cc_param.fec = param->connect_param.ter.fec;
			cc_param.inverse = param->connect_param.ter.spectrum;

			switch (cc_param.bandwidth) {
			case AUI_NIM_BANDWIDTH_6_MHZ:
				cc_param.bandwidth = 6;
				break;

			case AUI_NIM_BANDWIDTH_7_MHZ:
				cc_param.bandwidth = 7;
				break;

			case AUI_NIM_BANDWIDTH_8_MHZ:
				cc_param.bandwidth = 8;
				break;

			default:
				AUI_NIM_BUG();
				break;
			}
			break;

		default:
			AUI_NIM_BUG();
			aui_rtn(AUI_RTN_EIO, "error");
			break;
	}

	if (p_dev->nim_device->do_ioctl_ext(p_dev->nim_device, \
			NIM_DRIVER_CHANNEL_CHANGE, &cc_param))
	{
	    AUI_DBG("CHANNEL_CHANGE Fail usage_type: %d plp_id: %d plp_index: %d\n",
					cc_param.usage_type, cc_param.plp_id, cc_param.plp_index);
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	p_dev->info.std = AUI_STD_OTHER;
    if (AUI_NIM_OFDM == param->en_demod_type) { /* DVB-T */
        p_dev->info.std = param->connect_param.ter.std;
        // AUI_Support_Window #56685 ensure that the info.std is correct
    	if (cc_param.t2_signal == 1) {
    		p_dev->info.u.dvbt2.fec = cc_param.fec;
    		p_dev->info.u.dvbt2.guard_inter = cc_param.guard_interval;
    		p_dev->info.u.dvbt2.fft_mode = cc_param.fft_mode;
    		p_dev->info.u.dvbt2.modulation = cc_param.modulation;
    		p_dev->info.u.dvbt2.spectrum = cc_param.inverse;
    		p_dev->info.u.dvbt2.plp_num = cc_param.plp_num;
    		p_dev->info.u.dvbt2.plp_index = cc_param.plp_index;
    		p_dev->info.u.dvbt2.plp_id = cc_param.plp_id;
    		p_dev->info.u.dvbt2.system_id = cc_param.t2_system_id;
    		p_dev->info.u.dvbt2.trans_stream_id = cc_param.t_s_id;
    		p_dev->info.u.dvbt2.network_id = cc_param.network_id;
    		p_dev->info.std = AUI_STD_DVBT2;
    	} else {
    		//aui_ter_std nim_std = p_dev->nim_attr.en_ter_std;
    		p_dev->info.u.dvbt_isdbt.fec = cc_param.fec;
    		p_dev->info.u.dvbt_isdbt.guard_inter = cc_param.guard_interval;
    		p_dev->info.u.dvbt_isdbt.fft_mode = cc_param.fft_mode;
    		p_dev->info.u.dvbt_isdbt.modulation = cc_param.modulation;
    		p_dev->info.u.dvbt_isdbt.spectrum = cc_param.inverse;
    		p_dev->info.std = AUI_STD_DVBT;
    	}
		if (p_dev->nim_attr.en_ter_std == AUI_STD_ISDBT)
			p_dev->info.std = AUI_STD_ISDBT;
    }

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_is_lock(aui_hdl nim_handle, int *p_lock)
{
	if (NULL == nim_handle || NULL == p_lock)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;
	unsigned char lock_status;
	if (nim_get_lock(p_dev->nim_device, &lock_status))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	*p_lock = lock_status;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_close(aui_hdl nim_handle)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;
	if(p_dev == NULL)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(p_dev->nim_device == NULL)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if (p_dev->tone_active) {
		aui_nim_set22k_onoff(p_dev, AUI_NIM_22K_OFF);
	}

	if(nim_close(p_dev->nim_device) != 0)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	if (aui_dev_unreg(AUI_MODULE_NIM, nim_handle))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	nim_handle_list[p_dev->nim_attr.ul_nim_id] = NULL;
	memset(p_dev,0x00,sizeof(aui_nim_dev));
	free(p_dev);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_de_init(p_fun_cb pnim_cb)
{
	if (pnim_cb != NULL)
	{
		pnim_cb(p_nim_config);
	}

	if(p_nim_config != NULL)
	{
		free(p_nim_config);
		p_nim_config = NULL;
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_signal_info_get(aui_hdl nim_handle, aui_signal_status *p_signal_status)
{
	unsigned char signal_quality = 0;
	unsigned char signal_strength = 0;
	unsigned short rf_level = 0;
	unsigned short signal_cn = 0;

	if((NULL==nim_handle)||(NULL==p_signal_status))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;

	if(SUCCESS!=nim_get_snr(p_dev->nim_device, &signal_quality))
	{
		aui_rtn(AUI_RTN_EIO, "error");
	}
	p_signal_status->ul_signal_quality = signal_quality;
	if(SUCCESS!=nim_get_agc(p_dev->nim_device, &signal_strength))
	{
		aui_rtn(AUI_RTN_EIO, "error");
	}
	p_signal_status->ul_signal_strength = signal_strength;
	if(SUCCESS!=nim_get_ber(p_dev->nim_device, (unsigned long*)&(p_signal_status->ul_bit_error_rate)))
	{
		aui_rtn(AUI_RTN_EIO, "error");
	}
	if(SUCCESS!=nim_get_freq(p_dev->nim_device, (unsigned long*)&(p_signal_status->ul_freq)))
	{
		aui_rtn(AUI_RTN_EIO, "error");
	}
	if (p_dev->nim_attr.en_dmod_type != AUI_NIM_QPSK) {
		if(SUCCESS!=nim_ioctl_ext(p_dev->nim_device,NIM_DRIVER_GET_RF_LEVEL, (void *)&(rf_level)))
		{
			aui_rtn(AUI_RTN_EIO, "error");
		}
		p_signal_status->ul_rf_level = rf_level;
		if(SUCCESS!=nim_ioctl_ext(p_dev->nim_device,NIM_DRIVER_GET_CN_VALUE, (void *)&(signal_cn)))
		{
			aui_rtn(AUI_RTN_EIO, "error");
		}
		p_signal_status->ul_signal_cn = signal_cn;
	}
#if 0
	if(SUCCESS!=nim_get_fec(p_dev->nim_device, &p_signal_status->uc_fec))
	{
		aui_rtn(AUI_RTN_EIO, "error");
	}
#endif

	if (p_dev->info.std != AUI_STD_OTHER)
	{
		p_signal_status->std = p_dev->info.std;
		memcpy(&p_signal_status->u, &p_dev->info.u, sizeof(p_dev->info.u));
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_disconnect(aui_hdl nim_handle)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;
	struct NIM_CHANNEL_CHANGE cc_param;

	memset(&cc_param, 0, sizeof(struct NIM_CHANNEL_CHANGE));

	// It set the invaild frequence value (=1) and symbol value(=1) to the driver, resulting in the channel unlocked.
	cc_param.freq = 1;
	cc_param.sym = 1;;

	if(!p_dev)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(!p_dev->nim_device)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(!p_dev->nim_device->do_ioctl_ext)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(p_dev->nim_device->do_ioctl_ext(p_dev->nim_device, \
			NIM_DRIVER_CHANNEL_CHANGE, (void *)(&cc_param)))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return AUI_RTN_SUCCESS;
}

static INT32 fn_aui_ascallback_0(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle_list[0];
	if ((p_dev == NULL) || (p_dev->auto_scan_sat.aui_as_cb == NULL)) {
		return 0;
	}
	p_dev->auto_scan_sat.aui_as_cb(status, polar, freq,sym, fec, p_dev->auto_scan_sat.pv_user_data);
	return 0;
}
static INT32 fn_aui_ascallback_1(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle_list[1];
	if ((p_dev == NULL) || (p_dev->auto_scan_sat.aui_as_cb == NULL)) {
		return 0;
	}
	p_dev->auto_scan_sat.aui_as_cb(status, polar, freq,sym, fec, p_dev->auto_scan_sat.pv_user_data);
	return 0;
}
static INT32 fn_aui_ascallback_2(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle_list[2];
	if ((p_dev == NULL) || (p_dev->auto_scan_sat.aui_as_cb == NULL)) {
		return 0;
	}
	p_dev->auto_scan_sat.aui_as_cb(status, polar, freq,sym, fec, p_dev->auto_scan_sat.pv_user_data);
	return 0;
}
static INT32 fn_aui_ascallback_3(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle_list[3];
	if ((p_dev == NULL) || (p_dev->auto_scan_sat.aui_as_cb == NULL)) {
		return 0;
	}
	p_dev->auto_scan_sat.aui_as_cb(status, polar, freq,sym, fec, p_dev->auto_scan_sat.pv_user_data);
	return 0;
}
static INT32 fn_aui_ascallback_4(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle_list[4];
	if ((p_dev == NULL) || (p_dev->auto_scan_sat.aui_as_cb == NULL)) {
		return 0;
	}
	p_dev->auto_scan_sat.aui_as_cb(status, polar, freq,sym, fec, p_dev->auto_scan_sat.pv_user_data);
	return 0;
}
static INT32 fn_aui_ascallback_5(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec)
{
	aui_nim_dev *p_dev = (aui_nim_dev *)nim_handle_list[5];
	if ((p_dev == NULL) || (p_dev->auto_scan_sat.aui_as_cb == NULL)) {
		return 0;
	}
	p_dev->auto_scan_sat.aui_as_cb(status, polar, freq,sym, fec, p_dev->auto_scan_sat.pv_user_data);
	return 0;
}

static autoscan_callback as_cb_list[NIM_HANDLER_MAX] = {
			fn_aui_ascallback_0, fn_aui_ascallback_1, fn_aui_ascallback_2,
			fn_aui_ascallback_3, fn_aui_ascallback_4, fn_aui_ascallback_5
	};

AUI_RTN_CODE aui_nim_auto_scan_start(aui_hdl nim_handle,aui_autoscan_sat *p_as)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;
	struct NIM_AUTO_SCAN as_para;
	aui_nim_22k_e tone;
	unsigned int mode;

	if((NULL == p_as) || (p_dev == NULL)
			|| (p_dev->nim_attr.ul_nim_id >= NIM_HANDLER_MAX)) {
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	aui_nim_dev *cur_dev = (aui_nim_dev *)nim_handle_list[p_dev->nim_attr.ul_nim_id];
	cur_dev->auto_scan_sat = *p_as;

	// If ul_freq_band is invalid do not do the following works.
	if (p_as->ul_freq_band == AUI_NIM_LOW_BAND
			|| p_as->ul_freq_band == AUI_NIM_HIGH_BAND) {
		if (!(p_as->ul_polar & ~1)) {
			aui_nim_set_polar(nim_handle, p_as->ul_polar);
			osal_task_sleep(500);
		}

		mode = (p_as->ul_src == 0) ?
				NIM_DISEQC_MODE_BURST0 : NIM_DISEQC_MODE_BURST1;
		if (aui_diseqc_oper(nim_handle, mode, NULL, 0, NULL, NULL))
			aui_rtn(AUI_RTN_EIO, "select sat failed");
		osal_task_sleep(500);

//		if (p_as->ul_freq_band == AUI_NIM_LOW_BAND
//				|| p_as->ul_freq_band == AUI_NIM_HIGH_BAND) {
			tone = (p_as->ul_freq_band == AUI_NIM_HIGH_BAND)
					? AUI_NIM_22K_ON : AUI_NIM_22K_OFF;
			aui_nim_set22k_onoff(nim_handle, tone);
			osal_task_sleep(500);
//		}
	}

	memset(&as_para,0x00,sizeof( struct NIM_AUTO_SCAN));
	nim_io_control(p_dev->nim_device, NIM_DRIVER_STOP_ATUOSCAN, 0);

	osal_task_sleep(300);

	as_para.sfreq = p_as->ul_start_freq;
	as_para.efreq = p_as->ul_stop_freq;
	//as_para.callback = p_as->aui_as_cb;
	as_para.callback = as_cb_list[p_dev->nim_attr.ul_nim_id];

	if (nim_ioctl_ext(p_dev->nim_device, NIM_DRIVER_AUTO_SCAN, &as_para) != SUCCESS) {
	    AUI_DBG("NIM_DRIVER_AUTO_SCAN fail\n");
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_auto_scan_stop(aui_hdl nim_handle)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;

	if(!p_dev)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	// Passing 1 to stop the auto scan
	nim_io_control(p_dev->nim_device,NIM_DRIVER_STOP_ATUOSCAN, 1);

	if(p_dev->tone_active)
	{
		aui_nim_set22k_onoff(nim_handle,AUI_NIM_22K_OFF);
	}

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE  aui_diseqc_oper(aui_hdl nim_handle, unsigned int mode,
			      unsigned char *cmd, unsigned int cnt,
			      unsigned char *resp, unsigned int *resp_cnt)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;
	if(!p_dev)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(!p_dev->nim_device)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(!p_dev->nim_device->do_ioctl_ext)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	nim_diseqc_operate_para_t diseqc_para;
	memset(&diseqc_para,0x00,sizeof(nim_diseqc_operate_para_t));

	diseqc_para.mode = mode;
	diseqc_para.cmd = cmd;
	diseqc_para.cnt = cnt;
	diseqc_para.rt_value = resp;
	if(p_dev->nim_device->do_ioctl_ext(p_dev->nim_device, \
		NIM_DRIVER_DISEQC_OPERATION, (void *)&diseqc_para))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(resp&&resp_cnt)
	{
		*resp_cnt = *diseqc_para.rt_cnt;
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_set_polar(aui_hdl nim_handle,aui_nim_polar polar)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;

	if(!p_dev || (polar & ~1))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(nim_io_control(p_dev->nim_device,NIM_DRIVER_SET_POLAR,polar)!= SUCCESS)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_set22k_onoff(aui_hdl nim_handle,aui_nim_22k_e status_22k)
{
	aui_nim_dev *p_dev = (aui_nim_dev *) nim_handle;

	if(!p_dev || (status_22k & ~1))
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	nim_diseqc_operate_para_t diseqc_para;
	memset(&diseqc_para,0x00,sizeof(nim_diseqc_operate_para_t));
	diseqc_para.mode = status_22k;
	diseqc_para.cmd = NULL;
	diseqc_para.cnt = 0;

	if(p_dev->nim_device->do_ioctl_ext(p_dev->nim_device, \
		NIM_DRIVER_DISEQC_OPERATION, (void *)&diseqc_para) != 0)
	{
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	p_dev->tone_active = (status_22k == AUI_NIM_22K_ON);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_nim_set_state(aui_hdl handle, aui_nim_state state)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_nim_dev *p_dev = (aui_nim_dev *)handle;

	if(!p_dev) {
		AUI_NIM_BUG();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	if (state == AUI_NIM_TUNER_STANDBY) {
		// Put tuner to standby mode(low power)
		if (nim_io_control(p_dev->nim_device, NIM_TUNER_SET_STANDBY_CMD, 0)) {
			ret = AUI_RTN_FAIL;
			AUI_DBG("Tuner standby failed\n");
		}
	}
	return ret;
}
