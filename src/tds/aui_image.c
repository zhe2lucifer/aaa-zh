/**@file
*     @brief           AUI image module interface implement
*     @author        henry.xie
*     @date           2013-6-27
*     @version       1.0.0
*     @note           ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <mediatypes.h>
#include <api/libimagedec/imagedec.h>
#include <aui_image.h>
#include <api/libmp/pe.h>

AUI_MODULE( IMAGE );


/****************************LOCAL MACRO******************************************/
//the DE display coefficient. When the size of the picture is greater than than this value, the DE
//module will do some scaling to make sure that the displaying size would be no more than this vaule.
#define IMAGE_DE_WIDTH 720    
#define IMAGE_DE_HEIGHT 576

#define IMG_R_W 184
#define IMG_R_H 140
#define IMG_R_L 748
#define IMG_R_T 460

#define IMG_SRC_L 0
#define IMG_SRC_T 0
#define IMG_SRC_W 720
#define IMG_SRC_H 576

#define MAX_STEP_PIXS 3

/****************************LOCAL TYPE*******************************************/
struct image_position {
    unsigned short u_x;    // horizontal position .
    unsigned short      u_y;      // vertical position.
};

struct osd_rect {
    unsigned short      u_left;
    unsigned short      u_top;
    unsigned short      u_width;
    unsigned short      u_height;
};

typedef struct aui_image_buf_cfg {
    /*member to store address that client malloc for image module, the driver will allocate
    memory for three buffer used for DE displaying.*/
    unsigned long ul_image_de_buf;
    /*the length of ul_image_buffer, the recommend value is 0x8F8200*/
    unsigned long ul_image_de_buf_len;
    /* buffer used for decoder*/
    unsigned long ul_image_decoder_buf;
    /*the length of decoder buffer, the recommend value is 0x752800*/
    unsigned long ul_decoder_buf_len;
} aui_image_buf_cfg;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Image Module </b> to specify the image handle attributes
        </div> @endhtmlonly

        Struct to specify the image handle attributes
*/
typedef struct {
    /** attribute for image */
    aui_attr_image attr_image;
} aui_handle_image, *aui_p_handle_image;

extern AUI_RTN_CODE aui_get_mp_config( struct pe_video_cfg *config, struct pe_video_cfg_extra *extra );
extern AUI_RTN_CODE aui_get_music_config( struct pe_music_cfg *config );


/****************************LOCAL VAR********************************************/

/* we only one flag,one mutex in image decode */
OSAL_ID image_flag = INVALID_ID;
static OSAL_ID image_mutex = INVALID_ID;
static unsigned char zoom_times = 0;
static double image_scale[] = {
    1.41421,
    2.0,
    2.82843,
    4.0
};
static struct image_position image_center;
static aui_handle_image *g_image_handle = NULL;

/****************************LOCAL FUNC DECLEAR***********************************/
static int no_zoom_current()
{
    struct rect img_src_rect, img_dest_rect;
    img_dest_rect.u_start_x    = 0;
    img_dest_rect.u_start_y    = 0;
    img_dest_rect.u_width      = 720;
    img_dest_rect.u_height      = 576;
    
    img_src_rect.u_start_x      = 0;
    img_src_rect.u_start_y      = 0;
    img_src_rect.u_width     = 720;
    img_src_rect.u_height     = 576;
    
    return image_zoom( &img_dest_rect, &img_src_rect );
}

static void image_src_rect_cal( struct rect *img_src_rect, struct osd_rect *osd_src_rect )
{
    double zoom_scale;
    unsigned int half_width, half_height;
    
    zoom_scale = image_scale[zoom_times];
    osd_src_rect->u_width     = ( unsigned short )( IMG_R_W / zoom_scale );
    osd_src_rect->u_height     = ( unsigned short )( IMG_R_H / zoom_scale );
    half_width    = osd_src_rect->u_width / 2;
    half_height = osd_src_rect->u_height / 2;
    
    if ( image_center.u_x < half_width ) {
        image_center.u_x = half_width;
    }
    if ( image_center.u_x > ( IMG_R_W - half_width ) ) {
        image_center.u_x = ( IMG_R_W - half_width );
    }
    if ( image_center.u_y < half_height ) {
        image_center.u_y = half_height;
    }
    if ( image_center.u_y > ( IMG_R_H - half_height ) ) {
        image_center.u_y = IMG_R_H - half_height;
    }
    
    osd_src_rect->u_left = IMG_R_L + ( image_center.u_x - half_width );
    osd_src_rect->u_top  = IMG_R_T + ( image_center.u_y - half_height );
    
    img_src_rect->u_width      = osd_src_rect->u_width * IMG_SRC_W / IMG_R_W;
    img_src_rect->u_height      = osd_src_rect->u_height * IMG_SRC_H / IMG_R_H;
    img_src_rect->u_start_x    = IMG_SRC_L + ( osd_src_rect->u_left - IMG_R_L ) * IMG_SRC_W / IMG_R_W;
    img_src_rect->u_start_y    = IMG_SRC_T + ( osd_src_rect->u_top - IMG_R_T ) * IMG_SRC_H / IMG_R_H;
}

static int zoom_proc()
{
    struct rect img_src_rect, img_dest_rect;
    struct osd_rect osd_src_rect;
    image_src_rect_cal( &img_src_rect, &osd_src_rect );
    img_dest_rect.u_start_x    = 0;
    img_dest_rect.u_start_y    = 0;
    img_dest_rect.u_width      = 720;
    img_dest_rect.u_height      = 576;
    
    return image_zoom( &img_dest_rect, &img_src_rect );
}


static void move_proc( enum aui_image_direction act, int count )
{
    struct osd_rect osd_src_rect;
    struct rect img_src_rect;
    int step;
    
    image_src_rect_cal( &img_src_rect, &osd_src_rect );
    
    switch ( act ) {
        case AUI_DIR_LEFT:
            step = ( IMG_R_L + IMG_R_W ) - ( osd_src_rect.u_left + osd_src_rect.u_width );
            if ( step > 0 ) {
                if ( step > MAX_STEP_PIXS ) {
                    step = MAX_STEP_PIXS;
                }
                image_center.u_x += step * count;
            }
            break;
        case AUI_DIR_RIGHT:
            step = osd_src_rect.u_left - IMG_R_L;
            if ( step > 0 ) {
                if ( step > MAX_STEP_PIXS ) {
                    step = MAX_STEP_PIXS;
                }
                image_center.u_x -= step * count;
            }
            break;
        case AUI_DIR_UP:
            step = ( IMG_R_T + IMG_R_H ) - ( osd_src_rect.u_top + osd_src_rect.u_height );
            if ( step > 0 ) {
                if ( step > MAX_STEP_PIXS ) {
                    step = MAX_STEP_PIXS;
                }
                image_center.u_y += step * count;
            }
            break;
        case AUI_DIR_DOWN:
            step = osd_src_rect.u_top - IMG_R_T;
            if ( step > 0 ) {
                if ( step > MAX_STEP_PIXS ) {
                    step = MAX_STEP_PIXS;
                }
                image_center.u_y -= step * count;
            }
            break;
            
        default:
            break;
    }
    
    return;
}

void commom_image_callback_wrapper( unsigned long param1, unsigned long param2 )
{
    switch ( param1 ) {
        case MP_IMAGE_PLAYBACK_END:
            osal_flag_set( image_flag, 1 );
            
            if ( g_image_handle && g_image_handle->attr_image.aui_image_cb ) {
                g_image_handle->attr_image.aui_image_cb( AUI_IMAGE_SHOW_COMPLETE,
                        ( void * )param2,
                        g_image_handle->attr_image.user_data );
            }
            
            break;
            
        default:
            break;
    }
    return;
}

static aui_image_buf_cfg g_image_cfg;

AUI_RTN_CODE aui_image_memory_info_set( unsigned long addr1, unsigned long len1, unsigned long addr2,
                                        unsigned long len2 )
{
    g_image_cfg.ul_image_de_buf = addr1;
    g_image_cfg.ul_image_de_buf_len = len1;
    g_image_cfg.ul_image_decoder_buf = addr2;
    g_image_cfg.ul_decoder_buf_len = len2;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_image_init( aui_func_image_init fn_image_init )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL != fn_image_init ) {
        fn_image_init();
    }
    
    image_flag = osal_flag_create( 0 );
    if ( image_flag == INVALID_ID ) {
        aui_rtn( AUI_RTN_ENOMEM, "create image flag fail" );
    }
    
    image_mutex = osal_mutex_create();
    if ( image_mutex == INVALID_ID ) {
        aui_rtn( AUI_RTN_ENOMEM, "create image mutex fail" );
    }
    
    zoom_times = 0;
    image_center.u_x = IMG_R_W / 2;
    image_center.u_y = IMG_R_H / 2;
    
    return rtn_code;
}

static void image_decode_done()
{
    osal_flag_set( image_flag, 1 );
}
/****************************MODULE DRV IMPLEMENT*************************************/

AUI_RTN_CODE aui_image_open( aui_attr_image *pst_image_attr, void **pp_handle_image )
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    
    if ( NULL == pst_image_attr ) {
        aui_rtn( AUI_RTN_EINVAL, "pst_image_attr is null !\n" );
    }
    
    if ( pp_handle_image == NULL ) {
        aui_rtn( AUI_RTN_EINVAL, "pp_handle_image is null !\n" );
    }
    
    if ( g_image_handle == NULL ) {
        *pp_handle_image = ( aui_handle_image * )MALLOC( sizeof( aui_handle_image ) );
        
        if ( NULL == *pp_handle_image ) {
            aui_rtn( AUI_RTN_ENOMEM, "fail to malloc memory for pp_handle_image!\n" );
        }
        
        /* we must set it in aui_image_display_mode_set function */
        //pst_image_attr->en_mode = AUI_MODE_PREVIEW;   //AUI_MODE_FULL;
        
        
        MEMSET( ( aui_handle_image * )( *pp_handle_image ), 0, sizeof( aui_handle_image ) );
        MEMCPY( &( ( ( aui_handle_image * )( *pp_handle_image ) )->attr_image ),
                pst_image_attr,
                sizeof( aui_attr_image ) );
        g_image_handle = *pp_handle_image;
        ret = AUI_RTN_SUCCESS;
    } else {
        AUI_ERR( "Already opened." );
    }
    return ret;
}

AUI_RTN_CODE aui_image_close( aui_attr_image *pst_image_attr, void **pp_handle_image )
{

    if ( pst_image_attr != NULL ) {
        AUI_DBG( "pst_image_attr:%x may not be used!\n" );
    }
    
    if ( pp_handle_image == NULL ) {
        aui_rtn( AUI_RTN_EINVAL, "pp_handle_image is NULL\n" );
    }
    
    if ( g_image_handle != *pp_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pp_handle_image is wrong\n" );
    }
    
    if ( ( *pp_handle_image ) != NULL ) {
        FREE( *pp_handle_image );
        g_image_handle = NULL;
    }
    pe_cleanup(); // free malloc memory in PE
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_image_start( void *pv_handle_image, aui_image_rect *p_ary_rect, int i_num )
{
    int ret = -1;
    unsigned int flgptn;
    int i = 0;
    struct image_slideshow_effect effect;
    effect.mode = M_NORMAL;
    
    if ( NULL == pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null !\n" );
    }
    
    if ( g_image_handle != pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is wrong\n" );
    }
    
    enum aui_image_mode en_mode = ( ( ( aui_handle_image * )( pv_handle_image ) )->attr_image ).en_mode;
    char *pc_file_name = ( char * )( ( ( aui_handle_image * )( pv_handle_image ) )->attr_image ).uc_file_name;
    char *pc_multi_file_name = ( char * )( ( ( ( aui_handle_image * )(
            pv_handle_image ) )->attr_image ).uc_multi_file_name );
            
    struct pe_music_cfg p_music_cfg;
    struct pe_image_cfg p_image_cfg;
    struct pe_video_cfg p_video_cfg;
    struct pe_video_cfg_extra p_video_cfg_extra;
    
    
    memset( &p_music_cfg, 0, sizeof( struct pe_music_cfg ) );
    memset( &p_image_cfg, 0, sizeof( struct pe_image_cfg ) );
    memset( &p_video_cfg, 0, sizeof( struct pe_video_cfg ) );
    memset( &p_video_cfg_extra, 0, sizeof( struct pe_video_cfg_extra ) );
    
    //fill the image engine config, frm_y_addr is used to store luminance, frm_c_addr is used to store chroma(YUV mode)
    //these three buffer is used for DE displaying
    p_image_cfg.frm_y_addr = g_image_cfg.ul_image_de_buf;
    p_image_cfg.frm_y_size = 2 * ( g_image_cfg.ul_image_de_buf_len ) / 9;
    p_image_cfg.frm_c_addr = p_image_cfg.frm_y_addr + p_image_cfg.frm_y_size;
    p_image_cfg.frm_c_size = ( g_image_cfg.ul_image_de_buf_len ) / 9;
    p_image_cfg.frm2_y_addr = p_image_cfg.frm_c_addr + p_image_cfg.frm_c_size;
    p_image_cfg.frm2_y_size = 2 * ( g_image_cfg.ul_image_de_buf_len ) / 9;
    p_image_cfg.frm2_c_addr = p_image_cfg.frm2_y_addr + p_image_cfg.frm2_y_size;
    p_image_cfg.frm2_c_size = ( g_image_cfg.ul_image_de_buf_len ) / 9;
    p_image_cfg.frm3_y_addr = p_image_cfg.frm2_c_addr + p_image_cfg.frm2_c_size;
    p_image_cfg.frm3_y_size = 2 * ( g_image_cfg.ul_image_de_buf_len ) / 9;
    p_image_cfg.frm3_c_addr = p_image_cfg.frm3_y_addr + p_image_cfg.frm3_y_size;
    p_image_cfg.frm3_c_size = ( g_image_cfg.ul_image_de_buf_len ) / 9;
    p_image_cfg.decoder_buf = ( unsigned char * ) g_image_cfg.ul_image_decoder_buf;
    p_image_cfg.decoder_buf_len = g_image_cfg.ul_decoder_buf_len;
    p_image_cfg.mp_cb = commom_image_callback_wrapper;
    
#ifdef IMG_2D_TO_3D
    p_image_cfg.ali_pic_2dto3d = ali_pic_2dto3d;
#endif
    aui_get_mp_config( &p_video_cfg, &p_video_cfg_extra );
    aui_get_music_config( &p_music_cfg );
    pe_cleanup();
    ret = pe_init( &p_music_cfg, &p_image_cfg, &p_video_cfg );
    if ( -1 == ret ) {
        return AUI_RTN_FAIL;
    }
    
    
    if ( (INVALID_ID == image_mutex) || (INVALID_ID == image_flag) ) {
        aui_rtn( AUI_RTN_FAIL, "flag not init" );
    }
    
    switch ( en_mode ) {
        case AUI_MODE_FULL:
            osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
            osal_flag_clear( image_flag, 1 );
            ret = image_decode_ex( pc_file_name, IMAGEDEC_FULL_SRN,
                                   0, 0,
                                   IMAGE_DE_WIDTH, IMAGE_DE_HEIGHT, ANG_ORI, &effect );
            if ( ret == -1 ) {
                osal_mutex_unlock( image_mutex );
                aui_rtn( AUI_RTN_FAIL, "decode error" );
            }
            /*we must do image clear outside because we need to cover customer callback */
            osal_flag_wait( ( UINT32 * )&flgptn, image_flag, 0x1, OSAL_TWF_ORW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME );
            osal_mutex_unlock( image_mutex );
            break;
            
        case AUI_MODE_PREVIEW:
            if ( (NULL == p_ary_rect) || (i_num != 1) ) {
                aui_rtn( AUI_RTN_EINVAL, "p_ary_rect is null and i_num is wrong numbe!\n" );
            }
            
            if ( (p_ary_rect->start_x < 0) || (p_ary_rect->start_y < 0)
                 || (p_ary_rect->width < 0) || (p_ary_rect->height < 0) ) {
                aui_rtn( AUI_RTN_EINVAL, "p_ary_rect's paramater is wrong!\n" );
            }
            
            if ( (p_ary_rect->start_x + p_ary_rect->width > IMAGE_DE_WIDTH)
                 || (p_ary_rect->start_y + p_ary_rect->height > IMAGE_DE_HEIGHT) ) {
                aui_rtn( AUI_RTN_EINVAL, "p_ary_rect's paramater is wrong!\n" );
            }
            
            osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
            osal_flag_clear( image_flag, 1 );
			//init the special area on screen with th color of 0x108080(YUV)
            imagedec_ioctl( 1, IMAGEDEC_IO_CMD_FILL_LOGO,0x108080 );
            ret = image_decode( pc_file_name, IMAGEDEC_THUMBNAIL,
                                p_ary_rect->start_x, p_ary_rect->start_y,
                                p_ary_rect->width, p_ary_rect->height, ANG_ORI );
            if ( ret == -1 ) {
                osal_mutex_unlock( image_mutex );
                aui_rtn( AUI_RTN_FAIL, "preview decode error" );
            }
            osal_flag_wait( ( UINT32 * )&flgptn, image_flag, 0x1, OSAL_TWF_ORW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME );
            osal_mutex_unlock( image_mutex );
            break;
            
        case AUI_MODE_MULTIIEW:
            if ( (NULL == p_ary_rect) || (i_num == 1) ) {
                aui_rtn( AUI_RTN_EINVAL, "p_ary_rect is null and i_num is wrong numbe!\n" );
            }
            
            osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
            for ( i = 0; i < i_num ; i++ ) {
                if ( (p_ary_rect[i].start_x < 0) || (p_ary_rect[i].start_y < 0)
                     || (p_ary_rect[i].height < 0) || (p_ary_rect[i].width < 0) ) {
                    osal_mutex_unlock( image_mutex );
                    aui_rtn( AUI_RTN_EINVAL, "p_ary_rect's paramater is wrong!\n" );
                }
                
                if ( (p_ary_rect[i].start_x + p_ary_rect[i].start_y > IMAGE_DE_WIDTH)
                     || (p_ary_rect[i].width + p_ary_rect[i].height > IMAGE_DE_HEIGHT) ) {
                    osal_mutex_unlock( image_mutex );
                    aui_rtn( AUI_RTN_EINVAL, "p_ary_rect's paramater is wrong!\n" );
                }
                
                ret = image_decode( pc_multi_file_name + i * MAX_FILENAME_LEN, IMAGEDEC_MULTI_PIC,
                                    p_ary_rect[i].start_x, p_ary_rect[i].start_y,
                                    p_ary_rect[i].width, p_ary_rect[i].height, ANG_ORI );
                if ( ret == -1 ) {
                    osal_mutex_unlock( image_mutex );
                    aui_rtn( AUI_RTN_FAIL, "multiview decode error" );
                }
                osal_flag_wait( ( UINT32 * )&flgptn, image_flag, 0x1, OSAL_TWF_ORW | OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME );
            }
            osal_mutex_unlock( image_mutex );
            break;
            
        default:
            break;
    }
    
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_image_de_init( aui_func_image_init fn_image_de_init )
{
    if ( NULL != fn_image_de_init ) {
        fn_image_de_init();
    }
    
    if ( image_flag != INVALID_ID ) {
        osal_flag_delete( image_flag );
    }
    if ( image_mutex != INVALID_ID ) {
        osal_mutex_delete( image_mutex );
    }
    
    return AUI_RTN_SUCCESS;
}

char *image_strncpy( char *dest, const unsigned char *source, int count )
{
    char *p = dest;
    
    while ( count && ( *p++ = *source++ ) ) {
        count--;
    }
    while ( count-- ) {
        *p++ = '\0';
    }
    
    return dest;
}

AUI_RTN_CODE aui_image_set( void *pv_hdl_image, unsigned long ul_item, void *pv_param )
{
    unsigned int i = 0;
    
    if ( (NULL == pv_hdl_image) && (ul_item != AUI_IMAGE_SET_DECDONE_FLAG) ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null and flag is error!\n" );
    }
    
    if ( g_image_handle != pv_hdl_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_hdl_image is wrong\n" );
    }
    
    switch ( ul_item ) {
        case AUI_IMAGE_SET_DECDONE_FLAG:
            image_decode_done();
            break;
        case AUI_IMAGE_SET_MULTI_FILENAME:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            
            if ( AUI_MODE_MULTIIEW != ( ( ( aui_handle_image * )( pv_hdl_image ) )->attr_image ).en_mode ) {
                aui_rtn( AUI_RTN_EINVAL, "en_mode is wrong!\n" );
            }
            
            aui_multi_name_set *pst_file_ary = ( aui_multi_name_set * )pv_param;
            char *p_name = ( char * )( ( ( ( aui_handle_image * )( pv_hdl_image ) )->attr_image ).uc_multi_file_name );
            
            osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
            for ( i = 0; i < pst_file_ary->ui_file_num; i++ ) {
                image_strncpy( p_name + i * MAX_FILENAME_LEN, ( pst_file_ary->uc_multi_file_name )[i], MAX_FILENAME_LEN );
            }
            osal_mutex_unlock( image_mutex );
            break;
        case AUI_IMAGE_SET_FILE_NAME:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            
            if ( AUI_MODE_MULTIIEW == ( ( ( aui_handle_image * )( pv_hdl_image ) )->attr_image ).en_mode ) {
                aui_rtn( AUI_RTN_EINVAL, "en_mode is wrong!\n" );
            }
            
            char *p_file_name = ( char * )( ( ( ( aui_handle_image * )( pv_hdl_image ) )->attr_image ).uc_file_name );
            image_strncpy( p_file_name, ( unsigned char * )pv_param, MAX_FILENAME_LEN );
            
            break;
        default:
            break;
    }
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_image_get( void *pv_hdl_image, unsigned long ul_item, void *pv_param )
{
    if ( NULL == pv_hdl_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_hdl_image is null !\n" );
    }
    
    if ( g_image_handle != pv_hdl_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_hdl_image is wrong\n" );
    }
    
    switch ( ul_item ) {
        case AUI_IMAGE_GET_INFO:
            if ( AUI_MODE_MULTIIEW == ( ( ( aui_handle_image * )( pv_hdl_image ) )->attr_image ).en_mode ) {
                aui_rtn( AUI_RTN_FAIL, "not support" );
            } else {
                if ( NULL == pv_param ) {
                    aui_rtn( AUI_RTN_EINVAL, "pv_param is null\n" );
                }
                //struct image_info *pst_image_info = (struct image_info *)pv_param;
                struct image_info image_info;
                struct aui_image_info *p_image_info = pv_param;
                memset( &image_info, 0, sizeof( struct image_info ) );
                memset( p_image_info, 0, sizeof( struct aui_image_info ) );
                char *pc_file_name = ( char * )( ( ( aui_handle_image * )( pv_hdl_image ) )->attr_image ).uc_file_name;
                osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
                if ( -1 == image_get_info( pc_file_name, &image_info ) ) {
                    osal_mutex_unlock( image_mutex );
                    aui_rtn( AUI_RTN_FAIL, "get info fail" );
                }
                p_image_info->bbp = image_info.bbp;
                p_image_info->fsize = image_info.fsize;
                p_image_info->height = image_info.height;
                p_image_info->width = image_info.width;
                osal_mutex_unlock( image_mutex );
            }
            break;
        default:
            break;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_image_zoom( void *pv_handle_image, aui_image_rect *ps_src_rect, aui_image_rect *ps_dst_rect )
{
    struct rect img_src_rect, img_dest_rect;
    int ret = -1;
    
    if ( NULL == pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null !\n" );
    }
    
    if ( g_image_handle != pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is wrongL\n" );
    }
    
    img_dest_rect.u_start_x    = ps_src_rect->start_x;
    img_dest_rect.u_start_y    = ps_src_rect->start_y;
    img_dest_rect.u_width      = ps_src_rect->width;
    img_dest_rect.u_height      = ps_src_rect->height;
    
    img_src_rect.u_start_x      = ps_dst_rect->start_x;
    img_src_rect.u_start_y      = ps_dst_rect->start_y;
    img_src_rect.u_width     = ps_dst_rect->width;
    img_src_rect.u_height     = ps_dst_rect->height;
    
    osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
    ret = image_zoom( &img_dest_rect, &img_src_rect );
    
    if ( ret == -1 ) {
        osal_mutex_unlock( image_mutex );
        aui_rtn( AUI_RTN_ENOMEM, "zoom image fail" );
    }
    osal_mutex_unlock( image_mutex );
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_image_enlarge( void *pv_handle_image, enum aui_image_enlarge en_times )
{
    if ( NULL == pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null !\n" );
    }
    
    if ( g_image_handle != pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is wrongL\n" );
    }
    
    osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
    if ( AUI_IMAGE_ORG == en_times ) {
        if ( -1 == no_zoom_current() ) {
            osal_mutex_unlock( image_mutex );
            aui_rtn( AUI_RTN_ENOMEM, "enlarge image fail" );
        }
    } else {
        zoom_times = ( unsigned char )en_times - 1;
        if ( -1 == zoom_proc() ) {
            osal_mutex_unlock( image_mutex );
            aui_rtn( AUI_RTN_ENOMEM, "enlarge image fail" );
        }
    }
    osal_mutex_unlock( image_mutex );
    return AUI_RTN_SUCCESS;
}

extern int image_rotate( unsigned char rotate_angle );
AUI_RTN_CODE aui_image_rotate( void *pv_handle_image, enum aui_image_angle en_angle )
{
    int ret = -1;
    
    if ( NULL == pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null !\n" );
    }
    
    if ( g_image_handle != pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is wrongL\n" );
    }
    
    osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
    ret = image_rotate( ANG_ORI + ( unsigned char )en_angle );
    if ( ret == -1 ) {
        osal_mutex_unlock( image_mutex );
        aui_rtn( AUI_RTN_ENOMEM, "rotate image fail" );
    }
    osal_mutex_unlock( image_mutex );
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_image_display_mode_set( void *pv_handle_image, enum aui_image_mode en_mode )
{
    if ( NULL == pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null !\n" );
    }
    
    if ( g_image_handle != pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is wrongL\n" );
    }
    
    osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
    ( ( ( aui_handle_image * )( pv_handle_image ) )->attr_image ).en_mode = en_mode;
    osal_mutex_unlock( image_mutex );
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_image_move( void *pv_handle_image, enum aui_image_direction en_dir, int step )
{
    if ( NULL == pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is null !\n" );
    }
    
    if ( g_image_handle != pv_handle_image ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_image is wrongL\n" );
    }
    
    osal_mutex_lock( image_mutex, OSAL_WAIT_FOREVER_TIME );
    move_proc( en_dir, step );
    if ( -1 == zoom_proc() ) {
        osal_mutex_unlock( image_mutex );
        aui_rtn( AUI_RTN_ENOMEM, "move image fail" );
    }
    osal_mutex_unlock( image_mutex );
    return AUI_RTN_SUCCESS;
}

