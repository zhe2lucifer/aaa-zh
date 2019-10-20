
/****************************INCLUDE HEAD FILE************************************/
#include "aui_dsc_inner.h"
/****************************LOCAL VAR********************************************/
AUI_MODULE(DSC)

//#define AUI_KL_DEBUG   1
#define AUI_DSC_DEBUG_NEW 0


//static aui_dsc_module_attr s_dsc_module_attr;
static OSAL_ID s_mod_mutex_id_dsc = OSAL_INVALID_ID;
//static unsigned char s_clear_lpk[16]={0x15,0x34,0x57,0x79,0x91,0xab,0xcd,0xef,0x08,0x86,0x64,0x43,0x23,0xfe,0xdc,0xba};

#define R2R_PID 0x1234
#define HLD_SHA_MODE_CHECK(mode)	(((mode) != SHA_SHA_1) && \
		((mode) != SHA_SHA_224) && ((mode) != SHA_SHA_256) && \
		((mode) != SHA_SHA_384 )&& ((mode) != SHA_SHA_512))
/****************************LOCAL FUNC DECLEAR***********************************/
static void init_key_hdls(aui_handle_dsc *pst_hdl_dsc)
{
	int i=0;
	int j=0;

	for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
	{
		MEMSET(&(pst_hdl_dsc->key_hdls[i]),0,sizeof(aui_dsc_key_hdl));
		for(j=0;j<AUI_DSC_HDL_KEY_PID_CNT_MAX;j++)
		{
			pst_hdl_dsc->key_hdls[i].aul_key_param_handles[j]=0xff;
		}
	}
	return;
}


static AUI_RTN_CODE release_keys_hdls_by_grp_idx(aui_handle_dsc *pst_hdl_dsc,unsigned long ul_grp_idx)
{
	int i=0;
	AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;

	for(i=0;i<AUI_DSC_HDL_KEY_PID_CNT_MAX;i++)
	{
		if(NULL!=pst_hdl_dsc->p_dev_csa)
		{
			if(0xff!=(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]))
			{
				if(RET_SUCCESS!=csa_ioctl(pst_hdl_dsc->p_dev_csa, IO_DELETE_CRYPT_STREAM_CMD ,(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i])))
				{
					rtn_code=AUI_RTN_FAIL;
				}
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_key_used_flag=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_pid_cnt=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]=0xff;
			}
		}
		if(NULL!=pst_hdl_dsc->p_dev_aes)
		{
			if(0xff!=(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]))
			{
				if(RET_SUCCESS!=aes_ioctl(pst_hdl_dsc->p_dev_aes, IO_DELETE_CRYPT_STREAM_CMD ,(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i])))
				{
					rtn_code=AUI_RTN_FAIL;
				}
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_key_used_flag=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_pid_cnt=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]=0xff;
			}
		}
		if(NULL!=pst_hdl_dsc->p_dev_des)
		{
			if(0xff!=(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]))
			{
				if(RET_SUCCESS!=des_ioctl(pst_hdl_dsc->p_dev_des, IO_DELETE_CRYPT_STREAM_CMD ,(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i])))
				{
					rtn_code=AUI_RTN_FAIL;
				}
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_key_used_flag=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_pid_cnt=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]=0xff;
			}
		}
		if(NULL!=pst_hdl_dsc->p_dev_sha)
		{
			if(0xff!=(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]))
			{
				if(RET_SUCCESS!=sha_ioctl(pst_hdl_dsc->p_dev_sha, IO_DELETE_CRYPT_STREAM_CMD ,(pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i])))
				{
					rtn_code=AUI_RTN_FAIL;
				}
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_key_used_flag=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].ul_pid_cnt=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]=0;
				pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]=0xff;
			}
		}
	}


	return rtn_code;
}
static AUI_RTN_CODE search_pids_key_hdl(unsigned short us_pid,aui_handle_dsc *pst_hdl_dsc,unsigned long *pul_grp_idx,unsigned long *pul_key_hdl,unsigned long ul_new_flag)
{
	int i=0;
	int j=0;

	if(1==ul_new_flag)
	{
		for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
		{
			for(j=0;j<AUI_DSC_HDL_KEY_PID_CNT_MAX;j++)
			{
				//AUI_DBG("\r\n rd pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[i].aus_pids_key_hdls[j]);
				if(us_pid==pst_hdl_dsc->key_hdls[i].aus_pids_key_hdls[j])
				{
					AUI_DBG("\r\n rd return1 pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[i].aul_key_param_handles[j]);
					*pul_grp_idx=i;
					*pul_key_hdl=pst_hdl_dsc->key_hdls[i].aul_key_param_handles[j];
					return AUI_RTN_SUCCESS;
				}
			}
		}

		AUI_DBG("\r\n need open a new key hdl",i,j);
		for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
		{
			if(0==pst_hdl_dsc->key_hdls[i].ul_key_used_flag)
			{
				*pul_grp_idx=i;
				*pul_key_hdl=0xff;
				AUI_DBG("\r\n rd return2 pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,0xff);
				return AUI_RTN_SUCCESS;
			}
		}
		AUI_DBG("\r\n [%d][%d]all key hdl resource have be used.",i,j);
	}
	else if(0==ul_new_flag)
	{
		AUI_DBG("\r\n *pul_grp_idx=[%d][%d]",us_pid,*pul_grp_idx);

		for(j=0;j<AUI_DSC_HDL_KEY_PID_CNT_MAX;j++)
		{
			AUI_DBG("\r\n rd pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[*pul_grp_idx].aus_pids_key_hdls[j]);
			if(us_pid==pst_hdl_dsc->key_hdls[*pul_grp_idx].aus_pids_key_hdls[j])
			{
				AUI_DBG("\r\n rd return1 pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[*pul_grp_idx].aul_key_param_handles[j]);
				*pul_key_hdl=pst_hdl_dsc->key_hdls[*pul_grp_idx].aul_key_param_handles[j];
				return AUI_RTN_SUCCESS;
			}
		}
		if(AUI_DSC_HDL_KEY_PID_CNT_MAX==j)
		{
			for(j=0;j<AUI_DSC_HDL_KEY_PID_CNT_MAX;j++)
			{
				AUI_DBG("\r\n rd pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[*pul_grp_idx].aus_pids_key_hdls[j]);
				if(0==pst_hdl_dsc->key_hdls[*pul_grp_idx].aus_pids_key_hdls[j])
				{
					*pul_key_hdl=0xff;
					return AUI_RTN_SUCCESS;
				}
			}
			return AUI_RTN_FAIL;
		}


	}
	return AUI_RTN_FAIL;
}

static AUI_RTN_CODE set_pids_key_hdl(unsigned short us_pid,aui_handle_dsc *pst_hdl_dsc,unsigned long ul_key_hdl,unsigned long ul_grp_idx)
{

	int i=0;
	AUI_DBG("pid.dsc.keyhdl=[%08x][%08x][%08x][%08x]\n",us_pid,pst_hdl_dsc,ul_key_hdl,ul_grp_idx);

    AUI_DBG("Print the list:\n");
	for(i=0;i<AUI_DSC_HDL_KEY_PID_CNT_MAX;i++)
	{
		AUI_DBG("  i=[%d], pid=[%04x], hdl=[%08x]\n",i, pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i],pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]);
	}
	if(0==us_pid)
	{
        AUI_ERR("(0==us_pid)");
		return -1;
	}
	for(i=0;i<AUI_DSC_HDL_KEY_PID_CNT_MAX;i++)
	{
		AUI_DBG("Check i=[%d], pid:=[%04x], hdl=[%08x]", i, us_pid,pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]);
		if(us_pid==pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i])
		{
			AUI_DBG(" exist!!!  i = %d\n", i);

			return 0;
		}
		else if((us_pid!=pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i])&&(0==pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]))
		{
			AUI_DBG(" Not exist, add new, i = %d\n", i);
			pst_hdl_dsc->key_hdls[ul_grp_idx].ul_key_used_flag=1;
			pst_hdl_dsc->key_hdls[ul_grp_idx].aul_key_param_handles[i]=ul_key_hdl;
			pst_hdl_dsc->key_hdls[ul_grp_idx].aus_pids_key_hdls[i]=us_pid;

			return 0;
		}
	}
	AUI_ERR("\r\n set_pids_key_hdl rtn err ");
	return -1;
}


/* SHA Mode
Do the SHA Hash digest, data is in DRAM
*/
static RET_CODE ali_hw_sha_digest_dram(unsigned char *input, unsigned long input_len, \
						enum SHA_MODE sha_mode, unsigned char *output)
{
	unsigned long ShaDevID = ALI_INVALID_DSC_SUB_DEV_ID;
	p_sha_dev pShaDev = NULL;
	SHA_INIT_PARAM param;
	RET_CODE ret = RET_FAILURE;

	if ( (!input) || (! output) || (! input_len) || HLD_SHA_MODE_CHECK ( sha_mode ) )
	{
			return RET_FAILURE;
	}

	if (ALI_INVALID_DSC_SUB_DEV_ID == (ShaDevID = dsc_get_free_sub_device_id(SHA)))
	{
		return RET_FAILURE;
	}

	if (NULL == (pShaDev = (p_sha_dev)dev_get_by_id(HLD_DEV_TYPE_SHA, ShaDevID)))
	{
		dsc_set_sub_device_id_idle(SHA, ShaDevID);
		return RET_FAILURE;
	}

	memset(&param, 0, sizeof(SHA_INIT_PARAM));
	param.sha_data_source = SHA_DATA_SOURCE_FROM_DRAM;
	param.sha_work_mode = sha_mode;
	ret = sha_ioctl(pShaDev, IO_INIT_CMD, (unsigned long)&param);
	if(ret != RET_SUCCESS){
		dsc_set_sub_device_id_idle(SHA, ShaDevID);
		return ret;
	}
	ret = sha_digest(pShaDev, input, (unsigned char *)output, input_len);

	dsc_set_sub_device_id_idle(SHA, ShaDevID);
	return ret;
}

/* SHA Mode
Do the SHA Hash digest, data is in Flash
*/
static RET_CODE ali_hw_sha_digest_flash(unsigned char *input, unsigned long input_len, \
						enum SHA_MODE sha_mode, unsigned char *output)
{


	unsigned long ShaDevID = ALI_INVALID_DSC_SUB_DEV_ID;
	p_sha_dev pShaDev = NULL;
	SHA_INIT_PARAM param;
	RET_CODE ret = RET_FAILURE;

	if ( (!input) || (! output) || (! input_len) || HLD_SHA_MODE_CHECK ( sha_mode ) )
	{
			return RET_FAILURE;
	}

	if (ALI_INVALID_DSC_SUB_DEV_ID == (ShaDevID = dsc_get_free_sub_device_id(SHA)))
	{
		return RET_FAILURE;
	}

	if (NULL == (pShaDev = (p_sha_dev)dev_get_by_id(HLD_DEV_TYPE_SHA, ShaDevID)))
	{
		dsc_set_sub_device_id_idle(SHA, ShaDevID);
		return RET_FAILURE;
	}

	memset(&param, 0, sizeof(SHA_INIT_PARAM));
	param.sha_data_source = SHA_DATA_SOURCE_FROM_FLASH;
	param.sha_work_mode = sha_mode;
	ret = sha_ioctl(pShaDev, IO_INIT_CMD, (unsigned long)&param);
	if(ret != RET_SUCCESS){
		dsc_set_sub_device_id_idle(SHA, ShaDevID);
		return ret;
	}
	ret = sha_digest(pShaDev, input, (unsigned char *)output, input_len);

	dsc_set_sub_device_id_idle(SHA, ShaDevID);
	return ret;
}


/****************************MODULE IMPLEMENT*************************************/


AUI_RTN_CODE aui_dsc_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_DSC;
	return SUCCESS;
}

AUI_RTN_CODE aui_dsc_init(p_fun_cb p_call_back_init,void *pv_param)
{

	s_mod_mutex_id_dsc=osal_mutex_create();
	if(OSAL_INVALID_ID == s_mod_mutex_id_dsc)
	{
		aui_rtn(AUI_RTN_FAIL,"osal_mutex_create failed");
	}
	if((NULL!=p_call_back_init))
	{
		return p_call_back_init(pv_param);
	}
	return AUI_RTN_SUCCESS;

}


AUI_RTN_CODE aui_dsc_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if((NULL==p_call_back_init))
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	if(E_OK!=osal_mutex_delete(s_mod_mutex_id_dsc))
	{
		aui_rtn(AUI_RTN_FAIL,"osal_mutex_delete failed");
	}
	return p_call_back_init(pv_param);
}


AUI_RTN_CODE aui_dsc_open(const aui_attr_dsc *p_attr_dsc,aui_hdl* const pp_hdl_dsc)
{
	OSAL_ID dev_mutex_id = OSAL_INVALID_ID;
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
	aui_handle_dsc *pst_hdl_dsc=NULL;
	unsigned long ul_dsc_data_type=0XFFFFFFFF;

	AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_dsc,dev_mutex_id,AUI_MODULE_DSC,DSC_ERR);

	if((NULL==pp_hdl_dsc)||(NULL==p_attr_dsc))
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	pst_hdl_dsc=(aui_handle_dsc *)MALLOC(sizeof(aui_handle_dsc));
	if(NULL==pst_hdl_dsc)
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_ENOMEM,"Malloc failed");
	}
	if(p_attr_dsc->dsc_data_type>=AUI_DSC_DATA_TYPE_NB)
	{
		osal_mutex_unlock(dev_mutex_id);
		rtn_code= AUI_RTN_ERR_DATA_TYPE;
		goto ERR_END;
	}
	MEMSET(pst_hdl_dsc,0,sizeof(aui_handle_dsc));
	MEMCPY(&(pst_hdl_dsc->attr_dsc),p_attr_dsc,sizeof(aui_attr_dsc));
	//pst_hdl_dsc->ul_key_param_handle = 0xFF;//init handle
	//MEMSET(pst_hdl_dsc->aul_key_param_handles,0xff,AUI_DSC_HDL_KEY_GROUP_CNT_MAX);
	init_key_hdls(pst_hdl_dsc);
	pst_hdl_dsc->ul_stream_id=ALI_INVALID_CRYPTO_STREAM_ID;

	switch(p_attr_dsc->dsc_data_type)
	{
		case AUI_DSC_DATA_PURE:
		{
			ul_dsc_data_type=PURE_DATA_MODE;
			break;
		}
		case AUI_DSC_DATA_TS:
		{
			ul_dsc_data_type=TS_MODE;
			break;
		}
		default:
		{
			osal_mutex_unlock(dev_mutex_id);
			rtn_code= AUI_RTN_EINVAL;
			goto ERR_END;
			break;
		}
	}
	if (((pst_hdl_dsc->ul_stream_id) = dsc_get_free_stream_id(ul_dsc_data_type)) == ALI_INVALID_CRYPTO_STREAM_ID)
	{
		pst_hdl_dsc->ul_stream_id=ALI_INVALID_CRYPTO_STREAM_ID;
		osal_mutex_unlock(dev_mutex_id);
		rtn_code= AUI_RTN_NO_RESOURCE;
		goto ERR_END;
	}

	switch(p_attr_dsc->uc_algo)
	{
		case AUI_DSC_ALGO_AES:
		{
			pst_hdl_dsc->p_dev_dsc= (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
			if ((pst_hdl_dsc->ul_sub_aes_dev_id = dsc_get_free_sub_device_id(AES)) == INVALID_DSC_SUB_DEV_ID)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NO_RESOURCE;
				goto ERR_END;
			}
			pst_hdl_dsc->p_dev_aes = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, pst_hdl_dsc->ul_sub_aes_dev_id);
			if(NULL==pst_hdl_dsc->p_dev_aes)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NOT_INIT;
				goto ERR_END;
			}
			/*for block mode record*/
			pst_hdl_dsc->ul_sub_dev_id = pst_hdl_dsc->ul_sub_aes_dev_id;
			pst_hdl_dsc->dsc_sub_device = AES;
			AUI_DBG("pst_hdl_dsc->p_dev_aes:0x%p\n",pst_hdl_dsc->p_dev_aes);
			//aes_open(pst_hdl_dsc->p_dev_aes);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_DSC_ALGO_DES:
		case AUI_DSC_ALGO_TDES:
		{
			pst_hdl_dsc->p_dev_dsc= (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
			if ((pst_hdl_dsc->ul_sub_des_dev_id = dsc_get_free_sub_device_id(DES)) == INVALID_DSC_SUB_DEV_ID)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NO_RESOURCE;
				goto ERR_END;
			}
			pst_hdl_dsc->p_dev_des = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, pst_hdl_dsc->ul_sub_des_dev_id);
			if(NULL==pst_hdl_dsc->p_dev_des)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NOT_INIT;
				goto ERR_END;
			}
			pst_hdl_dsc->ul_sub_dev_id = pst_hdl_dsc->ul_sub_des_dev_id;
			pst_hdl_dsc->dsc_sub_device = TDES;
			AUI_DBG("pst_hdl_dsc->p_dev_des:0x%p\n",pst_hdl_dsc->p_dev_des);
			//des_open(pst_hdl_dsc->p_dev_des);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_DSC_ALGO_CSA:
		{
			pst_hdl_dsc->p_dev_dsc= (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
			if ((pst_hdl_dsc->ul_sub_csa_dev_id = dsc_get_free_sub_device_id(CSA)) == INVALID_DSC_SUB_DEV_ID)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NO_RESOURCE;
				goto ERR_END;
			}
			pst_hdl_dsc->p_dev_csa = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, pst_hdl_dsc->ul_sub_csa_dev_id);
			if(NULL==pst_hdl_dsc->p_dev_csa)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NOT_INIT;
				goto ERR_END;
			}

			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		case AUI_DSC_ALGO_SHA:
		{
			pst_hdl_dsc->p_dev_dsc= (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
			if ((pst_hdl_dsc->ul_sub_sha_dev_id = dsc_get_free_sub_device_id(SHA)) == INVALID_DSC_SUB_DEV_ID)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NO_RESOURCE;
				goto ERR_END;
			}
			pst_hdl_dsc->p_dev_sha = (p_sha_dev)dev_get_by_id(HLD_DEV_TYPE_SHA, pst_hdl_dsc->ul_sub_sha_dev_id);
			if(NULL==pst_hdl_dsc->p_dev_sha)
			{
				osal_mutex_unlock(dev_mutex_id);
				rtn_code=AUI_RTN_NOT_INIT;
				goto ERR_END;
			}
			//sha_open(pst_hdl_dsc->p_dev_sha);
			rtn_code=AUI_RTN_SUCCESS;
			break;
		}
		default:
		{
			osal_mutex_unlock(dev_mutex_id);
			rtn_code=AUI_RTN_NOT_INIT;
			goto ERR_END;
			break;
		}

	}
	pst_hdl_dsc->dev_mutex_id=dev_mutex_id;
	(pst_hdl_dsc->dev_priv_data).dev_idx = p_attr_dsc->uc_dev_idx;
	pst_hdl_dsc->dmx_src_mode_flag=0;
	aui_dev_reg(AUI_MODULE_DSC,pst_hdl_dsc);
	*pp_hdl_dsc=pst_hdl_dsc;
	AUI_DBG("\r\n *********************************open pp_hdl_dsc=[%08x][%08x]",pst_hdl_dsc,(pst_hdl_dsc->ul_stream_id));
	osal_mutex_unlock(dev_mutex_id);
    AUI_DBG("\r\n [%08x]dsc open[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]\r\n dsc puc_key:",*pp_hdl_dsc,
                                                        p_attr_dsc->uc_dev_idx,
                                                        p_attr_dsc->dsc_data_type,
                                                        p_attr_dsc->dsc_key_type,
                                                        p_attr_dsc->uc_algo,
                                                        p_attr_dsc->uc_mode,
                                                        p_attr_dsc->ul_key_len,
                                                        p_attr_dsc->ul_key_pattern,
                                                        p_attr_dsc->en_parity,
                                                        p_attr_dsc->en_residue,
                                                        p_attr_dsc->ul_key_pos,
                                                        p_attr_dsc->en_en_de_crypt,
                                                        p_attr_dsc->ul_pid_cnt);
	pst_hdl_dsc->attr_dsc.ul_key_pos = INVALID_ALI_CE_KEY_POS;
	return rtn_code;

ERR_END:
	if(NULL!=pst_hdl_dsc)
	{
		MEMSET(pst_hdl_dsc,0,sizeof(aui_handle_dsc));
		FREE(pst_hdl_dsc);
		pst_hdl_dsc=NULL;
	}
	return rtn_code;
}

extern INT32 dmx_main2see_buf_valid_size_set(UINT32 );
extern INT32 dmx_main2see_src_set(enum DMX_MAIN2SEE_SRC );

AUI_RTN_CODE aui_dsc_close(aui_hdl p_hdl_dsc)
{
	AUI_RTN_CODE rtn_code_last=AUI_RTN_SUCCESS;
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;
	int i=0;


	osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_dsc)
		||(NULL==pst_hdl_dsc->p_dev_dsc)
		||(INVALID_ID==pst_hdl_dsc->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	osal_mutex_lock(pst_hdl_dsc->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dsc);


	AUI_DBG("\r\n *******************close dsc [%08x]",p_hdl_dsc,pst_hdl_dsc->ul_stream_id );
	if(1==pst_hdl_dsc->dmx_src_mode_flag){			//for block mode test in nestor.
		 dmx_main2see_buf_valid_size_set(0);          //free all buffer in see
		 dmx_main2see_src_set(DMX_MAIN2SEE_SRC_NORMAL);    //return to normal mode
		 dmx_main2see_buf_valid_size_set(__MM_DMX_BLK_BUF_LEN);          //set the normal mode's BLK buffer length
		 pst_hdl_dsc->dmx_src_mode_flag=0;
	}
	if((NULL!=pst_hdl_dsc->p_dev_aes) && (!pst_hdl_dsc->pvr_crypto_mode))
	{
		for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
		{
			rtn_code=release_keys_hdls_by_grp_idx(pst_hdl_dsc,i);
			if(AUI_RTN_SUCCESS!=rtn_code)
			{
				rtn_code_last=rtn_code;
			}
		}
		if(ALI_INVALID_CRYPTO_STREAM_ID!=(pst_hdl_dsc->ul_stream_id))
		{
			if(RET_SUCCESS!=dsc_set_stream_id_idle(pst_hdl_dsc->ul_stream_id))
			{
				rtn_code_last=AUI_RTN_FAIL;
			}
		}
		if(RET_SUCCESS!=dsc_set_sub_device_id_idle(AES, pst_hdl_dsc->ul_sub_aes_dev_id))
		{
			rtn_code_last=AUI_RTN_FAIL;
		}
		//aes_close(pst_hdl_dsc->p_dev_aes);
		pst_hdl_dsc->p_dev_aes=NULL;
	}
	if((NULL!=pst_hdl_dsc->p_dev_des) && (!pst_hdl_dsc->pvr_crypto_mode))
	{
		for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
		{
			rtn_code=release_keys_hdls_by_grp_idx(pst_hdl_dsc,i);
			if(AUI_RTN_SUCCESS!=rtn_code)
			{
				rtn_code_last=rtn_code;
			}
		}

		if(ALI_INVALID_CRYPTO_STREAM_ID!=(pst_hdl_dsc->ul_stream_id))
		{
			if(RET_SUCCESS!=dsc_set_stream_id_idle(pst_hdl_dsc->ul_stream_id))
			{
				rtn_code_last=AUI_RTN_FAIL;
			}
		}
		if(RET_SUCCESS!=dsc_set_sub_device_id_idle(DES, pst_hdl_dsc->ul_sub_des_dev_id))
		{
			rtn_code_last=AUI_RTN_FAIL;
		}
		//des_close(pst_hdl_dsc->p_dev_des);
		pst_hdl_dsc->p_dev_des=NULL;
	}
	if((NULL!=pst_hdl_dsc->p_dev_csa) && (!pst_hdl_dsc->pvr_crypto_mode))
	{
		for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
		{
			rtn_code=release_keys_hdls_by_grp_idx(pst_hdl_dsc,i);
			if(AUI_RTN_SUCCESS!=rtn_code)
			{
				rtn_code_last=rtn_code;
			}
		}

		if(ALI_INVALID_CRYPTO_STREAM_ID!=(pst_hdl_dsc->ul_stream_id))
		{
			if(RET_SUCCESS!=dsc_set_stream_id_idle(pst_hdl_dsc->ul_stream_id))
			{
				rtn_code_last=AUI_RTN_FAIL;
			}
		}
		if(RET_SUCCESS!=dsc_set_sub_device_id_idle(CSA, pst_hdl_dsc->ul_sub_csa_dev_id))
		{
			rtn_code_last=AUI_RTN_FAIL;
		}
		//csa_close(pst_hdl_dsc->p_dev_csa);
		pst_hdl_dsc->p_dev_csa=NULL;
	}
	if((NULL!=pst_hdl_dsc->p_dev_sha) && (!pst_hdl_dsc->pvr_crypto_mode))
	{
		for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
		{
			rtn_code=release_keys_hdls_by_grp_idx(pst_hdl_dsc,i);
			if(AUI_RTN_SUCCESS!=rtn_code)
			{
				rtn_code_last=rtn_code;
			}
		}

		if(ALI_INVALID_CRYPTO_STREAM_ID!=(pst_hdl_dsc->ul_stream_id))
		{
			if(RET_SUCCESS!=dsc_set_stream_id_idle(pst_hdl_dsc->ul_stream_id))
			{
				rtn_code_last=AUI_RTN_FAIL;
			}
		}
		if(RET_SUCCESS!=dsc_set_sub_device_id_idle(SHA, pst_hdl_dsc->ul_sub_sha_dev_id))
		{
			rtn_code_last=AUI_RTN_FAIL;
		}
		//sha_close(pst_hdl_dsc->p_dev_sha);
		pst_hdl_dsc->p_dev_sha=NULL;
	}


	if(AUI_RTN_SUCCESS!=rtn_code_last)
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		return AUI_RTN_FAIL;
	}
	
	/*release resource in block mode*/
	if(pst_hdl_dsc->pvr_crypto_mode){
        aui_dsc_free_block_mode(pst_hdl_dsc);
				if(pst_hdl_dsc->dsc_sub_device == AES){	
						if(NULL!=pst_hdl_dsc->p_dev_aes){
								if(RET_SUCCESS!=dsc_set_sub_device_id_idle(AES, pst_hdl_dsc->ul_sub_aes_dev_id)){
										rtn_code_last=AUI_RTN_FAIL;
								}
						}
				}
				else if((pst_hdl_dsc->dsc_sub_device == TDES)|| (pst_hdl_dsc->dsc_sub_device == DES)){
						if(NULL!=pst_hdl_dsc->p_dev_des){
									if(RET_SUCCESS!=dsc_set_sub_device_id_idle(DES, pst_hdl_dsc->ul_sub_des_dev_id)){
											rtn_code_last=AUI_RTN_FAIL;
									}
						}		
				}
				else{
						AUI_ERR("hld dsc device type error!\n");
						rtn_code=AUI_RTN_FAIL;
				}
				
				
		#if 0
		if(pst_hdl_dsc->dsc_sub_device == AES){
			release_dev = (void *)dev_get_by_id(HLD_DEV_TYPE_AES, pst_hdl_dsc->ul_sub_dev_id);
			if(NULL == release_dev){
				AUI_DSC_DEBUG("get hld dsc device handle err!\n");
	            rtn_code_last = AUI_RTN_FAIL;
	        }
			if(RET_SUCCESS!=aes_ioctl(release_dev, IO_DELETE_CRYPT_STREAM_CMD ,pst_hdl_dsc->key_handle)){
				rtn_code=AUI_RTN_FAIL;
			}
		}else if((pst_hdl_dsc->dsc_sub_device == TDES)|| (pst_hdl_dsc->dsc_sub_device == DES)){
			release_dev = (void *)dev_get_by_id(HLD_DEV_TYPE_DES, pst_hdl_dsc->ul_sub_dev_id);
			if(NULL == release_dev){
				AUI_DSC_DEBUG("get hld dsc device handle err!\n");
	            rtn_code_last = AUI_RTN_FAIL;
	        }
			if(RET_SUCCESS!=des_ioctl(release_dev, IO_DELETE_CRYPT_STREAM_CMD ,pst_hdl_dsc->key_handle)){
				rtn_code=AUI_RTN_FAIL;
			}
		}else{
			AUI_DSC_DEBUG("hld dsc device type error!\n");
			rtn_code=AUI_RTN_FAIL;
		}
		
		
		if(ALI_INVALID_CRYPTO_STREAM_ID!=(pst_hdl_dsc->ul_stream_id))
		{
			if(RET_SUCCESS!=dsc_set_stream_id_idle(pst_hdl_dsc->ul_stream_id))
			{
				rtn_code_last=AUI_RTN_FAIL;
			}
		}

		if(RET_SUCCESS!=dsc_set_sub_device_id_idle(pst_hdl_dsc->dsc_sub_device, pst_hdl_dsc->ul_sub_dev_id))
		{
			rtn_code_last=AUI_RTN_FAIL;
		}
		/*release key pos*/
		if(pst_hdl_dsc->attr_dsc.ul_key_pos != INVALID_ALI_CE_KEY_POS){
            if(RET_SUCCESS != ce_ioctl(hld_ce_dev, IO_CRYPT_POS_SET_IDLE, pst_hdl_dsc->attr_dsc.ul_key_pos+1))
                rtn_code_last=AUI_RTN_FAIL;
            if(RET_SUCCESS != ce_ioctl(hld_ce_dev, IO_CRYPT_POS_SET_IDLE, pst_hdl_dsc->attr_dsc.ul_key_pos))
                rtn_code_last=AUI_RTN_FAIL;
		}
		#endif
	}
	
	osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);

	if(0!=osal_mutex_delete(pst_hdl_dsc->dev_mutex_id))
	{
		aui_rtn(AUI_RTN_FAIL,"\r\n_delete mutex err.");
	}
	aui_dev_unreg(AUI_MODULE_DSC,p_hdl_dsc);
	MEMSET(p_hdl_dsc,0,sizeof(aui_handle_dsc));
	FREE(p_hdl_dsc);
	p_hdl_dsc=NULL;

	return AUI_RTN_SUCCESS;
}



/* using retail key -> KEY_FROM_REG to set AES_CBC key for TS data scramble/de_scramble
pAES_DEV (pst_hdl_dsc->p_dev_aes) -> AES device
unsigned char *puc_key -> oddkey is in the lower memory addr, evenkey in higher memory addr
unsigned char *puc_iv -> when KEY_FROM_REG/OTP, no both even/odd iv supported by HW, there is only
				one iv register in HW
en_parity -> EVEN_PARITY_MODE ODD_PARITY_MODE AUTO_PARITY_MODE0
en_residue -> residue block handling method when alogorighm works in CBC mode
ul_key_len -> in bits //128,192,256
*/

AUI_RTN_CODE aui_aes_attach_key(aui_hdl p_hdl_dsc,aui_dsc_data_type dsc_data_type,aui_dsc_key_type dsc_key_type,enum WORK_MODE uc_mode, unsigned char *puc_key, unsigned long ul_key_len, const unsigned char *puc_iv_ctr,
	enum PARITY_MODE en_parity, enum RESIDUE_BLOCK en_residue,unsigned short *pus_pid,unsigned long ul_pid_cnt,unsigned long ul_key_pos, unsigned long ul_key_pattern)
{
	struct aes_init_param st_aes_param;
	KEY_PARAM key_param;
	AES_KEY_PARAM key_info;
	RET_CODE ret = RET_FAILURE;
	unsigned char *puc_iv_tmp=NULL;
	unsigned char *puc_ctr_tmp=NULL;
	enum RESIDUE_BLOCK en_residue_tmp=0xffffffff;
	//unsigned long ul_stream_id_tmp=ALI_INVALID_CRYPTO_STREAM_ID;
	unsigned short *pus_pid_list_tmp=pus_pid;
	//unsigned long ul_pid_cnt=0;
	unsigned long ul_key_pos_tmp=0;
	unsigned long ul_key_mode_tmp=0;
	unsigned long ul_key_length_tmp=0;
	AES_KEY_PARAM *p_key_info_tmp=NULL;
	enum KEY_TYPE en_key_tpye_tmp=0xffffffff;
	enum PARITY_MODE en_parity_tmp=0xffffffff;
	enum DMA_MODE en_data_type_tmp=0xffffffff;
	//unsigned long ul_stream_id=ALI_INVALID_CRYPTO_STREAM_ID;
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;
	unsigned short aus_pid[1] = {0x1234};
	int i=0;
//	  int j=0;
	unsigned long ul_key_hdl_tmp=0xff;
	unsigned long ul_key_grp_idx=0;
	AES_IV_INFO st_aes_iv;


	if((!pst_hdl_dsc)||(!(pst_hdl_dsc->p_dev_aes)))
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	if(dsc_data_type>=AUI_DSC_DATA_TYPE_NB)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	//unsigned short aus_pid[1] = {us_pid&0x1fff};
	MEMSET(&st_aes_iv,0,sizeof(AES_IV_INFO));

	switch(uc_mode)
	{
		case WORK_MODE_IS_ECB:
		{
			en_residue_tmp=RESIDUE_BLOCK_IS_NO_HANDLE;
			break;
		}
		case WORK_MODE_IS_CBC:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			if((RESIDUE_BLOCK_IS_NO_HANDLE != en_residue)
				&&(RESIDUE_BLOCK_IS_AS_ATSC != en_residue)
				&&(RESIDUE_BLOCK_IS_HW_CTS != en_residue)
				&&(RESIDUE_BLOCK_IS_RESERVED != en_residue))
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=en_residue;
			puc_iv_tmp=(unsigned char*)puc_iv_ctr;
			puc_ctr_tmp=NULL;
			break;
		}
		case WORK_MODE_IS_CTR:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=RESIDUE_BLOCK_IS_RESERVED;
			puc_iv_tmp=NULL;
			puc_ctr_tmp=(unsigned char*)puc_iv_ctr;
			break;
		}
		case WORK_MODE_IS_CFB:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=RESIDUE_BLOCK_IS_NO_HANDLE;
			puc_iv_tmp=(unsigned char*)puc_iv_ctr;
			puc_ctr_tmp=NULL;
			break;
		}
		case WORK_MODE_IS_OFB:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=RESIDUE_BLOCK_IS_NO_HANDLE;
			puc_iv_tmp=(unsigned char*)puc_iv_ctr;
			puc_ctr_tmp=NULL;
			break;
		}
		default:
		{
			aui_rtn(AUI_RTN_FAIL,"Invalid parameter");
			break;
		}
	}

	if(AUI_DSC_HOST_KEY_REG==dsc_key_type)
	{
		if(en_parity > (enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;
		}
		else
		{
			en_parity_tmp=EVEN_PARITY_MODE;
		}

		if(!puc_key)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		en_key_tpye_tmp=KEY_FROM_REG;
		ul_key_pos_tmp=0;
		if((ul_key_len != 128) &&  (ul_key_len != 192) && (ul_key_len != 256))
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		ul_key_mode_tmp=ul_key_len/64 - 1;
		ul_key_length_tmp=ul_key_len;
		if(128==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.aes_128bit_key.odd_key, puc_key, 16);
				MEMCPY(key_info.aes_128bit_key.even_key, puc_key+16, 16);
			}
			else
			{
				MEMCPY(key_info.aes_128bit_key.odd_key, puc_key, 16);
				MEMCPY(key_info.aes_128bit_key.even_key, puc_key, 16);
			}
		}
		else if(192==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.aes_192bit_key.odd_key, puc_key, 24);
				MEMCPY(key_info.aes_192bit_key.even_key, puc_key+24, 24);
			}
			else
			{
				MEMCPY(key_info.aes_192bit_key.odd_key, puc_key, 24);
				MEMCPY(key_info.aes_192bit_key.even_key, puc_key, 24);
			}
		}
		else //if(256==ul_key_len)//bug detective
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.aes_256bit_key.odd_key, puc_key, 32);
				MEMCPY(key_info.aes_256bit_key.even_key, puc_key+32, 32);
			}
			else
			{
				MEMCPY(key_info.aes_256bit_key.odd_key, puc_key, 32);
				MEMCPY(key_info.aes_256bit_key.even_key, puc_key, 32);				
			}
		}
		/*else
		{
			aui_rtn(AUI_RTN_FAIL,NULL);
		}*/
		p_key_info_tmp=&key_info;
	}
	else if(AUI_DSC_HOST_KEY_SRAM==dsc_key_type)
	{
		if(en_parity > (enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;

		}
		else
		{
			en_parity_tmp=EVEN_PARITY_MODE;

		}

		if(!puc_key)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		en_key_tpye_tmp=KEY_FROM_SRAM;

		ul_key_pos_tmp=0;
		if((ul_key_len != 128) &&  (ul_key_len != 192) && (ul_key_len != 256))
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		ul_key_mode_tmp=ul_key_len/64 - 1;
		ul_key_length_tmp=ul_key_len;
		if(128==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.aes_128bit_key.odd_key, puc_key, 16);
				MEMCPY(key_info.aes_128bit_key.even_key, puc_key+16, 16);
			}
			else
			{
				MEMCPY(key_info.aes_128bit_key.odd_key, puc_key, 16);
				MEMCPY(key_info.aes_128bit_key.even_key, puc_key, 16);
			}
		}
		else if(192==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.aes_192bit_key.odd_key, puc_key, 24);
				MEMCPY(key_info.aes_192bit_key.even_key, puc_key+24, 24);
			}
			else
			{
				MEMCPY(key_info.aes_192bit_key.odd_key, puc_key, 24);
				MEMCPY(key_info.aes_192bit_key.even_key, puc_key, 24);
			}
		}
		else //if(256==ul_key_len)//bug detective
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.aes_256bit_key.odd_key, puc_key, 32);
				MEMCPY(key_info.aes_256bit_key.even_key, puc_key+32, 32);
			}
			else
			{
				MEMCPY(key_info.aes_256bit_key.odd_key, puc_key, 32);
				MEMCPY(key_info.aes_256bit_key.even_key, puc_key, 32);				
			}
		}
		/*else//bug detective
		{
			aui_rtn(AUI_RTN_FAIL,NULL);
		}*/
		p_key_info_tmp=&key_info;
	}
	else if(AUI_DSC_CONTENT_KEY_OTP==dsc_key_type)
	{
		if(((enum PARITY_MODE)OTP_KEY_FROM_68!=en_parity)&&((enum PARITY_MODE)OTP_KEY_FROM_6C!=en_parity))
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		en_parity_tmp=en_parity;
		en_key_tpye_tmp=KEY_FROM_OTP;

		ul_key_pos_tmp=0;
		ul_key_mode_tmp=AES_128BITS_MODE;
		ul_key_length_tmp=128;
		p_key_info_tmp=NULL;
	}
	else if(AUI_DSC_CONTENT_KEY_KL==dsc_key_type)
	{
		if(en_parity>(enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;

		}
		else
		{
			en_parity_tmp=EVEN_PARITY_MODE;

		}
		en_key_tpye_tmp=KEY_FROM_CRYPTO;

		ul_key_pos_tmp=ul_key_pos;
		ul_key_mode_tmp=AES_128BITS_MODE;
		ul_key_length_tmp=128;
		p_key_info_tmp=NULL;
	}
	else
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid dsc_key_type");
	}

	if(AUI_DSC_DATA_TS==dsc_data_type)
	{
		en_data_type_tmp=TS_MODE;
	}
	else
	{
		en_data_type_tmp=PURE_DATA_MODE;
		pus_pid_list_tmp=aus_pid;
		ul_pid_cnt=1;
	}
	MEMSET( &st_aes_param, 0, sizeof ( struct aes_init_param ) );
	st_aes_param.dma_mode = en_data_type_tmp;
	st_aes_param.key_from = en_key_tpye_tmp;
	st_aes_param.key_mode = ul_key_mode_tmp ;
	st_aes_param.parity_mode = en_parity_tmp;
	st_aes_param.residue_mode = en_residue_tmp;
	st_aes_param.stream_id = pst_hdl_dsc->ul_stream_id;
	st_aes_param.work_mode = uc_mode ;
	ret = aes_ioctl ( (pst_hdl_dsc->p_dev_aes) , IO_INIT_CMD , ( unsigned long ) &st_aes_param );
	if(ret != RET_SUCCESS)
	{
		aui_rtn(AUI_RTN_FAIL,"IO_INIT_CMD failed");
	}

	if(!pst_hdl_dsc->pvr_crypto_mode){
		for(i=0;i<(int)ul_pid_cnt;i++)
		{
			MEMSET(&key_param, 0, sizeof(KEY_PARAM));
			if(AUI_DSC_DATA_TS==dsc_data_type)
			{
				pus_pid_list_tmp=pus_pid+i;
			}
			else if(AUI_DSC_DATA_PURE==dsc_data_type)
			{
				pus_pid_list_tmp=aus_pid;
			}
			if(0==i)
			{
				if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,1))
				{
					aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
				}
			}
			else
			{
				if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,0))
				{
					aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
				}
			}
			key_param.handle = ul_key_hdl_tmp;//pst_hdl_dsc->ul_key_param_handle;
			key_param.ctr_counter = puc_ctr_tmp;
			key_param.init_vector = puc_iv_tmp;//KEY_FROM_REG/KEY_FROM_OTP using this ptr
			key_param.key_length = ul_key_length_tmp;
			key_param.p_aes_key_info = p_key_info_tmp;

			key_param.p_aes_iv_info= &st_aes_iv;
			if(( AUI_DSC_KEY_PATTERN_SINGLE == ul_key_pattern)||
					( AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)){
				key_param.no_even = 0 ;
				key_param.no_odd = 1 ;
			}
			else if(AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern){
				key_param.no_even = 1 ;
				key_param.no_odd = 0 ;
			}
			else if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern){
				key_param.no_even = 0 ;
				key_param.no_odd = 0 ;
			}
			
			if (NULL != puc_iv_tmp) {
				if(( AUI_DSC_KEY_PATTERN_SINGLE == ul_key_pattern)||
					( AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)){
					MEMCPY(key_param.p_aes_iv_info->even_iv,puc_iv_tmp,16);
				}
				else if(AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern){
					MEMCPY(key_param.p_aes_iv_info->odd_iv,puc_iv_tmp,16);
				}
				else if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern){
					MEMCPY(key_param.p_aes_iv_info->odd_iv,puc_iv_tmp,16);
					MEMCPY(key_param.p_aes_iv_info->even_iv,puc_iv_tmp+16,16);
				}
			}
			key_param.pid_list = pus_pid_list_tmp;
			key_param.pid_len = 1;
			key_param.stream_id = pst_hdl_dsc->ul_stream_id;
			key_param.pos = ul_key_pos_tmp & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																		Bit[7:0]  --->key pos*/
			key_param.kl_sel = (ul_key_pos_tmp>>8) & 0xFF;
			pst_hdl_dsc->attr_dsc.ul_key_pos=key_param.pos;
			//key_param.stream_id = ALI_INVALID_CRYPTO_STREAM_ID;//no needed when key from reg
			if(0xFF == key_param.handle)
			{
				ret = aes_ioctl ((pst_hdl_dsc->p_dev_aes) ,IO_CREAT_CRYPT_STREAM_CMD , (unsigned long)&key_param);
			}
			else
			{
				ret = aes_ioctl ((pst_hdl_dsc->p_dev_aes) ,IO_KEY_INFO_UPDATE_CMD , (unsigned long)&key_param);
			}
			//pst_hdl_dsc->ul_key_param_handle=key_param.handle;
			if(ret != RET_SUCCESS)
			{
				aui_rtn(AUI_RTN_FAIL,"Create or update stream failed");
			}
			if(0!=set_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,key_param.handle,ul_key_grp_idx))
			{
				aui_rtn(AUI_RTN_FAIL,"\r\n set_pids_key_hdl fail");
			}
		}
	}else{/*pvr decrypt/encrypt TS stream*/
		MEMSET(&key_param, 0, sizeof(KEY_PARAM));
		key_param.handle = ul_key_hdl_tmp;//pst_hdl_dsc->ul_key_param_handle;
		key_param.ctr_counter = puc_ctr_tmp;
		key_param.init_vector = puc_iv_tmp;//KEY_FROM_REG/KEY_FROM_OTP using this ptr
		key_param.key_length = ul_key_length_tmp;
		key_param.p_aes_key_info = p_key_info_tmp;

		key_param.pid_list = pus_pid_list_tmp;
		key_param.pid_len = ul_pid_cnt;
		key_param.stream_id = pst_hdl_dsc->ul_stream_id;
		key_param.pos = ul_key_pos_tmp & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
														Bit[7:0]  --->key pos*/
		key_param.kl_sel = (ul_key_pos_tmp>>8) & 0xFF;
		pst_hdl_dsc->attr_dsc.ul_key_pos=key_param.pos;
		//key_param.stream_id = ALI_INVALID_CRYPTO_STREAM_ID;//no needed when key from reg
		ret = aes_ioctl ((pst_hdl_dsc->p_dev_aes) ,IO_CREAT_CRYPT_STREAM_CMD , (unsigned long)&key_param);
		//pst_hdl_dsc->ul_key_param_handle=pst_hdl_dsc->key_param.handle;
		unsigned int i;
		for(i = 0; i < ul_pid_cnt; i++)
			AUI_DBG("pus_pid[%d]: %d,", i, pus_pid_list_tmp[i]);
		AUI_DBG("pst_hdl_dsc->key_param.handle: 0x%08x\n",pst_hdl_dsc->key_handle);
		if(ret != RET_SUCCESS)
		{
			aui_rtn(AUI_RTN_FAIL,NULL);
		}
		pst_hdl_dsc->key_handle = key_param.handle;
		/*
		if(0!=set_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,pst_hdl_dsc->key_param.handle,ul_key_grp_idx))
		{
			aui_dbg_printf(AUI_MODULE_DSC,1,"\r\n set_pids_key_hdl fail");
			aui_rtn(AUI_RTN_FAIL,NULL);
		}*/
	}
	return AUI_RTN_SUCCESS;
}



/* using retail key -> KEY_FROM_REG to set DES_CBC key for TS data scramble/de_scramble
pDES_DEV (pst_hdl_dsc->p_dev_des) -> DES device
unsigned char *puc_key -> oddkey is in the lower memory addr, evenkey in higher memory addr
unsigned char *puc_iv -> when KEY_FROM_REG/OTP, no both even/odd iv supported by HW, there is only
				one iv register in HW
en_parity -> EVEN_PARITY_MODE ODD_PARITY_MODE AUTO_PARITY_MODE0
en_residue -> residue block handling method when alogorighm works in CBC mode
ul_key_len -> in bits //128,192,256
*/

AUI_RTN_CODE aui_des_attach_key(aui_hdl p_hdl_dsc,aui_dsc_data_type dsc_data_type,aui_dsc_key_type dsc_key_type,enum WORK_MODE uc_mode, unsigned char *puc_key,unsigned long ul_key_len, const unsigned char *puc_iv_ctr,
	enum PARITY_MODE en_parity, enum RESIDUE_BLOCK en_residue,unsigned short *pus_pid,unsigned long ul_pid_cnt,unsigned long ul_key_pos, unsigned long ul_key_pattern)
{
	struct des_init_param st_des_param;
	KEY_PARAM key_param;
	DES_KEY_PARAM key_info;
	RET_CODE ret = RET_FAILURE;
	unsigned char *puc_iv_tmp=NULL;
	unsigned char *puc_ctr_tmp=NULL;
	enum RESIDUE_BLOCK en_residue_tmp=0xffffffff;
	//unsigned long ul_stream_id_tmp=ALI_INVALID_CRYPTO_STREAM_ID;
	unsigned short *pus_pid_list_tmp=pus_pid;
	//unsigned long ul_pid_cnt=0;
	unsigned long ul_key_pos_tmp=0;
	unsigned long ul_key_mode_tmp=0;
	unsigned long ul_key_length_tmp=0;
	DES_KEY_PARAM *p_key_info_tmp=NULL;
	enum KEY_TYPE en_key_tpye_tmp=0xffffffff;
	enum WORK_SUB_MODULE en_sub_algo=0xffffffff;
	enum PARITY_MODE en_parity_tmp=0xffffffff;
	enum DMA_MODE en_data_type_tmp=0xffffffff;
	//unsigned long ul_stream_id=ALI_INVALID_CRYPTO_STREAM_ID;
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;
	unsigned short aus_pid[1] = {0x1234};
	int i=0;
//	  int j=0;
	unsigned long ul_key_hdl_tmp=0xff;
	unsigned long ul_key_grp_idx=0;
	DES_IV_INFO st_des_iv;

	if((!p_hdl_dsc)||(!(pst_hdl_dsc->p_dev_des)))
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	if(dsc_data_type>=AUI_DSC_DATA_TYPE_NB)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	//unsigned short aus_pid[1] = {us_pid&0x1fff};
	MEMSET(&st_des_iv,0,sizeof(DES_IV_INFO));

	switch(uc_mode)
	{
		case WORK_MODE_IS_ECB:
		{
			en_residue_tmp=RESIDUE_BLOCK_IS_NO_HANDLE;
			break;
		}
		case WORK_MODE_IS_CBC:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			if((RESIDUE_BLOCK_IS_NO_HANDLE != en_residue)
				&&(RESIDUE_BLOCK_IS_AS_ATSC != en_residue)
				&&(RESIDUE_BLOCK_IS_HW_CTS != en_residue)
				&&(RESIDUE_BLOCK_IS_RESERVED != en_residue))
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=en_residue;
			puc_iv_tmp=(unsigned char*)puc_iv_ctr;
			puc_ctr_tmp=NULL;
			break;
		}
		case WORK_MODE_IS_CTR:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=RESIDUE_BLOCK_IS_RESERVED;
			puc_iv_tmp=NULL;
			puc_ctr_tmp=(unsigned char*)puc_iv_ctr;
			break;
		}
		case WORK_MODE_IS_CFB:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=RESIDUE_BLOCK_IS_NO_HANDLE;
			puc_iv_tmp=(unsigned char*)puc_iv_ctr;
			puc_ctr_tmp=NULL;
			break;
		}
		case WORK_MODE_IS_OFB:
		{
			if(!puc_iv_ctr)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
			en_residue_tmp=RESIDUE_BLOCK_IS_NO_HANDLE;
			puc_iv_tmp=(unsigned char*)puc_iv_ctr;
			puc_ctr_tmp=NULL;
			break;
		}
		default:
		{
			aui_rtn(AUI_RTN_FAIL,"Not support work mode");
			break;
		}
	}

	if(AUI_DSC_HOST_KEY_REG==dsc_key_type)
	{
		if(en_parity>(enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;
		}
		else
		{
			en_parity_tmp=EVEN_PARITY_MODE;
		}

		if(!puc_key)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		en_key_tpye_tmp=KEY_FROM_REG;

		ul_key_pos_tmp=0;
		if((ul_key_len != 64) && (ul_key_len != 128) &&  (ul_key_len != 192) && (ul_key_len != 256))
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		ul_key_mode_tmp=ul_key_len/64 - 1;
		ul_key_length_tmp=ul_key_len;
		if(128==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.des_128bits_key.odd_key, puc_key, 16);
				MEMCPY(key_info.des_128bits_key.even_key, puc_key+16, 16);
			}
			else
			{
				MEMCPY(key_info.des_128bits_key.odd_key, puc_key, 16);
				MEMCPY(key_info.des_128bits_key.even_key, puc_key, 16);
			}
		}
		else if(192==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.des_192bits_key.odd_key, puc_key, 24);
				MEMCPY(key_info.des_192bits_key.even_key, puc_key+24, 24);
			}
			else
			{
				MEMCPY(key_info.des_192bits_key.odd_key, puc_key, 24);
				MEMCPY(key_info.des_192bits_key.even_key, puc_key, 24);
			}
		}
		else if(64==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)	
			{
				MEMCPY(key_info.des_64bits_key.odd_key, puc_key, 8);
				MEMCPY(key_info.des_64bits_key.even_key, puc_key+8, 8);
			}
			else
			{
				MEMCPY(key_info.des_64bits_key.odd_key, puc_key, 8);
				MEMCPY(key_info.des_64bits_key.even_key, puc_key, 8);
			}
		}
		else
		{
			aui_rtn(AUI_RTN_FAIL,"Not support ul_key_len");
		}
		p_key_info_tmp=&key_info;
		en_sub_algo=ul_key_len>64?TDES:DES;
	}
	else if(AUI_DSC_HOST_KEY_SRAM==dsc_key_type)
	{
		if(en_parity>(enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;

		}
		else
		{
			en_parity_tmp=EVEN_PARITY_MODE;
		}

		if(!puc_key)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		en_key_tpye_tmp=KEY_FROM_SRAM;


		ul_key_pos_tmp=0;
		if((ul_key_len != 64) && (ul_key_len != 128) &&  (ul_key_len != 192) && (ul_key_len != 256))
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		ul_key_mode_tmp=ul_key_len/64 - 1;
		ul_key_length_tmp=ul_key_len;
		if (AUI_DSC_DATA_TS==dsc_data_type)
        {
			if (128 == ul_key_len)
            {
				if (AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)
                {
					MEMCPY(key_info.des_128bits_key.even_key, puc_key, 16);
				}
                else if (AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern)
                {
					MEMCPY(key_info.des_128bits_key.odd_key, puc_key, 16);
				}
                else if (AUI_DSC_KEY_PATTERN_SINGLE== ul_key_pattern)
                {
					MEMCPY(key_info.des_128bits_key.odd_key, puc_key, 16);
					MEMCPY(key_info.des_128bits_key.even_key, puc_key, 16);
                }
				else
                {
					MEMCPY(key_info.des_128bits_key.odd_key, puc_key, 16);
					MEMCPY(key_info.des_128bits_key.even_key, puc_key+16, 16);
				}
			}
            else if (192==ul_key_len)
			{
				if (AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)
                {
					MEMCPY(key_info.des_192bits_key.even_key, puc_key, 24);
				}
                else if (AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern)
                {
					MEMCPY(key_info.des_192bits_key.odd_key, puc_key, 24);
				}
                else if (AUI_DSC_KEY_PATTERN_SINGLE == ul_key_pattern)
                {
					MEMCPY(key_info.des_192bits_key.odd_key, puc_key, 24);
					MEMCPY(key_info.des_192bits_key.even_key, puc_key, 24);
                }
                else
                {
					MEMCPY(key_info.des_192bits_key.odd_key, puc_key, 24);
					MEMCPY(key_info.des_192bits_key.even_key, puc_key+24, 24);
				}
			}
            else if (64==ul_key_len)
            {
				if (AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)
                {
					MEMCPY(key_info.des_64bits_key.even_key, puc_key, 8);
				}
                else if (AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern)
                {
					MEMCPY(key_info.des_64bits_key.odd_key, puc_key, 8);
				}
                else if (AUI_DSC_KEY_PATTERN_SINGLE == ul_key_pattern)
                {
					MEMCPY(key_info.des_64bits_key.odd_key, puc_key, 8);
					MEMCPY(key_info.des_64bits_key.even_key, puc_key, 8);
				}
                else
                {
					MEMCPY(key_info.des_64bits_key.odd_key, puc_key, 8);
					MEMCPY(key_info.des_64bits_key.even_key, puc_key+8, 8);
				}
			}
            else
            {
			
				aui_rtn(AUI_RTN_FAIL,"Not supported ul_key_len");
			}
		}
        else if (AUI_DSC_DATA_PURE==dsc_data_type)
		{
			if (ul_key_len == 128){
				MEMCPY(key_info.des_128bits_key.even_key, puc_key, ul_key_len/8);
			}else if (ul_key_len == 64){
				MEMCPY(key_info.des_64bits_key.even_key, puc_key, ul_key_len/8);
			}else if (ul_key_len == 192){
				MEMCPY(key_info.des_192bits_key.even_key, puc_key, ul_key_len/8);
			}else{
				aui_rtn(AUI_RTN_FAIL,"Not supported ul_key_len");
			}
		}
		p_key_info_tmp=&key_info;
		en_sub_algo=ul_key_len>64?TDES:DES;

	}
	else if(AUI_DSC_CONTENT_KEY_OTP==dsc_key_type)
	{
		if(((enum PARITY_MODE)OTP_KEY_FROM_68!=en_parity)&&((enum PARITY_MODE)OTP_KEY_FROM_6C!=en_parity))
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		en_parity_tmp=en_parity;
		en_key_tpye_tmp=KEY_FROM_OTP;

		ul_key_pos_tmp=0;
		ul_key_mode_tmp=TDES_ABA_MODE;//AES_128BITS_MODE;
		ul_key_length_tmp=128;
		p_key_info_tmp=NULL;
		en_sub_algo=TDES;
	}
	else if(AUI_DSC_CONTENT_KEY_KL==dsc_key_type)
	{
		if(en_parity>(enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;
		}
		else
		{
			en_parity_tmp=EVEN_PARITY_MODE;
		}

		en_key_tpye_tmp=KEY_FROM_CRYPTO;

		ul_key_pos_tmp=ul_key_pos;
		ul_key_mode_tmp=TDES_ABA_MODE;//AES_128BITS_MODE;
		ul_key_length_tmp=128;
		p_key_info_tmp=NULL;
		en_sub_algo=ul_key_len>64?TDES:DES;
	}
	else
	{
		aui_rtn(AUI_RTN_FAIL,"Not supported dsc_key_type");
	}

	if(AUI_DSC_DATA_TS==dsc_data_type)
	{
		en_data_type_tmp=TS_MODE;
	}
	else if(AUI_DSC_DATA_PURE==dsc_data_type)
	{
		en_data_type_tmp=PURE_DATA_MODE;
		pus_pid_list_tmp=aus_pid;
		ul_pid_cnt=1;
	}
	else
	{
		aui_rtn(AUI_RTN_FAIL,"Not supported dsc_data_type");
	}
	MEMSET( &st_des_param, 0, sizeof ( struct des_init_param ) );
	st_des_param.dma_mode = en_data_type_tmp;
	st_des_param.key_from = en_key_tpye_tmp;
	st_des_param.key_mode = ul_key_mode_tmp ;
	st_des_param.parity_mode = en_parity_tmp;
	st_des_param.residue_mode = en_residue_tmp;
	st_des_param.stream_id = pst_hdl_dsc->ul_stream_id;
	st_des_param.work_mode = uc_mode ;
	st_des_param.sub_module = en_sub_algo;
	ret = des_ioctl ( (pst_hdl_dsc->p_dev_des) , IO_INIT_CMD , ( unsigned long ) &st_des_param );
	if(ret != RET_SUCCESS)
	{
		aui_rtn(AUI_RTN_FAIL,"IO_INIT_CMD failed");
	}
	if(!pst_hdl_dsc->pvr_crypto_mode){
		for(i=0;i<(int)ul_pid_cnt;i++)
		{
			MEMSET(&key_param, 0, sizeof(KEY_PARAM));

			if(AUI_DSC_DATA_TS==dsc_data_type)
			{
				pus_pid_list_tmp=pus_pid+i;
			}
			else if(AUI_DSC_DATA_PURE==dsc_data_type)
			{
				pus_pid_list_tmp=aus_pid;
			}
			if(0==i)
			{
				if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,1))
				{
					aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
				}
			}
			else
			{
				if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,0))
				{
					aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
				}
			}
			key_param.handle = ul_key_hdl_tmp;//pst_hdl_dsc->ul_key_param_handle;//0xFF ;
			key_param.ctr_counter = puc_ctr_tmp;
			key_param.init_vector = puc_iv_tmp;//KEY_FROM_REG/KEY_FROM_OTP using this ptr
			key_param.key_length = ul_key_length_tmp;
			key_param.p_des_key_info = p_key_info_tmp;

			key_param.p_des_iv_info= &st_des_iv;
			if(( AUI_DSC_KEY_PATTERN_SINGLE == ul_key_pattern)||
					( AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)){
				key_param.no_odd = 1;
				key_param.no_even = 0;
			}
			else if(AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern){
				key_param.no_odd = 0;
				key_param.no_even = 1;
			}
			else if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern){
				key_param.no_odd = 0;
				key_param.no_even = 0;
			}
			
            if (NULL != puc_iv_tmp)            
            {
    			if(( AUI_DSC_KEY_PATTERN_SINGLE == ul_key_pattern)||
					( AUI_DSC_KEY_PATTERN_EVEN == ul_key_pattern)){
    				MEMCPY(key_param.p_des_iv_info->even_iv,puc_iv_tmp,8);
					
    			}
    			else if(AUI_DSC_KEY_PATTERN_ODD == ul_key_pattern){
    				MEMCPY(key_param.p_des_iv_info->odd_iv,puc_iv_tmp,8);
					
    			}
    			else if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern){
    				MEMCPY(key_param.p_des_iv_info->odd_iv,puc_iv_tmp,8);
    				MEMCPY(key_param.p_des_iv_info->even_iv,puc_iv_tmp+8,8);	
    			}
			}
			key_param.pid_list = pus_pid_list_tmp;
			key_param.pid_len = 1;
			key_param.stream_id = pst_hdl_dsc->ul_stream_id;
			key_param.pos = ul_key_pos_tmp & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																		Bit[7:0]  --->key pos*/
			key_param.kl_sel = (ul_key_pos_tmp>>8) & 0xFF;
			pst_hdl_dsc->attr_dsc.ul_key_pos=key_param.pos;
			AUI_DBG("\r\n ******************before attach key_param=[%08x][%08x]",key_param.stream_id,key_param.handle);
			//key_param.stream_id = ALI_INVALID_CRYPTO_STREAM_ID;//no needed when key from reg
			if(key_param.handle == 0xFF)
			{
				ret = des_ioctl ((pst_hdl_dsc->p_dev_des) ,IO_CREAT_CRYPT_STREAM_CMD , (unsigned long)&key_param);
			}
			else
			{
				ret = des_ioctl ((pst_hdl_dsc->p_dev_des) ,IO_KEY_INFO_UPDATE_CMD , (unsigned long)&key_param);
			}

			//pst_hdl_dsc->ul_key_param_handle=key_param.handle;
			
			AUI_DBG("\r\n ******************after attach key_param=[%08x]",key_param.handle);
			AUI_DBG("pst_hdl_dsc->key_param.handle: 0x%08x\n",key_param.handle);
			if(ret != RET_SUCCESS)
			{
				aui_rtn(AUI_RTN_FAIL,NULL);
			}
			if(0!=set_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,key_param.handle,ul_key_grp_idx))
			{
				aui_rtn(AUI_RTN_FAIL,"\r\n set_pids_key_hdl fail");
			}
		}
	}else{
	
		MEMSET(&key_param, 0, sizeof(KEY_PARAM));
		key_param.handle = ul_key_hdl_tmp;//pst_hdl_dsc->ul_key_param_handle;//0xFF ;
		key_param.ctr_counter = puc_ctr_tmp;
		key_param.init_vector = puc_iv_tmp;//KEY_FROM_REG/KEY_FROM_OTP using this ptr
		key_param.key_length = ul_key_length_tmp;
		key_param.p_des_key_info = p_key_info_tmp;

		key_param.p_des_iv_info= &st_des_iv;
        if (NULL != puc_iv_tmp)  
		    MEMCPY(key_param.p_des_iv_info->even_iv,puc_iv_tmp,8);
		key_param.pid_list = pus_pid_list_tmp;
		key_param.pid_len = ul_pid_cnt;
		key_param.stream_id = pst_hdl_dsc->ul_stream_id;
		key_param.pos = ul_key_pos_tmp & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																	Bit[7:0]  --->key pos*/
		key_param.kl_sel = (ul_key_pos_tmp>>8) & 0xFF;
		pst_hdl_dsc->attr_dsc.ul_key_pos=key_param.pos;
		AUI_DBG("\r\n ******************before attach key_param=[%08x][%08x]",key_param.stream_id,key_param.handle);
		//key_param.stream_id = ALI_INVALID_CRYPTO_STREAM_ID;//no needed when key from reg
		ret = des_ioctl ((pst_hdl_dsc->p_dev_des) ,IO_CREAT_CRYPT_STREAM_CMD , (unsigned long)&key_param);
		//pst_hdl_dsc->ul_key_param_handle=key_param.handle;
		
		if(ret != RET_SUCCESS)
		{
			aui_rtn(AUI_RTN_FAIL,NULL);
		}
		pst_hdl_dsc->key_handle = key_param.handle;
		AUI_DBG("pst_hdl_dsc->key_param.handle: 0x%08x\n",key_param.handle);
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_csa_attach_key(aui_hdl p_hdl_dsc,aui_dsc_data_type dsc_data_type,aui_dsc_key_type dsc_key_type,unsigned char *puc_key,unsigned long ul_key_len,
	enum PARITY_MODE en_parity,unsigned short *pus_pid,unsigned long ul_pid_cnt,unsigned long ul_key_pos, aui_dsc_csa_version csa_version, unsigned long ul_key_pattern)
{
	struct csa_init_param st_csa_param;
	KEY_PARAM key_param_tmp;
	CSA_KEY_PARAM key_info;
	RET_CODE ret = RET_FAILURE;
	unsigned char *puc_iv_tmp=NULL;
	unsigned char *puc_ctr_tmp=NULL;
	//enum RESIDUE_BLOCK en_residue_tmp=0xffffffff;
	//unsigned long ul_stream_id_tmp=ALI_INVALID_CRYPTO_STREAM_ID;
	unsigned short *pus_pid_list_tmp=pus_pid;
	//unsigned long ul_pid_cnt=0;
	unsigned long ul_key_pos_tmp=0;
	//unsigned long ul_key_mode_tmp=0;
	unsigned long ul_key_length_tmp=0;
	//DES_KEY_PARAM *p_key_info_tmp=NULL;
	enum KEY_TYPE en_key_tpye_tmp=0xffffffff;
	//enum WORK_SUB_MODULE en_sub_algo=0xffffffff;
	enum PARITY_MODE en_parity_tmp=0xffffffff;
	enum DMA_MODE en_data_type_tmp=0xffffffff;
	//unsigned long ul_stream_id_tmp=ALI_INVALID_CRYPTO_STREAM_ID;
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;
	int i=0;
	int j=0;
	unsigned long ul_key_hdl_tmp=0xff;
	unsigned long ul_key_grp_idx=0;

	AUI_DBG("r\n **************************attach DSC :p_hdl_dsc=[%08x][%08x][%08x][%08x][%08x][%08x][%08x]\r\n",p_hdl_dsc,dsc_data_type,dsc_key_type,ul_key_len,pus_pid[0],ul_pid_cnt,ul_key_pos);
	for(i=0;i<(int)ul_pid_cnt;i++)
	{
		AUI_DBG("\r\n +++++++++++++++++++++++++key pid =[%d][%d]\n",i, pus_pid[i]);
	}
	for(j=0;j<(int)AUI_DSC_HDL_KEY_GROUP_CNT_MAX;j++)
	{
		for(i=0;i<AUI_DSC_HDL_KEY_PID_CNT_MAX;i++)
		{
			AUI_DBG("\r\n hdl pid:keyhdl[%d]=[%08x][%08x]\n",i, pst_hdl_dsc->key_hdls[j].aus_pids_key_hdls[i],pst_hdl_dsc->key_hdls[j].aul_key_param_handles[i]);
		}
	}
	if((!p_hdl_dsc)||(!(pst_hdl_dsc->p_dev_csa)))
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	if(dsc_data_type>=AUI_DSC_DATA_TYPE_NB)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	if(AUI_DSC_DATA_TS!=dsc_data_type)
	{
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	unsigned short aus_pid[1] = {0x1234};

	if(AUI_DSC_HOST_KEY_SRAM==dsc_key_type)
	{
		if(NULL==puc_key)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(en_parity>(enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;
			if(pst_hdl_dsc->ul_stream_id == ALI_INVALID_CRYPTO_STREAM_ID)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}

		}
		else
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}

		/*if(!puc_key) //bug detective
		{
			aui_rtn(AUI_RTN_EINVAL,NULL);
		}*/
		en_key_tpye_tmp=KEY_FROM_SRAM;

		AUI_DBG("\r\n ul_stream_id=[%d].",pst_hdl_dsc->ul_stream_id);
		ul_key_pos_tmp=0;
		//ul_key_mode_tmp=ul_key_len/64 - 1;
		ul_key_length_tmp=ul_key_len;
		if(128==ul_key_len)
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.csa3_key.odd_key, puc_key, 16);
				MEMCPY(key_info.csa3_key.even_key, puc_key+16, 16);
			}
			else
			{
				MEMCPY(key_info.csa3_key.odd_key, puc_key, 16);
				MEMCPY(key_info.csa3_key.even_key, puc_key, 16);
			}
		}
		else if(64==ul_key_len) //bug detective
		{
			if(AUI_DSC_KEY_PATTERN_ODD_EVEN == ul_key_pattern)
			{
				MEMCPY(key_info.csa_key.odd_key, puc_key, 8);
				MEMCPY(key_info.csa_key.even_key, puc_key+8, 8);
			}
			else
			{
				MEMCPY(key_info.csa_key.odd_key, puc_key, 8);
				MEMCPY(key_info.csa_key.even_key, puc_key, 8);
			}
		}
		else
		{
			aui_rtn(AUI_RTN_FAIL,"Not supported ul_key_len");
		}
		//p_key_info_tmp=&key_info;
		//en_sub_algo=ul_key_len>64?TDES:DES;

	}
	else if(AUI_DSC_CONTENT_KEY_KL==dsc_key_type)
	{
		if(en_parity>(enum PARITY_MODE)OTP_KEY_FROM_6C)
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			en_parity_tmp=en_parity;
			if(pst_hdl_dsc->ul_stream_id == ALI_INVALID_CRYPTO_STREAM_ID)
			//if ((ul_stream_id = dsc_get_free_stream_id(TS_MODE)) == ALI_INVALID_CRYPTO_STREAM_ID)
			{
				aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			}
		}
		else
		{
			aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
		}

		en_key_tpye_tmp=KEY_FROM_CRYPTO;


		ul_key_pos_tmp=ul_key_pos;
		//ul_key_mode_tmp=AES_128BITS_MODE;
		ul_key_length_tmp= ul_key_len; /*In CSA1 or CSA2, ul_key_length_tmp should be equal to 64;In CSA3,ul_key_length_tmp should be equal to 128*/
		//p_key_info_tmp=NULL;
		//en_sub_algo=ul_key_len>64?TDES:DES;
	}
	else
	{
		aui_rtn(AUI_RTN_FAIL,"Not supported dsc_key_type");
	}


	if(AUI_DSC_DATA_TS==dsc_data_type)
	{
		en_data_type_tmp=TS_MODE;
	}
	else
	{
		en_data_type_tmp=PURE_DATA_MODE;
		pus_pid_list_tmp=aus_pid;
		ul_pid_cnt=1;
	}



	MEMSET( &st_csa_param, 0, sizeof ( struct csa_init_param ) );
	st_csa_param.dma_mode = en_data_type_tmp;
	st_csa_param.key_from = en_key_tpye_tmp;
	//st_csa_param.key_mode = ul_key_mode_tmp ;
	st_csa_param.parity_mode = en_parity_tmp;
	//st_csa_param.residue_mode = en_residue_tmp;
	st_csa_param.stream_id = pst_hdl_dsc->ul_stream_id;;
	st_csa_param.version = csa_version;//CSA1;//CSA2 ;
	//st_csa_param.sub_module = en_sub_algo;
	ret = csa_ioctl ( (pst_hdl_dsc->p_dev_csa) , IO_INIT_CMD , ( unsigned long ) &st_csa_param );
	if(ret != RET_SUCCESS)
	{
		aui_rtn(AUI_RTN_FAIL,"IO_INIT_CMD failed");
	}


	for(i=0;i<(int)ul_pid_cnt;i++)
	{
		MEMSET(&key_param_tmp, 0, sizeof(key_param_tmp));
		if(AUI_DSC_DATA_TS==dsc_data_type)
		{
			pus_pid_list_tmp=pus_pid+i;
		}
		else if(AUI_DSC_DATA_PURE==dsc_data_type)
		{
			pus_pid_list_tmp=aus_pid;
		}
		AUI_DBG("\r\n 11pidcnt:grpidx:pid=[%d]:[%d][%d][%d]",i,ul_pid_cnt,ul_key_grp_idx,*pus_pid_list_tmp);
		if(0==i)
		{
			if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,1))
			{
				aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
			}
			AUI_DBG("\r\n 22pidcnt:grpidx:pid=[%d]:[%d][%d][%d]",i,ul_pid_cnt,ul_key_grp_idx,*pus_pid_list_tmp);
		}
		else
		{
			AUI_DBG("\r\n 33pidcnt:grpidx:pid=[%d]:[%d][%d][%d]",i,ul_pid_cnt,ul_key_grp_idx,*pus_pid_list_tmp);
			if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,0))
			{
				aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
			}
			AUI_DBG("\r\n 44pidcnt:grpidx:pid=[%d]:[%d][%d][%d]",i,ul_pid_cnt,ul_key_grp_idx,*pus_pid_list_tmp);
		}
		key_param_tmp.handle = ul_key_hdl_tmp;//pst_hdl_dsc->ul_key_param_handle;//0xFF ;
		key_param_tmp.ctr_counter = puc_ctr_tmp;
		key_param_tmp.init_vector = puc_iv_tmp;//KEY_FROM_REG/KEY_FROM_OTP using this ptr
		key_param_tmp.key_length = ul_key_length_tmp;
		key_param_tmp.p_csa_key_info = &key_info;

		key_param_tmp.pid_list = pus_pid_list_tmp;
		key_param_tmp.pid_len = 1;
		key_param_tmp.stream_id = pst_hdl_dsc->ul_stream_id;;
		key_param_tmp.pos = ul_key_pos_tmp & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																	Bit[7:0]  --->key pos*/
		key_param_tmp.kl_sel = (ul_key_pos_tmp>>8) & 0xFF;
		pst_hdl_dsc->attr_dsc.ul_key_pos=key_param_tmp.pos;
		//key_param.stream_id = ALI_INVALID_CRYPTO_STREAM_ID;//no needed when key from reg

		//aui_dbg_printf(AUI_MODULE_DSC,1,"before aui_csa_attach_key, hdl=0x%x\n", pst_hdl_dsc->ul_key_param_handle);
		//pCSA_DEV tmp_dev = pst_hdl_dsc->p_dev_csa;
		//aui_dbg_printf(AUI_MODULE_DSC,1,"dev=0x%x, ioctl=0x%x, param=0x%x\n", tmp_dev, tmp_dev->Ioctl, (unsigned long)&key_param_tmp);
		if(key_param_tmp.handle == 0xFF)
		{
			ret = csa_ioctl ((pst_hdl_dsc->p_dev_csa) ,IO_CREAT_CRYPT_STREAM_CMD , (unsigned long)&key_param_tmp);
		}
		else
		{
			ret = csa_ioctl((pst_hdl_dsc->p_dev_csa) ,IO_KEY_INFO_UPDATE_CMD , (unsigned long)&key_param_tmp);
		}

		//pst_hdl_dsc->ul_key_param_handle=key_param_tmp.handle;
		AUI_DBG("\r\n pid=[%08x],key_param_tmp.handle=[%08x] ",pus_pid_list_tmp[0],key_param_tmp.handle);
		//aui_dbg_printf(AUI_MODULE_DSC,1,"after aui_csa_attach_key, hdl=0x%x\n", pst_hdl_dsc->ul_key_param_handle);
		if(ret != RET_SUCCESS)
		{
			AUI_ERR("IO_CREAT_CRYPT_STREAM_CMD fail, ret 0x%x\n", ret);
			return AUI_RTN_FAIL;
		}
		AUI_DBG("\r\n 55pidcnt:grpidx:pid=[%d]:[%d][%d][%d]",i,ul_pid_cnt,ul_key_grp_idx,*pus_pid_list_tmp);
		if(0!=set_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,key_param_tmp.handle,ul_key_grp_idx))
		{
			aui_rtn(AUI_RTN_FAIL,"\r\n set_pids_key_hdl fail");
		}
		AUI_DBG("\r\n 66pidcnt:grpidx:pid=[%d]:[%d][%d][%d]",i,ul_pid_cnt,ul_key_grp_idx,*pus_pid_list_tmp);
	}

	AUI_DBG("\r\n attach pp_hdl_dsc stream id=[%08x][%08x][%08x]",pst_hdl_dsc,pst_hdl_dsc->p_dev_csa,(pst_hdl_dsc->ul_stream_id));

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_attach_key_info2dsc(aui_hdl p_hdl_dsc,const aui_attr_dsc *p_attr_dsc)
{
    int i=0;
	AUI_RTN_CODE aui_rtn_last=AUI_RTN_SUCCESS; 
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;
	
	unsigned long ul_key_hdl_tmp=0xff;
	unsigned long ul_key_grp_idx=0;
	unsigned short *pus_pid_list_tmp=NULL;
	unsigned long old_pid_handle[AUI_DSC_HDL_KEY_PID_CNT_MAX];
	unsigned char flag_change_key=0;
	
    osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_dsc)
		||(NULL==pst_hdl_dsc->p_dev_dsc)
		||(OSAL_INVALID_ID==pst_hdl_dsc->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	osal_mutex_lock(pst_hdl_dsc->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dsc);
	if(NULL==p_attr_dsc)//bug detective
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

    //p_attr_dsc->uc_mode=AUI_DSC_WORK_MODE_IS_CBC;
    AUI_DBG("\r\n [%08x]dsc cfg[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]\r\n dsc puc_key:",p_hdl_dsc,
                                                        p_attr_dsc->uc_dev_idx,
                                                        p_attr_dsc->dsc_data_type,
                                                        p_attr_dsc->dsc_key_type,
                                                        p_attr_dsc->uc_algo,
                                                        p_attr_dsc->uc_mode,
                                                        p_attr_dsc->ul_key_len,
                                                        p_attr_dsc->ul_key_pattern,
                                                        p_attr_dsc->en_parity,
                                                        p_attr_dsc->en_residue,
                                                        p_attr_dsc->ul_key_pos,
                                                        p_attr_dsc->en_en_de_crypt,
                                                        p_attr_dsc->ul_pid_cnt);

    for(i=0;i<16;i++)
    {
        if(NULL!=p_attr_dsc->puc_key)
        {
            AUI_DBG("[%02x]",p_attr_dsc->puc_key[i]);
        }
    }
    AUI_DBG("\r\n puc_iv_ctr:");
    for(i=0;i<16;i++)
    {
        if(NULL!=p_attr_dsc->puc_iv_ctr)
        {
            AUI_DBG("[%02x]",p_attr_dsc->puc_iv_ctr[i]);
        }
    }
    AUI_DBG("\r\n pids:");
    for(i=0;i<2;i++)
    {
        if(NULL!=p_attr_dsc->pus_pids)
        {
            AUI_DBG("[%02x]",p_attr_dsc->pus_pids[i]);
        }
    }


	if(ALI_INVALID_CRYPTO_STREAM_ID==pst_hdl_dsc->ul_stream_id)
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	if((AUI_DSC_WORK_MODE_IS_ECB!=p_attr_dsc->uc_mode)&&(AUI_DSC_DATA_PURE==p_attr_dsc->dsc_data_type))//pure date and not ECB,so must update iv every 47k
	{
		
		pst_hdl_dsc->attr_dsc.uc_mode=p_attr_dsc->uc_mode;
		pst_hdl_dsc->attr_dsc.dsc_data_type=p_attr_dsc->dsc_data_type;
	}
	

	/* copy IV */
	//pid_len = (p_attr_dsc->ul_pid_cnt ? p_attr_dsc->ul_pid_cnt : 1);
	AUI_DBG("\npst_hdl_dsc->attr_dsc.uc_algo:%d\n",pst_hdl_dsc->attr_dsc.uc_algo);
	if (NULL != p_attr_dsc->puc_iv_ctr)
	{
		switch (pst_hdl_dsc->attr_dsc.uc_algo) {
		case AUI_DSC_ALGO_DES:
		case AUI_DSC_ALGO_TDES:
			MEMCPY(pst_hdl_dsc->iv_update_buffer,p_attr_dsc->puc_iv_ctr,8);
			break;
		case AUI_DSC_ALGO_AES:
			MEMCPY(pst_hdl_dsc->iv_update_buffer,p_attr_dsc->puc_iv_ctr,16);		
			break;
		case AUI_DSC_ALGO_CSA:
			/*algorithm don't use p_attr_dsc->puc_iv_ctr variable*/
			break;
		 default:
		 	osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		 	aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
			break;
		}
	}

	// Before attach key for PIDs, check the list whether these PIDs already created key handles.
	if(((int)p_attr_dsc->ul_pid_cnt)&&(p_attr_dsc->pus_pids!=NULL)){
		pus_pid_list_tmp=p_attr_dsc->pus_pids;
		if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,1)){
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
		}
	}
	//Check whether the key source(key type) is changed for these PIDs. If key type is changed, we have to create new key handles for these PIDs.
	//Here backup the old key handles, and clear handles in the list, so that we can create new key handles.
	flag_change_key=0;
	if(0xFF!=ul_key_hdl_tmp)//old pid
	{
		aui_dsc_key_type key_type_temp = pst_hdl_dsc->key_hdls[ul_key_grp_idx].key_from ;
		if(key_type_temp != p_attr_dsc->dsc_key_type )//source of key has been changed
		{			
			flag_change_key=1;			//source of key has been changed
			for(i = 0 ; i < AUI_DSC_HDL_KEY_PID_CNT_MAX ; i++)
				old_pid_handle[i]=(pst_hdl_dsc->key_hdls[ul_key_grp_idx].aul_key_param_handles[i]);	

			release_keys_hdls_by_grp_idx(pst_hdl_dsc,ul_key_grp_idx);//delete key_handle of old pid and old pid 
		}
	}
	pst_hdl_dsc->key_hdls[ul_key_grp_idx].key_from= p_attr_dsc->dsc_key_type ;//record source of key

	if(AUI_DSC_ALGO_AES==pst_hdl_dsc->attr_dsc.uc_algo)
	{
		aui_rtn_last = aui_aes_attach_key(p_hdl_dsc,p_attr_dsc->dsc_data_type,p_attr_dsc->dsc_key_type,p_attr_dsc->uc_mode,
								p_attr_dsc->puc_key,p_attr_dsc->ul_key_len,p_attr_dsc->puc_iv_ctr,p_attr_dsc->en_parity,
								p_attr_dsc->en_residue,p_attr_dsc->pus_pids,p_attr_dsc->ul_pid_cnt,p_attr_dsc->ul_key_pos,
								p_attr_dsc->ul_key_pattern);
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		return aui_rtn_last;
	}
	else if((AUI_DSC_ALGO_DES==pst_hdl_dsc->attr_dsc.uc_algo)||(AUI_DSC_ALGO_TDES==pst_hdl_dsc->attr_dsc.uc_algo))
	{
		aui_rtn_last = aui_des_attach_key(p_hdl_dsc,p_attr_dsc->dsc_data_type,p_attr_dsc->dsc_key_type,p_attr_dsc->uc_mode,
								p_attr_dsc->puc_key,p_attr_dsc->ul_key_len,p_attr_dsc->puc_iv_ctr,p_attr_dsc->en_parity,
								p_attr_dsc->en_residue,p_attr_dsc->pus_pids,p_attr_dsc->ul_pid_cnt,p_attr_dsc->ul_key_pos,
								p_attr_dsc->ul_key_pattern);
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		return aui_rtn_last;
	}
	else if((AUI_DSC_ALGO_CSA==pst_hdl_dsc->attr_dsc.uc_algo))
	{
		aui_rtn_last = aui_csa_attach_key(p_hdl_dsc,p_attr_dsc->dsc_data_type,p_attr_dsc->dsc_key_type,
								p_attr_dsc->puc_key,p_attr_dsc->ul_key_len,p_attr_dsc->en_parity,
								p_attr_dsc->pus_pids,p_attr_dsc->ul_pid_cnt,p_attr_dsc->ul_key_pos,
								p_attr_dsc->csa_version,p_attr_dsc->ul_key_pattern);
	}

	
	if(flag_change_key)				// Delete old key handles.
		for(i = 0 ; i < AUI_DSC_HDL_KEY_PID_CNT_MAX ; i++)
			if(0xFF!=old_pid_handle[i])
				dsc_ioctl(p_hdl_dsc,IO_DSC_DELETE_HANDLE_CMD,old_pid_handle[i]);
			
	osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
	return aui_rtn_last;

}




AUI_RTN_CODE aui_dsc_deattach_key_by_pid(aui_hdl p_hdl_dsc,unsigned short us_pid)
{
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;
	int i=0;
	int j=0;
	unsigned long ul_grp_idx=0;
	//unsigned long ul_key_hdl=0;

	osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_dsc)
		||(NULL==pst_hdl_dsc->p_dev_dsc)
		||(OSAL_INVALID_ID==pst_hdl_dsc->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	osal_mutex_lock(pst_hdl_dsc->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dsc);
	AUI_DBG("\r\n _____________________________del key pid=[%d]",us_pid);
	for(i=0;i<AUI_DSC_HDL_KEY_GROUP_CNT_MAX;i++)
	{
		for(j=0;j<AUI_DSC_HDL_KEY_PID_CNT_MAX;j++)
		{
			AUI_DBG("\r\n rd pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[i].aus_pids_key_hdls[j]);
			if(us_pid==pst_hdl_dsc->key_hdls[i].aus_pids_key_hdls[j])
			{
				AUI_DBG("\r\n rd return1 pid:keyhdl=[%d][%d][%08x][%08x]",i,j,us_pid,pst_hdl_dsc->key_hdls[i].aul_key_param_handles[j]);
				ul_grp_idx=i;
				if(AUI_RTN_SUCCESS!=release_keys_hdls_by_grp_idx(pst_hdl_dsc,ul_grp_idx))
				{
					osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
					aui_rtn(AUI_RTN_EINVAL,"Release key handle failed");
				}
				osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
				return AUI_RTN_SUCCESS;
			}
		}
	}
	osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
	aui_rtn(AUI_RTN_FAIL,"deattach failed");
}


AUI_RTN_CODE aui_dsc_encrypt(aui_hdl p_hdl_dsc,unsigned char *puc_data_in,unsigned char* puc_data_out,unsigned long ul_data_len)
{
	aui_attr_dsc *p_attr_dsc=NULL;
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;

	osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_dsc)
		||(NULL==pst_hdl_dsc->p_dev_dsc)
		||(OSAL_INVALID_ID==pst_hdl_dsc->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	if (((unsigned int)puc_data_in & 0x3) || ((unsigned int)puc_data_out & 0x3))
	{
		AUI_ERR("input address and output address need be 4-byte alignment\n");
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	osal_mutex_lock(pst_hdl_dsc->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dsc);


	if((NULL==puc_data_in)||(NULL==puc_data_out))//bug detective
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	p_attr_dsc=&(pst_hdl_dsc->attr_dsc);
	if(AUI_DSC_ALGO_AES==p_attr_dsc->uc_algo)
	{
		if(RET_SUCCESS!=aes_encrypt((pst_hdl_dsc->p_dev_aes),pst_hdl_dsc->ul_stream_id,puc_data_in,puc_data_out,ul_data_len))
		{
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,"AES encrypt failed");
		}
	}
	else if((AUI_DSC_ALGO_DES==p_attr_dsc->uc_algo)||(AUI_DSC_ALGO_TDES==p_attr_dsc->uc_algo))
	{
		if(RET_SUCCESS!=des_encrypt((pst_hdl_dsc->p_dev_des),pst_hdl_dsc->ul_stream_id,puc_data_in,puc_data_out,ul_data_len))
		{
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,"DES encrypt failed");
		}
	}
	else if((AUI_DSC_ALGO_SHA==p_attr_dsc->uc_algo))
	{
		if(RET_SUCCESS!=sha_digest((pst_hdl_dsc->p_dev_sha),puc_data_in,puc_data_out,ul_data_len))
		{
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,"SHA digest failed");
		}
	}
	else
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		aui_rtn(AUI_RTN_FAIL,"Not supported Algo");
	}
	osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dsc_decrypt(aui_hdl p_hdl_dsc,unsigned char *puc_data_in,unsigned char* puc_data_out,unsigned long ul_data_len)
{
	aui_attr_dsc *p_attr_dsc=NULL;
	aui_handle_dsc *pst_hdl_dsc=p_hdl_dsc;

	osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if((NULL==p_hdl_dsc)
		||(NULL==pst_hdl_dsc->p_dev_dsc)
		||(OSAL_INVALID_ID==pst_hdl_dsc->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	if (((unsigned int)puc_data_in & 0x3) || ((unsigned int)puc_data_out & 0x3))
	{
		AUI_ERR("input address and output address need be 4-byte alignment\n");
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}
	osal_mutex_lock(pst_hdl_dsc->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dsc);


	if((NULL==puc_data_in)||(NULL==puc_data_out))//bug detective
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	p_attr_dsc=&(pst_hdl_dsc->attr_dsc);
	if(AUI_DSC_ALGO_AES==p_attr_dsc->uc_algo)
	{
		if(RET_SUCCESS!=aes_decrypt((pst_hdl_dsc->p_dev_aes),pst_hdl_dsc->ul_stream_id,puc_data_in,puc_data_out,ul_data_len))
		{
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,"aes_decrypt failed");
		}
	}
	else if((AUI_DSC_ALGO_DES==p_attr_dsc->uc_algo)||(AUI_DSC_ALGO_TDES==p_attr_dsc->uc_algo))
	{
		if(RET_SUCCESS!=des_decrypt((pst_hdl_dsc->p_dev_des),pst_hdl_dsc->ul_stream_id,puc_data_in,puc_data_out,ul_data_len))
		{
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,"des_decrypt failed");
		}
	}
	else if((AUI_DSC_ALGO_SHA==p_attr_dsc->uc_algo))
	{
		if(RET_SUCCESS!=sha_digest((pst_hdl_dsc->p_dev_sha),puc_data_in,puc_data_out,ul_data_len))
		{
			osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
			aui_rtn(AUI_RTN_FAIL,"sha_digest failed");
		}
	}
	else
	{
		osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
		aui_rtn(AUI_RTN_FAIL,"Not supported Algo");
	}
	osal_mutex_unlock(pst_hdl_dsc->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_sha_digest(unsigned long source,
				unsigned char *puc_data_in, unsigned long ul_data_len,
				unsigned long sha_mode, unsigned char *puc_data_out)
{
    if ((sys_ic_get_chip_id() < ALI_C3505) && 
        ((unsigned int)puc_data_in & 0x3))
    {
        aui_rtn(AUI_RTN_EINVAL,"input address need be 4-byte alignment");
    }        

	switch (sha_mode)
	{
		case AUI_SHA_1: sha_mode = SHA_SHA_1; break;
		case AUI_SHA_224: sha_mode = SHA_SHA_224; break;
		case AUI_SHA_256: sha_mode = SHA_SHA_256; break;
		case AUI_SHA_384: sha_mode = SHA_SHA_384; break;
		case AUI_SHA_512: sha_mode = SHA_SHA_512; break;
		default: return AUI_RTN_EINVAL;
	}

	if(AUI_SHA_SRC_FROM_DRAM==source)
	{
		if(RET_SUCCESS!=ali_hw_sha_digest_dram(puc_data_in,ul_data_len,sha_mode,puc_data_out))
		{
			aui_rtn(AUI_RTN_FAIL,"ali_hw_sha_digest_dram failed");
		}
	}
	else if(AUI_SHA_SRC_FROM_FLASH==source)
	{
		if(RET_SUCCESS!=ali_hw_sha_digest_flash(puc_data_in,ul_data_len,sha_mode,puc_data_out))
		{
			aui_rtn(AUI_RTN_FAIL,"ali_hw_sha_digest_flash failed");
		}
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_set(void *p_hdl_dsc,unsigned long ul_item,void *pv_param)
{
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

    (void)pv_param;
    
	osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if(NULL==(aui_handle_dsc *)p_hdl_dsc)
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}


	switch(ul_item)
	{

		default:
		{
			osal_mutex_unlock(s_mod_mutex_id_dsc);
			aui_rtn(AUI_RTN_FAIL,"Not supported");
			break;
		}
	}
	if(AUI_RTN_SUCCESS!=rtn_code)
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		return rtn_code;
	}
	osal_mutex_unlock(s_mod_mutex_id_dsc);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dsc_get(aui_hdl p_hdl_dsc,unsigned long ul_item,void *pv_param)
{
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
    aui_handle_dsc *hdl=p_hdl_dsc;
	osal_mutex_lock(s_mod_mutex_id_dsc,OSAL_WAIT_FOREVER_TIME);
	if((NULL==hdl) || (NULL==pv_param))
	{
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
	}

	switch(ul_item)
	{
		case AUI_DSC_GET_DATA_TYPE:
            *(unsigned long *)pv_param= hdl->attr_dsc.dsc_data_type;
             rtn_code = AUI_RTN_SUCCESS;
             break;
		default:
		{
			osal_mutex_unlock(s_mod_mutex_id_dsc);
			aui_rtn(AUI_RTN_FAIL,"Not supported");
			break;
		}
	}
		osal_mutex_unlock(s_mod_mutex_id_dsc);
		return rtn_code;
}

// Fixed compile error. Function for linux
AUI_RTN_CODE aui_dsc_get_buffer(unsigned long size,  void **pp_buf)
{
	*pp_buf = MALLOC(size);
	return AUI_RTN_SUCCESS;
}

// Fixed compile error. Function for linux
AUI_RTN_CODE aui_dsc_release_buffer(unsigned long size, void *p_buf)
{
	(void)size;
	FREE(p_buf);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_set_playback(aui_hdl handle)
{
	aui_handle_dsc *pst_hdl_dsc=(aui_handle_dsc *)handle;

	if (!pst_hdl_dsc)
		aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");

    if(	(pst_hdl_dsc->pv_live_dsc_hdl==pst_hdl_dsc)
		&& (AUI_DSC_DATA_TS==pst_hdl_dsc->attr_dsc.dsc_data_type))
    {
        dsc_ioctl(pst_hdl_dsc->p_dev_dsc, IO_PARSE_DMX_ID_SET_CMD, pst_hdl_dsc->ul_stream_id);
    }
	
	return 0;
}

int aui_dsc_get_stream_id(aui_hdl handle)
{
	aui_handle_dsc *pst_hdl_dsc=(aui_handle_dsc *)handle;

	if (!pst_hdl_dsc)
		aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
	
	return pst_hdl_dsc->ul_stream_id;
}

unsigned int aui_dsc_get_subdev_id(aui_hdl handle)
{
    aui_handle_dsc *pst_hdl_dsc=(aui_handle_dsc *)handle;
    unsigned int subdev_id=0xFFFF;

    if (NULL !=pst_hdl_dsc){
        if((AUI_DSC_ALGO_TDES==pst_hdl_dsc->attr_dsc.uc_algo)||(AUI_DSC_ALGO_DES==pst_hdl_dsc->attr_dsc.uc_algo))
			subdev_id=pst_hdl_dsc->ul_sub_des_dev_id;
		else if(AUI_DSC_ALGO_AES==pst_hdl_dsc->attr_dsc.uc_algo)
            subdev_id=pst_hdl_dsc->ul_sub_aes_dev_id;
		else if(AUI_DSC_ALGO_CSA==pst_hdl_dsc->attr_dsc.uc_algo)
            subdev_id=pst_hdl_dsc->ul_sub_csa_dev_id;
        return subdev_id;
    }else{
        return 0xFFFF;
    }
}

AUI_RTN_CODE aui_dsc_process_attr_set (
    aui_hdl handle,
    aui_dsc_process_attr *p_process_attr)
{
    if ((NULL == handle) || (NULL == p_process_attr))
        return 1;
    
	return 0;
}

#ifndef _BUILD_LAUNCHER_E_

static int workmod_to_chainmod(enum aui_dsc_work_mode mode)
{
    int ret = -1;
    switch(mode) {
        case AUI_DSC_WORK_MODE_IS_CBC:
            ret = WORK_MODE_IS_CBC;
            break;
        case AUI_DSC_WORK_MODE_IS_ECB:
            ret = WORK_MODE_IS_ECB;
            break;
        case AUI_DSC_WORK_MODE_IS_OFB:
            ret = WORK_MODE_IS_OFB;
            break;
        case AUI_DSC_WORK_MODE_IS_CFB:
            ret = WORK_MODE_IS_CFB;
            break;
        case AUI_DSC_WORK_MODE_IS_CTR:
            ret = WORK_MODE_IS_CTR;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

static int caparity_to_parity(aui_dsc_parity_mode parity)
{
    int ret = -1;
    switch(parity) {
        case AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE:
            ret = EVEN_PARITY_MODE;
            break;
        case AUI_DSC_PARITY_MODE_ODD_PARITY_MODE:
            ret = ODD_PARITY_MODE;
            break;
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0:
			ret = AUTO_PARITY_MODE0;
            break;
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE1:
            ret = AUTO_PARITY_MODE1;
            break;
        default:
            ret = -1;
			break;
    }
    return ret;
}

static int cares_to_res(aui_dsc_residue_block res)
{
    int ret = -1;
    switch(res) {
        case AUI_DSC_RESIDUE_BLOCK_IS_RESERVED:
			ret = RESIDUE_BLOCK_IS_RESERVED;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE:
            ret = RESIDUE_BLOCK_IS_NO_HANDLE;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC:
            ret = RESIDUE_BLOCK_IS_AS_ATSC;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_HW_CTS:
            ret = RESIDUE_BLOCK_IS_HW_CTS;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

AUI_RTN_CODE aui_dsc_update_pvr_encrypt_key_info(
	aui_hdl handle,
	aui_attr_dsc *attr,
	struct aui_dsc_encrypt_kl_param *p_kl_attr,
	aui_dsc_process_status* p_encrypt_status)
{

	aui_handle_dsc *pst_hdl_dsc_en=(aui_handle_dsc *)handle;
	int ret = 0;
	struct PVR_BLOCK_ENC_PARAM input_enc;
	unsigned int key_length = 16;/*the unit byte,default aes*/
	unsigned int iv_length = 16;/*the unit byte,default aes*/
   	
	if ((NULL == handle ) || (NULL == attr))
			aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");

	MEMSET(&input_enc,0,sizeof(struct PVR_BLOCK_ENC_PARAM ));

    /* Check PIDs */
    if (attr->dsc_data_type == AUI_DSC_DATA_PURE) {
		input_enc.pid_num = 1;
		input_enc.pid_list[0] = R2R_PID;
    } else {
        if (!attr->ul_pid_cnt)
            aui_rtn( AUI_RTN_EINVAL,
                    "pid required in stream mode");
		if(attr->ul_pid_cnt > 32)/*max pid support 32*/
			input_enc.pid_num = 32;
		else
			input_enc.pid_num = attr->ul_pid_cnt;
		MEMCPY(&input_enc.pid_list,attr->pus_pids,(sizeof(unsigned short))*input_enc.pid_num);
	}
	if(AUI_DSC_ALGO_AES == attr->uc_algo){
		input_enc.dsc_sub_device = AES;
		key_length = 16;
		iv_length = 16;
	}else if((AUI_DSC_ALGO_DES == attr->uc_algo)
		|| (AUI_DSC_ALGO_TDES == attr->uc_algo)){/*only use TDES*/
		input_enc.dsc_sub_device = TDES;
		key_length = 16;
		iv_length = 8;
	}else{
		AUI_ERR("algorithm config error!\n");
		aui_rtn( AUI_RTN_EINVAL,
                    "parity not supported in RAW mode");
	}
	input_enc.work_mode = workmod_to_chainmod(attr->uc_mode);
	input_enc.source_mode = (attr->dsc_data_type == AUI_DSC_DATA_PURE)? PURE_DATA_MODE: TS_MODE;
	input_enc.residue_mode = cares_to_res(attr->en_residue);
	input_enc.key_mode = caparity_to_parity(attr->en_parity);
	//input_enc.update_key_flag |= PVR_AUI_USED_PVR_REMOTE; /*tell see pvr that AUI is using pvr resource*/
	switch(attr->dsc_key_type){
		case AUI_DSC_CONTENT_KEY_KL:{
			CE_FOUND_FREE_POS_PARAM get_key_pos;
			struct ce_device *hld_ce_dev = (struct ce_device *)dev_get_by_id(HLD_DEV_TYPE_CE,0);
			if(pst_hdl_dsc_en->attr_dsc.ul_key_pos == INVALID_ALI_CE_KEY_POS){
				MEMSET(&get_key_pos,0,sizeof(get_key_pos));
				get_key_pos.root = KEY_POS(p_kl_attr->rootkey_index);
				get_key_pos.number = 2;
				get_key_pos.pos=INVALID_ALI_CE_KEY_POS;
				get_key_pos.ce_key_level = THREE_LEVEL;
			    if (RET_SUCCESS!=ce_ioctl(hld_ce_dev, IO_CRYPT_FOUND_FREE_POS, (unsigned long)&get_key_pos)) {
			        aui_rtn(AUI_RTN_FAIL,"Get free key pos failed");
			    }
				pst_hdl_dsc_en->attr_dsc.ul_key_pos = get_key_pos.pos;
			}
			input_enc.root_key_pos = KEY_POS(p_kl_attr->rootkey_index); //only it is set to  invalid parameter,and isn't used.
			input_enc.target_key_pos = pst_hdl_dsc_en->attr_dsc.ul_key_pos;
			input_enc.kl_mode = (p_kl_attr->kl_algo == AUI_KL_ALGO_AES)? 0: 1;
			//input_enc.update_key_flag |= PVR_USE_AUI_CONFIG_KL;
			input_enc.kl_level = p_kl_attr->kl_level;
			if(p_kl_attr->control_word)
				MEMCPY(input_enc.input_key, p_kl_attr->control_word, p_kl_attr->kl_level*16);
			attr->ul_key_pos = pst_hdl_dsc_en->attr_dsc.ul_key_pos;
			break;
		}
		case AUI_DSC_HOST_KEY_SRAM:{
			input_enc.root_key_pos= 0xFF; //r2r key,don't use kl to generate key
			if((AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE == attr->en_parity) 
				|| (AUI_DSC_PARITY_MODE_ODD_PARITY_MODE == attr->en_parity)){
				if(attr->puc_key == NULL){
					aui_rtn( AUI_RTN_EINVAL,"R2R key shouldn't be empty\n");
				}
				MEMCPY(&input_enc.input_key[32],attr->puc_key,key_length);/*only support 128 bit even key*/
			}
			else{
				aui_rtn( AUI_RTN_EINVAL,"parity config error!\n");
			}
			break;
		}
		default:
			aui_rtn( AUI_RTN_EINVAL,"key type config error!\n");
			break;
	}

	if(attr->uc_mode != AUI_DSC_WORK_MODE_IS_ECB){
		if(NULL == attr->puc_iv_ctr){
			aui_rtn( AUI_RTN_EINVAL,"IV can't be empty\n");
		}
		MEMCPY(&input_enc.input_iv,attr->puc_iv_ctr,iv_length);
	}
	input_enc.sub_device_id = pst_hdl_dsc_en->ul_sub_dev_id;//(p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	/*first init,the following two variable is transfered to the see pvr from main cpu*/
	input_enc.stream_id= pst_hdl_dsc_en->ul_stream_id;
	

	AUI_DBG("input_enc.iv:",input_enc.input_iv,16);
	AUI_DBG("input_key.key:",input_enc.input_key,48);
	AUI_DBG("IO_DSC_START_BLOCK_PVR: dsc_sub_device=0x%x,work_mode=0x%x,source_mode=0x%x,root_key_pos=0x%x\n",input_enc.dsc_sub_device,input_enc.work_mode,input_enc.source_mode,input_enc.root_key_pos);
	AUI_DBG("key_mode=0x%x,sub_device_id=0x%x,stream_id=0x%x,target_key_pos=0x%x\n",input_enc.key_mode,input_enc.sub_device_id,input_enc.stream_id,input_enc.target_key_pos);
	int i = 0;
	AUI_DBG("pid count: %d,pid_list:",input_enc.pid_num);
	for(i = 0; i < input_enc.pid_num; i++)
		AUI_DBG(" [%d]: %d,",i,input_enc.pid_list[i]);
	AUI_DBG("\n");	

	if(!pst_hdl_dsc_en->pvr_crypto_mode){/*start block mode, init pvr resource in see cpu*/

		pst_hdl_dsc_en->pvr_crypto_mode = 1;/*this field identify the block mode have been initialized*/
		input_enc.stream_id = 0xFF;//setting stream_id to 0xFF will make the pvr_remote_see to allocate new stream ID
		/*PVR_IO_START_BLOCK_EVO will request the pvr_remote_see to allocate all necessary DSC&KL resources on SEE.       
		the DSC resources allocated in main will not be used pvr_remote_see module*/
		ret = pvr_rpc_ioctl(PVR_IO_START_BLOCK_EVO,(INT32 *)&input_enc);
		if(ret != 0){
			pst_hdl_dsc_en->pvr_crypto_mode = 1;/*this field identify the block mode have been initialized*/
			aui_rtn(AUI_RTN_FAIL,"start block mode fail!\n");
		}
		/*get the stream id allocated by pvr_remote_see, it will be used by aui_dmx_data_path_set()*/
		if(RET_SUCCESS!=dsc_set_stream_id_idle(pst_hdl_dsc_en->ul_stream_id))//set old stream id idle
		{
			aui_rtn(AUI_RTN_FAIL,"Free stream id failed");
		}
		pst_hdl_dsc_en->ul_stream_id = input_enc.stream_id;
	}
	input_enc.key_handle = pst_hdl_dsc_en->key_handle;
	AUI_DBG("input_enc.key_handle: 0x%08x\n",input_enc.key_handle);
	ret = pvr_rpc_ioctl(PVR_IO_UPDATE_ENC_PARAMTOR_EVO,(INT32 *)&input_enc);
	if(ret != 0){
		aui_rtn(AUI_RTN_FAIL,"PVR_IO_UPDATE_ENC_PARAMTOR_EVO failed");
	}
	if(attr->dsc_key_type == AUI_DSC_CONTENT_KEY_KL){
		pst_hdl_dsc_en->attr_dsc.ul_key_pos = input_enc.target_key_pos;
	}
	/*after init,the following two variable is transfered to main cpu from the see pvr */
	pst_hdl_dsc_en->ul_stream_id = input_enc.stream_id;
	pst_hdl_dsc_en->key_handle = input_enc.key_handle;
	pst_hdl_dsc_en->dsc_sub_device = input_enc.dsc_sub_device;
	pst_hdl_dsc_en->ul_sub_dev_id = input_enc.sub_device_id;
	p_encrypt_status->ul_block_count = input_enc.block_count;
	AUI_DBG("input_enc.key_handle: 0x%08x\n",input_enc.key_handle);
	aui_dsc_set_playback(pst_hdl_dsc_en);
	AUI_DBG("PVR_RPC_IO_UPDATE_ENC_PARAMTOR ret=%d ,input_enc.stream_id=%d,"
		"pst_hdl_dsc_en->ul_sub_dev_id: %d\n" ,ret, input_enc.stream_id,pst_hdl_dsc_en->ul_sub_dev_id);
	AUI_DBG("p_encrypt_status->ul_block_count = 0x%08x\n",p_encrypt_status->ul_block_count);
	AUI_DBG("input_enc.target_key_pos: %d\n",input_enc.target_key_pos);
	AUI_DBG("<<<<<<<<<<<<<<<<<<update block mode parameters success>>>>>>>>>>>>>>>>>>>>>>>\n");
	return ret;
}

AUI_RTN_CODE aui_dsc_free_block_mode(aui_hdl handle)
{
	aui_handle_dsc *pst_hdl_dsc_en=(aui_handle_dsc *)handle;
	int ret = 0;
	if (!handle)
		aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");

	ret = pvr_rpc_ioctl(PVR_IO_FREE_BLOCK_EVO, (UINT32 *)pst_hdl_dsc_en->ul_stream_id);
	if(ret != 0){
		aui_rtn(AUI_RTN_FAIL,"PVR_IO_FREE_BLOCK_EVO failed");
	}
	AUI_INFO("<<<<<<<<<<<<<<<<<<free block mode>>>>>>>>>>>>>>>>>>>>>>>\n");
	return ret;
}



/*in pure data mode, not ECB, must update iv every 47k.*/
AUI_RTN_CODE pure_data_update_iv(aui_handle_dsc *pst_hdl_dsc)
{
	
	unsigned long ul_key_hdl_tmp=0xff;//store key handle  
	unsigned long ul_key_grp_idx=0;  //store key grounp
	RET_CODE ret = RET_FAILURE;
	
	AES_IV_INFO st_aes_iv;//the buffer to store iv
	DES_IV_INFO st_des_iv;
	
	unsigned short *pus_pid_list_tmp=NULL;
	unsigned short aus_pid[1] = {0x1234};//pid in pure data mode
	KEY_PARAM key_param;//struct to store data send to dsc_driver

						
	
	MEMSET(&key_param, 0, sizeof(KEY_PARAM));		
	pus_pid_list_tmp=aus_pid;			
	if(AUI_RTN_SUCCESS!=search_pids_key_hdl(pus_pid_list_tmp[0],pst_hdl_dsc,&ul_key_grp_idx,&ul_key_hdl_tmp,1))
	{
	    aui_rtn(AUI_RTN_EINVAL,"Cannot find handle for PIDs");
	}								
	key_param.handle = ul_key_hdl_tmp;//key handle
	/*in pure data mode, dmx only need to update iv*/			
	if(0xFF != key_param.handle)
	{
		/*only update even iv whatever input key parity, because driver only use even key in pure data
			mode.so input key parity must be even, or else it can't update iv successfully.*/
		if(AUI_DSC_ALGO_AES==pst_hdl_dsc->attr_dsc.uc_algo)
		{		
			MEMSET(&st_aes_iv,0,sizeof(AES_IV_INFO));		
			key_param.p_aes_iv_info= &st_aes_iv;	
			/*only need to update even iv*/
			MEMCPY(key_param.p_aes_iv_info->even_iv,pst_hdl_dsc->iv_update_buffer,16);
			ret = aes_ioctl ((pst_hdl_dsc->p_dev_aes) ,IO_KEY_INFO_UPDATE_CMD , (unsigned long)&key_param);
		}		
		else if((AUI_DSC_ALGO_TDES==pst_hdl_dsc->attr_dsc.uc_algo)||
				(AUI_DSC_ALGO_DES==pst_hdl_dsc->attr_dsc.uc_algo)){
			MEMSET(&st_des_iv,0,sizeof(DES_IV_INFO));	
			key_param.p_des_iv_info= &st_des_iv;
			/*only need to update even iv*/
			MEMCPY(key_param.p_des_iv_info->even_iv,pst_hdl_dsc->iv_update_buffer,8);
			ret = des_ioctl ((pst_hdl_dsc->p_dev_des) ,IO_KEY_INFO_UPDATE_CMD , (unsigned long)&key_param);
		}
		else
		{
			aui_rtn(AUI_RTN_FAIL,"algorithm config error!");
		}			

		if(ret != RET_SUCCESS)
		{
			aui_rtn(AUI_RTN_FAIL,"failed");
		}
	}	
	return RET_SUCCESS;	
}



#define QUANTUM_SIZE	(47*1024)
extern UINT8 *dmx_main2see_buf_req(UINT32 req_len, UINT32 *ret_len);
extern INT32 dmx_main2see_buf_ret(UINT32 ret_len);
/*
 * send encrypted data to pvr see and decrypt the data sending to dmx
 */
AUI_RTN_CODE aui_encrypted_stream_inject_to_dmx(
	aui_hdl handle, UINT8 *ts_buf, int buf_len)
{
	aui_handle_dsc *pst_hdl_dsc_de=(aui_handle_dsc *)handle;
	PVR_RPC_RAW_DECRYPT de_input;
	MEMSET(&de_input,0,sizeof(de_input));
	if (!pst_hdl_dsc_de)
		aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->attr_dsc.uc_algo){
		de_input.algo = AES;
		de_input.dev = pst_hdl_dsc_de->p_dev_aes;
	}else if(AUI_DSC_ALGO_TDES	== pst_hdl_dsc_de->attr_dsc.uc_algo){
		de_input.algo = TDES;
		de_input.dev = pst_hdl_dsc_de->p_dev_des;
	}else{
		aui_rtn(AUI_RTN_FAIL,"algorithm config error!");
	}
	de_input.stream_id= pst_hdl_dsc_de->ul_stream_id;
	AUI_DBG("ts_buf: %p,buf_len: 0x%08x\n",ts_buf,buf_len);
#if 1
	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->attr_dsc.uc_algo)
	{
		pvr_block_aes_decrypt(de_input.dev, de_input.stream_id,ts_buf, buf_len);
	}
	else if(AUI_DSC_ALGO_TDES == pst_hdl_dsc_de->attr_dsc.uc_algo)
	{
		pvr_block_des_decrypt(de_input.dev, de_input.stream_id,ts_buf, buf_len);
	}
	return 0;
#else
	/*should not call this dmx_main2see_buf_req. 
		dmx_main2see_buf_req is already called in the outside of aui_encrypted_stream_inject_to_dmx
		call flow, in crypt_blk_inj_task,
		1. dmx_main2see_buf_req
		2. decrypted_data_fill --> pvr xxxx -> bc_pvr_block_decrypt --> BC_DVRDecrypt --> vmxlib -> VMXSEC_R2R_Run -> bc_run_r2r_engine -> aui_encrypted_stream_inject_to_dmx
		3. dmx_main2see_buf_ret
		*/
    int i;
	for (i = 0;i < buf_len;){
#if 0
#ifdef DUAL_ENABLE		
		buf = dmx_main2see_buf_req(QUANTUM_SIZE, &got_len);
#endif
#endif
		de_input.input = ts_buf + i;
		de_input.length = ((buf_len - i) > QUANTUM_SIZE) ? QUANTUM_SIZE : (buf_len - i);
		(void)de_input;
		if(pvr_rpc_ioctl(PVR_RPC_IO_RAW_DECRYPT,(UINT32)&de_input)){
			osal_task_sleep(10);
			continue;
		}	
		i += de_input.length;
		if( i%(20*QUANTUM_SIZE) == 0){
			AUI_DSC_DEBUG("send data to pvr success!data counts: %d bytes\n",i);
			AUI_DSC_DEBUG("de_input.algo: %d,de_input.dev: %p,de_input.input: %p,de_input.length: 0x%08x,de_input.stream_id: %d\n",
				de_input.algo,de_input.dev,de_input.input,de_input.length,de_input.stream_id);
		}
#ifdef DUAL_ENABLE	
		dmx_main2see_buf_ret(de_input.length);
#endif
		osal_task_sleep(5);
	}
	AUI_DSC_DEBUG("send data to pvr success!data counts: %d bytes\n",i);
	return 0;
#endif
}











#define BLOCK_VOB_BUFFER_SIZE	0
static unsigned long dsc_pvr_block_size =QUANTUM_SIZE ;//AUI_DSC_PVR_BLOCK_SIZE_DEFAULT;
extern INT32 dmx_main2see_buf_valid_size_set(UINT32 );
extern INT32 dmx_main2see_src_set(enum DMX_MAIN2SEE_SRC );

/*
 * send encrypted data to pvr see and decrypt the data sending to dmx
 */
AUI_RTN_CODE aui_encrypted_stream_inject_to_dmx_temp(
	aui_hdl handle, UINT8 *ts_buf, int buf_len)
{

	aui_handle_dsc *pst_hdl_dsc_de=(aui_handle_dsc *)handle;
	PVR_RPC_RAW_DECRYPT de_input;
	MEMSET(&de_input,0,sizeof(de_input));
	if (!pst_hdl_dsc_de)
		aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->attr_dsc.uc_algo){
		de_input.algo = AES;
		de_input.dev = pst_hdl_dsc_de->p_dev_aes;
	}else if(AUI_DSC_ALGO_TDES	== pst_hdl_dsc_de->attr_dsc.uc_algo){
		de_input.algo = TDES;
		de_input.dev = pst_hdl_dsc_de->p_dev_des;
	}else{
		aui_rtn(AUI_RTN_FAIL,"algorithm config error!");
	}
	de_input.stream_id= pst_hdl_dsc_de->ul_stream_id;
	AUI_DBG("ts_buf: %p,buf_len: 0x%08x\n",ts_buf,buf_len);
#if 0
	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->attr_dsc.uc_algo)
	{
		pvr_block_aes_decrypt(de_input.dev, de_input.stream_id,ts_buf, buf_len);
	}
	else if(AUI_DSC_ALGO_TDES == pst_hdl_dsc_de->attr_dsc.uc_algo)
	{
		pvr_block_des_decrypt(de_input.dev, de_input.stream_id,ts_buf, buf_len);
	}
	return 0;
#else

	if(0==pst_hdl_dsc_de->dmx_src_mode_flag){
		
		dmx_main2see_src_set(DMX_MAIN2SEE_SRC_CRYPT_BLK);//set see running in cryption playback mode.
		dmx_main2see_buf_valid_size_set((dsc_pvr_block_size+BLOCK_VOB_BUFFER_SIZE)*4);//malloc a specified buffer in see.
		pst_hdl_dsc_de->dmx_src_mode_flag=1;
	}

    int i;
	for (i = 0;i < buf_len;){
#if 0
#ifdef DUAL_ENABLE		
		
		buf = dmx_main2see_buf_req(QUANTUM_SIZE, &got_len);	//this function is called by crypt_blk_inj_task
#endif
#endif
		de_input.input = ts_buf + i;
		de_input.length = ((buf_len - i) > QUANTUM_SIZE) ? QUANTUM_SIZE : (buf_len - i);
		(void)de_input;

		/*instead of pvr_rpc_ioctl(PVR_RPC_IO_RAW_DECRYPT,(UINT32*)&de_input), func pvr_block_aes_decrypt and 
		pvr_block_des_decrypt make cache flushed before call pvr_rpc_ioctl directly*/
		if((AUI_DSC_WORK_MODE_IS_ECB!=pst_hdl_dsc_de->attr_dsc.uc_mode)&&
			(AUI_DSC_DATA_PURE==pst_hdl_dsc_de->attr_dsc.dsc_data_type))
			pure_data_update_iv(pst_hdl_dsc_de);
		
		if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->attr_dsc.uc_algo)
		{
			if(pvr_block_aes_decrypt(pst_hdl_dsc_de->p_dev_aes,pst_hdl_dsc_de->ul_stream_id,de_input.input,de_input.length))
			{
				osal_task_sleep(10);
				continue;
			}
		}
		else if(AUI_DSC_ALGO_TDES == pst_hdl_dsc_de->attr_dsc.uc_algo)
		{
			if(pvr_block_des_decrypt(pst_hdl_dsc_de->p_dev_des,pst_hdl_dsc_de->ul_stream_id,de_input.input,de_input.length))
			{
				osal_task_sleep(10);
				continue;
			}
		}
		else
		{
			aui_rtn(AUI_RTN_FAIL,"algorithm config error!");
		}
		
		
		i += de_input.length;
		if( i%(20*QUANTUM_SIZE) == 0){
			AUI_DBG("send data to pvr success!data counts: %d bytes\n",i);
			AUI_DBG("de_input.algo: %d,de_input.dev: %p,de_input.input: %p,de_input.length: 0x%08x,de_input.stream_id: %d\n",
				de_input.algo,de_input.dev,de_input.input,de_input.length,de_input.stream_id);
			
		}
#ifdef DUAL_ENABLE	
		dmx_main2see_buf_ret(de_input.length);//before here call dmx_main2see_buf_req function.
#endif
		osal_task_sleep(5);
	}

	AUI_DBG("send data to pvr success!data counts: %d bytes\n",i);
	return 0;
#endif

}

#else
AUI_RTN_CODE aui_dsc_update_pvr_encrypt_key_info(
	aui_hdl handle,
	aui_attr_dsc *attr,
	struct aui_dsc_encrypt_kl_param *p_kl_attr,
	aui_dsc_process_status *p_encrypt_status)
{
    (void)handle;
    (void)attr;
    (void)p_kl_attr;
    (void)p_encrypt_status;
    return 0;
}
AUI_RTN_CODE aui_dsc_free_block_mode(aui_hdl handle)
{
    (void)handle;
    return 0;
}
AUI_RTN_CODE aui_encrypted_stream_inject_to_dmx(
	aui_hdl handle,	UINT8 *ts_buf, int buf_len)
{
    (void)handle;
    (void)ts_buf;
    (void)buf_len;
    return 0;
}
#endif/* AUI_PVR_REMOTE */
