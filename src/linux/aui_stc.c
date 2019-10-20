/**@file
*    @brief 		AUI AV INJECTER module interface implement
*    @author		christian.xie
*    @date			2014-3-12
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_stc.h>
#include <alislavsync.h>

#include "aui_decv_priv.h"

AUI_MODULE(STC)

typedef struct aui_stc{
	aui_dev_priv_data data; /* struct for dev reg/unreg */
	alisl_handle handle;
} aui_stc_t;

/**
*   @brief      Initialize stc module
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  p_callback_init: Callback function, which is registered to 
*               initialize stc module.
*   @param[in]  pv_param: callback function parameter
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*/
AUI_RTN_CODE aui_stc_init(p_fun_cb p_callback_init, void *pv_param)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (p_callback_init) {
        p_callback_init(pv_param);
    }

    return ret; 
}

/**
*   @brief      De-initialize stc module
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  p_callback_de_init: Callback function, which is registered to 
*               de-initialize stc module.
*   @param[in]  pv_param: callback function parameter
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*/
AUI_RTN_CODE aui_stc_de_init(p_fun_cb p_callback_de_init, void *pv_param)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (p_callback_de_init) {
        p_callback_de_init(pv_param);
    }

    return ret;
}

/**
*   @brief      Open stc device 
*   @author     christian.xie
*   @date       2014-3-11
*   @param[out] stc_handle          Return STC device handle
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note		
*       
*/
AUI_RTN_CODE aui_stc_open(aui_hdl *stc_handle)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_stc_t *p_stc = NULL;
    p_stc = malloc(sizeof(aui_stc_t));

	if (p_stc) {
        memset(p_stc, 0, sizeof(aui_stc_t));
    } else {
        ret = AUI_RTN_FAIL;
        goto error_out;
    }

    if (0 != alislavsync_open(&(p_stc->handle))) {
    	free(p_stc);
        ret = AUI_RTN_FAIL;
        goto error_out;
    }

	p_stc->data.dev_idx = 0;
    *stc_handle = p_stc;

	aui_dev_reg(AUI_MODULE_STC, p_stc);

error_out:
    return ret;
}


/**
*   @brief      Close stc device 
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc_handle          STC device handle
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note		
*       
*/
AUI_RTN_CODE aui_stc_close(aui_hdl stc_handle)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_stc_t *p_stc = NULL;

    if (!stc_handle) {
        ret = AUI_RTN_FAIL;
        goto error_out;
    }

    p_stc = (aui_stc_t *)stc_handle;

    if (0 != alislavsync_close(p_stc->handle)) {
        ret = AUI_RTN_FAIL;
        goto error_out;
    }

	aui_dev_unreg(AUI_MODULE_STC, p_stc);

	if (p_stc) {
        free(p_stc);
        p_stc = NULL;
    }

error_out:
    return ret;
}


/**
*   @brief      Get current value of the STC timer.
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[out] val_out             stc value data
*                                   DVB mode: 45kHz,32bits
*                                   others: ms
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note		
*       
*/
AUI_RTN_CODE aui_stc_get(aui_hdl stc, unsigned long *val_out)
{
    aui_stc_t *p_stc = (aui_stc_t *)stc;;
    unsigned int stc_value;

    if (p_stc == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "stc has not been opened\n");
    }
    if (alislavsync_get_current_stc(p_stc->handle, &stc_value)) {
        aui_rtn(AUI_RTN_FAIL,"\r\n get stc\n");
    }  
    *val_out = stc_value;

    return AUI_RTN_SUCCESS;
}

/**
*   @brief      Set current value of the STC timer. 
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[in]  val                 stc valule data
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note		
*       
*/
AUI_RTN_CODE aui_stc_set(aui_hdl stc, unsigned long long val)
{
    // unimplemented
    (void) stc;
    (void) val;
    
    return AUI_RTN_SUCCESS;
}

/**
*   @brief      Pause/unpause the timer. 
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[in]  pause               pause/unpause
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note   When paused, \a ali_stc_get will alywas return the same value.		
*           Pausing STC should also pause the video display / audio playback.    
*/ 
AUI_RTN_CODE aui_stc_pause(aui_hdl stc, int pause)
{
    // unimplemented
    (void) stc;
    (void) pause;

    return AUI_RTN_SUCCESS;
}

/**
*   @brief      Change speed of the STC timer. 
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[in]  ppm                 correction to be made, in 1/1000000 units.
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note       All consecutive corrections are accumulative		
*              
*/ 
AUI_RTN_CODE aui_stc_change_speed(aui_hdl stc, int ppm)
{
    // unimplemented
    (void) stc;
    (void) ppm;

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_stc_avsync_set(aui_hdl stc, aui_stc_avsync_attr *avsync_attr)
{
	aui_stc_t *p_stc = (aui_stc_t *)stc;;
	struct avsync_pip_swap_param swap_param;
	unsigned char aui_video_id = 0;
	enum avsync_video_id sl_video_id = AVSYNC_VIDEO_NB;
	int see_dmx_id = 0;

    if (p_stc == NULL || avsync_attr == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "stc has not been opened\n");
    }
	memset(&swap_param, 0, sizeof(struct avsync_pip_swap_param));
	
    alislavsync_get_video_id(p_stc->handle, &sl_video_id);
  	if (aui_decv_get_video_id(avsync_attr->decv_handle, &aui_video_id)) {
  		aui_video_id = AVSYNC_VIDEO_NB;
    }
    swap_param.vdec_id = (enum avsync_video_id)aui_video_id;
    
    if (aui_decv_get_see_dmx_id(avsync_attr->decv_handle, &see_dmx_id)) {
    	see_dmx_id = 0;
    }
    swap_param.see_dmx_id = see_dmx_id;
	        
    switch(avsync_attr->data_type) {
    	case AUI_AV_DATA_TYPE_NIM_TS:
    		swap_param.data_type = AVSYNC_DATA_TYPE_TS;
    		swap_param.src_type = AVSYNC_FROM_TUNER;
    		break;
    	case AUI_AV_DATA_TYPE_RAM_TS:
    		swap_param.data_type = AVSYNC_DATA_TYPE_TS;
    		swap_param.src_type = AVSYNC_FROM_HDD_MP;
    		break;
    	case AUI_AV_DATA_TYPE_ES:
    		swap_param.data_type = AVSYNC_DATA_TYPE_ES;
    		swap_param.src_type = AVSYNC_FROM_HDD_MP;
    		break;
    	default:
    		aui_rtn(AUI_RTN_EINVAL, "invalid data type\n");
    		
    }
    switch(avsync_attr->sync_mode) {
    	case AUI_STC_AVSYNC_PCR:
    		swap_param.sync_mode = AVSYNC_PCR;
    		break;
    	case AUI_STC_AVSYNC_AUDIO:
    		swap_param.sync_mode = AVSYNC_AUDIO;
    		break;
    	case AUI_STC_AVSYNC_FREERUN:
    		swap_param.sync_mode = AVSYNC_AV_FREERUN;
    		break;
    	default:
    		swap_param.sync_mode = AVSYNC_AUDIO;
    		break;
    }

    /* set video_freerun_threadhold, when avsync mode is not freerun. */
    if (AUI_STC_AVSYNC_FREERUN != avsync_attr->sync_mode) {
        struct avsync_advance_param avsync_params;
        memset(&avsync_params, 0, sizeof(avsync_params));
        if (alislavsync_get_advance_params(p_stc->handle, &avsync_params)) {
            aui_rtn(AUI_RTN_FAIL, "alislavsync_get_advance_params return fail!\n");
        } else {
           /* Only change param video_freerun_threadhold.
            * If 0 is set, the driver will use the default value 15 second.
            */
            if (0 != avsync_attr->video_freerun_threadhold) {
                avsync_params.vfreerun_thres = (unsigned int)(avsync_attr->video_freerun_threadhold);
            }
            alislavsync_config_advance_params(p_stc->handle, &avsync_params);
        }
    }

	if(sl_video_id == AVSYNC_VIDEO_NB) {
		//AUI_DBG("========= set stc ========\n");
		if (alislavsync_set_data_type(p_stc->handle, swap_param.data_type))
			aui_rtn(AUI_RTN_FAIL,"\r\n set data type fail\n");		
		if (alislavsync_set_av_sync_mode(p_stc->handle, swap_param.sync_mode))
			aui_rtn(AUI_RTN_FAIL,"\r\n set sync mode fail\n");
		if (alislavsync_set_sourcetype(p_stc->handle, swap_param.src_type))
			aui_rtn(AUI_RTN_FAIL,"\r\n set src type fail\n");
		if (alislavsync_set_video_id(p_stc->handle, swap_param.vdec_id))
			aui_rtn(AUI_RTN_FAIL,"\r\n set video id fail\n");
		if (alislavsync_set_see_dmx_id(p_stc->handle, swap_param.see_dmx_id))
			aui_rtn(AUI_RTN_FAIL,"\r\n set see dmx id fail\n");
	} else {
		//AUI_DBG("========= swap audio stc: video id: %d, see dmx id: %d========\n", swap_param.vdec_id, swap_param.see_dmx_id);
		if (alislavsync_pip_swap_audio(p_stc->handle, &swap_param)) {
			aui_rtn(AUI_RTN_FAIL,"\r\n avsync swap audio fail\n");
		}
	}
	return AUI_RTN_SUCCESS;
}
