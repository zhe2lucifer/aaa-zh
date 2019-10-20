#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aui_ini_config.h"
#else
#include <api/libfs2/stdio.h>
#endif

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_tsg.h>
#include <aui_nim.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_av.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_nim.h"

#define NIM_TIMEOUT_MS 1000
/* input Ku-Band signal from LNB*/
#ifdef LNB_CFG_KU_BAND
/* Astra channel "Arte"
 * clear file : stream_arte.ts
 * csa1 file : stream_arte_csa1.ts
 * aes file : stream_arte_aes.ts
 */
#define FREQ 10743
#define SYMB 22000
static unsigned short pids[]; //= { 401 /* video */, 402 /* audio */, 401 /* pcr */ };
/* input C-Band signal from LNB*/
#elif LNB_CFG_C_BAND
/* cctv2 */
#define FREQ 3840
#define SYMB 27500
static unsigned short pids[];// = { 513 /* video */, 660 /* audio */, 8190 /* pcr */ };
#else
#define FREQ 0
#define SYMB 0
#endif

extern unsigned long s_play_mode;
static void show_nim_usage() {
    AUI_PRINTF("cmd_num [<nim id>]\n");
    AUI_PRINTF("if you want to changed the default configure,as fallow\n");
    AUI_PRINTF("arguments for DVBS and DVBC:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[symb],[vpid],[apid],[ppid],[video format],[audio format],[change_mode]\n");
    AUI_PRINTF("such as CCTV2,input:1 1,0,3840,27500,513,660,8190,0,1,0\n");
    AUI_PRINTF("        CCTV7,input:1 1,0,3840,27500,514,670,8190,0,1,0\n");
    AUI_PRINTF("arguments for DVBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[DVBT type]\n");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

/**@file
*    @note      Copyright(C) ALi Corporation. All rights reserved.
*   
*   this sample is used to test nim playing clear TS stream from DVB signal.
*   TS route:DVB signal-->NIM-->TSI-->DMX->|-->DECA------->SND-->|--->HDMI
*                                          |-->DECV-->|--->DIS-->|
*                                          |-->SUBT-->|
*/
unsigned long test_nim(unsigned long *argc,char **argv,char *sz_out_put)
{
    struct ali_aui_hdls aui_hdls;
    struct ali_app_modules_init_para init_para;

    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    /*assert arguement*/
    if (0 == *argc) {
        show_nim_usage();
        return AUI_RTN_EINVAL;
    }
    *sz_out_put = 0;


    /*init init_para variable to init device usecd*/
    ali_aui_init_para_for_test_nim(argc,argv,&init_para);
    
    /*nim_init_cb is callback function point,to init nim device about special board
     * for example M3733,M3515B*/
    
    //open tsi devcie,and set TS source
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_live;
    }
    AUI_PRINTF("\r\nAUI TSI opened\n");

#ifdef AUI_LINUX    
    /*nim_init_cb is callback function point,to init nim device about special board
     * for example M3733,M3515B*/
    if (aui_nim_init(nim_init_cb)) {
        AUI_PRINTF("\nnim init error\n");
        goto err_live;
    }
    AUI_PRINTF("AUI NIM opened\n");/* Start streaming */

    /*open and init display device*/
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_live;
    }
    AUI_PRINTF("AUI DIS opened\n");
#endif

    /*set deca coding type, when changing programs,showing mode in the screen*/
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        AUI_PRINTF("\r\n ali_app_decv_init failed!");
        goto err_live;
    }
    AUI_PRINTF("AUI DECV opened\n");

    /*set deca coding type,open deca*/
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        AUI_PRINTF("\r\n ali_app_audio_init failed!");
        goto err_live;
    }
    AUI_PRINTF("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    /*set vpid,apid,pcr and create av stream*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
            AUI_PRINTF("\r\n set dmx failed!");
            goto err_live;
        }
    AUI_PRINTF("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;    
    MEMSET(&path,0,sizeof(aui_dmx_data_path));
    path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
        goto err_live;
    }
    AUI_PRINTF("dmx data path set %d\n", path.data_path_type);

    /*lock special pid TS stream*/
    if (nim_connect(&init_para.init_para_nim,&aui_hdls.nim_hdl)) {
        AUI_PRINTF("\nnim connect error\n");
        goto err_live;
    }
    AUI_PRINTF("nim connect success\n");

    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices 
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_live;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
    	{
    		AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_live;
    	}
    }
    else{
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL)){
    		AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_live;
    	}
    }

    unsigned long ch;
    
    AUI_PRINTF("Press '1' to stop streaming");
    AUI_PRINTF("\n");

    while(1) {
         /*this function don't return until user input enter key.
         *this function must put code excuted last */
        aui_test_get_user_hex_input(&ch);
        if (ch == 1) /* stop stream */
            break;
        AUI_SLEEP(10);/*sleep 10ms*/
    }
err_live:
    ali_app_deinit(&aui_hdls);
    return 0;

}

/*====================================================*/
/*===============End of add===========================*/
/*====================================================*/
