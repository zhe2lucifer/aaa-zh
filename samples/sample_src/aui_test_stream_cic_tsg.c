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
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <aui_cic.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"

#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_cic_tsg.h"

/* TSG parameters */
#define PKT_SIZE 188
#define RD_PKT_NUM 512
#define RD_BLK_SIZE 1024
extern unsigned long s_play_mode;

static void show_cic_tsg_usage() {
    AUI_PRINTF("command as fallow:\n");
    AUI_PRINTF("cmd_num [path],[vpid],[apid],[ppid],[video format],[audio format],[dis format]\n");
    AUI_PRINTF("such as :4 /mnt/uda1/tvstream.ts,234,235,234,0,1,0");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

/**@file
*    @note      Copyright(C) ALi Corporation. All rights reserved.
*   
*   this sample is used to test TSG playing clear TS stream from local media.
* 
*   TS route:local media file-->TSG-->TSI-->DMX->|-->DECA------->SND-->|--->HDMI
*                                                |-->DECV-->|--->DIS-->|
*                                                |-->SUBT-->|
*
**/
unsigned long test_cic_tsg(unsigned long *argc,char **argv,char *sz_out_put)
{
    struct ali_aui_hdls aui_hdls;
    struct aui_attr_tsg tsg_attr;
    struct ali_app_modules_init_para init_para;
    unsigned long pkt_empty,pkt_in_tsg,pkt_cnt;
    int len, ret;
    FILE *file;

    *sz_out_put = 0;    
    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    unsigned long i;
    AUI_PRINTF("*argc = %d,argv: ",*argc);
    for(i = 0;i < *argc;i++){
        AUI_PRINTF("%s,",argv[i]);
    }
    AUI_PRINTF("\n");
    if (7 != *argc) {
        show_cic_tsg_usage();
        return 0;
    }

    len = (PKT_SIZE * RD_PKT_NUM) / RD_BLK_SIZE;  //94K 
    pkt_cnt = RD_PKT_NUM;

    /*init init_para variable*/
    ali_aui_init_para_for_test_tsg(argc,argv,&init_para);
	
    //init tsg device
    if(ali_app_tsg_init(&init_para.init_para_tsg, &aui_hdls.tsg_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsg_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSG opened\n");

    //init tsi device
    if (ali_app_tsi_init_cic(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSI opened\n");

#ifdef AUI_LINUX
    //init dis device
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DIS opened\n");
#endif

    //init decv device
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        AUI_PRINTF("\r\n ali_app_decv_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DECV opened\n");

    // init deca device
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        AUI_PRINTF("\r\n ali_app_audio_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    //set video and audio  synchronous ways,
    if(aui_decv_sync_mode(aui_hdls.decv_hdl,AUI_STC_AVSYNC_AUDIO)){
        AUI_PRINTF("Set AUI_DEVC_SYNC_AUDIO fail\n");
        goto err_tsg;
    }

    /*init dmx devce*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
            AUI_PRINTF("\r\n set dmx failed!");
            goto err_tsg;
        }
    AUI_PRINTF("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;
	MEMSET(&path, 0, sizeof(aui_dmx_data_path));
    path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
        goto err_tsg;
    }
    AUI_PRINTF("dmx data path set %d\n", path.data_path_type);


    /*set TS of dmx device from local media player*/
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,(void *)AUI_AVSYNC_FROM_HDD_MP)) {
        AUI_PRINTF("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
        goto err_tsg;
    }
    
    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices 
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    else{
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL))
         {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    
    //AUI_PRINTF("tsg file %s\n", argv[0]);
    AUI_PRINTF("tsg file: %s\n",argv[0]);
    file = fopen(argv[0], "r");
    if (!file) {
        AUI_PRINTF("tsg file open errno\n");
        goto err_tsg;
    }
    void *buffer = malloc((RD_BLK_SIZE*len));
    if(NULL == buffer)
    {
        AUI_PRINTF("tsg malloc errno\n");
        goto err_tsg;
    }
    
    
    
    while (1) {
        ret = fread(buffer, RD_BLK_SIZE, len, file);
        if (0 == ret) {
            AUI_PRINTF("tsg file open errno \n");
            fclose(file);
            goto err_tsg;
        }
    #ifdef AUI_LINUX
        if (ret != len) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len);
            break;
        }
    #else
        if (ret != len*RD_BLK_SIZE) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len*RD_BLK_SIZE);
            break;
        }
    #endif


        if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
        if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
            AUI_PRINTF("aui_dmx_get error\n");

        while(pkt_empty < (pkt_cnt + pkt_in_tsg)) {
            AUI_SLEEP(10);
            if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
                AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                goto err_tsg;
            }
            if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
                AUI_PRINTF("aui_dmx_get error\n");
        }
        /* push data in TSG */
        tsg_attr.ul_addr = (unsigned char *)buffer;
        tsg_attr.ul_pkg_cnt = pkt_cnt;
        tsg_attr.uc_sync_mode = 0;
        ret = aui_tsg_send(aui_hdls.tsg_hdl, &tsg_attr);
        if (ret) {
            AUI_PRINTF("\naui_tsg_send error 0x%x\n", ret);
            fclose(file);
            goto err_tsg;
        }
    }
    fclose(file);

err_tsg:
    ali_app_deinit(&aui_hdls);
    return AUI_RTN_SUCCESS;
}

/*====================================================*/
/*===============End of add===========================*/
/*====================================================*/

