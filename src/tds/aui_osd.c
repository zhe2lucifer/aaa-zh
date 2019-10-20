/**  @file
*	 @brief 	AUI OSD module
*	 @author	 andy.yu
*	 @date		   2013-6-18
*	 @version	  1.0.0
*	 @note		   ali corp. all rights reserved. 2013-2999 copyright (C)
*/

#include "aui_common_priv.h"
#include <aui_osd.h>
#include <hld/osd/osddrv.h>
#include <hld/osd/osddrv_dev.h>
#include <mediatypes.h>
#include <hld/hld_dev.h>
#include <aui_common.h>

AUI_MODULE(GFX)

#define INVALID_ID 0xFFFF

static OSAL_ID gfx_mutex = INVALID_ID;

#define MAX_OSD_LAYER 3

#define GFX_FUNCTION_ENTER AUI_INFO("enter\n");
#define GFX_FUNCTION_LEAVE AUI_INFO("leave\n");

#define GFX_LOCK(mutex)   osal_mutex_lock(mutex,OSAL_WAIT_FOREVER_TIME)
#define GFX_UNLOCK(mutex) osal_mutex_unlock(mutex)

#define GFX_MALLOC MALLOC
#define GFXF_FREE FREE

typedef struct gfx_layer_info
{
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

typedef struct surface_list
{
	//surface link list table
	struct surface_list    *st_next_list;
	aui_surface_info st_surface_info;
	OSAL_ID surface_mutex;
}surface_list;

surface_list	st_root_list;

//osd layer info
static gfx_layer_info st_gfx_layer_info[MAX_OSD_LAYER];// = {0};

static const unsigned long pixel_mask4bpp[] =
{
	// 0
	0x00000000, 0x000000f0, 0x000000ff, 0x0000f0ff,
	0x0000ffff, 0x00f0ffff, 0x00ffffff, 0xf0ffffff,
	// 1
	0x00000000, 0x0000000f, 0x0000f00f, 0x0000ff0f,
	0x00f0ff0f, 0x00ffff0f, 0xf0ffff0f, 0xffffff0f,
	// 2
	0x00000000, 0x0000f000, 0x0000ff00, 0x00f0ff00,
	0x00ffff00, 0xf0ffff00, 0xffffff00, 0xffffff00,
	// 3
	0x00000000, 0x00000f00, 0x00f00f00, 0x00ff0f00,
	0xf0ff0f00, 0xffff0f00, 0xffff0f00, 0xffff0f00,
	// 4
	0x00000000, 0x00f00000, 0x00ff0000, 0xf0ff0000,
	0xffff0000, 0xffff0000, 0xffff0000, 0xffff0000,
	// 5
	0x00000000, 0x000f0000, 0xf00f0000, 0xff0f0000,
	0xff0f0000, 0xff0f0000, 0xff0f0000, 0xff0f0000,
	// 6
	0x00000000, 0xf0000000, 0xff000000, 0xff000000,
	0xff000000, 0xff000000, 0xff000000, 0xff000000,
	// 7
	0x00000000, 0x0f000000
};

static inline BOOL check_operate_rect(struct aui_osd_rect *rect, unsigned long u_width, unsigned long u_height)
{
	if(((rect->uLeft + rect->uWidth) > u_width) || \
		((rect->uTop + rect->uHeight) > u_height))
	{
		return FALSE;
	}
	return TRUE;
}

static inline BOOL check_operate_coordinate(aui_coordinate *p_point, unsigned long u_width, unsigned long u_height)
{
	if((p_point->X > u_width) || (p_point->Y > u_height))
	{
		return FALSE;
	}
	return TRUE;
}

static enum osdcolor_mode aui_color_mode_tran_osd(enum aui_osd_pixel_format color_mode)
{
	switch(color_mode)
	{
		case AUI_OSD_4_COLOR:
			return OSD_4_COLOR;
			break;
		case AUI_OSD_256_COLOR:
			return OSD_256_COLOR;
			break;
		case AUI_OSD_16_COLOR_PIXEL_ALPHA:
			return OSD_16_COLOR_PIXEL_ALPHA;
			break;
		case AUI_OSD_HD_ACLUT88:
			return OSD_HD_ACLUT88;
			break;
		 case AUI_OSD_HD_RGB565:
			return OSD_HD_RGB565;
			break;
		 case AUI_OSD_HD_RGB888:
			return OSD_HD_RGB888;
			break;
		 case AUI_OSD_HD_RGB555:
			return OSD_HD_RGB555;
			break;
		 case AUI_OSD_HD_RGB444:
			return OSD_HD_RGB444;
			break;
		 case AUI_OSD_HD_ARGB8888:
			return OSD_HD_ARGB8888;
			break;
		 case AUI_OSD_HD_ARGB1555:
			return OSD_HD_ARGB1555;
			break;
		 case AUI_OSD_HD_ARGB4444:
			return OSD_HD_ARGB4444;
			break;
		 case AUI_OSD_HD_AYCbCr8888:
			return osd_hd_aycb_cr8888;
			break;
		 case AUI_OSD_HD_YCBCR888:
			return OSD_HD_YCBCR888;
			break;
		 default:
			return OSD_HD_ARGB1555;
			break;
	}
}

static enum clipmode ge_clip_mode2osd_mode(enum aui_ge_clip_mode ge_clip_mode)
{
	enum clipmode osd_clip_mode;

	switch(ge_clip_mode)
	{
		case AUI_GE_CLIP_DISABLE:
			osd_clip_mode	 = CLIP_OFF;
			break;
		case AUI_GE_CLIP_INSIDE:
			osd_clip_mode	 = CLIP_INSIDE_RECT;
			break;
		case AUI_GE_CLIP_OUTSIDE:
			osd_clip_mode	 = CLIP_OUTSIDE_RECT;
			break;
		default:
			osd_clip_mode	 = CLIP_OFF;
			break;
	}
	return osd_clip_mode;
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


//check input layer handle is valid
static BOOL layer_handle_check(gfx_layer_info *st_gfx_layer_handle)
{
	unsigned long i;

	for(i = 0;i < MAX_OSD_LAYER;i++)
	{
		if(&st_gfx_layer_info[i] == st_gfx_layer_handle)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//get osd layer open count
static unsigned long get_open_cnt(unsigned long osd_layer_id)
{
	unsigned long open_cnt = 0;

	open_cnt = st_gfx_layer_info[osd_layer_id].open_cnt;
	return open_cnt;
}

static void set_layer_mutex(OSAL_ID layer_mutex,unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].layer_mutex = layer_mutex;
}

//it will call when open the osd layer
static void incress_open_cnt(unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].open_cnt++;
}

//it will call when close the osd layer
static void redeuce_open_cnt(unsigned long osd_layer_id)
{
	if(st_gfx_layer_info[osd_layer_id].open_cnt > 0)
	{
		st_gfx_layer_info[osd_layer_id].open_cnt--;
	}
}

//set layer id
static void set_layer_id(unsigned long osd_layer_id)
{
	st_gfx_layer_info[osd_layer_id].layer_id = osd_layer_id;

}

//set layer global alpha
static void set_layer_galpha(unsigned long osd_layer_id,unsigned long global_alpha)
{
	st_gfx_layer_info[osd_layer_id].layer_id = global_alpha;
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

//set osd device to st_gfx_layer_info
static void set_osd_device(unsigned long osd_layer_id,struct osd_device* osd_dev)
{
	st_gfx_layer_info[osd_layer_id].osd_dev = osd_dev;
}

//get osd device from st_gfx_layer_info
static struct osd_device*  get_osd_device(unsigned long osd_layer_id)
{
	struct osd_device* osd_dev = NULL;

	osd_dev = st_gfx_layer_info[osd_layer_id].osd_dev;
	return osd_dev;
}

//get the st_gfx_layer_info by layer id
static	gfx_layer_info* get_layer_info(unsigned long osd_layer_id)
{
	gfx_layer_info* layer_info = NULL;

	layer_info = &st_gfx_layer_info[osd_layer_id];

	return layer_info;
}

//reset layer info ,it will call where the osd layer close
static void reset_layer_info(unsigned long osd_layer_id)
{
	MEMSET(&st_gfx_layer_info[osd_layer_id],0x0,sizeof(st_gfx_layer_info[0]));
}


//add new surface list
static void surface_list_add(surface_list *new)
{
	surface_list *prev = NULL;

	GFX_LOCK(gfx_mutex);
	for(prev = (surface_list *)(&st_root_list);
        prev->st_next_list != NULL;
        prev = (surface_list *)(prev->st_next_list))
	{

	}
	prev->st_next_list = (struct surface_list *)(new);
	new->st_next_list = NULL;
	GFX_UNLOCK(gfx_mutex);
}

//delete  surface list
static void surface_list_delete(surface_list *list)
{
	surface_list *tmp_list;

	GFX_LOCK(gfx_mutex);
	for(tmp_list = (surface_list *)(&st_root_list);
        tmp_list->st_next_list != NULL;
        tmp_list = (surface_list *)(tmp_list->st_next_list))
	{
		if((surface_list *)(tmp_list->st_next_list) == list)
		{
			tmp_list->st_next_list = (struct surface_list *)(list->st_next_list);
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

	for(tmp_list = (surface_list *)(&st_root_list);
        tmp_list->st_next_list != NULL;
        tmp_list = (surface_list *)(tmp_list->st_next_list))
	{
		if((surface_list *)(tmp_list->st_next_list) == list)
		{
			ret = TRUE;
			break;
		}
	}
	return ret;
}

/**
*	 @brief 		以32 bpp的方式填充一行的内容
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	buf32 所填充的所在行位置的起始地址
*	 @param[in] 	x	  所要填充行的起始位置即第几个点
*	 @param[in] 	w	  所要填充行的宽度
*	 @param[in] 	color  所要填充的颜色32bit形式
*	 @return		无
*	 @note		无
*/
static void fill_line32bpp(unsigned long *buf32, unsigned long x, unsigned long w, unsigned long color)
{
	unsigned long i;

	buf32 += x;
	for (i = 0; i < w; i++)
	{
		*buf32++ = color;
	}
}

/**
*	 @brief 		以16 bpp的方式填充一行的内容
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	buf32 所填充的行位置的起始地址
*	 @param[in] 	x	  所要填充行的起始位置即第几个点，注意这里可能有奇数点的时候，
					注意保护前一个点不要被修改
*	 @param[in] 	w	  所要填充行的宽度，注意这里可能有奇数点的时候，
					注意保护后一个点不要被修改
*	 @param[in] 	color  所要填充的颜色32bit形式
*	 @return		无
*	 @note		无
*/
static void fill_line16bpp(unsigned long *buf32, unsigned long x, unsigned long w, unsigned long color)
{
	unsigned long offset = x & 0x01;
	unsigned long data, mask;
	unsigned long i;

	buf32 += x >> 1;
	if (offset)
	{
		mask = 0xffff0000;
		data = (color & mask) | (*buf32 & ~mask);
		*buf32 = data;
		buf32++;
		x += 1;
		w -= 1;
	}
	if (w == 0)
		return;
	offset = w & 0x01;
	w >>= 1;
	for (i = 0; i < w; i++)
		*buf32++ = color;
	if (offset)
	{
		mask = 0x0000ffff;
		data = (color & mask) | (*buf32 & ~mask);
		*buf32 = data;
	}
}

/**
*	 @brief 		以8 bpp的方式填充一行的内容
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	buf32 所填充的行位置的起始地址
*	 @param[in] 	x	  所要填充行的起始位置即第几个点，注意这里可能不是4个点对齐的时候，
					注意保护前几个点不要被修改
*	 @param[in] 	w	  所要填充行的宽度，注意这里可能不是4个点对齐的时候，
					注意保护前几个点不要被修改
*	 @param[in] 	color  所要填充的颜色32bit形式
*	 @return		无
*	 @note		无
*/
static void fill_line8bpp(unsigned long *buf32, unsigned long x, unsigned long w, unsigned long color)
{
	unsigned long offset = x & 0x03;
	unsigned long pixels = 4 - offset;
	unsigned long data, mask;
	unsigned long i;

	buf32 += x >> 2;
	if (offset)
	{
		if (pixels > w)
			pixels = w;
		offset <<= 4; // offset *= PIXEL_MASK_4_NUM;
		offset += pixels << 1;
		mask = pixel_mask4bpp[offset];
		data = (color & mask) | (*buf32 & ~mask);
		*buf32 = data;
		buf32++;
		x += pixels;
		w -= pixels;
	}
	if (w == 0)
		return;
	offset = w & 0x03;
	w >>= 2;
	for (i = 0; i < w; i++)
		*buf32++ = color;
	if (offset)
	{
		offset <<= 1;
		mask = pixel_mask4bpp[offset];
		data = (color & mask) | (*buf32 & ~mask);
		*buf32 = data;
	}
}

/**
*	 @brief 		以4 bpp的方式填充一行的内容
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	buf32 所填充的位置的起始地址
*	 @param[in] 	x	  所要填充行的起始位置即第几个点，注意这里可能不是4个点对齐的时候，
					注意保护前几个点不要被修改
*	 @param[in] 	w	  所要填充行的宽度，注意这里可能不是4个点对齐的时候，
					注意保护前几个点不要被修改
*	 @param[in] 	color  所要填充的颜色32bit形式
*	 @return		无
*	 @note		无
*/
static void fill_line4bpp(unsigned long *buf32, unsigned long x, unsigned long w, unsigned long color)
{
	unsigned long offset = x & 0x07;
	unsigned long pixels = 8 - offset;
	unsigned long data, mask;
	unsigned long i;

	buf32 += x >> 3;
	if (offset)
	{
		if (pixels > w)
			pixels = w;
		offset <<= 3; // offset *= PIXEL_MASK_4_NUM;
		offset += pixels;
		mask = pixel_mask4bpp[offset];
		data = (color & mask) | (*buf32 & ~mask);
		*buf32 = data;
		buf32++;
		x += pixels;
		w -= pixels;
	}
	if (w == 0)
		return;
	offset = w & 0x07;
	w >>= 3;
	for (i = 0; i < w; i++)
	{
		*buf32++ = color;
	}
	if (offset)
	{
		mask = pixel_mask4bpp[offset];
		data = (color & mask) | (*buf32 & ~mask);
		*buf32 = data;
	}
}


/**
*	 @brief 		复制显示mem 内容从一个surface 到另一个surface，注意为同一颜色模式只支持bpp为8的整数倍的
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	src_ret 要复制的源surface的rect位置信息包括宽高起始坐标 位置
*	 @param[in] 	dst_rect	 要复制到的目的surface的rect位置信息包括宽高起始坐标 位置
*	 @param[in] 	src_pitch	 源surface一行所占的字节数
*	 @param[in] 	dst_pitch 目的surface一行所占的字节数
*	 @param[in] 	bpp  源和目的surface一个像素点占多少bit
*	 @param[in] 	src_buf  源surface显存的起始位置
*	 @param[in] 	dst_buf  目的surface显存的起始位置
*	 @return		无
*	 @note		无
*/
static void surface_copy_data(struct aui_osd_rect *src_ret, struct aui_osd_rect *dst_rect, unsigned long src_pitch, unsigned long dst_pitch,
							  unsigned long bpp, unsigned char *src_buf, unsigned char *dst_buf)
{
	#define BIT_TO_BYE 8
	unsigned long y;
	unsigned char *src_line_buf = NULL;
	unsigned char *dst_line_buf = NULL;
	unsigned long copy_pitch = 0;
	unsigned long src_bse_offset = 0;
	unsigned long dst_bse_offset = 0;

	if(bpp % 8 != 0 || bpp == 0)
	{
		AUI_ERR( "not support bpp == %d surface copy operation\n", bpp);
		return ;
	} 
	else 
	{
		copy_pitch = (src_ret->uWidth * bpp) / BIT_TO_BYE;
		src_bse_offset = (src_ret->uTop * src_pitch) + ((src_ret->uLeft * bpp) / BIT_TO_BYE);
		dst_bse_offset = (dst_rect->uTop * dst_pitch) + ((dst_rect->uLeft * bpp) / BIT_TO_BYE);
		src_line_buf = src_buf + src_bse_offset;
		dst_line_buf = dst_buf + dst_bse_offset;

		for(y = src_ret->uTop; y < src_ret->uHeight;y++) //fixme ,maybe have aligning issue
		{
			MEMCPY(dst_line_buf,src_line_buf,copy_pitch);
			src_line_buf += src_pitch;
			dst_line_buf += dst_pitch;
		}
	}
}

/**
*	 @brief 		填充显存的内容为某种颜色
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	dst_rect	 所要填充的区域信息包括起始位置和宽高信息
*	 @param[in] 	pitch	 所要填充的区域一行所占的字节数
*	 @param[in] 	color_mode 所要填充的显存的颜色模式具体请参考aui_osd_color_mode_type_em
*	 @param[in] 	dst_buf  所要填充的显存的首地址
*	 @param[in] 	color  所要填充的颜色
*	 @return		无
*	 @note		无
*/
static unsigned long surface_fill(struct aui_osd_rect *dst_rect, unsigned long pitch, enum aui_osd_pixel_format color_mode, unsigned char *dst_buf, unsigned long color)
{
	#define BIT_TO_BYE 8
	unsigned long y;
	unsigned char *dst_line_buf = NULL;
	unsigned long dst_bse_offset = 0;
	//unsigned long fill_pitch = 0;
	unsigned long star_y = 0;
	unsigned long end_y = 0;

	star_y = dst_rect->uTop;
	end_y = dst_rect->uHeight + dst_rect->uTop;
	dst_bse_offset = dst_rect->uTop * pitch ;
	//行首地址
	dst_line_buf = dst_buf + dst_bse_offset;
	switch (color_mode)
	{
		case AUI_OSD_256_COLOR:
			color &= 0xff;
			color |= color << 8;
			color |= color << 16;
			for(y = star_y; y < end_y;y++)
			{
				fill_line8bpp((unsigned long *)dst_line_buf,dst_rect->uLeft,dst_rect->uWidth,color);
				dst_line_buf += pitch;
			}
			break;
		case AUI_OSD_HD_ARGB4444:
		case AUI_OSD_HD_ARGB1555:
		case AUI_OSD_HD_ACLUT88:
		case AUI_OSD_HD_RGB565:
			color &= 0xffff;
			color |= color << 16;
			for(y = star_y; y < end_y;y++)
			{
				fill_line16bpp((unsigned long *)dst_line_buf,dst_rect->uLeft,dst_rect->uWidth,color);
				dst_line_buf += pitch;
			}
			break;
		case AUI_OSD_HD_ARGB8888:
		case AUI_OSD_HD_AYCbCr8888:
			for(y = star_y; y < end_y;y++)
			{
				fill_line32bpp((unsigned long *)dst_line_buf,dst_rect->uLeft,dst_rect->uWidth,color);
				dst_line_buf += pitch;
			}
			break;
		default:

			break;
	}
	return RET_SUCCESS;
}

//get how many bit per pixel according to color mode
static unsigned long get_bpp_color_by_color_mode(enum aui_osd_pixel_format en_color_mode)
{
	unsigned long bpp = 0;

	switch(en_color_mode)
	{
		case AUI_OSD_4_COLOR:
			bpp = 2;
			break;
		case AUI_OSD_16_COLOR_PIXEL_ALPHA:
			bpp = 4;
			break;
		case AUI_OSD_256_COLOR:
			bpp = 8;
			break;
		case AUI_OSD_HD_RGB444:
			bpp = 12;
			break;
		case AUI_OSD_HD_RGB555:
			bpp = 15;
		case AUI_OSD_HD_RGB565:
		case AUI_OSD_HD_ACLUT88:
		case AUI_OSD_HD_ARGB1555:
		case AUI_OSD_HD_ARGB4444:
			bpp = 16;
			break;
		case AUI_OSD_HD_RGB888:
		case AUI_OSD_HD_YCBCR888:
			bpp = 24;
			break;
		case AUI_OSD_HD_ARGB8888:
		case AUI_OSD_HD_AYCbCr8888:
			bpp = 32;
			break;
		default:
			AUI_ERR("unknow color mode,mode=%d\n",en_color_mode);
			break;
	}
	return bpp;
}

//delete hw surface
static AUI_RTN_CODE delete_sw_surface(surface_list *st_hw_surface_list)
{
	AUI_RTN_CODE ret = SUCCESS;
	//gfx_layer_info *layer_info = NULL;

	if(st_hw_surface_list->st_surface_info.p_surface_buf)
	{
		FREE(st_hw_surface_list->st_surface_info.p_surface_buf);
	}
	surface_list_delete(st_hw_surface_list);
	FREE(st_hw_surface_list);
	st_hw_surface_list = NULL;
	GFX_FUNCTION_LEAVE;
	return ret;

//ERROR:
	//GFX_FUNCTION_LEAVE;
	//aui_rtn(ret,NULL);
}

//delete sw surface
static AUI_RTN_CODE delete_hw_surface(surface_list *st_sw_surface_list)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *layer_info = NULL;

	layer_info = get_layer_info(st_sw_surface_list->st_surface_info.hw_surface_info.layer_id);

	//delete the osd region
	ret = osddrv_delete_region((unsigned long)layer_info->osd_dev,st_sw_surface_list->st_surface_info.hw_surface_info.hw_surface_id);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_delete_region failed \n");
		goto ERROR;
	}
	if(st_sw_surface_list->st_surface_info.hw_surface_info.isdouble_buf)
	{
		//free the malloc buffer
		if(st_sw_surface_list->st_surface_info.hw_surface_info.second_buf)
		{
			FREE(st_sw_surface_list->st_surface_info.hw_surface_info.second_buf);
		}
	}

	if(st_sw_surface_list->st_surface_info.p_surface_buf)
	{
		FREE(st_sw_surface_list->st_surface_info.p_surface_buf);
	}
	//delete it from the link list
	redeuce_hw_surface_cnt(layer_info->layer_id);
	surface_list_delete(st_sw_surface_list);
	FREE(st_sw_surface_list);
	st_sw_surface_list = NULL;
	return ret;
ERROR:
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		初始化gfx模块
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   call_back，初始化回调函数
*	 @return		 错误码
*	 @note		  gfx模块在使用之前一定要先调用此函数，在调用此函数前需要先初始化显示模块\n
，并且传入一个回调函数，aui_gfx_init里会调用此回调函数，回调函数一般是实现attach的内容
*
*/
AUI_RTN_CODE aui_gfx_init(p_fun_cb p_call_back_init,void *pv_param)
{
	AUI_RTN_CODE ret;

	GFX_FUNCTION_ENTER;
	if(p_call_back_init)
	{
		p_call_back_init(pv_param);
	}

	MEMSET(st_gfx_layer_info,0,sizeof(st_gfx_layer_info));
	MEMSET(&st_root_list,0,sizeof(surface_list));
	gfx_mutex = osal_mutex_create();
	if(INVALID_ID == gfx_mutex)
	{
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
*	 @brief 		去初始化gfx模块
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   call_back，去初始化回调函数
*	 @return		 错误码
*	 @note		  与aui_gfx_init配对使用，在退出gfx模块时调用，在退出前需要确保释放所有gfx的资源
*
*/
AUI_RTN_CODE aui_gfx_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	GFX_FUNCTION_ENTER;

	if(p_call_back_init)
	{
		p_call_back_init(pv_param);
	}
	if(INVALID_ID != gfx_mutex)
	{
		osal_mutex_delete(gfx_mutex);
		gfx_mutex = INVALID_ID;
	}
	GFX_FUNCTION_LEAVE;
	return SUCCESS;
}

/**
*	 @brief 	打开gfx硬件图层
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	layer_id，硬件图层索引，不同IC的最大图层索引可能不同，通常使用layer_id 为0
*	 @param[out]	gfx_layer_handle:硬件图层 handle
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_open(aui_osd_layer layer_id,aui_hdl *gfx_layer_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	struct osd_device*	   osd_dev;
	OSAL_ID layer_mutex;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( layer_id >= MAX_OSD_LAYER)
	{
		AUI_ERR("layer_id > MAX_OSD_LAYER,id=%d\n",layer_id);
		ret = AUI_GFX_PARAM_INVALID;
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	//check the osd layer whether have been opened before.if it have been opend ,just return the layer handle directly
	if(get_open_cnt(layer_id) > 0)
	{
		AUI_ERR( "open_param.layer_id has been opened,id=%d\n", layer_id);
		goto EXIT;
	}
	layer_mutex = osal_mutex_create();
	if(INVALID_ID == layer_mutex)
	{
		AUI_ERR("osal_mutex_create failed\n");
		ret = AUI_GFX_PARAM_INVALID;
		goto ERROR;
	}
	//get osd device
	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD,layer_id);
	if(NULL == osd_dev)
	{
		AUI_ERR("osd layer%d have been open,\n",layer_id);
		ret = AUI_GFX_DRIVER_ERROR;
		osal_mutex_delete(layer_mutex);
		goto ERROR;
	}

	//open osd device
	ret = osddrv_open((unsigned long)osd_dev, NULL);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_open failed\n");
		osal_mutex_delete(layer_mutex);
		goto ERROR;
	}
	set_layer_mutex(layer_mutex,layer_id);
	incress_open_cnt(layer_id);
	set_layer_id(layer_id);
	set_osd_device(layer_id,osd_dev);
	*gfx_layer_handle = get_layer_info(layer_id);

EXIT:
	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	
	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		关闭硬件图层
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle :调用aui_gfx_layer_open返回的句柄
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  此函数与aui_gfx_layer_open配对使用，在关闭硬件图层前上层
需要关闭所创建的属于该图层的硬件surface。
*
*/
AUI_RTN_CODE aui_gfx_layer_close(aui_hdl gfx_layer_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;
	OSAL_ID layer_mutex;

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
	if(get_open_cnt(st_layer_info->layer_id) == 0)
	{
		AUI_ERR( "gfx_layer_handle has been closed,id=%d\n", st_layer_info->layer_id);
		GFX_UNLOCK(gfx_mutex);
		goto EXIT;
	}
	layer_mutex = st_layer_info->layer_mutex;
	GFX_LOCK(layer_mutex);
	redeuce_open_cnt(st_layer_info->layer_id);
	GFX_UNLOCK(gfx_mutex);

	//check if it is the last one,if it is the last one,then close the osd device
	if(get_open_cnt(st_layer_info->layer_id) == 0)
	{
		ret = osddrv_close((unsigned long)(st_layer_info->osd_dev));
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR( "OSDDrv_close failed\n");
			GFX_UNLOCK(layer_mutex);
			goto ERROR;
		}
		reset_layer_info(st_layer_info->layer_id);
	}

	if(get_open_cnt(st_layer_info->layer_id) == 0)
	{
		GFX_UNLOCK(layer_mutex);
		osal_mutex_delete(layer_mutex);
	}
	else
	{
		GFX_UNLOCK(layer_mutex);
	}

EXIT:	 
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		设置硬件图层抗闪烁功能开关
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle ,调用aui_gfx_layer_open返回的句柄
*	 @param[in] 	   onoff: "1" 打开抗闪烁,"0"关闭抗闪烁
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_antifliker_on_off(aui_hdl gfx_layer_handle,unsigned long on_off)
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
	//GFX_UNLOCK(gfx_mutex);
	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);
	GFX_UNLOCK(gfx_mutex);

	ret = osddrv_io_ctl((unsigned long)(st_layer_info->osd_dev),OSD_IO_ENABLE_ANTIFLICK,on_off);
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

/**
*	 @brief 		显示或者隐藏硬件图层
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_hw_surface_handle ,调用aui_gfx_layer_open返回的句柄
*	 @param[in] 	   on_off:"1" 显示硬件图层,"0"隐藏硬件图层
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_show_on_off(aui_hdl gfx_layer_handle,unsigned long on_off)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
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

	ret = osddrv_show_on_off((unsigned long)(st_layer_info->osd_dev),on_off);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_show_on_off failed\n");
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
*	 @brief 		硬件图层全局透明度设置
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	gfx_layer_handle ,调用aui_gfx_layer_open返回的句柄
*	 @param[in] 	alpha，要设置的全局透明度值
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_alpha_set(aui_hdl gfx_layer_handle,unsigned long alpha)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
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

	ret = osddrv_io_ctl((unsigned long)(st_layer_info->osd_dev),OSD_IO_SET_GLOBAL_ALPHA,alpha);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSD_IO_SET_GLOBAL_ALPHA failed\n");
		GFX_UNLOCK(st_layer_info->layer_mutex);
		goto ERROR;
	}
	set_layer_galpha(st_layer_info->layer_id,alpha);

	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		获取硬件图层透明度
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   gfx_layer_handle ,调用aui_gfx_layer_open返回的句柄
*	 @param[out]	alpha，返回的透明度值
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_layer_alpha_get(aui_hdl gfx_layer_handle,unsigned long *alpha)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info;

	GFX_FUNCTION_ENTER;

	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == gfx_layer_handle) || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		goto ERROR;
	}
	st_layer_info = gfx_layer_handle;
	*alpha = st_layer_info->alpha;

	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:

	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		创建软件surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   e_color_mode:要创建的surface的颜色模式
*	 @param[in] 	   width:要创建的surface的宽度
*	 @param[in] 	   height:要创建的surface的高度
*	 @param[out]	p_surface_handle:返回所创建的surface句柄
*	 @return		 错误码
*	 @note		  软件surface的概念是相对于硬件surface所提出来的,它实际上是一片内存加上
一些对该内的描述(e_color_mode,width,height).如果内存足够大，软件surface是可以任意大小的
*
*/
AUI_RTN_CODE aui_gfx_sw_surface_create(enum aui_osd_pixel_format e_color_mode, unsigned long width, unsigned long height,aui_hdl *p_surface_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list = NULL;
    OSAL_ID surface_mutex = INVALID_ID;

	GFX_FUNCTION_ENTER;
	if((e_color_mode > AUI_OSD_COLOR_MODE_MAX)||(p_surface_handle == NULL))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("PARAM INVALID\n");
		goto ERROR;
	}
	//malloc surface list
	st_surface_list = GFX_MALLOC(sizeof(surface_list));
	if( NULL == st_surface_list )
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed\n");
		//GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
    surface_mutex = osal_mutex_create();
	if(INVALID_ID == surface_mutex)
	{
		ret = AUI_GFX_OTHTER_ERROR;
		AUI_ERR("osal_mutex_create failed\n");
		goto ERROR;

	}

	//GFX_UNLOCK(gfx_mutex);
	MEMSET(st_surface_list,0,sizeof(surface_list));
	//add info to st_hw_surface_list
	st_surface_list->surface_mutex = surface_mutex;
	st_surface_list->st_surface_info.is_hw_surface = FALSE;
	st_surface_list->st_surface_info.width = width;
	st_surface_list->st_surface_info.height = height;
	st_surface_list->st_surface_info.en_color_mode = e_color_mode;
	st_surface_list->st_surface_info.pitch = get_pitch(e_color_mode,width);
	st_surface_list->st_surface_info.en_color_mode = e_color_mode;
	//malloc surface addr
	st_surface_list->st_surface_info.buf_size = st_surface_list->st_surface_info.pitch * height;
	st_surface_list->st_surface_info.p_surface_buf = GFX_MALLOC(st_surface_list->st_surface_info.buf_size);
	if(NULL == st_surface_list->st_surface_info.p_surface_buf)
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed\n");
		goto ERROR;
	}
	MEMSET(st_surface_list->st_surface_info.p_surface_buf,0,st_surface_list->st_surface_info.buf_size);
	// add the surface to the link list
	surface_list_add(st_surface_list);

	*p_surface_handle = st_surface_list;
	GFX_FUNCTION_LEAVE;
	return ret;

	ERROR:
	//free the malloc buffer
	if(INVALID_ID != surface_mutex)
	{
        osal_mutex_delete(surface_mutex);
    }
	if(st_surface_list)
	{
		FREE(st_surface_list);
	}
	if(st_surface_list->st_surface_info.p_surface_buf)
	{
		FREE(st_surface_list->st_surface_info.p_surface_buf);
	}
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		创建硬件surface
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   gfx_layer_handle:要创建的surface所在的图层名柄
*	 @param[in] 	   e_color_mode:要创建的surface的颜色模式
*	 @param[in] 	   rect:要创建的surface的起点坐标，宽度，高度
*	 @param[in] 	   is_double_buf:要创建的surface是否双缓存标志，1为双缓存，0为单缓存
*	 @param[out]	p_hw_surface_handle:返回所创建的surface句柄
*	 @return		 错误码
*	 @note		  硬件surface的实质是某一硬件图层上的一块显示区域 ，如果需要显示OSD到屏幕上，最少需要\n
创建一个硬件surface。硬件surface大至用法与软件surface类似，有一点需要特别注意的是不同硬件surface之间\n
的水平方向不能有重叠 。比如硬件surface1的区域为水平垂直起始坐标和宽高度xywh分别为(1,1,100,100),\n
那么第二个硬件surface2的垂直起始坐标一定要大于100,不能小于100，也就是只能从(1,101,100,100)开始\n
否则硬件会出错。
*
*/
AUI_RTN_CODE aui_gfx_hw_surface_create(aui_hdl gfx_layer_handle,enum aui_osd_pixel_format e_color_mode, struct aui_osd_rect* rect,aui_hdl *p_hw_surface_handle,unsigned long is_double_buf)
{
	AUI_RTN_CODE ret = SUCCESS;
	gfx_layer_info *st_layer_info = NULL;
	struct osdpara region_para = {0,0,0,0};
	surface_list *st_hw_surface_list = NULL;
	unsigned char *second_buff = NULL;
	BOOL region_create = FALSE;
	unsigned long buf_size = 0;
	unsigned long surface_id = 0;
	unsigned long pitch = 0;
	BOOL layer_lock_flag = FALSE;
    OSAL_ID surface_mutex = INVALID_ID;

	GFX_FUNCTION_ENTER;
	if((e_color_mode > AUI_OSD_COLOR_MODE_MAX)||(p_hw_surface_handle == NULL))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("PARAM INVALID\n");
		goto ERROR;
	}
	GFX_LOCK(gfx_mutex);
	if(NULL == gfx_layer_handle || (FALSE == layer_handle_check((gfx_layer_info *)gfx_layer_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input gfx_layer_handle is null\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_layer_info = gfx_layer_handle;
	GFX_LOCK(st_layer_info->layer_mutex);//lock layer
	layer_lock_flag = TRUE;
	GFX_UNLOCK(gfx_mutex);

	//create osd region
	region_para.e_mode = aui_color_mode_tran_osd(e_color_mode);
	region_para.u_galpha = 0xff;
	region_para.u_galpha_enable = 0;
	surface_id = st_layer_info->hw_surface_cnt;
	ret = osddrv_create_region((unsigned long)(st_layer_info->osd_dev),surface_id,(struct osdrect *)rect,&region_para);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_create_region failed\n");
		goto ERROR;
	}

    surface_mutex = osal_mutex_create();
	if(INVALID_ID == surface_mutex)
	{
		ret = AUI_GFX_OTHTER_ERROR;
		AUI_ERR("osal_mutex_create failed\n");
		goto ERROR;

	}
    
	region_create = TRUE;
	pitch = get_pitch(e_color_mode,rect->uWidth);
	buf_size = pitch * rect->uHeight;
	//malloc surface list
	st_hw_surface_list = GFX_MALLOC(sizeof(surface_list));
	//if it is double buffer mode,malloc buffer for the second buffer
	if(is_double_buf)
	{
		second_buff = GFX_MALLOC(buf_size);
	}
	if( (NULL == st_hw_surface_list) || ((NULL == second_buff)	&& (TRUE ==is_double_buf)) )
	{
		ret = AUI_GFX_NO_MEMORY;
		AUI_ERR("GFX_MALLOC failed,buf_size = %d\n", buf_size);
		goto ERROR;
	}
	MEMSET(st_hw_surface_list,0,sizeof(surface_list));
	//add info to st_hw_surface_list
	st_hw_surface_list->surface_mutex = surface_mutex;
	st_hw_surface_list->st_surface_info.is_hw_surface = TRUE;
	st_hw_surface_list->st_surface_info.width = rect->uWidth;
	st_hw_surface_list->st_surface_info.height = rect->uHeight;
	st_hw_surface_list->st_surface_info.en_color_mode = e_color_mode;
	st_hw_surface_list->st_surface_info.pitch = pitch;
	st_hw_surface_list->st_surface_info.buf_size = buf_size;
	st_hw_surface_list->st_surface_info.hw_surface_info.isdouble_buf = is_double_buf;
	st_hw_surface_list->st_surface_info.hw_surface_info.second_buf = second_buff;
	st_hw_surface_list->st_surface_info.hw_surface_info.hw_surface_id = surface_id;
	st_hw_surface_list->st_surface_info.hw_surface_info.layer_handle = gfx_layer_handle;
	st_hw_surface_list->st_surface_info.hw_surface_info.layer_id = st_layer_info->layer_id;
	st_hw_surface_list->st_surface_info.hw_surface_info.left_off_set = rect->uLeft;
	st_hw_surface_list->st_surface_info.hw_surface_info.top_off_set = rect->uTop;

	//get surface addr
	ret = osddrv_get_region_addr((unsigned long)(st_layer_info->osd_dev),surface_id,0,(unsigned long *)(&st_hw_surface_list->st_surface_info.p_surface_buf));
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_get_region_addr failed\n");
		goto ERROR;
	}
	// add the surface to the link list
	surface_list_add(st_hw_surface_list);

	*p_hw_surface_handle = st_hw_surface_list;
	incress_hwsurface_cnt(st_layer_info->layer_id);

	GFX_UNLOCK(st_layer_info->layer_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	//free the malloc buffer
	if(INVALID_ID != surface_mutex)
	{
        osal_mutex_delete(surface_mutex);
    }
	if(region_create)
	{
		osddrv_delete_region((unsigned long)(st_layer_info->osd_dev),surface_id);
	}
	if(second_buff)
	{
		FREE(second_buff);
	}
	if(st_hw_surface_list)
	{
		FREE(st_hw_surface_list);
	}
	if(layer_lock_flag)
	{
		GFX_UNLOCK(st_layer_info->layer_mutex);
	}
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		删除surface,包括硬件和软件surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle:由aui_gfx_hw_surface_create或者aui_gfx_sw_surface_create返回的句柄
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_delete(aui_hdl p_surface_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_surface_list;
	BOOL is_hw_surface;
	OSAL_ID surface_mutex;
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
		GFX_LOCK(layer_info->layer_mutex);//unlock layer
	}
	surface_mutex = st_surface_list->surface_mutex;
	GFX_LOCK(surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	if(TRUE == is_hw_surface)
	{
		ret = delete_hw_surface(st_surface_list);
		GFX_UNLOCK(layer_info->layer_mutex);
	}
	else
	{
		ret = delete_sw_surface(st_surface_list);
	}

	GFX_UNLOCK(surface_mutex);
	osal_mutex_delete(surface_mutex);

	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("aui_gfx_surface_delete failed \n");
		goto ERROR;
	}

	GFX_FUNCTION_LEAVE;
	return ret;
ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		surface blit操作
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   dst_surface_handle :需要搬移到的目标surface
*	 @param[in] 	   fg_surface_handle:前景surface
*	 @param[in] 	   bg_surface_handle:背景surface,仅在需要做alpha blending或者bool运算时有效。背景surface可以是目标surface，也可以不是目标surface
*	 @param[in] 	   blit_param :blit时用到的参数
*	 @param[in] 	   blit_rect:需要搬移的区域。
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  1.当源和目标区域不相等的时候，需要做缩放处理。对于部分IC，在缩放时仅支持GFX_ROP_DERECT_COVER，不支持
GFX_ROP_ALPHA_BLENDING等操作。如果需要这些操作，首先需要保证源和目标surface的颜色模式为ARGB8888，宽度为16的倍数，
然后把操作分为两步来完成，首先将源数据进行缩放，把缩放结果放进一个临时缓存里，然后把这个缓存作
为源来与目标进行其它操作,对于部分IC没这个限制\n 2.对于没有GE的 IC仅支持GFX_ROP_DERECT_COVER，其它的任
何操作都不支持，并且源和目标surface的颜色模式和区域需要相同\n 3.只有部分IC支持旋转功能。\n 4.如果是目的surface
是硬件surface并且在创建指定了是双缓存，则需要调用aui_gfx_surface_flush来刷新才能显示最新的数据。\n
*
*/
AUI_RTN_CODE aui_gfx_surface_blit(aui_hdl dst_surface_handle,aui_hdl fg_surface_handle,aui_hdl bg_surface_handle,
											aui_blit_operation *blit_operation,aui_blit_rect *blit_rect)

{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_src_surface_list = NULL;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	unsigned long hw_surface_id = 0;
	unsigned long src_bpp = 0;//bit per pixel
	unsigned char *src_buf = NULL;
	unsigned char *dst_buf = NULL;
	unsigned long src_pitch = 0;
	unsigned long dst_pitch = 0;
	BOOL is_double_buf = FALSE;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == dst_surface_handle) || (NULL == fg_surface_handle)
		|| (FALSE == surface_list_check(dst_surface_handle))  || (FALSE == surface_list_check(fg_surface_handle)) || \
		(FALSE == surface_list_check(bg_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_src_surface_list = (surface_list *)fg_surface_handle;
	st_dst_surface_list = (surface_list *)dst_surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	if(st_dst_surface_list->surface_mutex != st_src_surface_list->surface_mutex)
		GFX_LOCK(st_src_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	src_buf = st_src_surface_list->st_surface_info.p_surface_buf;
	dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	src_pitch = st_src_surface_list->st_surface_info.pitch;
	dst_pitch = st_dst_surface_list->st_surface_info.pitch;
	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	src_bpp = get_bpp_color_by_color_mode(st_src_surface_list->st_surface_info.en_color_mode);
	/*check if feature not supported
	1.only support direct copy mode
	2.not support or not support scale
	3.not support color space conver
	*/
	if((AUI_GFX_ROP_DERECT_COVER != blit_operation->rop_operation) || (AUI_GE_CKEY_DISABLE != blit_operation->color_key_source)
		|| (blit_rect->fg_rect.uWidth != blit_rect->dst_rect.uWidth) || (blit_rect->fg_rect.uHeight != blit_rect->dst_rect.uHeight)
		|| (st_src_surface_list->st_surface_info.en_color_mode != st_dst_surface_list->st_surface_info.en_color_mode) )
	{
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("feature not supprot \n");
		GFX_UNLOCK(st_src_surface_list->surface_mutex);
		if(st_dst_surface_list->surface_mutex != st_src_surface_list->surface_mutex)
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);
		goto ERROR;
	}
	//check if it is hw surface and double buffer
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		surface_copy_data(&blit_rect->fg_rect,&blit_rect->dst_rect,src_pitch,dst_pitch,src_bpp,src_buf,dst_buf);

	}
	else if( (TRUE == is_double_buf) && (TRUE == st_dst_surface_list->st_surface_info.is_hw_surface))
	{
		dst_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;	  //COPY to second buffer,and will write to region when call flush
		surface_copy_data(&blit_rect->fg_rect,&blit_rect->dst_rect,src_pitch,dst_pitch,src_bpp,src_buf,dst_buf);
	}
	else //hardware surface and not double buffer, write it to region
	{
		ret = osddrv_region_write_by_surface((unsigned long)(osd_dev),hw_surface_id,src_buf,(struct osdrect *)(&blit_rect->dst_rect),(struct osdrect *)(&blit_rect->fg_rect),src_pitch);//TODO:FIXME,USE new osd api
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("OSDDrv_region_write_by_surface failed \n");
			GFX_UNLOCK(st_src_surface_list->surface_mutex);
			if(st_dst_surface_list->surface_mutex != st_src_surface_list->surface_mutex)
				GFX_UNLOCK(st_dst_surface_list->surface_mutex);
			goto ERROR;
		}
	}
	GFX_UNLOCK(st_src_surface_list->surface_mutex);
	if(st_dst_surface_list->surface_mutex != st_src_surface_list->surface_mutex)
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		把数据从缓存里搬移到另一个surface
*	 @author		andy.yu
*	 @date			2013-6-18
*	 @param[in] 	dst_surface_handle :需要搬移到的目标surface
*	 @param[in] 	p_data:缓存数据
*	 @param[in] 	blit_rect:需要搬移的区域。
*	 @param[in] 	blit_operation :blit时用到的参数，可参考aui_blit_operation,只支持AUI_GFX_ROP_DERECT_COVER
*	 @param[in] 	pitch:输入缓存每一行的字节数
*	 @param[in] 	en_color_mode:缓存的颜色模式
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  1.aui_gfx_data_blit 与aui_gfx_surface_blit的区别只在于aui_gfx_surface_blit的输入源是surface,而aui_gfx_data_blit的源是缓存，这个只是
方便上层不用创建一个surface就可以把一断缓存数据搬移到目的surface。\n
2.当源和目标区域不相等的时候，需要做缩放处理。对于部分IC，在缩放时仅支持GFX_ROP_DERECT_COVER，不支持
GFX_ROP_ALPHA_BLENDING等操作。如果需要这些操作，首先需要保证源和目标surface的颜色模式为ARGB8888，宽度为16的倍数，
然后把操作分为两步来完成，首先将源数据进行缩放，把缩放结果放进一个临时缓存里，然后把这个缓存作
为源来与目标进行其它操作,对于部分IC则没这个限制\n 3.对于没有GE的 IC仅支持GFX_ROP_DERECT_COVER，其它的任
何操作都不支持，并且源和目标surface的颜色模式和区域需要相同\n 4.只有部分 后的IC支持旋转功能。\n 5.如果是目的surface
是硬件surface并且在创建指定了是双缓存，则需要调用aui_gfx_surface_flush来刷新才能显示最新的数据。
*
*/
AUI_RTN_CODE aui_gfx_data_blit(aui_hdl dst_surface_handle,unsigned char *p_data,aui_blit_rect *blit_rect,aui_blit_operation *blit_operation,unsigned long pitch,
										enum aui_osd_pixel_format en_color_mode)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	unsigned long hw_surface_id = 0;
	unsigned long src_bpp = 0;//bit per pixel
	unsigned char *src_buf = NULL;
	unsigned char *dst_buf = NULL;
	BOOL is_double_buf = FALSE;
	unsigned long dst_pitch;
	unsigned long src_pitch;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == dst_surface_handle) || (FALSE == surface_list_check(dst_surface_handle))
		|| (NULL == p_data) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	/*check if feature not supported
	1.only support direct copy mode
	2.not support or not support scale
	3.not support color space conver
	*/
	if((AUI_GFX_ROP_DERECT_COVER != blit_operation->rop_operation) || (AUI_GE_CKEY_DISABLE != blit_operation->color_key_source)
		|| (blit_rect->fg_rect.uWidth != blit_rect->dst_rect.uWidth) || (blit_rect->fg_rect.uHeight != blit_rect->dst_rect.uHeight) )

	{
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("feature not supprot \n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)dst_surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);
	if(st_dst_surface_list->st_surface_info.en_color_mode != en_color_mode)
	{
 		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("not support different colormode\n");
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);
		goto ERROR;
	}
	src_buf = p_data;
	dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	src_bpp = get_bpp_color_by_color_mode(st_dst_surface_list->st_surface_info.en_color_mode);
	dst_pitch = st_dst_surface_list->st_surface_info.pitch;
	src_pitch = pitch;
	//check if it is hw surface and double buffer
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		surface_copy_data(&blit_rect->fg_rect,&blit_rect->dst_rect,src_pitch,dst_pitch,src_bpp,p_data,dst_buf);

	}
	else if( (TRUE == is_double_buf) && (TRUE == st_dst_surface_list->st_surface_info.is_hw_surface))
	{
		dst_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;	  //COPY to second buffer,and will write to region when call flush
		surface_copy_data(&blit_rect->fg_rect,&blit_rect->dst_rect,src_pitch,dst_pitch,src_bpp,src_buf,dst_buf);
	}
	else //hardware surface and not double buffer, write it to region
	{
		ret = osddrv_region_write_by_surface((unsigned long)osd_dev,hw_surface_id,src_buf,(struct osdrect *)(&blit_rect->dst_rect),(struct osdrect *)(&blit_rect->fg_rect),pitch);//TODO:FIXME,USE new osd api
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("OSDDrv_region_write_by_surface failed \n");
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
			goto ERROR;
		}
	}

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);

}


/**
*	 @brief 		获取指定surface的数据
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle,将要获取数据的surface
*	 @param[in] 	   rect:将要获取数据的surface数据区域
*	 @param[out]	pdata:存放从surface里获取的数据
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_capture(aui_hdl p_surface_handle,struct aui_osd_rect * rect,void *p_data)
{
	#define BIT_TO_BYE 8
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_src_surface_list = NULL;
	//unsigned long hw_surface_id = 0;
	unsigned long src_bpp = 0;//bit per pixel
	unsigned char *src_buf = NULL;
	unsigned long dst_pitch;
	unsigned long src_pitch;
	struct aui_osd_rect dst_rect;


	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (NULL == p_data) || (NULL == rect) || \
	 (FALSE == surface_list_check((surface_list *)p_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_src_surface_list = (surface_list *)p_surface_handle;
	if(FALSE == check_operate_rect(rect, st_src_surface_list->st_surface_info.width, st_src_surface_list->st_surface_info.height))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("rect is out of region\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	GFX_LOCK(st_src_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	src_buf = st_src_surface_list->st_surface_info.p_surface_buf;
	src_bpp = get_bpp_color_by_color_mode(st_src_surface_list->st_surface_info.en_color_mode);
	src_pitch = st_src_surface_list->st_surface_info.pitch;
	dst_pitch = (src_bpp * rect->uWidth) / BIT_TO_BYE;
	dst_rect.uTop = 0;
	dst_rect.uLeft = 0;
	dst_rect.uHeight = rect->uHeight;
	dst_rect.uWidth = rect->uWidth;
	//copy to pdata
	surface_copy_data(rect,&dst_rect,src_pitch,dst_pitch,src_bpp,src_buf,p_data);

	GFX_UNLOCK(st_src_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);

}

/**
*	 @brief 		用指定颜色填充目标surface
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   surface_handle:将要被填充的surface
*	 @param[in] 	   color:填充的颜色
*	 @param[in] 	   rect:填充的区域
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_fill(aui_hdl dst_surface_handle,unsigned long color,struct aui_osd_rect * rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	unsigned long hw_surface_id = 0;
	enum aui_osd_pixel_format color_mode = 0;
	unsigned char *dst_buf = NULL;
	BOOL is_double_buf = FALSE;
	unsigned long pitch = 0;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if((NULL == dst_surface_handle) || (FALSE == surface_list_check(dst_surface_handle)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)dst_surface_handle;

	if(FALSE == check_operate_rect(rect, st_dst_surface_list->st_surface_info.width, st_dst_surface_list->st_surface_info.height))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("rect is out of region\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	color_mode = st_dst_surface_list->st_surface_info.en_color_mode;
	pitch = st_dst_surface_list->st_surface_info.pitch;

	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{

		surface_fill(rect,pitch,color_mode,dst_buf,color);
	}
	else if( (TRUE == is_double_buf) && (TRUE == st_dst_surface_list->st_surface_info.is_hw_surface) )
	{
		dst_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
		surface_fill(rect,pitch,color_mode,dst_buf,color);
	}
	else
	{
		ret = osddrv_region_fill((unsigned long)osd_dev,hw_surface_id,(struct osdrect *)rect,color);
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("OSDDrv_region_fill failed \n");
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
			goto ERROR;
		}
	}

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		用指定颜色画一直线，可以为水平直线或者垂直直线
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   surface_handle:将要被画的surface
*	 @param[in] 	   color:直线的颜色
*	 @param[in] 	   start_coordinate:直线的起始坐标
*	 @param[in] 	   end_coordinate:直线的终止坐标
*	 @param[out]	NULL
*	 @return
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_draw_line(aui_hdl surface_handle,unsigned long color,aui_coordinate *start_coordinate,aui_coordinate *end_coordinate)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	  osd_dev = NULL;
	unsigned long hw_surface_id = 0;
	enum aui_osd_pixel_format color_mode = 0;
	unsigned char *dst_buf = NULL;
	BOOL is_double_buf = FALSE;
	unsigned long pitch = 0;
	struct aui_osd_rect rect;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == surface_handle) || (FALSE == surface_list_check(surface_handle)) || (NULL == start_coordinate) || (NULL == end_coordinate) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)surface_handle;

	if((FALSE == check_operate_coordinate(start_coordinate, st_dst_surface_list->st_surface_info.width, st_dst_surface_list->st_surface_info.height)) || \
	   (FALSE == check_operate_coordinate(start_coordinate, st_dst_surface_list->st_surface_info.width, st_dst_surface_list->st_surface_info.height)))
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input coordinate is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}


	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	dst_buf = st_dst_surface_list->st_surface_info.p_surface_buf;
	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	color_mode = st_dst_surface_list->st_surface_info.en_color_mode;
	pitch = st_dst_surface_list->st_surface_info.pitch;
	rect.uLeft = start_coordinate->X;
	rect.uTop = start_coordinate->Y;
	rect.uWidth = ((end_coordinate->X - start_coordinate->X) > 0)? (end_coordinate->X - start_coordinate->X) : 1;
	rect.uHeight = ((end_coordinate->Y - start_coordinate->Y) > 0)? (end_coordinate->Y - start_coordinate->Y) : 1;
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{

		surface_fill(&rect,pitch,color_mode,dst_buf,color);
	}
	else if( (TRUE == is_double_buf) && (TRUE == st_dst_surface_list->st_surface_info.is_hw_surface) )
	{
		dst_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
		surface_fill(&rect,pitch,color_mode,dst_buf,color);
	}
	else
	{
		ret = osddrv_region_fill((unsigned long)osd_dev,hw_surface_id,(struct osdrect *)&rect,color);
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("OSDDrv_region_fill failed \n");
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
			goto ERROR;
		}		 
	}

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		用指定颜色画矩形边框
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   surface_handle:将要被画的surface
*	 @param[in] 	   fore_color:边框颜色
*	 @param[in] 	   back_color:边框里面的颜色，仅在fill_background为1时有效
*	 @param[in] 	   rect:边框区域
*	 @param[in] 	   fill_background:是否填充边框里的区域，1为填充，0为不填充
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_draw_outline(aui_hdl surface_handle,unsigned long fore_color,unsigned long back_color,
													struct aui_osd_rect * rect,unsigned long fill_background)
{
	GFX_FUNCTION_ENTER;
	GFX_FUNCTION_LEAVE;

	return SUCCESS;

}

/**
*	 @brief 		刷新硬件surface的指定区域备份缓存数据到硬件framebuffer里
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   dst_surface_handle :将要刷新的硬件surface
*	 @param[in] 	   dst_rect:将要刷新的区域，刷新区域不能大于surface本身的区域
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  这个函数仅仅用于双缓存的硬件surface，对于单缓存的硬件surface或者软件surface,该函数会直接返回
*
*/
AUI_RTN_CODE aui_gfx_surface_flush(aui_hdl dst_surface_handle,struct aui_osd_rect *dst_rect)
{
	AUI_RTN_CODE ret = SUCCESS;
	//surface_list *st_src_surface_list = NULL;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	unsigned long hw_surface_id = 0;
	unsigned char *src_buf = NULL;
	//unsigned char *dst_buf = NULL;
	unsigned long pitch = 0;
	BOOL is_double_buf = FALSE;
	unsigned long is_hw_surface;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == dst_surface_handle) || (NULL == dst_surface_handle)
		|| (FALSE == surface_list_check(dst_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)dst_surface_handle;
	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	src_buf = st_dst_surface_list->st_surface_info.hw_surface_info.second_buf;
	pitch = st_dst_surface_list->st_surface_info.pitch;
	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	is_double_buf = st_dst_surface_list->st_surface_info.hw_surface_info.isdouble_buf;
	is_hw_surface = st_dst_surface_list->st_surface_info.is_hw_surface;
	//check if it is hw surface and double buffer
	if(FALSE == is_hw_surface)
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("is_hw_surface false \n");
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
		goto ERROR;
	}
	else if( (FALSE == is_double_buf) && (TRUE == is_hw_surface))
	{
		goto EXIT;
	}
	else //hardware surface and not double buffer, write it to region
	{
		ret = osddrv_region_write_by_surface((unsigned long)osd_dev,hw_surface_id,src_buf,(struct osdrect *)dst_rect,(struct osdrect *)dst_rect,pitch);//TODO:FIXME,USE new osd api
		if(RET_SUCCESS != ret)
		{
			ret = AUI_GFX_DRIVER_ERROR;
			AUI_ERR("OSDDrv_region_write_by_surface failed \n");
			GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
			goto ERROR;
		}
	}

EXIT:
	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		同步surface操作
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   dst_surface_handle :将要同步的surface
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_sync(aui_hdl dst_surface_handle)
{
	GFX_FUNCTION_ENTER;
	GFX_FUNCTION_LEAVE;

	return SUCCESS;
}

/**
*	 @brief 		获取 surface信息
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle:需要获取的surface
*	 @param[out]	p_surface_info:surface信息存取指针
*	 @return		 错误码
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
*	 @brief 		显示或者隐藏指定的硬件surface
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_hw_surface_handle:将要显示或者隐藏的surface
*	 @param[in] 	   on_off:"1" 显示,"0"隐藏
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_show_on_off(aui_hdl p_hw_surface_handle,unsigned long on_off)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_hw_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	unsigned long hw_surface_id = 0;

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

	if(FALSE == st_hw_surface_list->st_surface_info.is_hw_surface)
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is not hardware surface\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	GFX_LOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	osd_dev = get_osd_device(st_hw_surface_list->st_surface_info.hw_surface_info.layer_id);
	hw_surface_id = st_hw_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	ret = osddrv_region_show((unsigned long)osd_dev,hw_surface_id,on_off);
	st_hw_surface_list->st_surface_info.show_onoff = on_off;
	GFX_UNLOCK(st_hw_surface_list->surface_mutex);//unlock surface
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_RegionShow failed \n");
		goto ERROR;
	}

	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		设置指定硬件surface放大或者缩小参数
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_hw_surface_handle:将要设置参数的硬件surface
*	 @param[in] 	   scale_param:缩放参数
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  该函数仅仅用于硬件surface，当需要surface实际大小与显示大小不相等时，\n
可以通过设置scale_param参数来实现，一般是用于在电视制式切换，缩放率为输入/ 输出。\n
比如surface实际大小为1920 * 1080，当用户把电视制式设置为720 *576(PAL) ,这时候如果希望OSD画\n
面输出为720 * 576，需要调用本函数，设置\n
aui_scale_param->input_width = 1920;aui_scale_param->input_height = 1080;\n
aui_scale_param->output_width = 720;aui_scale_param->output_height = 576;
Please use "aui_gfx_layer_scale".
*
*/
AUI_RTN_CODE aui_gfx_surface_scale_param_set(aui_hdl p_hw_surface_handle,aui_scale_param *scale_param)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_hw_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	//unsigned long hw_surface_id = 0;

	static osd_scale_param drv_scale_param;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if((NULL== scale_param) || (0 == scale_param->input_height) || (0 == scale_param->input_width) || \
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

	if(FALSE == st_hw_surface_list->st_surface_info.is_hw_surface)
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is not hardware surface\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	GFX_LOCK(st_hw_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	osd_dev = get_osd_device(st_hw_surface_list->st_surface_info.hw_surface_info.layer_id);
	//hw_surface_id = st_hw_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	//caculate the scale param,horizontal scale = h_mul / h_div,vertical scale = v_mul / v_div
	drv_scale_param.h_div = scale_param->input_width;
	drv_scale_param.h_mul = scale_param->output_width;
	drv_scale_param.v_div = scale_param->input_height;
	drv_scale_param.v_mul = scale_param->output_height;
	ret = osddrv_scale((unsigned long)osd_dev, OSD_SCALE_WITH_PARAM, (unsigned long)(&drv_scale_param));

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
	gfx_layer_info* layer_info = NULL;
	struct osd_device*	osd_dev = NULL;
	unsigned long layer_id = 0;

	static osd_scale_param drv_scale_param;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if((NULL== scale_param) || (0 == scale_param->input_height) || (0 == scale_param->input_width) || \
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

    layer_id = layer_info->layer_id;
	osd_dev = get_osd_device(layer_id);
	//caculate the scale param,horizontal scale = h_mul / h_div,vertical scale = v_mul / v_div
	drv_scale_param.h_div = scale_param->input_width;
	drv_scale_param.h_mul = scale_param->output_width;
	drv_scale_param.v_div = scale_param->input_height;
	drv_scale_param.v_mul = scale_param->output_height;
	ret = osddrv_scale((unsigned long)osd_dev, OSD_SCALE_WITH_PARAM, (unsigned long)(&drv_scale_param));

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

/**
*	 @brief 		设置surface的colorkey值
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle:将要设置的surface
*	 @param[in] 	   colorkey:将要设置的colorkey最大与最小值，其中最大与最小值可以相等
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  仅在带GE的IC支持color key操作
*
*/
AUI_RTN_CODE aui_gfx_surface_colorkey_set(aui_hdl p_surface_handle,aui_color_key *color_key)
{
	GFX_FUNCTION_ENTER;

	AUI_ERR("not support color key \n");
	return AUI_GFX_FEATURE_NOT_SUPPORTED;

	GFX_FUNCTION_LEAVE;
}

/**
*	 @brief 		获取surface的colorkey值
*	 @author		andy.yu
*	 @date			  2013-6-18
*	 @param[in] 	   p_surface_handle:将要获取的surface
*	 @param[out]	colorkey:存放获取的color key
*	 @return		 错误码
*	 @note		  仅在带GE的IC支持color key操作
*
*/
AUI_RTN_CODE aui_gfx_surface_colorkey_get(aui_hdl p_surface_handle,aui_color_key *colorkey)
{
	GFX_FUNCTION_ENTER;

	AUI_ERR("not support color key \n");
	return AUI_GFX_FEATURE_NOT_SUPPORTED;

	GFX_FUNCTION_LEAVE;
}

/**
*	 @brief 		设置surface的剪切区域
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_surface_handle:将要设置的surface
*	 @param[in] 	   rect :剪切区域
*	 @param[in] 	   clip_mode:剪切模式，分区域内剪切和区域外剪切两种模式
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_clip_rect_set(aui_hdl p_surface_handle, struct aui_osd_rect *rect,enum aui_ge_clip_mode clip_mode)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	//unsigned long hw_surface_id = 0;
	enum clipmode osd_clip_mode;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)p_surface_handle;

	//only support	setting clip to hw surface
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		//MEMCPY(&st_dst_surface_list.st_surface_info.clip_rect,rect,sizeof(RECT));
		//st_dst_surface_list.st_surface_info.clip_rect
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("only support  setting clip to hw surface \n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	if(FALSE == check_operate_rect(rect,st_dst_surface_list->st_surface_info.width, st_dst_surface_list->st_surface_info.height))
	{
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("clip rect is out of surface.\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}

	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	//hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;
	 osd_clip_mode = ge_clip_mode2osd_mode(clip_mode);

	ret = osddrv_set_clip((unsigned long)osd_dev,osd_clip_mode,(struct osdrect *)rect);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_set_clip failed \n");

		GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
		goto ERROR;
	}

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		清除剪切区域
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_surface_handle:将要清除的surface
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  调用本函数后，目标surface的所有剪切区域都会清除掉
*
*/
AUI_RTN_CODE aui_gfx_surface_clip_rect_clear(aui_hdl p_surface_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	//unsigned long hw_surface_id = 0;

	GFX_FUNCTION_ENTER;
	GFX_LOCK(gfx_mutex);
	//check input param
	if( (NULL == p_surface_handle) || (FALSE == surface_list_check(p_surface_handle)) )
	{
		ret = AUI_GFX_PARAM_INVALID;
		AUI_ERR("input handle is invalid\n");
		GFX_UNLOCK(gfx_mutex);
		goto ERROR;
	}
	st_dst_surface_list = (surface_list *)p_surface_handle;

	//only support	  setting clip to hw surface
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		//MEMCPY(&st_dst_surface_list.st_surface_info.clip_rect,rect,sizeof(RECT));
		//st_dst_surface_list.st_surface_info.clip_rect
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("only support  setting clip to hw surface \n");
		GFX_UNLOCK(gfx_mutex);//lock surface
		goto ERROR;
	}

	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	//hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;

	ret = osddrv_clear_clip((unsigned long)osd_dev);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_clear_clip failed \n");
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);//lock surface
		goto ERROR;
	}

	GFX_UNLOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		设置surface调色板信息
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   p_surface_handle:将要设置的surface
*	 @param[in] 	   p_pallette_info:调色板信息
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  对于没有GE的IC仅仅支持硬件surface设置调色板
*
*/
AUI_RTN_CODE aui_gfx_surface_pallette_set(aui_hdl p_surface_handle,aui_pallette_info *p_pallette_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	struct osd_device*	osd_dev = NULL;
	//unsigned long hw_surface_id = 0;

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

	//only support	  setting clip to hw surface
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		//MEMCPY(&st_dst_surface_list.st_surface_info.clip_rect,rect,sizeof(RECT));
		//st_dst_surface_list.st_surface_info.clip_rect
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("only support  setting clip to hw surface \n");
		GFX_UNLOCK(gfx_mutex);//unlock surface
		goto ERROR;
	}

	GFX_LOCK(st_dst_surface_list->surface_mutex);//lock surface
	GFX_UNLOCK(gfx_mutex);

	osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	//hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;

	ret = osddrv_set_pallette((unsigned long)osd_dev,p_pallette_info->p_pallette,p_pallette_info->color_cnt,p_pallette_info->en_pallette_color_type);
	if(RET_SUCCESS != ret)
	{
		ret = AUI_GFX_DRIVER_ERROR;
		AUI_ERR("OSDDrv_region_pallette_set failed \n");
		GFX_UNLOCK(st_dst_surface_list->surface_mutex);//unlock surface
		goto ERROR;
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


/**
*	 @brief 		获取surface调色板信息
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   p_surface_handle:将要获取的surface
*	 @param[in] 	   p_pallette_info:调色板信息
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_pallette_get(aui_hdl p_surface_handle,aui_pallette_info *p_pallette_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	surface_list *st_dst_surface_list = NULL;
	//struct osd_device*	osd_dev = NULL;
	//unsigned long hw_surface_id = 0;

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

	//osd_dev = get_osd_device(st_dst_surface_list->st_surface_info.hw_surface_info.layer_id);
	//hw_surface_id = st_dst_surface_list->st_surface_info.hw_surface_info.hw_surface_id;

	//only support	  setting clip to hw surface
	if(FALSE == st_dst_surface_list->st_surface_info.is_hw_surface)
	{
		//MEMCPY(&st_dst_surface_list.st_surface_info.clip_rect,rect,sizeof(RECT));
		//st_dst_surface_list.st_surface_info.clip_rect
		ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
		AUI_ERR("only support  setting clip to hw surface \n");
		goto ERROR;
	}
	else
	{
		MEMCPY(p_pallette_info,&st_dst_surface_list->st_surface_info.pallettte_info,sizeof(aui_pallette_info));
	}

	GFX_UNLOCK(gfx_mutex);
	GFX_FUNCTION_LEAVE;
	return ret;

ERROR:
	GFX_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		设置surfacee global alpha
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_surface_handle:将要设置的surface
*	 @param[in] 	   global_alpha :全局alpha
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note		  1.设置了全局alpha后，需要在blit操作的时候指定使用全局alpha才能生效\n
2.仅在现GE的 IC有效
*
*/
AUI_RTN_CODE aui_gfx_surface_galpha_set(aui_hdl p_surface_handle,unsigned long global_alpha)
{
	GFX_FUNCTION_ENTER;

	AUI_ERR("not support surface global alpha \n");

	GFX_FUNCTION_LEAVE;
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
}

/**
*	 @brief 		获取 surfacee global alpha
*	 @author		andy.yu
*	 @date			  2013-6-19
*	 @param[in] 	   p_surface_handle:将要设置的surface
*	 @param[out]	global_alpha:返回surface的全局alpha
*	 @return
*	 @note
*
*/
AUI_RTN_CODE aui_gfx_surface_galpha_get(aui_hdl p_surface_handle,unsigned long *global_alpha)
{
	GFX_FUNCTION_ENTER;

	AUI_ERR("not support surface global alpha \n");

	GFX_FUNCTION_LEAVE;
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
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
	return AUI_GFX_FEATURE_NOT_SUPPORTED;
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
 *	@note			only for linux platform
 */
AUI_RTN_CODE aui_gfx_sw_surface_create_by_bitmap(aui_hdl *p_surface_handle,
												 const aui_gfx_bitmap_info_t *p_bitmap_info)
{
    AUI_RTN_CODE ret = SUCCESS;
    surface_list *st_surface_list = NULL;
    OSAL_ID surface_mutex = INVALID_ID;
    unsigned char *data = NULL;
    unsigned long pitch = 0;
    unsigned long width = 0;
    unsigned long height = 0;
    enum aui_osd_pixel_format e_color_mode = AUI_OSD_HD_ARGB8888;

    GFX_FUNCTION_ENTER;
    if((NULL == p_surface_handle)
        ||(NULL == p_bitmap_info)
        ||(NULL == p_bitmap_info->p_data))  {
        ret = AUI_GFX_PARAM_INVALID;
        aui_dbg_printf(AUI_MODULE_GFX,ERRO_INFO,"param invalid,%s,%d\n",__FUNCTION__,__LINE__);
        goto ERROR; 	
    }

    data = p_bitmap_info->p_data;
    pitch = p_bitmap_info->pitch;
    width = p_bitmap_info->width;
    height = p_bitmap_info->height;
    e_color_mode = p_bitmap_info->color_type;
#if 0
    libc_printf("--- bitmap info ---\n");
    libc_printf(" pdata: %p\n", p_bitmap_info->p_data);
    libc_printf(" pitch: %d\n", p_bitmap_info->pitch);
    libc_printf(" width: %d\n", p_bitmap_info->width);
    libc_printf(" height: %d\n", p_bitmap_info->height);

#endif
    if(e_color_mode > AUI_OSD_COLOR_MODE_MAX) {
        ret = AUI_GFX_PARAM_INVALID;
        aui_dbg_printf(AUI_MODULE_GFX,ERRO_INFO,"PARAM INVALID,%s,%d\n",__FUNCTION__,__LINE__);
        goto ERROR;
    }

    //malloc surface list
    st_surface_list = GFX_MALLOC(sizeof(surface_list));
    if( NULL == st_surface_list ) {
        ret = AUI_GFX_NO_MEMORY;
        aui_dbg_printf(AUI_MODULE_GFX,ERRO_INFO,"GFX_MALLOC failed,%s,%d\n",__FUNCTION__,__LINE__);
        //GFX_UNLOCK(gfx_mutex);
        goto ERROR;
    }
    surface_mutex = osal_mutex_create();
    if(INVALID_ID == surface_mutex) {
        ret = AUI_GFX_OTHTER_ERROR;
        aui_dbg_printf(AUI_MODULE_GFX,ERRO_INFO,"osal_mutex_create failed,%s,%d\n",__FUNCTION__,__LINE__);
        goto ERROR;
    }

    //GFX_UNLOCK(gfx_mutex);
    MEMSET(st_surface_list,0,sizeof(surface_list));
    //add info to st_surface_list
    st_surface_list->surface_mutex = surface_mutex;
    st_surface_list->st_surface_info.is_hw_surface = FALSE;
    st_surface_list->st_surface_info.width = width;
    st_surface_list->st_surface_info.height = height;
    st_surface_list->st_surface_info.en_color_mode = e_color_mode;
    st_surface_list->st_surface_info.pitch = pitch;//get_pitch(e_color_mode,width);
    st_surface_list->st_surface_info.en_color_mode = e_color_mode;
    st_surface_list->st_surface_info.buf_size = st_surface_list->st_surface_info.pitch * height;
    st_surface_list->st_surface_info.p_surface_buf = p_bitmap_info->p_data;

    // add the surface to the link list
    surface_list_add(st_surface_list);

    *p_surface_handle = st_surface_list;
    GFX_FUNCTION_LEAVE;
    return ret;

    ERROR:
    //free the malloc buffer
    if(INVALID_ID != surface_mutex){
        osal_mutex_delete(surface_mutex);
    }
    if(st_surface_list){
        FREE(st_surface_list);
    }
    GFX_FUNCTION_LEAVE;
    aui_rtn(AUI_MODULE_GFX,ret,NULL);

    return ret;
}

/**
 *	@brief			lock a surface, get the buffer address
 *
 *	@param[in]		p_surface_handle		the handle of the suface wait to locked
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			3/28/2014  15:13:32
 *
 *	@note			This must be done if you want get the surface buffer address\n
 *					by call aui_gfx_surface_info_get.
 */
AUI_RTN_CODE aui_gfx_surface_lock(aui_hdl p_surface_handle)
{
	return AUI_GFX_SUCCESS;
}

/**
 *	@brief			unlock a surface
 *
 *	@param[in]		p_surface_handle		the handle of the suface wait to unlocked
 *
 *	@return 		AUI_RTN_CODE
 *
 *	@author 		Peter Pan <peter.pan@alitech.com>
 *	@date			3/28/2014  15:13:32
 *
 *	@note			This must be done after surface buffer read/write complete.
 */
AUI_RTN_CODE aui_gfx_surface_unlock(aui_hdl p_surface_handle)
{
	return AUI_GFX_SUCCESS;
}