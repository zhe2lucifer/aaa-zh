
/****************************INCLUDE HEAD FILE************************************/
#include "aui_dis_test.h"
#include "aui_test_app.h"
#include "aui_decv_test.h"
#include "aui_help_print.h"
#include <aui_dis.h>
#include <aui_osd.h>

#ifndef  DUAL_VIDEO_OUTPUT_USE_VCAP
#define DUAL_VIDEO_OUTPUT_USE_VCAP
#endif

typedef enum aui_dis_video_formt {

    /**
	Value to specify the <b> 576I PAL </b> video display formats 
    */
    AUI_DIS_SD_576I_PAL = 0,
    
	/**
	Value to specify the <b> 576I  PAL_N</b> video display formats
	*/
	AUI_DIS_SD_576I_PAL_N,

	/**
	Value to specify the <b> 576I  PAL_NC</b> video display formats 
	*/
	AUI_DIS_SD_576I_PAL_NC,

	/**
	Value to specify the <b> 480I  PAL_M</b> video display formats 
	*/
	AUI_DIS_SD_480I_PAL_M,

	/**
	Value to specify the <b> 480I  PAL_60</b> video display formats
	*/
	AUI_DIS_SD_480I_PAL_60,
    
	/**
	Value to specify the <b> 480I  NTSC</b> video display formats 
	*/
	AUI_DIS_SD_480I_NTSC,

	/**
	Value to specify the <b> 480I  NTSC_443</b> video display formats
	*/
	AUI_DIS_SD_480I_NTSC_443,

    /**
	Value to specify the <b> 576P </b> video display formats 
    */
    AUI_DIS_HD_576P,

	/**
	Value to specify the <b> 480P </b> video display formats 
	*/
	AUI_DIS_HD_480P,

    /**
    Value to specify the <b> 720P </b> video display formats 

    @note   ALi chipsets don't support the video display format <b> 720I50 </b>
    */
    AUI_DIS_HD_720P50,

    /**
	Value to specify the <b> 720P </b> video display formats 

    @note   ALi chipsets don't support the video display format <b> 720I60 </b>
    */
    AUI_DIS_HD_720P60,

    /**
	Value to specify the <b> 1080I50 </b> video display formats
    */
    AUI_DIS_HD_1080I50,
	
	/**
	Value to specify the <b> 1080I60 </b> video display formats 
	*/
	AUI_DIS_HD_1080I60,

    /**
	Value to specify the <b> 1080P25 </b> video display formats
    */
    AUI_DIS_HD_1080P25,
    
	/**
	Value to specify the <b> 1080P30 </b> video display formats 
	*/
	AUI_DIS_HD_1080P30,

    /**
    	Value to specify the <b> 1080P24 </b> video display formats
    */
    AUI_DIS_HD_1080P24,

	/**
	Value to specify the <b> 1080P50 </b> video display formats 
    */
    AUI_DIS_HD_1080P50,

    /**
	Value to specify the <b> 1080P60 </b> video display formats 
    */
    AUI_DIS_HD_1080P60,
    
	/**
	3840x2160@24Hz 
	*/
	AUI_DIS_HD_4096X2160P24,
		
	/**
	4096x2160@24Hz
	*/
	AUI_DIS_HD_3840X2160P24,
		
	/**
	3840x2160@25Hz,
	*/
	AUI_DIS_HD_3840X2160P25,
		
	/**
	3840x2160@30Hz,
	*/
	AUI_DIS_HD_3840X2160P30,

	/**
	3840x2160@50Hz 
	*/
	AUI_DIS_HD_3840X2160P50,
	
	/**
	3840x2160@60Hz 
	*/		
	AUI_DIS_HD_3840X2160P60,
		
	/**
	4096x2160@25Hz 
	*/		
	AUI_DIS_HD_4096X2160P25,
		
	/**
	4096x2160@30Hz 
	*/		
	AUI_DIS_HD_4096X2160P30,

	/**
	4096x2160@50Hz 
	*/		
	AUI_DIS_HD_4096X2160P50,
		
	/**
	4096x2160@60Hz 
	*/			
	AUI_DIS_HD_4096X2160P60

} aui_dis_video_formt;


static void *g_dis_handle;
static void *g_dis_handle_sd;
static aui_hdl hw_suf_handle;
static aui_hdl g_layer_handle;
static unsigned short image_height;
static unsigned short image_width;
static int g_dis_disable_video = 0;

unsigned long test_dis_osd_draw_rect(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_osd_rect region_rect;
	struct aui_osd_rect out_line_rect;

	enum aui_osd_pixel_format e_color_mode = AUI_OSD_HD_ARGB8888;
	unsigned long fill_color = 0xFF00FF00;
	unsigned long back_color = 0xFFFFFFFF;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	 
	if (0 == *argc) {
		AUI_PRINTF("%s -> you can input :\n",__FUNCTION__);
		AUI_PRINTF("9 [ssize]\n");
		return 0;
	}  
	switch (atoi(argv[0])) {
		case 0: {
			image_height = 576;
			image_width = 720;
			break;
		}
		case 1: {
			image_height = 720;
			image_width = 1280;
			break;
		}
		case 2: {
			image_height = 1080;
			image_width = 1920;
			break;
		}
		case 3: {
			image_height = 2160;
			image_width = 3840;
			break;
		}
		case 4: {
			image_height = 2160;
			image_width = 4096;
			break;
		}
		default:
			AUI_PRINTF("%s: invalid ssize\n", __func__);
			return -1;
	}
	
    memset(&region_rect, 0, sizeof(struct aui_osd_rect));
    memset(&out_line_rect, 0, sizeof(struct aui_osd_rect));
    region_rect.uHeight = image_height;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = image_width;

    out_line_rect.uHeight = 288;
    out_line_rect.uLeft = 180;
    out_line_rect.uTop = 144;
    out_line_rect.uWidth = 360;

	ret = aui_gfx_layer_show_on_off(g_layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);

	//create hw/sw surface
	if (hw_suf_handle != NULL) {
        aui_gfx_surface_delete(hw_suf_handle);
		hw_suf_handle = NULL;
		ret = aui_gfx_hw_surface_create(g_layer_handle, e_color_mode, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
		AUI_TEST_CHECK_RTN(ret);   

		ret = aui_gfx_surface_fill(hw_suf_handle, 0, &region_rect);
		AUI_TEST_CHECK_RTN(ret);

    	aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
    	AUI_TEST_CHECK_RTN(ret);
    	ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);

    	aui_gfx_layer_alpha_set(g_layer_handle,255);
	}
	return 0;
}

unsigned long set_dis_osd_alpha(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int alpha = 0;
    /*assert arguement*/   
	if (NULL == g_layer_handle) {
		AUI_PRINTF("Please make sure g_layer_handle is open ");
		return 0;
	}  

	if (0 == *argc) {
		AUI_PRINTF("%s -> you can input :\n",__FUNCTION__);
		AUI_PRINTF("9 [alpha]\n");
		return 0;
	} 

	alpha = atoi(argv[0]);
	if (alpha < 256) {
    	aui_gfx_layer_alpha_set(g_layer_handle,alpha);
	} else {
		AUI_PRINTF("alpha range is 0~255\n");
		return 0;
	}
	
	return 0;
}

unsigned long test_dis_open(unsigned long *argc,char **argv,char *sz_out_put)
{	
	struct aui_osd_rect region_rect;
	struct aui_osd_rect out_line_rect;
	unsigned long layer_id = AUI_OSD_LAYER_GMA0;
	enum aui_osd_pixel_format e_color_mode = AUI_OSD_HD_ARGB8888;
	aui_hdl layer_handle = NULL;
    aui_attr_dis attr_dis;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if 0
   	unsigned long fill_color = 0xFF00FF00;
	unsigned long back_color = 0xFFFFFFFF;
#endif
	
	MEMSET(&attr_dis, 0 ,sizeof(aui_attr_dis));
	
	attr_dis.uc_dev_idx = AUI_DIS_HD;
	
	if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_HD, &g_dis_handle)){
    	if (aui_dis_open(&attr_dis, &g_dis_handle)){
    		AUI_PRINTF("\n aui_dis_open HD fail\n");
    		return -1;
    	}
    }

	attr_dis.uc_dev_idx = AUI_DIS_SD;
	if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_SD, &g_dis_handle_sd)){
    	if (aui_dis_open(&attr_dis, &g_dis_handle_sd)){
    		AUI_PRINTF("\n aui_dis_open SD fail\n");
    		return -1;
    	}
    }
#ifndef _AUI_OTA_
    test_decv_showlogo(NULL,NULL,NULL);
#endif

    memset(&region_rect, 0, sizeof(struct aui_osd_rect));
    memset(&out_line_rect, 0, sizeof(struct aui_osd_rect));
    region_rect.uHeight = 720;
    region_rect.uLeft = 0;
    region_rect.uTop = 0;
    region_rect.uWidth = 1280;

    out_line_rect.uHeight = 288;
    out_line_rect.uLeft = 180;
    out_line_rect.uTop = 144;
    out_line_rect.uWidth = 360;

#ifdef AUI_LINUX
	aui_gfx_init(NULL, NULL);
#endif

	AUI_TEST_CHECK_RTN( aui_gfx_layer_open(layer_id, (aui_hdl*)(&layer_handle)));
    g_layer_handle = layer_handle;

	ret = aui_gfx_layer_show_on_off(g_layer_handle, 1);
	AUI_TEST_CHECK_RTN(ret);
	
	//create hw/sw surface
	if (NULL == hw_suf_handle) {
		ret = aui_gfx_hw_surface_create(g_layer_handle, e_color_mode, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
		AUI_TEST_CHECK_RTN(ret);
	}
	ret = aui_gfx_surface_fill(hw_suf_handle, 0, &region_rect);
	AUI_TEST_CHECK_RTN(ret);
	
#if 0
	aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
	AUI_TEST_CHECK_RTN(ret);
#endif

	ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
	
	aui_gfx_layer_alpha_set(g_layer_handle,255);
	image_width = 1280;
	image_height = 720;

    AUI_SLEEP(2000);
    return AUI_RTN_SUCCESS;
}

unsigned long test_dis_HDTVencoder_output_CVBS(unsigned long *argc,char **argv,char *sz_out_put)
{
    /* before checkouting TV system,aui_dis_dac_unreg must be called*/
    AUI_TEST_CHECK_RTN(aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_YUV));
    AUI_TEST_CHECK_RTN(aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_RGB));
    AUI_TEST_CHECK_RTN(aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_SVIDEO));
    AUI_TEST_CHECK_RTN(aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_CVBS));
    AUI_TEST_CHECK_RTN(aui_dis_dac_unreg(g_dis_handle_sd, AUI_DIS_TYPE_UNREG_CVBS));
    AUI_PRINTF("unreg DAC success\n");

    AUI_PRINTF("change tvsys\n");
    /*HD TV encoder and SD TV encoder must be the same TV system, both of them the same
     * as NTSC or both of them the same as PAL.
     */
    AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_30, 0));
    //AUI_SLEEP(3000);
    AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));

    unsigned int tve_src = AUI_DIS_SD;
    AUI_TEST_CHECK_RTN(aui_dis_set(g_dis_handle, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
    /** samples here **/
    unsigned int cvbs_dac[4];
    cvbs_dac[0] = AUI_DIS_TYPE_V;
    cvbs_dac[1] = AUI_DIS_TYPE_U;
    cvbs_dac[2] = AUI_DIS_TYPE_Y;
    cvbs_dac[3] = 0xFF;
    aui_dis_dac_reg(g_dis_handle, cvbs_dac, 4);

    cvbs_dac[0] = 0xFF;   
    cvbs_dac[1] = 0xFF;
    cvbs_dac[2] = 0xFF;
    cvbs_dac[3] = AUI_DIS_TYPE_CVBS;   //normal CVBS output
    AUI_TEST_CHECK_RTN(aui_dis_dac_reg(g_dis_handle, cvbs_dac, 4));

    return 0;
}

unsigned long test_dis_mode(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_dis_zoom_rect src;
    aui_dis_zoom_rect dst;
    struct aui_osd_rect region_rect;
    struct aui_osd_rect out_line_rect;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned int dst_startX = 0;
    unsigned int dst_startY = 0;
    unsigned int dst_width = 0;
    unsigned int dst_height = 0;  
    enum aui_osd_pixel_format e_color_mode = AUI_OSD_HD_ARGB8888;
    unsigned long back_color = 0xFF00FF00;
    unsigned long fill_color = 0xFF00FF00;
    aui_hdl decv_hdl = NULL;
    unsigned int rect_switch_done = 0;
    
    if (*argc != 4) {
        AUI_PRINTF("set mode: \n");
        AUI_PRINTF("2 [startX][startY][width][height]\n");
        AUI_PRINTF("\nnote :Full screen 1280(width)*720(height)\n");
        AUI_PRINTF("Example    : \n"); 
        AUI_PRINTF("full screen(width:1280,height:720)           : 2 0,0,1280,720\n");
        AUI_PRINTF("nearly full screen(width:1200,height:680)    : 2 40,20,1200,680\n");
        AUI_PRINTF("1/4  screen(width/2,height/2)                 : 2 0,0,640,360\n");  
        AUI_PRINTF("1/8  screen(width/4,height/2)                 : 2 0,0,320,360\n"); 
        AUI_PRINTF("1/8  screen(width/2,height/4)                 : 2 0,0,640,180\n"); 
        AUI_PRINTF("1/16 screen(width/4,height/4)                 : 2 0,0,320,180\n"); 
        AUI_SLEEP(100);
        return AUI_RTN_FAIL;
    }
    
    /*assert arguement*/
    if ( (NULL == g_dis_handle) || (NULL == g_dis_handle_sd)){
        AUI_PRINTF("please ensure dis open\n");
        return AUI_RTN_FAIL;
    }

    dst_startX = ATOI(argv[0]);
    dst_startY = ATOI(argv[1]);
    dst_width = ATOI(argv[2]);
    dst_height= ATOI(argv[3]);
    
    if ((dst_startX + dst_width) > 1280 || (dst_startY + dst_height) > 720){
        AUI_PRINTF("invaild value !\n");
        return AUI_RTN_FAIL;
    }
    
    if ((dst_width == 1280) && (dst_height == 720)) {
        AUI_PRINTF("fullview mode \n");     
        AUI_TEST_CHECK_RTN(aui_dis_mode_set(g_dis_handle, AUI_VIEW_MODE_FULL, NULL,NULL));        
    }else{
        AUI_PRINTF("preview mode \n");       

        src.ui_startX = 0;
        src.ui_startY = 0;
        src.ui_width = 720;
        src.ui_height = 2880;
        //convert 1280*720 -->720*2880
        dst.ui_startX = (unsigned int)(dst_startX*720/1280);
        dst.ui_startY = (unsigned int)(dst_startY*2880/720);
        dst.ui_width =  (unsigned int)(dst_width*720/1280);
        dst.ui_height = (unsigned int)(dst_height*2880/720); 
        AUI_TEST_CHECK_RTN(aui_dis_mode_set(g_dis_handle, AUI_VIEW_MODE_PREVIEW, &src, &dst));      
    }
    
    if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
		AUI_PRINTF("video is not playing, please check display position after playing!\n");
	} else {
	    aui_decv_info decv_info;
	    memset(&decv_info, 0, sizeof(decv_info));
	    aui_decv_get(decv_hdl, AUI_DECV_GET_INFO, &decv_info);	   
	    rect_switch_done = decv_info.st_info.rect_switch_done;
	    
	    if (!decv_info.st_info.rect_switch_done) {
	        if (decv_info.st_info.u_cur_status != AUI_DECV_DECODING) {
	            AUI_PRINTF("decv status: %d\n", decv_info.st_info.u_cur_status);
    	        AUI_PRINTF("video decoder is not decoding, please check display position after playing\n");
    	    } else { 
    	        char ch=0;
                AUI_PRINTF("there may be no data for video to decoder!\n");  
                AUI_PRINTF("wait to get more video data, enter 'q' to quit waiting.\n");
                
                while (1) {
                    if (AUI_RTN_SUCCESS == aui_test_get_user_key_input(&ch)){
                		if (ch == 'q') {
                			break;
                		}
                	}
                	
                    usleep(500*1000);                    
                    aui_decv_get(decv_hdl, AUI_DECV_GET_INFO, &decv_info);
                    rect_switch_done = decv_info.st_info.rect_switch_done;
                    
                    if (decv_info.st_info.rect_switch_done) {
                        AUI_PRINTF("switch done!\n");
                        break;
                    } 
                }
                
                if (!rect_switch_done) {
                    AUI_PRINTF("wait video data fail, please check display position after video resuming!");
                    AUI_PRINTF("switch mode failed, close video display\n");
                    //off video layer when swich failed during playing
                    aui_dis_video_enable(g_dis_handle, 0);
                    g_dis_disable_video = 1;
                    return -1;
                }
            }
	    }
	}
    
	if (rect_switch_done) {
	    if (g_dis_disable_video) {
	        //on video layer after swiching OK
	        aui_dis_video_enable(g_dis_handle, 1);
	        g_dis_disable_video = 0;
	    }
	    if ((dst_width == 1280) && (dst_height == 720)) {
	        AUI_PRINTF("hide osd \n");
            aui_gfx_layer_show_on_off(g_layer_handle, 0); 
	    } else {
            AUI_PRINTF("create osd 1080*1920 and draw outline \n");
            memset(&region_rect, 0, sizeof(struct aui_osd_rect));
            memset(&out_line_rect, 0, sizeof(struct aui_osd_rect));
            
            region_rect.uLeft = 0;
            region_rect.uTop = 0;
            region_rect.uWidth = 1280;
            region_rect.uHeight = 720;
            
            ret = aui_gfx_layer_show_on_off(g_layer_handle, 1);
            AUI_TEST_CHECK_RTN(ret);
            //create hw/sw surface
            if (hw_suf_handle != NULL) {
                
                aui_gfx_surface_delete(hw_suf_handle);
                hw_suf_handle = NULL;
                ret = aui_gfx_hw_surface_create(g_layer_handle, e_color_mode, &region_rect, (aui_hdl*)&hw_suf_handle, 0);
                AUI_TEST_CHECK_RTN(ret);   
            
                ret = aui_gfx_surface_fill(hw_suf_handle, 0, &region_rect);
                AUI_TEST_CHECK_RTN(ret);
            
                //OSD Width(1280)*Height(720) 
                /*
                        -------------------------------------
                        -          -  osd top -             -        
                        -          ------------             -                       
                        -   osd    -   video  -             -             
                        -   left   -  preview -  osd right  -                                       
                        -          -          -             -         
                        -          ------------             -                        
                        -          -osd bottom-             -                                                         
                        -------------------------------------
                */
                //draw top field
                out_line_rect.uLeft = dst_startX;
                out_line_rect.uTop = 0;
                out_line_rect.uWidth = dst_width;
                out_line_rect.uHeight = dst_startY;
                aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
            
                //draw botom field
                out_line_rect.uLeft = dst_startX;
                out_line_rect.uTop = (dst_startY + dst_height);
                out_line_rect.uWidth = dst_width;
                out_line_rect.uHeight = 720 - (dst_startY + dst_height); 
                aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
            
                //draw left field
                out_line_rect.uLeft = 0;
                out_line_rect.uTop = 0;
                out_line_rect.uWidth = dst_startX;
                out_line_rect.uHeight = 720;
                aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
            
                //draw right field
                out_line_rect.uLeft =(dst_startX+dst_width);
                out_line_rect.uTop = 0;
                out_line_rect.uWidth = 1280 -(dst_startX+dst_width);
                out_line_rect.uHeight = 720;
                aui_gfx_surface_draw_outline(hw_suf_handle, fill_color, back_color, &out_line_rect, 1);
            
                AUI_TEST_CHECK_RTN(ret);
                ret = aui_gfx_surface_flush(hw_suf_handle, &region_rect);
            
                aui_gfx_layer_alpha_set(g_layer_handle,255);
                
            }
	    }
	} 
	
    return ret;
}

unsigned long test_enhance_set(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_dis_enhance_set(g_dis_handle,AUI_DIS_ENHANCE_BRIGHTNESS, 90));

    return AUI_RTN_SUCCESS;
}

unsigned long test_zoom(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_dis_zoom_rect src_rect;
    aui_dis_zoom_rect dst_rect;
    
    /** scale the whole logo to left-top of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 720;
    src_rect.ui_height = 2880;

    dst_rect.ui_startX = 0;
    dst_rect.ui_startY = 0;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));

    AUI_SLEEP(2000);
    
    /** scale the whole logo to right-top of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 720;
    src_rect.ui_height = 2880;

    dst_rect.ui_startX = 360;
    dst_rect.ui_startY = 0;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));

    AUI_SLEEP(2000);

    /** scale the whole logo to left-bottom of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 720;
    src_rect.ui_height = 2880;

    dst_rect.ui_startX = 0;
    dst_rect.ui_startY = 1440;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));

    AUI_SLEEP(2000);

    /** scale the whole logo to right-bottom of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 720;
    src_rect.ui_height = 2880;

    dst_rect.ui_startX = 360;
    dst_rect.ui_startY = 1440;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));

    AUI_SLEEP(2000);

    /** cropping the whole the logo to the center of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 720;
    src_rect.ui_height = 2880;

    dst_rect.ui_startX = 180;
    dst_rect.ui_startY = 720;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));

    AUI_SLEEP(2000);

    /** cropping the top-left of the logo to the center of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 360;
    src_rect.ui_height = 1440;

    dst_rect.ui_startX = 180;
    dst_rect.ui_startY = 720;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));

    /** recover the whole logo to full screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 720;
    src_rect.ui_height = 2880;

    dst_rect.ui_startX = 0;
    dst_rect.ui_startY = 0;
    dst_rect.ui_width = 720;
    dst_rect.ui_height = 2880;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));
    return AUI_RTN_SUCCESS;
}

unsigned long test_video_enable(unsigned long *argc,char **argv,char *sz_out_put)
{
    /** the logo will disappear,the OSD layer will remain **/
    AUI_TEST_CHECK_RTN(aui_dis_video_enable(g_dis_handle, 0));
    AUI_PRINTF("turn off\n");
    AUI_SLEEP(5000);

    /** recover the logo **/
    AUI_TEST_CHECK_RTN(aui_dis_video_enable(g_dis_handle, 1));
    return AUI_RTN_SUCCESS;
}

static void aspect_ratio_show_usage()
{
    AUI_PRINTF("If you want to changed the default configure,as fallow\n");
    AUI_PRINTF("\n");
    
    AUI_PRINTF("Arguments for aspect_ratio&match_mode:\n");
    AUI_PRINTF("match [aspect_ratio],[match_mode]\n");
    AUI_PRINTF("Value to specify the display aspect ratio by aspect_ratio\n");
    AUI_PRINTF("aspect_ratio = 0 : 16_9\n");
    AUI_PRINTF("               1 : 4_3\n");
    AUI_PRINTF("               2 : AUTO\n");
    AUI_PRINTF("match_mode =   0 : AUI_DIS_MM_PANSCAN(PS)\n");
    AUI_PRINTF("               1 : AUI_DIS_MM_LETTERBOX(LB)\n");
    AUI_PRINTF("               2 : AUI_DIS_MM_PILLBOX\n");
    AUI_PRINTF("               3 : AUI_DIS_MM_VERTICALCUT\n");
    AUI_PRINTF("               4 : AUI_DIS_MM_NORMAL_SCALE\n");
    AUI_PRINTF("               5 : AUI_DIS_MM_COMBINED_SCALE\n");
    AUI_PRINTF("\n");

    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

static void afd_show_usage()
{
    AUI_PRINTF("If you want to changed the default configure,as fallow\n");
    AUI_PRINTF("\n");  
    
    AUI_PRINTF("Arguments for AFD:\n");
    
    AUI_PRINTF("afd [afd_mode],[afd_spec],[afd_protect_enable]\n");
    AUI_PRINTF("afd_mode = 0 : AFD is implemented by the co-work of DIS and TV set\n");    
    AUI_PRINTF("           1 : AFD is implemented by DIS scaling or clipping\n");
    AUI_PRINTF("           2 : AFD is disabled\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("afd_spec = 0 : DTG\n");
    AUI_PRINTF("           1 : MinDig\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("afd_protect_enable = 0 : AFD protect mode disabled\n");
    AUI_PRINTF("                     1 : AFD protect mode enabled\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("such as HW afd,input: afd 0,1,0\n");
    AUI_PRINTF("        SW afd,input: afd 1,1,0\n");
    AUI_PRINTF("        normal match mode,input: afd 2,1,0\n");
    AUI_PRINTF("\n");

    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

AUI_RTN_CODE set_aspect_ratio_and_match_mode(unsigned long *argc,char **argv,char *sz_out_put)
{   int match_mode=0,en_asp_ratio=0;

    /*assert arguement*/
    if (NULL == g_dis_handle){
        AUI_PRINTF("please ensure dis open\n");
        return 0;
    }
    
	if (0 == *argc || 1 == *argc) {
		aspect_ratio_show_usage();
		return 0;
	}  
	
    /* extract para from argv*/
    if (*argc > 1) {     
        switch (atoi(argv[0])){
            case 0:
                en_asp_ratio = AUI_DIS_AP_16_9;
                break;
				
            case 1:
                en_asp_ratio = AUI_DIS_AP_4_3;
                break;
				
            case 2:
                en_asp_ratio = AUI_DIS_AP_AUTO;
                break;
				
            default:
                AUI_PRINTF("invalid aspect ratio,not have this aspet ratio:%d!\n",atoi(argv[0]));               				
                return AUI_RTN_EINVAL;
                break;
        }
		
        switch (atoi(argv[1])){
            case 0:
                match_mode = AUI_DIS_MM_PANSCAN;
                break;
				
            case 1:
                match_mode = AUI_DIS_MM_LETTERBOX;
                break;
				
            case 2:
                match_mode = AUI_DIS_MM_PILLBOX;
                break;
				
            case 3:
                match_mode = AUI_DIS_MM_VERTICALCUT;
                break;
				
            case 4:
                match_mode = AUI_DIS_MM_NORMAL_SCALE;
                break;
				
            case 5:
                match_mode = AUI_DIS_MM_COMBINED_SCALE;
                break;
				
            default:
				AUI_PRINTF("invalid match mode,not have this match mode:%d!\n",atoi(argv[1]));
                return AUI_RTN_EINVAL; 
                break;
        }
        AUI_TEST_CHECK_RTN(aui_dis_aspect_ratio_set_ext(g_dis_handle, en_asp_ratio,match_mode));
        AUI_PRINTF("set aspect_ratio & match_mode\n");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE set_dis_afd(unsigned long *argc,char **argv,char *sz_out_put)
{    
    /*assert arguement*/
     if (NULL == g_dis_handle){
        AUI_PRINTF("please ensure dis open\n");
        return 0;
    }
    
	if (0 == *argc) {
		afd_show_usage();
		return 0;
	}   
    
    aui_dis_afd_attr afd_attr;
    memset(&afd_attr, 0, sizeof(aui_dis_afd_attr));

    /* extract para from argv*/
    if (*argc > 0) {
        afd_attr.afd_mode = atoi(argv[0]);
    }
    if (*argc > 1) {
        afd_attr.afd_spec = atoi(argv[1]);
    }
    if (*argc > 2) {
        afd_attr.afd_protect_enable = atoi(argv[2]);
    }
    AUI_TEST_CHECK_RTN(aui_dis_afd_set(g_dis_handle,&afd_attr));
    AUI_PRINTF("set afd\n");
    return AUI_RTN_SUCCESS;
}

unsigned long test_single_output_tvsys_mode(unsigned long *argc,char **argv,char *sz_out_put)
{
 	aui_scale_param osd_param;
	unsigned int attrs[4];
	unsigned long cmd_id = 0;
	unsigned long output_width = 0;
	unsigned long output_height = 0;
	unsigned int tve_src = 0;
	
	/*assert arguement*/
	if (NULL == g_dis_handle){
		AUI_PRINTF("please ensure dis open\n");
		return 0;
	}
	if (*argc == 1) {
		cmd_id = ATOI(argv[0]); 
	} else {
		AUI_PRINTF("%s -> you can input :\n",__FUNCTION__);
		AUI_PRINTF("6 [cmd_id]\n");
		AUI_SLEEP(2000);
		return AUI_RTN_FAIL;
	}
		
	/** follow these steps to do tv system change **/
	/** 
		Step 1. Unregister DACs.(use aui_dis_dac_reg)
		Step 2. Set tv system, and use aui_gfx_layer_scale to make osd scale to the right
				position due to the tv system change
		Step 3. Set HD TV encoder signal source.
		Step 4. Register DACs			
	**/
	MEMSET((char*) attrs, 0, sizeof(unsigned int) * 4);
	MEMSET(&osd_param, 0, sizeof(aui_scale_param)); 
	/** samples here **/
	/** step 1 **/
	/*when register dac, we should firstly call aui_dis_dac_unreg to unregister dac.*/
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_YUV);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_RGB);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_SVIDEO);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_CVBS);
	aui_dis_dac_unreg(g_dis_handle_sd, AUI_DIS_TYPE_UNREG_CVBS);
	/** step 2 , set tv system meanwhile scale OSD **/
	/** the white osd rect should always in the center of the screen when tv system change **/
	switch (cmd_id) {
		case AUI_DIS_SD_576I_PAL: {// 0
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576i),CVBS TVSYS:PAL\n");
			output_width = 720;
    		output_height = 576;
		}
            break;
			
		case AUI_DIS_SD_576I_PAL_N: {// 1
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576i),CVBS TVSYS:PAL\n");
			output_width = 720;
    		output_height = 576;
		}	 
			break;
			
		case AUI_DIS_SD_576I_PAL_NC: {// 2
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576i),CVBS TVSYS:PAL\n");
			output_width = 720;
    		output_height = 576;
		}	
			break;
			
		//PAL_M and PAL_60 belong to NTSC
		case AUI_DIS_SD_480I_PAL_M: {// 3
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:NTSC\n");
			output_width = 720;
    		output_height = 480;
		}	
			break;
			
		case AUI_DIS_SD_480I_PAL_60: {// 4
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:NTSC\n");
			output_width = 720;
    		output_height = 480;
		}	
			break;
			
		case AUI_DIS_SD_480I_NTSC: {//5
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:NTSC\n");
			output_width = 720;
    		output_height = 480;
		}	
			break;
			
		case AUI_DIS_SD_480I_NTSC_443: {//6
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:NTSC\n");
			output_width = 720;
    		output_height = 480;
		}		
			break;
			
		case AUI_DIS_HD_576P: {//7
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 1));
			AUI_PRINTF("HDMI TVSYS:576P,CBVS NOT OUTPUT\n");
			output_width = 720;
			output_height = 576;
		}	
			break;
			
		case AUI_DIS_HD_480P: {//8
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 1));
			AUI_PRINTF("HDMI TVSYS:480P,CBVS NOT OUTPUT\n");
			output_width = 720;
			output_height = 480;
		}	
			break;

		case AUI_DIS_HD_720P50: {//9
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_720_50, 1));
			AUI_PRINTF("HDMI TVSYS:720P50,CBVS NOT OUTPUT\n");
			output_width = 1280;
    		output_height = 720;
		}
			break;

		case AUI_DIS_HD_720P60: {//10
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_720_60, 1));
			AUI_PRINTF("HDMI TVSYS:720P60,CBVS NOT OUTPUT\n");
			output_width = 1280;
    		output_height = 720;
		}
			break;

		case AUI_DIS_HD_1080I50: {//11
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_25, 0));
			AUI_PRINTF("HDMI TVSYS:1080I50,CBVS NOT OUTPUT\n");
			output_width = 1920;
    		output_height = 1080;
		}
			break;
			

        case AUI_DIS_HD_1080I60: {//12
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_30, 0));
			AUI_PRINTF("HDMI TVSYS:1080I60,CBVS NOT OUTPUT\n");
			output_width = 1920;
    		output_height = 1080;
		}
			break;
			
		case AUI_DIS_HD_1080P25: {//13
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_25, 1));
			AUI_PRINTF("HDMI TVSYS:1080P25,CBVS NOT OUTPUT\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;
			
		case AUI_DIS_HD_1080P30: {//14
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_30, 1));
			AUI_PRINTF("HDMI TVSYS:1080P30,CBVS NOT OUTPUT\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;
			
		case AUI_DIS_HD_1080P24: {//15
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_24, 1));
			AUI_PRINTF("HDMI TVSYS:1080P24,CBVS NOT OUTPUT\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;

		case AUI_DIS_HD_1080P50: {//16
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_50, 1));
			AUI_PRINTF("HDMI TVSYS:1080P50,CBVS NOT OUTPUT\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;

		case AUI_DIS_HD_1080P60: {//17
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_60, 1));
			AUI_PRINTF("HDMI TVSYS:1080P60,CBVS NOT OUTPUT\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;

		case AUI_DIS_HD_4096X2160P24: {//18
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_24, 1));
			AUI_PRINTF("HDMI TVSYS:4096X2160P24,CBVS NOT OUTPUT\n");
			output_width = 4096;
			output_height = 2160;
		}
			break;
			
		case AUI_DIS_HD_3840X2160P24: {//19
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_24, 1));
			AUI_PRINTF("HDMI TVSYS:3840X2160P24,CBVS NOT OUTPUT\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;
			
		case AUI_DIS_HD_3840X2160P25: {//20
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_25, 1));
			AUI_PRINTF("HDMI TVSYS:3840X2160P25,CBVS NOT OUTPUT\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;
			
		case AUI_DIS_HD_3840X2160P30: {//21
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_30, 1));
			AUI_PRINTF("HDMI TVSYS:3840X2160P30,CBVS NOT OUTPUT\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;
            
#ifdef AUI_LINUX
		case AUI_DIS_HD_3840X2160P50: {// 22
		    AUI_PRINTF("enter\n");
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_50, 1));
			AUI_PRINTF("out\n");
			AUI_PRINTF("HDMI TVSYS:3840X2160P50,CBVS NOT OUTPUT\n");
			output_width = 3840;
			output_height = 2160;

		}
			break;

		case AUI_DIS_HD_3840X2160P60: {// 23
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_60, 1));
			AUI_PRINTF("HDMI TVSYS:3840X2160P60,CBVS NOT OUTPUT\n");
			output_width = 3840;
			output_height = 2160;

		}
			break;

		case AUI_DIS_HD_4096X2160P25: {// 24
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_25, 1));
			AUI_PRINTF("HDMI TVSYS:4096X2160P25,CBVS NOT OUTPUT\n");
			output_width = 4096;
			output_height = 2160;

		}
			break;

		case AUI_DIS_HD_4096X2160P30: {// 25
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_30, 1));
			AUI_PRINTF("HDMI TVSYS:4096X2160P30,CBVS NOT OUTPUT\n");
			output_width = 4096;
			output_height = 2160;

		}
			break;

		case AUI_DIS_HD_4096X2160P50: {// 26
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_50, 1));
			AUI_PRINTF("HDMI TVSYS:4096X2160P50,CBVS NOT OUTPUT\n");
			output_width = 4096;
			output_height = 2160;

		}
			break;

		case AUI_DIS_HD_4096X2160P60: {// 27
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_60, 1));
			AUI_PRINTF("HDMI TVSYS:4096X2160P60,CBVS NOT OUTPUT\n");
			output_width = 4096;
			output_height = 2160;

		}
			break;
#endif				
		default:
			AUI_PRINTF("%s: invalid command \n", __func__);
			return AUI_RTN_FAIL;
	}


	/** scale OSD based on HD tv system,we create the surface on aui_dis_open,scale that hw surface**/
	/** to make the white rect in center of the screen in HD video format **/
	osd_param.input_width = image_width;
    osd_param.output_width = output_width;
    osd_param.input_height = image_height;
    osd_param.output_height = output_height;
	aui_gfx_layer_scale(g_layer_handle,&osd_param);
	/** step 3, set HD TV encoder signal source**/
	/** HD TV encoder will be attached to DEN**/
	tve_src = AUI_DIS_HD; 
	AUI_TEST_CHECK_RTN(aui_dis_set(g_dis_handle, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
		
	/** step 4 */
	/** in our sd products,there is only one tv encoder,cvbs should only register on this tv encoder **/
	/** register cvbs to the hd tv encoder **/
	attrs[0] = AUI_DIS_TYPE_NONE;
	attrs[1] = AUI_DIS_TYPE_NONE;
	attrs[2] = AUI_DIS_TYPE_NONE;
	attrs[3] = AUI_DIS_TYPE_CVBS;
	aui_dis_dac_reg(g_dis_handle, attrs, 4);
	
	AUI_PRINTF("\naui_dis_tv_system_set done\n"); 	
	return AUI_RTN_SUCCESS;

}


unsigned long test_dual_output_tvsys_mode(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_scale_param osd_param;
    unsigned int attrs[4];
	unsigned long cmd_id = 0;
	unsigned long output_width = 0;
	unsigned long output_height = 0;
	unsigned int tve_src = 0;

    /*assert arguement*/
     if (NULL == g_dis_handle || NULL == g_dis_handle_sd){
        AUI_PRINTF("please ensure dis open\n");
        return 0;
    }
	if(*argc == 1) {
		cmd_id = ATOI(argv[0]); 
	} else {
		AUI_PRINTF("%s -> you can input :\n",__FUNCTION__);
		AUI_PRINTF("7 [cmd_id]\n");
		AUI_PRINTF("Here you can test picture full screen display on HDMI device.");
		AUI_PRINTF("The testing process is described in the help.");
		AUI_SLEEP(2000);
		return AUI_RTN_FAIL;
	}
	
    /** follow these steps to do tv system change **/
    /** 
        Step 1. Unregister DACs.(use aui_dis_dac_reg)
        Step 2. Set tv system, and use aui_gfx_layer_scale to make osd scale to the right
          		  position due to the tv system change
        Step 3. Set HD TV encoder signal source.
        Step 4. Register DACs           
    **/
    MEMSET((char*) attrs, 0, sizeof(unsigned int) * 4);
    MEMSET(&osd_param, 0, sizeof(aui_scale_param));	
	/** samples here **/
	/** step 1 **/
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_YUV);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_RGB);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_SVIDEO);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_CVBS);
	aui_dis_dac_unreg(g_dis_handle_sd, AUI_DIS_TYPE_UNREG_CVBS);
	/** for dual output,you need to register HD output(such as YUV) to the HD tv encoder,
		sd output(such as cvbs output) need to registered on SD tv encoder **/
	/** 
		1.dual output is only avaliable on ALi hd products.
		2.in dual output we can set HD tv format(such as 1080i60) to hd output and meanwhile
		   set sd tv format(such as PAL and NTSC) to the sd output(such as cvbs).
		3.You need to keep the same frame rate when change on both side.Such as
			HD:1080i60	SD:NTSC(480i60) Allowed
			HD:1080i60	SD:PAL(576i50)	NOT allowed
			HD:1080p60	SD:NTSC(480i60) Allowed
			HD:1080p50	SD:PAL(576i50)	Allowed
			HD:1080p60	SD:PAL(576i50)	NOT allowed
	**/		

	/** step 2 , set tv system meanwhile scale OSD **/
	/** the white osd rect should always in the center of the screen when tv system change **/
	/** for dual output,you need to set tv system on each tv encoder **/
	switch (cmd_id) {
		case AUI_DIS_SD_576I_PAL: {// 0
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576i),CVBS TVSYS:PAL\n");
			output_width = 720;
    		output_height = 576;
		}	 
			break;
			
		case AUI_DIS_SD_576I_PAL_N: {// 1
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_PAL_N, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576i),CVBS TVSYS:PAL_N\n");
			output_width = 720;
    		output_height = 576;
		}	
			break;
			
		case AUI_DIS_SD_576I_PAL_NC: {// 2
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_PAL_NC, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576i),CVBS TVSYS:PAL_NC\n");
			output_width = 720;
    		output_height = 576;
		}	
			break;
			
		//PAL_M and PAL_60 belong to NTSC
		case AUI_DIS_SD_480I_PAL_M: {// 3
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_PAL_M, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:PAL_M(NTSC)\n");
			output_width = 720;
    		output_height = 480;
		}	
			break;
			
		case AUI_DIS_SD_480I_PAL_60: {// 4
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_PAL_60, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:PAL_60(NTSC)\n");
			output_width = 720;
    		output_height = 480;
		}	
			break;
			
		case AUI_DIS_SD_480I_NTSC: {// 5
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:NTSC\n");
			output_width = 720;
    		output_height = 480;
		}	
			break;
			
		case AUI_DIS_SD_480I_NTSC_443: {// 6
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC_443, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480i),CVBS TVSYS:NTSC_443\n");
			output_width = 720;
    		output_height = 480;
		}		
			break;
			
		case AUI_DIS_HD_576P: {// 7
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_PAL, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:PAL(576P),CVBS TVSYS:PAL\n");
			output_width = 720;
			output_height = 576;
		}	
			break;
			
		case AUI_DIS_HD_480P: {// 8
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_NTSC, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd,AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:NTSC(480P),CVBS TVSYS:NTSC\n");
			output_width = 720;
			output_height = 480;
		}	
			break;

		case AUI_DIS_HD_720P50: {// 9
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_720_50, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:720P50,CVBS TVSYS:PAL\n");
			output_width = 1280;
    		output_height = 720;
		}
			break;

		case AUI_DIS_HD_720P60: {// 10
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_720_60, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:720P60,CVBS TVSYS:NTSC\n");
			output_width = 1280;
    		output_height = 720;
		}
			break;

		case AUI_DIS_HD_1080I50: {// 11
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_25, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:1080I50,CVBS TVSYS:PAL\n");
			output_width = 1920;
    		output_height = 1080;
		}
			break;
			

        case AUI_DIS_HD_1080I60: {// 12
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_30, 0));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:1080I60,CVBS TVSYS:NTSC\n");
			output_width = 1920;
    		output_height = 1080;
		}
			break;
			
		case AUI_DIS_HD_1080P25: {// 13
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_25, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:1080P25,CVBS TVSYS:PAL\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;
			
		case AUI_DIS_HD_1080P30: {// 14
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_30, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:1080P30,CVBS TVSYS:NTSC\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;
			
		case AUI_DIS_HD_1080P24: {// 15
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_24, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:1080P24,CVBS TVSYS:PAL\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;

		case AUI_DIS_HD_1080P50: {// 16
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_50, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:1080P50,CVBS TVSYS:PAL\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;

		case AUI_DIS_HD_1080P60: {// 17
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_1080_60, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:1080P60,CVBS TVSYS:NTSC\n");
			output_width = 1920;
			output_height = 1080;
		}
			break;

		case AUI_DIS_HD_4096X2160P24: {// 18
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_24, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:4096X2160P24,CVBS TVSYS:PAL\n");
			output_width = 4096;
			output_height = 2160;
		}
			break;
			
		case AUI_DIS_HD_3840X2160P24: {// 19
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_24, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:3840X2160P24,CVBS TVSYS:PAL\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;
			
		case AUI_DIS_HD_3840X2160P25: {// 20
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_25, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:3840X2160P25,CVBS TVSYS:PAL\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;
			
		case AUI_DIS_HD_3840X2160P30: {// 21
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_30, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:3840X2160P30,CVBS TVSYS:NTSC\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;
			
#ifdef AUI_LINUX
		case AUI_DIS_HD_3840X2160P50: {
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_50, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:3840X2160P50,CVBS TVSYS:PAL\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;

		case AUI_DIS_HD_3840X2160P60: {// 23
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_3840X2160_60, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:3840X2160P60,CVBS TVSYS:NTSC\n");
			output_width = 3840;
			output_height = 2160;
		}
			break;

		case AUI_DIS_HD_4096X2160P25: {// 24
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_25, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:4096X2160P25,CVBS TVSYS:PAL\n");
			output_width = 4096;
			output_height = 2160;
		}
			break;

		case AUI_DIS_HD_4096X2160P30: {// 25
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_30, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:4096X2160P30,CVBS TVSYS:NTSC\n");
			output_width = 4096;
			output_height = 2160;
		}
			break;

		case AUI_DIS_HD_4096X2160P50: {// 26
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_50, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_PAL, 0));
			AUI_PRINTF("HDMI TVSYS:4096X2160P50,CVBS TVSYS:PAL\n");
			output_width = 4096;
			output_height = 2160;
		}
			break;

		case AUI_DIS_HD_4096X2160P60: {// 27
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle, AUI_DIS_TVSYS_LINE_4096X2160_60, 1));
			AUI_TEST_CHECK_RTN(aui_dis_tv_system_set(g_dis_handle_sd, AUI_DIS_TVSYS_NTSC, 0));
			AUI_PRINTF("HDMI TVSYS:4096X2160P60,CVBS TVSYS:NTSC\n");
			output_width = 4096;
			output_height = 2160;
		}
			break;
#endif			

		default:
			AUI_PRINTF("%s: invalid command \n", __func__);
			return AUI_RTN_FAIL;
	}
    /** scale OSD based on HD tv system,we create the surface on aui_dis_open,scale that hw surface**/
    /** to make the white rect in center of the screen in HD video format **/
	osd_param.input_width = image_width;
    osd_param.output_width = output_width;
    osd_param.input_height = image_height;
    osd_param.output_height = output_height;
	aui_gfx_layer_scale(g_layer_handle,&osd_param);
	/** step 3, set HD TV encoder signal source**/
	/** HD TV encoder will be attached to DEN**/
	tve_src = AUI_DIS_HD; 
	AUI_TEST_CHECK_RTN(aui_dis_set(g_dis_handle, AUI_DIS_SET_TVESDHD_SOURCE, &tve_src));
	
	/** step 4 */
	/** register YUV to the hd tv encoder **/
	attrs[0] = AUI_DIS_TYPE_V;
	attrs[1] = AUI_DIS_TYPE_U;
	attrs[2] = AUI_DIS_TYPE_Y;
	attrs[3] = AUI_DIS_TYPE_NONE;
	aui_dis_dac_reg(g_dis_handle, attrs, 4);
			
	/** register cvbs to the sd tv encoder **/
	attrs[0] = AUI_DIS_TYPE_NONE;
	attrs[1] = AUI_DIS_TYPE_NONE;
	attrs[2] = AUI_DIS_TYPE_NONE;
	attrs[3] = AUI_DIS_TYPE_CVBS;
	aui_dis_dac_reg(g_dis_handle_sd, attrs, 4);

    AUI_PRINTF("\naui_dis_tv_system_set done\n"); 
    return AUI_RTN_SUCCESS;
}

unsigned long test_set_background_color(unsigned long *argc,char **argv,char *sz_out_put)
{
    unsigned int r,g,b;
    aui_dis_color color;
    aui_dis_zoom_rect src_rect;
    aui_dis_zoom_rect dst_rect;
    /** cropping the top-left of the logo to the center of the screen,the white OSD rect should still in center **/
    src_rect.ui_startX = 0;
    src_rect.ui_startY = 0;
    src_rect.ui_width = 360;
    src_rect.ui_height = 1440;

    dst_rect.ui_startX = 180;
    dst_rect.ui_startY = 720;
    dst_rect.ui_width = 360;
    dst_rect.ui_height = 1440;

    AUI_TEST_CHECK_RTN(aui_dis_zoom(g_dis_handle,&src_rect, &dst_rect));
    if (*argc == 3){
        r = atoi(argv[0]);
        g = atoi(argv[1]);
        b = atoi(argv[2]);
    } else {
        r = 32;
        g = 178;
        b = 170;
        AUI_PRINTF("use default value\n");
    }
	color.y = (0.299*r + 0.587*g + 0.114*b);
    color.cb = (-0.1687*r-0.3313*g+0.5*b+128);
    color.cr = (0.5*r-0.4187*g-0.0813*b+128);
    AUI_PRINTF("y:%u,cb:%u,cr:%u\n", color.y,color.cb,color.cr);
    AUI_TEST_CHECK_RTN(aui_dis_set_background_color(g_dis_handle, &color));

	return 0;
}

unsigned long test_fill_black_screen(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_dis_fill_black_screen(g_dis_handle));
    return AUI_RTN_SUCCESS;
}

unsigned long test_dac_configured_get(unsigned long *argc,char **argv,char *sz_out_put)
{
    int attrs[4];
    int attrs_get_hd[4];
    int attrs_get_sd[4];
    unsigned long i = 0;

    /*assert arguement*/
     if ( (NULL == g_dis_handle) || (NULL == g_dis_handle_sd)){
        AUI_PRINTF("please ensure dis open\n");
        return 0;
    }
    //step1: get hd and sd registered dac
    aui_dis_dac_reg_get(g_dis_handle,attrs_get_hd,4);
    for( i = 0;i < 4;i++) {
        AUI_PRINTF("hd dac%ld type = %d\n",i,attrs_get_hd[i]);
    }
    aui_dis_dac_reg_get(g_dis_handle_sd,attrs_get_sd,4);
    for( i = 0;i < 4;i++) {
        AUI_PRINTF("sd dac%ld type = %d\n",i,attrs_get_sd[i]);
    }
    
    //step2: unreg all dac
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_YUV);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_RGB);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_SVIDEO);
	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_CVBS);
	aui_dis_dac_unreg(g_dis_handle_sd, AUI_DIS_TYPE_UNREG_CVBS);

    //step3: Reappear get hd and sd registered dac, to see if all returned to 0xff
    aui_dis_dac_reg_get(g_dis_handle,attrs,4);
    for( i = 0;i < 4;i++) {
        AUI_PRINTF("hd dac%ld type = %d\n",i,attrs[i]);
    }
    aui_dis_dac_reg_get(g_dis_handle_sd,attrs,4);
    for( i = 0;i < 4;i++) {
        AUI_PRINTF("sd dac%ld type = %d\n",i,attrs[i]);
    }

    //step4: Restore dac infomation
	attrs_get_hd[0] = AUI_DIS_TYPE_V;
	attrs_get_hd[1] = AUI_DIS_TYPE_U;
	attrs_get_hd[2] = AUI_DIS_TYPE_Y;
	attrs_get_hd[3] = AUI_DIS_TYPE_NONE;
	aui_dis_dac_reg(g_dis_handle, attrs_get_hd, 4);
			
	/** register cvbs to the sd tv encoder **/
	attrs_get_sd[0] = AUI_DIS_TYPE_NONE;
	attrs_get_sd[1] = AUI_DIS_TYPE_NONE;
	attrs_get_sd[2] = AUI_DIS_TYPE_NONE;
	attrs_get_sd[3] = AUI_DIS_TYPE_CVBS;
	aui_dis_dac_reg(g_dis_handle_sd, attrs_get_sd, 4);

    //step5: Check whether the restore is successful
    aui_dis_dac_reg_get(g_dis_handle,attrs_get_hd,4);
    for( i = 0;i < 4;i++) {
        AUI_PRINTF("hd dac%ld type = %d\n",i,attrs_get_hd[i]);
    }
    aui_dis_dac_reg_get(g_dis_handle_sd,attrs_get_sd,4);
    for( i = 0;i < 4;i++) {
        AUI_PRINTF("sd dac%ld type = %d\n",i,attrs_get_sd[i]);
    }

    return AUI_RTN_SUCCESS;
}

unsigned long test_onoff_dac(unsigned long *argc,char **argv,char *sz_out_put)
{
    int attrs[4];
    int attrs_get_hd[4];
    int attrs_get_sd[4];
	unsigned long cmd_id = 0;
    unsigned long i = 0;
    
    if (*argc == 1) {
		cmd_id = ATOI(argv[0]); 
	} else {
		AUI_PRINTF("Arguments for test_onoff_dac:\n");
		AUI_PRINTF("14 [cmd_id]\n");
        AUI_PRINTF("cmd_id = 0 : dac off\n");
        AUI_PRINTF("         1 : dac on\n");            
		AUI_SLEEP(100);
		return AUI_RTN_FAIL;
	}

    /*assert arguement*/
     if ( (NULL == g_dis_handle) || (NULL == g_dis_handle_sd)){
        AUI_PRINTF("please ensure dis open\n");
        return AUI_RTN_FAIL;
    }
    
    if(cmd_id == 0) {    
        //unreg all dac
    	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_YUV);
    	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_RGB);
    	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_SVIDEO);
    	aui_dis_dac_unreg(g_dis_handle, AUI_DIS_TYPE_UNREG_CVBS);
    	aui_dis_dac_unreg(g_dis_handle_sd, AUI_DIS_TYPE_UNREG_CVBS);
        
        AUI_PRINTF("unreg all dac\n");
        
        //get hd and sd registered dac, to see if all returned to 0xff
        aui_dis_dac_reg_get(g_dis_handle,attrs,4);
        for( i = 0;i < 4;i++) {
            AUI_PRINTF("hd dac%ld type = %d\n",i,attrs[i]);
        }
        aui_dis_dac_reg_get(g_dis_handle_sd,attrs,4);
        for( i = 0;i < 4;i++) {
            AUI_PRINTF("sd dac%ld type = %d\n",i,attrs[i]);
        }
    } else if(cmd_id == 1){
        //Restore dac infomation
    	attrs_get_hd[0] = AUI_DIS_TYPE_V;
    	attrs_get_hd[1] = AUI_DIS_TYPE_U;
    	attrs_get_hd[2] = AUI_DIS_TYPE_Y;
    	attrs_get_hd[3] = AUI_DIS_TYPE_NONE;
    	aui_dis_dac_reg(g_dis_handle, attrs_get_hd, 4);
    			
    	/** register cvbs to the sd tv encoder **/
    	attrs_get_sd[0] = AUI_DIS_TYPE_NONE;
    	attrs_get_sd[1] = AUI_DIS_TYPE_NONE;
    	attrs_get_sd[2] = AUI_DIS_TYPE_NONE;
    	attrs_get_sd[3] = AUI_DIS_TYPE_CVBS;
    	aui_dis_dac_reg(g_dis_handle_sd, attrs_get_sd, 4);
    	
    	AUI_PRINTF("register all dac\n");
    
        //Check whether the restore is successful
        aui_dis_dac_reg_get(g_dis_handle,attrs_get_hd,4);
        for( i = 0;i < 4;i++) {
            AUI_PRINTF("hd dac%ld type = %d\n",i,attrs_get_hd[i]);
        }
        aui_dis_dac_reg_get(g_dis_handle_sd,attrs_get_sd,4);
        for( i = 0;i < 4;i++) {
            AUI_PRINTF("sd dac%ld type = %d\n",i,attrs_get_sd[i]);
        }
    } else {
		AUI_PRINTF("Arguments for test_onoff_dac:\n");
		AUI_PRINTF("14 [cmd_id]\n");
        AUI_PRINTF("cmd_id = 0 : dac off\n");
        AUI_PRINTF("         1 : dac on\n");            
		AUI_SLEEP(100);
		return AUI_RTN_FAIL;
	}
    return AUI_RTN_SUCCESS;
}

unsigned long test_dis_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nDisplay Test Help");  

	/* DIS_1_HELP */
	#define 	DIS_1_HELP		"Before doing the other DIS test items, the step of opening the DIS module should be executed first.\n"
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Open the DIS module\n");
	aui_print_help_instruction_newline(DIS_1_HELP);

	/* DIS_2_HELP */
    #define 	DIS_2_HELP_PART1       "test Steps:"
    #define 	DIS_2_HELP_PART2       "    1  Open dis"
    #define 	DIS_2_HELP_PART3       "    2  Play stream"
    #define 	DIS_2_HELP_PART4       "    3  Set mode:"
    #define 	DIS_2_HELP_PART5       "    4           2 [startX][startY][width][height]\n"
    #define 	DIS_2_HELP_PART6       "Example :"
    #define 	DIS_2_HELP_PART7       "        test (H264 stream,like 'ekt.ts')"
    #define 	DIS_2_HELP_PART8       "        Step1:  Open DIS"
    #define 	DIS_2_HELP_PART9       "                Enter \"1\" to open DIS"
    #define 	DIS_2_HELP_PART10      "        Step2:  Enter stream and play stream"
    #define 	DIS_2_HELP_PART11      "                play 1,0,4150,27500,101,105,101,1,1,0,0"
    #define 	DIS_2_HELP_PART12      "        Step4:  Enter DIS and set mode"
    #define 	DIS_2_HELP_PART13      "                full screen(width:1280,height:720)       : 2 0,0,1280,720"
    #define 	DIS_2_HELP_PART14      "                1/4  screen(width/2,height/2)  : 2 0,0,640,360"
    #define 	DIS_2_HELP_PART15      "                1/8  screen(width/4,height/2)  : 2 0,0,320,360"
    #define 	DIS_2_HELP_PART16      "                1/8  screen(width/2,height/4)  : 2 0,0,640,180"
    #define 	DIS_2_HELP_PART17      "                1/16 screen(width/4,height/4)  : 2 0,0,320,180"
    aui_print_help_command("\'2\'");
    aui_print_help_instruction_newline("test full and preview mode ");
    aui_print_help_instruction_newline(DIS_2_HELP_PART1);
    aui_print_help_instruction_newline(DIS_2_HELP_PART2);
    aui_print_help_instruction_newline(DIS_2_HELP_PART3);
    aui_print_help_instruction_newline(DIS_2_HELP_PART4);
    aui_print_help_instruction_newline(DIS_2_HELP_PART5);
    aui_print_help_instruction_newline(DIS_2_HELP_PART6);
    aui_print_help_instruction_newline(DIS_2_HELP_PART7);
    aui_print_help_instruction_newline(DIS_2_HELP_PART8);
    aui_print_help_instruction_newline(DIS_2_HELP_PART9);
    aui_print_help_instruction_newline(DIS_2_HELP_PART10);
    aui_print_help_instruction_newline(DIS_2_HELP_PART11);
    aui_print_help_instruction_newline(DIS_2_HELP_PART12);
    aui_print_help_instruction_newline(DIS_2_HELP_PART13);
    aui_print_help_instruction_newline(DIS_2_HELP_PART14);
    aui_print_help_instruction_newline(DIS_2_HELP_PART15);
    aui_print_help_instruction_newline(DIS_2_HELP_PART16);
    aui_print_help_instruction_newline(DIS_2_HELP_PART17);    


	/* DIS_3_HELP */
	#define 	DIS_3_HELP		"Set the brightness of the screen to 90. (The highest brightness is 100)\n"
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline(DIS_3_HELP);

	
	/* DIS_4_HELP */
	#define 	DIS_4_HELP		"Zoom out the screen to 50%. Then the screen can be recoverd normal using the \'2\' (full screen) command.\n"
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline("Test the zoom function\n");
	aui_print_help_instruction_newline(DIS_4_HELP);

	/* DIS_5_HELP */
	#define 	DIS_5_HELP_PART1		"The test is following the below steps: enable the display -> wait 5s -> disable the display -> wait 5s -> enable the display again.\n"
	#define 	DIS_5_HELP_PART2		"It takes some time for the enable/disable display test. Please wait with patience until the test finishes.\n"
	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline("Enable and disable the video display\n");
	aui_print_help_instruction_newline(DIS_5_HELP_PART1);
	aui_print_help_instruction_newline(DIS_5_HELP_PART2);
	
	/* DIS_6_HELP */ 
	#define 	DIS_6_HELP_PART1	"Format:       6   [cmd_id]"
    #define     DIS_6_HELP_PART2	"              	   [cmd_id]: video display formats(ranges 0~21)"
	#define     DIS_6_HELP_PART3	"                          0: PAL "
	#define     DIS_6_HELP_PART4	"                          1: PAL_N "
	#define     DIS_6_HELP_PART5	"                          2: PAL_NC"
	#define     DIS_6_HELP_PART6	"                          3: PAL_M"
	#define 	DIS_6_HELP_PART7	"			 			   4: PAL_60"
	#define 	DIS_6_HELP_PART8	"						   5: NTSC"
    #define     DIS_6_HELP_PART9	"                          6: NTSC_443"
	#define     DIS_6_HELP_PART10	"                          7: 576P"
	#define     DIS_6_HELP_PART11	"                          8: 480P"
	#define     DIS_6_HELP_PART12	"                          9: 720P50"
	#define 	DIS_6_HELP_PART13	"						   10: 720P60"
	#define 	DIS_6_HELP_PART14	"						   11: 1080I50"
    #define     DIS_6_HELP_PART15	"                          12: 1080I60"
	#define 	DIS_6_HELP_PART16	"						   13: 1080P25"
	#define 	DIS_6_HELP_PART17	"						   14: 1080P30"
	#define 	DIS_6_HELP_PART18	"						   15: 1080P24"
	#define     DIS_6_HELP_PART19	"                          16: 1080P50"
	#define     DIS_6_HELP_PART20	"                          17: 1080P60"
	#define 	DIS_6_HELP_PART21	"						   18: 4096X2160P24"
	#define 	DIS_6_HELP_PART22	"						   19: 3840X2160P24"
	#define 	DIS_6_HELP_PART23	"						   20: 3840X2160P25"
	#define 	DIS_6_HELP_PART24	"						   21: 3840X2160P30"
	#define 	DIS_6_HELP_PART25	"						   22: 3840X2160P50"
	#define 	DIS_6_HELP_PART26	"						   23: 3840X2160P60"
	#define 	DIS_6_HELP_PART27	"						   24: 4096X2160P25"
	#define 	DIS_6_HELP_PART28	"						   25: 4096X2160P30"
	#define 	DIS_6_HELP_PART29	"						   24: 4096X2160P50"
	#define 	DIS_6_HELP_PART30	"						   25: 4096X2160P60\n"
	
	aui_print_help_command("\'6\'");
	aui_print_help_instruction_newline("Test the switching of TV playing format(single output)\n\n");
	aui_print_help_instruction_newline("The default is dual output mode\n");
	aui_print_help_instruction_newline("Before using the test, check whether the output is single-output or dual-output\n");

	aui_print_help_instruction_newline(DIS_6_HELP_PART1);
	aui_print_help_instruction_newline(DIS_6_HELP_PART2);
	aui_print_help_instruction_newline(DIS_6_HELP_PART3);
	aui_print_help_instruction_newline(DIS_6_HELP_PART4);
	aui_print_help_instruction_newline(DIS_6_HELP_PART5);
    aui_print_help_instruction_newline(DIS_6_HELP_PART6);
	aui_print_help_instruction_newline(DIS_6_HELP_PART7);
	aui_print_help_instruction_newline(DIS_6_HELP_PART8);
	aui_print_help_instruction_newline(DIS_6_HELP_PART9);
	aui_print_help_instruction_newline(DIS_6_HELP_PART10);
	aui_print_help_instruction_newline(DIS_6_HELP_PART11);
	aui_print_help_instruction_newline(DIS_6_HELP_PART12);
    aui_print_help_instruction_newline(DIS_6_HELP_PART13);
	aui_print_help_instruction_newline(DIS_6_HELP_PART14);
	aui_print_help_instruction_newline(DIS_6_HELP_PART15);
	aui_print_help_instruction_newline(DIS_6_HELP_PART16);
	aui_print_help_instruction_newline(DIS_6_HELP_PART17);
	aui_print_help_instruction_newline(DIS_6_HELP_PART18);
	aui_print_help_instruction_newline(DIS_6_HELP_PART19);
    aui_print_help_instruction_newline(DIS_6_HELP_PART20);
	aui_print_help_instruction_newline(DIS_6_HELP_PART21);
	aui_print_help_instruction_newline(DIS_6_HELP_PART22);
	aui_print_help_instruction_newline(DIS_6_HELP_PART23);
	aui_print_help_instruction_newline(DIS_6_HELP_PART24);
	aui_print_help_instruction_newline(DIS_6_HELP_PART25);
    aui_print_help_instruction_newline(DIS_6_HELP_PART26);
	aui_print_help_instruction_newline(DIS_6_HELP_PART27);
	aui_print_help_instruction_newline(DIS_6_HELP_PART28);
	aui_print_help_instruction_newline(DIS_6_HELP_PART29);
	aui_print_help_instruction_newline(DIS_6_HELP_PART30);
	
	/* DIS_7_HELP */ 
	#define 	DIS_7_HELP_PART1	"Format:	   7   [cmd_id]"
    #define 	DIS_7_HELP_PART2	"				   [cmd_id]: video display formats(ranges 0~14)"
	#define     DIS_7_HELP_PART3	"                            The command is the same as '6'"
	#define 	DIS_7_HELP_PART4	"Full-screen display picture test steps(Only Linux)\n"
	#define 	DIS_7_HELP_PART5	"						   1  modify the DBF size"
	#define 	DIS_7_HELP_PART6	"						   2  open dis"
	#define 	DIS_7_HELP_PART7	"						   3  initializes the same surface as the image resolution"
	#define 	DIS_7_HELP_PART8	"						   4  osd show image"
	#define 	DIS_7_HELP_PART9	"						   5  switch video display formats and osd scale"
	#define 	DIS_7_HELP_PART10	"						   6  adjust the transparency(optional)"
	#define 	DIS_7_HELP_PART11	"Example :\n"
	#define 	DIS_7_HELP_PART12	"	 Test 1920*1080 image full-screen display in different formats"
	#define 	DIS_7_HELP_PART13	"						   step1 :	vi /etc/directfbrc change into 'mode=1920x1080'"
	#define 	DIS_7_HELP_PART14	"						   step2 :	Enter \"dis\"->\"1\" to open dis\n"
	#define 	DIS_7_HELP_PART15	"						   step3 :	Enter \"9 2\" to initializes osd surface\n"
	#define 	DIS_7_HELP_PART16	"						   step4 :	Enter \"up\"->\"osd\"->\"13 /mnt/usb/sda1/1920_1080.png,1920,1080\""
	#define 	DIS_7_HELP_PART17	"						   step5 :	Enter \"up\"->\"dis\"->\"7 18\"\n"
	#define 	DIS_7_HELP_PART18	"						   step6 :	Enter \"10 255\"\n"

	aui_print_help_command("\'7\'");
	aui_print_help_instruction_newline("Test the switching of TV playing format(dual output)\n\n");
	aui_print_help_instruction_newline("The default is dual output mode\n");
	aui_print_help_instruction_newline("Before using the test, check whether the output is single-output or dual-output\n");
	aui_print_help_instruction_newline(DIS_7_HELP_PART1);
	aui_print_help_instruction_newline(DIS_7_HELP_PART2);
	aui_print_help_instruction_newline(DIS_7_HELP_PART3);
	aui_print_help_instruction_newline(DIS_7_HELP_PART4);
	aui_print_help_instruction_newline(DIS_7_HELP_PART5);
	aui_print_help_instruction_newline(DIS_7_HELP_PART6);
	aui_print_help_instruction_newline(DIS_7_HELP_PART7);
	aui_print_help_instruction_newline(DIS_7_HELP_PART8);
	aui_print_help_instruction_newline(DIS_7_HELP_PART9);
	aui_print_help_instruction_newline(DIS_7_HELP_PART10);
	aui_print_help_instruction_newline(DIS_7_HELP_PART11);
	aui_print_help_instruction_newline(DIS_7_HELP_PART12);
	aui_print_help_instruction_newline(DIS_7_HELP_PART13);
	aui_print_help_instruction_newline(DIS_7_HELP_PART14);
	aui_print_help_instruction_newline(DIS_7_HELP_PART15);
	aui_print_help_instruction_newline(DIS_7_HELP_PART16);
	aui_print_help_instruction_newline(DIS_7_HELP_PART17);
	aui_print_help_instruction_newline(DIS_7_HELP_PART18);

    #ifdef AUI_LINUX
    #define 	DIS_8_HELP_PART1        "Set the background color(RGB),e.g,9 255,0,0\n"
    aui_print_help_command("\'8\'");
	aui_print_help_instruction_newline(DIS_8_HELP_PART1);
    #endif  

	/* DIS_9_HELP */ 
	#define 	DIS_9_HELP_PART1	"Format:	   9   [ssize]"
	#define 	DIS_9_HELP_PART2	"				   [ssize]: set surface size"
	#define 	DIS_9_HELP_PART3	"							0 : 576 * 480"
	#define 	DIS_9_HELP_PART4	"							1 : 1280 * 720"
	#define 	DIS_9_HELP_PART5	"							2 : 1920 * 1080"
	#define 	DIS_9_HELP_PART6	"							3 : 3840 * 2160"
	#define 	DIS_9_HELP_PART7	"							4 : 4096 * 2160"

	aui_print_help_command("\'9\'");
	aui_print_help_instruction_newline("Test the OSD drawing a rectangle on display layer\n");
	aui_print_help_instruction_newline("This case will set a surface size, this size should be the same as the image to be displayed\n");
	aui_print_help_instruction_newline("The default size is 1280 * 720\n");
	aui_print_help_instruction_newline(DIS_9_HELP_PART1);
	aui_print_help_instruction_newline(DIS_9_HELP_PART2);
	aui_print_help_instruction_newline(DIS_9_HELP_PART3);
	aui_print_help_instruction_newline(DIS_9_HELP_PART4);
	aui_print_help_instruction_newline(DIS_9_HELP_PART5);
	aui_print_help_instruction_newline(DIS_9_HELP_PART6);
	aui_print_help_instruction_newline(DIS_9_HELP_PART7);

	/* DIS_10_HELP */ 
	aui_print_help_command("\'10\'");
	aui_print_help_instruction_newline("Set the transparency of the image.\n");
	aui_print_help_instruction_newline("The range of transparency values is 0 ~ 255\n");

    /* DIS_MATCH_HELP */      
	#define 	DIS_MATCH_HELP_PART1		"Used to set the display match mode\n"
	#define 	DIS_MATCH_HELP_PART2		"and to set different display aspect ratio\n"
    aui_print_help_command("\'match\'");
	aui_print_help_instruction_newline("Set aspect_ratio & match_mode\n");
	aui_print_help_instruction_newline(DIS_MATCH_HELP_PART1);
	aui_print_help_instruction_newline(DIS_MATCH_HELP_PART2);
    

    /* DIS_AFD_HELP */
	#define 	DIS_AFD_HELP_PART1		"AFD test Steps:\n"
	#define 	DIS_AFD_HELP_PART2		"    1  Open dis\n"
    #define 	DIS_AFD_HELP_PART3		"    2  Set afd\n"
	#define 	DIS_AFD_HELP_PART4		"    3  Set match\n"
    #define 	DIS_AFD_HELP_PART5		"    4  Play stream\n"
	#define 	DIS_AFD_HELP_PART6		"Example :\n"
    #define 	DIS_AFD_HELP_PART7		"    SW AFD test (SD Test stream 4:3 -> Aspect Ratio 16:9 , MinBig\n)"
	#define 	DIS_AFD_HELP_PART8		"        Step1:  Open DIS"
    #define 	DIS_AFD_HELP_PART9      "                Enter \"1\" to open DIS"
    #define 	DIS_AFD_HELP_PART10		"        Step2:  afd 1,1,0\n"
	#define 	DIS_AFD_HELP_PART11		"        Step3:  match 0,6\n"
    #define 	DIS_AFD_HELP_PART12		"        Step4:  Enter stream and play stream"
    #define 	DIS_AFD_HELP_PART13	    "                play 0,1,1320,27500,2031,2032,2031,1,2,0,H\n"
    aui_print_help_command("\'afd\'");
	aui_print_help_instruction_newline("Set the Active Format Description (AFD) display mode\n");
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART1);
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART2);
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART3);
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART4);
    aui_print_help_instruction_newline(DIS_AFD_HELP_PART5);
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART6);
    aui_print_help_instruction_newline(DIS_AFD_HELP_PART7);
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART8);
    aui_print_help_instruction_newline(DIS_AFD_HELP_PART9);
	aui_print_help_instruction_newline(DIS_AFD_HELP_PART10);
    aui_print_help_instruction_newline(DIS_AFD_HELP_PART11);
    aui_print_help_instruction_newline(DIS_AFD_HELP_PART12);
    aui_print_help_instruction_newline(DIS_AFD_HELP_PART13);
    
    return AUI_RTN_HELP;
}


void dis_test_reg(void)
{
	aui_tu_reg_group("dis", "dis tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_dis_open, "test_dis_open");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_dis_mode, "test_dis_mode");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_enhance_set, "test_enhance_set");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_zoom, "test_zoom");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_video_enable, "test_video_enable");
//#ifndef DUAL_VIDEO_OUTPUT_USE_VCAP 
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_single_output_tvsys_mode, "test_single_output_tvsys_mode");
//#else
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_dual_output_tvsys_mode, "test_dual_output_tvsys_mode");
//#endif
	aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, test_set_background_color, "test_set_background_color");   
	aui_tu_reg_item(2, "9", AUI_CMD_TYPE_API, test_dis_osd_draw_rect, "test_dis_osd_draw_rect");
	aui_tu_reg_item(2, "10", AUI_CMD_TYPE_API, set_dis_osd_alpha, "set_dis_osd_alpha");
	aui_tu_reg_item(2, "11", AUI_CMD_TYPE_API, test_dis_HDTVencoder_output_CVBS, "test_HD TV encoder connect to DEO and output CVBS");
    aui_tu_reg_item(2, "12", AUI_CMD_TYPE_API, test_fill_black_screen, "test_fill_black_screen");
    aui_tu_reg_item(2, "13", AUI_CMD_TYPE_API, test_dac_configured_get, "test register dac infomation get");
    aui_tu_reg_item(2, "14", AUI_CMD_TYPE_API, test_onoff_dac, "test on/off dac");
    aui_tu_reg_item(2, "match", AUI_CMD_TYPE_API, set_aspect_ratio_and_match_mode, "set_aspect_ratio_and_match_mode");
    aui_tu_reg_item(2, "afd", AUI_CMD_TYPE_API, set_dis_afd, "set_dis_afd");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_dis_help, "display help");
}

