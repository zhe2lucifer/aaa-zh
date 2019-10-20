#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "aui_help_print.h"
#include "aui_osd_test.h"
#include "aui_misc.h"
#include "aui_os.h"
#include "resource/osd/alpha_w65_h51_argb1555.dat"
#include "resource/osd/alpha_w65_h51_argb4444.dat"
#include "resource/osd/img_w512_h384_argb4444.dat"
#include "resource/osd/img_w512_h384_argb8888.dat"
#include "resource/osd/color.dat"
#include "resource/osd/img_w640_h530_clut8.dat"
#include "resource/osd/img_w512_h384_argb1555.dat"
#include "resource/osd/img_w512_h384_argb565.dat"

#include "resource/osd/img_w800_h800_argb8888.h"



#ifdef AUI_LINUX
#define STEP(str)                                   \
    do {                                            \
        char c;                                     \
        AUI_PRINTF("[ %s ], enter to continue:\n", str);  \
        scanf("%c", &c);                            \
    } while (0)
#else
#define STEP(str)  \
    if(!aui_test_user_confirm("Press 'y' to continue, press 'n' to quit.")) \
	{ \
		return AUI_RTN_FAIL; \
	}
#endif

#define HW_SURFACE_CNT 5
static AUI_RTN_CODE aui_osd_scale(aui_hdl* handle, unsigned long region_width, unsigned long region_height)
{
    // we need to scale the osd according to the system mode, otherwise the osd was cut off.
    // Now the default system mod is 1080p(1920*1080)
    // more info: Support_Flow #46193
    aui_scale_param scale_param;
    memset(&scale_param, 0, sizeof(aui_scale_param));
    scale_param.input_width = 1280;//region_width;
    scale_param.output_width = 1920;
    scale_param.input_height = 720;//region_height;
    scale_param.output_height = 1080;
    AUI_PRINTF("====> osd scale: %ld, %ld -> %ld, %ld\n", 
        scale_param.input_width, scale_param.input_height, 
        scale_param.output_width, scale_param.output_height);
    aui_gfx_layer_scale(handle, &scale_param);

	return 0;
}
unsigned long test_osd_draw_line(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long layer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect region_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
	unsigned long fill_color_draw_hline = GREEN_8888;
	unsigned long fill_color_draw_vline = WHITE_8888;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_coordinate start_coordinate;
	aui_coordinate end_coordinate;

    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&start_coordinate, 0, sizeof(struct aui_coordinate));
    MEMSET(&end_coordinate, 0, sizeof(struct aui_coordinate));
    region_rect.uHeight = 720;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 576;
	
    start_coordinate.X = 100;
    start_coordinate.Y = 0;
    end_coordinate.X = 100;
    end_coordinate.Y = 200;

	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(layer_id, (aui_hdl*)(&layer_handle));
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_layer_show_on_off(layer_handle, 1);
    AUI_TEST_CHECK_RTN(ret);
    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
	ret = aui_gfx_hw_surface_create(layer_handle, pixel_format, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
    AUI_TEST_CHECK_RTN(ret);
    aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);

    ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_draw_line(hw_suf_handle, fill_color_draw_hline, &start_coordinate, &end_coordinate);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
    AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF("draw h line\n");
    AUI_SLEEP(5000);
    
    start_coordinate.X = 0;
    start_coordinate.Y = 200;
    end_coordinate.X = 200;
    end_coordinate.Y = 200;
    aui_gfx_surface_draw_line(hw_suf_handle, fill_color_draw_vline, &start_coordinate, &end_coordinate);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
    AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF("draw v line\n");
    AUI_SLEEP(5000);

    //delete the surface
    ret = aui_gfx_surface_delete(hw_suf_handle);
    AUI_TEST_CHECK_RTN(ret);
#if 0
    if(!aui_test_user_confirm("do you see one horizontal line and vertical line")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    EXIT:
#endif
    ret = aui_gfx_layer_close(layer_handle);
    AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}

unsigned long test_osd_draw_out_line(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long layer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect region_rect;
	struct aui_osd_rect out_line_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
	unsigned long fill_color = GREEN_8888;
	unsigned long back_color = WHITE_8888;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&out_line_rect, 0, sizeof(struct aui_osd_rect));
    region_rect.uHeight = 576;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 720;

    out_line_rect.uHeight = 180;
    out_line_rect.uLeft = 100;
    out_line_rect.uTop = 110;
    out_line_rect.uWidth = 300;

#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(layer_id, (aui_hdl*)(&layer_handle));
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

	//create hw/sw surface
	// On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
	ret = aui_gfx_hw_surface_create(layer_handle, pixel_format, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
	AUI_TEST_CHECK_RTN(ret);
    aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);

	ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
	AUI_TEST_CHECK_RTN(ret);
    aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
    AUI_TEST_CHECK_RTN(ret);
	
    //STEP("draw outline done!\n");
	AUI_SLEEP(5000);

#if 0
    if(!aui_test_user_confirm("do you see the rectangle?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    EXIT:
#endif
	//delete the surface
	ret = aui_gfx_surface_delete(hw_suf_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}

unsigned long test_osd_clip_mode(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long lyaer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect rect;
	struct aui_osd_rect region_rect;
	struct aui_osd_rect clip_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_256_COLOR, AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555, AUI_OSD_HD_ARGB8888, AUI_OSD_HD_RGB565};
	unsigned long width = 512;
	unsigned long height = 384;
	const unsigned char *data[] = {Image8Data, Image4444Data, Image1555Data, Image8888Data, Image565Data};
	unsigned long pitch[] = {width, width * 2, width * 2, width * 4, width * 2};
	//unsigned long bpp[] = {1, 2, 2, 4, 2};
	unsigned long color_cnt = 5;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long i = 0;
	aui_blit_operation blit_op;
	aui_blit_rect blit_rect;
	enum aui_ge_clip_mode clip_mode;
	aui_pallette_info p_pallette_info;

    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&clip_rect, 0, sizeof(struct aui_osd_rect));    
    MEMSET(&p_pallette_info, 0, sizeof(struct aui_st_pallette_info)); 

#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(lyaer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

    rect.uHeight = height;
    rect.uLeft = 0;
    rect.uTop = 0;
    rect.uWidth = width;

    region_rect.uHeight = height;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = width;

    MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
    MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.dst_rect, &rect, sizeof(struct aui_osd_rect));

	MEMSET(&blit_op, 0X0, sizeof(aui_blit_operation));
	blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
	//blit_op.mirror_type = GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT;
	blit_op.alpha_blend_mode = AUI_GE_ALPHA_BLEND_DST_OVER;
	for(i = 0; i < color_cnt; i++) {
		
		if(pixel_format[i] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
		//create hw/sw surface
		// On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[i], &region_rect, (aui_hdl*)&hw_suf_handle, 0);
		AUI_TEST_CHECK_RTN(ret);

        aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);
		
		if(AUI_OSD_256_COLOR == pixel_format[i]) {
			p_pallette_info.p_pallette = (unsigned char*)Data_Color1;
			p_pallette_info.color_cnt = 256;
			p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
			ret = aui_gfx_surface_pallette_set(hw_suf_handle, &p_pallette_info);
			AUI_TEST_CHECK_RTN(ret);
		}
		ret = aui_gfx_data_blit(hw_suf_handle, (unsigned char *)data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(3000);
		ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(1000);

		//set clip mode
        clip_rect.uHeight = 280;
        clip_rect.uLeft = 128;
        clip_rect.uTop = 100;
        clip_rect.uWidth = 256;
        clip_mode = AUI_GE_CLIP_INSIDE;
		AUI_PRINTF("----> AUI_GE_CLIP_INSIDE\n");
		ret = aui_gfx_surface_clip_rect_set(hw_suf_handle, &clip_rect, clip_mode);
		ret = aui_gfx_data_blit(hw_suf_handle, (unsigned char *)data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &clip_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_PRINTF("----> DONE\n");
        AUI_SLEEP(3000);

        ret = aui_gfx_surface_clip_rect_clear(hw_suf_handle);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);

		AUI_PRINTF("----> AUI_GE_CLIP_OUTSIDE\n");
        clip_mode = AUI_GE_CLIP_OUTSIDE;
        ret = aui_gfx_surface_clip_rect_set(hw_suf_handle, &clip_rect, clip_mode);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_data_blit(hw_suf_handle, (unsigned char *)data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
		AUI_TEST_CHECK_RTN(ret);
		
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
		AUI_PRINTF("----> DONE\n");
        AUI_SLEEP(3000);
		
        //delete the surface
        ret = aui_gfx_surface_delete(hw_suf_handle);
        AUI_TEST_CHECK_RTN(ret);

    }
#if 0
    if(!aui_test_user_confirm("do you see 5 pictures  one by one?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    EXIT:
#endif
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}

unsigned long test_osd_data_blit(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long lyaer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect rect;
	struct aui_osd_rect region_rect;
	struct aui_osd_rect clip_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_256_COLOR, AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555, AUI_OSD_HD_ARGB8888, AUI_OSD_HD_RGB565};
	unsigned long width = 512;
	unsigned long height = 384;
	const unsigned char *data[] = {Image8Data, Image4444Data, Image1555Data, Image8888Data, Image565Data};
	unsigned long pitch[] = {width, width * 2, width * 2, width * 4, width * 2};
	unsigned long bpp[] = {1, 2, 2, 4, 2};
	unsigned long color_cnt = 5;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long i = 0;
	unsigned char *p_data = NULL;
	aui_blit_operation blit_op;
	aui_blit_rect blit_rect;
	aui_pallette_info p_pallette_info;

	p_data = MALLOC(width * height * 4);
	//AUI_TEST_CHECK_RTN(!p_data, NULL);
	MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&clip_rect, 0, sizeof(struct aui_osd_rect));    
    MEMSET(&p_pallette_info, 0, sizeof(struct aui_st_pallette_info)); 
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(lyaer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off((aui_hdl)layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

    rect.uHeight = height;
    rect.uLeft = 0;
    rect.uTop = 0;
    rect.uWidth = width;

    region_rect.uHeight = height;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = width;

    MEMSET(&blit_op, 0X0, sizeof(aui_blit_operation));
    blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
    //blit_op.mirror_type = GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT;
    blit_op.alpha_blend_mode = AUI_GE_ALPHA_BLEND_DST_OVER;
    for(i = 0; i < color_cnt; i++) {
        //create hw/sw surface
		if(pixel_format[i] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
        MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
		MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
		MEMCPY(&blit_rect.dst_rect, &rect, sizeof(struct aui_osd_rect));
        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create((aui_hdl)layer_handle, pixel_format[i], &region_rect, (aui_hdl*)&hw_suf_handle, 0);
		AUI_TEST_CHECK_RTN(ret);
        aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);
    
		if(AUI_OSD_256_COLOR == pixel_format[i]) {
			p_pallette_info.p_pallette = (unsigned char *)Data_Color1;
			p_pallette_info.color_cnt = 256;
			p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
			ret = aui_gfx_surface_pallette_set((aui_hdl)hw_suf_handle, &p_pallette_info);
			AUI_TEST_CHECK_RTN(ret);
		}

		AUI_PRINTF("----> AUI_GE_ORINGINAL_IMAGE\n");
		ret = aui_gfx_surface_fill((aui_hdl)hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush((aui_hdl)hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(1000);

		ret = aui_gfx_data_blit((aui_hdl)hw_suf_handle, (unsigned char *)data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush((aui_hdl)hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(3000);
		//test capture
		AUI_PRINTF("----> DONE\n");
#if 1
		clip_rect.uHeight = 200;
		clip_rect.uLeft = 100;
		clip_rect.uTop = 100;
		clip_rect.uWidth = 300;
		AUI_PRINTF("----> AUI_GE_DATA_BLIT\n");

		MEMSET(p_data, 0, width * height * 4);
		ret = aui_gfx_surface_capture((aui_hdl)hw_suf_handle, &clip_rect, p_data);
		AUI_TEST_CHECK_RTN(ret);
		
        aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
        AUI_TEST_CHECK_RTN(ret);

        AUI_SLEEP(1000);
        MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
        MEMCPY(&blit_rect.dst_rect, &clip_rect, sizeof(struct aui_osd_rect));
        MEMCPY(&blit_rect.fg_rect, &clip_rect, sizeof(struct aui_osd_rect));
        blit_rect.fg_rect.uLeft = 0;  //after capture,the starting coordinate is zero
        blit_rect.fg_rect.uTop = 0;
        ret = aui_gfx_data_blit(hw_suf_handle, p_data, &blit_rect, &blit_op,bpp[i] * clip_rect.uWidth, pixel_format[i]);
        AUI_TEST_CHECK_RTN(ret);

		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(3000);
		AUI_PRINTF("----> DONE\n");
#endif
		//delete the surface
		ret = aui_gfx_surface_delete(hw_suf_handle);
		AUI_TEST_CHECK_RTN(ret);

	}
#if 0
    if(!aui_test_user_confirm("do you see 5 pictures  one by one?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    EXIT:
#endif
	if(p_data) {
		free(p_data);
	}
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}
unsigned long test_osd_surface_blit(unsigned long *argc, char **argv, char *sz_out_put)
{
	struct aui_osd_rect rect;
	struct aui_osd_rect fill_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl sw_suf_handle1= NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_256_COLOR, AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555, AUI_OSD_HD_ARGB8888, AUI_OSD_HD_RGB565};
	unsigned long width = 640;
	unsigned long height = 360;
	unsigned long color[] = {RED_CLUT8, GREEN_4444, BLUE_1555, WHITE_8888, RED_565};
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long color_cnt = 5;
	unsigned long i = 0;
	aui_blit_operation blit_op;
	aui_blit_rect blit_rect;
	unsigned long layer_id = AUI_OSD_LAYER_GMA0;
	aui_pallette_info p_pallette_info;

    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&fill_rect, 0, sizeof(struct aui_osd_rect));
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(layer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

    rect.uHeight = height;
    rect.uLeft = 0;
    rect.uTop = 0;
    rect.uWidth = width;

    fill_rect.uHeight = height;
    fill_rect.uLeft = 0;
    fill_rect.uTop = 0;
    fill_rect.uWidth = width;

    MEMSET(&blit_rect, 0, sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.dst_rect, &rect, sizeof(struct aui_osd_rect));
    MEMSET(&blit_op, 0, sizeof(aui_blit_operation));
    blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
    for(i = 0; i < color_cnt; i++) {
		//create hw/sw surface
		if(pixel_format[i] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[i], &rect, (aui_hdl*)&hw_suf_handle, 1);
		AUI_TEST_CHECK_RTN(ret);
        aui_osd_scale(layer_handle, rect.uWidth, rect.uHeight);
    
		ret = aui_gfx_sw_surface_create(pixel_format[i], width, height, (aui_hdl*)&sw_suf_handle1);
		AUI_TEST_CHECK_RTN(ret);

		if(AUI_OSD_256_COLOR == pixel_format[i]) {
            MEMSET(&p_pallette_info, 0, sizeof(struct aui_st_pallette_info)); 
			p_pallette_info.p_pallette = (unsigned char *)P_Color1;
			p_pallette_info.color_cnt = 256;
			p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
			ret = aui_gfx_surface_pallette_set(hw_suf_handle, &p_pallette_info);
			AUI_TEST_CHECK_RTN(ret);
		}

		//fill sw surface
		ret = aui_gfx_surface_fill(sw_suf_handle1, color[i], &fill_rect);
		AUI_TEST_CHECK_RTN(ret);
		//blit to hw surface
		ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle1, NULL, &blit_op, &blit_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &fill_rect);
		AUI_TEST_CHECK_RTN(ret);

        AUI_SLEEP(3000);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_delete(sw_suf_handle1);
        AUI_TEST_CHECK_RTN(ret);

		//delete the surface
		ret = aui_gfx_surface_delete(hw_suf_handle);
    }

	AUI_SLEEP(3);

//EXIT:
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}
unsigned long test_osd_sw_surface_test(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long layer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect rect;
	struct aui_osd_rect fill_rect;
	aui_hdl  hw_suf_handle[HW_SURFACE_CNT] = {0};
	aui_hdl sw_suf_handle[HW_SURFACE_CNT] = {0};
	aui_hdl layer_handle = 0;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_256_COLOR, AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555, AUI_OSD_HD_ARGB8888, AUI_OSD_HD_RGB565};
	unsigned long width = 720;
	unsigned long height = 576 / HW_SURFACE_CNT;
	unsigned long hw_surface_id = 0;
	unsigned long color[HW_SURFACE_CNT] = {RED_CLUT8, GREEN_4444, BLUE_1555, WHITE_8888, RED_565};
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_blit_rect blit_rect;
	aui_blit_operation blit_op;
	aui_pallette_info p_pallette_info;

    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&fill_rect, 0, sizeof(struct aui_osd_rect));
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(layer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

	rect.uHeight = height;
	rect.uLeft = 0;
	rect.uTop = 0;
	rect.uWidth = width;

	fill_rect.uHeight = height;
	fill_rect.uLeft = 0;
	fill_rect.uTop = 0;
	fill_rect.uWidth = width;
	memset(&blit_op, 0, sizeof(blit_op));
	blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
	MEMCPY(&blit_rect.dst_rect, &rect, sizeof(struct aui_osd_rect));
	MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
	//blit_op.mirror_type = GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT;
	//create hw/sw surface and fill it
	for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) 
	{
		if(pixel_format[hw_surface_id] == AUI_OSD_256_COLOR) {
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
		rect.uTop = hw_surface_id * height;

        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[hw_surface_id], &rect, (aui_hdl*)&hw_suf_handle[hw_surface_id], 0);
		AUI_TEST_CHECK_RTN(ret);
        aui_osd_scale(layer_handle, rect.uWidth, 576);
    
		ret = aui_gfx_surface_fill(hw_suf_handle[hw_surface_id], 0xA5A5A5A5, &fill_rect);

		AUI_SLEEP(1000);


		AUI_TEST_CHECK_RTN(ret);

		ret = aui_gfx_sw_surface_create(pixel_format[hw_surface_id], width, height, (aui_hdl*)&sw_suf_handle[hw_surface_id]);

		if(AUI_OSD_256_COLOR == pixel_format[hw_surface_id]) {
            MEMSET(&p_pallette_info, 0, sizeof(struct aui_st_pallette_info));
			p_pallette_info.p_pallette = (unsigned char *)P_Color1;
			p_pallette_info.color_cnt = 256;
			p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
			ret = aui_gfx_surface_pallette_set(hw_suf_handle[hw_surface_id], &p_pallette_info);
			AUI_TEST_CHECK_RTN(ret);
		}

		ret = aui_gfx_surface_fill(sw_suf_handle[hw_surface_id], color[hw_surface_id], &fill_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_blit(hw_suf_handle[hw_surface_id], sw_suf_handle[hw_surface_id], NULL, &blit_op, &blit_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle[hw_surface_id], &fill_rect);
		AUI_TEST_CHECK_RTN(ret);
	}

	AUI_SLEEP(3000);
	for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) {
        if(hw_surface_id== 0)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
        ret = aui_gfx_surface_show_on_off(hw_suf_handle[hw_surface_id], 0);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(1000);
	}
	
//EXIT:
	for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) {
		if(hw_surface_id== 0)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
		ret = aui_gfx_surface_delete(hw_suf_handle[hw_surface_id]);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_delete(sw_suf_handle[hw_surface_id]);
		AUI_TEST_CHECK_RTN(ret);
		//AUI_SLEEP(1000);
	}
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}

unsigned long test_osd_color_key(unsigned long *argc, char **argv, char *sz_out_put)
{
#define COLOR_CNT 4
#define REGION_HEIGHT 360
#define REGION_WIDTH 640

	unsigned long layer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect region_rect;
	struct aui_osd_rect fill_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl sw_suf_handle = NULL;
	aui_hdl sw_suf_handle2 = NULL;
	aui_hdl layer_handle = NULL;
	unsigned long width = REGION_WIDTH;
	unsigned long height = REGION_HEIGHT / COLOR_CNT;
#if 0 
    enum aui_osd_pixel_format e_color_mode = AUI_OSD_HD_ARGB8888;
    unsigned long color[COLOR_CNT] = {RED_8888, GREEN_8888, BLUE_8888, WHITE_8888};
    unsigned long color2[COLOR_CNT] = {BLUE_8888, WHITE_8888, RED_8888, GREEN_8888};
    unsigned long color_key[COLOR_CNT] = {RED_8888, GREEN_8888, WHITE_8888, BLUE_8888};
#else 
    aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB1555;
    unsigned long color[COLOR_CNT] = {RED_1555, GREEN_1555, BLUE_1555, WHITE_1555};
    unsigned long color2[COLOR_CNT] = {BLUE_1555, WHITE_1555, RED_1555, GREEN_1555};
    unsigned long color_key[COLOR_CNT] = {RED_1555, GREEN_1555, WHITE_1555, BLUE_1555};

#endif    
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_blit_rect blit_rect;
	aui_blit_operation blit_op;
	unsigned long i = 0;
	aui_color_key st_color_key;

    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&fill_rect, 0, sizeof(struct aui_osd_rect));
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(layer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

	region_rect.uHeight = REGION_HEIGHT;
	region_rect.uLeft = 0;
	region_rect.uTop = 0;
	region_rect.uWidth = REGION_WIDTH;

	fill_rect.uHeight = height;
	fill_rect.uLeft = 0;
	fill_rect.uTop = 0;
	fill_rect.uWidth = width;

	MEMSET(&blit_op, 0, sizeof(aui_blit_operation));
	blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
	blit_op.color_key_source = AUI_GE_CKEY_PTN_POST_CLUT;
#ifndef AUI_LINUX
	/** DFB doesn't support these modes. */
	blit_op.color_key_red_match_mode = AUI_GE_CKEY_MATCH_IN_RANGE;
	blit_op.color_key_green_match_mode = AUI_GE_CKEY_MATCH_IN_RANGE;
	blit_op.color_key_blue_match_mode = AUI_GE_CKEY_MATCH_IN_RANGE;
#endif
    MEMSET(&blit_rect, 0, sizeof(struct aui_osd_rect));
	MEMCPY(&blit_rect.dst_rect, &region_rect, sizeof(struct aui_osd_rect));
	MEMCPY(&blit_rect.fg_rect, &region_rect, sizeof(struct aui_osd_rect));
	//blit_op.mirror_type = GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT;
	//create hw/sw surface and fill it
    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
	ret = aui_gfx_hw_surface_create(layer_handle, pixel_format, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
	AUI_TEST_CHECK_RTN(ret);
    aui_osd_scale(layer_handle, region_rect.uWidth, 576);

	ret = aui_gfx_surface_fill(hw_suf_handle, 0, &region_rect);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
	AUI_TEST_CHECK_RTN(ret);
	
	ret = aui_gfx_sw_surface_create(pixel_format, REGION_WIDTH, REGION_HEIGHT, (aui_hdl*)&sw_suf_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_surface_fill(sw_suf_handle, 0, &region_rect);
	AUI_TEST_CHECK_RTN(ret);
	
	ret = aui_gfx_sw_surface_create(pixel_format, REGION_WIDTH, REGION_HEIGHT, (aui_hdl*)&sw_suf_handle2);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_surface_fill(sw_suf_handle2, 0, &region_rect);
	AUI_TEST_CHECK_RTN(ret);
	AUI_SLEEP(1000);
	
    for(i = 0; i < COLOR_CNT; i++) {
        fill_rect.uTop = i * height;

        ret = aui_gfx_surface_fill(sw_suf_handle, color[i], &fill_rect);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_fill(sw_suf_handle2, color2[i], &fill_rect);
        AUI_TEST_CHECK_RTN(ret);

        MEMSET(&st_color_key, 0, sizeof(struct aui_st_color_key));
		st_color_key.color_key_max = color_key[i];
		st_color_key.color_key_min = color_key[i];
        AUI_PRINTF("\n set color_key: 0x%x\n", color_key[i]);
        if(!(i % 2)) {
            AUI_PRINTF("blit 0x%x\n", color[i]);
            aui_gfx_surface_colorkey_set(sw_suf_handle, &st_color_key);
            AUI_PRINTF("filter color 0x%x\n", color_key[i]);
            ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
        } else {
            AUI_PRINTF("blit 0x%x\n", color2[i]);
            aui_gfx_surface_colorkey_set(sw_suf_handle2, &st_color_key);
            AUI_PRINTF("filter color 0x%x\n", color_key[i]);
            ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle2, NULL, &blit_op, &blit_rect);
        }
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &fill_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(3000);
	}
#if 0
    if(!aui_test_user_confirm("do you see 4 kinds of color ?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    EXIT:
#endif

    ret = aui_gfx_surface_delete(hw_suf_handle);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_delete(sw_suf_handle);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_delete(sw_suf_handle2);
    AUI_TEST_CHECK_RTN(ret);
    AUI_SLEEP(1000);
    ret = aui_gfx_layer_close(layer_handle);
    AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}


unsigned long test_osd_hw_surface_test(unsigned long *argc, char **argv, char *sz_out_put)
{
	struct aui_osd_rect rect;
	struct aui_osd_rect fill_rect;
	aui_hdl hw_suf_handle[HW_SURFACE_CNT] = {NULL};
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_256_COLOR, AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555, AUI_OSD_HD_ARGB8888, AUI_OSD_HD_RGB555};
	unsigned long color[HW_SURFACE_CNT] = {RED_CLUT8, GREEN_4444, BLUE_1555, WHITE_8888, RED_555};
	unsigned long width = 720;
	unsigned long height = 576 / HW_SURFACE_CNT - 1;
	unsigned long hw_surface_id = 0;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_pallette_info p_pallette_info;

    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&fill_rect, 0, sizeof(struct aui_osd_rect));
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(0, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off((aui_hdl)layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

    rect.uHeight = height;
    rect.uLeft = 0;
    rect.uTop = 0;
    rect.uWidth = width;

	fill_rect.uHeight = height;
	fill_rect.uLeft = 0;
	fill_rect.uTop = 0;
	fill_rect.uWidth = width;
	//create hw surface and fill it
	for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) {

		if(pixel_format[hw_surface_id] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
		rect.uTop = hw_surface_id * height;
        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[hw_surface_id], &rect, (aui_hdl*)&hw_suf_handle[hw_surface_id], 1);
        aui_osd_scale(layer_handle, rect.uWidth, 576);
    
		if(AUI_OSD_256_COLOR == pixel_format[hw_surface_id]) {
            MEMSET(&p_pallette_info, 0, sizeof(struct aui_st_pallette_info));
			p_pallette_info.p_pallette = (unsigned char*)P_Color1;
			p_pallette_info.color_cnt = 256;
			p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
			ret = aui_gfx_surface_pallette_set(hw_suf_handle[hw_surface_id], &p_pallette_info);
			AUI_TEST_CHECK_RTN(ret);
		}
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_fill(hw_suf_handle[hw_surface_id], color[hw_surface_id], &fill_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle[hw_surface_id], &fill_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(1000);
	}
#if 0
    if(!aui_test_user_confirm("do you see 5 kinds of color ?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
#endif
    //hide the hw surface
    for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) {
        if(pixel_format[hw_surface_id] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
        ret = aui_gfx_surface_show_on_off(hw_suf_handle[hw_surface_id], 0);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(1000);
    }

    //show the hw surface
    for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) {
        if(pixel_format[hw_surface_id] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
        ret = aui_gfx_surface_show_on_off(hw_suf_handle[hw_surface_id], 1);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(1000);
    }

    //hide osd layer
    ret = aui_gfx_layer_show_on_off(layer_handle, 0);
    AUI_SLEEP(1000);
    ret = aui_gfx_layer_show_on_off(layer_handle, 1);
    AUI_SLEEP(1000);
#if 0
    if(!aui_test_user_confirm("do you see 5 colors hide one bye one ?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;

    }
    //AUI_SLEEP(10000);
EXIT:
#endif
	for(hw_surface_id = 0; hw_surface_id < HW_SURFACE_CNT; hw_surface_id++) {
		if(hw_surface_id== 0)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
		ret = aui_gfx_surface_delete(hw_suf_handle[hw_surface_id]);
		AUI_TEST_CHECK_RTN(ret);
	}
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}


/** don't support by linux aui */
unsigned long test_osd_bool_test(unsigned long *argc, char **argv, char *sz_out_put)
{
	struct aui_osd_rect rect;
	struct aui_osd_rect fill_rect;
	aui_hdl hw_suf_handle[HW_SURFACE_CNT] = {NULL};
	aui_hdl sw_suf_handle[HW_SURFACE_CNT] = {NULL};
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
	unsigned long width = 720;
	unsigned long height = 576 ;
	unsigned long hw_surface_id = AUI_OSD_LAYER_GMA0;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_blit_operation blit_op;
	aui_blit_rect blit_rect;
    
    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&fill_rect, 0, sizeof(struct aui_osd_rect));
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(0, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

    rect.uHeight = height;
    rect.uLeft = 0;
    rect.uTop = 0;
    rect.uWidth = width;

	fill_rect.uHeight = height;
	fill_rect.uLeft = 0;
	fill_rect.uTop = 0;
	fill_rect.uWidth = width;
	//create hw surface and fill it
	rect.uTop = hw_surface_id * height;
    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
	ret = aui_gfx_hw_surface_create(layer_handle, pixel_format, &rect, (aui_hdl*)&hw_suf_handle[hw_surface_id], 0);
	AUI_TEST_CHECK_RTN(ret);
    aui_osd_scale(layer_handle, rect.uWidth, rect.uHeight);

	ret = aui_gfx_surface_fill(hw_suf_handle[hw_surface_id], 0xFF000000, &fill_rect);
	AUI_TEST_CHECK_RTN(ret);

	ret = aui_gfx_sw_surface_create(pixel_format, rect.uWidth, rect.uHeight, (aui_hdl*)&sw_suf_handle[hw_surface_id]);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_surface_fill(sw_suf_handle[hw_surface_id], 0xFF, &fill_rect);
	AUI_TEST_CHECK_RTN(ret);

    MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
    MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.dst_rect, &rect, sizeof(struct aui_osd_rect));
    MEMSET(&blit_op, 0, sizeof(aui_blit_operation));
    blit_op.rop_operation = AUI_GFX_ROP_BOOLEAN;
    blit_op.bop_mode = AUI_GE_BOP_SETWHITE;
    ret = aui_gfx_surface_blit(hw_suf_handle[hw_surface_id], sw_suf_handle[hw_surface_id], NULL, &blit_op, &blit_rect);
    AUI_TEST_CHECK_RTN(ret);


    AUI_SLEEP(2000);
#if 0
    if(!aui_test_user_confirm("do you see 5 kinds of color ?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
#endif
    //hide the hw surface
    ret = aui_gfx_surface_show_on_off(hw_suf_handle[hw_surface_id], 0);
    AUI_TEST_CHECK_RTN(ret);
    //AUI_SLEEP(10000);
//EXIT:
	ret = aui_gfx_surface_delete(hw_suf_handle[hw_surface_id]);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}

unsigned long test_osd_aphablending_blit(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long lyaer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect rect;
	struct aui_osd_rect region_rect;
	struct aui_osd_rect dst_rect;
	struct aui_osd_rect clip_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555};
	unsigned long width = 65;
	unsigned long height = 51;
	const unsigned char *data[] = {Alpha4444Data, Alpha1555Data};
	unsigned long pitch[] = {width * 2, width * 2};
	unsigned long back_color[] = {BLUE_4444, BLUE_1555};
	//unsigned long bpp[] = {2, 2};
	unsigned long color_cnt = 2;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long i = 0;
	unsigned char *p_data = NULL;
	aui_blit_operation blit_op;
	aui_blit_rect blit_rect;

	p_data = MALLOC(width * height * 4);
	//AUI_TEST_CHECK_RTN(!p_data, NULL);
	MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&dst_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&clip_rect, 0, sizeof(struct aui_osd_rect));
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(lyaer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

	rect.uHeight = height;
	rect.uLeft = 0;
	rect.uTop = 0;
	rect.uWidth = width;

	region_rect.uHeight = 576;
	region_rect.uLeft = 0;
	region_rect.uTop = 0;
	region_rect.uWidth = 720;

	dst_rect.uHeight = height * 2;
	dst_rect.uLeft = 100;
	dst_rect.uTop = 100;
	dst_rect.uWidth = width * 2;

	MEMSET(&blit_op, 0X0, sizeof(aui_blit_operation));
	blit_op.rop_operation = AUI_GFX_ROP_ALPHA_BLENDING;
	//blit_op.mirror_type = AUI_GFX_MIRROR_RIGHT_LEFT;
	blit_op.alpha_blend_mode = AUI_GE_ALPHA_BLEND_SRC_OVER;
	for(i = 0; i < color_cnt; i++) {
		//create hw/sw surface
		MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
		MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
		MEMCPY(&blit_rect.dst_rect, &dst_rect, sizeof(struct aui_osd_rect));
        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[i], &region_rect, (aui_hdl*)&hw_suf_handle, 0);
		AUI_TEST_CHECK_RTN(ret);
        aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);
    
		ret = aui_gfx_surface_fill(hw_suf_handle, back_color[i], &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(1000);
		ret = aui_gfx_data_blit(hw_suf_handle, (unsigned char*)data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(5000);
		//delete the surface
		ret = aui_gfx_surface_delete(hw_suf_handle);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(1000);

	}
#if 0
	if(!aui_test_user_confirm("do you see 5 pictures  one by one?")) {
		ret = AUI_RTN_FAIL;
		goto EXIT;
	}
	EXIT:
#endif
	if(p_data) {
		free(p_data);
	}
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}

unsigned long test_osd_scale_blit(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long lyaer_id = AUI_OSD_LAYER_GMA0;
	struct aui_osd_rect rect;
	struct aui_osd_rect region_rect;
	struct aui_osd_rect dst_rect;
	struct aui_osd_rect clip_rect;
	aui_hdl hw_suf_handle = NULL;
	aui_hdl layer_handle = NULL;
	aui_osd_pixel_format pixel_format[] = {AUI_OSD_256_COLOR, AUI_OSD_HD_ARGB4444, AUI_OSD_HD_ARGB1555, AUI_OSD_HD_ARGB8888, AUI_OSD_HD_RGB565};
	unsigned long width = 512;
	unsigned long height = 384;
	const unsigned char *data[] = {Image8Data, Image4444Data, Image1555Data, Image8888Data, Image565Data};
	unsigned long pitch[] = {width, width * 2, width * 2, width * 4, width * 2};
	unsigned long bpp[] = {1, 2, 2, 4, 2};
	unsigned long color_cnt = 5;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long i = 0;
	unsigned char *p_data = NULL;
	aui_blit_operation blit_op;
	aui_blit_rect blit_rect;
	aui_pallette_info p_pallette_info;

	p_data = MALLOC(width * height * 4);
	MEMSET(&rect,0,sizeof(struct aui_osd_rect));
	MEMSET(&region_rect,0,sizeof(struct aui_osd_rect));
	MEMSET(&dst_rect,0,sizeof(struct aui_osd_rect));
	MEMSET(&clip_rect,0,sizeof(struct aui_osd_rect));	
	//AUI_TEST_CHECK_RTN(!p_data, NULL);
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
	ret = aui_gfx_layer_open(lyaer_id, (aui_hdl*)&layer_handle);
	AUI_TEST_CHECK_RTN(ret);
	ret = aui_gfx_layer_show_on_off(layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

    rect.uHeight = height;
    rect.uLeft = 0;
    rect.uTop = 0;
    rect.uWidth = width;

    region_rect.uHeight = 576;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 720;

    dst_rect.uHeight = height / 2;
    dst_rect.uLeft = 0;
    dst_rect.uTop = 0;
    dst_rect.uWidth = width / 2;

    MEMSET(&blit_op, 0X0, sizeof(aui_blit_operation));
    blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
    //blit_op.mirror_type = GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT;
    blit_op.alpha_blend_mode = AUI_GE_ALPHA_BLEND_SRC_OVER;
    for(i = 0; i < color_cnt; i++) {
        //create hw/sw surface
		if(pixel_format[i] == AUI_OSD_256_COLOR)
		{
		    //Do not support "AUI_OSD_256_COLOR" on "AUI_OSD_LAYER_GMA0"
			continue;
		}
        MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
		MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
		MEMCPY(&blit_rect.dst_rect, &dst_rect, sizeof(struct aui_osd_rect));
        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
		ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[i], &region_rect, (aui_hdl*)&hw_suf_handle, 0);
		AUI_TEST_CHECK_RTN(ret);
        aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);
    
		if(AUI_OSD_256_COLOR == pixel_format[i]) {
            MEMSET(&p_pallette_info,0,sizeof(aui_pallette_info));
			p_pallette_info.p_pallette = (unsigned char*)Data_Color1;
			p_pallette_info.color_cnt = 256;
			p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
			ret = aui_gfx_surface_pallette_set(hw_suf_handle, &p_pallette_info);
			AUI_TEST_CHECK_RTN(ret);
		}

		ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(1000);

        AUI_PRINTF("----> scale blit :(%d, %d) - %d * %d ==> (%d, %d) - %d * %d\n",
            blit_rect.fg_rect.uLeft, blit_rect.fg_rect.uTop, blit_rect.fg_rect.uWidth, blit_rect.fg_rect.uHeight,
            blit_rect.dst_rect.uLeft, blit_rect.dst_rect.uTop, blit_rect.dst_rect.uWidth, blit_rect.dst_rect.uHeight);
		ret = aui_gfx_data_blit(hw_suf_handle, (unsigned char*)data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
		AUI_TEST_CHECK_RTN(ret);
		ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
		AUI_TEST_CHECK_RTN(ret);
		AUI_SLEEP(5000);
		//test capture
#if 1
        clip_rect.uHeight = 100;
        clip_rect.uLeft = 20;
        clip_rect.uTop = 20;
        clip_rect.uWidth = 100;

        MEMSET(p_data, 0, width * height * 4);
        ret = aui_gfx_surface_capture(hw_suf_handle, &clip_rect, p_data);
        AUI_TEST_CHECK_RTN(ret);
        
        AUI_PRINTF("----> clear screen\n");
        ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(1000);
        
        MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
        MEMCPY(&blit_rect.dst_rect, &clip_rect, sizeof(struct aui_osd_rect));
        MEMCPY(&blit_rect.fg_rect, &clip_rect, sizeof(struct aui_osd_rect));
        AUI_PRINTF("----> capture & blit: (%d, %d) - %d * %d\n", clip_rect.uLeft, clip_rect.uTop, clip_rect.uWidth, clip_rect.uHeight);
        blit_rect.fg_rect.uLeft = 0;  //after capture,the starting coordinate is zero
        blit_rect.fg_rect.uTop = 0;
        ret = aui_gfx_data_blit(hw_suf_handle, p_data, &blit_rect, &blit_op, bpp[i] * clip_rect.uWidth, pixel_format[i]);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_flush(hw_suf_handle, &clip_rect);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(3000);
        AUI_PRINTF("----> done\n\n");
#endif
        //delete the surface
        ret = aui_gfx_surface_delete(hw_suf_handle);
        AUI_TEST_CHECK_RTN(ret);

    }
#if 0
    if(!aui_test_user_confirm("do you see 5 pictures  one by one?")) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    EXIT:
#endif
	if(p_data) {
		free(p_data);
	}
	ret = aui_gfx_layer_close(layer_handle);
	AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
	AUI_TEST_CHECK_RTN(ret);

	return AUI_RTN_SUCCESS;
}



unsigned long test_osd_two_layer(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long layer_id1 = AUI_OSD_LAYER_GMA0;
    unsigned long layer_id2 = AUI_OSD_LAYER_GMA1;
	struct aui_osd_rect rect;
	struct aui_osd_rect region_rect1;
    struct aui_osd_rect region_rect2;
    aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB4444;
    aui_hdl layer_handle1 = NULL;
    aui_hdl layer_handle2 = NULL;
    aui_hdl hw_surface_handle1 = NULL;
    aui_hdl hw_surface_handle2 = NULL;
    aui_hdl sw_surface_handle = NULL;
    aui_blit_operation blit_op;
	aui_blit_rect blit_rect;
   	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_pallette_info p_pallette_info;

    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect1, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect2, 0, sizeof(struct aui_osd_rect));

    AUI_PRINTF(" %s ---> \n", __func__);
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif

    if (aui_find_dev_by_idx(AUI_MODULE_GFX, layer_id1, &layer_handle1))
    {
        AUI_PRINTF("\n aui_find_dev_by_idx layer_id1 fail, call aui_gfx_layer_open\n");
        ret = aui_gfx_layer_open(layer_id1, (aui_hdl*)&layer_handle1);
        AUI_TEST_CHECK_RTN(ret);
    }
	ret = aui_gfx_layer_show_on_off(layer_handle1, 1);
	AUI_TEST_CHECK_RTN(ret);

    if (aui_find_dev_by_idx(AUI_MODULE_GFX, layer_id2, &layer_handle2))
    {
        AUI_PRINTF("\n aui_find_dev_by_idx layer_id2 fail, call aui_gfx_layer_open\n");
        ret = aui_gfx_layer_open(layer_id2, (aui_hdl*)&layer_handle2);
        AUI_TEST_CHECK_RTN(ret);
    }
	ret = aui_gfx_layer_show_on_off(layer_handle2, 1);
	AUI_TEST_CHECK_RTN(ret);

    region_rect1.uLeft = 0;
    region_rect1.uTop = 0;
    region_rect1.uHeight = 576;
    region_rect1.uWidth = 720;

    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
    ret = aui_gfx_hw_surface_create(layer_handle1,pixel_format,&region_rect1,&hw_surface_handle1,0);
    AUI_TEST_CHECK_RTN(ret);
    aui_osd_scale(layer_handle1, region_rect1.uWidth, region_rect1.uHeight);

    region_rect2.uLeft = 100;
    region_rect2.uTop = 50;
    region_rect2.uHeight = 50;
    region_rect2.uWidth = 150;    

    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
    ret = aui_gfx_hw_surface_create(layer_handle2,pixel_format,&region_rect2,&hw_surface_handle2,0);
    AUI_TEST_CHECK_RTN(ret);
    if(AUI_OSD_256_COLOR == pixel_format) {
        AUI_PRINTF("\n >>> need to set pallette! <<<<\n");
        p_pallette_info.p_pallette = (unsigned char*)Data_Color1;
        p_pallette_info.color_cnt = 256;
        p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
        ret = aui_gfx_surface_pallette_set(hw_surface_handle2, &p_pallette_info);
        AUI_TEST_CHECK_RTN(ret);

        // In this color mode, the fb2 driver would close fb2 layer, to avoid flower screen.
        // so need to clear fb2 with transparent color and show on the layer again.
        ret = aui_gfx_layer_show_on_off(layer_handle2, 1);
        AUI_TEST_CHECK_RTN(ret);
    }
    
    rect.uTop = 0;
    rect.uLeft = 0;
    rect.uHeight = 50;
    rect.uWidth = 40;

    ret = aui_gfx_sw_surface_create(pixel_format,rect.uWidth,rect.uHeight,&sw_surface_handle);
    AUI_TEST_CHECK_RTN(ret);

    ret = aui_gfx_surface_fill(hw_surface_handle1,0x4f00,&region_rect1);
    AUI_TEST_CHECK_RTN(ret);
    // Flush to show on screen immediately.
    ret = aui_gfx_surface_flush(hw_surface_handle1, &region_rect1);
    AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF(" %s ---> fill rect[(%d, %d) - %d * %d] with 0x4f00 on fb0.\n", 
        __func__,region_rect1.uTop, region_rect1.uLeft, region_rect1.uWidth, region_rect1.uHeight);

    region_rect2.uLeft = 0;
    region_rect2.uTop = 0;

    ret = aui_gfx_surface_fill(hw_surface_handle2,0xffff,&region_rect2);
    AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF(" %s ---> fill rect[(%d, %d) - %d * %d] with 0xffff on fb2.\n", 
        __func__,region_rect2.uTop, region_rect2.uLeft, region_rect2.uWidth, region_rect2.uHeight);


    ret = aui_gfx_surface_fill(sw_surface_handle,0xf00f,&rect);
    AUI_TEST_CHECK_RTN(ret);    

    MEMSET(&blit_op,0,sizeof(aui_blit_operation));

    blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));

    MEMCPY((&blit_rect.fg_rect),(&rect),sizeof(struct aui_osd_rect));

    rect.uLeft = 55;

    MEMCPY((&blit_rect.dst_rect),(&rect),sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_surface_handle2, sw_surface_handle, NULL, &blit_op, &blit_rect);
    AUI_TEST_CHECK_RTN(ret); 
    AUI_SLEEP(3000);
    AUI_PRINTF(" %s ---> blit rect[0xf00f, (%d, %d), %d * %d] on fb2.\n", 
        __func__,rect.uTop, rect.uLeft, rect.uWidth, rect.uHeight);


    ret = aui_gfx_layer_show_on_off(layer_handle1, 0);
	AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF(" %s ---> fb0 off.\n",__func__);
    AUI_SLEEP(3000);

    ret = aui_gfx_layer_show_on_off(layer_handle1, 1);
	AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF(" %s ---> fb0 on.\n",__func__);
    AUI_SLEEP(3000);

    ret = aui_gfx_layer_show_on_off(layer_handle2, 0);
	AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF(" %s ---> fb2 off.\n",__func__);
    AUI_SLEEP(3000);

    ret = aui_gfx_layer_show_on_off(layer_handle2, 1);
	AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF(" %s ---> fb2 on.\n",__func__);
    AUI_SLEEP(3000);
//EXIT:
    if(hw_surface_handle1!=NULL)
    {
        aui_gfx_surface_delete(hw_surface_handle1);
    }
    
    if(hw_surface_handle2!=NULL)
    {
        aui_gfx_surface_delete(hw_surface_handle2);
    }

    if(sw_surface_handle!=NULL)
    {
        aui_gfx_surface_delete(sw_surface_handle);
    }

    aui_gfx_layer_close(layer_handle1);

    aui_gfx_layer_close(layer_handle2);

#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
    AUI_PRINTF(" %s <--- done! \n", __func__);
    return ret;

}

#ifdef AUI_LINUX
unsigned long test_osd_show_image(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long layer_id = AUI_OSD_LAYER_GMA0;
    struct aui_osd_rect region_rect;
    aui_hdl hw_suf_handle = NULL;
    aui_hdl layer_handle = NULL;
    aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    int auto_clean = 0;
    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    region_rect.uHeight = 720;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 1280;

    if (*argc < 1) {
        AUI_PRINTF("Usage: 13 image_path,[width],[height],[auto_clean]\ndefault width: 1280, height: 720\n");
        AUI_PRINTF("'auto_clean = 1' mean after test, image will hide.");
        return -1;
    }	
    AUI_PRINTF("image path: %s\n", argv[0]);
    if (*argc > 2) {
        char *tailprt = NULL;
        AUI_PRINTF("argv[1]: %d, argv[2]: %d\n", argv[1], argv[2]);
        region_rect.uWidth = strtol(argv[1], &tailprt, 0);
        region_rect.uHeight = strtol(argv[2], &tailprt, 0);
        auto_clean = strtol(argv[3], &tailprt, 0);
        AUI_PRINTF("get image size from user: width: %d, height: %d\n", region_rect.uWidth, region_rect.uHeight);
    }

    aui_gfx_init(NULL, NULL);
	/* check osd if already opened */
	if (aui_find_dev_by_idx(AUI_MODULE_GFX, layer_id, &layer_handle)) {
		ret = aui_gfx_layer_open(layer_id, (aui_hdl*)(&layer_handle));
		AUI_TEST_CHECK_RTN(ret);
    }
    ret = aui_gfx_layer_show_on_off(layer_handle, 1);
    AUI_TEST_CHECK_RTN(ret);
    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
    ret = aui_gfx_hw_surface_create(layer_handle, pixel_format, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
    AUI_TEST_CHECK_RTN(ret);
    aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);

    ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_render_image_to_surface(hw_suf_handle, argv[0]);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
    AUI_TEST_CHECK_RTN(ret);
    AUI_PRINTF("show image\n");
    AUI_SLEEP(5000);
    if (auto_clean) {
        ret = aui_gfx_layer_show_on_off(layer_handle, 0);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_delete(hw_suf_handle);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_layer_close(layer_handle);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_de_init(NULL, NULL);
        AUI_TEST_CHECK_RTN(ret);
    }

    return AUI_RTN_SUCCESS;
}

unsigned long test_osd_run_performace_test(unsigned long *argc, char **argv, char *sz_out_put)
{
    if (*argc < 1) {
        AUI_PRINTF("Usage: \n");
        AUI_PRINTF("\t run performace test:  14 1\n");
        AUI_PRINTF("\t kill performace test:  14 0\n");
        return -1;
    }	
    AUI_PRINTF("argv[0]: %s\n", argv[0]);

    int run = 0;
    run = atoi(argv[0]);
    AUI_PRINTF("%s df_dok.\n", run?"RUN":"KILL"); 
    if (run) {
        if(-1 == system("df_dok&")) {
			AUI_PRINTF("failed to run system call\n");
			return -1;
		}
	} else {
        if(-1 == system("killall df_dok")){
			AUI_PRINTF("failed to run system call\n");
			return -1;
		}
	}
    return AUI_RTN_SUCCESS;
}
#endif // #ifdef AUI_LINUX

unsigned long test_osd_note(unsigned long *argc, char **argv, char *sz_out_put)
{

	AUI_PRINTF("For s3503b change \"sys_memmap_3503.h\"\n \
				line 1067:\n \
				#define _MM_OSD1_LEN			  1280*720*2\n \
				to\n \
				#define _MM_OSD1_LEN			  1280*720*4\n");

	return AUI_RTN_SUCCESS;
}
unsigned long test_osd_split_scale_blit(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long lyaer_id = AUI_OSD_LAYER_GMA0;
    struct aui_osd_rect rect;
    struct aui_osd_rect region_rect;
    struct aui_osd_rect dst_rect;
    struct aui_osd_rect clip_rect;
    aui_hdl hw_suf_handle = NULL;
    aui_hdl layer_handle = NULL;
    aui_osd_pixel_format pixel_format[] = { AUI_OSD_HD_ARGB8888 };
    unsigned long width = 800;
    unsigned long height = 800;
    unsigned char *data[] = { aucResData2 };
    unsigned long pitch[] = { width * 4 };
    //unsigned long bpp[] = { 4};
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned long i = 0;
    unsigned char *p_data = NULL;
    aui_blit_operation blit_op;
    aui_blit_rect blit_rect;
    aui_pallette_info p_pallette_info;

    p_data = MALLOC(width * height * 4);
    //AUI_TEST_CHECK_RTN(!p_data, NULL);
    MEMSET(&rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&dst_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&clip_rect, 0, sizeof(struct aui_osd_rect));
	
#ifdef AUI_LINUX
    aui_gfx_init(NULL, NULL);
#endif
    ret = aui_gfx_layer_open(lyaer_id, &layer_handle);
    AUI_TEST_CHECK_RTN(ret);
    ret = aui_gfx_layer_show_on_off(layer_handle, 1);
    AUI_TEST_CHECK_RTN(ret);


    region_rect.uHeight = 576;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 720;

    MEMSET(&blit_op, 0X0, sizeof(aui_blit_operation));
    blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
    //blit_op.mirror_type = GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT;
    blit_op.alpha_blend_mode = AUI_GE_ALPHA_BLEND_SRC_OVER;
    i=0;
     {
        //create hw/sw surface
        // On linux, it always create double buffer. 
        // Because the directfb would create double buffer according to /etc/directfbrc.
        ret = aui_gfx_hw_surface_create(layer_handle, pixel_format[i], &region_rect, &hw_suf_handle, 0);
        AUI_PRINTF("\n%s %d I>Reg[%d]\n",__FUNCTION__,__LINE__,ret);
        AUI_TEST_CHECK_RTN(ret);
        aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);
    
        if(AUI_OSD_256_COLOR == pixel_format[i]) {
            MEMSET(&p_pallette_info, 0, sizeof(struct aui_st_pallette_info));
            p_pallette_info.p_pallette = (unsigned char *)Data_Color1;
            p_pallette_info.color_cnt = 256;
            p_pallette_info.en_pallette_color_type = AUI_PALLETTE_COLOR_TYPE_RGB;
            ret = aui_gfx_surface_pallette_set(hw_suf_handle, &p_pallette_info);
            AUI_TEST_CHECK_RTN(ret);
            AUI_PRINTF("%s %d I>PAL[%d]\n",__FUNCTION__,__LINE__,ret);
        }

        ret = aui_gfx_surface_fill(hw_suf_handle, TRANSPARENT_COLOR, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(1000);

		/*
		 * If we scale two portion of src image to destination
		 *  i.e Src( 0,0  - 256 x 96 ) to Dst( 0,0,   - 720 x 288 ) and
		 *      Src( 0,96 - 256 x 96 ) to Dst( 0, 288 - 720 x 288 )
		 * then additional black line is visible in destination region.
		 *	 In the below test code, we have scaled portion of the src image to
		 *   dest buffer i.e scale portion of image ( 0,0 - 256x96) to ( 0,0 - 720 x 288 ) of original image
		 *	 of size ( 512 x 384)
        	 */

		/* get some part from left top quater postion of Image */
		rect.uLeft=0;
		rect.uTop=0;
		rect.uWidth=256;
		rect.uHeight =96;

		dst_rect.uLeft=0;
		dst_rect.uTop=0;
		dst_rect.uWidth=region_rect.uWidth;
		dst_rect.uHeight=region_rect.uHeight>>1;
        MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
	   	MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
	    MEMCPY(&blit_rect.dst_rect, &dst_rect, sizeof(struct aui_osd_rect));
   	    AUI_PRINTF("%s %d I>RECT\n",__FUNCTION__,__LINE__);
        ret = aui_gfx_data_blit(hw_suf_handle, data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
        AUI_PRINTF("%s %d I>BLIT1[%d]\n",__FUNCTION__,__LINE__,ret);
        AUI_TEST_CHECK_RTN(ret);

        ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(3000);


		/*
		 *	 In the below test code, we have scaled portion of the src image to
		 *   dest buffer i.e scale portion of image ( 0,96 - 256x96) to (0,288 - 720 x 288 ) of original image
		 *	 of size ( 512 x 384)
          */

		/* get some part from  left bottom quater postion of Image */
		rect.uLeft=0;
		rect.uTop=96;
		rect.uWidth=256;
		rect.uHeight =96;

        dst_rect.uLeft=0;
        dst_rect.uTop=region_rect.uHeight>>1;
        dst_rect.uWidth=region_rect.uWidth;
        dst_rect.uHeight=region_rect.uHeight>>1;
        MEMSET(&blit_rect, 0, sizeof(struct aui_st_blit_rect));
        MEMCPY(&blit_rect.fg_rect, &rect, sizeof(struct aui_osd_rect));
        MEMCPY(&blit_rect.dst_rect, &dst_rect, sizeof(struct aui_osd_rect));

        ret = aui_gfx_data_blit(hw_suf_handle, data[i], &blit_rect, &blit_op, pitch[i], pixel_format[i]);
        AUI_PRINTF("%s %d I>BLIT2[%d]\n",__FUNCTION__,__LINE__,ret);
        AUI_TEST_CHECK_RTN(ret);

        ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
        AUI_TEST_CHECK_RTN(ret);
        AUI_SLEEP(5000);

        //delete the surface
        ret = aui_gfx_surface_delete(hw_suf_handle);
        AUI_TEST_CHECK_RTN(ret);
        AUI_PRINTF("%s %d I>BLIT_SUCCESS\n",__FUNCTION__,__LINE__);

    }
//EXIT:
    if(p_data) {
        free(p_data);
    }
    ret = aui_gfx_layer_close(layer_handle);
    AUI_TEST_CHECK_RTN(ret);
#ifdef AUI_LINUX
    ret = aui_gfx_de_init(NULL, NULL);
#endif
    AUI_TEST_CHECK_RTN(ret);


    return AUI_RTN_SUCCESS;
}

unsigned long test_osd_scale_precision(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long lyaer_id = AUI_OSD_LAYER_GMA0;
    struct aui_osd_rect src_rect;
    struct aui_osd_rect dst_rect;
    struct aui_osd_rect region_rect;
    aui_hdl hw_suf_handle = NULL;
    aui_hdl sw_suf_handle = NULL;
    aui_hdl layer_handle = NULL;
    aui_osd_pixel_format pixel_format = AUI_OSD_HD_ARGB8888;
    aui_blit_operation blit_op;
    aui_blit_rect blit_rect;
    unsigned long ret = AUI_RTN_SUCCESS;
    unsigned long mid_width = 300;
    unsigned long mid_height = 200;
    MEMSET(&src_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&dst_rect, 0, sizeof(struct aui_osd_rect));
    MEMSET(&region_rect, 0, sizeof(struct aui_osd_rect));
    
    ret = aui_gfx_layer_open(lyaer_id, &layer_handle);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("layer open fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    ret = aui_gfx_layer_show_on_off(layer_handle, 1);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("layer show fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    region_rect.uHeight = 576;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 720;

    MEMSET(&blit_op, 0X0, sizeof(aui_blit_operation));
    blit_op.rop_operation = AUI_GFX_ROP_DERECT_COVER;
    blit_op.alpha_blend_mode = AUI_GE_ALPHA_BLEND_SRC_OVER;

    // On linux, it always create double buffer. 
    // Because the directfb would create double buffer according to /etc/directfbrc.
    ret = aui_gfx_hw_surface_create(layer_handle, pixel_format, &region_rect, &hw_suf_handle, 0);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Create hardware surface fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
    aui_osd_scale(layer_handle, region_rect.uWidth, region_rect.uHeight);
    
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xffffffff, &region_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 0;
    src_rect.uTop = 0;
    src_rect.uWidth= 200;
    src_rect.uHeight= 200;
    ret = aui_gfx_sw_surface_create(pixel_format,src_rect.uWidth,src_rect.uHeight,&sw_suf_handle);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Create software surface fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }    

    dst_rect.uLeft = 0;
    dst_rect.uTop = 0;
    dst_rect.uWidth = 100;
    dst_rect.uHeight = 200;
    ret = aui_gfx_surface_fill(sw_suf_handle, 0xffff0000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill software surface left part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 0;
    dst_rect.uWidth = 100;
    dst_rect.uHeight = 200;
    ret = aui_gfx_surface_fill(sw_suf_handle, 0xff0000ff, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill software surface right part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    src_rect.uLeft = 150;
    src_rect.uTop = 0;
    src_rect.uWidth= 50;
    src_rect.uHeight= mid_height;

    dst_rect.uLeft = 50;
    dst_rect.uTop = 200;
    dst_rect.uWidth = 50;
    dst_rect.uHeight = mid_height;

    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit left rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 0;
    src_rect.uTop = 0;
    src_rect.uWidth= 50;
    src_rect.uHeight= mid_height;

    dst_rect.uLeft = 100 + mid_width;
    dst_rect.uTop = 200;
    dst_rect.uWidth = 50;
    dst_rect.uHeight = mid_height;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit right rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 150;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = 50;
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xff000000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface bottom part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 200 + mid_height;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = 50;
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xff000000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface bottom part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 95;
    src_rect.uTop = 0;
    src_rect.uWidth= 10;
    src_rect.uHeight= 7;

    dst_rect.uLeft = 100;
    dst_rect.uTop = 200;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = mid_height;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    AUI_PRINTF("(95,0,105,7)=>(100,200,400,400)!\n");

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit mid rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }
   
    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);

    AUI_SLEEP(5000);

    ret = aui_gfx_surface_fill(hw_suf_handle, 0xffffffff, &region_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Clear hardware surface fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    mid_width = 303;
    mid_height = 40;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    src_rect.uLeft = 150;
    src_rect.uTop = 0;
    src_rect.uWidth= 50;
    src_rect.uHeight= mid_height;

    dst_rect.uLeft = 50;
    dst_rect.uTop = 200;
    dst_rect.uWidth = 50;
    dst_rect.uHeight = mid_height;

    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit left rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 0;
    src_rect.uTop = 0;
    src_rect.uWidth= 50;
    src_rect.uHeight= mid_height;

    dst_rect.uLeft = 100 + mid_width;
    dst_rect.uTop = 200;
    dst_rect.uWidth = 50;
    dst_rect.uHeight = mid_height;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit right rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 150;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = 50;
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xff000000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface bottom part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 200 + mid_height;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = 50;
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xff000000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface bottom part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 95;
    src_rect.uTop = 0;
    src_rect.uWidth= 9;
    src_rect.uHeight= 200;

    dst_rect.uLeft = 100;
    dst_rect.uTop = 200;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = mid_height;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    AUI_PRINTF("(95,0,104,200)=>(100,200,403,240)!\n");

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit mid rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);

    AUI_SLEEP(5000);

    ret = aui_gfx_surface_fill(hw_suf_handle, 0xffffffff, &region_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Clear hardware surface fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    mid_width = 200;
    mid_height = 200;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    src_rect.uLeft = 150;
    src_rect.uTop = 0;
    src_rect.uWidth= 50;
    src_rect.uHeight= mid_height;

    dst_rect.uLeft = 50;
    dst_rect.uTop = 200;
    dst_rect.uWidth = 50;
    dst_rect.uHeight = mid_height;

    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit left rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 150;
    src_rect.uTop = 0;
    src_rect.uWidth= 50;
    src_rect.uHeight= mid_height;

    dst_rect.uLeft = 100 + mid_width;
    dst_rect.uTop = 200;
    dst_rect.uWidth = 50;
    dst_rect.uHeight = mid_height;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit right rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 150;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = 50;
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xff000000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface bottom part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    dst_rect.uLeft = 100;
    dst_rect.uTop = 200 + mid_height;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = 50;
    ret = aui_gfx_surface_fill(hw_suf_handle, 0xff000000, &dst_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Fill hardware surface bottom part fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    src_rect.uLeft = 0;
    src_rect.uTop = 0;
    src_rect.uWidth= 1;
    src_rect.uHeight= 20;

    dst_rect.uLeft = 100;
    dst_rect.uTop = 200;
    dst_rect.uWidth = mid_width;
    dst_rect.uHeight = mid_height;

    MEMSET(&blit_rect,0,sizeof(aui_blit_rect));
    MEMCPY(&blit_rect.dst_rect,&dst_rect,sizeof(struct aui_osd_rect));
    MEMCPY(&blit_rect.fg_rect,&src_rect,sizeof(struct aui_osd_rect));

    AUI_PRINTF("(0,0,1,20)=>(100,200,300,400)!\n");

    ret = aui_gfx_surface_blit(hw_suf_handle, sw_suf_handle, NULL, &blit_op, &blit_rect);
    if(ret!=AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("Blit mid rect fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);

    AUI_SLEEP(5000);
    

EXIT:
    if(sw_suf_handle != NULL)
    {
        aui_gfx_surface_delete(sw_suf_handle);
        sw_suf_handle = NULL;
    }
    if(hw_suf_handle != NULL)
    {
        aui_gfx_surface_delete(hw_suf_handle);
        hw_suf_handle = NULL;
    }
    aui_gfx_layer_close(layer_handle);

    return ret;
}

unsigned long test_osd_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nOSD Test Help");  

	/* OSD_1_HELP */
	#define 	OSD_1_HELP		"The horizontal line is drawn and then the vertical line is drawn in the screen.\n"
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Test the line drawing function of OSD\n");
	aui_print_help_instruction_newline(OSD_1_HELP);

	/* OSD_2_HELP */
	#define 	OSD_2_HELP 	"The rect is filled write and the rect's out line is drawn green in the screen.\n"
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("Test the out line and the rect drawing function of OSD\n");
	aui_print_help_instruction_newline(OSD_2_HELP);
	
	/* OSD_3_HELP */
	#define 	OSD_3_HELP_PART1 	"The test is following the below steps: show the original image -> show the image inside -> show the image outside -> test 4 times -> test end.\n"
	#define		OSD_3_HELP_PART2	"Clipping: Any procedure which identifies that portion of a picture which is either inside or outside a region is referred to as clipping.\n"
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline("Test the clipping function of drawing.\n");
	aui_print_help_instruction_newline(OSD_3_HELP_PART1);
	aui_print_help_instruction_newline(OSD_3_HELP_PART2);

	/* OSD_4_HELP */
	#define 	OSD_4_HELP_PART1	"The test is following the below steps: show the original image -> test blit image -> test 4 times -> test end.\n"
	#define 	OSD_4_HELP_PART2	"Blit: several bitmaps are combined into one using a raster operator. \n"
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline("Test the blit function of drawing.\n");
	aui_print_help_instruction_newline(OSD_4_HELP_PART1);
	aui_print_help_instruction_newline(OSD_4_HELP_PART2);

	
	/* OSD_5_HELP */
	#define 	OSD_5_HELP_PART1	"The test is following the below steps:  show the green rect"
	#define 	OSD_5_HELP_PART2	"-> show the blue rect -> show the white rect -> show the red rect.\n"

	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline("Test the surface blit function of drawing.\n");
	aui_print_help_instruction_newline(OSD_5_HELP_PART1);
	aui_print_help_instruction_newline(OSD_5_HELP_PART2);

	
	/* OSD_6_HELP */
	#define 	OSD_6_HELP_PART1	"The test is following the below steps (using software surface drawing): " 
	#define 	OSD_6_HELP_PART2	"show the green strip -> show the blue strip below green -> show the white strip below blue -> show the red strip below white"
	#define 	OSD_6_HELP_PART3	"-> hide the green strip -> hide the blue strip -> hide the white strip -> hide the red strip.\n"
	#define 	OSD_6_HELP_PART4	"The software drawing is designed relative to the hardware drawing. It is redescribed, using the software memory, on the basis of the hardware drawing."
	#define 	OSD_6_HELP_PART5	"The software drawing can draw any size of image as long as the software memory is large enough.\n"

	aui_print_help_command("\'6\'");
	aui_print_help_instruction_newline("Test the software surface drawing function.\n");
	aui_print_help_instruction_newline(OSD_6_HELP_PART1);
	aui_print_help_instruction_newline(OSD_6_HELP_PART2);
	aui_print_help_instruction_newline(OSD_6_HELP_PART3);
	aui_print_help_instruction_newline(OSD_6_HELP_PART4);
	aui_print_help_instruction_newline(OSD_6_HELP_PART5);

	/* OSD_7_HELP */
    #define     OSD_7_HELP		"Test the color key drawing function.\n"
    #define     OSD_7_HELP_PART1	"show white strip ->show blue strip below white -> show green strip below blue"
	aui_print_help_command("\'7\'");
	aui_print_help_instruction_newline(OSD_7_HELP);
    aui_print_help_instruction_newline(OSD_7_HELP_PART1);

	/* OSD_8_HELP */
    #define     OSD_8_HELP_PART1	"The test is following the below steps (using software surface drawing): "
    #define     OSD_8_HELP_PART2	"show the green strip-> show the blue strip below green -> show the white strip below blue -> show the orange strip below white"
    #define     OSD_8_HELP_PART3	"-> hide the green strip -> hide the blue strip -> hide the white strip -> hide the orange strip.\n"
    #define     OSD_8_HELP_PART4	"-> show the green strip-> show the blue strip below green -> show the white strip below blue -> show the orange strip below white"
    #define     OSD_8_HELP_PART5	"->hide the all strip->show the all strip"
    #define     OSD_8_HELP_PART6	"The difference between the hardware drawing and the software drawing can be found in the \"6\" command help.\n"

	aui_print_help_command("\'8\'");
	aui_print_help_instruction_newline("Test the hardware surface drawing function.\n");
	aui_print_help_instruction_newline(OSD_8_HELP_PART1);
	aui_print_help_instruction_newline(OSD_8_HELP_PART2);
	aui_print_help_instruction_newline(OSD_8_HELP_PART3);
	aui_print_help_instruction_newline(OSD_8_HELP_PART4);
	aui_print_help_instruction_newline(OSD_8_HELP_PART5);
	aui_print_help_instruction_newline(OSD_8_HELP_PART6);

	
	/* OSD_9_HELP */
	#define 	OSD_9_HELP_PART1	"The two image is drawing with apha blending blit.\n" 
	#define 	OSD_9_HELP_PART2	"The test is following the below steps:  show the blue rect -> show the image on the blue rect -> hide all-> test 2 times -> test end.\n"
	#define 	OSD_9_HELP_PART3	"Apha blending: Alpha blending is the process of combining an image with a background to create the appearance of partial or full transparency.\n"

	aui_print_help_command("\'9\'");
	aui_print_help_instruction_newline("Test apha blending blit drawing function.\n");
	aui_print_help_instruction_newline(OSD_9_HELP_PART1);
	aui_print_help_instruction_newline(OSD_9_HELP_PART2);
	aui_print_help_instruction_newline(OSD_9_HELP_PART3);

	/* OSD_10_HELP */
    #define     OSD_10_HELP	"Test scale blit drawing function.\n"
    #define     OSD_10_HELP_PART1	"This test include 5 diffrect color mode, each color mode test is following the below steps:\n"
    #define     OSD_10_HELP_PART2	" scale blit -> clear screen -> capture surface & blit-> test 4 times -> test end.\n"


	aui_print_help_command("\'10'");
	aui_print_help_instruction_newline(OSD_10_HELP);
    aui_print_help_instruction_newline(OSD_10_HELP_PART1);
	aui_print_help_instruction_newline(OSD_10_HELP_PART2);

	
	/* OSD_11_HELP */
	#define 	OSD_11_HELP	"Test split scale blit drawing function.\n" 
    #define 	OSD_11_HELP_PART1	"This test would split the soure raw data to two part .\n" 
    #define 	OSD_11_HELP_PART2	"Part 1 do scale & blit to (0, 0) - 720 * 288.\n" 
    #define 	OSD_11_HELP_PART3	"Part 2 do scale & blit to (0, 288) - 720 * 288..\n" 

	aui_print_help_command("\'11'");
	aui_print_help_instruction_newline(OSD_11_HELP);
    aui_print_help_instruction_newline(OSD_11_HELP_PART1);
    aui_print_help_instruction_newline(OSD_11_HELP_PART2);
    aui_print_help_instruction_newline(OSD_11_HELP_PART3);

    /* OSD_12_HELP */
	#define 	OSD_12_HELP	"Test drawing on two layer.\n" 
    #define 	OSD_12_HELP_PART1	"This test would draw on fb0 and fb2 . The test flow is:\n" 
    #define 	OSD_12_HELP_PART2	"Step1. fill rectangle on fb0 (0, 0) 720 * 576 with color 0x4f00.\n" 
    #define 	OSD_12_HELP_PART3	"Step2. fill rectangle on fb2 (0, 0) 150 * 50 with color 0xffff and (0,0) 40 * 50 with color 0xf00f .\n"
	#define 	OSD_12_HELP_PART4	"blit rect[0xf00f, (0, 55), 40 * 50] on fb2.\n"
	#define 	OSD_12_HELP_PART5	"Step3. turn off/on fb0.\n"
    #define 	OSD_12_HELP_PART6	"Step4. turn off/on fb2.\n"


	aui_print_help_command("\'12'");
	aui_print_help_instruction_newline(OSD_12_HELP);
    aui_print_help_instruction_newline(OSD_12_HELP_PART1);
    aui_print_help_instruction_newline(OSD_12_HELP_PART2);
    aui_print_help_instruction_newline(OSD_12_HELP_PART3);
    aui_print_help_instruction_newline(OSD_12_HELP_PART4);
    aui_print_help_instruction_newline(OSD_12_HELP_PART5);
    aui_print_help_instruction_newline(OSD_12_HELP_PART6);

#ifdef AUI_LINUX
    /* OSD_13_HELP */
    #define 	OSD_13_HELP	"Test render image on OSD, only for LINUX.\n" 
    #define 	OSD_13_HELP_PART1	"This test would Load the image from the specified path set by user.\n" 
    #define 	OSD_13_HELP_PART2	"Usage: 13 image_path,[width],[height],[auto_clean]\n default width: 1280, height: 720\n"
    #define 	OSD_13_HELP_PART3	"'auto_clean = 1' mean after test, image will hide.\n"
    #define 	OSD_13_HELP_PART4	"And show the image on the screen.\n"


    aui_print_help_command("\'13'");
    aui_print_help_instruction_newline(OSD_13_HELP);
    aui_print_help_instruction_newline(OSD_13_HELP_PART1);
    aui_print_help_instruction_newline(OSD_13_HELP_PART2);
    aui_print_help_instruction_newline(OSD_13_HELP_PART3);
    aui_print_help_instruction_newline(OSD_13_HELP_PART4);

    /* OSD_14_HELP */
    #define 	OSD_14_HELP	"Run/Kill directfb performance test, only for LINUX.\n" 
    #define 	OSD_14_HELP_PART1	"14 1: run directfb performance test app.\n" 
    #define 	OSD_14_HELP_PART2	"14 0: kill directfb performance test app.\n" 

    aui_print_help_command("\'14'");
    aui_print_help_instruction_newline(OSD_14_HELP);
    aui_print_help_instruction_newline(OSD_14_HELP_PART1);
    aui_print_help_instruction_newline(OSD_14_HELP_PART2);
#endif //#ifdef AUI_LINUX
	
	return AUI_RTN_HELP;
}


void osd_test_reg(void)
{
	aui_tu_reg_group("osd", "gfx tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_osd_draw_line, "test_osd_draw_line");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_osd_draw_out_line, "test_osd_draw_out_line");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_osd_clip_mode, "test_osd_clip_mode");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_osd_data_blit, "test_osd_data_blit");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_osd_surface_blit, "test_osd_surface_blit");
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_osd_sw_surface_test, "test_osd_sw_surface_test");
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_osd_color_key, "test_osd_color_key");
	aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, test_osd_hw_surface_test, "test_osd_hw_surface_test");
	aui_tu_reg_item(2, "9", AUI_CMD_TYPE_API, test_osd_aphablending_blit, "test_osd_aphablending_blit");
	aui_tu_reg_item(2, "10", AUI_CMD_TYPE_API, test_osd_scale_blit, "test_osd_scale_blit");
	aui_tu_reg_item(2, "11", AUI_CMD_TYPE_API, test_osd_split_scale_blit, "test_osd_split_scale_blit");
    aui_tu_reg_item(2, "12", AUI_CMD_TYPE_API, test_osd_two_layer, "test_osd_two_layer");
#ifdef AUI_LINUX
    aui_tu_reg_item(2, "13", AUI_CMD_TYPE_API, test_osd_show_image, "test_osd_show_image");
    aui_tu_reg_item(2, "14", AUI_CMD_TYPE_API, test_osd_run_performace_test, "test_osd_run_performace_test");
#endif
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_osd_help, "osd test help");
	aui_tu_reg_item(2, "note", AUI_CMD_TYPE_API, test_osd_note, "please increase the size of OSD buffer, otherwise it causes exceptions.");
}

