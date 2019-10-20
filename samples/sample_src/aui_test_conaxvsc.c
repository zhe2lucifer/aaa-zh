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
#include <aui_nim.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_av.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_nim.h"
#include "aui_test_conaxvsc.h"
#include "aui_help_print.h"

#include <aui_kl.h>
#include <aui_dsc.h>
#include <aui_conaxvsc.h>
#include <aui_misc.h>
#include <aui_pvr.h>
#include <aui_ca_pvr.h>

#ifdef _SMI_CF_ENABLE_
#include "SMK.h"
#endif

enum
{
	 VSC_PI_HOST_VER = 0x10,
	 VSC_PI_CAT = 0x11,
	 VSC_PI_EMM = 0x12,
	 VSC_PI_ECM = 0x14,
	 VSC_PI_RET_CHAN_CONFIG = 0x1B,
	 VSC_PI_CA_STATUS_SELECT = 0x1C,
	 VSC_PI_PIN_IN = 0x1D,
 	 VSC_PI_CASS_VER = 0x20,
	 VSC_PI_CA_DESC_EMM =  0x22,
	 VSC_PI_SESSION_INFO = 0x23,
	 VSC_PI_CW =  0x25,
	 VSC_PI_CA_SYS_ID =  0x28,
	 VSC_PI_REQ_BLOCK_SIZE = 0x65,
	 VSC_PI_REQ_CARD_NUMBER = 0x66,
	 VSC_PI_REQ_SEQUENCE_NUMBER = 0x67,
	 VSC_PI_RESET_SESSION = 0x69,
	 VSC_PI_CRYPTO_BLOCK_SIZE = 0x73,
	 VSC_PI_CARD_NUMBER = 0x74,
	 VSC_PI_SEQUENCE_NUMBER =  0x75,
	 VSC_PI_HOST_DATA = 0x80,
	 VSC_PI_REQ_VSC_VERSION = 0xC1,
	 VSC_PI_CF =  0xCF,
	 VSC_PI_VSC_VERSION = 0xD1
};

#ifdef AUI_LINUX
void aui_os_task_sleep(unsigned int tims)
{
	usleep(tims*1000);
}
#endif

static struct ali_aui_hdls *s_p_hdls=NULL;
int cnx_vsc(aui_hdl *hdl_vsc)
{
	aui_conaxvsc_attr vsc_attr;
	MEMSET(&vsc_attr,0,sizeof(aui_conaxvsc_attr));

	if(aui_conaxvsc_open(&vsc_attr,hdl_vsc))
	{
		AUI_PRINTF("aui_conaxvsc_open failed");
		return 1;
	}
	return 0;
}

int cnx_kl(void)
{
	aui_attr_kl attr_kl;
	memset(&attr_kl,0,sizeof(aui_attr_kl));
	attr_kl.uc_dev_idx = 0;
	attr_kl.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
#ifdef VSC_SMI
	attr_kl.en_level = AUI_KL_KEY_FIVE_LEVEL;
#else
	attr_kl.en_level = AUI_KL_KEY_THREE_LEVEL;
#endif
	attr_kl.en_root_key_idx = AUI_KL_ROOT_KEY_0_0;
	attr_kl.en_key_ladder_type = AUI_KL_TYPE_CONAXVSC;
	
	if (aui_kl_open(&attr_kl, &s_p_hdls->kl_hdl)) 
	{
	    AUI_PRINTF("aui_kl_open fail!\n");
	    return 1;
	}

	return 0;
}

int cnx_dsc(unsigned short vpid,unsigned short apid)
{
	aui_attr_dsc attr_dsc;
	unsigned short pidlist[8];

	if(aui_find_dev_by_idx(AUI_MODULE_KL, 0, &s_p_hdls->kl_hdl)){
		AUI_PRINTF("get kl handler fail!\n");
		return 1;
	}
	
	memset(&attr_dsc,0,sizeof(aui_attr_dsc));
	attr_dsc.uc_dev_idx=0; 
	attr_dsc.dsc_data_type=AUI_DSC_DATA_TS;	
	attr_dsc.en_en_de_crypt=AUI_DSC_DECRYPT;  
	attr_dsc.uc_algo=AUI_DSC_ALGO_CSA;
	attr_dsc.csa_version=AUI_DSC_CSA2;
	attr_dsc.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;

	if (aui_dsc_open(&attr_dsc, &s_p_hdls->dsc_hdl)) 
	{
	    AUI_PRINTF("aui_dsc_open fail!\n");
	    return 1;
	}

	pidlist[0] = vpid;
	pidlist[1] = apid;
	attr_dsc.ul_pid_cnt=2;
	attr_dsc.pus_pids= pidlist;
	attr_dsc.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
	attr_dsc.en_parity=AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
	attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;
	aui_kl_get(s_p_hdls->kl_hdl, AUI_KL_GET_KEY_POS, &attr_dsc.ul_key_pos);
	aui_kl_get(s_p_hdls->kl_hdl, AUI_KL_GET_KEY_SIZE, &attr_dsc.ul_key_len);
	//AUI_PRINTF("cnx_dsc key:%u,%u\n",attr_dsc.ul_key_pos,attr_dsc.ul_key_len);

	if (aui_dsc_attach_key_info2dsc(s_p_hdls->dsc_hdl, &attr_dsc)) 
	{
		AUI_PRINTF("aui_dsc_attach_key_info2dsc fail!\n");
		return 1;
	}
	return 0;
}

static unsigned short cnx_pvr_callback(unsigned short program_id,aui_hdl *p_dsc_handler)
{
	*p_dsc_handler = s_p_hdls->dsc_hdl;
	return 0;
}

int cnx_PVR_init(unsigned char *pvr_buffer,unsigned int pvr_buffer_len)
{
	aui_pvr_disk_attach_info attach_param;
	
	#ifdef AUI_LINUX
	aui_pvr_init_param ini_param;
	pvr_buffer = (unsigned char *)malloc(pvr_buffer_len);
	if(pvr_buffer == 0)
	{
		AUI_PRINTF("pvr_buffer is %p\n",pvr_buffer);
		return 1;
	}
	memset(&ini_param,0,sizeof(ini_param));
	ini_param.max_rec_number = 2;
	ini_param.max_play_number = 1;
	ini_param.ac3_decode_support = 1;
	ini_param.continuous_tms_en = 0;
	ini_param.debug_level   = AUI_PVR_DEBUG_NONE;
	STRCPY(ini_param.dvr_path_prefix,"ALIDVRS2");
	STRCPY(ini_param.info_file_name,"info.dvr");	
	STRCPY(ini_param.info_file_name_new,"info3.dvr");
	STRCPY(ini_param.ts_file_format,"dvr");	
	STRCPY(ini_param.ts_file_format_new, "ts");	
	STRCPY(ini_param.ps_file_format,"mpg");	
	STRCPY(ini_param.test_file1,"test_write1.dvr");
	STRCPY(ini_param.test_file2,"test_write2.dvr");
	STRCPY(ini_param.storeinfo_file_name,"storeinfo.dvr");
	ini_param.record_min_len = 15;
	ini_param.tms_time_max_len = 7200;
	ini_param.tms_file_min_size = 2;
	ini_param.prj_mode  = AUI_PVR_DVBS2; 
	ini_param.cache_addr = (unsigned int)pvr_buffer;
	ini_param.cache_size = pvr_buffer_len;	
	
	if(aui_pvr_init(&ini_param))
	{
		AUI_PRINTF("aui_pvr_init fail!\n");
		return 1;
	}
	#endif

	MEMSET(&attach_param, 0, sizeof(aui_pvr_disk_attach_info));
#ifdef AUI_LINUX
	STRCPY(attach_param.mount_name,"/mnt/usb/sda1");
#else
	STRCPY(attach_param.mount_name,"/mnt/uda1");
#endif
	attach_param.disk_usage = AUI_PVR_REC_AND_TMS_DISK;
	attach_param.sync = 1;
	attach_param.init_list = 1;
	attach_param.check_speed = 1;
	if(aui_pvr_disk_attach(&attach_param))
	{
		AUI_PRINTF("aui_pvr_disk_attach fail!\n");
		return 1;
	}

	return 0;
}

int cnx_PVR_record(aui_hdl *pvr_handler,unsigned short vpid,unsigned short apid,unsigned short ppid,unsigned short dpid)
{
	aui_record_prog_param st_arp;
	aui_ca_pvr_callback ca_pvr_callback;

#ifdef AUI_LINUX
	aui_ca_pvr_config config;
	MEMSET(&config,0,sizeof(aui_ca_pvr_config));
#endif

	MEMSET(&st_arp,0,sizeof(st_arp));
	st_arp.av_flag = 1;
	st_arp.dmx_id = 0;
	st_arp.is_tms_record = AUI_REC_MODE_NORMAL;
	st_arp.rec_type = AUI_PVR_REC_TYPE_TS;
	st_arp.pid_info.audio_count = 1;
	st_arp.pid_info.ttx_pid_count = 1;
	st_arp.h264_flag = AUI_DECV_FORMAT_MPEG;
	st_arp.pid_info.video_pid =vpid;
	st_arp.pid_info.pcr_pid =ppid;
	st_arp.pid_info.audio_pid[0] = apid;
	st_arp.pid_info.audio_type[0] =AUI_DECA_STREAM_TYPE_MPEG1;
	st_arp.pid_info.ttx_pids[0] = dpid;
	st_arp.is_reencrypt = 1;
	st_arp.rec_special_mode = AUI_PVR_CAS9_RE_ENCRYPTION;
	//st_arp.rec_special_mode = AUI_PVR_NONE;
	st_arp.is_scrambled = 0;
	st_arp.ca_mode = 0;
#ifdef AUI_LINUX
	config.special_mode = st_arp.rec_special_mode;
	aui_ca_pvr_init_ext(&config);
#else
	aui_ca_pvr_init();
#endif
	ca_pvr_callback.fp_pure_data_callback = NULL;
	ca_pvr_callback.fp_ts_callback = cnx_pvr_callback;
	aui_ca_register_callback(&ca_pvr_callback);
	sprintf(st_arp.folder_name,"/%s","pvr1");
	AUI_PRINTF("pvr recorder filename:%s\n",st_arp.folder_name);
	if(aui_pvr_rec_open(&st_arp,pvr_handler))
	{
		AUI_PRINTF("pvr recorder open fail\n");
		return 1;
	}
	return 0;
}

int cnx_PVR_play(aui_hdl *pvr_handler,unsigned short int play_index,aui_pvr_play_state play_state)
{
	aui_ply_param st_app;
	MEMSET(&st_app,0,sizeof(st_app));
	st_app.dmx_id =2;
	st_app.index = play_index;
	st_app.live_dmx_id =0;
#ifdef AUI_LINUX
	STRCPY(st_app.path,"/mnt/usb/sda1/ALIDVRS2/pvr1");
#else
	STRCPY(st_app.path,"/mnt/uda1/ALIDVRS2/pvr1");
#endif
	st_app.preview_mode =0;
	st_app.speed  = AUI_PVR_PLAY_SPEED_1X;
	st_app.start_mode = AUI_P_OPEN_FROM_HEAD;
	st_app.start_pos =0 ;
	st_app.start_time =0;
	st_app.state = play_state;

	if(aui_pvr_ply_open(&st_app,pvr_handler))
	{
		AUI_PRINTF("pvr play open fail\n");
		return 1;
	}
	return 0;
}

static unsigned char *vsc_find_next_token(unsigned char *buf, unsigned char tag, int *len)
{
	unsigned char *p = buf;
	int PI;
	int PL;
	unsigned char *PV;

	while (*len >= 2) {
		PI = p[0];
		PL = p[1];
		PV = &p[2];

		if (PI == tag)
			break;

		*len -= (PL + 2);
		p = &PV[PL];
	}
	return  p;
}

unsigned char even_cw[16];
unsigned char odd_cw[16];

long vsc_process_ecm(aui_hdl flt_hdl, unsigned char *buf,unsigned long len, void *data, void *reserved)
{
	unsigned char *p;
	int PL;
	unsigned char *PV;
	int size;
	aui_hdl hdl_vsc = 0;
	aui_conaxvsc_decw_attr attr;
	aui_conaxvsc_tx_buf *command = (aui_conaxvsc_tx_buf *)malloc(sizeof(aui_conaxvsc_tx_buf));
	aui_conaxvsc_tx_buf *response = (aui_conaxvsc_tx_buf *)malloc(sizeof(aui_conaxvsc_tx_buf));
	int sw1 = 0;
	int sw2 = 0;

	if (len < 30) {
		AUI_PRINTF("read error ret=%d\n", len);
		goto exit;
	}

	memset(&command[0], 0, sizeof(aui_conaxvsc_tx_buf));
	memset(&response[0], 0, sizeof(aui_conaxvsc_tx_buf));

	command->p_uc_data[0] = 0x14; 
	command->p_uc_data[1] = 3 + len; 
	command->p_uc_data[2] = 0x00;
	memcpy(&command->p_uc_data[3], buf, len);
	command->n_size= 3 + len;

	if(aui_find_dev_by_idx(AUI_MODULE_KL, 0, &s_p_hdls->kl_hdl)){
		AUI_PRINTF("get kl handler fail!\n");
		goto exit;
	}

	if(aui_find_dev_by_idx(AUI_MODULE_CONAXVSC, 0, &hdl_vsc)){
		AUI_PRINTF("get vsc handler fail!\n");
		goto exit;
	}

	if(aui_conaxvsc_cmd_transfer(hdl_vsc, 0, command, response, &sw1, &sw2))
	{
		AUI_PRINTF("ioctl aui_conaxvsc_cmd_transfer failed");
		goto exit;
	}

	if (sw1 != 0x90 || sw2 != 0x00) {
		AUI_PRINTF("ioctl VSC_CMD_DISPATCH response len=%d sw=%02x,%02x",	response->n_size, sw1, sw2);
		goto exit;
	}

	size = response->n_size;
	p = 	response->p_uc_data;

	while (size > 2) {
		p = vsc_find_next_token(p, VSC_PI_CW, &size);

		if (p[0] != VSC_PI_CW)
			break;

		if (size < 23) {
			AUI_PRINTF("CW parameter too short:%d", size);
			goto exit;
		}

		PL = (int)p[1];
		PV = &p[2];
		memset(&attr, 0, sizeof(aui_conaxvsc_decw_attr));
		attr.kl_handle = s_p_hdls->kl_hdl;
		attr.key_id = p[3];
		attr.key_parity = p[4] ? AUI_CONAXVSC_KEY_ODD : AUI_CONAXVSC_KEY_EVEN;
		memcpy(attr.decw, &p[7], AUI_CONAXVSC_DECW_LEN);

		if(attr.key_parity==AUI_CONAXVSC_KEY_ODD)
		{
			if(MEMCMP(&odd_cw[0],&attr.decw[0],16))
			{
				MEMCPY(&odd_cw[0], &attr.decw[0], 16);
				if(aui_conaxvsc_decrypt_cw(hdl_vsc, &attr))
				{
					AUI_PRINTF("ioctl VSC_DECW_KEY failed !");
					goto exit;
				}
			}
		}
		else if(attr.key_parity==AUI_CONAXVSC_KEY_EVEN)
		{
			if(MEMCMP(&even_cw[0],&attr.decw[0],16))
			{
				MEMCPY(&even_cw[0], &attr.decw[0], 16);
				if(aui_conaxvsc_decrypt_cw(hdl_vsc, &attr))
				{
					AUI_PRINTF("ioctl VSC_DECW_KEY failed !");
					goto exit;
				}
			}
		}

		p = &PV[PL];
		size -= (PL + 2);
	}
	free(command);
	free(response);
	return 0;
exit:
	free(command);
	free(response);
	return 1;
}

#ifdef _SMI_CF_ENABLE_
aui_hdl cf_kl_hdl[3];

int cf_init(void)
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

	CfVectorStruct_t vectorStruct_03F0={
	  	0, 	//VectorType
	  	0x0F, 	//OutputUsageAllowed
	  	{  	//Vector
	  		0xF8121D46,0x57E04C6B,0x5C86240E,0x161BE0C4,0xB19A4665,
	  		0x8F14B43B,0xDFE6D16C,0x90EB31B0,0x03632E94,0x70D5FC0F,
	  		0x36082567,0x74BEA72C,0x617C5144,0x35ABC053,0x3028041B,
	  		0x4B25AE71,0xA40DF8D5,0x52DCE48F,0xEC25512A,0xEC481344
		}
	};
  	status = SMK_ProcessSmm_VectorStruct(&vectorStruct_03F0, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_VectorStruct(vectorStruct_03F0) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	// store ERK
  	CfErkStruct_t erkStruct_03F0_00AB=
  	{
  		0x00AB, //ProductRange
  		0x00,   //AddressingMode
  		{	//Bitmap
  			0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
        		0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
        	},
        	0x03,   //KeySenseIndicator
        	0x00000000, //Activation0
        	0xFFFFFFFF, //Expiration0
        	0xFFFFFFFF, //IRV0
        	{	//ERK0
        		0xE800E14A,0x8A18A03A,0x69E47A5E,0xE2F09553
        	},
        	0x00000000, //Activation1
        	0xFFFFFFFF, //Expiration1
        	0xFFFFFFFF, //IRV1
        	{	//ERK1
        		0xE800E14A,0x8A18A03A,0x69E47A5E,0xE2F09553
        	},
  	};

  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AB, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AB) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	// run SHVs
  	CfCwcStruct_t  cwcStruct_03F0_00AB;
  	uint32_t       shv_03F0_00AB[4];
  	uint32_t       expected_shv_03F0_00AB[4]={0x0267EE50,0xFEBA5B9A,0x84BD1341,0x9783C27D};
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

  	status = memcmp(expected_shv_03F0_00AB, shv_03F0_00AB, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	// run SHV - unique
  	cwcStruct_03F0_00AB.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AB[4]={0x9D9EEF7C,0x0729FBAA,0x45981129,0xC9F00D9A};
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

  	status = memcmp(expectedUniq_shv_03F0_00AB, shv_03F0_00AB, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	// store ERK
  	CfErkStruct_t erkStruct_03F0_00AC=
  	{
  		0x00AC, //ProductRange
  		0x00,   //AddressingMode
  		{	//Bitmap
  			0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
        		0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
        	},
        	0x03,   //KeySenseIndicator
        	0x00000000, //Activation0
        	0xFFFFFFFF, //Expiration0
        	0xFFFFFFFF, //IRV0
        	{	//ERK0
        		0xEE470C7E,0xFE1B37F7,0xEC6A1D0A,0xF00EEAA9
        	},
        	0x00000000, //Activation1
        	0xFFFFFFFF, //Expiration1
        	0xFFFFFFFF, //IRV1
        	{	//ERK1
        		0xEE470C7E,0xFE1B37F7,0xEC6A1D0A,0xF00EEAA9
        	},
  	};

  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AC, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AC) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	// run SHVs
  	CfCwcStruct_t  cwcStruct_03F0_00AC;
  	uint32_t       shv_03F0_00AC[4];
  	uint32_t       expected_shv_03F0_00AC[4]={0x9F91B820,0x3B474D09,0x1D6456FE,0x3566A691};
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

  	status = memcmp(expected_shv_03F0_00AC, shv_03F0_00AC, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
   	// run SHV - unique
  	cwcStruct_03F0_00AC.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AC[4]={0x11D40BE2,0x6A4F2F10,0xE5FEC187,0xEE2D0530};
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

  	status = memcmp(expectedUniq_shv_03F0_00AC, shv_03F0_00AC, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	// store ERK 
  	CfErkStruct_t erkStruct_03F0_00AD=
  	{
  		0x00AD, //ProductRange
  		0x00,   //AddressingMode
  		{	//Bitmap
  			0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
        		0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
        	},
        	0x03,   //KeySenseIndicator
        	0x00000000, //Activation0
        	0xFFFFFFFF, //Expiration0
        	0xFFFFFFFF, //IRV0
        	{	//ERK0
        		0xE9C85DAC,0xF8270B6C,0x28637330,0xA7F4E186
        	},
        	0x00000000, //Activation1
        	0xFFFFFFFF, //Expiration1
        	0xFFFFFFFF, //IRV1
        	{	//ERK1
        		0xE9C85DAC,0xF8270B6C,0x28637330,0xA7F4E186
        	},
  	};
  
  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AD, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AD) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	// run SHVs
  	CfCwcStruct_t  cwcStruct_03F0_00AD;
  	uint32_t       shv_03F0_00AD[4];
  	uint32_t       expected_shv_03F0_00AD[4]={0x94B3F1AE,0xE64FBC3E,0x0199B42D,0x762D8DA4};
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

  	status = memcmp(expected_shv_03F0_00AD, shv_03F0_00AD, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	// run SHV - unique
  	cwcStruct_03F0_00AD.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AD[4]={0x4489BF37,0x684C6622,0x432D400D,0xCCCEB7A7};
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

  	status = memcmp(expectedUniq_shv_03F0_00AD, shv_03F0_00AD, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
  
  	// store ERK
  	CfErkStruct_t erkStruct_03F0_00AE=
  	{
  		0x00AE, //ProductRange
  		0x00,   //AddressingMode
  		{	//Bitmap
  			0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,
        		0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF
        	},
        	0x03,   //KeySenseIndicator
        	0x00000000, //Activation0
        	0xFFFFFFFF, //Expiration0
        	0xFFFFFFFF, //IRV0
        	{	//ERK0
        		0xBCCB213B,0x8815F65E,0x8C3973E3,0x2A0737D0
        	},
        	0x00000000, //Activation1
        	0xFFFFFFFF, //Expiration1
        	0xFFFFFFFF, //IRV1
        	{	//ERK1
        		0xBCCB213B,0x8815F65E,0x8C3973E3,0x2A0737D0
        	},
  	};

  	status = SMK_ProcessSmm_ErkStruct(&erkStruct_03F0_00AE, 0x03F0);
  	if (status != SMK_OK)
  	{
    		printf("SMK_ProcessSmm_ErkStruct(erkStruct_03F0_00AE) failed with code: 0x%08X\n", status);
    		return -1;
  	}

  	// run SHVs
  	CfCwcStruct_t  cwcStruct_03F0_00AE;
  	uint32_t       shv_03F0_00AE[4];
  	uint32_t       expected_shv_03F0_00AE[4]={0xD88C812E,0xA7F4828F,0xD56F73C6,0x34AB4181};
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

  	status = memcmp(expected_shv_03F0_00AE, shv_03F0_00AE, 16);
  	if (status != 0)
  	{
    		printf("Expected Global SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}	
  
  	// run SHV - unique
  	cwcStruct_03F0_00AE.Timestamp     = 0x000003E8;
  	uint32_t       expectedUniq_shv_03F0_00AE[4]={0x2885D0DC,0x15556235,0x1B9BED5C,0x0A70710B};
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

  	status = memcmp(expectedUniq_shv_03F0_00AE, shv_03F0_00AE, 16);
  	if (status != 0)
  	{
    		printf("Expected Unique SHV comparison failed 0x%08X\n", status);
    		return -1;
  	}
	cf_inited = 1;
    return 0;
}

long cf_ecm_callback(aui_hdl flt_hdl, unsigned char *buf,unsigned long len, void *usr_data, void *reserved)
{
	aui_hdl hdl_vsc;
	aui_kl_key_source_attr key_source;
	aui_kl_key_source_attr data_source;
	aui_cfg_kl cfg;
	int session_id = 0;
	aui_conaxvsc_decw_attr attr;
	aui_conaxvsc_tx_buf *command = (aui_conaxvsc_tx_buf *)malloc(sizeof(aui_conaxvsc_tx_buf));
	aui_conaxvsc_tx_buf *response = (aui_conaxvsc_tx_buf *)malloc(sizeof(aui_conaxvsc_tx_buf));
	int sw1, sw2;
	int ret;
	uint8_t *p;
	int PI, PL;
	uint8_t *PV;
	int size;

	(void) PI;
	(void) PL;
	(void) flt_hdl;
	(void) reserved;
	(void) usr_data;

	if(aui_find_dev_by_idx(AUI_MODULE_CONAXVSC, 0, &hdl_vsc)){
		AUI_PRINTF("get vsc handler fail!\n");
		goto exit;
	}

	command->p_uc_data[0] = 0x14;
	command->p_uc_data[1] = 3 + len;
	command->p_uc_data[2] = 0x00;
	memcpy(&command->p_uc_data[3], buf, len);
	command->n_size = 3 + len;

	ret = aui_conaxvsc_cmd_transfer(hdl_vsc, session_id, command,
		response, &sw1, &sw2);
	if (ret || sw1 != 0x90 || sw2 != 0x00) {
		AUI_PRINTF("aui_conaxvsc_cmd_transfer failed. ret=%d sw=%02x%02x\n",
			ret, sw1, sw2);
		goto exit;
	}

	size = response->n_size;
	p = response->p_uc_data;

	while (size > 2) {
		PI = p[0]; PL = p[1]; PV = &p[2];

		if(PI == VSC_PI_CW)
		{
			memset(&attr, 0, sizeof(aui_conaxvsc_decw_attr));
			attr.kl_handle = cf_kl_hdl[0];
			attr.key_id = p[3];
			attr.key_parity = p[4] ? AUI_CONAXVSC_KEY_ODD : AUI_CONAXVSC_KEY_EVEN;
			memcpy(attr.decw, &p[7], AUI_CONAXVSC_DECW_LEN);
			ret = aui_conaxvsc_decrypt_cw(hdl_vsc, &attr);
			if(aui_conaxvsc_decrypt_cw(hdl_vsc, &attr))
			{
					AUI_PRINTF("aui_conaxvsc_decrypt_cw fail!");
					goto exit;
			}
		}
		else if(PI == VSC_PI_HOST_DATA && p[2] == VSC_PI_CF)
		{
			SMK_error_t status;
  			uint32_t shv_result[4]={0};			
			aui_kl_cf_target_attr target_set;		

			memset(&target_set,0,sizeof(aui_kl_cf_target_attr));
			target_set.parity=AUI_KL_CW_KEY_EVEN;
			target_set.uc_cf_dev_idx=0;
			if(aui_kl_cf_target_set(cf_kl_hdl[1],&target_set))
			{
					AUI_PRINTF("aui_kl_cf_target_set fail!");
					goto exit;
			}	
	    
  		status = SMK_GenerateCWC(&p[4], p[3], 1, 0, 0x03F0);
  		if (status != SMK_OK)
  		{
    			printf("SMK_GenerateCWC failed with code: 0x%08X\n", status);
    			goto exit;
  		}
  		status=SMK_FinalizeCWC(shv_result);
  		if (status != SMK_OK)
  		{
    			printf("SMK_FinalizeCWC failed with code: 0x%08X\n", status);
    			goto exit;   			
			}
		}
		
		p = &PV[PL];
		size -= (PL + 2);
	}

	memset(&key_source,0x0,sizeof(aui_kl_key_source_attr));
	memset(&data_source,0x0,sizeof(aui_kl_key_source_attr));
	memset(&cfg,0x0,sizeof(struct aui_cfg_kl));
	key_source.key_param.key_ladder_handle = cf_kl_hdl[1];
	data_source.key_param.key_ladder_handle = cf_kl_hdl[0];
	aui_kl_gen_key_by_cfg_ext(cf_kl_hdl[2], &cfg, &data_source, &key_source);
	
	free(command);
	free(response);
	return 0;
exit:
	free(command);
	free(response);
	return 1;
}

int cf_kl(unsigned char uc_dev_idx,aui_kl_output_key_pattern en_key_pattern)
{
	aui_attr_kl attr_kl;
	memset(&attr_kl,0,sizeof(aui_attr_kl));
	attr_kl.uc_dev_idx = uc_dev_idx;
	attr_kl.en_key_pattern = en_key_pattern;
#ifdef VSC_SMI
	attr_kl.en_level = AUI_KL_KEY_FIVE_LEVEL;
#else
	attr_kl.en_level = AUI_KL_KEY_THREE_LEVEL;
#endif
	attr_kl.en_root_key_idx = AUI_KL_ROOT_KEY_0_0;
	attr_kl.en_key_ladder_type = AUI_KL_TYPE_CONAXVSC;
	
	if(aui_kl_open(&attr_kl, &cf_kl_hdl[(int)uc_dev_idx])) 
	{
	    AUI_PRINTF("aui_kl_open fail!\n");
	    return 1;
	}

	return 0;
}

int cf_dmx_filter_channel(void)
{
	aui_hdl hdl_channel;
	aui_hdl hdl_filter;
	aui_attr_dmx_channel channel_attr;
	aui_attr_dmx_filter filter_attr;
	unsigned char mask[] = {0x08};
	unsigned char value[] = {0x00};	
	unsigned char reverse[] = {0x00};

	memset(&channel_attr, 0, sizeof(aui_attr_dmx_channel));
	channel_attr.us_pid = 0x1d57;
	channel_attr.dmx_data_type = AUI_DMX_DATA_SECT;
	
	if (aui_dmx_channel_open(s_p_hdls->dmx_hdl, &channel_attr, &hdl_channel))
	{
		AUI_PRINTF("aui_dmx_channel_open failed!\n");	
		return 1;
	}

	if (aui_dmx_channel_start(hdl_channel, &channel_attr)) {	
		AUI_PRINTF("aui_dmx_channel_start failed!\n");	
		return 1;
	}

	memset(&filter_attr, 0, sizeof(aui_attr_dmx_filter));
	filter_attr.uc_crc_check = 0;
	filter_attr.uc_continue_capture_flag = 1;
	filter_attr.puc_val = value;	
	filter_attr.puc_mask = mask;
	filter_attr.puc_reverse = reverse;
	filter_attr.ul_mask_val_len = 0;

	if (aui_dmx_filter_open(hdl_channel, &filter_attr, &hdl_filter)) {	
		AUI_PRINTF("aui_dmx_filter_open failed!\n");	
		return 1;	
	}
	
	aui_dmx_reg_sect_call_back(hdl_filter, cf_ecm_callback);
	if (aui_dmx_filter_start(hdl_filter, &filter_attr)) {	
		AUI_PRINTF("aui_dmx_filter_start failed!\n");	
		return 1;	
	}
	return 0;
}

int cf_dsc(unsigned short vpid,unsigned short apid)
{
	aui_attr_dsc attr_dsc;
	unsigned short pidlist[8];
	
	memset(&attr_dsc,0,sizeof(aui_attr_dsc));
	attr_dsc.uc_dev_idx=0; 
	attr_dsc.dsc_data_type=AUI_DSC_DATA_TS;	
	attr_dsc.en_en_de_crypt=AUI_DSC_DECRYPT;  
	attr_dsc.uc_algo=AUI_DSC_ALGO_CSA;
	attr_dsc.csa_version=AUI_DSC_CSA2;
	attr_dsc.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;

	if (aui_dsc_open(&attr_dsc, &s_p_hdls->dsc_hdl)) 
	{
	    AUI_PRINTF("aui_dsc_open fail!\n");
	    return 1;
	}

	pidlist[0] = vpid;
	pidlist[1] = apid;
	attr_dsc.ul_pid_cnt=2;
	attr_dsc.pus_pids= pidlist;
	attr_dsc.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
	attr_dsc.en_parity=AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
	attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;
    aui_kl_get(cf_kl_hdl[2], AUI_KL_GET_KEY_POS, &attr_dsc.ul_key_pos);
	aui_kl_get(cf_kl_hdl[2], AUI_KL_GET_KEY_SIZE, &attr_dsc.ul_key_len);
	//AUI_PRINTF("cnx_dsc key:%u,%u\n",attr_dsc.ul_key_pos,attr_dsc.ul_key_len);

	if (aui_dsc_attach_key_info2dsc(s_p_hdls->dsc_hdl, &attr_dsc)) 
	{
		AUI_PRINTF("aui_dsc_attach_key_info2dsc fail!\n");
		return 1;
	}
	return 0;
}

#endif

int cnx_dmx_filter_channel(void)
{
	aui_hdl hdl_channel;
	aui_hdl hdl_filter;
	aui_attr_dmx_channel channel_attr;
	aui_attr_dmx_filter filter_attr;
	unsigned char mask[] = {0x08};
	unsigned char value[] = {0x00};	
	unsigned char reverse[] = {0x00};

	memset(&channel_attr, 0, sizeof(aui_attr_dmx_channel));
	channel_attr.us_pid = 0x1771;
	channel_attr.dmx_data_type = AUI_DMX_DATA_SECT;
	
	if (aui_dmx_channel_open(s_p_hdls->dmx_hdl, &channel_attr, &hdl_channel))
	{
		AUI_PRINTF("aui_dmx_channel_open failed!\n");	
		return 1;
	}

	if (aui_dmx_channel_start(hdl_channel, &channel_attr)) {	
		AUI_PRINTF("aui_dmx_channel_start failed!\n");	
		return 1;
	}

	memset(&filter_attr, 0, sizeof(aui_attr_dmx_filter));
	filter_attr.uc_crc_check = 0;
	filter_attr.uc_continue_capture_flag = 1;
	filter_attr.puc_val = value;	
	filter_attr.puc_mask = mask;
	filter_attr.puc_reverse = reverse;
	filter_attr.ul_mask_val_len = 0;

	if (aui_dmx_filter_open(hdl_channel, &filter_attr, &hdl_filter)) {	
		AUI_PRINTF("aui_dmx_filter_open failed!\n");	
		return 1;	
	}
	
	aui_dmx_reg_sect_call_back(hdl_filter, vsc_process_ecm);
	if (aui_dmx_filter_start(hdl_filter, &filter_attr)) {	
		AUI_PRINTF("aui_dmx_filter_start failed!\n");	
		return 1;	
	}
	return 0;
}

static void show_cmd_usage()
{
    AUI_PRINTF("usage:\n");
    AUI_PRINTF("personalization\n");
    AUI_PRINTF("play_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar]\n");
    AUI_PRINTF("PVR_rec_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar]\n");
    AUI_PRINTF("PVR_play_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar]\n");
}

static int trimwhitespace(char *in, char *out)
{
	char *s, *d;
	s = in;
	d = out;

	while (*s != 0) {
		if (*s != '\f'  && *s != '\v'  && *s != '\t'  && *s != ' '  && *s != '\r'  && *s != '\n' && *s != '\"') {
			*d = *s;
			d++;
		}
		s++;
	}
	return (d - out);
}

static int hexstring_to_bytearray(const char *hexstr, unsigned char *buf, int len)
{
	int i = 0;
	int j=0;
	const char *pos = hexstr;
	int strnum[2]={0,0};

	for(i = 0; i < (int)(len/2); i++) {
		for(j=0;j<2;j++)
		{
			if(*(pos+j) >= '0' && *(pos+j) <= '9')
			{
				strnum[j] = (*(pos+j) - '0');
			}
			else if (*(pos+j)>= 'A' && *(pos+j)<= 'F')
			{
				strnum[j] = (10 + (*(pos+j) - 'A'));
			}
			else if (*(pos+j) >= 'a' && *(pos+j) <= 'f')
			{
				strnum[j] = (10 + (*(pos+j) - 'a'));
			}
		}
		buf[i] = (unsigned char)((strnum[0]*16 + strnum[1])&0xff);
		pos += 2;
	}
	return 0;
}

static int read_perso_file(char *path, aui_conaxvsc_tx_buf *perso, int max_size)
{
	FILE* in;
	int len, total;
	char *buf = NULL;
	char *saveptr;
	char *pch;
	int i = 0;

	in = fopen(path, "rb");
	if (!in) {
		AUI_PRINTF("open %s failed error\n", path);
		return 1;
	}

	fseek(in, 0, SEEK_END);
	total = ftell(in);
	fseek(in, 0, SEEK_SET);

	buf = malloc(total + 1);
	if (!buf) {
		AUI_PRINTF("malloc failed");
		fclose(in);
		return 1;
	}

	len = fread(buf,1, total,in);
	if (len != total) {
		AUI_PRINTF("readv error ret=%d error\n", len);
		fclose(in);
		return 1;
	}

	buf[total] = '\0';
	total = trimwhitespace(buf, buf);
	buf[total-2] = '\0';
	buf = strstr(buf, "[") + 1;
	pch = strtok_r(buf, ",", &saveptr);
	while (pch != NULL && i < max_size) {
		hexstring_to_bytearray(&pch[0], perso[i].p_uc_data, strlen(&pch[0]) - 0);
		perso[i].n_size = (strlen(pch) - 0) / 2;
		pch = strtok_r(NULL, ",", &saveptr);
		i++;
	}
	fclose(in);
	return 0;
}

unsigned long test_do_personalization(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_hdl vsc_handle;
	int i=0;	
	const int num_cmd = 64;
	aui_conaxvsc_tx_buf *vsc_data = (aui_conaxvsc_tx_buf *)malloc(num_cmd*sizeof(aui_conaxvsc_tx_buf));
	char* infile = (char*) malloc(64);

	MEMSET(&infile[0],0,64);
	MEMSET(&vsc_data[0],0,num_cmd*sizeof(aui_conaxvsc_tx_buf));

	if (*argc > 0) {
		STRCPY(infile,argv[0]);
	}
	else
	{
#ifdef AUI_LINUX
		STRCPY(infile,"/mnt/usb/sda1/demo_00001.vcf");
#else
		STRCPY(infile,"/mnt/uda1/demo_00001.vcf");
#endif
	}
    
	if(cnx_vsc(&vsc_handle))  //initial VSC
		goto exit;

	if(read_perso_file(infile, vsc_data, num_cmd)){
		AUI_PRINTF("read_perso_file failed\n");
		goto exit;
	}

	for(i=0;i<num_cmd;i++) // execute perso cmd
	{
		int sw1 = 0;
		int sw2 = 0;
		aui_conaxvsc_tx_buf *vsc_response =0;

		if (!vsc_data[i].n_size)
			break;

		vsc_response = (aui_conaxvsc_tx_buf *)malloc(sizeof(aui_conaxvsc_tx_buf));
		MEMSET(&vsc_response[0],0,sizeof(aui_conaxvsc_tx_buf));

		aui_conaxvsc_cmd_transfer(vsc_handle, 0,&vsc_data[i], vsc_response, &sw1,&sw2);
		if(0x90 != sw1 || 0x00 != sw2)
		{
			AUI_PRINTF("vsc_lib_dispatch_cmd error:%d,%d\n",sw1,sw2);
			free(vsc_response);
			goto exit;
		}
		free(vsc_response);
	}

	if(aui_conaxvsc_close(vsc_handle))
	{
		AUI_PRINTF("aui_conaxvsc_close failed");
		goto exit;
	}

	free(vsc_data);
	free(infile);
	AUI_PRINTF("persernalization finish!\n");
	return 0;
exit:
	free(vsc_data);
	free(infile);
	return 1;
}

unsigned long test_aui_play_VSC_stream(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int rec_times = 0;
	aui_hdl vsc_handle;
	struct ali_app_modules_init_para init_para;
	enum aui_decv_vbv_buf_mode vbv_buf_mode = AUI_DECV_VBV_BUF_BLOCK_FULL_MODE;
	int use_cf = 0;

	if(0 == *argc) {
		show_cmd_usage();
		return 0;
	}
	s_p_hdls = malloc(sizeof(struct ali_aui_hdls));
	if(NULL == s_p_hdls) return -1;
	MEMSET(s_p_hdls, 0, sizeof(struct ali_aui_hdls));
	ali_aui_init_para_for_test_nim(argc,argv,&init_para);

	#ifdef _SMI_CF_ENABLE_
	if(*argc>13)
		use_cf = atoi(argv[13]);
	#endif

#ifdef AUI_LINUX
	if(aui_nim_init(nim_init_cb)) {
	    AUI_PRINTF("\nnim init error\n");
	    goto err_live;
	}
	AUI_PRINTF("AUI NIM opened\n");

	if(ali_app_dis_init(init_para.init_para_dis,&s_p_hdls->dis_hdl)) {
	    AUI_PRINTF("\r\n ali_app_dis_init failed!");
	    goto err_live;
	}
	AUI_PRINTF("AUI DIS opened\n");
#endif

	if(nim_connect(&init_para.init_para_nim,&s_p_hdls->nim_hdl)) {
		AUI_PRINTF("\nnim connect error\n");
		goto err_live;
	}

	if(ali_app_tsi_init(&init_para.init_para_tsi, &s_p_hdls->tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI TSI opened\n");

	if(cnx_vsc(&vsc_handle))
		goto err_live;
	AUI_PRINTF("AUI VSC opened\n");

	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(cf_init())
			goto err_live;
		AUI_PRINTF("SMK init\n");

		if(cf_kl(0,AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN))
			goto err_live;
		AUI_PRINTF("AUI KL 0 opened\n");

		if(cf_kl(1,AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN))
			goto err_live;
		AUI_PRINTF("AUI KL 1 opened\n");

		if(cf_kl(2,AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN))
			goto err_live;
		AUI_PRINTF("AUI KL 2 opened\n");
		#endif
	}
	else
	{
		if(cnx_kl())
			goto err_live;
		AUI_PRINTF("AUI KL opened\n");
	}
	
	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(cf_dsc(init_para.dmx_create_av_para.video_pid,init_para.dmx_create_av_para.audio_pid))
			goto err_live;
		#endif
	}
	else
	{	
		if(cnx_dsc(init_para.dmx_create_av_para.video_pid,init_para.dmx_create_av_para.audio_pid))
			goto err_live;
	}
	AUI_PRINTF("AUI DSC opened\n");

	if(ali_app_decv_init(init_para.init_para_decv,&s_p_hdls->decv_hdl)) {
		AUI_PRINTF("\r\n ali_app_decv_init failed!");
		goto err_live;
	}
	
	if(aui_decv_set(s_p_hdls->decv_hdl, AUI_DECV_SET_VBV_BUF_MODE, &vbv_buf_mode))
	{
		AUI_PRINTF("\r\n aui_decv_set AUI_DECV_SET_VBV_BUF_MODE failed!\n");
		goto err_live;
	}
	AUI_PRINTF("AUI DECV opened\n");
	
	if(ali_app_audio_init(init_para.init_para_audio,&s_p_hdls->deca_hdl,&s_p_hdls->snd_hdl)) {
		AUI_PRINTF("\r\n ali_app_audio_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI audio opened[%08x]\n",(int)s_p_hdls->deca_hdl);

	if(aui_decv_sync_mode(s_p_hdls->decv_hdl,AUI_STC_AVSYNC_AUDIO)){
		AUI_PRINTF("Set AUI_STC_AVSYNC_AUDIO fail\n");
		goto err_live;
	}
	AUI_PRINTF("AUI decv sync\n");

	if(set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id,
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
	path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
	path.p_hdl_de_dev = s_p_hdls->dsc_hdl;
	if (aui_dmx_data_path_set(s_p_hdls->dmx_hdl, &path)) {
		AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
		goto err_live;
	}
	AUI_PRINTF("dmx data path set %d\n", path.data_path_type);

	if(aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE, (void *)AUI_AVSYNC_FROM_TUNER)) {
		AUI_PRINTF("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
	 	goto err_live;
	}

	if(aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)){
		AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
		goto err_live;
	}

	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(cf_dmx_filter_channel())
			goto err_live;
		#endif
	}
	else
	{
		if(cnx_dmx_filter_channel())
			goto err_live;
	}
	AUI_PRINTF("set dmx channel success\n");

	for(rec_times = 0;rec_times<300;rec_times++)
	{
		aui_os_task_sleep(100);
	}	
err_live:
	ali_app_deinit(s_p_hdls);
	#ifdef AUI_LINUX
	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(aui_kl_close(cf_kl_hdl[0]))
		AUI_PRINTF("aui_kl_close 0 failed\n");
	
		if(aui_kl_close(cf_kl_hdl[1]))
			AUI_PRINTF("aui_kl_close 1 failed\n");

		if(aui_kl_close(cf_kl_hdl[2]))
			AUI_PRINTF("aui_kl_close 2 failed\n");
		#endif
	}
	else
	{
		if(aui_kl_close(s_p_hdls->kl_hdl))
			AUI_PRINTF("aui_kl_close failed\n");
	}

	if(aui_dsc_close(s_p_hdls->dsc_hdl))
		AUI_PRINTF("aui_dsc_close failed\n");
		
	if(aui_conaxvsc_close(vsc_handle))
		AUI_PRINTF("aui_conaxvsc_close failed");
	#endif
	AUI_PRINTF("video play stop\n");
	return AUI_RTN_SUCCESS;
}

unsigned long test_aui_rec_VSC_PVR(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_hdl vsc_handle;
	aui_hdl pvr_handler;
	unsigned int rec_times = 0;
	struct ali_app_modules_init_para init_para;
	unsigned int pvr_buffer_len = 5*1024*1024;
	unsigned char *pvr_buffer = (unsigned char *)malloc(pvr_buffer_len);
	unsigned int duarion =0;
	unsigned int temp_time =0;
	enum aui_decv_vbv_buf_mode vbv_buf_mode = AUI_DECV_VBV_BUF_BLOCK_FULL_MODE;
	int encypt_param = 1;
	int use_cf = 0;

	if(0 == *argc) {
		show_cmd_usage();
		return 0;
	}
	s_p_hdls = malloc(sizeof(struct ali_aui_hdls));
	if(NULL == s_p_hdls) return -1;
	MEMSET(s_p_hdls, 0, sizeof(struct ali_aui_hdls));
	ali_aui_init_para_for_test_nim(argc,argv,&init_para);

	#ifdef _SMI_CF_ENABLE_
	if(*argc>13)
		use_cf = atoi(argv[13]);
	#endif

#ifdef AUI_LINUX
	if(aui_nim_init(nim_init_cb)) {
	    AUI_PRINTF("\nnim init error\n");
	    goto err_live;
	}
	AUI_PRINTF("AUI NIM opened\n");

	if(ali_app_dis_init(init_para.init_para_dis,&s_p_hdls->dis_hdl)) {
	    AUI_PRINTF("\r\n ali_app_dis_init failed!");
	    goto err_live;
	}
	AUI_PRINTF("AUI DIS opened\n");
#endif

	if(nim_connect(&init_para.init_para_nim,&s_p_hdls->nim_hdl)) {
		AUI_PRINTF("\nnim connect fail\n");
		goto err_live;
	}
	AUI_PRINTF("nim connect success\n");

	if(ali_app_tsi_init(&init_para.init_para_tsi, &s_p_hdls->tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI TSI opened\n");

	if(cnx_PVR_init(pvr_buffer,pvr_buffer_len))
		goto err_live;

	AUI_PRINTF("AUI PVR opened\n");

	if(cnx_vsc(&vsc_handle))
		goto err_live;
	AUI_PRINTF("AUI VSC opened\n");

	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(cf_init())
			goto err_live;
		AUI_PRINTF("SMK init\n");

		if(cf_kl(0,AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN))
			goto err_live;
		AUI_PRINTF("AUI KL 0 opened\n");

		if(cf_kl(1,AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN))
			goto err_live;
		AUI_PRINTF("AUI KL 1 opened\n");

		if(cf_kl(2,AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN))
			goto err_live;
		AUI_PRINTF("AUI KL 2 opened\n");
		#endif
	}
	else
	{
		if(cnx_kl())
			goto err_live;
		AUI_PRINTF("AUI KL opened\n");
	}

	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(cf_dsc(init_para.dmx_create_av_para.video_pid,init_para.dmx_create_av_para.audio_pid))
			goto err_live;
		#endif
	}
	else
	{	
		if(cnx_dsc(init_para.dmx_create_av_para.video_pid,init_para.dmx_create_av_para.audio_pid))
			goto err_live;
	}
	AUI_PRINTF("AUI DSC opened\n");
	
	if(ali_app_decv_init(init_para.init_para_decv,&s_p_hdls->decv_hdl)) {
		AUI_PRINTF("\r\n ali_app_decv_init failed!");
		goto err_live;
	}
	
	if(aui_decv_set(s_p_hdls->decv_hdl, AUI_DECV_SET_VBV_BUF_MODE, &vbv_buf_mode))
	{
		AUI_PRINTF("\r\n aui_decv_set AUI_DECV_SET_VBV_BUF_MODE failed!\n");
		goto err_live;
	}
	AUI_PRINTF("AUI DECV opened\n");
	
	if(ali_app_audio_init(init_para.init_para_audio,&s_p_hdls->deca_hdl,&s_p_hdls->snd_hdl)) {
		AUI_PRINTF("\r\n ali_app_audio_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI audio opened[%08x]\n",(int)s_p_hdls->deca_hdl);

	if(aui_decv_sync_mode(s_p_hdls->decv_hdl,AUI_STC_AVSYNC_AUDIO)){
		AUI_PRINTF("Set AUI_STC_AVSYNC_AUDIO fail\n");
		goto err_live;
	}
	AUI_PRINTF("AUI decv sync\n");

	if(set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id,
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
	path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
	path.p_hdl_de_dev = s_p_hdls->dsc_hdl;
	if(aui_dmx_data_path_set(s_p_hdls->dmx_hdl, &path)) {
		AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
		goto err_live;
	}
	AUI_PRINTF("dmx data path set %d\n", path.data_path_type);

	if(aui_dmx_set(s_p_hdls->dmx_hdl,AUI_DMX_SET_IO_REC_MODE,&encypt_param))
	{
		AUI_PRINTF("\r\n AUI_DMX_SET_IO_REC_MODE fail\n");
		goto err_live;
	}

	if(aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE, (void *)AUI_AVSYNC_FROM_TUNER)) {
		AUI_PRINTF("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
	 	goto err_live;
	}

	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(cf_dmx_filter_channel())
			goto err_live;
		#endif
	}
	else
	{
		if(cnx_dmx_filter_channel())
			goto err_live;
	}
	AUI_PRINTF("set dmx channel success\n");

	if(aui_dmx_set(s_p_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
	    AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
	    goto err_live;
	}
	
	aui_os_task_sleep(3000);
	
	if(cnx_PVR_record(&pvr_handler,init_para.dmx_create_av_para.video_pid,init_para.dmx_create_av_para.audio_pid,init_para.dmx_create_av_para.pcr_pid,init_para.dmx_create_av_para.audio_desc_pid))
	{
		AUI_PRINTF("cnx_PVR_record fail\n");
		goto err_live;
	}
	
	AUI_PRINTF("pvr record start\n");

	for(rec_times = 0;rec_times<300;rec_times++)
	{
		aui_pvr_get(pvr_handler,AUI_PVR_REC_TIME_S,&duarion,0,0);
		if(temp_time!= duarion)
		{
			AUI_PRINTF("Record time:%ds\n",(int)duarion);
			temp_time = duarion;
		}
		aui_os_task_sleep(100);
	}

	if(aui_pvr_rec_close(pvr_handler,1))
	{
		AUI_PRINTF("PVR record close fail\n");
		goto err_live;
	}
err_live:
	ali_app_deinit(s_p_hdls);
	#ifdef AUI_LINUX
	if(aui_pvr_deinit())
		AUI_PRINTF("ali_pvr_deinit failed\n");
		
	if(use_cf == 1)
	{
		#ifdef _SMI_CF_ENABLE_
		if(aui_kl_close(cf_kl_hdl[0]))
		AUI_PRINTF("aui_kl_close 0 failed\n");
	
		if(aui_kl_close(cf_kl_hdl[1]))
			AUI_PRINTF("aui_kl_close 1 failed\n");

		if(aui_kl_close(cf_kl_hdl[2]))
			AUI_PRINTF("aui_kl_close 2 failed\n");
		#endif
	}
	else
	{
		if(aui_kl_close(s_p_hdls->kl_hdl))
			AUI_PRINTF("aui_kl_close failed\n");
	}

	if(aui_dsc_close(s_p_hdls->dsc_hdl))
		AUI_PRINTF("aui_dsc_close failed\n");
		
	if(aui_conaxvsc_close(vsc_handle))
		AUI_PRINTF("aui_conaxvsc_close failed");
	#endif
	free(pvr_buffer);
	AUI_PRINTF("pvr record stop\n");
	return AUI_RTN_SUCCESS;
}

unsigned long test_aui_play_VSC_PVR(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_hdl vsc_handle;
	aui_hdl pvr_handler;
	aui_pvr_stop_ply_param st_apsp;
	unsigned int pvr_buffer_len = 5*1024*1024;
	unsigned char *pvr_buffer = (unsigned char *)malloc(pvr_buffer_len);
	unsigned int duarion =0;
	unsigned int total_time =0;
	unsigned int play_time = 0;
	unsigned short int play_index = 0;
	aui_pvr_play_state play_state = AUI_PVR_PLAY_STATE_PLAY;

	if(0 == *argc) {
		show_cmd_usage();
		return 0;
	}
	s_p_hdls = malloc(sizeof(struct ali_aui_hdls));
	if(NULL == s_p_hdls) return -1;
	MEMSET(s_p_hdls, 0, sizeof(struct ali_aui_hdls));
	play_index = (unsigned short int)atoi(argv[0]);
	play_state = (*argc>1) ? (aui_pvr_play_state)atoi(argv[1]) : play_state;

	if(cnx_PVR_init(pvr_buffer,pvr_buffer_len))
		goto err_live;
	AUI_PRINTF("AUI PVR opened\n");

	if(cnx_vsc(&vsc_handle))
		goto err_live;
	AUI_PRINTF("AUI VSC opened\n");

	if(cnx_PVR_play(&pvr_handler,play_index,play_state))
	{
		AUI_PRINTF("cnx_PVR_play fail\n");
		goto err_live;
	}
	AUI_PRINTF("pvr play start\n");

	aui_pvr_get(pvr_handler,AUI_PVR_ITEM_DURATION,&total_time,0,0);
	AUI_PRINTF("total time:%us\n",total_time);
	while(duarion+1<total_time)
	{
		aui_pvr_get(pvr_handler,AUI_PVR_PLAY_TIME_S,&duarion,0,0);
		if(play_time!=duarion)
		{
			AUI_PRINTF("play time:%ds\n",(int)duarion);
			play_time = duarion;
		}
		aui_os_task_sleep(100);
	}

	st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
	st_apsp.sync =TRUE;
	st_apsp.vpo_mode=0;
	if(aui_pvr_ply_close(pvr_handler,&st_apsp))
	{
		AUI_PRINTF("PVR play close fail\n");
		goto err_live;
	}
err_live:
	ali_app_deinit(s_p_hdls);
	#ifdef AUI_LINUX
	if(aui_pvr_deinit())
		AUI_PRINTF("aui_pvr_deinit failed\n");

	if(aui_conaxvsc_close(vsc_handle))
		AUI_PRINTF("aui_conaxvsc_close failed");
	#endif	
	free(pvr_buffer);
	AUI_PRINTF("pvr play stop\n");
	return AUI_RTN_SUCCESS;
}

unsigned long test_vsc_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nConax VSC Test Help");

	aui_print_help_command("personalization [file path]");
	aui_print_help_instruction_newline("Do personalize from USB device with VSC data file(default demo_00001.vcf)");

	#ifdef _SMI_CF_ENABLE_
	aui_print_help_command("play_vsc");
	aui_print_help_instruction_newline("play conax vsc encrypt stream from tuner usage:");
	aui_print_help_instruction_newline("play_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar],[PIP index],[PIP id],[cf enable]");
	aui_print_help_command("example:");
	aui_print_help_instruction_newline("play_vsc 0,0,11250,27500,502,602,502,0,0,0,H,0,0,1");

	aui_print_help_command("PVR_rec_vsc");
	aui_print_help_instruction_newline("record conax vsc encrypt stream from tuner to USB /ALIDVRS2/pvr1/ folder usage:");
	aui_print_help_instruction_newline("PVR_rec_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar],[PIP index],[PIP id],[cf enable]");
	aui_print_help_command("example:");
	aui_print_help_instruction_newline("PVR_rec_vsc 0,0,11250,27500,502,602,502,0,0,0,H,0,0,1");
	#else
	aui_print_help_command("play_vsc");
	aui_print_help_instruction_newline("play conax vsc encrypt stream from tuner usage:");
	aui_print_help_instruction_newline("play_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar]");
	aui_print_help_command("example:");
	aui_print_help_instruction_newline("play_vsc 0,0,11250,27500,502,602,502,0,0,0,H");

	aui_print_help_command("PVR_rec_vsc");
	aui_print_help_instruction_newline("record conax vsc encrypt stream from tuner to USB /ALIDVRS2/pvr1/ folder usage:");
	aui_print_help_instruction_newline("PVR_rec_vsc [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[dis_format],[DVBT(T2,ISDBT) type/DVBS polar]");
	aui_print_help_command("example:");
	aui_print_help_instruction_newline("PVR_rec_vsc 0,0,11250,27500,502,602,502,0,0,0,H");
	#endif

	aui_print_help_command("PVR_play_vsc");
	aui_print_help_instruction_newline("play conax vsc encrypt stream from USB /ALIDVRS2/pvr1/folder usage:");
	aui_print_help_instruction_newline("PVR_play_vsc [pvr_index],[play_state]");
	aui_print_help_command("example:");
	aui_print_help_instruction_newline("PVR_play_vsc 0,1");

	return AUI_RTN_HELP;
}


void test_conaxvsc_reg()
{
	aui_tu_reg_group("conaxvsc", "vsc");
	aui_tu_reg_item(2, "personalization", AUI_CMD_TYPE_API, test_do_personalization, "Personalization");
	aui_tu_reg_item(2, "play_vsc", AUI_CMD_TYPE_API, test_aui_play_VSC_stream, "play conax vsc stream");
	aui_tu_reg_item(2, "PVR_rec_vsc", AUI_CMD_TYPE_API, test_aui_rec_VSC_PVR, "PVR record conax vsc stream");
	aui_tu_reg_item(2, "PVR_play_vsc", AUI_CMD_TYPE_API, test_aui_play_VSC_PVR, "PVR play conax vsc stream");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_vsc_help, "stream test help");
}


