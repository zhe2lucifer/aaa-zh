/****************************INCLUDE HEAD FILE************************************/
#ifndef AUI_LINUX
#include <api/libc/string.h>
#include <api/libc/printf.h>
#endif
#include <aui_dmx.h>
#include "aui_pvr_test.h"

aui_hdl ali_qt_recorder;
aui_hdl ali_qt_player;
int qt_test_timeshift_flag;

static void test_aui_qt_board_test_pvr_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data)
{
	//AUI_PRINTF("%p, %u, %u, %p\n",  handle, msg_type, msg_code, user_data);
	(void) handle;
	(void) msg_type;
	(void) msg_code;
	(void) user_data;
	return ;
}

/*
Use dmx1 to record for QT board test.
*/
unsigned long qt_board_test_pvr_test_record(unsigned long *argc,char **argv,char *sz_out_put)
{
	int ret = 0;
	unsigned int pos = 0;
	unsigned int i = 0;
	unsigned char *filename=NULL;
	unsigned int vcount = 0;
	unsigned int vpid = 0x1fff;
	unsigned int vtype = 0;
	unsigned int acount = 0;
	unsigned int apids[10] = {0x1fff,};
	unsigned int atypes[10] = {0,};
	unsigned int pcr_pid = 0;
	unsigned int is_reencrypt = 0;
	
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	
	if(*argc <7){
		AUI_PRINTF("\r\n record command format:qt_record filename,video_count,video_pid,video_type,audio_count,audio_pid0,audio_type0,audio_pid1,audio_type1,...,pcr_pid,ecnrypt_mode.\r\n");
		AUI_PRINTF("\r\n For example:qt_record pvr_001,1,513,0,1,660,0,8190,0.\r\n");
		AUI_PRINTF("\r\n video_type:\r\n0:MPEG2;\r\n 1:H264;\r\n2:ACS;\r\n3:H265\r\n");
		AUI_PRINTF("\r\n encrypt mode:\r\n0:FTA;\r\n 1:FTA to re-ecrypt;\r\n2:for conax 6;\r\n3:for nagura\r\n4:for raw ts record\r\n5:for gen ca ts record\r\n");
		return 1;
	}

	filename = argv[0];
	AUI_PRINTF("\r\n filename:%s\r\n",filename);
	//get video info
	vcount = ATOI(argv[1]);
	AUI_PRINTF("\r\n vcount:%d\r\n",vcount);
	if(vcount == 0) {
		AUI_PRINTF("\r\n Record no video!\r\n");
		pos = 2;
	}
	else if(vcount == 1) {
		vpid = ATOI(argv[2]);
		vtype = ATOI(argv[3]);
		AUI_PRINTF("\r\n vpid:%d vpid:%d\r\n",vpid,vtype);
		pos = 4;
	}
	else {
		AUI_PRINTF("\r\n Video count is error!\r\n");
		return 1;
	}
	//get audio info
	acount = ATOI(argv[pos++]);
	if(*argc != 5 + 2 * vcount + 2 * acount ) {
		AUI_PRINTF("\r\n the video count & auidio count error \r\n");
		return 1;
	}

	AUI_PRINTF("\r\n acount : %d \r\n",acount);
	if((acount <= 0)|| (AUI_MAX_PVR_AUDIO_PID <= acount)) {
		AUI_PRINTF("\r\n audio count is error!\r\n");
		return 1;
	}
	else {
		for(i = 0; i < acount; i++) {
			apids[i] = ATOI(argv[pos++]);
			atypes[i] = ATOI(argv[pos++]);
			AUI_PRINTF("\r\n apids[%d] : %d \r\n",i,apids[i]);
			AUI_PRINTF("\r\n atypes[%d] : %d \r\n",i,atypes[i]);
		}
	}
	//get pcr info
	pcr_pid = ATOI(argv[pos++]);
	//get reencrypt info
	is_reencrypt = ATOI(argv[pos]);
    AUI_PRINTF("\r\n ensure TSI route have config to dmx1 \r\n");
	AUI_PRINTF("\r\n pcr_pid : %d \r\n",pcr_pid);
	AUI_PRINTF("\r\n is_reencrypt : %d \r\n",is_reencrypt);

	AUI_PRINTF("********************pvr record start********************************\n");
	aui_hdl aui_pvr_handler=NULL;
	AUI_PRINTF("start recording....\n");
	if(0 != ali_pvr_record_open(&aui_pvr_handler, AUI_DMX_ID_DEMUX1 /* dmx_id */,vpid /* video pid*/, vtype /* video type*/,
								acount /* audio count*/,apids /* audio pid*/,atypes /* audio type*/,pcr_pid /* pcr pid*/,
								AUI_REC_MODE_NORMAL /* rec mode*/,is_reencrypt /* is reencrypt*/,0 /* ca mode*/,filename /* file name*/))
	{
		AUI_PRINTF("ali_pvr_record_open failed\n");
		ret = 1;
		return ret;
	}
	AUI_PRINTF("************************PVR recod is configured %p****************************\n", aui_pvr_handler);
	return 0;
}

/*
Use dmx0 to timeshift for QT board test.
*/
unsigned long qt_board_test_pvr_test_timeshift(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_PRINTF("********************pvr timeshift start********************************\n");
	unsigned int pos = 0;
	unsigned int i = 0;
	unsigned char *filename=NULL;
	unsigned int vcount = 0;
	unsigned int vpid = 0x1fff;
	unsigned int vtype = 0;
	unsigned int acount = 0;
	unsigned int apids[10] = {0x1fff,};
	unsigned int atypes[10] = {0,};
	unsigned int pcr_pid = 0;
	unsigned int is_reencrypt = 0;
	unsigned int duarion =0;
	unsigned long rec_time = 0;

	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);

	if(*argc <7){
		AUI_PRINTF("\r\n timeshift command format:qt_timeshift filename,video_count,video_pid,video_type,audio_count,audio_pid0,audio_type0,audio_pid1,audio_type1,...,pcr_pid,ecnrypt_mode.\r\n");
		AUI_PRINTF("\r\n For example:qt_timeshift pvr_001,1,513,0,1,660,0,8190,0.\r\n");
		AUI_PRINTF("\r\n video_type:\r\n0:MPEG2;\r\n 1:H264;\r\n2:ACS;\r\n3:H265\r\n");
		AUI_PRINTF("\r\n encrypt mode:\r\n0:FTA;\r\n 1:FTA to re-ecrypt;\r\n2:for conax 6;\r\n3:for nagura\r\n4:for raw ts record\r\n");
		return 1;
	}

	filename = argv[0];
	//get video info
	vcount = ATOI(argv[1]);

	if(vcount == 0) {
		AUI_PRINTF("\r\n Record no video!\r\n");
		pos = 2;
	}
	else if(vcount == 1) {
		vpid = ATOI(argv[2]);
		vtype = ATOI(argv[3]);
		AUI_PRINTF("\r\n vpid:%d vpid:%d\r\n",vpid,vtype);
		pos = 4;
	}
	else {
		AUI_PRINTF("\r\n Video count is error!\r\n");
		return 1;
	}
	//get audio info
	acount = ATOI(argv[pos++]);
	AUI_TEST_CHECK_VAL(*argc,5 + 2 * vcount + 2 * acount);

	if(acount <= 0) {
		AUI_PRINTF("\r\n audio count is error!\r\n");
		return 1;
	}
	else {
		for(i = 0; i < acount; i++) {
			apids[i] = ATOI(argv[pos++]);
			atypes[i] = ATOI(argv[pos++]);
		}
	}
	//get pcr info
	pcr_pid = ATOI(argv[pos++]);
	//get reencrypt info
	is_reencrypt = ATOI(argv[pos]);

    AUI_PRINTF("ensure TSI route have config to dmx1\n");
	AUI_PRINTF("input the duration between record and playback: (>5s)\n");
	aui_test_get_user_dec_input(&rec_time);
	AUI_PRINTF("start recording....\n");
	
	if(0 != ali_pvr_record_open(&ali_qt_recorder,AUI_DMX_ID_DEMUX0 /* dmx_id */,vpid /* video pid*/, vtype /* video type*/, 
								acount /* audio count*/,apids /* audio pid*/,atypes /* audio type*/,pcr_pid /* pcr pid*/,
								AUI_REC_MODE_TMS /* rec mode*/,is_reencrypt /* is reencrypt*/,0 /* ca mode*/,filename /* file name*/))
	{
		AUI_PRINTF("ali_pvr_record_open failed\n");
		ali_qt_recorder = NULL;
		goto exit;
	}
	AUI_PRINTF("************************PVR timeshift is configured %p****************************\n", ali_qt_recorder);
	AUI_PRINTF("************************PVR recorder start****************************\n");
	//For low bitrate stream,if the start play time is too fast maybe no enough data to send to dmx, 
	//and may case block for a while,to avoid this can set a longer start play time.
	while(duarion < rec_time){
		aui_pvr_get(ali_qt_recorder,AUI_PVR_REC_TIME_S,&duarion,0,0);
		AUI_PRINTF("************************PVR recorder duarion[%d]****************************\n",duarion);
		AUI_SLEEP(1*1000);
	}

	ali_stop_live_play();
	AUI_PRINTF("************************PVR player playback****************************\n");
	if(0 != ali_pvr_play_open(&ali_qt_player,1,NULL,AUI_P_OPEN_FROM_HEAD|AUI_PVR_PB_STOP_MODE))	{
		AUI_PRINTF("ali_pvr_play_open failed\n");
		ali_qt_player = NULL;
		goto exit;
	}else {
		AUI_PRINTF("qt_test_timeshift_flag set 1,ali_qt_recorder :0x%x,ali_qt_player: 0x%x\n",(unsigned int)ali_qt_recorder,(unsigned int)ali_qt_player);
		qt_test_timeshift_flag = 1;
		return 0;
	}
	//should never run here.
exit:
	if(ali_qt_player!=NULL){
		ali_pvr_play_close(ali_qt_player);
	}
	if(ali_qt_recorder!=NULL){
		ali_pvr_record_close(&ali_qt_recorder);
	}
	ali_recover_live_play(vpid,apids[0],vtype,atypes[0],pcr_pid);
	return -1;
}

