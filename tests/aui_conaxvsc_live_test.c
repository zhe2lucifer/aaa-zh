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

#define _GNU_SOURCE         
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
#include <aui_gpio.h>
#ifdef ENABLE_SMK
#include "SMK.h" 
#endif
#define FD_INVALID     -1
#define MAX_PID_CNT    32

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

static const int dec_idx = 0;
static const int enc_idx = 1;
static const int ply_idx = 2;

int32_t pids[32];
int pid_cnt = 0;

int tuner = 0;
int freq = 1500;
int sym_rate = 27500;
int vpid = 502;
int apid = 602;
int ppid = 577;
int epid = 0x1d57;
#ifdef ENABLE_SMK
int cf_flag = 0;
#endif
int playback = -1;
int record = -1;
int session_id = 0;
long time_last = 30;

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
static aui_hdl hdl_nim, hdl_dis_hd, hdl_dis_sd, hdl_decv, hdl_deca, hdl_dmx, hdl_snd, hdl_dsc_dec, hdl_vsc, hdl_kl, cf_hdl, target_hdl;

/*-------------------------------------------------------------------------------------------------------*/
/* set tuner#0 for M3529/M3527 */
/*-------------------------------------------------------------------------------------------------------*/
struct dvb_tsi_config {
	unsigned long ul_hw_init_val;
	unsigned long ul_input_src;
};

struct dvb_tsi_config tsi_cfg[1];

AUI_RTN_CODE tsi_config(struct dvb_tsi_config *tsi_cfg)
{
	struct dvb_tsi_config *cfg = tsi_cfg;
	
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;
	
	return 0;	
}

AUI_RTN_CODE dvbfe_nim_init_cb(void *pv_param)
{
	struct aui_nim_config *nim = (struct aui_nim_config *)pv_param;
	AUI_RTN_CODE ret;
	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;

	if (!nim)
		return AUI_RTN_FAIL;
		
	//Initial TSI
	ret = tsi_config(tsi_cfg);
	if (ret!=0) {
		printf("ERROR in tsi_config !\n");
	}

	/* first NIM : DVB-S with internal demod and ALi M3031 tuner */
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB1;
	tuner->i2c_base_addr = 0x40;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_C3505_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = 71;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->QPSK_Config = 0xf9;	
	
	return 0;
}

void do_nim_connect(void)
{
	AUI_RTN_CODE ret;
  aui_hdl hdl = 0;
  int count;
  int lock_status;
  struct aui_nim_attr nim_attr;
  struct aui_nim_connect_param param;
	
	ret = aui_nim_init(dvbfe_nim_init_cb);
	if (ret!=AUI_RTN_SUCCESS) {
		printf("aui_nim_init failure code =%d\n", (int)ret);
		return;
	} else {
		printf("aui_nim_init successfully\n");
	}
	memset(&nim_attr, 0, sizeof(struct aui_nim_attr));
	nim_attr.ul_nim_id = 0;
  nim_attr.en_dmod_type = AUI_NIM_QPSK;
  nim_attr.en_ter_std = AUI_STD_OTHER; 
  
	if (aui_find_dev_by_idx(AUI_MODULE_NIM, 0, &hdl) == 0) {
		printf("warning: nim0 already opened\n");
	}
	
	ret = aui_nim_open(&nim_attr, &hdl);
	if (ret!=AUI_RTN_SUCCESS) {
		printf("fe aui_nim_open failure code =%d\n", (int)ret);
		return;
	} else {
		printf("fe aui_nim_open successfully hdl=%d\n",(int)hdl);
	}
	
	memset(&param, 0, sizeof(struct aui_nim_connect_param));
	param.en_demod_type = AUI_NIM_QPSK;	
	param.ul_freq = freq;
	param.connect_param.sat.ul_symrate = sym_rate;
	param.connect_param.sat.ul_freq_band = AUI_NIM_LOW_BAND;
	param.connect_param.sat.fec = AUI_NIM_FEC_AUTO;
	param.connect_param.sat.ul_polar = AUI_NIM_POLAR_HORIZONTAL;
	param.connect_param.sat.ul_src = 0;
	ret = aui_nim_connect(hdl, &param);
	if (ret!=AUI_RTN_SUCCESS) {
		printf("aui_nim_connect failure code =%d\n", (int)ret);
		return;		
	} else {
		count = 5;
		lock_status = AUI_NIM_STATUS_UNLOCKED;
		while (count > 0) {
			ret = aui_nim_is_lock(hdl, &lock_status);
			if (ret!=AUI_RTN_SUCCESS) {
				printf("aui_nim_is_lock failure code =%d\n", (int)ret);
				return;		
			} else {
				if (lock_status == AUI_NIM_STATUS_LOCKED) {
					break;
				}
			}
			count --;
			sleep(1);
		}
		if (lock_status == AUI_NIM_STATUS_LOCKED) 
			printf("tuner lock !!\n");
		else
			printf("tuner time out !!\n");
	}
	hdl_nim = hdl;
	return;
}

void do_nim_disconnect(void)
{
	AUI_RTN_CODE ret;
	ret = aui_nim_close(hdl_nim);
	if (ret!=AUI_RTN_SUCCESS) {
		printf("aui_nim_close failure code =%d\n", (int)ret);
	} 
	return;
}
/*-------------------------------------------------------------------------------------------------------*/
/* tsi dmx connect */
/*-------------------------------------------------------------------------------------------------------*/
void do_tsi_dmx_connect(void)
{
	aui_attr_tsi attr_tsi;
	aui_hdl tsi_hdl;
	int channelno;
	int dmx_id;
	
  memset(&attr_tsi, 0, sizeof(aui_attr_tsi));
  attr_tsi.ul_init_param = tsi_cfg[0].ul_hw_init_val;
  channelno = AUI_TSI_CHANNEL_0;
  dmx_id = AUI_TSI_OUTPUT_DMX_0;  
	if (aui_find_dev_by_idx(AUI_MODULE_TSI, 0 , &tsi_hdl)!=0) {
		if (aui_tsi_open(&tsi_hdl)) {
			printf("aui_tsi_open error !!\n");
			return;
		}
	}

	printf("tsi %d src %ld src_conf 0x%lx channel %d dmx %d\n", 1, tsi_cfg[0].ul_input_src,
	        attr_tsi.ul_init_param, channelno, dmx_id);

	if (aui_tsi_src_init(tsi_hdl, tsi_cfg[0].ul_input_src, &attr_tsi)) {
		printf("aui_tsi_src_init error \n");
		aui_tsi_close(tsi_hdl);
		return;
	}
	if (aui_tsi_route_cfg(tsi_hdl, tsi_cfg[0].ul_input_src, channelno, dmx_id)) {
		printf("aui_tsi_route_cfg error \n");
		aui_tsi_close(tsi_hdl);
		return;
	}
	return;
}
/*-------------------------------------------------------------------------------------------------------*/
int display(aui_hdl *phdl_dis_hd, aui_hdl *phdl_dis_sd, int cmd)
{
	aui_attr_dis attr_dis, attr_dis_sd;
	AUI_RTN_CODE aui_ret;
	
	if (cmd==1) {
	  memset(&attr_dis, 0, sizeof(aui_attr_dis));
	  attr_dis.uc_dev_idx = AUI_DIS_HD;
	  if (aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_HD, phdl_dis_hd)) {
	  	aui_ret = aui_dis_open(&attr_dis, phdl_dis_hd);
	    if (aui_ret!=AUI_RTN_SUCCESS) {
	    	printf("\n aui_dis_open HD fail (%d)\n", (int)aui_ret);
	      return -1;
	    } else {
	    	printf("\n aui_dis_open success (0x%x)\n", (unsigned int)(*phdl_dis_hd));
	    }
	  } 
	  
	  memset(&attr_dis_sd, 0, sizeof(aui_attr_dis));
	  attr_dis_sd.uc_dev_idx = AUI_DIS_SD;
	  if(aui_find_dev_by_idx(AUI_MODULE_DIS, AUI_DIS_SD, phdl_dis_sd)) {
	  	aui_ret = aui_dis_open(&attr_dis_sd, phdl_dis_sd);
	    if (aui_ret!=AUI_RTN_SUCCESS) {
	    	printf("\n aui_dis_open HD fail (%d)\n", (int)aui_ret);
	      return -1;
	    }
	    else {
	    	printf("\n aui_dis_open success (0x%x) \n", (unsigned int)(*phdl_dis_sd));
	    }
	  }
	} else {
		if (phdl_dis_sd) {
		  aui_ret = aui_dis_close(NULL, phdl_dis_sd);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_dis_close failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_dis_close success\n");
		  }
		}
		if (phdl_dis_hd) {
		  aui_ret = aui_dis_close(NULL, phdl_dis_hd);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_dis_close failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_dis_close success\n");
		  }
		}
	}
	return 0;
}

int decv(aui_hdl *phdl_decv, int cmd)
{
	aui_attr_decv attr_decv;
	AUI_RTN_CODE aui_ret;
	
	if (cmd==1) {
		memset(&attr_decv, 0, sizeof(aui_attr_decv));
	  attr_decv.callback_nodes[0].type= AUI_DECV_CB_ERROR;
	  attr_decv.callback_nodes[0].callback = NULL;
	  attr_decv.en_chg_mode = AUI_CHG_BLACK;
		attr_decv.uc_dev_idx = 0;
		if (aui_find_dev_by_idx(AUI_MODULE_DECV, attr_decv.uc_dev_idx, phdl_decv)) {
			aui_ret = aui_decv_open(&attr_decv, phdl_decv);
	    if (aui_ret!=AUI_RTN_SUCCESS) {
	    	printf("\n aui_decv_open fail (%d)\n", (int)aui_ret);
	      return -1;
	    }
		}
	} else {
		if (phdl_decv) {
		  aui_ret = aui_decv_stop(*phdl_decv);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_decv_stop failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_decv_stop success\n");
		  }
		  aui_ret = aui_decv_close(NULL, phdl_decv);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_decv_close failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_decv_close success\n");
		  }
		}
	}
	printf("decv success\n");
	return 0;
}

int deca(aui_hdl *phdl_deca, int cmd)
{
	aui_attr_deca attr_deca;
	AUI_RTN_CODE aui_ret;
	
	if (cmd==1) {
		memset(&attr_deca, 0, sizeof(aui_attr_deca));
		attr_deca.uc_dev_idx = 0;
		if (aui_find_dev_by_idx(AUI_MODULE_DECA, attr_deca.uc_dev_idx, phdl_deca)) {
			aui_ret = aui_deca_open(&attr_deca, phdl_deca);
	    if (aui_ret!=AUI_RTN_SUCCESS) {
	    	printf("\n aui_deca_open fail (%d)\n", (int)aui_ret);
	      return -1;
	    }
		}
	} else {
		if (phdl_deca) {
		  aui_ret = aui_deca_stop(*phdl_deca, NULL);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_deca_stop failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_deca_stop success\n");
		  }
		  aui_ret = aui_deca_close(*phdl_deca);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_deca_close failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_deca_close success\n");
		  }
		}
	}
	printf("deca success\n");
	return 0;
}

int snd(aui_hdl *phdl_snd, int cmd)
{
	aui_attr_snd attr_snd;
	AUI_RTN_CODE aui_ret;
	
	if (cmd==1) {
	  memset(&attr_snd, 0, sizeof(aui_attr_snd));
	  if (aui_find_dev_by_idx(AUI_MODULE_SND, 0, phdl_snd)) {
			aui_ret = aui_snd_open(&attr_snd, phdl_snd);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_snd_open failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_snd_open success\n");
		  }
		}
		
	  aui_ret = aui_snd_start(*phdl_snd, NULL);
	  if (aui_ret!=AUI_RTN_SUCCESS) {
			printf("aui_snd_start failed (%d)\n", (int)aui_ret);
			return -1;
		} else {
	    printf("aui_snd_start success\n");
	  }
	} else {
		if (phdl_snd) {
		  aui_ret = aui_snd_stop(*phdl_snd, NULL);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_snd_stop failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_snd_stop success\n");
		  }
		  aui_ret = aui_snd_close(*phdl_snd);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
				printf("aui_snd_close failed (%d)\n", (int)aui_ret);
				return -1;
			} else {
		    printf("aui_snd_close success\n");
		  }
		}
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
			printf("parse CW\n");
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
			printf("parse CF \n");
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
		aui_hdl hdl_kl, aui_hdl hdl_cf_kl, aui_hdl hdl_target_kl, unsigned short epid)
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
	g_context.session_id = 0;	//data->session_id;	???
	g_context.pid = epid;

	return 0;
}

int configure_conaxvsc_cw_decrypt(int dsc_idx, int kl_idx, int vsc_idx, int dmx_idx)
{
	aui_attr_kl attr_kl;
	aui_attr_dsc attr_dsc[2];
	aui_conaxvsc_attr attr_vsc;
	unsigned short pid_tmp[8];
	unsigned char iv[16];

	/*if (aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_idx, &hdl_dmx)) {
		printf("aui_find_dev_by_idx dmx %d failed\n", dmx_idx);
		return -1;
	}*/

	(void) dmx_idx;

	memset(&attr_vsc, 0, sizeof(aui_conaxvsc_attr));
	attr_vsc.uc_dev_idx = vsc_idx;
	if (aui_find_dev_by_idx(AUI_MODULE_CONAXVSC,
			attr_vsc.uc_dev_idx, &hdl_vsc)) {
		if (aui_conaxvsc_open(&attr_vsc, &hdl_vsc)) {
			printf("aui_conaxvsc_open failed\n");
			return -1;
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
				printf("aui_kl_open cf_hdl fail\n");
				return -1;
			}
		}

		attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
		attr_kl.uc_dev_idx = 2;
		if (aui_find_dev_by_idx(AUI_MODULE_KL,
				attr_kl.uc_dev_idx, &cf_hdl)) {
			if (aui_kl_open(&attr_kl, &cf_hdl)) {
				printf("aui_kl_open target_hdl fail\n");
				return -1;
			}
		}
	}
	#endif
	memset(&attr_dsc[0], 0, sizeof(aui_attr_dsc));
	attr_dsc[0].uc_dev_idx = dsc_idx;
	attr_dsc[0].uc_algo = AUI_DSC_ALGO_CSA;
	attr_dsc[0].dsc_data_type = AUI_DSC_DATA_TS;
	if (aui_find_dev_by_idx(AUI_MODULE_DSC, attr_dsc[0].uc_dev_idx, &hdl_dsc_dec)) {
		if (aui_dsc_open(&attr_dsc[0], &hdl_dsc_dec)) {
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
	aui_dsc_attach_key_info2dsc(hdl_dsc_dec, &attr_dsc[1]);

	vsc_start_ecm_filter(hdl_vsc, hdl_dmx, hdl_kl, cf_hdl, target_hdl, epid);

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
#endif //ENABLE_SMK
int dmx(aui_hdl *phdl_dmx, int cmd, unsigned int vpid, unsigned apid, unsigned ppid)
{
	aui_attr_dmx attr_dmx;
	aui_dmx_stream_pid pid_list;
	aui_dmx_data_path path;
	AUI_RTN_CODE aui_ret;
	
	if (cmd==1) {
		memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
		memset(&pid_list, 0, sizeof(aui_dmx_stream_pid));
		attr_dmx.uc_dev_idx = 0;
		printf("aui_dmx_open %d\n", attr_dmx.uc_dev_idx);
		if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, phdl_dmx)) {
			aui_ret = aui_dmx_open(&attr_dmx, phdl_dmx);
		  if (aui_ret!=AUI_RTN_SUCCESS) {
		  	printf("\n aui_dmx_open fail (%d)\n", (int)aui_ret);
		    return -1;
		  } else {
	      printf("aui_dmx_open success\n");
	    }
		}
		aui_ret = aui_dmx_start(*phdl_dmx, &attr_dmx);
		if (aui_ret!=AUI_RTN_SUCCESS) {
			printf("aui_dmx_start failed (%d)\n", (int) aui_ret);
			return -1;
		} else {
	    printf("aui_dmx_start success\n");
	  } 
	#ifdef ENABLE_SMK
	if(cf_flag == 1)
	{
		if(cf_smk_init() !=0)
			printf("cf_smk_init failed");
		printf("cf_smk_init finish!\n");
	}
	#endif
	if (configure_conaxvsc_cw_decrypt(0, 0, 0, 0)) {
		printf("configure_conaxvsc_cw_decrypt failed\n");
		return -1;
	}
		
	pid_list.ul_pid_cnt=4;
		pid_list.stream_types[0]=AUI_DMX_STREAM_VIDEO;
		pid_list.stream_types[1]=AUI_DMX_STREAM_AUDIO;
		pid_list.stream_types[2]=AUI_DMX_STREAM_PCR;
		pid_list.stream_types[3]=AUI_DMX_STREAM_AUDIO_DESC;
	
		pid_list.aus_pids_val[0]=vpid;
		pid_list.aus_pids_val[1]=apid;
		pid_list.aus_pids_val[2]=ppid;
		pid_list.aus_pids_val[3]=8191;
		printf("dmx dev %d opened, pids %d %d %d\n", 0, vpid, apid, ppid);
	
		aui_ret = aui_dmx_set(*phdl_dmx, AUI_DMX_STREAM_CREATE_AV, &pid_list);
		if (aui_ret!=AUI_RTN_SUCCESS) {
			printf("aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
			return -1;
		} else {
	    printf("aui_dmx_set AUI_DMX_STREAM_CREATE_AV success\n");
	  }
		
		memset(&path, 0, sizeof(aui_dmx_data_path));
		path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
		path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
		path.p_hdl_de_dev = hdl_dsc_dec;
		//path.p_hdl_en_dev = hdl_dsc_enc;
		if (aui_dmx_data_path_set(hdl_dmx, &path)) {
			printf("aui_dmx_data_path_set failed\n");
			return -1;
		}else {
	    printf("aui_dmx_data_path_set success\n");
	  }


	} else {
	  aui_ret = aui_dmx_set(*phdl_dmx, AUI_DMX_STREAM_DISABLE, (void *)1);
	  if (aui_ret!=AUI_RTN_SUCCESS) {
			printf("aui_dmx_set [AUI_DMX_STREAM_DISABLE] failed (%d)\n", (int)aui_ret);
			return -1;
		} else {
	    printf("aui_dmx_set [AUI_DMX_STREAM_DISABLE] success\n");
	  }
	  aui_ret = aui_dmx_close(*phdl_dmx);
	  if (aui_ret!=AUI_RTN_SUCCESS) {
			printf("aui_dmx_close failed (%d)\n", (int)aui_ret);
			return -1;
		} else {
	    printf("aui_dmx_close success\n");
	  }
	}
	
	return 0;
}

void do_stream_play(int count)
{
	int ret;
	AUI_RTN_CODE aui_ret;
	
	ret = display(&hdl_dis_hd, &hdl_dis_sd, 1);
	if (ret!=0) {
		printf("display open fail \n");
		return;
	}
	ret = decv(&hdl_decv, 1);
	if (ret!=0) {
		printf("decode video fail \n");
		return;
	}
	ret = deca(&hdl_deca, 1);
	if (ret!=0) {
		printf("decode video fail \n");
		return;
	}
	
	aui_ret = aui_decv_decode_format_set(hdl_decv, 0);	//video MPEG2
  if (aui_ret!=AUI_RTN_SUCCESS) {
  	printf("\n aui_decv_decode_format_set fail (%d)\n", (int)aui_ret);
    return;
  }
	aui_ret = aui_deca_type_set(hdl_deca, 11);		//audio MP3
  if (aui_ret!=AUI_RTN_SUCCESS) {
  	printf("\n aui_deca_type_set fail (%d)\n", (int)aui_ret);
    return;
  }
	ret = dmx(&hdl_dmx, 1, vpid, apid, ppid);
	if (ret!=0) {
		printf("demux set video pid, audio pid and pcr pid fail \n");
		return;
	}
	
  aui_ret = aui_decv_sync_mode(hdl_decv, AUI_DECV_SYNC_PTS);
  if (aui_ret!=AUI_RTN_SUCCESS) {
		printf("aui_decv_sync_mode  AUI_DMX_SET_AVSYNC_MODE failed\n");
		return;
	} else {
    printf("aui_decv_sync_mode AUI_DMX_SET_AVSYNC_MODE success\n");
  }
  aui_ret = aui_decv_start(hdl_decv);
  if (aui_ret!=AUI_RTN_SUCCESS) {
		printf("aui_decv_start failed (%d)\n", (int)aui_ret);
		return;
	} else {
    printf("aui_decv_start success\n");
  }
	  
  aui_ret = aui_deca_start(hdl_deca, NULL);
  if (aui_ret!=AUI_RTN_SUCCESS) {
		printf("aui_deca_start failed (%d)\n", (int)aui_ret);
		return;
	} else {
    printf("aui_deca_start success\n");
  }
  ret = snd(&hdl_snd, 1);
	if (ret!=0) {
		printf("sound open fail \n");
		return;
	}
	aui_ret = aui_snd_mute_set(hdl_snd, 0);
  if (aui_ret!=AUI_RTN_SUCCESS) {
		printf("aui_snd_mute_set failed (%d)\n", (int)aui_ret);
		return;
	} else {
    printf("aui_snd_mute_set success\n");
  }
	aui_ret = aui_snd_vol_set(hdl_snd, 50);
  if (aui_ret!=AUI_RTN_SUCCESS) {
		printf("aui_snd_vol_set failed (%d)\n", (int)aui_ret);
		return;
	} else {
    printf("aui_snd_vol_set success\n");
  }
  aui_ret = aui_dmx_set(hdl_dmx, AUI_DMX_STREAM_ENABLE, NULL);
  if (aui_ret!=AUI_RTN_SUCCESS) {
		printf("aui_dmx_set failed (%d)\n", (int)aui_ret);
		return;
	} else {
    printf("aui_dmx_set AUI_DMX_STREAM_ENABLE success\n");
  }
  
  while (count>0) {
  	count --;
  	sleep(1);
  }
  
	ret = deca(&hdl_deca, 0);
	if (ret!=0) {
		printf("decode audio close fail \n");
		return;
	}
	ret = snd(&hdl_snd, 0);
	if (ret!=0) {
		printf("sound close fail \n");
		return;
	}
	ret = decv(&hdl_decv, 0);
	if (ret!=0) {
		printf("decode video close fail \n");
		return;
	}
	ret = display(&hdl_dis_hd, &hdl_dis_sd, 0);
	if (ret!=0) {
		printf("display close fail \n");
		return;
	}
	ret = dmx(&hdl_dmx, 0, vpid, apid, ppid);
	if (ret!=0) {
		printf("demux close fail \n");
		return;
	}
	
	return;
}

/*-------------------------------------------------------------------------------------------------------*/
void print_help(char *sz_appname)
{
	printf("Usage %s [OPTIONS]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t--freq\t\t-f <freq> frequency (950~2150MHz)\n");
	printf("\t--sr\t\t-r <symbol rate> symbol rate\n");
	printf("\t--sid\t\t-s <session_id> value between 0 and 5 (0 default)\n");
	printf("\t--vpid\t\t-v <pid> video pid\n");
	printf("\t--apid\t\t-a <pid> audio pid\n");
	printf("\t--ppid\t\t-p <pid> pcr pid\n");
	printf("\t--epid\t\t-e <pid> ecm pid\n");
	printf("\t--time\t\t-t duration for playing\n");
	#ifdef ENABLE_SMK
	printf("\t--cf_flag\t-c CF enable or not(0 default)\n");
	#endif
	printf("\t--help\t\t-h help\n");
}

int parse_input(int argc, char **argv)
{
	int c;
	int option_index = 0;
	#ifdef ENABLE_SMK
	char *short_options = "s:f:r:t:v:a:p:e:c:h";
	#else
	char *short_options = "s:f:r:t:v:a:p:e:h";
	#endif

	struct option long_options[] = {
		{"sid", required_argument, 0, 's'},
		{"freq", required_argument, 0, 'f'},
		{"sr", required_argument, 0, 'r'},
		{"vpid", required_argument, 0, 'v'},
		{"apid", required_argument, 0, 'a'},
		{"ppid", required_argument, 0, 'p'},
		{"epid", required_argument, 0, 'e'},
		#ifdef ENABLE_SMK
		{"cf_flag", required_argument, 0, 'c'},
		#endif
		{"help", required_argument, 0, 'h'},
	};

	while ((c = getopt_long_only(argc, argv, short_options, long_options,
			&option_index)) != -1) {
		switch (c) {
		case 's':
			session_id = atoi(optarg);
			break;
		case 'f':
			freq = atoi(optarg);
			break;
		case 'r':
			sym_rate = atoi(optarg);
			break;
		case 't':
			time_last = atol(optarg);
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

	ret = parse_input(argc, argv);
	if (ret) {
		print_help(argv[0]);
		exit(-1);
	}

	printf("Start Conax VSC playback ...\n");
	printf("frequency %d\n", freq);
	printf("symbol rate %d\n", sym_rate);
	printf("duration %ld seconds\n", time_last);
	printf("demux parameters v:%04x a:%04x p:%04x e:%04x\n",
			vpid, apid, ppid, epid);
	printf("session_id %d\n", session_id);
	
	do_nim_connect();
	do_tsi_dmx_connect();
	do_stream_play(time_last);
	do_nim_disconnect();

	return 0;
}
