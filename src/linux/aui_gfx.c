/** @file       aui_gfx.c
*   @brief      aui graphic module
*   @author     peter.pan@alitech.com
*   @date       2014-1-24
*   @version    1.0.0
*   @note       ali corp. all rights reserved. 2013-2999 copyright (C)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alisldis.h>
#include <aui_osd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <aui_common.h>
#include "aui_common_priv.h"
#ifndef LINUX_AUI_DISABLE_GFX
#include <directfb.h>
#include <direct/direct.h>
#include <direct/types.h>
#endif


#define u32 uint32_t
#define u16 uint16_t

AUI_MODULE(GFX)

#define GFX_FUNCTION_ENTER AUI_DBG("enter\n");
#define GFX_FUNCTION_LEAVE AUI_DBG("leave\n");

/* ioctls
   0x46 is 'F'								*/
#define FBIOGET_VSCREENINFO	0x4600
#define FBIOPUT_VSCREENINFO	0x4601
#define FBIOGET_FSCREENINFO	0x4602
#define FBIOPAN_DISPLAY		0x4606
#define FBIOGETCMAP		0x4604
#define FBIOPUTCMAP		0x4605
#define FB_ACTIVATE_VBL	       16	/* activate values on next vbl  */
#define FB_ACTIVATE_NOW	       0	/* activate values now  */
#define FB_ACTIVATE_FORCE     128	/* force apply even when no change*/
/* Interpretation of offset for color fields: All offsets are from the right,
 * inside a "pixel" value, which is exactly 'bits_per_pixel' wide (means: you
 * can use the offset as right argument to <<). A pixel afterwards is a bit
 * stream and is written to video memory as that unmodified. This implies
 * big-endian byte order if bits_per_pixel is greater than 8.
 */
struct fb_bitfield {
	u32 offset;			/* beginning of bitfield	*/
	u32 length;			/* length of bitfield		*/
	u32 msb_right;			/* != 0 : Most significant bit is */
					/* right */ 
};
struct fb_fix_screeninfo {
	char id[16];			/* identification string eg "TT Builtin" */
	unsigned long smem_start;	/* Start of frame buffer mem */
					/* (physical address) */
	u32 smem_len;			/* Length of frame buffer mem */
	u32 type;			/* see FB_TYPE_*		*/
	u32 type_aux;			/* Interleave for interleaved Planes */
	u32 visual;			/* see FB_VISUAL_*		*/
	u16 xpanstep;			/* zero if no hardware panning  */
	u16 ypanstep;			/* zero if no hardware panning  */
	u16 ywrapstep;			/* zero if no hardware ywrap    */
	u32 line_length;		/* length of a line in bytes    */
	unsigned long mmio_start;	/* Start of Memory Mapped I/O   */
					/* (physical address) */
	u32 mmio_len;			/* Length of Memory Mapped I/O  */
	u32 accel;			/* Indicate to driver which	*/
					/*  specific chip/card we have	*/
	u16 reserved[3];		/* Reserved for future compatibility */
};
struct fb_var_screeninfo {
	u32 xres;			/* visible resolution		*/
	u32 yres;
	u32 xres_virtual;		/* virtual resolution		*/
	u32 yres_virtual;
	u32 xoffset;			/* offset from virtual to visible */
	u32 yoffset;			/* resolution			*/

	u32 bits_per_pixel;		/* guess what			*/
	u32 grayscale;			/* != 0 Graylevels instead of colors */

	struct fb_bitfield red;		/* bitfield in fb mem if true color, */
	struct fb_bitfield green;	/* else only length is significant */
	struct fb_bitfield blue;
	struct fb_bitfield transp;	/* transparency			*/	

	u32 nonstd;			/* != 0 Non standard pixel format */

	u32 activate;			/* see FB_ACTIVATE_*		*/

	u32 height;			/* height of picture in mm    */
	u32 width;			/* width of picture in mm     */

	u32 accel_flags;		/* (OBSOLETE) see fb_info.flags */

	/* Timing: All values in pixclocks, except pixclock (of course) */
	u32 pixclock;			/* pixel clock in ps (pico seconds) */
	u32 left_margin;		/* time from sync to picture	*/
	u32 right_margin;		/* time from picture to sync	*/
	u32 upper_margin;		/* time from sync to picture	*/
	u32 lower_margin;
	u32 hsync_len;			/* length of horizontal sync	*/
	u32 vsync_len;			/* length of vertical sync	*/
	u32 sync;			/* see FB_SYNC_*		*/
	u32 vmode;			/* see FB_VMODE_*		*/
	u32 rotate;			/* angle we rotate counter clockwise */
	u32 reserved[5];		/* Reserved for future compatibility */
};

struct fb_cmap {
	u32 start;			/* First entry	*/
	u32 len;			/* Number of entries */
	u16 *red;			/* Red values	*/
	u16 *green;
	u16 *blue;
	u16 *transp;			/* transparency, can be NULL */
};

static unsigned char *g_fb2_addr = NULL;
int fb2_fd = -1;
struct fb_var_screeninfo orig_var;      /* fbdev variable screeninfo
                                                before DirectFB was started */
struct fb_var_screeninfo current_var;   /* fbdev variable screeninfo
                                              set by DirectFB */
void *orig_cmap_memory = NULL;
void *current_cmap_memory = NULL;
struct fb_cmap           orig_cmap;     /* original palette */

struct fb_cmap           current_cmap;  /* our copy of the cmap */
//#define fb2_debug 1
#ifdef fb2_debug  
static void prt_fb2_var_info(struct fb_var_screeninfo orig_var)
{
    AUI_DBG(" ================================================\n");
    AUI_DBG("xres = %d\n", orig_var.xres);
    AUI_DBG("yres = %d\n", orig_var.yres);
    AUI_DBG("xres_virtual = %d\n", orig_var.xres_virtual);
    AUI_DBG("yres_virtual = %d\n", orig_var.yres_virtual);
    AUI_DBG("xoffset = %d\n", orig_var.xoffset);
    AUI_DBG("yoffset = %d\n", orig_var.yoffset);
    AUI_DBG("bits_per_pixel = %d\n", orig_var.bits_per_pixel);
    AUI_DBG("red.length = %d\n", orig_var.red.length);
    AUI_DBG("red.offset = %d\n", orig_var.red.offset);
    AUI_DBG("green.length = %d\n", orig_var.green.length);
    AUI_DBG("green.offset = %d\n", orig_var.green.offset);
    AUI_DBG("blue.length = %d\n", orig_var.blue.length);
    AUI_DBG("blue.offset = %d\n", orig_var.blue.offset);
    AUI_DBG("transp.length = %d \n", orig_var.transp.length);
    AUI_DBG("transp.offset = %d\n", orig_var.transp.offset);
    AUI_DBG(" ================================================\n");
}
#endif

/**
*    @brief         according to the color mode for each pixel to get the number of bytes
*    @date          2017-7-26
*    @param[in]     color_mode  the surface's color mode
*    @return           the pixel bits of color mode.
*/

int get_bytes_per_pixel(enum aui_osd_pixel_format color_mode)
{
    int bytes_per_pixel = 0;
    switch (color_mode) {
        case AUI_OSD_HD_ARGB1555:
        case AUI_OSD_HD_RGB565:
        case AUI_OSD_HD_ARGB4444: // 2 bytes
        case AUI_OSD_HD_RGB444:
        case AUI_OSD_HD_RGB555:
            bytes_per_pixel = 2;
            break;

        case AUI_OSD_HD_AYCbCr8888:
        case AUI_OSD_HD_ARGB8888: // 4 bytes
        case AUI_OSD_HD_RGB888: //4 bytes on IC
            bytes_per_pixel = 4;
            break;

        default: // default 1 byte
            bytes_per_pixel = 1;
            break;
        }
    return bytes_per_pixel;
}

static int init_fb2()
{
    char fb2_dev[12] = "/dev/fb2";
    // open fb2
    fb2_fd = open(fb2_dev, O_RDWR );
    if (fb2_fd < 0) {
        AUI_ERR( "DirectFB/FBDev: Error opening '%s'!\n", fb2_dev);
        goto error;
    }

    /* fbdev fixed screeninfo, contains infos about memory and type of card */
    struct fb_fix_screeninfo fix;   
    memset(&fix, 0, sizeof(struct fb_fix_screeninfo));
    /* Retrieve fixed informations like video ram size */
    AUI_DBG("dev_fb: %d, FBIOGET_FSCREENINFO\n", fb2_fd);
    if (ioctl( fb2_fd, FBIOGET_FSCREENINFO, &fix ) < 0) {
        AUI_DBG("dev_fb: %d, FBIOGET_FSCREENINFO fail\n", fb2_fd);
        AUI_ERR( "AUI GFX: "
                "Could not get fixed screen information of fb2!\n" );
        goto error;
    }

    AUI_DBG("dev_fb: %d, FBIOGET_FSCREENINFO\n success", fb2_fd);
    AUI_INFO( "AUI GFX: Found '%s' (ID %d) with frame buffer2 at 0x%08lx, %dk (MMIO 0x%08lx, %dk)\n",
                fix.id, fix.accel,
                fix.smem_start, fix.smem_len >> 10,
                fix.mmio_start, fix.mmio_len >> 10);

    /* Map the framebuffer */
    g_fb2_addr = mmap( NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb2_fd, 0 );
    if (g_fb2_addr == MAP_FAILED) {
        AUI_ERR( "AUI GFX: "
                "Could not mmap the framebuffer 2!\n");
        g_fb2_addr = NULL;
        goto error;
    }

    // get fb2 variable screen info
    memset(&orig_var, 0, sizeof(struct fb_var_screeninfo));
    memset(&current_var, 0, sizeof(struct fb_var_screeninfo));
    AUI_DBG("dev_fb: %d, FBIOGET_VSCREENINFO\n", fb2_fd);
    if (ioctl( fb2_fd, FBIOGET_VSCREENINFO, &orig_var ) < 0) {
        AUI_DBG("dev_fb: %d, FBIOGET_VSCREENINFO fail\n", fb2_fd);
        AUI_ERR( "AUI GFX:"
                "Could not get variable screen information!\n" );
        goto error;
    }

    AUI_DBG("dev_fb: %d, FBIOGET_VSCREENINFO success\n", fb2_fd);
	AUI_INFO("fb2 orig var, xres: %d, yres: %d, bits per pixel: %d\n",
                     orig_var.xres, orig_var.yres, orig_var.bits_per_pixel);
	
    current_var = orig_var;    

    // get fb2 color map
    orig_cmap_memory = malloc( 256 * 2 * 4 );
    if (!orig_cmap_memory) {
        AUI_ERR( "AUI GFX:"
            "no memory for malloc!\n" );
        goto error;
    }

     orig_cmap.start  = 0;
     orig_cmap.len    = 256;
     orig_cmap.red    = (u16 *)(orig_cmap_memory + 256 * 2 * 0);
     orig_cmap.green  = (u16 *)(orig_cmap_memory + 256 * 2 * 1);
     orig_cmap.blue   = (u16 *)(orig_cmap_memory + 256 * 2 * 2);
     orig_cmap.transp = (u16 *)(orig_cmap_memory + 256 * 2 * 3);

     AUI_DBG("dev_fb: %d, FBIOGETCMAP\n", fb2_fd);
     if (ioctl( fb2_fd, FBIOGETCMAP, &orig_cmap ) < 0) {
          AUI_DBG("dev_fb: %d, FBIOGETCMAP fail\n", fb2_fd);
          AUI_ERR( "AUI GFX: "
                   "Could not retrieve palette for backup!\n" );

          memset( &orig_cmap, 0, sizeof(orig_cmap) );

          free(orig_cmap_memory);
          orig_cmap_memory = NULL;
          goto error;
     }
     AUI_DBG("dev_fb: %d, FBIOGETCMAP success\n", fb2_fd);
     AUI_DBG("dev_fb: %d, FBIOGETCMAP: {start:%d,len:%d,red:0x%x,green:0x%x,blue:0x%x,transp:0x%x}\n",
         fb2_fd,orig_cmap.start,orig_cmap.len,(int)orig_cmap.red,(int)orig_cmap.green,(int)orig_cmap.blue,(int)orig_cmap.transp);

     current_cmap_memory = malloc( 256 * 2 * 4 );
     if (!current_cmap_memory) {
        AUI_ERR( "AUI GFX:"
                "no memory for malloc!\n" );
        goto error;
     }

     current_cmap.start  = 0;
     current_cmap.len    = 256;
     current_cmap.red    = (u16 *)(current_cmap_memory + 256 * 2 * 0);
     current_cmap.green  = (u16 *)(current_cmap_memory + 256 * 2 * 1);
     current_cmap.blue   = (u16 *)(current_cmap_memory + 256 * 2 * 2);
     current_cmap.transp = (u16 *)(current_cmap_memory + 256 * 2 * 3);


    // clear fb2
    memset(g_fb2_addr, 0, fix.smem_len);

    //ioctl(fb2_fd, FBIOPAN_DISPLAY, &current_var);

    return AUI_GFX_SUCCESS;

error:
    return AUI_GFX_DRIVER_ERROR;
}

void de_init_fb2() 
{
    orig_var.activate = FB_ACTIVATE_NOW;
    AUI_DBG("dev_fb: %d, FBIOPUT_VSCREENINFO: FB_ACTIVATE_NOW\n", fb2_fd);
    if (ioctl( fb2_fd, FBIOPUT_VSCREENINFO, &orig_var ) < 0) {
        AUI_DBG("dev_fb: %d, FBIOPUT_VSCREENINFO fail\n", fb2_fd);
        AUI_ERR( "AUI GFX: "
                "reset var info fail on fb2!\n" );        
    } 
    AUI_DBG("dev_fb: %d, FBIOPUT_VSCREENINFO success\n", fb2_fd);
	
    AUI_DBG("dev_fb: %d, FBIOPUTCMAP: {start:%d,len:%d,red:0x%x,green:0x%x,blue:0x%x,transp:0x%x}\n",
        fb2_fd,orig_cmap.start,orig_cmap.len,(int)orig_cmap.red,(int)orig_cmap.green,(int)orig_cmap.blue,(int)orig_cmap.transp);
    if (ioctl( fb2_fd, FBIOPUTCMAP, &orig_cmap) < 0) {
        AUI_DBG("dev_fb: %d, FBIOPUTCMAP fail\n", fb2_fd);
        AUI_ERR( "AUI GFX: "
                "reset color map on fb2 fail!\n" );        
    } 
    AUI_DBG("dev_fb: %d, FBIOPUTCMAP success\n", fb2_fd);

    // clear fb2
    memset(g_fb2_addr, 0, orig_var.xres * orig_var.yres);
    munmap(g_fb2_addr, orig_var.xres * orig_var.yres);
    close(fb2_fd);
    fb2_fd = -1;
    if(orig_cmap_memory) {
        free(orig_cmap_memory);
        orig_cmap_memory = NULL;
    }
    if(current_cmap_memory) {
        free(current_cmap_memory);
        current_cmap_memory = NULL;
    }
}

typedef struct {
    struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
    aui_osd_layer layer_id;
    alisl_handle alisl_dis;
    unsigned char global_alpha;
} aui_layer_handle_t;

/**
*    @brief         set fb2's screen info according to the color pass by aui_gfx_hw_surface_create
*    @date          2015-12-24
*    @param[in]     color_mode  the surface's color mode
*    @return            AUI_GFX_SUCCESS if success.
*                       return -1, if ioctl return fail.
*/
//the warring that this function not used when disable DEBUG ,unless take out static 
AUI_RTN_CODE set_fb2_mode(enum aui_osd_pixel_format color_mode)
{
    AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
    u32 max_width = orig_var.xres * sqrt(orig_var.bits_per_pixel/8);
    u32 max_height = orig_var.yres * sqrt(orig_var.bits_per_pixel/8);

	AUI_INFO("fb2 max width: %d, max height: %d, xres: %d, yres: %d, bits per pixel: %d\n",
                    max_width, max_height, orig_var.xres, orig_var.yres, orig_var.bits_per_pixel);
    /* the region width * height * byte_per_pixel <= orig_var.xres * orig_var.yres
       width / height = orig_var.xres / orig_var.yres
       => so, 
        width = orign_var.xres / sqrt(byte_per_pixel);
        height = orign_var.yres / sqrt(byte_per_pixel);
    */
    
    switch (color_mode)
    {
        case AUI_OSD_4_COLOR:
            // not support 3922
            current_var.bits_per_pixel = 8;
            break;
        case AUI_OSD_256_COLOR: // CLUT8
            // Note: 
            // The driver would colse fb2, to avoid flower screen.
            // User need to call aui_gfx_set_pallette to set pallette, 
            // and then fill the fb2 with transparent color and open the layer.
            current_var.bits_per_pixel = 8;
            break;
        case AUI_OSD_16_COLOR_PIXEL_ALPHA: // 2
            // 8bit, not support 3922
            current_var.bits_per_pixel = 8;
            break;
        case AUI_OSD_HD_ACLUT88: // 3
            // not support
            current_var.bits_per_pixel = 16;
            max_width = (int) ((double)max_width / 1.414);
            max_height = (int) ((double)max_height / 1.414); 
            break;
        
        case AUI_OSD_HD_RGB565: // 4
            current_var.bits_per_pixel = 16; 
            current_var.transp.length = 0;
            current_var.transp.offset = 0;
            current_var.red.length = 5;
            current_var.red.offset = 11;
            current_var.green.length = 6;
            current_var.green.offset = 5;
            current_var.blue.length = 5;
            current_var.blue.offset = 0;
            //the max memory of fb2 is 960*540*4, using size divide 1.4 will exceed the max value,
            //so we should use size divide 1.414
            max_width = (int) ((double)max_width / 1.414);
            max_height = (int) ((double)max_height / 1.414);
            break;
        case AUI_OSD_HD_RGB888: // 5
            current_var.bits_per_pixel = 24;
            current_var.transp.length = 0;
            current_var.transp.offset = 0;
            current_var.red.length = 8;
            current_var.red.offset = 16;
            current_var.green.length = 8;
            current_var.green.offset = 8;
            current_var.blue.length = 8;
            current_var.blue.offset = 0;
            max_width = (int) ((double)max_width / 2);
            max_height = (int) ((double)max_height / 2);
            break;
        case AUI_OSD_HD_RGB555: // 6
            current_var.bits_per_pixel = 16; 
            current_var.transp.length = 0;
            current_var.transp.offset = 0;
            current_var.red.length = 5;
            current_var.red.offset = 10;
            current_var.green.length = 5;
            current_var.green.offset = 5;
            current_var.blue.length = 5;
            current_var.blue.offset = 0;
            //the max memory of fb2 is 960*540*4, using size divide 1.4 will exceed the max value,
            //so we should use size divide 1.414
            max_width = (int) ((double)max_width / 1.414);
            max_height = (int) ((double)max_height / 1.414);
            break;
        case AUI_OSD_HD_RGB444: // 7
            current_var.bits_per_pixel = 16; 
            current_var.transp.length = 0;
            current_var.transp.offset = 0;
            current_var.red.length = 4;
            current_var.red.offset = 8;
            current_var.green.length = 4;
            current_var.green.offset = 4;
            current_var.blue.length = 4;
            current_var.blue.offset = 0;
            //the max memory of fb2 is 960*540*4, using size divide 1.4 will exceed the max value,
            //so we should use size divide 1.414
            max_width = (int) ((double)max_width / 1.414);
            max_height = (int) ((double)max_height / 1.414);
            break;
        case AUI_OSD_HD_ARGB8888: // 8
            current_var.bits_per_pixel = 32; 
            current_var.transp.length = 8;
            current_var.transp.offset = 24;
            current_var.red.length = 8;
            current_var.red.offset = 16;
            current_var.green.length = 8;
            current_var.green.offset = 8;
            current_var.blue.length = 8;
            current_var.blue.offset = 0;
            max_width = (int) ((double)max_width / 2);
            max_height = (int) ((double)max_height / 2);
            break;
        case AUI_OSD_HD_ARGB1555: // 9
            current_var.bits_per_pixel = 16; 
            current_var.transp.length = 1;
            current_var.transp.offset = 15;
            current_var.red.length = 5;
            current_var.red.offset = 10;
            current_var.green.length = 5;
            current_var.green.offset = 5;
            current_var.blue.length = 5;
            current_var.blue.offset = 0;
            //the max memory of fb2 is 960*540*4, using size divide 1.4 will exceed the max value,
            //so we should use size divide 1.414
            max_width = (u32) ((double)max_width / 1.414);
            max_height = (u32) ((double)max_height / 1.414);
            break;
        case AUI_OSD_HD_ARGB4444: // 10
            current_var.bits_per_pixel = 16; 
            current_var.transp.length = 4;
            current_var.transp.offset = 12;
            current_var.red.length = 4;
            current_var.red.offset = 8;
            current_var.green.length = 4;
            current_var.green.offset = 4;
            current_var.blue.length = 4;
            current_var.blue.offset = 0;
            //the max memory of fb2 is 960*540*4, using size divide 1.4 will exceed the max value,
            //so we should use size divide 1.414
            max_width = (int) ((double)max_width / 1.414);
            max_height = (int) ((double)max_height / 1.414);
            break;
        
        
        default:            
            break;
    }

    // width and height should be 2bytes aligned.
    if(max_width%2 != 0)
        max_width -= 1;
    if(max_height%2 != 0)
        max_height -= 1;

    AUI_INFO("fb2 max width: %d, max height: %d\n",
                    max_width, max_height);
    
    current_var.xres = max_width;
    current_var.yres = max_height;
    current_var.xres_virtual = current_var.xres;
    current_var.yres_virtual = current_var.yres;
    current_var.xoffset = 0;
    current_var.yoffset = 0;
    current_var.accel_flags = 0;
	/*
     *  In 3505, the default color mode is AUI_OSD_256_COLOR when the board is turned on, 
     *  when the color mode is set for the first time and the color mode is AUI_OSD_256_COLOR,there are two cases :
     *  1. activate = FB_ACTIVATE_NOW
     *  Fb2's color mode setup process cannot be completed in full, the fill and other operations cannot be displayed on the screen.
     *  2. activate = FB_ACTIVATE_FORCE
     *  Fb2's color mode setup process can be completed in full, the fill and other operations can be displayed on the screen.
     *                                  */
    if (AUI_OSD_256_COLOR == color_mode)
        current_var.activate = FB_ACTIVATE_FORCE;
    else
        current_var.activate = FB_ACTIVATE_NOW;   
    AUI_DBG("dev_fb: %d, FBIOPUT_VSCREENINFO: {current_var.activate: %d}\n",  fb2_fd, current_var.activate);
    if (ioctl( fb2_fd, FBIOPUT_VSCREENINFO, &current_var ) < 0) {
        AUI_DBG("dev_fb: %d, FBIOPUT_VSCREENINFO fail\n",  fb2_fd);
        AUI_ERR( "AUI GFX: "
                "set var info fail!\n" );
        return -1;
    }
    AUI_DBG("dev_fb: %d, FBIOPUT_VSCREENINFO success\n",  fb2_fd); 

#ifdef fb2_debug
    //check config
    memset(&current_var, 0, sizeof(struct fb_var_screeninfo));
    AUI_DBG("dev_fb: %d, FBIOGET_VSCREENINFO\n", fb2_fd);
    if (ioctl( fb2_fd, FBIOGET_VSCREENINFO, &current_var ) < 0) {
        AUI_DBG("dev_fb: %d, FBIOGET_VSCREENINFO fail\n", fb2_fd);
        AUI_ERR( "AUI GFX:"
                "Could not get variable screen information!\n" );
        return -1;
    }
    AUI_DBG("dev_fb: %d, FBIOGET_VSCREENINFO success\n", fb2_fd);
    
    AUI_DBG("\n original info: \n");
    prt_fb2_var_info(orig_var);

    AUI_DBG(" after set variable info, the info changed to: \n");
    prt_fb2_var_info(current_var);  
#endif
    return ret;
}

/**
*    @brief         Open OSD layer
*    @author        peter.pan@alitech.com
*    @date          2014-1-24
*    @param[in]     layer_id            OSD layer ID, this version, only 0 ID OSD layer is used for drawing UI.
*    @param[out]    gfx_layer_handle    OSD layer handle.
*    @return        return AUI_GFX_SUCCESS if success, return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_open(aui_osd_layer layer_id,
								aui_hdl *gfx_layer_handle)
{
    GFX_FUNCTION_ENTER;

    AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
    aui_layer_handle_t *layer = NULL;

    if ((AUI_OSD_LAYER_GMA0 != layer_id) && (AUI_OSD_LAYER_GMA1 != layer_id)) {
        AUI_ERR("invalid layer_id.\n");
        ret = AUI_RTN_EINVAL;
        goto error_out;
    }

    layer = (aui_layer_handle_t *)malloc(sizeof(aui_layer_handle_t));
    if (!layer) {
        AUI_ERR("malloc fail.\n");
        ret = AUI_RTN_ENOMEM;
        goto error_out;
    }
    memset(layer, 0, sizeof(*layer));

    if (DIS_ERR_NONE != alisldis_open(DIS_HD_DEV, &layer->alisl_dis)) {
        AUI_ERR("alisldis_open return fail.\n");
        ret = AUI_RTN_FAIL;
        free(layer);
        goto error_out;
    }

    layer->layer_id = layer_id;
    layer->global_alpha = 0xff;

    if (layer_id == AUI_OSD_LAYER_GMA1) {
        if (AUI_GFX_SUCCESS != init_fb2()) {
            AUI_ERR("init_fb2 fail.\n");
            ret = AUI_RTN_FAIL;
            free(layer);
            goto error_out;
        }
    }

    *gfx_layer_handle = layer;

    layer->data.dev_idx = layer_id;
    aui_dev_reg(AUI_MODULE_GFX, layer);
    
error_out:
    if (ret)
	    AUI_ERR("ret %d\n", ret);
    GFX_FUNCTION_LEAVE;
    return ret;
}


/**
*    @brief         Close OSD layer
*    @author        peter.pan@alitech.com
*    @date          2014-1-24
*    @param[in]     gfx_layer_handle    OSD layer handle
*    @return        return AUI_GFX_SUCCESS if success, return AU_RTN_EINVAL if wrong param.
*    @note          Corresponding to aui_gfx_layer_open API, the application can call this API
*                   to close the GE and GMA. Before call this API, application need to delete
*                   the hardware surface created in this OSD layer.
*
*/
AUI_RTN_CODE aui_gfx_layer_close(aui_hdl gfx_layer_handle)
{
    GFX_FUNCTION_ENTER;

    AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

    if (!gfx_layer_handle) {
        AUI_ERR("invalid gfx_layer_handle\n");
        ret = AUI_RTN_EINVAL;
        goto error_out;
    }

    aui_layer_handle_t *layer = (aui_layer_handle_t *)gfx_layer_handle;

    if (!layer->alisl_dis) {
        ret = AUI_RTN_EINVAL;
        goto error_out;
    }
    if (layer->layer_id == AUI_OSD_LAYER_GMA1) 
        de_init_fb2();
    alisldis_close(layer->alisl_dis);  
    aui_dev_unreg(AUI_MODULE_GFX, layer);
    free(layer);

error_out:
    if (ret)
	    AUI_ERR("ret %d\n", ret);
    GFX_FUNCTION_LEAVE;
    return ret;
}

/**
*    @brief     turn on/off anti-flicker
*    @author    peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] gfx_layer_handle    OSD layer handle
*    @param[in] onoff           "1" on,"0"off
*    @return    return AUI_GFX_SUCCESS if success, return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_antifliker_on_off(aui_hdl gfx_layer_handle,
											 unsigned long on_off)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = 0;
	aui_layer_handle_t *layer = NULL;

	if (!gfx_layer_handle) {
        AUI_ERR("invalid gfx_layer_handle\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	layer = (aui_layer_handle_t *)gfx_layer_handle;

	if (AUI_OSD_LAYER_GMA0 != layer->layer_id) {
        AUI_ERR("only for GMA0.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (DIS_ERR_NONE != alisldis_anti_flicker_onoff(layer->alisl_dis, on_off)) {
        AUI_ERR("alisldis_anti_flicker_onoff fail.\n");
		ret = AUI_RTN_FAIL;
	}

error_out:
	if (ret)
		AUI_ERR("failed err %d\n", ret);
	GFX_FUNCTION_LEAVE;
	return ret;
}

/**
*    @brief     Turn on/off displaying OSD layer
*    @author    peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_hw_surface_handle OSD layer handle
*    @param[in] on_off              "1" on,"0" off
*    @return    return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_show_on_off(aui_hdl gfx_layer_handle,
									   unsigned long on_off)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	aui_layer_handle_t *gfx_layer = NULL;

	if (!gfx_layer_handle) {
        AUI_ERR("invalid gfx_layer_handle\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	gfx_layer = (aui_layer_handle_t *)gfx_layer_handle;

	enum dis_layer sl_layer = (AUI_OSD_LAYER_GMA0 == gfx_layer->layer_id) ? \
							  DIS_LAYER_GMA1 : DIS_LAYER_GMA2;

	if (DIS_ERR_NONE != alisldis_win_onoff_by_layer(gfx_layer->alisl_dis, on_off,
													sl_layer)) {
		AUI_ERR("alisldis_win_onoff_by_layer fail.\n");											
		ret = AUI_RTN_FAIL;
	}

error_out:
	if (ret)
		AUI_ERR("failed err %d\n", ret);
	GFX_FUNCTION_LEAVE;
	return ret;
}

/**
*    @brief     Configure alpha value of OSD layer
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] gfx_layer_handle    OSD layer handle
*    @param[in] alpha           alpha value
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_alpha_set(aui_hdl gfx_layer_handle,
									 unsigned long alpha)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	aui_layer_handle_t *gfx_layer = NULL;

	if (!gfx_layer_handle) {
        AUI_ERR("invalid gfx_layer_handle\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	gfx_layer = (aui_layer_handle_t *)gfx_layer_handle;

	if (AUI_OSD_LAYER_GMA0 != gfx_layer->layer_id) {
        AUI_ERR("only for GMA0.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	gfx_layer->global_alpha = (unsigned char)alpha;
	alisldis_set_global_alpha_by_layer(gfx_layer->alisl_dis, DIS_LAYER_GMA1,
									   gfx_layer->global_alpha);

error_out:
	if (ret)
		AUI_ERR("failed err %d\n", ret);
	return ret;
}

/**
*    @brief     Get alpha value of OSD layer
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] gfx_layer_handle    OSD layer handle
*    @param[out]    alpha           alpha value
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_alpha_get(aui_hdl gfx_layer_handle,
									 unsigned long *alpha)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	aui_layer_handle_t *gfx_layer = NULL;

	if (!gfx_layer_handle) {
        AUI_ERR("invalid gfx_layer_handle\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	gfx_layer = (aui_layer_handle_t *)gfx_layer_handle;

	if (AUI_OSD_LAYER_GMA0 != gfx_layer->layer_id) {
        AUI_ERR("only for GMA0\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	*alpha = gfx_layer->global_alpha;

error_out:
	if (ret)
		AUI_ERR("failed err %d\n", ret);
	return ret;
}

/*
 *	@brief			scale the gma layer, this is useful when change tv system.
 *
 *	@param[in]		gfx_layer_handle	OSD layer handle.
 *	@param[in]		scale_param			scale parameters.
 *
 *	@return			AUI_RTN_CODE
 *
 *	@author			Peter Pan <peter.pan@alitech.com>
 *
 *	@note			
 */
AUI_RTN_CODE aui_gfx_layer_scale(aui_hdl gfx_layer_handle,
								 const aui_scale_param *scale_param)
{
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	struct scale_param slparam;
	aui_layer_handle_t *gfx_layer = NULL;

	if (!gfx_layer_handle) {
        AUI_ERR("invalid gfx_layer_handle\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	gfx_layer = (aui_layer_handle_t *)gfx_layer_handle;

	enum dis_layer sl_layer = (AUI_OSD_LAYER_GMA0 == gfx_layer->layer_id) ? \
							  DIS_LAYER_GMA1 : DIS_LAYER_GMA2;

	slparam.h_dst = scale_param->output_width;
	slparam.h_src = scale_param->input_width;
	slparam.v_dst = scale_param->output_height;
	slparam.v_src = scale_param->input_height;
	if (DIS_ERR_NONE != alisldis_gma_scale(gfx_layer->alisl_dis, &slparam, sl_layer)) {
        AUI_ERR("alisldis_gma_scale fail\n");
		ret = AUI_RTN_FAIL;
	}

error_out:
	if (ret)
	    AUI_ERR("ret %d\n", ret);
	return ret;
}

#ifndef LINUX_AUI_DISABLE_GFX

//#include <directfb.h>
//#include <direct/direct.h>
//#include <direct/types.h>

typedef struct {
	/** hardware surface flag */
	int b_hw_surface;
	/** dobule buffering */
	int b_dobule_buffered;
	/** if show or not */
	int b_show_onoff;
	/** color key */
	aui_color_key color_key;
	/** global alpha */
	unsigned char global_alpha;
	/** clipping mode */
	enum aui_ge_clip_mode clip_mode;
	/** clipping region */
	DFBRegion clip_region;
	/** pallette information */
	aui_pallette_info pallette_info;
	/** buffer address */
	void *lock_address;
	/** pitch */
	int pitch;
    /** gma layer*/
    aui_osd_layer layer_id;

	IDirectFBWindow *dfb_window;
	IDirectFBSurface *dfb_surface;
} aui_surface_handle_t;

static int b_init = 0;
static IDirectFB *gdfb = NULL;
static IDirectFBDisplayLayer *g_dfb_layer = NULL;

static DFBSurfacePixelFormat aui_pixelformat_to_dfb(enum aui_osd_pixel_format e_color_mode)
{
	DFBSurfacePixelFormat format = DSPF_UNKNOWN;

	switch (e_color_mode) {
		case AUI_OSD_4_COLOR:
			format = DSPF_LUT2;
			break;
		case AUI_OSD_256_COLOR:
			format = DSPF_LUT8;
			break;
		case AUI_OSD_16_COLOR_PIXEL_ALPHA:
			format = DSPF_ALUT44;
			break;
		case AUI_OSD_HD_RGB555:
			format = DSPF_RGB555;
			break;
		case AUI_OSD_HD_RGB565:
			format = DSPF_RGB16;
			break;
		case AUI_OSD_HD_RGB444:
			format = DSPF_RGB444;
			break;
		case AUI_OSD_HD_RGB888:
			format = DSPF_RGB24;
			break;
		case AUI_OSD_HD_ARGB8888:
			format = DSPF_ARGB;
			break;
		case AUI_OSD_HD_ARGB1555:
			format = DSPF_ARGB1555;
			break;
		case AUI_OSD_HD_ARGB4444:
			format = DSPF_ARGB4444;
			break;
		case AUI_OSD_HD_AYCbCr8888:
			format = DSPF_AYUV;
			break;
		case AUI_OSD_HD_YCBCR888:
			format = DSPF_YUV444P;
			break;
		case AUI_OSD_HD_YCBCR420MB:
			format = DSPF_I420;
			break;
		default:
			break;
	}

	return format;
}

static enum aui_osd_pixel_format dfb_pixelformat_to_aui(DFBSurfacePixelFormat dfb_color_mode)
{
	enum aui_osd_pixel_format format = AUI_OSD_COLOR_MODE_MAX;

	switch (dfb_color_mode) {
		case DSPF_LUT2:
			format = AUI_OSD_4_COLOR;
			break;
		case DSPF_LUT8:
			format = AUI_OSD_256_COLOR;
			break;
		case DSPF_ALUT44:
			format = AUI_OSD_16_COLOR_PIXEL_ALPHA;
			break;
		case DSPF_RGB555:
			format = AUI_OSD_HD_RGB555;
			break;
		case DSPF_RGB16:
			format = AUI_OSD_HD_RGB565;
			break;
		case DSPF_RGB444:
			format = AUI_OSD_HD_RGB444;
			break;
		case DSPF_RGB24:
			format = AUI_OSD_HD_RGB888;
			break;
		case DSPF_ARGB:
			format = AUI_OSD_HD_ARGB8888;
			break;
		case DSPF_ARGB1555:
			format = AUI_OSD_HD_ARGB1555;
			break;
		case DSPF_ARGB4444:
			format = AUI_OSD_HD_ARGB4444;
			break;
		case DSPF_AYUV:
			format = AUI_OSD_HD_AYCbCr8888;
			break;
		case DSPF_YUV444P:
			format = AUI_OSD_HD_YCBCR888;
			break;
		case DSPF_I420:
			format = AUI_OSD_HD_YCBCR420MB;
			break;
		default:
			break;
	}

	return format;
}

static int linux_aui_gfx_get_argb(unsigned long color,
								  DFBSurfacePixelFormat format,
								  unsigned char global_alpha,
								  unsigned char *A,
								  unsigned char *R,
								  unsigned char *G,
								  unsigned char *B)
{
	int ret = 0;

	switch (format) {
		case DSPF_RGB555:
			*A = global_alpha;
			*R = ((color >> 10) & 0x1f) << 3;
			*G = ((color >> 5) & 0x1f) << 3;
			*B = (color & 0x1f) << 3;
			break;
		/** RGB565 */
		case DSPF_RGB16:
			*A = global_alpha;
			*R = ((color >> 11) & 0x1f) << 3;
			*G = ((color >> 6) & 0x3f) << 2;
			*B = (color & 0x1f) << 3;
			break;
		/** RGB888 */
		case DSPF_RGB24:
			*A = global_alpha;
			*R = (color >> 16) & 0xff;
			*G = (color >> 8) & 0xff;
			*B = color & 0xff;
			break;
		case DSPF_RGB444:
			*A = global_alpha;
			*R = ((color >> 8) & 0xf) << 4;
			*G = ((color >> 4) & 0xf) << 4;
			*B = (color & 0xf) << 4;
			break;
		case DSPF_ARGB:
			*A = (color >> 24) & 0xff;
			*R = (color >> 16) & 0xff;
			*G = (color >> 8) & 0xff;
			*B = color & 0xff;
			break;
		case DSPF_ARGB1555:
			*A = ((color >> 15) & 0x1) * 0xff;
			*R = ((color >> 10) & 0xff) << 3;
			*G = ((color >> 5) & 0xff) << 3;
			*B = (color & 0x1f) << 3;
			break;
		case DSPF_ARGB4444:
			*A = ((color >> 12) & 0xf) << 4;
			*R = ((color >> 8) & 0xf) << 4;
			*G = ((color >> 4) & 0xf) << 4;
			*B = (color & 0xf) << 4;
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

static DFBSurfacePorterDuffRule get_dfb_poterduff_rule(enum aui_ge_alpha_blend_mode alpha_blend_mode)
{
	DFBSurfacePorterDuffRule pd_flag = DSPD_NONE;

	switch(alpha_blend_mode) {
		case AUI_GE_ALPHA_BLEND_SRC_OVER:
			pd_flag |= DSPD_SRC_OVER;
			break;
		case AUI_GE_ALPHA_BLEND_CLEAR:
			pd_flag |= DSPD_CLEAR;
			break;
		case AUI_GE_ALPHA_BLEND_DST:
			pd_flag |= DSPD_DST;
			break;
		case AUI_GE_ALPHA_BLEND_DST_ATOP:
			pd_flag |= DSPD_DST_ATOP;
			break;
		case AUI_GE_ALPHA_BLEND_DST_IN:
			pd_flag |= DSPD_DST_IN;
			break;
		case AUI_GE_ALPHA_BLEND_DST_OUT:
			pd_flag |= DSPD_DST_OUT;
			break;
		case AUI_GE_ALPHA_BLEND_DST_OVER:
			pd_flag |= DSPD_DST_OVER;
			break;
		case AUI_GE_ALPHA_BLEND_SRC:
			pd_flag |= DSPD_SRC;
			break;
		case AUI_GE_ALPHA_BLEND_SRC_ATOP:
			pd_flag |= DSPD_SRC_ATOP;
			break;
		case AUI_GE_ALPHA_BLEND_SRC_IN:
			pd_flag |= DSPD_SRC_IN;
			break;
		case AUI_GE_ALPHA_BLEND_SRC_OUT:
			pd_flag |= DSPD_SRC_OUT;
			break;
		case AUI_GE_ALPHA_BLEND_XOR:
			pd_flag |= DSPD_XOR;
			break;
		default:
			break;
	}

	return pd_flag;
}

static AUI_RTN_CODE linux_aui_gfx_blit(aui_hdl dst_surface_handle,
									   aui_hdl src_surface_handle,
									   aui_blit_operation *blit_operation,
									   aui_blit_rect *blit_rect)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	DFBRectangle src_rect, dst_rect;
	DFBSurfaceBlittingFlags blit_flag = DSBLIT_NOFX;
	DFBSurfacePorterDuffRule pd_flag = DSPD_NONE;
	IDirectFBSurface *dfb_dst = NULL;
	IDirectFBSurface *dfb_src = NULL;
	DFBSurfacePixelFormat format;
	aui_surface_handle_t *p_dst = (aui_surface_handle_t *)dst_surface_handle;
	aui_surface_handle_t *p_src = (aui_surface_handle_t *)src_surface_handle;
	unsigned char b_rotate = 0;
	unsigned char A, R, G, B;

	if (!(dfb_dst = p_dst->dfb_surface) || !(dfb_src = p_src->dfb_surface)) {
        AUI_ERR("invalid params\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	/** set src/dst rect */
	dst_rect.x = blit_rect->dst_rect.uLeft;
	dst_rect.y = blit_rect->dst_rect.uTop;
	dst_rect.h = blit_rect->dst_rect.uHeight;
	dst_rect.w = blit_rect->dst_rect.uWidth;

	src_rect.x = blit_rect->fg_rect.uLeft;
	src_rect.y = blit_rect->fg_rect.uTop;
	src_rect.h = blit_rect->fg_rect.uHeight;
	src_rect.w = blit_rect->fg_rect.uWidth;

	switch(blit_operation->rotate_degree) {
		case 90:
		case -270:
			b_rotate = 1;
			blit_flag |= DSBLIT_ROTATE90;
			break;
		case 180:
		case -180:
			b_rotate = 1;
			blit_flag |= DSBLIT_ROTATE180;
			break;
		case 270:
		case -90:
			b_rotate = 1;
			blit_flag |= DSBLIT_ROTATE270;
			break;
		default:
			break;
	}

	if (!b_rotate) {
		switch (blit_operation->mirror_type) {
			case AUI_GFX_MIRROR_RIGHT_LEFT:
				blit_flag |= DSBLIT_FLIP_HORIZONTAL;
				break;
			case AUI_GFX_MIRROR_BOTTON_TOP:
				blit_flag |= DSBLIT_FLIP_VERTICAL;
				break;
			case AUI_GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT:
				ret = AUI_RTN_EINVAL;
				goto error_out;
				break;
			default:
				break;
		}
	}

	if (AUI_GE_CKEY_DISABLE != blit_operation->color_key_source) {
		switch (blit_operation->color_key_source) {
			case AUI_GE_CKEY_DST:
				dfb_dst->GetPixelFormat(dfb_dst, &format);
				if (0 != linux_aui_gfx_get_argb(p_dst->color_key.color_key_max, format, p_dst->global_alpha,
												&A, &R, &G, &B)) {
					ret = AUI_RTN_EINVAL;
					goto error_out;
				}
				dfb_dst->SetDstColorKey(dfb_dst, R, G, B);
				blit_flag |= DSBLIT_DST_COLORKEY;
				break;
			case AUI_GE_CKEY_PTN_POST_CLUT:
				dfb_src->GetPixelFormat(dfb_src, &format);
				if (0 != linux_aui_gfx_get_argb(p_src->color_key.color_key_max,
												format, p_src->global_alpha, &A, &R, &G, &B)) {
					ret = AUI_RTN_EINVAL;
					goto error_out;
				}
				dfb_src->SetSrcColorKey(dfb_src, R, G, B);
				blit_flag |= DSBLIT_SRC_COLORKEY;
				break;
			case AUI_GE_CKEY_PTN_PRE_CLUT:
				dfb_src->SetSrcColorKeyIndex(dfb_src, p_src->color_key.color_key_max);
				blit_flag |= DSBLIT_SRC_COLORKEY;
				break;
			default:
				break;
		}
	}

	if (AUI_GFX_ROP_ALPHA_BLENDING == blit_operation->rop_operation) {
		blit_flag |= DSBLIT_BLEND_ALPHACHANNEL;
        /* For Support_Flow #43300, do alpha blending with "AUI_GE_ALPHA_BLEND_SRC_OVER", 
         * the result on Linux is different from on TDS. To fix this, need to add blit flag "DSBLIT_SRC_PREMULTIPLY".
        */
        /*
        *   According to different alpha out mode, the fc(color of foreground) is different.
        *   In case the alpha out mode is AUI_ALPHA_OUT_NORMAL, fc = fc * fa, 
        *   and we need to inform GE by set the bit DSBLIT_SRC_PREMULTIPLY.
        */
        if (AUI_ALPHA_OUT_NORMAL == blit_operation->alpha_out_mode)
            blit_flag |= DSBLIT_SRC_PREMULTIPLY;
        /*
            For TASK #58340, for irdeto mw, do alpha blending with "AUI_GE_ALPHA_BLEND_DST_OVER",
            the result is not expected. To fix this, need to add blit flag "DSBLIT_DST_PREMULTIPLY".
        */
        if (AUI_GE_ALPHA_BLEND_DST_OVER == blit_operation->alpha_blend_mode) {
            //AUI_DBG("\n----- dstover, set dst premultiply flag. ----\n\n");
            blit_flag |= DSBLIT_DST_PREMULTIPLY;        
        }
        
		pd_flag |= get_dfb_poterduff_rule(blit_operation->alpha_blend_mode);
	}

	dfb_dst->SetBlittingFlags(dfb_dst, blit_flag);
	dfb_dst->SetPorterDuff(dfb_dst, pd_flag);

	if (AUI_GE_CLIP_DISABLE == p_dst->clip_mode) {
		dfb_dst->StretchBlit(dfb_dst, dfb_src, &src_rect, &dst_rect);
	} else {
		if (AUI_GE_CLIP_INSIDE == p_dst->clip_mode) {
			dfb_dst->SetClip(dfb_dst, &(p_dst->clip_region));
    /*
        *    Update by mikko.ma@20170831, fixed:     http://prj.alitech.com/qt/issues/75504
        *    After set clip region, the auto scale do not work when call data_blit.
        *    Use strechblit instead of blit, since the strechblit suppose to call:
        *    1. blit: src rect = dst rect
        *    2. strechblit: src rect != dst rect
        */
			dfb_dst->StretchBlit(dfb_dst, dfb_src, &src_rect, &dst_rect);
			dfb_dst->SetClip(dfb_dst, NULL);
		} else {
			DFBSurfaceDescription dsc;
			IDirectFBSurface *surface_tmp = NULL;

			dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT;
			dsc.width = blit_rect->dst_rect.uWidth;
			dsc.height = blit_rect->dst_rect.uHeight;
			dfb_dst->GetPixelFormat(dfb_dst, &format);
			dsc.pixelformat = format;
			gdfb->CreateSurface(gdfb, &dsc, &surface_tmp);
			surface_tmp->Blit(surface_tmp, dfb_dst, &dst_rect, 0, 0);

    /*
        *    Update by mikko.ma@20170831, fixed:    http://prj.alitech.com/qt/issues/75504
        *    After set clip region, the auto scale do not work when call data_blit.
        *    Use strechblit instead of blit, since the strechblit suppose to call:
        *    1. blit: src rect = dst rect
        *    2. strechblit: src rect != dst rect
        */
			dfb_dst->StretchBlit(dfb_dst, dfb_src, &src_rect, &dst_rect);

			blit_flag = DSBLIT_NOFX;
			dfb_dst->SetClip(dfb_dst, &(p_dst->clip_region));
			dfb_dst->SetBlittingFlags(dfb_dst, blit_flag);
			dfb_dst->Blit(dfb_dst, surface_tmp, &dst_rect, 0, 0);

			dfb_dst->SetClip(dfb_dst, NULL);
			surface_tmp->Release(surface_tmp);
		}
	}

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

static unsigned long get_pitch(enum aui_osd_pixel_format color_mode, unsigned long width)
{
	unsigned long pitch = 0;

	switch (color_mode)
	{		
		case AUI_OSD_HD_ARGB1555:
		case AUI_OSD_HD_RGB565:
		case AUI_OSD_HD_ARGB4444: // 2 bytes
		case AUI_OSD_HD_RGB444:
		case AUI_OSD_HD_RGB555:
			pitch = width << 1;
			break;
		case AUI_OSD_HD_AYCbCr8888:
		case AUI_OSD_HD_ARGB8888: // 4 bytes
		case AUI_OSD_HD_RGB888: //4 bytes on IC
			pitch = width << 2;
			break;
			
		default: // default 1 byte
			pitch = width;
			break;
	}
	return pitch;
}

/**
*    @brief         initialize OSD module
*    @author        peter.pan@alitech.com
*    @date          2014-1-24
*    @param[in]     p_call_back_init    callback that used for attach ali driver.
*    @param[in]     pv_param        callback param list
*    @return        AUI_GFX_SUCCESS if success, AUI_RTN_EINVAL if wrong param.
*    @note          This function must be called firstly before you call any OSD APIs,and ony need call once when system startup.
*
*/
AUI_RTN_CODE aui_gfx_init(p_fun_cb p_call_back_init,
						  void *pv_param)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	DFBSurfaceDescription desc;
	IDirectFBSurface *dfb_surface = NULL;

	if (b_init) {
        AUI_WARN("already init.\n");
		goto error_out;
	}

	if (NULL != p_call_back_init) {
		p_call_back_init(pv_param);
	}

	if (!gdfb) {
		if (DFB_OK != DirectFBInit(0, 0)) {
            AUI_ERR("DirectFBInit fail\n");
			ret = AUI_RTN_FAIL;
			goto error_out;
		}

		if (DFB_OK != DirectFBCreate(&gdfb)) {
            AUI_ERR("DirectFBCreate fail\n");
			ret = AUI_RTN_FAIL;
			goto error_out;
		}
	}

	/** get the primary surface, and clear it. */
	desc.flags = DSDESC_CAPS;
	desc.caps = DSCAPS_PRIMARY;
	gdfb->CreateSurface(gdfb, &desc, &dfb_surface);
	dfb_surface->Clear(dfb_surface, 0, 0, 0, 0);
	dfb_surface->Flip(dfb_surface, NULL, DSFLIP_NONE);
	dfb_surface->Release(dfb_surface);

	if (!g_dfb_layer) {
		gdfb->GetDisplayLayer(gdfb, DLID_PRIMARY, &g_dfb_layer);
	}

	b_init = 1;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	GFX_FUNCTION_LEAVE;
	return ret;
}

/**
*    @brief         Destroy the AUI structure,free memory and de-initialization other things for OSD working environment.
*    @author        peter.pan@alitech.com
*    @date          2014-1-24
*    @param[in]     p_call_back_init    callback that used for deattach ali driver.
*    @param[in]     pv_param        callback param list
*    @return        return AUI_GFX_SUCCESS if success, return AUI_RTN_EINVAL if wrong param.
*    @note          Corresponding to aui_gfx_init API,Please call this API when you want to close and release resources of OSD module.
*                   Before call this function,you should call aui_gfx_layer_close.
*                   when system going to shutdown,or reboot,this function shuld be called to clear the resources of OSD module.
*/
AUI_RTN_CODE aui_gfx_de_init(p_fun_cb p_call_back_init,
							 void *pv_param)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	if (!b_init) {
		goto error_out;
	}

	if (p_call_back_init) {
		p_call_back_init(pv_param);
	}

	if (g_dfb_layer) {
		if ((DirectResult)DFB_OK != g_dfb_layer->Release(g_dfb_layer)) {
            AUI_ERR("layer Release fail\n");
			ret = AUI_RTN_FAIL;
			goto error_out;
		}
		g_dfb_layer = NULL;
	}

	if (gdfb) {
		if ((DirectResult)DFB_OK != gdfb->Release(gdfb)) {
            AUI_ERR("Release fail\n");
			ret = AUI_RTN_FAIL;
			goto error_out;
		}
		gdfb = NULL;
	}

	g_dfb_layer = NULL;

	b_init = 0;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	GFX_FUNCTION_LEAVE;
	return ret;
}

/**
*    @brief     Create software surface
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] e_color_mode        color space
*    @param[in] width           surface width
*    @param[in] height          surface height
*    @param[out]    p_surface_handle    return software surface handle
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough.
*    @note      Software surface is created for application to manage GE drawing. Such as, application
*               can store the UI material or the results of GE drawing in the software surface, after
*               GE drawing is finished, GE can blit the data of software surface to OSD buffer
*               "Hardware surface" for display purpose.
*/
AUI_RTN_CODE aui_gfx_sw_surface_create(enum aui_osd_pixel_format e_color_mode,
									   unsigned long width,
									   unsigned long height,
									   aui_hdl *p_surface_handle)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	DFBSurfaceDescription surface_dsc;
	IDirectFBSurface *dfb_surface = NULL;
	aui_surface_handle_t *p_surface = NULL;
	void *p_data = NULL;
	int pitch = 0;

	memset(&surface_dsc, 0, sizeof(surface_dsc));
	surface_dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT; //| DSDESC_CAPS;
	surface_dsc.width = width;
	surface_dsc.height = height;
	//surface_dsc.caps = DSCAPS_VIDEOONLY;
	/*
	    update by mikko@20171213, fixed http://project.alitech.com/issues/87989 
	    donnt set caps, it have two case:
	    1. video memory is enough, surface will malloc from video memory( frame buffer memory)
	    2. video memory is not enought( e.g. lite linux ), surface will malloc from system memory 
	*/
	if (DSPF_UNKNOWN == (surface_dsc.pixelformat = aui_pixelformat_to_dfb(e_color_mode))) {
        AUI_ERR("unknown pixelformat.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (DFB_OK != gdfb->CreateSurface(gdfb, &surface_dsc, &dfb_surface)) {
        AUI_ERR("CreateSurface fail.\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
	}
	//dfb_surface->Clear(dfb_surface, 0x0, 0x0, 0x0, 0x0);

	if (DFB_OK != dfb_surface->Lock(dfb_surface, DSLF_READ, &p_data, &pitch)) {
        AUI_ERR("Lock fail.\n");
        ret = AUI_RTN_FAIL;
		goto error_out;
    }
	dfb_surface->Unlock(dfb_surface);
    dfb_surface->Clear(dfb_surface, 0x0, 0x0, 0x0, 0x0);

	p_surface = malloc(sizeof(aui_surface_handle_t));
	if (!p_surface) {
        AUI_ERR("malloc fail.\n");
		ret = AUI_RTN_ENOMEM;
		goto error_out;
	}
	memset(p_surface, 0, sizeof(aui_surface_handle_t));
	p_surface->b_hw_surface = 0;
	p_surface->dfb_surface = dfb_surface;
	p_surface->global_alpha = 0xff;
	p_surface->pitch = pitch;

	*p_surface_handle = p_surface;
    return ret;
error_out:
    if (dfb_surface) {
        dfb_surface->Release(dfb_surface);
        dfb_surface = NULL;
    }
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
 *  @brief          create a software surface by use preallocated bitmap source
 *
 *  @param[out]     p_surface_handle        returned surface handle
 *  @param[in]      p_bitmap_info           bitmap information
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/17/2014  14:29:17
 *
 *  @note
 */
AUI_RTN_CODE aui_gfx_sw_surface_create_by_bitmap(aui_hdl *p_surface_handle,
												 const aui_gfx_bitmap_info_t *p_bitmap_info)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	DFBSurfaceDescription surface_dsc;
	IDirectFBSurface *dfb_surface = NULL;
	aui_surface_handle_t *p_surface = NULL;

	if (!p_bitmap_info) {
        AUI_ERR("invalid param.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	memset(&surface_dsc, 0, sizeof(surface_dsc));
	surface_dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_PREALLOCATED;
	surface_dsc.width = p_bitmap_info->width;
	surface_dsc.height = p_bitmap_info->height;
	if (DSPF_UNKNOWN == (surface_dsc.pixelformat = aui_pixelformat_to_dfb(p_bitmap_info->color_type))) {
        AUI_ERR("unknown pixelformat.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}
	surface_dsc.preallocated[0].data = p_bitmap_info->p_data;
	surface_dsc.preallocated[0].pitch = p_bitmap_info->pitch;
	surface_dsc.preallocated[1].data = NULL;
	surface_dsc.preallocated[1].pitch = 0;

	if (DFB_OK != (gdfb->CreateSurface(gdfb, &surface_dsc, &dfb_surface))) {
        AUI_ERR("CreateSurface fail.\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
	}

	p_surface = malloc(sizeof(aui_surface_handle_t));
	if (!p_surface) {
        AUI_ERR("malloc fail\n");
		ret = AUI_RTN_ENOMEM;
		goto error_out;
	}
	memset(p_surface, 0, sizeof(aui_surface_handle_t));
	p_surface->b_hw_surface = 0;
	p_surface->dfb_surface = dfb_surface;
	p_surface->global_alpha = 0xff;
	p_surface->pitch = p_bitmap_info->pitch;

	*p_surface_handle = p_surface;
    return ret;
error_out:
    if (dfb_surface) {
        dfb_surface->Release(dfb_surface);
        dfb_surface = NULL;
    }
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     Create hardware surface
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] gfx_layer_handle    the handle of the OSD layer where application want create hardware surface.
*    @param[in] e_color_mode        color space
*    @param[in] rect                rectangular area of hardware surface
*    @param[in] is_double_buf       enable/disenable double buffer. 1-enable; 0-disable.
*    @param[out]    p_hw_surface_handle return hardware surface handle
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GMA work error.
*    @note      Hardware surface is used to manage the OSD buffer which will be directly used in the OSD layer for display purpose.
*               The application can create several hardware surface, but can't be overlap in row.
*
*/
AUI_RTN_CODE aui_gfx_hw_surface_create(aui_hdl gfx_layer_handle,
									   enum aui_osd_pixel_format e_color_mode,
									   struct aui_osd_rect* rect,
									   aui_hdl *p_hw_surface_handle,
									   unsigned long is_double_buf)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	DFBWindowDescription win_desc;
	IDirectFBWindow *dfb_window = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	aui_surface_handle_t *p_surface = NULL;
	IDirectFBDisplayLayer *dfb_layer = NULL;

	//(void)gfx_layer_handle; // unused
	aui_layer_handle_t *layer = NULL;

	if (!gfx_layer_handle) {
        AUI_ERR("gfx_layer_handle is null, call aui_gfx_init first.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	layer = (aui_layer_handle_t *)gfx_layer_handle;

	if (AUI_OSD_LAYER_GMA0 == layer->layer_id) {
    	dfb_layer = g_dfb_layer;

    	win_desc.flags = DWDESC_WIDTH | DWDESC_HEIGHT | DWDESC_POSX |
    					 DWDESC_POSY | DWDESC_PIXELFORMAT;
    	win_desc.width = rect->uWidth;
    	win_desc.height = rect->uHeight;
    	win_desc.posx = rect->uLeft;
    	win_desc.posy = rect->uTop;
    	win_desc.pixelformat = aui_pixelformat_to_dfb(e_color_mode);
    	dfb_layer->CreateWindow(dfb_layer, &win_desc, &dfb_window);
    	if (!dfb_window) {
            AUI_ERR("CreateWindow fail.\n");
    		ret = AUI_RTN_FAIL;
    		goto error_out;
    	}

    	dfb_window->GetSurface(dfb_window, &dfb_surface);
    	dfb_surface->Clear(dfb_surface, 0x0, 0x0, 0x0, 0xff);

    	dfb_window->SetColor(dfb_window, 0, 0, 0, 0xff);
    	dfb_window->SetOpacity(dfb_window, 0xff);

    	p_surface = malloc(sizeof(aui_surface_handle_t));
    	if (!p_surface) {
            AUI_ERR("malloc fail.\n");
    		ret = AUI_RTN_ENOMEM;
    		goto error_out;
    	}
    	memset(p_surface, 0, sizeof(aui_surface_handle_t));
    	p_surface->b_hw_surface = 1;
    	p_surface->dfb_surface = dfb_surface;
    	p_surface->dfb_window = dfb_window;
    	p_surface->b_dobule_buffered = (int)is_double_buf;
    	p_surface->b_show_onoff = 1;
    	p_surface->global_alpha = 0xff;
        p_surface->layer_id = layer->layer_id;

    	void *p_data = NULL;
    	int pitch;
    	dfb_surface->Lock(dfb_surface, DSLF_READ, &p_data, &pitch);
    	dfb_surface->Unlock(dfb_surface);

    	p_surface->pitch = pitch;
	} else {
        // Create sw surface with base address of fb2,
        // and then the data write to this sw surface would shown on screen at once.

        if (0 != set_fb2_mode(e_color_mode)) {
            AUI_ERR("set GMA1 mode fail.\n");
            ret = AUI_RTN_EINVAL;
            goto error_out;
        }

        // 1. check the params
        if (rect->uLeft + rect->uWidth > current_var.xres
            || rect->uTop + rect->uHeight > current_var.yres) {
            AUI_ERR("%s: out of range of fb2: %d * %d -> %d * %d!\n", __func__, rect->uLeft + rect->uWidth, rect->uTop + rect->uHeight,current_var.xres, current_var.yres);
            ret = AUI_RTN_EINVAL;
            goto error_out;
        }

        DFBSurfaceDescription surface_dsc;
        //IDirectFBSurface *dfb_surface = NULL;
        unsigned long pitch = get_pitch(e_color_mode, current_var.xres);
        memset(&surface_dsc, 0, sizeof(surface_dsc));        
        surface_dsc.flags = DSDESC_CAPS|DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_PREALLOCATED;
        surface_dsc.width = rect->uWidth;
        surface_dsc.height = rect->uHeight;
        surface_dsc.caps = DSCAPS_SYSTEMONLY;// to tell dfb to fall back to software
        if (DSPF_UNKNOWN == (surface_dsc.pixelformat = aui_pixelformat_to_dfb(e_color_mode))) {
            ret = AUI_RTN_EINVAL;
            goto error_out;
        }
        if (NULL == g_fb2_addr) {
            ret = AUI_RTN_FAIL;
            goto error_out;
        }

        /* we need to calculate the offset to fb2's start address according to the rect->uTop and rect->uLeft */
        unsigned int offset = 0; 
        unsigned int bytes_per_pixel = current_var.bits_per_pixel / 8;
        offset = rect->uTop * current_var.xres * bytes_per_pixel + rect->uLeft * bytes_per_pixel;
        surface_dsc.preallocated[0].data = g_fb2_addr + offset; //fb2 address
        surface_dsc.preallocated[0].pitch = pitch;
        surface_dsc.preallocated[1].data = NULL;
        surface_dsc.preallocated[1].pitch = 0;

        AUI_DBG(" surface_dsc.preallocated[0].data: 0x%08x\n", (unsigned int)(surface_dsc.preallocated[0].data));

        if (DFB_OK != (gdfb->CreateSurface(gdfb, &surface_dsc, &dfb_surface))) {
            AUI_ERR("CreateSurface fail\n");
            ret = AUI_RTN_FAIL;
            goto error_out;
        }
        p_surface = malloc(sizeof(aui_surface_handle_t));
        if (!p_surface) {
            AUI_ERR("malloc fail.\n");
            ret = AUI_RTN_ENOMEM;
            goto error_out;
        }
        memset(p_surface, 0, sizeof(aui_surface_handle_t));
        p_surface->b_hw_surface = 0;
        p_surface->dfb_surface = dfb_surface;
        p_surface->global_alpha = 0xff;
        p_surface->layer_id = layer->layer_id;
        p_surface->pitch = pitch;    
    }

	*p_hw_surface_handle = p_surface;
    return ret;
error_out:
    if (dfb_surface) {
        dfb_surface->Release(dfb_surface);
        dfb_surface = NULL;
    }
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     delete surface, include hardware surface or software surface.
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle    surface handle
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_delete(aui_hdl p_surface_handle)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = (aui_surface_handle_t *)p_surface_handle;
	if (!p_surface) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (!p_surface->b_hw_surface) {
		if (p_surface->dfb_surface) {
			if ((DirectResult)DFB_OK != (p_surface->dfb_surface)->Release(p_surface->dfb_surface)) {
				ret = AUI_RTN_FAIL;
				goto error_out;
			}
		}
	} else {
		if (p_surface->dfb_window)
			(p_surface->dfb_window)->Release(p_surface->dfb_window);
		if (p_surface->dfb_surface)
			(p_surface->dfb_surface)->Release(p_surface->dfb_surface);
	}

	free(p_surface);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     surface blit operation
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] dst_surface_handle  destination surface
*    @param[in] fg_surface_handle       foreground surface
*    @param[in] bg_surface_handle       background surface, it is valid when the operation is boolean or alpha blend.
*                                   background surface can be the same surface as destination. If background surface is null,when the
*                                   operation is boolean or alpha blend, the destination surface will be used as background surface.
*    @param[in] blit_param          configuration of blit operation.
*    @param[in] blit_rect               area for blit operation.
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*               return AUI_GFX_NOT_SUPPORTED if IC can't support this feature.
*    @note      If the sizes of rectangular area are different between foreground, background and destination,
*               scaling will be executed firstly to make size of the rectangle in foreground and background the
*               same. This scaling operation will be done by ALi GE automatically, application does not need to
*               take care of this. But this scaling operation can only be supported when OSD is set to ARGB888 mode.
*
*/
AUI_RTN_CODE aui_gfx_surface_blit(aui_hdl dst_surface_handle,
								  aui_hdl fg_surface_handle,
								  aui_hdl bg_surface_handle,
								  aui_blit_operation *blit_operation,
								  aui_blit_rect *blit_rect)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	IDirectFBSurface *dfb_dst = NULL;
	IDirectFBSurface *dfb_bg = NULL;
	DFBSurfaceBlittingFlags blit_flag = DSBLIT_NOFX;
	DFBRectangle src_rect, dst_rect;
	aui_surface_handle_t *p_dst = NULL;
	aui_surface_handle_t *p_bg = NULL;

	if (!dst_surface_handle || !fg_surface_handle || !blit_operation || !blit_rect) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}
	if ((AUI_GFX_ROP_BOOLEAN == blit_operation->rop_operation) ||
		(AUI_GFX_ROP_BOOL_ALPHA_BLENDIN == blit_operation->rop_operation) ||
		(AUI_ALPHA_OUT_NORMAL != blit_operation->alpha_out_mode
		 && AUI_ALPHA_OUT_BLENDING_FG_GLOBAL_ONLY != blit_operation->alpha_out_mode)) {
		AUI_ERR("some blit_operation is not support.\n");
		ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
		goto error_out;
	}

	if (bg_surface_handle && (bg_surface_handle != dst_surface_handle)) {
		p_dst = (aui_surface_handle_t *)dst_surface_handle;
        p_bg = (aui_surface_handle_t *)bg_surface_handle;
		dfb_dst = p_dst->dfb_surface;
		dfb_bg = p_bg->dfb_surface;

		dfb_dst->SetBlittingFlags(dfb_dst, blit_flag);

		/** set src/dst rect */
		dst_rect.x = blit_rect->dst_rect.uLeft;
		dst_rect.y = blit_rect->dst_rect.uTop;
		dst_rect.h = blit_rect->dst_rect.uHeight;
		dst_rect.w = blit_rect->dst_rect.uWidth;

		src_rect.x = blit_rect->bg_rect.uLeft;
		src_rect.y = blit_rect->bg_rect.uTop;
		src_rect.h = blit_rect->bg_rect.uHeight;
		src_rect.w = blit_rect->bg_rect.uWidth;
		dfb_dst->StretchBlit(dfb_dst, dfb_bg, &src_rect, &dst_rect);
	}

	ret = linux_aui_gfx_blit(dst_surface_handle, fg_surface_handle, blit_operation,
							 blit_rect);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     blit data to surface
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] dst_surface_handle  destination surface.
*    @param[in] p_data              data buffer pointer.
*    @param[in] blit_rect               rectangular area in destination surface,rectangular area in source surface.
*    @param[in] blit_operation          configuration of blit operation
*    @param[in] pitch               number of the bytes in one row
*    @param[in] en_color_mode       color space of the data
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GEwork error.
*               return AUI_GFX_NOT_SUPPORTED if IC can't support this feature.
*    @note      API "aui_gfx_data_blit" is used for blting the pure data to the destination surface. This
*               operation does not need to creating surfaces, therefore this operation can be used to copy
*               UI data to the surfaces.
*/
AUI_RTN_CODE aui_gfx_data_blit(aui_hdl dst_surface_handle,
							   unsigned char *p_data,
							   aui_blit_rect *blit_rect,
							   aui_blit_operation *blit_operation,
							   unsigned long pitch,
							   enum aui_osd_pixel_format en_color_mode)
{
    GFX_FUNCTION_ENTER;

    AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
    int size_per_pix = 0;
    aui_surface_handle_t *p_src = NULL;
    aui_gfx_bitmap_info_t p_bitmap;

    if (!p_data || !blit_rect || !blit_operation) {
        AUI_ERR("invalid params\n");
        ret = AUI_RTN_EINVAL;
        goto error_out;
    }

    size_per_pix = get_bytes_per_pixel(en_color_mode) ;
    if (size_per_pix <= 1){
        /*
         * Modified by Fawn.Fu@20180627, for #95229.
         * Do not support do data blit for CLUT8, because we can not set the palette 
         * for the p_src surface created with the data.
         */
        AUI_ERR("do not support this color mode: %d\n", en_color_mode);
        ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
        goto error_out;
    }

    /** create source surface */
    /*to unify the rectange of p_data and surface rectange*/
    p_bitmap.color_type = en_color_mode;
    p_bitmap.height = blit_rect->fg_rect.uHeight + blit_rect->fg_rect.uTop;
    p_bitmap.pitch = pitch;
    p_bitmap.width = pitch / size_per_pix;
    p_bitmap.p_data = p_data;

	if (AUI_GFX_SUCCESS != aui_gfx_sw_surface_create_by_bitmap((aui_hdl *)(&p_src),
															   &p_bitmap)) {
        AUI_ERR("aui_gfx_sw_surface_create_by_bitmap fail.\n");
        ret = AUI_GFX_DRIVER_ERROR;
		goto error_out;
	}

	if (AUI_GFX_SUCCESS != (ret = linux_aui_gfx_blit(dst_surface_handle, p_src,
													 blit_operation, blit_rect))) {
		//goto error_out;
		AUI_ERR("linux_aui_gfx_blit fail.\n");
	}

	aui_gfx_surface_delete(p_src);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief         capture the data in the surface
*    @author        peter.pan@alitech.com
*    @date          2014-1-24
*    @param[in]     p_surface_handle        surface handle
*    @param[in]     rect                    the rectangular area for capturing data
*    @param[out]    pdata                   output buffer
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*                   return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*                   return AUI_GFX_NOT_SUPPORTED if IC can't support this feature.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_capture(aui_hdl p_surface_handle,
									 struct aui_osd_rect * rect,
									 void *p_data)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	void *surface_data = NULL;
	int pitch = 0, width = 0, height = 0, i = 0, size_per_pix = 0;
	aui_coordinate line_start;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !p_data || !rect) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
        AUI_ERR("dfb_surface is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface->GetSize(dfb_surface, &width, &height);
	if (dfb_surface->Lock(dfb_surface, DSLF_READ, &surface_data, &pitch)) {
        AUI_ERR("Lock fail.\n");
        ret = AUI_GFX_DRIVER_ERROR;
		goto error_out;
    }
	size_per_pix = pitch / width;

	for (i = 0; i < rect->uHeight; i ++) {
		line_start.X = rect->uLeft;
		line_start.Y = rect->uTop + i;

		memcpy((void *)(p_data + (i * rect->uWidth * size_per_pix)), (void *)(surface_data + \
				((width * line_start.Y) + line_start.X) * size_per_pix), rect->uWidth * size_per_pix);
	}
	dfb_surface->Unlock(dfb_surface);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     fill surface with solid color
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] surface_handle  destination surface handle
*    @param[in] color               color for filling
*    @param[in] rect                rectangular area for filling
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_fill(aui_hdl surface_handle,
								  unsigned long color,
								  struct aui_osd_rect * rect)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	DFBSurfacePixelFormat format;
	int width, height;
	unsigned char A, R, G, B;

	p_surface = (aui_surface_handle_t *)surface_handle;

	if (!p_surface || !rect) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
        AUI_ERR("dfb_surface is NULL.\n");        
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface->GetSize(dfb_surface, &width, &height);

	if ((rect->uWidth > width) || (rect->uHeight > height)) {
        AUI_ERR("out of region.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface->GetPixelFormat(dfb_surface, &format);
	if ((DSPF_LUT8 == format) || (DSPF_LUT2 == format) || (DSPF_ALUT44 == format)) {
		dfb_surface->SetColorIndex(dfb_surface, color);
	} else {
		if (0 != linux_aui_gfx_get_argb(color, format, p_surface->global_alpha, &A, &R, &G, &B)) {
            AUI_ERR("unknow color format.\n");
			ret = AUI_GFX_FEATURE_NOT_SUPPORTED;
			goto error_out;
		}
		dfb_surface->SetColor(dfb_surface, R, G, B, A);
	}

	if (DFB_OK != dfb_surface->FillRectangle(dfb_surface, rect->uLeft, rect->uTop,
											 rect->uWidth, rect->uHeight)) {
		AUI_ERR("fill rectangle fail!\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
	}

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     Draw a line in the horizontal direction and the vertical direction with specified color,
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] surface_handle      destination surface handle
*    @param[in] color                   specified color
*    @param[in] start_coordinate        line start point coordinate
*    @param[in] end_coordinate      line end point coordinate
*    @param[out]    NULL
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_draw_line(aui_hdl surface_handle,
									   unsigned long color,
									   aui_coordinate *start_coordinate,
									   aui_coordinate *end_coordinate)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	DFBSurfacePixelFormat format;
	unsigned char A, R, G, B;

	p_surface = (aui_surface_handle_t *)surface_handle;

	if (!p_surface || !start_coordinate || !end_coordinate) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
        AUI_ERR("dfb_surface is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface->GetPixelFormat(dfb_surface, &format);
	if ((DSPF_LUT8 == format) || (DSPF_LUT2 == format) || (DSPF_ALUT44 == format)) {
		dfb_surface->SetColorIndex(dfb_surface, color);
	} else {
		if (0 != linux_aui_gfx_get_argb(color, format, p_surface->global_alpha, &A, &R, &G, &B)) {
            AUI_ERR("unknow color format.\n");
			ret = AUI_RTN_EINVAL;
			goto error_out;
		}
		dfb_surface->SetColor(dfb_surface, R, G, B, A);
	}
	dfb_surface->DrawLine(dfb_surface, start_coordinate->X, start_coordinate->Y,
						  end_coordinate->X, end_coordinate->Y);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief         draw rectangle with specified color
*    @author            peter.pan@alitech.com
*    @date          2014-1-24
*    @param[in]     surface_handle  destination surface handle
*    @param[in]     fore_color      border color
*    @param[in]     back_color      filling color
*    @param[in]     rect                rectangular area
*    @param[in]     fill_background filling flag; 1-fill, 0-not fill;
*    @return            AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*                           AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_draw_outline(aui_hdl surface_handle,
										  unsigned long fore_color,
										  unsigned long back_color,
										  struct aui_osd_rect * rect,
										  unsigned long fill_background)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	DFBSurfacePixelFormat format;
	int width, height;
	unsigned char A, R, G, B;

	p_surface = (aui_surface_handle_t *)surface_handle;

	if (!p_surface || !rect) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
        AUI_ERR("dfb_surface is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface->GetSize(dfb_surface, &width, &height);

	if (((rect->uLeft + rect->uWidth) > width) || (rect->uTop + rect->uHeight) > height) {
        AUI_ERR("out of region.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	/** step1. fill rectangle */
	dfb_surface->GetPixelFormat(dfb_surface, &format);
	if (fill_background) {
		if ((DSPF_LUT8 == format) || (DSPF_LUT2 == format) || (DSPF_ALUT44 == format)) {
			dfb_surface->SetColorIndex(dfb_surface, back_color);
		} else {
			if (0 != linux_aui_gfx_get_argb(back_color, format, p_surface->global_alpha, &A, &R, &G, &B)) {
                AUI_ERR("unknown color format.\n");
				ret = AUI_RTN_EINVAL;
				goto error_out;
			}
			dfb_surface->SetColor(dfb_surface, R, G, B, A);
		}
		dfb_surface->FillRectangle(dfb_surface, rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);
	}

	/** step2. draw rectangle */
	if ((DSPF_LUT8 == format) || (DSPF_LUT2 == format) || (DSPF_ALUT44 == format)) {
		dfb_surface->SetColorIndex(dfb_surface, fore_color);
	} else {
		if (0 != linux_aui_gfx_get_argb(fore_color, format, p_surface->global_alpha, &A, &R, &G, &B)) {
            AUI_ERR("unknown color format.\n");
			ret = AUI_RTN_EINVAL;
			goto error_out;
		}
		dfb_surface->SetColor(dfb_surface, R, G, B, A);
	}
	dfb_surface->DrawRectangle(dfb_surface, rect->uLeft, rect->uTop, rect->uWidth, rect->uHeight);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     flush the data in backup OSD buffer to OSD buffer.
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] dst_surface_handle  hardware surface handle
*    @param[in] dst_rect                the rectangular area for flushing, it can't be out of the range of the surface
*    @param[out]    NULL
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*    @note      it is valid when the surface is hardware surface which enable double_buff. Otherwise it return
*               success directly with doing nothing.
*/
AUI_RTN_CODE aui_gfx_surface_flush(aui_hdl dst_surface_handle,
								   struct aui_osd_rect *dst_rect)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	DFBRegion region;
	int width, height;

	p_surface = (aui_surface_handle_t *)dst_surface_handle;

	if (!p_surface) {
        AUI_ERR("dst_surface_handle is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
        AUI_ERR("dfb_surface is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (!dst_rect || !(dst_rect->uHeight) || !(dst_rect->uWidth)) {
		dfb_surface->GetSize(dfb_surface, &width, &height);

		region.x1 = 0;
		region.y1 = 0;
		region.x2 = width - 1;
		region.y2 = height - 1;
	} else {
		region.x1 = dst_rect->uLeft;
		region.y1 = dst_rect->uTop;
		region.x2 = dst_rect->uLeft + dst_rect->uWidth - 1;
		region.y2 = dst_rect->uTop + dst_rect->uHeight - 1;
	}

	dfb_surface->Flip(dfb_surface, &region, DSFLIP_NONE);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}
/**
*    @brief     sync surface
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] dst_surface_handle  surface handle
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_sync(aui_hdl dst_surface_handle)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	if (!dst_surface_handle) {
		ret = AUI_RTN_EINVAL;
	}

	return ret;
}

/**
 *  @brief          lock a surface, get the buffer address
 *
 *  @param[in]      p_surface_handle        the handle of the suface wait to locked
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           3/28/2014  15:13:32
 *
 *  @note           This must be done if you want get the surface buffer by call aui_gfx_surface_info_get.
 */
AUI_RTN_CODE aui_gfx_surface_lock(aui_hdl p_surface_handle)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	int pitch;
	void *p_data = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;
	if (!p_surface) {
        AUI_ERR("p_surface_handle is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
        AUI_ERR("dfb_surface is NULL.\n");
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (p_surface->lock_address) {
		goto error_out;
	}

	if (DFB_OK != dfb_surface->Lock(dfb_surface, DSLF_WRITE, &p_data, &pitch)) {
        AUI_ERR("Lock fail.\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
	}

	p_surface->lock_address = p_data;
	p_surface->pitch = pitch;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
 *  @brief          unlock a surface
 *
 *  @param[in]      p_surface_handle        the handle of the suface wait to unlocked
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           3/28/2014  15:13:32
 *
 *  @note
 */
AUI_RTN_CODE aui_gfx_surface_unlock(aui_hdl p_surface_handle)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;
	if (!p_surface) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (!(p_surface->lock_address)) {
		goto error_out;
	}

	dfb_surface->Unlock(dfb_surface);
	p_surface->lock_address = NULL;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     get surface information
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[out]    p_surface_info      surface information structure pointer
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_info_get(aui_hdl p_surface_handle,
									  aui_surface_info *p_surface_info)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	DFBRegion region;
	DFBSurfacePixelFormat format;
	int width, height;

	p_surface = (aui_surface_handle_t *)p_surface_handle;
	if (!p_surface || !p_surface_info) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	dfb_surface = p_surface->dfb_surface;
	if (!dfb_surface) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	/** is hardware surface */
	p_surface_info->is_hw_surface = p_surface->b_hw_surface;

	/** get size */
	dfb_surface->GetSize(dfb_surface, &width, &height);
	p_surface_info->width = width;
	p_surface_info->height = height;

	/** get color mode */
	dfb_surface->GetPixelFormat(dfb_surface, &format);
	p_surface_info->en_color_mode = dfb_pixelformat_to_aui(format);

	p_surface_info->show_onoff = p_surface->b_show_onoff;
	p_surface_info->color_key = p_surface->color_key;
	p_surface_info->golbal_alpha = p_surface->global_alpha;

	/** get pitch */
	p_surface_info->pitch = p_surface->pitch;
	p_surface_info->p_surface_buf = p_surface->lock_address;
	p_surface_info->p_surface_buf_backup = p_surface->lock_address;

	p_surface_info->palette = p_surface->pallette_info.p_pallette;

	dfb_surface->GetClip(dfb_surface, &region);
	p_surface_info->clip_rect.uLeft = region.x1;
	p_surface_info->clip_rect.uTop = region.y1;
	p_surface_info->clip_rect.uWidth = region.x2 - region.x1;
	p_surface_info->clip_rect.uHeight = region.y2 - region.y1;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     turn on/off showing surface
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_hw_surface_handle surface handle
*    @param[in] on_off              1-show; 0-hide
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_show_on_off(aui_hdl p_hw_surface_handle,
										 unsigned long on_off)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBWindow *dfb_window = NULL;
	unsigned char alpha = 0;

	p_surface = (aui_surface_handle_t *)p_hw_surface_handle;
	if (!p_surface || !(p_surface->b_hw_surface)) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}
	dfb_window = p_surface->dfb_window;

	alpha = on_off ? p_surface->global_alpha : 0;
	dfb_window->SetOpacity(dfb_window, alpha);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     configuration for scaling hardware surface
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_hw_surface_handle hardware surface handle
*    @param[in] scale_param         scaling param
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*               return AUI_RTN_ENOMEM if memory is not enough. return AUI_ERR if GE work error.
*    @note      The surface handle can only be hardware surface. When the surface for displaying UI isn't fit to
*               the screen, the application can call this API to scale the surface.
*               for example surface size is 1920*1080, and the TV system is 720*576(PAL), the application need scale OSD output
*               to 720*576 by calling this API, please configure as the following\n
*               aui_scale_param->input_width=1920;aui_scale_param->input_height=1080;\n
*               aui_scale_param->output_width=720;aui_scale_param->output_height=576;
*
*/
AUI_RTN_CODE aui_gfx_surface_scale_param_set(aui_hdl p_hw_surface_handle,
											 aui_scale_param *scale_param)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBWindow *dfb_window = NULL;

	p_surface = (aui_surface_handle_t *)p_hw_surface_handle;
	if (!p_surface || !(p_surface->b_hw_surface)) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}
	dfb_window = p_surface->dfb_window;

	dfb_window->Resize(dfb_window, (int)(scale_param->output_width),
					   (int)(scale_param->output_height));

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     configure surface color key
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[in] colorkey                the range of color key
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note      IC with GE can support color key
*
*/
AUI_RTN_CODE aui_gfx_surface_colorkey_set(aui_hdl p_surface_handle,
										  aui_color_key *color_key)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !color_key) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	memcpy(&(p_surface->color_key), color_key, sizeof(aui_color_key));

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     get surface color key
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle: surface handle
*    @param[out]    colorkey: color key structure pointer for output
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*    @note      IC with GE can support color key
*
*/
AUI_RTN_CODE aui_gfx_surface_colorkey_get(aui_hdl p_surface_handle,
										  aui_color_key *colorkey)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !colorkey) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	memcpy(colorkey, &(p_surface->color_key), sizeof(aui_color_key));

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     set clipping rectangle
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[in] rect                    rectangular area for clipping
*    @param[in] clip_mode           clipping mode - in range, out of range
*    @param[out]    NULL
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_clip_rect_set(aui_hdl p_surface_handle,
										   struct aui_osd_rect* rect,
										   enum aui_ge_clip_mode clip_mode)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !rect) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	p_surface->clip_region.x1 = rect->uLeft;
	p_surface->clip_region.y1 = rect->uTop;
	p_surface->clip_region.x2 = rect->uLeft + rect->uWidth;
	p_surface->clip_region.y2 = rect->uTop + rect->uHeight;

	p_surface->clip_mode = clip_mode;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     clear clipping rectangle
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_clip_rect_clear(aui_hdl p_surface_handle)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	memset(&(p_surface->clip_region), 0, sizeof(DFBRegion));
	p_surface->clip_mode = AUI_GE_CLIP_DISABLE;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     configure palette
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[in] p_pallette_info     palette information
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note      in the chipsets without GE, the palette is valid when the surface is hardware surface.
*
*/
AUI_RTN_CODE aui_gfx_surface_pallette_set(aui_hdl p_surface_handle,
										  aui_pallette_info *p_pallette_info)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	DFBPaletteDescription desc;
	IDirectFBPalette *dfb_palette = NULL;
	IDirectFBSurface *dfb_surface = NULL;
	unsigned int *p_data = NULL;
	DFBColor *entries = NULL;
	unsigned long i;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !p_pallette_info ||
		(AUI_PALLETTE_COLOR_TYPE_RGB != p_pallette_info->en_pallette_color_type)) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

    
    p_data = (unsigned int *)p_pallette_info->p_pallette;
	dfb_surface = p_surface->dfb_surface;

	memcpy((void *)(&(p_surface->pallette_info)), (void *)p_pallette_info,
		   sizeof(aui_pallette_info));

	entries = malloc(sizeof(DFBColor) * p_pallette_info->color_cnt);
	if (!entries) {
		ret = AUI_RTN_ENOMEM;
		goto error_out;
	}
	memset(entries, 0, sizeof(DFBColor) * p_pallette_info->color_cnt);
	for (i = 0; i < p_pallette_info->color_cnt; i ++) {
		entries[i].a = (p_data[i] >> 24) & 0xff;
		entries[i].r = (p_data[i] >> 16) & 0xff;
		entries[i].g = (p_data[i] >> 8) & 0xff;
		entries[i].b = p_data[i] & 0xff;
	}

	memset(&desc, 0, sizeof(desc));
	desc.flags = DPDESC_SIZE | DPDESC_ENTRIES;
	desc.size = p_pallette_info->color_cnt;
	desc.entries = entries;

	if (DFB_OK != gdfb->CreatePalette(gdfb, &desc, &dfb_palette)) {
		AUI_ERR("create palette fail!\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
	}
	if (DFB_OK != dfb_surface->SetPalette(dfb_surface, dfb_palette)) {
		AUI_ERR("set palette fail!\n");
		ret = AUI_RTN_FAIL;
        dfb_palette->Release(dfb_palette);
		goto error_out;
	}
    dfb_palette->Release(dfb_palette);

    if (p_surface->layer_id == AUI_OSD_LAYER_GMA1) {
        // if the surface is based on fb2, 
        // then we need to do more to set the palette directly into the driver.
        current_cmap.len = p_pallette_info->color_cnt <= 256 ? p_pallette_info->color_cnt : 256;
        p_data = (unsigned int *)p_pallette_info->p_pallette;
        for (i = 0; i < (unsigned int)current_cmap.len; i++) {
            current_cmap.red[i]     = (p_data[i] >> 16) & 0xff;
            current_cmap.green[i]   = (p_data[i] >> 8) & 0xff;
            current_cmap.blue[i]    = p_data[i] & 0xff;
            current_cmap.transp[i]  = (p_data[i] >> 24) & 0xff;
        }
        if (ioctl( fb2_fd, FBIOPUTCMAP, &current_cmap ) < 0) {
            AUI_ERR( "AUI GFX: Could not set the palette for fb2!\n" );
            ret = AUI_RTN_FAIL;
    		goto error_out;
        }        

    }
    return ret;

error_out:
    if (entries) {
        free(entries);
        entries = NULL;
    }
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     get surface palette information
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[in] p_pallette_info     palette information
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surface_pallette_get(aui_hdl p_surface_handle,
										  aui_pallette_info *p_pallette_info)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !p_pallette_info) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	memcpy((void *)p_pallette_info, (void *)(&(p_surface->pallette_info)),
		   sizeof(aui_pallette_info));

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     set surfacee global alpha
*    @author        peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[in] global_alpha            global alpha value
*    @return        return AUI_GFX_SUCCESS if success.return AUI_RTN_EINVAL if wrong param.
*    @note      Global alpha may be used in blit operation according alpha output mode.
*
*/
AUI_RTN_CODE aui_gfx_surfaceGAlpha_set(aui_hdl p_surface_handle,
									   unsigned long global_alpha)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	p_surface->global_alpha = global_alpha;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
*    @brief     Get surfacee global alpha
*    @author    peter.pan@alitech.com
*    @date      2014-1-24
*    @param[in] p_surface_handle        surface handle
*    @param[out]    global_alpha            global alpha value
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_surfaceGAlpha_get(aui_hdl p_surface_handle,
									   unsigned long *global_alpha)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;

	p_surface = (aui_surface_handle_t *)p_surface_handle;

	if (!p_surface || !global_alpha) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	*global_alpha = p_surface->global_alpha;

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
 *  @brief          render a image to a surface
 *
 *  @param[in]      surface_handle      destination surface
 *  @param[in]      image_path          the path of the input image
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/17/2014  14:32:6
 *
 *  @note
 */
AUI_RTN_CODE aui_gfx_render_image_to_surface(aui_hdl surface_handle,
											 const char *image_path)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	aui_surface_handle_t *p_surface = NULL;
	IDirectFBImageProvider *dfb_provider = NULL;
	IDirectFBSurface *dfb_surface = NULL;

	p_surface = (aui_surface_handle_t *)surface_handle;

	if (!p_surface || !image_path) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}
	dfb_surface = p_surface->dfb_surface;

	if (DFB_OK != (gdfb->CreateImageProvider(gdfb, image_path, &dfb_provider))) {
        AUI_ERR("CreateImageProvider fail.\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
	}

	dfb_provider->RenderTo(dfb_provider, dfb_surface, NULL);
	dfb_provider->Release(dfb_provider);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

AUI_RTN_CODE aui_gfx_render_amimation_to_surface()
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_RTN_FEATURE_NOT_SUPPORTED;

	return ret;
}

/**
 *  @brief          get the width/height/pitch of a specified image
 *
 *  @param[in]      *image_path     the path of the image
 *  @param[out]     *width          image width
 *  @param[out]     *height         image height
 *  @param[out]     *pitch          pitch of the buffer
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/17/2014  16:40:56
 *
 *  @note
 */
AUI_RTN_CODE aui_gfx_get_image_info(const char *image_path,
									int *width,
									int *height)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	DFBSurfaceDescription surface_dsc;
	IDirectFBImageProvider *provider = NULL;

	if (!image_path || !width || !height) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	if (DFB_OK != gdfb->CreateImageProvider(gdfb, image_path, &provider)){
        AUI_ERR("CreateImageProvider fail.\n");
		ret = AUI_RTN_FAIL;
		goto error_out;
    }
	provider->GetSurfaceDescription(provider, &surface_dsc);
	*width = surface_dsc.width;
	*height = surface_dsc.height;

	provider->Release(provider);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
 *  @brief          decode a specified image file
 *
 *  @param[out]     pp_bitmap       output image data
 *  @param[in]      dst_format      the pixel format of the output bitmap
 *  @param[in]      image_buf       input image data
 *  @param[in]      buf_size        size of the input image data
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/17/2014  19:27:52
 *
 *  @note
 */
AUI_RTN_CODE aui_gfx_image_decode(aui_gfx_bitmap_info_t **pp_bitmap,
								  enum aui_osd_pixel_format dst_format,
								  const char *image_buf,
								  int buf_size)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	IDirectFBSurface *dfb_surface = NULL;
	DFBSurfaceDescription dsc, surface_dsc;
	IDirectFBImageProvider *provider = NULL;
	DFBDataBufferDescription ddsc;
	IDirectFBDataBuffer *buffer = NULL;
	aui_gfx_bitmap_info_t *p_bitmap_info = malloc(sizeof(aui_gfx_bitmap_info_t));
	int pitch;

	if (!p_bitmap_info) {
		ret = AUI_RTN_ENOMEM;
		goto error_out;
	}

	if (!image_buf) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	ddsc.flags = DBDESC_MEMORY;
	ddsc.memory.data = image_buf;
	ddsc.memory.length = buf_size;
	gdfb->CreateDataBuffer(gdfb, &ddsc, &buffer);
	buffer->CreateImageProvider(buffer, &provider);
	provider->GetSurfaceDescription(provider, &surface_dsc);
	dsc.width = surface_dsc.width;
	dsc.height = surface_dsc.height;

	switch (dst_format) {
		case AUI_OSD_4_COLOR:
		case AUI_OSD_256_COLOR:
		case AUI_OSD_16_COLOR_PIXEL_ALPHA:
			pitch = dsc.width;
			break;
		case AUI_OSD_HD_RGB555:
		case AUI_OSD_HD_RGB565:
		case AUI_OSD_HD_ARGB1555:
		case AUI_OSD_HD_ARGB4444:
		case AUI_OSD_HD_RGB444:
			pitch = dsc.width * 2;
			break;
		case AUI_OSD_HD_ARGB8888:
			pitch = dsc.width * 4;
			break;
		default:
			ret = AUI_RTN_EINVAL;
			goto error_out;
			break;
	}
	memset(p_bitmap_info, 0, sizeof(aui_gfx_bitmap_info_t));

	p_bitmap_info->color_type = dst_format;
	p_bitmap_info->width = dsc.width;
	p_bitmap_info->height = dsc.height;
	p_bitmap_info->pitch = pitch;
	p_bitmap_info->p_data = malloc(dsc.height * pitch);
	if (!p_bitmap_info->p_data) {
		ret = AUI_RTN_ENOMEM;
		goto error_out;
	}

	dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_PREALLOCATED;
	dsc.preallocated[0].data = p_bitmap_info->p_data;
	dsc.preallocated[0].pitch = pitch;
	dsc.pixelformat = aui_pixelformat_to_dfb(dst_format);
	gdfb->CreateSurface(gdfb, &dsc, &dfb_surface);

	provider->RenderTo(provider, dfb_surface, NULL);
	dfb_surface->Release(dfb_surface);
	provider->Release(provider);

	*pp_bitmap = p_bitmap_info;
	return ret;

error_out:
	free(p_bitmap_info);
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

/**
 *  @brief          release a bitmap data
 *
 *  @param[in]      p_bitmap        point to the bitmap structure want to be released
 *
 *  @return         AUI_RTN_CODE
 *
 *  @author         Peter Pan <peter.pan@alitech.com>
 *  @date           2/17/2014  19:30:22
 *
 *  @note
 */
AUI_RTN_CODE aui_gfx_image_release(aui_gfx_bitmap_info_t *p_bitmap)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;

	if (!p_bitmap) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}
	if (p_bitmap->p_data) {
		free(p_bitmap->p_data);
	}
	free(p_bitmap);

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

#else

AUI_RTN_CODE aui_gfx_init(p_fun_cb p_call_back_init,
						  void *pv_param)
{
	(void)p_call_back_init;
	(void)pv_param;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_de_init(p_fun_cb p_call_back_init,
							 void *pv_param)
{
	(void)p_call_back_init;
	(void)pv_param;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_sw_surface_create(enum aui_osd_pixel_format e_color_mode,
									   unsigned long width,
									   unsigned long height,
									   aui_hdl *p_surface_handle)
{
	(void)e_color_mode;
	(void)width;
	(void)height;
	(void)p_surface_handle;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_sw_surface_create_by_bitmap(aui_hdl *p_surface_handle,
												 const aui_gfx_bitmap_info_t *p_bitmap_info)
{
	(void)p_surface_handle;
	(void)p_bitmap_info;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_hw_surface_create(aui_hdl gfx_layer_handle,
									   enum aui_osd_pixel_format e_color_mode,
									   struct aui_osd_rect* rect,
									   aui_hdl *p_hw_surface_handle,
									   unsigned long is_double_buf)
{
	(void)gfx_layer_handle;
	(void)e_color_mode;
	(void)rect;
	(void)p_hw_surface_handle;
	(void)is_double_buf;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_delete(aui_hdl p_surface_handle)
{
	(void)p_surface_handle;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_blit(aui_hdl dst_surface_handle,
								  aui_hdl fg_surface_handle,
								  aui_hdl bg_surface_handle,
								  aui_blit_operation *blit_operation,
								  aui_blit_rect *blit_rect)
{
	(void)dst_surface_handle;
	(void)fg_surface_handle;
	(void)bg_surface_handle;
	(void)blit_operation;
	(void)blit_rect;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_data_blit(aui_hdl dst_surface_handle,
							   unsigned char *p_data,
							   aui_blit_rect *blit_rect,
							   aui_blit_operation *blit_operation,
							   unsigned long pitch,
							   enum aui_osd_pixel_format en_color_mode)
{
	(void)dst_surface_handle;
	(void)p_data;
	(void)blit_rect;
	(void)blit_operation;
	(void)pitch;
	(void)en_color_mode;
	
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_capture(aui_hdl p_surface_handle,
									 struct aui_osd_rect * rect,
									 void *p_data)
{
	(void)p_surface_handle;
	(void)rect;
	(void)p_data;
	
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_fill(aui_hdl surface_handle,
								  unsigned long color,
								  struct aui_osd_rect * rect)
{
	(void)surface_handle;
	(void)color;
	(void)rect;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_draw_line(aui_hdl surface_handle,
									   unsigned long color,
									   aui_coordinate *start_coordinate,
									   aui_coordinate *end_coordinate)
{
	(void)surface_handle;
	(void)color;
	(void)start_coordinate;
	(void)end_coordinate;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_draw_outline(aui_hdl surface_handle,
										  unsigned long fore_color,
										  unsigned long back_color,
										  struct aui_osd_rect * rect,
										  unsigned long fill_background)
{
	(void)surface_handle;
	(void)fore_color;
	(void)back_color;
	(void)rect;
	(void)fill_background;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_flush(aui_hdl dst_surface_handle,
								   struct aui_osd_rect *dst_rect)
{
	(void)dst_surface_handle;
	(void)dst_rect;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_sync(aui_hdl dst_surface_handle)
{
	(void)dst_surface_handle;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_lock(aui_hdl p_surface_handle)
{
	(void)p_surface_handle;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_unlock(aui_hdl p_surface_handle)
{
	(void)p_surface_handle;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_info_get(aui_hdl p_surface_handle,
									  aui_surface_info *p_surface_info)
{
	(void)p_surface_handle;
	(void)p_surface_info;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_show_on_off(aui_hdl p_hw_surface_handle,
										 unsigned long on_off)
{
	(void)p_hw_surface_handle;
	(void)on_off;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_scale_param_set(aui_hdl p_hw_surface_handle,
											 aui_scale_param *scale_param)
{
	(void)p_hw_surface_handle;
	(void)scale_param;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_colorkey_set(aui_hdl p_surface_handle,
										  aui_color_key *color_key)
{
	(void)p_surface_handle;
	(void)color_key;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_colorkey_get(aui_hdl p_surface_handle,
										  aui_color_key *colorkey)
{
	(void)p_surface_handle;
	(void)colorkey;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_clip_rect_set(aui_hdl p_surface_handle,
										   struct aui_osd_rect* rect,
										   enum aui_ge_clip_mode clip_mode)
{
	(void)p_surface_handle;
	(void)rect;
	(void)clip_mode;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_clip_rect_clear(aui_hdl p_surface_handle)
{
	(void)p_surface_handle;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_pallette_set(aui_hdl p_surface_handle,
										  aui_pallette_info *p_pallette_info)
{
	(void)p_surface_handle;
	(void)p_pallette_info;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_pallette_get(aui_hdl p_surface_handle,
										  aui_pallette_info *p_pallette_info)
{
	(void)p_surface_handle;
	(void)p_pallette_info;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surfaceGAlpha_set(aui_hdl p_surface_handle,
									   unsigned long global_alpha)
{
	(void)p_surface_handle;
	(void)global_alpha;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_surfaceGAlpha_get(aui_hdl p_surface_handle,
									   unsigned long *global_alpha)
{
	(void)p_surface_handle;
	(void)global_alpha;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_render_image_to_surface(aui_hdl surface_handle,
											 const char *image_path)
{
	(void)surface_handle;
	(void)image_path;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_render_amimation_to_surface()
{
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_get_image_info(const char *image_path,
									int *width,
									int *height)
{
	(void)image_path;
	(void)width;
	(void)height;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_image_decode(aui_gfx_bitmap_info_t **pp_bitmap,
								  enum aui_osd_pixel_format dst_format,
								  const char *image_buf,
								  int buf_size)
{
	(void)pp_bitmap;
	(void)dst_format;
	(void)image_buf;
	(void)buf_size;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

AUI_RTN_CODE aui_gfx_image_release(aui_gfx_bitmap_info_t *p_bitmap)
{	
	(void)p_bitmap;
	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	ret = AUI_RTN_FEATURE_NOT_SUPPORTED;
	return ret;
}

#endif	///LINUX_AUI_SUPPORT_GFX

/**
*    @brief     Configure enhance value of OSD layer
*    @author 	Vedic.Fu@alitech.com
*    @date      2015-8-7
*    @param[in] gfx_layer_handle    OSD layer handle
*    @param[in] en_type          enhance type
*    @param[in] en_value           enhance value
*    @return        return AUI_GFX_SUCCESS if success. return AUI_RTN_EINVAL if wrong param.
*    @note
*
*/
AUI_RTN_CODE aui_gfx_layer_enhance_set(aui_hdl gfx_layer_handle,
										 enum aui_osd_enhance  en_type,
										 unsigned int en_value)
{
	GFX_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_GFX_SUCCESS;
	aui_layer_handle_t *gfx_layer = NULL;
	struct dis_enhance enhance;
	
	if (!gfx_layer_handle || en_value > 100) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	gfx_layer = (aui_layer_handle_t *)gfx_layer_handle;

	/* the OSD enhance just support GMA1 now*/
	if (AUI_OSD_LAYER_GMA0 != gfx_layer->layer_id) {
		ret = AUI_RTN_EINVAL;
		goto error_out;
	}

	switch(en_type) {
		case AUI_OSD_ENHANCE_BRIGHTNESS:
			enhance.type       = DIS_ENHANCE_BRIGHTNESS;
			enhance.brightness = en_value;
			break;
		case AUI_OSD_ENHANCE_CONTRAST:
			enhance.type       = DIS_ENHANCE_CONTRAST;
			enhance.contrast   = en_value;
			break;
		case AUI_OSD_ENHANCE_SATURATION:
			enhance.type       = DIS_ENHANCE_SATURATION;
			enhance.saturation = en_value;
			break;
		case AUI_OSD_ENHANCE_SHARPNESS:
			enhance.type       = DIS_ENHANCE_SHARPNESS;
			enhance.sharpeness = en_value;
			break;
		case AUI_OSD_ENHANCE_HUE:
			enhance.type       = DIS_ENHANCE_HUE;
			enhance.hue        = en_value;
			break;

		default:
			aui_rtn(AUI_RTN_EINVAL,
					"\n Invalidate enhance type specified! \n");
	}

	if (DIS_ERR_NONE != alisldis_set_enhance_osd(gfx_layer->alisl_dis, &enhance))
		aui_rtn(AUI_RTN_FAIL, "\n Set enhance fail! \n");

error_out:
	if (ret)
		AUI_ERR("ret %d\n", ret);
	return ret;
}

AUI_RTN_CODE aui_gfx_surface_galpha_set (
    aui_hdl p_surface_handle,
    unsigned long ul_global_alpha)
{
    (void)p_surface_handle;
    (void)ul_global_alpha;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gfx_surface_galpha_get (
    aui_hdl p_surface_handle,
    unsigned long *pul_global_alpha)
{
    (void)p_surface_handle;
    (void)pul_global_alpha;
    return AUI_RTN_SUCCESS;
}

