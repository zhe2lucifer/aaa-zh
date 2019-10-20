/****************************INCLUDE HEAD FILE************************************/
#include "aui_image_test.h"
#include "aui_fs.h"
#ifdef AUI_TDS
    //#include <hld_dev.h>
    #include<aui_dis.h>
    #include <api/libmp/pe.h>
#endif
/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/
void *image_handle = NULL;
//static int first_init = 1;
static int dis_mode = 0;
//static int dis_mode = 2;





/****************************EXTERN VAR*******************************************/

/****************************EXTERN FUNC *****************************************/
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

static void test_aui_image_message_callback( aui_image_message msg, void *pv_data, void *pv_user_data )
{
    unsigned long msg_code = ( unsigned long )pv_data;
    
    switch ( msg ) {
        case AUI_IMAGE_SHOW_COMPLETE:
            AUI_PRINTF( "Got AUI_IMAGE_SHOW_COMPLETE, msg code %d, user_data 0x%X\n", msg_code, pv_user_data );
            break;
        default:
            AUI_PRINTF( "Got unkown msg\n" );
    }
    
}

/****************************TEST MODULE IMPLEMENT********************************/
AUI_RTN_CODE test_aui_image_open( unsigned long *argc, char **argv, char *sz_out_put )
{
    aui_attr_image attr_image;
    int ul_item;
    MEMSET( &attr_image, 0, sizeof( aui_attr_image ) );
    if ( *argc < 1 ) {
        AUI_PRINTF( "Example: open /mnt/uda1/image1.jpg" );
        return AUI_RTN_EINVAL;
    }
    //init usb fs
    waiting_usb_fs_init();
    
    aui_f_hdl *fp_w = NULL;
    //fp_w = fopen("/mnt/uda1/image1.jpg","rb");
    fp_w = ( aui_f_hdl * )aui_fs_open( argv[0], "r" );
    if ( NULL == fp_w ) {
        AUI_PRINTF( "open file image file fail\n" );
        return AUI_RTN_EINVAL;
    }
    AUI_PRINTF( "fp_w is 0x%x\n", fp_w );
    aui_fs_close( fp_w );
    //add mp init
    //aui_win_media_player_init(commom_mp_callback_wrapper);
    
    STRCPY( attr_image.uc_file_name, argv[0] );
    ul_item = atoi( argv[1] );
    //AUI_PRINTF("open file %s\n", attr_image.uc_file_name);
    switch ( ul_item ) {
        case 0:
            attr_image.en_mode = AUI_MODE_FULL;
            dis_mode = AUI_MODE_FULL;
            break;
        case 1:
            attr_image.en_mode = AUI_MODE_PREVIEW;
            dis_mode = AUI_MODE_PREVIEW;
            break;
        case 2:
            attr_image.en_mode = AUI_MODE_MULTIIEW;
            dis_mode = AUI_MODE_MULTIIEW;
            break;
        default:
            break;
    }
    attr_image.aui_image_cb = test_aui_image_message_callback;
    attr_image.user_data = 0;
    if (NULL != image_handle){
        aui_image_close(NULL,&image_handle);
        image_handle = NULL;
    }
    AUI_TEST_CHECK_RTN(aui_image_open(&attr_image, &image_handle));
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE test_aui_image_close( unsigned long *argc, char **argv, char *sz_out_put )
{
    aui_attr_image attr_image;
    MEMSET( &attr_image, 0, sizeof( aui_attr_image ) );
    
    aui_attr_dis attr_dis;
    void *g_dis_handle;
    
    MEMSET( &attr_dis, 0 , sizeof( aui_attr_dis ) );
    
    attr_dis.uc_dev_idx = AUI_DIS_HD;
    
    if ( aui_find_dev_by_idx( AUI_MODULE_DIS, AUI_DIS_HD, &g_dis_handle ) ) {
        if ( aui_dis_open( &attr_dis, &g_dis_handle ) ) {
            AUI_PRINTF( "\n aui_dis_open HD fail\n" );
            return -1;
        }
    }
    //aui_dis_video_enable function is used to close video coverage to relizing clean picture on the screen
    //when change to music module
    aui_dis_video_enable(g_dis_handle, 0);
    AUI_TEST_CHECK_RTN(aui_image_close(&attr_image, &image_handle));
    image_handle =NULL;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_image_start( unsigned long *argc, char **argv, char *sz_out_put )
{
    if ( dis_mode == AUI_MODE_FULL ) {
        AUI_TEST_CHECK_RTN( aui_image_start( image_handle, NULL, 0 ) );
    } else if ( dis_mode == AUI_MODE_PREVIEW ) {
        aui_image_rect image_rect_preview;
        image_rect_preview.start_x = 0;
        image_rect_preview.start_y = 0;
        image_rect_preview.width = 100;
        image_rect_preview.height = 100;
        
        AUI_TEST_CHECK_RTN( aui_image_start( image_handle, &image_rect_preview, 1 ) );
    } else {
        aui_image_rect *image_rect = ( aui_image_rect * )MALLOC( 5 * sizeof( aui_image_rect ) );
        image_rect[0].start_x = 0;
        image_rect[0].start_y = 0;
        image_rect[0].width = 100;
        image_rect[0].height = 100;
        
        image_rect[1].start_x = 100;
        image_rect[1].start_y = 0;
        image_rect[1].width = 100;
        image_rect[1].height = 100;
        
        image_rect[2].start_x = 200;
        image_rect[2].start_y = 0;
        image_rect[2].width = 100;
        image_rect[2].height = 100;
        
        image_rect[3].start_x = 300;
        image_rect[3].start_y = 0;
        image_rect[3].width = 100;
        image_rect[3].height = 100;
        
        image_rect[4].start_x = 400;
        image_rect[4].start_y = 0;
        image_rect[4].width = 100;
        image_rect[4].height = 100;
        AUI_TEST_CHECK_RTN( aui_image_start( image_handle, image_rect, 5 ) );
        
        
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_image_set( unsigned long *argc, char **argv, char *sz_out_put )
{
    //AUI_TEST_CHECK_NULL(argc);
    //AUI_TEST_CHECK_NULL(argv);
    //AUI_TEST_CHECK_NULL(sz_out_put);
    //AUI_TEST_CHECK_VAL(*argc,1);
    
    
    char *fp_w = NULL;
    //fp_w = "/mnt/uda1/pic1.jpg";
    fp_w = "/mnt/uda1/Error 1024x768_014.jpg";
    if ( NULL  == fp_w ) {
        AUI_PRINTF( "error\n" );
    }
    
    
    if ( dis_mode == AUI_MODE_FULL ) {
        AUI_TEST_CHECK_RTN( aui_image_set( image_handle, AUI_IMAGE_SET_FILE_NAME, fp_w ) );
    } else if ( dis_mode == AUI_MODE_PREVIEW ) {
        AUI_TEST_CHECK_RTN( aui_image_set( image_handle, AUI_IMAGE_SET_FILE_NAME, argv[0] ) );
    } else {
        aui_multi_name_set *pst_file_ary = ( aui_multi_name_set * )MALLOC( sizeof( aui_multi_name_set ) );
        //pst_file_ary->ui_file_num = 5;
        //MEMCPY((pst_file_ary->uc_multi_file_name)[0],"/mnt/uda1/123.jpg", strlen("/mnt/uda1/123.jpg") + 1);
        //MEMCPY((pst_file_ary->uc_multi_file_name)[1],"/mnt/uda1/bambi.jpg", strlen("/mnt/uda1/bambi.jpg") + 1);
        //MEMCPY((pst_file_ary->uc_multi_file_name)[2],"/mnt/uda1/19201280.jpg", strlen("/mnt/uda1/19201280.jpg") + 1);
        //MEMCPY((pst_file_ary->uc_multi_file_name)[3],"/mnt/uda1/1920X1280.jpg", strlen("/mnt/uda1/1920X1280.jpg") + 1);
        //MEMCPY((pst_file_ary->uc_multi_file_name)[4],"/mnt/uda1/IMG_9097.jpg", strlen("/mnt/uda1/IMG_9097.jpg") + 1);
        
        pst_file_ary->ui_file_num = 5;
        MEMCPY( ( pst_file_ary->uc_multi_file_name )[0], "/mnt/uda1/pic1.jpg", strlen( "/mnt/uda1/pic1.jpg" ) + 1 );
        MEMCPY( ( pst_file_ary->uc_multi_file_name )[1], "/mnt/uda1/pic2.jpg", strlen( "/mnt/uda1/pic2.jpg" ) + 1 );
        MEMCPY( ( pst_file_ary->uc_multi_file_name )[2], "/mnt/uda1/pic3.jpg", strlen( "/mnt/uda1/pic3.jpg" ) + 1 );
        MEMCPY( ( pst_file_ary->uc_multi_file_name )[3], "/mnt/uda1/pic1.jpg", strlen( "/mnt/uda1/pic1.jpg" ) + 1 );
        MEMCPY( ( pst_file_ary->uc_multi_file_name )[4], "/mnt/uda1/pic2.jpg", strlen( "/mnt/uda1/pic2.jpg" ) + 1 );
        AUI_TEST_CHECK_RTN( aui_image_set( image_handle, AUI_IMAGE_SET_MULTI_FILENAME, pst_file_ary ) );
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_image_get( unsigned long *argc, char **argv, char *sz_out_put )
{
    struct aui_image_info p_image_info;
    AUI_TEST_CHECK_RTN( aui_image_get( image_handle, AUI_IMAGE_GET_INFO, &p_image_info ) );
    AUI_PRINTF( "the fsize is %lu\n", p_image_info.fsize );
    AUI_PRINTF( "the width is %lu\n", p_image_info.width );
    AUI_PRINTF( "the height is %lu\n", p_image_info.height );
    AUI_PRINTF( "the whole pixel is %lu\n", p_image_info.bbp );
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE test_aui_image_zoom( unsigned long *argc, char **argv, char *sz_out_put )
{
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE test_aui_image_rotate( unsigned long *argc, char **argv, char *sz_out_put )
{
    AUI_TEST_CHECK_NULL( argc );
    AUI_TEST_CHECK_NULL( argv );
    AUI_TEST_CHECK_NULL( sz_out_put );
    AUI_TEST_CHECK_VAL( *argc, 1 );
    
    unsigned long val = get_num_by_string( argv[0] );
    AUI_TEST_CHECK_RTN( aui_image_rotate( image_handle, val ) );
    return AUI_RTN_SUCCESS;
    
}

AUI_RTN_CODE test_aui_image_display_mode_set( unsigned long *argc, char **argv, char *sz_out_put )
{
    AUI_TEST_CHECK_NULL( argc );
    AUI_TEST_CHECK_NULL( argv );
    AUI_TEST_CHECK_NULL( sz_out_put );
    AUI_TEST_CHECK_VAL( *argc, 1 );
    
    unsigned long val = get_num_by_string( argv[0] );
    
    dis_mode = val;
    
    AUI_TEST_CHECK_RTN( aui_image_display_mode_set( image_handle, val ) );
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE test_aui_image_move( unsigned long *argc, char **argv, char *sz_out_put )
{
    AUI_TEST_CHECK_NULL( argc );
    AUI_TEST_CHECK_NULL( argv );
    AUI_TEST_CHECK_NULL( sz_out_put );
    AUI_TEST_CHECK_VAL( *argc, 2 );
    
    unsigned long val = get_num_by_string( argv[0] );
    unsigned long val2 = get_num_by_string( argv[1] );
    
    AUI_TEST_CHECK_RTN( aui_image_move( image_handle, val, val2 ) );
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE test_aui_image_enlarge( unsigned long *argc, char **argv, char *sz_out_put )
{
    AUI_TEST_CHECK_NULL( argc );
    AUI_TEST_CHECK_NULL( argv );
    AUI_TEST_CHECK_NULL( sz_out_put );
    AUI_TEST_CHECK_VAL( *argc, 1 );
    
    unsigned long val = get_num_by_string( argv[0] );
    AUI_TEST_CHECK_RTN( aui_image_enlarge( image_handle, val ) );
    return AUI_RTN_SUCCESS;
}

void image_test_reg()
{
    aui_tu_reg_group( "image", "image tests" );
    //aui_tu_reg_item(2, "init", AUI_CMD_TYPE_API, test_aui_image_init, "image init");
    aui_tu_reg_item( 2, "open", AUI_CMD_TYPE_API, test_aui_image_open, "image open" );
    aui_tu_reg_item( 2, "set", AUI_CMD_TYPE_API, test_aui_image_set, "image set" );
    aui_tu_reg_item( 2, "start", AUI_CMD_TYPE_API, test_aui_image_start, "image start" );
    aui_tu_reg_item( 2, "rotate", AUI_CMD_TYPE_API, test_aui_image_rotate, "image rotate" );
    //aui_tu_reg_item(2, "zoom", AUI_CMD_TYPE_API, test_aui_image_zoom, "image zoom");
    aui_tu_reg_item( 2, "move", AUI_CMD_TYPE_API, test_aui_image_move, "image move" );
    aui_tu_reg_item( 2, "enlarge", AUI_CMD_TYPE_API, test_aui_image_enlarge, "image enlarge" );
    aui_tu_reg_item( 2, "info", AUI_CMD_TYPE_API, test_aui_image_get, "image infomation" );
    aui_tu_reg_item( 2, "close", AUI_CMD_TYPE_API, test_aui_image_close, "image close" );
}


