#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include <aui_common.h>
#include <aui_av.h>
#include <aui_deca.h>
#include <aui_decv.h>
#include <aui_snd.h>
#include <aui_dmx.h>
#include <sys/times.h>


#define MINIMUM(a,b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a,b) ((a) > (b) ? (a) : (b))


pthread_t th;
pthread_attr_t attr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
FILE *file = NULL;
aui_hdl av_hdl = NULL;
uint8_t *buffer = NULL;
size_t buf_size;

aui_attrAV pst_attrAV;
int speed = AUI_PLAYBACK_SPEED_1;
int direction = AUI_PLAYBACK_FORWARD;
aui_decv_info pst_info;
static volatile int is_paused = 0;
static volatile int is_quited = 0;
int quit_flag = 0;
int first_displayed = 1;
int64_t new_frame_cnt = 0;
int64_t new_speed_start = 0;

#if 0
#if 0
char *sz_file= "/mnt/usb/sda1/ddplus2_cut2_9M.ts";
int audio_pid = 130;
int video_pid =120;
int pcr_pid = 120;

int video_type = 1;
int audio_type = 14;
int bitrate = 24100 * 1000;
int chunk_size = 47 * 1024 * 4;
int gopsize = 20;
int frame_rate = 25;
int do_loop = 0;
int file_offset = 0;
int file_size = 0;
#else
char *sz_file= "/mnt/usb/sda1/caiqin_non_block_data.ts";
//char *sz_file= "/mnt/usb/sda1/caiqin.ts";
int audio_pid = 852;
int video_pid =851;
int pcr_pid = 851;

int video_type = 1;
int audio_type = 3;
int bitrate = 15135 * 1000;
int chunk_size = 47 * 1024 * 4;
int gopsize = 20;
int frame_rate = 25;
int do_loop = 0;
int file_offset = 0;
int file_size = 0;
#endif
#if 0
/char *sz_file = "/mnt/streams/avatar1080P25fps30Mbps.ts";
int audio_pid = 4352;
int video_pid = 4113;
int pcr_pid = 4097;
#endif
#else
char *sz_file = "/mnt/usb/sda1/cctv2_block_rec.ts"; //"/mnt/streams/FASHIONTV.ts";
int audio_pid = 660;
int video_pid = 513;
int pcr_pid = 8190;

int video_type = 0;
int audio_type = 1;
int bitrate = 3223 * 1000;
int chunk_size = 47 * 1024 * 4;
int gopsize = 20;
int frame_rate = 25;
int do_loop = 0;
int file_offset = 0;
int file_size = 0;
#endif
#if 0
int video_type = 0;
int audio_type = 1;
int bitrate = 3223 * 1024;
int chunk_size = 47 * 1024 * 4;
int gopsize = 20;
int frame_rate = 25;
int do_loop = 0;
int file_offset = 0;
int file_size = 0;
#endif

aui_av_trickmode_status trickmode_status;
int64_t trickmode_timestamp;
int trickmode_bytecnt = 0;



int mygetch ( void )
{
#if 0
  int ch;
  system ("/bin/stty raw");
  ch = getchar();
  system ("/bin/stty cooked");
  return ch;
#else
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
#endif
}

int64_t getcurrtime_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

static int get_next_speed_value(int speed)
{
	if (speed ==  AUI_PLAYBACK_SPEED_1)
		return  AUI_PLAYBACK_SPEED_2;
	else if (speed ==  AUI_PLAYBACK_SPEED_2)
		return  AUI_PLAYBACK_SPEED_4;
	else if (speed ==  AUI_PLAYBACK_SPEED_4)
		return  AUI_PLAYBACK_SPEED_8;
	else if (speed ==  AUI_PLAYBACK_SPEED_8)
		return  AUI_PLAYBACK_SPEED_16;
	else if (speed ==  AUI_PLAYBACK_SPEED_16)
		return  AUI_PLAYBACK_SPEED_32;
	else if (speed ==  AUI_PLAYBACK_SPEED_32)
		return  AUI_PLAYBACK_SPEED_64;
	else if (speed ==  AUI_PLAYBACK_SPEED_64)
		return  AUI_PLAYBACK_SPEED_128;

	return AUI_PLAYBACK_SPEED_1;
}

static int get_speed_factor(int speed)
{
	if (speed ==  AUI_PLAYBACK_SPEED_2)
		return  2;
	else if (speed ==  AUI_PLAYBACK_SPEED_4)
		return  4;
	else if (speed ==  AUI_PLAYBACK_SPEED_8)
		return  8;
	else if (speed ==  AUI_PLAYBACK_SPEED_16)
		return  16;
	else if (speed ==  AUI_PLAYBACK_SPEED_32)
		return  32;
	else if (speed ==  AUI_PLAYBACK_SPEED_64)
		return  64;
	else if (speed ==  AUI_PLAYBACK_SPEED_128)
		return  128;

	return 1;
}


void inject_chunk(int size)
{
	aui_av_inject_packet_info_t inject_set;
	int total = 0;
	inject_set.buffer_channel=AUI_AV_BUFFER_CHANNEL_DMX_FOR_BLOCK_ENCRYPTED_DATA;

	if (is_paused) {
		return;
	}

	while (total < size) {
		int to_read = size - total;
						
		int cnt = fread(buffer, 1, to_read, file);
		if (cnt > 0) {
			aui_av_write(av_hdl, &inject_set, buffer, cnt);
			total += cnt;
		}

		if (feof(file)) {
			fseek(file, 0 , SEEK_SET);
			printf("end of file\n");
			return;
			//exit(0);
		}
	}
}

static void* playback_thread_loop(void* arg)
{
	int hopsize = 0;
	int cnt = 0;
	int last_backward_pos = 0;

	pthread_detach(pthread_self());

	speed =  AUI_PLAYBACK_SPEED_1;

	while (0 == quit_flag) {

		usleep(2 * 1000);

		if (speed == AUI_PLAYBACK_SPEED_1)
		{
			pthread_mutex_lock(&mutex);
			inject_chunk(chunk_size);
			pthread_mutex_unlock(&mutex);

			file_offset += chunk_size;
		} else {

			pthread_mutex_lock(&mutex);
			inject_chunk(trickmode_status.ul_chunk_size);
			aui_av_trickmode_status_get(av_hdl, &trickmode_status);
			pthread_mutex_unlock(&mutex);

			file_offset += trickmode_status.ul_chunk_size;
			trickmode_bytecnt += trickmode_status.ul_chunk_size;

			if (trickmode_status.action == AUI_AV_TRICK_MODE_ACTION_NEW_FRAME)
			{
				struct timeval tv;
				gettimeofday(&tv, NULL);

#if 1
				int speed_factor = get_speed_factor(speed);
				int64_t cur_time = getcurrtime_ms();
				int64_t expected_time = (double) (trickmode_bytecnt * 1000.0 * 2) / (double) (speed_factor * bitrate/2); // x2
				int64_t elapsed_time = cur_time - trickmode_timestamp;
				if(0==expected_time) {
					expected_time++;
				}
				int64_t gop_skip = elapsed_time / expected_time - 1;

				if (elapsed_time < expected_time)
					usleep((expected_time - elapsed_time) * 1000);

#endif
				int64_t dur = (cur_time-new_speed_start)/1000;
				if(0 == dur)
					dur++;
				new_frame_cnt++;

				#if 1
				printf("new frame %d.%03d byte=%lu(%d) time=%lld (%lld), %d, speed: %d, dur:%lld, frm_cnt: %lld, fps: %lld\n",
						(int)tv.tv_sec, (int) tv.tv_usec / 1000, trickmode_status.ul_chunk_size, trickmode_bytecnt, elapsed_time, expected_time, cnt,
						speed_factor, dur, new_frame_cnt, (new_frame_cnt/dur));
				#endif
				if (direction == AUI_PLAYBACK_BACKWARD) {
					// go back to where we alread read

					// need to calculate hopsize based on play speed
					hopsize = -12 * trickmode_status.ul_chunk_size;
					file_offset = last_backward_pos + hopsize;
					file_offset -= (gop_skip > 0) ? gop_skip * trickmode_bytecnt  : 0;

					if (file_offset < 0) {
						file_offset += file_size;
					}

					fseek(file, file_offset , SEEK_SET);
					file_offset = ftell(file);
					printf("new search from  %d -> %d\n", file_offset, last_backward_pos);
					last_backward_pos = file_offset;
				} else {
					if (gop_skip > 0) {
						fseek(file, gop_skip * trickmode_bytecnt , SEEK_CUR);
					}
				}

				trickmode_timestamp = getcurrtime_ms();
				trickmode_bytecnt = 0;
				cnt = 0;
			}
			else
			{
				cnt++;
			}
		}

	}
	printf("%s -> it is time to quit\n", __FUNCTION__);
	aui_av_stop(av_hdl);
	aui_av_close(av_hdl);
	if(buffer){
		free(buffer);
		buffer = NULL;
	}
	is_quited = 1;
	printf("%s -> quit successfully\n", __FUNCTION__);
	return 0;
}

int handle_user_input()
{
	while(1) {

		char ch = mygetch();

			if (ch == 'q') {
				quit_flag = 1;
				while(is_quited == 0) {
					printf("%s -> wait feed thread to quit first\n", __FUNCTION__);
					usleep(200);
				}
				return 0;
			} else if (ch == 0x20) {// SPACE

				is_paused = !is_paused;
				printf("SPACE pause=%d\n", is_paused);

			} else if (ch == 'p') {
				if (speed !=  AUI_PLAYBACK_SPEED_1) {

					pthread_mutex_lock(&mutex);

					speed =  AUI_PLAYBACK_SPEED_1;
					direction = AUI_PLAYBACK_FORWARD;

					aui_av_trickmode_params trick_params;

					trick_params.mode = AUI_AV_TRICK_MODE_NONE;
					trick_params.speed =  speed;
					trick_params.direction = direction;
					trick_params.ul_bitrate = bitrate;

					aui_av_trickmode_set(av_hdl, &trick_params);

					pthread_mutex_unlock(&mutex);
				}
			} else if (ch == 'b') {

				pthread_mutex_lock(&mutex);

				speed = get_next_speed_value(speed);
				direction = AUI_PLAYBACK_BACKWARD;

				aui_av_trickmode_params trick_params;

				trick_params.mode = AUI_AV_TRICK_MODE_APP;
				trick_params.speed =  speed;
				trick_params.direction = direction;
				trick_params.ul_bitrate = bitrate;

				aui_av_trickmode_set(av_hdl, &trick_params);

				/* get recommended chunk_size */
				aui_av_trickmode_status_get(av_hdl, &trickmode_status);
				trickmode_timestamp = getcurrtime_ms();		
				trickmode_bytecnt = 0;
				pthread_mutex_unlock(&mutex);

			} else if (ch == 'f') {

				pthread_mutex_lock(&mutex);
				speed = get_next_speed_value(speed);
				direction = AUI_PLAYBACK_FORWARD;

				aui_av_trickmode_params trick_params;

				trick_params.mode = AUI_AV_TRICK_MODE_APP;
				trick_params.speed =  speed;
				trick_params.direction = direction;
				trick_params.ul_bitrate = bitrate;

				aui_av_trickmode_set(av_hdl, &trick_params);

				/* get recommended chunk_size */
				aui_av_trickmode_status_get(av_hdl, &trickmode_status);
				trickmode_timestamp = getcurrtime_ms();
				new_speed_start = trickmode_timestamp;
				new_frame_cnt = 0;
				trickmode_bytecnt = 0;
				pthread_mutex_unlock(&mutex);
			} 
		}
}

static int do_trick_play(aui_dmx_dsc_id *p_dmx_dsc_id)
{
	aui_av_stream_info stream_info;
	int ret = 0;

	/* open input file */
	file = fopen(sz_file, "rb");
	if (file == NULL) {
		printf("Could not open file %s!\n", sz_file);
		return -1;
	}

	buffer = (uint8_t *) malloc(bitrate);

	if (!buffer) {
		printf("malloc for playback buffer failed!\n");
		return -1;
	}

	// need to disable audio in trick mode
	stream_info.st_av_info.b_audio_enable = 0;
	stream_info.st_av_info.b_video_enable = 1;
	stream_info.st_av_info.b_pcr_enable = 0;
	stream_info.st_av_info.b_dmx_enable = 1;

	stream_info.st_av_info.ui_audio_pid = audio_pid;
    stream_info.st_av_info.ui_video_pid = video_pid;
	stream_info.st_av_info.ui_pcr_pid = pcr_pid;
	stream_info.st_av_info.en_audio_stream_type = audio_type;
	stream_info.st_av_info.en_video_stream_type = video_type;

	stream_info.stream_type.dmx_id = AUI_DMX_ID_SW_DEMUX0;
	stream_info.stream_type.data_type = AUI_AV_DATA_TYPE_RAM_TS;

	aui_av_init_attr(&pst_attrAV, &stream_info);
	aui_av_open(&pst_attrAV, &av_hdl);

	aui_hdl dmx_hdl = NULL;
	aui_attr_dmx attr_dmx;
	memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
	attr_dmx.uc_dev_idx = AUI_DMX_ID_SW_DEMUX0;
	if(aui_find_dev_by_idx(AUI_MODULE_DMX, AUI_DMX_ID_SW_DEMUX0, &dmx_hdl)) {
        AUI_PRINTF("find aui_dmx handle fail\n");
		free(buffer);
		return -1;
	}

	aui_dmx_data_path data_path;
	memset(&data_path, 0, sizeof(data_path));
	data_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
	data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_ID;
	data_path.p_dsc_id = p_dmx_dsc_id;
	if(aui_dmx_data_path_set(dmx_hdl, &data_path)) {
		AUI_PRINTF("data path set failed\n");
		free(buffer);
		return -1;
	}
	
	aui_av_start(av_hdl);

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	speed =  AUI_PLAYBACK_SPEED_1;

	ret = pthread_create(&th, NULL, playback_thread_loop, NULL);
    if (ret) {
        printf("pthread_create error\n");
		free(buffer);
        return -1;
    }

	handle_user_input();

	return 0;	
}

void print_help(char *sz_appname)
{
	printf("Uasage %s [f:x:y:v:a:p:lh]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t-f --file <filepath>\n");
	printf("\t-v --vpid <video_pid>\n");
	printf("\t-a --apid <audio_pid>\n");
	printf("\t-p --ppid <pcr_pid>\n");
	printf("\t-V --vtype <video_type>\n");
	printf("\t-A --atype <audio_type>\n");
	printf("\t-b --bitrate <bitrate> bps\n");
	printf("\t-c --cz <chunk_size> bytes\n");
	printf("\t-g --gs <trick_param> bytes\n");
	printf("\t-l --loop\n");
	printf("\t-h --help\n");
	printf("\nTrick mode control\n\n");
	printf("\t[p] - normal playback mode\n");
	printf("\t[b] - REW playback mode\n");
	printf("\t[f] - FF playback mode\n");
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

	char *short_options = "f:V:A:v:a:p:b:c:g:r:l:x:h";

	struct option long_options[] = {
	    {"file",     required_argument, 0,  'f'},
	    {"vtype",  required_argument, 0,  'V'},
	    {"atype",  required_argument, 0,  'A'},
	    {"vpid", required_argument,       0, 'v'},
	    {"apid",  required_argument, 0, 'a'},
	    {"ppid",    required_argument, 0,  'p'},
	    {"bitrate",    required_argument, 0,  'b'},
	    {"cs",    required_argument, 0,  'c'},
	    {"gs",    required_argument, 0,  'g'},
	    {"fps",    required_argument, 0,  'r'},
	    {"loop",    no_argument, 0,  'l'},
	    {"magic_string",    no_argument, 0,  'x'},
	    {"help",    no_argument, 0,  'h'},
	    {0,         0,                 0,  0 }
	};

	aui_dmx_dsc_id dmx_dsc_id;
	memset(&dmx_dsc_id, 0, sizeof(dmx_dsc_id));

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {

		switch (c) {
		case 'f':
			sz_file = optarg;
			break;
		case 'V':
			video_type = atoi(optarg);
			break;
		case 'A':
			audio_type = atoi(optarg);
			break;
		case 'v':
			video_pid = atoi(optarg);
			break;
		case 'a':
			audio_pid = atoi(optarg);
			break;
		case 'p':
			pcr_pid = atoi(optarg);
			break;
		case 'b':
			bitrate = atoi(optarg);
			break;
		case 'c':
			chunk_size = atoi(optarg);
			break;
		case 'g':
			gopsize = atoi(optarg);
			break;
		case 'r':
			frame_rate = atoi(optarg);
			break;
		case 'l':
			do_loop = 1;
			break;
		case 'x':
			printf("%s -> %s\n", __FUNCTION__, optarg);
			str2dmx_dsc_id(optarg, &dmx_dsc_id);
			break;
		case 'h':
			print_help(argv[0]);
			return 0;
		default:
			print_help(argv[0]);
			return -1;
		}
	}

	printf("filepath=%s vpid=%d apid=%d ppid=%d\n", sz_file, video_pid, audio_pid, pcr_pid);
	printf("bitrate=%d chunk_size=%d gopsize=%d\n", bitrate, chunk_size, gopsize);

	int res = do_trick_play(&dmx_dsc_id);

	return res;
}
