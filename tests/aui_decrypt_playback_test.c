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
#include <aui_dsc.h>


#define MINIMUM(a,b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a,b) ((a) > (b) ? (a) : (b))

#define EN_KEY_LEN (16)


FILE *file = NULL;
uint8_t *buffer = NULL;

char *sz_file = "/mnt/streams/FASHIONTV.ts";
int audio_pid = 0x25a;
int video_pid = 0x1f6;
int pcr_pid = 0x1f6;

int video_type = 0;
int audio_type = 11;
int chunk_size = 48128;
uint8_t en_key[EN_KEY_LEN];
int do_loop = 0;


static int hexstring_to_bytearray(const char *hexstr, unsigned char *buf, int len)
{
    int i = 0;
    const char *pos = hexstr;

    /* WARNING: no sanitization or error-checking whatsoever */
    for(i = 0; i < len; i++) {
        sscanf(pos, "%2hhx", &buf[i]);

        pos += 2;
    }

    return 0;
}

int do_playback()
{
	aui_hdl hdl_av = NULL;
	aui_hdl hdl_dsc = NULL;
	aui_attr_dsc attr_dsc;
	aui_dmx_data_path dmx_path;
	unsigned short pid_tmp[8];
	unsigned char iv[16];

	aui_av_stream_info_t stream_info;
	aui_attrAV pst_attrAV;

	/* open input file */
	file = fopen(sz_file, "rb");
	if (file == NULL) {
		printf("could not open file %s!\n", sz_file);
		return -1;
	}

	buffer = (uint8_t *) malloc(chunk_size);
	if (!buffer) {
		printf("malloc for playback buffer failed!\n");
		return -1;
	}

	memset(&attr_dsc, 0, sizeof(aui_attr_dsc));
	attr_dsc.uc_dev_idx = 0;
	attr_dsc.uc_algo = AUI_DSC_ALGO_AES;
	attr_dsc.dsc_data_type = AUI_DSC_DATA_TS;
	aui_dsc_open(&attr_dsc, &hdl_dsc);

	memset(&attr_dsc, 0, sizeof(aui_attr_dsc));
	attr_dsc.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	attr_dsc.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr_dsc.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
	attr_dsc.en_en_de_crypt = AUI_DSC_DECRYPT;
	attr_dsc.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;

	attr_dsc.puc_key = en_key;
	attr_dsc.ul_key_len = EN_KEY_LEN * 8; //128;
	//attr_dsc.csa_version = AUI_DSC_CSA2;

	attr_dsc.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
	attr_dsc.pus_pids = pid_tmp;
	pid_tmp[0] = video_pid;
	pid_tmp[1] = audio_pid;
	attr_dsc.ul_pid_cnt = 2;
	attr_dsc.dsc_data_type = AUI_DSC_DATA_TS;
	memset(iv, 0, sizeof(iv));
	attr_dsc.puc_iv_ctr = iv;
	aui_dsc_attach_key_info2dsc(hdl_dsc, &attr_dsc);

	printf("attr_dsc.ul_key_len=%d\n", (int)attr_dsc.ul_key_len);

	memset(&stream_info, 0, sizeof(aui_av_stream_info_t));
	memset(&pst_attrAV, 0, sizeof(aui_attrAV));

	stream_info.st_av_info.b_audio_enable = 1;
	stream_info.st_av_info.b_video_enable = 1;
	stream_info.st_av_info.b_pcr_enable = 1;
	stream_info.st_av_info.b_dmx_enable = 1;
	stream_info.st_av_info.ui_audio_pid = audio_pid;
    stream_info.st_av_info.ui_video_pid = video_pid;
	stream_info.st_av_info.ui_pcr_pid = pcr_pid;
	stream_info.st_av_info.en_audio_stream_type = audio_type;
	stream_info.st_av_info.en_video_stream_type = video_type;
	stream_info.st_av_info.en_spdif_type = AUI_SND_OUT_MODE_DECODED;
	stream_info.stream_type.dmx_id = AUI_DMX_ID_SW_DEMUX0;
	stream_info.stream_type.data_type = AUI_AV_DATA_TYPE_RAM_TS;
	aui_av_init_attr(&pst_attrAV, &stream_info);
	aui_av_open(&pst_attrAV, &hdl_av);

	memset(&dmx_path, 0, sizeof(aui_dmx_data_path));
	dmx_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
	//dmx_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
	dmx_path.p_hdl_de_dev = hdl_dsc;
	aui_dmx_data_path_set(pst_attrAV.pv_hdl_dmx, &dmx_path);

	aui_av_start(hdl_av);

	while (1) {
		//usleep(2 * 1000);
		int total = 0;

		while (total < chunk_size) {
			int to_read = chunk_size - total;

			int cnt = fread(buffer, 1, to_read, file);
			if (cnt > 0) {
				aui_av_write(hdl_av, NULL, buffer, cnt);
				total += cnt;
			}

			if (feof(file)) {
				printf("end of file\n");
				if (do_loop) {
					fseek(file, 0 , SEEK_SET);
					continue;
				} else {
					usleep(10*1000*1000);
					goto exit;
				}
			}
		}
	}

exit:
	aui_av_stop(hdl_av);
	aui_av_close(hdl_av);
	aui_dsc_close(hdl_dsc);

	return 0;	
}

void print_help(char *sz_appname)
{
	printf("Uasage %s [f:x:y:v:a:p:s:d:lh]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t-f --file <filepath>\n");
	printf("\t-v --vpid <video_pid>\n");
	printf("\t-a --apid <audio_pid>\n");
	printf("\t-p --ppid <pcr_pid>\n");
	printf("\t-V --vtype <video_type> 0 for mpeg2, 1 for AVC\n");
	printf("\t-A --atype <audio_type>\n");
	printf("\t-k --enkey <en_key> decryption in hexstring\n");
	printf("\t-l --loop\n");
	printf("\t-h --help\n");
}

int main(int argc, char **argv)
{
	int i;
	int c;
	int option_index = 0;

	char *short_options = "f:V:A:v:a:p:k:lh";

	struct option long_options[] = {
	    {"file",     required_argument, 0,  'f'},
	    {"vtype",  required_argument, 0,  'V'},
	    {"atype",  required_argument, 0,  'A'},
	    {"vpid", required_argument,       0, 'v'},
	    {"apid",  required_argument, 0, 'a'},
	    {"ppid",    required_argument, 0,  'p'},
	    {"enkey",    required_argument, 0,  'k'},
	    {"loop",    no_argument, 0,  'l'},
	    {"help",    no_argument, 0,  'h'},
	    {0,         0,                 0,  0 }
	};

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
			video_pid = (int) strtol(optarg, NULL, 0);
			break;
		case 'a':
			audio_pid = (int) strtol(optarg, NULL, 0);
			break;
		case 'p':
			pcr_pid = (int) strtol(optarg, NULL, 0);
			break;
		case 'k':
			hexstring_to_bytearray(optarg, en_key, EN_KEY_LEN);
			break;
		case 'l':
			do_loop = 1;
			break;
		case 'h':
			print_help(argv[0]);
			return 0;
		default:
			print_help(argv[0]);
			return -1;
		}
	}

	printf("filepath=%s vpid=%d apid=%d ppid=%d\n",
			sz_file, video_pid, audio_pid, pcr_pid);
	printf("en key=");
	for (i = 0; i < EN_KEY_LEN; i++)
		printf("%02x", en_key[i]);
	printf("\n");

	int res = do_playback();

	return res;
}
