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
#include <aui_kl.h>
#include <aui_conaxvsc.h>


#define MINIMUM(a,b) ((a) < (b) ? (a) : (b))
#define MAXIMUM(a,b) ((a) > (b) ? (a) : (b))

#define CRYPTO_BLOCK_SIZE           48128
#define MAX_KEY_CNT    32


FILE *file = NULL;
uint8_t *buffer = NULL;

char *sz_file = "/mnt/streams/FASHIONTV.ts";
int audio_pid = 0x25a;
int video_pid = 0x1f6;
int pcr_pid = 0x1f6;

int video_type = 0;
int audio_type = 11;
int chunk_size = CRYPTO_BLOCK_SIZE;
uint8_t en_key[MAX_KEY_CNT+1][AUI_CONAXVSC_EN_KEY_LEN];
int max_key_cnt = 0;
int cur_key_idx = 0;
uint32_t total_size = 0;
uint32_t crypto_period = 200; /* in number of blocks */
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

int configure_conaxvsc_decrypt(aui_hdl hdl_kl, int parity, uint8_t *en_key)
{
	aui_hdl hdl_vsc = NULL;
	aui_conaxvsc_attr attr_vsc;
	aui_conaxvsc_en_key_attr attr_key;
	int i = 0;

	if (aui_find_dev_by_idx(AUI_MODULE_CONAXVSC, 0, &hdl_vsc)) {
		memset(&attr_vsc, 0, sizeof(aui_conaxvsc_attr));
		attr_vsc.uc_dev_idx = 0;
		if (aui_conaxvsc_open(&attr_vsc, &hdl_vsc)) {
			printf("aui_conaxvsc_open failed\n");
			return -1;
		}
	}

	memset(&attr_key, 0, sizeof(aui_conaxvsc_en_key_attr));
	attr_key.kl_handle = hdl_kl;
	attr_key.key_parity = !parity ? AUI_CONAXVSC_KEY_EVEN : AUI_CONAXVSC_KEY_ODD;
	memcpy(attr_key.en_key, en_key, AUI_CONAXVSC_EN_KEY_LEN);
	if (aui_conaxvsc_set_en_key(hdl_vsc, &attr_key)) {
		printf("aui_conaxvsc_set_en_key failed\n");
		return -1;
	}

    printf("parity:%s en_key:", !parity ? "EVEN" : "ODD");
	for (i = 0; i < AUI_CONAXVSC_EN_KEY_LEN; i++)
		printf("%02x", en_key[i]);
	printf("\n");

	return 0;
}

int do_playback()
{
	aui_hdl hdl_vsc = NULL;
	aui_hdl hdl_av = NULL;
	aui_hdl hdl_kl = NULL;
	aui_hdl hdl_dsc = NULL;
	aui_attr_kl attr_kl;
	aui_attr_dsc attr_dsc;
	aui_dmx_data_path dmx_path;
	unsigned short pid_tmp[8];
	unsigned char iv[16];
	int parity = 0;

	aui_av_stream_info stream_info;
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

	memset(&attr_kl, 0, sizeof(aui_attr_kl));
	attr_kl.uc_dev_idx = 0;
	attr_kl.en_root_key_idx = 2;
	attr_kl.en_level = 3;
	attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	attr_kl.en_key_ladder_type = AUI_KL_TYPE_CONAXVSC;
	aui_kl_open(&attr_kl, &hdl_kl);

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
	attr_dsc.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	aui_kl_get(hdl_kl, AUI_KL_GET_KEY_POS, &attr_dsc.ul_key_pos);
	aui_kl_get(hdl_kl, AUI_KL_GET_KEY_SIZE, &attr_dsc.ul_key_len);
	attr_dsc.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE;
	attr_dsc.pus_pids = pid_tmp;
	pid_tmp[0] = video_pid;
	pid_tmp[1] = audio_pid;
	attr_dsc.ul_pid_cnt = 2;
	attr_dsc.dsc_data_type = AUI_DSC_DATA_TS;
	memset(iv, 0, sizeof(iv));
	attr_dsc.puc_iv_ctr = iv;
	aui_dsc_attach_key_info2dsc(hdl_dsc, &attr_dsc);

	printf("attr_dsc.ul_key_pos=%d\n", (int)attr_dsc.ul_key_pos);
	printf("attr_dsc.ul_key_len=%d\n", (int)attr_dsc.ul_key_len);

	memset(&stream_info, 0, sizeof(aui_av_stream_info));
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
	dmx_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
	dmx_path.p_hdl_de_dev = hdl_dsc;
	aui_dmx_data_path_set(pst_attrAV.pv_hdl_dmx, &dmx_path);

	aui_av_start(hdl_av);

	parity = 0;
	configure_conaxvsc_decrypt(hdl_kl, (parity + 0) % 2, en_key[cur_key_idx + 0]);
	configure_conaxvsc_decrypt(hdl_kl, (parity + 1) % 2, en_key[cur_key_idx + 1]);

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

					/* perform key update here */
					total_size = 0;
					parity = 0;
					cur_key_idx = 0;
					configure_conaxvsc_decrypt(hdl_kl, (parity + 0) % 2, en_key[cur_key_idx + 0]);
					configure_conaxvsc_decrypt(hdl_kl, (parity + 1) % 2, en_key[cur_key_idx + 1]);

					continue;
				} else {
					usleep(5*1000*1000);
					goto exit;
				}
			}
#if 1
			total_size += cnt;
			if (total_size >= (crypto_period * CRYPTO_BLOCK_SIZE)) {
				/* perform key update here */
				printf("total_size = %d\n", total_size);
				cur_key_idx = (cur_key_idx + 1) % max_key_cnt;
				parity = cur_key_idx % 2;
				configure_conaxvsc_decrypt(hdl_kl, (parity + 0) % 2, en_key[cur_key_idx + 0]);
				configure_conaxvsc_decrypt(hdl_kl, (parity + 1) % 2, en_key[cur_key_idx + 1]);
				total_size = 0;
			}
#endif
		}
	}

exit:
	aui_av_stop(hdl_av);
	aui_av_close(hdl_av);
	if (!aui_find_dev_by_idx(AUI_MODULE_CONAXVSC, 0, &hdl_vsc)) {
		aui_conaxvsc_close(hdl_vsc);
	}
	aui_dsc_close(hdl_dsc);
	aui_kl_close(hdl_kl);

	return 0;	
}

void print_help(char *sz_appname)
{
	printf("Uasage %s [f:x:y:v:a:p:s:d:t:lh]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t-f --file <filepath>\n");
	printf("\t-v --vpid <video_pid>\n");
	printf("\t-a --apid <audio_pid>\n");
	printf("\t-p --ppid <pcr_pid>\n");
	printf("\t-V --vtype <video_type> 0 for mpeg2, 1 for AVC\n");
	printf("\t-A --atype <audio_type>\n");
	printf("\t-t --kp    <crypto_period> in number of blocks\n");
	printf("\t-k --enkey <en_key> decryption in hexstring\n");
	printf("\t-l --loop\n");
	printf("\t-h --help\n");
}

int main(int argc, char **argv)
{
	int i, j;
	int c;
	int option_index = 0;

	char *short_options = "f:V:A:v:a:p:k:t:lh";

	struct option long_options[] = {
	    {"file",     required_argument, 0,  'f'},
	    {"vtype",  required_argument, 0,  'V'},
	    {"atype",  required_argument, 0,  'A'},
	    {"vpid", required_argument,       0, 'v'},
	    {"apid",  required_argument, 0, 'a'},
	    {"ppid",    required_argument, 0,  'p'},
	    {"kp",    required_argument, 0,  't'},
	    {"enkey",    required_argument, 0,  'k'},
	    {"loop",    no_argument, 0,  'l'},
	    {"help",    no_argument, 0,  'h'},
	    {0,         0,                 0,  0 }
	};

	memset(en_key, 0, MAX_KEY_CNT * AUI_CONAXVSC_EN_KEY_LEN);

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
		case 't':
			crypto_period = (int) strtol(optarg, NULL, 0);
			break;
		case 'k':
			if (strlen(optarg) < AUI_CONAXVSC_EN_KEY_LEN)
				return -1;

			if (max_key_cnt >= MAX_KEY_CNT)
				return -1;

			hexstring_to_bytearray(optarg, en_key[max_key_cnt],
					AUI_CONAXVSC_EN_KEY_LEN);
			max_key_cnt++;
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

	if (max_key_cnt < 2) {
		printf("please give at least 2 keys");
		print_help(argv[0]);
		exit(-1);
	}

	printf("filepath=%s vpid=%d apid=%d ppid=%d\n",
			sz_file, video_pid, audio_pid, pcr_pid);
	printf("crypto_period %d\n", crypto_period);
	for (j = 0; j < max_key_cnt; j++) {
		printf("en_key: ");
		for (i = 0; i < AUI_CONAXVSC_EN_KEY_LEN; i++)
			printf("%02x", en_key[j][i]);
		printf("\n");
	}

	int res = do_playback();

	return res;
}
