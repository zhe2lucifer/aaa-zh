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
#include <aui_kl.h>
#include <aui_dsc.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_nim_dsc.h"

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
#if 0
		/* even only */
		crypto_param.dsc_attr.puc_key = (unsigned char *)clear_key + KEY_LEN;
		crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_EVEN;
		crypto_param.dsc_attr.ul_key_len = KEY_LEN * 8;
		crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
#endif
#if 0
		/* odd only */
		crypto_param.dsc_attr.puc_key = (unsigned char *)clear_key;
		crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD;
		crypto_param.dsc_attr.ul_key_len = KEY_LEN * 8;
		crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
#endif
#if 1
		/* odd and even */
		crypto_param.dsc_attr.puc_key = (unsigned char *)clear_key;
		crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
		crypto_param.dsc_attr.ul_key_len = KEY_LEN * 8;
		crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
#endif
	} else {
		/* odd and even */
		crypto_param.dsc_attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;

		crypto_param.dsc_attr.puc_key = NULL;
		crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
		crypto_param.dsc_attr.ul_key_len = KEY_LEN * 8;
		crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
	}

	crypto_param.dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	crypto_param.dsc_attr.puc_iv_ctr = crypto_param.iv;
	crypto_param.dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

#if 1
	for (i=0; i<2; i++) {
		crypto_param.dsc_attr.ul_pid_cnt = 1;
		crypto_param.dsc_attr.pus_pids = pids + i;

		if (aui_dsc_attach_key_info2dsc(*p_dsc_hdl, &crypto_param.dsc_attr)) {
			AUI_PRINTF("dsc attach key error\n");
			return 1;
		}
	}
#else
	crypto_param.dsc_attr.ul_pid_cnt = 2;
	crypto_param.dsc_attr.pus_pids = pids;

	if (aui_dsc_attach_key_info2dsc(*p_dsc_hdl, &crypto_param.dsc_attr)) {
		AUI_PRINTF("dsc attach key error\n");
		return 1;
	}
#endif
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

static void show_nim_usage() {
    AUI_PRINTF("cmd_num [<nim id>]\n");
    AUI_PRINTF("if you want to changed the default configure,as fallow\n");
    AUI_PRINTF("arguments for DVBS and DVBC:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[symb],[vpid],[apid],[ppid],[video format],[audio format],[change_mode]\n");
    AUI_PRINTF("such as CCTV2,input:2 1,0,3840,27500,513,660,8190,0,1,0\n");
    AUI_PRINTF("        CCTV7,input:2 1,0,3840,27500,514,670,8190,0,1,0\n");
    AUI_PRINTF("arguments for DVBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[DVBT type]\n");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

/**@file
*    @note      Copyright(C) ALi Corporation. All rights reserved.
*   
*   this sample is used to test nim playing encrypted TS stream from DVB signal.
*                                         
*   signal route:DVB signal-->NIM-->TSI-->DMX-->DSC-->DMX-->|-->DECA------->SND-->|--->HDMI
*                                                           |-->DECV-->|--->DIS-->|
*                                                           |-->SUBT-->|
*
*   
*/

unsigned long test_nim_dsc(unsigned long *argc,char **argv,char *sz_out_put)
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

    //open tsi devcie,and set TS source
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_live;
    }
    AUI_PRINTF("\r\nAUI TSI opened\n");

    /*init dsc device device*/
    MEMSET(&crypto_param, 0, sizeof(crypto_param));
    crypto_param.mode = STREAM_DSC;
    if (ali_app_crypto_init(1,/* dsc id */
                &aui_hdls.dsc_hdl, NULL)){
        AUI_PRINTF("\r\n ali_app_dsc_init failed!");
        goto err_live;
    }
    AUI_PRINTF("AUI DSC opened %p\n", aui_hdls.dsc_hdl);

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
    path.p_hdl_de_dev = aui_hdls.dsc_hdl;
    path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
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
    int iv_toggle = 0;

    AUI_PRINTF("Press '1' to stop streaming");
    AUI_PRINTF(", '2' remove/add keys, '3' change IV");
    AUI_PRINTF("\n");

    while(1) {
        if (ch == 2) {
            int i;
            AUI_PRINTF("test key detach\n");
            for (i=0; i<3; i++)
                if (aui_dsc_deattach_key_by_pid(aui_hdls.dsc_hdl, pids[i]))
                    AUI_PRINTF("warning dsc detach key with pid %d error\n", pids[i]);
            AUI_SLEEP(5);
            crypto_param.dsc_attr.ul_pid_cnt = 2;
            crypto_param.dsc_attr.pus_pids = pids;
            if (aui_dsc_attach_key_info2dsc(aui_hdls.dsc_hdl, &crypto_param.dsc_attr)) {
                AUI_PRINTF("dsc attach key error\n");
                return 1;
            }
        } else if (ch == 3) {
            AUI_PRINTF("change IV\n");
            iv_toggle = !iv_toggle;
            MEMSET(crypto_param.iv, iv_toggle, 16*2);
            if (aui_dsc_attach_key_info2dsc(aui_hdls.dsc_hdl, &crypto_param.dsc_attr)) {
                AUI_PRINTF("dsc attach key error\n");
                return 1;
            }
        }
        /*this function don't return until user input enter key.
         *this function must put code excuted last */
        aui_test_get_user_hex_input(&ch);
        if (ch == 1) /* stop stream */
            break;
        AUI_SLEEP(10);
    }

err_live:
#ifdef AUI_LINUX
    AUI_PRINTF("\r\n close dsi aui");
    if (aui_hdls.dsc_hdl && ali_app_dsc_deinit(aui_hdls.dsc_hdl))
        AUI_PRINTF("\r\n ali_app_dsc_deinit failed!");
#endif
    ali_app_deinit(&aui_hdls);
    return AUI_RTN_SUCCESS;
}

#define AUI_TEST_CA_AUD_PID     (0x1120)
#define AUI_TEST_CA_VID_PID     (0x1122)
#define AUI_TEST_CA_PCR_PID     (0x1120)
unsigned long test_aui_av_live_play_encrypt_stream(unsigned long *argc,char **argv,char *sz_out_put)
{
   
    aui_dmx_data_path aui_dmx_route;
    aui_hdl aui_hdl_dmx=NULL;
    aui_hdl aui_hdl_dsc=NULL;
    aui_attr_dsc attr_dsc;
    //unsigned short apids_audio[1]={AUI_TEST_CA_AUD_PID};
    //unsigned short apids_video[1]={AUI_TEST_CA_VID_PID};
    unsigned short apids[2]={AUI_TEST_CA_AUD_PID,AUI_TEST_CA_VID_PID};
    //odd key Even key
    unsigned char acKey[16]={0x83, 0xb4, 0xa2, 0xd9, 0x9f, 0x4a, 0xee, 0xd7,0xbe, 0x2b, 0x15, 0xfe, 0xff, 0xc, 0x61, 0x6c};
    //C com
    //unsigned char acKey[16]={0xfe,0x17,0x2c,0x41,0xa2,0xe1,0xd4,0x57, 0x34,0xe6,0x74,0x8e,0xea,0x2c,0x1c,0x32};
    MEMSET(&aui_dmx_route,0, sizeof(aui_dmx_data_path));
    MEMSET(&attr_dsc,0, sizeof(aui_attr_dsc));

    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX,0,&aui_hdl_dmx))
    {
        AUI_PRINTF("\r\n no dmx dev open");
        return AUI_RTN_FAIL;
    }   
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DSC,0,&aui_hdl_dsc))
    {
        AUI_PRINTF("\r\n no dsc dev open");
        attr_dsc.csa_version=AUI_DSC_CSA2;
        attr_dsc.dsc_data_type=AUI_DSC_DATA_TS;
        attr_dsc.uc_dev_idx=0;
        attr_dsc.uc_algo=AUI_DSC_ALGO_CSA;
        attr_dsc.dsc_key_type=AUI_DSC_HOST_KEY_SRAM;
        if(AUI_RTN_SUCCESS!=aui_dsc_open(&attr_dsc,&aui_hdl_dsc))
        {
            AUI_PRINTF("\r\n can not open dsc dev");
            return AUI_RTN_FAIL;
        } 
    } 
    
    AUI_PRINTF("\r\n dmx0=[%08x],dsc0=[%08x]",aui_hdl_dmx,aui_hdl_dsc);
    aui_dmx_route.data_path_type=AUI_DMX_DATA_PATH_DE_PLAY;
    aui_dmx_route.p_hdl_de_dev=aui_hdl_dsc;

    if(AUI_RTN_SUCCESS!=aui_dmx_data_path_set(aui_hdl_dmx,&aui_dmx_route))
    {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed");
        return AUI_RTN_FAIL;
    } 

    attr_dsc.csa_version=AUI_DSC_CSA2;
    attr_dsc.dsc_data_type=AUI_DSC_DATA_TS;
    attr_dsc.dsc_key_type=AUI_DSC_HOST_KEY_SRAM;
    attr_dsc.en_en_de_crypt=AUI_DSC_DECRYPT;
    //attr_dsc.ul_key_pos=key_pos_4_aud;
    attr_dsc.puc_key=acKey;
    attr_dsc.uc_dev_idx=0;
    attr_dsc.uc_algo=AUI_DSC_ALGO_CSA;
    attr_dsc.ul_key_len=64;
    attr_dsc.pus_pids=apids;
    attr_dsc.ul_pid_cnt=2;
    attr_dsc.en_parity=AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    
    if(AUI_RTN_SUCCESS!=aui_dsc_attach_key_info2dsc(aui_hdl_dsc,&attr_dsc))
    {
        AUI_PRINTF("\r\n aui_dsc_attach_key_info2dsc failed.");
    }  

	return AUI_RTN_SUCCESS;
    //
}
//unsigned char ac_encrypt_cwpk[16]={0x20,0xA4,0xB0,0xB3,0x87,0x36,0x31,0x25,0x77,0xB1,0x27,0x3C,0x86,0xA1,0x29,0x83};
unsigned char ac_encrypt_cwpk[16]={0x77,0xB1,0x27,0x3C,0x86,0xA1,0x29,0x83,0x20,0xA4,0xB0,0xB3,0x87,0x36,0x31,0x25};
unsigned char ac_encrypt_cw[16]={0x23,0x5B,0x9F,0xCA,0x2E,0xE5,0x5D,0xDE,0x26,0x98,0x25,0xD5,0xEE,0x6F,0xA0,0xB5};
//unsigned char ac_encrypt_cw[16]={0x26,0x98,0x25,0xD5,0xEE,0x6F,0xA0,0xB5,0x23,0x5B,0x9F,0xCA,0x2E,0xE5,0x5D,0xDE};
unsigned char acKey[16]={0xfe,0x17,0x2c,0x41,0xa2,0xe1,0xd4,0x57, 0x34,0xe6,0x74,0x8e,0xea,0x2c,0x1c,0x32};

AUI_RTN_CODE aui_dsc_play_encrypt_stream_csa_v2_kl()
{
    aui_hdl kl_hdl=NULL;
    aui_attr_kl attr_kl;
    aui_cfg_kl cfg_kl;
    aui_dmx_data_path aui_dmx_route;
    aui_hdl aui_hdl_dmx=NULL;
    aui_hdl aui_hdl_dsc=NULL;
    aui_attr_dsc attr_dsc;
    //unsigned short apids_audio[1]={602};
    //unsigned short apids_video[1]={502};
    unsigned short apids[2]={602,502};
    unsigned long ul_dst_keypos_lv1=0;
    unsigned long ul_dst_keypos_lv2=0;
    MEMSET(&aui_dmx_route,0, sizeof(aui_dmx_data_path));
    MEMSET(&attr_dsc,0, sizeof(aui_attr_dsc));
    MEMSET(&cfg_kl,0,sizeof(aui_cfg_kl));
    
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX,0,&aui_hdl_dmx))
    {
        AUI_PRINTF("\r\n no dmx dev open");
        return AUI_RTN_FAIL;
    }   
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DSC,0,&aui_hdl_dsc))
    {
        AUI_PRINTF("\r\n no dsc dev open");
        attr_dsc.csa_version=AUI_DSC_CSA2;
        attr_dsc.dsc_data_type=AUI_DSC_DATA_TS;
        attr_dsc.uc_dev_idx=0;
        attr_dsc.uc_algo=AUI_DSC_ALGO_CSA;
        attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;
        if(AUI_RTN_SUCCESS!=aui_dsc_open(&attr_dsc,&aui_hdl_dsc))
        {
            AUI_PRINTF("\r\n can not open dsc dev");
            return AUI_RTN_FAIL;
        } 
    } 
    
    AUI_PRINTF("\r\n dmx0=[%08x],dsc0=[%08x]",aui_hdl_dmx,aui_hdl_dsc);
    aui_dmx_route.data_path_type=AUI_DMX_DATA_PATH_DE_PLAY;
    aui_dmx_route.p_hdl_de_dev=aui_hdl_dsc;

    if(AUI_RTN_SUCCESS!=aui_dmx_data_path_set(aui_hdl_dmx,&aui_dmx_route))
    {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed");
        return AUI_RTN_FAIL;
    } 


    MEMSET(&attr_kl,0,sizeof(aui_attr_kl));

    attr_kl.uc_dev_idx=0;
    attr_kl.en_level=AUI_KL_KEY_TWO_LEVEL;
    attr_kl.en_key_pattern=AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
    attr_kl.en_root_key_idx=AUI_KL_ROOT_KEY_0_0;
    if(AUI_RTN_SUCCESS!=aui_kl_open(&attr_kl,&kl_hdl))
    {
        return AUI_RTN_FAIL;
    }

    MEMSET(&attr_kl,0,sizeof(aui_attr_kl));
    MEMSET(&cfg_kl,0,sizeof(aui_cfg_kl));

    cfg_kl.en_crypt_mode=AUI_KL_DECRYPT;
    cfg_kl.en_cw_key_attr=AUI_KL_CW_KEY_ODD_EVEN;
    cfg_kl.en_kl_algo=AUI_KL_ALGO_TDES;
    cfg_kl.run_level_mode=AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST;
    MEMCPY(cfg_kl.ac_key_val,ac_encrypt_cwpk,sizeof(ac_encrypt_cwpk));
    if(AUI_RTN_SUCCESS!=aui_kl_gen_key_by_cfg(kl_hdl,&cfg_kl,&ul_dst_keypos_lv1))
    {
        return AUI_RTN_FAIL;
    }


    MEMSET(&attr_kl,0,sizeof(aui_attr_kl));
    MEMSET(&cfg_kl,0,sizeof(aui_cfg_kl));

    cfg_kl.en_crypt_mode=AUI_KL_DECRYPT;
    cfg_kl.en_cw_key_attr=AUI_KL_CW_KEY_ODD_EVEN;
    cfg_kl.en_kl_algo=AUI_KL_ALGO_TDES;
    cfg_kl.run_level_mode=AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND;
    MEMCPY(cfg_kl.ac_key_val,ac_encrypt_cw,sizeof(ac_encrypt_cw));
    if(AUI_RTN_SUCCESS!=aui_kl_gen_key_by_cfg(kl_hdl,&cfg_kl,&ul_dst_keypos_lv2))
    {
        return AUI_RTN_FAIL;
    }

    attr_dsc.csa_version=AUI_DSC_CSA2;
    attr_dsc.dsc_data_type=AUI_DSC_DATA_TS;
    attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;//AUI_DSC_HOST_KEY_SRAM;//AUI_DSC_CONTENT_KEY_KL;
    attr_dsc.en_en_de_crypt=AUI_DSC_DECRYPT;
    attr_dsc.ul_key_pos=ul_dst_keypos_lv2;
    attr_dsc.puc_key=acKey;
    attr_dsc.uc_dev_idx=0;
    attr_dsc.uc_algo=AUI_DSC_ALGO_CSA;
    attr_dsc.ul_key_len=64;
    attr_dsc.pus_pids=apids;
    attr_dsc.ul_pid_cnt=2;
    attr_dsc.en_parity=AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    
    if(AUI_RTN_SUCCESS!=aui_dsc_attach_key_info2dsc(aui_hdl_dsc,&attr_dsc))
    {
        AUI_PRINTF("\r\n aui_dsc_attach_key_info2dsc failed.");
    }  
    return AUI_RTN_SUCCESS;

  
}

unsigned long test_aui_av_live_play_contentkey_stream(unsigned long *argc,char **argv,char *sz_out_put)
{
    return aui_dsc_play_encrypt_stream_csa_v2_kl();
}

/*====================================================*/
/*===============End of add===========================*/
/*====================================================*/
