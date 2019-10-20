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
#include "aui_test_stream_play_live_stream.h"

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

unsigned long s_play_mode=0;

static unsigned short static_ad_pid = AUI_INVALID_PID;

static void show_usage()
{
    AUI_PRINTF("cmd_num [<nim id>]\n");
    AUI_PRINTF("If you want to changed the default configure,as fallow\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for DVBS and DVBC:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[symb],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[polar]\n");
    AUI_PRINTF("such as CCTV2,input: play 1,0,3840,27500,513,660,8190,0,1,0,H,0\n");
    AUI_PRINTF("        CCTV7,input: play 1,0,3840,27500,514,670,8190,0,1,0,H,0\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for DVBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[DVBT(T2/ISDBT) type]\n");
    AUI_PRINTF("Example for DVBT:\n");
    AUI_PRINTF("        Input: play 0,2,850000,8,1029,1028,1029,0,0,0,1\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for DVB-T2 PLP:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[DVBT(T2/ISDBT) type],[PLP index]\n");
    AUI_PRINTF("PLP index is 0 ~ 255. When index is AUI_NIM_PLP_AUTO");
    AUI_PRINTF("When PLP index is AUI_NIM_PLP_AUTO, it will select the 0 PLP and you can get the PLP number from aui_signal_status\n");
    AUI_PRINTF("Example for play certain PLP index:\n");
    AUI_PRINTF("        Input: play 0,2,722000,8,141,142,141,1,2,0,2,3\n");
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for ISDBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[DVBT(T2/ISDBT) type]\n");
    AUI_PRINTF("Example for ISDBT:\n");
    AUI_PRINTF("        Input: play 0,2,587143,6,481,482,481,1,1,0,0\n");
    AUI_PRINTF("If you want to play with audio description, first configure audio description pid with ad_set command, then use play command\n");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}


static struct ali_aui_hdls *s_p_hdls=NULL;
unsigned long test_aui_av_live_play_clear_stream(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct ali_app_modules_init_para init_para;
	/*assert arguement*/
	if (0 == *argc) {
		show_usage();
		return 0;
	}
	s_p_hdls = malloc(sizeof(struct ali_aui_hdls));
	if(NULL == s_p_hdls) return -1;
	MEMSET(s_p_hdls, 0, sizeof(struct ali_aui_hdls));
	/*init all used device in this mode*/
	ali_aui_init_para_for_test_nim(argc,argv,&init_para);

#ifdef AUI_LINUX
    /*nim_init_cb is callback function point,to init nim device about special board
     * for example M3733,M3515B*/
    if (aui_nim_init(nim_init_cb)) {
        AUI_PRINTF("\nnim init error\n");
        goto err_live;
    }
    AUI_PRINTF("AUI NIM opened\n");/* Start streaming */

    /*open and init display device*/
    if(ali_app_dis_init(init_para.init_para_dis,&s_p_hdls->dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_live;
    }
    AUI_PRINTF("AUI DIS opened\n");
#endif
	// Rivaille.Zhu: change nim_connect location due to masoic issue
	// when live stream playing starts to execute in aui sample test.
	if (nim_connect(&init_para.init_para_nim,&s_p_hdls->nim_hdl)) {
		AUI_PRINTF("\nnim connect error\n");
		goto err_live;
	}
	AUI_PRINTF("nim connect success\n");

	if (ali_app_tsi_init(&init_para.init_para_tsi, &s_p_hdls->tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI TSI opened\n");

	/*init decv device */
	if (ali_app_decv_init(init_para.init_para_decv,&s_p_hdls->decv_hdl)) {
		AUI_PRINTF("\r\n ali_app_decv_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI DECV opened\n");
	
	/*init deva device*/
	if (ali_app_audio_init(init_para.init_para_audio,&s_p_hdls->deca_hdl,&s_p_hdls->snd_hdl)) {
		AUI_PRINTF("\r\n ali_app_audio_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI audio opened[%08x]\n",(int)s_p_hdls->deca_hdl);

	/*set video and audio  synchronous ways,signal from nim,set to AUI_STC_AVSYNC_PCR.
	 *signal from tsg,set to AUI_STC_AVSYNC_AUDIO*/
	if(aui_decv_sync_mode(s_p_hdls->decv_hdl,AUI_STC_AVSYNC_PCR)){
		AUI_PRINTF("Set AUI_STC_AVSYNC_PCR fail\n");
		goto err_live;
	}
	/* In TDS, create av streams should be done after audio and video init
	   because DMX needs the right format for audio and video
	*/
	/*set vpid,apid,pcr and create av stream*/
	if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id,
						init_para.dmx_create_av_para.video_pid,
						init_para.dmx_create_av_para.audio_pid,
					 	init_para.dmx_create_av_para.audio_desc_pid,
					 	init_para.dmx_create_av_para.pcr_pid,
					 	&s_p_hdls->dmx_hdl)) {
		AUI_PRINTF("\r\n set dmx failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI DMX opened[%08x]\n",(int)s_p_hdls->dmx_hdl);

	aui_dmx_data_path path;
	MEMSET(&path, 0, sizeof(aui_dmx_data_path));
	path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
	if (aui_dmx_data_path_set(s_p_hdls->dmx_hdl, &path)) {
		AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
		goto err_live;
	}
	AUI_PRINTF("dmx data path set %d\n", path.data_path_type);

    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_live;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_live;
        }
    }
    else{
        if (aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL)){
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_live;
        }
    }

    if (nim_get_signal_info(&init_para.init_para_nim)) {
        AUI_PRINTF("nim_get_signal_info failed\n");
        goto err_live;
    }
    
	return AUI_RTN_SUCCESS;

err_live:
	ali_app_deinit(s_p_hdls);
	return AUI_RTN_FAIL;
}

unsigned long test_aui_av_live_stop_clear_stream(unsigned long *argc,char **argv,char *sz_out_put)
{
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	if(NULL==s_p_hdls)
	{
		return AUI_RTN_FAIL;
	}
	ali_app_deinit(s_p_hdls);
	FREE(s_p_hdls);
	s_p_hdls=NULL;
	static_ad_pid = AUI_INVALID_PID;
	return AUI_RTN_SUCCESS;
}

unsigned long test_aui_change_audio_track(unsigned long *argc,char **argv,char *sz_out_put)
{
	*sz_out_put = 0;
	aui_dmx_stream_pid pid_list;
	memset(&pid_list, 0, sizeof(aui_dmx_stream_pid));
	unsigned short audio_pid=AUI_INVALID_PID, ad_pid=AUI_INVALID_PID;
	unsigned long audio_type;
    if (*argc == 2)
    {
        audio_pid = ATOI(argv[0]);
        audio_type = ATOI(argv[1]);
    }
    else if (*argc == 3)
    {	
    	audio_pid = ATOI(argv[0]);
        audio_type = ATOI(argv[1]);
        ad_pid = ATOI(argv[2]);
    }
    else
    {
        AUI_PRINTF("cmd_num [audio pid],[audio type],[audio description pid]\n");
        return AUI_RTN_FAIL;
    }
	if(NULL==s_p_hdls)
	{
		return AUI_RTN_FAIL;
	}
	aui_deca_set(s_p_hdls->deca_hdl, AUI_DECA_PREPARE_CHANGE_AUD_TRACK, NULL);
    aui_deca_stop(s_p_hdls->deca_hdl, NULL);

	pid_list.ul_pid_cnt=4;
	pid_list.stream_types[0]=AUI_DMX_STREAM_LAST;
	pid_list.stream_types[1]=AUI_DMX_STREAM_AUDIO;
	pid_list.stream_types[2]=AUI_DMX_STREAM_LAST;
	pid_list.stream_types[3]=AUI_DMX_STREAM_AUDIO_DESC;
	pid_list.aus_pids_val[0]=0;

    pid_list.aus_pids_val[2]=0;
    pid_list.aus_pids_val[3]=ad_pid;

    pid_list.aus_pids_val[1]=audio_pid;
    aui_deca_type_set(s_p_hdls->deca_hdl, audio_type);
    aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_SET_CHANGE_AUD_STREM, (void *)&pid_list);
    aui_deca_start(s_p_hdls->deca_hdl, NULL);

	return AUI_RTN_SUCCESS;
}

void get_audio_description_pid(unsigned short *ad_pid) {
	if (!ad_pid)
		return;
	*ad_pid = static_ad_pid;
}

unsigned long test_aui_cfg_ad_pid(unsigned long *argc,char **argv,char *sz_out_put)
{
	(void) sz_out_put;
    if (*argc == 1)
    {
        static_ad_pid = ATOI(argv[0])&AUI_INVALID_PID;
    }
    else
    {
        AUI_PRINTF("ad_set [audio description pid]\n");
        return AUI_RTN_FAIL;
    }
	return AUI_RTN_SUCCESS;
}

unsigned long test_aui_live_play_mode_set(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
    if (*argc != 1)
    {
        AUI_PRINTF("usage: play [play mode]\n");
        AUI_PRINTF("            [play mode] 0: have sound and video picture when playing stream\n");
        AUI_PRINTF("                        1: have sound and no video picture when playing stream\n");
        AUI_PRINTF("                        2: have video picture and no sound when playing stream\n");
        AUI_PRINTF("       for example: modeset 0\n");
        AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
        return AUI_RTN_FAIL;
    }
    s_play_mode =ATOI(argv[0]);
    return AUI_RTN_SUCCESS;
}


