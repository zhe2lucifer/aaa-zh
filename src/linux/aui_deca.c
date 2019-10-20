#include "aui_common_priv.h"
#include <aui_deca.h>
#include <alislsnd.h>
#include <alislavsync.h>


AUI_MODULE(DECA)

static bool change_audio_track = false;

extern void *g_snd_deca_handle;

typedef struct aui_st_handle_snd
{
	aui_dev_priv_data dev_priv_data;
    void *aui_deca_snd_handle;
    aui_attr_deca attr_deca;
    alisl_handle avsync_handle;
    struct aui_st_deca_io_reg_callback_para cb_nodes[AUI_DECA_CB_MAX];
}aui_handle_deca,*aui_p_handle_deca;

static aui_handle_deca *s_deca_device = NULL;

static snd_deca_cb_type aui2sl_cb_type(aui_deca_cb_type_e type)
{
    snd_deca_cb_type sl_type = SL_DECA_CB_TYPE_MAX;
    
    switch(type)
    {
        case AUI_DECA_CB_FIRST_FRAME:
            sl_type = SL_DECA_FIRST_FRAME;
            break;
        case AUI_DECA_CB_MONITOR_NEW_FRAME:
            sl_type = SL_DECA_MONITOR_NEW_FRAME;
            break;
        case AUI_DECA_CB_MONITOR_START:
            sl_type = SL_DECA_MONITOR_START;
            break;
        case AUI_DECA_CB_MONITOR_STOP:
            sl_type = SL_DECA_MONITOR_STOP;
            break;
        case AUI_DECA_CB_MONITOR_DECODE_ERR:
            sl_type = SL_DECA_MONITOR_DECODE_ERR;
            break;
        case AUI_DECA_CB_MONITOR_OTHER_ERR:
            sl_type = SL_DECA_MONITOR_OTHER_ERR;
            break;
        default:
            AUI_ERR("unsupported type: %d \n", type);
            sl_type = SL_DECA_CB_TYPE_MAX;
            break;
    }
    return sl_type;
}

static aui_deca_cb_type_e sl2aui_cb_type(snd_deca_cb_type type)
{
    aui_deca_cb_type_e aui_type = AUI_DECA_CB_MAX;
    
    switch(type)
    {
        case SL_DECA_FIRST_FRAME:
            aui_type = AUI_DECA_CB_FIRST_FRAME; 
            break;
        case  SL_DECA_MONITOR_NEW_FRAME:
            aui_type = AUI_DECA_CB_MONITOR_NEW_FRAME;
            break;
        case SL_DECA_MONITOR_START :
            aui_type = AUI_DECA_CB_MONITOR_START;
            break;
        case SL_DECA_MONITOR_STOP:
            aui_type = AUI_DECA_CB_MONITOR_STOP;
            break;
        case SL_DECA_MONITOR_DECODE_ERR :
            aui_type = AUI_DECA_CB_MONITOR_DECODE_ERR;
            break;
        case SL_DECA_MONITOR_OTHER_ERR:
            aui_type = AUI_DECA_CB_MONITOR_OTHER_ERR;
            break;
        default:
            AUI_ERR("unsupported type: %d \n", type);
            aui_type = AUI_DECA_CB_MAX;
            break;
    }
    return aui_type;
}

static void aui_deca_common_cb(snd_deca_cb_type type, uint32_t param1, uint32_t param2)
{
    if(NULL == s_deca_device) {
		AUI_ERR("s_deca_device is NULL\n");
		return;
	}
//    if(type >= SL_DECA_CB_TYPE_MAX)
//    {
//        AUI_ERR("type:%d is invalid.\n", type);
//		return;
//    }
    aui_deca_cb_type_e aui_type = sl2aui_cb_type(type);
    if (aui_type >= AUI_DECA_CB_MAX) {
	    AUI_ERR("aui_deca_cb_type_e:%d is invalid.\n", aui_type);
        return;
    }
    if(NULL != s_deca_device->cb_nodes[aui_type].p_cb) {
		s_deca_device->cb_nodes[aui_type].p_cb((void *)param1, (void *)param2, 
          s_deca_device->cb_nodes[aui_type].pv_param);
	}
}

static AUI_RTN_CODE aui_deca_callback_reg(void *sl_deca_handle, struct aui_st_deca_io_reg_callback_para *p_cb_node)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    int sl_ret = 0;
    audio_regcb_param_s regcb_param;
    
    if(NULL == sl_deca_handle) {
		aui_rtn(AUI_RTN_FAIL, "deca_dev is NULL\n");
	}
	if (NULL == p_cb_node)
		aui_rtn(AUI_RTN_FAIL, "p_cb_node is NULL.\n");
    if(p_cb_node->en_cb_type >= AUI_DECA_CB_MAX)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    AUI_DBG("reg cb type: %d\n", p_cb_node->en_cb_type);
    memset(&regcb_param, 0, sizeof(regcb_param));
    regcb_param.monitor_rate = p_cb_node->monitor_rate;
    regcb_param.timeout_threshold = p_cb_node->timeout;
    regcb_param.e_cb_type = aui2sl_cb_type(p_cb_node->en_cb_type);
    if((SL_DECA_MONITOR_NEW_FRAME == regcb_param.e_cb_type) && (0 == regcb_param.monitor_rate))
    {
        /*
        #38876, callback is so frequent that platform will be abnormal. 
        */
        regcb_param.monitor_rate = 50; 
    }
    sl_ret = alislsnd_reg_callback(sl_deca_handle, &regcb_param, aui_deca_common_cb);
    if(sl_ret)
    {
        AUI_ERR("reg cb type %d fail, sl_ret: %d\n", p_cb_node->en_cb_type, sl_ret);
        aui_ret = AUI_RTN_FAIL;
    }
    return aui_ret;
}

static AUI_RTN_CODE aui_deca_callback_unreg(void *sl_deca_handle, aui_deca_cb_type_e type)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    int sl_ret = 0;
    audio_regcb_param_s regcb_param;

	if (NULL == sl_deca_handle)
		aui_rtn(AUI_RTN_FAIL, "sl_deca_handle is NULL.\n");
    memset(&regcb_param, 0, sizeof(regcb_param));
    regcb_param.e_cb_type = aui2sl_cb_type(type);

    sl_ret = alislsnd_reg_callback(sl_deca_handle, &regcb_param, NULL);
    if(sl_ret)
    {
	    AUI_ERR("unreg cb type %d fail, sl_ret: %d\n", type, sl_ret);
        aui_ret = AUI_RTN_FAIL;
	}

	return aui_ret;
}

AUI_RTN_CODE aui_deca_version_get(unsigned long* const pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	*pul_version=AUI_MODULE_VERSION_NUM_DECA;
	return AUI_RTN_SUCCESS;

}


/**
*    @brief 		initialize deca module
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_call_back_init 	callback function to initialize DECA
*    @param[in]		pv_param 		parameters of the callback function
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_init(p_fun_cb p_call_back_init,void *pv_param)
{
    /*
    if(g_snd_deca_handle == NULL)
    {
        if(0 != alislsnd_construct(&g_snd_deca_handle) || NULL == g_snd_deca_handle)
        {
    		aui_rtn(AUI_RTN_ENOMEM,"Deca construct fail!\n");
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
*    @brief 		de-initialize deca module
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_call_back_init 	callback function to de-initialize DECA
*    @param[in]		pv_param 		parameters of the callback function
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    /*
    if(g_snd_deca_handle != NULL)
    {
        if(0 != alislsnd_destruct(&g_snd_deca_handle) || NULL != g_snd_deca_handle)
        {
    		aui_rtn(AUI_RTN_FAIL,"deca deconstruct fail!\n");
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
*    @brief 		open deca device
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_attr_deca 		set deca device attribute
*    @param[out]	pp_hdl_deca 		point to aui deca device
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_open(const aui_attr_deca *p_attr_deca,aui_hdl* const pp_hdl_deca)
{
    aui_handle_deca *p_handle_deca = NULL;
    
    if (!pp_hdl_deca) {
        aui_rtn(AUI_RTN_EINVAL, "pp_hdl_deca is NULL\n");
    }
    if (p_attr_deca == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "deca NULL attribute!\n");
    }
    if ((p_attr_deca->uc_dev_idx != 0) && (p_attr_deca->uc_dev_idx != 1)) {
        aui_rtn(AUI_RTN_EINVAL, "Only index 0 and 1 deca is supported!\n");
    }

    p_handle_deca = (aui_handle_deca *)malloc(sizeof(aui_handle_deca));
    if (NULL == p_handle_deca) {
        aui_rtn(AUI_RTN_ENOMEM, "p_handle_deca is NULL, malloc p_handle_deca fail!\n");
    }
    memset(p_handle_deca, 0, sizeof(aui_handle_deca));
    
    /* we don't need any pre-configuration here, just copy it */
    memcpy(&(p_handle_deca->attr_deca), p_attr_deca, sizeof(aui_attr_deca));

    if (0 != alislsnd_open(&g_snd_deca_handle)) {
        free(p_handle_deca);
        p_handle_deca = NULL;
        aui_rtn(AUI_RTN_FAIL, "deca open fail!\n");
    }

    if (g_snd_deca_handle == NULL) {
        free(p_handle_deca);
        p_handle_deca = NULL;
        aui_rtn(AUI_RTN_FAIL, "deca module is not constructed!\n");
    }

    (p_handle_deca->dev_priv_data).dev_idx = p_attr_deca->uc_dev_idx;
    aui_dev_reg(AUI_MODULE_DECA, p_handle_deca);

    alislavsync_open(&(p_handle_deca->avsync_handle));

    p_handle_deca->aui_deca_snd_handle = g_snd_deca_handle;

    if (alislsnd_ioctl(g_snd_deca_handle, SND_DECA_INIT_TONE_VOICE, 0)) {
        free(p_handle_deca);
        p_handle_deca = NULL;
        /* To prevent the memory leak for alislsnd_open */
        if (NULL != g_snd_deca_handle) {
            alislsnd_close(g_snd_deca_handle);
        }
        aui_rtn(AUI_RTN_FAIL, "SND_DECA_INIT_TONE_VOICE error!\n");
    }
    
    *(aui_handle_deca **)pp_hdl_deca = p_handle_deca;
    
    if (p_attr_deca->uc_dev_idx == 0) { /* deca 1 used for mixing audio data */
        s_deca_device = p_handle_deca;
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		close deca device
*    @author		ray.gong
*    @date			2013-5-20
*    @param[in]		p_hdl_deca 		aui deca device handle
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_close(aui_hdl p_hdl_deca)
{
    if(p_hdl_deca == NULL) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_close(g_snd_deca_handle)) {
        aui_rtn(AUI_RTN_FAIL,"deca close fail!\n");
    }

    alislavsync_close(((aui_handle_deca *)p_hdl_deca)->avsync_handle);

    if (((aui_handle_deca *)p_hdl_deca)->attr_deca.uc_dev_idx == 0) {//deca 1 used for mixing audio data
        s_deca_device = NULL;
    }
    
    aui_dev_unreg(AUI_MODULE_DECA,p_hdl_deca);
    MEMSET(p_hdl_deca,0,sizeof(aui_handle_deca));
    FREE(p_hdl_deca);
    p_hdl_deca=NULL;   
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		start deca device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		aui deca device handle
*    @param[in]		p_attr_deca 	start parameters, reserved for future, now can be set to NULL
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_start(aui_hdl p_hdl_deca, const aui_attr_deca *p_attr_deca)
{
	(void)p_attr_deca;
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_start(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"deca start fail!\n");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		stop deca device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		aui deca device handle
*    @param[in]		p_attr_deca 	start parameters, reserved for future, now can be set to NULL
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_stop(aui_hdl p_hdl_deca, const aui_attr_deca *p_attr_deca)
{
	(void)p_attr_deca;
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_stop(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Deca stop fail!\n");
    }
    if (change_audio_track ==  true) {
        change_audio_track = false;
        if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_CHANGE_AUD_TRACK, 1)) {
    		    aui_rtn(AUI_RTN_FAIL,"Deca change audio track fail!\n");
		}
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		pause deca device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		aui deca device handle
*    @param[in]		p_attr_deca 	start parameters, reserved for future, now can be set to NULL
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note			need work with aui_deca_resume, after pause then need resume to work.
*
*/
AUI_RTN_CODE aui_deca_pause(aui_hdl p_hdl_deca, const aui_attr_deca *p_attr_deca)
{
	(void)p_attr_deca;
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_pause(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Deca pause fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		resume deca device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		aui deca device handle
*    @param[in]		p_attr_deca 	start parameters, reserved for future, now can be set to NULL
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note			need work with aui_deca_pause
*/
AUI_RTN_CODE aui_deca_resume(aui_hdl p_hdl_deca, const aui_attr_deca *p_attr_deca)
{
	(void)p_attr_deca;
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_resume(g_snd_deca_handle))
    {
		aui_rtn(AUI_RTN_FAIL,"Deca resume fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		status and parameters set interface for deca device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		aui deca device handle
*    @param[in]		ul_item 		set item
*    @param[in]		pv_param 		set parameters
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_set(aui_hdl p_hdl_deca,unsigned long ul_item,void *pv_param)
{
    AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;
    aui_handle_deca * deca_hdl = (aui_handle_deca *)p_hdl_deca;

    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && deca_hdl->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	if(ul_item>=AUI_DECA_SET_CMD_LAST)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	switch(ul_item) {
		case AUI_DECA_PREPARE_CHANGE_AUD_TRACK: {
    		change_audio_track = true;
    		break;
		}
	    case AUI_DECA_DEOCDER_TYPE_SET: {
            if(!pv_param)
            {
                AUI_ERR("decoder type pointer is null\n");
                return AUI_RTN_FAIL;
            }
	        aui_deca_type_set(p_hdl_deca, *((unsigned long*)pv_param));
	        break;
	    }
	    case AUI_DECA_AUD_SYNC_MODE_SET: {
            if (!pv_param) {
                AUI_ERR("decoder type pointer is null\n");
                return AUI_RTN_FAIL;
            }        
            
            enum avsync_sync_mode avsync_mod;
            unsigned long ul_deca_sync_mode = *((unsigned long *)pv_param); 

            if (ul_deca_sync_mode == AUI_DECA_DEOCDER_SYNC)
                avsync_mod = AVSYNC_PCR;        
            else if (ul_deca_sync_mode == AUI_DECA_DEOCDER_ASYNC)
                avsync_mod = AVSYNC_AV_FREERUN;
            else
        		aui_rtn(AUI_RTN_EINVAL, "Invalid parameter!\n");

            if (0 != alislavsync_set_av_sync_mode(((aui_handle_deca *)p_hdl_deca)->avsync_handle, avsync_mod)) {
        		aui_rtn(AUI_RTN_FAIL, "Deca set sync mode fail!\n");
                return AUI_RTN_FAIL;
            }
	        break;
	    }
        case AUI_DECA_REG_CB: {
            struct aui_st_deca_io_reg_callback_para *p_cb_param=pv_param;
            if(!p_cb_param || p_cb_param->en_cb_type >= AUI_DECA_CB_MAX)
            {
                rtn_code = AUI_RTN_EINVAL;
                break;
            }
            if(p_cb_param->p_cb)
            {
                memcpy(&deca_hdl->cb_nodes[p_cb_param->en_cb_type], p_cb_param, sizeof(struct aui_st_deca_io_reg_callback_para));
                rtn_code = aui_deca_callback_reg(g_snd_deca_handle, p_cb_param);
            }
            else
            {
                memset(&deca_hdl->cb_nodes[p_cb_param->en_cb_type], 0, sizeof(struct aui_st_deca_io_reg_callback_para));
                rtn_code = aui_deca_callback_unreg(g_snd_deca_handle, p_cb_param->en_cb_type);
            }
            break;
        }
        case AUI_DECA_ADD_BS_SET: {
            if(!pv_param)
            {
                AUI_ERR("bitstream type pointer is null\n");
                return AUI_RTN_FAIL;
            }
            enum aui_deca_stream_type tmp = *((unsigned int *)pv_param);
            enum Snd_decoder_type sl_type = SND_TYPE_INVALID;
            if(tmp == AUI_DECA_STREAM_TYPE_EC3)
            {
                sl_type = SND_TYPE_EC3;
            }
            else if(tmp == AUI_DECA_STREAM_TYPE_AC3)
            {
                sl_type = SND_TYPE_AC3;
            }
            else if(tmp == AUI_DECA_STREAM_TYPE_DTS)
            {
                sl_type = SND_TYPE_DTS;
            }
            else
            {
                AUI_ERR("this type %d not support bs out\n", *((unsigned int *)pv_param));
                return AUI_RTN_FAIL;
            }
			AUI_DBG("SND_DECA_ADD_BS_SET %u\n", sl_type);
            if(alislsnd_ioctl(g_snd_deca_handle, SND_DECA_ADD_BS_SET, sl_type))
            {
                AUI_ERR("SND_DECA_ADD_BS_SET %u fail\n", sl_type);
                rtn_code = AUI_RTN_FAIL;
            }
            break;
        }
        case AUI_DECA_EMPTY_BS_SET: {
            if(alislsnd_ioctl(g_snd_deca_handle, SND_DECA_EMPTY_BS_SET, 0))
            {
                AUI_ERR("SND_DECA_EMPTY_BS_SET %u fail\n", *((unsigned int *)pv_param));
                rtn_code = AUI_RTN_FAIL;
            }
            break;
        }
        default: {
            AUI_ERR("unsupported set type: %lu\n", ul_item);
            rtn_code = AUI_RTN_FAIL;
            break;
        }
    }

    return rtn_code;
}

static unsigned long switch_deca_type_from_drv_2_aui(unsigned long ul_drv_type)
{
    unsigned long aui_deca_type=AUI_DECA_STREAM_TYPE_LAST;
    switch(ul_drv_type)
    {
	   case SND_TYPE_MPEG1:
			aui_deca_type = AUI_DECA_STREAM_TYPE_MPEG1;
			break;
	   case SND_TYPE_MPEG2:
			aui_deca_type = AUI_DECA_STREAM_TYPE_MPEG2;
			break;
	   case SND_TYPE_MPEG_AAC:
			aui_deca_type = AUI_DECA_STREAM_TYPE_AAC_LATM;
			break;
	   case SND_TYPE_AC3:
			aui_deca_type = AUI_DECA_STREAM_TYPE_AC3;
			break;
	   case SND_TYPE_DTS:
			aui_deca_type = AUI_DECA_STREAM_TYPE_DTS;
			break;
	   case SND_TYPE_PPCM:
			aui_deca_type = AUI_DECA_STREAM_TYPE_PPCM;
			break;
	   case SND_TYPE_LPCM_V:
			aui_deca_type = AUI_DECA_STREAM_TYPE_LPCM_V;
			break;
	   case SND_TYPE_LPCM_A:
			aui_deca_type = AUI_DECA_STREAM_TYPE_LPCM_A;
			break;
	   case SND_TYPE_PCM:
			aui_deca_type = AUI_DECA_STREAM_TYPE_PCM;
			break;
	   case SND_TYPE_WMA:
	   		aui_deca_type = AUI_DECA_STREAM_TYPE_BYE1;
			break;
	   case SND_TYPE_RA8:
			aui_deca_type = AUI_DECA_STREAM_TYPE_RA8;
			break;
	   case SND_TYPE_MP3:
			aui_deca_type = AUI_DECA_STREAM_TYPE_MP3;
			break;
	   case SND_TYPE_INVALID:
			aui_deca_type = AUI_DECA_STREAM_TYPE_INVALID;
			break;
	   case SND_TYPE_MPEG_ADTS_AAC:
			aui_deca_type = AUI_DECA_STREAM_TYPE_AAC_ADTS;
			break;
	   case SND_TYPE_OGG:
			aui_deca_type = AUI_DECA_STREAM_TYPE_OGG;
			break;
	   case SND_TYPE_EC3:
			aui_deca_type = AUI_DECA_STREAM_TYPE_EC3;
			break;
       default:
            aui_deca_type=AUI_DECA_STREAM_TYPE_LAST;
            break;
    }
    return aui_deca_type;
}

/**
*    @brief 		status and parameters get interface for deca device
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		aui deca device handle
*    @param[in]		ul_item 		get item
*    @param[out]	pv_param 		get parameters
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_get(aui_hdl p_hdl_deca,unsigned long ul_item,void *pv_param)
{
    struct Snd_stream_info deca_info;
    AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;

    memset(&deca_info, 0, sizeof(deca_info));

    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	if((NULL==(aui_handle_deca *)p_hdl_deca)||(ul_item>AUI_DECA_GET_CMD_LAST))
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

    switch(ul_item)
    {
		case AUI_DECA_DATA_INFO_GET:
		{
			struct snd_deca_buf_info buf_info;
			struct avsync_statistics_info statistics;
			memset(&buf_info, 0, sizeof(buf_info));
			memset(&statistics, 0, sizeof(statistics));
			if(NULL==pv_param)
			{
				aui_rtn(AUI_RTN_EINVAL,NULL);
			}
            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_DECA_GET_PLAY_PARAM, (unsigned int)&deca_info))
            {
        		aui_rtn(AUI_RTN_FAIL,"Deca get play param fail!\n");
            }
            ((aui_deca_data_info *)pv_param)->uc_deca_type=switch_deca_type_from_drv_2_aui(deca_info.str_type);
			((aui_deca_data_info *)pv_param)->uc_bit_width=deca_info.bit_depth;
			((aui_deca_data_info *)pv_param)->ul_sample_rate=deca_info.sample_rate;
			((aui_deca_data_info *)pv_param)->ul_sample_cnt=deca_info.samp_num;
			((aui_deca_data_info *)pv_param)->ul_channel_cnt=deca_info.chan_num;
			((aui_deca_data_info *)pv_param)->ul_frame_cnt=deca_info.frm_cnt;
			((aui_deca_data_info *)pv_param)->ul_last_pts=deca_info.cur_frm_pts;
			((aui_deca_data_info *)pv_param)->ul_decode_error_cnt=deca_info.decode_error_cnt;

			if (0 != alislavsync_get_statistics(s_deca_device->avsync_handle, &statistics)) {
				aui_rtn(AUI_RTN_FAIL,"Deca get avsync statistics info fail!\n");
			}
			((aui_deca_data_info *)pv_param)->ul_drop_cnt=statistics.total_a_drop_cnt;

            if(0 != alislsnd_ioctl(g_snd_deca_handle,SND_DECA_GET_ES_BUFF_STATE, (unsigned int)&buf_info))
            {
        		aui_rtn(AUI_RTN_FAIL,"Deca get play param fail!\n");
            }
            ((aui_deca_data_info *)pv_param)->ul_buffer_total_size=buf_info.buf_len;
			((aui_deca_data_info *)pv_param)->ul_buffer_valid_size=buf_info.used_len;
			((aui_deca_data_info *)pv_param)->ul_buffer_free_size=buf_info.remain_len; 
			((aui_deca_data_info *)pv_param)->audio_mode.ac_mode = 0;

			if ((SND_TYPE_AC3 == deca_info.str_type) || (SND_TYPE_EC3 == deca_info.str_type)) {
				unsigned int acmod = 0xffffffff;
				if (0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_GET_DDP_INMOD, (unsigned int)&acmod)) {
	                aui_rtn(AUI_RTN_FAIL, "Deca get dd acmod fail!\n");
	            }
				((aui_deca_data_info *)pv_param)->audio_mode.ac_mode = acmod;
			}

			if ((SND_TYPE_MPEG1 == deca_info.str_type) || (SND_TYPE_MPEG2 == deca_info.str_type)) {
                unsigned int channel_mode = 0xffffffff;			
				if (0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_GET_AUDIO_INFO, (unsigned int)&channel_mode)) {
	        	    aui_rtn(AUI_RTN_FAIL, "Deca get channel_mode fail!\n");
	            } else {					
					((aui_deca_data_info *)pv_param)->audio_mode.channel_mode = channel_mode;					
				}
			}	
			
            rtn_code = AUI_RTN_SUCCESS;
			break;
		}

        case AUI_DECA_DEOCDER_STATUS_GET:
        {
            enum snd_status deca_state = DECA_STATUS_DETACH;
            if(NULL==pv_param)
			{
				aui_rtn(AUI_RTN_EINVAL,NULL);
			}
            if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_GET_DECA_STATE, (unsigned int)&deca_state))
            {
        		aui_rtn(AUI_RTN_FAIL,"deca ioctl SND_DECA_GET_DECA_STATE fail!\n");
            }
            if(DECA_STATUS_DETACH == deca_state)
            {
                *((unsigned long *)pv_param)=AUI_DECA_DEATTACH;
            } 
            else if(DECA_STATUS_ATTACH == deca_state)
            {
                *((unsigned long *)pv_param)=AUI_DECA_ATTACH;
            }
            else if(DECA_STATUS_IDLE == deca_state)
            {
                *((unsigned long *)pv_param)=AUI_DECA_STOP;
            }
            else if(DECA_STATUS_PLAY == deca_state)
            {
                *((unsigned long *)pv_param)=AUI_DECA_RUN;
            }
            else if(DECA_STATUS_PAUSE == deca_state)
            {
                *((unsigned long *)pv_param)=AUI_DECA_PAUSE;
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default:
    		aui_rtn(AUI_RTN_FAIL,"aui_deca_set fail!\n");
            break;
    }

    return rtn_code;
}

/**
*    @brief 		get deca count
*    @author		ray.gong
*    @date			2013-6-8
*    @param[out]	pul_deca_cnt    deca count
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_dev_cnt_get(unsigned long* const pul_deca_cnt)
{
	if(NULL==pul_deca_cnt)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	*pul_deca_cnt=1;//???? need add driver
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get current deca coding type
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		    aui deca device handle
*    @param[out]	pul_deca_cur_type 	deca coding type
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_type_get(aui_hdl p_hdl_deca,unsigned long* const pul_deca_cur_type)
{
    enum Snd_decoder_type decode_type;
    enum aui_deca_stream_type deca_type;

    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
	if (pul_deca_cur_type == NULL)
		aui_rtn(AUI_RTN_EINVAL, "deca NULL pul_deca_cur_type!\n");

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_get_decoder_type(g_snd_deca_handle,&decode_type))
    {
		aui_rtn(AUI_RTN_FAIL,"Deca get decoder type fail!\n");
    }

    switch(decode_type)
    {
       case SND_TYPE_MPEG1:
            deca_type = AUI_DECA_STREAM_TYPE_MPEG1;
            break;
       case SND_TYPE_MPEG2:
            deca_type = AUI_DECA_STREAM_TYPE_MPEG2;
            break;
       case SND_TYPE_MPEG_AAC:
            deca_type = AUI_DECA_STREAM_TYPE_AAC_LATM;
            break;
       case SND_TYPE_AC3:
            deca_type = AUI_DECA_STREAM_TYPE_AC3;
            break;
       case SND_TYPE_DTS:
            deca_type = AUI_DECA_STREAM_TYPE_DTS;
            break;
       case SND_TYPE_PPCM:
            deca_type = AUI_DECA_STREAM_TYPE_PPCM;
            break;
       case SND_TYPE_LPCM_V:
            deca_type = AUI_DECA_STREAM_TYPE_LPCM_V;
            break;
       case SND_TYPE_LPCM_A:
            deca_type = AUI_DECA_STREAM_TYPE_LPCM_A;
            break;
       case SND_TYPE_PCM:
            deca_type = AUI_DECA_STREAM_TYPE_PCM;
            break;
       case SND_TYPE_WMA:
            deca_type = AUI_DECA_STREAM_TYPE_BYE1;
            break;
       case SND_TYPE_RA8:
            deca_type = AUI_DECA_STREAM_TYPE_RA8;
            break;
       case SND_TYPE_MP3:
            deca_type = AUI_DECA_STREAM_TYPE_MP3;
            break;
       case SND_TYPE_INVALID:
            deca_type = AUI_DECA_STREAM_TYPE_INVALID;
            break;
       case SND_TYPE_MPEG_ADTS_AAC:
            deca_type = AUI_DECA_STREAM_TYPE_AAC_ADTS;
            break;
       case SND_TYPE_OGG:
            deca_type = AUI_DECA_STREAM_TYPE_OGG;
            break;
       case SND_TYPE_EC3:
            deca_type = AUI_DECA_STREAM_TYPE_EC3;
            break;
        default:
    		aui_rtn(AUI_RTN_FAIL,"Deca invalid type!\n");
            break;
    }

    *pul_deca_cur_type = deca_type;

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set current deca coding type
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		    aui deca device handle
*    @param[in]	    pul_deca_cur_type 	deca coding type
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_type_set(aui_hdl p_hdl_deca,unsigned long ul_deca_cur_type)
{
    enum Snd_decoder_type deca_type;
    //enum aui_audio_stream_type_em deca_type;

    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    enum snd_status deca_state = DECA_STATUS_DETACH;
    if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_GET_DECA_STATE, (unsigned int)&deca_state))
    {
		aui_rtn(AUI_RTN_FAIL,"deca ioctl SND_DECA_GET_DECA_STATE fail!\n");
    }
    //Don`t permit to set audio format while decoding
    if ((DECA_STATUS_PLAY == deca_state) ||(DECA_STATUS_PAUSE == deca_state)) {
        AUI_WARN("deca_status[%d] is a wrong status! need to stop deca before set deca format.\n", deca_state);
        if (alislsnd_stop(g_snd_deca_handle)) {
            aui_rtn(AUI_RTN_FAIL, "alislsnd_stop return fail!\n");
        }
    }
    switch(ul_deca_cur_type)
    {
       case AUI_DECA_STREAM_TYPE_MPEG1:
            deca_type = SND_TYPE_MPEG1;
            break;
       case AUI_DECA_STREAM_TYPE_MPEG2:
            deca_type = SND_TYPE_MPEG2;
            break;
       case AUI_DECA_STREAM_TYPE_AAC_LATM:
            deca_type = SND_TYPE_MPEG_AAC;
            break;
       case AUI_DECA_STREAM_TYPE_AC3:
            deca_type = SND_TYPE_AC3;
            break;
       case AUI_DECA_STREAM_TYPE_DTS:
            deca_type = SND_TYPE_DTS;
            break;
       case AUI_DECA_STREAM_TYPE_PPCM:
            deca_type = SND_TYPE_PPCM;
            break;
       case AUI_DECA_STREAM_TYPE_LPCM_V:
            deca_type = SND_TYPE_LPCM_V;
            break;
       case AUI_DECA_STREAM_TYPE_LPCM_A:
            deca_type = SND_TYPE_LPCM_A;
            break;
       case AUI_DECA_STREAM_TYPE_PCM:
            deca_type = SND_TYPE_PCM;
            break;
       case AUI_DECA_STREAM_TYPE_BYE1:
            deca_type = SND_TYPE_WMA;
            break;
       case AUI_DECA_STREAM_TYPE_RA8:
            deca_type = SND_TYPE_RA8;
            break;
       case AUI_DECA_STREAM_TYPE_MP3_L3:
       case AUI_DECA_STREAM_TYPE_MP3:
       case AUI_DECA_STREAM_TYPE_MP3_2:
            deca_type = SND_TYPE_MP3;
            break;
       case AUI_DECA_STREAM_TYPE_INVALID:
            deca_type = SND_TYPE_INVALID;
            break;
       case AUI_DECA_STREAM_TYPE_AAC_ADTS:
            deca_type = SND_TYPE_MPEG_ADTS_AAC;
            break;
       case AUI_DECA_STREAM_TYPE_OGG:
            deca_type = SND_TYPE_OGG;
            break;
       case AUI_DECA_STREAM_TYPE_EC3:
            deca_type = SND_TYPE_EC3;
            break;
        default:
    		aui_rtn(AUI_RTN_FAIL,"Deca invalid type!\n");
            break;
    }


    if(0 != alislsnd_set_decoder_type(g_snd_deca_handle,deca_type))
    {
		aui_rtn(AUI_RTN_FAIL,"Deca set decoder type fail!\n");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get deca sync mode
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		    aui deca device handle
*    @param[out]	pul_deca_sync_mode  current sync mode status
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_sync_mode_get(aui_hdl p_hdl_deca,unsigned long* const pul_deca_sync_mode)
{
    enum avsync_sync_mode sync_mod;

    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
	if (NULL == pul_deca_sync_mode)
		aui_rtn(AUI_RTN_EINVAL, "pul_deca_sync_mode is NULL.\n");
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislavsync_get_av_sync_mode(((aui_handle_deca *)p_hdl_deca)->avsync_handle,&sync_mod))
    {
		aui_rtn(AUI_RTN_FAIL,"Deca get sync mode fail!\n");
    }

    if(sync_mod == AVSYNC_PCR || sync_mod == AVSYNC_AUDIO)
    {
        *pul_deca_sync_mode = AUI_DECA_DEOCDER_SYNC;
    }
    else
    {
		*pul_deca_sync_mode = AUI_DECA_DEOCDER_ASYNC;
    }
    return AUI_RTN_SUCCESS;
}

/**
*    @brief 		set deca sync mode
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		    aui deca device handle
*    @param[in]     pul_deca_sync_mode  sync mode that want to be setted,refer to <aui_stc_avsync_mode>
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_sync_mode_set(aui_hdl p_hdl_deca, unsigned long ul_deca_sync_mode)
{
    if(p_hdl_deca == NULL) {
		aui_rtn(AUI_RTN_EINVAL, "Invalid handle!\n");
        return AUI_RTN_FAIL;
    }
    
    if ((g_snd_deca_handle != NULL) && (((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)) {
		aui_rtn(AUI_RTN_EINVAL, "Invalid handle!\n");
        return AUI_RTN_FAIL;
    }
    
    return aui_deca_set(p_hdl_deca, AUI_DECA_AUD_SYNC_MODE_SET, &ul_deca_sync_mode);
}

/**
*    @brief 		get deca status
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		    aui deca device handle
*    @param[out]	pul_deca_status 	point to deca status
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_status_get(aui_hdl p_hdl_deca,unsigned long* const pul_deca_status)
{
	AUI_RTN_CODE ret=AUI_RTN_FAIL;

	if(NULL == p_hdl_deca)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	if(NULL == pul_deca_status)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid arg!\n");
	}

	ret=aui_deca_get(p_hdl_deca,AUI_DECA_DEOCDER_STATUS_GET, pul_deca_status);
	if(AUI_RTN_SUCCESS!=ret)
	{
		return ret;
	}

	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		inject audio data to deca
*    @author		ray.gong
*    @date			2013-6-8
*    @param[in]		p_hdl_deca 		    aui deca device handle
*    @param[in]		puc_data 		    audio data address
*    @param[in]		ul_data_len 	    audio data length
*    @param[in]		st_ctl_blk 		    STC and PCR information used for synchronization
*    @param[out]	pul_real_wt_len 	point to the real length of injected data
*	 @return 		AUI_RTN_SUCCESS     success
*	 @return 		AUI_RTN_EINVAL      invalid parameters
*	 @return 		others  		    failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_inject_write_data(aui_hdl p_hdl_deca,const unsigned char *puc_data,unsigned long ul_data_len,struct aui_avsync_ctrl st_ctl_blk,unsigned long* const pul_real_wt_len)
{
    (void)p_hdl_deca;
    (void)puc_data;
    (void)ul_data_len;
    (void)st_ctl_blk;
    (void)pul_real_wt_len;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_bee_tone_start(aui_hdl p_hdl_deca, unsigned char level, int* data, unsigned int data_len)
{
    
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
	if (NULL == data)
		aui_rtn(AUI_RTN_EINVAL, "data is NULL.\n");
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    snd_audio_ioctl_tone_voice tone_data;
    memset(&tone_data, 0, sizeof(tone_data));

    tone_data.buffer_add = (unsigned int)data;
    tone_data.buffer_len = data_len;
    if(alislsnd_ioctl(g_snd_deca_handle, SND_DECA_GEN_TONE_VOICE, (unsigned int)&tone_data))
    {
        AUI_ERR("SND_DECA_GEN_TONE_VOICE fail\n");
        return AUI_RTN_FAIL;
    }
    (void)level;
    
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_bee_tone_stop(aui_hdl p_hdl_deca)
{
	if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    
    if(alislsnd_ioctl(g_snd_deca_handle, SND_DECA_STOP_TONE_VOICE, 0))
    {
        AUI_ERR("SND_DECA_STOP_TONE_VOICE fail\n");
        return AUI_RTN_FAIL;
    }
	return AUI_RTN_SUCCESS;
}

/**
*    @brief 		get deca data info
*    @author		amu.tu
*    @date			2015-4-3
*    @param[in]		p_hdl_deca 	    aui deca handle
*    @param[in]		pul_data_info 	deca data info to be get
*	 @return 		AUI_RTN_SUCCESS success
*	 @return 		AUI_RTN_EINVAL  invalid parameters
*	 @return 		others  		failed
*    @note
*
*/
AUI_RTN_CODE aui_deca_data_info_get(aui_hdl p_hdl_deca,aui_deca_data_info *pul_data_info)
{
	AUI_RTN_CODE aui_rtn=AUI_RTN_FAIL;

    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	if(NULL == pul_data_info)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid arg!\n");
	}
	aui_rtn=aui_deca_get(p_hdl_deca,AUI_DECA_DATA_INFO_GET, pul_data_info);
	if(AUI_RTN_SUCCESS!=aui_rtn)
	{
		return aui_rtn;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_init_ase(aui_hdl p_hdl_deca)
{
    if(p_hdl_deca == NULL)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(0 != alislsnd_init_ase(g_snd_deca_handle))
    {
         aui_rtn(AUI_RTN_FAIL,"alislsnd_init_ase fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_str_play(aui_hdl p_hdl_deca, unsigned int param)
{
    if(p_hdl_deca == NULL)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    /* The diver create a task to decode MPEG format beep data when call alislsnd_init_ase, 
     * and it has already judge ase_task_id. Although call alislsnd_init_ase for many times,
     * but in fact only create a task.
     */
    if(0 != alislsnd_init_ase(g_snd_deca_handle))
    {
        aui_rtn(AUI_RTN_FAIL,"alislsnd_init_ase fail!\n");
    }
    if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_STR_PLAY, param))
    {
        aui_rtn(AUI_RTN_FAIL,"alislsnd_ioctl SND_DECA_STR_PLAY fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_str_stop(aui_hdl p_hdl_deca)
{
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_STR_STOP, 0))
    {
		aui_rtn(AUI_RTN_FAIL,"alislsnd_ioctl SND_DECA_STR_STOP fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_set_beep_interval(aui_hdl p_hdl_deca, unsigned int param)
{
    if(p_hdl_deca == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(g_snd_deca_handle != NULL && ((aui_handle_deca *)p_hdl_deca)->aui_deca_snd_handle != g_snd_deca_handle)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }
    if(0 != alislsnd_ioctl(g_snd_deca_handle, SND_DECA_BEEP_INTERVAL, param))
    {
		aui_rtn(AUI_RTN_FAIL,"deca set beep interval fail!\n");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_audio_desc_enable (aui_hdl p_hdl_deca, unsigned char enable)
{   
    aui_handle_deca *deca = p_hdl_deca;

    if(p_hdl_deca == NULL) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

    if(0 != alislsnd_audio_description_enable(deca->aui_deca_snd_handle, enable)) {
        aui_rtn(AUI_RTN_FAIL,"set audio description output fail!\n");
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_get_sl_handle(aui_hdl pv_hdl_deca, alisl_handle * pv_hdl_sl_snd)
{
    aui_handle_deca *dev = pv_hdl_deca;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (NULL == dev || NULL == pv_hdl_sl_snd) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    
    *pv_hdl_sl_snd = dev->aui_deca_snd_handle;
    return ret;
}

AUI_RTN_CODE aui_deca_get_audio_id(aui_hdl pv_hdl_deca, unsigned char *audio_id)
{
    aui_handle_deca *dev = pv_hdl_deca;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (NULL == dev || NULL == audio_id) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    
    *audio_id = dev->attr_deca.uc_dev_idx;
    return ret;
}

