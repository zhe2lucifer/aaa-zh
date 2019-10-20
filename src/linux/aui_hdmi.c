/*
 *  @file        HDMI(High Definition Multimedia Interface) file based on LINUX
 *  @brief      input file brief description here
 *  @author   ze.hong
 *  @date      2013-8-7
 *  @version  1.0.0
 *  @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 *     If you want to use HDMI module, you can according to this flow:
 *         1. Use aui_hdmi_init to initilize HDMI some parameters, which about board.
 *         2. Use aui_hdmi_open to initilize HDMI hardware, after this HDMI is ready to work.
 *         3. Use aui_hdmi_start to start HDMI working, now HDMI can send video and audio to TV.
 *
 *	  If you want to use someother functions about HDMI, you can see below information.
 *         1. If you don't want to display video and audio on TV, you can use aui_hdmi_off. and then,
 *		    if you want to recover video and audio again, you can use aui_hdmi_on.
 *         2. If you just want to disable audio, you can use aui_hdmi_audio_off. Then if you want
 *		    to enable audio again, you can use aui_hdmi_audio_on.
 *         3. If you want to disable HDCP, you can use aui_hdmi_hdcp_off. If not, you can use aui_hdmi_hdcp_on.
 *         4. If you want to use your private HDCP key, you can set the key when hdmi initilize, in callback function in aui_hdmi_init.
 *         5. If you want to set your vendor_name, product_description, also you can set this in callback function in aui_hdmi_init.
 *         6. If you want to get EDID from TV, you can use aui_hdmi_ediddata_read to get. But not suggest, because
 *		    driver will get EDID and parse it, you just need to get the result.
 *         7. You can get the suggest resolution of the TV, also called  native resolution.
 *         8. You can use aui_hdmi_sink_preferaudio_get to get the suggest audio coding type.
 *         9. If you want to add callback function, you can use aui_hdmi_callback_reg. If not, you can use
 *		    aui_hdmi_callback_del to delete callback function.
 *         10. If you want to select audio coding type, you can use aui_hdmi_audio_select to select PCM or spdif.
 *		     But don't suggest to do this, because except hdmi, you must also set audio/sound module, if you know
 *		     how to set audio/sound and hdmi, you can do this.
 */

/****************************INCLUDE*******************************************/

#include <malloc.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <pthread.h>
#include "aui_common_priv.h"

#include <aui_hdmi.h>

#include "aui_common_priv.h"
#include <aui_vbi.h>
#include <aui_dis.h>
#include "alipltfretcode.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alislhdmi.h"
#include "alisldis.h"
#include <alislsnd.h>

AUI_MODULE(HDMI)

static unsigned char aui_hdcp_key[286] = {0};
#define HDCP22_MEM_KEY_LENGTH 402
#define HDCP14_MEM_KEY_LENGTH 286
#define HDCP22_CE_KEY_LENGTH  416

/****************************LOCAL TYPE*******************************************/

/** hdmi handle */
typedef struct aui_st_handle_hdmi {
	aui_dev_priv_data dev_priv_data;
	/**device handle form driver layer*/
	void *pst_dev_hdmi;
	/**device properties*/
	aui_attr_hdmi attr_hdmi;
} aui_handle_hdmi,*aui_p_handle_hdmi;

typedef int (*aui_hdmi_edid_callback)(void*);
typedef int (*aui_hdmi_hotplug_callback)(void*);
typedef int (*aui_hdmi_hdcp_fail_callback)(unsigned char*, unsigned char, void*);
typedef int (*aui_hdmi_cec_callback)(unsigned char*, unsigned char, void*);

/****************************LOCAL VAR********************************************/

static pthread_mutex_t s_mod_mutex_id_hdmi;

static aui_hdmi_property aui_hdmiproperty;
static unsigned char aui_vendorname_length;
static unsigned char aui_prodes_length;
static void *hdmi_dev;//alisl hdmi handle
static void *sl_snd_handle = NULL;
unsigned char hdcp_key[286] = {0};

static aui_hdmi_callback  cb_edid_ready;
static aui_hdmi_callback  cb_hot_plug_out;
static aui_hdmi_callback  cb_hot_plug_in;
static aui_hdmi_callback  cb_cec_msgrcv;
static aui_hdmi_callback  cb_hdcp_fail;

/****************************MODULE IMPLEMENT*************************************/

/**
 * @brief          get hdmi aui version information
 * @author        ze.hong
 * @date           2013-8-7
 * @param[in]     pul_version   point module version number
 * @return        AUI_RTN_SUCCESS : success
 * @return        AUI_RTN_EINVAL :Invalid input parameter
 * @return        others   fail
 * @note
 *
 */
AUI_RTN_CODE aui_hdmi_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version) {
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_HDMI;
	return AUI_RTN_SUCCESS;
}

/**
 * @brief         hdmi init
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_call_back_init  point to function of hdmi init
 * @return        AUI_RTN_SUCCESS : success
 * @return        AUI_RTN_EINVAL :Invalid input parameter
 * @return        others   fail
 * @note          we can call p_call_back_init function, to initialize HDMI parameters,\n
 *                such as board information, HDCP key.
 */
AUI_RTN_CODE aui_hdmi_init(p_fun_cb p_call_back_init)
{
	if(0 != pthread_mutex_init(&s_mod_mutex_id_hdmi, NULL))
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");

	if (p_call_back_init != NULL) {
		p_call_back_init(NULL);
	}
	return AUI_RTN_SUCCESS;
}



/**
 * @brief		  hdmi hdmi de-initialize
 * @author		  ze.hong
 * @date		  2013-8-7
 * @param[in]	  p_call_back_init point to hdmi de-initialize function.
 * @return		  AUI_RTN_SUCCESS  success
 * @return		  AUI_RTN_EINVAL   invalid input parameter.
 * @return		  others		   failed
 * @note		  we can call p_call_back_init function, to de-initialize HDMI.
 */
AUI_RTN_CODE aui_hdmi_de_init(p_fun_cb p_call_back_de_init)
{
	if(0 != pthread_mutex_destroy(&s_mod_mutex_id_hdmi))
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	if (p_call_back_de_init != NULL) {
		p_call_back_de_init(NULL);
	}
	return AUI_RTN_SUCCESS;
}

/**
 * @brief         open hdmi device
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_attr_hdmi     config hdmi attribute, when open hdmi device.
 * @param[in]	  pp_hdl_hdmi 	  a handle, point to aui hdmi device.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          get HDMI handle, and initialize HDMI hardware.
 *
 */
AUI_RTN_CODE aui_hdmi_open(aui_attr_hdmi *p_attr_hdmi, aui_hdl *pp_hdl_hdmi)
{
	/** device index*/
	unsigned char uc_dev_idx = 0;
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	aui_handle_hdmi *hdl_hdmi = NULL;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	hdl_hdmi = (aui_handle_hdmi *)MALLOC(sizeof(aui_handle_hdmi));
	if (NULL == hdl_hdmi) {
		retcode = AUI_RTN_ENOMEM;
		goto FUNC_EXIT;
	}
	MEMSET(hdl_hdmi, 0, sizeof(aui_handle_hdmi));

	if (NULL == p_attr_hdmi) {
		hdl_hdmi->attr_hdmi.uc_dev_idx = uc_dev_idx;
	} else {
		MEMCPY(&(hdl_hdmi->attr_hdmi), p_attr_hdmi, sizeof(aui_attr_hdmi));
	}

	//set aui hdmi property
	MEMSET(&aui_hdmiproperty, 0, sizeof(aui_hdmi_property));
	aui_hdmiproperty.hdcp_key_info.n_ksv_length = 8;
	aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length = 312;
	aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv = MALLOC(aui_hdmiproperty.hdcp_key_info.n_ksv_length);
	if (NULL == aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv) {
		retcode = AUI_RTN_ENOMEM;
		goto FUNC_EXIT;
	}
	MEMSET(aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv, 0, aui_hdmiproperty.hdcp_key_info.n_ksv_length);

	aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys = MALLOC(aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length);
	if (NULL == aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys) {
		retcode = AUI_RTN_ENOMEM;
		goto FUNC_EXIT;
	}
	MEMSET(aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys, 0, aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length);

    if (NULL == p_attr_hdmi) {
        hdl_hdmi->dev_priv_data.dev_idx = uc_dev_idx;
    } else {
        hdl_hdmi->dev_priv_data.dev_idx = p_attr_hdmi->uc_dev_idx;
	}

	if (0 != alislhdmi_open(&hdmi_dev, aui_hdcp_key)) {//for hdcp test
		retcode = AUI_RTN_FAIL;
		AUI_ERR("hdmi start error\n");
		goto FUNC_EXIT;
	}

    if (sl_snd_handle == NULL) {
        if (0 != alislsnd_open(&sl_snd_handle)) {
            AUI_ERR("Snd open fail!\n");
            goto FUNC_EXIT;
        }
        if (sl_snd_handle == NULL) {
            AUI_ERR("Snd module is not constructed!\n");
            goto FUNC_EXIT;
        }
    }
    *pp_hdl_hdmi = (aui_hdl *)hdl_hdmi;
    aui_dev_reg(AUI_MODULE_HDMI, *pp_hdl_hdmi);

    pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
    return retcode;

FUNC_EXIT:
    AUI_ERR("aui_hdmi_open fail\n");
    FREE(hdl_hdmi);
    hdl_hdmi = NULL;

    FREE(aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv);
    FREE(aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys);
    MEMSET(&aui_hdmiproperty, 0, sizeof(aui_hdmi_property));

    if (hdmi_dev) {
        if (!alislhdmi_close(hdmi_dev))
            hdmi_dev = NULL;
    }

    if (sl_snd_handle) {
        if (!alislsnd_close(sl_snd_handle))
            sl_snd_handle = NULL;
    }

    pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
    return retcode;
}

/**
 * @brief         close hdmi device
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi      aui hdmi handle.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          free aui HDMI handle, and de-initalize HDMI hardware.
 */
AUI_RTN_CODE aui_hdmi_close(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if (NULL == p_hdl_hdmi) {
		AUI_ERR("aui_hdmi_close a NULL handle\n");
		goto FUNC_EXIT;
	}

	// 1. If unreg OK, run the close process
	// 2. Ignore error in close process
	// 3. Return the error code of close process

	if (aui_dev_unreg(AUI_MODULE_HDMI, p_hdl_hdmi) != AUI_RTN_SUCCESS) {
		AUI_ERR("Unreg AUI_MODULE_HDMI fail\n");
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if (0 != alislhdmi_close(hdmi_dev)) {
		AUI_ERR("alislhdmi_close fail in aui_hdmi_close\n");
		retcode = AUI_RTN_FAIL;
		//goto FUNC_EXIT;
	} else {
		hdmi_dev = NULL;
	}
	if (0 != alislsnd_close(sl_snd_handle)) {
		retcode = AUI_RTN_FAIL;
		AUI_ERR("alislsnd_close fail in aui_hdmi_close\n");
		//goto FUNC_EXIT;
	} else {
		sl_snd_handle = NULL;
	}

	MEMSET(p_hdl_hdmi, 0, sizeof(aui_handle_hdmi));
	FREE(p_hdl_hdmi);
	p_hdl_hdmi = NULL;

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

/**
 * @brief         select which method to get hdcp key
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  mem_sel 	      select which method to get hdcp key.
 *                                1: CE; 0:software
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          You can get clear hdcp key from encrypted hdcp key. There are two method
 * to decrypt, one is CE(hanrdware) decrypt, the other one is software decrypt.
 */
AUI_RTN_CODE aui_hdmi_mem_sel(aui_hdl p_hdl_hdmi, unsigned int mem_sel)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	unsigned int hdcp_key_sel = mem_sel;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_mem_sel(hdmi_dev, hdcp_key_sel)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}


/**
 * @brief         hdmi start
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note
 */
AUI_RTN_CODE aui_hdmi_start(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi = hdmi_dev;

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

/**
 * @brief         enable hdmi output.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          HDMI will output video and audio signal on TV.
 */
AUI_RTN_CODE aui_hdmi_on(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_set_switch_status(hdmi_dev, 1)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}



/**
 * @brief         disable hdmi output.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          HDMI will not output signal on TV.
 */
AUI_RTN_CODE aui_hdmi_off(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_set_switch_status(hdmi_dev, 0)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;

	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}


/**
 * @brief         set hdmi parameter.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  ul_item 	      input command or parameters.
 * @param[out]	  pv_param_out 	  a pointer, point to input parameter data.
 * @param[in]	  pv_param_in 	  a pointer, point to output parameter data.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          set HDMI parameters, or implement some functions, like ioctl function.
 */
AUI_RTN_CODE aui_hdmi_set(aui_hdl p_hdl_hdmi,unsigned long ul_item,void *pv_param_out, void *pv_para_in)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	(void)pv_param_out;

	if(NULL==p_hdl_hdmi) {
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);
	switch(ul_item) {
		case AUI_HDMI_IOCT_SET_COLOR_SPACE: {
			if(NULL==pv_para_in) {
				AUI_ERR("pv_para_in is NULL!\n");
				retcode = AUI_RTN_EINVAL;
				goto FUNC_EXIT;
			}
			aui_hdmi_color_space aui_color_space = *((aui_hdmi_color_space*)pv_para_in);
			enum alisl_video_picfmt sl_color_space;

			switch(aui_color_space) {
				case AUI_HDMI_YCBCR_422: sl_color_space = ALISL_Video_PicFmt_YCBCR_422; break;
				case AUI_HDMI_YCBCR_444: sl_color_space = ALISL_Video_PicFmt_YCBCR_444; break;
				case AUI_HDMI_RGB_MODE1: sl_color_space = ALISL_Video_PicFmt_RGB_MODE1; break;
				case AUI_HDMI_RGB_MODE2: sl_color_space = ALISL_Video_PicFmt_RGB_MODE2; break;
				default: aui_rtn(AUI_RTN_EINVAL,NULL); break;
			}
			if(AUI_RTN_SUCCESS!=alislhdmi_set_color_space(hdmi_dev, sl_color_space)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_IOCT_SET_DEEP_COLOR: {
			if(NULL==pv_para_in) {
				AUI_ERR("pv_para_in is NULL!\n");
				retcode = AUI_RTN_EINVAL;
				goto FUNC_EXIT;
			}
			enum aui_hdmi_deepcolor aui_deep_color = *((enum aui_hdmi_deepcolor*)pv_para_in);
			enum alisl_hdmi_deepcolor sl_deep_color;
			switch(aui_deep_color) {
				case AUI_HDMI_DEEPCOLOR_24: sl_deep_color = ALISL_HDMI_DEEPCOLOR_24; break;
				case AUI_HDMI_DEEPCOLOR_30: sl_deep_color = ALISL_HDMI_DEEPCOLOR_30; break;
				case AUI_HDMI_DEEPCOLOR_36: sl_deep_color = ALISL_HDMI_DEEPCOLOR_36; break;
				case AUI_HDMI_DEEPCOLOR_48: sl_deep_color = ALISL_HDMI_DEEPCOLOR_48; break;
				default:
					AUI_ERR("aui_deep_color is error!\n");
					retcode = AUI_RTN_EINVAL;
					goto FUNC_EXIT;
					break;
			}
			if(AUI_RTN_SUCCESS!=alislhdmi_set_deep_color(hdmi_dev, sl_deep_color)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_IOCT_SET_AV_BLANK: {
			if(NULL==pv_para_in) {
				AUI_ERR("pv_para_in is NULL!\n");
				retcode = AUI_RTN_EINVAL;
				goto FUNC_EXIT;
			}
			unsigned int on_off = *(unsigned int *)pv_para_in;
			if(AUI_RTN_SUCCESS!=alislhdmi_set_av_blank(hdmi_dev, on_off)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_AV_MUTE_SET: {
			(void)pv_para_in;
			if(AUI_RTN_SUCCESS!=alislhdmi_set_av_mute(hdmi_dev, 1)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_AV_UNMUTE_SET: {
			(void)pv_para_in;
			if(AUI_RTN_SUCCESS!=alislhdmi_set_av_mute(hdmi_dev, 0)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		default:
			AUI_ERR("error ul_item in aui_hdmi_get()\n");
			break;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}


/**
 * @brief         get hdmi parameter.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  ul_item 	      input command or parameters.
 * @param[out]	  pv_param_out    a pointer, point to input parameter data.
 * @param[in]	  pv_param_in     a pointer, point to output parameter data.
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          get HDMI parameters, or implement some functions, like ioctl function.
 */
AUI_RTN_CODE aui_hdmi_get(aui_hdl p_hdl_hdmi,unsigned long ul_item,void *pv_param_out, void *pv_para_in)
{
	unsigned char len=0;
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	aui_hdmi_sink_capability video_caps;
	unsigned int tmp_data;
	(void)pv_para_in;
    aui_short_audio_desc audio_desc;
    aui_short_video_desc video_desc;

	MEMSET(&video_caps, 0, sizeof(aui_hdmi_sink_capability));
    MEMSET(&audio_desc, 0, sizeof(aui_short_audio_desc));
    MEMSET(&video_desc, 0, sizeof(aui_short_video_desc));

	if((NULL==p_hdl_hdmi)||(NULL==pv_param_out)) {
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	switch(ul_item) {
	case AUI_HDMI_EDID_VERSION_GET: {
		unsigned short v_edid = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_version(hdmi_dev, &v_edid)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned short *)pv_param_out = v_edid;
		break;
	}

	case AUI_HDMI_CONNECT_INFO_GET:	{
		if(AUI_RTN_SUCCESS!=alislhdmi_get_link_status(hdmi_dev, (unsigned char *)&tmp_data)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		if(tmp_data & AUI_HDMI_STATUS_LINK)
			*((unsigned int *)pv_param_out) = AUI_HDMI_STATUS_LINK;
		else
			*((unsigned int *)pv_param_out) = AUI_HDMI_STATUS_UNLINK;
		break;
	}

	case AUI_HDMI_STATUS_GET: {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_switch_status(hdmi_dev, (unsigned int *)pv_param_out)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		break;
	}

	case AUI_HDMI_HDCP_STATUS_GET: {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_link_status(hdmi_dev, (unsigned char *)&tmp_data)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;

		}
		if(tmp_data & AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED)
			*((unsigned int *)pv_param_out) = AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED;
		else if(tmp_data & AUI_HDMI_STATUS_LINK_HDCP_FAILED)
			*((unsigned int *)pv_param_out) = AUI_HDMI_STATUS_LINK_HDCP_FAILED;
		else
			*((unsigned int *)pv_param_out) = AUI_HDMI_STATUS_LINK_HDCP_IGNORED;
		break;
	}

	case AUI_HDMI_HDCP_ENABLE_GET: {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_hdcp_status(hdmi_dev, (unsigned int *)pv_param_out)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		break;
	}

	case AUI_HDMI_AUDIO_STATUS_GET: {
// for #73525 use snd instea of hdmi API, because #68880 change API of setting audio status.
#if 0
		if(AUI_RTN_SUCCESS!=alislhdmi_get_audio_status(hdmi_dev, (unsigned int *)pv_param_out)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
#else
		bool statues;
		if(0 != alislsnd_get_mute_state(sl_snd_handle, SND_IO_HDMI, &statues)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		if(statues) {
			*((unsigned int *)pv_param_out) = 1;
		} else {
			*((unsigned int *)pv_param_out) = 0;
		}
#endif
		break;
	}

	case AUI_HDMI_PREFER_VIDEO_RES_GET:	 {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_nativeres(hdmi_dev, (enum alisl_hdmi_res *)&tmp_data)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*((enum aui_hdmi_res *)pv_param_out) = (enum aui_hdmi_res)tmp_data;
		break;
	}

	case AUI_HDMI_ALL_VIDEO_RES_GET: {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_allres(hdmi_dev, &video_caps.hdmi_prefer_video_resolution, (enum alisl_hdmi_res *)video_caps.hdmi_supported_video_mode)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		MEMCPY(pv_param_out, &video_caps, sizeof(aui_hdmi_sink_capability));
		break;
	}

    case AUI_HDMI_VIC_NUM_GET: {
        if(AUI_RTN_SUCCESS != alislhdmi_get_vic_num(hdmi_dev,
													(unsigned int *)pv_param_out)) {
            retcode = AUI_RTN_FAIL;
            goto FUNC_EXIT;
        }
        break;
    }

    case AUI_HDMI_AUD_NUM_GET: {
        if(AUI_RTN_SUCCESS != alislhdmi_get_aud_num(hdmi_dev,
                                                    (unsigned int *)pv_param_out)) {
            retcode = AUI_RTN_FAIL;
            goto FUNC_EXIT;
        }
        break;
    }

    case AUI_HDMI_VIDEO_FMT_ID_GET: {
        if (AUI_RTN_SUCCESS != alislhdmi_get_edid_CEA_item(
                                    hdmi_dev,
                                    *(int *)pv_para_in,
                                    ALISL_HDMI_CEA_VIDEO_TYPE,
                                    &video_desc)){
            retcode = AUI_RTN_FAIL;
            goto FUNC_EXIT;
        }
        MEMCPY(pv_param_out, &video_desc, sizeof(aui_short_video_desc));
        break;
    }

	case AUI_HDMI_AUDIO_FMT_ID_GET:	{
        if (AUI_RTN_SUCCESS != alislhdmi_get_edid_CEA_item(
                                    hdmi_dev,
                                    *(int *)pv_para_in,
                                    ALISL_HDMI_CEA_AUD_TYPE,
                                    &audio_desc)){
            retcode = AUI_RTN_FAIL;
            goto FUNC_EXIT;
        }
        MEMCPY(pv_param_out, &audio_desc, sizeof(aui_short_audio_desc));
		break;
	}

	case AUI_HDMI_EDID_VENNAME_GET: {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_vendor_name(hdmi_dev,
													(unsigned char *)pv_param_out, &len)){
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		((char *)pv_param_out)[len] = '\0';
		break;
	}

	case AUI_HDMI_EDID_PRONAME_GET:	{
		if(AUI_RTN_SUCCESS!=alislhdmi_get_product_desc(hdmi_dev,
													(unsigned char *)pv_param_out, &len)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		((char *)pv_param_out)[len] = '\0';
		break;
	}

	case AUI_HDMI_EDID_MONNAME_GET:	{
		if(AUI_RTN_SUCCESS!=alislhdmi_get_monitor_name(hdmi_dev,
													(unsigned char *)pv_param_out, &len)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		((char *)pv_param_out)[len] = '\0';
		break;
	}

	case AUI_HDMI_EDID_MANNAME_GET:	{
		if(AUI_RTN_SUCCESS!=alislhdmi_get_manufacturer_name(hdmi_dev,
													(unsigned char *)pv_param_out, &len)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		((char *)pv_param_out)[len] = '\0';
		break;
	}
	case AUI_HDMI_PHYSICAL_ADDR_GET: {
		if(AUI_RTN_SUCCESS!=alislhdmi_cec_get_pa(hdmi_dev, (unsigned short *)pv_param_out)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		break;
	}

	case AUI_HDMI_IOCT_GET_COLOR_SPACE: {
		enum alisl_video_picfmt sl_color_space;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_color_space(hdmi_dev, &sl_color_space)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		switch(sl_color_space) {
			case ALISL_Video_PicFmt_RGB_MODE1: *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_RGB_MODE1; break;
			case ALISL_Video_PicFmt_YCBCR_422: *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_YCBCR_422; break;
			case ALISL_Video_PicFmt_YCBCR_444: *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_YCBCR_444; break;
			/**The customer can not set to YCBCR 420,
			When switching to 4k, the driver is set to 420 by default*/
			//case ALISL_Video_PicFmt_YCBCR_420: *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_YCBCR_420; break;
			default: retcode = AUI_RTN_FAIL; break;
		}
		break;
	}

	case AUI_HDMI_IOCT_GET_DEEP_COLOR: {
		enum alisl_hdmi_deepcolor sl_deep_color;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_deep_color(hdmi_dev, &sl_deep_color)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		switch(sl_deep_color) {
			case ALISL_HDMI_DEEPCOLOR_24: *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_24; break;
			case ALISL_HDMI_DEEPCOLOR_30: *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_30; break;
			case ALISL_HDMI_DEEPCOLOR_36: *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_36; break;
			case ALISL_HDMI_DEEPCOLOR_48: *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_48; break;
			default: retcode = AUI_RTN_FAIL; break;
		}
		break;
	}

	case AUI_HDMI_IOCT_GET_EDID_DEEP_COLOR: {
		int sl_dc_fmt=0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_deep_color(hdmi_dev, &sl_dc_fmt)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*((int*)pv_param_out) = sl_dc_fmt;
		break;
	}

	case AUI_HDMI_IOCT_GET_AV_BLANK: {
		unsigned int av_blank_status=0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_av_blank(hdmi_dev, &av_blank_status)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*((unsigned int*)pv_param_out) = av_blank_status;
		break;
	}

	case AUI_HDMI_TYPE_GET: {
		unsigned int hdmi_type=0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_hdmi_type(hdmi_dev, &hdmi_type)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		if(1 == hdmi_type) {
			*(aui_hdmi_type_e *)pv_param_out = AUI_HDMI_TYPE_HDMI;
		} else {
			*(aui_hdmi_type_e *)pv_param_out = AUI_HDMI_TYPE_DVI;
		}
		break;
	}

	case AUI_HDMI_EDID_LEN_GET: {
		unsigned int edid_len = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_total_length(hdmi_dev, &edid_len)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned int *)pv_param_out = edid_len;
		break;
	}

	case AUI_HDMI_PREFER_VIDEO_FMT_GET: {
		aui_hdmi_edid_video_format_e prefered_video_fmt;
		MEMSET(&prefered_video_fmt,0,sizeof(aui_hdmi_edid_video_format_e));

		if(AUI_RTN_SUCCESS!=alislhdmi_get_prefer_video_formt(
								hdmi_dev,
								(unsigned int*)&prefered_video_fmt,
								0)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		MEMCPY(pv_param_out, &prefered_video_fmt, sizeof(aui_hdmi_edid_video_format_e));
		break;
	}

	case AUI_HDMI_LOGICAL_ADDR_GET: {
		unsigned char logical_addr = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_cec_get_la(hdmi_dev, &logical_addr)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;

		}
		*(unsigned char *)pv_param_out = logical_addr;
		break;
	}

	case AUI_HDMI_EDID_PROID_GET: {
		unsigned short product_id = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_product_id(hdmi_dev, &product_id)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned short *)pv_param_out = product_id;
		break;
	}

	case AUI_HDMI_EDID_SENUM_GET: {
		unsigned long s_number = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_serial_number(hdmi_dev, &s_number)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned long *)pv_param_out = s_number;
		break;
	}

	case AUI_HDMI_EDID_MANWEEK_GET: {
		unsigned char manu_week = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_week_manufacturer(hdmi_dev, &manu_week)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned char *)pv_param_out = manu_week;
		break;
	}

	case AUI_HDMI_EDID_MANYEAR_GET: {
		unsigned short manu_year = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_year_manufacturer(hdmi_dev, &manu_year)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned short *)pv_param_out = manu_year;
		break;
	}

	case AUI_HDMI_EDID_REVISION_GET: {
		unsigned short re_edid = 0;
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_revision(hdmi_dev, &re_edid)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned short *)pv_param_out = re_edid;
		break;
	}

	case AUI_HDMI_EDID_FIRSTVID_GET: {
		if (AUI_RTN_SUCCESS != alislhdmi_get_edid_CEA_item(
									hdmi_dev,
									0,
									ALISL_HDMI_CEA_VIDEO_TYPE,
									&video_desc)){
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		*(unsigned long *)pv_param_out = video_desc.video_id_code;
		break;
	}

	case AUI_HDMI_3D_DESC_GET: {
		alisl_hdmi_3d_descriptor hdmi_3d_desc;
		MEMSET(&hdmi_3d_desc,0,sizeof(aui_hdmi_3d_descriptor));

		if (AUI_RTN_SUCCESS != alislhdmi_get_3d_descriptor(hdmi_dev, &hdmi_3d_desc)){
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		MEMCPY(pv_param_out, &hdmi_3d_desc, sizeof(alisl_hdmi_3d_descriptor));
		break;
	}

	default:
		AUI_ERR("error ul_item in aui_hdmi_get()\n");
		retcode = AUI_RTN_FAIL;
		break;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}


/**
 * @brief         hdmi count get
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]	  p_hdmi_device_count 	  aui hdmi handle
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note
 *
 */
AUI_RTN_CODE aui_hdmi_cnt_get(unsigned long *p_hdmi_device_count)
{
	if(NULL==p_hdmi_device_count) {
		return AUI_RTN_FAIL;
	}
	*p_hdmi_device_count=1;
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set hdcp key, which hdmi will use when HDCP is enabled.
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    hdmi aui handle.
*    @param[in]	    p_hdcpkey 	    a pointer, point to hdcp key.
*    @param[in]	    length 	        hdcp key length.
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note          this function can set hdcp key information.
*/
AUI_RTN_CODE aui_hdcp_params_set(aui_hdl p_hdl_hdmi, void * const p_hdcpkey, unsigned long length)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);
	if((NULL==p_hdl_hdmi)||((length != HDCP14_MEM_KEY_LENGTH)&&(length != HDCP22_MEM_KEY_LENGTH)&&(length != HDCP22_CE_KEY_LENGTH))) {
		retcode = AUI_RTN_FAIL;
		AUI_ERR("para is null\n");
		goto FUNC_EXIT;
	}
 //this function is used to set hdcp key to hdmi, or hdmi will not work, you can
 //set ture hdcp key, or you can set a false hdcp key when disable hdcp function
 //length must be 286 bytes or 402 bytes.
	if(AUI_RTN_SUCCESS!=alislhdmi_set_hdcpkey(hdmi_dev, p_hdcpkey, length))  {//alisl
		retcode = AUI_RTN_FAIL;
		AUI_ERR("alislhdmi_set_hdcpkey err code\n");
		goto FUNC_EXIT;
	}

	if( HDCP14_MEM_KEY_LENGTH == length)	{
		MEMCPY(aui_hdcp_key, p_hdcpkey, length);
		MEMCPY(aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv, (p_hdcpkey+1), 5);
		MEMCPY(aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys, (p_hdcpkey+6), 280);
	}
FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}


/**
*    @author 		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle.
*    @param[out]    pBRepeater 	    point to repeater parameter.
*    @param[out]    p_ediddata 	    point to hdcp status parameter.
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note	        get hdmi status. We can know if hdmi is a repeater, or if hdcp is success.
*/
AUI_RTN_CODE	aui_hdcp_status_get(aui_hdl p_hdl_hdmi, unsigned char *pBRepeater, unsigned long *p_success)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	(void)pBRepeater;
#if 0

	if(TRUE==api_get_hdcp_repeater())//alisl
	{
		*pBRepeater = TRUE;
	}
	else
	{
		*pBRepeater = FALSE;
	}
#endif

	retcode = alislhdmi_get_hdcp_status((alisl_handle)hdmi_dev, (unsigned int *)p_success);

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}

/**
*    @brief 		get edid data.
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle.
*    @param[out]    p_ediddata 	    point to edid data.
*    @param[out]    datalen 	    edid data length.
*    @param[in]     block_idx 	    edid data block index.
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note		    get EDID data from TV, and send it to up layer.
*
*/
AUI_RTN_CODE	aui_hdmi_ediddata_read(aui_hdl p_hdl_hdmi, unsigned long *p_ediddata,
											unsigned int *datalen, unsigned int block_idx)
{
	unsigned int edid_len = 0;
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if( (NULL==p_hdl_hdmi)|| (NULL==p_ediddata)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(block_idx == AUI_HDMI_RAW_EDID_ALL){
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_total_length(hdmi_dev, &edid_len)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
		if(AUI_RTN_SUCCESS!=alislhdmi_get_raw_edid(hdmi_dev, edid_len, (unsigned char*)p_ediddata)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
	} else {
		if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_block(hdmi_dev, (unsigned char*)p_ediddata, &edid_len, block_idx)) {
			retcode = AUI_RTN_FAIL;
			goto FUNC_EXIT;
		}
	}
	*datalen = edid_len;

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}


/**
*    @brief 		enable hdcp
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note		    enable hdcp function, make sure you get corrcet hdcp key.
*/
AUI_RTN_CODE	aui_hdmi_hdcp_on(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_set_hdcp_status(hdmi_dev, 0x1)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}


/**
*    @brief 		disable hdcp
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note
*
*/
AUI_RTN_CODE	aui_hdmi_hdcp_off(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_set_hdcp_status(hdmi_dev, 0x0)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

/**
*    @brief 		select hdmi audio output type
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle.
*    @param[in]     pcm_out 	    pcm output or not
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note          select hdmi audio output type, you can selcet PCM or BS(spdif). \n
*                   you must make sure the type is the same in sound module and hdmi module.
*/
AUI_RTN_CODE	aui_hdmi_audio_select(aui_hdl p_hdl_hdmi, int pcm_out)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}
	(void)pcm_out;
/*
	if(AUI_RTN_SUCCESS!=set_audio_switch(int pcm_out))//alisl
	{
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;

	}
*/

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

/**
*    @brief 		hdmi audio output enable
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note          enable HDMI audio output
*
*/
AUI_RTN_CODE	aui_hdmi_audio_on(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}


    // for #68880, use snd instead of hdmi API,
    // or the hdmi mute would be clear when snd_start/stop
    if(0 != alislsnd_set_mute(sl_snd_handle, true, SND_IO_HDMI)) {
		AUI_ERR("Snd set interface fail!\n");
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
    }
    AUI_DBG("unmute ok.\n");

	if(AUI_RTN_SUCCESS!=alislhdmi_set_audio_status(hdmi_dev, 0x1)){
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}


FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

/**
*    @brief 		hdmi audio output disable
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed
*    @note          close HDMI audio output
*
*/
AUI_RTN_CODE	aui_hdmi_audio_off(aui_hdl p_hdl_hdmi)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}


    // for #68880, use snd instead of hdmi API,
    // or the hdmi mute would be clear when snd_start/stop
    if(0 != alislsnd_set_mute(sl_snd_handle, false, SND_IO_HDMI)) {
		AUI_ERR("Snd set interface fail!\n");
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
    }
    AUI_DBG("mute ok.\n");

	if(AUI_RTN_SUCCESS!=alislhdmi_set_audio_status(hdmi_dev, 0x0)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}


FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

static AUI_RTN_CODE
aui_hdmi_sink_prefervideo_get_internal(aui_hdl p_hdl_hdmi,
                                       aui_hdmi_sink_capability *p_caps)
{
	/* get hdmi prefer video res */
	enum alisl_hdmi_res edid_res = ALISL_HDMI_RES_INVALID;

	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_nativeres(hdmi_dev, &edid_res)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	} else {
		switch(edid_res) {
			case ALISL_HDMI_RES_480I:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_480I;
			 	break;
			case ALISL_HDMI_RES_480P:
				 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_480P;
			 	break;
			case ALISL_HDMI_RES_576I:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_576I;
			 	break;
			case ALISL_HDMI_RES_576P:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_576P;
			 	break;
			case ALISL_HDMI_RES_720P_50:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_50;
			 	break;
			case ALISL_HDMI_RES_720P_60:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_60;
			 	break;
			case ALISL_HDMI_RES_1080I_25:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080I_25;
			 	break;
			case ALISL_HDMI_RES_1080I_30:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080I_30;
			 	break;
			case ALISL_HDMI_RES_1080P_24:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_24;
			 	break;
			case ALISL_HDMI_RES_1080P_25:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_25;
			 	break;
			case ALISL_HDMI_RES_1080P_30:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_30;
			 	break;
			case ALISL_HDMI_RES_1080P_50:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_50;
			 	break;
			case ALISL_HDMI_RES_1080P_60:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_60;
			 	break;

			case ALISL_HDMI_RES_4096X2160_24:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_4096X2160_24;
				break;
			case ALISL_HDMI_RES_3840X2160_24:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_3840X2160_24;
				break;
			case ALISL_HDMI_RES_3840X2160_25:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_3840X2160_25;
				break;
			case ALISL_HDMI_RES_3840X2160_30:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_3840X2160_30;
				break;

		/* --- extend for 861-F --- */
			case ALISL_HDMI_RES_3840X2160_50:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_3840X2160_50;
				break;
			case ALISL_HDMI_RES_3840X2160_60:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_3840X2160_60;
				break;
			case ALISL_HDMI_RES_4096X2160_25:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_4096X2160_25;
				break;
			case ALISL_HDMI_RES_4096X2160_30:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_4096X2160_30;
				break;
			case ALISL_HDMI_RES_4096X2160_50:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_4096X2160_50;
				break;
			case ALISL_HDMI_RES_4096X2160_60:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_4096X2160_60;
				break;

			default:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_50;
			 	break;
		}
	}

FUNC_EXIT:
	return retcode;
}

/**
 * @brief         get hdmi prefered video resolution.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[out]    p_caps            point to prefered video resolution.
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note          get TV prefered video resolution, also called native resolution.
 */
AUI_RTN_CODE	aui_hdmi_sink_prefervideo_get(aui_hdl p_hdl_hdmi, aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);
	retcode = aui_hdmi_sink_prefervideo_get_internal(p_hdl_hdmi, p_caps);
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);

	return retcode;
}

static AUI_RTN_CODE
aui_hdmi_sink_preferaudio_get_internal(aui_hdl p_hdl_hdmi,
                                       aui_hdmi_sink_capability *p_caps)
{
	//aui_hdmi_sink_capability *pTVCaps = p_caps;
	enum alisl_audio_coding_type aud_fmt;//get hdmi prefer audio fmt
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_audio_prefercodetype(hdmi_dev, &aud_fmt)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	} else {
		switch(aud_fmt) {
			case ALISL_EDID_AUDIO_LPCM:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_LPCM;
				 break;
			case ALISL_EDID_AUDIO_AC3:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_AC3;
				 break;
			case ALISL_EDID_AUDIO_MPEG1:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MPEG1;
				 break;
			case ALISL_EDID_AUDIO_MP3:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MP3;
				 break;

			case ALISL_EDID_AUDIO_MPEG2:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MPEG2;
				 break;
			case ALISL_EDID_AUDIO_AAC:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_AAC;
				 break;
			case ALISL_EDID_AUDIO_DTS:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DTS;
				 break;
			case ALISL_EDID_AUDIO_ATRAC:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_ATRAC;
				 break;
			case ALISL_EDID_AUDIO_ONEBITAUDIO:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_ONEBITAUDIO;
				 break;
			case ALISL_EDID_AUDIO_DD_PLUS:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DD_PLUS;
				 break;
			case ALISL_EDID_AUDIO_DTS_HD:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DTS_HD;
				 break;
			case ALISL_EDID_AUDIO_MAT_MLP:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MAT_MLP;
				 break;
			case ALISL_EDID_AUDIO_DST:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DST;
				 break;
			case ALISL_EDID_AUDIO_WMAPRO:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_BYE1PRO;
				 break;
			default:
				 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_LPCM;
				 break;
		}
	}

FUNC_EXIT:
	return retcode;

}

/**
 * @brief         get TV prefered audio coding type.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[out]    p_caps            point to audio coding type.
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note          we can get TV prefered audio coding type, such as LPCM, or AC3, or MP3, and so on.
 */
AUI_RTN_CODE	aui_hdmi_sink_preferaudio_get(aui_hdl p_hdl_hdmi, aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);
	retcode = aui_hdmi_sink_preferaudio_get_internal(p_hdl_hdmi, p_caps);
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);

	return retcode;
}

/**
 * @brief		internal function to get the all hdmi support the type of video resource
 * @author        ze.hong
 * @date          2014-7-25
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[in]     p_caps            point to need to determine the video resolution.
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note          get the prefer video and other support video resource
 *
 */
static AUI_RTN_CODE
aui_hdmi_sink_video_support_internal(aui_hdl p_hdl_hdmi,
                                     aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	aui_hdmi_sink_capability video_caps;
	MEMSET(&video_caps, 0, sizeof(aui_hdmi_sink_capability));

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_allres(hdmi_dev,
									&video_caps.hdmi_prefer_video_resolution,
									(enum alisl_hdmi_res *)video_caps.hdmi_supported_video_mode)
									) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}
	MEMCPY(p_caps, &video_caps, sizeof(aui_hdmi_sink_capability));

FUNC_EXIT:
	return retcode;
}


/**
 * @brief         judge if one video resolution is supported.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[in]     p_caps            point to one video resolution.
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note          we can judge if one video resolution is supported.
 */
AUI_RTN_CODE	aui_hdmi_sink_video_support(aui_hdl p_hdl_hdmi, aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);
	retcode = aui_hdmi_sink_video_support_internal(p_hdl_hdmi, p_caps);
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);

	return retcode;
}

/**
 * @brief		internal function to get the all hdmi support the type of video resource
 * @author        ze.hong
 * @date          2014-7-25
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[in]     p_caps            point to need to determine the audio resolution.
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note         To determine whether a certain audio coding format is supported.
 */
static AUI_RTN_CODE
aui_hdmi_sink_audio_support_internal(aui_hdl p_hdl_hdmi, aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	unsigned short audio_fmt=0;
	aui_hdmi_audio_fmt *audio_support_fmt = p_caps->hdmi_supported_audio_mode;
	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_get_edid_audio_allcodetype(hdmi_dev, &audio_fmt)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	MEMSET(p_caps->hdmi_supported_audio_mode, AUI_EDID_AUDIO_INVALID,
			sizeof(p_caps->hdmi_supported_audio_mode));

	if(audio_fmt & ALISL_EDID_AUDIO_LPCM)
		*audio_support_fmt++ = AUI_EDID_AUDIO_LPCM;
	if(audio_fmt & ALISL_EDID_AUDIO_AC3)
		*audio_support_fmt++ = AUI_EDID_AUDIO_AC3;
	if(audio_fmt & ALISL_EDID_AUDIO_MPEG1)
		*audio_support_fmt++ = AUI_EDID_AUDIO_MPEG1;
	if(audio_fmt & ALISL_EDID_AUDIO_MP3)
		*audio_support_fmt++ = AUI_EDID_AUDIO_MP3;
	if(audio_fmt & ALISL_EDID_AUDIO_MPEG2)
		*audio_support_fmt++ = AUI_EDID_AUDIO_MPEG2;
	if(audio_fmt & ALISL_EDID_AUDIO_AAC)
		*audio_support_fmt++ = AUI_EDID_AUDIO_AAC;
	if(audio_fmt & ALISL_EDID_AUDIO_DTS)
		*audio_support_fmt++ = AUI_EDID_AUDIO_DTS;
	if(audio_fmt & ALISL_EDID_AUDIO_ATRAC)
		*audio_support_fmt++ = AUI_EDID_AUDIO_ATRAC;
	if(audio_fmt & ALISL_EDID_AUDIO_ONEBITAUDIO)
		*audio_support_fmt++ = AUI_EDID_AUDIO_ONEBITAUDIO;
	if(audio_fmt & ALISL_EDID_AUDIO_DD_PLUS)
		*audio_support_fmt++ = AUI_EDID_AUDIO_DD_PLUS;
	if(audio_fmt & ALISL_EDID_AUDIO_DTS_HD)
		*audio_support_fmt++ = AUI_EDID_AUDIO_DTS_HD;
	if(audio_fmt & ALISL_EDID_AUDIO_MAT_MLP)
		*audio_support_fmt++ = AUI_EDID_AUDIO_MAT_MLP;
	if(audio_fmt & ALISL_EDID_AUDIO_DST)
		*audio_support_fmt++ = AUI_EDID_AUDIO_DST;
	if(audio_fmt & ALISL_EDID_AUDIO_WMAPRO)
		*audio_support_fmt++ = AUI_EDID_AUDIO_BYE1PRO;

FUNC_EXIT:
	return retcode;
}

/**
 * @brief         judge if one audio coding type is supported.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[in]     p_caps            point to one audio coding type.
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note          we can judge if one audio coding type is supported.
 */
AUI_RTN_CODE	aui_hdmi_sink_audio_support(aui_hdl p_hdl_hdmi, aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);
	retcode = aui_hdmi_sink_audio_support_internal(p_hdl_hdmi, p_caps);
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);

	return retcode;
}


/**
 * @brief         get TV prefered audio coding type, and video resolution.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_hdl_hdmi 	    aui hdmi handle.
 * @param[in]     p_caps            point to sink capability
 * @return        AUI_RTN_SUCCESS   success.
 * @return        AUI_RTN_EINVAL    invalid input parameter.
 * @return        others            failed
 * @note          we can get TV prefered audio coding type, and video resolution.
 */
AUI_RTN_CODE	aui_hdmi_sinkcapability_get(aui_hdl p_hdl_hdmi, aui_hdmi_sink_capability *p_caps)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=aui_hdmi_sink_prefervideo_get_internal(p_hdl_hdmi, p_caps)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}
	if(AUI_RTN_SUCCESS!=aui_hdmi_sink_video_support_internal(p_hdl_hdmi, p_caps)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;

	}
	if(AUI_RTN_SUCCESS!=aui_hdmi_sink_preferaudio_get_internal(p_hdl_hdmi, p_caps)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;

	}
	if(AUI_RTN_SUCCESS!=aui_hdmi_sink_audio_support_internal(p_hdl_hdmi, p_caps)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;

	}
    if(AUI_RTN_SUCCESS!=alislhdmi_get_hdcp_support(hdmi_dev, &(p_caps->hdmi_hdcp_support))) {
        retcode = AUI_RTN_FAIL;
        goto FUNC_EXIT;
    }
FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}

static void aui_hdmi_cb_edid_ready()
{
	void *pdata = NULL;
	aui_hdmi_edid_callback aui_cb = NULL;
	if( NULL == cb_edid_ready.fnCallback) {
		return;
	}
	pdata = cb_edid_ready.pvUserData;

	aui_cb = (aui_hdmi_edid_callback)(cb_edid_ready.fnCallback);
	aui_cb(pdata);
	AUI_INFO(" aui_hdmi_plug_in_callback!\n");
    return;
}

static void aui_hdmi_plug_out_callback()
{
	void *pdata = NULL;
	aui_hdmi_hotplug_callback aui_cb = NULL;
	if( NULL == cb_hot_plug_out.fnCallback) {
		return;
	}
	pdata = cb_hot_plug_out.pvUserData;
	aui_cb = (aui_hdmi_hotplug_callback)(cb_hot_plug_out.fnCallback);
	aui_cb(pdata);
	AUI_INFO(" aui_hdmi_plug_out_callback!\n");
    return;
}

static void aui_hdmi_plug_in_callback()
{
	void *pdata = NULL;
	aui_hdmi_hotplug_callback aui_cb = NULL;
	if( NULL == cb_hot_plug_in.fnCallback) {
		return;
	}
	pdata = cb_hot_plug_in.pvUserData;

	aui_cb = (aui_hdmi_hotplug_callback)(cb_hot_plug_in.fnCallback);
	aui_cb(pdata);
	AUI_INFO(" aui_hdmi_plug_in_callback!/n");
    return;
}

static void aui_hdmi_cec_receive_callback(unsigned char *buf, unsigned char length)
{
    void *pdata = NULL;
	aui_hdmi_cec_callback aui_cb = NULL;
	if( NULL == buf || 0 == length) {
		return;
	}
	if( NULL == cb_cec_msgrcv.fnCallback) {
		return;
	}
	pdata = cb_cec_msgrcv.pvUserData;
	aui_cb = (aui_hdmi_cec_callback)(cb_cec_msgrcv.fnCallback);
	aui_cb(buf, length, pdata);
	AUI_INFO(" aui_hdmi_cec_receive_callback!/n");
    return;
}

static void aui_hdmi_hdcp_fail_callback_hdl(unsigned char *buf, unsigned char length)
{
    void *pdata = NULL;
    aui_hdmi_hdcp_fail_callback aui_cb = NULL;
    if( NULL == buf) {
        AUI_ERR("%s, buf is null.\n", __func__);
        return;
    }
    if (0 == length) {
        AUI_ERR("buf len is 0.\n");
        return;
    }
    if( NULL == cb_hdcp_fail.fnCallback) {
        AUI_ERR("%s, do not have hdcp fail cb.\n", __func__);
        return;
    }
    pdata = cb_hdcp_fail.pvUserData;

    aui_cb = (aui_hdmi_hdcp_fail_callback)(cb_hdcp_fail.fnCallback);
    aui_cb(buf, length, pdata);
    AUI_DBG(" aui_hdmi_hdcp_fail_callback!/n");
    return;
}

/**
*    @brief 		add hdmi callback function.
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]	    p_hdl_hdmi 	         aui hdmi handle.
*    @param[in]	    HDMICBType 	         hdmi callback function event type.
*    @param[in]	    hdmi_callback_func   point to hdmi callback function.
*    @param[in]	    p_user_data 	     point to callback function data.
*    @return        AUI_RTN_SUCCESS      success.
*    @return        AUI_RTN_EINVAL       invalid input parameter.
*    @return        others               failed
*    @note		    when plung in, plug out, or receive cec message, will call callback function.
*/
AUI_RTN_CODE aui_hdmi_callback_reg(aui_hdl p_hdl_hdmi, aui_hdmi_cb_type HDMICBType, void * hdmi_callback_func,void * p_user_data)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	pthread_mutex_lock(&s_mod_mutex_id_hdmi);;
	if((NULL==p_hdl_hdmi) || (NULL==hdmi_callback_func)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(HDMICBType > AUI_HDMI_CB_HDCP_FAIL) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	switch(HDMICBType) { //alisl
		case AUI_HDMI_CB_EDID_READY: {
			cb_edid_ready.fnCallback = hdmi_callback_func;
			cb_edid_ready.pvUserData = p_user_data;
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
												HDMI_CALLBACK_EDID_READY,
												&aui_hdmi_cb_edid_ready)
												) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_CB_HOT_PLUG_OUT: {
			cb_hot_plug_out.fnCallback = hdmi_callback_func;
			cb_hot_plug_out.pvUserData = p_user_data;
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
												HDMI_CALLBACK_HOT_PLUGOUT,
												&aui_hdmi_plug_out_callback)
												) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_CB_HOT_PLUG_IN: {
			cb_hot_plug_in.fnCallback = hdmi_callback_func;
			cb_hot_plug_in.pvUserData = p_user_data;
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
												HDMI_CALLBACK_HOT_PLUGIN,
												&aui_hdmi_plug_in_callback)
												) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_CB_DBG_MSG: {
			/*
			if(AUI_RTN_SUCCESS!=hdmi_proc_ioctl(hdmi_dev,
												HDMI_CMD_REG_CALLBACK,
												HDMI_CB_DBG_MSG,
												(UINT32)hdmi_callback_func)
												) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			*/
			break;
		}

		case AUI_HDMI_CB_CEC_MESSAGE: {
			cb_cec_msgrcv.fnCallback = hdmi_callback_func;
			cb_cec_msgrcv.pvUserData = p_user_data;
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
												HDMI_CALLBACK_CEC_MSGRCV,
												&aui_hdmi_cec_receive_callback)
												) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;
		}

		case AUI_HDMI_CB_HDCP_FAIL:	{
            cb_hdcp_fail.fnCallback = hdmi_callback_func;
            cb_hdcp_fail.pvUserData = p_user_data;
            if(AUI_RTN_SUCCESS != alislhdmi_callback_register(hdmi_dev,
                                    HDMI_CALLBACK_HDCP_FAIL,
                                    &aui_hdmi_hdcp_fail_callback_hdl)
                                    ) {
                retcode = AUI_RTN_FAIL;
                goto FUNC_EXIT;
            }
            break;
		}

		default:
			AUI_ERR("HDMI callback type error\n");
			break;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}

/**
*    @brief 		delete hdmi callback function.
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]	    p_hdl_hdmi 	         aui hdmi handle.
*    @param[in]	    HDMICBType 	         hdmi callback function event type.
*    @param[in]	    hdmi_callback_func   point to hdmi callback function.
*    @param[in]	    p_user_data 	     point to callback function data.
*    @return        AUI_RTN_SUCCESS      success.
*    @return        AUI_RTN_EINVAL       invalid input parameter.
*    @return        others               failed
*    @note		    delete hdmi callback function, if don't need one callback function.
*
*
*/
AUI_RTN_CODE	aui_hdmi_callback_del(aui_hdl p_hdl_hdmi, aui_hdmi_cb_type HDMICBType, void * hdmi_callback_func,void * p_user_data)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	(void)p_user_data;
	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if((NULL==p_hdl_hdmi) || (NULL==hdmi_callback_func)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(HDMICBType > AUI_HDMI_CB_HDCP_FAIL) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	switch(HDMICBType) {//alisl
		case AUI_HDMI_CB_EDID_READY:
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
													HDMI_CALLBACK_EDID_READY,
													NULL)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;

		case AUI_HDMI_CB_HOT_PLUG_OUT:
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
													HDMI_CALLBACK_HOT_PLUGOUT,
													NULL)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;

		case AUI_HDMI_CB_HOT_PLUG_IN:
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
													HDMI_CALLBACK_HOT_PLUGIN,
													NULL)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}

			break;

		case AUI_HDMI_CB_DBG_MSG:
			/*
			if(AUI_RTN_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_DBG_MSG, (UINT32)hdmi_callback_func))
				{
					retcode = AUI_RTN_FAIL;
					goto FUNC_EXIT;

				}
			*/
			break;

		case AUI_HDMI_CB_CEC_MESSAGE:
			if(AUI_RTN_SUCCESS!=alislhdmi_callback_register(hdmi_dev,
													HDMI_CALLBACK_CEC_MSGRCV,
													NULL)) {
				retcode = AUI_RTN_FAIL;
				goto FUNC_EXIT;
			}
			break;

		case AUI_HDMI_CB_HDCP_FAIL:
			/*
			if(AUI_RTN_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HDCP_FAIL, (UINT32)hdmi_callback_func))
				{
					retcode = AUI_RTN_FAIL;
					goto FUNC_EXIT;
				}
			*/
			break;
		default:
			AUI_ERR("HDMI callback type error\n");
			break;

	}


FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}

/**
*    @brief 		set hdmi property, reference to aui_hdmi_property_item.
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]	    p_hdl_hdmi        aui hdmi handle.
*    @param[in]	    en_propety_type   property type.
*    @param[in]	    p_data            point to property data.
*    @param[in]	    p_len 	          point to property data length.
*    @return        AUI_RTN_SUCCESS   success.
*    @return        AUI_RTN_EINVAL    invalid input parameter.
*    @return        others            failed
*    @note
*
*/
AUI_RTN_CODE	aui_hdmi_property_set(aui_hdl p_hdl_hdmi, aui_hdmi_property_item en_propety_type, void* p_data, int* p_len)
{
	aui_hdmi_hdcp_key_info hdcp_key_temp;
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	switch(en_propety_type) {
		case AUI_HDMI_VENDOR_NAME: {
			aui_vendorname_length = *p_len;
			strncpy((char *)aui_hdmiproperty.vendor_name, (char *)p_data, aui_vendorname_length);
			alislhdmi_set_vendor_name(hdmi_dev, aui_hdmiproperty.vendor_name, aui_vendorname_length);
			break;
		}

		case AUI_HDMI_PRODUCT_DESCRIPTION: {
			aui_prodes_length = *p_len;
			strncpy((char *)aui_hdmiproperty.product_description, (char *)p_data, aui_prodes_length);
			alislhdmi_set_product_desc(hdmi_dev, aui_hdmiproperty.product_description, aui_prodes_length);
			AUI_DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>(%d, %s, %d)\n", en_propety_type, aui_hdmiproperty.product_description, aui_prodes_length);
			break;
		}

		case AUI_HDMI_HDCP_KEY: {
			hdcp_key_temp = *(aui_hdmi_hdcp_key_info *)p_data;
			MEMCPY(aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv, hdcp_key_temp.puc_hdcp_ksv, aui_hdmiproperty.hdcp_key_info.n_ksv_length);
			MEMCPY(aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys, hdcp_key_temp.puc_encrypted_hdcp_keys, aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length);
			aui_hdmiproperty.hdcp_key_info.n_ksv_length = hdcp_key_temp.n_ksv_length;
			aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length= hdcp_key_temp.n_hdcp_keys_length;
			break;
		}

		case AUI_HDMI_LINK_STATUS: {
			aui_hdmiproperty.link_status = *(aui_hdmi_link_status *)p_data;
			break;
		}

		default:
			AUI_ERR("HDMI property error\n");
			break;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}

/**
*    @brief 		set hdmi property, reference to aui_hdmi_property_item.
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]	    p_hdl_hdmi        aui hdmi handle.
*    @param[in]	    en_propety_type   property type.
*    @param[out]	p_data            point to property data.
*    @param[out]	p_len 	          point to property data length.
*    @return        AUI_RTN_SUCCESS   success.
*    @return        AUI_RTN_EINVAL    invalid input parameter.
*    @return        others            failed
*    @note
*
*/

AUI_RTN_CODE	aui_hdmi_property_get(aui_hdl p_hdl_hdmi, aui_hdmi_property_item en_propety_type, void *p_data, int *p_len)
{
	aui_hdmi_hdcp_key_info hdcp_key_temp;
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	switch(en_propety_type) {
		case AUI_HDMI_VENDOR_NAME: {
			alislhdmi_get_vendor_name(hdmi_dev, aui_hdmiproperty.vendor_name, &aui_vendorname_length);
			strncpy((char *)p_data, (char *)aui_hdmiproperty.vendor_name, aui_vendorname_length);
			(*p_len) = aui_vendorname_length;
			break;
		}

		case AUI_HDMI_PRODUCT_DESCRIPTION: {
			alislhdmi_get_product_desc(hdmi_dev, aui_hdmiproperty.product_description, &aui_prodes_length);
			strncpy((char *)p_data, (char *)aui_hdmiproperty.product_description, aui_prodes_length);
			(*p_len) = aui_prodes_length;
			break;
		}

		case AUI_HDMI_HDCP_KEY: {
			hdcp_key_temp.puc_hdcp_ksv = aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv;
			hdcp_key_temp.puc_encrypted_hdcp_keys = aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys;
			hdcp_key_temp.n_ksv_length= aui_hdmiproperty.hdcp_key_info.n_ksv_length;
			hdcp_key_temp.n_hdcp_keys_length = aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length;
			MEMCPY(p_data, &hdcp_key_temp, sizeof(aui_hdmi_hdcp_key_info));
			(*p_len) = sizeof(aui_hdmi_hdcp_key_info);
			break;
		}

		case AUI_HDMI_LINK_STATUS: {
			alislhdmi_get_link_status(hdmi_dev, &aui_hdmiproperty.link_status);
			*(unsigned char *)p_data = aui_hdmiproperty.link_status;
			(*p_len) = sizeof(aui_hdmiproperty.link_status);
			break;
		}

		default:
			AUI_ERR("HDMI property error\n");
			break;

	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;

}


AUI_RTN_CODE aui_hdmi_cec_set_onoff(aui_hdl p_hdl_hdmi, unsigned char bOnOff)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_set_cec_status(hdmi_dev, (unsigned int)bOnOff)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

AUI_RTN_CODE aui_hdmi_cec_get_onoff(aui_hdl p_hdl_hdmi, unsigned char *bOnOff)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;
	unsigned int state = 0;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if (AUI_RTN_SUCCESS!=alislhdmi_get_cec_status(hdmi_dev, &state)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;

	}
	*bOnOff = (unsigned char)state;

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

AUI_RTN_CODE aui_hdmi_cec_get_logical_address(aui_hdl p_hdl_hdmi, unsigned char* LogicalAddr)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_cec_get_la(hdmi_dev, LogicalAddr)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

AUI_RTN_CODE aui_hdmi_cec_set_logical_address(aui_hdl p_hdl_hdmi, unsigned char LogicalAddr)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		AUI_ERR("aui_hdmi_cec_set_logical_address handle is fail.\n");
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_cec_set_la(hdmi_dev, LogicalAddr)) {
		AUI_ERR("alislhdmi_cec_set_la fail.\n");
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

AUI_RTN_CODE aui_hdmi_cec_transmit(aui_hdl p_hdl_hdmi, unsigned char *msg, unsigned char msg_length)
{
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	pthread_mutex_lock(&s_mod_mutex_id_hdmi);

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

	if(AUI_RTN_SUCCESS!=alislhdmi_transmit_cec(hdmi_dev, msg, msg_length)) {
		retcode = AUI_RTN_FAIL;
		goto FUNC_EXIT;
	}

FUNC_EXIT:
	pthread_mutex_unlock(&s_mod_mutex_id_hdmi);
	return retcode;
}

AUI_RTN_CODE aui_hdmi_set_color(aui_hdl p_hdl_hdmi, aui_hdmi_color_space color)
{
	alisl_handle dis_hdl;
	unsigned int param;
	AUI_RTN_CODE  retcode = AUI_RTN_SUCCESS;

	if(NULL==p_hdl_hdmi) {
		retcode = AUI_RTN_FAIL;
	}
	if (DIS_ERR_NONE != alisldis_open(DIS_HD_DEV, &dis_hdl)) {
		AUI_ERR("alisldis open failed!\n");
		return AUI_RTN_FAIL;
	}

	switch (color) {
		case AUI_HDMI_YCBCR_422:
			param = DIS_HDMI_YCBCR_422;
			break;
		case AUI_HDMI_YCBCR_444:
			param = DIS_HDMI_YCBCR_444;
			break;
		case AUI_HDMI_RGB_MODE1:
			param = DIS_HDMI_RGB_MODE1;
			break;
		case AUI_HDMI_RGB_MODE2:
			param = DIS_HDMI_RGB_MODE2;
			break;
		/**now only support 4k*/
		//case AUI_HDMI_YCBCR_420:
		//	param = DIS_HDMI_YCBCR_420;
		//	break;
		default:
			AUI_ERR("invalid parameter!\n");
			retcode = AUI_RTN_FAIL;
			goto error_out;
			break;
	}
	alisldis_set_attr(dis_hdl, DIS_ATTR_HDMI_OUT_PIC_FMT, param);

/*The new IC(3821,3921) will use HDMI set the space color
*  but just one of the DE/HDMI valid*/
	if(aui_hdmi_set(p_hdl_hdmi, AUI_HDMI_IOCT_SET_COLOR_SPACE ,NULL, (void *)&color)){
		AUI_ERR("aui_hdmi_set_color error!\n");
		retcode = AUI_RTN_FAIL;
	}

error_out:
	alisldis_close(dis_hdl);

	return retcode;
}
