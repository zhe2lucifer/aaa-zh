/**  @file	   aui ge+osd module
*	 @brief 	input file breif description here
*	 @author	 andy.yu
*	 @date		   2013-6-18
*	 @version	  1.0.0
*	 @note		   ali corp. all rights reserved. 2013-2999 copyright (C)
*				 input file detail description here
*				 input file detail description here
*				 input file detail description here
*/
#include "aui_common_priv.h"

#include <aui_osd.h>
#include <hld/osd/osddrv.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>
#include <mediatypes.h>
#include <hld/hld_dev.h>
#include <aui_common.h>

AUI_MODULE(GFX)

/**
* the two function is implemented in /Elephant/src/hld/ge/ge_new.c
*/
extern RET_CODE ge_set_onoff_matrix(struct ge_device *dev, UINT32 cmd_pos, BOOL enble);
extern RET_CODE ge_set_scale_param(struct ge_device *dev, UINT32 cmd_pos, INT32 h_div, INT32 h_mul, INT32 v_div, INT32 v_mul);

#define ENABLE_BG_SCALE
/** color space type used in GE driver */
enum aui_ge_pixel_format_type_em {
	AUI_GE_PF_RGB565		= 0x00,
	AUI_GE_PF_RGB888		= 0x01,
	AUI_GE_PF_RGB555		= 0x02,
	AUI_GE_PF_RGB444		= 0x03,
	AUI_GE_PF_ARGB565		= 0x04,
	AUI_GE_PF_ARGB8888	= 0x05,
	AUI_GE_PF_ARGB1555	= 0x06,
	AUI_GE_PF_ARGB4444	= 0x07,
	AUI_GE_PF_CLUT1 	= 0x08,
	AUI_GE_PF_CLUT2 	= 0x09,
	AUI_GE_PF_CLUT4 	= 0x0A,
	AUI_GE_PF_CLUT8 	= 0x0B,
	AUI_GE_PF_ACLUT88		= 0x0C,
	AUI_GE_PF_YCBCR888	= 0x10,
	AUI_GE_PF_YCBCR422	= 0x12,
	AUI_GE_PF_YCBCR422MB	= 0x13,
	AUI_GE_PF_YCBCR420MB	= 0x14,
	AUI_GE_PF_AYCBCR8888	= 0x15,
	AUI_GE_PF_A1			= 0x18,
	AUI_GE_PF_A8			= 0x19,
	AUI_GE_PF_CK_CLUT2		 = 0x89,
	AUI_GE_PF_CK_CLUT4		 = 0x8A,
	AUI_GE_PF_CK_CLUT8		 = 0x8B,
	AUI_GE_PF_ABGR1555		 = 0x90,
	AUI_GE_PF_ABGR4444		 = 0x91,
	AUI_GE_PF_BGR565		   = 0x92,
	AUI_GE_PF_ACLUT44		 = 0x93,
	AUI_GE_PF_YCBCR444		 = 0x94,
	AUI_GE_PF_YCBCR420	= 0x95,
	AUI_GE_PF_MASK_A1		= AUI_GE_PF_A1,
	AUI_GE_PF_MASK_A8		= AUI_GE_PF_A8,
};

static OSAL_ID gfx_mutex = INVALID_ID;
#define MIN_ALIGN 64 //the min align size in byte
#define MIN_ALIGN_MASK (~0x3F) //  align  mask

#define MAX_OSD_LAYER 3

//#define aui_dbg_printf(a,b,c,d...) libc_printf(c,##d)
#define GFX_FUNCTION_ENTER AUI_INFO("enter\n");
#define GFX_FUNCTION_LEAVE AUI_INFO("leave\n");

#define GFX_LOCK(mutex)   osal_mutex_lock(mutex,OSAL_WAIT_FOREVER_TIME)
#define GFX_UNLOCK(mutex) osal_mutex_unlock(mutex)

#define GFX_MALLOC MALLOC
#define GFXF_FREE FREE
static struct ge_device *st_ge_device;
unsigned long aui_gfx_chip_id;
static unsigned long aui_layer_show_status[MAX_OSD_LAYER];
static unsigned long aui_layer_show_last_status[MAX_OSD_LAYER];

typedef unsigned long cmd_list_handle;

typedef struct gfx_layer_info
{
    aui_dev_priv_data dev_priv_data; /* struct for dev reg/unreg */
    OSAL_ID layer_mutex;
    struct osd_device*	 osd_dev;
    /**osd layer id*/
    unsigned long layer_id;
    /**osd layer open count.it will +1 when open,and will -1 when close*/
    unsigned long open_cnt;
    /**the numbers of hardware surface that have been create*/
    unsigned long hw_surface_cnt;
    /**osd layer show or not*/
    BOOL osd_layer_show_on_off;
    /**antifliker  flag,"1"means on,"0" means off*/
    BOOL antifliker_flag;
    /**global alpha value,"0" means transparent,"0xff" means not transparent*/
    unsigned long alpha;
    /**global alpha flag,"1"means on,"0" means off*/
    BOOL global_alpha_flag;
}gfx_layer_info;

typedef struct ge_private_info
{
	ge_cmd_list_hdl cmd_list_handle;
}ge_private_info;

typedef struct surface_list
{
	//surface link list table
	struct surface_list    *st_next_list;
	aui_surface_info st_surface_info;
	ge_private_info st_ge_private_info;
	OSAL_ID surface_mutex;
}surface_list;

typedef struct st_blit_base_info
{
	struct aui_osd_rect dst_rect;
	struct aui_osd_rect fg_rect;
	struct aui_osd_rect bg_rect;
	unsigned char* dst_buf;
	unsigned char* fg_buf;
	unsigned char* bg_buf;
	unsigned long dst_pitch;
	unsigned long fg_pitch;
	unsigned long bg_pitch;
	aui_color_key fg_color_key;
	aui_color_key bg_color_key;
	aui_color_key dst_color_key;
	enum aui_ge_clip_mode clip_mode;
	struct aui_osd_rect clip_ret;
	enum aui_ge_pixel_format_type_em dst_ge_color_mode;
	enum aui_ge_pixel_format_type_em fg_ge_color_mode;
	enum aui_ge_pixel_format_type_em bg_ge_color_mode;
	unsigned long fg_global_alpha;
	unsigned long bg_global_alpha;
}st_blit_base_info;

typedef enum st_draw_type
{
	DRAW_TYPE_FILL = 0,//only fill
	DRAW_TYPE_OUTLINE,	  //only draw the outline of rectangle
	DRAW_TYPE_OUTLINE_FILL,// draw the outline of  rectangle and fill the background color
}st_draw_type;

typedef struct fill_base_param
{
	st_draw_type draw_type;
	enum aui_ge_pixel_format_type_em color_mode;
	struct aui_osd_rect fill_rect;
	unsigned long pitch;
	unsigned char *buf;
	unsigned long fore_color;
	unsigned long back_color;
}fill_base_param;

surface_list	st_root_list;

//osd layer info
static gfx_layer_info st_gfx_layer_info[MAX_OSD_LAYER];// = {0};

static unsigned long get_pixel_per_line_by_pitch(unsigned long pitch,enum aui_ge_pixel_format_type_em ge_color_mode)
{
	unsigned long pixel_per_line = 0;

	switch(ge_color_mode)
	{
		case AUI_GE_PF_CLUT2:
			pixel_per_line = pitch << 2;
			break;
		case AUI_GE_PF_CLUT4:
			pixel_per_line = pitch << 1;
			break;
		case AUI_GE_PF_CLUT8:
			pixel_per_line = pitch;
			break;
		case AUI_GE_PF_RGB565:
		case AUI_GE_PF_ARGB1555:
		case AUI_GE_PF_ARGB4444:
			pixel_per_line = pitch >> 1;
			break;
		case AUI_GE_PF_ARGB8888:
		case AUI_GE_PF_AYCBCR8888:
			pixel_per_line = pitch >> 2;
			break;
		default:
			AUI_ERR("unknow color mode,mode=%d\n",ge_color_mode);
			break;
	}
	return pixel_per_line;
}

#ifdef ENABLE_BG_SCALE
static unsigned long get_bytes_per_line_by_format(unsigned long width,enum aui_ge_pixel_format_type_em ge_color_mode)
{
	unsigned long bytes_per_line = 0;

	switch(ge_color_mode)
	{
		case AUI_GE_PF_CLUT2:
			bytes_per_line = width >> 2;
			break;
		case AUI_GE_PF_CLUT4:
			bytes_per_line = width >> 1;
			break;
		case AUI_GE_PF_CLUT8:
			bytes_per_line = width;
			break;
		case AUI_GE_PF_RGB565:
		case AUI_GE_PF_ARGB1555:
		case AUI_GE_PF_ARGB4444:
			bytes_per_line = width << 1;
			break;
		case AUI_GE_PF_ARGB8888:
		case AUI_GE_PF_AYCBCR8888:
			bytes_per_line = width << 2;
			break;
		default:
			AUI_ERR("unknow color mode,mode=%d\n",ge_color_mode);
			break;
	}
	return bytes_per_line;
}
#endif

static void init_ge_base_addr(st_blit_base_info *blit_info,ge_base_addr_t *dst_base_addr,ge_base_addr_t *src_base_addr1,ge_base_addr_t *src_base_addr2)
{
	if(NULL != src_base_addr1)
	{
		src_base_addr1->color_format = blit_info->fg_ge_color_mode;
		src_base_addr1->base_address = (unsigned long)(blit_info->fg_buf);
		src_base_addr1->data_decoder = GE_DECODER_DISABLE;
		src_base_addr1->base_addr_sel = GE_BASE_ADDR;
		src_base_addr1->pixel_pitch = get_pixel_per_line_by_pitch(blit_info->fg_pitch,blit_info->fg_ge_color_mode);
		src_base_addr1->modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	}
	if(NULL != src_base_addr2)
	{
		src_base_addr2->color_format = blit_info->bg_ge_color_mode;
		src_base_addr2->base_address = (unsigned long)(blit_info->bg_buf);
		src_base_addr2->data_decoder = GE_DECODER_DISABLE;
		src_base_addr2->base_addr_sel = GE_BASE_ADDR;
		src_base_addr2->pixel_pitch = get_pixel_per_line_by_pitch(blit_info->bg_pitch,blit_info->bg_ge_color_mode);
		src_base_addr2->modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	}
	if(NULL != dst_base_addr)
	{
		dst_base_addr->color_format = blit_info->dst_ge_color_mode;
		dst_base_addr->base_address = (unsigned long)(blit_info->dst_buf);
		dst_base_addr->data_decoder = GE_DECODER_DISABLE;
		dst_base_addr->base_addr_sel = GE_BASE_ADDR;
		dst_base_addr->pixel_pitch = get_pixel_per_line_by_pitch(blit_info->dst_pitch,blit_info->dst_ge_color_mode);
		dst_base_addr->modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	}

}


//set clip mode and rect
static AUI_RTN_CODE set_clip_mode_rect(struct ge_device *ge_dev,st_blit_base_info *blit_info,unsigned long cmd_handle)
{
	AUI_RTN_CODE ret = SUCCESS;

	if(AUI_GE_CLIP_DISABLE != blit_info->clip_mode)
	{
		ret = ge_set_clip_mode(ge_dev,cmd_handle,blit_info->clip_mode);
		ret |= ge_set_clip_rect(ge_dev,cmd_handle,blit_info->clip_ret.uLeft,blit_info->clip_ret.uTop,\
								blit_info->clip_ret.uWidth,blit_info->clip_ret.uHeight);
	}
	return ret;
}

//set alpha out mode and value
static AUI_RTN_CODE set_alpha(struct ge_device *ge_dev,st_blit_base_info *blit_info,aui_blit_operation *blit_operation, unsigned long cmd_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	enum aui_alpha_out_mode alpha_out_mode;
	#define MAX_GLOBAL_ALPHA 255

	if((AUI_GFX_ROP_ALPHA_BLENDING == blit_operation->rop_operation) || (AUI_GFX_ROP_BOOL_ALPHA_BLENDIN == blit_operation->rop_operation))
	{
		alpha_out_mode = blit_operation->alpha_out_mode;
		switch(alpha_out_mode)
		{
			case AUI_ALPHA_OUT_NORMAL:
				ret = ge_set_global_alpha(ge_dev, cmd_handle, MAX_GLOBAL_ALPHA);
				break;
			case AUI_ALPHA_OUT_FG_GLOBAL_ALPHA:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_REG);
				ret |= ge_set_dst_alpha_out(ge_dev, cmd_handle, blit_info->fg_global_alpha);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, MAX_GLOBAL_ALPHA);
				break;
			case AUI_ALPHA_OUT_BG_GLOBAL_ALPHA:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_REG);
				ret |= ge_set_dst_alpha_out(ge_dev, cmd_handle, blit_info->bg_global_alpha);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, MAX_GLOBAL_ALPHA);
				break;
			case AUI_ALPHA_OUT_FG_PIXEL_ALPHA:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_PTN);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, MAX_GLOBAL_ALPHA);
				break;
			case AUI_ALPHA_OUT_BG_PIXEL_ALPHA:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_SRC);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, MAX_GLOBAL_ALPHA);
				break;
			case AUI_ALPHA_OUT_FG_GLOBAL_ALPHA_BLENDING:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_BLENDING);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, blit_info->fg_global_alpha);
				ret |= ge_set_global_alpha_layer(ge_dev, cmd_handle, GE_GALPHA_LAYER_PTN);
				break;
			case AUI_ALPHA_OUT_BG_GLOBAL_ALPHA_BLENDING:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_BLENDING);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, blit_info->bg_global_alpha);
				ret |= ge_set_global_alpha_layer(ge_dev, cmd_handle, GE_GALPHA_LAYER_SRC);
				break;
            case AUI_ALPHA_OUT_BLENDING_FG_GLOBAL_ONLY:
				ret = ge_set_alpha_out_mode(ge_dev,cmd_handle,GE_OUTALPHA_FROM_BLENDING);
				ret |= ge_set_global_alpha(ge_dev, cmd_handle, blit_info->fg_global_alpha);
				ret |= ge_set_global_alpha_layer(ge_dev, cmd_handle, GE_GALPHA_LAYER_PTN);
                ret |= ge_set_bitmap_alpha_mode(ge_dev,cmd_handle,GE_PTN,GE_BITMAP_PRE_MUL_ALPHA);
               break; 
			default:
				break;
		}
	}
	return ret;
}

#define AUI_ARGB1555_MASK 0xfff8f8f8
#define AUI_RGB565_MASK 0xfff8fcf8
#define AUI_ARGB4444_MASK 0xfff0f0f0
//set color key
static AUI_RTN_CODE set_color_key(struct ge_device *ge_dev,st_blit_base_info *blit_info,aui_blit_operation *blit_operation, unsigned long cmd_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
    unsigned long color_key_min = 0, color_key_max = 0;
    enum aui_ge_pixel_format_type_em ge_color_format = AUI_GE_PF_ARGB8888;

	if(AUI_GE_CKEY_DISABLE != blit_operation->color_key_source)
	{
		if(AUI_GE_CKEY_DST == blit_operation->color_key_source)
		{
            color_key_min = blit_info->dst_color_key.color_key_min;
            color_key_max = blit_info->dst_color_key.color_key_max;
            ge_color_format = blit_info->dst_ge_color_mode;
            ret = ge_set_rgb_expansion(ge_dev, cmd_handle, GE_SRC, GE_RGB_EXPAN_PAD0_TO_LSB);
		}
		else
		{
            color_key_min = blit_info->fg_color_key.color_key_min;
            color_key_max = blit_info->fg_color_key.color_key_max;
            ge_color_format =  blit_info->fg_ge_color_mode;
            ret = ge_set_rgb_expansion(ge_dev, cmd_handle, GE_PTN, GE_RGB_EXPAN_PAD0_TO_LSB);		
	    }
		ret |= ge_set_colorkey_mode(ge_dev, cmd_handle, blit_operation->color_key_source);
		ret |= ge_set_colorkey_match_mode(ge_dev, cmd_handle, GE_CKEY_CHANNEL_R, blit_operation->color_key_red_match_mode);
		ret |= ge_set_colorkey_match_mode(ge_dev, cmd_handle, GE_CKEY_CHANNEL_G, blit_operation->color_key_green_match_mode);
		ret |= ge_set_colorkey_match_mode(ge_dev, cmd_handle, GE_CKEY_CHANNEL_B, blit_operation->color_key_blue_match_mode);
        ret |= ge_set_colorkey_match_mode(ge_dev, cmd_handle, GE_CKEY_CHANNEL_A, AUI_GE_CKEY_MATCH_ALWAYS); 
        switch(ge_color_format)
        {
            case AUI_GE_PF_ARGB1555:
                {
                    color_key_min &= AUI_ARGB1555_MASK;
                    color_key_max &= AUI_ARGB1555_MASK;
                }
                break;
            case AUI_GE_PF_RGB565:
                {
                    color_key_min &= AUI_RGB565_MASK;
                    color_key_max &= AUI_RGB565_MASK;
                }
                break;
            case AUI_GE_PF_ARGB4444:
                {
                    color_key_min &= AUI_ARGB4444_MASK;
                    color_key_max &= AUI_ARGB4444_MASK;
                }
                break;
            default:
                break;            
        }
 
        ret |= ge_set_colorkey_range(ge_dev, cmd_handle, color_key_min, color_key_max);
    }    
	return ret;
}

static AUI_RTN_CODE set_scale_param(struct ge_device *ge_dev,unsigned long cmd_handle,struct aui_osd_rect *src_rect,struct aui_osd_rect *dst_rect)
{
	//ge_scale_info_t scale_info;
	//double matrix[6] = {0};
	AUI_RTN_CODE ret = SUCCESS;
	unsigned long src_width = src_rect->uWidth;
	unsigned long src_height = src_rect->uHeight;
	unsigned long dst_width = dst_rect->uWidth;
	unsigned long dst_height = dst_rect->uHeight;
	
#ifdef 	ALI_S3603
	if(ALI_S3603 == aui_gfx_chip_id)
	{
		scale_info.blend_info_en = 0;
		scale_info.blend_info_addr = 0;
		scale_info.color_premuled = 1;
		ret |= ge_set_scale_info(ge_dev, cmd_handle, &scale_info);
	}
	else
#endif	
    {
    	#if 1
    	if((0 != src_rect->uWidth) && (0 != src_rect->uHeight))
    	{
    		ret = ge_set_onoff_matrix(ge_dev, (unsigned long)cmd_handle, 1); 		
			ret = ge_set_scale_param(ge_dev,(unsigned long)cmd_handle,dst_width,src_width,dst_height,src_height);
    	}
    	#endif
	}
	return ret;
}
//color mode change between osd mode to ge color mode
static enum aui_ge_pixel_format_type_em osd_color_mode2ge(enum aui_osd_pixel_format en_color_mode)
{
	enum aui_ge_pixel_format_type_em ge_color_mode = AUI_GE_PF_ARGB1555;

	switch (en_color_mode)
	{
		case AUI_OSD_4_COLOR:
			ge_color_mode = AUI_GE_PF_CLUT2;
			break;
		case AUI_OSD_256_COLOR:
			ge_color_mode = AUI_GE_PF_CLUT8;
			break;
		case AUI_OSD_HD_ARGB1555:
			ge_color_mode = AUI_GE_PF_ARGB1555;
			break;
		case AUI_OSD_HD_RGB565:
			ge_color_mode = AUI_GE_PF_RGB565;
			break;
		case AUI_OSD_HD_ARGB4444:
			ge_color_mode = AUI_GE_PF_ARGB4444;
			break;
		case AUI_OSD_HD_AYCbCr8888:
			ge_color_mode = AUI_GE_PF_AYCBCR8888;
			break;
		case AUI_OSD_HD_ARGB8888:
			ge_color_mode = AUI_GE_PF_ARGB8888;
			break;
		default:
			break;
	}

	return ge_color_mode;
}

static unsigned long get_pitch(enum aui_osd_pixel_format color_mode, unsigned long width)
{
	unsigned long pitch = 0;

	switch (color_mode)
	{
		case AUI_OSD_256_COLOR:
			pitch = width;
			break;
		case AUI_OSD_HD_ARGB1555:
		case AUI_OSD_HD_RGB565:
		case AUI_OSD_HD_ARGB4444:
			pitch = width << 1;
			break;
		case AUI_OSD_HD_AYCbCr8888:
		case AUI_OSD_HD_ARGB8888:
			pitch = width << 2;
			break;
		default:
			pitch = width;
			break;
	}
	return pitch;
}

//get how many bit per pixel according to color mode
static unsigned long get_bpp_by_color_mode(enum aui_ge_pixel_format_type_em en_color_mode)
{
	unsigned long bpp = 0;

	switch(en_color_mode)
	{
		case AUI_GE_PF_CLUT2:
			bpp = 2;
			break;
		case AUI_GE_PF_CLUT4:
			bpp = 4;
			break;
		case AUI_GE_PF_CLUT8:
			bpp = 8;
			break;
		case AUI_GE_PF_RGB565:
		case AUI_GE_PF_ARGB1555:
		case AUI_GE_PF_ARGB4444:
			bpp = 16;
			break;
		case AUI_GE_PF_ARGB8888:
		case AUI_GE_PF_AYCBCR8888:
			bpp = 32;
			break;
		default:
			AUI_ERR("unknow color mode,mode=%d\n",en_color_mode);
			break;
	}
	return bpp;
}

static struct ge_device* get_ge_device()
{
	return st_ge_device;
}

//check input layer handle is valid
static BOOL layer_handle_check(gfx_layer_info *st_gfx_layer_handle)
{
	unsigned long i;

	for(i = 0;i < MAX_OSD_LAYER;i++)
	{
		if((&st_gfx_layer_info[i] == st_gfx_layer_handle ) && (st_gfx_layer_info[i].open_cnt > 0))
		{
			return TRUE;
		}
	}
	return FALSE;
}

//it will call when open the osd layer
static void incress_open_cnt(unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].open_cnt++;
}

#if 0
//it will call when close the osd layer
static void redeuce_open_cnt(unsigned long osd_layer_id)
{
	if(st_gfx_layer_info[osd_layer_id].open_cnt > 0)
	{
		st_gfx_layer_info[osd_layer_id].open_cnt--;
	}
}
#endif

static void set_layer_mutex(OSAL_ID layer_mutex,unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].layer_mutex = layer_mutex;
}

//set layer id
static void set_layer_id(unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].layer_id = osd_layer_id;

}

//incress hardware surface count,it will call when hardware surface create
static void incress_hwsurface_cnt(unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].hw_surface_cnt++;
}

//reduce hardware surface count,it will call when the hardware surface delete
static void redeuce_hw_surface_cnt(unsigned long osd_layer_id)
{
	if(st_gfx_layer_info[osd_layer_id].hw_surface_cnt > 0)
	{
		st_gfx_layer_info[osd_layer_id].hw_surface_cnt--;
	}
}

static BOOL check_all_layer_closed(void) 
{
    int i = 0;
     for (i = 0; i < MAX_OSD_LAYER; i++) {
        AUI_DBG("layer: %d, open_cnt: %d\n", i, st_gfx_layer_info[i].open_cnt);
        if (st_gfx_layer_info[i].open_cnt > 0)
            return false;
    }
    return true;
}

//get the st_gfx_layer_info by layer id
static	gfx_layer_info* get_layer_info(unsigned long osd_layer_id)
{
	gfx_layer_info* layer_info = NULL;

	layer_info = &st_gfx_layer_info[osd_layer_id];

	return layer_info;
}

#if 0
//reset layer info ,it will call where the osd layer close
static void reset_layer_info(unsigned long osd_layer_id)
{
	MEMSET(&st_gfx_layer_info[osd_layer_id],0x0,sizeof(st_gfx_layer_info[0]));
}
#endif

//add new surface list
static void surface_list_add(surface_list *new)
{
	surface_list *prev = NULL;

	GFX_LOCK(gfx_mutex);
	for(prev = &st_root_list;prev->st_next_list != NULL;prev = prev->st_next_list)
	{

	}
	prev->st_next_list = new;
	new->st_next_list = NULL;
	GFX_UNLOCK(gfx_mutex);
}

//delete  surface list
static void surface_list_delete(surface_list *list)
{
	surface_list *tmp_list;

	GFX_LOCK(gfx_mutex);
	for(tmp_list = &st_root_list;tmp_list->st_next_list != NULL;tmp_list = tmp_list->st_next_list)
	{
		if(tmp_list->st_next_list == list)
		{
			tmp_list->st_next_list = list->st_next_list;
			break;
		}
	}
	GFX_UNLOCK(gfx_mutex);
}

//check the given list if it is in the link list
static BOOL surface_list_check(surface_list *list)
{
	surface_list *tmp_list;
	BOOL ret = FALSE;

	for(tmp_list = (surface_list *)&st_root_list;tmp_list->st_next_list != NULL;tmp_list = tmp_list->st_next_list)
	{
		if(tmp_list->st_next_list == list)
		{
			ret = TRUE;
			break;
		}
	}
	return ret;
}

// delete hardware surface

static AUI_RTN_CODE delete_hw_surface(surface_list *st_hw_surface_list)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *layer_info = NULL;
	struct ge_device*	ge_device;
	unsigned long hw_surface_id;


	ge_device = get_ge_device();
	hw_surface_id = st_hw_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	layer_info = get_layer_info(st_hw_surface_list->st_surface_info.hw_surface_info.layer_id);

	//delete the osd region
	ret = ge_gma_delete_region(ge_device,layer_info->layer_id,hw_surface_id);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("ge_gma_delete_region failed \n");
		goto ERROR;
	}
	ret = ge_cmd_list_destroy(ge_device, st_hw_surface_list->st_ge_private_info.cmd_list_handle);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("ge_cmd_list_destroy\n");
		goto ERROR;
	}
	if(st_hw_surface_list->st_surface_info.hw_surface_info.isdouble_buf)
	{
		//free the malloc buffer
		if(st_hw_surface_list->st_surface_info.hw_surface_info.second_buf_backup)
		{
			GFXF_FREE(st_hw_surface_list->st_surface_info.hw_surface_info.second_buf_backup);
		}
	}

#if 0
	if(st_hw_surface_list->st_surface_info.p_surface_buf_backup)
	{
		GFXF_FREE(st_hw_surface_list->st_surface_info.p_surface_buf_backup);
	}
#endif
    
	//delete it from the link list
	redeuce_hw_surface_cnt(layer_info->layer_id);
    if(st_gfx_layer_info[layer_info->layer_id].hw_surface_cnt == 0)
    {   
        /* 
        Just reset the show status to the previous status.
        Because when we delete surface on layer whiout close layer, 
        and then create surface again, we need to open the layer in "aui_gfx_hw_surface_create".
        */
        aui_layer_show_status[layer_info->layer_id] = aui_layer_show_last_status[layer_info->layer_id];
        aui_layer_show_last_status[layer_info->layer_id] = 0xff;
    }	
	surface_list_delete(st_hw_surface_list);
	GFXF_FREE(st_hw_surface_list);
	st_hw_surface_list = NULL;
	return ret;
ERROR:
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		delete surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   handle
*	 @param[out]	NULL
*	 @return
*	 @note
*
*/
static AUI_RTN_CODE delete_sw_surface(surface_list *sw_surface_list)
{
	AUI_RTN_CODE ret = SUCCESS;
	struct ge_device*	ge_device;

    GFX_FUNCTION_ENTER;
	//delete ge cmd list
	ge_device = get_ge_device();
	ret = ge_cmd_list_destroy(ge_device, sw_surface_list->st_ge_private_info.cmd_list_handle);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("ge_cmd_list_destroy\n");
		goto ERROR;
	}
	if(sw_surface_list->st_surface_info.p_surface_buf_backup)
	{
		GFXF_FREE(sw_surface_list->st_surface_info.p_surface_buf_backup);
	}
	surface_list_delete(sw_surface_list);
	GFXF_FREE(sw_surface_list);
	sw_surface_list = NULL;
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

//blit data from one surface to another surface
static AUI_RTN_CODE do_surface_fill(fill_base_param *fill_param,cmd_list_handle cmd_list)
{
	struct ge_device* ge_dev;
	ge_base_addr_t dst_base_addr;

	unsigned long cmd_handle;
	AUI_RTN_CODE ret = SUCCESS;

	dst_base_addr.color_format = fill_param->color_mode;
	dst_base_addr.base_address = (unsigned long)(fill_param->buf);
	dst_base_addr.data_decoder = GE_DECODER_DISABLE;
	dst_base_addr.pixel_pitch = get_pixel_per_line_by_pitch(fill_param->pitch,fill_param->color_mode);
	dst_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

	ge_dev = get_ge_device();
	ret = ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
	if(DRAW_TYPE_FILL == fill_param->draw_type)
	{
		cmd_handle = ge_cmd_begin(ge_dev, cmd_list, GE_FILL_RECT_BACK_COLOR);
		ret = ge_set_back_color(ge_dev,cmd_handle,fill_param->back_color);
	}
	else if(DRAW_TYPE_OUTLINE == fill_param->draw_type)
	{
		cmd_handle = ge_cmd_begin(ge_dev, cmd_list, GE_DRAW_RECT_FRAME);
		ret = ge_set_draw_color(ge_dev,cmd_handle,fill_param->fore_color);
	}
	else
	{
		cmd_handle = ge_cmd_begin(ge_dev, cmd_list, GE_DRAW_RECT_FRAME_FILL);
		ret = ge_set_draw_color(ge_dev,cmd_handle,fill_param->fore_color);
		ret |= ge_set_back_color(ge_dev,cmd_handle,fill_param->back_color);
	}
	ret |= ge_set_base_addr(ge_dev, cmd_handle, GE_DST, &dst_base_addr);
	ret |= ge_set_color_format(ge_dev,cmd_handle,fill_param->color_mode);

	ret |= ge_set_wh(ge_dev, cmd_handle, GE_DST, fill_param->fill_rect.uWidth, fill_param->fill_rect.uHeight);
	ret |= ge_set_xy(ge_dev, cmd_handle, GE_DST, fill_param->fill_rect.uLeft, fill_param->fill_rect.uTop);
	ret |= ge_cmd_end(ge_dev, cmd_handle);
	ret |= ge_cmd_list_end(ge_dev,cmd_list);

	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
        AUI_ERR("fail!\n");
	}
	return ret;

}

//blit data from one surface to another surface
static AUI_RTN_CODE do_surface_blit(st_blit_base_info *blit_info,aui_blit_operation *blit_operation,cmd_list_handle cmd_list)
{
	struct ge_device* ge_dev;
	ge_base_addr_t dst_base_addr;
	ge_base_addr_t src_base_addr1;
	ge_base_addr_t src_base_addr2;
	unsigned long cmd_handle;
	enum GE_PRIMITIVE_MODE ge_operaton;
	BOOL direct_copy_flag;
	BOOL scale_flag;
#ifdef  ENABLE_BG_SCALE
    BOOL bg_scale_flag;
    unsigned long bg_pitch;
    unsigned char *bg_scale_buf_bak = NULL;
    unsigned char *bg_scale_buf;
#endif
	unsigned long hor_scan_type;//horizontal scan type
	unsigned long ver_scan_type;//vertical scan type
	AUI_RTN_CODE ret = SUCCESS;

	GFX_FUNCTION_ENTER;

	hor_scan_type = ((AUI_GFX_MIRROR_NORMAL == blit_operation->mirror_type) \
					|| (AUI_GFX_MIRROR_BOTTON_TOP == blit_operation->mirror_type))? GE_SCAN_LEFT_TO_RIGHT : GE_SCAN_RIGHT_TO_LEFT;
	ver_scan_type = ((AUI_GFX_MIRROR_NORMAL == blit_operation->mirror_type) \
					|| (AUI_GFX_MIRROR_RIGHT_LEFT == blit_operation->mirror_type))? GE_SCAN_TOP_TO_BOTTOM : GE_SCAN_BOTTOM_TO_TOP;
	scale_flag = ((blit_info->dst_rect.uWidth == blit_info->fg_rect.uWidth) \
					&& (blit_info->dst_rect.uHeight == blit_info->fg_rect.uHeight))? FALSE : TRUE;
#ifdef ENABLE_BG_SCALE
    bg_scale_flag = ((blit_info->dst_rect.uWidth == blit_info->bg_rect.uWidth) \
					&& (blit_info->dst_rect.uHeight == blit_info->bg_rect.uHeight))? FALSE : TRUE;
#endif
	direct_copy_flag = ((AUI_GFX_MIRROR_NORMAL == blit_operation->mirror_type) && (AUI_GE_CKEY_DISABLE == blit_operation->color_key_source) \
						&& (AUI_GFX_ROP_DERECT_COVER == blit_operation->rop_operation) && (TRUE != scale_flag))? TRUE : FALSE;

#ifdef 	ALI_S3603
	if((TRUE == scale_flag) && (ALI_S3603 == aui_gfx_chip_id))
	{
		ge_operaton = GE_PRIM_SCALING;
	}
	else
#endif
    {
    	if(TRUE == direct_copy_flag)
    	{
    		ge_operaton = GE_PRIM_DISABLE;
    	}
    	else if(AUI_GFX_ROP_DERECT_COVER == blit_operation->rop_operation)
    	{
    		ge_operaton = GE_DRAW_BITMAP;
    	}
    	else if(AUI_GFX_ROP_BOOLEAN == blit_operation->rop_operation)
    	{
    		ge_operaton = GE_BOOLEAN_OPERATION;
    	}
    	else if(AUI_GFX_ROP_BOOL_ALPHA_BLENDIN == blit_operation->rop_operation)
    	{
    		ge_operaton = GE_BOOLEAN_OPERATION_WITHOUT_ALPHA;
    	}
    	else
    	{
    		ge_operaton = GE_DRAW_BITMAP_ALPHA_BLENDING;
    	}
	}

	ge_dev = get_ge_device();

#ifdef ENABLE_BG_SCALE
    if((bg_scale_flag == TRUE)&&(blit_info->bg_rect.uWidth!=0)&&(blit_info->bg_rect.uHeight!=0))
    {
        bg_pitch = get_bytes_per_line_by_format(blit_info->dst_rect.uWidth, blit_info->bg_ge_color_mode);
        bg_scale_buf_bak = GFX_MALLOC(bg_pitch * blit_info->dst_rect.uHeight + MIN_ALIGN);

        if(bg_scale_buf_bak == NULL)
        {
            AUI_ERR("malloc fail!\n");
            ret = AUI_GFX_NO_MEMORY;
			GFX_FUNCTION_LEAVE;
            return ret;
        }
        
        bg_scale_buf = (unsigned char*)((((unsigned long)bg_scale_buf_bak + MIN_ALIGN)|(0xa<<28)) & MIN_ALIGN_MASK);

        src_base_addr2.color_format = blit_info->bg_ge_color_mode;
		src_base_addr2.base_address = (UINT32)blit_info->bg_buf;
		src_base_addr2.data_decoder = GE_DECODER_DISABLE;
		src_base_addr2.base_addr_sel = GE_BASE_ADDR;
		src_base_addr2.pixel_pitch = get_pixel_per_line_by_pitch(blit_info->bg_pitch,blit_info->bg_ge_color_mode);
		src_base_addr2.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
        
        dst_base_addr.color_format = blit_info->bg_ge_color_mode;
		dst_base_addr.base_address = (UINT32) bg_scale_buf;
		dst_base_addr.data_decoder = GE_DECODER_DISABLE;
		dst_base_addr.base_addr_sel = GE_BASE_ADDR;
		dst_base_addr.pixel_pitch = blit_info->dst_rect.uWidth;
		dst_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    	ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
    	cmd_handle = ge_cmd_begin(ge_dev, cmd_list, GE_DRAW_BITMAP);

        ret = ge_set_base_addr(ge_dev, cmd_handle, GE_PTN, &src_base_addr2);
		ret |= ge_set_wh(ge_dev, cmd_handle, GE_PTN, blit_info->bg_rect.uWidth, blit_info->bg_rect.uHeight);
		ret |= ge_set_xy(ge_dev, cmd_handle, GE_PTN, blit_info->bg_rect.uLeft, blit_info->bg_rect.uTop);        

    	ret |= ge_set_base_addr(ge_dev, cmd_handle, GE_DST, &dst_base_addr);
    	ret |= ge_set_wh(ge_dev, cmd_handle, GE_DST, blit_info->dst_rect.uWidth, blit_info->dst_rect.uHeight);
    	ret |= ge_set_xy(ge_dev, cmd_handle, GE_DST, 0, 0);      

		ret |= set_scale_param(ge_dev,cmd_handle,&(blit_info->bg_rect),&(blit_info->dst_rect));
	    ret |= ge_cmd_end(ge_dev, cmd_handle);        

        if(SUCCESS != ret)
        {
            AUI_ERR("bg scale fail!\n");
            ret = AUI_GFX_DRIVER_ERROR;
            GFXF_FREE(bg_scale_buf_bak);
			GFX_FUNCTION_LEAVE;
            return ret;
        }

        blit_info->bg_buf = bg_scale_buf;
        blit_info->bg_rect.uLeft = 0;
        blit_info->bg_rect.uTop = 0;
        blit_info->bg_pitch = bg_pitch;
        blit_info->bg_rect.uHeight = blit_info->dst_rect.uHeight;
        blit_info->bg_rect.uWidth = blit_info->dst_rect.uWidth;
    }
#endif    

	if((GE_DRAW_BITMAP_ALPHA_BLENDING == ge_operaton) || (GE_BOOLEAN_OPERATION == ge_operaton) \
		|| (GE_BOOLEAN_OPERATION_WITHOUT_ALPHA == ge_operaton))
	{
		init_ge_base_addr(blit_info,&dst_base_addr,&src_base_addr1,&src_base_addr2); //two source
	}
	else
	{
		init_ge_base_addr(blit_info,&dst_base_addr,&src_base_addr1,NULL); //only one source
	}

	ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
	cmd_handle = ge_cmd_begin(ge_dev, cmd_list, ge_operaton);

	if((GE_PRIM_DISABLE == ge_operaton) || (GE_PRIM_SCALING == ge_operaton))
	{
		ret = ge_set_base_addr(ge_dev, cmd_handle, GE_SRC, &src_base_addr1);
		ret |= ge_set_xy(ge_dev, cmd_handle, GE_SRC, blit_info->fg_rect.uLeft, blit_info->fg_rect.uTop);
		ret |= ge_set_wh(ge_dev, cmd_handle, GE_SRC, blit_info->fg_rect.uWidth, blit_info->fg_rect.uHeight);
	}
	else if(GE_DRAW_BITMAP == ge_operaton)
	{
		ret = ge_set_base_addr(ge_dev, cmd_handle, GE_PTN, &src_base_addr1);
		ret |= ge_set_xy(ge_dev, cmd_handle, GE_PTN, blit_info->fg_rect.uLeft, blit_info->fg_rect.uTop);
		ret |= ge_set_wh(ge_dev, cmd_handle, GE_PTN, blit_info->fg_rect.uWidth, blit_info->fg_rect.uHeight);
	}
	else
	{
		ret = ge_set_base_addr(ge_dev, cmd_handle, GE_SRC, &src_base_addr2);
		ret |= ge_set_base_addr(ge_dev, cmd_handle, GE_PTN, &src_base_addr1);
		ret |= ge_set_wh(ge_dev, cmd_handle, GE_SRC, blit_info->bg_rect.uWidth, blit_info->bg_rect.uHeight);
		ret |= ge_set_xy(ge_dev, cmd_handle, GE_SRC, blit_info->bg_rect.uLeft, blit_info->bg_rect.uTop);
		ret |= ge_set_xy(ge_dev, cmd_handle, GE_PTN, blit_info->fg_rect.uLeft, blit_info->fg_rect.uTop);
		ret |= ge_set_wh(ge_dev, cmd_handle, GE_PTN, blit_info->fg_rect.uWidth, blit_info->fg_rect.uHeight);
	}
	ret |= ge_set_base_addr(ge_dev, cmd_handle, GE_DST, &dst_base_addr);
	ret |= ge_set_wh(ge_dev, cmd_handle, GE_DST, blit_info->dst_rect.uWidth, blit_info->dst_rect.uHeight);
	ret |= ge_set_xy(ge_dev, cmd_handle, GE_DST, blit_info->dst_rect.uLeft, blit_info->dst_rect.uTop);
	ret |= set_clip_mode_rect(ge_dev,blit_info,cmd_handle);
	ret |= set_color_key(ge_dev,blit_info,blit_operation,cmd_handle);
	ret |= set_alpha(ge_dev,blit_info,blit_operation,cmd_handle);
	if(AUI_GFX_MIRROR_NORMAL != blit_operation->mirror_type)
	{
		ret |= ge_set_scan_order(ge_dev, cmd_handle, GE_PTN, ver_scan_type, hor_scan_type);
	}
	if(TRUE == scale_flag )
	{
		ret |= set_scale_param(ge_dev,cmd_handle,&(blit_info->fg_rect),&(blit_info->dst_rect));
	}
	if((AUI_GFX_ROP_ALPHA_BLENDING == blit_operation->rop_operation) ||(AUI_GFX_ROP_BOOL_ALPHA_BLENDIN == blit_operation->rop_operation))
	{
		ret |= ge_set_alpha_blend_mode(ge_dev,cmd_handle,blit_operation->alpha_blend_mode);
	}
	if((AUI_GFX_ROP_BOOLEAN == blit_operation->rop_operation) ||(AUI_GFX_ROP_BOOL_ALPHA_BLENDIN == blit_operation->rop_operation))
	{
		ret |= ge_set_bop_mode(ge_dev,cmd_handle,blit_operation->bop_mode);
	}
	ret |= ge_cmd_end(ge_dev, cmd_handle);

    ret |= ge_cmd_list_end(ge_dev, cmd_list);
#ifdef ENABLE_BG_SCALE
    GFXF_FREE(bg_scale_buf_bak);
#endif

	if(RET_SUCCESS != ret)
	{
	    AUI_ERR("fail!\n");
		ret = AUI_GFX_DRIVER_ERROR;
	}

	GFX_FUNCTION_LEAVE;
	return ret;
}

#ifdef ALI_S3603
//scale and copy data from one surface to another surface,no bop nor alpha blending operation
static AUI_RTN_CODE surface_scale_copy(st_blit_base_info *blit_info,aui_blit_operation *blit_operation,cmd_list_handle cmd_list)
{
	#define MAX_SCALE_RATIO_PER_TIME 3 //the max scale ratio per time that GC can supprot
	#define SCALE_DOWN 0
	#define SCALE_UP 1
	st_blit_base_info tmp_blit_info;
	unsigned char* transfer_buf = NULL;
	unsigned char* transfer_buf_backup = NULL;
	unsigned char* tmp_buf;
	unsigned long src_bpp;
	unsigned long dst_bpp;
	unsigned long horzotal_ratio;
	unsigned long vertical_ratio;
	unsigned long hor_scale_falg;//o means scale down,1 means scale up
	unsigned long ver_scale_falg;//o means scale down,1 means scale up
	unsigned long scale_cnt;
	AUI_RTN_CODE ret;

	MEMCPY(&tmp_blit_info,blit_info,sizeof(st_blit_base_info));
	src_bpp = get_bpp_by_color_mode(blit_info->fg_ge_color_mode);
	dst_bpp = get_bpp_by_color_mode(blit_info->dst_ge_color_mode);
	//check it is scale up or scale down
	hor_scale_falg = (blit_info->dst_rect.uWidth > blit_info->fg_rect.uWidth)? SCALE_UP : SCALE_DOWN;
	ver_scale_falg = (blit_info->dst_rect.uHeight > blit_info->fg_rect.uHeight)? SCALE_UP : SCALE_DOWN;
	horzotal_ratio = (hor_scale_falg == SCALE_UP)? (blit_info->dst_rect.uWidth / blit_info->fg_rect.uWidth) : (blit_info->fg_rect.uWidth / blit_info->fg_rect.uWidth);
	vertical_ratio = (ver_scale_falg == SCALE_UP)? (blit_info->dst_rect.uHeight / blit_info->fg_rect.uHeight) : (blit_info->fg_rect.uHeight / blit_info->fg_rect.uHeight);
	//if scale cnt > 1,means  use a tmp buf
	if((horzotal_ratio > MAX_SCALE_RATIO_PER_TIME) || (vertical_ratio > MAX_SCALE_RATIO_PER_TIME))
	{
		transfer_buf_backup = GFX_MALLOC((blit_info->dst_pitch * blit_info->dst_rect.uHeight) + MIN_ALIGN);
		transfer_buf = (unsigned char *)((unsigned long)transfer_buf_backup & MIN_ALIGN_MASK);
		if(NULL == transfer_buf)
		{
			AUI_ERR("MALLOC failed \n");
			aui_rtn(AUI_GFX_NO_MEMORY,NULL);
		}
	 }
	scale_cnt = 0;
	//because GC only support scale ratio less than 3,we must do several times to finish it
	do
	{

		if(SCALE_UP == hor_scale_falg)
		{
			tmp_blit_info.dst_rect.uWidth = (horzotal_ratio > MAX_SCALE_RATIO_PER_TIME)? \
								(tmp_blit_info.fg_rect.uWidth / MAX_SCALE_RATIO_PER_TIME) : blit_info->dst_rect.uWidth;
		}
		else
		{
			tmp_blit_info.dst_rect.uWidth = (horzotal_ratio > MAX_SCALE_RATIO_PER_TIME)? \
								(tmp_blit_info.fg_rect.uWidth * MAX_SCALE_RATIO_PER_TIME) : blit_info->dst_rect.uWidth;
		}
		if(SCALE_UP == ver_scale_falg)
		{
			tmp_blit_info.dst_rect.uHeight = (vertical_ratio > MAX_SCALE_RATIO_PER_TIME)? \
								(tmp_blit_info.fg_rect.uHeight / MAX_SCALE_RATIO_PER_TIME) : blit_info->dst_rect.uHeight;
		}
		else
		{
			tmp_blit_info.dst_rect.uHeight = (vertical_ratio > MAX_SCALE_RATIO_PER_TIME)? \
								(tmp_blit_info.fg_rect.uHeight * MAX_SCALE_RATIO_PER_TIME) : blit_info->dst_rect.uHeight;
		}
		//update the horzotal_ratio and vertical_ratio
		horzotal_ratio = (hor_scale_falg == SCALE_UP)? (blit_info->dst_rect.uWidth / tmp_blit_info.fg_rect.uWidth) : \
							(tmp_blit_info.fg_rect.uWidth / blit_info->dst_rect.uWidth);
		vertical_ratio = (ver_scale_falg == SCALE_UP)? (blit_info->dst_rect.uHeight / tmp_blit_info.fg_rect.uHeight) : \
							(tmp_blit_info.fg_rect.uHeight / blit_info->dst_rect.uHeight);
		//update the pitch
		tmp_blit_info.fg_pitch = tmp_blit_info.fg_rect.uWidth * (src_bpp >> 3);
		tmp_blit_info.dst_pitch = tmp_blit_info.dst_rect.uWidth * (dst_bpp >> 3);
		ret = do_surface_blit(&tmp_blit_info,blit_operation,cmd_list);

		//ping pong the buf and rect,dst change to src,src change to dst
		if((scale_cnt == 0) && (transfer_buf != NULL))
		{
			tmp_blit_info.fg_buf = tmp_blit_info.dst_buf;
			tmp_blit_info.dst_buf = transfer_buf;
		}
		else
		{
			tmp_buf = tmp_blit_info.fg_buf;
			tmp_blit_info.fg_buf = tmp_blit_info.dst_buf;
			tmp_blit_info.dst_buf = tmp_buf;

		}
		MEMCPY(&tmp_blit_info.fg_rect, &tmp_blit_info.dst_rect, sizeof(struct aui_osd_rect));
		scale_cnt++;
	}while((blit_info->fg_rect.uHeight == tmp_blit_info.fg_rect.uHeight) && (blit_info->fg_rect.uWidth == tmp_blit_info.fg_rect.uWidth));

	if(((scale_cnt % 2) != 0) && (scale_cnt > 1))//because we use ping pong buffer,the last time we maybe not blit to the dst buf,so we must blit one more time
	{
		ret |= do_surface_blit(&tmp_blit_info,blit_operation,cmd_list);
	}
	if(NULL != transfer_buf_backup)
	{
		GFXF_FREE(transfer_buf_backup);
		transfer_buf_backup = NULL;
	}
	if(ret != RET_SUCCESS)
	{
		AUI_ERR("failed\n");
		aui_rtn(AUI_GFX_NO_MEMORY,NULL);
	}
	return SUCCESS;
}
#endif


/**
*	 @brief 		Init gfx module
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_call_back_init:  Initialize the callback function

*	 @param[in] 	   pv_param:call back args
*	 @return		 error flags
*	 @note		  gfx module call before using and need to initialize display module\n
  pass in a callback function aui_gfx_init  will call the callback function attach will call the callback function
*
*/
AUI_RTN_CODE aui_gfx_init(p_fun_cb p_call_back_init,void *pv_param)
{
	AUI_RTN_CODE ret = RET_SUCCESS;

	GFX_FUNCTION_ENTER;
	if(p_call_back_init)
	{
		p_call_back_init(pv_param);
	}
	aui_gfx_chip_id = sys_ic_get_chip_id ();
	st_ge_device = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if(NULL == st_ge_device)
	{
		AUI_ERR("get ge_device failed \n");
		ret = AUI_GFX_DRIVER_ERROR;
		goto ERROR;
	}
    /*
        modify by fawn.fu@20151117.
        The ge and gma driver has been separated into two parts from S3821, 
        so do not call "ge_open" here but call "osddrv_open" in "aui_gfx_open".
    */
	if((ALI_S3821 != sys_ic_get_chip_id()) 
        && (ALI_C3505 != sys_ic_get_chip_id())){
		//osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,layer_id);
		ret = ge_open(st_ge_device);
	}
	
	if(RET_SUCCESS != ret)
	{
		AUI_ERR("ge_open failed \n");
		ret = AUI_GFX_DRIVER_ERROR;
		goto ERROR;
	}
	MEMSET(st_gfx_layer_info,0,sizeof(st_gfx_layer_info));
	MEMSET(&st_root_list,0,sizeof(surface_list));
	gfx_mutex = osal_mutex_create();
	if(INVALID_ID == gfx_mutex)
	{
	    AUI_ERR("invalid params.\n");
		ret = AUI_GFX_PARAM_INVALID;
		goto ERROR;
	}

	GFX_FUNCTION_LEAVE;
	return SUCCESS;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		deinit gfx module
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_call_back_init:  deinitialize the callback function
*	 @param[in] 	   pv_param: call back args
*	 @return		 error flags
*	 @note		  aui_gfx_init pairs using , need to make sure to release all before exit gfx resources
*
*/
AUI_RTN_CODE aui_gfx_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	AUI_RTN_CODE ret = RET_SUCCESS;

	GFX_FUNCTION_ENTER;

	if(p_call_back_init)
	{
		p_call_back_init(pv_param);
	}
    
	/*
        modify by fawn.fu@20151117.
        The ge and gma driver has been separated into two parts from S3821, 
        so we do not need to call "ge_close" here, but to call "osddrv_close" in "aui_gfx_layer_close".
    */
	if((ALI_S3821 != sys_ic_get_chip_id()) 
        && (ALI_C3505 != sys_ic_get_chip_id())) { 
	    // we need to check before call ge_close
	    if (check_all_layer_closed()) {
            AUI_DBG(" aui_gfx_de_init: to call ge_close!!!!!\n");
    		ret = ge_close(st_ge_device);
    		if(RET_SUCCESS != ret)
    		{
    			AUI_ERR("ge_open failed\n");
    			ret = AUI_GFX_DRIVER_ERROR;
    			goto ERROR;
    		}
	    }
	} 

	if(INVALID_ID != gfx_mutex)
	{
		osal_mutex_delete(gfx_mutex);
		gfx_mutex = INVALID_ID;
	}
	GFX_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 	      open gfx layer
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   layer_id , layer index , the maximum layer index of different IC may be different
*	 @param[out]	gfx_layer_handle: gfx layer handle
*	 @return		 error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_open(aui_osd_layer layer_id, aui_hdl *gfx_layer_handle)
{
    AUI_RTN_CODE ret = SUCCESS;
    OSAL_ID layer_mutex;
    gfx_layer_info *layer_info;

    GFX_FUNCTION_ENTER;
    GFX_LOCK(gfx_mutex);
    //check input param
    if( layer_id >= MAX_OSD_LAYER ) {
        AUI_ERR("open_param.layer_id > MAX_OSD_LAYER,id=%d\n",layer_id);
        ret = AUI_GFX_PARAM_INVALID;
        GFX_UNLOCK(gfx_mutex);
        goto ERROR;
    }
    layer_info = get_layer_info(layer_id);
    if(layer_info->open_cnt == 0) {//check if the layer have been open
        layer_mutex = osal_mutex_create();
        if(INVALID_ID == layer_mutex) {
            AUI_ERR("osal_mutex_create failed\n");
            ret = AUI_GFX_PARAM_INVALID;
            GFX_UNLOCK(gfx_mutex);
            goto ERROR;
        }

        if((ALI_S3821 == sys_ic_get_chip_id()) 
            || (ALI_C3505 == sys_ic_get_chip_id())) {
            layer_info->osd_dev  = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,layer_id);
            ret = osddrv_open((UINT32)layer_info->osd_dev,NULL);
            if(ret != SUCCESS) {
                goto ERROR;	
            }
        }
    } else {
        layer_mutex = layer_info->layer_mutex;
    }
    GFX_LOCK(layer_mutex);
    GFX_UNLOCK(gfx_mutex);

    incress_open_cnt(layer_id);
    if(layer_info->open_cnt <= 1) {
        set_layer_id(layer_id);
        set_layer_mutex(layer_mutex,layer_id);
    }
    *gfx_layer_handle = layer_info;

    layer_info->dev_priv_data.dev_idx = layer_id;
    aui_dev_reg(AUI_MODULE_GFX, layer_info);
    GFX_UNLOCK(layer_mutex);
    GFX_FUNCTION_LEAVE;
    return ret;

ERROR:
    GFX_FUNCTION_LEAVE;
    aui_rtn(ret,NULL);
}


/**
*	 @brief 		 close gfx layer
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle : handler return by aui_gfx_layer_open
*	 @param[out]	NULL
*	 @return		 error flags
*	 @note		this function is used with aui_gfx_layer_open , before closing the gfx layer,
*      The upper layer needs to close all the surface created by this layer.
*
*/
AUI_RTN_CODE aui_gfx_layer_close(aui_hdl gfx_layer_handle)
{
    AUI_RTN_CODE ret = SUCCESS;
    gfx_layer_info *st_layer_info;
    ge_anti_flick_t af;
    struct ge_device*	ge_device;

    GFX_FUNCTION_ENTER;

    GFX_LOCK(gfx_mutex);
    if( (NULL == gfx_layer_handle) 
        || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
    {
        ret = AUI_GFX_PARAM_INVALID;
        AUI_ERR("input gfx_layer_handle is null\n");
        GFX_UNLOCK(gfx_mutex);
        goto ERROR;
    }

    st_layer_info = gfx_layer_handle;
    if(st_layer_info->open_cnt == 0) {
        AUI_ERR("gfx_layer_handle has been closed,id=%d\n", st_layer_info->layer_id);
        GFX_UNLOCK(gfx_mutex);
        goto EXIT;
    }
    GFX_LOCK(st_layer_info->layer_mutex);
    //redeuce_open_cnt(st_layer_info->layer_id);
    // do not use reduce_open_cnt, or there will got cpptest warning!
    st_layer_info->open_cnt--;
    GFX_UNLOCK(gfx_mutex);

    if(st_layer_info->open_cnt == 0) {// maybe some tasks have opened
        ge_device = get_ge_device();
        af.layer = st_layer_info->layer_id;
        af.valid = 0;
        ret = ge_io_ctrl(ge_device,GE_IO_ANTI_FLICK_29E,(unsigned long)&af);
        if(RET_SUCCESS != ret) {
            ret = AUI_GFX_DRIVER_ERROR;
            AUI_ERR("OSD_IO_DISABLE_ANTIFLICK failed\n");
            GFX_UNLOCK(st_layer_info->layer_mutex);
            goto ERROR;
        }
        if((ALI_S3821 == sys_ic_get_chip_id()) 
            || (ALI_C3505 == sys_ic_get_chip_id())) {
            ret = osddrv_close((UINT32)st_layer_info->osd_dev);
            if(ret != SUCCESS) {
                goto ERROR;	
            }
        }
    }

    GFX_UNLOCK(st_layer_info->layer_mutex);

    if(st_layer_info->open_cnt == 0) {
        osal_mutex_delete(st_layer_info->layer_mutex);
    }
    aui_dev_unreg(AUI_MODULE_GFX, st_layer_info);
    
EXIT:
    GFX_FUNCTION_LEAVE;
    return ret;

ERROR:
    GFX_FUNCTION_LEAVE;
    aui_rtn(ret,NULL);
}

/**
*	 @brief 		  set the switch of the gfx layer function anti-flicker
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle , handler return by aui_gfx_layer_open
*	 @param[in] 	   onoff: "1" open anti-flicker, "0" close anti-flicker
*	 @param[out]	NULL
*	 @return		error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_antifliker_on_off(aui_hdl gfx_layer_handle,unsigned long on_off)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;
	ge_anti_flick_t af;
	struct ge_device*	ge_device;

	GFX_FUNCTION_ENTER;


	GFX_LOCK(gfx_mutex);
	if( (NULL == gfx_layer_handle) || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);
	GFX_UNLOCK(gfx_mutex);

	ge_device = get_ge_device();
	af.layer = st_layer_info->layer_id;
	af.valid = on_off;
	ret = ge_io_ctrl(ge_device,GE_IO_ANTI_FLICK_29E,(unsigned long)&af);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSD_IO_ENABLE_ANTIFLICK failed\n");
		GFX_UNLOCK(st_layer_info->layer_mutex);
		goto ERROR;
	}

	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}



AUI_RTN_CODE aui_gfx_layer_show_on_off(aui_hdl gfx_layer_handle,unsigned long on_off)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;
	struct ge_device*	ge_device;

	GFX_FUNCTION_ENTER;

	GFX_LOCK(gfx_mutex);
	if( (NULL == gfx_layer_handle) || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);
	GFX_UNLOCK(gfx_mutex);
	ge_device = get_ge_device();
    if(st_gfx_layer_info[((gfx_layer_info *)gfx_layer_handle)->layer_id].hw_surface_cnt > 0)
    {
        aui_layer_show_status[st_layer_info->layer_id] = on_off;
        aui_layer_show_last_status[st_layer_info->layer_id] = on_off;
	    ret = ge_gma_show_onoff(ge_device,st_layer_info->layer_id,on_off);
    }
    else
    {   
        aui_layer_show_status[st_layer_info->layer_id] = on_off;
        ret = RET_SUCCESS;
    }
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("ge_gma_show_onoff failed\n");
		GFX_UNLOCK(st_layer_info->layer_mutex);
		goto ERROR;
	}

	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		set gfx layer transparency 
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle , handler return by aui_gfx_layer_open
*	 @param[in] 	   alpha , The value of the transparency to be set
*	 @param[out]	NULL
*	 @return		error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_alpha_set(aui_hdl gfx_layer_handle,unsigned long alpha)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;
	struct ge_device*	ge_device;

	GFX_FUNCTION_ENTER;

	GFX_LOCK(gfx_mutex);
	if( (NULL == gfx_layer_handle) || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);
	GFX_UNLOCK(gfx_mutex);

	ge_device = get_ge_device();
	ret = ge_io_ctrl_ext(ge_device,st_layer_info->layer_id, GE_IO_SET_GLOBAL_ALPHA,alpha);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("GE_IO_SET_GLOBAL_ALPHA failed\n");
		GFX_UNLOCK(st_layer_info->layer_mutex);
		goto ERROR;
	}

	st_layer_info->alpha =	alpha;
	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		get gfx layer transparency 
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle , handler return by aui_gfx_layer_open
*	 @param[out]	alpha , return the value of transparency
*	 @return		error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_alpha_get(aui_hdl gfx_layer_handle,unsigned long *alpha)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;

	GFX_FUNCTION_ENTER;

	GFX_LOCK(gfx_mutex);
	if( (NULL == gfx_layer_handle) || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);
	GFX_UNLOCK(gfx_mutex);

	*alpha = st_layer_info->alpha;

	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		create software surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   e_color_mode:  surface color space
*	 @param[in] 	   width:       surface width
*	 @param[in] 	   height:      surface height
*	 @param[out]	p_surface_handle:  return software surface handle
*	 @return		 error flags
*	 @note		  the concept of the software surface is based on the hardware surface , It's actually a piece of memory 
*            plus some descriptions of that (e_color_mode,width,height). If memory is large enough, the software surface can be of any size
*/
AUI_RTN_CODE aui_gfx_sw_surface_create(enum aui_osd_pixel_format e_color_mode, unsigned long width, unsigned long height,aui_hdl *p_surface_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;
	struct ge_device*	ge_device;
	ge_cmd_list_hdl  cmd_list;
	OSAL_ID surface_mutex = INVALID_ID;

	GFX_FUNCTION_ENTER;

    if((e_color_mode < AUI_OSD_4_COLOR) 
        || (e_color_mode > AUI_OSD_COLOR_MODE_MAX) 
        || (width > OSD_GMA_MAX_WIDTH) 
        || (height > OSD_GMA_MAX_HEIGHT)) {
        ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("param invalid\n");
		goto ERROR; 
    }
	
	surface_mutex = osal_mutex_create();
	if(INVALID_ID == surface_mutex)
	{
		ret = AUI_GFX_OTHTER_ERROR;
		AUI_ERR("osal_mutex_create failed\n");
		goto ERROR; 	

	}
	//malloc surface list 
	st_surface_list = GFX_MALLOC(sizeof(surface_list));
	if( NULL == st_surface_list )
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed\n");
		goto ERROR; 		
	}
	
	MEMSET(st_surface_list,0,sizeof(surface_list));
	//add info to st_hw_surface_list 
	st_surface_list->surface_mutex = surface_mutex;
	st_surface_list->st_surface_info.is_hw_surface = FALSE;
	st_surface_list->st_surface_info.width = width;
	st_surface_list->st_surface_info.height = height;
	st_surface_list->st_surface_info.en_color_mode = e_color_mode;
	st_surface_list->st_surface_info.pitch = get_pitch(e_color_mode,width); 
	//malloc surface addr
	st_surface_list->st_surface_info.buf_size = st_surface_list->st_surface_info.pitch * height;
	st_surface_list->st_surface_info.p_surface_buf_backup = GFX_MALLOC(st_surface_list->st_surface_info.buf_size + MIN_ALIGN);
    if(NULL == st_surface_list->st_surface_info.p_surface_buf_backup)
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed\n");
		goto ERROR; 		
	}
	//st_surface_list->st_surface_info.p_surface_buf_backup = (unsigned long)st_surface_list->st_surface_info.p_surface_buf_backup |(0xa<<28);
	st_surface_list->st_surface_info.p_surface_buf = (unsigned char*)(((unsigned long)(st_surface_list->st_surface_info.p_surface_buf_backup + MIN_ALIGN)|(0xa<<28)) & MIN_ALIGN_MASK);
	//st_surface_list->st_surface_info.p_surface_buf = (st_surface_list->st_surface_info.p_surface_buf) |;

    //sync code from //depot/Customer/SDK4.0/TPE/Korea/humax/Full_13.3@changelist 248488
    osal_cache_invalidate(st_surface_list->st_surface_info.p_surface_buf_backup,
						  st_surface_list->st_surface_info.buf_size+MIN_ALIGN);
	//MEMSET(st_surface_list->st_surface_info.p_surface_buf,0,st_surface_list->st_surface_info.buf_size);
	//create ge cmd list
	ge_device = get_ge_device();
	cmd_list = ge_cmd_list_create(ge_device, 1);	
	if(0 == cmd_list)
	{
		AUI_ERR("ge_cmd_list_create failed\n");
		ret = AUI_GFX_DRIVER_ERROR;
		goto ERROR;
	}	
	st_surface_list->st_ge_private_info.cmd_list_handle = cmd_list;
	
	// add the surface to the link list
	surface_list_add(st_surface_list);

	*p_surface_handle = st_surface_list;
	GFX_FUNCTION_LEAVE;
	return ret;

	ERROR:
	if(INVALID_ID != surface_mutex)
	{
		osal_mutex_delete(surface_mutex);
	}	
	//free the malloc buffer
	if(st_surface_list && st_surface_list->st_surface_info.p_surface_buf_backup)
	{
		GFXF_FREE(st_surface_list->st_surface_info.p_surface_buf_backup);
	}
	if(st_surface_list)
	{
		GFXF_FREE(st_surface_list);
	}

	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


AUI_RTN_CODE aui_gfx_hw_surface_create(aui_hdl gfx_layer_handle,enum aui_osd_pixel_format e_color_mode, struct aui_osd_rect* rect,aui_hdl *p_hw_surface_handle,unsigned long is_double_buf)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info = NULL;
	surface_list *st_hw_surface_list = NULL;
	unsigned char *second_buff = NULL;
	BOOL region_create = FALSE;
	unsigned long buf_size = 0;
	unsigned long surface_id = 0;
	unsigned long pitch = 0;
	ge_gma_region_t region_info;
	ge_cmd_list_hdl  cmd_list;
	struct ge_device*	ge_device = NULL;
	BOOL layer_lock_flag = FALSE;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	if((NULL == gfx_layer_handle) 
        || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle))){
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);//lock layer
	layer_lock_flag = TRUE;
	GFX_UNLOCK(gfx_mutex);

	ge_device = get_ge_device();
	//create ge cmd list
	cmd_list = ge_cmd_list_create(ge_device, 1);	
	if(0 == cmd_list)
	{
		AUI_ERR("ge_cmd_list_create failed\n");
		ret = AUI_GFX_DRIVER_ERROR;
		goto ERROR;
	}		
	pitch = get_pitch(e_color_mode,rect->uWidth);
	surface_id = st_layer_info->hw_surface_cnt;
	//create osd region
	MEMSET(&region_info,0x0,sizeof(region_info));
	region_info.galpha_enable = 0;
	region_info.global_alpha = 0xff;
	region_info.region_x = rect->uLeft;
	region_info.region_y = rect->uTop;
	region_info.region_w = rect->uWidth;
	region_info.region_h = rect->uHeight;
	region_info.bitmap_x = 0;
	region_info.bitmap_y = 0;
	region_info.bitmap_w = rect->uWidth;
	region_info.bitmap_h = rect->uHeight;
	region_info.pixel_pitch = rect->uWidth;
	region_info.color_format = osd_color_mode2ge(e_color_mode);
	if((ALI_S3821 == sys_ic_get_chip_id()) 
        || (ALI_C3505 == sys_ic_get_chip_id())) {
		if((st_layer_info->layer_id == 0) 
            && (region_info.color_format == AUI_GE_PF_CLUT8)) {
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("ge_gma_create_region failed,layer id 0 dont support AUI_GE_PF_CLUT8\n");
			goto ERROR; 
		}
	}
	//get surface addr
	ret = ge_gma_create_region(ge_device,st_layer_info->layer_id,surface_id,&region_info);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("ge_gma_create_region failed\n");
		goto ERROR; 		
	}
	region_create = TRUE;
	MEMSET(&region_info,0x0,sizeof(region_info));
	ret = ge_gma_get_region_info(ge_device,st_layer_info->layer_id,surface_id,&region_info);	//get region info
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_get_region_addr failed\n");
		goto ERROR; 		
	}	
	buf_size = pitch * rect->uHeight;
	//malloc surface list 
	st_hw_surface_list = GFX_MALLOC(sizeof(surface_list));
	//if it is double buffer mode,malloc buffer for the second buffer
	if(is_double_buf)
	{
		second_buff = GFX_MALLOC(buf_size + MIN_ALIGN);
	}
	if( (NULL == st_hw_surface_list) || ((NULL == second_buff)&& (TRUE == is_double_buf)) )
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed,buf_size = %d\n", buf_size);
		goto ERROR; 		
	}
	MEMSET(st_hw_surface_list,0,sizeof(surface_list));
	//add info to st_hw_surface_list 
	st_hw_surface_list->st_surface_info.is_hw_surface = TRUE;
	st_hw_surface_list->st_surface_info.width = rect->uWidth;
	st_hw_surface_list->st_surface_info.height = rect->uHeight;
	st_hw_surface_list->st_surface_info.en_color_mode = e_color_mode;
	st_hw_surface_list->st_surface_info.pitch = pitch;		
	st_hw_surface_list->st_surface_info.buf_size = buf_size;
	st_hw_surface_list->st_surface_info.hw_surface_info.isdouble_buf = is_double_buf;
	st_hw_surface_list->st_surface_info.hw_surface_info.second_buf_backup = second_buff;
	st_hw_surface_list->st_surface_info.hw_surface_info.hw_surface_id = surface_id;
	st_hw_surface_list->st_surface_info.hw_surface_info.layer_id = st_layer_info->layer_id;
	st_hw_surface_list->st_surface_info.hw_surface_info.layer_handle = gfx_layer_handle;
	st_hw_surface_list->st_surface_info.hw_surface_info.left_off_set = rect->uLeft;
	st_hw_surface_list->st_surface_info.hw_surface_info.top_off_set = rect->uTop;
	st_hw_surface_list->st_surface_info.p_surface_buf = (unsigned char *)(region_info.bitmap_addr|0x80000000);
	st_hw_surface_list->st_ge_private_info.cmd_list_handle = cmd_list;
	if(is_double_buf)
	{
		st_hw_surface_list->st_surface_info.hw_surface_info.second_buf = (unsigned char*)((((unsigned long)second_buff + MIN_ALIGN)|(0xa<<28)) & MIN_ALIGN_MASK);
        //sync code from //depot/Customer/SDK4.0/TPE/Korea/humax/Full_13.3@changelist 248488
        osal_cache_invalidate(second_buff, buf_size+MIN_ALIGN);
	}		
	st_hw_surface_list->surface_mutex = osal_mutex_create();

	if(INVALID_ID == st_hw_surface_list->surface_mutex)
	{
		ret = AUI_GFX_OTHTER_ERROR;
		AUI_ERR("osal_mutex_create failed\n");
		goto ERROR;
	}
	// add the surface to the link list
	surface_list_add(st_hw_surface_list);
    if((aui_layer_show_status[st_layer_info->layer_id] != aui_layer_show_last_status[st_layer_info->layer_id])
        || (aui_layer_show_status[st_layer_info->layer_id] == 1)) {//If the color_format in region_info changed, the GE would close gma.
        aui_layer_show_last_status[st_layer_info->layer_id] = aui_layer_show_status[st_layer_info->layer_id];
        ret = ge_gma_show_onoff(ge_device,st_layer_info->layer_id, aui_layer_show_status[st_layer_info->layer_id]);
    }
    if(RET_SUCCESS != ret) {
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("ge_gma_layer_show failed\n");
		goto ERROR; 		
	}
	*p_hw_surface_handle = st_hw_surface_list;
	incress_hwsurface_cnt(st_layer_info->layer_id);

	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	//free the malloc buffer
	if(region_create)
	{
		ge_gma_delete_region(ge_device,st_layer_info->layer_id,surface_id);
	}
	if(second_buff) {
		GFXF_FREE(second_buff);
	}
	if( (NULL != st_layer_info)  
        && (st_gfx_layer_info[st_layer_info->layer_id].hw_surface_cnt == 0)) {
        /*reset the show status, and keep right status in "aui_layer_show_status"*/
		aui_layer_show_status[st_layer_info->layer_id] = aui_layer_show_last_status[st_layer_info->layer_id];
        aui_layer_show_last_status[st_layer_info->layer_id] = 0xff;
    }	
	if((st_hw_surface_list!=NULL) 
        && (INVALID_ID != st_hw_surface_list->surface_mutex))
    {
		osal_mutex_delete(st_hw_surface_list->surface_mutex);
    }
	if(st_hw_surface_list)
	{
		GFXF_FREE(st_hw_surface_list);
	}
	if(layer_lock_flag)
	{
		GFX_UNLOCK(st_layer_info->layer_mutex);
	}

	GFX_FUNCTION_LEAVE; 
	aui_rtn(ret,NULL);		
}

/**
*	 @brief 		delete surface, include hardware surface or software surface.
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle : aui_gfx_hw_surface_create or aui_gfx_sw_surface_create return handle
*	 @return	             error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_delete(aui_hdl p_surface_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list;
	unsigned long is_hw_surface;
	gfx_layer_info *layer_info = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input handle
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_surface_list = (surface_list *)p_surface_handle;
	is_hw_surface = st_surface_list->st_surface_info.is_hw_surface;
	if(TRUE == is_hw_surface)
	{
		layer_info = get_layer_info(st_surface_list->st_surface_info.hw_surface_info.layer_id);
		//for delete region,off layer show,so param sync.
		GFX_LOCK(layer_info->layer_mutex);//lock layer
	}
	GFX_LOCK(st_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	if(TRUE == is_hw_surface)
	{
		ret = delete_hw_surface(st_surface_list);
		GFX_UNLOCK(layer_info->layer_mutex);//unlock layer
	}
	else
	{
		ret = delete_sw_surface(st_surface_list);
	}

	GFX_UNLOCK(st_surface_list->surface_mutex);
	osal_mutex_delete(st_surface_list->surface_mutex);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("surface delete failed\n");
		goto ERROR;
	}
	GFX_FUNCTION_LEAVE;
	return ret;
ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


AUI_RTN_CODE aui_gfx_surface_blit(aui_hdl dst_surface_handle,aui_hdl fg_surface_handle,aui_hdl bg_surface_handle,
										aui_blit_operation *blit_operation,aui_blit_rect *blit_rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_fg_surface_list = NULL;
	surface_list *st_bg_surface_list = NULL;
	surface_list *st_dst_surface_list = NULL;
	unsigned long is_hw_surface = FALSE;
	unsigned long is_double_buf = FALSE;
	cmd_list_handle cmd_list;
	st_blit_base_info blit_info;
	BOOL scale_flag;
	
	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == dst_surface_handle) 
        || (NULL == fg_surface_handle) 
        || (NULL == blit_operation) 
        || (NULL == blit_rect)
		|| (FALSE == surface_list_check(dst_surface_handle)) 
        || (FALSE == surface_list_check(fg_surface_handle)) 
		|| ((NULL != bg_surface_handle) && (FALSE == surface_list_check(bg_surface_handle))))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR; 	
	}	
	st_fg_surface_list = (surface_list *)fg_surface_handle;
	GFX_LOCK(st_fg_surface_list->surface_mutex);//lock surface
	st_dst_surface_list = (surface_list *)dst_surface_handle; 
	if(st_dst_surface_list->surface_mutex != st_fg_surface_list->surface_mutex)
	{
		GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	}	 
	if(NULL != bg_surface_handle)
	{
		st_bg_surface_list = (surface_list *)bg_surface_handle;	
		if((st_bg_surface_list->surface_mutex != st_fg_surface_list->surface_mutex) && \
			(st_bg_surface_list->surface_mutex != st_dst_surface_list->surface_mutex))
		{
			GFX_LOCK(st_bg_surface_list->surface_mutex);//lock surface
		}	 
	}
	else
	{   // if the bg_surface_handle is NULL, set the blit_rect->bg_rect equal to blit_rect->dst_rect
        blit_rect->bg_rect.uLeft = blit_rect->dst_rect.uLeft;
        blit_rect->bg_rect.uTop = blit_rect->dst_rect.uTop;
        blit_rect->bg_rect.uHeight= blit_rect->dst_rect.uHeight;
        blit_rect->bg_rect.uWidth = blit_rect->dst_rect.uWidth;
        st_bg_surface_list = (surface_list *)dst_surface_handle;	
	}
	GFX_UNLOCK(gfx_mutex);
	
	if((blit_rect->fg_rect.uLeft + blit_rect->fg_rect.uWidth > st_fg_surface_list->st_surface_info.width) 
        || (blit_rect->fg_rect.uTop+blit_rect->fg_rect.uHeight >  st_fg_surface_list->st_surface_info.height)
        || (blit_rect->dst_rect.uLeft+  blit_rect->dst_rect.uWidth> st_dst_surface_list->st_surface_info.width) 
        || (blit_rect->dst_rect.uTop+blit_rect->dst_rect.uHeight> st_dst_surface_list->st_surface_info.height) 
        || (blit_rect->bg_rect.uLeft+blit_rect->bg_rect.uWidth> st_bg_surface_list->st_surface_info.width) 
        || (blit_rect->bg_rect.uTop+blit_rect->bg_rect.uHeight>st_bg_surface_list->st_surface_info.height))
	{
		GFX_UNLOCK(st_fg_surface_list->surface_mutex);
		if(st_dst_surface_list->surface_mutex != st_fg_surface_list->surface_mutex)
		{
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);
		}
		if((st_bg_surface_list->surface_mutex != st_fg_surface_list->surface_mutex) && \
			(st_bg_surface_list->surface_mutex != st_dst_surface_list->surface_mutex))
		{
			GFX_UNLOCK(st_bg_surface_list->surface_mutex);
		}
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("surface blit err failed\n");
		goto ERROR; 
	}
	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;
	MEMSET(&blit_info,0x0,sizeof(st_blit_base_info));
	MEMCPY(&(blit_info.fg_rect),&(blit_rect->fg_rect),sizeof(struct aui_osd_rect));
    if(NULL != bg_surface_handle)
    {
	    MEMCPY(&(blit_info.bg_rect),&(blit_rect->bg_rect),sizeof(struct aui_osd_rect));
    }
    else
    {
	    MEMCPY(&(blit_info.bg_rect),&(blit_rect->dst_rect),sizeof(struct aui_osd_rect));
    }
	MEMCPY(&(blit_info.dst_rect),&(blit_rect->dst_rect),sizeof(struct aui_osd_rect));
	MEMCPY(&(blit_info.clip_ret),&(st_dst_surface_list->st_surface_info.clip_rect),sizeof(struct aui_osd_rect));
	MEMCPY(&(blit_info.fg_color_key),&(st_fg_surface_list->st_surface_info.color_key),sizeof(aui_color_key));
	MEMCPY(&(blit_info.bg_color_key),&(st_bg_surface_list->st_surface_info.color_key),sizeof(aui_color_key));
	MEMCPY(&(blit_info.dst_color_key),&(st_dst_surface_list->st_surface_info.color_key),sizeof(aui_color_key));
	//check if it is scale,only check the source 1
	scale_flag = ((blit_info.dst_rect.uWidth == blit_info.fg_rect.uWidth) \
					&& (blit_info.dst_rect.uHeight == blit_info.fg_rect.uHeight) && (blit_info.dst_rect.uWidth == blit_info.bg_rect.uWidth)\
					&& (blit_info.dst_rect.uHeight == blit_info.bg_rect.uHeight))? FALSE : TRUE;	
	if((st_fg_surface_list->st_surface_info.is_hw_surface)
        && (st_fg_surface_list->st_surface_info.hw_surface_info.isdouble_buf)) {
		blit_info.fg_buf =	st_fg_surface_list->st_surface_info.hw_surface_info.second_buf;
	} else {
		blit_info.fg_buf = st_fg_surface_list->st_surface_info.p_surface_buf;
	}
	//blit_info.bg_buf = st_bg_surface_list->st_surface_info.p_surface_buf;
	if((st_bg_surface_list->st_surface_info.is_hw_surface)
        && (st_bg_surface_list->st_surface_info.hw_surface_info.isdouble_buf)) {
		blit_info.bg_buf =	st_bg_surface_list->st_surface_info.hw_surface_info.second_buf;
 	} else {
		blit_info.bg_buf = st_bg_surface_list->st_surface_info.p_surface_buf;
	}
	blit_info.dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;	
	blit_info.fg_pitch = st_fg_surface_list->st_surface_info.pitch;
	blit_info.bg_pitch = st_bg_surface_list->st_surface_info.pitch;
	blit_info.dst_pitch = st_dst_surface_list->st_surface_info.pitch;
	blit_info.fg_global_alpha = st_fg_surface_list->st_surface_info.golbal_alpha;
	blit_info.bg_global_alpha = st_dst_surface_list->st_surface_info.golbal_alpha;
	blit_info.fg_ge_color_mode = osd_color_mode2ge(st_fg_surface_list->st_surface_info.en_color_mode);
	blit_info.bg_ge_color_mode = osd_color_mode2ge(st_bg_surface_list->st_surface_info.en_color_mode);
	blit_info.dst_ge_color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	blit_info.clip_mode = st_dst_surface_list->st_surface_info.clip_mode;
	
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;
	if(TRUE == is_hw_surface)
	{
		is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	}
	
	//check if it is hw surface and double buffer 
	if((TRUE == is_hw_surface) && (TRUE == is_double_buf))
	{
		blit_info.dst_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;	//COPY to second buffer,and will write to region when call flush
	}
	//flush the data from cpu cache to the ddr
	osal_cache_flush(blit_info.fg_buf + blit_info.fg_pitch * blit_info.fg_rect.uTop, blit_info.fg_pitch * blit_info.fg_rect.uHeight);
	//if it is not the src_surface_handle2 , must flush it
	if(bg_surface_handle != st_dst_surface_list)
	{
		osal_cache_flush(blit_info.bg_buf + blit_info.bg_pitch * blit_info.bg_rect.uTop, blit_info.bg_pitch * blit_info.bg_rect.uHeight);
	}
#ifdef ALI_S3603
	if(TRUE == scale_flag && (ALI_S3603 == aui_gfx_chip_id))
	{
		ret = surface_scale_copy(&blit_info,blit_operation,cmd_list);
	}
	else
#else
	(void)scale_flag;
#endif	
	{
		ret = do_surface_blit(&blit_info,blit_operation,cmd_list);
	}
	
	GFX_UNLOCK(st_fg_surface_list->surface_mutex);
	if(st_dst_surface_list->surface_mutex != st_fg_surface_list->surface_mutex)
	{
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);
	}	 
	if(st_bg_surface_list != st_dst_surface_list)
	{
		GFX_UNLOCK(st_bg_surface_list->surface_mutex);
	}
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("surface_direct_copy failed\n");
		goto ERROR; 			
	}
	GFX_FUNCTION_LEAVE;
	return ret;
	
ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		blit data to surface from buffer 
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   dst_surface_handle : destination surface.
*	 @param[in] 	   p_data:data buffer pointer.
*	 @param[in] 	   blit_rect:rectangular area in destination surface.
*	 @param[in] 	   blit_operation :configuration of blit operation
*	 @param[in] 	   pitch: number of the bytes in one row of data buffer input
*	 @param[in] 	   en_color_mode:color space of the data buffer
*	 @param[out]	NULL
*	 @return		 error flags
*	 @note		  1.The difference between aui_gfx_data_blit and aui_gfx_surface_blit is that the input source of aui_gfx_surface_blit is the 
surface, while the source of aui_gfx_data_blit is data buffer,This is just a convenient way to move the data buffer to the surface without 
creating a surface. 
2.Scaling is required when the source and target area are not equal. For some IC, only support AUI_GFX_ROP_DERECT_COVER when scaling,
does not support the AUI_GFX_ROP_ALPHA_BLENDING operation, etc . If you need these operations , you must first ensure that the color mode 
of the source and target surface  is ARGB8888 and their width is a multiple of 16 , then divide the operation into two steps.First, you scale the source data, put 
the scaling results into a temporary buffer, and then use the buffer as the source to do other operations with the target , for some IC, there is no limit. 
3.For the IC without GE, it only supports AUI_GFX_ROP_DERECT_COVER , any other operation is not supported, and the source and target surface 
color mode and region need the same
4.Only part of the IC supports rotation. 
5.If the target surface is the hardware surface and is creating a double buffer, then aui_gfx_surface_flush is required to refresh to display the latest data.
*
*/
AUI_RTN_CODE aui_gfx_data_blit(aui_hdl dst_surface_handle,unsigned char *p_data,aui_blit_rect *blit_rect,aui_blit_operation *blit_operation,unsigned long pitch,
									enum aui_osd_pixel_format en_color_mode)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	unsigned long is_hw_surface = FALSE;
	unsigned long is_double_buf = FALSE;
	cmd_list_handle cmd_list;
	st_blit_base_info blit_info;
	BOOL scale_flag;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == dst_surface_handle) || (NULL == blit_operation) || (NULL == blit_rect) || (NULL == p_data)
		|| (FALSE == surface_list_check(dst_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)dst_surface_handle; 
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;
	//init the src 1
	MEMSET(&blit_info,0x0,sizeof(st_blit_base_info));
	MEMCPY(&(blit_info.fg_rect),&(blit_rect->fg_rect),sizeof(struct aui_osd_rect));
	MEMCPY(&(blit_info.clip_ret),&(st_dst_surface_list->st_surface_info.clip_rect),sizeof(struct aui_osd_rect));
	blit_info.fg_buf = p_data;
	blit_info.fg_pitch = pitch;
	blit_info.fg_ge_color_mode = osd_color_mode2ge(en_color_mode);
	//init the dst
	MEMCPY(&(blit_info.dst_rect),&(blit_rect->dst_rect),sizeof(struct aui_osd_rect));

	blit_info.bg_global_alpha = st_dst_surface_list->st_surface_info.golbal_alpha;
	blit_info.dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	blit_info.dst_pitch = st_dst_surface_list->st_surface_info.pitch;
	blit_info.dst_ge_color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	blit_info.clip_mode = st_dst_surface_list->st_surface_info.clip_mode;
	//if it will do alpha blending or bool operation,the src2 is the same as dst
	if(AUI_GFX_ROP_DERECT_COVER != blit_operation->rop_operation)
	{
		MEMCPY(&(blit_info.bg_rect),&(blit_info.dst_rect),sizeof(struct aui_osd_rect));
		blit_info.bg_buf = blit_info.dst_buf;
		blit_info.bg_pitch = blit_info.dst_pitch;
		blit_info.bg_ge_color_mode = blit_info.dst_ge_color_mode;
	}
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;
	scale_flag = ((blit_info.dst_rect.uWidth == blit_info.fg_rect.uWidth) \
					|| (blit_info.dst_rect.uHeight == blit_info.fg_rect.uHeight))? FALSE : TRUE;
	if(TRUE == is_hw_surface)
	{
		is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	}

	//check if it is hw surface and double buffer
	if((TRUE == is_hw_surface) && (TRUE == is_double_buf))
	{
		blit_info.dst_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf; //COPY to second buffer,and will write to region when call flush
	}
	//flush the data from cpu cache to ddr
	osal_cache_flush(blit_info.fg_buf + blit_info.fg_pitch * blit_info.fg_rect.uTop, blit_info.fg_pitch * blit_info.fg_rect.uHeight);
#ifdef 	ALI_S3603
	if((TRUE == scale_flag) && (ALI_S3603 == aui_gfx_chip_id))
	{
		ret = surface_scale_copy(&blit_info,blit_operation,cmd_list);
	}
	else
#else
	(void)scale_flag;
#endif	
	{
		ret = do_surface_blit(&blit_info,blit_operation,cmd_list);
	}
	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("aui_gfx_data_blit failed\n");
		goto ERROR;
	}
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}



/**
*	 @brief 		   capture the data in the surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle,   surface handle
*	 @param[in] 	   rect: the rectangular area for capturing data
*	 @param[out]	pdata:    output buffer
*	 @return		error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_capture(aui_hdl p_surface_handle,struct aui_osd_rect * rect,void *p_data)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	unsigned long is_hw_surface = FALSE;
	cmd_list_handle cmd_list;
	unsigned long dst_bpp;//dst bit per pixel
	st_blit_base_info blit_info;
	aui_blit_operation blit_operation;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (NULL == p_data) || (NULL == rect)
		|| (FALSE == surface_list_check(p_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;
	MEMSET(&blit_info,0x0,sizeof(st_blit_base_info));
	MEMCPY(&(blit_info.fg_rect),rect,sizeof(struct aui_osd_rect));
	//MEMCPY(&(blit_info.dst_rect),rect,sizeof(struct aui_osd_rect));
	blit_info.dst_rect.uLeft = 0;
	blit_info.dst_rect.uTop = 0;
	blit_info.dst_rect.uWidth = rect->uWidth;
	blit_info.dst_rect.uHeight = rect->uHeight;

	blit_info.fg_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	blit_info.dst_buf = p_data;
	blit_info.fg_ge_color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	blit_info.dst_ge_color_mode = blit_info.fg_ge_color_mode;
	blit_info.fg_pitch = st_dst_surface_list->st_surface_info.pitch;
	dst_bpp = get_bpp_by_color_mode(blit_info.fg_ge_color_mode);
	blit_info.dst_pitch = (dst_bpp >> 3) * rect->uWidth;
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;
	if(TRUE != is_hw_surface)//if not hardware surface ,some data may be in the cpu cache
	{
		osal_cache_flush(blit_info.fg_buf + blit_info.fg_pitch * blit_info.fg_rect.uTop, blit_info.fg_pitch * blit_info.fg_rect.uHeight);
	}
    // fix #60873, invalidate the cached buffer before ge access the buffer, 
    // to make sure data in cache keep the same with physical ram.
    osal_cache_invalidate(p_data, blit_info.dst_pitch * blit_info.dst_rect.uHeight);
	MEMSET(&blit_operation,0,sizeof(aui_blit_operation));
	blit_operation.rop_operation = AUI_GFX_ROP_DERECT_COVER;
	ret = do_surface_blit(&blit_info,&blit_operation,cmd_list);
	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("surface_direct_copy failed\n");
		goto ERROR;
	}
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		Draw a line in the horizontal direction and the vertical direction with specified color.
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   surface_handle: destination surface handle
*	 @param[in] 	   color:    specified color
*	 @param[in] 	   start_coordinate:line start point coordinate
*	 @param[in] 	   end_coordinate:line end point coordinate
*	 @param[out]	NULL
*	 @return
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_draw_line(aui_hdl surface_handle,unsigned long color,aui_coordinate *start_coordinate,aui_st_coordinate *end_coordinate)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	cmd_list_handle cmd_list;
	fill_base_param fill_param;
	unsigned long is_hw_surface = FALSE;
	unsigned long is_double_buf = FALSE;


	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == surface_handle) || (NULL == start_coordinate) || (NULL == end_coordinate)
		|| (FALSE == surface_list_check(surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);
	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;
	fill_param.fill_rect.uLeft = start_coordinate->X;
	fill_param.fill_rect.uTop = start_coordinate->Y;
	fill_param.fill_rect.uWidth = ((end_coordinate->X - start_coordinate->X) > 0)? (end_coordinate->X - start_coordinate->X) : 1;
	fill_param.fill_rect.uHeight = ((end_coordinate->Y - start_coordinate->Y) > 0)? (end_coordinate->Y - start_coordinate->Y) : 1;
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;
	fill_param.buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	//check if the input surface is the hw surface and doubble buffer mode
	if(TRUE == is_hw_surface)
	{
		is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
		if(TRUE == is_double_buf)
		{
			fill_param.buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
		}
	}	 
	fill_param.back_color = color;
	fill_param.pitch = st_dst_surface_list->st_surface_info.pitch;
	fill_param.color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	fill_param.draw_type = DRAW_TYPE_FILL;
	ret = do_surface_fill(&fill_param,cmd_list);
	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("do_surface_fill failed\n");
		goto ERROR;
	}
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		fill surface with specified color
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   surface_handle:destination surface handle
*	 @param[in] 	   color:color for filling
*	 @param[in] 	   rect:rectangular area for filling
*	 @param[out]	NULL
*	 @return		 error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_fill(aui_hdl surface_handle,unsigned long color,struct aui_osd_rect * rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	cmd_list_handle cmd_list;
	fill_base_param fill_param;
	unsigned long is_hw_surface = FALSE;
	unsigned long is_double_buf = FALSE;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == surface_handle) || (NULL == rect)
		|| (FALSE == surface_list_check(surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	if((rect->uLeft+ rect->uWidth > st_dst_surface_list->st_surface_info.width) 
        || (rect->uTop+rect->uHeight > st_dst_surface_list->st_surface_info.height)) {

		GFX_UNLOCK(st_dst_surface_list->surface_mutex);
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		goto ERROR;
	}

	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;
	MEMCPY(&(fill_param.fill_rect),rect,sizeof(struct aui_osd_rect));
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;

	fill_param.buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	//check if the input surface is the hw surface and doubble buffer mode
	if(TRUE == is_hw_surface)
	{
		is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
		if(TRUE == is_double_buf)
		{
			fill_param.buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
		}
	}
	fill_param.back_color = color;
	fill_param.pitch = st_dst_surface_list->st_surface_info.pitch;
	fill_param.color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	fill_param.draw_type = DRAW_TYPE_FILL;
	ret = do_surface_fill(&fill_param,cmd_list);
	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("do_surface_fill failed\n");
		goto ERROR;
	}
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		draw rectangle with specified color
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   surface_handle: destination surface handle
*	 @param[in] 	   fore_color: border color
*	 @param[in] 	   back_color:filling color , only effective if fill_background is 1
*	 @param[in] 	   fill_background:1-fill, 0-not fill;
*	 @return		 error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_draw_outline(aui_hdl surface_handle,unsigned long fore_color,unsigned long back_color,
													struct aui_osd_rect * rect,unsigned long fill_background)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	cmd_list_handle cmd_list;
	fill_base_param fill_param;
	unsigned long is_hw_surface = FALSE;
	unsigned long is_double_buf = FALSE;


	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == surface_handle) || (NULL == rect)
		|| (FALSE == surface_list_check(surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;
	MEMCPY(&(fill_param.fill_rect),rect,sizeof(struct aui_osd_rect));
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;

	fill_param.buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	//check if the input surface is the hw surface and doubble buffer mode
	if(TRUE == is_hw_surface)
	{
		is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
		if(TRUE == is_double_buf)
		{
			fill_param.buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
		}
	}
	fill_param.fore_color = fore_color;
	fill_param.back_color = back_color;
	fill_param.pitch = st_dst_surface_list->st_surface_info.pitch;
	fill_param.color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	if(fill_background)
	{
		fill_param.draw_type = DRAW_TYPE_OUTLINE_FILL;
	}
	else
	{
		fill_param.draw_type = DRAW_TYPE_OUTLINE;
	}
	ret = do_surface_fill(&fill_param,cmd_list);

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//lock surface
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("do_surface_draw_outline failed\n");
		goto ERROR;
	}
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		flush the data in backup OSD buffer to OSD buffer.
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   dst_surface_handle : hardware surface handle
*	 @param[in] 	   dst_rect:the rectangular area for flushing, it can't be out of the range of the surface
*	 @param[out]	NULL
*	 @return		 error flags
*	 @note		  this function is used only for double buffers hardware surface, and for a single cached hardware surface or software surface, the function returns directly
*/
AUI_RTN_CODE aui_gfx_surface_flush(aui_hdl dst_surface_handle,struct aui_osd_rect *dst_rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	unsigned long is_hw_surface = FALSE;
	unsigned long is_double_buf = FALSE;
	cmd_list_handle cmd_list;
	st_blit_base_info blit_info;
	aui_blit_operation blit_operation;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == dst_surface_handle) || (NULL == dst_rect)
		|| (FALSE == surface_list_check(dst_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)dst_surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	if((dst_rect->uHeight + dst_rect->uTop > st_dst_surface_list->st_surface_info.height) 
        || (dst_rect->uLeft + dst_rect->uWidth > st_dst_surface_list->st_surface_info.width)) {
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input param is invalid\n");
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);
		goto ERROR;
	}

	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;
	//check if the input surface is the hw surface and doubble buffer mode,if not, just return sucess
	if(TRUE == is_hw_surface)
	{
		is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
		if(FALSE == is_double_buf)
		{
			AUI_WARN("the surface is single buffer \n");
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
			goto EXIT;
		}
	}
	cmd_list = st_dst_surface_list->st_ge_private_info.cmd_list_handle;

	MEMSET(&blit_info,0x0,sizeof(st_blit_base_info));
	MEMSET(&blit_operation,0x0,sizeof(aui_blit_operation));

	MEMCPY(&(blit_info.fg_rect),dst_rect,sizeof(struct aui_osd_rect));
	MEMCPY(&(blit_info.dst_rect),dst_rect,sizeof(struct aui_osd_rect));
	blit_info.fg_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
	blit_info.dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	blit_info.fg_pitch = st_dst_surface_list->st_surface_info.pitch;
	blit_info.dst_pitch = blit_info.fg_pitch;
	blit_info.fg_ge_color_mode = osd_color_mode2ge(st_dst_surface_list->st_surface_info.en_color_mode);
	blit_info.dst_ge_color_mode = blit_info.fg_ge_color_mode;
	//flush the cpu cache data to ddr
	osal_cache_flush(blit_info.fg_buf + blit_info.fg_pitch * blit_info.fg_rect.uTop, blit_info.fg_pitch * blit_info.fg_rect.uHeight);

	MEMSET(&blit_operation,0,sizeof(aui_blit_operation));
	blit_operation.rop_operation = AUI_GFX_ROP_DERECT_COVER;
	ret = do_surface_blit(&blit_info,&blit_operation,cmd_list);

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	if(SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("urface_direct_copy failed\n");
		goto ERROR;
	}
EXIT:
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		sync surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   dst_surface_handle : surface handle
*	 @param[out]	NULL
*	 @return		error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_sync(aui_hdl dst_surface_handle)
{
    (void) dst_surface_handle;
	GFX_FUNCTION_ENTER;
	GFX_FUNCTION_LEAVE;

	return SUCCESS;
}

/**
*	 @brief 		get surface information
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle:surface handle
*	 @param[out]	p_surface_info:surface information structure pointer
*	 @return		 error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_info_get(aui_hdl p_surface_handle,aui_surface_info *p_surface_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) || (NULL == p_surface_info))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	MEMCPY(p_surface_info,&st_surface_list->st_surface_info,sizeof(aui_surface_info));

	GFX_UNLOCK(st_surface_list->surface_mutex);//lock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		turn on/off showing surface
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_hw_surface_handle:surface handle
*	 @param[in] 	   on_off: 1-show; 0-hide
*	 @param[out]	NULL
*	 @return		 error flags
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_show_on_off(aui_hdl p_hw_surface_handle,unsigned long on_off)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_hw_surface_list = NULL;
	struct ge_device* ge_dev;
	unsigned long hw_surface_id;
	unsigned long layer_id;

	if((ALI_S3821 == sys_ic_get_chip_id()) 
        || (ALI_C3505 == sys_ic_get_chip_id())) {
		return RET_SUCCESS;
	}
	
	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_hw_surface_handle) || (FALSE == surface_list_check(p_hw_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR; 	
	}	
	st_hw_surface_list = (surface_list *)p_hw_surface_handle;
	GFX_LOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);
	
	ge_dev = get_ge_device();
	if(FALSE == st_hw_surface_list->st_surface_info.is_hw_surface)
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is not hardware surface\n");
		GFX_UNLOCK(st_hw_surface_list->surface_mutex);//lock surface
		goto ERROR;
	}
	hw_surface_id = st_hw_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	layer_id = st_hw_surface_list->st_surface_info.hw_surface_info.layer_id;
	ret = ge_gma_show_region(ge_dev,layer_id,hw_surface_id,on_off);
	st_hw_surface_list->st_surface_info.show_onoff = on_off;
	GFX_UNLOCK(st_hw_surface_list->surface_mutex);//unlock surface
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_region_show failed \n"); 			
		goto ERROR;
	}		
	
	GFX_FUNCTION_LEAVE;
	return ret;
	
ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);	
}


AUI_RTN_CODE aui_gfx_surface_scale_param_set(aui_hdl p_hw_surface_handle,aui_scale_param *scale_param)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_hw_surface_list = NULL;
	struct ge_device* ge_dev;
	unsigned long layer_id;

	static osd_scale_param drv_scale_param;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
    if((NULL == scale_param) || (0 == scale_param->input_height) || (0 == scale_param->input_width) || \
       (0 == scale_param->output_height) || (0 == scale_param->output_width)) 
    {
        ret = AUI_GFX_PARAM_INVALID;
        AUI_ERR("input scale param is invalid\n");
        GFX_UNLOCK(gfx_mutex);
        goto ERROR;
    }
    
	if( (NULL == p_hw_surface_handle) || (FALSE == surface_list_check(p_hw_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR; 	
	}	
	st_hw_surface_list = (surface_list *)p_hw_surface_handle;
	GFX_LOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);
	
	ge_dev = get_ge_device();
	if(FALSE == st_hw_surface_list->st_surface_info.is_hw_surface)
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is not hardware surface\n");
		GFX_UNLOCK(st_hw_surface_list->surface_mutex);//lock surface
		goto ERROR;
	}
	layer_id = st_hw_surface_list->st_surface_info.hw_surface_info.layer_id;
	//caculate the scale param,horizontal scale = h_mul / h_div,vertical scale = v_mul / v_div
	drv_scale_param.h_div = (UINT16)(scale_param->input_width);
	drv_scale_param.h_mul = (UINT16)(scale_param->output_width);
	drv_scale_param.v_div = (UINT16)(scale_param->input_height);
	drv_scale_param.v_mul = (UINT16)(scale_param->output_height);
	ret = ge_gma_scale(ge_dev,layer_id,GE_SET_SCALE_PARAM,(UINT32)&drv_scale_param);
	GFX_UNLOCK(st_hw_surface_list->surface_mutex);//lock surface
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_scale failed \n");
		goto ERROR;
	}

	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/*
 *	@brief			scale the gma layer, this is useful when change tv system.
 *
 *	@param[in]		gfx_layer_handle	OSD layer handle.
 *	@param[in]		scale_param			scale parameters.
 *
 *	@return			AUI_RTN_CODE
 *
 *	@author			Fawn Fu
 *
 *	@note			
 */
AUI_RTN_CODE aui_gfx_layer_scale(aui_hdl gfx_layer_handle, const aui_scale_param *scale_param)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *layer_info = NULL;
	struct ge_device* ge_dev;
	unsigned long layer_id;

	static osd_scale_param drv_scale_param;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
    if((NULL == scale_param) || (0 == scale_param->input_height) || (0 == scale_param->input_width) || \
       (0 == scale_param->output_height) || (0 == scale_param->output_width)) 
    {
        ret = AUI_GFX_PARAM_INVALID;
        AUI_ERR("input scale param is invalid\n");
        GFX_UNLOCK(gfx_mutex);
        goto ERROR;
    }
    
	if(NULL == gfx_layer_handle)
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR; 	
	}	
	layer_info = (gfx_layer_info *)gfx_layer_handle;
	GFX_LOCK(layer_info->layer_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);
    
	ge_dev = get_ge_device();
	layer_id = layer_info->layer_id;
	//caculate the scale param,horizontal scale = h_mul / h_div,vertical scale = v_mul / v_div
	drv_scale_param.h_div = (UINT16)(scale_param->input_width);
	drv_scale_param.h_mul = (UINT16)(scale_param->output_width);
	drv_scale_param.v_div = (UINT16)(scale_param->input_height);
	drv_scale_param.v_mul = (UINT16)(scale_param->output_height);
	ret = ge_gma_scale(ge_dev,layer_id,GE_SET_SCALE_PARAM,(UINT32)&drv_scale_param);
	GFX_UNLOCK(layer_info->layer_mutex);//lock surface
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_scale failed \n");
		goto ERROR;
	}

	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}



AUI_RTN_CODE aui_gfx_surface_colorkey_set(aui_hdl p_surface_handle,aui_color_key *color_key)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_hw_surface_list = NULL;

	GFX_FUNCTION_ENTER;

	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) || (NULL == color_key))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_hw_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	MEMCPY(&(st_hw_surface_list->st_surface_info.color_key),color_key,sizeof(aui_color_key));
	GFX_UNLOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}



AUI_RTN_CODE aui_gfx_surface_colorkey_get(aui_hdl p_surface_handle,aui_color_key *colorkey)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_hw_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) || (NULL == colorkey))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_hw_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	MEMCPY(colorkey,&st_hw_surface_list->st_surface_info.color_key,sizeof(aui_color_key));

	GFX_UNLOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

AUI_RTN_CODE aui_gfx_surface_clip_rect_set(aui_hdl p_surface_handle,struct aui_osd_rect *rect,enum aui_ge_clip_mode clip_mode)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle))|| (NULL == rect))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_surface_list->surface_mutex);//unlock surface
	GFX_UNLOCK(gfx_mutex);

	MEMCPY(&(st_surface_list->st_surface_info.clip_rect),rect,sizeof(struct aui_osd_rect));
	st_surface_list->st_surface_info.clip_mode = clip_mode;

	GFX_UNLOCK(st_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


AUI_RTN_CODE aui_gfx_surface_clip_rect_clear(aui_hdl p_surface_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	MEMSET(&(st_surface_list->st_surface_info.clip_rect),0,sizeof(struct aui_osd_rect));
	st_surface_list->st_surface_info.clip_mode = AUI_GE_CLIP_DISABLE;

	GFX_UNLOCK(st_surface_list->surface_mutex);//lock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


AUI_RTN_CODE aui_gfx_surface_pallette_set(aui_hdl p_surface_handle,aui_pallette_info *p_pallette_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct ge_device* ge_dev;
	unsigned long layer_id;
	ge_pal_attr_t pallette_attr;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) || (NULL == p_pallette_info))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)p_surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	ge_dev = get_ge_device();
	layer_id = st_dst_surface_list->st_surface_info.hw_surface_info.layer_id;
	MEMSET(&pallette_attr,0,sizeof(ge_pal_attr_t));
	pallette_attr.pal_type = GE_PAL_RGB;
	pallette_attr.rgb_order = GE_RGB_ORDER_ARGB;
	pallette_attr.alpha_range = GE_ALPHA_RANGE_0_255;
	pallette_attr.alpha_pol = GE_ALPHA_POLARITY_0;
	//only support	  setting clip to hw surface
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		//MEMCPY(&st_dst_surface_list.st_surface_info.clip_rect,rect,sizeof(RECT));
		//st_dst_surface_list.st_surface_info.clip_rect
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("only support  setting clip to hw surface \n");
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
		goto ERROR;
	}
	else
	{
		if(AUI_PALLETTE_COLOR_TYPE_RGB == p_pallette_info->en_pallette_color_type)
		{
			ret = ge_gma_set_pallette(ge_dev,layer_id,p_pallette_info->p_pallette,p_pallette_info->color_cnt,&pallette_attr);
		}
		else
		{
			ret = ge_gma_set_pallette(ge_dev,layer_id,p_pallette_info->p_pallette,p_pallette_info->color_cnt,NULL);
		}
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("OSDDrv_region_pallette_set failed \n");
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
			goto ERROR;
		}
	}
	//save pallette info to surface
	MEMCPY(&st_dst_surface_list->st_surface_info.pallettte_info,p_pallette_info,sizeof(aui_pallette_info));

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


AUI_RTN_CODE aui_gfx_surface_pallette_get(aui_hdl p_surface_handle,aui_pallette_info *p_pallette_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) || (NULL == p_pallette_info))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)p_surface_handle;

	MEMCPY(p_pallette_info,&st_dst_surface_list->st_surface_info.pallettte_info,sizeof(aui_pallette_info));
	//GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_UNLOCK(gfx_mutex);
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

AUI_RTN_CODE aui_gfx_surface_galpha_set(aui_hdl p_surface_handle,unsigned long global_alpha)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_surface_list = (surface_list *)p_surface_handle;
	st_surface_list->st_surface_info.golbal_alpha = global_alpha;

	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

AUI_RTN_CODE aui_gfx_surface_galpha_get(aui_hdl p_surface_handle,unsigned long *global_alpha)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_surface_list = (surface_list *)p_surface_handle;
	*global_alpha = st_surface_list->st_surface_info.golbal_alpha;

	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);	 
}


/**
 *	@brief			render a image to a surface
 *
 *	@param[in]		surface_handle		destination surface
 *	@param[in]		image_path			the path of the input image
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			2/17/2014  14:32:6
 *
 *	@note			only for linux platform
 */
AUI_RTN_CODE aui_gfx_render_image_to_surface(aui_hdl surface_handle,
											 const char *image_path)
{
    (void) surface_handle;
    (void) image_path;
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
}

/**
 *	@brief			get the width/height/pitch of a specified image
 *
 *	@param[in]		*image_path 	the path of the image
 *	@param[out] 	*width			image width
 *	@param[out] 	*height 		image height
 *	@param[out] 	*pitch			pitch
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			2/17/2014  16:40:56
 *
 *	@note			only for linux platform
 */
AUI_RTN_CODE aui_gfx_get_image_info(const char *image_path,
									int *width,
									int *height)
{
    (void) image_path;
    (void) width;
    (void) height;
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
}

/**
 *	@brief			decode a specified image file
 *
 *	@param[out] 	pp_bitmap		output image data
 *	@param[in]		dst_format		the pixel format of the output bitmap
 *	@param[in]		image_buf		input image data
 *	@param[in]		buf_size		size of the input image data
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			2/17/2014  19:27:52
 *
 *	@note			only for linux platform
 */
AUI_RTN_CODE aui_gfx_image_decode(aui_gfx_bitmap_info_t **pp_bitmap,
								  enum aui_osd_pixel_format dst_format,
								  const char *image_buf,
								  int buf_size)
{
    (void) pp_bitmap;
    (void) dst_format;
    (void) image_buf;
    (void) buf_size;
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
}

/**
 *	@brief			release a bitmap data
 *
 *	@param[in]		p_bitmap		point to the bitmap structure want to be released
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			2/17/2014  19:30:22
 *
 *	@note			only for linux platform
 */
AUI_RTN_CODE aui_gfx_image_release(aui_gfx_bitmap_info_t *p_bitmap)
{
    (void ) p_bitmap;
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
}


/**
 *	@brief			lock a surface, get the buffer address
 *
 *	@param[in]		p_surface_handle		the handle of the suface wait to locked
 *
 *	@return			AUI_RTN_CODE
 *
 *	@author			Peter Pan <peter.pan@alitech.com>
 *	@date			3/28/2014  15:13:32
 *
 *	@note			This must be done if you want get the surface buffer address\n
 *					by call aui_gfx_surface_info_get.
 */
AUI_RTN_CODE aui_gfx_surface_lock(aui_hdl p_surface_handle)
{
    (void) p_surface_handle;
	return AUI_GFX_SUCCESS;
}

/**
 *	@brief			unlock a surface
 *
 *	@param[in]		p_surface_handle		the handle of the suface wait to unlocked
 *
 *	@return			AUI_RTN_CODE
 *
 *	@author			Peter Pan <peter.pan@alitech.com>
 *	@date			3/28/2014  15:13:32
 *
 *	@note			This must be done after surface buffer read/write complete.
 */
AUI_RTN_CODE aui_gfx_surface_unlock(aui_hdl p_surface_handle)
{
    (void) p_surface_handle;
	return AUI_GFX_SUCCESS;
}

extern RET_CODE ge_set_matrix(struct ge_device *dev, UINT32 cmd_pos, const double m[6]);
AUI_RTN_CODE aui_gfx_surface_affine_matrix_transform(aui_hdl p_src_surface_handle, aui_hdl p_dst_surface_handle, aui_gfx_affine_matrix *p_affine_matrix, aui_blit_rect *p_tran_rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *src_surface_list = NULL;
    surface_list *dst_surface_list = NULL;
    struct ge_device *ge_dev = NULL;
    ge_base_addr_t dst_base_addr;
	ge_base_addr_t src_base_addr;
    double matrix[6];
    ge_cmd_list_hdl cmd_list = (unsigned long)NULL;
    unsigned long cmd_handle;
    unsigned long src_pitch = 0;
    unsigned long dst_pitch = 0;
    

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if((NULL == p_src_surface_handle) || (FALSE == surface_list_check(p_src_surface_handle))||(NULL == p_dst_surface_handle) || (FALSE == surface_list_check(p_dst_surface_handle))\
     ||(p_src_surface_handle==p_dst_surface_handle) || (p_affine_matrix == NULL))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	src_surface_list = (surface_list *)p_src_surface_handle;
    dst_surface_list = (surface_list *)p_dst_surface_handle;

    GFX_LOCK(src_surface_list->surface_mutex);
    GFX_LOCK(dst_surface_list->surface_mutex);

	GFX_UNLOCK(gfx_mutex);

    src_pitch = src_surface_list->st_surface_info.width;
    dst_pitch = dst_surface_list->st_surface_info.width;
        
    cmd_list = dst_surface_list->st_ge_private_info.cmd_list_handle;
    
    src_base_addr.base_address = (unsigned long)src_surface_list->st_surface_info.p_surface_buf;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.color_format = osd_color_mode2ge(src_surface_list->st_surface_info.en_color_mode);
    src_base_addr.data_decoder = GE_DECODER_DISABLE;
    src_base_addr.pixel_pitch = src_pitch;
    src_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    dst_base_addr.base_address = (unsigned long)dst_surface_list->st_surface_info.p_surface_buf;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.color_format = osd_color_mode2ge(dst_surface_list->st_surface_info.en_color_mode);
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.pixel_pitch = dst_pitch;
    dst_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    ge_dev = get_ge_device();

    ret|=ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
    cmd_handle = ge_cmd_begin(ge_dev, cmd_list, GE_DRAW_BITMAP);
    ret|=ge_set_base_addr(ge_dev, cmd_handle, GE_PTN, &src_base_addr);
    ret|=ge_set_xy(ge_dev, cmd_handle, GE_PTN, p_tran_rect->fg_rect.uLeft, p_tran_rect->fg_rect.uTop);
    ret|=ge_set_wh(ge_dev, cmd_handle, GE_PTN, p_tran_rect->fg_rect.uWidth, p_tran_rect->fg_rect.uHeight); 

    matrix[0] = p_affine_matrix->a_0_0;
    matrix[1] = p_affine_matrix->a_0_1;
    matrix[2] = p_affine_matrix->a_0_2;
    matrix[3] = p_affine_matrix->a_1_0;
    matrix[4] = p_affine_matrix->a_1_1;
    matrix[5] = p_affine_matrix->a_1_2;

    ret|=ge_set_onoff_matrix(ge_dev,cmd_handle,1);
    ret|=ge_set_matrix(ge_dev,cmd_handle,matrix);

    ret|=ge_set_base_addr(ge_dev, cmd_handle, GE_DST, &dst_base_addr);
    ret|=ge_set_xy(ge_dev, cmd_handle, GE_DST, p_tran_rect->dst_rect.uLeft, p_tran_rect->dst_rect.uTop);
    ret|=ge_set_wh(ge_dev, cmd_handle, GE_DST, p_tran_rect->dst_rect.uWidth, p_tran_rect->dst_rect.uHeight); 

    ret|= ge_cmd_end(ge_dev, cmd_handle);
    ret|= ge_cmd_list_end(ge_dev,cmd_list); 

    GFX_UNLOCK(src_surface_list->surface_mutex);
    GFX_UNLOCK(dst_surface_list->surface_mutex);

    if(ret!=SUCCESS)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("driver operation fail!\n");
		goto ERROR;
	}
    
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);	    
}



static enum aui_ge_pixel_format_type_em mask_format2ge(enum aui_gfx_mask_format mask_format)
{
    enum aui_ge_pixel_format_type_em ge_mask_format = AUI_GE_PF_A1;
    switch(mask_format)
    {
        case AUI_GFX_MASK_1BIT:
            ge_mask_format = AUI_GE_PF_A1;
            break;
        case AUI_GFX_MASK_8BIT:
            ge_mask_format = AUI_GE_PF_A8;
            break;
        default:
            ge_mask_format = AUI_GE_PF_A8;
            break;
    }
    return ge_mask_format;
}



AUI_RTN_CODE aui_gfx_surface_mask_filter(aui_hdl p_src_surface_handle, aui_hdl p_dst_surface_handle, aui_gfx_mask_filter *p_mask_filter,aui_blit_rect *p_tran_rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *src_surface_list = NULL;
    surface_list *dst_surface_list = NULL;
    struct ge_device *ge_dev = NULL;
    ge_base_addr_t dst_base_addr;
	ge_base_addr_t src_base_addr;
    ge_base_addr_t msk_base_addr;
    ge_cmd_list_hdl cmd_list = (unsigned long)NULL;
    unsigned long cmd_handle;
    unsigned long src_pitch = 0;
    unsigned long dst_pitch = 0;
    

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if((NULL == p_src_surface_handle) || (FALSE == surface_list_check(p_src_surface_handle))||(NULL == p_dst_surface_handle) || (FALSE == surface_list_check(p_dst_surface_handle))\
     ||(p_src_surface_handle==p_dst_surface_handle) || (p_mask_filter == NULL) || (p_mask_filter->mask_format >= AUI_GFX_MASK_MAX) || (p_mask_filter->mask_buf == NULL))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	src_surface_list = (surface_list *)p_src_surface_handle;
    dst_surface_list = (surface_list *)p_dst_surface_handle;

    GFX_LOCK(src_surface_list->surface_mutex);
    GFX_LOCK(dst_surface_list->surface_mutex);

	GFX_UNLOCK(gfx_mutex);

    src_pitch = src_surface_list->st_surface_info.width;
    dst_pitch = dst_surface_list->st_surface_info.width;
        
    cmd_list = dst_surface_list->st_ge_private_info.cmd_list_handle;
    
    src_base_addr.base_address = (unsigned long)src_surface_list->st_surface_info.p_surface_buf;
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.color_format = osd_color_mode2ge(src_surface_list->st_surface_info.en_color_mode);
    src_base_addr.data_decoder = GE_DECODER_DISABLE;
    src_base_addr.pixel_pitch = src_pitch;
    src_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    dst_base_addr.base_address = (unsigned long)dst_surface_list->st_surface_info.p_surface_buf;
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.color_format = osd_color_mode2ge(dst_surface_list->st_surface_info.en_color_mode);
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.pixel_pitch = dst_pitch;
    dst_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    msk_base_addr.base_address = (unsigned long)p_mask_filter->mask_buf;
    msk_base_addr.base_addr_sel = GE_BASE_ADDR;
    msk_base_addr.color_format = mask_format2ge(p_mask_filter->mask_format);
    msk_base_addr.data_decoder = GE_DECODER_DISABLE;
    msk_base_addr.pixel_pitch = p_mask_filter->pixel_per_line;
    msk_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

    ge_dev = get_ge_device();

    ret|=ge_cmd_list_new(ge_dev, cmd_list, GE_COMPILE_AND_EXECUTE);
    cmd_handle = ge_cmd_begin(ge_dev, cmd_list, GE_DRAW_BITMAP_ALPHA_BLENDING);
    ret|=ge_set_base_addr(ge_dev, cmd_handle, GE_PTN, &src_base_addr);
    ret|=ge_set_xy(ge_dev, cmd_handle, GE_PTN, p_tran_rect->bg_rect.uLeft, p_tran_rect->bg_rect.uTop);
    ret|=ge_set_wh(ge_dev, cmd_handle, GE_PTN, p_tran_rect->bg_rect.uWidth, p_tran_rect->bg_rect.uHeight); 

    ret|=ge_set_base_addr(ge_dev, cmd_handle, GE_SRC, &dst_base_addr);
    ret|=ge_set_xy(ge_dev, cmd_handle, GE_SRC, p_tran_rect->dst_rect.uLeft, p_tran_rect->dst_rect.uTop);
    ret|=ge_set_wh(ge_dev, cmd_handle, GE_SRC, p_tran_rect->dst_rect.uWidth, p_tran_rect->dst_rect.uHeight);

    ret|=ge_set_base_addr(ge_dev, cmd_handle, GE_DST, &dst_base_addr);
    ret|=ge_set_xy(ge_dev, cmd_handle, GE_DST, p_tran_rect->dst_rect.uLeft, p_tran_rect->dst_rect.uTop);
    ret|=ge_set_wh(ge_dev, cmd_handle, GE_DST, p_tran_rect->dst_rect.uWidth, p_tran_rect->dst_rect.uHeight); 

    ret|=ge_set_base_addr(ge_dev, cmd_handle, GE_MSK, &msk_base_addr);
    ret|=ge_set_xy(ge_dev, cmd_handle, GE_MSK, p_tran_rect->fg_rect.uLeft, p_tran_rect->fg_rect.uTop);
    ret|=ge_set_wh(ge_dev, cmd_handle, GE_MSK, p_tran_rect->fg_rect.uWidth, p_tran_rect->fg_rect.uHeight); 

    ret|=ge_set_msk_mode(ge_dev, cmd_handle, GE_MSK_ENABLE);

    ret|= ge_cmd_end(ge_dev, cmd_handle);
    ret|= ge_cmd_list_end(ge_dev,cmd_list);

    GFX_UNLOCK(src_surface_list->surface_mutex);
    GFX_UNLOCK(dst_surface_list->surface_mutex);

    if(ret!=SUCCESS)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("driver operation fail!\n");
		goto ERROR;
	}
    
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);	     
}

/**
 *	@brief			create a software surface by use preallocated bitmap source
 *
 *	@param[out] 	p_surface_handle		returned surface handle
 *	@param[in]		p_bitmap_info			bitmap information
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			2/17/2014  14:29:17
 *
 *	@note			
 */
AUI_RTN_CODE aui_gfx_sw_surface_create_by_bitmap(aui_hdl *p_surface_handle,
												 const aui_gfx_bitmap_info_t *p_bitmap_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;
	struct ge_device*	ge_device;
	ge_cmd_list_hdl  cmd_list;
	OSAL_ID surface_mutex = INVALID_ID;
    unsigned char *data = NULL;
    unsigned long pitch = 0;
    unsigned long width = 0;
    unsigned long height = 0;
    enum aui_osd_pixel_format e_color_mode = AUI_OSD_HD_ARGB8888;

	GFX_FUNCTION_ENTER;
    if((NULL==p_surface_handle)||(NULL==p_bitmap_info)||(NULL==p_bitmap_info->p_data))
    {
        ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("param invalid.\n");
		goto ERROR; 	
    }

    data = p_bitmap_info->p_data;
    pitch = p_bitmap_info->pitch;
    width = p_bitmap_info->width;
    height = p_bitmap_info->height;
    e_color_mode = p_bitmap_info->color_type;
    
	surface_mutex = osal_mutex_create();
	if(INVALID_ID == surface_mutex)
	{
		ret = AUI_GFX_OTHTER_ERROR;
		AUI_ERR("osal_mutex_create failed.\n");
		goto ERROR; 	

	}
	//malloc surface list 
	st_surface_list = GFX_MALLOC(sizeof(surface_list));
	if( NULL == st_surface_list )
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed.\n");
		goto ERROR; 		
	}
	
	MEMSET(st_surface_list,0,sizeof(surface_list));
	//add info to st_hw_surface_list 
	st_surface_list->surface_mutex = surface_mutex;
	st_surface_list->st_surface_info.is_hw_surface = FALSE;
	st_surface_list->st_surface_info.width = width;
	st_surface_list->st_surface_info.height = height;
	st_surface_list->st_surface_info.en_color_mode = e_color_mode;
	st_surface_list->st_surface_info.pitch = pitch; 
	st_surface_list->st_surface_info.buf_size = st_surface_list->st_surface_info.pitch * height;
	st_surface_list->st_surface_info.p_surface_buf_backup = NULL;
 
	st_surface_list->st_surface_info.p_surface_buf = data; //(unsigned char *)((unsigned long)data | 0xa0000000);

    //sync code from //depot/Customer/SDK4.0/TPE/Korea/humax/Full_13.3@changelist 248488
    osal_cache_invalidate(st_surface_list->st_surface_info.p_surface_buf,
						  st_surface_list->st_surface_info.buf_size);
	ge_device = get_ge_device();
	cmd_list = ge_cmd_list_create(ge_device, 1);	
	if(0 == cmd_list)
	{
		AUI_ERR("ge_cmd_list_create failed\n");
		ret = AUI_GFX_DRIVER_ERROR;
		goto ERROR;
	}	
	st_surface_list->st_ge_private_info.cmd_list_handle = cmd_list;
	
	// add the surface to the link list
	surface_list_add(st_surface_list);

	*p_surface_handle = st_surface_list;
	GFX_FUNCTION_LEAVE;
	return ret;

	ERROR:
	if(INVALID_ID != surface_mutex)
	{
		osal_mutex_delete(surface_mutex);
	}	
	//free the malloc buffer
	if(st_surface_list)
	{
		GFXF_FREE(st_surface_list);
	}
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);  
}


AUI_RTN_CODE aui_gfx_create_surface_by_rle_pixmap(aui_hdl *p_surface_handle, enum aui_osd_pixel_format e_color_mode, unsigned long width, unsigned long height, unsigned char *data, unsigned long data_length)
{
    AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;
	struct ge_device*	ge_device;
	ge_cmd_list_hdl  cmd_list;
	OSAL_ID surface_mutex = INVALID_ID;
    unsigned long cmd_handle = (unsigned long)NULL;
    ge_base_addr_t dst_base_addr;
	ge_base_addr_t src_base_addr;

	GFX_FUNCTION_ENTER;
    if((NULL==p_surface_handle)||(NULL==data))
    {
        ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("param invalid.\n");
		goto ERROR; 	
    }
    
	surface_mutex = osal_mutex_create();
	if(INVALID_ID == surface_mutex)
	{
		ret = AUI_GFX_OTHTER_ERROR;
		AUI_ERR("osal_mutex_create failed.\n");
		goto ERROR; 	

	}
	//malloc surface list 
	st_surface_list = GFX_MALLOC(sizeof(surface_list));
	if( NULL == st_surface_list )
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed.\n");
		goto ERROR; 		
	}
	
	MEMSET(st_surface_list,0,sizeof(surface_list));
	//add info to st_hw_surface_list 
	st_surface_list->surface_mutex = surface_mutex;
	st_surface_list->st_surface_info.is_hw_surface = FALSE;
	st_surface_list->st_surface_info.width = width;
	st_surface_list->st_surface_info.height = height;
	st_surface_list->st_surface_info.en_color_mode = e_color_mode;
	st_surface_list->st_surface_info.pitch = get_pitch(e_color_mode,width); 
	st_surface_list->st_surface_info.buf_size = st_surface_list->st_surface_info.pitch * height;
    st_surface_list->st_surface_info.p_surface_buf_backup = GFX_MALLOC(st_surface_list->st_surface_info.buf_size + MIN_ALIGN);
    if(NULL == st_surface_list->st_surface_info.p_surface_buf_backup)
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed\n");
		goto ERROR; 		
	}
	st_surface_list->st_surface_info.p_surface_buf = (unsigned char*)(((unsigned long)(st_surface_list->st_surface_info.p_surface_buf_backup + MIN_ALIGN)|(0xa<<28)) & MIN_ALIGN_MASK);

	ge_device = get_ge_device();
	cmd_list = ge_cmd_list_create(ge_device, 1);	
	if(0 == cmd_list)
	{
		AUI_ERR("ge_cmd_list_create failed\n");
		ret = AUI_GFX_DRIVER_ERROR;
		goto ERROR;
	}	
	st_surface_list->st_ge_private_info.cmd_list_handle = cmd_list;

    osal_cache_flush(data,data_length);

    src_base_addr.base_address = (unsigned long)data|(0xa<<28);
    src_base_addr.color_format = osd_color_mode2ge(e_color_mode);
    src_base_addr.base_addr_sel = GE_BASE_ADDR;
    src_base_addr.data_decoder = GE_DECODER_RLE;
    src_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
    src_base_addr.pixel_pitch = width;
    
    dst_base_addr.base_address = (unsigned long)st_surface_list->st_surface_info.p_surface_buf;
    dst_base_addr.color_format = osd_color_mode2ge(e_color_mode);
    dst_base_addr.base_addr_sel = GE_BASE_ADDR;
    dst_base_addr.data_decoder = GE_DECODER_DISABLE;
    dst_base_addr.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
    dst_base_addr.pixel_pitch = width;

    ret|=ge_cmd_list_new(ge_device, cmd_list, GE_COMPILE_AND_EXECUTE);
	cmd_handle = ge_cmd_begin(ge_device, cmd_list, GE_DRAW_BITMAP);
    ret|=ge_set_base_addr(ge_device, cmd_handle, GE_PTN, &src_base_addr);
    ret|=ge_set_xy(ge_device, cmd_handle, GE_PTN, 0, 0);
    ret|=ge_set_wh(ge_device, cmd_handle, GE_PTN, width, height); 

    ret|=ge_set_base_addr(ge_device, cmd_handle, GE_DST, &dst_base_addr);
    ret|=ge_set_xy(ge_device, cmd_handle, GE_DST, 0, 0);
    ret|=ge_set_wh(ge_device, cmd_handle, GE_DST, width, height); 

    ret |= ge_cmd_end(ge_device, cmd_handle);
	ret |= ge_cmd_list_end(ge_device,cmd_list); 
	
	// add the surface to the link list
	surface_list_add(st_surface_list);

	*p_surface_handle = st_surface_list;
	GFX_FUNCTION_LEAVE;
	return ret;

	ERROR:
	if(INVALID_ID != surface_mutex)
	{
		osal_mutex_delete(surface_mutex);
	}	
	//free the malloc buffer
	if(st_surface_list)
	{
		GFXF_FREE(st_surface_list);
	}
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL); 
}

/**
*    @brief     Configure enhance value of OSD layer
*    @author 	Vedic.Fu@alitech.com
*    @date      2015-8-7
*    @param[in] gfx_layer_handle    OSD layer handle
*    @param[in] en_type          enhance type
*    @param[in] en_value           enhance value
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_VAL(AUI_MODULE_GFX, AUI_GFX_PARAM_INVALID) if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_enhance_set(aui_hdl gfx_layer_handle,
										 enum aui_osd_enhance  en_type,
										 unsigned int en_value)
{
    (void)gfx_layer_handle;
    (void)en_type;
    (void)en_value;
	return AUI_GFX_SUCCESS;
}


