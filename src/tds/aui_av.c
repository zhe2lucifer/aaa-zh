/**@file
*	 @brief 		AUI av module interface implement
*	 @author		henry.xie
*	 @date			  2013-6-27
*	  @version		   1.0.0
*	 @note			  ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"


#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_snd.h>
#include <aui_av.h>

AUI_MODULE(AV)

/****************************LOCAL MACRO******************************************/
#define MAX_PID_NUM 32

#define AUI_AC3_DES_EXIST					 (1<<13)//0x2000
#define AUI_AAC_DES_EXIST					 (2<<13)//0x4000//LATM_AAC
#define AUI_EAC3_DES_EXIST					  (3<<13)//0x0001//EAC3
#define AUI_ADTS_AAC_DES_EXIST				  (4<<13)//0x8000//ADTS_AAC


/****************************LOCAL TYPE*******************************************/
typedef struct
{
    /** handle of decv */
    aui_hdl pv_hdl_decv;
    /** handle of deca */
    aui_hdl pv_hdl_deca;
    /** handle of dmx */
    aui_hdl pv_hdl_dmx;
    /** handle of snd */
    aui_hdl pv_hdl_snd;
   
    /** attribute of av module */
    aui_attrAV attrAV;
}aui_handleAV,*aui_p_handleAV;
/****************************LOCAL VAR********************************************/
int aui_av_magic_num = 0;

/****************************LOCAL FUNC DECLEAR***********************************/


/****************************MODULE DRV IMPLEMENT*************************************/

AUI_RTN_CODE aui_av_open(aui_attrAV *pst_attr_av,void **ppv_handle_av)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pst_attr_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	*ppv_handle_av = (aui_handleAV *)MALLOC(sizeof(aui_handleAV));

	if(NULL == *ppv_handle_av)
	{
		aui_rtn(AUI_RTN_ENOMEM, NULL);
	}

	MEMSET((aui_handleAV *)(*ppv_handle_av), 0, sizeof(aui_handleAV));
	MEMCPY(&(((aui_handleAV *)(*ppv_handle_av))->attrAV),
											pst_attr_av,
								   sizeof(aui_attrAV));

	((aui_handleAV *)(*ppv_handle_av))->pv_hdl_decv = pst_attr_av->pv_hdl_decv;
	((aui_handleAV *)(*ppv_handle_av))->pv_hdl_deca = pst_attr_av->pv_hdl_deca;
	((aui_handleAV *)(*ppv_handle_av))->pv_hdl_dmx = pst_attr_av->pv_hdl_dmx;
	((aui_handleAV *)(*ppv_handle_av))->pv_hdl_snd = pst_attr_av->pv_hdl_snd;

	return rtn_code;
}

//used by aui_pvr.c
void av_get_audio_pid(enum aui_deca_stream_type type, unsigned short pid,unsigned short *new_pid)
{
	unsigned short aud_pid = pid;

	switch(type)
	{
		case AUI_DECA_STREAM_TYPE_AC3:
			aud_pid |= AUI_AC3_DES_EXIST;
			break;

		case AUI_DECA_STREAM_TYPE_AAC_LATM:
			aud_pid |= AUI_AAC_DES_EXIST;
			break;

		case AUI_DECA_STREAM_TYPE_AAC_ADTS:
			aud_pid |= AUI_ADTS_AAC_DES_EXIST;
			break;

		case AUI_DECA_STREAM_TYPE_EC3:
			aud_pid |= AUI_EAC3_DES_EXIST;
			break;
		default :
			break;
	}

	if(new_pid != NULL)
	{
		*new_pid = aud_pid;
	}
}

unsigned char cc_av_act(void *pv_handle_av, av_info *info)
{
	unsigned short pid_list[MAX_PID_NUM];
	enum aui_deca_stream_type type = info->en_audio_stream_type;
	enum aui_snd_data_type spdif_type = info->en_spdif_type;
	RET_CODE _ret = RET_SUCCESS;
	unsigned long k=0;
	unsigned char have_audio = info->b_audio_enable;
	unsigned char have_video = info->b_video_enable;
	unsigned char have_dmx = info->b_dmx_enable;
	unsigned char have_pcr = info->b_pcr_enable;
	unsigned char have_subtitle = info->b_sub_enable;

	void *pv_hdl_vdec = ((aui_handleAV *)(pv_handle_av))->pv_hdl_decv;
	void *pv_hdl_deca = ((aui_handleAV *)(pv_handle_av))->pv_hdl_deca;
	void *pv_hdl_dmx = ((aui_handleAV *)(pv_handle_av))->pv_hdl_dmx;
	void *pv_hdl_snd = ((aui_handleAV *)(pv_handle_av))->pv_hdl_snd;

	/*
	struct vdec_device *p_video_dev = ((aui_handleAV *)(pv_handleAV))->pst_dev_decv;
	struct deca_device *p_audio_dev = ((aui_handleAV *)(pv_handleAV))->pst_dev_deca;
	struct dmx_device *p_dmx_dev = ((aui_handleAV *)(pv_handleAV))->pst_dev_dmx;
	struct snd_device *p_snd_dev = ((aui_handleAV *)(pv_handleAV))->pst_dev_snd;
	*/

	unsigned int create_stream = 0, enable_stream = 0;

	if(info->b_modify)
	{
		if(have_audio && pv_hdl_deca)
		{
			aui_deca_stop(pv_hdl_deca, NULL);
		}
		if(have_video && pv_hdl_vdec)
		{
			aui_decv_stop(pv_hdl_vdec);
		}
		if(have_dmx && pv_hdl_dmx)
		{
			aui_dmx_stop(pv_hdl_dmx,NULL);
			aui_dmx_set(((aui_hdl)pv_hdl_dmx), AUI_DMX_STREAM_DISABLE,NULL);
		}
	}

	pid_list[0] = pid_list[1] = pid_list[2] = pid_list[3] = pid_list[4] = AUI_INVALID_PID;

	if(have_audio && pv_hdl_deca)
	{
		//dmx will add pid mask
		pid_list[1] = info->ui_audio_pid;
		aui_deca_set(pv_hdl_deca, AUI_DECA_DEOCDER_TYPE_SET, &type);
		if (pv_hdl_snd)
			aui_snd_set(pv_hdl_snd, AUI_SND_SPIDF_BYPASS_SET, &spdif_type);
		aui_deca_start(pv_hdl_deca, NULL);	
	}

	if(have_video && pv_hdl_vdec)
	{
		//dmx will add pid mask
		pid_list[0] = info->ui_video_pid;
		aui_decv_decode_format_set(pv_hdl_vdec, info->en_video_stream_type);
		aui_decv_start(pv_hdl_vdec);
	}

	if(have_pcr)
	{
		pid_list[2] = info->ui_pcr_pid;
	}
	//pid_list[3] is for AUDIO_DESC, not support here
	//if(have_ttx)
	//{
	//	pid_list[3] = info->ui_ttx_pid;
	//}
	if(have_subtitle)
	{
		pid_list[4] = info->ui_sub_pid;
	}
		
	if(have_dmx && pv_hdl_dmx)
	{
		struct aui_dmx_stream_pid pids_info;
		pids_info.ul_pid_cnt=5;
		for(k=0;k<pids_info.ul_pid_cnt;k++)
		{
			pids_info.aus_pids_val[k]=pid_list[k];
		}
		pids_info.stream_types[0] = AUI_DMX_STREAM_VIDEO;
		pids_info.stream_types[1] = AUI_DMX_STREAM_AUDIO;
		pids_info.stream_types[2] = AUI_DMX_STREAM_PCR;
		pids_info.stream_types[3] = AUI_DMX_STREAM_AUDIO_DESC;
		pids_info.stream_types[4] = AUI_DMX_STREAM_SUBTITLE;
		pids_info.ul_magic_num = aui_av_magic_num;
		create_stream = have_audio ? (have_video ? AUI_DMX_STREAM_CREATE_AV : AUI_DMX_STREAM_CREATE_AUDIO) : (have_video ? AUI_DMX_STREAM_CREATE_VIDEO: 0);
		_ret = aui_dmx_set(((aui_hdl)pv_hdl_dmx), create_stream, (void *)&pids_info);
		if(_ret != AUI_RTN_SUCCESS)
		{
			goto RETURN;
		}
		enable_stream = have_audio ? (have_video ? AUI_DMX_STREAM_ENABLE : AUI_DMX_STREAM_ENABLE_AUDIO) : (have_video ? AUI_DMX_STREAM_ENABLE_VIDEO : 0);
		_ret = aui_dmx_set(((aui_hdl)pv_hdl_dmx), enable_stream, (void *)NULL);
		if(_ret != AUI_RTN_SUCCESS)
		{
			goto RETURN;
		}
		aui_dmx_start(pv_hdl_dmx, NULL);
	}

RETURN:

	return _ret;
}


AUI_RTN_CODE aui_av_close(void *pv_handle_av)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_handle_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	FREE(pv_handle_av);

	return rtn_code;
}

AUI_RTN_CODE aui_av_start(void *pv_handle_av)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_handle_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	av_info *p_info = &((((aui_handleAV *)(pv_handle_av))->attrAV).st_av_info);
	if(RET_SUCCESS != cc_av_act(pv_handle_av,p_info))
	{
		aui_rtn(AUI_RTN_EINVAL, "\nstart fail in av start\n");
	}
	return rtn_code;
}


AUI_RTN_CODE aui_av_stop(void *pv_handle_av)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_handle_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	void *pv_hdl_vdec = ((aui_handleAV *)(pv_handle_av))->pv_hdl_decv;
	void *pv_hdl_deca = ((aui_handleAV *)(pv_handle_av))->pv_hdl_deca;
	void *pv_hdl_dmx = ((aui_handleAV *)(pv_handle_av))->pv_hdl_dmx;

	av_info *p_info = &((((aui_handleAV *)(pv_handle_av))->attrAV).st_av_info);

	/* stop audio */
	if(p_info->b_audio_enable)
	{
		if (AUI_RTN_SUCCESS!=aui_deca_stop(pv_hdl_deca, NULL))
		{
			aui_rtn(AUI_RTN_EINVAL, "\nstop audio fail in av close\n");
		}
	}

	if(p_info->b_video_enable)
	{
		if (RET_SUCCESS!=aui_decv_stop(pv_hdl_vdec))
		{
			aui_rtn(AUI_RTN_EINVAL, "\nstop video fail in av close\n");
		}
	}

	if(p_info->b_dmx_enable)
	{
		aui_dmx_set(((aui_hdl)pv_hdl_dmx), AUI_DMX_STREAM_DISABLE, NULL);
	}
	return rtn_code;
}

AUI_RTN_CODE aui_av_pause(void *pv_handle_av)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_handle_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	void *pv_hdl_vdec = ((aui_handleAV *)(pv_handle_av))->pv_hdl_decv;
	void *pv_hdl_deca = ((aui_handleAV *)(pv_handle_av))->pv_hdl_deca;


	/* stop audio */
	if (AUI_RTN_SUCCESS!=aui_deca_stop(pv_hdl_deca, NULL))
	{
		aui_rtn(AUI_RTN_EINVAL, "\nstop audio fail in av pause\n");
	}

	if (RET_SUCCESS!=aui_decv_stop(pv_hdl_vdec))
	{
		aui_rtn(AUI_RTN_EINVAL, "\nstop video fail in av pause\n");
	}
	return rtn_code;
}



AUI_RTN_CODE aui_av_resume(void *pv_handle_av)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_handle_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	
	void *pv_hdl_vdec = ((aui_handleAV *)(pv_handle_av))->pv_hdl_decv;
	void *pv_hdl_deca = ((aui_handleAV *)(pv_handle_av))->pv_hdl_deca;


	/* stop audio */
	if (AUI_RTN_SUCCESS!=aui_deca_start(pv_hdl_deca, NULL))
	{
		aui_rtn(AUI_RTN_EINVAL, "\nstop audio fail in av start\n");
	}

	if (RET_SUCCESS!=aui_decv_start(pv_hdl_vdec))
	{
		aui_rtn(AUI_RTN_EINVAL, "\nstop video fail in av start\n");
	}
	return rtn_code;
}



AUI_RTN_CODE aui_av_init(aui_funcAVInit fn_avinit)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	if(NULL != fn_avinit)
	{
		fn_avinit();
	}
	return rtn_code;
}


AUI_RTN_CODE aui_av_de_init(aui_funcAVInit fn_avde_init)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	if(NULL == fn_avde_init)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	return rtn_code;
}

AUI_RTN_CODE aui_av_set(void *pv_hdl_av,aui_av_item_set ul_item, void *pv_param)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_hdl_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	av_info *p_info = &((((aui_handleAV *)(pv_hdl_av))->attrAV).st_av_info);
	switch(ul_item)
	{
		case AUI_AV_VIDEO_PID_SET:
			p_info->b_video_enable = 1;
			p_info->ui_video_pid = *((unsigned short *)pv_param);
			break;

		case AUI_AV_AUDIO_PID_SET:
			p_info->b_audio_enable = 1;
			p_info->ui_audio_pid = *((unsigned short *)pv_param);
			break;

		case AUI_AV_PCR_PID_SET:
			p_info->b_pcr_enable = 1;
			p_info->ui_pcr_pid = *((unsigned short *)pv_param);
			break;

		case AUI_AV_TTX_PID_SET:
			p_info->b_ttx_enable = 1;
			p_info->ui_ttx_pid = *((unsigned short *)pv_param);
			break;

		case AUI_AV_SUB_PID_SET:
			p_info->b_sub_enable = 1;
			p_info->ui_sub_pid = *((unsigned short *)pv_param);
			break;

		case AUI_AV_VIDEO_TYPE_SET:
			p_info->en_video_stream_type = *((unsigned int *)pv_param);
			break;

		case AUI_AV_AUDIO_TYPE_SET:
			p_info->en_audio_stream_type = *((unsigned int *)pv_param);
			break;

		case AUI_AV_SPDIF_TYPE_SET:
			p_info->en_spdif_type = *((unsigned int *)pv_param);
			break;

		case AUI_AV_MODIFY_SET:
			p_info->b_modify = *((unsigned int *)pv_param);
			break;

		case AUI_AV_DMX_ENABLE:
			p_info->b_dmx_enable = *((unsigned int *)pv_param);
			break;

		case AUI_AV_VIDEO_ENABLE:
			p_info->b_video_enable = *((unsigned int *)pv_param);
			break;

		case AUI_AV_AUDIO_ENABLE:
			p_info->b_audio_enable = *((unsigned int *)pv_param);
			break;

		case AUI_AV_PCR_ENABLE:
			p_info->b_pcr_enable = *((unsigned int *)pv_param);
			break;

		case AUI_AV_TTX_ENABLE:
			p_info->b_ttx_enable = *((unsigned int *)pv_param);
			break;

		case AUI_AV_SUB_ENABLE:
			p_info->b_sub_enable = *((unsigned int *)pv_param);
			break;
		default:
			break;
	}
	return rtn_code;
}



AUI_RTN_CODE aui_av_get(void *pv_hdl_av, aui_av_item_get ul_item, void *pv_param)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(NULL == pv_hdl_av)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	av_info *p_info = &((((aui_handleAV *)(pv_hdl_av))->attrAV).st_av_info);
	switch(ul_item)
	{
		case AUI_AV_VIDEO_PID_GET:
			if(p_info->b_video_enable)
			{
				*((unsigned short *)pv_param) = p_info->ui_video_pid;
			}
			else
			{
				aui_rtn(AUI_RTN_EINVAL, "\nvideo disable,can't get pid\n");
			}
			break;
  
		case AUI_AV_AUDIO_PID_GET:
			if(p_info->b_audio_enable)
			{
				*((unsigned short *)pv_param) = p_info->ui_audio_pid;
			}
			else
			{
				aui_rtn(AUI_RTN_EINVAL, "\naudio disable,can't get pid\n");
			}
			break;

		case AUI_AV_PCR_PID_GET:
			if(p_info->b_pcr_enable)
			{
				*((unsigned short *)pv_param) = p_info->ui_pcr_pid;
			}
			else
			{
				aui_rtn(AUI_RTN_EINVAL, "\npcr disable,can't get pid\n");
			}
			break;

		case AUI_AV_TTX_PID_GET:
			if(p_info->b_ttx_enable)
			{
				*((unsigned short *)pv_param) = p_info->ui_ttx_pid;
			}
			else
			{
				aui_rtn(AUI_RTN_EINVAL, "\nttx disable,can't get pid\n");
			}
			break;

		case AUI_AV_SUB_PID_GET:
			if(p_info->b_sub_enable)
			{
				*((unsigned short *)pv_param) = p_info->ui_sub_pid;
			}
			else
			{
				aui_rtn(AUI_RTN_EINVAL, "subtitle disable,can't get pid\n");
			}
			break;

		case AUI_AV_VIDEO_TYPE_GET:
			*((unsigned int *)pv_param) = p_info->en_video_stream_type;
			break;

		case AUI_AV_AUDIO_TYPE_GET:
			*((unsigned int *)pv_param) = p_info->en_audio_stream_type ;
			break;

		case AUI_AV_SPDIF_TYPE_GET:
			*((unsigned int *)pv_param) = p_info->en_spdif_type;
			break;

		case AUI_AV_MODIFY_GET:
			*((unsigned int *)pv_param) = p_info->b_modify;
			break;

		default:
			break;
	}
	return rtn_code;

}
