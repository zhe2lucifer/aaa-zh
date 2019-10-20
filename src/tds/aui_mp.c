/**@file
*     @brief         AUI decv module interface implement
*     @author        henry.xie
*     @date              2013-6-27
*     @version           1.0.0
*     @note              ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"

#include <aui_mp.h>
#include <aui_av.h>
#include <api/libmp/pe.h>
#include <api/libmp/media_player_api.h>
/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

AUI_MODULE(MPLAY);
typedef struct aui_mp_buf_cfg {
    /*
    Buffer that will be used to allocate memory for some buffer in video
    engine process, such as ring buffer, which will be used to store PCM data in decoding audio process;
    and display buffer which will be used to store some special frame data, and cmd queue buffer used
    to store decode information. And in retail scheme, pe_cache_buf can also malloc memory from here.*/
    unsigned long ul_mp_decoder_buf;
    //the size of decoder_buf, the recommend value is 0x754f00
    unsigned long ul_mp_decoder_buf_len;
    //buffer used to store frame data after decoding video file by video driver.
    //unsigned long ul_frame_buf;
    //the length of frame buffer, the recommend value is 0x1da4800
    //unsigned long ul_frame_buf_len;
    //buffer used to download the source data for see CPU reading.
    unsigned long ul_pe_cache_buf;
    //the length of pe cache buffer, and if set ul_pe_cache_buf= p_cfg->ul_frame_buf,
    //media player engine will allocate pe_cache buffer from ul_frame_buf. the size is
    //0x120000 byte
    unsigned long ul_pe_cache_buf_len;
    //buffer to store private address
    unsigned long ul_mp_private_buf;
    //the length of ul_mp_private_buf
    unsigned long ul_mp_private_buf_len;
} aui_mp_buf_cfg;

typedef struct {
    /** attribute of the container */
    aui_attr_mp         attr_mp;
    /** video track id */
    unsigned int ui_video_id;
    /** audio track id */
    unsigned int ui_audio_id;
    /** subtitle track id */
    unsigned int ui_subtitle_id;
    /**flag of stop play*/
    unsigned long end_flag;
    
    enum aui_mp_speed     en_mp_speed;
} aui_handle_mp, *aui_p_handle_mp;

/****************************LOCAL VAR********************************************/
pn_message_callback pe_callback[AUI_MP_MESSAGE_MAX] = {0};
static aui_handle_mp *g_mp_handle = NULL;

/****************************LOCAL FUNC DECLEAR***********************************/
extern DWORD mpg_cmd_stop_proc( int stop_type );
extern int video_dec_file( char *path, BOOL preview );
extern DWORD mpg_cmd_pause_proc();
extern DWORD mpg_cmd_play_proc();
extern DWORD mpg_cmd_search_proc( DWORD search_time );
extern DWORD MPGGetTotalPlayTimeMS();
extern DWORD MPGFileDecoderGetPlayTimeMS();
extern DWORD mpg_cmd_change_prog ( int prog_id );
extern DWORD mpg_cmd_change_audio_track( INT32 *aud_pid );
extern DWORD mpg_cmd_change_subtitle( INT32 sub_pid );

extern DWORD mpg_file_get_stream_info( PDEC_STREAM_INFO p_dec_stream_info );
extern AUI_RTN_CODE aui_get_music_config( struct pe_music_cfg *config );


/****************************MODULE DRV IMPLEMENT*************************************/

extern int get_next_play_state( void );
void commom_mp_callback_wrapper( UINT32 param1, UINT32 param2 )
{
    switch ( param1 ) {
    
        case MP_VIDEO_PLAYBACK_END:
            if ( pe_callback[AUI_MP_PLAY_END] != NULL ) {
                ( pe_callback[AUI_MP_PLAY_END] )();
            }
            
            if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_PLAY_END,
                                                       ( void * )param2,
                                                       g_mp_handle->attr_mp.user_data );
            }
            if ( NULL == g_mp_handle ) {
                AUI_ERR( "g_mp_handle is NULL!\n" );
                return;
            }
            g_mp_handle->end_flag = 1;
            break;
        case MP_VIDEO_PARSE_END:
        
            if ( param2 & MP_DERR_UNSUPPORTED_VIDEO_CODEC ) {
            
                if ( pe_callback[AUI_MP_VIDEO_CODEC_NOT_SUPPORT] != NULL ) {
                
                    ( pe_callback[AUI_MP_VIDEO_CODEC_NOT_SUPPORT] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_VIDEO_CODEC_NOT_SUPPORT,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( param2 & MP_DERR_UNSUPPORTED_RESOLUTION ) {
            
                if ( pe_callback[AUI_MP_RESOLUTION_NOT_SUPPORT] != NULL ) {
                
                    ( pe_callback[AUI_MP_RESOLUTION_NOT_SUPPORT] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_RESOLUTION_NOT_SUPPORT,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( param2 & MP_DERR_UNKNOWN ) {
            
                if ( pe_callback[AUI_MP_ERROR_UNKNOWN] != NULL ) {
                
                    ( pe_callback[AUI_MP_ERROR_UNKNOWN] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_ERROR_UNKNOWN,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( param2 & MP_DERR_UNSUPPORTED_AUDIO_CODEC ) {
            
                if ( pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT] != NULL ) {
                
                    ( pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_AUDIO_CODEC_NOT_SUPPORT,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( 0 == param2 ) {
            
                if ( pe_callback[AUI_MP_PLAY_BEGIN] != NULL ) {
                
                    ( pe_callback[AUI_MP_PLAY_BEGIN] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_PLAY_BEGIN,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            }
            break;
        case MP_VIDEO_DECODER_ERROR:
            if ( pe_callback[AUI_MP_DECODE_ERROR] != NULL ) {
            
                ( pe_callback[AUI_MP_DECODE_ERROR] )();
            }
            if ( param2 & MP_DERR_UNSUPPORTED_VIDEO_CODEC ) {
            
                if ( pe_callback[AUI_MP_VIDEO_CODEC_NOT_SUPPORT] != NULL ) {
                
                    ( pe_callback[AUI_MP_VIDEO_CODEC_NOT_SUPPORT] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_VIDEO_CODEC_NOT_SUPPORT,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( param2 & MP_DERR_UNSUPPORTED_RESOLUTION ) {
            
                if ( pe_callback[AUI_MP_RESOLUTION_NOT_SUPPORT] != NULL ) {
                
                    ( pe_callback[AUI_MP_RESOLUTION_NOT_SUPPORT] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_RESOLUTION_NOT_SUPPORT,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( param2 & MP_DERR_TOO_BIG_INTERLEAVE ) {
            
                if ( pe_callback[AUI_MP_NO_MEMORY] != NULL ) {
                    ( pe_callback[AUI_MP_NO_MEMORY] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_NO_MEMORY,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            } else if ( param2 & MP_DERR_UNSUPPORTED_AUDIO_CODEC ) {
            
                if ( pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT] != NULL ) {
                
                    ( pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT] )();
                }
                if ( g_mp_handle && g_mp_handle->attr_mp.aui_mp_stream_cb ) {
                    g_mp_handle->attr_mp.aui_mp_stream_cb( AUI_MP_AUDIO_CODEC_NOT_SUPPORT,
                                                           0,
                                                           g_mp_handle->attr_mp.user_data );
                }
            }
            break;
        default:
            break;
    }
    return;
}

static aui_mp_buf_cfg g_video_cfg;
AUI_RTN_CODE aui_mp_memory_info_set( unsigned long addr1, unsigned long len1, unsigned long addr2,
                                     unsigned long len2, unsigned long addr3, unsigned long len3 )
{
    g_video_cfg.ul_mp_decoder_buf = addr1;
    g_video_cfg.ul_mp_decoder_buf_len = len1;
    g_video_cfg.ul_pe_cache_buf = addr2;
    g_video_cfg.ul_pe_cache_buf_len = len2;
    g_video_cfg.ul_mp_private_buf = addr3;
    g_video_cfg.ul_mp_private_buf_len = len3;
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_get_mp_config( struct pe_video_cfg *config, struct pe_video_cfg_extra *extra )
{
    if ( ( NULL == config ) || ( NULL == extra ) ) {
        AUI_ERR( "fail to get the right input!\n" );
        return AUI_RTN_FAIL;
    }
    struct pe_video_cfg *mp_cfg = ( struct pe_video_cfg * )config;
    
    mp_cfg->decoder_buf = g_video_cfg.ul_mp_decoder_buf;
    mp_cfg->decoder_buf_len = g_video_cfg.ul_mp_decoder_buf_len;
    //used to deceting vbv data.
    mp_cfg->mm_vbv_len = __MM_VBV_LEN;
    //Used to identify whether support seek action in video engine. Special used in network mode.
    //As some network link do not support seek action, the video engine need a flag to tell them to
    //do relevant operation
    mp_cfg->disable_seek = TRUE;
    //Point to the callback function regestered into video engine by AUI, which can be used
    //to transfer some information of video engine and necessory work status to AUI.
    mp_cfg->mp_cb = commom_mp_callback_wrapper;
    mp_cfg->set_start_play_time_ms = 0;
    //just used in network mode to identify a mechnism that optimize the start time of webcasting.
    mp_cfg->new_retry_mechanism = TRUE;
    extra->frame_buf_base = __MM_FB_BOTTOM_ADDR;
    extra->frame_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    
    extra->reserved[2] = g_video_cfg.ul_pe_cache_buf;
    extra->reserved[3] = g_video_cfg.ul_pe_cache_buf_len;
    extra->reserved[6] = g_video_cfg.ul_mp_private_buf;
    extra->reserved[7] = g_video_cfg.ul_mp_private_buf_len;
    mp_cfg->reserved = ( unsigned long )( extra );
#ifndef SD_PVR
    mp_cfg->set_sbm_size = 0;
#else
    mp_cfg->set_sbm_size = 0x300000;
#endif
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_mp_init( aui_func_mp_init fn_mp_init )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == fn_mp_init ) {
        aui_rtn( AUI_RTN_EINVAL, NULL );
    }
    fn_mp_init();
    return rtn_code;
}
AUI_RTN_CODE aui_mp_open( aui_attr_mp *pst_mp_attr, void **pp_handle_mp )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    
    if ( NULL == pst_mp_attr ) {
        aui_rtn( AUI_RTN_EINVAL, "pst_mp_attr is null!\n" );
    }
    
    if ( g_mp_handle == NULL ) {
        *pp_handle_mp = ( aui_handle_mp * )malloc( sizeof( aui_handle_mp ) );
        
        if ( NULL == *pp_handle_mp ) {
            aui_rtn( AUI_RTN_ENOMEM, "pp_handle_mp is null!\n" );
        }
        
        MEMSET( ( aui_handle_mp * )( *pp_handle_mp ), 0, sizeof( aui_handle_mp ) );
        MEMCPY( &( ( ( aui_handle_mp * )( *pp_handle_mp ) )->attr_mp ),
                pst_mp_attr,
                sizeof( aui_attr_mp ) );
        g_mp_handle = *pp_handle_mp;
        rtn_code = AUI_RTN_SUCCESS;
    } else {
        AUI_ERR( "Already opened." );
    }
    
    return rtn_code;
}


AUI_RTN_CODE aui_mp_close( aui_attr_mp *pst_mp_attr, void **pp_handle_mp )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( pp_handle_mp == NULL ) {
        aui_rtn( AUI_RTN_EINVAL, "pp_handle_mp is NULL\n" );
    }
    
    if ( g_mp_handle != *pp_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the *pp_handle_mp is wrong\n" );
    }
    
    if ( pst_mp_attr != NULL ) {
        AUI_DBG( "pst_mp_attr:%x may not be used!\n" );
    }
    
    if ( ( *pp_handle_mp ) != NULL ) {
        free( *pp_handle_mp );
        g_mp_handle = NULL;
    }
    pe_cleanup(); // free malloc memory in PE
    return rtn_code;
}

AUI_RTN_CODE aui_mp_start( void *pv_handle_mp )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    struct pe_music_cfg p_music_cfg;
    struct pe_image_cfg p_image_cfg;
    struct pe_video_cfg p_video_cfg;
    struct pe_video_cfg_extra p_video_cfg_extra;
    //initial end_flag=0 to avoid getting wrong currenttime when start again in the condition
    //that do not open again
    g_mp_handle->end_flag = 0;
    
    memset( &p_music_cfg, 0, sizeof( struct pe_music_cfg ) );
    memset( &p_image_cfg, 0, sizeof( struct pe_image_cfg ) );
    memset( &p_video_cfg, 0, sizeof( struct pe_video_cfg ) );
    memset( &p_video_cfg_extra, 0, sizeof( struct pe_video_cfg_extra ) );
    
    p_video_cfg.decoder_buf = g_video_cfg.ul_mp_decoder_buf;
    p_video_cfg.decoder_buf_len = g_video_cfg.ul_mp_decoder_buf_len;
    //used to deceting vbv data.
    p_video_cfg.mm_vbv_len = __MM_VBV_LEN;
    //Used to identify whether support seek action in video engine. Special used in network mode.
    //As some network link do not support seek action, the video engine need a flag to tell them to
    //do relevant operation
    p_video_cfg.disable_seek = TRUE;
    //Point to the callback function regestered into video engine by AUI, which can be used
    //to transfer some information of video engine and necessory work status to AUI.
    p_video_cfg.mp_cb = commom_mp_callback_wrapper;
    p_video_cfg.set_start_play_time_ms = ( ( ( aui_handle_mp * )( pv_handle_mp ) )->attr_mp ).start_time;
    //just used in network mode to identify a mechnism that optimize the start time of webcasting.
    p_video_cfg.new_retry_mechanism = TRUE;
    p_video_cfg_extra.frame_buf_base = __MM_FB_BOTTOM_ADDR;
    p_video_cfg_extra.frame_buf_len = __MM_FB_TOP_ADDR - __MM_FB_BOTTOM_ADDR;
    //used to point to the information table of subtile typeface
    //p_video_cfg_extra.reserved[0] = 0;
    //the number of elements in the information table of subtile typeface
    //p_video_cfg_extra.reserved[1] = 0;
    p_video_cfg_extra.reserved[2] = g_video_cfg.ul_pe_cache_buf;
    p_video_cfg_extra.reserved[3] = g_video_cfg.ul_pe_cache_buf_len;
    p_video_cfg_extra.reserved[6] = g_video_cfg.ul_mp_private_buf;
    p_video_cfg_extra.reserved[7] = g_video_cfg.ul_mp_private_buf_len;
    p_video_cfg.reserved = ( unsigned long )( &p_video_cfg_extra );
#ifndef SD_PVR
    /*As the MP module is sharing memory with PVR module, and the memory of 
    MP module will allocate 10M to SBM which is used to cache data of video file 
    in driver, and as the memory size of SBM can just use 4M to ensure the video 
    file play success, so the memory size of MP module can be reduced by 6M, 
    that is, the memory size of PVR module can be reduced by 6M.The heap space 
    of aui version will increase 6M.*/

    p_video_cfg.set_sbm_size = 0x400000;
#else
    p_video_cfg.set_sbm_size = 0x300000;
#endif
    
    aui_get_music_config( &p_music_cfg );
    pe_cleanup();
    rtn_code = pe_init( &p_music_cfg, &p_image_cfg, &p_video_cfg );
    if ( -1 == rtn_code ) {
        return AUI_RTN_FAIL;
    }
    unsigned char *pc_file_name = ( ( ( aui_handle_mp * )( pv_handle_mp ) )->attr_mp ).uc_file_name;
    unsigned char b_preview = ( ( ( aui_handle_mp * )( pv_handle_mp ) )->attr_mp ).b_is_preview;
    ( ( aui_handle_mp * )( pv_handle_mp ) )->en_mp_speed = AUI_MP_SPEED_1;
    h264_decoder_select( 0, b_preview );
    if ( 0 != video_dec_file( ( char * )pc_file_name, b_preview ) ) {
        aui_rtn( AUI_RTN_FAIL, "\n dec file fail! \n" );
    }
    return rtn_code;
}


AUI_RTN_CODE aui_mp_stop( void *pv_handle_mp )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( 0 != mpg_cmd_stop_proc( 0 ) ) {
        aui_rtn( AUI_RTN_FAIL, "stop fail !\n" );
    }
    return rtn_code;
}

AUI_RTN_CODE aui_mp_pause( void *pv_handle_mp )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( 0 != mpg_cmd_pause_proc() ) {
        aui_rtn( AUI_RTN_FAIL, "\n mp pause fail! \n" );
    }
    return rtn_code;
}


AUI_RTN_CODE aui_mp_resume( void *pv_handle_mp )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( 0 != mpg_cmd_play_proc() ) {
        aui_rtn( AUI_RTN_FAIL, "\n mp play fail! \n" );
    }
    return rtn_code;
}

AUI_RTN_CODE aui_mp_de_init( aui_func_mp_init fn_mp_de_init )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == fn_mp_de_init ) {
        aui_rtn( AUI_RTN_EINVAL, "fn_mp_de_init is null!\n" );
    }
    fn_mp_de_init();
    return rtn_code;
}

AUI_RTN_CODE aui_mp_seek( void *pv_handle_mp, unsigned long ul_time_in_ms )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( 0 != mpg_cmd_search_proc( ul_time_in_ms / 1000 ) ) {
        aui_rtn( AUI_RTN_FAIL, "\n mp search fail! \n" );
    }
    return rtn_code;
}


AUI_RTN_CODE aui_mp_speed_set( void *pv_handle_mp, enum aui_mp_speed en_speed )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    AF_PE_PLAY_SPEED speed;
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    switch ( en_speed ) {
        case AUI_MP_SPEED_FASTREWIND_24:
            speed = PE_SPEED_FASTREWIND_24;
            break;
        case AUI_MP_SPEED_FASTREWIND_16:
            speed = PE_SPEED_FASTREWIND_16;
            break;
        case AUI_MP_SPEED_FASTREWIND_8:
            speed = PE_SPEED_FASTREWIND_8;
            break;
        case AUI_MP_SPEED_FASTREWIND_4:
            speed = PE_SPEED_FASTREWIND_4;
            break;
        case AUI_MP_SPEED_FASTREWIND_2:
            speed = PE_SPEED_FASTREWIND_2;
            break;
        case AUI_MP_SPEED_SLOWREWIND_2:
            speed = PE_SPEED_SLOWREWIND_2;
            break;
        case AUI_MP_SPEED_SLOWREWIND_4:
            speed = PE_SPEED_SLOWREWIND_4;
            break;
        case AUI_MP_SPEED_SLOWREWIND_8:
            speed = PE_SPEED_SLOWREWIND_8;
            break;
        case AUI_MP_SPEED_SLOWFORWARD_2:
            speed = PE_SPEED_SLOWFORWARD_2;
            break;
        case AUI_MP_SPEED_SLOWFORWARD_4:
            speed = PE_SPEED_SLOWFORWARD_4;
            break;
        case AUI_MP_SPEED_SLOWFORWARD_8:
            speed = PE_SPEED_SLOWFORWARD_8;
            break;
        case AUI_MP_SPEED_FASTFORWARD_2:
            speed = PE_SPEED_FASTFORWARD_2;
            break;
        case AUI_MP_SPEED_FASTFORWARD_4:
            speed = PE_SPEED_FASTFORWARD_4;
            break;
        case AUI_MP_SPEED_FASTFORWARD_8:
            speed = PE_SPEED_FASTFORWARD_8;
            break;
        case AUI_MP_SPEED_FASTFORWARD_16:
            speed = PE_SPEED_FASTFORWARD_16;
            break;
        case AUI_MP_SPEED_FASTFORWARD_24:
            speed = PE_SPEED_FASTFORWARD_24;
            break;
        case AUI_MP_SPEED_1:
            speed = PE_SPEED_NORMAL;
            break;
        default:
            aui_rtn( AUI_RTN_FAIL, "\n mp set wrong speed! \n" );
            break;
    }
    ( ( aui_handle_mp * )( pv_handle_mp ) )->en_mp_speed = en_speed;
    if ( 0 != mpg_cmd_set_speed( speed ) ) {
        aui_rtn( AUI_RTN_FAIL, "\n mp set speed fail! \n" );
    }
    return rtn_code;
}

AUI_RTN_CODE aui_mp_total_time_get( void *pv_handle_mp, unsigned int *pui_total_time )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    DEC_STREAM_INFO mp_info;
    
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( NULL == pui_total_time ) {
        aui_rtn( AUI_RTN_EINVAL, "pui_total_time is null!\n" );
    }
    
    mpg_file_get_stream_info( &mp_info );
    *pui_total_time = mp_info.total_time;
    return rtn_code;
}

AUI_RTN_CODE aui_mp_cur_time_get( void *pv_handle_mp, unsigned int *pui_cur_time )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( NULL == pui_cur_time ) {
        aui_rtn( AUI_RTN_EINVAL, "pui_total_time is null!\n" );
    }
    
    *pui_cur_time = mpgfile_decoder_get_play_time_ms() / 1000;
    return rtn_code;
}

AUI_RTN_CODE aui_mp_get_total_time( void *pv_handle_mp, unsigned int *pui_total_time )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, NULL );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( NULL == pui_total_time ) {
        aui_rtn( AUI_RTN_EINVAL, NULL );
    }
    
    *pui_total_time = mpgget_total_play_time_ms();
    return rtn_code;
}

AUI_RTN_CODE aui_mp_get_cur_time( void *pv_handle_mp, unsigned int *pui_cur_time )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, NULL );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( NULL == pui_cur_time ) {
        aui_rtn( AUI_RTN_EINVAL, NULL );
    }
    //As when the video play end, the driver do not close the thread, mpgfile_decoder_get_play_time()
    //will return a wrong value. So when the end_flag is 0, call the interface; when the end_flag is 1,do
    //not call the function.When get MP_VIDEO_PLAYBACK_END message, let end_flag=1.
    if ( g_mp_handle->end_flag ) {
        *pui_cur_time = 0;
    } else {
        *pui_cur_time = mpgfile_decoder_get_play_time();
    }
    return rtn_code;
}


extern void pecache_reg_filehdl( void *hdl );
AUI_RTN_CODE aui_mp_set( void *pv_hdl_mp, unsigned long ul_item, void *pv_param )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_hdl_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_hdl_mp is null!\n" );
    }
    if ( g_mp_handle != pv_hdl_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_hdl_mp is wrong\n" );
    }
    
    switch ( ul_item ) {
        case AUI_MP_SET_VIDEO_ID:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            
            ( ( aui_handle_mp * )( pv_hdl_mp ) )->ui_video_id = *( ( unsigned int * )pv_param );
            if ( 0 != mpg_cmd_change_prog( *( ( unsigned int * )pv_param ) ) ) {
                aui_rtn( AUI_RTN_FAIL, "\n mp change program fail! \n" );
            }
            break;
            
        case AUI_MP_SET_AUDIO_ID:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            
            ( ( aui_handle_mp * )( pv_hdl_mp ) )->ui_audio_id = *( ( unsigned int * )pv_param );
            if ( 0 != mpg_cmd_change_audio_track( pv_param ) ) {
                aui_rtn( AUI_RTN_FAIL, "\n mp change audio track fail! \n" );
            }
            break;
            
        case AUI_MP_SET_SUBTITLE_ID:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            
            ( ( aui_handle_mp * )( pv_hdl_mp ) )->ui_subtitle_id = *( ( unsigned int * )pv_param );
            if ( 0 != mpg_cmd_change_subtitle( *( ( unsigned int * )pv_param ) ) ) {
                aui_rtn( AUI_RTN_FAIL, "\n mp change subtitle track fail! \n" );
            }
            break;
        case AUI_MP_SET_FD_CB_REG: {
            aui_pe_cache_cb_fp aui_pe_cache_cb;
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            aui_pe_cache_cb = *( aui_pe_cache_cb_fp * )pv_param;
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb = aui_pe_cache_cb;
            pecache_reg_filehdl( &aui_pe_cache_cb );
            break;
        }
        case AUI_MP_SET_FD_CB_UNREG: {
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb.file_open  = NULL;
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb.file_read  = NULL;
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb.file_seek  = NULL;
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb.file_eof   = NULL;
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb.file_tell  = NULL;
            ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).aui_pe_cache_cb.file_close = NULL;
            pecache_reg_filehdl( NULL );
            break;
        }
        default:
            break;
    }
    return rtn_code;
}

AUI_RTN_CODE aui_mp_get( void *pv_hdl_mp, unsigned long ul_item, void *pv_param )
{

    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    DEC_STREAM_INFO mp_info;
    aui_mp_video_info *p_mp_info = ( ( aui_mp_video_info * )pv_param );
    
    
    if ( NULL == pv_hdl_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_hdl_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_hdl_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_hdl_mp is wrong\n" );
    }
    
    switch ( ul_item ) {
        case AUI_MP_GET_STREAM_INFO:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            
            mpg_file_get_stream_info( &mp_info );
            
            p_mp_info->audio_bitrate = mp_info.audio_bitrate;
            p_mp_info->audio_stream_num = mp_info.audio_stream_num;
            p_mp_info->sub_stream_num = mp_info.sub_stream_num;
            p_mp_info->total_frame_num = mp_info.total_frame_num;
            p_mp_info->frame_period = mp_info.frame_period;
            p_mp_info->total_time = mp_info.total_time;
            p_mp_info->width = mp_info.width;
            p_mp_info->height = mp_info.height;
            p_mp_info->video_bitrate = mp_info.video_bitrate;
            
            p_mp_info->audio_bitrate = mp_info.audio_bitrate;
            p_mp_info->audio_channel_num = mp_info.audio_channel_num;
            p_mp_info->file_size = mp_info.fsize;
            memcpy( p_mp_info->video_dec, mp_info.video_dec, 10 );
            memcpy( p_mp_info->audio_dec, mp_info.audio_dec, 10 );
            
#if 0
            MEMCPY( &( ( ( ( aui_handle_mp * )( pv_hdl_mp ) )->attr_mp ).st_stream_info ),
                    ( aui_mp_video_info ** )pv_param,
                    sizeof( aui_mp_video_info ) );
#endif
            break;
        case AUI_MP_GET_STREAM_INFO_SPEED:
            if ( NULL == pv_param ) {
                aui_rtn( AUI_RTN_EINVAL, "pv_param is null!\n" );
            }
            *( ( int * )pv_param ) = ( ( aui_handle_mp * )( pv_hdl_mp ) )->en_mp_speed;
            break;
            
        default:
            break;
    }
    return rtn_code;
}

AUI_RTN_CODE aui_mp_set_message_callback( void *pv_handle_mp, enum aui_mp_message msg, pn_message_callback func )
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if ( NULL == pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "pv_handle_mp is null!\n" );
    }
    
    if ( g_mp_handle != pv_handle_mp ) {
        aui_rtn( AUI_RTN_EINVAL, "the pv_handle_mp is wrong\n" );
    }
    
    if ( msg >= AUI_MP_MESSAGE_MAX ) {
        aui_rtn( AUI_RTN_EINVAL, "aui_mp_set_message_callback error!\n" );
    }
    
    pe_callback[msg] = func;
    return rtn_code;
}



AUI_RTN_CODE aui_mp_get_snapshot( void *pv_handle_mp )
{
    ( void )pv_handle_mp;
    
    return 0;
}

