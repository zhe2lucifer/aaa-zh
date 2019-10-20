
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_snd.h>
#include <hld/snd/snd.h>
/****************************LOCAL MACRO******************************************/
AUI_MODULE(SND)

/****************************LOCAL TYPE*******************************************/
typedef struct aui_st_handle_snd
{
	aui_dev_priv_data dev_priv_data;
	OSAL_ID dev_mutex_id;	
	struct snd_device *pst_dev_snd;
	aui_attr_snd attr_snd;
}aui_handle_snd,*aui_p_handle_snd;
/****************************LOCAL VAR********************************************/
static OSAL_ID s_mod_mutex_id_snd=0;
/****************************LOCAL FUNC DECLEAR***********************************/



/****************************MODULE IMPLEMENT*************************************/

AUI_RTN_CODE aui_snd_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	*pul_version=AUI_MODULE_VERSION_NUM_SND;
	return SUCCESS;
}


AUI_RTN_CODE aui_snd_init(p_fun_cb p_call_back_init,void *pv_param)
{
    AUI_RTN_CODE rtn = AUI_RTN_FAIL;
	
    if ((NULL == p_call_back_init)) {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    s_mod_mutex_id_snd = osal_mutex_create();
    if (0 == s_mod_mutex_id_snd) {
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
	
    rtn = p_call_back_init(pv_param);
	
    osal_mutex_lock(s_mod_mutex_id_snd, OSAL_WAIT_FOREVER_TIME);
    
    /* snd open */
    unsigned char uc_dev_idx = 0;  /* Only one snd module is supported */
    struct snd_device *pst_dev_snd = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, uc_dev_idx);
    if (NULL == pst_dev_snd) {
        osal_mutex_unlock(s_mod_mutex_id_snd);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    if (RET_SUCCESS != snd_open(pst_dev_snd)) {
        osal_mutex_unlock(s_mod_mutex_id_snd);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }

    osal_mutex_unlock(s_mod_mutex_id_snd);
    return rtn;
}


AUI_RTN_CODE aui_snd_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    if ((NULL == p_call_back_init)) {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    
    osal_mutex_lock(s_mod_mutex_id_snd, OSAL_WAIT_FOREVER_TIME);
	
    /* close snd */
    unsigned char uc_dev_idx = 0;  /* Only one snd module is supported */
    struct snd_device *pst_dev_snd = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, uc_dev_idx);
    if (NULL == pst_dev_snd) {
        osal_mutex_unlock(s_mod_mutex_id_snd);
        aui_rtn(AUI_RTN_FAIL, NULL);
	}
    if (RET_SUCCESS != snd_close(pst_dev_snd)) {
        osal_mutex_unlock(s_mod_mutex_id_snd);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
	
    osal_mutex_unlock(s_mod_mutex_id_snd);
    
    if (E_OK != osal_mutex_delete(s_mod_mutex_id_snd)) {
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
	
    return p_call_back_init(pv_param);
}

#if 0
/**
*	 @brief 		get the snd module initialize status
*	 @author		ray.gong
*	 @date			  2013-5-20
*	 @param[out]	pul_module_init_status point to the module initialize status
*	  @return		  AUI_RTN_SUCCESS de_initialize the module successful
*	  @return		  EINVAL  de_initialize the module failed,because input parameters invalid
*	  @return		  others  de_initialize the module failed
*	 @note
*
*/
AUI_RTN_CODE aui_snd_init_status_get(unsigned long *pul_module_init_status)
{
	if(NULL==pul_module_init_status)
	{
		aui_rtn(AUI_MODULE_SYS,AUI_RTN_EINVAL,NULL);
	}
	*pul_module_init_status=s_snd_module_attr.ul_init_flag;
	return AUI_RTN_SUCCESS;
}
#endif

AUI_RTN_CODE aui_snd_open(const aui_attr_snd *p_attr_snd,aui_hdl* const pp_hdl_snd)
{

    /** device index*/
    //unsigned char uc_dev_idx = 0;
    OSAL_ID dev_mutex_id = INVALID_ID;
    aui_handle_snd *pst_hdl_snd=NULL;
    aui_snd_out_mode mode;
    memset(&mode, 0, sizeof(aui_snd_out_mode));
	
    if (p_attr_snd == NULL) {
        aui_rtn(AUI_RTN_EINVAL,"NULL attribute!\n");
    }
    if (p_attr_snd->uc_dev_idx != 0) {
        aui_rtn(AUI_RTN_EINVAL, "Only one snd module is supported!\n");
    }
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_snd,dev_mutex_id, AUI_MODULE_SND, AUI_RTN_FAIL);

    pst_hdl_snd=(aui_handle_snd *)MALLOC(sizeof(aui_handle_snd));
    if (NULL == pst_hdl_snd) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }
    MEMSET(pst_hdl_snd, 0, sizeof(aui_handle_snd));

    pst_hdl_snd->pst_dev_snd=(struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, pst_hdl_snd->attr_snd.uc_dev_idx);
    if (NULL == pst_hdl_snd->pst_dev_snd) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    
#if 0  /* have moved into aui_snd_init */ 
    if (RET_SUCCESS != snd_open(pst_hdl_snd->pst_dev_snd)) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
#endif

    MEMCPY(&(pst_hdl_snd->attr_snd), p_attr_snd, sizeof(aui_attr_snd));
    pst_hdl_snd->dev_mutex_id = dev_mutex_id;

    (pst_hdl_snd->attr_snd).snd_outI2so_attr.snd_out_type = AUI_SND_OUT_I2SO;
    (pst_hdl_snd->attr_snd).snd_outI2so_attr.uc_enabel = TRUE;
    (pst_hdl_snd->attr_snd).snd_out_spdif_attr.snd_out_type = AUI_SND_OUT_SPDIF;
    (pst_hdl_snd->attr_snd).snd_out_spdif_attr.uc_enabel = TRUE;
    (pst_hdl_snd->attr_snd).snd_out_hdmi_attr.snd_out_type = AUI_SND_OUT_HDMI;
    (pst_hdl_snd->attr_snd).snd_out_hdmi_attr.uc_enabel = TRUE;

    (pst_hdl_snd->dev_priv_data).dev_idx = p_attr_snd->uc_dev_idx;
    aui_dev_reg(AUI_MODULE_SND, pst_hdl_snd);
    *pp_hdl_snd = pst_hdl_snd;
    osal_mutex_unlock(dev_mutex_id);

    //set hdmi/spdif output pcm default,because snd driver hdmi/spdif output bypass default
    mode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
    mode.snd_out_type = AUI_SND_OUT_HDMI;
    aui_snd_out_data_type_set(pst_hdl_snd, mode);

    mode.snd_data_type = AUI_SND_OUT_MODE_DECODED;
    mode.snd_out_type = AUI_SND_OUT_SPDIF;
    aui_snd_out_data_type_set(pst_hdl_snd, mode);
	
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_close(aui_hdl p_hdl_snd)
{
	osal_mutex_lock(s_mod_mutex_id_snd, OSAL_WAIT_FOREVER_TIME);
	if ((NULL == p_hdl_snd)
		|| (NULL == ((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		|| (INVALID_ID == ((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	
#if 0  /* have moved into aui_snd_de_init */ 
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)//bug detective
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	if(RET_SUCCESS!=snd_close(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd))
	{
		aui_rtn(AUI_RTN_FAIL,NULL);
	}

	osal_mutex_lock(s_mod_mutex_id_snd, OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
#endif

	if (0 != osal_mutex_delete(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_FAIL, "\r\n_delete mutex err.");
	}
	aui_dev_unreg(AUI_MODULE_SND, p_hdl_snd);
	MEMSET(p_hdl_snd, 0, sizeof(aui_handle_snd));
	FREE(p_hdl_snd);
	p_hdl_snd = NULL;

	osal_mutex_unlock(s_mod_mutex_id_snd);

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_start(aui_hdl p_hdl_snd, __attribute__((unused))const aui_attr_snd *p_attr_snd)
{
	osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	snd_start(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd);
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_stop(void * p_hdl_snd, __attribute__((unused))const aui_attr_snd *p_attr_snd)
{
	osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	snd_stop(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd);
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_pause(aui_hdl p_hdl_snd, __attribute__((unused))const aui_attr_snd *p_attr_snd)
{
	osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	if(RET_SUCCESS!=snd_pause(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd))
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_FAIL,NULL);
	}
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_resume(aui_hdl p_hdl_snd, __attribute__((unused))const aui_attr_snd *p_attr_snd)
{
	osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	if(RET_SUCCESS!=snd_resume(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd))
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_FAIL,NULL);
	}
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_snd_set(aui_hdl p_hdl_snd,unsigned long ul_item,void *pv_param)
{
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

	osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

    if((AUI_SND_FADE_ENABLE!=ul_item)&&(AUI_SND_FADE_SPEED!=ul_item))
    {
        if((NULL==pv_param))
    	{
    		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
    		aui_rtn(AUI_RTN_EINVAL,NULL);
    	}
    }
    
	if(ul_item>=AUI_SND_SET_CMD_LAST)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	switch(ul_item)
	{
		case AUI_SND_VOL_SET:
		{
			if(RET_SUCCESS!=snd_set_volume(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SUB_OUT, (unsigned char)(*(unsigned char *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_MUTE_SET:
		{
			if(RET_SUCCESS!=snd_set_mute(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SUB_OUT, (unsigned char)(*(unsigned char *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			((aui_handle_snd *)p_hdl_snd)->attr_snd.uc_snd_mute_status=(*(unsigned char *)pv_param);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_CHANNEL_MODE_SET:
		{
			if(RET_SUCCESS!=snd_set_duplicate(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, (enum snd_dup_channel)(*(enum snd_dup_channel *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			((aui_handle_snd *)p_hdl_snd)->attr_snd.en_snd_channel_mode=(*(enum snd_dup_channel *)pv_param);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_OUT_INTERFACE_SET:
		{
			aui_snd_out_type_status snd_out_type_status_tmp=(*(aui_snd_out_type_status *)pv_param);
			UINT32 ul_enable_flag=0;
			UINT32 ul_interface_type=0xffffffff;
			AUI_DBG("input [%d][%d]\n", snd_out_type_status_tmp.snd_out_type, snd_out_type_status_tmp.uc_enabel);
			AUI_DBG("!!! [%d][%d][%d][%d]\n", AUI_SND_OUT_I2SO, AUI_SND_OUT_SPDIF, AUI_SND_OUT_HDMI, snd_out_type_status_tmp.snd_out_type);
#if 1
			if(AUI_SND_OUT_I2SO==snd_out_type_status_tmp.snd_out_type)
			{
				ul_interface_type=SND_I2S_OUT;
			}
			else if(AUI_SND_OUT_SPDIF==snd_out_type_status_tmp.snd_out_type)
			{
				ul_interface_type=SND_SPDIF_OUT;
			}
			else if(AUI_SND_OUT_HDMI==snd_out_type_status_tmp.snd_out_type)
			{
				ul_interface_type=SND_HDMI_OUT;
			}
			else
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}

			if(snd_out_type_status_tmp.uc_enabel)
			{
				ul_enable_flag=1;
			}
			else
			{
				ul_enable_flag=0;
			}
			AUI_DBG("output[%d][%d]\n", ul_interface_type, ul_enable_flag);
			if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, ul_interface_type, (unsigned long)ul_enable_flag))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}

			if(AUI_SND_OUT_I2SO==snd_out_type_status_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_out_type=AUI_SND_OUT_I2SO;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
			}
			else if(AUI_SND_OUT_SPDIF==snd_out_type_status_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_out_type=AUI_SND_OUT_SPDIF;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
			}
			else if(AUI_SND_OUT_HDMI==snd_out_type_status_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_out_type=AUI_SND_OUT_HDMI;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
			}
#else
			if(SND_OUT_I2SO==snd_out_type_status_tmp.snd_out_type)
			{
				if(snd_out_type_status_tmp.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffe;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000001;
				}
				if(((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr).uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffd;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000002;
				}
				if((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffb;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000004;
				}
			}
			else if(SND_OUT_SPDIF==snd_out_type_status_tmp.snd_out_type)
			{
				if((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffe;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000001;
				}
				if(snd_out_type_status_tmp.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffd;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000002;
				}
				if((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffb;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000004;
				}
			}
			else if(SND_OUT_HDMI==snd_out_type_status_tmp.snd_out_type)
			{
				if((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffe;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000001;

				}
				if((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffd;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000002;

				}
				if(snd_out_type_status_tmp.uc_enabel)
				{
					ul_enable_flag=ul_enable_flag&0xfffffffb;
				}
				else
				{
					ul_enable_flag=ul_enable_flag|0x00000004;

				}
			}
			else
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
			}
			aui_dbg_printf(AUI_MODULE_SND,1,"\r\nul_enable_flag=%08x",ul_enable_flag);
			if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_OUTPUT_CTRL, (unsigned long)ul_enable_flag))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
			}
			if(SND_OUT_I2SO==snd_out_type_status_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_out_type=SND_OUT_I2SO;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
			}
			else if(SND_OUT_SPDIF==snd_out_type_status_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_out_type=SND_OUT_SPDIF;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
			}
			else if(SND_OUT_HDMI==snd_out_type_status_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_out_type=SND_OUT_HDMI;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel=((aui_snd_out_type_status *)pv_param)->uc_enabel;
			}
#endif //<-- #if 1
			AUI_DBG("i2so:[%08x]:[%08x]\n",(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_out_type,
									(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel);
			AUI_DBG("spdif:[%08x]:[%08x]\n",(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_out_type,
									(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.uc_enabel);
			AUI_DBG("hdmi:[%08x]:[%08x]\n",(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_out_type,
									(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel);

			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_OUT_INTERFACE_MODE_SET:
		{
#if 1
			aui_snd_out_mode snd_out_mode_tmp=(*(aui_snd_out_mode *)pv_param);
			enum asnd_out_spdif_type edrv_out_mode;
			if((AUI_SND_OUT_I2SO == snd_out_mode_tmp.snd_out_type) && 
			    (AUI_SND_OUT_MODE_DECODED != snd_out_mode_tmp.snd_data_type))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			if(AUI_SND_OUT_MODE_ENCODED==snd_out_mode_tmp.snd_data_type)
			{
				edrv_out_mode=SND_OUT_SPDIF_BS;
			}
			else if(AUI_SND_OUT_MODE_DECODED==snd_out_mode_tmp.snd_data_type)
			{
				edrv_out_mode=SND_OUT_SPDIF_PCM;
			}
			else if(AUI_SND_OUT_MODE_FORCE_DD==snd_out_mode_tmp.snd_data_type)
			{
				edrv_out_mode=SND_OUT_SPDIF_FORCE_DD;
			}
			else
			{
				edrv_out_mode=SND_OUT_SPDIF_INVALID;
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
            if(SND_OUT_SPDIF_FORCE_DD==edrv_out_mode)
            {
            	AUI_PRINTF("set dd mode, output interface %d\n",snd_out_mode_tmp.snd_out_type);
                if(AUI_SND_OUT_SPDIF==snd_out_mode_tmp.snd_out_type)
    			{
    				if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_SPO_INTF_CFG, 1))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_RTN_FAIL,NULL);
        			}
    			}
                else if(AUI_SND_OUT_HDMI==snd_out_mode_tmp.snd_out_type)
                {
                    if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_DDP_SPO_INTF_CFG, 1))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_RTN_FAIL,NULL);
        			}                    
                }
            }
            else if(SND_OUT_SPDIF_BS==edrv_out_mode)
            {
            	AUI_PRINTF("set bs mode,output interface %d\n",snd_out_mode_tmp.snd_out_type);
                if(AUI_SND_OUT_SPDIF==snd_out_mode_tmp.snd_out_type)
    			{
    				if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_SPO_INTF_CFG, 1))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_RTN_FAIL,NULL);
        			}
    			}
                else if(AUI_SND_OUT_HDMI==snd_out_mode_tmp.snd_out_type)
                {
                    if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_DDP_SPO_INTF_CFG, 2))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_RTN_FAIL,NULL);
        			}                    
                }              
            }
			else if (SND_OUT_SPDIF_PCM==edrv_out_mode)
			{
				AUI_DBG("set pcm mode, output interface %d\n", snd_out_mode_tmp.snd_out_type);
				if(AUI_SND_OUT_SPDIF==snd_out_mode_tmp.snd_out_type)
    			{
    				if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_SPO_INTF_CFG, 0))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_RTN_FAIL,NULL);
        			}
    			}
                else if(AUI_SND_OUT_HDMI==snd_out_mode_tmp.snd_out_type)
                {
                    if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_DDP_SPO_INTF_CFG, 0))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_RTN_FAIL,NULL);
        			}                    
                }    
			}
			else
			{
				AUI_ERR("%s %d: invalid mode.\n", __FUNCTION__, __LINE__);
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
   				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			
			if(AUI_SND_OUT_I2SO == snd_out_mode_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_out_type = snd_out_mode_tmp.snd_out_type;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_data_type = snd_out_mode_tmp.snd_data_type;
			}
			else if(AUI_SND_OUT_SPDIF == snd_out_mode_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_out_type = snd_out_mode_tmp.snd_out_type;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_data_type = snd_out_mode_tmp.snd_data_type;
			}
			else if(AUI_SND_OUT_HDMI == snd_out_mode_tmp.snd_out_type)
			{
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_out_type = snd_out_mode_tmp.snd_out_type;
				(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_data_type = snd_out_mode_tmp.snd_data_type;
			}
#else
			aui_snd_out_mode snd_out_mode_tmp=(*(aui_snd_out_mode *)pv_param);
			enum asnd_out_spdif_type edrv_out_mode;
			if(AUI_SND_OUT_I2SO==snd_out_mode_tmp.snd_out_type && \
				AUI_SND_OUT_MODE_DECODED!=snd_out_mode_tmp.snd_data_type)
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
			}
			if(AUI_SND_OUT_MODE_ENCODED==snd_out_mode_tmp.snd_data_type)
			{
				edrv_out_mode=SND_OUT_SPDIF_BS;
			}
			else if(AUI_SND_OUT_MODE_DECODED==snd_out_mode_tmp.snd_data_type)
			{
				edrv_out_mode=SND_OUT_SPDIF_PCM;
			}
			else if(AUI_SND_OUT_MODE_FORCE_DD==snd_out_mode_tmp.snd_data_type)
			{
				edrv_out_mode=SND_OUT_SPDIF_FORCE_DD;
			}
			else
			{
				edrv_out_mode=SND_OUT_SPDIF_INVALID;
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
			}
            if(SND_OUT_SPDIF_FORCE_DD==edrv_out_mode)
            {
                if(AUI_SND_OUT_SPDIF==snd_out_mode_tmp.snd_out_type)
    			{
    				if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, FORCE_SPDIF_TYPE, (unsigned long)edrv_out_mode))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
        			}
    			}
                else if(AUI_SND_OUT_HDMI==snd_out_mode_tmp.snd_out_type)
                {
                    if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, FORCE_SPDIF_TYPE, (unsigned long)SND_OUT_SPDIF_BS))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
        			}
					////0: DDP_SPO_OUT from DDP_SPO_INTF(default),1: DDP_SPO_OUT from SPO_INTF 
                    if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SET_BS_OUTPUT_SRC, 1))
        			{
        				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
        				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
        			}
                    
                }
                    
            }
            else
            {
                if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, FORCE_SPDIF_TYPE, (unsigned long)edrv_out_mode))
    			{
    				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
    				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
    			}
                
                if(SND_OUT_SPDIF_BS==edrv_out_mode)
                {
                    if(AUI_SND_OUT_HDMI==snd_out_mode_tmp.snd_out_type)
                    {
                        if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SET_BS_OUTPUT_SRC, 0))
            			{
            				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
            				aui_rtn(AUI_MODULE_SND,AUI_RTN_FAIL,NULL);
            			}
                    }
                        
                }                
            }

			(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_out_type=snd_out_mode_tmp.snd_out_type;
			(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_out_type=snd_out_mode_tmp.snd_out_type;
			(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_out_type=snd_out_mode_tmp.snd_out_type;
#endif //<-- #if 1
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_SAMPLE_RATE_SET:
		{
			break;
		}
		case AUI_SND_HDMI_BYPASS_SET:
		{
			if(RET_SUCCESS!=snd_set_spdif_type(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, (enum asnd_out_spdif_type)(*(enum asnd_out_spdif_type *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_SPIDF_BYPASS_SET:
		{
			if(RET_SUCCESS!=snd_set_spdif_type(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, (enum asnd_out_spdif_type)(*(enum asnd_out_spdif_type *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_DAC_MUTE_SET:
		{
			if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_DAC_MUTE, (unsigned long)(*(unsigned long *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_SYNC_LEVEL_SET:
		{
			if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SET_SYNC_LEVEL, (unsigned long)(*(unsigned long *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			((aui_handle_snd *)p_hdl_snd)->attr_snd.ul_sync_level=(*(unsigned long *)pv_param);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_SYNC_DELAY_TIME:
		{
			if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SET_SYNC_DELAY, (unsigned long)(*(unsigned long *)pv_param)))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			((aui_handle_snd *)p_hdl_snd)->attr_snd.ul_sync_level=(*(unsigned long *)pv_param);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
        case AUI_SND_REG_CB:
        {
            struct aui_snd_io_reg_callback_para *p_cb_param=pv_param;
            struct snd_io_reg_callback_para hld_param;
            if(NULL==p_cb_param)
            {
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
            MEMSET(&hld_param,0,sizeof(hld_param));
            hld_param.e_cbtype=p_cb_param->e_cbtype;
            hld_param.p_cb=p_cb_param->p_cb;
            hld_param.threshold=p_cb_param->threshold;
            hld_param.pv_param=p_cb_param->pv_param;
            
            if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_REG_CALLBACK, (unsigned long)((unsigned long *)(&hld_param))))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
            rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_SPDIF_DELAY_ENABLE:
        {
            if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_MPEG_M8DB_ENABLE, (unsigned long)(*(unsigned long *)pv_param)))
            {
                osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
            }
            rtn_code=AUI_RTN_SUCCESS;
			break;
        }
        case AUI_SND_SPDIF_DELAY_TIME: {
            unsigned long delay_time_ms = (unsigned long)(*(unsigned long *)pv_param);
            if (AUI_SPDIF_DELAY_TIME_MAX < delay_time_ms) {
                aui_rtn(AUI_RTN_FAIL, "Snd SPDIF delay time setting is invalid! Must be within the scope of 0 to 250 ms.\n");
            }
            if (RET_SUCCESS != snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_ONLY_SET_SPDIF_DELAY_TIME, delay_time_ms)) {
                osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
		case AUI_SND_FADE_ENABLE:
        {
            if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_IO_SET_FADE_ENBALE, (unsigned long)pv_param))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
            rtn_code=AUI_RTN_SUCCESS;
			break;
        }
        case AUI_SND_FADE_SPEED:
        {
            if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SET_FADE_SPEED, (unsigned long)pv_param))
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
            rtn_code=AUI_RTN_SUCCESS;
			break;
        }
		default:
		{
			osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,NULL);
			break;
		}
	}

	if(AUI_RTN_SUCCESS!=rtn_code)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		return rtn_code;
	}
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_get(aui_hdl p_hdl_snd,unsigned long ul_item,void *pv_param)
{
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

	osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if((NULL==pv_param)||(ul_item>=AUI_SND_GET_CMD_LAST))
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	switch(ul_item)
	{
		case AUI_SND_VOL_GET:
		{
			*((unsigned char *)pv_param)= snd_get_volume(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_MUTE_GET:
		{
			*(unsigned char *)pv_param=((aui_handle_snd *)p_hdl_snd)->attr_snd.uc_snd_mute_status;
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_CHANNEL_MODE_GET:
		{
			*(enum snd_dup_channel *)pv_param=((aui_handle_snd *)p_hdl_snd)->attr_snd.en_snd_channel_mode;
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_OUT_INTERFACE_GET:
		{
			if(AUI_SND_OUT_I2SO==((aui_snd_out_type_status *)pv_param)->snd_out_type)
			{
				(*(aui_snd_out_type_status *)pv_param).uc_enabel=((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.uc_enabel);
			}
			else if(AUI_SND_OUT_SPDIF==((aui_snd_out_type_status *)pv_param)->snd_out_type)
			{
				(*(aui_snd_out_type_status *)pv_param).uc_enabel=((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.uc_enabel);
			}
			else if(AUI_SND_OUT_HDMI==((aui_snd_out_type_status *)pv_param)->snd_out_type)
			{
				(*(aui_snd_out_type_status *)pv_param).uc_enabel=((((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.uc_enabel);
			}
			else
			{
				osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL,NULL);
			}
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_SND_OUT_INTERFACE_MODE_GET:
		{
			if(AUI_SND_OUT_I2SO == (*(aui_snd_out_mode *)pv_param).snd_out_type)
			{
				(*(aui_snd_out_mode *)pv_param).snd_data_type = (((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_outI2so_attr.snd_data_type;
			}
			if(AUI_SND_OUT_SPDIF == (*(aui_snd_out_mode *)pv_param).snd_out_type)
			{
				(*(aui_snd_out_mode *)pv_param).snd_data_type=(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_spdif_attr.snd_data_type;
			}
			if(AUI_SND_OUT_HDMI == (*(aui_snd_out_mode *)pv_param).snd_out_type)
			{
				(*(aui_snd_out_mode *)pv_param).snd_data_type=(((aui_handle_snd *)p_hdl_snd)->attr_snd).snd_out_hdmi_attr.snd_data_type;
			}
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
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
			*(unsigned long *)pv_param=((aui_handle_snd *)p_hdl_snd)->attr_snd.ul_sync_level;
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		default:
		{
			break;
		}
	}
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return rtn_code;
}



AUI_RTN_CODE aui_snd_vol_set(aui_hdl p_hdl_snd,unsigned char uc_vol)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_VOL_SET, &uc_vol);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_vol_get(aui_hdl p_hdl_snd,unsigned char* const puc_vol)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_get(p_hdl_snd,AUI_SND_VOL_GET, puc_vol);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_mute_set(aui_hdl p_hdl_snd,unsigned char uc_enabel)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_MUTE_SET, &uc_enabel);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	aui_dbg_printf(AUI_MODULE_SND,1,"\r\n-> 		   [%s]    :[%d] \r\n",__FUNCTION__,uc_enabel);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_snd_mute_get(aui_hdl p_hdl_snd,unsigned char* const puc_enabel)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_get(p_hdl_snd,AUI_SND_MUTE_GET, puc_enabel);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	aui_dbg_printf(AUI_MODULE_SND,1,"\r\n-> 		   [%s]    :[%d] \r\n",__FUNCTION__,*puc_enabel);
	return AUI_RTN_SUCCESS;
}

/* this function is only used for nestor test.Fobidden to use it other destination. */
AUI_RTN_CODE aui_snd_underrun_times_get(aui_hdl p_hdl_snd,unsigned char* const underrun_times)
{
	if((p_hdl_snd == NULL) || (underrun_times == NULL)){
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
	}
	*((unsigned char *)underrun_times)=  snd_get_underrun_times(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd);
	AUI_DBG("\r\n-> 		   [%s]    :[%d] \r\n",__FUNCTION__,*underrun_times);
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_snd_channel_set(aui_hdl p_hdl_snd,enum aui_snd_channel_mode en_channel)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_CHANNEL_MODE_SET, &en_channel);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	AUI_DBG("\r\n->			[%s]	:[%d] \r\n", __FUNCTION__, en_channel);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_channel_get(aui_hdl p_hdl_snd,enum aui_snd_channel_mode *p_channel_mode)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_get(p_hdl_snd,AUI_SND_CHANNEL_MODE_GET, p_channel_mode);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_out_interface_type_set(aui_hdl p_hdl_snd,aui_snd_out_type_status snd_out_type)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	if(snd_out_type.snd_out_type>=AUI_SND_OUT_LAST)
	{
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	rtn=aui_snd_set(p_hdl_snd,AUI_SND_OUT_INTERFACE_SET, &snd_out_type);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	AUI_DBG("\r\n->			[%s]	:[%d][%d] \r\n", __FUNCTION__, snd_out_type.snd_out_type, snd_out_type.uc_enabel);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_out_interface_type_get(aui_hdl p_hdl_snd,aui_snd_out_type_status* const p_snd_out_type)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_get(p_hdl_snd,AUI_SND_OUT_INTERFACE_GET, p_snd_out_type);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_out_data_type_set(aui_hdl p_hdl_snd,aui_snd_out_mode snd_out_type)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_OUT_INTERFACE_MODE_SET, &snd_out_type);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	AUI_DBG("\r\n->			[%s]	:[%d][%d] \r\n", __FUNCTION__, snd_out_type.snd_out_type, snd_out_type.snd_data_type);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_out_data_type_get(aui_hdl p_hdl_snd,aui_snd_out_mode* const p_snd_out_type)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_get(p_hdl_snd,AUI_SND_OUT_INTERFACE_MODE_GET, p_snd_out_type);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_sync_level_set(aui_hdl p_hdl_snd,unsigned long ul_sync_level)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_SYNC_LEVEL_SET, &ul_sync_level);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_snd_sync_level_get(aui_hdl p_hdl_snd,unsigned long* const pul_sync_level)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_get(p_hdl_snd,AUI_SND_SYNC_LEVEL_GET, pul_sync_level);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_spdif_enable_delay(aui_hdl p_hdl_snd,BOOL flag)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_SPDIF_DELAY_ENABLE, &flag);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_snd_spdif_delay_time(aui_hdl p_hdl_snd,unsigned int time)
{
	AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    if(p_hdl_snd == NULL)
    {
		aui_rtn(AUI_RTN_EINVAL,"Invalid handle!\n");
    }

	rtn=aui_snd_set(p_hdl_snd,AUI_SND_SPDIF_DELAY_TIME, &time);
	if(AUI_RTN_SUCCESS!=rtn)
	{
		return rtn;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_desc_volume_offset_set (aui_hdl p_hdl_snd,int offset)
{
    osal_mutex_lock(s_mod_mutex_id_snd,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_snd)
		||(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
		||(INVALID_ID==((aui_handle_snd *)p_hdl_snd)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_snd);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	osal_mutex_lock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_snd);

	if(NULL==((aui_handle_snd *)p_hdl_snd)->pst_dev_snd)
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
    if(offset < -3)
    {
        offset = -3;
    }
    else if(offset > 3)
    {
        offset = 3;
    }
	if(RET_SUCCESS!=snd_io_control(((aui_handle_snd *)p_hdl_snd)->pst_dev_snd, SND_SET_DESC_VOLUME_OFFSET, (unsigned int)(offset)))
	{
		osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
		aui_rtn(AUI_RTN_FAIL,NULL);
	}
	osal_mutex_unlock(((aui_handle_snd *)p_hdl_snd)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_snd_mix_balance_set (aui_hdl handle, int balance)
{
    //not support right now
    (void)handle;
    (void)balance;
    return AUI_RTN_SUCCESS;
}
