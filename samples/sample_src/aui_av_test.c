#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/select.h>
#else
#include <api/libfs2/stdio.h>
#endif
//#include "unity_fixture.h"
#include "aui_help_print.h"
#include "aui_common.h"
#include "aui_av_test.h"
#include "aui_nim.h"
#include <aui_dmx.h>
#include <aui_tsg.h>
#include <aui_tsi.h>
#include <aui_dis.h>
#include <aui_av.h>
#include "aui_nim_init.h"
#include "aui_test_stream.h"



struct av_inject_ts_audio_codec
{
	unsigned char *command;
	enum aui_deca_stream_type ts_audio_codec;
};


/****************************EXTERN VAR*******************************************/

//extern aui_hdl g_p_handle_player;

//extern aui_hdl g_p_handle_player_avc;



/****************************EXTERN FUNC *****************************************/

//extern unsigned long get_num_by_string(char *str);

/****************************TEST MODULE IMPLEMENT********************************/
void *av_handle = NULL;
//extern void *av_handle;
static struct ali_aui_hdls *s_av_hdls=NULL;

static unsigned long get_num_by_string(char *str)
{
	unsigned long val = 0;
	/*

	sscanf(str,"%s",temp);

	if(AUI_RTN_SUCCESS!=aui_common_str2num(temp,strlen(temp),&val))

	{

		return -1;

	}

	 */

	aui_test_get_user_hex_input(&val);

	return val;
}

unsigned long test_av_init(unsigned long *argc,char **argv,char *sz_out_put)
{
	return 0;
}

static void show_av_usage() 
{
    AUI_PRINTF("cmd_num [<nim id>]\n");
    AUI_PRINTF("if you want to changed the default configure,as fallow\n");
    AUI_PRINTF("arguments for DVBS and DVBC:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[symb],[vpid],[apid],[ppid],[video format],[audio format],[dis_format]\n");
    AUI_PRINTF("such as CCTV2,input:1 1,0,3840,27500,513,660,8190,0,1,0\n");
    AUI_PRINTF("        CCTV7,input:1 1,0,3840,27500,514,670,8190,0,1,0\n");
    AUI_PRINTF("arguments for DVBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT type]\n");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

static int test_av_prepare_nim_tsi(struct ali_app_modules_init_para *init_para, unsigned long *argc, char **argv)
{
	/*assert arguement*/
	if (0 == *argc) {
		show_av_usage();
		return -1;
	}
	s_av_hdls = (struct ali_aui_hdls *)MALLOC(sizeof(struct ali_aui_hdls));
	if(NULL == s_av_hdls) 
		return -1;
	MEMSET(s_av_hdls, 0, sizeof(struct ali_aui_hdls));
	/*init all used device in this mode*/
	ali_aui_init_para_for_test_nim(argc,argv,init_para);

#ifdef AUI_LINUX
    /*nim_init_cb is callback function point,to init nim device about special board
     * for example M3733,M3515B*/
    if (aui_nim_init(nim_init_cb)) {
        AUI_PRINTF("\nnim init error\n");
        goto err_live;
    }
    AUI_PRINTF("AUI NIM opened\n");/* Start streaming */

    /*open and init display device*/
    if(ali_app_dis_init(init_para->init_para_dis, &s_av_hdls->dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_live;
    }
    AUI_PRINTF("AUI DIS opened\n");
#endif

	if (ali_app_tsi_init(&init_para->init_para_tsi, &s_av_hdls->tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI TSI opened\n");
		
	if (nim_connect(&init_para->init_para_nim,&s_av_hdls->nim_hdl)) {
		AUI_PRINTF("\nnim connect error\n");
		goto err_live;
	}
	AUI_PRINTF("nim connect success\n");

	return AUI_RTN_SUCCESS;
	
err_live:
	ali_app_deinit(s_av_hdls);
	return AUI_RTN_SUCCESS;
}

unsigned long test_av_open(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attrAV attr_av;
	struct ali_app_modules_init_para init_para;
	MEMSET(&init_para, 0, sizeof(struct ali_app_modules_init_para));
	MEMSET(&attr_av, 0, sizeof(aui_attrAV));
	
	AUI_TEST_CHECK_RTN(test_av_prepare_nim_tsi(&init_para, argc, argv));	
	
	if (init_para.dmx_create_av_para.audio_pid != 0x1FFF)
		attr_av.st_av_info.b_audio_enable = 1;
	attr_av.st_av_info.b_dmx_enable = 1;
	if (init_para.dmx_create_av_para.pcr_pid != 0x1FFF)
		attr_av.st_av_info.b_pcr_enable = 1;
	if (init_para.dmx_create_av_para.video_pid != 0x1FFF)
		attr_av.st_av_info.b_video_enable = 1;
		
	attr_av.st_av_info.en_audio_stream_type = init_para.init_para_audio.ul_audio_type;
	attr_av.st_av_info.en_spdif_type = AUI_SND_OUT_MODE_DECODED;
	attr_av.st_av_info.en_video_stream_type = init_para.init_para_decv.ul_video_type;
	attr_av.st_av_info.ui_audio_pid = init_para.dmx_create_av_para.audio_pid;
	attr_av.st_av_info.ui_video_pid = init_para.dmx_create_av_para.video_pid;
	attr_av.st_av_info.ui_pcr_pid = init_para.dmx_create_av_para.pcr_pid;

	if(aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &attr_av.pv_hdl_deca))
	{
		aui_attr_deca attr_deca;
		MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
		AUI_TEST_CHECK_RTN(aui_deca_open(&attr_deca, &attr_av.pv_hdl_deca));
		s_av_hdls->deca_hdl = attr_av.pv_hdl_deca;
	}

	if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &attr_av.pv_hdl_snd))
	{
		aui_attr_snd attr_snd;
		MEMSET(&attr_snd,0,sizeof(attr_snd));
		AUI_TEST_CHECK_RTN(aui_snd_open(&attr_snd, &attr_av.pv_hdl_snd));
		s_av_hdls->snd_hdl = attr_av.pv_hdl_snd;
	    aui_snd_mute_set(s_av_hdls->snd_hdl, 0);
    	aui_snd_vol_set(s_av_hdls->snd_hdl, init_para.init_para_audio.ul_volume);
	}

	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &attr_av.pv_hdl_decv))
	{
		aui_attr_decv attr_decv;
		MEMSET(&attr_decv,0,sizeof(aui_attr_decv));
		AUI_TEST_CHECK_RTN(aui_decv_open(&attr_decv, &attr_av.pv_hdl_decv));
		s_av_hdls->decv_hdl = attr_av.pv_hdl_decv;
	}

	if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &attr_av.pv_hdl_dmx))
	{
		aui_attr_dmx attr_dmx;
		MEMSET(&attr_dmx, 0, sizeof(attr_dmx));
		AUI_TEST_CHECK_RTN(aui_dmx_open(&attr_dmx, &attr_av.pv_hdl_dmx));
		s_av_hdls->dmx_hdl = attr_av.pv_hdl_dmx;
	}

	AUI_TEST_CHECK_RTN(aui_av_open(&attr_av, &av_handle));
	//AUI_PRINTF("[AV TEST] av handle:%08x\n",av_handle);
	//AUI_PRINTF("[AV TEST] dmx hdl:%08x,decv hdl:%08x,deca hdl:%08x,snd hdl:%08x\n",attr_av->pv_hdl_dmx, attr_av->pv_hdl_decv,attr_av->pv_hdl_deca,attr_av->pv_hdl_snd);
	AUI_PRINTF("av open success\n");
	return AUI_RTN_SUCCESS;
}

unsigned long test_av_start(unsigned long *argc,char **argv,char *sz_out_put)
{
#if 0
	HDI_DVB_DELIVERY tuner;
	tuner.type = 3;
	tuner.demodulator_flags = 0;
	tuner.delivery.satellite.frequency = 1310;
	tuner.delivery.satellite.symbol_rate = 27498;
	tuner.delivery.satellite.w_e_pol_mod_fec = 0;
	aui_attrAV attr_av;

	MEMSET(&attr_av, 0, sizeof(aui_attrAV));

	attr_av.stAVInfo.b_audio_enable = 1;
	attr_av.stAVInfo.b_dmx_enable = 1;
	attr_av.stAVInfo.b_pcr_enable = 1;
	attr_av.stAVInfo.b_video_enable = 1;
	attr_av.stAVInfo.en_audio_stream_type = AUDIO_MPEG2;
	attr_av.stAVInfo.en_spdif_type = SND_OUT_SPDIF_BS;
	attr_av.stAVInfo.en_video_stream_type = AUI_VIDEO_MPEG;
	attr_av.stAVInfo.ui_audio_pid = 700;
	attr_av.stAVInfo.ui_video_pid = 517;
	attr_av.stAVInfo.ui_pcr_pid = 8190;
#endif
#if 0
	AUI_TEST_CHECK_NULL(sz_out_put);
	MP_HDI_TUNER_init();
	MP_HDI_TUNER_events_callback(tunertest_callback);
	MP_HDI_TUNER_connect_index(0, &tuner);
	aui_av_open(&attr_av, &av_handle);
#endif
	AUI_PRINTF("enter av start test\n");
	//_nim_connect(1200,27500,2);
	//AUI_PRINTF("nim connected!\n");
	aui_hdl dmx_hdl;
	if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &dmx_hdl))
	{
		AUI_PRINTF("%s -> dmx_hdl can not be found\n",__func__);
		return -1;
	}
	//App must set data path  
	aui_dmx_data_path data_path_info;
	memset(&data_path_info, 0, sizeof(data_path_info));
	data_path_info.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
	AUI_TEST_CHECK_RTN(aui_dmx_data_path_set(dmx_hdl, &data_path_info));
	AUI_TEST_CHECK_RTN(aui_av_start(av_handle));
	return AUI_RTN_SUCCESS;
}



unsigned long test_av_stop(unsigned long *argc,char **argv,char *sz_out_put)
{
	//AUI_TEST_CHECK_RTN(aui_av_stop(av_handle));
	//AUI_TEST_CHECK_RTN(aui_av_stop(av_handle));
	AUI_PRINTF("enter av stop test\n");
	AUI_TEST_CHECK_RTN(aui_av_stop(av_handle));

	return AUI_RTN_SUCCESS;
}

unsigned long test_av_pause(unsigned long *argc,char **argv,char *sz_out_put)
{
	//AUI_TEST_CHECK_RTN(aui_av_pause(av_handle));
	AUI_PRINTF("enter av pause test, same as stop for live play\n");
	AUI_TEST_CHECK_RTN(aui_av_pause(av_handle));

	return AUI_RTN_SUCCESS;
}

unsigned long test_av_resume(unsigned long *argc,char **argv,char *sz_out_put)
{
	//AUI_TEST_CHECK_RTN(aui_av_resume(av_handle));
	AUI_PRINTF("enter av resume test\n");
	AUI_TEST_CHECK_RTN(aui_av_resume(av_handle));

	return AUI_RTN_SUCCESS;
}



unsigned long test_av_close(unsigned long *argc,char **argv,char *sz_out_put)
{
	//AUI_TEST_CHECK_RTN(aui_av_close(av_handle));
	if (s_av_hdls == NULL) {
		AUI_PRINTF("aui av not opened!\n");
		return AUI_RTN_SUCCESS;
	}
	AUI_TEST_CHECK_RTN(aui_av_close(av_handle));
	ali_app_deinit(s_av_hdls);
	FREE(s_av_hdls);
	s_av_hdls=NULL;
	return AUI_RTN_SUCCESS;
}



unsigned long test_video_pid_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,1);
	unsigned long val = get_num_by_string(argv[0]);
	AUI_TEST_CHECK_RTN(aui_av_set(av_handle,AUI_AV_VIDEO_PID_SET,&val));

	return AUI_RTN_SUCCESS;
}

unsigned long test_audio_pid_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,1);
	unsigned long val = get_num_by_string(argv[0]);
	AUI_TEST_CHECK_RTN(aui_av_set(av_handle,AUI_AV_AUDIO_PID_SET,&val));

	return AUI_RTN_SUCCESS;
}

unsigned long test_pcr_pid_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,1);
	unsigned long val = get_num_by_string(argv[0]);
	AUI_TEST_CHECK_RTN(aui_av_set(av_handle,AUI_AV_PCR_PID_SET,&val));

	return AUI_RTN_SUCCESS;

}

unsigned long test_av_set(unsigned long *argc,char **argv,char *sz_out_put)

{
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,2);
	unsigned long val0 = get_num_by_string(argv[0]);
	unsigned long val1 = get_num_by_string(argv[1]);
	AUI_TEST_CHECK_RTN(aui_av_set(av_handle,val0,&val1));

	return AUI_RTN_SUCCESS;

}

unsigned long test_av_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	//AUI_TEST_CHECK_NULL(argc);
	//AUI_TEST_CHECK_NULL(argv);
	//AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,1);
	unsigned long val = 0;
	unsigned long val0 = get_num_by_string(argv[0]);
	AUI_TEST_CHECK_RTN(aui_av_get(av_handle,val0,&val));
	AUI_PRINTF("\nval get:%d\n",val);

	return AUI_RTN_SUCCESS;
}

unsigned long test_av_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nAV Test Help");

	/* AV_1_HELP */
#define 	AV_1_HELP		"The AV module test process for Live play: open -> start -> pause -> resume -> close"	
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Open the AV module\n");
	aui_print_help_instruction_newline(AV_1_HELP);
	test_stream_with_nim_help("1");
	/* AV_2_HELP */
#define 	AV_2_HELP		"The operation of test is making AV resume actually. The test is same to the \'5\' command."
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("Start running of AV module\n");
	aui_print_help_instruction_newline(AV_2_HELP);

	/* AV_3_HELP */
#define 	AV_3_HELP 		"The operation of test is making AV pause actually. The test is same to the \'4\' command."
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline("Stop running of AV module\n");
	aui_print_help_instruction_newline(AV_3_HELP);

	/* AV_4_HELP */
#define 	AV_4_HELP		"Pause AV modual is meaning that the DECA and DECV moduals are stopped"
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline("Pause running of AV module\n");
	aui_print_help_instruction_newline(AV_4_HELP);

	/* AV_5_HELP */
#define 	AV_5_HELP		"Resume AV modual is meaning that the DECA and DECV moduals re-start working"
	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline("Resume running of AV module\n");
	aui_print_help_instruction_newline(AV_5_HELP);
	
	aui_print_help_command("\'6\'");
	aui_print_help_instruction_newline("Close the AV module\n");
#ifndef AUI_TDS
#define 	AV_7_HELP_PART1		"The AV module test for local play\n"
#define     AV_7_HELP_PART2     "fomart    7  [Audio PID],[Video PID],[PCR PID],[Auido type],[Video type],[Demux id],[Ts path],[Dynamic resolution],[Sleep ms],[Audio pid],[Audio type]..."
#define     AV_7_HELP_PART3     "             [Audio PID]: audio PID, as the audio track index 0 if there are more than one audio track in the stream"
#define     AV_7_HELP_PART4     "             [Video PID]: video PID"
#define     AV_7_HELP_PART5     "             [PCR PID]: PCR PID"
#define     AV_7_HELP_PART6     "             [Auido type]:   MPEG1: AUI_DECA_STREAM_TYPE_MPEG1"
#define     AV_7_HELP_PART7     "                             MPEG2: AUI_DECA_STREAM_TYPE_MPEG2"
#define     AV_7_HELP_PART8     "                               AAC: AUI_DECA_STREAM_TYPE_AAC_LATM"
#define     AV_7_HELP_PART9     "                               AC3: AUI_DECA_STREAM_TYPE_AC3"
#define     AV_7_HELP_PART10    "                               DTS: AUI_DECA_STREAM_TYPE_DTS"
#define     AV_7_HELP_PART11    "                              PPCM: AUI_DECA_STREAM_TYPE_PPCM"
#define     AV_7_HELP_PART12    "                            LPCM_V: AUI_DECA_STREAM_TYPE_LPCM_V"
#define     AV_7_HELP_PART13    "                            LPCM_A: AUI_DECA_STREAM_TYPE_LPCM_A"
#define     AV_7_HELP_PART14    "                              BYE1: AUI_DECA_STREAM_TYPE_BYE1"
#define     AV_7_HELP_PART15    "                               RA8: AUI_DECA_STREAM_TYPE_RA8"
#define     AV_7_HELP_PART16    "                               MP3: AUI_DECA_STREAM_TYPE_MP3"
#define     AV_7_HELP_PART17    "                          ADTS_AAC: AUI_DECA_STREAM_TYPE_AAC_ADTS"
#define     AV_7_HELP_PART18    "                               OGG: AUI_DECA_STREAM_TYPE_OGG"
#define     AV_7_HELP_PART19    "                               EC3: AUI_DECA_STREAM_TYPE_EC3"
#define     AV_7_HELP_PART20    "                            MP3_L3: AUI_DECA_STREAM_TYPE_MP3_L3"
#define     AV_7_HELP_PART21    "                               PCM: AUI_DECA_STREAM_TYPE_RAW_PCM"
#define     AV_7_HELP_PART22    "             [Video type]: 0:AUI_DECV_FORMAT_MPEG"
#define     AV_7_HELP_PART23    "                           1:AUI_DECV_FORMAT_AVC"
#define     AV_7_HELP_PART24    "                           10:AUI_DECV_FORMAT_HEVC"
#define     AV_7_HELP_PART25    "             [Demux id]: 0:AUI_DMX_ID_DEMUX0"
#define     AV_7_HELP_PART26    "                         1:AUI_DMX_ID_DEMUX1"
#define     AV_7_HELP_PART27    "                         2:AUI_DMX_ID_SW_DEMUX0"
#define     AV_7_HELP_PART28    "             [Ts path]: TS file path"
#define     AV_7_HELP_PART29    "             [Dynamic resolution]: set to 1 when the resolution of stream is dynamic, default: 0"
#define     AV_7_HELP_PART30    "             [Sleep ms]: sleep ms after writing data, optional for changing audio track, default: 0"
#define     AV_7_HELP_PART31    "             [Audio PID]: optional for changing audio track, as the audio track index 1(2~4)"
#define     AV_7_HELP_PART32    "             [Audio type]: optional for changing audio track, as the audio track index 1(2~4)"
#define     AV_7_HELP_PART33    "Example:" 
#define     AV_7_HELP_PART34    "Normal stream: 7 105,101,101,MPEG2,1,2,/mnt/usb/sda1/xxx.ts"
#define     AV_7_HELP_PART35    "Dynamic resolution stream: 7 105,101,101,MPEG2,1,2,/mnt/usb/sda1/xxx.ts,1"
#define     AV_7_HELP_PART36    "stream with multi audio tracks: 7 105,101,101,MPEG2,1,2,/mnt/usb/sda1/xxx.ts,0,150,117,AC3,147,AC3"

	aui_print_help_command("\'7\'");
	aui_print_help_instruction_newline(AV_7_HELP_PART1);
	aui_print_help_instruction_newline(AV_7_HELP_PART2);
	aui_print_help_instruction_newline(AV_7_HELP_PART3);
	aui_print_help_instruction_newline(AV_7_HELP_PART4);
	aui_print_help_instruction_newline(AV_7_HELP_PART5);
	aui_print_help_instruction_newline(AV_7_HELP_PART6);
	aui_print_help_instruction_newline(AV_7_HELP_PART7);
	aui_print_help_instruction_newline(AV_7_HELP_PART8);
	aui_print_help_instruction_newline(AV_7_HELP_PART9);
	aui_print_help_instruction_newline(AV_7_HELP_PART10);
	aui_print_help_instruction_newline(AV_7_HELP_PART11);
	aui_print_help_instruction_newline(AV_7_HELP_PART12);
	aui_print_help_instruction_newline(AV_7_HELP_PART13);
	aui_print_help_instruction_newline(AV_7_HELP_PART14);
	aui_print_help_instruction_newline(AV_7_HELP_PART15);
	aui_print_help_instruction_newline(AV_7_HELP_PART16);
	aui_print_help_instruction_newline(AV_7_HELP_PART17);
	aui_print_help_instruction_newline(AV_7_HELP_PART18);
	aui_print_help_instruction_newline(AV_7_HELP_PART19);
	aui_print_help_instruction_newline(AV_7_HELP_PART20);
	aui_print_help_instruction_newline(AV_7_HELP_PART21);
	aui_print_help_instruction_newline(AV_7_HELP_PART22);
	aui_print_help_instruction_newline(AV_7_HELP_PART23);
	aui_print_help_instruction_newline(AV_7_HELP_PART24);
	aui_print_help_instruction_newline(AV_7_HELP_PART25);
	aui_print_help_instruction_newline(AV_7_HELP_PART26);
	aui_print_help_instruction_newline(AV_7_HELP_PART27);
	aui_print_help_instruction_newline(AV_7_HELP_PART28);
	aui_print_help_instruction_newline(AV_7_HELP_PART29);
	aui_print_help_instruction_newline(AV_7_HELP_PART30);
	aui_print_help_instruction_newline(AV_7_HELP_PART31);
	aui_print_help_instruction_newline(AV_7_HELP_PART32);
	aui_print_help_instruction_newline(AV_7_HELP_PART33);
	aui_print_help_instruction_newline(AV_7_HELP_PART34);
	aui_print_help_instruction_newline(AV_7_HELP_PART35);	
	aui_print_help_instruction_newline(AV_7_HELP_PART36);
#endif
	return AUI_RTN_HELP;
}

#ifndef AUI_TDS

void ts_playback_usages(void)
{
	printf("usage:[Test Items] [Audio PID],[Video PID],[PCR PID],[Auido type],[Video type],[Demux id],[Ts path],[Dynamic resolution],[Sleep ms],[Audio pid],[Audio type]...\n");
	printf("    Test Items:  7: ts playback and do avsync\n");
	printf("    Audio PID:  [Audio PID]:audio track index 0\n");
	printf("    Video PID:  [Video PID]\n");
	printf("    PCR   PID:  [PCR PID]\n");
	printf("    Audio type: [MPEG1,MPEG2,AAC,AC3,DTS etc]\n");
	printf("    Video type: [0:AUI_DECV_FORMAT_MPEG, 1:AUI_DECV_FORMAT_AVC], 10:AUI_DECV_FORMAT_HEVC\n");
	printf("    Demux id:   [0:AUI_DMX_ID_DEMUX0, 1:AUI_DMX_ID_DEMUX1, 2:AUI_DMX_ID_SW_DEMUX0]\n");
	printf("    File path:  [TS file path]\n");
	printf("    Dynamic resolution:  set to 1 when the resolution of stream is dynamic, default: 0\n");
	printf("    Sleep ms:   optional for changing audio track[sleep ms after writing data default: 0]\n");
	printf("    Audio PID:  optional,[Audio PID]:audio track index 1(2~4)\n");
	printf("    Audio type: optional,[MPEG1,MPEG2,AAC,AC3,DTS etc]\n");
	printf("    e.g. normal: 7 105,101,101,MPEG2,1,2,/mnt/usb/sda1/xxx.ts\n");
	printf("    e.g. dynamic resolution: 7 105,101,101,MPEG2,1,2,/mnt/usb/sda1/xxx.ts,1\n");
	printf("    e.g. change audio track: 7 105,101,101,MPEG2,1,2,/mnt/usb/sda1/xxx.ts,0,150,117,AC3,147,AC3");
}

struct av_inject_ts_audio_pid_types
{
	unsigned short audio_pid;
	enum aui_deca_stream_type audio_codec;
};

struct av_inject_ts_audio_codec ts_audio_codec_arry[] =
{
		/* various PCM "codecs" */
		{"MPEG1",   AUI_DECA_STREAM_TYPE_MPEG1},
		{"MPEG2",   AUI_DECA_STREAM_TYPE_MPEG2},
		{"AAC",     AUI_DECA_STREAM_TYPE_AAC_LATM},
		{"AC3",     AUI_DECA_STREAM_TYPE_AC3},
		{"DTS",     AUI_DECA_STREAM_TYPE_DTS},
		{"PPCM",    AUI_DECA_STREAM_TYPE_PPCM},
		{"LPCM_V",  AUI_DECA_STREAM_TYPE_LPCM_V},
		{"LPCM_A",  AUI_DECA_STREAM_TYPE_LPCM_A},
		{"BYE1",    AUI_DECA_STREAM_TYPE_BYE1},
		{"RA8",     AUI_DECA_STREAM_TYPE_RA8},
		{"MP3",     AUI_DECA_STREAM_TYPE_MP3},
		{"ADTS_AAC",    AUI_DECA_STREAM_TYPE_AAC_ADTS},
		{"OGG",     AUI_DECA_STREAM_TYPE_OGG},
		{"EC3",     AUI_DECA_STREAM_TYPE_EC3},
		{"MP3_L3",      AUI_DECA_STREAM_TYPE_MP3_L3},
		{"PCM",     AUI_DECA_STREAM_TYPE_RAW_PCM}
};

void aui_get_ts_audio_codec(char *src, enum aui_deca_stream_type *codec_id)
{
	unsigned int i = 0;

	for (i = 0; i < (sizeof(ts_audio_codec_arry)/sizeof(ts_audio_codec_arry[0])); i++)
	{
		if (strcmp(ts_audio_codec_arry[i].command, src) == 0)
		{
			*codec_id = ts_audio_codec_arry[i].ts_audio_codec;
			break;
		}
	}

	if(i >= (sizeof(ts_audio_codec_arry)/sizeof(ts_audio_codec_arry[0])))
	{
		printf("Cannot get match audio codec, use default one");
		*codec_id = AUI_DECA_STREAM_TYPE_MPEG1;
	}
	return ;
}

static char kbhit(void)
{

	struct timeval tv;
	fd_set read_fd;

	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);

	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
		return 0;

	if (FD_ISSET(0,&read_fd)){
		char c = getchar();
		if (c == 0xa)
			return 0;
		return c;
	}
	return 0;
}

static void test_av_ts_video_info_change_cb(void * p_user_hld, unsigned int parm1, unsigned parm2)
{
	struct aui_decv_info_cb *new_info = (struct aui_decv_info_cb *)parm1;
	switch (new_info->flag) {
    	case 0:
    	    printf("video info change width =%d,height =%d\n", new_info->pic_width,new_info->pic_height);
    	    break;
    	case 1:
    	    printf("video info change frame_rate =%d\n", new_info->frame_rate);
    	    break;
    	case 2:
    	    printf("video info change active_format =%d\n", new_info->active_format);
    	    break;
    	case 3:
    	    printf("video info change sar_width =%d, sar_height =%d\n", 
    	        new_info->sar_width, new_info->sar_height);
    	    break;
    	default:
    	    break;
	}
}

static void test_av_ts_playback_get_buffer_status(aui_attrAV *pst_attrAV)
{
	    unsigned long total_size = 0;
        unsigned long free_size = 0;
        //AUI_DMX_GET_TOTAL_BUF_LEN only for SW demux
        aui_dmx_get(pst_attrAV->pv_hdl_dmx, AUI_DMX_GET_TOTAL_BUF_LEN, (void *)&total_size);
        aui_dmx_get(pst_attrAV->pv_hdl_dmx, AUI_DMX_GET_FREE_BUF_LEN, (void *)&free_size);
        printf("dmx buf info: total:%lu, free: %lu\n", total_size, free_size);

        aui_deca_data_info deca_info;
        memset(&deca_info, 0, sizeof(aui_deca_data_info));
        aui_deca_get(pst_attrAV->pv_hdl_deca, AUI_DECA_DATA_INFO_GET, &deca_info);
        printf("audio buf info: total:%lu, used: %lu, remain: %lu\n", 
            deca_info.ul_buffer_total_size, deca_info.ul_buffer_valid_size, deca_info.ul_buffer_free_size);

    	aui_decv_info decv_info;
    	memset(&decv_info, 0, sizeof(aui_decv_info));
        aui_decv_get(pst_attrAV->pv_hdl_decv, AUI_DECV_GET_INFO, &decv_info);
        printf("video buf info: total:%lu, used: %lu\n", 
            decv_info.st_info.vbv_size, decv_info.st_info.valid_size);

        printf("video: h:%d, w:%d, a: %d\n", decv_info.st_info.sar_height, decv_info.st_info.sar_width, decv_info.st_info.active_format);
}

static void test_av_ts_playback_change_audio_track(aui_attrAV *pst_attrAV, 
    struct av_inject_ts_audio_pid_types *audio_track)
{
    aui_dmx_stream_pid pid_list;
	//change audio track
	AUI_PRINTF("switch to audio track: audio pid: %d, audio type: %d\n",
			audio_track->audio_pid, audio_track->audio_codec);
	aui_deca_set(pst_attrAV->pv_hdl_deca, AUI_DECA_PREPARE_CHANGE_AUD_TRACK, NULL);
	aui_deca_stop(pst_attrAV->pv_hdl_deca, NULL);
	
	memset(&pid_list, 0, sizeof(pid_list));
	pid_list.ul_pid_cnt=4;
	pid_list.stream_types[0]=AUI_DMX_STREAM_LAST;
	pid_list.stream_types[1]=AUI_DMX_STREAM_AUDIO;
	pid_list.stream_types[2]=AUI_DMX_STREAM_LAST;
	pid_list.stream_types[3]=AUI_DMX_STREAM_AUDIO_DESC;

	pid_list.aus_pids_val[0]=0;
	pid_list.aus_pids_val[1]=audio_track->audio_pid;
	pid_list.aus_pids_val[2]=0;
	pid_list.aus_pids_val[3]=0x1FFF;

	aui_deca_type_set(pst_attrAV->pv_hdl_deca, audio_track->audio_codec);
	aui_dmx_set(pst_attrAV->pv_hdl_dmx, AUI_DMX_SET_CHANGE_AUD_STREM, (void *)&pid_list);
	aui_deca_start(pst_attrAV->pv_hdl_deca, NULL);
}

/* PlayBack TS and inject data to Demux or TSG and playback*/
static unsigned long test_av_ts_playback(unsigned long *argc, char **argv, char *sz_out_put)
{
	aui_av_stream_info stream_info;
	enum aui_decv_format v_type;
	enum aui_deca_stream_type a_type;
	aui_dmx_id_t dmx_id;
	aui_attrAV pst_attrAV;
	aui_hdl av_hdl;
//	aui_av_inject_settings_t inject_set;
	aui_av_inject_packet_info_t packet_info;
	struct aui_decv_callback_node info_change_callback;
	aui_hdl snd_hdl;

	uint32_t key_press = -1;

	char filepath[256];
	const char* filename = NULL;
	FILE *f_ts_data;
	uint8_t *ts_data_buf = NULL;
	uint32_t ts_frm_size, ts_read_size;
	uint64_t ts_total_size;
	int sleep_ms = 0;
	struct av_inject_ts_audio_pid_types audio_track[5];
	int audio_track_index = 0;
	int pause = 0;
	unsigned char enable_dynamic_resolution = 0;
	aui_decv_info decv_info;
	aui_deca_data_info deca_info;
    unsigned int last_valid_size = 0;
    int i = 0;

	if(*argc < 7)
	{
		ts_playback_usages();
		return -1;
	}

	if (s_av_hdls){
		test_av_close(argc, argv, sz_out_put);
	}

	//Open ts data
	strcpy(filepath, argv[6]);
	filename = filepath;
	f_ts_data = fopen(filename, "rb");
	if(f_ts_data == NULL)
	{
		AUI_PRINTF("Open file %s fail\n", filename);
		return -1;
	}
	fseek(f_ts_data, 0, SEEK_END);
	ts_total_size = ftell(f_ts_data);
	if(ts_total_size <= 0)
	{
		AUI_PRINTF("Video data file size error %lld\n", ts_total_size);
		goto EXIT;
	}
	fseek(f_ts_data, 0, SEEK_SET);
	fflush(f_ts_data);
	AUI_PRINTF("Video data size %lld\n", ts_total_size);

	aui_get_ts_audio_codec(argv[3], &a_type);
	audio_track[0].audio_pid = strtoul(argv[0], 0, 0);
	audio_track[0].audio_codec = a_type;

	v_type = strtoul(argv[4], 0, 0);
	dmx_id = (strtoul(argv[5], 0, 0) == 0) ? AUI_DMX_ID_DEMUX0 : ((strtoul(argv[5], 0, 0) == 1) ? AUI_DMX_ID_DEMUX1 : AUI_DMX_ID_SW_DEMUX0);
	AUI_PRINTF("*argc: %d\n", *argc);
	if (*argc >= 8) {
	    enable_dynamic_resolution = strtoul(argv[7], 0, 0);
	    AUI_PRINTF("enable dynamic resolution: %d\n", enable_dynamic_resolution);
	}
	if (*argc >= 9) {
		sleep_ms = strtoul(argv[8], 0, 0);
		AUI_PRINTF("sleep_ms: %d\n", sleep_ms);
	}
	
	if (*argc >= 11 && *argc <= 17) {
		unsigned long i = 1;
		int index = 9;
		audio_track_index += (*argc - 9)/2;
		for (i = 1; i <= (*argc - 9)/2; i++){
			index = 9+(i-1)*2;
			audio_track[i].audio_pid = strtoul(argv[index], 0, 0);
			aui_get_ts_audio_codec(argv[index+1], &audio_track[i].audio_codec);
			AUI_PRINTF("\naudio %d: pid: %d, type: %d\n", i, audio_track[i].audio_pid, audio_track[i].audio_codec);
		}
	}
	MEMSET(&stream_info, 0, sizeof(aui_av_stream_info));

	stream_info.st_av_info.b_audio_enable = 1;
	stream_info.st_av_info.b_video_enable = 1;
	stream_info.st_av_info.b_pcr_enable = 1;
	stream_info.st_av_info.b_dmx_enable = 1;
	stream_info.st_av_info.ui_audio_pid = strtoul(argv[0], 0, 0);
	stream_info.st_av_info.ui_video_pid = strtoul(argv[1], 0, 0);
	stream_info.st_av_info.ui_pcr_pid   = strtoul(argv[2], 0, 0);
	stream_info.st_av_info.en_audio_stream_type = a_type;
	stream_info.st_av_info.en_video_stream_type = v_type;
	stream_info.st_av_info.en_spdif_type = AUI_SND_OUT_MODE_DECODED;
	stream_info.stream_type.dmx_id = dmx_id;
	stream_info.stream_type.data_type = AUI_AV_DATA_TYPE_RAM_TS;

	AUI_PRINTF("Audio pid = %d, Video pid = %d, Pcr pid = %d, a_type = %d, v_type = %d, dmx_id = %d\n", \
			stream_info.st_av_info.ui_audio_pid, stream_info.st_av_info.ui_video_pid, stream_info.st_av_info.ui_pcr_pid, a_type, v_type, dmx_id);

	//ts data buffer
	ts_data_buf = (uint8_t *)malloc(0x200000);
	if(ts_data_buf == NULL)
	{
		AUI_PRINTF("Malloc ts data buffer fail\n");
		goto EXIT;
	}

	aui_av_init_attr(&pst_attrAV, &stream_info);
	if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl)) {
		aui_attr_snd attr_snd;
		memset(&attr_snd,0,sizeof(aui_attr_snd));
		aui_snd_open(&attr_snd, &snd_hdl);
	}
	aui_hdl dmx_hdl;
	if (aui_find_dev_by_idx(AUI_MODULE_DMX, stream_info.stream_type.dmx_id, &dmx_hdl)) {
		AUI_PRINTF("%s-> can not find dmx hdl\n",__func__);
		goto EXIT;
	}
	//App must set data path  
	aui_dmx_data_path data_path_info;
	memset(&data_path_info, 0, sizeof(data_path_info));
	data_path_info.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
	aui_dmx_data_path_set(dmx_hdl, &data_path_info);
	
	aui_snd_mute_set(snd_hdl, 0);
	aui_snd_vol_set(snd_hdl, 50);
	aui_av_open(&pst_attrAV, &av_hdl);

	memset(&packet_info,0,sizeof(packet_info));
	memset(&info_change_callback,0,sizeof(struct aui_decv_callback_node));
	info_change_callback.callback = test_av_ts_video_info_change_cb;
	info_change_callback.type = AUI_DECV_CB_INFO_CHANGED;
	aui_decv_set(pst_attrAV.pv_hdl_decv, AUI_DECV_SET_REG_CALLBACK, &info_change_callback);    
    
  	if (enable_dynamic_resolution == 1)
        aui_decv_set(pst_attrAV.pv_hdl_decv, AUI_DECV_SET_VARIABLE_RESOLUTION, &enable_dynamic_resolution); 	    
	
	aui_av_start(av_hdl);
	//pst_attrAV.stream_type.data_type = AUI_AV_DATA_TYPE_RAM_TS;
	//Ready to send data to Demux
	ts_frm_size = 188;

	AUI_PRINTF("\n\n");
	AUI_PRINTF("                         Please input command.\n");
	AUI_PRINTF("                     q : Quit\n");
	AUI_PRINTF("                     p : Pause\n");
	AUI_PRINTF("                     r : Resume\n");
	AUI_PRINTF("                     b : Get buffer status\n");
	if (audio_track_index > 0) {
		AUI_PRINTF("                  0~%d : Change audio track\n", audio_track_index);
	}
	AUI_PRINTF("\n\n");

	while(ts_total_size > 0 )
	{
		key_press = kbhit();
		if (key_press) {
			if (key_press == 'q') {
				break;
			} else if (key_press == 'b') {
				AUI_PRINTF("Get buffer status\n");
                test_av_ts_playback_get_buffer_status(&pst_attrAV);
			} else if (key_press == 'p') {
				AUI_PRINTF("Pause\n");
				aui_av_pause(av_hdl);
				pause = 1;
				continue;
			} else if (key_press == 'r') {
				AUI_PRINTF("Resume\n");
				aui_av_resume(av_hdl);
				pause = 0;
				continue;
			} else {
				if (key_press >= '0' && key_press <= (uint32_t)('0' + audio_track_index)
						&& audio_track_index > 0){
					int i = key_press-'0';
					AUI_PRINTF("switch to %d audio track\n", i);
					test_av_ts_playback_change_audio_track(&pst_attrAV, &audio_track[i]);					
				}
			}
		}

		if (pause) {
			usleep(sleep_ms*1000);
			continue;
		}
		ts_read_size = fread(ts_data_buf, 1024, ts_frm_size, f_ts_data);
		//AUI_PRINTF("ts_read_size: %d, ts_total_size: %d\n", ts_read_size, ts_total_size);

		if(ts_read_size == ts_frm_size)
		{
			aui_av_write(av_hdl, &packet_info, ts_data_buf, ts_frm_size*1024);
			ts_total_size -= ts_read_size*1024;
			usleep(sleep_ms*1000);
		}
		else if( ts_read_size >0 && ts_read_size < ts_frm_size)
		{
			aui_av_write(av_hdl, &packet_info, ts_data_buf, ts_read_size*1024);
			ts_total_size -= ts_read_size*1024;
		}
		else {
            sleep(1);
            if (stream_info.st_av_info.ui_video_pid != 0x1FFF){
	            memset(&decv_info, 0, sizeof(decv_info));
	    		aui_decv_get(pst_attrAV.pv_hdl_decv, AUI_DECV_GET_INFO, &decv_info);
	            AUI_PRINTF("video wait %ld\n", decv_info.st_info.valid_size);
	            if (decv_info.st_info.valid_size == last_valid_size){
	                if(i++ > 2)
	                    break;
	            } else {
	                i = 0;
	            }
	            last_valid_size = decv_info.st_info.valid_size;
            } else if (stream_info.st_av_info.ui_audio_pid != 0x1FFF){
            	memset(&deca_info, 0, sizeof(deca_info));
            	aui_deca_get(pst_attrAV.pv_hdl_deca, AUI_DECA_DATA_INFO_GET, &deca_info);
            	AUI_PRINTF("audio wait %ld\n", deca_info.ul_buffer_valid_size);
            	if (deca_info.ul_buffer_valid_size == last_valid_size){
            		if (i++ > 2)
            			break;           			
            	} else {
            		i = 0;
            	}
            	last_valid_size = deca_info.ul_buffer_valid_size;
            } else {
            	AUI_PRINTF("Please check the video pid and audio pid!\n");
            	break;
            }
		}
	}
	
	aui_av_stop(av_hdl);
	aui_av_close(av_hdl);
	EXIT:
	if (ts_data_buf)
		free(ts_data_buf);
	fclose(f_ts_data);

	return 0;
}
#endif

void aui_load_tu_av()
{
	aui_tu_reg_group("av", "mod test help info");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_UNIT, test_av_open, "finish av init, now you can test av");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_SYS, test_av_start, "av start");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_SYS, test_av_pause, "av pause");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_SYS, test_av_resume, "av resume");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_SYS, test_av_stop, "av stop");
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_SYS, test_av_close, "av close");

#ifndef AUI_TDS
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_av_ts_playback, "ts play avsync pause/resume");
#endif
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_SYS, test_av_help, "AV help");
}

