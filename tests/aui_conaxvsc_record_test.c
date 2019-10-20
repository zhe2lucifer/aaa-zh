/*
 * Conax Virtual Smart Card test module
 * Copyright(C) 2014 ALi Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#define _GNU_SOURCE         /* See feature_test_macros(7) */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include <stdarg.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>

#include <aui_conaxvsc.h>
#include <aui_nim.h>
#include <aui_dsc.h>
#include <aui_dmx.h>
#include <aui_kl.h>
#include <aui_tsg.h>
#include <aui_tsi.h>
#include <aui_common.h>
#include <aui_av.h>
#include <aui_deca.h>
#include <aui_decv.h>
#include <aui_snd.h>
#ifdef ENABLE_SMK
#include "SMK.h" 
#endif
#define FD_INVALID     -1
#define MAX_PID_CNT    32
#define MAX_KEY_CNT    32

#define VSC_PI_HOST_VER             0x10
#define VSC_PI_CAT                  0x11
#define VSC_PI_EMM                  0x12
#define VSC_PI_ECM                  0x14
#define VSC_PI_RET_CHAN_CONFIG      0x1B
#define VSC_PI_CA_STATUS_SELECT     0x1C
#define VSC_PI_PIN_IN               0x1D
#define VSC_PI_REQ_BLOCK_SIZE       0x65
#define VSC_PI_REQ_CARD_NUMBER      0x66
#define VSC_PI_REQ_SEQUENCE_NUMBER  0x67
#define VSC_PI_RESET_SESSION        0x69
#define VSC_PI_CASS_VER             0x20
#define VSC_PI_CA_DESC_EMM          0x22
#define VSC_PI_SESSION_INFO         0x23
#define VSC_PI_CW                   0x25
#define VSC_PI_CA_SYS_ID            0x28
#define VSC_PI_CRYPTO_BLOCK_SIZE    0x73
#define VSC_PI_CARD_NUMBER          0x74
#define VSC_PI_SEQUENCE_NUMBER      0x75
#define VSC_PI_HOST_DATA            0x80

#define VSC_PI_REQ_VSC_VERSION      0xC1
#define VSC_PI_CF  					0xCF
#define VSC_PI_VALIDATION_NUMBER    0xD0
#define VSC_PI_VSC_VERSION          0xD1

#define CRYPTO_BLOCK_SIZE           48128

char infile[1028] = {'\0'};
char outfile[1028] = {'\0'};
int32_t pids[32];
int pid_cnt = 0;

int tuner = 0;
int freq = 1500;//3450;
int sym_rate = 27500;
int polar = AUI_NIM_POLAR_HORIZONTAL;
int vpid = 502;
int apid = 602;
int ppid = 502;
int epid = 0x1771;// epid of ts1_CL12v1.trp
#ifdef ENABLE_SMK
int cf_flag = 0;
#endif
int playback = -1;
int record = -1;
int session_id = 0;
uint8_t en_key[MAX_KEY_CNT][AUI_CONAXVSC_EN_KEY_LEN];
int max_key_cnt = 0;
int cur_key_idx = 0;
uint32_t crypto_period = 200; /* in number of blocks */

char *buffer = NULL;
int buf_size = CRYPTO_BLOCK_SIZE * 2;
uint32_t total_size = 0;

FILE *out_fp = NULL;


struct vsc_context {
	aui_hdl hdl_vsc;
	aui_hdl hdl_kl;
	aui_hdl hdl_cf_kl;
	aui_hdl hdl_target_kl;
	aui_hdl hdl_dmx;
	aui_hdl hdl_ch;
	aui_hdl hdl_flt;
	int session_id;
	int state;
	char buf[4096];
	int pid;
};

static struct vsc_context g_context;

int configure_conaxvsc_encrypt(int dsc_idx, int parity, uint8_t *en_key);


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

static long rb_request_cb(void *p_user_hdl, unsigned long ul_req_size, void ** pp_req_buf,
				  unsigned long *req_buf_size, struct aui_avsync_ctrl *pst_ctrl_blk)
{
	*pp_req_buf = buffer;
	*req_buf_size = buf_size;

	(void) p_user_hdl;
	(void) pp_req_buf;
	(void) req_buf_size;
	(void)ul_req_size;
	(void)pst_ctrl_blk;
	return 0;
}

static long rb_update_cb(void *p_user_hdl, unsigned long ul_size)
{
	int dsc_enc_idx = 1;
	int parity = 0;

	//printf("%s -> upd_size: %lu\n", __func__, ul_size);
	(void) p_user_hdl;

	fwrite(buffer, 1, ul_size, out_fp);

	total_size += ul_size;
	if (total_size >= (crypto_period * CRYPTO_BLOCK_SIZE)) {
		/* perform key update here */
		cur_key_idx = (cur_key_idx + 1) % max_key_cnt;
		parity = cur_key_idx % 2;
		configure_conaxvsc_encrypt(dsc_enc_idx, parity, en_key[cur_key_idx]);
		total_size = 0;
	}

	return 0;
}

/* callback function for filtered ECM sections.
 * Send ECM to Virtual Smart Card and parse/handle CW response
 * according to Conax Smart Card Interface specifications  */
static long vsc_ecm_callback_func(aui_hdl flt_hdl, unsigned char *buf,
	unsigned long len, void *usr_data, void *reserved)
{
	struct vsc_context *vsc_ctx = (struct vsc_context *) usr_data;
	aui_hdl hdl_vsc = vsc_ctx->hdl_vsc;
	aui_hdl hdl_kl = vsc_ctx->hdl_kl;
	#ifdef ENABLE_SMK
	aui_hdl hdl_cf_kl = vsc_ctx->hdl_cf_kl;
	aui_hdl hdl_target_kl = vsc_ctx->hdl_target_kl;
	#endif
	int sid = vsc_ctx->session_id;
	aui_conaxvsc_decw_attr attr;
	#ifdef ENABLE_SMK
	struct aui_cfg_kl cfg;
	aui_kl_key_source_attr key_source;	
	aui_kl_key_source_attr data_source;
	#endif
	aui_conaxvsc_tx_buf *command;
	aui_conaxvsc_tx_buf *response;
	int sw1, sw2;
	int ret;
	unsigned char *p;
	int PI, PL;
	unsigned char *PV;
	int size;

	(void) PI;
	(void) PL;
	(void) flt_hdl;
	(void) reserved;

	//printf("table_id:%02x len=%lu\n", buf[0], len);

	command = (aui_conaxvsc_tx_buf *) malloc(sizeof(aui_conaxvsc_tx_buf));
	response = (aui_conaxvsc_tx_buf *) malloc(sizeof(aui_conaxvsc_tx_buf));
	if (!command || !response) {
		printf("malloc failed\n");
		return -1;
	}

	command->p_uc_data[0] = 0x14;
	command->p_uc_data[1] = 3 + len;
	command->p_uc_data[2] = 0x00;
	memcpy(&command->p_uc_data[3], buf, len);
	command->n_size = 3 + len;

	ret = aui_conaxvsc_cmd_transfer(hdl_vsc, sid, command, response, &sw1, &sw2);
	if (ret || sw1 != 0x90 || sw2 != 0x00) {
		printf("aui_conaxvsc_cmd_transfer failed. ret=%d sw=%02x%02x\n",
			ret, sw1, sw2);
		ret = -1;
		goto exit;
	}

	size = response->n_size;
	p = response->p_uc_data;

	while (size > 2) {
		PI = p[0]; PL = p[1]; PV = &p[2];

		if(PI == VSC_PI_CW)
		{
			//printf("parse CW \n");
			memset(&attr, 0, sizeof(aui_conaxvsc_decw_attr));

			attr.kl_handle = hdl_kl;
			attr.key_id = p[3];
			attr.key_parity = p[4] ? AUI_CONAXVSC_KEY_ODD : AUI_CONAXVSC_KEY_EVEN;
			memcpy(attr.decw, &p[7], AUI_CONAXVSC_DECW_LEN);

			if (aui_conaxvsc_decrypt_cw(hdl_vsc, &attr)) {
				printf("aui_conaxvsc_decrypt_cw failed!");
				goto exit;
			}
		}
		#ifdef ENABLE_SMK
		if(PI == VSC_PI_HOST_DATA && p[2] == VSC_PI_CF)
		{
			//printf("parse CF \n");
			aui_kl_cf_target_attr target_set;

			//set cf kl handle
            		memset(&target_set,0,sizeof(aui_kl_cf_target_attr));
            		target_set.parity=AUI_KL_CW_KEY_EVEN;
            		target_set.uc_cf_dev_idx=0;
            		ret=aui_kl_cf_target_set(hdl_cf_kl,&target_set);
			if(ret != 0)
			{
        			printf("aui_kl_cf_target_set failed \n");
        			return -1;
    			}	
			SMK_error_t status;
    			uint32_t shv_result[4]={0};
    
    			status = SMK_GenerateCWC(&p[4], p[3], 1, 0, 0x03F0);
    			if (status != SMK_OK)
    			{
        			printf("SMK_GenerateCWC failed with code: 0x%08X\n", status);
        			return -1;
    			}
    			status=SMK_FinalizeCWC(shv_result);
    			if (status != SMK_OK)
    			{
        			printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
        			return -1;
    			}
		}
		#endif

		//printf("PI %02x len %d ind %d key_id %d\n", PI, PL, p[4], p[3]);

		p = &PV[PL];
		size -= (PL + 2);
	}
	#ifdef ENABLE_SMK
	if(cf_flag == 1)
	{
		memset(&key_source,0x0,sizeof(aui_kl_key_source_attr));
		memset(&data_source,0x0,sizeof(aui_kl_key_source_attr));
		memset(&cfg,0x0,sizeof(struct aui_cfg_kl));
		
		key_source.key_param.key_ladder_handle = hdl_cf_kl;
		data_source.key_param.key_ladder_handle = hdl_kl;

		ret = aui_kl_gen_key_by_cfg_ext(hdl_target_kl, &cfg, &data_source, &key_source);
		if (ret!= 0) 	
		{
			printf("aui_kl_gen_key_by_cfg_ext failed !\n");
			return -1;		
		}
	}
	#endif

	free(command);
	free(response);
	return 0;

exit:
	free(command);
	free(response);
	return -1;
}

int vsc_start_ecm_filter(aui_hdl hdl_vsc, aui_hdl hdl_dmx,
		aui_hdl hdl_kl, unsigned short epid, int sid, aui_hdl hdl_cf_kl, aui_hdl hdl_target_kl)
{
	aui_hdl hdl_ch;
	aui_hdl hdl_flt;
	int ret = 0;
	aui_attr_dmx_channel channel_attr;
	aui_attr_dmx_filter filter_attr;
	(void) ret;

	memset(&channel_attr, 0, sizeof(channel_attr));
	channel_attr.us_pid = epid;
	channel_attr.dmx_data_type = AUI_DMX_DATA_SECT;
	aui_dmx_channel_open(hdl_dmx, &channel_attr, &hdl_ch);

	memset(&filter_attr, 0, sizeof(filter_attr));
	filter_attr.uc_crc_check = 0;
	filter_attr.uc_continue_capture_flag = 1;
	filter_attr.puc_val = 0;
	filter_attr.puc_mask = 0;
	filter_attr.puc_reverse = 0;
	filter_attr.ul_mask_val_len = 0;
	filter_attr.p_fun_sectionCB = vsc_ecm_callback_func;
	filter_attr.usr_data = &g_context;
	aui_dmx_filter_open(hdl_ch, &filter_attr, &hdl_flt);

	aui_dmx_filter_start(hdl_flt, &filter_attr);

	aui_dmx_channel_start(hdl_ch, &channel_attr);

	printf("%s,%d: pid=%d,%04x!\n",
		__func__, __LINE__, epid, epid);

	g_context.hdl_vsc = hdl_vsc;
	g_context.hdl_kl = hdl_kl;
	g_context.hdl_cf_kl = hdl_cf_kl;
	g_context.hdl_target_kl = hdl_target_kl;
	g_context.hdl_dmx = hdl_dmx;
	g_context.session_id = sid;	//data->session_id;	???
	g_context.pid = epid;

	return 0;
}

//read initialization data from path
int conaxvsc_read_store_file(char *path, aui_conaxvsc_store *store)
{
	FILE *read_fd = fopen(path, "rb");
	int ret = 0;

	if (!read_fd) {
		printf("open %s failed err!\n", path);
		return 1;
	}

	if (!fread((unsigned char*)store, 1, sizeof(aui_conaxvsc_store), read_fd)) {
		printf("conaxvsc_read_store_file error!\n");
		ret = 2;
	}

	fclose(read_fd);
	return ret;
}

int configure_conaxvsc_cw_decrypt(int dsc_idx, int kl_idx, int vsc_idx, int dmx_idx)
{
	aui_hdl hdl_kl = NULL;
	aui_hdl cf_hdl = NULL;
	aui_hdl target_hdl = NULL;
	aui_hdl hdl_dsc = NULL;
	aui_hdl hdl_vsc = NULL;
	aui_hdl hdl_dmx = NULL;
	aui_attr_kl attr_kl;
	aui_attr_dsc attr_dsc[2];
	aui_conaxvsc_attr attr_vsc;
	aui_conaxvsc_store *store = NULL;
	char store_file[64] = "/mnt/usb/sda1/demo_00001.keystore";		//initialization data path
	unsigned short pid_tmp[8];
	unsigned char iv[16];
	int ret = 0;

	if (aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_idx, &hdl_dmx)) {
		printf("aui_find_dev_by_idx dmx %d failed\n", dmx_idx);
		return -1;
	}

	memset(&attr_vsc, 0, sizeof(aui_conaxvsc_attr));
	attr_vsc.uc_dev_idx = vsc_idx;
	if (aui_find_dev_by_idx(AUI_MODULE_CONAXVSC,
			attr_vsc.uc_dev_idx, &hdl_vsc)) {

		//initialization when first open conaxvsc		
		FILE *tmp_fd = fopen("/tmp/pesro.txt","rb");		//personalization flag
		if (!tmp_fd) {										//file is empty, perso hasn't yet execute
			printf("initialization when first open conaxvsc...\n");
			store = malloc(sizeof(aui_conaxvsc_store));
			if (!store) {
				printf("malloc for store failed!\n");
				return -1;
			}
			
			ret = conaxvsc_read_store_file(store_file, store);	//read store file to initialization
			if (ret) {
				printf("conaxvsc_read_store_file failed\n");
				free(store);		//free store and set store to NULL
				store = NULL;
				return -1;
			}			
			attr_vsc.p_vsc_store = store;
			
			if (aui_conaxvsc_open(&attr_vsc, &hdl_vsc)) {
				free(store);	
				store = NULL;
				printf("aui_conaxvsc_open failed\n");
				return -1;
			}
			free(store);
			store = NULL;			
			tmp_fd = fopen("/tmp/pesro.txt","wb");
			fclose(tmp_fd);				
		} else {
			fclose(tmp_fd);
			if (aui_conaxvsc_open(&attr_vsc, &hdl_vsc)) {
				printf("aui_conaxvsc_open failed\n");
				return -1;
			}
		}
	}

	memset(&attr_kl, 0, sizeof(aui_attr_kl));
	attr_kl.uc_dev_idx = kl_idx;
	attr_kl.en_root_key_idx = 0;
	attr_kl.en_level = 3;
	#ifdef ENABLE_SMK
	if(cf_flag == 1)
		attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	else
		attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	#else
	attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	#endif
	attr_kl.en_key_ladder_type = AUI_KL_TYPE_CONAXVSC;
	if (aui_find_dev_by_idx(AUI_MODULE_KL,
			attr_kl.uc_dev_idx, &hdl_kl)) {
		if (aui_kl_open(&attr_kl, &hdl_kl)) {
			printf("aui_kl_open failed\n");
			return -1;
		}
	}
	
	#ifdef ENABLE_SMK
	if(cf_flag == 1)
	{
		attr_kl.uc_dev_idx = 1;
		if (aui_find_dev_by_idx(AUI_MODULE_KL,
				attr_kl.uc_dev_idx, &target_hdl)) {
			if (aui_kl_open(&attr_kl, &target_hdl)) {
				printf("aui_kl_open target_hdl fail\n");
				return -1;
			}
		}

		attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
		attr_kl.uc_dev_idx = 2;
		if (aui_find_dev_by_idx(AUI_MODULE_KL,
				attr_kl.uc_dev_idx, &cf_hdl)) {
			if (aui_kl_open(&attr_kl, &cf_hdl)) {
				printf("aui_kl_open cf_hdl fail\n");
				return -1;
			}
		}
	}
	#endif
	memset(&attr_dsc[0], 0, sizeof(aui_attr_dsc));
	attr_dsc[0].uc_dev_idx = dsc_idx;
	attr_dsc[0].uc_algo = AUI_DSC_ALGO_CSA;
	attr_dsc[0].dsc_data_type = AUI_DSC_DATA_TS;
	if (aui_find_dev_by_idx(AUI_MODULE_DSC, attr_dsc[0].uc_dev_idx, &hdl_dsc)) {
		if (aui_dsc_open(&attr_dsc[0], &hdl_dsc)) {
			printf("aui_dsc_open failed\n");
			return -1;
		}
	}

	memset(&attr_dsc[1], 0, sizeof(aui_attr_dsc));
	attr_dsc[1].uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	attr_dsc[1].ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr_dsc[1].en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
	attr_dsc[1].en_en_de_crypt = AUI_DSC_DECRYPT;
	attr_dsc[1].dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	attr_dsc[1].csa_version = AUI_DSC_CSA2;
	#ifdef ENABLE_SMK
	if(cf_flag == 0)
	{
		aui_kl_get(hdl_kl, AUI_KL_GET_KEY_POS, &attr_dsc[1].ul_key_pos);
		aui_kl_get(hdl_kl, AUI_KL_GET_KEY_SIZE, &attr_dsc[1].ul_key_len);
	}
	else
	{
		aui_kl_get(target_hdl, AUI_KL_GET_KEY_POS, &attr_dsc[1].ul_key_pos);
		aui_kl_get(target_hdl, AUI_KL_GET_KEY_SIZE, &attr_dsc[1].ul_key_len);
	}
	#else
	aui_kl_get(hdl_kl, AUI_KL_GET_KEY_POS, &attr_dsc[1].ul_key_pos);
	aui_kl_get(hdl_kl, AUI_KL_GET_KEY_SIZE, &attr_dsc[1].ul_key_len);
	#endif
	attr_dsc[1].en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE;
	attr_dsc[1].pus_pids = pid_tmp;
	pid_tmp[0] = vpid;
	pid_tmp[1] = apid;
	attr_dsc[1].ul_pid_cnt = 2;
	attr_dsc[1].dsc_data_type = AUI_DSC_DATA_TS;
	memset(iv, 0, sizeof(iv));
	attr_dsc[1].puc_iv_ctr = iv;
	aui_dsc_attach_key_info2dsc(hdl_dsc, &attr_dsc[1]);

	vsc_start_ecm_filter(hdl_vsc, hdl_dmx, hdl_kl, epid, session_id, cf_hdl, target_hdl);

	return 0;
}
#ifdef ENABLE_SMK
static int cf_smk_init(void)
{
	static int cf_inited = 0;
	CAV_ID_T caId[1] = {0x03f0};

	if (cf_inited)
		return 0;

	INIT_PARAMS_T initParams;
	initParams.iNumCAVId = sizeof(caId)/sizeof(CAV_ID_T);
	initParams.pCAVId = (CAV_ID_T *)caId;

	SMK_error_t status = SMK_Initialize(&initParams);
	if (status != SMK_OK)
	{
 	printf("SMK_Initialize failed with error code %d\n", status);
    		return -1;
    	}

	CfVectorStruct_t vectorStruct_03F0;
  	vectorStruct_03F0.VectorType = 0;
  	vectorStruct_03F0.OutputUsageAllowed = 0x0F;
  	vectorStruct_03F0.Vector[ 0] = 0xF8121D46;
  	vectorStruct_03F0.Vector[ 1] = 0x57E04C6B;
  	vectorStruct_03F0.Vector[ 2] = 0x5C86240E;
  	vectorStruct_03F0.Vector[ 3] = 0x161BE0C4;
  	vectorStruct_03F0.Vector[ 4] = 0xB19A4665;
  	vectorStruct_03F0.Vector[ 5] = 0x8F14B43B;
  	vectorStruct_03F0.Vector[ 6] = 0xDFE6D16C;
  	vectorStruct_03F0.Vector[ 7] = 0x90EB31B0;
  	vectorStruct_03F0.Vector[ 8] = 0x03632E94;
  	vectorStruct_03F0.Vector[ 9] = 0x70D5FC0F;
  	vectorStruct_03F0.Vector[10] = 0x36082567;
  	vectorStruct_03F0.Vector[11] = 0x74BEA72C;
  	vectorStruct_03F0.Vector[12] = 0x617C5144;
  	vectorStruct_03F0.Vector[13] = 0x35ABC053;
  	vectorStruct_03F0.Vector[14] = 0x3028041B;
  	vectorStruct_03F0.Vector[15] = 0x4B25AE71;
  	vectorStruct_03F0.Vector[16] = 0xA40DF8D5;
  	vectorStruct_03F0.Vector[17] = 0x52DCE48F;
  	vectorStruct_03F0.Vector[18] = 0xEC25512A;
  	vectorStruct_03F0.Vector[19] = 0xEC481344;
  	status = SMK_ProcessSmm_VectorStruct(&vectorStruct_03F0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_VectorStruct(vectorStruct_03F0) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	/* store ERK */
  	CfErkStruct_t erkStruct_03F0_00AB;
  	erkStruct_03F0_00AB.ProductRange      = 0x00AB;
  	erkStruct_03F0_00AB.AddressingMode    = 0x00;
  	erkStruct_03F0_00AB.KeySenseIndicator = 0x03;
  	erkStruct_03F0_00AB.Bitmap[0]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[1]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[2]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[3]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[4]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[5]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[6]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Bitmap[7]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.Activation0       = 0x00000000;
  	erkStruct_03F0_00AB.Expiration0       = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.IRV0              = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.ERK0[0]           = 0xE800E14A;
  	erkStruct_03F0_00AB.ERK0[1]           = 0x8A18A03A;
  	erkStruct_03F0_00AB.ERK0[2]           = 0x69E47A5E;
  	erkStruct_03F0_00AB.ERK0[3]           = 0xE2F09553;
  	erkStruct_03F0_00AB.Activation1       = 0x00000000;
  	erkStruct_03F0_00AB.Expiration1       = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.IRV1              = 0xFFFFFFFF;
  	erkStruct_03F0_00AB.ERK1[0]           = 0xE800E14A;
  	erkStruct_03F0_00AB.ERK1[1]           = 0x8A18A03A;
  	erkStruct_03F0_00AB.ERK1[2]           = 0x69E47A5E;
  	erkStruct_03F0_00AB.ERK1[3]           = 0xE2F09553;
  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AB, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AB) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	/* run SHVs */
  	CfCwcStruct_t  cwcStruct_03F0_00AB;
  	uint32_t       shv_03F0_00AB[4];
  	uint32_t       expected_shv_03F0_00AB[4];
  	expected_shv_03F0_00AB[0]         = 0x0267EE50;
  	expected_shv_03F0_00AB[1]         = 0xFEBA5B9A;
  	expected_shv_03F0_00AB[2]         = 0x84BD1341;
  	expected_shv_03F0_00AB[3]         = 0x9783C27D;
  	memset(&cwcStruct_03F0_00AB, 0, sizeof(cwcStruct_03F0_00AB));
  	cwcStruct_03F0_00AB.Timestamp     = 0x00000000;
  	cwcStruct_03F0_00AB.ErkKeySense   = 0;
  	cwcStruct_03F0_00AB.NumProductIDs = 1;
  	cwcStruct_03F0_00AB.ProductID[0]  = 0x00AB;
  	cwcStruct_03F0_00AB.OutputUsage   = 0;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AB, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AB);
  	if (status != SMK_OK)
  	{
    	printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    	return -1;
  	}
  	/*printf("shv_03F0_00AB: %08X %08X %08X %08X\n", 
    		shv_03F0_00AB[0], shv_03F0_00AB[1], shv_03F0_00AB[2], shv_03F0_00AB[3]);*/ 
  	status = memcmp(expected_shv_03F0_00AB, shv_03F0_00AB, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	/* run SHV - unique */
  	cwcStruct_03F0_00AB.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AB[4];
  	expectedUniq_shv_03F0_00AB[0]     = 0x9D9EEF7C;
  	expectedUniq_shv_03F0_00AB[1]     = 0x0729FBAA;
  	expectedUniq_shv_03F0_00AB[2]     = 0x45981129;
  	expectedUniq_shv_03F0_00AB[3]     = 0xC9F00D9A;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AB, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AB);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AB: %08X %08X %08X %08X\n", 
    		shv_03F0_00AB[0], shv_03F0_00AB[1], shv_03F0_00AB[2], shv_03F0_00AB[3]);*/ 
  	status = memcmp(expectedUniq_shv_03F0_00AB, shv_03F0_00AB, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	/* store ERK */	

	CfErkStruct_t erkStruct_03F0_00AC;
  	erkStruct_03F0_00AC.ProductRange      = 0x00AC;
  	erkStruct_03F0_00AC.AddressingMode    = 0x00;
  	erkStruct_03F0_00AC.KeySenseIndicator = 0x03;
  	erkStruct_03F0_00AC.Bitmap[0]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[1]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[2]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[3]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[4]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[5]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[6]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Bitmap[7]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.Activation0       = 0x00000000;
  	erkStruct_03F0_00AC.Expiration0       = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.IRV0              = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.ERK0[0]           = 0xEE470C7E;
  	erkStruct_03F0_00AC.ERK0[1]           = 0xFE1B37F7;
  	erkStruct_03F0_00AC.ERK0[2]           = 0xEC6A1D0A;
  	erkStruct_03F0_00AC.ERK0[3]           = 0xF00EEAA9;
  	erkStruct_03F0_00AC.Activation1       = 0x00000000;
  	erkStruct_03F0_00AC.Expiration1       = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.IRV1              = 0xFFFFFFFF;
  	erkStruct_03F0_00AC.ERK1[0]           = 0xEE470C7E;
  	erkStruct_03F0_00AC.ERK1[1]           = 0xFE1B37F7;
  	erkStruct_03F0_00AC.ERK1[2]           = 0xEC6A1D0A;
  	erkStruct_03F0_00AC.ERK1[3]           = 0xF00EEAA9;
  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AC, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AC) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	/* run SHVs */
  	CfCwcStruct_t  cwcStruct_03F0_00AC;
  	uint32_t       shv_03F0_00AC[4];
  	uint32_t       expected_shv_03F0_00AC[4];
  	expected_shv_03F0_00AC[0]         = 0x9F91B820;
  	expected_shv_03F0_00AC[1]         = 0x3B474D09;
  	expected_shv_03F0_00AC[2]         = 0x1D6456FE;
  	expected_shv_03F0_00AC[3]         = 0x3566A691;
  	memset(&cwcStruct_03F0_00AC, 0, sizeof(cwcStruct_03F0_00AC));
  	cwcStruct_03F0_00AC.Timestamp     = 0x00000000;
  	cwcStruct_03F0_00AC.ErkKeySense   = 0;
  	cwcStruct_03F0_00AC.NumProductIDs = 1;
  	cwcStruct_03F0_00AC.ProductID[0]  = 0x00AC;
  	cwcStruct_03F0_00AC.OutputUsage   = 0;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AC, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AC);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AC: %08X %08X %08X %08X\n", 
    		shv_03F0_00AC[0], shv_03F0_00AC[1], shv_03F0_00AC[2], shv_03F0_00AC[3]);*/ 
  	status = memcmp(expected_shv_03F0_00AC, shv_03F0_00AC, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
   	/* run SHV - unique */
  	cwcStruct_03F0_00AC.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AC[4];
  	expectedUniq_shv_03F0_00AC[0]     = 0x11D40BE2;
  	expectedUniq_shv_03F0_00AC[1]     = 0x6A4F2F10;
  	expectedUniq_shv_03F0_00AC[2]     = 0xE5FEC187;
  	expectedUniq_shv_03F0_00AC[3]     = 0xEE2D0530;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AC, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AC);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AC: %08X %08X %08X %08X\n", 
    		shv_03F0_00AC[0], shv_03F0_00AC[1], shv_03F0_00AC[2], shv_03F0_00AC[3]);*/ 
  	status = memcmp(expectedUniq_shv_03F0_00AC, shv_03F0_00AC, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	/* store ERK */
  	CfErkStruct_t erkStruct_03F0_00AD;
  	erkStruct_03F0_00AD.ProductRange      = 0x00AD;
  	erkStruct_03F0_00AD.AddressingMode    = 0x00;
  	erkStruct_03F0_00AD.KeySenseIndicator = 0x03;
  	erkStruct_03F0_00AD.Bitmap[0]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[1]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[2]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[3]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[4]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[5]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[6]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Bitmap[7]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.Activation0       = 0x00000000;
  	erkStruct_03F0_00AD.Expiration0       = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.IRV0              = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.ERK0[0]           = 0xE9C85DAC;
  	erkStruct_03F0_00AD.ERK0[1]           = 0xF8270B6C;
  	erkStruct_03F0_00AD.ERK0[2]           = 0x28637330;
  	erkStruct_03F0_00AD.ERK0[3]           = 0xA7F4E186;
  	erkStruct_03F0_00AD.Activation1       = 0x00000000;
  	erkStruct_03F0_00AD.Expiration1       = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.IRV1              = 0xFFFFFFFF;
  	erkStruct_03F0_00AD.ERK1[0]           = 0xE9C85DAC;
  	erkStruct_03F0_00AD.ERK1[1]           = 0xF8270B6C;
  	erkStruct_03F0_00AD.ERK1[2]           = 0x28637330;
  	erkStruct_03F0_00AD.ERK1[3]           = 0xA7F4E186;
  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AD, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AD) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	/* run SHVs */
  	CfCwcStruct_t  cwcStruct_03F0_00AD;
  	uint32_t       shv_03F0_00AD[4];
  	uint32_t       expected_shv_03F0_00AD[4];
  	expected_shv_03F0_00AD[0]         = 0x94B3F1AE;
  	expected_shv_03F0_00AD[1]         = 0xE64FBC3E;
  	expected_shv_03F0_00AD[2]         = 0x0199B42D;
  	expected_shv_03F0_00AD[3]         = 0x762D8DA4;
  	memset(&cwcStruct_03F0_00AD, 0, sizeof(cwcStruct_03F0_00AD));
  	cwcStruct_03F0_00AD.Timestamp     = 0x00000000;
  	cwcStruct_03F0_00AD.ErkKeySense   = 0;
  	cwcStruct_03F0_00AD.NumProductIDs = 1;
  	cwcStruct_03F0_00AD.ProductID[0]  = 0x00AD;
  	cwcStruct_03F0_00AD.OutputUsage   = 0;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AD, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AD);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AD: %08X %08X %08X %08X\n", 
    		shv_03F0_00AD[0], shv_03F0_00AD[1], shv_03F0_00AD[2], shv_03F0_00AD[3]);*/ 
  	status = memcmp(expected_shv_03F0_00AD, shv_03F0_00AD, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	/* run SHV - unique */
  	cwcStruct_03F0_00AD.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AD[4];
  	expectedUniq_shv_03F0_00AD[0]     = 0x4489BF37;
  	expectedUniq_shv_03F0_00AD[1]     = 0x684C6622;
  	expectedUniq_shv_03F0_00AD[2]     = 0x432D400D;
  	expectedUniq_shv_03F0_00AD[3]     = 0xCCCEB7A7;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AD, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AD);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AD: %08X %08X %08X %08X\n", 
    		shv_03F0_00AD[0], shv_03F0_00AD[1], shv_03F0_00AD[2], shv_03F0_00AD[3]);*/ 
  	status = memcmp(expectedUniq_shv_03F0_00AD, shv_03F0_00AD, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	/* store ERK */
  	CfErkStruct_t erkStruct_03F0_00AE;
  	erkStruct_03F0_00AE.ProductRange      = 0x00AE;
  	erkStruct_03F0_00AE.AddressingMode    = 0x00;
  	erkStruct_03F0_00AE.KeySenseIndicator = 0x03;
  	erkStruct_03F0_00AE.Bitmap[0]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[1]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[2]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[3]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[4]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[5]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[6]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Bitmap[7]         = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.Activation0       = 0x00000000;
  	erkStruct_03F0_00AE.Expiration0       = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.IRV0              = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.ERK0[0]           = 0xBCCB213B;
  	erkStruct_03F0_00AE.ERK0[1]           = 0x8815F65E;
  	erkStruct_03F0_00AE.ERK0[2]           = 0x8C3973E3;
  	erkStruct_03F0_00AE.ERK0[3]           = 0x2A0737D0;
  	erkStruct_03F0_00AE.Activation1       = 0x00000000;
  	erkStruct_03F0_00AE.Expiration1       = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.IRV1              = 0xFFFFFFFF;
  	erkStruct_03F0_00AE.ERK1[0]           = 0xBCCB213B;
  	erkStruct_03F0_00AE.ERK1[1]           = 0x8815F65E;
  	erkStruct_03F0_00AE.ERK1[2]           = 0x8C3973E3;
  	erkStruct_03F0_00AE.ERK1[3]           = 0x2A0737D0;
  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AE, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AE) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	/* run SHVs */
  	CfCwcStruct_t  cwcStruct_03F0_00AE;
  	uint32_t       shv_03F0_00AE[4];
  	uint32_t       expected_shv_03F0_00AE[4];
  	expected_shv_03F0_00AE[0]         = 0xD88C812E;
  	expected_shv_03F0_00AE[1]         = 0xA7F4828F;
  	expected_shv_03F0_00AE[2]         = 0xD56F73C6;
  	expected_shv_03F0_00AE[3]         = 0x34AB4181;
  	memset(&cwcStruct_03F0_00AE, 0, sizeof(cwcStruct_03F0_00AE));
  	cwcStruct_03F0_00AE.Timestamp     = 0x00000000;
  	cwcStruct_03F0_00AE.ErkKeySense   = 0;
  	cwcStruct_03F0_00AE.NumProductIDs = 1;
  	cwcStruct_03F0_00AE.ProductID[0]  = 0x00AE;
  	cwcStruct_03F0_00AE.OutputUsage   = 0;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AE, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AE);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AE: %08X %08X %08X %08X\n", 
    		shv_03F0_00AE[0], shv_03F0_00AE[1], shv_03F0_00AE[2], shv_03F0_00AE[3]);*/ 
  	status = memcmp(expected_shv_03F0_00AE, shv_03F0_00AE, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}	
  
  	/* run SHV - unique */
  	cwcStruct_03F0_00AE.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AE[4];
  	expectedUniq_shv_03F0_00AE[0]     = 0x2885D0DC;
  	expectedUniq_shv_03F0_00AE[1]     = 0x15556235;
  	expectedUniq_shv_03F0_00AE[2]     = 0x1B9BED5C;
  	expectedUniq_shv_03F0_00AE[3]     = 0x0A70710B;
  	status = SMK_GenerateCWC_Struct(&cwcStruct_03F0_00AE, 0, 0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_GenerateCWC_Struct failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	status = SMK_FinalizeCWC(shv_03F0_00AE);
  	if (status != SMK_OK)
  	{
    		printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    		return -1;
  	}
  	/*printf("shv_03F0_00AE: %08X %08X %08X %08X\n", 
    		shv_03F0_00AE[0], shv_03F0_00AE[1], shv_03F0_00AE[2], shv_03F0_00AE[3]);*/ 
  	status = memcmp(expectedUniq_shv_03F0_00AE, shv_03F0_00AE, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}

    extern uint64_t getCFId();
    printf("SMK_Initialize complete, cfID=0x%016llX\n", getCFId());
    cf_inited = 1;

    return 0;
}
#endif 	//ENABLE_SMK
int configure_conaxvsc_encrypt(int dsc_idx, int parity, uint8_t *en_key)
{
	aui_hdl hdl_dsc = NULL;
	aui_hdl hdl_vsc = NULL;
	aui_hdl hdl_kl = NULL;
	aui_attr_dsc attr_dsc;
	aui_attr_kl attr_kl;
	aui_conaxvsc_attr attr_vsc;
	aui_conaxvsc_en_key_attr attr_key;
	aui_dsc_process_attr p_dsc;
	unsigned short pid_tmp[8];
	unsigned char iv[16];
	int i;

	//data flow: conaxvsc --> KL --> dsc, so we should first configure vsc, than kl and dsc
	memset(&attr_vsc, 0, sizeof(aui_conaxvsc_attr));
	attr_vsc.uc_dev_idx = 1;	//vsc idx 0 is already in use by decrypt playback module
	if (aui_find_dev_by_idx(AUI_MODULE_CONAXVSC, attr_vsc.uc_dev_idx, &hdl_vsc)) {
		if (aui_conaxvsc_open(&attr_vsc, &hdl_vsc)) {
			printf("aui_conaxvsc_open failed\n");
			return -1;
		}
	}	
	
	memset(&attr_kl, 0, sizeof(aui_attr_kl));
	attr_kl.uc_dev_idx = 1;		//kl idx 0 is already in use by decrypt playback module
	attr_kl.en_root_key_idx = 2;	//appoint parameter
	attr_kl.en_level = 1;		//key management in 1 level mode, this value must be 1.
	attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	attr_kl.en_key_ladder_type = AUI_KL_TYPE_CONAXVSC;
	if (aui_find_dev_by_idx(AUI_MODULE_KL, attr_kl.uc_dev_idx, &hdl_kl)) {
		if (aui_kl_open(&attr_kl, &hdl_kl)) {
			printf("aui_kl_open failed\n");
			return -1;
		}
	}

	memset(&attr_key, 0, sizeof(aui_conaxvsc_en_key_attr));
	attr_key.kl_handle = hdl_kl;
	attr_key.key_parity = !parity ? AUI_CONAXVSC_KEY_EVEN : AUI_CONAXVSC_KEY_ODD;
	memcpy(attr_key.en_key, en_key, AUI_CONAXVSC_EN_KEY_LEN);
	if (aui_conaxvsc_set_en_key(&hdl_vsc, &attr_key)) {		//set original key to vsc
		printf("aui_conaxvsc_set_en_key failed\n");
		return -1;
	}
	
	if (aui_find_dev_by_idx(AUI_MODULE_DSC, dsc_idx, &hdl_dsc)) {
		memset(&attr_dsc, 0, sizeof(aui_attr_dsc));
		attr_dsc.uc_dev_idx = dsc_idx;
		attr_dsc.uc_algo = AUI_DSC_ALGO_AES;
		attr_dsc.dsc_data_type = AUI_DSC_DATA_TS;

		if (aui_dsc_open(&attr_dsc, &hdl_dsc)) {
			printf("aui_dsc_open failed\n");
			return -1;
		}

		printf("%s,%d: ...\n", __func__, __LINE__);

		memset(&p_dsc, 0, sizeof(aui_dsc_process_attr));
		p_dsc.process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
		if (aui_dsc_process_attr_set(hdl_dsc, &p_dsc)) {
			printf("aui_dsc_process_attr_set failed\n");
			return -1;
		}
	}

	memset(&attr_dsc, 0, sizeof(aui_attr_dsc));
	attr_dsc.uc_algo = AUI_DSC_ALGO_AES;
	attr_dsc.dsc_data_type = AUI_DSC_DATA_TS;
	attr_dsc.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	attr_dsc.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr_dsc.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
	attr_dsc.en_en_de_crypt = AUI_DSC_ENCRYPT;
	attr_dsc.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	aui_kl_get(hdl_kl, AUI_KL_GET_KEY_POS, &attr_dsc.ul_key_pos);
	aui_kl_get(hdl_kl, AUI_KL_GET_KEY_SIZE, &attr_dsc.ul_key_len);
	attr_dsc.en_parity = !parity ?
		AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE : AUI_DSC_PARITY_MODE_ODD_PARITY_MODE;
	attr_dsc.pus_pids = pid_tmp;
	pid_tmp[0] = vpid;
	pid_tmp[1] = apid;
	attr_dsc.ul_pid_cnt = 2;
	attr_dsc.dsc_data_type = AUI_DSC_DATA_TS;
	memset(iv, 0, sizeof(iv));
	attr_dsc.puc_iv_ctr = iv;

	//attach key to dsc
	aui_dsc_attach_key_info2dsc(hdl_dsc, &attr_dsc);
		
	printf("parity:%s en_key:", !parity? "EVEN" : "ODD");
	for (i = 0; i < AUI_CONAXVSC_EN_KEY_LEN; i++)
		printf("%02x", en_key[i]);
	printf("\n");

	return 0;
}

void *do_playback(void *cbk)
{
	aui_hdl hdl_av = NULL;
	aui_hdl snd_hdl;

	FILE *file = NULL;
	uint8_t *buffer;

	aui_av_stream_info stream_info;
	aui_attrAV pst_attrAV;

	(void) cbk;

	/* open input file */
	file = fopen(infile, "rb");
	if (file == NULL) {
		printf("could not open file %s!\n", infile);
		return NULL;
	}

	buffer = (uint8_t *) malloc(buf_size);
	if (!buffer) {
		printf("malloc for playback buffer failed!\n");
		return NULL;
	}

	memset(&stream_info, 0, sizeof(aui_av_stream_info));
	memset(&pst_attrAV, 0, sizeof(aui_attrAV));

	stream_info.st_av_info.b_audio_enable = 1;
	stream_info.st_av_info.b_video_enable = 1;
	stream_info.st_av_info.b_pcr_enable = 1;
	stream_info.st_av_info.b_dmx_enable = 1;
	stream_info.st_av_info.ui_audio_pid = apid;
	stream_info.st_av_info.ui_video_pid = vpid;
	stream_info.st_av_info.ui_pcr_pid = ppid;
	stream_info.st_av_info.en_audio_stream_type = 11;
	stream_info.st_av_info.en_video_stream_type = 0;
	stream_info.st_av_info.en_spdif_type = AUI_SND_OUT_MODE_DECODED;
	stream_info.stream_type.dmx_id = 0; // playback via TSG
	stream_info.stream_type.data_type = AUI_AV_DATA_TYPE_RAM_TS;
	aui_av_init_attr(&pst_attrAV, &stream_info);

	if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl)) {
		aui_attr_snd attr_snd;
		memset(&attr_snd,0,sizeof(aui_attr_snd));
		aui_snd_open(&attr_snd, &snd_hdl);
	}
	aui_snd_mute_set(snd_hdl, 0);
	aui_snd_vol_set(snd_hdl, 50);

	aui_av_open(&pst_attrAV, &hdl_av);

	aui_av_start(hdl_av);

	while (1) {
		//usleep(2 * 1000);
		int total = 0;

		while (total < buf_size) {
			int to_read = buf_size - total;

			int cnt = fread(buffer, 1, to_read, file);
			if (cnt > 0) {
				aui_av_write(hdl_av, NULL, buffer, cnt);
				total += cnt;
			}

			if (feof(file)) {
				printf("end of file\n");
				fseek(file, 0 , SEEK_SET);
				continue;
			}
		}
	}

	aui_av_stop(hdl_av);

	return NULL;
}

int do_stream_record()
{
	aui_hdl hdl_dmx;
	aui_hdl hdl_dsc_dec;
	aui_hdl hdl_dsc_enc;
	aui_hdl hdl_ch;
	aui_hdl hdl_flt;
	aui_attr_dmx attr_dmx;
	aui_dmx_data_path dmx_path;
	aui_attr_dmx_channel attr_channel;
	aui_attr_dmx_filter attr_filter;

	int dsc_dec_idx = 0;
	int kl_dec_idx = 0;
	int vsc_idx = 0;
	int dmx_idx = 0;
	int dsc_enc_idx = 1;

	int parity = cur_key_idx % 2;

	/* open input file */
	out_fp = fopen(outfile, "wb");
	if (out_fp == NULL) {
		printf("could not open file %s!\n", outfile);
		return 0;
	}

	memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
	attr_dmx.uc_dev_idx = dmx_idx;
	if (aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_idx, &hdl_dmx)) {
		if (aui_dmx_open(&attr_dmx, &hdl_dmx)) {
			printf("dmx open failed\n");
			return -1;
		}
	}

	if (aui_dmx_start(hdl_dmx, &attr_dmx)) {
		printf("aui_dmx_start failed\n");
		return -1;
	}

	#ifdef ENABLE_SMK
	if(cf_flag == 1)
	{
		if(cf_smk_init() !=0)
			printf("cf_smk_init failed");
		printf("cf_smk_init finish!\n");
	}
	#endif
	
	if (configure_conaxvsc_cw_decrypt(dsc_dec_idx,
			kl_dec_idx, vsc_idx, dmx_idx)) {
		printf("configure_conaxvsc_cw_decrypt failed\n");
		return -1;
	}

	if (configure_conaxvsc_encrypt(dsc_enc_idx, parity,
			en_key[cur_key_idx])) {
		printf("configure_conaxvsc_encrypt failed\n");
		return -1;
	}

	if (aui_find_dev_by_idx(AUI_MODULE_DSC, dsc_dec_idx, &hdl_dsc_dec)) {
		printf("aui_find_dev_by_id dsc %d failed\n", dsc_dec_idx);
		return -1;
	}

	if (aui_find_dev_by_idx(AUI_MODULE_DSC, dsc_enc_idx, &hdl_dsc_enc)) {
		printf("aui_find_dev_by_id dsc %d failed\n", dsc_enc_idx);
		return -1;
	}

	memset(&dmx_path, 0, sizeof(aui_dmx_data_path));
	dmx_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY_EN_REC;
	dmx_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
	dmx_path.p_hdl_de_dev = hdl_dsc_dec;
	dmx_path.p_hdl_en_dev = hdl_dsc_enc;
	if (aui_dmx_data_path_set(hdl_dmx, &dmx_path)) {
		printf("aui_dmx_data_path_set failed\n");
		return -1;
	}

	memset(&attr_channel, 0, sizeof(attr_channel));
	attr_channel.us_pid = AUI_INVALID_PID;
	attr_channel.ul_pid_cnt = 3;
	attr_channel.us_pid_list[0] = vpid;
	attr_channel.us_pid_list[1] = apid;
	attr_channel.us_pid_list[2] = ppid;
	attr_channel.dmx_data_type = AUI_DMX_DATA_REC;
	if (aui_dmx_channel_open(hdl_dmx, &attr_channel, &hdl_ch)) {
        printf("aui_dmx_channel_open failed\n");
        return -1;
	}

	if (aui_dmx_channel_start(hdl_ch, &attr_channel)) {
		printf("aui_dmx_channel_start failed\n");
		return -1;
	}

	memset(&attr_filter, 0, sizeof(attr_filter));
	attr_filter.usr_data = NULL;
	attr_filter.p_fun_data_req_wtCB = rb_request_cb;
	attr_filter.p_fun_data_up_wtCB = rb_update_cb;
	if (aui_dmx_filter_open(hdl_ch, &attr_filter, &hdl_flt)) {
		printf("aui_dmx_filter_open failed\n");
		return -1;
	}

	if (aui_dmx_filter_start(hdl_flt, &attr_filter)) {
		printf("aui_dmx_filter_start failed\n");
		return -1;
	}

	buffer = malloc(buf_size);
	if (!buffer) {
		printf("malloc failed\n");
		return -1;
	}

	do_playback(NULL);

	aui_dmx_filter_stop(hdl_flt, NULL);
	aui_dmx_filter_close(hdl_flt);
	aui_dmx_channel_stop(hdl_ch, NULL);
	aui_dmx_channel_close(&hdl_ch);

	return 0;
}



void print_help(char *sz_appname)
{
	printf("Usage %s [OPTIONS]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t--in\t\t-i <inpath> input TS file path\n");
	printf("\t--out\t\t-o <outpath> output TS file path\n");
	printf("\t--sid\t\t-s <session_id> value between 0 and 5 (0 default)\n");
	printf("\t--kp\t\t-t <crypto_period> in number of blocks\n");
	printf("\t--enkey\t\t-k <en_key> recording key in hexstring format\n");
	printf("\t--vpid\t\t-v <pid> video pid\n");
	printf("\t--apid\t\t-a <pid> audio pid\n");
	printf("\t--ppid\t\t-p <pid> pcr pid\n");
	printf("\t--epid\t\t-e <pid> ecm pid\n");
	#ifdef ENABLE_SMK
	printf("\t--cf_flag\t-c CF enable or not(0 default)\n");
	#endif
	printf("\t-l --loop input\n");
	printf("\t-h --help\n");
}

int parse_input(int argc, char **argv)
{
	int c;
	int option_index = 0;
	
	#ifdef ENABLE_SMK
	char *short_options = "s:i:o:v:a:p:e:c:k:t:lh";
	#else
	char *short_options = "s:i:o:v:a:p:e:k:t:lh";
	#endif

	struct option long_options[] = {
		{"sid", required_argument, 0, 's'},
		{"in", required_argument, 0, 'i'},
		{"out", required_argument, 0, 'o'},
		{"vpid", required_argument, 0, 'v'},
		{"apid", required_argument, 0, 'a'},
		{"ppid", required_argument, 0, 'p'},
		{"epid", required_argument, 0, 'e'},
		#ifdef ENABLE_SMK
		{"cf_flag", required_argument, 0, 'c'},
		#endif
		{"kp", required_argument, 0, 't'},
		{"enkey", required_argument, 0, 'k'},
		{"help", required_argument, 0, 'h'},
	};

	memset(en_key, 0, MAX_KEY_CNT * AUI_CONAXVSC_EN_KEY_LEN);

	while ((c = getopt_long_only(argc, argv, short_options, long_options,
			&option_index)) != -1) {
		switch (c) {
		case 's':
			session_id = atoi(optarg);
			break;
		case 'i':
			strcpy(infile, optarg);
			break;
		case 'o':
			strcpy(outfile, optarg);
			break;
		case 'v':
			vpid = (int) strtol(optarg, NULL, 0);
			break;
		case 'a':
			apid = (int) strtol(optarg, NULL, 0);
			break;
		case 'p':
			ppid = (int) strtol(optarg, NULL, 0);
			break;
		case 'e':
			epid = (int) strtol(optarg, NULL, 0);
			break;
		#ifdef ENABLE_SMK
		case 'c':
			cf_flag = (int) strtol(optarg, NULL, 0);
			break;
		#endif
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
		case 'h':
		default:
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
	int ret = 0;
	int i, j;

	ret = parse_input(argc, argv);
	if (ret) {
		print_help(argv[0]);
		exit(-1);
	}

	if (max_key_cnt == 0) {
		print_help(argv[0]);
		exit(-1);
	}

	printf("Start playback/recording\n");
	printf("input file %s\n", infile);
	printf("demux parameters v:%04x a:%04x p:%04x e:%04x\n",
			vpid, apid, ppid, epid);
	printf("output file %s\n", outfile);
	printf("session_id %d\n", session_id);
	printf("crypto_period %d\n", crypto_period);
	for (j = 0; j < max_key_cnt; j++) {
		printf("en_key: ");
		for (i = 0; i < AUI_CONAXVSC_EN_KEY_LEN; i++)
			printf("%02x", en_key[j][i]);
		printf("\n");
	}

	do_stream_record();

	return 0;
}
