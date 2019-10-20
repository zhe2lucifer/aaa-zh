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

#include <getopt.h>
#include <termios.h>

#include <aui_dmx.h>
#include <aui_tsg.h>
#include <aui_tsi.h>
#include <aui_dis.h>
#include <aui_av.h>
#include <aui_dsc.h>

//#define WB_CHG_KEY

#ifndef WB_CHG_KEY
#define FILE_NAME "/mnt/usb/sda1/xxx359.ts"
#define DSC_NUM 1
int blk_cnt[]={0, 359};
#else
#define FILE_NAME "/mnt/usb/sda1/aes_ecb_pure.ts"
#define DSC_NUM 3
int blk_cnt[]={0, 133,223,337};
#endif

int wb_video_type = AUI_DECV_FORMAT_MPEG;
int wb_video_pid = 513;
int wb_audio_type = AUI_DECA_STREAM_TYPE_MPEG1;
int wb_audio_pid = 660;
char *wb_file = FILE_NAME;

#define BLOCK_SIZE (48128) //47*1024
unsigned long test_magic_number = 0;
static int wb_start_block_idx = 0;

static void *block_data_write(aui_hdl av_hdl, int dsc_id, int len, FILE* file, unsigned char *buf, int offset)
{
	unsigned int buf_len = 0;
	int ret = -1;
	unsigned int i = 0;
	aui_av_inject_packet_info_t pkt_info;

	memset(&pkt_info, 0, sizeof(pkt_info));
	pkt_info.buffer_channel = AUI_AV_BUFFER_CHANNEL_DMX_FOR_BLOCK_ENCRYPTED_DATA;
	
	for(i = 0;i < len;){
		buf_len = ((len - i) > 2*BLOCK_SIZE) ? 2*BLOCK_SIZE : (len - i);
		if(fseek(file, offset + i, SEEK_SET)){
			printf("%s,%d,fseek error!\n",__func__,__LINE__);
			goto read_err;
		}
		ret = fread(buf,1,buf_len,file);
		
		if(ret != buf_len){
			printf("%s,%d,read file error, ret: %d, err: %d, eof: %d\n",__func__,__LINE__, ret, ferror(file), feof(file));
			goto read_err;
		}
		
		aui_av_buffer_status status;
		memset(&status, 0, sizeof(status));
		aui_av_buffer_status_get(av_hdl, AUI_AV_BUFFER_CHANNEL_DMX_FOR_BLOCK_ENCRYPTED_DATA, &status);
		printf("%s -> buffer status: total: %lu, free: %lu, valid: %lu\n", __FUNCTION__, status.total_size, status.free_size, status.valid_size);
		pkt_info.buffer_info.pts=wb_start_block_idx;
		ret = aui_av_write(av_hdl, &pkt_info, (const unsigned char *)buf, buf_len);
		if(ret){
			printf("%s,%d,decrypt streams error!\n",__func__,__LINE__);
			goto crypt_err;
		}
		i += buf_len;
		wb_start_block_idx += buf_len/BLOCK_SIZE;
	}

crypt_err:	
read_err:
	return NULL;
}


int do_inject_block(aui_dmx_dsc_id * p_dmx_dsc_id)
{
	aui_attrAV pst_attrAV;
	aui_hdl av_hdl=NULL;
	aui_hdl snd_hdl=NULL;

	FILE *f_ts_data=NULL;
	uint8_t *ts_data_buf = NULL;
	uint64_t ts_total_size;
    
	aui_av_stream_info stream_info;

	f_ts_data = fopen(wb_file, "rb");
	if(f_ts_data == NULL)
	{
		AUI_PRINTF("Open file %s fail\n", FILE_NAME);
		return 1;
	}
	fseek(f_ts_data, 0, SEEK_END);
	ts_total_size = ftell(f_ts_data);
	if(ts_total_size <= 0)
	{
		AUI_PRINTF("Total data file size error %lld\n", ts_total_size);
		goto EXIT;
	}
	fseek(f_ts_data, 0, SEEK_SET);

	AUI_PRINTF("Total data size %lld\n", ts_total_size);

	MEMSET(&stream_info, 0, sizeof(aui_av_stream_info));
	stream_info.st_av_info.b_audio_enable = 1;
	stream_info.st_av_info.b_video_enable = 1;
	stream_info.st_av_info.b_pcr_enable = 0;
	stream_info.st_av_info.b_dmx_enable = 1;
	stream_info.st_av_info.ui_audio_pid = wb_audio_pid; //660;
	stream_info.st_av_info.ui_video_pid = wb_video_pid; //513;
	stream_info.st_av_info.ui_pcr_pid = 8190;
	stream_info.st_av_info.en_audio_stream_type = AUI_DECA_STREAM_TYPE_MPEG1;
	stream_info.st_av_info.en_video_stream_type = AUI_DECV_FORMAT_MPEG;
	stream_info.stream_type.dmx_id = AUI_DMX_ID_SW_DEMUX0;
	stream_info.stream_type.data_type = AUI_AV_DATA_TYPE_RAM_TS;

	AUI_PRINTF("Audio pid = %d, Video pid = %d, Pcr pid = %d, a_type = %d, v_type = %d, dmx_id = %d\n", \
			stream_info.st_av_info.ui_audio_pid, stream_info.st_av_info.ui_video_pid, stream_info.st_av_info.ui_pcr_pid, 
			stream_info.st_av_info.en_audio_stream_type, stream_info.st_av_info.en_video_stream_type, stream_info.stream_type.dmx_id);

	//ts data buffer
	ts_data_buf = (uint8_t *)malloc(4*BLOCK_SIZE);
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
	aui_snd_mute_set(snd_hdl, 0);
	aui_snd_vol_set(snd_hdl, 50);
	aui_av_open(&pst_attrAV, &av_hdl);	
	
	aui_hdl dmx_hdl = NULL;
	aui_attr_dmx attr_dmx;
	memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
	attr_dmx.uc_dev_idx = AUI_DMX_ID_SW_DEMUX0;
	if(aui_find_dev_by_idx(AUI_MODULE_DMX, AUI_DMX_ID_SW_DEMUX0, &dmx_hdl)) {
        AUI_PRINTF("find aui_dmx handle fail\n");
		goto EXIT;
	}

	aui_dmx_data_path data_path;
	memset(&data_path, 0, sizeof(data_path));
	data_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
	data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_ID;
	data_path.p_dsc_id = p_dmx_dsc_id;
	if(aui_dmx_data_path_set(dmx_hdl, &data_path)) {
		AUI_PRINTF("data path set failed\n");
		goto EXIT;
	}

	aui_av_start(av_hdl);
	
	block_data_write(av_hdl, 0, (blk_cnt[1]-blk_cnt[0])*BLOCK_SIZE, f_ts_data, ts_data_buf, blk_cnt[0]*BLOCK_SIZE);
#ifdef WB_CHG_KEY
	block_data_write(av_hdl, 1, (blk_cnt[2]-blk_cnt[1])*BLOCK_SIZE, f_ts_data, ts_data_buf, blk_cnt[1]*BLOCK_SIZE);
	block_data_write(av_hdl, 2, (blk_cnt[3]-blk_cnt[2])*BLOCK_SIZE, f_ts_data, ts_data_buf, blk_cnt[2]*BLOCK_SIZE);
#endif
	AUI_SLEEP(10000);
	aui_av_stop(av_hdl);
	aui_av_close(av_hdl);
	EXIT:
	if (ts_data_buf)
		free(ts_data_buf);
	fclose(f_ts_data);
	printf("%s -> write block process exit\n", __FUNCTION__);
	return 0;
}

void print_help(char *sz_appname)
{
	printf("Uasage %s [m:h]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t-m --magic_number <magic_number>\n");
	printf("\t[q] - Exit application\n");
}

static int char2num(char c){
	if((c >='0') && (c<='9')) {
		return (c-'0');
	} else if((c >='a') && (c<='f')) {
		return 10 + (c-'a');
	} else if((c >='A') && (c<='F')) {
		return 10 + (c-'A');
	} 
	return 0;
}

static void str2dmx_dsc_id(char *id, aui_dmx_dsc_id *p_dmx_dsc_id) {
	unsigned int i = 0;
	printf("%s -> strlen: %d\n", __func__, strlen(id));
	for(i=0;i<strlen(id);) {
		p_dmx_dsc_id->identifier[i/2] = char2num(id[i])*16 + char2num(id[i+1]); 
		printf("%02x", p_dmx_dsc_id->identifier[i/2]);
		i +=2;
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	int c;
	int option_index = 0;

	char *short_options = "m:d:p:s:b:V:v:A:a:f:x:h";

	struct option long_options[] = {
	    {"magic_num",     required_argument, 0,  'm'},
		{"dmx id",     required_argument, 0,  'd'},
		{"data_path_type",     required_argument, 0,  'p'},
		{"dsc_process_mode",     required_argument, 0,  's'},
		{"block size",     required_argument, 0,  'b'},
		{"video type",     required_argument, 0,  'V'},
		{"video pid",     required_argument, 0,  'v'},
		{"audio type",     required_argument, 0,  'A'},
		{"audio pid",     required_argument, 0,  'a'},
		{"file path",     required_argument, 0,  'f'},
		{"magic_string",     required_argument, 0,  'x'},
	    {"help",    no_argument, 0,  'h'},
	    {0,         0,                 0,  0 }
	};

	aui_dmx_dsc_id dmx_dsc_id;
	memset(&dmx_dsc_id, 0, sizeof(dmx_dsc_id));

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {

		switch (c) {
			case 'm':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'd':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'p':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 's':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'b':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'V':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				wb_video_type= strtoul(optarg, 0, 10);
				break;
			case 'v':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				wb_video_pid= strtoul(optarg, 0, 10);
				break;
			case 'A':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				wb_audio_type= strtoul(optarg, 0, 10);
				break;
			case 'a':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				wb_audio_pid= strtoul(optarg, 0, 10);
				break;
			case 'f':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				wb_file= optarg;
				break;
			case 'x':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				str2dmx_dsc_id(optarg, &dmx_dsc_id);
				//return 0;
				break;
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				print_help(argv[0]);
				return -1;
		}
	}
	
	do_inject_block(&dmx_dsc_id);
	return 0;
}


