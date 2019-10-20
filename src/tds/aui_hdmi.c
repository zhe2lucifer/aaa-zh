/*
 *  @file        HDMI(High Definition Multimedia Interface) file based on TDS
 *  @brief      input file brief description here
 *  @author   ze.hong
 *  @date      2013-8-7
 *  @version  1.0.0
 *  @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 *  @maintainer  niker.li
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

#include "aui_common_priv.h"

#include <aui_hdmi.h>
#include <bus/hdmi/m36/hdmi_api.h>
#include <bus/hdmi/m36/hdmi_dev.h>
#include <hld/dis/vpo.h>
#include <hld/snd/snd.h>

AUI_MODULE(HDMI)

/****************************LOCAL TYPE*******************************************/
extern INT32 hdmi_tx_close(struct hdmi_device *dev);
extern INT32 hdmi_tx_open(struct hdmi_device *dev);
extern void hdmi_set_audio_cap_reset(BOOL b_rst);
extern INT32 hdmi_proc_ioctl(struct hdmi_device * dev, enum HDMI_IO_CMD_TYPE cmd, 
	UINT32 param1, UINT32 param2);
extern void load_hdmi_app_param(struct hdmi_device *hdmi_dev, 
	struct config_hdmi_parm *hdmi_param);
//extern board_cfg* advance_cfg(board_cfg* cfg);

#ifndef HDMI_HDCP_KEY_LEN
#define HDMI_HDCP_KEY_LEN 286
#endif

/** hdmi device handle*/
typedef struct aui_st_handle_hdmi {
    /** internal data */
	aui_dev_priv_data dev_priv_data;
	/** device mutex, used in aui hdmi module */
	OSAL_ID dev_mutex_id;
	/**device handle for HLD */	
	struct hdmi_device *pst_dev_hdmi;
	/** aui hdmi attribute */
	aui_attr_hdmi attr_hdmi;
	unsigned char hdcp_key[HDMI_HDCP_KEY_LEN];
	unsigned char hdcp_key_mode;
	/**hdmi on off flag,1:on,0:off*/
	unsigned char hdmi_on_off;
}aui_handle_hdmi,*aui_p_handle_hdmi;

typedef int (*aui_hdmi_edid_callback)(void*);
typedef int (*aui_hdmi_hotplug_callback)(void*);
typedef int (*aui_hdmi_cec_callback)(unsigned char*, unsigned char, void*);

/****************************LOCAL VAR********************************************/

static OSAL_ID s_mod_mutex_id_hdmi=0;
static aui_hdmi_property aui_hdmiproperty;
static int aui_vendorname_length;
static int aui_prodes_length;
static struct hdmi_device *hdmi_dev;
static struct snd_device *snd_dev;

static aui_hdmi_callback  cb_edid_ready;  
static aui_hdmi_callback  cb_hot_plug_out;
static aui_hdmi_callback  cb_hot_plug_in;
static aui_hdmi_callback  cb_cec_msgrcv;

/****************************LOCAL FUNC DECLEAR***********************************/


/****************************MODULE DRV IMPLEMENT*************************************/


/****************************MODULE IMPLEMENT*************************************/

/**
 * @brief         get hdmi version.
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     pul_version     version pointer.
 * @return        AUI_RTN_SUCCESS success
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed.
 * @note          
 *
 */
AUI_RTN_CODE aui_hdmi_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version) {
		aui_rtn(AUI_RTN_EINVAL,"\n para is NULL!\n");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_HDMI;
	
	return AUI_RTN_SUCCESS;
}

/**
 * @brief         hdmi initialize
 * @author        ze.hong
 * @date          2013-8-7
 * @param[in]     p_call_back_init point to hdmi initialize function.
 * @return        AUI_RTN_SUCCESS  success
 * @return        AUI_RTN_EINVAL   invalid input parameter.
 * @return        others           failed
 * @note          we can call p_call_back_init function, to initialize HDMI parameters,\n
 *                such as board information, HDCP key.
 */
AUI_RTN_CODE aui_hdmi_init(p_fun_cb p_call_back_init)
{
	s_mod_mutex_id_hdmi=osal_mutex_create();
	if(0==s_mod_mutex_id_hdmi) {
		aui_rtn(AUI_RTN_EINVAL,"\n para is NULL!\n");
	}
	if(NULL!=p_call_back_init) {
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
	( void)p_call_back_de_init;
	if(E_OK!=osal_mutex_delete(s_mod_mutex_id_hdmi)) {
		aui_rtn(AUI_RTN_EINVAL,"\n osal_mutex_delete is error!\n");
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
	OSAL_ID dev_mutex_id;
	void *dev=NULL;
	/** device index*/
	unsigned char uc_dev_idx=0;

	AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_hdmi,dev_mutex_id,AUI_MODULE_HDMI,AUI_RTN_EINVAL);

	(*(aui_handle_hdmi **)pp_hdl_hdmi)=(aui_handle_hdmi *)MALLOC(sizeof(aui_handle_hdmi));
	if(NULL==(aui_handle_hdmi *)(*(aui_handle_hdmi **)pp_hdl_hdmi)) {
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	MEMSET((aui_handle_hdmi*)(*(aui_handle_hdmi**)pp_hdl_hdmi),0,sizeof(aui_handle_hdmi));

	if(NULL==p_attr_hdmi) {
		(*(aui_handle_hdmi **)pp_hdl_hdmi)->attr_hdmi.uc_dev_idx=uc_dev_idx;
	} else {
		MEMCPY(&((*(aui_handle_hdmi **)pp_hdl_hdmi)->attr_hdmi),p_attr_hdmi,sizeof(aui_attr_hdmi));
	}

	(*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi=(struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);
	hdmi_dev = (struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);
	
	if(NULL==(*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	(*(aui_handle_hdmi **)pp_hdl_hdmi)->dev_mutex_id=dev_mutex_id;
	if(RET_SUCCESS!=hdmi_tx_open((*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi)) {
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n hdmi_tx_open fail!\n");
	}
	
	if(RET_SUCCESS!=api_hdmi_register()) {
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_hdmi_register fail!\n");
	}
	dev = NULL;
	dev = dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	if(dev!=NULL) {
		if (vpo_ioctl((struct vpo_device*)dev, VPO_IO_REG_CB_HDMI, (UINT32)set_video_info_to_hdmi) != RET_SUCCESS) {
			osal_mutex_unlock(dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n VPO_IO_REG_CB_HDMI fail!\n");
		}
	}
	//dev = NULL;
	snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
	if(snd_dev!=NULL) {
		if (snd_io_control(snd_dev, SND_REG_HDMI_CB, (UINT32)set_audio_info_to_hdmi) != RET_SUCCESS) {
			osal_mutex_unlock(dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n SND_REG_HDMI_CB fail!\n");
		}
	}
	//set aui hdmi property
	MEMSET(&aui_hdmiproperty,0,sizeof(aui_hdmi_property));
	aui_hdmiproperty.hdcp_key_info.n_ksv_length = 8;
	aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length= 312;
	aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv = MALLOC(aui_hdmiproperty.hdcp_key_info.n_ksv_length);
	aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys= MALLOC(aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length);

	(*(aui_handle_hdmi **)pp_hdl_hdmi)->dev_priv_data.dev_idx = uc_dev_idx;
	aui_dev_reg(AUI_MODULE_HDMI,*pp_hdl_hdmi);
	osal_mutex_unlock(dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(INVALID_ID==((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	aui_dev_unreg(AUI_MODULE_HDMI, p_hdl_hdmi);
	if(RET_SUCCESS!=hdmi_tx_close(hdmi_dev)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n hdmi_tx_close is error!\n");
	}

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);

	if(0!=osal_mutex_delete(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\r\n_delete mutex err.");
	}
	MEMSET(p_hdl_hdmi,0,sizeof(aui_handle_hdmi));
	FREE(p_hdl_hdmi);
	p_hdl_hdmi=NULL;

	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	return AUI_RTN_SUCCESS;
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
	(void)p_hdl_hdmi;
#if 0
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	if(RET_SUCCESS!=hdmi_tx_open(((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n hdmi_tx_open is error!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) { //bug detective 
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	if(RET_SUCCESS!=api_hdmi_register()) {
		//osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		//aui_rtn(AUI_RTN_EINVAL,"\n api_hdmi_register is error!\n");
	}
	if (vpo_ioctl((struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_REG_CB_HDMI, (UINT32)set_video_info_to_hdmi) != RET_SUCCESS) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n VPO_IO_REG_CB_HDMI is error!\n");
	}
	if (snd_io_control((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0), SND_REG_HDMI_CB, (UINT32)set_audio_info_to_hdmi) != RET_SUCCESS) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n SND_REG_HDMI_CB is error!\n");
	}
	//vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_HDMI_OUT_PIC_FMT, RGB_MODE1);
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
#endif
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void*)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) { //bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	/*use api_hdmi_switch to replace api_set_av_blank,	because api_set_av_blank will display green screen 
	* when call api_set_av_blank with different value repeately.and etree early version also use api_hdmi_switch.
	*/
	api_hdmi_switch(1);
	((aui_handle_hdmi *)p_hdl_hdmi)->hdmi_on_off = 1;
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {//bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	/*use api_hdmi_switch to replace api_set_av_blank,	because api_set_av_blank will display green screen 
	* when call api_set_av_blank with different value repeately.and etree early version also use api_hdmi_switch.
	*/
	api_hdmi_switch(0);
	((aui_handle_hdmi *)p_hdl_hdmi)->hdmi_on_off = 0;
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	int ret = AUI_RTN_SUCCESS;

	(void )pv_param_out;
	if((NULL==p_hdl_hdmi)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}	
	switch(ul_item) {
		case AUI_HDMI_AV_MUTE_SET: {
			if(0==api_set_av_mute(1)) {
				AUI_ERR("aui hdmi set av mute success..\n");
				ret = AUI_RTN_SUCCESS;
			} else {
				AUI_ERR("aui hdmi set av mute failed..\n");
				ret = AUI_RTN_FAIL;
			}
			break;
		}
		
		case AUI_HDMI_AV_UNMUTE_SET: {
			if(0==api_set_av_mute(0)) {
				AUI_ERR("aui hdmi set av unmute success..\n");
				ret = AUI_RTN_SUCCESS;
			} else {
				AUI_ERR("aui hdmi set av unmute failed..\n");
				ret = AUI_RTN_FAIL;
			}
			break;
		}
		case AUI_HDMI_AUD_BS_OUTPUT_CTRL_SET: {
			if(NULL==pv_para_in) {
				aui_rtn(AUI_RTN_EINVAL,"pv_para_in is NULL!\n");
			}
			aui_hdmi_audio_bs_output_set_e aud_mode = *(aui_hdmi_audio_bs_output_set_e *)pv_para_in;
			if(AUI_HDMI_AUD_BS_AUTO==aud_mode) {
				api_set_audio_transcoding_manual(0);
			}
			else {
				api_set_audio_transcoding_manual(1);
			}
			break;
		}
		case AUI_HDMI_IOCT_SET_AV_BLANK: {
			if(NULL==pv_para_in) {
				aui_rtn(AUI_RTN_EINVAL,"pv_para_in is NULL!\n");
			}
			if(0==api_set_av_blank(*((int*)pv_para_in))) {
				AUI_ERR("aui hdmi set av blank success..\n");
				ret = AUI_RTN_SUCCESS;
			} else {
				AUI_ERR("aui hdmi set av blank failed..\n");
				ret = AUI_RTN_FAIL;
			}
			break;
		}
		case AUI_HDMI_IOCT_SET_COLOR_SPACE: {
			if(NULL==pv_para_in) {
				aui_rtn(AUI_RTN_EINVAL,"pv_para_in is NULL!\n");
			}
			aui_hdmi_color_space aui_color_space = *((aui_hdmi_color_space*)pv_para_in);
			enum HDMI_API_COLOR_SPACE sl_color_space;

			switch(aui_color_space) {
				case AUI_HDMI_YCBCR_422: {
                    sl_color_space = HDMI_YCBCR_422; 
                    break;
                }
                
				case AUI_HDMI_YCBCR_444: {
                    sl_color_space = HDMI_YCBCR_444; 
                    break;
                }
                
				case AUI_HDMI_RGB_MODE1: {
                    sl_color_space = HDMI_RGB; 
                    break;
                }
                
				default: {
                    aui_rtn(AUI_RTN_EINVAL,NULL); 
                    break;
                }
			}
			if(AUI_RTN_SUCCESS!=api_set_hdmi_color_space(sl_color_space)) {
				aui_rtn(AUI_RTN_EINVAL,NULL);
			}
			break;
		}

		case AUI_HDMI_IOCT_SET_DEEP_COLOR: {	
			if(NULL==pv_para_in) {
				aui_rtn(AUI_RTN_EINVAL,"pv_para_in is NULL!\n");
			}
			enum aui_hdmi_deepcolor aui_deep_color = *((enum aui_hdmi_deepcolor*)pv_para_in);
			enum HDMI_API_DEEPCOLOR deep_color;
			switch(aui_deep_color) {
				case AUI_HDMI_DEEPCOLOR_24: {
                    deep_color = HDMI_DEEPCOLOR_24; 
                    break;
                }
                
				case AUI_HDMI_DEEPCOLOR_30: {
                    deep_color = HDMI_DEEPCOLOR_30; 
                    break;
                }
                
				case AUI_HDMI_DEEPCOLOR_36: {
                    deep_color = HDMI_DEEPCOLOR_36; 
                    break;
                }
                
				case AUI_HDMI_DEEPCOLOR_48: {
                    deep_color = HDMI_DEEPCOLOR_48;
                    break;
                }
                
				default: {
                    aui_rtn(AUI_RTN_EINVAL,NULL); 
                    break;
                }
			}
			if(AUI_RTN_SUCCESS!=api_set_hdmi_deep_color(deep_color)) {
				aui_rtn(AUI_RTN_EINVAL,NULL);
			}
			break;
		}

		default:
			AUI_ERR("para error\n");
			break;
	}
	
	return ret;
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
AUI_RTN_CODE aui_hdmi_get(aui_hdl p_hdl_hdmi,unsigned long ul_item,void *pv_param_out, void *pv_param_in)
{
	int i = 0;
	unsigned short nedid = 0;
    unsigned long *pcea_item= 0;
	
	if((NULL==p_hdl_hdmi)||(NULL==pv_param_out)) {
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	switch(ul_item) {
		case AUI_HDMI_CONNECT_INFO_GET: {
			if(0==api_get_hdmi_state()) {
				*(aui_hdmi_link_status *)pv_param_out = AUI_HDMI_STATUS_UNLINK;
			} else {
				*(aui_hdmi_link_status *)pv_param_out = AUI_HDMI_STATUS_LINK;
			}
			break;
		}
		case AUI_HDMI_TYPE_GET: {
			if(1==api_get_hdmi_device_support()) {
				*(aui_hdmi_type_e *)pv_param_out = AUI_HDMI_TYPE_HDMI;
			} else {
				*(aui_hdmi_type_e *)pv_param_out = AUI_HDMI_TYPE_DVI;
			}
			break;
		}
		
		case AUI_HDMI_HDCP_STATUS_GET: {
			if(0==api_get_hdcp_result()) {
				*(aui_hdmi_link_status *)pv_param_out = AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED;
			} else {
				*(aui_hdmi_link_status *)pv_param_out = AUI_HDMI_STATUS_LINK_HDCP_FAILED;
			}				
			break;
		}
	
		case AUI_HDMI_HDCP_ENABLE_GET: {
			if(0 == api_get_hdcp_result()) {
				*(unsigned int *)pv_param_out = 1;
			} else {
				*(unsigned int *)pv_param_out = 0;
			}				
			break;
		}
		
		case AUI_HDMI_PREFER_VIDEO_FMT_GET: {
			aui_hdmi_edid_video_format_e prefered_video_fmt;
			MEMSET(&prefered_video_fmt,0,sizeof(aui_hdmi_edid_video_format_e));		
			prefered_video_fmt.video_format = api_get_HDMI_stand_timings(0)->horiz_add_pixel;
			prefered_video_fmt.field_rate   = api_get_HDMI_stand_timings(0)->field_fresh_rate;
			prefered_video_fmt.aspect_ratio = api_get_HDMI_stand_timings(0)->image_aspect_ratio;
			memcpy(pv_param_out, &prefered_video_fmt, sizeof(aui_hdmi_edid_video_format_e));
			break;
		}
		
		case AUI_HDMI_LOGICAL_ADDR_GET: {
			*(unsigned char *)pv_param_out = api_get_logical_address();
			break;
		}
		
		case AUI_HDMI_PHYSICAL_ADDR_GET: {
			*(unsigned short *)pv_param_out = api_get_physical_address();
			break;
		}
		
		case AUI_HDMI_VIC_NUM_GET: {
			*(unsigned short *)pv_param_out = api_get_edid_CEA_num(CEA_VIDEO_TYPE);
			break;
		}
		
		case AUI_HDMI_AUD_NUM_GET: {
			*(unsigned short *)pv_param_out = api_get_edid_CEA_num(CEA_AUD_TYPE);
			break;
		}
			
		case AUI_HDMI_VIDEO_FMT_ID_GET: {
			i = *(int *)pv_param_in;
			pcea_item = api_get_edid_CEA_item(i,CEA_VIDEO_TYPE);
			memcpy(pv_param_out, pcea_item, sizeof(aui_short_video_desc));
			break;
		}
		
		case AUI_HDMI_AUDIO_FMT_ID_GET: {
			i = *(int *)pv_param_in;	
			pcea_item = api_get_edid_CEA_item(i,CEA_AUD_TYPE);
            memcpy(pv_param_out, pcea_item, sizeof(aui_short_audio_desc));
			break;
		}
		
		case AUI_HDMI_3D_DESC_GET: {            
			pcea_item = (unsigned long*) api_get_CEA_3D_descriptor();
            memcpy(pv_param_out, pcea_item, sizeof(aui_hdmi_3d_descriptor_t));
			break;
		}
		
		case AUI_HDMI_EDID_MANNAME_GET: {
			unsigned short m_name[14];
			if(RET_SUCCESS!=api_get_edid_manufacturer_name(m_name)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_manufacturer_name is error!\n");
			}
			MEMCPY(pv_param_out, m_name, sizeof(m_name));
			break;
		}

		case AUI_HDMI_EDID_PROID_GET: {
			unsigned short productid = 0;
			if(RET_SUCCESS!=api_get_edid_product_id(&productid)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_product_id is error!\n");
			}
			*(unsigned short *)pv_param_out = productid;			
			break;
		}

		case AUI_HDMI_EDID_SENUM_GET: {
			unsigned long s_number = 0;
			if(RET_SUCCESS!=api_get_edid_serial_number(&s_number)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_serial_number is error!\n");
			}
			*(unsigned int *)pv_param_out = s_number;			
			break;
		}

		case AUI_HDMI_EDID_MONNAME_GET: {
			unsigned char monitorname[14];
			if(RET_SUCCESS!=api_get_edid_monitor_name(monitorname)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_monitor_name is error!\n");
			}
			MEMCPY(pv_param_out, monitorname, sizeof(monitorname));
			break;
		}

		case AUI_HDMI_EDID_MANWEEK_GET: {
			unsigned char manufweek = 0;
			if(RET_SUCCESS!=api_get_edid_week_manufacturer(&manufweek)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_week_manufacturer is error!\n");
			}
			*(unsigned char *)pv_param_out = manufweek;			
			break;
		}
		
		case AUI_HDMI_EDID_MANYEAR_GET: {
			unsigned short manufyear = 0;
			if(RET_SUCCESS!=api_get_edid_year_manufacturer(&manufyear)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_year_manufacturer is error!\n");
			}
			*(unsigned short *)pv_param_out = manufyear;
			break;
		}

		case AUI_HDMI_EDID_VERSION_GET: {
			if(RET_SUCCESS!=api_get_edid_version(&nedid )) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_version is error!\n");
			}
			*(unsigned short *)pv_param_out = nedid ;
			break;
		}

		case AUI_HDMI_EDID_FIRSTVID_GET: {
			unsigned long *pcea_item = NULL;
			pcea_item = api_get_edid_CEA_item(0,CEA_VIDEO_TYPE);
			*(UINT8 *)pv_param_out = ((short_video_descriptor_t *)pcea_item)->video_id_code;
			break;
		}

		case AUI_HDMI_EDID_LEN_GET: {
			unsigned int edid_len = 128;
			if(RET_SUCCESS!=api_get_raw_edid_length((UINT16*)&edid_len)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_raw_edid_length is error!\n");
			}
			*(unsigned int *)pv_param_out = edid_len;
			break;
		}
		
		case AUI_HDMI_EDID_REVISION_GET: {
			if(RET_SUCCESS!=api_get_edid_version(&nedid)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_version is error!\n");
			}
			*(unsigned short *)pv_param_out = nedid ;
			break;
		}

		case AUI_HDMI_PREFER_VIDEO_RES_GET: {
			enum HDMI_API_RES prefer_res = HDMI_RES_INVALID;
			if(RET_SUCCESS!=api_get_edid_video_resolution(&prefer_res)) {
				aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_video_resolution is error!\n");
			}	
			*(enum aui_hdmi_res *)pv_param_out = (enum aui_hdmi_res)prefer_res ;
			break;
		}

		case AUI_HDMI_ALL_VIDEO_RES_GET: {
			aui_hdmi_sink_capability video_caps;
			if(RET_SUCCESS != api_get_edid_all_video_resolution((UINT32*)&video_caps.hdmi_prefer_video_resolution,
				(enum HDMI_API_RES *)video_caps.hdmi_supported_video_mode)) {
					aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_all_video_resolution is error!\n");
			}
			memcpy(((aui_hdmi_sink_capability*)pv_param_out)->hdmi_supported_video_mode, video_caps.hdmi_supported_video_mode, AUI_HDMI_RES_NUM*sizeof(aui_hdmi_res));
			break;
		}

		case AUI_HDMI_STATUS_GET:
			*(unsigned int *)pv_param_out = ((aui_handle_hdmi *)p_hdl_hdmi)->hdmi_on_off;
			break;
		
		case AUI_HDMI_AUDIO_STATUS_GET: {
			*(unsigned int *)pv_param_out = (unsigned int)api_get_hdmi_audio_onoff();
			break;
		}
			
		case AUI_HDMI_IOCT_GET_COLOR_SPACE: {
			enum HDMI_API_COLOR_SPACE color_space;
			
			color_space = api_get_hdmi_color_space();	
			switch(color_space) {
				case HDMI_RGB: {
                    *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_RGB_MODE1; 
                    break;
                }
                
				case HDMI_YCBCR_422: {
                    *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_YCBCR_422; 
                    break;
                }
                
				case HDMI_YCBCR_444: {
                    *((aui_hdmi_color_space*)pv_param_out) = AUI_HDMI_YCBCR_444; 
                    break;
                }
                
				default: {
                    aui_rtn(AUI_RTN_FAIL,NULL);
                    break;
                }
			}
			break;
		}
			
		case AUI_HDMI_IOCT_GET_DEEP_COLOR: {
			enum HDMI_API_DEEPCOLOR deep_color;
			
			deep_color = api_get_hdmi_deep_color();	
			switch(deep_color) {
				case HDMI_DEEPCOLOR_24: {
                    *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_24;
                    break;
                }
                
				case HDMI_DEEPCOLOR_30: {
                    *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_30;
                    break;
                }
                
				case HDMI_DEEPCOLOR_36: {
                    *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_36;
                    break;
                }
                
				case HDMI_DEEPCOLOR_48: {
                    *((enum aui_hdmi_deepcolor*)pv_param_out) = AUI_HDMI_DEEPCOLOR_48;
                    break;
                }
                
				default: {
                    aui_rtn(AUI_RTN_FAIL,NULL);
                    break;
                }
			}
			break;
		}
			
		case AUI_HDMI_IOCT_GET_EDID_DEEP_COLOR: {
			enum EDID_DEEPCOLOR_CODE dc_fmt = 0;
			if(AUI_RTN_SUCCESS!=api_get_edid_deep_color(&dc_fmt)) {
				aui_rtn(AUI_RTN_FAIL,NULL);	
			}
			*((int*)pv_param_out) = (int)dc_fmt;
			break;
		}

		default:
			break;
	}
	
	return AUI_RTN_SUCCESS;

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
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	*p_hdmi_device_count=1;//???? need add driver
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

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(INVALID_ID==((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_hdcpkey)
		|| (HDMI_HDCP_KEY_LEN!=length)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	MEMCPY(((aui_handle_hdmi *)p_hdl_hdmi)->hdcp_key, p_hdcpkey, HDMI_HDCP_KEY_LEN);
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;

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
AUI_RTN_CODE	aui_hdcp_status_get(aui_hdl p_hdl_hdmi, unsigned char *p_brepeater, unsigned long *p_success)
{
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_brepeater)
		||(NULL==p_success)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {//bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	
	if(0==api_get_hdcp_repeater()) {
		*p_brepeater = TRUE;
	} else {
		*p_brepeater = FALSE;
	}

	if(0==api_get_hdcp_result()) {
		*p_success = TRUE;
	} else {
		*p_success = FALSE;
	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
AUI_RTN_CODE	aui_hdmi_ediddata_read(aui_hdl p_hdl_hdmi, unsigned long *p_ediddata, unsigned int *datalen, unsigned int block_idx)
{
	unsigned int edid_len = 128;
	unsigned char *edid_buf = 0;

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_ediddata)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) { //bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	if(block_idx == AUI_HDMI_RAW_EDID_ALL){
		if(RET_SUCCESS!=api_get_raw_edid_length((UINT16*)&edid_len)) {
			osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n api_get_raw_edid_length is error!\n");
		}
		//driver bug
		//edid_len = 512;
		*datalen = edid_len;
		edid_buf =MALLOC(edid_len);
		if(edid_buf==NULL) {
			osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n edid_buf is NULL!\n");
		}
		memset(edid_buf,0x00,edid_len);

		if(RET_SUCCESS!=api_get_raw_edid(edid_len,edid_buf)) {
			free(edid_buf);
			osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n api_get_raw_edid is error!\n");
		}

		MEMCPY(p_ediddata, edid_buf, edid_len);
	} else {
		edid_buf =MALLOC(128);
		if(edid_buf==NULL) {
			osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n edid_buf is NULL!\n");
		}
		memset(edid_buf,0x00,edid_len);		

		if(RET_SUCCESS != api_get_raw_edid_block(block_idx, edid_buf, edid_len)) {
			free(edid_buf);
			osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"\n api_get_raw_edid_block is error!\n");
		}
		MEMCPY(p_ediddata, edid_buf, edid_len);
		*datalen = edid_len;
	}
	
	free(edid_buf);
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	unsigned char hdcp_key_mode=0;
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(INVALID_ID==((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	
	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	hdcp_key_mode = ((aui_handle_hdmi *)p_hdl_hdmi)->hdcp_key_mode;
	if(hdcp_key_mode==0) {//sw hdcp key 
		AUI_INFO("load sw hdcp key !\n");
		api_set_hdcp_key(((aui_handle_hdmi *)p_hdl_hdmi)->hdcp_key, HDMI_HDCP_KEY_LEN);
	} else if(hdcp_key_mode==1) { //ce hdcp key
		//do nothing ,for ce key loaded by aui_kl_load_hdcp_key function.
	} else {
		AUI_INFO("never come to here!\n");
		return AUI_RTN_FAIL;
	}

	api_set_hdmi_hdcp_onoff(1);
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(INVALID_ID==((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	//fix bug #76446 : do not support hdcp TV not show
	//if(0==api_get_hdcp_result()) { //hdcp authen is ok.
	//	MEMSET(((aui_handle_hdmi *)p_hdl_hdmi)->hdcp_key,0,HDMI_HDCP_KEY_LEN);
	api_set_hdmi_hdcp_onoff(0);
	//}
	
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	(void)pcm_out;
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {//bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	/*
	 *set_audio_switch, this functions is not in hdmi hld, it is called by snd driver,
	 *so, hdmi aui will not support this operation.
	 */
#if 0
	if(RET_SUCCESS!=set_audio_switch(int pcm_out))
	{
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n set_audio_switch is error!\n");
	}
#endif
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	
	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) { //bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
#if 1
    // for #68880, use snd instead of hdmi API, 
    // or the hdmi mute would be clear when snd_start/stop
    if(RET_SUCCESS!=snd_io_control(snd_dev, SND_HDMI_OUT, 1))
    {
        osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
#else
	api_set_hdmi_audio_onoff(TRUE);
#endif
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		hdmi audio output disable
*    @author		ze.hong
*    @date			2013-6-20
*    @param[in]     p_hdl_hdmi 	    aui hdmi handle
*    @return        AUI_RTN_SUCCESS success.
*    @return        AUI_RTN_EINVAL  invalid input parameter.
*    @return        others          failed		
*    @note          close HDMI audio output??¨º\n
*
*/
AUI_RTN_CODE	aui_hdmi_audio_off(aui_hdl p_hdl_hdmi)
{
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	
	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) { //bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
#if 1
    // for #68880, use snd instead of hdmi API, 
    // or the hdmi mute would be clear when snd_start/stop
    if(RET_SUCCESS!=snd_io_control(snd_dev, SND_HDMI_OUT, 0))
    {
        osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
#else
	api_set_hdmi_audio_onoff(FALSE);
#endif
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	enum HDMI_API_RES edid_res = HDMI_RES_INVALID;//get hdmi prefer video res

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_caps)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)//bug detective
	{
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	if(RET_SUCCESS!=api_get_edid_video_resolution(&edid_res)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_video_resolution is rror!\n");
	} else {
		switch(edid_res) {
		case HDMI_RES_480I:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_480I;
			 break;
		case HDMI_RES_480P:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_480P;
			 break;
		case HDMI_RES_576I:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_576I;
			 break;
		case HDMI_RES_576P:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_576P;
			 break;
		case HDMI_RES_720P_50:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_50;
			 break;
		case HDMI_RES_720P_60:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_60;
			 break;
		case HDMI_RES_1080I_25:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080I_25;
			 break;
		case HDMI_RES_1080I_30:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080I_30;
			 break;
		case HDMI_RES_1080P_24:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_24;
			 break;
		case HDMI_RES_1080P_25:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_25;
			 break;
		case HDMI_RES_1080P_30:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_30;
			 break;
		case HDMI_RES_1080P_50:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_50;
			 break;
		case HDMI_RES_1080P_60:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_60;
			 break;
		default:
			 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_50;
			 break;
		}
	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	enum EDID_AUD_FMT_CODE aud_fmt;//get hdmi prefer audio fmt

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_caps)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is error!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {//bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is error!\n");
	}

	if(RET_SUCCESS!=api_get_edid_audio_out(&aud_fmt)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_audio_out is error!\n");
	} else {
		switch(aud_fmt) {
		case EDID_AUDIO_LPCM:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_LPCM;
			 break;
		case EDID_AUDIO_AC3:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_AC3;
			 break;
		case EDID_AUDIO_MPEG1:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MPEG1;
			 break;
		case EDID_AUDIO_MP3:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MP3;
			 break;
		case EDID_AUDIO_MPEG2:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MPEG2;
			 break;
		case EDID_AUDIO_AAC:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_AAC;
			 break;
		case EDID_AUDIO_DTS:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DTS;
			 break;
		case EDID_AUDIO_ATRAC:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_ATRAC;
			 break;
		case EDID_AUDIO_ONEBITAUDIO:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_ONEBITAUDIO;
			 break;
		case EDID_AUDIO_DD_PLUS:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DD_PLUS;
			 break;
		case EDID_AUDIO_DTS_HD:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DTS_HD;
			 break;
		case EDID_AUDIO_MAT_MLP:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MAT_MLP;
			 break;
		case EDID_AUDIO_DST:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DST;
			 break;
		case EDID_AUDIO_BYE1PRO:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_BYE1PRO;
			 break;
		default:
			 p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_LPCM;
			 break;
		}
	}
	
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_caps)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi){ //bug detective
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	if(AUI_RTN_SUCCESS == aui_hdmi_get(p_hdl_hdmi, AUI_HDMI_ALL_VIDEO_RES_GET, p_caps, NULL)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		return AUI_RTN_SUCCESS;
	} else {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		return AUI_RTN_FAIL;
	}	
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
	//aui_hdmi_sink_capability *pTVCaps = p_caps;
	HDMI_AUDIO_CAPABLE audio_cap;

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_caps)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);	
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	MEMSET(&audio_cap, 0 ,sizeof(HDMI_AUDIO_CAPABLE));
	if(0 == api_get_audio_capable((unsigned char*)(&audio_cap))) {
		int i=0;
		for(i=0; i<MAX_NUM_AUDIO_INFO; i++)
			p_caps->hdmi_supported_audio_mode[i]=(aui_hdmi_audio_fmt)(audio_cap.audio_info[i].audio_type);
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		return AUI_RTN_SUCCESS;
	} else {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		return AUI_RTN_FAIL;
	}
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
	//aui_hdmi_sink_capability *pTVCaps = p_caps;
	enum HDMI_API_RES edid_res = HDMI_RES_INVALID;//get hdmi prefer video res
	enum EDID_AUD_FMT_CODE aud_fmt;//get hdmi prefer audio fmt
	HDMI_AUDIO_CAPABLE audio_cap;

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==p_caps)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	if(RET_SUCCESS!=api_get_hdmi_sink_hdcp_cap(&p_caps->hdmi_hdcp_support)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_get_hdmi_sink_hdcp_cap is error!\n");
	}
	if(RET_SUCCESS!=api_get_edid_video_resolution(&edid_res)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_video_resolution is error!\n");
	} else {
		switch(edid_res) {
			case HDMI_RES_480I:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_480I;
			 	break;
			case HDMI_RES_480P:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_480P;
			 	break;
			case HDMI_RES_576I:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_576I;
			 	break;
			case HDMI_RES_576P:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_576P;
			 	break;
			case HDMI_RES_720P_50:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_50;
			 	break;
			case HDMI_RES_720P_60:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_60;
			 	break;
			case HDMI_RES_1080I_25:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080I_25;
			 	break;
			case HDMI_RES_1080I_30:
				p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080I_30;
			 	break;
			case HDMI_RES_1080P_24:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_24;
			 	break;
			case HDMI_RES_1080P_25:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_25;
			 	break;
			case HDMI_RES_1080P_30:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_30;
			 	break;
			case HDMI_RES_1080P_50:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_50;
			 	break;
			case HDMI_RES_1080P_60:
			 	p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_1080P_60;
			 	break;
			default:
				 p_caps->hdmi_prefer_video_resolution = AUI_HDMI_RES_720P_50;
			 	break;
		}
	}
	
	if(RET_SUCCESS != api_get_edid_all_video_resolution((UINT32*)(&(p_caps->hdmi_prefer_video_resolution)),
												(enum HDMI_API_RES *)(p_caps->hdmi_supported_video_mode))) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_all_video_resolution is error!\n");
	}

	if(RET_SUCCESS!=api_get_edid_audio_out(&aud_fmt)) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n api_get_edid_audio_out is error!\n");
	} else {
		switch(aud_fmt) {
			case EDID_AUDIO_LPCM:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_LPCM;
			 	break;
			case EDID_AUDIO_AC3:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_AC3;
			 	break;
			case EDID_AUDIO_MPEG1:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MPEG1;
			 	break;
			case EDID_AUDIO_MP3:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MP3;
			 	break;
			case EDID_AUDIO_MPEG2:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MPEG2;
			 	break;
			case EDID_AUDIO_AAC:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_AAC;
			 	break;
			case EDID_AUDIO_DTS:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DTS;
			 	break;
			case EDID_AUDIO_ATRAC:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_ATRAC;
			 	break;
			case EDID_AUDIO_ONEBITAUDIO:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_ONEBITAUDIO;
			 	break;
			case EDID_AUDIO_DD_PLUS:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DD_PLUS;
			 	break;
			case EDID_AUDIO_DTS_HD:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DTS_HD;
			 	break;
			case EDID_AUDIO_MAT_MLP:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_MAT_MLP;
			 	break;
			case EDID_AUDIO_DST:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_DST;
			 	break;
			case EDID_AUDIO_BYE1PRO:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_BYE1PRO;
				break;
			default:
			 	p_caps->hdmi_prefer_audio_mode = AUI_EDID_AUDIO_LPCM;
			 	break;
		}
	}
	
	MEMSET(&audio_cap, 0 ,sizeof(HDMI_AUDIO_CAPABLE));
	if(0 == api_get_audio_capable((unsigned char*)(&audio_cap))) {
		int i = 0;
		for(i = 0; i<MAX_NUM_AUDIO_INFO; i++) {
			p_caps->hdmi_supported_audio_mode[i]=(aui_hdmi_audio_fmt)(audio_cap.audio_info[i].audio_type);
		}
	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	if( ( NULL == buf) || 
            (0 == length)) {
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
AUI_RTN_CODE	aui_hdmi_callback_reg(aui_hdl p_hdl_hdmi, aui_hdmi_cb_type hdmicbtype, void * hdmi_callback_func,void * p_user_data)
{
	(void) p_user_data;
	
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)
		||(NULL==hdmi_callback_func)) {
			osal_mutex_unlock(s_mod_mutex_id_hdmi);
			aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	if(hdmicbtype>AUI_HDMI_CB_HDCP_FAIL) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n hdmicbtype is error!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	switch(hdmicbtype) {
		case AUI_HDMI_CB_EDID_READY: {
			cb_edid_ready.fnCallback = hdmi_callback_func;
			cb_edid_ready.pvUserData = p_user_data;
//(*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi=(struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);// hdmi_dev			
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_EDID_READY, (UINT32)aui_hdmi_cb_edid_ready)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_EDID_READY is error!\n");
			}
			break;
		}

		case AUI_HDMI_CB_HOT_PLUG_OUT: {
			cb_hot_plug_out.fnCallback = hdmi_callback_func;
			cb_hot_plug_out.pvUserData = p_user_data;
//(*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi=(struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);// hdmi_dev			
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HOT_PLUG_OUT, (UINT32)aui_hdmi_plug_out_callback)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_HOT_PLUG_OUT is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_HOT_PLUG_IN: {
			cb_hot_plug_in.fnCallback = hdmi_callback_func;
			cb_hot_plug_in.pvUserData = p_user_data;
//(*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi=(struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);// hdmi_dev			
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HOT_PLUG_IN, (UINT32)aui_hdmi_plug_in_callback)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_HOT_PLUG_IN is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_DBG_MSG: {
//(*(aui_handle_hdmi **)pp_hdl_hdmi)->pst_dev_hdmi=(struct hdmi_device *)dev_get_by_id(HLD_DEV_TYPE_HDMI, 0);// hdmi_dev			
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_DBG_MSG, (UINT32)hdmi_callback_func)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_DBG_MSG is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_CEC_MESSAGE: {
			cb_cec_msgrcv.fnCallback = hdmi_callback_func;
			cb_cec_msgrcv.pvUserData = p_user_data;
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_CEC_MESSAGE, (UINT32)aui_hdmi_cec_receive_callback)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_CEC_MESSAGE is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_HDCP_FAIL: {
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HDCP_FAIL, (UINT32)hdmi_callback_func)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_HDCP_FAIL is error!\n");
			}
			break;
		}
		default:
			AUI_ERR("hdmi cb type error\n");
			break;
	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
AUI_RTN_CODE	aui_hdmi_callback_del(aui_hdl p_hdl_hdmi, aui_hdmi_cb_type hdmicbtype, void * hdmi_callback_func,void * p_user_data)
{
	(void)hdmi_callback_func;
	(void) p_user_data;

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	if(hdmicbtype>AUI_HDMI_CB_HDCP_FAIL) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n hdmicbtype is error!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	
	osal_mutex_unlock(s_mod_mutex_id_hdmi);
	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	switch(hdmicbtype) {
		case AUI_HDMI_CB_EDID_READY: {
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_EDID_READY, (UINT32)NULL)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_EDID_READY is error!\n");
			}
			break;
		}
	
		case AUI_HDMI_CB_HOT_PLUG_OUT: {
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HOT_PLUG_OUT, (UINT32)NULL)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_HOT_PLUG_OUT is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_HOT_PLUG_IN: {
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HOT_PLUG_IN, (UINT32)NULL)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_HOT_PLUG_IN is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_DBG_MSG: {
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_DBG_MSG, (UINT32)NULL)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_DBG_MSG is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_CEC_MESSAGE: {
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_CEC_MESSAGE, (UINT32)NULL)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_CEC_MESSAGE is error!\n");
			}
			break;
		}
		
		case AUI_HDMI_CB_HDCP_FAIL:	{	
			if(RET_SUCCESS!=hdmi_proc_ioctl(hdmi_dev, HDMI_CMD_REG_CALLBACK, HDMI_CB_HDCP_FAIL, (UINT32)NULL)) {
				osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,"\n HDMI_CB_HDCP_FAIL is error!\n");
			}
			break;
		}
		
		default:
			AUI_ERR("hdmi cb type error\n");
			break;
	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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

	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	//osal_mutex_lock(OSAL_ID id,UINT32 timeout)(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
			osal_mutex_unlock(s_mod_mutex_id_hdmi);
			aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
		}

	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	switch(en_propety_type)
	{
		case AUI_HDMI_VENDOR_NAME: {
			aui_vendorname_length = *p_len;
			strncpy((char*)(aui_hdmiproperty.vendor_name), p_data, aui_vendorname_length);
			break;
		}

		case AUI_HDMI_PRODUCT_DESCRIPTION: {
			aui_prodes_length = *p_len;
			strncpy((char*)aui_hdmiproperty.product_description, p_data, aui_prodes_length);
			AUI_DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>(%d, %s, %d)\n",en_propety_type, aui_hdmiproperty.product_description, aui_prodes_length);
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
			AUI_ERR("propety type error\n");
			break;
	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
AUI_RTN_CODE	aui_hdmi_property_get(aui_hdl p_hdl_hdmi, aui_hdmi_property_item en_propety_type, void* p_data, int* p_len)
{
	aui_hdmi_hdcp_key_info hdcp_key_temp;
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(NULL==(void*)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);

	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi) {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}

	switch(en_propety_type)
	{
		case AUI_HDMI_VENDOR_NAME: {
			strncpy(p_data, (char*)aui_hdmiproperty.vendor_name, aui_vendorname_length);
			(*p_len) = aui_vendorname_length;
			break;
		}
		
		case AUI_HDMI_PRODUCT_DESCRIPTION: {
			strncpy(p_data, (char*)aui_hdmiproperty.product_description, aui_prodes_length);
			(*p_len) = aui_prodes_length;
			AUI_DBG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>(%d, %s, %d)\n",en_propety_type, aui_hdmiproperty.product_description, *p_len);
			break;
		}
		
		case AUI_HDMI_HDCP_KEY: {
			hdcp_key_temp = *(aui_hdmi_hdcp_key_info *)p_data;
			MEMCPY(hdcp_key_temp.puc_hdcp_ksv, aui_hdmiproperty.hdcp_key_info.puc_hdcp_ksv, aui_hdmiproperty.hdcp_key_info.n_ksv_length);
			MEMCPY(hdcp_key_temp.puc_encrypted_hdcp_keys, aui_hdmiproperty.hdcp_key_info.puc_encrypted_hdcp_keys, aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length);
			hdcp_key_temp.n_ksv_length= aui_hdmiproperty.hdcp_key_info.n_ksv_length;
			hdcp_key_temp.n_hdcp_keys_length = aui_hdmiproperty.hdcp_key_info.n_hdcp_keys_length;
			break;
		}
		 
		case AUI_HDMI_LINK_STATUS: {
			*(aui_hdmi_link_status *)p_data = aui_hdmiproperty.link_status;
			break;
		}

		default:
			AUI_ERR("propety type error\n");
			break;

	}
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
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
	osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_hdmi)
		||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
		||(INVALID_ID==((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
		osal_mutex_unlock(s_mod_mutex_id_hdmi);
		aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
	}
	osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_hdmi);

	if(mem_sel==0) {
		((aui_handle_hdmi *)p_hdl_hdmi)->hdcp_key_mode = 0;
	} else if (mem_sel==1) {
		((aui_handle_hdmi *)p_hdl_hdmi)->hdcp_key_mode = 1;
	} else {
		osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"\n mem_sel is error!\n");
	}
	api_set_hdmi_mem_sel(mem_sel);
	AUI_DBG("set hdcp mode %d\n",mem_sel);
	osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_hdmi_set_color(aui_hdl p_hdl_hdmi, aui_hdmi_color_space color)
{
	unsigned long hdmi_color;
	(void)p_hdl_hdmi;
	switch(color)
	{
	    case AUI_HDMI_YCBCR_422: {
            hdmi_color = YCBCR_422; 
            break;
        }
    
	    case AUI_HDMI_YCBCR_444: {
            hdmi_color = YCBCR_444; 
            break;
        }
        
	    case AUI_HDMI_RGB_MODE1: {
            hdmi_color = RGB_MODE1; 
            break;
            }
        
	    case AUI_HDMI_RGB_MODE2: {
            hdmi_color = RGB_MODE2; 
            break;
        }
        
	    default: {
		    AUI_ERR("ERROR color value !");
		    return AUI_RTN_EINVAL;
        }
	}
	
	vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0), VPO_IO_HDMI_OUT_PIC_FMT, hdmi_color);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_hdmi_set_avblank(aui_hdl p_hdl_hdmi, unsigned char blank)
{
	(void)p_hdl_hdmi;
	if(blank)
		api_set_av_blank(TRUE);//blank av 
	else
		api_set_av_blank(FALSE);
	
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_hdmi_set_avmute(aui_hdl p_hdl_hdmi, unsigned char flag)
{
	(void)p_hdl_hdmi;
	if(flag)
		api_set_av_mute(TRUE);
	else
		api_set_av_mute(FALSE);
	
	return AUI_RTN_SUCCESS;
}


/**
 * @brief         set CEC feature on/off
 * @author     
 * @date          
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  bOnOff 	      set CEC feature on/off.
 *                                1: CEC on; 0:CEC off
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          You can use this API to set CEC feature on or off 
 *
 */
AUI_RTN_CODE aui_hdmi_cec_set_onoff(aui_hdl p_hdl_hdmi, unsigned char bOnOff)
{
    osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_hdmi)
        ||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
        ||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_hdmi);
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
    }
    osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_hdmi);

    api_set_hdmi_cec_onoff(bOnOff);

    osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
    return AUI_RTN_SUCCESS;

}

/**
 * @brief         get CEC feature enable or not
 * @author     
 * @date          
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  bOnOff 	      point to get CEC feature on/off.
 *                                1: CEC on; 0:CEC off
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          You can use this API to know CEC feature enable or not
 *
 */
AUI_RTN_CODE aui_hdmi_cec_get_onoff(aui_hdl p_hdl_hdmi, unsigned char *bOnOff)
{
    osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_hdmi)
        ||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
        ||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_hdmi);
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
    }    
    osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_hdmi);

    *bOnOff = api_get_hdmi_cec_onoff();

    osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
    return AUI_RTN_SUCCESS;    
}

/**
 * @brief         set STB logical address
 * @author     
 * @date          
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  LogicalAddr 	  STB logical address.
 *                                
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          You can use this API to set STB's logical address
 *
 */
AUI_RTN_CODE aui_hdmi_cec_set_logical_address(aui_hdl p_hdl_hdmi, unsigned char LogicalAddr)
{
    int ret = AUI_RTN_SUCCESS;
    osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_hdmi)
        ||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
        ||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_hdmi);
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");
    }
    osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_hdmi);

    if(LogicalAddr>0xf) {
        osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"\n LogicalAddr is error!\n");
    }

    ret = api_set_logical_address(LogicalAddr);
    if(ret != AUI_RTN_SUCCESS) {
        osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"\n api_set_logical_address is error!\n");       
    }
    osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
    return AUI_RTN_SUCCESS;        
}

/**
 * @brief         get STB logical address
 * @author     
 * @date          
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  LogicalAddr 	      point to get STB logical address.
 *                                
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          You can use this API to get STB's logical address
 *
 */
AUI_RTN_CODE aui_hdmi_cec_get_logical_address(aui_hdl p_hdl_hdmi, unsigned char* LogicalAddr)
{
    osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_hdmi)
        ||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
        ||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_hdmi);
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");   
    }
    osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_hdmi);

    if(NULL==LogicalAddr) {
        osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");  
    }

   *LogicalAddr = api_get_logical_address();    
    osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);
    return AUI_RTN_SUCCESS;        
}

/**
 * @brief         transmit CEC message
 * @author     
 * @date          
 * @param[in]	  p_hdl_hdmi 	  aui hdmi handle.
 * @param[in]	  *buf 	      message that need to transmit
 * @param[in]	  length         message length
 *                                
 * @return        AUI_RTN_SUCCESS success.
 * @return        AUI_RTN_EINVAL  invalid input parameter.
 * @return        others          failed
 * @note          Use this API to send CEC message.
 *
 *                    buf[0]: address
 *                    buf[1]: opcode
 *                    buf[2]~: operand 
 */
AUI_RTN_CODE aui_hdmi_cec_transmit(aui_hdl p_hdl_hdmi, unsigned char *msg, unsigned char msg_length)
{
    int cec_ret = RET_SUCCESS;
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;

    osal_mutex_lock(s_mod_mutex_id_hdmi,OSAL_WAIT_FOREVER_TIME);
    
    if((NULL==p_hdl_hdmi)
        ||(NULL==((aui_handle_hdmi *)p_hdl_hdmi)->pst_dev_hdmi)
        ||(NULL==(void *)((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_hdmi);
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n");  
    }
    osal_mutex_lock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_hdmi);	
    cec_ret = api_hdmi_cec_transmit(msg,msg_length);
    
    if(RET_SUCCESS != cec_ret) {
        AUI_ERR("send cec msg fail, error = %d \n",cec_ret);
        aui_ret =  AUI_RTN_FAIL;
    }

    osal_mutex_unlock(((aui_handle_hdmi *)p_hdl_hdmi)->dev_mutex_id);

    return aui_ret;       
}

