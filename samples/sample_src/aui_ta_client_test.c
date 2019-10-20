/**@file             aui_ta_stream_test.c
*  @brief            ALi TEE AUI test ta client
*  @author          niker.li
*  @date            2016-4-24
*  @version         1.0.0
*  @note            ali corp. all rights reserved. 2013~2999 copyright (C)\n
*/
/****************************INCLUDE HEAD FILE************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <aui_kl.h>
#include <aui_dsc.h>

#include <otz_id.h>
#include <otz_tee_client_api.h>
#include <aui_ta_test_cmd.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_help_print.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_tsg_dsc.h"

/****************************LOCAL MACRO******************************************/
#define AUI_TA_UUID {2570268142u, 60478u, 17647u, { 158, 226, 120, 109, 150, 250, 242, 238 }}
static const TEEC_UUID aui_ta_uuid = AUI_TA_UUID;

static int g_tee_aui_log_level = AUI_LOG_PRIO_ERR;
TEEC_Session ta_client_session;
TEEC_Context ta_client_context;
TEEC_Operation ta_client_operation;

static int open_flags = 0;
/****************************FUNCTION******************************************/
static unsigned long ta_init_session(unsigned long *argc,char **argv,char *sz_out_put)
{
	TEEC_Result result = TEEC_SUCCESS;
    if(0 != open_flags) {
        AUI_PRINTF("ta client  has been open!\n");
        TEEC_CloseSession(&ta_client_session);
        TEEC_FinalizeContext(&ta_client_context); 
        open_flags = 0;
    }
    memset(&ta_client_session, 0 , sizeof(TEEC_Session));
    memset(&ta_client_context, 0 , sizeof(TEEC_Context));
    memset(&ta_client_operation, 0 , sizeof(TEEC_Operation));
	result = TEEC_InitializeContext(NULL, &ta_client_context);
	if(result != TEEC_SUCCESS) {
		AUI_PRINTF("TEEC_InitializeContext error!\n");
        return AUI_RTN_FAIL;
	}

	result = TEEC_OpenSession(
			&ta_client_context,
			&ta_client_session,
			&aui_ta_uuid,
			TEEC_LOGIN_PUBLIC,
			NULL,
			NULL,
			NULL);
	if(result != TEEC_SUCCESS) {
		AUI_PRINTF("TEEC_OpenSession error!\n");
        return AUI_RTN_FAIL;
	}
    open_flags = 1;
	AUI_PRINTF("open session id 0x%x\n", ta_client_session.session_id);
	AUI_PRINTF("ta_init_session success\n");
    return AUI_RTN_SUCCESS;
}

static unsigned long ta_deinit_session(unsigned long *argc,char **argv,char *sz_out_put)
{
    TEEC_CloseSession(&ta_client_session);
    TEEC_FinalizeContext(&ta_client_context);
    open_flags = 0;
	AUI_PRINTF("close ta_deinit_session\n");
    return AUI_RTN_SUCCESS;
}

unsigned long aui_ta_set_log_level_test(unsigned long *argc,char **argv,char *sz_out_put)
{
    int level = 0;
    TEEC_Result result = TEEC_SUCCESS;
    
    if (*argc != 1) {
        AUI_PRINTF("ERROR, only accept one parameter\n");
        goto err;
    }

    level = atoi(argv[0]);


    switch (level) {
        case AUI_LOG_PRIO_ERR:
        case AUI_LOG_PRIO_WARNING:
        case AUI_LOG_PRIO_INFO:
        case AUI_LOG_PRIO_DEBUG:
            break;
        default:
            AUI_PRINTF("Error, level value not correct\n");
            goto err;
    }

    g_tee_aui_log_level = level;
    AUI_PRINTF("Set TEE AUI log level %d\n", level);

    ta_client_operation.imp.session = &ta_client_session;
    ta_client_operation.imp.cancel_req = 0;
    ta_client_operation.imp.oper_state = OTZ_STATE_APP;
    ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
                TEEC_VALUE_INPUT,
                TEEC_NONE,
                TEEC_NONE,
                TEEC_NONE);
    ta_client_operation.started = 1;
    ta_client_operation.params[0].value.a = g_tee_aui_log_level;

    result = TEEC_InvokeCommand(
            &ta_client_session,
            AUI_TA_CMD_LOG_LEVEL_SET,
            &ta_client_operation,
            NULL);
    if (result != TEEC_SUCCESS) {
        AUI_PRINTF("[REE] TEEC_InvokeCommand failed!\n");
        return -1;
    }

    return 0;
    
err:
    AUI_PRINTF("Usage:\n");
    AUI_PRINTF("   set ERR  level: log 3\n");
    AUI_PRINTF("   set WARN level: log 4\n");
    AUI_PRINTF("   set INFO level: log 6\n");
    AUI_PRINTF("   set DBG  level: log 7\n");

    return -1;
}

/***************case 1: ta_sample_test    start **************************/
static unsigned long ta_sample_test(unsigned long *argc,char **argv,char *sz_out_put)
{
	TEEC_Result result = TEEC_SUCCESS;
	AUI_PRINTF("open session id 0x%x\n", ta_client_session.session_id);

    ta_client_operation.imp.session = &ta_client_session;
    ta_client_operation.imp.cancel_req = 0;
    ta_client_operation.imp.oper_state = OTZ_STATE_APP;
    ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
    				TEEC_VALUE_INPUT,
    				TEEC_VALUE_INPUT,
    				TEEC_NONE,
    				TEEC_NONE);
    ta_client_operation.started = 1;
    ta_client_operation.params[0].value.a = 0x83;
    ta_client_operation.params[0].value.b = TEEC_VALUE_UNDEF;
    ta_client_operation.params[1].value.a = 0x46;
    ta_client_operation.params[1].value.b = TEEC_VALUE_UNDEF;

	result = TEEC_InvokeCommand(
			&ta_client_session,
			AUI_TA_CMD_ECHO,
			&ta_client_operation,
			NULL);
	if (result != TEEC_SUCCESS) {
		AUI_PRINTF("TEEC_InvokeCommand error!\n");
        return AUI_RTN_FAIL;
	}
	AUI_PRINTF("ta_sample_test success\n");
	return AUI_RTN_SUCCESS;
}
/***************case 1: ta_sample_test   end **************************/

/***************case 2: ta_dsc_ram2ram  start*********************/
static unsigned long ta_dsc_ram2ram(unsigned long *argc,char **argv,char *sz_output)
{
		int ret = 0;
		TEEC_Result result = TEEC_SUCCESS;
	
		ta_client_operation.imp.session = &ta_client_session;
		ta_client_operation.imp.cancel_req = 0;
		ta_client_operation.imp.oper_state = OTZ_STATE_APP;
		ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
					TEEC_VALUE_INPUT,
					TEEC_NONE,
					TEEC_NONE,
					TEEC_NONE);
		ta_client_operation.started = 1;
		ta_client_operation.params[0].value.a = 0x83;
		ta_client_operation.params[0].value.b = 0x46;
	
		result = TEEC_InvokeCommand(
				&ta_client_session,
				AUI_TA_CMD_DSC_R2R,
				&ta_client_operation,
				NULL);
		if (result != TEEC_SUCCESS) {
			AUI_PRINTF("[REE] TEEC_InvokeCommand failed!\n");
			ret = result;
		}
		return ret;
}
/***************case 2: ta_dsc_ram2ram  end*********************/

/***************case 3: ta_dsc_ram2ram_kl  start*********************/
static unsigned long ta_dsc_ram2ram_kl(unsigned long *argc,char **argv,char *sz_output)
{
		int ret = 0;
		TEEC_Result result = TEEC_SUCCESS;
	
		ta_client_operation.imp.session = &ta_client_session;
		ta_client_operation.imp.cancel_req = 0;
		ta_client_operation.imp.oper_state = OTZ_STATE_APP;
		ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
					TEEC_VALUE_INPUT,
					TEEC_NONE,
					TEEC_NONE,
					TEEC_NONE);
		ta_client_operation.started = 1;
		ta_client_operation.params[0].value.a = 0x83;
		ta_client_operation.params[0].value.b = 0x46;
	
		result = TEEC_InvokeCommand(
				&ta_client_session,
				AUI_TA_CMD_DSC_R2R_KL,
				&ta_client_operation,
				NULL);
		if (result != TEEC_SUCCESS) {
			AUI_PRINTF("[REE] TEEC_InvokeCommand failed!\n");
			ret = result;
		}
		return ret;
}
/***************case 3: ta_dsc_ram2ram_kl  end*********************/

/***************case 4: ta_test_tsg_dsc  start*********************/
/* TSG parameters */
#define PKT_SIZE 188
#define RD_PKT_NUM 512
#define RD_BLK_SIZE 1024

extern unsigned long s_play_mode;

struct decrypt_conf {
    void *dev;
    unsigned short algo;
};
struct crypto_param {
    int mode;
    struct aui_cfg_kl kl_attr;
    struct aui_st_attr_dsc dsc_attr;
    unsigned char iv[16 * 2]; /* odd and even */
};
static struct crypto_param crypto_param;

static int ali_app_dsc_deinit(aui_hdl hdl)
{
    int ret = 0;
    TEEC_Result result = TEEC_SUCCESS;
    
    ta_client_operation.imp.session = &ta_client_session;
    ta_client_operation.imp.cancel_req = 0;
    ta_client_operation.imp.oper_state = OTZ_STATE_APP;
    ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
                TEEC_VALUE_INPUT,
                TEEC_NONE,
                TEEC_NONE,
                TEEC_NONE);
    ta_client_operation.started = 1;
    ta_client_operation.params[0].value.a = 0x83;
    ta_client_operation.params[0].value.b = 0x46;
    
    result = TEEC_InvokeCommand(
            &ta_client_session,
            AUI_TA_CMD_DECRYPT_CLOSE,
            &ta_client_operation,
            NULL);
    if (result != TEEC_SUCCESS) {
        AUI_PRINTF("[REE] TEEC_InvokeCommand failed!\n");
        ret = result;
    }
    return ret;

}

typedef enum aui_ta_decrypt_type {

    DECRYPT_STREAM_DSC = 1,

    DECRYPT_STREAM_DSC_KL
    
}aui_ta_decrypt_type;

static int aui_ta_app_crypto_session_init(aui_ta_decrypt_type type)
{
	int ret = 0;
	TEEC_Result result = TEEC_SUCCESS;
    int open_cmd = 0;
    int attach_cmd = 0;
    
    if(DECRYPT_STREAM_DSC == type) {
        open_cmd = AUI_TA_CMD_DSC_OPEN;
        attach_cmd = AUI_TA_CMD_DSC_ATTACH;
    } else if (DECRYPT_STREAM_DSC_KL == type){
        open_cmd = AUI_TA_CMD_DSC_KL_OPEN;
        attach_cmd = AUI_TA_CMD_DSC_KL_ATTACH;
    } else {
        AUI_PRINTF("ta type is error\n");
    }
    
	//step 1: ta dsc or kl open
	ta_client_operation.imp.session = &ta_client_session;
	ta_client_operation.imp.cancel_req = 0;
	ta_client_operation.imp.oper_state = OTZ_STATE_APP;
	ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
				TEEC_NONE,
				TEEC_NONE,
				TEEC_NONE,
				TEEC_NONE);
	ta_client_operation.started = 1;
	result = TEEC_InvokeCommand(
			&ta_client_session,
			open_cmd,
			&ta_client_operation,
			NULL);
	if (result != TEEC_SUCCESS) {
		AUI_PRINTF("TEEC_InvokeCommand failed\n");
		ret = result;
	}

	//step 2: ta  dsc or kl attach
	ta_client_operation.imp.session = &ta_client_session;
	ta_client_operation.imp.cancel_req = 0;
	ta_client_operation.imp.oper_state = OTZ_STATE_APP;
	ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
				TEEC_NONE,
				TEEC_NONE,
				TEEC_NONE,
				TEEC_NONE);
	ta_client_operation.started = 1;
	result = TEEC_InvokeCommand(
			&ta_client_session,
			attach_cmd,
			&ta_client_operation,
			NULL);
	if (result != TEEC_SUCCESS) {
		AUI_PRINTF("TEEC_InvokeCommand failed2\n");
		ret = result;
	}

	//step 3: ta get dmx dsc id

	return ret;
}

#define PAGE_SIZE 4096
#define ROUND_UP(N, S) ((((N) + (S) - 1) / (S)) * (S))


static int ta_get_dmx_dsc_id(int dmx_id, aui_dmx_dsc_id*  p_dmx_dsc_id)
{
	int ret = 0;
	static const TEEC_UUID aui_ta_uuid = AUI_TA_UUID;
	TEEC_SharedMemory sharedMem;
	TEEC_Result result = TEEC_SUCCESS;

	if(NULL == p_dmx_dsc_id) {
		AUI_PRINTF("p_dmx_dsc_id is null\n");
		return -1;
	}

	int buf_len = ROUND_UP(sizeof(aui_dmx_dsc_id), PAGE_SIZE);

	/*Allocate share memory*/
	sharedMem.size = buf_len;
	sharedMem.flags = TEEC_MEM_OUTPUT ;
	result = TEEC_AllocateSharedMemory(
			&ta_client_context,
			&sharedMem);
	if(result != TEEC_SUCCESS) {
		printf("TEEC_AllocateSharedMemory failed2\n");
        ret = result;
        goto cleanup_1;
    }

	printf("[SharedMemory]---------sharedMem.buffer: 0x%x\n", (unsigned int)sharedMem.buffer);
	memset(sharedMem.buffer, 0, buf_len);

	//step 3: ta get dmx dsc id
	ta_client_operation.imp.session = &ta_client_session;
	ta_client_operation.imp.cancel_req = 0;
	ta_client_operation.imp.oper_state = OTZ_STATE_APP;
	ta_client_operation.paramTypes = TEEC_PARAM_TYPES(
				TEEC_VALUE_INPUT,
				TEEC_MEMREF_PARTIAL_OUTPUT,
				TEEC_NONE,
				TEEC_NONE);
	ta_client_operation.started = 1;
	ta_client_operation.params[0].value.a = dmx_id;

	ta_client_operation.params[1].memref.parent = &sharedMem;
	ta_client_operation.params[1].memref.size = sizeof(aui_dmx_dsc_id);
	ta_client_operation.params[1].memref.offset = 0;

	result = TEEC_InvokeCommand(
			&ta_client_session,
			AUI_TA_CMD_DMX_DSC_ID_GET,
			&ta_client_operation,
			NULL);
	if (result != TEEC_SUCCESS) {
		printf("TEEC_InvokeCommand failed2\n");
		ret = result;
	}
	memcpy(p_dmx_dsc_id, ta_client_operation.params[1].memref.parent->buffer,  sizeof(aui_dmx_dsc_id));
    int i =0;
    for(i = 0;i<16;i++) {
        printf("523 ta_get_dmx_dsc_id :\n");
        printf("%02x",p_dmx_dsc_id->identifier[i]);
    }
    
cleanup_1:
	TEEC_ReleaseSharedMemory(&sharedMem);
	return ret;
}

static void show_tsg_usage() {
    AUI_PRINTF("command as fallow:\n");
    AUI_PRINTF("cmd_num [path],[vpid],[apid],[ppid],[video format],[audio format],[dis format]\n");
    AUI_PRINTF("such as :4 /mnt/usb/sda1/dsc_kl_test_streams/stream_nagra_csa.ts,4368,4370,4368,0,0,0\n");
    AUI_PRINTF("such as :5 /mnt/usb/sda1/dsc_kl_test_streams/nagra_128_odd_even_TDES.ts,4368,4370,4368,0,0,0\n");
}

static unsigned long ta_test_tsg_dsc(unsigned long *argc,char **argv,char *sz_out_put)
{
    struct ali_aui_hdls aui_hdls;
    struct aui_attr_tsg tsg_attr;
    struct ali_app_modules_init_para init_para;
    unsigned long pkt_empty,pkt_in_tsg,pkt_cnt;
    int len, ret;
    FILE *file;

    *sz_out_put = 0;    
    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    unsigned long i;
    AUI_PRINTF("*argc = %d,argv: ",*argc);
    for(i = 0;i < *argc;i++){
        AUI_PRINTF("%s,",argv[i]);
    }
    AUI_PRINTF("\n");
    if (7 != *argc) {
        show_tsg_usage();
        return 0;
    }

    len = (PKT_SIZE * RD_PKT_NUM) / RD_BLK_SIZE;  //94K 
    pkt_cnt = RD_PKT_NUM;

    /*init init_para variable*/
    ali_aui_init_para_for_test_tsg(argc,argv,&init_para);
	
    //init tsg device
    if(ali_app_tsg_init(&init_para.init_para_tsg, &aui_hdls.tsg_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsg_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSG opened\n");

    //init tsi device
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSI opened\n");

    //init dsc device device
    MEMSET(&crypto_param, 0, sizeof(crypto_param));
    crypto_param.mode = DECRYPT_STREAM_DSC;
    if (aui_ta_app_crypto_session_init((aui_ta_decrypt_type)crypto_param.mode)){
        AUI_PRINTF("\r\n ali_ta_app_crypto_session_init failed!");
        return -1;
    }

    //init dis device
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DIS opened\n");

    //init decv device
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        AUI_PRINTF("\r\n ali_app_decv_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DECV opened\n");

    // init deca device
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        AUI_PRINTF("\r\n ali_app_audio_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    /*set video and audio  synchronous ways,TS from nim,set to AUI_STC_AVSYNC_PCR.
     *TS from tsg,set to AUI_STC_AVSYNC_AUDIO*/
    if(aui_decv_sync_mode(aui_hdls.decv_hdl,AUI_STC_AVSYNC_AUDIO)){
        AUI_PRINTF("Set AUI_DEVC_SYNC_AUDIO fail\n");
        goto err_tsg;
    }

    /*init dmx devce*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
        AUI_PRINTF("\r\n set dmx failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;
    MEMSET(&path, 0, sizeof(aui_dmx_data_path));

    //path.p_hdl_de_dev = aui_hdls.dsc_hdl;
    path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
	aui_dmx_dsc_id dmx_dsc_id;
	memset(&dmx_dsc_id, 0, sizeof(aui_dmx_dsc_id));
    if(ta_get_dmx_dsc_id(init_para.dmx_create_av_para.dmx_id, &dmx_dsc_id)) {
        AUI_PRINTF("\r\n ta_get_dmx_dsc_id failed!");
        return -1;
    }
    path.p_dsc_id = &dmx_dsc_id;
		//AUI_PRINTF("AUI DSC/KL opened %p/%p\n", aui_hdls.dsc_hdl, aui_hdls.kl_hdl);
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
        goto err_tsg;
    }
    AUI_PRINTF("dmx data path set %d\n", path.data_path_type);


    /*set TS of dmx device from local media player*/
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,(void *)AUI_AVSYNC_FROM_HDD_MP)) {
        AUI_PRINTF("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
        goto err_tsg;
    }
    
    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices 
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    else{
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL))
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    
    AUI_PRINTF("tsg file %s\n", argv[0]);
    file = fopen(argv[0], "r");
    if (!file) {
        AUI_PRINTF("tsg file open errno\n");
        goto err_tsg;
    }
    void *buffer = malloc((RD_BLK_SIZE*len));
    if(NULL == buffer)
    {
        AUI_PRINTF("tsg malloc errno\n");
        goto err_tsg;
    }


    while (1) {
		usleep(1000);
        ret = fread(buffer, RD_BLK_SIZE, len, file);
        if (0 == ret) {
            AUI_PRINTF("tsg file open errno \n");
            fclose(file);
            goto err_tsg;
        }
    #ifdef AUI_LINUX
        if (ret != len) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len);
			sleep(10);
            break;
        }
    #else
        if (ret != len*RD_BLK_SIZE) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len*RD_BLK_SIZE);
            break;
        }
    #endif


        if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
        if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
            AUI_PRINTF("aui_dmx_get error\n");

        while(pkt_empty < (pkt_cnt + pkt_in_tsg)) {
            AUI_SLEEP(10);
            if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
                AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                goto err_tsg;
            }
            if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
                AUI_PRINTF("aui_dmx_get error\n");
        }
        /* push data in TSG */
        tsg_attr.ul_addr = (unsigned char *)buffer;
        tsg_attr.ul_pkg_cnt = pkt_cnt;
        tsg_attr.uc_sync_mode = 0;
        ret = aui_tsg_send(aui_hdls.tsg_hdl, &tsg_attr);
        if (ret) {
            AUI_PRINTF("\naui_tsg_send error 0x%x\n", ret);
            fclose(file);
            goto err_tsg;
        }
    }
    fclose(file);

err_tsg:
	sleep(10);
    AUI_PRINTF("\r\n close dsi aui");
    if (aui_hdls.dsc_hdl && ali_app_dsc_deinit(aui_hdls.dsc_hdl))
        AUI_PRINTF("\r\n ali_app_dsc_deinit failed!");
    ali_app_deinit(&aui_hdls);
    return AUI_RTN_SUCCESS;
}
/***************case 4: ta_test_tsg_dsc  end*********************/


/***************case 5: ta_test_tsg_dsc_kl  start*********************/
static unsigned long ta_test_tsg_dsc_kl(unsigned long *argc,char **argv,char *sz_out_put)
{
    struct ali_aui_hdls aui_hdls;
    struct aui_attr_tsg tsg_attr;
    struct ali_app_modules_init_para init_para;
    unsigned long pkt_empty,pkt_in_tsg,pkt_cnt;
    int len, ret;
    FILE *file;

    *sz_out_put = 0;    
    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    unsigned long i;
    AUI_PRINTF("*argc = %d,argv: ",*argc);
    for(i = 0;i < *argc;i++){
        AUI_PRINTF("%s,",argv[i]);
    }
    AUI_PRINTF("\n");
    if (7 != *argc) {
        show_tsg_usage();
        return 0;
    }

    len = (PKT_SIZE * RD_PKT_NUM) / RD_BLK_SIZE;  //94K 
    pkt_cnt = RD_PKT_NUM;

    /*init init_para variable*/
    ali_aui_init_para_for_test_tsg(argc,argv,&init_para);
	
    //init tsg device
    if(ali_app_tsg_init(&init_para.init_para_tsg, &aui_hdls.tsg_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsg_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSG opened\n");

    //init tsi device
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSI opened\n");

    //init dsc device device
    MEMSET(&crypto_param, 0, sizeof(crypto_param));
    crypto_param.mode = DECRYPT_STREAM_DSC_KL;
    if (aui_ta_app_crypto_session_init((aui_ta_decrypt_type)crypto_param.mode)){
        AUI_PRINTF("\r\n ali_ta_app_crypto_session_init failed!");
        return -1;
    }
		//AUI_PRINTF("AUI DSC/KL opened %p/%p\n", aui_hdls.dsc_hdl, aui_hdls.kl_hdl);

#ifdef AUI_LINUX
    //init dis device
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DIS opened\n");
#endif

    //init decv device
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        AUI_PRINTF("\r\n ali_app_decv_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DECV opened\n");

    // init deca device
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        AUI_PRINTF("\r\n ali_app_audio_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    /*set video and audio  synchronous ways,TS from nim,set to AUI_STC_AVSYNC_PCR.
     *TS from tsg,set to AUI_STC_AVSYNC_AUDIO*/
    if(aui_decv_sync_mode(aui_hdls.decv_hdl,AUI_STC_AVSYNC_AUDIO)){
        AUI_PRINTF("Set AUI_DEVC_SYNC_AUDIO fail\n");
        goto err_tsg;
    }

    /*init dmx devce*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
        AUI_PRINTF("\r\n set dmx failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;
    MEMSET(&path, 0, sizeof(aui_dmx_data_path));

    //path.p_hdl_de_dev = aui_hdls.dsc_hdl;
    path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
    aui_dmx_dsc_id dmx_dsc_id;
	memset(&dmx_dsc_id, 0, sizeof(aui_dmx_dsc_id));
    if(ta_get_dmx_dsc_id(init_para.dmx_create_av_para.dmx_id, &dmx_dsc_id)) {
        AUI_PRINTF("\r\n ta_get_dmx_dsc_id failed!");
        return -1;
    }
    path.p_dsc_id = &dmx_dsc_id;
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
        goto err_tsg;
    }
    AUI_PRINTF("dmx data path set %d\n", path.data_path_type);


    /*set TS of dmx device from local media player*/
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,(void *)AUI_AVSYNC_FROM_HDD_MP)) {
        AUI_PRINTF("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
        goto err_tsg;
    }
    
    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices 
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    else{
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL))
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }

    
    AUI_PRINTF("tsg file %s\n", argv[0]);
    file = fopen(argv[0], "r");
    if (!file) {
        AUI_PRINTF("tsg file open errno\n");
        goto err_tsg;
    }
    void *buffer = malloc((RD_BLK_SIZE*len));
    if(NULL == buffer)
    {
        AUI_PRINTF("tsg malloc errno\n");
        goto err_tsg;
    }


    while (1) {
		usleep(1000);
        ret = fread(buffer, RD_BLK_SIZE, len, file);
        if (0 == ret) {
            AUI_PRINTF("tsg file open errno \n");
            fclose(file);
            goto err_tsg;
        }
    #ifdef AUI_LINUX
        if (ret != len) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len);
            break;
        }
    #else
        if (ret != len*RD_BLK_SIZE) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len*RD_BLK_SIZE);
            break;
        }
    #endif


        if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
        if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
            AUI_PRINTF("aui_dmx_get error\n");

        while(pkt_empty < (pkt_cnt + pkt_in_tsg)) {
            AUI_SLEEP(10);
            if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
                AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                goto err_tsg;
            }
            if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
                AUI_PRINTF("aui_dmx_get error\n");
        }
        /* push data in TSG */
        tsg_attr.ul_addr = (unsigned char *)buffer;
        tsg_attr.ul_pkg_cnt = pkt_cnt;
        tsg_attr.uc_sync_mode = 0;
        ret = aui_tsg_send(aui_hdls.tsg_hdl, &tsg_attr);
        if (ret) {
            AUI_PRINTF("\naui_tsg_send error 0x%x\n", ret);
            fclose(file);
            goto err_tsg;
        }
    }
    fclose(file);

err_tsg:
	sleep(10);
    AUI_PRINTF("\r\n close dsi aui");
    if (aui_hdls.dsc_hdl && ali_app_dsc_deinit(aui_hdls.dsc_hdl))
        AUI_PRINTF("\r\n ali_app_dsc_deinit failed!");

    ali_app_deinit(&aui_hdls);
    return AUI_RTN_SUCCESS;
}
/***************case 5: ta_test_tsg_dsc_kl  end*********************/

void ta_test_reg(void)
{
	aui_tu_reg_group("ta", "ta tests");
    aui_tu_reg_item(2, "open", AUI_CMD_TYPE_API, ta_init_session, "ta init & open session");
    aui_tu_reg_item(2, "close", AUI_CMD_TYPE_API, ta_deinit_session, "ta close & deinit session");
    aui_tu_reg_item(2, "log", AUI_CMD_TYPE_API, aui_ta_set_log_level_test, "Set log level in TEE. ERR=3, WARN=4,INFO=6, DBG=7,e.g log 7");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, ta_sample_test, "ta_sample_test");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, ta_dsc_ram2ram, "ta_dsc_ram2ram decrypt raw");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, ta_dsc_ram2ram_kl, "ta_dsc_ram2ram_kl decrypt raw");
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, ta_test_tsg_dsc, "ta_test_tsg_dsc");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, ta_test_tsg_dsc_kl, "ta_test_tsg_dsc_kl");
}

