/**@file
 *    @brief         AUI diss module interface implement
 *    @author        Glen.Dai
 *    @date          2013-6-27
 *    @version       1.0.0
 *    @note          ali corp. all rights reserved. 2013~2999 copyright (C)
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "aui_common_priv.h"
#include "aui_decv.h"
#include "aui_decv_priv.h"

#include "alisl_types.h"
#include "alisldis.h"
#include "alislvdec.h"
#include "alisldmx.h"

AUI_MODULE(DIS)

#define MAX_YUV_NUM 2
#define MAX_RGB_NUM 2
#define MAX_SVIDEO_NUM 3
#define MAX_CVBS_NUM 6

#define IS_CUR_STREAM_HD(info)                                          \
    ({ ((info).pic_width > 720) || ((info).pic_height > 576) ? true: false;})

#define IS_CUR_STREAM_SD(info) (!IS_CUR_STREAM_HD(info))
#define IS_CUR_DECODER_AVC(decoder)  (VDEC_DECODER_AVC == decoder)
#define IS_CUR_DECODER_MPEG(decoder) (VDEC_DECODER_MPEG == decoder)


/* if there is enough data, we wait at most 5s to make sure switching done
 * Bug #91442 one media(hevc) need 3s to switch done, driver suggest 
 * setting the wait time to 5s to cover most test cases
 */
#define VE_MODE_SWITCH_TIME 5000        /* 5000ms */

/* if there is no data, we just make sure there no data in 500ms
 * and return fail for mode switching 
 */
#define VE_MODE_SWITCH_NO_DATA_TIME 500 /* 500ms */

#define SL_ASSERT(s)                                                    \
    do {                                                                \
        alisl_retcode ret = s;                                          \
        if (ret)                                                       \
            aui_rtn(AUI_RTN_FAIL, "\n Call "#s" failed! \n"); \
    } while(0)

typedef struct {
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	alisl_handle      slhdl;          /**< share lib device handle */
	aui_attr_dis      attr_dis;
	enum dis_aspect_ratio  aspect;
	enum dis_display_mode  display_mode;
	unsigned char sar_scale;//0: disable, 1:enable(default)
	/* analog protect system 0-3: 
	 * 0: off
	 * 1: Automatic Gain Control
	 * 2: 2 line color burst
	 * 3: 4 line color burst
     */
	unsigned char aps; 
	/* 0-3: 
	 * 0: Copy Freely
	 * 1: Copy No More
	 * 2: Copy Once
	 * 3: Copy Never
	 */
	unsigned char cgms_a; 
} aui_handle_dis, *aui_p_handle_dis;

/* locall mutex lock */
static pthread_mutex_t m_dis_mutex = PTHREAD_MUTEX_INITIALIZER;

static enum aui_dis_tvsys
to_aui_tvsys(enum dis_tvsys dis_tvsys)
{
	switch (dis_tvsys) {
		case DIS_TVSYS_PAL:
				return AUI_DIS_TVSYS_PAL;
		case DIS_TVSYS_NTSC:
			return AUI_DIS_TVSYS_NTSC;
		case DIS_TVSYS_PAL_M:
			return AUI_DIS_TVSYS_PAL_M;
		case DIS_TVSYS_PAL_N:
			return AUI_DIS_TVSYS_PAL_N;
		case DIS_TVSYS_PAL_60:
			return AUI_DIS_TVSYS_PAL_60;
		case DIS_TVSYS_NTSC_443:
			return AUI_DIS_TVSYS_NTSC_443;
		case DIS_TVSYS_MAC:
			return AUI_DIS_TVSYS_MAC;
		case DIS_TVSYS_LINE_720_25:
			return AUI_DIS_TVSYS_LINE_720_50;
		case DIS_TVSYS_LINE_720_30:
			return AUI_DIS_TVSYS_LINE_720_60;
		case DIS_TVSYS_LINE_1080_25:
			return AUI_DIS_TVSYS_LINE_1080_25;
		case DIS_TVSYS_LINE_1080_30:
			return AUI_DIS_TVSYS_LINE_1080_30;
		case DIS_TVSYS_LINE_1080_50:
			return AUI_DIS_TVSYS_LINE_1080_50;
		case DIS_TVSYS_LINE_1080_60:
			return AUI_DIS_TVSYS_LINE_1080_60;
		case DIS_TVSYS_LINE_1080_24:
			return AUI_DIS_TVSYS_LINE_1080_24;
		case DIS_TVSYS_LINE_1152_ASS:
			return AUI_DIS_TVSYS_LINE_1152_ASS;
		case DIS_TVSYS_LINE_1080_ASS:
			return AUI_DIS_TVSYS_LINE_1080_ASS;
		case DIS_TVSYS_PAL_NC:
			return AUI_DIS_TVSYS_PAL_NC;
		case DIS_TVSYS_LINE_4096X2160_24:
			return AUI_DIS_TVSYS_LINE_4096X2160_24;
		case DIS_TVSYS_LINE_3840X2160_24:
			return AUI_DIS_TVSYS_LINE_3840X2160_24;
		case DIS_TVSYS_LINE_3840X2160_25:
			return AUI_DIS_TVSYS_LINE_3840X2160_25;
		case DIS_TVSYS_LINE_3840X2160_30:
			return AUI_DIS_TVSYS_LINE_3840X2160_30;
			
		default:
			return -1;
	}
}

static aui_dis_output_type
to_aui_dac_info(enum dis_output_type dac_type)
{
    switch (dac_type) {
        case DIS_TYPE_Y:
            return AUI_DIS_TYPE_Y;
        case DIS_TYPE_U:
            return AUI_DIS_TYPE_U;
        case DIS_TYPE_V:
            return AUI_DIS_TYPE_V;
        case DIS_TYPE_R:
            return AUI_DIS_TYPE_R;
        case DIS_TYPE_G:
            return AUI_DIS_TYPE_G;
        case DIS_TYPE_B:
            return AUI_DIS_TYPE_B;
        case DIS_TYPE_CVBS:
            return AUI_DIS_TYPE_CVBS;
        case DIS_TYPE_SVIDEO_Y:
            return AUI_DIS_TYPE_SVIDEO_Y;
        case DIS_TYPE_SVIDEO_C:
            return AUI_DIS_TYPE_SVIDEO_C;
        case DIS_TYPE_NONE:
            return AUI_DIS_TYPE_NONE;

        default:
            AUI_ERR("This dac registered error dac_type = %d\n", dac_type);
            return AUI_DIS_TYPE_NONE;
    }
}

AUI_RTN_CODE
aui_dis_init(aui_func_dis_init fn_dis_init)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if (fn_dis_init) {
		fn_dis_init();
	}

	return rtn_code;
}

AUI_RTN_CODE
aui_dis_open(aui_attr_dis  *pst_attr_dis,
			 void         **ppv_hdldis)
{
	aui_handle_dis *dev_hdl;
	alisl_retcode   sl_ret;
	enum dis_dev_id dev_id;

	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if ((NULL == pst_attr_dis) || (NULL == ppv_hdldis))
		aui_rtn(AUI_RTN_EINVAL, NULL);

	if ((pst_attr_dis->uc_dev_idx != AUI_DIS_HD) && (pst_attr_dis->uc_dev_idx != AUI_DIS_SD))
		aui_rtn(AUI_RTN_EINVAL,
				"\n Not a validate DEVICE (Must be SD/HD)!\n");

	dev_id = (AUI_DIS_HD == pst_attr_dis->uc_dev_idx) ? DIS_HD_DEV : DIS_SD_DEV;

	dev_hdl = (aui_handle_dis *)malloc(sizeof(aui_handle_dis));
	if (NULL == dev_hdl)
		aui_rtn(AUI_RTN_ENOMEM, NULL);

	memset(dev_hdl, 0, sizeof(aui_handle_dis));

	sl_ret = alisldis_open(dev_id, &(dev_hdl->slhdl));
	if (DIS_ERR_NONE != sl_ret) {
		free(dev_hdl);
		aui_rtn(AUI_RTN_FAIL, "\n Share library open failed! \n");
	}

	dev_hdl->aspect = DIS_AR_4_3;
	dev_hdl->display_mode = DIS_DM_NORMAL_SCALE;

	memcpy((void *)(&(dev_hdl->attr_dis)),
		   (void *)pst_attr_dis, sizeof(aui_attr_dis));

	dev_hdl->data.dev_idx = pst_attr_dis->uc_dev_idx;
	aui_dev_reg(AUI_MODULE_DIS, dev_hdl);

	*ppv_hdldis = dev_hdl;
	return rtn_code;
}


AUI_RTN_CODE
aui_dis_close(aui_attr_dis  *pst_attr_dis,
			  void         **ppv_hdl_dis)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
    
	aui_handle_dis *dev_hdl  = NULL;
 
    if (pst_attr_dis) {
        ;
    }

	if (!ppv_hdl_dis || (NULL == *ppv_hdl_dis)) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	dev_hdl  = (aui_handle_dis *)(*ppv_hdl_dis);
	if (DIS_ERR_NONE != alisldis_close(dev_hdl->slhdl))
		aui_rtn(AUI_RTN_FAIL, "\n Display close fail! \n");

	aui_dev_unreg(AUI_MODULE_DIS, dev_hdl);

	free(*ppv_hdl_dis);
	*ppv_hdl_dis = NULL;
	return rtn_code;
}

AUI_RTN_CODE
aui_dis_enhance_set(void                          *pv_hdl_dis,
					enum aui_dis_video_enhance  en_eh_type,
					unsigned int                   ui_value)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	struct dis_enhance enhance;

	if ((NULL == pv_hdl_dis) || (AUI_DIS_ENHANCE_SHARPNESS == en_eh_type && ui_value > 10 ))
		aui_rtn(AUI_RTN_EINVAL, NULL);
    memset(&enhance, 0, sizeof(enhance));
	switch(en_eh_type) {
		case AUI_DIS_ENHANCE_BRIGHTNESS:
			enhance.type       = DIS_ENHANCE_BRIGHTNESS;
			enhance.brightness = ui_value;
			break;
		case AUI_DIS_ENHANCE_CONTRAST:
			enhance.type       = DIS_ENHANCE_CONTRAST;
			enhance.contrast   = ui_value;
			break;
		case AUI_DIS_ENHANCE_SATURATION:
			enhance.type       = DIS_ENHANCE_SATURATION;
			enhance.saturation = ui_value;
			break;
		case AUI_DIS_ENHANCE_SHARPNESS:
			enhance.type       = DIS_ENHANCE_SHARPNESS;
			enhance.sharpeness = ui_value;
			break;
		case AUI_DIS_ENHANCE_HUE:
			enhance.type       = DIS_ENHANCE_HUE;
			enhance.hue        = ui_value;
			break;

		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate enhance type specified! \n");
	}

	if (DIS_ERR_NONE != alisldis_set_enhance(dev_hdl->slhdl, &enhance))
		aui_rtn(AUI_RTN_FAIL, "\n Set enhanc fail! \n");

	return rtn_code;
}

AUI_RTN_CODE
aui_dis_zoom(void              *pv_hdl_dis,
			 aui_dis_zoom_rect *pst_src_rect,
			 aui_dis_zoom_rect *pst_dst_rect)
{
	AUI_RTN_CODE     rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis  *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	struct dis_rect  srect, drect;
	alisl_retcode    sl_ret;

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

	srect.x = pst_src_rect->ui_startX;
	srect.y = pst_src_rect->ui_startY;
	srect.w = pst_src_rect->ui_width;
	srect.h = pst_src_rect->ui_height;

	drect.x = pst_dst_rect->ui_startX;
	drect.y = pst_dst_rect->ui_startY;
	drect.w = pst_dst_rect->ui_width;
	drect.h = pst_dst_rect->ui_height;

	sl_ret = alisldis_zoom_by_layer(dev_hdl->slhdl,
									&srect, &drect, DIS_LAYER_MAIN);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Display zoom fail! \n");

	return rtn_code;
}


AUI_RTN_CODE 
aui_dis_zoom_ext(void *pv_hdl_dis, aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect, int layer)

{
	AUI_RTN_CODE     rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis  *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	struct dis_rect  srect, drect;
	alisl_retcode    sl_ret;
	enum dis_layer   sl_layer = DIS_LAYER_MAIN;

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

	srect.x = pst_src_rect->ui_startX;
	srect.y = pst_src_rect->ui_startY;
	srect.w = pst_src_rect->ui_width;
	srect.h = pst_src_rect->ui_height;

	drect.x = pst_dst_rect->ui_startX;
	drect.y = pst_dst_rect->ui_startY;
	drect.w = pst_dst_rect->ui_width;
	drect.h = pst_dst_rect->ui_height;

	if (layer == 1) {
		sl_layer = DIS_LAYER_AUXP;
	} else {
		sl_layer = DIS_LAYER_MAIN;
	}
	sl_ret = alisldis_zoom_by_layer(dev_hdl->slhdl,
									&srect, &drect, sl_layer);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Display zoom fail! \n");

	return rtn_code;
}

AUI_RTN_CODE
aui_dis_video_enable(void          *pv_hdl_dis,
					 unsigned char  b_enable)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	bool            onoff    = b_enable ? true : false;
	alisl_retcode   sl_ret;             /* share lib ret code */

	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	sl_ret = alisldis_win_onoff_by_layer(dev_hdl->slhdl, onoff, DIS_LAYER_MAIN);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Winonff display failed! \n");

	return rtn_code;
}

AUI_RTN_CODE
aui_dis_aspect_ratio_set(void                         *pv_hdl_dis,
						 enum aui_dis_aspect_ratio  en_asp_ratio)
{
	AUI_RTN_CODE           rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis        *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	enum dis_aspect_ratio  aspect_mode;
	alisl_retcode          sl_ret;

	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	switch (en_asp_ratio) {
		case AUI_DIS_AP_16_9 :
			aspect_mode = DIS_AR_16_9;
            if(DIS_DM_PANSCAN == dev_hdl->display_mode)
            {
                dev_hdl->display_mode = DIS_DM_VERTICALCUT;
            }
            else if(DIS_DM_LETTERBOX == dev_hdl->display_mode)
            {
                dev_hdl->display_mode = DIS_DM_PILLBOX;
            }
			break;
		case AUI_DIS_AP_4_3  :
			aspect_mode = DIS_AR_4_3;
            if(DIS_DM_VERTICALCUT== dev_hdl->display_mode)
            {
                dev_hdl->display_mode = DIS_DM_PANSCAN ;
            }
            else if(DIS_DM_PILLBOX == dev_hdl->display_mode)
            {
                dev_hdl->display_mode = DIS_DM_LETTERBOX;
            }  
			break;
		case AUI_DIS_AP_AUTO :
			aspect_mode = DIS_AR_AUTO;
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate aspect ratio specified! \n");
	}

	sl_ret = alisldis_set_aspect_mode(dev_hdl->slhdl, dev_hdl->display_mode, aspect_mode);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Set aspect ratio failed!\n");

	dev_hdl->aspect = aspect_mode;

	return rtn_code;
}

AUI_RTN_CODE
aui_dis_match_mode_set(void                       *pv_hdl_dis,
					   enum aui_dis_match_mode  en_match_mode)
{
	AUI_RTN_CODE           rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis        *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	enum dis_display_mode  display_mode;
	alisl_retcode          sl_ret;

	if(NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	switch (en_match_mode) {
		case AUI_DIS_MM_PANSCAN          :
			display_mode = DIS_DM_PANSCAN;
            if(DIS_AR_16_9 == dev_hdl->aspect)
            {
                display_mode = DIS_DM_VERTICALCUT;
            }
			break;
		case AUI_DIS_MM_PANSCAN_NOLINEAR :
			display_mode = DIS_DM_PANSCAN_NOLINEAR;
			break;
		case AUI_DIS_MM_LETTERBOX        :
			display_mode = DIS_DM_LETTERBOX;
			if(DIS_AR_16_9 == dev_hdl->aspect)
			 {
				 display_mode = DIS_DM_PILLBOX;
			 }
			break;
		case AUI_DIS_MM_TWOSPEED         :
			display_mode = DIS_DM_TWOSPEED;
			break;
		case AUI_DIS_MM_PILLBOX          :
			display_mode = DIS_DM_PILLBOX;
			if(DIS_AR_4_3 == dev_hdl->aspect)
			 {
				 display_mode = DIS_DM_LETTERBOX;
			 }	
			break;
		case AUI_DIS_MM_VERTICALCUT      :
			display_mode = DIS_DM_VERTICALCUT;
            if(DIS_AR_4_3 == dev_hdl->aspect)
            {
                display_mode = DIS_DM_PANSCAN;
            }  
			break;
		case AUI_DIS_MM_NORMAL_SCALE     :
			display_mode = DIS_DM_NORMAL_SCALE;
			break;
		case AUI_DIS_MM_LETTERBOX149     :
			display_mode = DIS_DM_LETTERBOX149;
			break;
		case AUI_DIS_MM_AFDZOOM          :
			display_mode = DIS_DM_AFDZOOM;
			break;
		case AUI_DIS_MM_PANSCAN43ON169   :
			display_mode = DIS_DM_PANSCAN43ON169;
			break;
		case AUI_DIS_MM_COMBINED_SCALE   :
			display_mode = DIS_DM_COMBINED_SCALE;
			break;
		case AUI_DIS_MM_IGNORE           :
			display_mode = DIS_DM_IGNORE;
			break;
		case AUI_DIS_MM_VERTICALCUT_149         :
			display_mode = DIS_DM_VERTICALCUT_149;
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate match mode specified! \n");
	}

	sl_ret = alisldis_set_aspect_mode(dev_hdl->slhdl, display_mode, dev_hdl->aspect);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Display set match_mode failed! \n");

	dev_hdl->display_mode = display_mode;

	return rtn_code;
}

AUI_RTN_CODE aui_dis_aspect_ratio_set_ext(void *pv_hdl_dis,
										  enum aui_dis_aspect_ratio en_asp_ratio,
										  enum aui_dis_match_mode match_mode)
{
	AUI_RTN_CODE           rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis        *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	enum dis_aspect_ratio  aspect_mode;
	enum dis_display_mode  display_mode;
	alisl_retcode          sl_ret;

	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	switch (en_asp_ratio) {
		case AUI_DIS_AP_16_9 :
			aspect_mode = DIS_AR_16_9;
			break;
		case AUI_DIS_AP_4_3  :
			aspect_mode = DIS_AR_4_3;
			break;
		case AUI_DIS_AP_AUTO :
			aspect_mode = DIS_AR_AUTO;
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate aspect ratio specified! \n");
	}

	switch (match_mode) {
		case AUI_DIS_MM_PANSCAN          :
			display_mode = DIS_DM_PANSCAN;
			/* when tv mode set to 16:9, there is effect with PANSCAN
			 * force to VERTICALCUT to meet customer in common use
			 */
			if (aspect_mode == DIS_AR_16_9)
			    display_mode = DIS_DM_VERTICALCUT;
			break;
		case AUI_DIS_MM_PANSCAN_NOLINEAR :
			display_mode = DIS_DM_PANSCAN_NOLINEAR;
			break;
		case AUI_DIS_MM_LETTERBOX        :
			display_mode = DIS_DM_LETTERBOX;
			/* when tv mode set to 16:9, there is effect with LETTERBOX
			 * force to PILLBOX to meet customer in common use
			 */
			if (aspect_mode == DIS_AR_16_9)
			    display_mode = DIS_DM_PILLBOX;
			break;
		case AUI_DIS_MM_TWOSPEED         :
			display_mode = DIS_DM_TWOSPEED;
			break;
		case AUI_DIS_MM_PILLBOX          :
			display_mode = DIS_DM_PILLBOX;
			/* when tv mode set to 4:3, there is effect with PILLBOX
			 * force to LETTERBOX to meet customer in common use
			 */
			if (aspect_mode == DIS_AR_4_3)
			    display_mode = DIS_DM_LETTERBOX;
			break;
		case AUI_DIS_MM_VERTICALCUT      :
		    /* when tv mode set to 4:3, there is effect with VERTICALCUT 
			 * force to PANSCAN to meet customer in common use
			 */
			display_mode = DIS_DM_VERTICALCUT;
			if (aspect_mode == DIS_AR_4_3)
			    display_mode = DIS_DM_PANSCAN;
			break;
		case AUI_DIS_MM_NORMAL_SCALE     :
			display_mode = DIS_DM_NORMAL_SCALE;
			break;
		case AUI_DIS_MM_LETTERBOX149     :
			display_mode = DIS_DM_LETTERBOX149;
			break;
		case AUI_DIS_MM_AFDZOOM          :
			display_mode = DIS_DM_AFDZOOM;
			break;
		case AUI_DIS_MM_PANSCAN43ON169   :
			display_mode = DIS_DM_PANSCAN43ON169;
			break;
		case AUI_DIS_MM_COMBINED_SCALE   :
			display_mode = DIS_DM_COMBINED_SCALE;
			break;
		case AUI_DIS_MM_IGNORE           :
			display_mode = DIS_DM_IGNORE;
			break;
		case AUI_DIS_MM_VERTICALCUT_149         :
			display_mode = DIS_DM_VERTICALCUT_149;
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate match mode specified! \n");
	}

	sl_ret = alisldis_set_aspect_mode(dev_hdl->slhdl, display_mode, aspect_mode);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Set aspect ratio failed!\n");

	dev_hdl->aspect = aspect_mode;
	dev_hdl->display_mode = display_mode;

	return rtn_code;
}

AUI_RTN_CODE
aui_dis_tv_system_set(void                      *pv_hdl_dis,
					  enum aui_dis_tvsys  en_tv_sys,
					  unsigned char              b_progressive)
{
	AUI_RTN_CODE    rtn_code    = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl     = (aui_handle_dis *)(pv_hdl_dis);
	bool            progressive = b_progressive ? true : false;
	alisl_retcode   sl_ret;
	enum dis_tvsys  tvsys;

	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	switch (en_tv_sys) {
		case AUI_DIS_TVSYS_PAL           :
			tvsys = DIS_TVSYS_PAL;
			break;
		case AUI_DIS_TVSYS_NTSC          :
			tvsys = DIS_TVSYS_NTSC;
			break;
		case AUI_DIS_TVSYS_PAL_M         :
			tvsys = DIS_TVSYS_PAL_M;
			break;
		case AUI_DIS_TVSYS_PAL_N         :
			tvsys = DIS_TVSYS_PAL_N;
			break;
		case AUI_DIS_TVSYS_PAL_60        :
			tvsys = DIS_TVSYS_PAL_60;
			break;
		case AUI_DIS_TVSYS_NTSC_443      :
			tvsys = DIS_TVSYS_NTSC_443;
			break;
		case AUI_DIS_TVSYS_MAC           :
			tvsys = DIS_TVSYS_MAC;
			break;
		case AUI_DIS_TVSYS_LINE_720_50   :
			tvsys = DIS_TVSYS_LINE_720_25;
			break;
		case AUI_DIS_TVSYS_LINE_720_60   :
			tvsys = DIS_TVSYS_LINE_720_30;
			break;
		case AUI_DIS_TVSYS_LINE_1080_25  :
			tvsys = DIS_TVSYS_LINE_1080_25;
			break;
		case AUI_DIS_TVSYS_LINE_1080_30  :
			tvsys = DIS_TVSYS_LINE_1080_30;
			break;
		case AUI_DIS_TVSYS_LINE_1080_50  :
			tvsys = DIS_TVSYS_LINE_1080_50;
			break;
		case AUI_DIS_TVSYS_LINE_1080_60  :
			tvsys = DIS_TVSYS_LINE_1080_60;
			break;
		case AUI_DIS_TVSYS_LINE_1080_24  :
			tvsys = DIS_TVSYS_LINE_1080_24;
			break;
		case AUI_DIS_TVSYS_LINE_1152_ASS :
			tvsys = DIS_TVSYS_LINE_1152_ASS;
			break;
		case AUI_DIS_TVSYS_LINE_1080_ASS :
			tvsys = DIS_TVSYS_LINE_1080_ASS;
			break;
		case AUI_DIS_TVSYS_PAL_NC :
			tvsys = DIS_TVSYS_PAL_NC;
			break;
		case AUI_DIS_TVSYS_LINE_4096X2160_24:
			tvsys = DIS_TVSYS_LINE_4096X2160_24;
			break;
		case AUI_DIS_TVSYS_LINE_3840X2160_24:
			tvsys = DIS_TVSYS_LINE_3840X2160_24;
			break;
		case AUI_DIS_TVSYS_LINE_3840X2160_25:
			tvsys = DIS_TVSYS_LINE_3840X2160_25;
			break;
		case AUI_DIS_TVSYS_LINE_3840X2160_30:
			tvsys = DIS_TVSYS_LINE_3840X2160_30;
			break;
		case AUI_DIS_TVSYS_LINE_3840X2160_50:
			tvsys = DIS_TVSYS_LINE_3840X2160_50;
			break;
		case AUI_DIS_TVSYS_LINE_3840X2160_60:
			tvsys = DIS_TVSYS_LINE_3840X2160_60;
			break;
		case AUI_DIS_TVSYS_LINE_4096X2160_25:
			tvsys = DIS_TVSYS_LINE_4096X2160_25;
			break;
		case AUI_DIS_TVSYS_LINE_4096X2160_30:
			tvsys = DIS_TVSYS_LINE_4096X2160_30;
			break;
		case AUI_DIS_TVSYS_LINE_4096X2160_50:
			tvsys = DIS_TVSYS_LINE_4096X2160_50;
			break;
		case AUI_DIS_TVSYS_LINE_4096X2160_60:
			tvsys = DIS_TVSYS_LINE_4096X2160_60;
			break; 
    
		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate tvsys specified! \n");
	}
	sl_ret = alisldis_set_tvsys(dev_hdl->slhdl, tvsys, progressive);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n vpo_tv_system fail! \n");

	return rtn_code;
}


AUI_RTN_CODE aui_dis_set(void *pv_hdl_dis, enum aui_dis_item_set ul_item, void *pv_param)
{
    AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
    aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
    alisl_retcode   sl_ret;
    enum aui_dis_format fmt_dis = AUI_DIS_SD;

    if (NULL == pv_hdl_dis)
    aui_rtn(AUI_RTN_EINVAL, NULL);

    switch (ul_item) {
        case AUI_DIS_SET_AUTO_WINONOFF:
            if(pv_param == NULL)
                aui_rtn(AUI_RTN_EINVAL, NULL);

            sl_ret = alisldis_set_attr(dev_hdl->slhdl,
                        DIS_ATTR_DISAUTO_WIN_ONOFF,
                        *((uint32_t *)pv_param));
            if (DIS_ERR_NONE != sl_ret)
                aui_rtn(AUI_RTN_FAIL,
                    "\n Display set auto winonoff failed! \n");

            break;
            
        case AUI_DIS_SET_TVESDHD_SOURCE:
            fmt_dis = *((int *) pv_param);

            if(AUI_DIS_HD != fmt_dis && AUI_DIS_SD != fmt_dis){
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            sl_ret = alisldis_set_attr(dev_hdl->slhdl,
                        DIS_ATTR_TVESDHD_SOURCE_SEL,
                        (AUI_DIS_HD == fmt_dis)? 0 : 1); 

            if (DIS_ERR_NONE != sl_ret) 
                aui_rtn(AUI_RTN_FAIL,
                    "\n Display set auto winonoff failed! \n");

            break;

        case AUI_DIS_SET_CGMS: {
            struct dis_cgms_aps_info dis_cgms;
            
            if(pv_param == NULL)
                aui_rtn(AUI_RTN_EINVAL, NULL);
                
            memset(&dis_cgms, 0, sizeof(dis_cgms));
            dis_cgms.aps = dev_hdl->aps;
            dis_cgms.cgms_a = *((unsigned int *)pv_param);
            sl_ret = alisldis_set_cgms_aps(dev_hdl->slhdl, &dis_cgms);

            if (DIS_ERR_NONE != sl_ret) 
                aui_rtn(AUI_RTN_FAIL,
                    "\n Display set CGMS_A failed! \n");                   

            break;
        }

        case AUI_DIS_SET_APS: {
            struct dis_cgms_aps_info dis_cgms;
            
            if(pv_param == NULL)
                aui_rtn(AUI_RTN_EINVAL, NULL);

            memset(&dis_cgms, 0, sizeof(dis_cgms));
            dis_cgms.aps = *((unsigned int *)pv_param);
            dis_cgms.cgms_a = dev_hdl->cgms_a;
            sl_ret = alisldis_set_cgms_aps(dev_hdl->slhdl, &dis_cgms);

            if (DIS_ERR_NONE != sl_ret) 
                aui_rtn(AUI_RTN_FAIL,
                    "\n Display set APS failed! \n");                   
            break;  
        }
        
        default:
            aui_rtn(AUI_RTN_EINVAL,
                "\n Invalidate attribute value! \n");
    }

    return rtn_code;
}

AUI_RTN_CODE aui_dis_get(void *pv_hdl_dis, enum aui_dis_item_get ul_item, void *pv_param)
{
    AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
    aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);

    struct aui_dis_info *st_dis_info = (struct aui_dis_info *)pv_param;
    struct dis_info      dis_info;
    alisl_retcode        sl_ret;
    enum dis_display_mode display_match_mode = DIS_DM_PANSCAN;
    enum dis_aspect_ratio dis_aspect_ratio = DIS_AR_AUTO;
    
    if (NULL == pv_hdl_dis)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    switch(ul_item) {
        case AUI_DIS_GET_INFO: {
            struct dis_rect dst_rect = {0,0,1280,720}, src_rect = {0,0,1280,720};

            if (pv_param == NULL)
                aui_rtn(AUI_RTN_EINVAL, NULL);

            sl_ret = alisldis_get_mp_info(dev_hdl->slhdl, &dis_info);
            if (DIS_ERR_NONE != sl_ret)
                aui_rtn(AUI_RTN_FAIL,
                    "\n Display get display info failed! \n");
            st_dis_info->bprogressive  = dis_info.progressive;
            st_dis_info->des_height    = dis_info.dst_height;
            st_dis_info->des_width     = dis_info.dst_width;
            st_dis_info->gma1_onoff    = dis_info.gma1_onoff;
            st_dis_info->source_height = dis_info.source_height;
            st_dis_info->source_width  = dis_info.source_width;
            st_dis_info->tvsys         = to_aui_tvsys(dis_info.tvsys);
            
            alisldis_get_display_mode(dev_hdl->slhdl, &display_match_mode);            
            switch(display_match_mode) {
                case DIS_DM_PANSCAN:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PANSCAN;
                    break;
                case DIS_DM_PANSCAN_NOLINEAR:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PANSCAN_NOLINEAR;
                    break;
                case DIS_DM_LETTERBOX:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_LETTERBOX;
                    break;
                case DIS_DM_TWOSPEED:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_TWOSPEED;
                    break; 
                case DIS_DM_PILLBOX:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PILLBOX;
                    break;
                case DIS_DM_VERTICALCUT:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_VERTICALCUT;
                    break;
                case DIS_DM_NORMAL_SCALE:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_NORMAL_SCALE;
                    break;
                case DIS_DM_LETTERBOX149:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_LETTERBOX149;
                    break;
                case DIS_DM_AFDZOOM:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_AFDZOOM;
                    break;
                case DIS_DM_PANSCAN43ON169:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_PANSCAN43ON169;
                    break;      
                case DIS_DM_COMBINED_SCALE:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_COMBINED_SCALE;
                    break;
                case DIS_DM_IGNORE:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_IGNORE;
                    break;
                case DIS_DM_VERTICALCUT_149:
                    st_dis_info->dis_match_mode = AUI_DIS_MM_VERTICALCUT_149;
                    break;
                default:
                    aui_rtn(AUI_RTN_EINVAL,"invalid match mode!");
                    break;
            }
            
            alisldis_get_aspect(dev_hdl->slhdl, &dis_aspect_ratio);
            if(dis_aspect_ratio == DIS_AR_16_9)
                st_dis_info->dis_aspect_ratio = AUI_DIS_AP_16_9;
            else if(dis_aspect_ratio == DIS_AR_4_3)
                st_dis_info->dis_aspect_ratio = AUI_DIS_AP_4_3;
            else if(dis_aspect_ratio == DIS_AR_AUTO)
                st_dis_info->dis_aspect_ratio = AUI_DIS_AP_AUTO;
            else
                aui_rtn(AUI_RTN_EINVAL,"invalid aspect ratio!");
    

            if (alisldis_get_mp_source_rect(dev_hdl->slhdl, &src_rect) 
                || st_dis_info->source_width == 0 || st_dis_info->source_height == 0) {
                st_dis_info->src_rect.ui_startX = 0;
                st_dis_info->src_rect.ui_startY = 0;
                st_dis_info->src_rect.ui_width = 720;
                st_dis_info->src_rect.ui_height = 2880;
            } else {
                st_dis_info->src_rect.ui_startX = src_rect.x * 720 /st_dis_info->source_width;
                st_dis_info->src_rect.ui_startY = src_rect.y * 2880 /st_dis_info->source_height;
                st_dis_info->src_rect.ui_width = src_rect.w * 720 /st_dis_info->source_width;
                st_dis_info->src_rect.ui_height = src_rect.h * 2880 /st_dis_info->source_height;
            }

            if (alisldis_get_mp_screen_rect(dev_hdl->slhdl, &dst_rect)
                || st_dis_info->des_width == 0 || st_dis_info->des_height == 0) {
                st_dis_info->dst_rect.ui_startX = 0;
                st_dis_info->dst_rect.ui_startY = 0;
                st_dis_info->dst_rect.ui_width = 720;
                st_dis_info->dst_rect.ui_height = 2880;
            } else {
                st_dis_info->dst_rect.ui_startX = dst_rect.x * 720 /st_dis_info->des_width;
                st_dis_info->dst_rect.ui_startY = dst_rect.y * 2880 /st_dis_info->des_height;
                st_dis_info->dst_rect.ui_width = dst_rect.w * 720 /st_dis_info->des_width;
                st_dis_info->dst_rect.ui_height = dst_rect.h * 2880 /st_dis_info->des_height;
            }        

            break;
        }
        case AUI_DIS_GET_BOOT_MEDIA_STATUS:
            if (pv_param == NULL || dev_hdl->attr_dis.uc_dev_idx != AUI_DIS_HD)
                aui_rtn(AUI_RTN_EINVAL, "pv_param is NULL or handle is not DEN\n");
            if(DIS_ERR_NONE != alisldis_get_boot_media_status(dev_hdl->slhdl, (int *)pv_param))			
                aui_rtn(AUI_RTN_EINVAL, "Get boot media status fail!\n");
            break;

        default:
            aui_rtn(AUI_RTN_EINVAL,
                "\n Invalidate item to get value \n");
    }

    return rtn_code;
}

/* this interface is only useful for mpeg2 */
AUI_RTN_CODE
aui_dis_capture_pic(void            *pv_hdl_dis,
					aui_capture_pic *p_cap_param)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	unsigned char need_close = 0;
	alisl_handle sl_vdec = NULL;
	struct vdec_frame_cap cap;

	if ((p_cap_param == NULL) || (NULL == pv_hdl_dis))
		aui_rtn(AUI_RTN_EINVAL, NULL);

	if (aui_decv_get_current_opened_sl_handle(&sl_vdec)) {
		need_close = 1;
		//open video 0 as default
		SL_ASSERT(alislvdec_open(&sl_vdec, VDEC_ID_VIDEO_0));
	}	

	cap.buf_addr = p_cap_param->puc_out_data_buf;
	cap.buf_sz   = p_cap_param->ui_out_data_buf_size;

	alisl_retcode sl_ret = alislvdec_captrue_frame(sl_vdec, &cap);

	if (need_close) {
		SL_ASSERT(alislvdec_close(sl_vdec)); /* close device after use it */
	}

	if (0 == sl_ret) {
		p_cap_param->ui_pic_width      = cap.cap_pic_width;
		p_cap_param->ui_pic_height     = cap.cap_pic_height;
		p_cap_param->ui_out_size_valid = cap.cap_buf_sz;
	} else
		aui_rtn(AUI_RTN_FAIL, "\n vpo_capture_pic fail! \n");

	return rtn_code;
}


static enum dis_dac_id
dis_dac_id(uint32_t dac_index)
{
	switch (dac_index) {
		case 0:
				return DIS_DAC1;
		case 1:
			return DIS_DAC2;
		case 2:
			return DIS_DAC3;
		case 3:
			return DIS_DAC4;
		case 4:
			return DIS_DAC5;
		case 5:
			return DIS_DAC6;
		default:
			return (enum dis_dac_id) - 1;
	}
}

AUI_RTN_CODE
aui_dis_dac_reg(void         *pv_hdl_dis,
				unsigned int *pui_dac_attr,
				unsigned int  ui_ary_num)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	unsigned int i;
	unsigned int ty, tu, tv;
	unsigned int tr, tg, tb;
	unsigned int tsy, tsc;
	unsigned int tcvbs;
	struct {
		unsigned long y;
		unsigned long u;
		unsigned long v;
	} stYUV[2];
	struct {
		unsigned long r;
		unsigned long g;
		unsigned long b;
	} stRGB[2];
	struct {
		unsigned long sy;
		unsigned long sc;
	} stSVIDEO[6];
	unsigned long stCVBS[6];

	struct dis_dac_group dac_grp;

	/* TODO */
	unsigned char b_pro;

	ty = tu = tv = tr = tg = tb = tsy = tsc = tcvbs = 0;
	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	b_pro = (((aui_handle_dis *)(pv_hdl_dis))->attr_dis).b_dac_progressive;

	for (i = 0; i < ui_ary_num; i++) {
		if (AUI_DIS_TYPE_Y == pui_dac_attr[i]) {
			stYUV[ty].y = i;
			if (++ty > MAX_YUV_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if(AUI_DIS_TYPE_U == pui_dac_attr[i]) {
			stYUV[tu].u = i;
			if (++tu > MAX_YUV_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_V == pui_dac_attr[i]) {
			stYUV[tv].v = i;
			if (++tv > MAX_YUV_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_R == pui_dac_attr[i]) {
			stRGB[tr].r = i;
			if (++tr > MAX_RGB_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_G == pui_dac_attr[i]) {
			stRGB[tg].g = i;
			if (++tg > MAX_RGB_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_B == pui_dac_attr[i]) {
			stRGB[tb].b = i;
			if (++tb > MAX_RGB_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_SVIDEO_Y == pui_dac_attr[i]) {
			stSVIDEO[tsy].sy = i;
			if(++tsy > MAX_SVIDEO_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_SVIDEO_C == pui_dac_attr[i]) {
			stSVIDEO[tsc].sc = i;
			if (++tsc > MAX_SVIDEO_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		} else if (AUI_DIS_TYPE_CVBS == pui_dac_attr[i]) {
			stCVBS[tcvbs] = i;
			if (++tcvbs > MAX_CVBS_NUM - 1) {
				aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
			}
		}
	}

	if (ty != tv || tu != tv) {
		aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
	}

	if (tr != tb || tg != tb) {
		aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
	}

	if (tsc != tsy) {
		aui_rtn(AUI_RTN_FAIL, "\n Wrong parameter \n");
	}

	if (ty) {
		for (i = 0; i < ty; i++) {
			memset(&dac_grp, 0, sizeof(dac_grp));
			if (i == 0) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_YUV1;
			} else {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_YUV2;
			}

			dac_grp.yuv.y = dis_dac_id(stYUV[i].y);
			dac_grp.yuv.u = dis_dac_id(stYUV[i].u);
			dac_grp.yuv.v = dis_dac_id(stYUV[i].v);
			if (DIS_ERR_NONE != alisldis_reg_dac(dev_hdl->slhdl, &dac_grp, b_pro))
				aui_rtn(AUI_RTN_FAIL, "\n Display register dac failed!! \n");
		}
	}

	if (tr) {
		for (i = 0; i < tr; i++) {
			memset(&dac_grp, 0, sizeof(dac_grp));
			if (i == 0) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_RGB1;
			} else {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_RGB2;
			}

			dac_grp.rgb.r = dis_dac_id(stRGB[i].r);
			dac_grp.rgb.g = dis_dac_id(stRGB[i].g);
			dac_grp.rgb.b = dis_dac_id(stRGB[i].b);

			if (DIS_ERR_NONE != alisldis_reg_dac(dev_hdl->slhdl, &dac_grp, b_pro)) {
				aui_rtn(AUI_RTN_FAIL, "\n Display register dac failed!! \n");
			}
		}
	}

	if (tsc) {
		for (i = 0; i < tsc; i++) {
			memset(&dac_grp, 0, sizeof(dac_grp));

			dac_grp.dac_grp_type = DIS_DAC_GROUP_SVIDEO1;
			dac_grp.svideo.y = dis_dac_id(stSVIDEO[i].sy);
			dac_grp.svideo.c = dis_dac_id(stSVIDEO[i].sc);
			if (DIS_ERR_NONE != alisldis_reg_dac(dev_hdl->slhdl, &dac_grp, b_pro)) {
				aui_rtn(AUI_RTN_FAIL, "\n Display register dac failed!! \n");
			}
		}
	}

	if (tcvbs) {
		for (i = 0; i < tcvbs; i++) {
			memset(&dac_grp, 0, sizeof(dac_grp));
			if (i == 0) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS1;
			} else if (1 == i) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS2;
			} else if (2 == i) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS3;
			} else if (3 == i) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS4;
			} else if (4 == i) {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS5;
			} else {
				dac_grp.dac_grp_type = DIS_DAC_GROUP_CVBS6;
			}

			dac_grp.cvbs = dis_dac_id(stCVBS[i]);
			if (DIS_ERR_NONE != alisldis_reg_dac(dev_hdl->slhdl, &dac_grp, b_pro)) {
				aui_rtn(AUI_RTN_FAIL, "\n Display register dac failed!! \n");
			}
		}
	}

	return rtn_code;
}


AUI_RTN_CODE
aui_dis_dac_unreg(void                   *pv_hdl_dis,
				  enum aui_dis_unreg_type  type)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	enum dis_dac_group_type dac_type;

	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	switch (type) {
		case AUI_DIS_TYPE_UNREG_YUV    :
			dac_type = DIS_DAC_GROUP_YUV1;
			break;
		case AUI_DIS_TYPE_UNREG_RGB    :
			dac_type = DIS_DAC_GROUP_RGB1;
			break;
		case AUI_DIS_TYPE_UNREG_SVIDEO :
			dac_type = DIS_DAC_GROUP_SVIDEO1;
			break;
		case AUI_DIS_TYPE_UNREG_CVBS   :
			dac_type = DIS_DAC_GROUP_CVBS1;
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate DAC type specified! \n");
	}

	if (DIS_ERR_NONE != alisldis_unreg_dac(dev_hdl->slhdl, dac_type)) {
		aui_rtn(AUI_RTN_FAIL, "\n Share lib unreg DAC failed! \n");
	}

	return rtn_code;
}

static AUI_RTN_CODE
aui_dis_switch_view(alisl_handle  dis_dev,
					   alisl_handle  vdec_dev,
					   enum dis_layer  sl_layer,
					   enum vdec_out_mode mode, 
					   struct dis_rect     *src_rect,
					   struct dis_rect     *dst_rect)
{
    struct vdec_info       vdec_info;
    struct vdec_display_setting display_setting;
    enum vdec_dis_layer layer = VDEC_DIS_MAIN_LAYER;
    (void)dis_dev;

    //fix bug#86697,when play MPEG stream ,preview(scale to nearly full ) -->preview(scale to 1/8 size) will display corrupt  
    if (mode == VDEC_OUT_PREVIEW) {
        if ((dst_rect->w < src_rect->w/2) || (dst_rect->h < src_rect->h/2)) {
            display_setting.out_mode = VDEC_OUT_PREVIEW;
        } else {
            display_setting.out_mode = VDEC_OUT_FULLVIEW;
        }
    } else {
        display_setting.out_mode = mode;
    }

    display_setting.srect.x = src_rect->x;
    display_setting.srect.y = src_rect->y;
    display_setting.srect.w = src_rect->w;
    display_setting.srect.h = src_rect->h;
    display_setting.drect.x = dst_rect->x;
    display_setting.drect.y = dst_rect->y;
    display_setting.drect.w = dst_rect->w;
    display_setting.drect.h = dst_rect->h;

    if (sl_layer == DIS_LAYER_AUXP) {
        layer = VDEC_DIS_AUX_LAYER;
    } 

    /* don't call DE zoom for bug #86697, it may bring DE underrun
     * or video display quality
     */
    if (vdec_dev == NULL) {
        alislvdec_store_display_rect(layer, &display_setting);
        return AUI_RTN_SUCCESS;//maybe just for init, return success
    }

    if (alislvdec_get_info(vdec_dev, &vdec_info)) {//get info error, may no video playing
        alislvdec_store_display_rect(layer, &display_setting); 
        return AUI_RTN_SUCCESS;//maybe just for init, return success
    }

    /* the function will store display rect, all video type support this API right now
     *
     * there are two cases this API will return fail
     * (1) no video decoder is using
     * (2) when the video is decoding, swich mode will return fail if the last mode 
     *     didn't finish switching mode because of no enough data
     *     (need new P and I frame to implement deview)
     *
     *     If this API return fail, when video decoder starts to decode or get enough data
     * to implement the mode switching, it will display the video frame in the right
     * rect with this setting even though it returns fail.
     *     So we don't return fail here.
     */
    alislvdec_set_display_mode(vdec_dev, &display_setting);

    /*
     *  Try to make preview mode switch really take effect.
     *  For switching done we need to wait new I and P frame 
     *  to implement deview
     *
     *  For bug #86697 we cannot call DE zoom , 
     *  it may bring DE underrun or video display quality.
     * 
     *  Beacause we don't call DE zoom in this API right now, 
     *  so the SD video needs to be checked too.
     */
    bool mode_switch_need_time = true;
    int wait_time = 0;
    unsigned int last_decoded = 0;
    unsigned int last_buffer_valid_size = 0;
    unsigned int no_data_times = 0; //it means wait 500 ms if no data in video decoder

    if (vdec_info.state != VDEC_STATE_STARTED)
        mode_switch_need_time = false;

    if (mode_switch_need_time) {
        while (wait_time < VE_MODE_SWITCH_TIME 
                && no_data_times < VE_MODE_SWITCH_NO_DATA_TIME) {
            SL_ASSERT(alislvdec_get_info(vdec_dev, &vdec_info));

            //switch OK
            if (vdec_info.rect_switch_done) {
                pthread_mutex_unlock(&m_dis_mutex);
                usleep(50*1000);//wait 50ms for DE to handle switch done
                pthread_mutex_lock(&m_dis_mutex);
                break;
            }

            /* no more frame decorded
             * vbv or sbm buffer size didn't change
             * it may be no data or something wrong
             * for this case we only wait 500ms
             * Note:
             * one abnormal case cannot cover:
             * the video decorder is waiting avsync during checking
             */
            if ((last_decoded != 0 && vdec_info.frames_decoded == last_decoded)
                    || (last_buffer_valid_size != 0 && 
                vdec_info.buffer_used == last_buffer_valid_size)) {
                no_data_times += 100;
            } else {
                no_data_times = 0;
            }

            last_decoded = vdec_info.frames_decoded;
            last_buffer_valid_size = vdec_info.buffer_used;

            /*
             * fix bug from bugdetective: Do not use blocking functions while holding a lock
             */
            pthread_mutex_unlock(&m_dis_mutex);
            usleep(100*1000);
            pthread_mutex_lock(&m_dis_mutex);
            wait_time += 100;
        } 
    }

    return AUI_RTN_SUCCESS;
}

/* HD preview zoom: only for live playing*/
AUI_RTN_CODE
aui_dis_mode_set(void              *pv_hdl_dis,
				 enum aui_view_mode   mode,
				 aui_dis_zoom_rect *pst_src_rect,
				 aui_dis_zoom_rect *pst_dst_rect)
{
    AUI_RTN_CODE     rtn_code = AUI_RTN_SUCCESS;
    aui_handle_dis  *dev_hdl = NULL;
    struct dis_rect  srect={0, 0, 720, 2880}, drect={0, 0, 720, 2880};
    alisl_handle     vdec_dev = NULL;
    unsigned char need_close = 0;

    if (!pv_hdl_dis || mode < AUI_VIEW_MODE_PREVIEW || mode > AUI_VIEW_MODE_FULL) {
	    aui_rtn(AUI_RTN_EINVAL, "\n vpo mode set handle null! \n");
    }    


    dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
    
    if (aui_decv_get_current_opened_sl_handle(&vdec_dev)) {
        need_close = 1;
        //open video 0 as default
        SL_ASSERT(alislvdec_open(&vdec_dev, VDEC_ID_VIDEO_0));
    }

    pthread_mutex_lock(&m_dis_mutex);

    switch (mode) {
        case AUI_VIEW_MODE_PREVIEW: {
            if ((!pst_src_rect) || (!pst_dst_rect)) {
                AUI_ERR("src or dst rectangle is invalidate! \n");
                rtn_code = AUI_RTN_EINVAL;
                break;
            }

            srect.x = pst_src_rect->ui_startX;
            srect.y = pst_src_rect->ui_startY;
            srect.w = pst_src_rect->ui_width;
            srect.h = pst_src_rect->ui_height;

            drect.x = pst_dst_rect->ui_startX;
            drect.y = pst_dst_rect->ui_startY;
            drect.w = pst_dst_rect->ui_width;
            drect.h = pst_dst_rect->ui_height;

            rtn_code = aui_dis_switch_view(dev_hdl->slhdl, vdec_dev, DIS_LAYER_MAIN, 
                    VDEC_OUT_PREVIEW, &srect, &drect);
            break;
        }

        case AUI_VIEW_MODE_MULTI: {
            AUI_ERR("MODE_MULTI not implemented yet! \n");
            rtn_code = AUI_RTN_FAIL;
            break;
        }
    
        case AUI_VIEW_MODE_FULL: {
            //select an area and display it in full screen, for example: src:(0,0,360,1440), dst:(0,0,720,2880)
            if (pst_src_rect != NULL) {
                srect.x = pst_src_rect->ui_startX;
                srect.y = pst_src_rect->ui_startY;
                srect.w = pst_src_rect->ui_width;
                srect.h = pst_src_rect->ui_height;
            }

            if (pst_dst_rect != NULL) {
                drect.x = pst_dst_rect->ui_startX;
                drect.y = pst_dst_rect->ui_startY;
                drect.w = pst_dst_rect->ui_width;
                drect.h = pst_dst_rect->ui_height;
            }
            
            rtn_code = aui_dis_switch_view(dev_hdl->slhdl,vdec_dev, DIS_LAYER_MAIN, 
                VDEC_OUT_FULLVIEW, &srect, &drect);
            break;    
        }

        default: {
            AUI_ERR("Invalidate mode! \n");
            rtn_code =  AUI_RTN_EINVAL;
            break;
        }
    }

    pthread_mutex_unlock(&m_dis_mutex);

    if (need_close) {
        SL_ASSERT(alislvdec_close(vdec_dev));
    }
    
    return rtn_code;
}

AUI_RTN_CODE aui_dis_layer_order(aui_hdl pv_hdl_dis, enum aui_dis_layer_blend_order order)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;

    if (NULL == pv_hdl_dis || order < AUI_DIS_LAYER_ORDER_MP_GMAS_GMAF_AUXP || order > AUI_DIS_LAYER_ORDER_AUXP_MP_GMAF_GMAS) {
            aui_rtn(AUI_RTN_EINVAL, NULL);
    }

	if (alisldis_set_fb_order(p_dis->slhdl, order) != DIS_ERR_NONE) {
		rtn_code = AUI_RTN_FAIL;
	}

	return rtn_code;
}

AUI_RTN_CODE aui_dis_set_background_color(aui_hdl pv_hdl_dis, aui_dis_color * color)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
	struct dis_color dis_color;
	if (NULL == pv_hdl_dis || NULL == color)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	dis_color.y = color->y;
	dis_color.cb = color->cb;
	dis_color.cr = color->cr;
	
	if (alisldis_set_bgcolor(p_dis->slhdl, &dis_color) != DIS_ERR_NONE) {
		rtn_code = AUI_RTN_FAIL;
	}

	return rtn_code;
}

AUI_RTN_CODE aui_dis_3d_set(aui_hdl pv_hdl_dis, enum aui_dis_3D_mode dis_3d_mode)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
	struct dis_3d_attr sldis_3d_paras;
	enum dis_tvsys sl_dis_tvsys;
	bool progressive;

	if (NULL == pv_hdl_dis || dis_3d_mode >= AUI_DIS_3D_MODE_MAX || dis_3d_mode < AUI_DIS_3D_MODE_NONE)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	memset(&sldis_3d_paras, 0, sizeof(sldis_3d_paras));
	if(AUI_DIS_3D_MODE_NONE == dis_3d_mode)
	{
		if (alisldis_set_3d(p_dis->slhdl, &sldis_3d_paras) != DIS_ERR_NONE)
			aui_rtn(AUI_RTN_EINVAL, "AUI_DIS_3D_MODE_NONE error!\n");
	}
	else
	{
		sldis_3d_paras.display_3d_enable = 1;
			
		switch(dis_3d_mode)
		{
		case AUI_DIS_3D_MODE_SIDE_BY_SIDE:{
			sldis_3d_paras.eInputSource = DIS_INPUT_3D;
			
			sldis_3d_paras.display_2d_to_3d = 0;
			sldis_3d_paras.mode_2d_to_3d = 0;
			sldis_3d_paras.depth_2d_to_3d = 0;
			
			sldis_3d_paras.side_by_side = 1;//SIDE_BY_SIDE_HALF;
			sldis_3d_paras.top_and_bottom = 0;
			sldis_3d_paras.red_blue = 0;
			break;
		}
		case AUI_DIS_3D_MODE_TOP_AND_BOTTOM:{
			sldis_3d_paras.eInputSource = DIS_INPUT_3D;
			
			sldis_3d_paras.display_2d_to_3d = 0;
			sldis_3d_paras.mode_2d_to_3d = 0;
			sldis_3d_paras.depth_2d_to_3d = 0;
			
			sldis_3d_paras.side_by_side = 0;//SIDE_BY_SIDE_HALF;
			sldis_3d_paras.top_and_bottom = 1;
			sldis_3d_paras.red_blue = 0;
			break;
		}
		case AUI_DIS_3D_MODE_2D_TO_3D:{
			sldis_3d_paras.eInputSource = DIS_INPUT_2D;
			
			sldis_3d_paras.display_2d_to_3d = 1;
			sldis_3d_paras.mode_2d_to_3d = 0;
			sldis_3d_paras.depth_2d_to_3d = 0;
			
			sldis_3d_paras.side_by_side = 0;//SIDE_BY_SIDE_HALF;
			sldis_3d_paras.top_and_bottom = 0;
			sldis_3d_paras.red_blue = 0;
			break;
		}
		case AUI_DIS_3D_MODE_2D_TO_RED_BLUE:{
			sldis_3d_paras.eInputSource = DIS_INPUT_2D;
			
			sldis_3d_paras.display_2d_to_3d = 1;
			sldis_3d_paras.mode_2d_to_3d = 1;
			sldis_3d_paras.depth_2d_to_3d = 50;
			
			sldis_3d_paras.side_by_side = 0;//SIDE_BY_SIDE_HALF;
			sldis_3d_paras.top_and_bottom = 0;
			sldis_3d_paras.red_blue = 0;
			break;
		}
		default:{
			aui_rtn(AUI_RTN_EINVAL, "dis_3d_mode error!\n");
		}
		}
		
		if (alisldis_set_3d(p_dis->slhdl, &sldis_3d_paras) !=DIS_ERR_NONE) {
			aui_rtn(AUI_RTN_EINVAL, "alisldis_set_3d() error!\n");
		}
	}

	/*make 3D valid need set tvsys to flush*/
	if (alisldis_get_tvsys(p_dis->slhdl, &sl_dis_tvsys, &progressive) !=DIS_ERR_NONE) {
		aui_rtn(AUI_RTN_EINVAL, "alisldis_get_tvsys() error!\n");
	}

	if (alisldis_set_tvsys(p_dis->slhdl, sl_dis_tvsys, progressive) !=DIS_ERR_NONE) {
		aui_rtn(AUI_RTN_EINVAL, "alisldis_set_tvsys() error!\n");
	}

	return rtn_code;
}

AUI_RTN_CODE aui_dis_afd_set(aui_hdl pv_hdl_dis, aui_dis_afd_attr *dis_afd_para)
{
    aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
    struct dis_afd_attr sldis_afd_paras;
    alisl_handle vdec_dev = NULL;
    unsigned char need_close = 0;

    if (NULL == pv_hdl_dis || NULL == dis_afd_para)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    memset(&sldis_afd_paras, 0, sizeof(sldis_afd_paras));
    if (aui_decv_get_current_opened_sl_handle(&vdec_dev)) {
        need_close = 1;
        //open video 0 as default
        SL_ASSERT(alislvdec_open(&vdec_dev, VDEC_ID_VIDEO_0));
    }

    sldis_afd_paras.afd_solution = dis_afd_para->afd_spec;
    sldis_afd_paras.bSwscaleAfd = dis_afd_para->afd_mode;
    sldis_afd_paras.protect_mode_enable = dis_afd_para->afd_protect_enable;
    /*
    * There three ways to implement the match mode:
    * (1) SAR scale
    * (2) afd scale
    * (3) general scale
    * and the ways will be used in following conditions
    * (a) SAR
    *     SAR scale
    * (b) 4:3 or 16:9
    *     1) hw afd
    *         a) get match mode value 
    * (aui hw)->  i) scale mode = 0: afd value store in stream will affect the match
    *            ii) scale mode = 1: use match mode set by API
    *         b) general scale
    *     2) sf afd
    *         a) get match mode value 
    *  (aui sf)-> i) scale mode = 0: afd value store in stream will affect the match
    *            ii) scale mode = 1: use match mode set by API
    *         b) scale mode
    *  (aui sf)-> i) scale mode = 0: afd scale
    *            ii) scale mode = 1: general scale
    * 
    * now the set is:
    *    aui hw afd: SAR disable, scale mode=0, afd mode=0
    *    aui sf afd: SAR disable, scale mode=0, afd mode=1
    *    aui afd disable: SAR enable/disable, scale mode=1, afd mode=0
    * defaule value:
    *    SAR enable, scale mode=0, afd mode=0
    */
    if (dis_afd_para->afd_mode == 2) {
        sldis_afd_paras.bSwscaleAfd = 0;
        alisldis_set_afd_scale_mode(p_dis->slhdl, 1);
        //when disable afd, video can not parse afd field
        if (p_dis->sar_scale) {
            alislvdec_set_parse_afd(vdec_dev, 0);//default value
        } else {
            alislvdec_set_parse_afd(vdec_dev, 1);//disable sar scale
        }    
    } else {
        //when enable afd, need video parse afd field
        alislvdec_set_parse_afd(vdec_dev, 1);
        alisldis_set_afd_scale_mode(p_dis->slhdl, 0);
    }
    
    if (need_close) {
        SL_ASSERT(alislvdec_close(vdec_dev)); /* close device after use it */
    }
    
    if (alisldis_set_afd(p_dis->slhdl, &sldis_afd_paras) !=DIS_ERR_NONE)
        aui_rtn(AUI_RTN_EINVAL, "aui_dis_afd_set error!\n");

    return AUI_RTN_SUCCESS;
}

/*it can only be called when the display of video frame is still*/
AUI_RTN_CODE aui_dis_fill_black_screen(void *pv_hdl_dis)
{
    aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
    if (NULL == pv_hdl_dis)
        aui_rtn(AUI_RTN_EINVAL, NULL);
    alisldis_free_backup_picture(p_dis->slhdl);
    SL_ASSERT(alisldis_backup_picture(p_dis->slhdl, DIS_CHG_BLACK));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_layer_enable(aui_hdl pv_hdl_dis, aui_dis_layer layer, unsigned char b_enable)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	bool            onoff    = b_enable ? true : false;
	alisl_retcode   sl_ret;             /* share lib ret code */
	enum dis_layer  sl_layer = DIS_LAYER_MAIN;

	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_EINVAL, NULL);
	switch (layer) {
		case AUI_DIS_LAYER_VIDEO:
			sl_layer = DIS_LAYER_MAIN;
			break;
		case AUI_DIS_LAYER_AUXP: 
			sl_layer = DIS_LAYER_AUXP;
			break;
		case AUI_DIS_LAYER_OSD_0:
			sl_layer = DIS_LAYER_GMA1;
			break;
		case AUI_DIS_LAYER_OSD_1:
			sl_layer = DIS_LAYER_GMA2;
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL, NULL);
			break;
	}
	//AUI_DBG("%s: onoff: %d, sl_layer: %d\n", __func__, onoff, sl_layer);
	//not ready now
	//return rtn_code;
	sl_ret = alisldis_win_onoff_by_layer(dev_hdl->slhdl, onoff, sl_layer);
	if (DIS_ERR_NONE != sl_ret)
		aui_rtn(AUI_RTN_FAIL, "\n Winonff display failed! \n");
	
	return rtn_code;
}

static alisl_handle aui_dis_get_sl_decv_hdl_by_dis_layer(aui_dis_layer layer)
{
	int i = 0;
	alisl_handle sl_vdec_dev = NULL;	
	enum vdec_dis_layer sl_vdec_layer = VDEC_DIS_MAIN_LAYER;	
	for (i = 0; i < VDEC_NB_VIDEO; i++) {
		alislvdec_get_decoder_by_id((enum vdec_video_id)i, &sl_vdec_dev);
		if (sl_vdec_dev){
			alislvdec_get_display_layer(sl_vdec_dev, &sl_vdec_layer);
			if ((sl_vdec_layer == VDEC_DIS_MAIN_LAYER && layer == AUI_DIS_LAYER_VIDEO)
				|| (sl_vdec_layer == VDEC_DIS_AUX_LAYER && layer == AUI_DIS_LAYER_AUXP)) {
				break;
			} else {
				sl_vdec_dev = NULL;
			}
		}
	}
	return sl_vdec_dev;
}

AUI_RTN_CODE aui_dis_video_display_rect_set(aui_hdl pv_hdl_dis, aui_dis_layer layer, 
		aui_dis_zoom_rect *pst_src_rect, aui_dis_zoom_rect *pst_dst_rect)
{
    AUI_RTN_CODE     rtn_code = AUI_RTN_SUCCESS;
    aui_handle_dis  *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
    struct dis_rect  srect={0, 0, 720, 2880}, drect={0, 0, 720, 2880};
    enum aui_view_mode   mode = AUI_VIEW_MODE_FULL;
    alisl_handle     sl_vdec_dev = NULL;		
    enum dis_layer sl_dis_layer = DIS_LAYER_MAIN;


    if (NULL == pv_hdl_dis) {
        aui_rtn(AUI_RTN_FAIL, "\n vpo mode set handle null! \n");
    }    

    if (pst_src_rect == NULL || pst_dst_rect == NULL) {
        mode = AUI_VIEW_MODE_FULL;
    } else {
        if (pst_dst_rect->ui_height >= pst_src_rect->ui_height
            && pst_dst_rect->ui_width >= pst_src_rect->ui_width) {
            mode = AUI_VIEW_MODE_FULL;
        } else {
            mode = AUI_VIEW_MODE_PREVIEW;
        }
    }
    
    /* get decv sl handle , if NULL use dis zoom only*/
    sl_vdec_dev = aui_dis_get_sl_decv_hdl_by_dis_layer(layer);

    if (layer == AUI_DIS_LAYER_VIDEO) {
        sl_dis_layer = DIS_LAYER_MAIN;
    } else if (layer == AUI_DIS_LAYER_AUXP) {
        sl_dis_layer = DIS_LAYER_AUXP;
    }

    pthread_mutex_lock(&m_dis_mutex);

    switch (mode) {
        case AUI_VIEW_MODE_PREVIEW: {
            if ((!pst_src_rect) || (!pst_dst_rect)) {
                AUI_ERR("src/dst rectangle is invalidate! \n");
                rtn_code = AUI_RTN_FAIL;
                break;
            }

            srect.x = pst_src_rect->ui_startX;
            srect.y = pst_src_rect->ui_startY;
            srect.w = pst_src_rect->ui_width;
            srect.h = pst_src_rect->ui_height;

            drect.x = pst_dst_rect->ui_startX;
            drect.y = pst_dst_rect->ui_startY;
            drect.w = pst_dst_rect->ui_width;
            drect.h = pst_dst_rect->ui_height;

            rtn_code = aui_dis_switch_view(dev_hdl->slhdl, sl_vdec_dev, sl_dis_layer, 
                VDEC_OUT_PREVIEW, &srect, &drect);

            break;
        }	
        case AUI_VIEW_MODE_FULL: {
            //select an area and display it in full screen, for example: src:(0,0,360,1440), dst:(0,0,720,2880)
            if (pst_src_rect != NULL) {
                srect.x = pst_src_rect->ui_startX;
                srect.y = pst_src_rect->ui_startY;
                srect.w = pst_src_rect->ui_width;
                srect.h = pst_src_rect->ui_height;
            }

            if (pst_dst_rect != NULL) {
                drect.x = pst_dst_rect->ui_startX;
                drect.y = pst_dst_rect->ui_startY;
                drect.w = pst_dst_rect->ui_width;
                drect.h = pst_dst_rect->ui_height;
            }

            rtn_code = aui_dis_switch_view(dev_hdl->slhdl,sl_vdec_dev, sl_dis_layer, 
                VDEC_OUT_FULLVIEW, &srect, &drect);
            break;
        }
        default: {
            AUI_ERR("Invalidate mode! \n");
            rtn_code =  AUI_RTN_FAIL;
            break;
        }    
    }

    pthread_mutex_unlock(&m_dis_mutex);
    return rtn_code;
}

static void aui_dis_get_display_setting(struct vdec_display_setting *display_setting,
	aui_dis_zoom_rect *p_src_rect, aui_dis_zoom_rect *p_dst_rect)
{
	display_setting->srect.x = 0;
	display_setting->srect.y = 0;
	display_setting->srect.w = 720;
	display_setting->srect.h = 2880;
	display_setting->drect.x = 0;
	display_setting->drect.y = 0;
	display_setting->drect.w = 720;
	display_setting->drect.h = 2880;

	if (p_src_rect) {
		display_setting->srect.x = p_src_rect->ui_startX;
		display_setting->srect.y = p_src_rect->ui_startY;
		display_setting->srect.w = p_src_rect->ui_width;
		display_setting->srect.h = p_src_rect->ui_height;
	}
	if (p_dst_rect) {
		display_setting->drect.x = p_dst_rect->ui_startX;
		display_setting->drect.y = p_dst_rect->ui_startY;
		display_setting->drect.w = p_dst_rect->ui_width;
		display_setting->drect.h = p_dst_rect->ui_height;
	}

	if (display_setting->srect.x == display_setting->drect.x &&
		display_setting->srect.y == display_setting->drect.y &&
		display_setting->srect.w == display_setting->drect.w &&
		display_setting->srect.h == display_setting->drect.h) {
		display_setting->out_mode = VDEC_OUT_FULLVIEW;	
	} else {
		display_setting->out_mode = VDEC_OUT_PREVIEW;
	}
}

AUI_RTN_CODE aui_dis_video_attr_set(aui_hdl pv_hdl_dis, aui_dis_video_attr * p_main_video_attr, 
		aui_dis_video_attr * p_pip_video_attr)
{
	AUI_RTN_CODE     rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis  *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
	alisl_handle     sl_vdec_dev_main = NULL, sl_vdec_dev_aux = NULL;
	unsigned char exchanged = 0;
	struct vdec_info info;
	struct vdec_display_setting display_setting;
	if (NULL == pv_hdl_dis)
		aui_rtn(AUI_RTN_FAIL, "\n dis handle null! \n");
	if (p_main_video_attr == NULL && p_pip_video_attr == NULL)
		aui_rtn(AUI_RTN_FAIL, "\n dis set video attr handle null! \n");

	if (p_main_video_attr) {
		if (p_main_video_attr->video_source) {
			aui_decv_get_sl_handle(p_main_video_attr->video_source, &sl_vdec_dev_main);
		}
	}

	if (p_pip_video_attr) {
		if (p_pip_video_attr->video_source) {
			aui_decv_get_sl_handle(p_pip_video_attr->video_source, &sl_vdec_dev_aux);
		}
	}
	/* 1. exchange dis layer if needed
	 * 2. set dis layer to video 
	 * 3. set dis rect to video
	 */
	if (sl_vdec_dev_main) {
		memset(&info, 0, sizeof(struct vdec_info));
		alislvdec_get_info(sl_vdec_dev_main, &info);
		//AUI_DBG("main current layer: %d\n", info.layer);
		if (info.layer == VDEC_DIS_MAIN_LAYER) {
			exchanged = 0;
		} else {
			exchanged = 1;
			//exchange video layer
			//AUI_DBG("exchange layer 0\n");
			alisldis_exchange_video_layer(dev_hdl->slhdl);
			//set dis layer
			alislvdec_set_display_layer(sl_vdec_dev_main, VDEC_DIS_MAIN_LAYER);
		}
	}

	if (sl_vdec_dev_aux) {
		memset(&info, 0, sizeof(struct vdec_info));
		alislvdec_get_info(sl_vdec_dev_aux, &info);
		//AUI_DBG("aux current layer: %d\n", info.layer);
		if (info.layer == VDEC_DIS_AUX_LAYER) {
			exchanged = 0;
		} else {
			if (exchanged == 0) {
				//exchange video layer
				//AUI_DBG("exchange layer 1\n");
				alisldis_exchange_video_layer(dev_hdl->slhdl);
			}
			//set dis layer
			alislvdec_set_display_layer(sl_vdec_dev_aux, VDEC_DIS_AUX_LAYER);
		}
	}
	
	if (sl_vdec_dev_main) {
		memset(&display_setting, 0, sizeof(struct vdec_display_setting));
		aui_dis_get_display_setting(&display_setting, p_main_video_attr->p_src_rect, p_main_video_attr->p_dst_rect);
		alislvdec_set_display_mode(sl_vdec_dev_main, &display_setting);
	}

	if (sl_vdec_dev_aux) {
		memset(&display_setting, 0, sizeof(struct vdec_display_setting));
		aui_dis_get_display_setting(&display_setting, p_pip_video_attr->p_src_rect, p_pip_video_attr->p_dst_rect);
		alislvdec_set_display_mode(sl_vdec_dev_aux, &display_setting);
	}
	return rtn_code;
}

AUI_RTN_CODE 
aui_dis_cvbs_start(void *pv_hdl_dis)
{
    aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
    if (NULL == pv_hdl_dis)
        aui_rtn(AUI_RTN_EINVAL, NULL);
    if (p_dis->attr_dis.uc_dev_idx == AUI_DIS_HD)
    	aui_rtn(AUI_RTN_EINVAL, NULL);
    //enable auto onoff
    alisldis_set_attr(p_dis->slhdl, DIS_ATTR_DISAUTO_WIN_ONOFF, 0);
    //disable deo onoff control, can not control deo onoff furthermore
	alisldis_deo_control_onoff(p_dis->slhdl, 0);
	//open VPO
	alisldis_win_onoff_by_layer(p_dis->slhdl, 1, DIS_LAYER_MAIN);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE 
aui_dis_cvbs_stop(void *pv_hdl_dis)
{
    aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
    if (NULL == pv_hdl_dis)
        aui_rtn(AUI_RTN_EINVAL, NULL);
    if (p_dis->attr_dis.uc_dev_idx == AUI_DIS_HD)
    	aui_rtn(AUI_RTN_EINVAL, NULL);
    //disable auto onoff
    alisldis_set_attr(p_dis->slhdl, DIS_ATTR_DISAUTO_WIN_ONOFF, 1);
    //enable deo onoff control, or can not close deo
	alisldis_deo_control_onoff(p_dis->slhdl, 1);
	//close VPO
	alisldis_win_onoff_by_layer(p_dis->slhdl, 0, DIS_LAYER_MAIN);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_enhance_max_value_set(aui_hdl pv_hdl_dis, unsigned int max)
{
	AUI_RTN_CODE    rtn_code = AUI_RTN_SUCCESS;
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
    
    if ((NULL == pv_hdl_dis) || (0 == max) || (max > 0xFFFF))
        aui_rtn(AUI_RTN_EINVAL, NULL);

    alisldis_set_enhance_max_value(dev_hdl->slhdl, max);  
    
    return rtn_code;
}

AUI_RTN_CODE aui_dis_sar_scale_enable (aui_hdl pv_hdl_dis, unsigned char enable)
{
    aui_handle_dis *p_dis = (aui_handle_dis *)pv_hdl_dis;
    alisl_handle vdec_dev = NULL;
    unsigned char need_close = 0;

    if (aui_decv_get_current_opened_sl_handle(&vdec_dev)) {
        need_close = 1;
        //open video 0 as default
        SL_ASSERT(alislvdec_open(&vdec_dev, VDEC_ID_VIDEO_0));
    }		

    if (enable) {
        /* 
        * when disable afd, video can not parse afd field
        * for h264 video decoder will pass image info with SAR to DE, 
        * DE handle match mode with SAR info, different flow of AFD and Normal scale
        */
        alislvdec_set_parse_afd(vdec_dev, 0);
        p_dis->sar_scale = 1;
    } else {
        //when enable afd, need video parse afd field
        alislvdec_set_parse_afd(vdec_dev, 1);
        p_dis->sar_scale = 0;
    }

    if (need_close) {
        SL_ASSERT(alislvdec_close(vdec_dev)); /* close device after use it */
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dis_dac_reg_get(aui_hdl pv_hdl_dis,
                         unsigned int *pui_dac_attr,
                         unsigned int ui_ary_num) {
    
	aui_handle_dis *dev_hdl  = (aui_handle_dis *)(pv_hdl_dis);
    dis_dac_info dac_info; 
    unsigned int i;
    
    if ((NULL == pv_hdl_dis) || (NULL == pui_dac_attr))
        aui_rtn(AUI_RTN_EINVAL, NULL);

    //Use up to 4 dac
    if(ui_ary_num > 4) {
	    aui_rtn(AUI_RTN_EINVAL, "This value must be less than four\n");
	}

    memset(&dac_info, 0 ,sizeof(dis_dac_info));
    if (alisldis_get_dac_configure(dev_hdl->slhdl, &dac_info)) 
        aui_rtn(AUI_RTN_EINVAL, "alisldis_get_dac_configure Error!");

    for(i = 0; i < ui_ary_num;i++) {
        pui_dac_attr[i] = to_aui_dac_info(dac_info.dac_type[i]);
    }
    return AUI_RTN_SUCCESS;
}

