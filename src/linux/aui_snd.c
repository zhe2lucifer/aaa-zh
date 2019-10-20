#include "aui_common_priv.h"
#include "aui_snd.h"
#include <alislsnd.h>


AUI_MODULE(SND)

void *g_snd_deca_handle = NULL;

typedef struct aui_st_handle_snd
{
	aui_dev_priv_data dev_priv_data;
    void *aui_deca_snd_handle;
    aui_attr_snd attr_snd;
    struct aui_snd_io_reg_callback_para cb_nodes[AUI_SND_CB_MONITOR_LAST];
}aui_handle_snd,*aui_p_handle_snd;

static aui_handle_snd *s_snd_device = NULL;

static enum aui_snd_cbtype sl2aui_snd_cb_type(snd_deca_cb_type type)
{
    enum aui_snd_cbtype aui_type = AUI_SND_CB_MONITOR_LAST;
    
    switch(type)
    {
        case SL_SND_MONITOR_REMAIN_DATA_BELOW_THRESHOLD:
            aui_type = AUI_SND_CB_MONITOR_REMAIN_DATA_BELOW_THRESHOLD; 
            break;
        case SL_SND_MONITOR_OUTPUT_DATA_END:
            aui_type = AUI_SND_CB_MONITOR_OUTPUT_DATA_END;
            break;
        case SL_SND_MONITOR_MIX_DATA_END:
            aui_type = AUI_SND_CB_MONITOR_MIX_DATA_END;
            break;
        case SL_SND_MONITOR_ERRORS_OCCURED :
            aui_type = AUI_SND_CB_MONITOR_ERRORS_OCCURED;
            break;
        default:
            AUI_ERR("unsupported type: %d \n", type);
            aui_type = AUI_SND_CB_MONITOR_LAST;
            break;
    }
    return aui_type;
}

static snd_deca_cb_type aui2sl_snd_cb_type(enum aui_snd_cbtype type)
{
    snd_deca_cb_type sl_type = SL_DECA_CB_TYPE_MAX;
    
    switch(type)
    {
        case AUI_SND_CB_MONITOR_REMAIN_DATA_BELOW_THRESHOLD:
            sl_type = SL_SND_MONITOR_REMAIN_DATA_BELOW_THRESHOLD;
            break;
        case AUI_SND_CB_MONITOR_OUTPUT_DATA_END:
            sl_type = SL_SND_MONITOR_OUTPUT_DATA_END;
            break;
        case AUI_SND_CB_MONITOR_MIX_DATA_END:
        case AUI_SND_CB_MONITOR_MIX_PAUSED:
            sl_type = SL_SND_MONITOR_MIX_DATA_END;
            break;
        case AUI_SND_CB_MONITOR_ERRORS_OCCURED:
            sl_type = SL_SND_MONITOR_ERRORS_OCCURED;
            break; 
        default:
            AUI_ERR("unsupported type: %d \n", type);
            sl_type = SL_DECA_CB_TYPE_MAX;
            break;
    }
    return sl_type;
}

static void aui_snd_common_cb(snd_deca_cb_type type, uint32_t param1, uint32_t param2)
{
    if(NULL == s_snd_device) {
        AUI_ERR("s_snd_device is NULL\n");
        return;
    }
    
    if(type >= SL_DECA_CB_TYPE_MAX) {
        AUI_ERR("type:%d is invalid.\n", type);
        return;
    }
    
    enum aui_snd_cbtype aui_type = sl2aui_snd_cb_type(type);

    if (type == SL_SND_MONITOR_MIX_DATA_END && param1 == SL_SND_MIX_PAUSED) {
        aui_type = AUI_SND_CB_MONITOR_MIX_PAUSED;
    }

    AUI_DBG("aui_cb_type: %d, cb: %p\n", aui_type, s_snd_device->cb_nodes[aui_type].p_cb);
    if((aui_type < AUI_SND_CB_MONITOR_LAST)&&(NULL != s_snd_device->cb_nodes[aui_type].p_cb)) {
        s_snd_device->cb_nodes[aui_type].p_cb(s_snd_device->cb_nodes[aui_type].pv_param);
    }
    
    //just remove for compile warning
    (void)param1;
    (void)param2;
    //param1 = param1+param2;
}

static AUI_RTN_CODE aui_snd_callback_reg(void *sl_snd_handle, struct aui_snd_io_reg_callback_para *p_cb_node)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    int sl_ret = 0;
    audio_regcb_param_s regcb_param;
    
    if(NULL == sl_snd_handle) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
    if(p_cb_node->e_cbtype >= AUI_SND_CB_MONITOR_LAST)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    AUI_DBG("reg cb type: %d\n", p_cb_node->e_cbtype);
    memset(&regcb_param, 0, sizeof(regcb_param));
    regcb_param.timeout_threshold = p_cb_node->threshold;
    regcb_param.e_cb_type = aui2sl_snd_cb_type(p_cb_node->e_cbtype);
    sl_ret = alislsnd_reg_callback(sl_snd_handle, &regcb_param, aui_snd_common_cb);
    if(sl_ret)
    {
        AUI_ERR("reg cb type %d fail, sl_ret: %d\n", p_cb_node->e_cbtype, sl_ret);
        aui_ret = AUI_RTN_FAIL;
    }
    return aui_ret;
}

static AUI_RTN_CODE aui_snd_callback_unreg(void *sl_snd_handle, enum aui_snd_cbtype type)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    int sl_ret = 0;
    audio_regcb_param_s regcb_param;

    memset(&regcb_param, 0, sizeof(regcb_param));
    regcb_param.e_cb_type = aui2sl_snd_cb_type(type);

    sl_ret = alislsnd_reg_callback(sl_snd_handle, &regcb_param, NULL);
	if(sl_ret)
    {
		AUI_ERR("unreg cb type %d fail, sl_ret: %d\n", type, sl_ret);
        aui_ret = AUI_RTN_FAIL;
	}

	return aui_ret;
}

AUI_RTN_CODE aui_snd_version_get(unsigned long* const pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	*pul_version=AUI_MODULE_VERSION_NUM_SND;
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		initialize SOUND device
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_call_back_init 	callback function to initialize SOUND
*    @param[in]		pv_param 		    parameters of the callback function
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_init(p_fun_cb p_call_back_init,void *pv_param)
{
    /*if(g_snd_deca_handle == NULL)
    {
        if(0 != alislsnd_construct(&g_snd_deca_handle) || NULL == g_snd_deca_handle)
        {
    		aui_rtn(AUI_RTN_ENOMEM,"Construct fail!\n");
        }
    }*/

	if((NULL != p_call_back_init))
	{
        p_call_back_init(pv_param);
	}
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		de-initialize SOUND device
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_call_back_init 	callback function to de-initialize SOUND
*    @param[in]		pv_param            parameters of the callback function
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    /*
    if(g_snd_deca_handle != NULL)
    {
        if(0 != alislsnd_destruct(&g_snd_deca_handle) || NULL != g_snd_deca_handle)
        {
    		aui_rtn(AUI_RTN_FAIL,"Deconstruct fail!\n");
        }
    }
    */

	if((NULL != p_call_back_init))
	{
        p_call_back_init(pv_param);
	}
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		open SOUND device
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_attr_snd 		    set SOUND device attribute
*    @param[out]	pp_hdl_snd 		    ponint to SOUND device
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_open(const aui_attr_snd *p_attr_snd,aui_hdl* const pp_hdl_snd)
{
	
    if(p_attr_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"NULL attribute!\n");
    }

    if(p_attr_snd->uc_dev_idx != 0)
    {
		aui_rtn(AUI_RTN_EINVAL,"Only one snd module is supported!\n");
    }

	(*(aui_handle_snd **)pp_hdl_snd) = (aui_handle_snd *)malloc(sizeof(aui_handle_snd));
	if(NULL == (aui_handle_snd *)(*(aui_handle_snd **)pp_hdl_snd))
	{
		aui_rtn(AUI_RTN_ENOMEM,NULL);
	}
	memset((aui_handle_snd*)(*(aui_handle_snd**)pp_hdl_snd),0,sizeof(aui_handle_snd));

    /** we don't need any pre-configuration here,just copy it **/
    memcpy(&((*(aui_handle_snd **)pp_hdl_snd)->attr_snd), p_attr_snd, sizeof(aui_attr_snd));

    if(0 != alislsnd_open(&g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd open fail!\n");
    }

    if(g_snd_deca_handle == NULL)
    {
		aui_rtn(AUI_RTN_FAIL,"Snd module is not constructed!\n");
    }

    ((*(aui_handle_snd **)pp_hdl_snd)->dev_priv_data).dev_idx = p_attr_snd->uc_dev_idx;
    aui_dev_reg(AUI_MODULE_SND,*pp_hdl_snd);

    (*(aui_handle_snd **)pp_hdl_snd)->aui_deca_snd_handle = g_snd_deca_handle;
	#if 0  //this part code will be bug under multi-process environment, adjust vol process will change spdif/hdmi below setting.
	//set hdmi/spdif output pcm default,because snd driver hdmi/spdif output bypass default
	aui_snd_out_mode mode;
	mode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
	mode.snd_out_type = AUI_SND_OUT_HDMI;
	aui_snd_out_data_type_set(*pp_hdl_snd, mode);
	mode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
	mode.snd_out_type = AUI_SND_OUT_SPDIF;
	aui_snd_out_data_type_set(*pp_hdl_snd, mode);
	#endif
    s_snd_device = *(aui_handle_snd **)pp_hdl_snd;
	return AUI_RTN_SUCCESS;

}

/**
*    @brief 		close SOUND device
*    @author		ray.gong
*    @date			2013-5-20
*    @param[out]	p_hdl_snd 		    AUI SOUND device handle
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_close(aui_hdl p_hdl_snd)
{
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_close(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd close fail!\n");
    }

	aui_dev_unreg(AUI_MODULE_SND,p_hdl_snd);
	MEMSET(p_hdl_snd,0,sizeof(aui_handle_snd));
	FREE(p_hdl_snd);
	p_hdl_snd=NULL;
    s_snd_device = NULL;
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		start SOUND device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 			AUI SOUND device handle
*    @param[in]		p_attr_snd 		    set SOUND device attribute, can be set to NULL
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_start(aui_hdl p_hdl_snd,const aui_attr_snd *p_attr_snd)
{
	(void)p_attr_snd;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_start(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd start fail!\n");
    }

    return AUI_RTN_SUCCESS;

}

/**
*    @brief 		stop SOUND device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 			AUI SOUND device handle
*    @param[in]		p_attr_snd 		    set SOUND device attribute, can be set to NULL
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_stop(void * p_hdl_snd,const aui_attr_snd *p_attr_snd)
{
	(void)p_attr_snd;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_stop(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd stop fail!\n");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		pause SOUND device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 			AUI SOUND device handle
*    @param[in]		p_attr_snd 		    set SOUND device attribute, can be set to NULL
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note			need work with aui_snd_resume
*
*/
AUI_RTN_CODE aui_snd_pause(aui_hdl p_hdl_snd,const aui_attr_snd *p_attr_snd)
{
	(void)p_attr_snd;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_SND_IO_PAUSE_SND, 0))
    {
    	aui_rtn(AUI_RTN_FAIL,"Snd pause fail!\n");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		resume SOUND device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 			AUI SOUND device handle
*    @param[in]		p_attr_snd 		    set SOUND device attribute, can be set to NULL
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note			need work with aui_snd_resume
*
*/
AUI_RTN_CODE aui_snd_resume(aui_hdl p_hdl_snd,const aui_attr_snd *p_attr_snd)
{
	(void)p_attr_snd;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_SND_IO_RESUME_SND, 0))
    {
    	aui_rtn(AUI_RTN_FAIL,"Snd resume fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		status and parameters set interface for SOUND device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 			AUI SOUND device handle
*    @param[in]		ul_item 			set item
*    @param[in]		pv_param 		    set parameters
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_set(aui_hdl p_hdl_snd,unsigned long ul_item,void *pv_param)
{
    enum SndTrackMode track_mode;
    AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;

    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	if((NULL==pv_param)||(ul_item>=AUI_SND_SET_CMD_LAST))
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	switch(ul_item)
	{
		case AUI_SND_CHANNEL_MODE_SET:
		{
            if(AUI_SND_CHANNEL_MODE_SND_DUP_L == *((int *)(pv_param)))
            {
                track_mode = SND_TRACK_L;
            }
            else if(AUI_SND_CHANNEL_MODE_SND_DUP_R == *((int *)(pv_param)))
            {
                track_mode = SND_TRACK_R;
            }
            else if(AUI_SND_CHANNEL_MODE_SND_DUP_MONO == *((int *)(pv_param)))
            {
                track_mode = SND_TRACK_MONO;
            }
            else
            {
                //output stereo
        		track_mode = SND_TRACK_NONE; 
            }
            if(0 != alislsnd_set_trackmode(g_snd_deca_handle,track_mode,SND_IO_ALL))
            {
        		aui_rtn(AUI_RTN_FAIL,"Snd set trackmode fail!\n");
            }
            ((aui_handle_snd *)p_hdl_snd)->attr_snd.en_snd_channel_mode = (aui_snd_channel_mode)track_mode;
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_SYNC_DELAY_TIME:
		{
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_SET_SYNC_DELAY,(unsigned long)(*(unsigned long *)pv_param)))
            {
  			    aui_rtn(AUI_RTN_FAIL,"Snd set sync_delay fail!\n");
            }
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
        case AUI_SND_OUT_INTERFACE_SET:
        {
            enum SndIoPort snd_port;
            aui_snd_out_type_status snd_out_type = *((aui_snd_out_type_status *)(pv_param));
            
            switch(snd_out_type.snd_out_type)
            {
                case AUI_SND_OUT_I2SO:
                    snd_port = SND_IO_RCA;
                    (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
                    break;
                case AUI_SND_OUT_SPDIF:
                    (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
                    snd_port = SND_IO_SPDIF;
                    break;
                case AUI_SND_OUT_HDMI:
                    (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
                    snd_port = SND_IO_HDMI;
                    break;
                default:
                    {
                		aui_rtn(AUI_RTN_EINVAL,"Invalid snd output!\n");
                        break;
                    }
            }

            if(0 != alislsnd_set_mute(g_snd_deca_handle, snd_out_type.uc_enabel, snd_port))
            {
        		aui_rtn(AUI_RTN_FAIL,"Snd set interface fail!\n");
            }
            break;
        }
		case AUI_SND_REG_CB:
        {
            struct aui_snd_io_reg_callback_para *p_cb_param=pv_param;
            aui_handle_snd *snd_hdl = p_hdl_snd;
            
            if(p_cb_param->e_cbtype >= AUI_SND_CB_MONITOR_LAST)
            {
                rtn_code = AUI_RTN_EINVAL;
                break;
            }
            if(p_cb_param->p_cb)
            {
                memcpy(&snd_hdl->cb_nodes[p_cb_param->e_cbtype], p_cb_param, sizeof(struct aui_snd_io_reg_callback_para));
                AUI_DBG("type: %d, cb: %p\n", p_cb_param->e_cbtype, s_snd_device->cb_nodes[p_cb_param->e_cbtype].p_cb);
                rtn_code = aui_snd_callback_reg(g_snd_deca_handle, p_cb_param);
            }
            else
            {
                memset(&snd_hdl->cb_nodes[p_cb_param->e_cbtype], 0, sizeof(struct aui_snd_io_reg_callback_para));
                rtn_code = aui_snd_callback_unreg(g_snd_deca_handle, p_cb_param->e_cbtype);
            }
            break;
		}
        case AUI_SND_SPDIF_DELAY_TIME: {
            unsigned int delay_time_ms = *(unsigned int *)pv_param;
            if (AUI_SPDIF_DELAY_TIME_MAX < delay_time_ms) {
                aui_rtn(AUI_RTN_FAIL, "Snd SPDIF delay time setting is invalid! Must be within the scope of 0 to 250 ms.\n");
            }
            if (0 != alislsnd_ioctl(g_snd_deca_handle, SND_SND_ONLY_SET_SPDIF_DELAY_TIME, delay_time_ms)) {
                aui_rtn(AUI_RTN_FAIL, "Snd set spdif delay fail!\n");
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default:
            aui_rtn(AUI_RTN_FAIL, "aui_snd_set error!\n");
            break;
    }


    return rtn_code;
}

/**
*    @brief 		status and parameters get interface for SOUND device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 			AUI SOUND device handle
*    @param[in]		ul_item 			get item
*    @param[out]	pv_param 		    get parameters
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_get(aui_hdl p_hdl_snd,unsigned long ul_item,void *pv_param)
{
	if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if((NULL==pv_param)||(ul_item>=AUI_SND_SET_CMD_LAST))
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

    switch(ul_item)
	{
		case AUI_SND_VOL_GET:
		{
			//*((unsigned char *)pv_param)= snd_get_volume(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd);
			break;
		}
		case AUI_SND_MUTE_GET:
		{
			//*(unsigned char *)pv_param=((aui_handle_snd *)p_hdl_snd)->attr_snd.uc_snd_mute_status;
			break;
		}
		case AUI_SND_CHANNEL_MODE_GET:
		{
			*(enum aui_snd_channel_mode *)pv_param=((aui_handle_snd *)p_hdl_snd)->attr_snd.en_snd_channel_mode;
			break;
		}
		case AUI_SND_OUT_INTERFACE_GET:
		{
			enum SndIoPort snd_port;
            bool mute_state;
            aui_snd_out_type_status *p_snd_out_type = (aui_snd_out_type_status *)pv_param;
            
            switch(p_snd_out_type->snd_out_type)
            {
                case AUI_SND_OUT_I2SO:
                    snd_port = SND_IO_RCA;
                    break;
                case AUI_SND_OUT_SPDIF:
                    snd_port = SND_IO_SPDIF;
                    break;
                case AUI_SND_OUT_HDMI:
                    snd_port = SND_IO_HDMI;
                    break;
                default:
                    {
                		aui_rtn(AUI_RTN_EINVAL,"Invalid snd output!\n");
                        break;
                    }
            }

            if(0 != alislsnd_get_mute_state(g_snd_deca_handle, snd_port ,&mute_state))
            {
        		aui_rtn(AUI_RTN_FAIL,"Snd get mute state fail!\n");
            }

            p_snd_out_type->uc_enabel = mute_state;
			break;
		}
		case AUI_SND_SPDIF_DELAY_TIME_GET:
			{
				unsigned int spdif_delay_time=0;
				if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_ONLY_GET_SPDIF_DELAY_TIME, (unsigned int)&spdif_delay_time))
	            {
	  			    aui_rtn(AUI_RTN_FAIL,"Snd get spdif delay fail!\n");
	            }
				*(unsigned int *)pv_param=spdif_delay_time;
			}
			break;
		case AUI_SND_SAMPLE_RATE_GET:
		{
			break;
		}
		case AUI_SND_HDMI_BYPASS_GET:
		{
			break;
		}
		case AUI_SND_SPIDF_BYPASS_GET:
		{
			break;
		}
		case AUI_SND_DAC_MUTE_GET:
		{
			break;
		}
		case AUI_SND_SYNC_LEVEL_GET:
		{
			//*(unsigned long *)pv_param=((aui_handle_snd *)p_hdl_snd)->attr_snd.ul_sync_level;
			break;
		}
		default:
		{
			break;
		}
	}
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set volume
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		uc_vol 			    volume value
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_vol_set(aui_hdl p_hdl_snd,unsigned char uc_vol)
{
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_set_volume(g_snd_deca_handle,uc_vol,SND_IO_ALL))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd volume set fail!\n");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get volume
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[out]    uc_vol              volume value
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_vol_get(aui_hdl p_hdl_snd,unsigned char* const puc_vol)
{
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_get_volume(g_snd_deca_handle,SND_IO_ALL,puc_vol))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd volume get fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set SOUND mute
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		uc_enabel 		    mute control, TRUE:mute;FALSE:unmute
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_mute_set(aui_hdl p_hdl_snd,unsigned char uc_enabel)
{
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_set_mute(g_snd_deca_handle, uc_enabel, SND_IO_ALL))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd set mute fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get mute status
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[out]	puc_enabel 		    point to mute status
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_mute_get(aui_hdl p_hdl_snd,unsigned char* const puc_enabel)
{
    bool statues;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_get_mute_state(g_snd_deca_handle, SND_IO_ALL, &statues))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd get mute fail!\n");
    }

    if(statues)
    {
        *puc_enabel = 1;
    }
    else
    {
        *puc_enabel = 0;
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get underrun
*    @date			2017-1-22
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		underrun_times	    underrun value
*    @return 		AUI_RTN_SUCCESS     success
*    @return 		AUI_RTN_EINVAL      invalid parameters
*    @return 		others  		    failed
*    @note		this function is only used for nestor test.Fobidden to use it other destination. 
*
*/
AUI_RTN_CODE aui_snd_underrun_times_get(aui_hdl p_hdl_snd,unsigned char* const underrun_times)
{
	if((p_hdl_snd == NULL) || (underrun_times == NULL)){
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
	}
	if(0 != alislsnd_get_underrun_times(g_snd_deca_handle,SND_IO_ALL,underrun_times)){
		aui_rtn(AUI_RTN_FAIL,"Snd get underrun fail!\n");
	}
	AUI_DBG("\r\n-> 		   [%s]    :[%d] \r\n",__FUNCTION__,*underrun_times);
	return AUI_RTN_SUCCESS;
}


/**
*    @brief 		CVBS,SPDIF and HDMI interface set, enable or disable
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		snd_out_type 		output interface type and output enable set
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note			used for SPDIF and HDMI output enable or disable, not for CVBS.
*
*/
AUI_RTN_CODE aui_snd_out_interface_type_set(aui_hdl p_hdl_snd,aui_snd_out_type_status snd_out_type)
{
    AUI_RTN_CODE aui_rtn=AUI_RTN_FAIL;

	if(snd_out_type.snd_out_type>=AUI_SND_OUT_LAST)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	aui_rtn=aui_snd_set(p_hdl_snd, AUI_SND_OUT_INTERFACE_SET, &snd_out_type);
	if(AUI_RTN_SUCCESS!=aui_rtn)
	{
		return aui_rtn;
	}
	AUI_DBG("type %d enable %d\n", snd_out_type.snd_out_type,snd_out_type.uc_enabel);
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		CVBS,SPDIF and HDMI interface get, enable or disable
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		snd_out_type 		output interface type and output enable set
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note			used for SPDIF and HDMI output enable or disable, not for CVBS.
*
*/
AUI_RTN_CODE aui_snd_out_interface_type_get(aui_hdl p_hdl_snd,aui_snd_out_type_status* const p_snd_out_type)
{
    AUI_RTN_CODE aui_rtn=AUI_RTN_FAIL;

	aui_rtn=aui_snd_get(p_hdl_snd,AUI_SND_OUT_INTERFACE_GET, p_snd_out_type);
	if(AUI_RTN_SUCCESS!=aui_rtn)
	{
		return aui_rtn;
	}
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set output interface data type
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		snd_out_type 		data type
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_out_data_type_set(aui_hdl p_hdl_snd,aui_snd_out_mode snd_out_type)
{
    enum SndOutFormat data_type;
    aui_snd_out_type interface_type;

    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    
    interface_type = snd_out_type.snd_out_type;
    if((AUI_SND_OUT_I2SO == interface_type)&&(snd_out_type.snd_data_type != AUI_SND_OUT_MODE_DECODED))
    {
		aui_rtn(AUI_RTN_EINVAL,"CVBS do not support undecoded data!\n");
    }
    switch(snd_out_type.snd_data_type)
    {
        case AUI_SND_OUT_MODE_DECODED:
            data_type = SND_OUT_FORMAT_PCM;
            break;
        case AUI_SND_OUT_MODE_ENCODED:
            data_type = SND_OUT_FORMAT_BS;
            break;
        case AUI_SND_OUT_MODE_FORCE_DD:
            data_type = SND_OUT_FORMAT_FORCE_DD;
            break;
        default:
            {
        		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
            }
            break;
    }
    /*
    refer adf_snd.h
    enum spdif_output_data_type
    {
    	SPDIF_OUT_PCM = 0,//!<The data format is pcm.
    	SPDIF_OUT_DD,//!<The data format is dd.
    };
    enum hdmi_output_data_type
    {
        HDMI_OUT_PCM = 0,//!<The data format is pcm.
        HDMI_OUT_DD,//!<The data format is dd by the trancoding.
        HDMI_OUT_BS,//!<The data format is same as the bitstream format.
    };
    */
    if(SND_OUT_FORMAT_FORCE_DD == data_type)
    {
        if(AUI_SND_OUT_HDMI == interface_type)
        {
            if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_DOLBYPLUS_CONVERT_ONOFF, 1))
            {
  			    aui_rtn(AUI_RTN_FAIL,"SND_DECA_DOLBYPLUS_CONVERT_ON fail!\n");
            }
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_IO_DDP_SPO_INTF_CFG, 1))
            {
  			    aui_rtn(AUI_RTN_FAIL,"dd@HDMI fail!\n");
            }        
        }
        else if(AUI_SND_OUT_SPDIF == interface_type)
        {
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_IO_SPO_INTF_CFG, 1))
            {
  			    aui_rtn(AUI_RTN_FAIL,"dd@SPDIF fail!\n");
            }
        }
    }
    else if(SND_OUT_FORMAT_BS == data_type)
    {
        if(AUI_SND_OUT_HDMI == interface_type)
        {
            if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_DOLBYPLUS_CONVERT_ONOFF, 0))
            {
  			    aui_rtn(AUI_RTN_FAIL,"SND_DECA_DOLBYPLUS_CONVERT_OFF fail!\n");
            }
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_IO_DDP_SPO_INTF_CFG, 2))
            {
  			    aui_rtn(AUI_RTN_FAIL,"bs@hdmi fail!\n");
            }
            
        }
        else if(AUI_SND_OUT_SPDIF == interface_type)
        {
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_IO_SPO_INTF_CFG, 1))
            {
  			    aui_rtn(AUI_RTN_FAIL,"bs->dd@spdif fail!\n");
            }
        }
    }
    else if(SND_OUT_FORMAT_PCM == data_type)
    {
        if(AUI_SND_OUT_HDMI == interface_type)
        {
            #if 0
            if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_DOLBYPLUS_CONVERT_ONOFF, 1))
            {
  			    aui_rtn(AUI_RTN_FAIL,"SND_DECA_DOLBYPLUS_CONVERT_ON fail!\n");
            }
            #endif
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_IO_DDP_SPO_INTF_CFG, 0))
            {
  			    aui_rtn(AUI_RTN_FAIL,"pcm@hdmi fail!\n");
            }
        }
        else if(AUI_SND_OUT_SPDIF == interface_type)
        {
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_IO_SPO_INTF_CFG, 0))
            {
  			    aui_rtn(AUI_RTN_FAIL,"pcm@spdif fail!\n");
            }
        }
    }
    else
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid arg!\n");
    }
    if(AUI_SND_OUT_I2SO == interface_type)
    {
        (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_out_type = snd_out_type.snd_out_type;
        (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_data_type = snd_out_type.snd_data_type;
    }
    else if(AUI_SND_OUT_SPDIF == interface_type)
    {
        (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_out_type = snd_out_type.snd_out_type;
        (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_data_type = snd_out_type.snd_data_type;
    }
    else if(AUI_SND_OUT_HDMI == interface_type)
    {
        (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_out_type = snd_out_type.snd_out_type;
        (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_data_type = snd_out_type.snd_data_type;
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get output interface data type
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[out]    snd_out_type        data type
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/

AUI_RTN_CODE aui_snd_out_data_type_get(aui_hdl p_hdl_snd,aui_snd_out_mode* const p_snd_out_type)
{	
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }


    if(AUI_SND_OUT_I2SO == p_snd_out_type->snd_out_type)
    {
        p_snd_out_type->snd_data_type = (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_data_type;
    }
    else if(AUI_SND_OUT_SPDIF== p_snd_out_type->snd_out_type)
    {
        p_snd_out_type->snd_data_type = (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_data_type;
    }
    else if(AUI_SND_OUT_HDMI == p_snd_out_type->snd_out_type)
    {
        p_snd_out_type->snd_data_type = (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_data_type;
    }
    else
    {
        aui_rtn(AUI_RTN_FAIL,"Error snd out type!\n");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set SOUND sync level
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		ul_sync_level 	    sync level
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_sync_level_set(aui_hdl p_hdl_snd,unsigned long ul_sync_level)
{
	(void)p_hdl_snd;
	(void)ul_sync_level;
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get SOUND sync level
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		pul_sync_level      sync level
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_sync_level_get(aui_hdl p_hdl_snd,unsigned long* const pul_sync_level)
{
	(void)p_hdl_snd;
	(void)pul_sync_level;
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set SOUND channel
*    @author		amu.tu
*    @date			2015-4-3
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		en_channel 	        channel to be set
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_channel_set(aui_hdl p_hdl_snd,enum aui_snd_channel_mode en_channel)
{
	AUI_RTN_CODE aui_rtn=AUI_RTN_FAIL;

    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	aui_rtn=aui_snd_set(p_hdl_snd,AUI_SND_CHANNEL_MODE_SET, &en_channel);
	if(AUI_RTN_SUCCESS!=aui_rtn)
	{
		return aui_rtn;
	}
	AUI_DBG("set channel %d",en_channel);
	return AUI_RTN_SUCCESS;
}


/**
*    @brief 		get SOUND channel
*    @author		amu.tu
*    @date			2015-4-3
*    @param[in]		p_hdl_snd 		    AUI SOUND device handle
*    @param[in]		pen_channel         point to the got channel
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_snd_channel_get(aui_hdl p_hdl_snd,enum aui_snd_channel_mode *pen_channel)
{
	AUI_RTN_CODE aui_rtn=AUI_RTN_FAIL;

    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(pen_channel == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid arg!\n");
    }
	aui_rtn=aui_snd_get(p_hdl_snd,AUI_SND_CHANNEL_MODE_GET, pen_channel);
	if(AUI_RTN_SUCCESS!=aui_rtn)
	{
		return aui_rtn;
	}
	AUI_DBG("get channel %d", *pen_channel);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_desc_volume_offset_set (aui_hdl p_hdl_snd,int offset)
{
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_snd *)p_hdl_snd)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(offset < -3)
    {
        offset = -3;
    }
    else if(offset > 3)
    {
        offset = 3;
    }
	if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_SND_SET_DESC_VOLUME_OFFSET, offset))
    {
		aui_rtn(AUI_RTN_FAIL,"Snd volume set fail!\n");
    }
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_mix_balance_set (aui_hdl handle, int balance)
{
    aui_handle_snd *dev = (aui_handle_snd *)handle;
    
    if(handle == NULL) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_set_mix_balance(dev->aui_deca_snd_handle, balance)) {
        aui_rtn(AUI_RTN_FAIL,"Snd mix balance set fail!\n");
    }
    
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_i2s_output_capture_start(aui_hdl handle, aui_snd_i2s_output_capture_attr *p_attr)
{
    aui_handle_snd *dev = (aui_handle_snd *)handle;
    if (NULL == handle) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if (0 != alislsnd_pcm_capture_start(dev->aui_deca_snd_handle)) {
        aui_rtn(AUI_RTN_FAIL,"pcm cap start fail!\n");
    }
    (void)p_attr;
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_snd_i2s_output_capture_stop(aui_hdl handle)
{
    aui_handle_snd *dev = (aui_handle_snd *)handle;
    if (NULL == handle) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if (0 != alislsnd_pcm_capture_stop(dev->aui_deca_snd_handle)) {
        aui_rtn(AUI_RTN_FAIL,"pcm cap start fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_i2s_output_capture_buffer_get(aui_hdl handle, aui_snd_i2s_output_capture_buffer **pp_buffer, 
            unsigned long * pul_buffer_num)
{
    aui_handle_snd *dev = (aui_handle_snd *)handle;
    if ((NULL == handle)||(NULL == pp_buffer)||(NULL == pul_buffer_num)) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    sl_snd_capture_buffer *sl_cap_buff = NULL;
    int cnt = 0;
    *pp_buffer = NULL;
    *pul_buffer_num = 0;
    if (0 != alislsnd_pcm_capture_buf_info_get(dev->aui_deca_snd_handle, &sl_cap_buff, &cnt)) {
        aui_rtn(AUI_RTN_FAIL,"pcm cap info get fail!\n");
    }
    *pp_buffer = (aui_snd_i2s_output_capture_buffer *)sl_cap_buff;
    *pul_buffer_num = cnt;
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_snd_i2s_output_capture_buffer_release(aui_hdl handle, unsigned long ul_buffer_num)
{
    aui_handle_snd *dev = (aui_handle_snd *)handle;
    if (NULL == handle) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if (0 == ul_buffer_num)
        return AUI_RTN_SUCCESS;
    if (0 != alislsnd_pcm_capture_buf_rd_update(dev->aui_deca_snd_handle, ul_buffer_num)) {
        aui_rtn(AUI_RTN_FAIL,"pcm cap rd update fail!\n");
    }
    return AUI_RTN_SUCCESS;
}
