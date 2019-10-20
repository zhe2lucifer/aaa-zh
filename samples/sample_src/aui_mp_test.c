/****************************INCLUDE FILE************************************/
#include <stdio.h>
#include <stdlib.h>
#include "aui_common.h"
#include "aui_common_list.h"
#include "aui_mp.h"
#include "aui_dis.h"

#ifdef AUI_TDS
    //#include <hld_dev.h>
    #include <api/libmp/pe.h>
#endif

#define __SHOW_AV__ 0
#if __SHOW_AV__
    #include "resource/aui_mp_ts_data.h"
#endif

#include "aui_test_app.h"
#include"aui_mp_test.h"
#include "aui_fs.h"
/****************************GLOBAL MACRO************************************/
extern void commom_mp_callback_wrapper( unsigned long param1, unsigned long param2 );

/****************************GLOBAL TYPE************************************/
#ifdef __cplusplus
extern "C" {
#endif
#ifdef AUI_TDS
static aui_hdl g_mp_hdl = NULL;

//the messages sent to the message queue to realize mp play function
typedef enum aui_mp_cmd_message {
    AUI_MP_OPEN = 1,
    
    AUI_MP_CLOSE,
    
    AUI_MP_START,
    
    AUI_MP_STOP,
    
    AUI_MP_PAUSE,
    
    AUI_MP_RESUME,
    
    AUI_MP_SEEK,
    
    AUI_MP_SPEED,
    
    AUI_MP_GET_INFO,
    
    AUI_MP_GET_TOTAL_TIME,
    
    AUI_MP_GET_CURRENT_TIME,
    
    AUI_MP_HELP
} aui_mp_cmd_message;

typedef struct _mp_task_info {
    aui_hdl task_hdl;
    aui_hdl msg_hdl;
    aui_attr_mp attr_mp;
    unsigned int seek_time;
    unsigned int speed;
    aui_mp_video_info p_mp_info;
    unsigned int total_time;
    unsigned int current_time;
} mp_task_info;
static mp_task_info g_mp_task_info = {NULL, NULL,};

/*********************************mp init start*********************************/







/*********************************mp init end***********************************/

static unsigned long get_num_by_string( char *str )
{
    unsigned char temp[128] = {0};
    unsigned long val = 0;
    sscanf( str, "%s", temp );
    if ( AUI_RTN_SUCCESS != str2ulong( temp, strlen( temp ), &val ) ) {
        return -1;
    }
    return val;
}



static void waiting_usb_fs_init()
{
    unsigned int fs_dev = 0;
    int actual_num;
    while ( ( aui_fs_get_alldevid( &fs_dev, 1, &actual_num ) == AUI_RTN_SUCCESS ) && ( fs_dev == 0 ) ) {
        osal_task_sleep( 10 );
    }
    
    AUI_PRINTF( "\n %s: %d\n", __FUNCTION__, __LINE__ );
}


/*********************************mp callback func start***********************************/
#if __SHOW_AV__//def FD_PATH_HEADER

unsigned long aui_mp_fd_open( char *path, char *mode )
{
    int ret = 0;
    mpdata_hdl *hdl  = NULL;
    unsigned long addr = 0;
    unsigned long len = 0;
    
    if ( path == NULL ) {
        return AUI_RTN_FAIL;
    }
    
    hdl = MALLOC( sizeof( mpdata_hdl ) );
    if ( hdl == NULL ) {
        return AUI_RTN_FAIL;
    }
    
    MEMSET( hdl, 0, sizeof( mpdata_hdl ) );
    hdl->rlock = OSAL_INVALID_ID;
    
    hdl->rlock = osal_mutex_create();
    if ( hdl->rlock == OSAL_INVALID_ID ) {
        aui_mp_fd_close( hdl );
        return AUI_RTN_FAIL;
    }
    
    hdl->data_addr = aui_mp_ts_data;//mp data addr in buffer
    hdl->data_len  = sizeof( aui_mp_ts_data );
    AUI_PRINTF( ".........hdl->data_addr     : 0x%x \n", hdl->data_addr );
    AUI_PRINTF( ".........hdl->data_len      : 0x%x \n", hdl->data_len );
    
    return ( unsigned long )hdl;
}

void aui_mp_fd_close( void *file )
{
    mpdata_hdl *hdl = NULL;
    
    if ( file ) {
        hdl = ( mpdata_hdl * )file;
        if ( hdl->rlock != OSAL_INVALID_ID ) {
            osal_mutex_delete( hdl->rlock );
        }
        FREE( hdl );
    }
}

//read data from buffer
int aui_mp_fd_read( void *ptr, int size, int nmemb, void *file )
{
    mpdata_hdl *hdl = NULL;
    int len = size * nmemb;
    
    if ( file == NULL || ptr == NULL || len == 0 ) {
        return 0;
    }
    
    hdl = ( mpdata_hdl * )file;
    if ( hdl->err ) {
        return 0;
    }
    
    osal_mutex_lock( hdl->rlock, OSAL_WAIT_FOREVER_TIME );
    
    if ( hdl->cur_off + len > hdl->data_len ) {
        len = hdl->data_len - hdl->cur_off;
    }
    
    if ( len > 0 ) {
    
        AUI_PRINTF( ".........aui_mp_fd_read  len     : 0x%x \n", len );
        MEMCPY( ptr, ( hdl->data_addr + hdl->cur_off ), len );
        if ( len > 0 ) {
            hdl->cur_off += len;
        } else {
            hdl->err = 1;
            len = 0;
        }
        
        AUI_PRINTF( ".........aui_mp_fd_read  addr   : 0x%x \n", hdl->cur_off );
        
    }
    osal_mutex_unlock( hdl->rlock );
    return len;
}

int aui_mp_fd_write( void *ptr, int size, int nmemb, void *file )
{
    //aui_dbg_printf("unsupport!!\n");
    return 0;
}

int aui_mp_fd_seek( void *file, long long offset, int whence )
{
    int ret = AUI_RTN_SUCCESS;
    mpdata_hdl *hdl = NULL;
    unsigned long off = ( unsigned long ) offset;
    
    if ( file == NULL ) {
        return -1;
    }
    
    hdl = ( mpdata_hdl * )file;
    osal_mutex_lock( hdl->rlock, OSAL_WAIT_FOREVER_TIME );
    switch ( whence ) {
        case SEEK_SET:
            hdl->cur_off = off;
            break;
        case SEEK_CUR:
            hdl->cur_off += off;
            break;
        case SEEK_END:
            hdl->cur_off = hdl->data_len;
            break;
        default:
            ret = -1;
            break;
    }
    osal_mutex_unlock( hdl->rlock );
    
    //aui_dbg_printf("seek: whence:%d, offset:%d, ret:%d\n", whence, offset, ret);
    return ret;
}

long long aui_mp_fd_tell( void *file )
{
    mpdata_hdl *hdl = NULL;
    
    if ( file == NULL ) {
        return 0;
    }
    
    hdl = ( mpdata_hdl * )file;
    return ( long long )hdl->cur_off;
}


int aui_mp_fd_eof( void *file )
{
    int eof = 0;
    mpdata_hdl *hdl = NULL;
    
    if ( file == NULL ) {
        return AUI_RTN_FAIL;
    }
    
    hdl = ( mpdata_hdl * )file;
    eof = ( ( hdl->cur_off == hdl->data_len )
            || ( hdl->err ) );
    //aui_dbg_printf("eof: %d\n", eof);
    return eof;
}


int aui_mp_fd_cb_test()
{
    aui_pe_cache_cb_fp aui_mp_fd_cb = {
        .file_open = aui_mp_fd_open,
        .file_read = aui_mp_fd_read,
        .file_tell = aui_mp_fd_tell,
        .file_eof = aui_mp_fd_eof,
        .file_seek = aui_mp_fd_seek,
        .file_close = aui_mp_fd_close
    };
    AUI_TEST_CHECK_RTN( aui_mp_set( g_mp_hdl, AUI_MP_SET_FD_CB_REG, &aui_mp_fd_cb ) );
    return AUI_RTN_SUCCESS;
}
#endif

static void start_to_play( void )
{
    AUI_PRINTF( "start to play\n" );
    return;
}
static void video_play_end( void )
{
    AUI_PRINTF( "video play end!\n" );
    
    aui_mp_cmd_message mp_cmd_msg;
    int ret;
    
    AUI_PRINTF( "start to play\n" );
    mp_cmd_msg = AUI_MP_START;
    //send message to message queue
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        return;
    }
    return;
}
static void unsupport_video_codec( void )
{
    AUI_PRINTF( "UNSUPPORTED_VIDEO_CODEC\n" );
    return;
}
static void unsupport_resolution( void )
{
    AUI_PRINTF( "MP_DERR_UNSUPPORTED_RESOLUTION\n" );
    return;
}
static void unsupport_frame_rate( void )
{
    AUI_PRINTF( "MP_DERR_UNSUPPORTED_FRAME_RATE\n" );
    return;
}
static void too_big( void )
{
    AUI_PRINTF( "MP_DERR_TOO_BIG_INTERLEAVE\n" );
    return;
}
static void unkonow( void )
{
    AUI_PRINTF( "MP_DERR_UNKNOWN\n" );
    return;
}

static void unsupport_audio_codec( void )
{
    AUI_PRINTF( "MP_DERR_UNSUPPORTED_AUDIO_CODEC\n" );
    return;
}

static void decode_error( void )
{
    AUI_PRINTF( "AUI_MP_DECODE_ERROR\n" );
    return;
}

unsigned long test_aui_mp_set_callback( unsigned long *argc, char **argv, char *sz_out_put )
{
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_PLAY_BEGIN, start_to_play ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_PLAY_END, video_play_end ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_VIDEO_CODEC_NOT_SUPPORT, unsupport_video_codec ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_AUDIO_CODEC_NOT_SUPPORT, unsupport_audio_codec ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_RESOLUTION_NOT_SUPPORT, unsupport_resolution ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_FRAMERATE_NOT_SUPPORT, unsupport_frame_rate ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_NO_MEMORY, too_big ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_DECODE_ERROR, decode_error ) );
    AUI_TEST_CHECK_RTN( aui_mp_set_message_callback( g_mp_hdl, AUI_MP_ERROR_UNKNOWN, unkonow ) );
    return AUI_RTN_SUCCESS;
}

/*********************************mp callback func end***********************************/


static long test_mp_set_full_view( void )
{
    void *dis_hdl = NULL;
    aui_attr_dis attr_dis;
    
    MEMSET( &attr_dis, 0 , sizeof( aui_attr_dis ) );
    
    attr_dis.uc_dev_idx = AUI_DIS_HD;
    
    if ( aui_find_dev_by_idx( AUI_MODULE_DIS, AUI_DIS_HD, &dis_hdl ) ) {
        if ( aui_dis_open( &attr_dis, &dis_hdl ) ) {
            AUI_PRINTF( "\n aui_dis_open HD fail\n" );
            return -1;
        }
    }
    
    /* only for full view mode */
    AUI_TEST_CHECK_RTN( aui_dis_mode_set( dis_hdl, AUI_VIEW_MODE_FULL, NULL, NULL ) );
    
    return 0;
}

static void test_aui_mp_message_callback( aui_mp_message msg, void *pv_data, void *pv_user_data )
{
    unsigned long msg_code = ( unsigned long )pv_data;
    aui_mp_cmd_message mp_cmd_msg;
    int ret;
    switch ( msg ) {
        case AUI_MP_PLAY_BEGIN:
            AUI_PRINTF( "Got AUI_MP_PLAY_BEGIN, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_PLAY_END:
            AUI_PRINTF( "Got AUI_MP_PLAY_END, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            AUI_PRINTF( "start to play\n" );
            mp_cmd_msg = AUI_MP_START;
            //send message to the message, if the message is excepted, call aui_mp_start() function
            ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
            if ( AUI_RTN_SUCCESS != ret ) {
                AUI_PRINTF( "send message fail\n" );
                return;
            }
            break;
        case AUI_MP_VIDEO_CODEC_NOT_SUPPORT:
            AUI_PRINTF( "Got AUI_MP_VIDEO_CODEC_NOT_SUPPORT, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_RESOLUTION_NOT_SUPPORT:
            AUI_PRINTF( "Got AUI_MP_RESOLUTION_NOT_SUPPORT, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_FRAMERATE_NOT_SUPPORT:
            AUI_PRINTF( "Got AUI_MP_FRAMERATE_NOT_SUPPORT, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_NO_MEMORY:
            AUI_PRINTF( "Got AUI_MP_NO_MEMORY, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_ERROR_UNKNOWN:
            AUI_PRINTF( "Got AUI_MP_ERROR_UNKNOWN, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_AUDIO_CODEC_NOT_SUPPORT:
            AUI_PRINTF( "Got AUI_MP_AUDIO_CODEC_NOT_SUPPORT, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        case AUI_MP_DECODE_ERROR:
            AUI_PRINTF( "Got AUI_MP_DECODE_ERROR, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        default:
            AUI_PRINTF( "Got unkown msg\n" );
            break;
    }
}

/****************************GLOBAL FUNC DECLEAR*****************************/
unsigned long test_mp_play_task( void *param1, void *param2 )
{
    unsigned long ret;
    
    unsigned long received_bytes = 0;
    //the message sent to the message queue
    aui_mp_cmd_message mp_cmd_msg;
    
    while ( 1 ) {
        aui_os_task_sleep( 10 );
        //receive the message, and check the message. If the message is excepted, call aui_mp_stop() and aui_mp_start()to play
        //the video again
        mp_cmd_msg = 0;
        ret = aui_os_msgq_rcv( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), &received_bytes, 0 );
        if ( AUI_RTN_SUCCESS != ret ) {
            aui_os_task_sleep( 10 );
        }
        
        switch ( mp_cmd_msg ) {
            case AUI_MP_OPEN:
                if ( NULL != g_mp_hdl ) {
                    AUI_PRINTF( "close the opened video file!\n" );
                    aui_mp_close( NULL, &g_mp_hdl );
                    g_mp_hdl = NULL;
                }
                ret = aui_mp_open( &( g_mp_task_info.attr_mp ), &g_mp_hdl );
                if ( AUI_RTN_SUCCESS != ret ) {
                    return AUI_RTN_FAIL;
                }
                // TDS MP default set preview, so here set fullview before start
                AUI_TEST_CHECK_RTN( test_mp_set_full_view() );
                break;
                
            case AUI_MP_CLOSE:
                aui_mp_set( g_mp_hdl, AUI_MP_SET_FD_CB_UNREG, NULL );
                AUI_TEST_CHECK_RTN( aui_mp_close( NULL, &g_mp_hdl ) );
                g_mp_hdl = NULL;
                break;
            case AUI_MP_START:
                AUI_PRINTF( "get message for mp rewind to start\n" );
                if ( NULL != g_mp_hdl ) {
                    AUI_TEST_CHECK_RTN( aui_mp_stop( g_mp_hdl ) );
                }
                AUI_TEST_CHECK_RTN( aui_mp_start( g_mp_hdl ) );
                break;
            case AUI_MP_STOP:
                AUI_TEST_CHECK_RTN( aui_mp_stop( g_mp_hdl ) );
                break;
            case AUI_MP_PAUSE:
                AUI_TEST_CHECK_RTN( aui_mp_resume( g_mp_hdl ) );
                break;
            case AUI_MP_RESUME:
                AUI_TEST_CHECK_RTN( aui_mp_resume( g_mp_hdl ) );
                break;
            case AUI_MP_SEEK:
                AUI_TEST_CHECK_RTN( aui_mp_seek( g_mp_hdl, ( g_mp_task_info.seek_time ) * 1000 ) );
                break;
            case AUI_MP_SPEED:
                AUI_TEST_CHECK_RTN( aui_mp_speed_set( g_mp_hdl, g_mp_task_info.speed ) );
                break;
            case AUI_MP_GET_INFO:
                AUI_TEST_CHECK_RTN( aui_mp_get( g_mp_hdl, AUI_MP_GET_STREAM_INFO, &( g_mp_task_info.p_mp_info ) ) );
                AUI_PRINTF( "the size of the video file is %lld byte\n", g_mp_task_info.p_mp_info.file_size );
                AUI_PRINTF( "the size of the video total_time is %d s\n", g_mp_task_info.p_mp_info.total_time );
                break;
            case AUI_MP_GET_TOTAL_TIME:
                AUI_TEST_CHECK_RTN( aui_mp_total_time_get( g_mp_hdl, &( g_mp_task_info.total_time ) ) );
                AUI_PRINTF( "\n_total time: %d\n", g_mp_task_info.total_time );
                break;
            case AUI_MP_GET_CURRENT_TIME:
                AUI_TEST_CHECK_RTN( aui_mp_cur_time_get( g_mp_hdl, &( g_mp_task_info.current_time ) ) );
                AUI_PRINTF( "\n_current time: %d\n", g_mp_task_info.current_time );
                break;
            default:
                break;
        }
        
    }
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_open( unsigned long *argc, char **argv, char *sz_out_put )
{
    aui_attr_task attr_task;
    unsigned long ret;
    aui_attr_msgq msg_info;
    aui_mp_cmd_message mp_cmd_msg;
    
    
    if ( *argc < 1 ) {
        AUI_PRINTF( "Example: open /mnt/uda1/Sevtrek3.mpg" );
        return AUI_RTN_EINVAL;
    }
    
    //
    waiting_usb_fs_init();
    
    
    aui_f_hdl *fp_w = NULL;
    //fp_w = fopen("/mnt/uda1/my.mkv","rb");
    fp_w = ( aui_f_hdl * ) aui_fs_open( argv[0], "r" );
    if ( NULL == fp_w ) {
        AUI_PRINTF( "open file mp file fail\n" );
        return AUI_RTN_EINVAL;
    }
    AUI_PRINTF( "fp_w is 0x%x\n", fp_w );
    aui_fs_close( fp_w );
    
    //Create the task to check the received message,
    //and realize the all mp play function.
    MEMSET( &attr_task, 0, sizeof( aui_attr_task ) );
    strcpy( attr_task.sz_name, "mp play" );
    attr_task.ul_priority = OSAL_PRI_NORMAL;
    attr_task.p_fun_task_entry = test_mp_play_task;
    attr_task.ul_quantum = 10;
    attr_task.ul_stack_size = 0x10000;
    //If the mp task has already been created, do not create another one
    if ( NULL == g_mp_task_info.task_hdl ) {
        AUI_TEST_CHECK_RTN( aui_os_task_create( &attr_task, &( g_mp_task_info.task_hdl ) ) );
        AUI_PRINTF( "[%s %d], task created!\n", __FUNCTION__, __LINE__ );
    }
    
    //STRCPY(attr_mp.uc_file_name, "/mnt/uda1/Sevtrek3.mpg");
    STRCPY( g_mp_task_info.attr_mp.uc_file_name, argv[0] );
    g_mp_task_info.attr_mp.start_time = atoi( argv[1] );
    AUI_PRINTF( "open file %s\n", g_mp_task_info.attr_mp.uc_file_name );
    g_mp_task_info.attr_mp.aui_mp_stream_cb = test_aui_mp_message_callback;
    g_mp_task_info.attr_mp.user_data = 0;
    
    //create the message queue
    memset( &msg_info, 0, sizeof( aui_attr_msgq ) );
    strcpy( msg_info.sz_name, "testQue" );
    msg_info.ul_buf_size = 10 * sizeof( unsigned int );
    msg_info.ul_msg_size_max = sizeof( unsigned int );
    //If the message queue has already been created, do not create another one
    if ( NULL == g_mp_task_info.msg_hdl ) {
        ret = aui_os_msgq_create( &msg_info, &( g_mp_task_info.msg_hdl ) );
        AUI_PRINTF( "create the message queue!\n" );
        if ( AUI_RTN_SUCCESS != ret ) {
            AUI_PRINTF( "create message queue fail\n" );
            return AUI_RTN_FAIL;
        }
    }
    
    //send the open message
    mp_cmd_msg = AUI_MP_OPEN;
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

#if __SHOW_AV__
unsigned long test_mp_show_av( unsigned long *argc, char **argv, char *sz_out_put )
{

    aui_attr_mp attr_mp;
    UINT8 str_cmd[64] = {0};
    
    //add mp init
    //aui_win_media_player_init(commom_mp_callback_wrapper);
    
    MEMSET( &attr_mp , 0, sizeof( aui_attr_mp ) );
    //STRCPY(attr_mp.uc_file_name, "/mnt/uda1/test1.mkv");
    
    sprintf( str_cmd, "%s%d:%x:%x:%s", "/flshchk/", 0, 0x02FD0400, 0x189b78, ".ts" );
    STRCPY( attr_mp.uc_file_name, str_cmd );
    
    AUI_TEST_CHECK_RTN( aui_mp_open( &attr_mp, &g_mp_hdl ) );
    aui_pe_cache_cb_fp aui_pe_cache_cb;
    aui_pe_cache_cb.file_open  = aui_mp_fd_open;
    aui_pe_cache_cb.file_read  = aui_mp_fd_read;
    aui_pe_cache_cb.file_close = aui_mp_fd_close;
    aui_pe_cache_cb.file_seek  = aui_mp_fd_seek;
    aui_pe_cache_cb.file_tell  = aui_mp_fd_tell;
    aui_pe_cache_cb.file_eof   = aui_mp_fd_eof;
    
    aui_mp_set( g_mp_hdl, AUI_MP_SET_FD_CB_REG, &aui_pe_cache_cb );
    
    return AUI_RTN_SUCCESS;
    return AUI_RTN_SUCCESS;
    
}
#endif

unsigned long test_mp_close( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_CLOSE;
    //send the message to close the mp handle
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}



unsigned long test_mp_start( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_START;
    //send the message to start play
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_stop( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_STOP;
    //send the message to stop play
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_pause( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_PAUSE;
    //send the message to pause
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_resume( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_RESUME;
    //send the message to resume
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_seek( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    AUI_TEST_CHECK_NULL( argc );
    AUI_TEST_CHECK_NULL( argv );
    AUI_TEST_CHECK_NULL( sz_out_put );
    AUI_TEST_CHECK_VAL( *argc, 1 );
    g_mp_task_info.seek_time = get_num_by_string( argv[0] );
    mp_cmd_msg = AUI_MP_SEEK;
    //send the message to seek a time to play
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_speed( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    AUI_TEST_CHECK_NULL( argc );
    AUI_TEST_CHECK_NULL( argv );
    AUI_TEST_CHECK_NULL( sz_out_put );
    AUI_TEST_CHECK_VAL( *argc, 1 );
    g_mp_task_info.speed = atoi( argv[0] );
    mp_cmd_msg = AUI_MP_SPEED;
    //send the message to set the play speed
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_get_info( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    //aui_mp_video_info p_mp_info;
    //aui_handle_mp p_mp_speed;
    // unsigned long ul_item = atoi(argv[0]);
    //switch (ul_item){
    //    case 0:
    mp_cmd_msg = AUI_MP_GET_INFO;
    //send the message to get the info of video file
    
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    // break;
    // case 1:
    //aui_mp_get(g_mp_hdl, AUI_MP_GET_STREAM_INFO_SPEED, &p_mp_speed);
    //   break;
    //default:
    // break;
    //}
    return AUI_RTN_SUCCESS;
    
}

unsigned long test_mp_get_total_time( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_GET_TOTAL_TIME;
    //send the message to get the total time
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}



unsigned long test_mp_get_current_time( unsigned long *argc, char **argv, char *sz_out_put )
{
    unsigned long ret;
    aui_mp_cmd_message mp_cmd_msg;
    mp_cmd_msg = AUI_MP_GET_CURRENT_TIME;
    //send the message to get the current time
    ret = aui_os_msgq_snd( g_mp_task_info.msg_hdl, &mp_cmd_msg, sizeof( unsigned int ), 0 );
    if ( AUI_RTN_SUCCESS != ret ) {
        AUI_PRINTF( "send message fail\n" );
        return AUI_RTN_FAIL;
    }
    
    return AUI_RTN_SUCCESS;
}

unsigned long test_mp_help( unsigned long *argc, char **argv, char *sz_out_put )
{
    ( void )argc;
    ( void )argv;
    ( void )sz_out_put;
    
    char help[] =
        "\n\nMedia Player Help \n\
COMMAND \n\
        'open': \n\
                open the media file. \n\
                Example:        open /mnt/uda1/Sevtrek3.mpg \n\
                                Open Sevtrek3.mpg in the flash drive. \n\
\n\n";

    AUI_PRINTF( "%s", help );
    return AUI_RTN_HELP;
}
#endif
#ifdef AUI_LINUX
extern FILE* stream;
extern unsigned long aui_get_user_cmd_string(char *sz_input,char *sz_cmd,unsigned char *argc,char **argv);
extern st_aui_cmd g_st_aui_cmd_str;
static void mpCallbackFunc( enum aui_mp_message msg, void *data, void *user_data )
{
    AUI_PRINTF( GREEN_STR"callback msg:" );
    switch ( msg ) {
        case AUI_MP_PLAY_BEGIN: {
            AUI_PRINTF( "AUI_MP_PLAY_BEGIN\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_PLAY_END: {
            AUI_PRINTF( "AUI_MP_PLAY_END\n"DEFAULT_STR );
            *((int*)user_data) = TRUE; // user_data is play end flag
            break;
        }
        case AUI_MP_VIDEO_CODEC_NOT_SUPPORT: {
            AUI_PRINTF( "AUI_MP_VIDEO_CODEC_NOT_SUPPORT\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_AUDIO_CODEC_NOT_SUPPORT: {
            AUI_PRINTF( "AUI_MP_AUDIO_CODEC_NOT_SUPPORT\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_RESOLUTION_NOT_SUPPORT: {
            AUI_PRINTF( "AUI_MP_RESOLUTION_NOT_SUPPORT\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_FRAMERATE_NOT_SUPPORT: {
            AUI_PRINTF( "AUI_MP_FRAMERATE_NOT_SUPPORT\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_NO_MEMORY: {
            AUI_PRINTF( "AUI_MP_NO_MEMORY\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_DECODE_ERROR: {
            AUI_PRINTF( "AUI_MP_DECODE_ERROR\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_ERROR_UNKNOWN: {
            AUI_PRINTF( "AUI_MP_ERROR_UNKNOWN\n"
                        "%s\n"DEFAULT_STR, ( char * )data );
            break;
        }
        case AUI_MP_BUFFERING: {
            AUI_PRINTF( "AUI_MP_BUFFERING\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_ERROR_SOUPHTTP: {
            AUI_PRINTF( "AUI_MP_ERROR_SOUPHTTP\n"DEFAULT_STR );
            break;
        }
        case AUI_MP_FRAME_CAPTURE: {
            char path[128], frameInfo[128];
            unsigned int h, w;
            sscanf( ( char * )data, "%[^;];h=%u,w=%u", path, &h, &w );
            AUI_PRINTF( "frame captured path %s\n", path );
            AUI_PRINTF( "frame width=%u, height=%u\n", w, h );
            AUI_PRINTF( "AUI_MP_FRAME_CAPTURE\n"DEFAULT_STR );
            break;
        }
        default:
            AUI_PRINTF( RED_STR"unkown callback message:%d\n"DEFAULT_STR, msg );
            break;
    }
    ( void )user_data;
}

static const char *audDecType2Str( aui_deca_stream_type decType )
{
    switch ( decType ) {
        case AUI_DECA_STREAM_TYPE_MPEG1: {
            return "mpeg1";
        }
        case AUI_DECA_STREAM_TYPE_MPEG2: {
            return "mpeg2";
        }
        case AUI_DECA_STREAM_TYPE_AAC_LATM: {
            return "aac latm";
        }
        case AUI_DECA_STREAM_TYPE_AC3: {
            return "ac3";
        }
        case AUI_DECA_STREAM_TYPE_DTS: {
            return "dts";
        }
        case AUI_DECA_STREAM_TYPE_PPCM: {
            return "ppcm";
        }
        case AUI_DECA_STREAM_TYPE_LPCM_V: {
            return "lpcm DVD video";
        }
        case AUI_DECA_STREAM_TYPE_LPCM_A: {
            return "lpcm DVD audio";
        }
        case AUI_DECA_STREAM_TYPE_PCM: {
            return "pcm";
        }
        case AUI_DECA_STREAM_TYPE_BYE1: {
            return "bye1";
        }
        case AUI_DECA_STREAM_TYPE_RA8: {
            return "ra8";
        }
        case AUI_DECA_STREAM_TYPE_MP3: {
            return "mp3";
        }
        case AUI_DECA_STREAM_TYPE_AAC_ADTS: {
            return "aac adts";
        }
        case AUI_DECA_STREAM_TYPE_OGG: {
            return "ogg";
        }
        case AUI_DECA_STREAM_TYPE_EC3: {
            return "ec3";
        }
        case AUI_DECA_STREAM_TYPE_MP3_L3: {
            return "mp3 L3";
        }
        case AUI_DECA_STREAM_TYPE_RAW_PCM: {
            return "raw pcm";
        }
        case AUI_DECA_STREAM_TYPE_BYE1PRO: {
            return "bye1 pro";
        }
        case AUI_DECA_STREAM_TYPE_FLAC: {
            return "flac";
        }
        case AUI_DECA_STREAM_TYPE_APE: {
            return "ape";
        }
        case AUI_DECA_STREAM_TYPE_MP3_2: {
            return "mp3 2";
        }
        case AUI_DECA_STREAM_TYPE_AMR: {
            return "amr";
        }
        case AUI_DECA_STREAM_TYPE_ADPCM: {
            return "adpcm";
        }
        default: {
            AUI_PRINTF( RED_STR"unkown aui_deca_stream_type:%d\n"DEFAULT_STR, decType );
            return "unknow";
        }
        
    }
}

static const char *vidformat2Str( aui_decv_format vidfmt )
{
    switch ( vidfmt ) {
        case AUI_DECV_FORMAT_MPEG: {
            return "MPEG";
        }
        case AUI_DECV_FORMAT_AVC: {
            return "AVC";
        }
        case AUI_DECV_FORMAT_AVS: {
            return "AVS";
        }
        case AUI_DECV_FORMAT_XVID: {
            return "XVID";
        }
        case AUI_DECV_FORMAT_FLV1: {
            return "FLV1";
        }
        case AUI_DECV_FORMAT_VP8: {
            return "VP8";
        }
        case AUI_DECV_FORMAT_WVC1: {
            return "WVC1";
        }
        case AUI_DECV_FORMAT_WX3: {
            return "WX3";
        }
        case AUI_DECV_FORMAT_RMVB: {
            return "RMVB";
        }
        case AUI_DECV_FORMAT_MJPG: {
            return "MJPG";
        }
        case AUI_DECV_FORMAT_INVALID: {
            return "INVALID";
        }
        default: {
            //imposible to be here
            return "fatal error";
        }
    }
}
static void showStreamInfo( aui_mp_stream_info_type infoType, aui_mp_stream_info *info )
{
    unsigned int i;
    switch ( infoType ) {
        case AUI_MP_STREAM_INFO_TYPE_MEDIA_SIZE:
            AUI_PRINTF( GREEN_STR"media size:%lld\n"DEFAULT_STR, info->stream_info.mediaSize );
            break;
        case AUI_MP_STREAM_INFO_TYPE_VIDEO: {
            aui_mp_video_track_info *video_info = info->stream_info.video_track_info;
            AUI_PRINTF( GREEN_STR "video info count:%d\n", info->count );
            for ( i = 0; i < info->count; i++ ) {
                AUI_PRINTF( "video format:%s\n", vidformat2Str( video_info[i].vidCodecFmt ) );
                AUI_PRINTF( "width:%u\n", video_info[i].width );
                AUI_PRINTF( "height:%u\n", video_info[i].height );
                AUI_PRINTF( "frame rate:%u\n", video_info[i].framerate );
            }
            AUI_PRINTF( DEFAULT_STR );
            
            break;
        }
        case AUI_MP_STREAM_INFO_TYPE_AUDIO: {
            aui_mp_audio_track_info *audio_info = info->stream_info.audio_track_info;
            AUI_PRINTF( GREEN_STR"audio info count:%d\n", info->count );
            for ( i = 0; i < info->count; i++ ) {
                AUI_PRINTF( "*** track index:%u ***\n", audio_info[i].track_index );
                AUI_PRINTF( "language code:%s\n", audio_info[i].lang_code );
                if ( audio_info[i].audDetailInfo ) {
                    AUI_PRINTF( "channel:%u\n", audio_info[i].audDetailInfo->channels );
                    AUI_PRINTF( "depth:%u\n", audio_info[i].audDetailInfo->depth );
                    AUI_PRINTF( "sample rate:%u\n", audio_info[i].audDetailInfo->samplerate );
                    AUI_PRINTF( "audio codec:%s\n", audDecType2Str( audio_info[i].audDetailInfo->audioCodecType ) );
                }
            }
            AUI_PRINTF( DEFAULT_STR );
            break;
        }
        case AUI_MP_STREAM_INFO_TYPE_SUBTITLE: {
            aui_mp_subtitle_info *subtitle_info = info->stream_info.subtitle_info;
            AUI_PRINTF( GREEN_STR"subtitle info count:%d\n", info->count );
            for ( i = 0; i < info->count; i++ ) {
                AUI_PRINTF( "*** track index:%u ***\n", subtitle_info[i].track_index );
                AUI_PRINTF( "language code:%s\n", subtitle_info[i].lang_code );
            }
            AUI_PRINTF( DEFAULT_STR );
            break;
        }
        
    }
}
const char *playRate2String( enum aui_mp_speed play_rate )
{
    switch ( play_rate ) {
        case AUI_MP_SPEED_FASTREWIND_24:
            return "AUI_MP_SPEED_FASTREWIND_24";
        case AUI_MP_SPEED_FASTREWIND_16:
            return "AUI_MP_SPEED_FASTREWIND_16";
        case AUI_MP_SPEED_FASTREWIND_8:
            return "AUI_MP_SPEED_FASTREWIND_8";
        case AUI_MP_SPEED_FASTREWIND_4:
            return "AUI_MP_SPEED_FASTREWIND_4";
        case AUI_MP_SPEED_FASTREWIND_2:
            return "AUI_MP_SPEED_FASTREWIND_2";
        case AUI_MP_SPEED_SLOWREWIND_2:
            return "AUI_MP_SPEED_SLOWREWIND_2";
        case AUI_MP_SPEED_SLOWREWIND_4:
            return "AUI_MP_SPEED_SLOWREWIND_4";
        case AUI_MP_SPEED_SLOWREWIND_8:
            return "AUI_MP_SPEED_SLOWREWIND_8";
        case AUI_MP_SPEED_SLOWFORWARD_2:
            return "AUI_MP_SPEED_SLOWFORWARD_2";
        case AUI_MP_SPEED_SLOWFORWARD_4:
            return "AUI_MP_SPEED_SLOWFORWARD_4";
        case AUI_MP_SPEED_SLOWFORWARD_8:
            return "AUI_MP_SPEED_SLOWFORWARD_8";
        case AUI_MP_SPEED_FASTFORWARD_2:
            return "AUI_MP_SPEED_FASTFORWARD_2";
        case AUI_MP_SPEED_FASTFORWARD_4:
            return "AUI_MP_SPEED_FASTFORWARD_4";
        case AUI_MP_SPEED_FASTFORWARD_8:
            return "AUI_MP_SPEED_FASTFORWARD_8";
        case AUI_MP_SPEED_FASTFORWARD_16:
            return "AUI_MP_SPEED_FASTFORWARD_16";
        case AUI_MP_SPEED_FASTFORWARD_24:
            return "AUI_MP_SPEED_FASTFORWARD_24";
        case AUI_MP_SPEED_1:
            return "AUI_MP_SPEED_1";
        case AUI_MP_SPEED_0:
            return "AUI_MP_SPEED_0";
    }
    return NULL;
}
#define NO_INPUT -2
int scan_timeout()
{

    fd_set input_set;
    struct timeval  timeout;
    int ready_for_reading = 0;
    int read_bytes = 0;
    int rtn = NO_INPUT;
    char buf[3]={0};

    /* Empty the FD Set */
    FD_ZERO(&input_set );
    /* Listen to the input descriptor */
    FD_SET(0, &input_set);

    /* Waiting for some seconds */
    timeout.tv_sec = 2;    // WAIT seconds
    timeout.tv_usec = 0;    // 0 milliseconds    
    
    /* Listening for input stream for any activity */
    ready_for_reading = select(1, &input_set, NULL, NULL, &timeout);    
    if(ready_for_reading)
    {
        read_bytes = read(0, buf, 3);
        if(read_bytes)
        {
            buf[read_bytes] = '\0';
            rtn = atoi(buf);
        }
       
        //printf("rtn=%d, read_bytes:%d\n", rtn, read_bytes);
    }
    return rtn;
}
unsigned long test_mp( unsigned long *argc, char **argv, char *sz_out_put )
{
#define NUM_PLAY     1
#define NUM_PAUSE   (NUM_PLAY+1)
#define NUM_RESUME (NUM_PAUSE+1)
#define NUM_STOP       (NUM_RESUME+1)
#define NUM_SEEK      (NUM_STOP+1)
#define NUM_SPEED_SET (NUM_SEEK+1)
#define NUM_GET_TOTAL_TIME (NUM_SPEED_SET+1)
#define NUM_CLOSE (NUM_GET_TOTAL_TIME+1)
#define NUM_CURRENT_TIME_GET (NUM_CLOSE+1)
#define NUM_GET_ALL_STREAM_INFO (NUM_CURRENT_TIME_GET+1)
#define NUM_GET_CUR_STREAM_INFO (NUM_GET_ALL_STREAM_INFO+1)
#define NUM_QUIT_PROCESS (NUM_GET_CUR_STREAM_INFO+1)
#define NUM_GET_SNAPSHOT (NUM_QUIT_PROCESS+1)
#define NUM_CHANGE_AUD (NUM_GET_SNAPSHOT+1)
#define NUM_CHANGE_SUB (NUM_CHANGE_AUD+1)
#define NUM_GET_DOWNLOAD_SPEED (NUM_CHANGE_SUB+1)
    //#define NUM_SET_VOL (NUM_GET_SNAPSHOT+1)
    //#define NUM_GET_VOL (NUM_SET_VOL+1)
    int cmd = 0;
    int tmpVal;
    unsigned int total_time = 0;
    unsigned int cur_time = 0;
    aui_attr_mp *pst_mp_attr = NULL;
    void *pp_handle_mp = NULL;
    void *pv_param = NULL;
    char name[1024];
	char get_buf[1024];
    int len = 0;
	int ul_rtn = 0;
    int f_isPlayEnd = FALSE;
    aui_mp_stream_info *info;
    
    AUI_PRINTF( "pls enter the file you want to play:\n" );
    AUI_PRINTF( "Example: file:///mnt/usb/sda1/Sevtrek3.mpg\n"
                "\tOr, http://sever.name/path/test.mp4\n" );
 
	if (NULL != stream) {
		if(NULL == fgets(name, sizeof(name), stream)) {//can't get any words
			if (stream)
				fclose(stream);//handle end of file
			stream = NULL;
		}
	} else if (NULL == fgets( name, sizeof( name ) - 1, stdin )) {
        AUI_PRINTF( "change audio stream failed!!\n" );
    }
    len = strlen( name );
    if ( len > 0 && '\n' == name[len - 1] ) {
        name[len - 1] = 0;
    }
    AUI_PRINTF( "URL:%s\n", name );
    
    pst_mp_attr = ( aui_attr_mp * )malloc( sizeof( aui_attr_mp ) );
    memset( pst_mp_attr, 0, sizeof( aui_attr_mp ) );
    strcpy( pst_mp_attr->uc_file_name, name );
    pst_mp_attr->aui_mp_stream_cb = mpCallbackFunc;
    pst_mp_attr->user_data = (void*)&f_isPlayEnd;
    aui_mp_open( pst_mp_attr, &pp_handle_mp );
    if ( aui_mp_set_buffering_time( pp_handle_mp, 10, 5, 30 ) != AUI_RTN_SUCCESS ) {
        AUI_PRINTF( RED_STR "aui_mp_set_buffering_time() failed!!\n"DEFAULT_STR );
    }
    if ( aui_mp_set_start2play_percent( pp_handle_mp, 3 ) != AUI_RTN_SUCCESS ) {
        AUI_PRINTF( RED_STR "aui_mp_set_start2play_percent() failed!!\n"DEFAULT_STR );
    }

mp_loop:
    cmd = 0;
    setbuf( stdin, NULL );
	if (NULL != stream) {//run auto test mode
	   ul_rtn = aui_get_user_cmd_string(get_buf,
			   g_st_aui_cmd_str.ac_cmdID,
			   (unsigned char *)&(g_st_aui_cmd_str.ui_cmd_param_cnt),
			   g_st_aui_cmd_str.argv);
	   if (0 == ul_rtn) {
		   if (0 == strcmp("wait", g_st_aui_cmd_str.ac_cmdID)) {
			   sleep(atoi(g_st_aui_cmd_str.argv[0]));
		   } else if (0 == strcmp("select", g_st_aui_cmd_str.ac_cmdID)){
			   cmd = atoi(g_st_aui_cmd_str.argv[0]);
		   }
	   }
	} else { //run manual mode
		AUI_PRINTF( "pls input your cmd:\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n"
				"%d --- %-30s%d --- %s\n",
				NUM_PLAY, "play", NUM_PAUSE, "pause",
				NUM_RESUME, "resume", NUM_STOP, "stop",
				NUM_SEEK, "seek", NUM_SPEED_SET, "speed set",
				NUM_GET_TOTAL_TIME, "total time get", NUM_CLOSE, "close",
				NUM_CURRENT_TIME_GET, "current time get", NUM_GET_ALL_STREAM_INFO, "get all stream information",
				NUM_GET_CUR_STREAM_INFO, "get current stream information", NUM_QUIT_PROCESS, "quit process",
				NUM_GET_SNAPSHOT, "get snapshot", NUM_CHANGE_AUD, "change audio",
				NUM_CHANGE_SUB, "change subtitle", NUM_GET_DOWNLOAD_SPEED, "stream download speed" );
                while((cmd = scan_timeout()) == NO_INPUT)
                {
                    if(f_isPlayEnd)
                    {
                        aui_mp_stop(pp_handle_mp);
                        aui_mp_start(pp_handle_mp);
                        f_isPlayEnd = FALSE;
		    }
                }
    }

    switch ( cmd ) {
        case NUM_PLAY:
            //aui_mp_open(pst_mp_attr, &pp_handle_mp);
            aui_mp_start( pp_handle_mp );
            break;
        case NUM_PAUSE:
            aui_mp_pause( pp_handle_mp );
            break;
        case NUM_RESUME:
            aui_mp_resume( pp_handle_mp );
            break;
        case NUM_STOP:
            aui_mp_stop( pp_handle_mp );
            break;
        case NUM_SEEK:
            do {
                aui_mp_is_seekable( pp_handle_mp, &tmpVal );
                if ( tmpVal != -1 ) {
                    break;
                }
                usleep( 50000 );
            } while ( 1 );
            if ( tmpVal ) {
                AUI_PRINTF( "pls input seek time(ms):" );
                if ( 0 == scanf( "%d", &tmpVal ) ) {
                    printf( "scanf failed\n" );
                }
                
                aui_mp_seek( pp_handle_mp, tmpVal );
            } else {
                AUI_PRINTF( "the stream isn't seekable!!\n" );
            }
            break;
        case NUM_SPEED_SET: {
            enum aui_en_play_speed play_rate;
trick_mode:
            AUI_PRINTF( "pls input speed:\n" );
            AUI_PRINTF( "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n"
                        "%-30s%-30s\n",
                        "1 -> 24 time fast rewind", "2 -> 16 time fast rewind",
                        "3 -> 8 time fast rewind", "4 -> 4 time fast rewind",
                        "5 -> 2 time fast rewind", "6 -> 2 time slow rewind",
                        "7 -> 4 time slow rewind", "8 -> 8 time slow rewind",
                        "9 -> 2 time slow forward", "10 -> 4 time slow forward",
                        "11 -> 8 time slow forward", "12 -> 2 time fast forward",
                        "13 -> 4 time fast forward", "14 -> 8 time fast forward",
                        "15 -> 16 time fast forward", "16 -> 24 time fast forward",
                        "17 -> normal play", "18 -> pause" );
            if ( 0 == scanf( "%d", &tmpVal ) ) {
                printf( "scanf failed\n" );
            }
            
            switch ( tmpVal ) {
                case 1:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTREWIND_24 );
                    break;
                case 2:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTREWIND_16 );
                    break;
                case 3:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTREWIND_8 );
                    break;
                case 4:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTREWIND_4 );
                    break;
                case 5:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTREWIND_2 );
                    break;
                case 6:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_SLOWREWIND_2 );
                    break;
                case 7:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_SLOWREWIND_4 );
                    break;
                case 8:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_SLOWREWIND_8 );
                    break;
                case 9:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_SLOWFORWARD_2 );
                    break;
                case 10:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_SLOWFORWARD_4 );
                    break;
                case 11:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_SLOWFORWARD_8 );
                    break;
                case 12:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTFORWARD_2 );
                    break;
                case 13:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTFORWARD_4 );
                    break;
                case 14:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTFORWARD_8 );
                    break;
                case 15:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTFORWARD_16 );
                    break;
                case 16:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_FASTFORWARD_24 );
                    break;
                case 17:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_1 );
                    goto mp_loop;
                    break;
                case 18:
                    aui_mp_speed_set( pp_handle_mp, AUI_MP_SPEED_0 );
                    break;
                default:
                    setbuf( stdin, NULL );
                    goto mp_loop;
                    break;
            }
            if ( aui_mp_get_playrate( pp_handle_mp, &play_rate ) == AUI_RTN_SUCCESS ) {
                AUI_PRINTF( "play rate:%s\n", playRate2String( play_rate ) );
            } else {
                AUI_PRINTF( "aui_mp_get_playrate() failed!!\n" );
            }
            goto trick_mode;
        }
        break;
        case NUM_GET_TOTAL_TIME:
            aui_mp_total_time_get( pp_handle_mp, &total_time );
            AUI_PRINTF( "total time is %d s.\n", total_time );
            break;
        case NUM_CLOSE:
            //          aui_mp_close(0, 0);
            aui_mp_close( pst_mp_attr, &pp_handle_mp );
            break;
        case NUM_CURRENT_TIME_GET:
            aui_mp_cur_time_get( pp_handle_mp, &cur_time );
            AUI_PRINTF( "cur time is %d\n", cur_time );
            break;
        case NUM_GET_ALL_STREAM_INFO:
            if ( aui_mp_get_stream_info( pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_MEDIA_SIZE, &info ) == AUI_RTN_SUCCESS ) {
                showStreamInfo( AUI_MP_STREAM_INFO_TYPE_MEDIA_SIZE, info );
                aui_mp_free_stream_info( pp_handle_mp, info );
            } else {
                AUI_PRINTF( RED_STR"aui_mp_get_stream_info() failed for media size!!\n"DEFAULT_STR); }
                if(aui_mp_get_stream_info(pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_VIDEO, &info) == AUI_RTN_SUCCESS)
                {
                    showStreamInfo(AUI_MP_STREAM_INFO_TYPE_VIDEO, info);
                    aui_mp_free_stream_info(pp_handle_mp, info);
                }
                else
                    AUI_PRINTF(RED_STR"aui_mp_get_stream_info() failed for video!!\n"DEFAULT_STR);    
                if(aui_mp_get_stream_info(pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_AUDIO, &info) == AUI_RTN_SUCCESS)
                {
                    showStreamInfo(AUI_MP_STREAM_INFO_TYPE_AUDIO, info);
                    aui_mp_free_stream_info(pp_handle_mp, info);
                }
                else
                    AUI_PRINTF(RED_STR"aui_mp_get_stream_info() failed for audio!!\n"DEFAULT_STR);
                if(aui_mp_get_stream_info(pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_SUBTITLE, &info) == AUI_RTN_SUCCESS)
                {
                    showStreamInfo(AUI_MP_STREAM_INFO_TYPE_SUBTITLE, info);
                    aui_mp_free_stream_info(pp_handle_mp, info);
                }
                else
                    AUI_PRINTF(RED_STR"aui_mp_get_stream_info() failed for subtitle!!\n"DEFAULT_STR);                    
			break;
             case NUM_GET_CUR_STREAM_INFO:
                    if(aui_mp_get_cur_stream_info(pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_VIDEO, &info) == AUI_RTN_SUCCESS)
                    {
                        showStreamInfo(AUI_MP_STREAM_INFO_TYPE_VIDEO, info);
                        aui_mp_free_stream_info(pp_handle_mp, info);
                    }
                    else
                        AUI_PRINTF(RED_STR"aui_mp_get_cur_stream_info() failed for video!!\n"DEFAULT_STR); 
                    if(aui_mp_get_cur_stream_info(pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_AUDIO, &info) == AUI_RTN_SUCCESS)
                    {
                        showStreamInfo(AUI_MP_STREAM_INFO_TYPE_AUDIO, info);
                        aui_mp_free_stream_info(pp_handle_mp, info);
                    }
                    else
                        AUI_PRINTF(RED_STR"aui_mp_get_cur_stream_info() failed for audio!!\n"DEFAULT_STR); 
                    if(aui_mp_get_cur_stream_info(pp_handle_mp, AUI_MP_STREAM_INFO_TYPE_SUBTITLE, &info) == AUI_RTN_SUCCESS)
                    {
                        showStreamInfo(AUI_MP_STREAM_INFO_TYPE_SUBTITLE, info);
                        aui_mp_free_stream_info(pp_handle_mp, info);
                    }
                    else
                        AUI_PRINTF(RED_STR"aui_mp_get_cur_stream_info() failed for subtitle!!\n"DEFAULT_STR); 
                    break;
		case NUM_QUIT_PROCESS:
			aui_mp_close(pst_mp_attr, &pp_handle_mp);
			goto quit;
		case NUM_GET_SNAPSHOT:
			aui_mp_get_snapshot(pp_handle_mp);
			break;
              case NUM_CHANGE_AUD:
                    AUI_PRINTF("pls input audio track index:");
					if( 0 == scanf("%d", &tmpVal)) {
						printf("scanf failed\n");
					}

                    if(aui_mp_change_audio(pp_handle_mp, tmpVal) != AUI_RTN_SUCCESS)
                    {
                        AUI_PRINTF("change audio stream failed!!\n");
                    }
                    break;
              case NUM_CHANGE_SUB:
                    AUI_PRINTF("pls input subtitle track index:");
					if( 0 == scanf("%d", &tmpVal)) {
						printf("scanf failed\n");
					}

                    if(aui_mp_change_subtitle(pp_handle_mp, tmpVal) != AUI_RTN_SUCCESS)
                    {
                        AUI_PRINTF("change subtitle stream failed!!\n");
                    }
                    break;                    
              case NUM_GET_DOWNLOAD_SPEED:
          {
                unsigned long long dlSpeed;
                if(aui_mp_get_download_speed(pp_handle_mp, &dlSpeed) == AUI_RTN_SUCCESS)
                {
                    AUI_PRINTF("stream download speed:%llu bps\n", dlSpeed);
                }
                break;
          }
		default:
			setbuf(stdin, NULL);
			break;
	}
	goto mp_loop;

quit:
	free(pst_mp_attr);
	pst_mp_attr = NULL;
	free(pv_param);
	pv_param = NULL;
	return 0;
}
#endif

void mp_test_reg()
{
	#ifdef AUI_LINUX
	aui_tu_reg_group("mp", "mp tests");
	aui_tu_reg_item(2, "test_all_mp", AUI_CMD_TYPE_API, test_mp, "linux media player");
	#endif

	#ifdef AUI_TDS
	aui_tu_reg_group("mp", "mp tests");
	aui_tu_reg_item(2, "open", AUI_CMD_TYPE_API, test_mp_open, "media player open");
	aui_tu_reg_item(2, "close", AUI_CMD_TYPE_API, test_mp_close, "media player close");
	aui_tu_reg_item(2, "start", AUI_CMD_TYPE_API, test_mp_start, "media player start");
	aui_tu_reg_item(2, "stop", AUI_CMD_TYPE_API, test_mp_stop, "media player stop");
	aui_tu_reg_item(2, "pause", AUI_CMD_TYPE_API, test_mp_pause, "media pause");
	aui_tu_reg_item(2, "resume", AUI_CMD_TYPE_API, test_mp_resume, "media player resume");
	aui_tu_reg_item(2, "seek", AUI_CMD_TYPE_API, test_mp_seek, "media player seek");
	aui_tu_reg_item(2, "speed", AUI_CMD_TYPE_API, test_mp_speed, "media player speed");
	aui_tu_reg_item(2, "totaltime", AUI_CMD_TYPE_API, test_mp_get_total_time, "media player get total time");
	aui_tu_reg_item(2, "currenttime", AUI_CMD_TYPE_API, test_mp_get_current_time, "media player get current time");
    aui_tu_reg_item(2, "info", AUI_CMD_TYPE_API, test_mp_get_info, "media player get info");
    aui_tu_reg_item(2,"setcallback", AUI_CMD_TYPE_API, test_aui_mp_set_callback, "set start play callback");
#if __SHOW_AV__
	aui_tu_reg_item(2, "playts", AUI_CMD_TYPE_API, test_mp_show_av, "media player show av logo");
#endif
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_mp_help, "Media Player Help");
	#endif
}

#ifdef __cplusplus
}
#endif


