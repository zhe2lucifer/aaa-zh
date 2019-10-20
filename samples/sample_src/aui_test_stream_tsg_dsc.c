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
#include <aui_kl.h>
#include <aui_dsc.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_tsg_dsc.h"

/* TSG parameters */
#define PKT_SIZE 188
#define RD_PKT_NUM 512
#define RD_BLK_SIZE 1024

enum {
    STREAM,
    STREAM_DSC,
    STREAM_DSC_KL
};

struct decrypt_conf {
    void *dev;
    unsigned short algo;
};

struct crypto_param {
    int mode;
    struct aui_cfg_kl kl_attr;
    struct aui_st_attr_dsc dsc_attr;
    unsigned char iv[16 * 2]; /* odd and even */
};
static struct crypto_param crypto_param;

//#define KEY_AES128
#define KEY_CSA1

/* CSA1 crypto parameters */
#ifdef KEY_CSA1
#define KEY_LEN 8
#define KEY_ALGO AUI_DSC_ALGO_CSA
static unsigned char clear_key[][8] = {
    { 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd }, /* odd */
    { 0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 } }; /* even */

/* TDES protected and content key that gives above clear_key */
static const unsigned char protected_keys[][16] = {
    { 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
      0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
    { 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
      0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f }};

static const unsigned char protected_cw_odd[8] = {
    0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f };

static const unsigned char protected_cw_even[8] = {
    0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef };
#endif

/* AES crypto parameters */
#ifdef KEY_AES128
#define KEY_LEN 16
#define KEY_ALGO AUI_DSC_ALGO_AES
/* protected keys */
static unsigned char protected_keys[][16] = {
    { 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
      0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
    { 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
      0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f }};

static const unsigned char protected_cw_odd[16] = {
    0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
    0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef };

static const unsigned char protected_cw_even[16] = {
    0xf5,0x70,0x13,0x7e,0x9f,0x1d,0x05,0x4f,
    0x13,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xfe };

/* expected result with root key at zero */
static unsigned char result_keys[][16] = {
    { 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
      0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
    { 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
      0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
    { 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd, /* odd */
      0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 },
    { 0x15,0x75,0xf8,0x6d,0xf1,0x9c,0x79,0xb4, /* even */
      0x3c,0x42,0xd5,0x1a,0x4c,0x50,0xac,0x32 }};

static unsigned char *clear_key = result_keys[2];
#endif


extern unsigned short pids[];
extern unsigned long s_play_mode;

AUI_RTN_CODE aui_kl_read_key(aui_hdl handle, unsigned char *key);

static int ali_app_crypto_init(unsigned short dsc_id, aui_hdl *p_dsc_hdl, aui_hdl *p_kl_hdl)
{
    int i;

    crypto_param.dsc_attr.uc_dev_idx = dsc_id;
    crypto_param.dsc_attr.uc_algo = KEY_ALGO;
    crypto_param.dsc_attr.csa_version = AUI_DSC_CSA1;
    crypto_param.dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;

    if (crypto_param.mode != STREAM_DSC_KL)
        goto skip_kl_init;

    /*
     * Key ladder initialization
     */
    struct aui_attr_kl attr;
    attr.uc_dev_idx = 0;
    attr.en_key_pattern = (KEY_LEN == 8 ) ? AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN :
        AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
    attr.en_level = AUI_KL_KEY_THREE_LEVEL;
    attr.en_root_key_idx = 0;

    if (aui_kl_open(&attr, p_kl_hdl)) {
        AUI_PRINTF("aui_kl_open error\n");
        return 1;
    }

    crypto_param.kl_attr.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
    crypto_param.kl_attr.en_kl_algo = AUI_KL_ALGO_TDES;
    crypto_param.kl_attr.en_crypt_mode = AUI_KL_DECRYPT;
    crypto_param.kl_attr.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;

    MEMCPY(crypto_param.kl_attr.ac_key_val, protected_keys, sizeof(protected_keys));

    if (crypto_param.kl_attr.en_cw_key_attr == AUI_KL_CW_KEY_EVEN)
        MEMCPY(crypto_param.kl_attr.ac_key_val + sizeof(protected_keys),
               protected_cw_even, KEY_LEN);
    else if (crypto_param.kl_attr.en_cw_key_attr == AUI_KL_CW_KEY_ODD)
        MEMCPY(crypto_param.kl_attr.ac_key_val + sizeof(protected_keys),
               protected_cw_odd, KEY_LEN);
    else if (crypto_param.kl_attr.en_cw_key_attr == AUI_KL_CW_KEY_ODD_EVEN) {
        /* even before odd for AES algo */
        MEMCPY(crypto_param.kl_attr.ac_key_val + sizeof(protected_keys),
               protected_cw_odd, KEY_LEN);
        MEMCPY(crypto_param.kl_attr.ac_key_val + sizeof(protected_keys) + KEY_LEN,
               protected_cw_even, KEY_LEN);
    }
    if (aui_kl_gen_key_by_cfg(*p_kl_hdl, &crypto_param.kl_attr,
                  &crypto_param.dsc_attr.ul_key_pos)) {
        AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
        return 1;
    }

    unsigned char result_key[KEY_LEN]; /* 128 bits */
    if (aui_kl_read_key(*p_kl_hdl, result_key)) {
        AUI_PRINTF("aui_kl_read_key error\n");
        return 1;
    }
    AUI_PRINTF("KL key pos %d: ", (int)crypto_param.dsc_attr.ul_key_pos);
    for (i=0; i<KEY_LEN; i++)
        AUI_PRINTF("%02x ", result_key[i]);
    AUI_PRINTF("\n");

skip_kl_init:

    /*
     * Descrambler initialization
     */
    if (aui_dsc_open(&crypto_param.dsc_attr, p_dsc_hdl)) {
        AUI_PRINTF("dsc open error\n");
        return 1;
    }

    if (crypto_param.mode == STREAM_DSC) {
        crypto_param.dsc_attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
        /* odd and even */
        crypto_param.dsc_attr.puc_key = (unsigned char *)clear_key;
        crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
        crypto_param.dsc_attr.ul_key_len = KEY_LEN*8;
        crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    } else {
        /* odd and even */
        crypto_param.dsc_attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;

        crypto_param.dsc_attr.puc_key = NULL;
        crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
        crypto_param.dsc_attr.ul_key_len = KEY_LEN*8;
        crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    }

    crypto_param.dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
    crypto_param.dsc_attr.puc_iv_ctr = crypto_param.iv;
    crypto_param.dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

    for (i=0; i<2; i++) {
        crypto_param.dsc_attr.ul_pid_cnt = 1;
        crypto_param.dsc_attr.pus_pids = pids + i;

        if (aui_dsc_attach_key_info2dsc(*p_dsc_hdl, &crypto_param.dsc_attr)) {
            AUI_PRINTF("dsc attach key error\n");
            return 1;
        }
    }

    crypto_param.dsc_attr.ul_pid_cnt = 2;
    crypto_param.dsc_attr.pus_pids = pids;

    if (aui_dsc_attach_key_info2dsc(*p_dsc_hdl, &crypto_param.dsc_attr)) {
        AUI_PRINTF("dsc attach key error\n");
        return 1;
    }
    return 0;
}

#ifdef AUI_LINUX
static int ali_app_dsc_deinit(aui_hdl hdl)
{
    if(aui_dsc_close(hdl)) {
        AUI_PRINTF("\r\n aui_dsc_close error \n");
        return 1;
    }
    return 0;
}
#endif

static void show_tsg_usage() {
    AUI_PRINTF("command as fallow:\n");
    AUI_PRINTF("cmd_num [path],[vpid],[apid],[ppid],[video format],[audio format],[change_mode]\n");
    AUI_PRINTF("such as :5 /mnt/uda1/tvstream.ts,234,235,234,0,1,0");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");

}
/**@file
*    @note      Copyright(C) ALi Corporation. All rights reserved.
*   
*   this sample is used to test TSG playing encrypted TS stream from local media.
*                                         
*   TS route:local media file-->TSG-->TSI-->DMX-->DSC-->DMX-->|-->DECA------->SND-->|--->HDMI
*                                                             |-->DECV-->|--->DIS-->|
*                                                             |-->SUBT-->|
*
*   
*/
unsigned long test_tsg_dsc(unsigned long *argc,char **argv,char *sz_out_put)
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
        show_tsg_usage();
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
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSI opened\n");

    //init dsc device device
    MEMSET(&crypto_param, 0, sizeof(crypto_param));
    crypto_param.mode = STREAM_DSC;
    if (crypto_param.mode != STREAM) {
        if (ali_app_crypto_init(1 ,// dsc id
                    &aui_hdls.dsc_hdl, &aui_hdls.kl_hdl)){
            AUI_PRINTF("\r\n ali_app_dsc_init failed!");
            return -1;
        }
        AUI_PRINTF("AUI DSC/KL opened %p/%p\n", aui_hdls.dsc_hdl, aui_hdls.kl_hdl);
    }

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

    /*set video and audio  synchronous ways,TS from nim,set to AUI_STC_AVSYNC_PCR.
     *TS from tsg,set to AUI_STC_AVSYNC_AUDIO*/
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
    MEMSET(&path,0,sizeof(aui_dmx_data_path));
    path.p_hdl_de_dev = aui_hdls.dsc_hdl;
    path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
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

    
    AUI_PRINTF("tsg file %s\n", argv[0]);
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
#ifdef AUI_LINUX
    AUI_PRINTF("\r\n close dsi aui");
    if (aui_hdls.dsc_hdl && ali_app_dsc_deinit(aui_hdls.dsc_hdl))
        AUI_PRINTF("\r\n ali_app_dsc_deinit failed!");
#endif

    ali_app_deinit(&aui_hdls);
    return AUI_RTN_SUCCESS;
}
/*====================================================*/
/*===============End of add===========================*/
/*====================================================*/
