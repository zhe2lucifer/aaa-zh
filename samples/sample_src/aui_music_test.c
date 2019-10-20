/****************************INCLUDE HEAD FILE************************************/
#include "aui_music_test.h"
#include "aui_mp.h"
#include "aui_fs.h"
//#include <hld_dev.h>
#include <api/libmp/pe.h>

/****************************LOCAL MACRO******************************************/


/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/
static aui_hdl music_handle = NULL;
/****************************LOCAL FUNC DECLEAR***********************************/
///////////////////aui music init start/////////////////////////

//static int first_init = 1;




/*********************************mp init start*********************************/



static void waiting_usb_fs_init()
{
    unsigned int fs_dev = 0;
    int actual_num;
    while((aui_fs_get_alldevid(&fs_dev,1,&actual_num) == AUI_RTN_SUCCESS)&& (fs_dev == 0)){
        osal_task_sleep(10);
    }

    AUI_PRINTF("\n %s: %d\n", __FUNCTION__, __LINE__);
}




/*********************************mp init end***********************************/




///////////////////aui music init end/////////////////////////






static unsigned long get_num_by_string(char *str)
{
    unsigned char temp[128] = {0};
    unsigned long val = 0;
    sscanf(str,"%s",temp);
    if(AUI_RTN_SUCCESS != str2ulong(temp,strlen(temp),&val)){
        return -1;
    }
    return val;
}

static void test_aui_music_message_callback(aui_music_message msg, void *pv_data, void *pv_user_data)
{
    unsigned long msg_code = (unsigned long)pv_data;

    switch (msg) {
        case AUI_MUSIC_PLAY_END:
            AUI_PRINTF("Got AUI_MUSIC_PLAY_END, msg code %d, user_data 0x%X\n", msg_code, pv_user_data);
            break;
        default:
            AUI_PRINTF("Got unkown msg\n");
    }
    
}


/****************************EXTERN VAR*******************************************/

/****************************EXTERN FUNC *****************************************/
AUI_RTN_CODE test_aui_music_open(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_attr_music attr_music;
    MEMSET(&attr_music ,0, sizeof(aui_attr_music));

    if (*argc < 1) {
        AUI_PRINTF("Example: open /mnt/uda1/tamasha.mp3");
        //STRCPY(attr_music.uc_file_name, "/mnt/uda1/tamasha.mp3");
        return AUI_RTN_EINVAL;        
    }
   
    waiting_usb_fs_init();

    aui_f_hdl *fp_w = NULL;
    //fp_w = fopen("/mnt/uda1/test.mp3","rb");
    fp_w = (aui_f_hdl *)aui_fs_open(argv[0], "r");
    if (NULL == fp_w) {
     AUI_PRINTF("open file music file fail\n");
     return AUI_RTN_EINVAL;

    }
    AUI_PRINTF("fp_w is 0x%x\n", fp_w);
    aui_fs_close(fp_w);   
    
    STRCPY( attr_music.uc_file_name, argv[0] );
    AUI_PRINTF( "open file %s\n", attr_music.uc_file_name );
    attr_music.aui_music_cb = test_aui_music_message_callback;
    attr_music.user_data = 0;
    
    if ( NULL != music_handle ) {
        aui_music_close( NULL, &music_handle );
        music_handle = NULL;
    }
    AUI_TEST_CHECK_RTN( aui_music_open( &attr_music, &music_handle ) );
    return AUI_RTN_SUCCESS;
    
}


AUI_RTN_CODE test_aui_music_close( unsigned long *argc, char **argv, char *sz_out_put )
{
    AUI_TEST_CHECK_RTN( aui_music_close( NULL, &music_handle ) );
    music_handle = NULL;
    return AUI_RTN_SUCCESS;
    
}

AUI_RTN_CODE test_aui_music_start(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_music_start(music_handle));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_music_stop(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_music_stop(music_handle));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_music_pause(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_music_pause(music_handle));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_music_resume(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_music_resume(music_handle));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_test_music_seek(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_NULL(argc);
    AUI_TEST_CHECK_NULL(argv);
    AUI_TEST_CHECK_NULL(sz_out_put);
    AUI_TEST_CHECK_VAL(*argc,1);

    unsigned long val = get_num_by_string(argv[0]);

    AUI_TEST_CHECK_RTN(aui_music_seek(music_handle, val));
    return AUI_RTN_SUCCESS;

}

AUI_RTN_CODE aui_test_total_time_get(unsigned long *argc,char **argv,char *sz_out_put)
{
    unsigned int val = 0;
    AUI_TEST_CHECK_RTN(aui_music_total_time_get(music_handle, &val));
    AUI_PRINTF("total time :%d \n",val);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_test_cur_time_get(unsigned long *argc,char **argv,char *sz_out_put)
{
    unsigned int val = 0;
    AUI_TEST_CHECK_RTN(aui_music_cur_time_get(music_handle, &val));
    AUI_PRINTF("cur time :%d \n",val);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE test_aui_music_de_init(unsigned long *argc,char **argv,char *sz_out_put)
{
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE test_aui_music_set(unsigned long *argc,char **argv,char *sz_out_put)
{
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_music_get(unsigned long *argc,char **argv,char *sz_out_put)
{
       aui_music_info p_music_info ;
    aui_decoder_info p_decoder_info;
    unsigned long item = atoi( argv[0] );
    char *ptitle = NULL;
    char *partist = NULL;
    char *pcomment = NULL;
    char *palbum = NULL;
    char *pyear = NULL;
    int i = 0;
    switch ( item ) {
        case 0:
            AUI_TEST_CHECK_RTN( aui_music_get( music_handle, AUI_MUSIC_GET_MUSIC_INFO, &p_music_info ) );
            ptitle = p_music_info.title;
            AUI_PRINTF( "the title is :" );
            for ( i = 0; i < sizeof( p_music_info.title ); i++ ) {
                AUI_PRINTF( "0x%02x  ", ( char ) * ( ptitle + i ) );
            }
            AUI_PRINTF( "\n" );
            
            pcomment = p_music_info.comment;
            AUI_PRINTF( "the comment is :" );
            for ( i = 0; i < sizeof( p_music_info.comment ); i++ ) {
                AUI_PRINTF( "0x%02x  ", ( char ) * ( pcomment + i ) );
            }
            AUI_PRINTF( "\n" );
            
            partist = p_music_info.artist;
            AUI_PRINTF( "the artist is :" );
            for ( i = 0; i < sizeof( p_music_info.artist ); i++ ) {
                AUI_PRINTF( "0x%02x  ", ( char ) * ( partist + i ) );
            }
            AUI_PRINTF( "\n" );
            
            palbum = p_music_info.album;
            AUI_PRINTF( "the album is :" );
            for ( i = 0; i < sizeof( p_music_info.album ); i++ ) {
                AUI_PRINTF( "0x%02x  ", ( char ) * ( palbum + i ) );
            }
            AUI_PRINTF( "\n" );
            
            pyear = p_music_info.year;
            AUI_PRINTF( "the year is :" );
            for ( i = 0; i < sizeof( p_music_info.year ); i++ ) {
                AUI_PRINTF( "0x%02x  ", ( char ) * ( pyear + i ) );
            }
            AUI_PRINTF( "\n" );
            AUI_PRINTF( "the genre is %lu\n", p_music_info.genre );
            AUI_PRINTF( "the track is %lu\n", p_music_info.track );
            AUI_PRINTF( "the time is %lu s\n", p_music_info.time );
            AUI_PRINTF( "the file_length is %lu byte\n", p_music_info.file_length );
            break;
        case 1:
            AUI_TEST_CHECK_RTN( aui_music_get( music_handle, AUI_MUSIC_GET_DECODER_INFO, &p_decoder_info ) );
#if 1
            AUI_PRINTF( "the bit_rate is %lu\n", p_decoder_info.bit_rate );
            AUI_PRINTF( "the sample_rate is %lu\n", p_decoder_info.sample_rate );
            AUI_PRINTF( "the channel_mode is %lu\n", p_decoder_info.channel_mode );
#endif
            break;
        default:
            break;
    }
    return AUI_RTN_SUCCESS;
}

void music_test_reg()
{
    aui_tu_reg_group( "music", "music tests" );
    aui_tu_reg_item( 2, "open", AUI_CMD_TYPE_API, test_aui_music_open, "music open" );
    aui_tu_reg_item( 2, "close", AUI_CMD_TYPE_API, test_aui_music_close, "music close" );
    aui_tu_reg_item( 2, "start", AUI_CMD_TYPE_API, test_aui_music_start, "music start" );
    aui_tu_reg_item( 2, "stop", AUI_CMD_TYPE_API, test_aui_music_stop, "music stop" );
    aui_tu_reg_item( 2, "pause", AUI_CMD_TYPE_API, test_aui_music_pause, "music pause" );
    aui_tu_reg_item( 2, "resume", AUI_CMD_TYPE_API, test_aui_music_resume, "music resume" );
    aui_tu_reg_item( 2, "seek", AUI_CMD_TYPE_API, aui_test_music_seek, "music seek" );
    aui_tu_reg_item( 2, "totaltime", AUI_CMD_TYPE_API, aui_test_total_time_get, "music get total time" );
    aui_tu_reg_item( 2, "currenttime", AUI_CMD_TYPE_API, aui_test_cur_time_get, "music get current time" );
    aui_tu_reg_item( 2, "info", AUI_CMD_TYPE_API, test_aui_music_get, "music get audio information" );
}


