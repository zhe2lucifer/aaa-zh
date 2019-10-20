/**@file
*	 @brief 		AUI diss module interface implement
*	 @author		henry.xie
*	 @date			  2013-6-27
*	  @version		   1.0.0
*	 @note			  ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"

#include <aui_dis.h>
#include <hld/decv/decv.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>
#include <aui_av.h>
#include <hld/nim/nim.h>
#if defined(SUPPORT_MPEG4_TEST)
#include <api/libmp/pe.h>
#endif 
#include <osal/osal.h>

AUI_MODULE(DIS)

/****************************LOCAL MACRO******************************************/
#define MAX_YUV_NUM 2
#define MAX_RGB_NUM 2
#define MAX_SVIDEO_NUM 3
#define MAX_CVBS_NUM 6

#define VE_FIRST_SHOW_TIME     2000 // ms
#define MODE_SWITCH_TIME 8000

#define AUI_LEFT_MARGIN 	32
#ifdef _S3281_
#define AUI_TOP_MARGIN		48
#else
#define AUI_TOP_MARGIN		96
#endif

#define AUI_DIS_ENHANCE_SHARPNESS_MAX 10 //max value of sharpness is 10, cannot be changed
/* default max value of enhance is 100 except sharpness, 
   the max value can be changed by function aui_dis_enhance_max_value_set*/
#define AUI_DIS_ENHANCE_MAX_DEFAULT 100

/****************************LOCAL TYPE*******************************************/

/**
	Enum to specify the afd mode, the afd mode will effect the match mode of the video
*/
typedef enum aui_dis_afd_mode{
	/**
	  AFD is implemented by the co-work of DIS and TV set,
	  DIS will output the video frame with WSS(Wide Screen Signalling) signal 
	  defined in spec which specified by @b afd_spec in #aui_st_dis_afd_attr
	  and the TV supported AFD will show the video frame with match mode
	*/
	AUI_DIS_AFD_MODE_SPEC_WSS,
	/**
	  AFD is implemented by DIS scaling,
	  DIS will output the video frame match mode with afd scaling defined in spec
	  which specified by @b afd_spec in #aui_st_dis_afd_attr
	*/
	AUI_DIS_AFD_MODE_SPEC_SCALE,
	/**
	  AFD is implemented by DIS scaling,
	  DIS will output the video frame match mode as user expected without afd scaling
	*/
	AUI_DIS_AFD_MODE_USER_SCALE
}aui_dis_afd_mode;

typedef struct
{
	unsigned long y;
	unsigned long u;
	unsigned long v;
}temp_yuv;

typedef struct
{
	unsigned long r;
	unsigned long g;
	unsigned long b;
}temp_rgb;

typedef struct
{
	unsigned long sy;
	unsigned long sc;
}temp_svideo;

typedef struct
{
	aui_dev_priv_data dev_priv_data;
	/** handle device */
	struct vpo_device *pst_dev_dis;
	/** mutex syn in module */
	OSAL_ID dev_mutex_id;
	/** dis attr */
	aui_attr_dis attr_dis;
    /* the max value of video enhancement*/
	unsigned int video_enhance_max;	
}aui_handle_dis,*aui_p_handle_dis;



static aui_dis_zoom_rect scaled_src;
static aui_dis_zoom_rect scaled_dst;
/****************************LOCAL VAR********************************************/
static OSAL_ID s_mod_mutex_id_dis = OSAL_INVALID_ID;
static OSAL_ID g_mutex_scale = OSAL_INVALID_ID;
aui_view_mode aui_dis_mode = AUI_VIEW_MODE_FULL;
static int cgms_last = 0;
static int aps_last = 0;
/****************************LOCAL FUNC DECLEAR***********************************/

static AUI_RTN_CODE aui_dis_set_YPbPr_resolution_same_as_CVBS();
static AUI_RTN_CODE aui_dis_set_YPbPr_resolution_same_as_HDMI();

/****************************MODULE DRV IMPLEMENT*************************************/



AUI_RTN_CODE aui_dis_init(aui_func_dis_init fn_dis_init)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	
	s_mod_mutex_id_dis=osal_mutex_create();
	if(OSAL_INVALID_ID==s_mod_mutex_id_dis)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

    g_mutex_scale = osal_mutex_create();
    if(OSAL_INVALID_ID == g_mutex_scale)
    {
		aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    scaled_src.ui_startX = 0;
    scaled_src.ui_startY = 0;
    scaled_src.ui_width = 720;
    scaled_src.ui_height = 2880;
    
    scaled_dst.ui_startX = 0;
    scaled_dst.ui_startY = 0;
    scaled_dst.ui_width = 720;
    scaled_dst.ui_height = 2880;
   
	if(fn_dis_init != NULL)
	{
		fn_dis_init();
	}
	return rtn_code;
}

AUI_RTN_CODE aui_dis_open(aui_attr_dis *pst_attr_dis, void **ppv_hdldis)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	OSAL_ID dev_mutex_id;
	struct vp_init_info st_vp_init_info;

    
	AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_dis, dev_mutex_id, AUI_MODULE_DIS, AUI_RTN_EINVAL);

	if ((NULL == pst_attr_dis) || (NULL == ppv_hdldis))
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	(*ppv_hdldis) = (aui_handle_dis *)MALLOC(sizeof(aui_handle_dis));

	if(NULL == *ppv_hdldis)
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_ENOMEM, NULL);
	}

	MEMSET((aui_handle_dis *)(*ppv_hdldis), 0, sizeof(aui_handle_dis));
	MEMCPY(&(((aui_handle_dis *)(*ppv_hdldis))->attr_dis),
											pst_attr_dis,
								   sizeof(aui_attr_dis));

	((aui_handle_dis *)(*ppv_hdldis))->pst_dev_dis = (struct vpo_device *)dev_get_by_id (HLD_DEV_TYPE_DIS,
																			pst_attr_dis->uc_dev_idx);

	if(NULL == ((aui_handle_dis *)(*ppv_hdldis))->pst_dev_dis)
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n can't get device while open! \n");
	}

	MEMSET(&st_vp_init_info, 0, sizeof(struct vp_init_info));
	if(((((aui_handle_dis *)(*ppv_hdldis))->attr_dis).get_param) == NULL)
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n can't get param while open! \n");
	}

	((((aui_handle_dis *)(*ppv_hdldis))->attr_dis).get_param)((void *)(&st_vp_init_info));

	if(RET_SUCCESS != vpo_open(((aui_handle_dis *)(*ppv_hdldis))->pst_dev_dis,
						&st_vp_init_info))
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n open vpo fail \n");
	}
	
	(*(aui_handle_dis **)ppv_hdldis)->dev_mutex_id=dev_mutex_id;
	(*(aui_handle_dis **)ppv_hdldis)->dev_priv_data.dev_idx=pst_attr_dis->uc_dev_idx;
	(*(aui_handle_dis **)ppv_hdldis)->video_enhance_max = AUI_DIS_ENHANCE_MAX_DEFAULT;
	
	aui_dev_reg(AUI_MODULE_DIS, *ppv_hdldis);
	
	vpo_ioctl(((aui_handle_dis *)(*ppv_hdldis))->pst_dev_dis,\
                                    VPO_IO_SELECT_SCALE_MODE, TRUE);

	osal_mutex_unlock(dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_dis_dac_reg(void *pv_hdl_dis, unsigned int *pui_dac_attr, unsigned int ui_ary_num)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	unsigned int i;
	struct vp_io_reg_dac_para treg_info[2];
	temp_yuv st_yuv[2];
	temp_rgb st_rgb[2];
	temp_svideo st_svideo[3];
	unsigned long st_cvbs[6];
	unsigned int ty,tu,tv;
	unsigned int tr,tg,tb;
	unsigned int tsy,tsc;
	unsigned int tcvbs;
	unsigned char b_pro;

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	ty = tu = tv = tr = tg = tb = tsy = tsc = tcvbs = 0;
    b_pro = (((aui_handle_dis *)(pv_hdl_dis))->attr_dis).b_dac_progressive;

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	for(i = 0; i < ui_ary_num; i++)
	{
		if(AUI_DIS_TYPE_Y == pui_dac_attr[i])
		{
			st_yuv[ty].y = i;
			if(++ty > MAX_YUV_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_U == pui_dac_attr[i])
		{
			st_yuv[tu].u = i;
			if(++tu > MAX_YUV_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_V == pui_dac_attr[i])
		{
			st_yuv[tv].v = i;
			if(++tv > MAX_YUV_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_R == pui_dac_attr[i])
		{
			st_rgb[tr].r = i;
			if(++tr > MAX_RGB_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_G == pui_dac_attr[i])
		{
			st_rgb[tg].g = i;
			if(++tg > MAX_RGB_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_B == pui_dac_attr[i])
		{
			st_rgb[tb].b = i;
			if(++tb > MAX_RGB_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_SVIDEO_Y == pui_dac_attr[i])
		{
			st_svideo[tsy].sy = i;
			if(++tsy > MAX_SVIDEO_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_SVIDEO_C== pui_dac_attr[i])
		{
			st_svideo[tsc].sc = i;
			if(++tsc > MAX_SVIDEO_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
		else if(AUI_DIS_TYPE_CVBS == pui_dac_attr[i])
		{
			st_cvbs[tcvbs] = i;
			if(++tcvbs > MAX_CVBS_NUM - 1)
			{
				aui_rtn(AUI_RTN_EINVAL, "dac register error!");
			}
		}
	}

	if( (ty != tv) || (tu != tv))
	{
		aui_rtn(AUI_RTN_EINVAL, "dac register error!");
	}

	if( (tr != tb) || (tg != tb))
	{
		aui_rtn(AUI_RTN_EINVAL, "dac register error!");
	}

	if(tsc != tsy)
	{
		aui_rtn(AUI_RTN_EINVAL, "dac register error!");
	}

	if(ty!=0)
	{
		for(i = 1;i <= ty; i++)
		{
			if(i == 1)
			{
				treg_info[i].e_dac_type = YUV_1;
			}
			else
			{
				treg_info[i].e_dac_type = YUV_2;
			}
			treg_info[i].dac_info.b_enable = TRUE;
			treg_info[i].dac_info.e_vgamode = VGA_NOT_USE;
			treg_info[i].dac_info.b_progressive = b_pro;

			treg_info[i].dac_info.t_dac_index.u_dac_first = 1<<(st_yuv[i - 1].y);
			treg_info[i].dac_info.t_dac_index.u_dac_second = 1<<(st_yuv[i - 1].u);
			treg_info[i].dac_info.t_dac_index.u_dac_third = 1<<(st_yuv[i - 1].v);
			vpo_ioctl(p_vpo_dev, VPO_IO_REG_DAC, (unsigned int)(&treg_info[i]));
			osal_task_sleep(50);
		}
	}

	if(tr!=0)
	{
		for(i = 1;i <= tr; i++)
		{
			if(i == 1)
			{
				treg_info[i].e_dac_type = RGB_1;
			}
			else
			{
				treg_info[i].e_dac_type = RGB_2;
			}
			treg_info[i].dac_info.b_enable = TRUE;
			treg_info[i].dac_info.e_vgamode = VGA_NOT_USE;
			treg_info[i].dac_info.b_progressive = b_pro;

			treg_info[i].dac_info.t_dac_index.u_dac_first = 1<<(st_rgb[i - 1].r);
			treg_info[i].dac_info.t_dac_index.u_dac_second = 1<<(st_rgb[i - 1].g);
			treg_info[i].dac_info.t_dac_index.u_dac_third = 1<<(st_rgb[i - 1].b);
			vpo_ioctl(p_vpo_dev, VPO_IO_REG_DAC, (unsigned int)(&treg_info[i]));
			osal_task_sleep(50);
		}
	}

    if(tsy != 0)
    {
    	for(i = 1;i <= tsy; i++)
    	{
    		if(i == 1)
    		{
    			treg_info[i].e_dac_type = SVIDEO_1;
    		}
    		else if(i == 2)
    		{
    			treg_info[i].e_dac_type = SVIDEO_2;
    		}
    		else
    		{
    			treg_info[i].e_dac_type = SVIDEO_3;
    		}

			treg_info[i].dac_info.b_enable = TRUE;
			treg_info[i].dac_info.e_vgamode = VGA_NOT_USE;
			treg_info[i].dac_info.b_progressive = b_pro;

			treg_info[i].dac_info.t_dac_index.u_dac_first = 1<<(st_svideo[i - 1].sy);
			treg_info[i].dac_info.t_dac_index.u_dac_second = 1<<(st_svideo[i - 1].sc);
			vpo_ioctl(p_vpo_dev, VPO_IO_REG_DAC, (unsigned int)(&treg_info[i]));
			osal_task_sleep(50);
		}
	}

	if(tcvbs!=0)
	{
		for(i = 1;( i <= tcvbs) && (i < 2); i++)    /* i must be less than or equal to 1, because treg_info define treg_info[2] */
		{
			treg_info[i].e_dac_type = CVBS_1 + i - 1;

			treg_info[i].dac_info.b_enable = TRUE;
			treg_info[i].dac_info.e_vgamode = VGA_NOT_USE;
			treg_info[i].dac_info.b_progressive = b_pro;

			treg_info[i].dac_info.t_dac_index.u_dac_first = 1<<(st_cvbs[i - 1]);

			vpo_ioctl(p_vpo_dev, VPO_IO_REG_DAC, (unsigned int)(&treg_info[i]));
			osal_task_sleep(50);
		}
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_dis_enhance_set(void *pv_hdl_dis, enum aui_dis_video_enhance en_eh_type, unsigned int ui_value)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    struct vpo_io_video_enhance st_ve_param;
    aui_handle_dis *p_dis_dev = (aui_handle_dis *)(pv_hdl_dis);

	if ((NULL == pv_hdl_dis) || ( (AUI_DIS_ENHANCE_SHARPNESS == en_eh_type) && (ui_value > 10) ))
		aui_rtn(AUI_RTN_EINVAL,NULL);

    if (s_mod_mutex_id_dis == OSAL_INVALID_ID) {
        aui_rtn(AUI_RTN_EINVAL, "Display not init!");
    }

    osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);


    if (((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID) {
        osal_mutex_unlock(s_mod_mutex_id_dis);
        aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
    }

    osal_mutex_lock(p_dis_dev->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dis);
    
    struct vpo_device *p_vpo_dev = p_dis_dev->pst_dev_dis;

    if ((ui_value > p_dis_dev->video_enhance_max)
            || ( (AUI_DIS_ENHANCE_SHARPNESS == en_eh_type) && (ui_value > AUI_DIS_ENHANCE_SHARPNESS_MAX))) {
        osal_mutex_unlock(p_dis_dev->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    MEMSET(&st_ve_param, 0, sizeof(st_ve_param));

    switch (en_eh_type) {
        case AUI_DIS_ENHANCE_BRIGHTNESS:
            st_ve_param.changed_flag = VPO_IO_SET_ENHANCE_BRIGHTNESS;
            st_ve_param.grade = ui_value;
            st_ve_param.grade_range_max = p_dis_dev->video_enhance_max;
            st_ve_param.grade_range_min = 0;
            break;
        case AUI_DIS_ENHANCE_CONTRAST:
            st_ve_param.changed_flag = VPO_IO_SET_ENHANCE_CONTRAST;
            st_ve_param.grade = ui_value;
            st_ve_param.grade_range_max = p_dis_dev->video_enhance_max;
            st_ve_param.grade_range_min = 0;
            break;
        case AUI_DIS_ENHANCE_SATURATION:
            st_ve_param.changed_flag = VPO_IO_SET_ENHANCE_SATURATION;
            st_ve_param.grade = ui_value;
            st_ve_param.grade_range_max = p_dis_dev->video_enhance_max;
            st_ve_param.grade_range_min = 0;
            break;
        case AUI_DIS_ENHANCE_SHARPNESS:
            st_ve_param.changed_flag = VPO_IO_SET_ENHANCE_SHARPNESS;
            st_ve_param.grade = ui_value;
            st_ve_param.grade_range_max = AUI_DIS_ENHANCE_SHARPNESS_MAX;
            st_ve_param.grade_range_min = 0;
            break;
        case AUI_DIS_ENHANCE_HUE:
            st_ve_param.changed_flag = VPO_IO_SET_ENHANCE_HUE;
            st_ve_param.grade = ui_value;
            st_ve_param.grade_range_max = p_dis_dev->video_enhance_max;
            st_ve_param.grade_range_min = 0;
            break;

        default:
            osal_mutex_unlock(p_dis_dev->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL, NULL);
            break;
    }

    if (RET_SUCCESS != vpo_ioctl(p_vpo_dev, VPO_IO_VIDEO_ENHANCE, (unsigned int)&st_ve_param)) {
        osal_mutex_unlock(p_dis_dev->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, "\n set enhance fail! \n");
    }
    
    osal_mutex_unlock(p_dis_dev->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_dis_zoom(void *pv_hdl_dis, aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	struct rect st_src_rect;
	struct rect st_dst_rect;

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	/* validate input args */
	if ((NULL == pv_hdl_dis)       ||
		(NULL == pst_src_rect)     ||
		(NULL == pst_dst_rect)     ||
		(pst_src_rect->ui_width == 0)  ||
		(pst_src_rect->ui_height == 0) ||
		(pst_dst_rect->ui_width == 0)  ||
		(pst_dst_rect->ui_height == 0)) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);


	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	st_src_rect.u_start_x = pst_src_rect->ui_startX;
	st_src_rect.u_start_y = pst_src_rect->ui_startY;
	st_src_rect.u_width = pst_src_rect->ui_width;
	st_src_rect.u_height = pst_src_rect->ui_height;

	st_dst_rect.u_start_x = pst_dst_rect->ui_startX;
	st_dst_rect.u_start_y = pst_dst_rect->ui_startY;
	st_dst_rect.u_width = pst_dst_rect->ui_width;
	st_dst_rect.u_height = pst_dst_rect->ui_height;

	if(RET_SUCCESS != vpo_zoom(p_vpo_dev, &st_src_rect, &st_dst_rect))
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_zoom fail! \n");
	}

    osal_mutex_lock(g_mutex_scale,OSAL_WAIT_FOREVER_TIME);
	scaled_src.ui_startX = pst_src_rect->ui_startX;
	scaled_src.ui_startY = pst_src_rect->ui_startY;
	scaled_src.ui_width = pst_src_rect->ui_width;
	scaled_src.ui_height = pst_src_rect->ui_height;

	scaled_dst.ui_startX = pst_dst_rect->ui_startX;
	scaled_dst.ui_startY = pst_dst_rect->ui_startY;
	scaled_dst.ui_width = pst_dst_rect->ui_width;
	scaled_dst.ui_height = pst_dst_rect->ui_height;
    osal_mutex_unlock(g_mutex_scale);
    
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_dis_zoom_ext(void *pv_hdl_dis, aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect, int layer)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	struct rect st_src_rect;
	struct rect st_dst_rect;

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);


	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);


	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	st_src_rect.u_start_x = pst_src_rect->ui_startX;
	st_src_rect.u_start_y = pst_src_rect->ui_startY;
	st_src_rect.u_width = pst_src_rect->ui_width;
	st_src_rect.u_height = pst_src_rect->ui_height;

	st_dst_rect.u_start_x = pst_dst_rect->ui_startX;
	st_dst_rect.u_start_y = pst_dst_rect->ui_startY;
	st_dst_rect.u_width = pst_dst_rect->ui_width;
	st_dst_rect.u_height = pst_dst_rect->ui_height;

	if( (st_src_rect.u_width == 0) || 
            (st_src_rect.u_height == 0) || 
            (st_dst_rect.u_width == 0)  || 
            (st_dst_rect.u_height == 0))
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

    if(layer == 0)
    {
    	if(RET_SUCCESS != vpo_zoom(p_vpo_dev, &st_src_rect, &st_dst_rect))
    	{
    		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
    		aui_rtn(AUI_RTN_EINVAL, "\n vpo_zoom fail! \n");
    	}

        osal_mutex_lock(g_mutex_scale,OSAL_WAIT_FOREVER_TIME);
    	scaled_src.ui_startX = pst_src_rect->ui_startX;
    	scaled_src.ui_startY = pst_src_rect->ui_startY;
    	scaled_src.ui_width = pst_src_rect->ui_width;
    	scaled_src.ui_height = pst_src_rect->ui_height;

    	scaled_dst.ui_startX = pst_dst_rect->ui_startX;
    	scaled_dst.ui_startY = pst_dst_rect->ui_startY;
    	scaled_dst.ui_width = pst_dst_rect->ui_width;
    	scaled_dst.ui_height = pst_dst_rect->ui_height;
        osal_mutex_unlock(g_mutex_scale);
    }
    else
    {
    	if(RET_SUCCESS != vpo_zoom_ext(p_vpo_dev, &st_src_rect, &st_dst_rect,VPO_LAYER_AUXP))
    	{
    		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
    		aui_rtn(AUI_RTN_EINVAL, "\n vpo_zoom fail! \n");
    	}
    }
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;
}

#ifndef _AUI_OTA_
extern BOOL cc_backup_picture(aui_decv_chg_mode en_chg_mode);
extern void cc_backup_free(void);
#endif

#ifndef _AUI_OTA_
AUI_RTN_CODE aui_dis_fill_black_screen(void *pv_hdl_dis)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if(s_mod_mutex_id_dis == OSAL_INVALID_ID) {
        aui_rtn(AUI_RTN_EINVAL, "Display not init!");
    }

    osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

    if(NULL == pv_hdl_dis) {
        osal_mutex_unlock(s_mod_mutex_id_dis);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID) {
        osal_mutex_unlock(s_mod_mutex_id_dis);
        aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
    }

    osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dis);

    struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

    if(p_vpo_dev) {
#ifdef CHANCHG_VIDEOTYPE_SUPPORT
        /* need use the backup frame memory to backup current frame, 
           otherwise, fill black screen can not avoid mess screen,
           only backup by AUI need this, see do it the same way, 
           just uniform code for backuping in see*/
        cc_backup_free();
        cc_backup_picture(AUI_DECV_CHG_BLACK);
#else
        //If not support CHANCHG_VIDEOTYPE_SUPPORT, just close vpo
        if(RET_SUCCESS != vpo_win_onoff(p_vpo_dev, 0)) {
            rtn_code = AUI_RTN_FAIL;
        }
#endif	
    } else {
        rtn_code = AUI_RTN_FAIL;
    }
    
    osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
    return rtn_code;
}
#endif

AUI_RTN_CODE aui_dis_video_enable(void *pv_hdl_dis, unsigned char b_enable)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if(s_mod_mutex_id_dis == OSAL_INVALID_ID) {
        aui_rtn(AUI_RTN_EINVAL, "Display not init!");
    }

    //use get current info from DE instead
    /*new add begin ,if decv showed nothing.Open vpo may show green screen.  
    if((aui_decv_first_i_showed()==0)&&(b_enable))
    {
    return AUI_RTN_SUCCESS;
    }*/
    //new add end

    osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

    if(NULL == pv_hdl_dis) {
        osal_mutex_unlock(s_mod_mutex_id_dis);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID) {
        osal_mutex_unlock(s_mod_mutex_id_dis);
        aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
    }

    osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dis);

    struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

    if (b_enable) {//use DE current info instead of decv first show
        struct vpo_io_get_picture_info info_src;
        MEMSET(&info_src,0,sizeof(struct vpo_io_get_picture_info));

        if (RET_SUCCESS == vpo_ioctl(p_vpo_dev, VPO_IO_GET_CURRENT_DISPLAY_INFO, (UINT32)&info_src)) {
            if (info_src.status != 2) {//only status = 2 has something show on VPO
                osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
                return AUI_RTN_SUCCESS;
            }
        }
    }
    
    //new add begin  
#if 0//If need inject black frame .Open a new ext interface
    if(aui_decv_first_i_showed()&&(0==b_enable))
    {
    aui_dis_fill_black_screen(p_vpo_dev);
    }
#endif
    //new add end 

    if(RET_SUCCESS != vpo_win_onoff(p_vpo_dev,b_enable)) {
        osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, "\n vpo_win_onoff fail! \n");
    }

    osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_dis_aspect_ratio_set_ext(void *pv_hdl_dis, enum aui_dis_aspect_ratio en_asp_ratio, enum aui_dis_match_mode match_mode)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	enum display_mode en_dis_mode;
	enum tvmode en_tv_mode;
	
	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}
	
	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	if(en_asp_ratio == AUI_DIS_AP_16_9)
		en_tv_mode = TV_16_9;
	else if(en_asp_ratio == AUI_DIS_AP_4_3)
		en_tv_mode = TV_4_3;
	else if(en_asp_ratio == AUI_DIS_AP_AUTO)
		en_tv_mode = TV_AUTO;
	else
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "invalid tv mode!");
	}

	switch(match_mode)
	{
		case AUI_DIS_MM_PANSCAN:
			en_dis_mode = PANSCAN;
			break;
		case AUI_DIS_MM_PANSCAN_NOLINEAR:
			en_dis_mode = PANSCAN_NOLINEAR;
			break;
		case AUI_DIS_MM_LETTERBOX:
			en_dis_mode = LETTERBOX;
			break;
		case AUI_DIS_MM_TWOSPEED:
			en_dis_mode = TWOSPEED;
			break;
		case AUI_DIS_MM_PILLBOX:
			en_dis_mode = PILLBOX;
			break;
		case AUI_DIS_MM_VERTICALCUT:
			en_dis_mode = VERTICALCUT;
			break;
		case AUI_DIS_MM_NORMAL_SCALE:
			en_dis_mode = NORMAL_SCALE;
			break;
		case AUI_DIS_MM_LETTERBOX149:
			en_dis_mode = LETTERBOX149;
			break;
		case AUI_DIS_MM_AFDZOOM:
			en_dis_mode = AFDZOOM;
			break;
		case AUI_DIS_MM_PANSCAN43ON169:
			en_dis_mode = PANSCAN43ON169;
			break;
		case AUI_DIS_MM_COMBINED_SCALE:
			en_dis_mode = COMBINED_SCALE;
			break;
		case AUI_DIS_MM_IGNORE:
			en_dis_mode = DONT_CARE;
			break;
		case AUI_DIS_MM_VERTICALCUT_149:
			en_dis_mode = VERTICALCUT_149;
			break;
		default:
			osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL, "invalid match mode!");
			break;
	}
	   
	if(RET_SUCCESS != vpo_aspect_mode(p_vpo_dev, en_tv_mode, en_dis_mode))
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_aspect_mode fail! \n");
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_dis_aspect_ratio_set(void *pv_hdl_dis, enum aui_dis_aspect_ratio en_asp_ratio)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	enum display_mode en_dis_mode;
	enum tvmode en_tv_mode;
	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL,"Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}
	
	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	if(en_asp_ratio == AUI_DIS_AP_16_9)
		en_tv_mode = TV_16_9;
	else if(en_asp_ratio == AUI_DIS_AP_4_3)
		en_tv_mode = TV_4_3;
	else
		en_tv_mode = TV_AUTO;

	vpo_ioctl(p_vpo_dev, VPO_IO_GET_REAL_DISPLAY_MODE, (unsigned int)&en_dis_mode);

	if(RET_SUCCESS != vpo_aspect_mode(p_vpo_dev, en_tv_mode, en_dis_mode))
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_aspect_mode fail! \n");
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_dis_match_mode_set(void *pv_hdl_dis, enum aui_dis_match_mode en_match_mode)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	enum display_mode en_dis_mode = (unsigned int)en_match_mode;
	enum tvmode en_tv_mode;

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	
	switch(en_match_mode)
	{
		case AUI_DIS_MM_PANSCAN:
			en_dis_mode = PANSCAN;
			break;
		case AUI_DIS_MM_PANSCAN_NOLINEAR:
			en_dis_mode = PANSCAN_NOLINEAR;
			break;
		case AUI_DIS_MM_LETTERBOX:
			en_dis_mode = LETTERBOX;
			break;
		case AUI_DIS_MM_TWOSPEED:
			en_dis_mode = TWOSPEED;
			break;
		case AUI_DIS_MM_PILLBOX:
			en_dis_mode = PILLBOX;
			break;
		case AUI_DIS_MM_VERTICALCUT:
			en_dis_mode = VERTICALCUT;
			break;
		case AUI_DIS_MM_NORMAL_SCALE:
			en_dis_mode = NORMAL_SCALE;
			break;
		case AUI_DIS_MM_LETTERBOX149:
			en_dis_mode = LETTERBOX149;
			break;
		case AUI_DIS_MM_AFDZOOM:
			en_dis_mode = AFDZOOM;
			break;
		case AUI_DIS_MM_PANSCAN43ON169:
			en_dis_mode = PANSCAN43ON169;
			break;
		case AUI_DIS_MM_COMBINED_SCALE:
			en_dis_mode = COMBINED_SCALE;
			break;
		case AUI_DIS_MM_IGNORE:
			en_dis_mode = DONT_CARE;
			break;
		case AUI_DIS_MM_VERTICALCUT_149:
			en_dis_mode = VERTICALCUT_149;
			break;
		default:
			osal_mutex_unlock(s_mod_mutex_id_dis);
			aui_rtn(AUI_RTN_EINVAL, "invalid match mode!");
			break;
	}

	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);


	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	vpo_ioctl(p_vpo_dev, VPO_IO_GET_TV_ASPECT, (unsigned int)&en_tv_mode);

	if(RET_SUCCESS != vpo_aspect_mode(p_vpo_dev, en_tv_mode, en_dis_mode))
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_match_mode fail! \n");
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_dis_tv_system_set(void *pv_hdl_dis, enum aui_dis_tvsys en_tv_sys, unsigned char b_progressive)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	enum tvsystem en_tv_system = (unsigned int)en_tv_sys;

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

	switch(en_tv_sys)
	{
		case AUI_DIS_TVSYS_PAL:
			en_tv_system = PAL;
			break;
		case AUI_DIS_TVSYS_NTSC:
			en_tv_system = NTSC;
			break;
		case AUI_DIS_TVSYS_PAL_M:
			en_tv_system = PAL_M;
			break;
		case AUI_DIS_TVSYS_PAL_N:
			en_tv_system = PAL_N;
			break;
		case AUI_DIS_TVSYS_PAL_60:
			en_tv_system = PAL_60;
			break;
		case AUI_DIS_TVSYS_NTSC_443:
			en_tv_system = NTSC_443;
			break;
		case AUI_DIS_TVSYS_MAC:
			en_tv_system = MAC;
			break;
		case AUI_DIS_TVSYS_LINE_720_50:
			en_tv_system = LINE_720_25;
			break;
		case AUI_DIS_TVSYS_LINE_720_60:
			en_tv_system = LINE_720_30;
			break;
		case AUI_DIS_TVSYS_LINE_1080_25:
			en_tv_system = LINE_1080_25;
			break;
		case AUI_DIS_TVSYS_LINE_1080_30:
			en_tv_system = LINE_1080_30;
			break;
		case AUI_DIS_TVSYS_LINE_1080_50:
			en_tv_system = LINE_1080_50;
			break;
		case AUI_DIS_TVSYS_LINE_1080_60:
			en_tv_system = LINE_1080_60;
			break;
		case AUI_DIS_TVSYS_LINE_1080_24:
			en_tv_system = LINE_1080_24;
			break;
		case AUI_DIS_TVSYS_LINE_1152_ASS:
			en_tv_system = LINE_1152_ASS;
			break;
		case AUI_DIS_TVSYS_LINE_1080_ASS:
			en_tv_system = LINE_1080_ASS;
			break;
		case AUI_DIS_TVSYS_PAL_NC:
			en_tv_system = PAL_NC;
			break;
		default:
			osal_mutex_unlock(s_mod_mutex_id_dis);
			aui_rtn(AUI_RTN_EINVAL, "invalid match mode!");
			break;
	}
	
	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}


	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;
	if(RET_SUCCESS != vpo_tvsys_ex(p_vpo_dev, en_tv_system,b_progressive))
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_tv_system fail! \n");
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_dis_close(aui_attr_dis *pst_attr_dis, void **ppv_hdl_dis)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(pst_attr_dis != NULL)
	{
		AUI_INFO("pst_attr_dis:%x not used,may cause bugs!\n",pst_attr_dis);
	}

	if ( (!ppv_hdl_dis) || (NULL == *ppv_hdl_dis)) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

	if((*(aui_handle_dis **)ppv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock((*(aui_handle_dis **)ppv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	
	aui_dev_unreg(AUI_MODULE_DIS, *ppv_hdl_dis);
	
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(*ppv_hdl_dis))->pst_dev_dis;
	if(RET_SUCCESS != vpo_close(p_vpo_dev))
	{
		osal_mutex_unlock((*(aui_handle_dis **)ppv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_close fail! \n");
	}
	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock((*(aui_handle_dis **)ppv_hdl_dis)->dev_mutex_id);

	if(0!=osal_mutex_delete((*(aui_handle_dis **)ppv_hdl_dis)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "\r\n_delete mutex err.");
	}

	FREE(*ppv_hdl_dis);
	*ppv_hdl_dis = NULL;
	osal_mutex_unlock(s_mod_mutex_id_dis);
	return rtn_code;
}

AUI_RTN_CODE aui_dis_set(void *pv_hdl_dis,enum aui_dis_item_set ul_item, void *pv_param)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	enum aui_dis_format fmt_dis;

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}
	
	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;
	//struct vpo_device *p_vpo_dev_sel = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
    struct vpo_io_cgms_info cy_protected;
    MEMSET(&cy_protected, 0, sizeof(struct vpo_io_cgms_info));
	switch(ul_item)
	{
		case AUI_DIS_SET_AUTO_WINONOFF:
			if(pv_param == NULL)
			{
				osal_mutex_unlock(s_mod_mutex_id_dis);
				aui_rtn(AUI_RTN_EINVAL, NULL);
			}
			vpo_ioctl(p_vpo_dev, VPO_IO_DISAUTO_WIN_ONOFF, *((unsigned int *)pv_param));
			break;
        case AUI_DIS_SET_APS:
			if(pv_param == NULL)
			{
				osal_mutex_unlock(s_mod_mutex_id_dis);
				aui_rtn(AUI_RTN_EINVAL, NULL);
			}
            cy_protected.aps = *((unsigned int *)pv_param);
            cy_protected.cgms = cgms_last;
            vpo_ioctl(p_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cy_protected);
            break;
        case AUI_DIS_SET_CGMS:
			if(pv_param == NULL)
			{
				osal_mutex_unlock(s_mod_mutex_id_dis);
				aui_rtn(AUI_RTN_EINVAL, NULL);
			}
            cy_protected.aps = aps_last;
            cy_protected.cgms = *((unsigned int *)pv_param);
            vpo_ioctl(p_vpo_dev, VPO_IO_SET_CGMS_INFO, (UINT32)&cy_protected);
            break;
        case AUI_DIS_SET_AUXP_ENABLE:
            vpo_win_onoff_ext(p_vpo_dev, *((unsigned int *)pv_param),VPO_LAYER_AUXP);
            break;
        case AUI_DIS_SET_TVESDHD_SOURCE:
        	  fmt_dis = *((int *)pv_param);
        	  if(fmt_dis == AUI_DIS_HD)
        	  {
        	  	aui_dis_set_YPbPr_resolution_same_as_HDMI();
        	  }
        	  else if(fmt_dis == AUI_DIS_SD)
        	  {
        	  	aui_dis_set_YPbPr_resolution_same_as_CVBS();
        	  }
        	  else
        	  {
        	  	aui_rtn(AUI_RTN_EINVAL, NULL);
        	  }
        	  break;
		default:
			break;
	}
	osal_mutex_unlock(s_mod_mutex_id_dis);
	return rtn_code;
}

AUI_RTN_CODE aui_dis_get(void *pv_hdl_dis, enum aui_dis_item_get ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if(s_mod_mutex_id_dis == OSAL_INVALID_ID) {
        aui_rtn(AUI_RTN_EINVAL, "Display not init!");
    }

    osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
    
    if(NULL == pv_hdl_dis) {
        osal_mutex_unlock(s_mod_mutex_id_dis);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    
    struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;
    struct vpo_io_get_info dis_info;
    struct aui_dis_info *st_dis_info = (struct aui_dis_info *)pv_param;
    enum tvmode tv_mode = TV_AUTO;
    enum display_mode dis_mode = PANSCAN;
    struct rect dst_rect = {0,0,1280,720}, src_rect = {0,0,1280,720};

    switch(ul_item) {
        case AUI_DIS_GET_INFO:
            if(pv_param == NULL) {
                osal_mutex_unlock(s_mod_mutex_id_dis);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            vpo_ioctl(p_vpo_dev, VPO_IO_GET_INFO, (unsigned int)&dis_info);
            st_dis_info->bprogressive = dis_info.bprogressive;
            st_dis_info->des_height = dis_info.des_height;
            st_dis_info->des_width = dis_info.des_width;
            st_dis_info->gma1_onoff = dis_info.gma1_onoff;
            st_dis_info->source_height = dis_info.source_height;
            st_dis_info->source_width = dis_info.source_width;
            
            switch(dis_info.tvsys) {
                case PAL:
                    st_dis_info->tvsys = AUI_DIS_TVSYS_PAL;
                    break;
                case NTSC :
                    st_dis_info->tvsys =AUI_DIS_TVSYS_NTSC;
                    break;
                case PAL_M :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_PAL_M;
                    break;
                case PAL_N :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_PAL_N;
                    break;
                case PAL_60 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_PAL_60;
                    break;
                case NTSC_443 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_NTSC_443;
                    break;
                case MAC :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_MAC;
                    break;
                case LINE_720_25 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_720_50;
                    break;
                case LINE_720_30 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_720_60;
                    break;
                case LINE_1080_25 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1080_25;
                    break;
                case LINE_1080_30 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1080_30;
                    break;
                case LINE_1080_50 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1080_50;
                    break;
                case LINE_1080_60 : //12
                    //libc_printf("%s(): tvsys from %u to %u \n",__FUNCTION__,LINE_1080_60,AUI_DIS_TVSYS_LINE_1080_60);
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1080_60;
                    break;
                case LINE_1080_24 :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1080_24;
                    break;
                case LINE_1152_ASS :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1152_ASS;
                    break;
                case LINE_1080_ASS :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_LINE_1080_ASS;
                    break;
                case PAL_NC :
                    st_dis_info->tvsys = AUI_DIS_TVSYS_PAL_NC;
                    break;
                default:
                    osal_mutex_unlock(s_mod_mutex_id_dis);
                    aui_rtn(AUI_RTN_EINVAL, "invalid tv sys!");
                    break;
            } 
            
            vpo_ioctl(p_vpo_dev, VPO_IO_GET_REAL_DISPLAY_MODE, (unsigned int)&dis_mode);       
            switch(dis_mode) {
                case PANSCAN:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PANSCAN;
                    break;
                case PANSCAN_NOLINEAR:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PANSCAN_NOLINEAR;
                    break;
                case LETTERBOX:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_LETTERBOX;
                    break;
                case TWOSPEED:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_TWOSPEED;
                    break; 
                case PILLBOX:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PILLBOX;
                    break;
                case VERTICALCUT:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_VERTICALCUT;
                    break;
                case NORMAL_SCALE:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_NORMAL_SCALE;
                    break;
                case LETTERBOX149:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_LETTERBOX149;
                    break;
                case AFDZOOM:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_AFDZOOM;
                    break;
                case PANSCAN43ON169:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PANSCAN43ON169;
                    break;      
                case COMBINED_SCALE:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_COMBINED_SCALE;
                    break;
                case DONT_CARE:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_IGNORE;
                    break;
                case VERTICALCUT_149:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_VERTICALCUT_149;
                    break;
                default:
                    osal_mutex_unlock(s_mod_mutex_id_dis);
                    aui_rtn(AUI_RTN_EINVAL, "invalid match mode!");
                    break;
            }
            
            vpo_ioctl(p_vpo_dev, VPO_IO_GET_TV_ASPECT, (unsigned int)&tv_mode);
            if(tv_mode == TV_16_9)
                st_dis_info->dis_aspect_ratio = AUI_DIS_AP_16_9;
            else if(tv_mode == TV_4_3)
                st_dis_info->dis_aspect_ratio = AUI_DIS_AP_4_3;
            else if(tv_mode == TV_AUTO)
                st_dis_info->dis_aspect_ratio = AUI_DIS_AP_AUTO;
            else {
                osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid aspect ratio!");
            }

            if ( (vpo_ioctl(p_vpo_dev, VPO_IO_GET_MP_SOURCE_RECT, (unsigned int)&src_rect))
                || (st_dis_info->source_width == 0) || (st_dis_info->source_width == 0)) {
                st_dis_info->src_rect.ui_startX = 0;
                st_dis_info->src_rect.ui_startY = 0;
                st_dis_info->src_rect.ui_width = 720;
                st_dis_info->src_rect.ui_height = 2880;
            } else {
                st_dis_info->src_rect.ui_startX = ((unsigned int)src_rect.u_start_x) * 720 /st_dis_info->source_width;
                st_dis_info->src_rect.ui_startY = ((unsigned int)src_rect.u_start_y) * 2880 /st_dis_info->source_height;
                st_dis_info->src_rect.ui_width = ((unsigned int)src_rect.u_width) * 720 /st_dis_info->source_width;
                st_dis_info->src_rect.ui_height = ((unsigned int)src_rect.u_height) * 2880 /st_dis_info->source_height;
            }

            if ( (vpo_ioctl(p_vpo_dev, VPO_IO_GET_MP_SCREEN_RECT, (unsigned int)&dst_rect))
                || (st_dis_info->des_width == 0) || (st_dis_info->des_width == 0)) {
                st_dis_info->dst_rect.ui_startX = 0;
                st_dis_info->dst_rect.ui_startY = 0;
                st_dis_info->dst_rect.ui_width = 720;
                st_dis_info->dst_rect.ui_height = 2880;
            } else {
                st_dis_info->dst_rect.ui_startX = ((unsigned int)dst_rect.u_start_x) * 720 /st_dis_info->des_width;
                st_dis_info->dst_rect.ui_startY = ((unsigned int)dst_rect.u_start_y) * 2880 /st_dis_info->des_height;
                st_dis_info->dst_rect.ui_width = ((unsigned int)dst_rect.u_width) * 720 /st_dis_info->des_width;
                st_dis_info->dst_rect.ui_height = ((unsigned int)dst_rect.u_height) * 2880 /st_dis_info->des_height;
            }

            break;
            
        default:
            break;
    }
    
    osal_mutex_unlock(s_mod_mutex_id_dis);
    return rtn_code;
}

#ifndef _AUI_OTA_
AUI_RTN_CODE aui_dis_capture_pic(void *pv_hdl_dis, aui_capture_pic *p_cap_param)
{
	//this interface is only useful for mpeg2
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	struct vdec_picture st_capture_picture;

	if ((p_cap_param == NULL) || (NULL == pv_hdl_dis))
		aui_rtn(AUI_RTN_EINVAL,NULL);

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}
	
	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}


	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);


	st_capture_picture.out_data_buf = p_cap_param->puc_out_data_buf;
	st_capture_picture.out_data_buf_size = p_cap_param->ui_out_data_buf_size;

	if(RET_SUCCESS == vdec_capture_display_frame(get_selected_decoder(),(struct vdec_picture*)&st_capture_picture))
	{
		p_cap_param->ui_pic_width = st_capture_picture.pic_width;
		p_cap_param->ui_pic_height = st_capture_picture.pic_height;
		p_cap_param->ui_out_size_valid = st_capture_picture.out_data_valid_size;
	}
	else
	{
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n vpo_capture_pic fail! \n");
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
	return rtn_code;
}

#endif

#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
static void aui_dis_mode_set_smoothly(aui_view_mode mode, 
	aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect)
{
    struct mpsource_call_back mpcall_back;
    struct pipsource_call_back pipcall_back;
    struct vdec_status_info vdec_info;
    struct vdec_pipinfo t_init_info;
    struct vpo_io_get_info dis_info;
    UINT32 waittime=0, wait_total_time = 0;
    struct vdec_device *p_decv_device = (struct vdec_device *)get_selected_decoder();
    struct vpo_device *p_dis_device = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct vdec_device *p_mpeg2device = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *p_avc_device = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    struct vdec_device *p_hevc_device = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    enum VDEC_OUTPUT_MODE output_mode;
    struct rect  src_rect={0, 0, PICTURE_WIDTH, PICTURE_HEIGHT};
    struct rect  dst_rect={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    aui_dis_mode = mode;
    vpo_ioctl(p_dis_device, VPO_IO_GET_INFO, (UINT32) &dis_info);
    t_init_info.adv_setting.switch_mode = 1;//smoothly switch
    t_init_info.adv_setting.out_sys = dis_info.tvsys;
    t_init_info.adv_setting.bprogressive = dis_info.bprogressive;

    if (pst_src_rect != NULL) {
        src_rect.u_start_x = pst_src_rect->ui_startX;
        src_rect.u_start_y = pst_src_rect->ui_startY;
        src_rect.u_width = pst_src_rect->ui_width;
        src_rect.u_height = pst_src_rect->ui_height;
    }

    if (pst_dst_rect != NULL) {
        dst_rect.u_start_x = pst_dst_rect->ui_startX;
        dst_rect.u_start_y = pst_dst_rect->ui_startY;
        dst_rect.u_width = pst_dst_rect->ui_width;
        dst_rect.u_height = pst_dst_rect->ui_height;
    }
    
    if (mode == AUI_VIEW_MODE_PREVIEW) {
        t_init_info.adv_setting.init_mode = 1;
        output_mode = PREVIEW_MODE;
    } else {
        t_init_info.adv_setting.init_mode = 0;
        output_mode = MP_MODE;
    }

    t_init_info.src_rect.u_start_x = src_rect.u_start_x;
    t_init_info.src_rect.u_start_y = src_rect.u_start_y;
    t_init_info.src_rect.u_width = src_rect.u_width;
    t_init_info.src_rect.u_height = src_rect.u_height;
    t_init_info.dst_rect.u_start_x = dst_rect.u_start_x;
    t_init_info.dst_rect.u_start_y = dst_rect.u_start_y;
    t_init_info.dst_rect.u_width = dst_rect.u_width;
    t_init_info.dst_rect.u_height = dst_rect.u_height;

    vdec_io_control(p_mpeg2device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&t_init_info);
    vdec_io_control(p_avc_device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&t_init_info);
    vdec_io_control(p_hevc_device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&t_init_info);

    vdec_set_output(p_decv_device, output_mode, &t_init_info, &mpcall_back, &pipcall_back);

#if defined(SUPPORT_MPEG4_TEST)
    /* for PE module, need set for Media Player
    */
    video_set_output(output_mode, &t_init_info);
#endif 

    vdec_io_control(p_decv_device, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);

    if (vdec_info.u_cur_status == VDEC_STOPPED) {
        wait_total_time = 0;
    } else {
        wait_total_time = MODE_SWITCH_TIME;
    }

    while(waittime < wait_total_time) {
        vdec_io_control(p_decv_device, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);

        if(vdec_info.output_mode == output_mode) {
            if(vdec_info.u_cur_status == VDEC_DECODING) {
                osal_task_sleep(50);
                break;
            }
        }
        
        osal_task_sleep(1);
        waittime++;
    }
    
    if(vdec_info.output_mode != output_mode) {
        //AUI_PRINTF("switch to mode %d forcely\n", output_mode);
        vdec_stop(p_decv_device, FALSE, FALSE);
        vdec_set_output(p_decv_device, output_mode, &t_init_info, &mpcall_back, &pipcall_back);
        vdec_start(p_decv_device);
    }
    
    //vpo_zoom is optional for non-smoothly switch
    if(output_mode == PREVIEW_MODE) {
        /* 
        * HD can not support big scaling, 
        * need to modify the limited scaling ratio depends on multi tests
        * now set it to 1/2, the reason: 
        * no one the right scale will make DE underrun right now, 
        * it depends one the specific video
        * but when the HD scaling ratio is 1/4, DE underrun
        * a 720P stream set to 1/3: underrun
        * a 1080P stream set to 1/2: underrun
        */
        vpo_ioctl(p_dis_device, VPO_IO_GET_INFO, (UINT32) &dis_info);
        
        if (( (dis_info.source_width > 720) || (dis_info.source_height > 576) ) 
            && ( (dst_rect.u_height*2 < src_rect.u_height)
            || (dst_rect.u_width*2 < src_rect.u_width) )) {
            AUI_INFO("DE under run, not call DE zoom\n");//to avoid DE under run
        } else {
            vpo_zoom(p_dis_device, &src_rect, &dst_rect);
        }
    } else {
        vpo_zoom(p_dis_device, &src_rect, &dst_rect);
    }
    
    osal_mutex_lock(g_mutex_scale,OSAL_WAIT_FOREVER_TIME);
    scaled_src.ui_startX = t_init_info.src_rect.u_start_x;
    scaled_src.ui_startY = t_init_info.src_rect.u_start_y;
    scaled_src.ui_width = t_init_info.src_rect.u_width;
    scaled_src.ui_height = t_init_info.src_rect.u_height;
    scaled_dst.ui_startX = t_init_info.dst_rect.u_start_x;
    scaled_dst.ui_startY = t_init_info.dst_rect.u_start_y;
    scaled_dst.ui_width = t_init_info.dst_rect.u_width;
    scaled_dst.ui_height = t_init_info.dst_rect.u_height;
    osal_mutex_unlock(g_mutex_scale);
    return;
}
#else
static void aui_dis_mode_set_non_smoothly(aui_view_mode mode, 
	aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect)
{
    struct mpsource_call_back mpcall_back;
    struct pipsource_call_back pipcall_back;
    struct vdec_status_info vdec_info;
    struct vdec_pipinfo t_init_info;
    struct vpo_io_get_info dis_info;
    UINT32 waittime=0, wait_total_time = 0;
    struct vdec_device *p_decv_device = (struct vdec_device *)get_selected_decoder();
    struct vpo_device *p_dis_device = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    struct vdec_device *p_mpeg2device = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
    struct vdec_device *p_avc_device = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
    struct vdec_device *p_hevc_device = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
    enum VDEC_OUTPUT_MODE output_mode;
    BOOL need_to_wait = FALSE;
    struct rect  src_rect={0, 0, PICTURE_WIDTH, PICTURE_HEIGHT};
    struct rect  dst_rect={0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

    aui_dis_mode = mode;//used for video_decoder_select

    MEMSET(&mpcall_back, 0, sizeof(mpcall_back));
    MEMSET(&pipcall_back, 0, sizeof(pipcall_back));
    MEMSET(&vdec_info, 0, sizeof(vdec_info));
    MEMSET(&t_init_info, 0, sizeof(t_init_info));
    MEMSET(&dis_info, 0, sizeof(dis_info));

    if (pst_src_rect != NULL) {
        src_rect.u_start_x = pst_src_rect->ui_startX;
        src_rect.u_start_y = pst_src_rect->ui_startY;
        src_rect.u_width = pst_src_rect->ui_width;
        src_rect.u_height = pst_src_rect->ui_height;
    }

    if (pst_dst_rect != NULL) {
        dst_rect.u_start_x = pst_dst_rect->ui_startX;
        dst_rect.u_start_y = pst_dst_rect->ui_startY;
        dst_rect.u_width = pst_dst_rect->ui_width;
        dst_rect.u_height = pst_dst_rect->ui_height;
    }

    vpo_ioctl(p_dis_device, VPO_IO_GET_INFO, (UINT32) &dis_info);

    vdec_io_control(p_decv_device, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);

    /* 
    *	all High Definition streams need to close VPO and than the VPO will be opened
    *  automatically by VE to avoid DE underrun 
    */
    if((vdec_info.pic_width > 720) || (vdec_info.pic_height > 576)) {       
        if(VDEC_DECODING == vdec_info.u_cur_status) {
            /* If the current video is High Definition, it needs to wait some time, 
            Then DE will get the right size picture for preview screen display */
            need_to_wait = TRUE;
            vdec_stop(p_decv_device, TRUE, FALSE);//close VPO and stop
        }
    } else {
        //for SD, we can only call vpo_room too
        need_to_wait = FALSE;
    }

    t_init_info.adv_setting.switch_mode = 0;
    t_init_info.adv_setting.out_sys = dis_info.tvsys;
    t_init_info.adv_setting.bprogressive = dis_info.bprogressive;
    
    if (mode == AUI_VIEW_MODE_PREVIEW) {
        t_init_info.adv_setting.init_mode = 1;
        output_mode = PREVIEW_MODE;
    } else {
        t_init_info.adv_setting.init_mode = 0;
        output_mode = MP_MODE;
    }

    t_init_info.src_rect.u_start_x = src_rect.u_start_x;
    t_init_info.src_rect.u_start_y = src_rect.u_start_y;
    t_init_info.src_rect.u_width = src_rect.u_width;
    t_init_info.src_rect.u_height = src_rect.u_height;
    t_init_info.dst_rect.u_start_x = dst_rect.u_start_x;
    t_init_info.dst_rect.u_start_y= dst_rect.u_start_y;
    t_init_info.dst_rect.u_width = dst_rect.u_width;
    t_init_info.dst_rect.u_height = dst_rect.u_height;


    vdec_io_control(p_mpeg2device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&t_init_info);
    vdec_io_control(p_avc_device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&t_init_info);
    vdec_io_control(p_hevc_device, VDEC_IO_SET_OUTPUT_RECT, (UINT32)&t_init_info);

    vdec_set_output(p_decv_device, output_mode, &t_init_info, &mpcall_back, &pipcall_back);

#if defined(SUPPORT_MPEG4_TEST)
    /*
    for PE module, need set for Media Player
    for HD, non smoothly not support changing mode when playing, 
    if change display mode when playing, there will be no video 
    frame displayed (black screen)
    */
    video_set_output(output_mode, &t_init_info);
#endif 

    if(TRUE == need_to_wait) {
        vdec_start(p_decv_device); //restart for HD
        /* For HD, we have to wait for 1st picture decoded, then call vpo_zoom, otherwise, it could cause below issue:
        *DE under run, because DE can scale down HD full size picture to preview size
        */
        wait_total_time = VE_FIRST_SHOW_TIME;
        
        while (waittime < wait_total_time) {
            vdec_io_control(p_decv_device, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
            
            if(vdec_info.u_first_pic_showed) {
                wait_total_time = MODE_SWITCH_TIME;
            }

            if(output_mode == vdec_info.output_mode) {
                if(VDEC_DECODING == vdec_info.u_cur_status) {
                    osal_task_sleep(50);
                }
                break;
            }
            
            osal_task_sleep(1);
            waittime++;
        }
    }

    if (output_mode == PREVIEW_MODE) {
        /*
        * for SD, can only use vpo_zoom, for HD, if the first frame is 
        * not shown after video decoder restart, there will be some mess screen because the DE underrun issue
        * now set it to 1/2, the reason: 
        * no one the right scale will make DE underrun right now, 
        * it depends one the specific video
        * but when the HD scaling ratio is 1/4, DE underrun
        * a 720P stream set to 1/3: underrun
        * a 1080P stream set to 1/2: underrun
        */
        vpo_ioctl(p_dis_device, VPO_IO_GET_INFO, (UINT32) &dis_info);
        vdec_io_control(p_decv_device, VDEC_IO_GET_STATUS, (UINT32)&vdec_info);
        
        /* vpo_zoom must for non-smoothly switch
        * if not call vpo_zoom, the next playing will not show in right rect
        * for HD, we suggest customer close VPO before setting preview mode,
        * and the VPO will be open automaticly by VE when a new frame is going to show
        */
        if (dis_info.mp_on_off && (vdec_info.output_mode != output_mode || vdec_info.u_cur_status != VDEC_DECODING)
            && (dis_info.source_width > 720 || dis_info.source_height > 576)
            && (dst_rect.u_height*2 < src_rect.u_height
            || dst_rect.u_width*2 < src_rect.u_width)) {		
            AUI_INFO("DE under run, not call DE zoom\n");				
        } else {
            //vpo_zoom must for non-smoothly switch
            vpo_zoom(p_dis_device, &src_rect, &dst_rect);
            vpo_ioctl(p_dis_device, VPO_IO_SET_PREVIEW_MODE, 1);//will disable match mode in preview mode
        }
    } else {
        vpo_zoom(p_dis_device, &src_rect, &dst_rect);
        vpo_ioctl(p_dis_device, VPO_IO_SET_PREVIEW_MODE, 0);//will enable match mode in fullview mode
    }

    osal_mutex_lock(g_mutex_scale,OSAL_WAIT_FOREVER_TIME);
    scaled_src.ui_startX = t_init_info.src_rect.u_start_x;
    scaled_src.ui_startY = t_init_info.src_rect.u_start_y;
    scaled_src.ui_width = t_init_info.src_rect.u_width;
    scaled_src.ui_height = t_init_info.src_rect.u_height;
    scaled_dst.ui_startX = t_init_info.dst_rect.u_start_x;
    scaled_dst.ui_startY = t_init_info.dst_rect.u_start_y;
    scaled_dst.ui_width = t_init_info.dst_rect.u_width;
    scaled_dst.ui_height = t_init_info.dst_rect.u_height;
    osal_mutex_unlock(g_mutex_scale);
}
#endif

AUI_RTN_CODE aui_dis_mode_set(void *pv_hdl_dis,aui_view_mode mode, 
	aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	
    if ( (!pv_hdl_dis) || (mode < AUI_VIEW_MODE_PREVIEW) || (mode > AUI_VIEW_MODE_FULL)) {
        	aui_rtn(AUI_RTN_EINVAL, "\n vpo mode set handle null! \n");
    }    

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	
	switch(mode)
	{
		case AUI_VIEW_MODE_PREVIEW:
			if ( (pst_dst_rect == NULL) || (pst_src_rect == NULL)) {
				osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
				aui_rtn(AUI_RTN_EINVAL, NULL);
			}
			#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
			aui_dis_mode_set_smoothly(AUI_VIEW_MODE_PREVIEW, pst_src_rect, pst_dst_rect);
			#else
			aui_dis_mode_set_non_smoothly(AUI_VIEW_MODE_PREVIEW, pst_src_rect, pst_dst_rect);
			#endif
			break;
		case AUI_VIEW_MODE_MULTI:
			break;
		case AUI_VIEW_MODE_FULL:			
			// convert VE to MP mode
			#ifdef MP_PREVIEW_SWITCH_SMOOTHLY
			aui_dis_mode_set_smoothly(AUI_VIEW_MODE_FULL, pst_src_rect, pst_dst_rect);
			#else
			aui_dis_mode_set_non_smoothly(AUI_VIEW_MODE_FULL, pst_src_rect, pst_dst_rect);
			#endif
            break;
		default:
	        osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
			aui_rtn(AUI_RTN_EINVAL, NULL);
            break;
	}
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_dis_dac_unreg(void *pv_hdl_dis,enum aui_dis_unreg_type type)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_hdl_dis)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id == OSAL_INVALID_ID)
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, "Error dev_mutex_id!");
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;
	switch(type)
	{
		case AUI_DIS_TYPE_UNREG_YUV:
			vpo_ioctl(p_vpo_dev,VPO_IO_UNREG_DAC,YUV_1);//close YUV DAC
			break;
		case AUI_DIS_TYPE_UNREG_RGB:
			vpo_ioctl(p_vpo_dev,VPO_IO_UNREG_DAC,RGB_1);//close YUV DAC
			break;
		case AUI_DIS_TYPE_UNREG_SVIDEO:
			#ifdef VDAC_USE_SVIDEO_TYPE
			vpo_ioctl(p_vpo_dev,VPO_IO_UNREG_DAC,VDAC_USE_SVIDEO_TYPE);//close YUV DAC
			#endif
			break;
		case AUI_DIS_TYPE_UNREG_CVBS:
			vpo_ioctl(p_vpo_dev,VPO_IO_UNREG_DAC,CVBS_1);//close YUV DAC
			break;
	}

    osal_task_sleep(50);
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);

	return rtn_code;

}

AUI_RTN_CODE aui_dis_layer_order(aui_hdl pv_hdl_dis, enum aui_dis_layer_blend_order order)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	struct vpo_device *p_vpo_dev = NULL;

    if ( (NULL == pv_hdl_dis) || 
            (order < AUI_DIS_LAYER_ORDER_MP_GMAS_GMAF_AUXP) || 
            (order > AUI_DIS_LAYER_ORDER_AUXP_MP_GMAF_GMAS)) {
            aui_rtn(AUI_RTN_EINVAL, NULL);
    }

	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);
	p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;
	switch(order)
	{
		case AUI_DIS_LAYER_ORDER_MP_GMAS_GMAF_AUXP:
			order = MP_GMAS_GMAF_AUXP;
			break;
		case AUI_DIS_LAYER_ORDER_MP_GMAS_AUXP_GMAF:
		  order = MP_GMAS_AUXP_GMAF;
			break;
		case AUI_DIS_LAYER_ORDER_MP_GMAF_GMAS_AUXP:
		  order = MP_GMAF_GMAS_AUXP;
			break;
		case AUI_DIS_LAYER_ORDER_MP_GMAF_AUXP_GMAS:
		  order = MP_GMAF_AUXP_GMAS;
			break;	
		case AUI_DIS_LAYER_ORDER_MP_AUXP_GMAS_GMAF:
		  order = MP_AUXP_GMAS_GMAF;
			break; 	
		case AUI_DIS_LAYER_ORDER_MP_AUXP_GMAF_GMAS:
		  order = MP_AUXP_GMAF_GMAS;
			break;
		case AUI_DIS_LAYER_ORDER_AUXP_MP_GMAS_GMAF:
		  order = AUXP_MP_GMAS_GMAF;
			break;
		case AUI_DIS_LAYER_ORDER_AUXP_MP_GMAF_GMAS:
		  order = AUXP_MP_GMAF_GMAS;
			break;
		default:
		  osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
			return AUI_RTN_FAIL;
			break;
	}
	vpo_ioctl(p_vpo_dev, VPO_IO_SET_LAYER_ORDER, order);
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_dis_cvbs_start(void *pv_hdl_dis)
{
    (void) pv_hdl_dis;
	struct vpo_device *p_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
	vpo_ioctl(p_vpo_dev, VPO_IO_DISAUTO_WIN_ONOFF, FALSE);
	vpo_ioctl(p_vpo_dev, VPO_IO_CLOSE_DEO, FALSE);
	vpo_win_onoff(p_vpo_dev,1);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_cvbs_stop(void *pv_hdl_dis)
{
    (void) pv_hdl_dis;
	struct vpo_device *p_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 1);
	vpo_ioctl(p_vpo_dev, VPO_IO_DISAUTO_WIN_ONOFF, TRUE);
	vpo_ioctl(p_vpo_dev, VPO_IO_CLOSE_DEO, TRUE);
	vpo_win_onoff(p_vpo_dev,0);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_set_background_color(aui_hdl pv_hdl_dis, aui_dis_color * color)
{
	struct  ycb_cr_color bgcolor; 
	if(s_mod_mutex_id_dis == OSAL_INVALID_ID)
	{
		aui_rtn(AUI_RTN_EINVAL, "Display not init!");
	}
	
	osal_mutex_lock(s_mod_mutex_id_dis,OSAL_WAIT_FOREVER_TIME);
	if( (NULL == pv_hdl_dis) || (NULL == color))
	{
		osal_mutex_unlock(s_mod_mutex_id_dis);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_dis);

	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;

	bgcolor.u_y = color->y;
	bgcolor.u_cb = color->cb;
	bgcolor.u_cr = color->cr;
	if (RET_SUCCESS != vpo_ioctl(p_vpo_dev,VPO_IO_SET_BG_COLOR,(unsigned int)&bgcolor)) {
		osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\n set bg color fail! \n");
	}
	
	osal_mutex_unlock(((aui_handle_dis *)pv_hdl_dis)->dev_mutex_id);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_3d_set(aui_hdl pv_hdl_dis, enum aui_dis_3D_mode dis_3d_mode)
{
    if ( (NULL == pv_hdl_dis) || 
            (dis_3d_mode >= AUI_DIS_3D_MODE_MAX) || 
            (dis_3d_mode < AUI_DIS_3D_MODE_NONE))
        aui_rtn(AUI_RTN_EINVAL, NULL);
	return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_dis_set_YPbPr_resolution_same_as_CVBS()
{
	struct vpo_device *p_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0); // attr_dis.uc_dev_idx = AUI_DIS_HD;
	vpo_ioctl(p_vpo_dev, VPO_IO_TVESDHD_SOURCE_SEL, TVESDHD_SRC_DEO);  //TVESDHD_SRC_DEO is CVBS

	return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_dis_set_YPbPr_resolution_same_as_HDMI()
{
	struct vpo_device *p_vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0); // attr_dis.uc_dev_idx = AUI_DIS_HD;
	vpo_ioctl(p_vpo_dev, VPO_IO_TVESDHD_SOURCE_SEL, TVESDHD_SRC_DEN);  //TVESDHD_SRC_DEO is HDMI

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_afd_set(aui_hdl pv_hdl_dis, aui_dis_afd_attr *dis_afd_para)
{
	if( (NULL == pv_hdl_dis) || (NULL == dis_afd_para))
		aui_rtn(AUI_RTN_EINVAL, "pv_hdl_dis or dis_afd_para is NULL!");

	struct vp_io_afd_para afd_para;
	struct vpo_device *p_vpo_dev = ((aui_handle_dis *)(pv_hdl_dis))->pst_dev_dis;
	MEMSET(&afd_para, 0, sizeof(struct vp_io_afd_para));

	afd_para.b_swscale_afd = dis_afd_para->afd_mode;
	afd_para.afd_solution = dis_afd_para->afd_spec;
	afd_para.protect_mode_enable = dis_afd_para->afd_protect_enable;
	if (dis_afd_para->afd_mode == AUI_DIS_AFD_MODE_USER_SCALE) {
		vpo_ioctl(p_vpo_dev, VPO_IO_SELECT_SCALE_MODE, TRUE);
		afd_para.b_swscale_afd = 0;
	} else {
		vpo_ioctl(p_vpo_dev, VPO_IO_SELECT_SCALE_MODE, FALSE);
	}
	return vpo_ioctl(p_vpo_dev, VPO_IO_AFD_CONFIG, (UINT32)&afd_para);
}

AUI_RTN_CODE aui_dis_enhance_max_value_set(aui_hdl pv_hdl_dis, unsigned int max)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *p_dis_hdl  = (aui_handle_dis *)(pv_hdl_dis);
    
    if ((NULL == pv_hdl_dis) || (0 == max) || (max > 0xFFFF))
        aui_rtn(AUI_RTN_EINVAL, NULL);

    p_dis_hdl->video_enhance_max = max;
    
    return rtn_code;
}

AUI_RTN_CODE aui_dis_sar_scale_enable (aui_hdl pv_hdl_dis, unsigned char enable)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	(void)pv_hdl_dis;
	(void)enable;
	AUI_INFO("TODO...");
	return rtn_code;
}

