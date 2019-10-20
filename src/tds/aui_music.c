/**@file
*     @brief         AUI music module interface implement
*     @author        henry.xie
*     @date              2013-6-27
*     @version           1.0.0
*     @note              ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"

#include <aui_music.h>
#include <api/libmp/pe.h>
#include <aui_mp.h>

AUI_MODULE( MUSIC )

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/
#define DECODER_BUF_LEN 0x5000
typedef struct aui_music_buf_cfg {
    /*member to store address that client malloc for music module, this buffer will
    be used to allocate memory for two buffers,one is used to store decoded data, and the
    size should not be less than 0x5000; another is used to store the processed decoded
    data, and the size should not be less than 0x1f5400*/
    unsigned long ul_music_buffer;
    /*the length of ul_music_buffer, the value should not be less than 0x1fa400*/
    unsigned long ul_len;
} aui_music_buf_cfg;
typedef struct {
    /** attribute of the music file */
    aui_attr_music attr_music;
} aui_handle_music, *aui_p_handle_music;

/****************************LOCAL VAR********************************************/
pn_message_callback pe_music_callback[AUI_MUSIC_MESSAGE_MAX] = {0};

pn_message_callback pe_music_end_callback = NULL;

static aui_handle_music *g_music_handle = NULL;



/****************************LOCAL FUNC DECLEAR***********************************/
extern void music_release();
extern AUI_RTN_CODE aui_get_mp_config( struct pe_video_cfg *config, struct pe_video_cfg_extra *extra );


/****************************MODULE DRV IMPLEMENT*************************************/
void commom_music_callback_wrapper( UINT32 param1, UINT32 param2 )
{

    switch ( param1 ) {
        case MP_MUSIC_PLAYBACK_END:
            if ( g_music_handle && g_music_handle->attr_music.aui_music_cb ) {
                g_music_handle->attr_music.aui_music_cb( AUI_MUSIC_PLAY_END,
                        ( void * )param2,
                        g_music_handle->attr_music.user_data );
            }
            break;
        default:
            break;
    }
    return;
}

static aui_music_buf_cfg g_music_cfg;
AUI_RTN_CODE aui_music_memory_info_set( unsigned long addr1, unsigned long len1 )
{
    g_music_cfg.ul_music_buffer = addr1;
    g_music_cfg.ul_len = len1;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_get_music_config( struct pe_music_cfg *config )
{
    if ( NULL == config ) {
        AUI_ERR( "fail to get right input!\n" );
        return AUI_RTN_FAIL;
    }
    
    struct pe_music_cfg *music_cfg = ( struct pe_music_cfg * )config;
    
    music_cfg->pcm_out_buff = g_music_cfg.ul_music_buffer & 0x8fffffff;                          
    music_cfg->pcm_out_buff_size = DECODER_BUF_LEN;            
    music_cfg->processed_pcm_buff = ( music_cfg->pcm_out_buff + music_cfg->pcm_out_buff_size ) & 0x8fffffff;
    music_cfg->processed_pcm_buff_size = ( g_music_cfg.ul_len - DECODER_BUF_LEN ) & 0x8fffffff; 
    music_cfg->mp_cb = commom_music_callback_wrapper;
        
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_music_init( aui_func_music_init fn_music_init )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == fn_music_init ) {
        aui_rtn( AUI_RTN_EINVAL, NULL );
    }
    fn_music_init();
    return rtn_code;
}

AUI_RTN_CODE aui_music_open( aui_attr_music *pst_music_attr, void **pp_handle_music )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    
    if ( NULL == pst_music_attr ) {
        aui_rtn( AUI_RTN_EINVAL, "pst_music_attr is null\n" );
    }
    if ( g_music_handle == NULL ) {
    
        *pp_handle_music = ( aui_handle_music * )MALLOC( sizeof( aui_handle_music ) );
        
        if ( NULL == *pp_handle_music ) {
            aui_rtn( AUI_RTN_ENOMEM, "*pp_handle_music is null\n" );
        }
        
        MEMSET( ( aui_handle_music * )( *pp_handle_music ), 0, sizeof( aui_handle_music ) );
        MEMCPY( &( ( ( aui_handle_music * )( *pp_handle_music ) )->attr_music ),
                pst_music_attr,
                sizeof( aui_attr_music ) );
        g_music_handle = *pp_handle_music;
        rtn_code = AUI_RTN_SUCCESS;
    } else {
        AUI_ERR( "Already opened." );
    }
    
    return rtn_code;
}


AUI_RTN_CODE aui_music_close( aui_attr_music *pst_music_attr, void **pp_handle_music )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( pst_music_attr != NULL ) {
        AUI_ERR( "pst_music_attr:%x may not be used!\n" );
    }
    
    if ( pp_handle_music == NULL ) {
        aui_rtn( AUI_RTN_EINVAL, "pp_handle_music is NULL\n" );
    }
    
    if ( g_music_handle != *pp_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "*pp_handle_music is wrong\n" );
    }
    
    if ( ( *pp_handle_music ) != NULL ) {
        FREE( *pp_handle_music );
        g_music_handle = NULL;
    }
    pe_cleanup(); // free malloc memory in PE
    return rtn_code;
}

AUI_RTN_CODE aui_music_start( void *pv_handle_music )
{
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    int ret_val = 0;
    struct pe_music_cfg p_music_cfg;
    struct pe_image_cfg p_image_cfg;
    struct pe_video_cfg p_video_cfg;
    struct pe_video_cfg_extra p_video_cfg_extra;
    
    memset( &p_music_cfg, 0, sizeof( struct pe_music_cfg ) );
    memset( &p_image_cfg, 0, sizeof( struct pe_image_cfg ) );
    memset( &p_video_cfg, 0, sizeof( struct pe_video_cfg ) );
    memset( &p_video_cfg_extra, 0, sizeof( struct pe_video_cfg_extra ) );
    
    //AUI_DEBUG("music addr:%x, image:%x, video:%x", &p_music_cfg, &p_image_cfg, &p_video_cfg);
    
    p_music_cfg.pcm_out_buff = g_music_cfg.ul_music_buffer & 0x8fffffff;                            
    p_music_cfg.pcm_out_buff_size = DECODER_BUF_LEN & 0x8fffffff;             
    p_music_cfg.processed_pcm_buff = ( p_music_cfg.pcm_out_buff + p_music_cfg.pcm_out_buff_size ) & 0x8fffffff;      
    p_music_cfg.processed_pcm_buff_size = ( g_music_cfg.ul_len - DECODER_BUF_LEN ) & 0x8fffffff; 
    p_music_cfg.mp_cb = commom_music_callback_wrapper;                                              
        
    aui_get_mp_config( &p_video_cfg, &p_video_cfg_extra );
    
    pe_cleanup();
    ret_val = pe_init( &p_music_cfg, &p_image_cfg, &p_video_cfg );
    if ( -1 == ret_val ) {
        return AUI_RTN_FAIL;
    }
    unsigned char *pc_file_name = ( ( ( aui_handle_music * )( pv_handle_music ) )->attr_music ).uc_file_name;
    
    ret_val = music_play( ( char * )pc_file_name );
    
    if ( ret_val < 0 ) {
        aui_rtn( AUI_RTN_FAIL, "\n start file fail! \n" );
    }
    
    return rtn_code;
}


AUI_RTN_CODE aui_music_stop( void *pv_handle_music )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null\n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    music_stop();
    
    return rtn_code;
}

AUI_RTN_CODE aui_music_pause( void *pv_handle_music )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    music_pause();
    
    return rtn_code;
}


AUI_RTN_CODE aui_music_resume( void *pv_handle_music )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    music_pause();
    
    return rtn_code;
    
}



AUI_RTN_CODE aui_music_de_init( aui_func_music_init fn_music_de_init )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == fn_music_de_init ) {
        aui_rtn( AUI_RTN_EINVAL, "fn_music_de_init is null \n" );
    }
    fn_music_de_init();
    return rtn_code;
}

AUI_RTN_CODE aui_music_seek( void *pv_handle_music, unsigned long ul_time_in_second )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    int cur_time = music_get_time();
    cur_time -= 2 * cur_time;
    music_seek( cur_time );
    music_seek( ul_time_in_second );
    return rtn_code;
    
}


AUI_RTN_CODE aui_music_total_time_get( void *pv_handle_music, unsigned int *pui_total_time_in_ms )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    music_info msc_info;
    int info_ret = 0;
    
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    if ( NULL == pui_total_time_in_ms ) {
        aui_rtn( AUI_RTN_EINVAL, "pui_total_time_in_ms is null \n" );
    }
    
    unsigned char *pc_file_name = ( ( ( aui_handle_music * )( pv_handle_music ) )->attr_music ).uc_file_name;
    
    
    MEMSET( &msc_info, 0, sizeof( music_info ) );
    info_ret = music_get_song_info( ( char * )pc_file_name, &msc_info );
    if ( ( info_ret == 2 ) || ( info_ret == 3 ) ) {
        ;
    } else {
        aui_rtn( AUI_RTN_FAIL, "\n get info fail! \n" );
    }
    
    *pui_total_time_in_ms = msc_info.time;
    return rtn_code;
}


AUI_RTN_CODE aui_music_cur_time_get( void *pv_handle_music, unsigned int *pui_cur_time_in_ms )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    if ( NULL == pui_cur_time_in_ms ) {
        aui_rtn( AUI_RTN_EINVAL, "pui_cur_time_in_ms is null \n" );
    }
    
    *pui_cur_time_in_ms = music_get_time();
    return rtn_code;
}

AUI_RTN_CODE aui_music_set_playend_callback( void *pv_handle_music, enum aui_music_message msg,
                                             fn_music_end_callback mc_cb )
{
    ( void ) msg;
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is null \n" );
    }
    
    if ( g_music_handle != pv_handle_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_music is wrong\n" );
    }
    
    pe_music_end_callback = mc_cb;
    return rtn_code;
}

AUI_RTN_CODE aui_music_set( void *pv_hdl_music, unsigned long ul_item, void *pv_param )
{
    ( void ) pv_hdl_music;
    ( void ) ul_item;
    ( void ) pv_param;
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_music_get( void *pv_hdl_music, unsigned long ul_item, void *pv_param )
{
    if ( ( NULL == pv_hdl_music ) || ( NULL == pv_param ) ) {
        AUI_ERR( "fail to get the right address!\n" );
        return AUI_RTN_FAIL;
    }
    
    if ( g_music_handle != pv_hdl_music ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_hdl_music is wrong\n" );
    }
    
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    int ret_info = 0;
    aui_handle_music *pst_hdl_music = pv_hdl_music;
    music_info music_info;
    aui_music_info *p_music_info = ( aui_music_info * ) pv_param;
    unsigned char *pc_file_name = NULL;
    decoder_info decoder_info;
    aui_decoder_info *p_decoder_info = ( aui_decoder_info * ) pv_param;
    
    //music_info = pv_param;
    pc_file_name = pst_hdl_music->attr_music.uc_file_name;
    //decoder_info = pv_param;
    
    switch ( ul_item ) {
        case AUI_MUSIC_GET_MUSIC_INFO:
            memset( &music_info, 0, sizeof( music_info ) );
            memset( p_music_info, 0, sizeof( aui_music_info ) );
            
            ret_info = music_get_song_info( ( char * )pc_file_name, &music_info );
            if ( ( ret_info == 2 ) || ( ret_info == 3 ) ) {
                memcpy( p_music_info->title, music_info.title, sizeof( music_info.title ) );
                memcpy( p_music_info->artist, music_info.artist, sizeof( music_info.artist ) );
                memcpy( p_music_info->album, music_info.album, sizeof( music_info.album ) );
                memcpy( p_music_info->year, music_info.year, sizeof( music_info.year ) );
                memcpy( p_music_info->comment, music_info.comment, sizeof( music_info.comment ) );
                p_music_info->file_length = music_info.file_length;
                p_music_info->genre = music_info.genre;
                p_music_info->time = music_info.time;
                p_music_info->track = music_info.track;
            } else {
                aui_rtn( AUI_RTN_FAIL, "\n get info fail! \n" );
            }
            break;
        case AUI_MUSIC_GET_DECODER_INFO:
            memset( &decoder_info, 0, sizeof( decoder_info ) );
            memset( p_decoder_info, 0, sizeof( aui_decoder_info ) );
            
            ret_info = music_get_decoder_info( ( char * )pc_file_name, &decoder_info );
            p_decoder_info->bit_rate = decoder_info.bit_rate;
            p_decoder_info->channel_mode = decoder_info.channel_mode;
            p_decoder_info->sample_rate = decoder_info.sample_rate;
            /*
            if((ret_info == 2) || (ret_info == 3))
            {
                ;
            }
            else
            {
                aui_rtn(AUI_MODULE_MUSIC,MUSIC_ERR,"\n get info fail! \n");
            }
            */
            /* need to check,not used in our demo */
            
            break;
        default:
            break;
    }
    return rtn_code;
}
