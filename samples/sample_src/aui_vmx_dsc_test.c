#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

#include <aui_dsc.h>
#include <aui_kl.h>
#include <aui_vmx.h>
#include <aui_dmx.h>

#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <linux/ali_sec.h>
//#include <ca_dsc.h>
#include <alidefinition/adf_dsc.h>
#include <alislvmx.h>

#include "aui_help_print.h"
#include "aui_test_stream.h"

#include "aui_test_app_cmd.h"


extern unsigned long s_play_mode;
extern int aui_dsc_is_vmx_module(aui_hdl handle);
extern AUI_RTN_CODE aui_vmx_fd_get(aui_hdl handle, int *vmx_fd);
extern AUI_RTN_CODE aui_vmx_service_index_get(aui_hdl handle, int *service_index);

#define VMX_TEST_DEBUG
#ifdef VMX_TEST_DEBUG
   #define VMX_TEST_PRINT(fmt, ...) \
     printf("\033[0;32;32mFUNC:%s,LINE:%d-->\033[m"fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
 
   #define VMX_DSC_DUMP(prompt,data,len) \
     do{ \
         int k, l=(len); \
         printf("\033[0;32;32m%s\033[m\n",prompt);\
         for(k=0; k<l; k++){ \
             printf("0x%02X,",*(unsigned char*)((unsigned int)data+k)); \
             if((k+1)%16==0) \
                 printf("\n"); \
         } \
         if((k)%16!=0) \
             printf("\n"); \
     }while(0)                                                                                                                                                                                                 
     
#else
    #define VMX_TEST_PRINT(fmt, ...)   do{}while(0)
    #define VMX_DSC_DUMP(prompt,data,len) do{}while(0)  
#endif

enum {
    STREAM,
    STREAM_DSC,
    STREAM_DSC_KL
};

#define KEY_LEN 16
#define PKT_SIZE 188
#define RD_PKT_NUM 512
#define RD_BLK_SIZE 1024

static int vmx_msg_callback_hdl(void *pv_user_data, void *param)
{
    aui_vmx_callback_attr *attr = (aui_vmx_callback_attr *)param;
	int ret = 0;
	printf("@@@%s[%d]: callback_type: %d, buff[0]=%c, ecm_value[%d]=0x%x\n",__func__,__LINE__,\
			attr->callback_type, attr->buffer[0], attr->buffer_size-1, attr->buffer[attr->buffer_size-1]);
    
	return ret;
}

static unsigned long vmx_test_open(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;
    int cnt = 0;
    int i;
    aui_hdl hdl;
    int err = 0;
    aui_vmx_msg_callback_attr callback_attr;
    aui_vmx_dsc_attr vmx_attr;

    int service_index[] = {
        AUI_VMX_SERVICE_DVB0,
        AUI_VMX_SERVICE_DVB1,
        /*
        AUI_VMX_SERVICE_DVB2,
        AUI_VMX_SERVICE_IPTV0,
        AUI_VMX_SERVICE_IPTV1,
        AUI_VMX_SERVICE_IPTV2,
        AUI_VMX_SERVICE_DVR0,
        AUI_VMX_SERVICE_DVR1,
        AUI_VMX_SERVICE_DVR2,
        AUI_VMX_SERVICE_OTT0,
        AUI_VMX_SERVICE_OTT1,
        */
    };    
    memset(&vmx_attr, 0, sizeof(aui_vmx_dsc_attr));

////////////////////////////            
	aui_hdl aui_dsc_hdl;
	aui_attr_dsc dsc_attr;

    memset(&dsc_attr,0, sizeof(aui_attr_dsc));
    dsc_attr.csa_version=AUI_DSC_CSA2;
    dsc_attr.dsc_data_type=AUI_DSC_DATA_TS;
    dsc_attr.uc_dev_idx=0;
    dsc_attr.uc_algo=AUI_DSC_ALGO_CSA;
    dsc_attr.dsc_key_type=AUI_DSC_HOST_KEY_SRAM;
    if(AUI_RTN_SUCCESS!=aui_dsc_open(&dsc_attr,&aui_dsc_hdl)) {
        printf("\r\n can not open dsc dev!\n");
        return 0;
    }
    if (aui_dsc_is_vmx_module(aui_dsc_hdl)){
        printf("line: %d, aui dsc handle:0x%x is VMX handle!\n", __LINE__, aui_dsc_hdl);
    }
    else{
        printf("line: %d, aui dsc handle:0x%x is NOT VMX handle!\n", __LINE__, aui_dsc_hdl);
    }
    aui_dsc_close(aui_dsc_hdl);
////////////////////////////            


    for (i = 0; i < (int)(sizeof(service_index)/sizeof(int)); i ++)  
    {
        printf("%s(),(%d):service_index = 0x%x!\n", __FUNCTION__, i, service_index[i]);
        vmx_attr.uc_dev_idx = i;
        vmx_attr.service_index = service_index[i];
        if(aui_vmx_open(&vmx_attr, &hdl)){
            printf("%d open device fail!\n", cnt);
            err ++;
        }
        else{
            printf("%d open device OK!\n", cnt);
            callback_attr.pv_user_data = NULL;
            callback_attr.p_fn_vmx_msg_cb = vmx_msg_callback_hdl;
            aui_vmx_register_message_callback(hdl, &callback_attr);

//////////////////////////////////////////////
            if (aui_dsc_is_vmx_module(hdl)){
                printf("aui handle:0x%x is an VMX handle!\n", hdl);
            }
            else{
                printf("aui handle:0x%x is NOT an VMX handle!\n", hdl);
            }
//////////////////////////////////////////////            

        }
        sleep(10);
        if(aui_vmx_close(hdl)){
            printf("%d close device fail!\n", cnt);
            err ++;
        }
        else
            printf("%d close device OK!\n", cnt);
        cnt ++;
    }


    return err;    
}


static unsigned long vmx_test_ca_algo(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;

    int err = 0;
    int i;
    int cnt = 0;
    aui_hdl hdl;
    AUI_RTN_CODE ret;
    aui_vmx_dsc_algo dsc_algo;
    aui_vmx_dsc_attr vmx_attr;

    aui_vmx_service_index service_index[] = {
        AUI_VMX_SERVICE_DVB0,
        AUI_VMX_SERVICE_DVB1,
        /*
        AUI_VMX_SERVICE_DVB2,
        AUI_VMX_SERVICE_IPTV0,
        AUI_VMX_SERVICE_IPTV1,
        AUI_VMX_SERVICE_IPTV2,
        AUI_VMX_SERVICE_DVR0,
        AUI_VMX_SERVICE_DVR1,
        AUI_VMX_SERVICE_DVR2,
        AUI_VMX_SERVICE_OTT0,
        AUI_VMX_SERVICE_OTT1,
        */
    };    
    memset(&vmx_attr, 0, sizeof(aui_vmx_dsc_attr));

    unsigned char ivbuf[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

    dsc_algo.ca_algo = AUI_VMX_ALGO_FTA;
    dsc_algo.ca_mode = AUI_VMX_CRYPTO_MODE_ECB;
    memcpy(dsc_algo.iv_value, ivbuf, sizeof(ivbuf));
    
    for (i = 0; i < (int)(sizeof(service_index)/sizeof(int)); i ++)  
    {
        vmx_attr.uc_dev_idx = i;
        vmx_attr.service_index = service_index[i];
        if(aui_vmx_open(&vmx_attr, &hdl)){
            printf("%d open device fail!\n", cnt);
            err ++;
        }
        ret = aui_vmx_dsc_algo_set(hdl, &dsc_algo);
        if (ret){
            printf("aui_vmx_dsc_algo_set() fail!\n");
            err ++;
        }
        else
            printf("aui_vmx_dsc_algo_set() OK!\n");
        
        if(aui_vmx_close(hdl)){
            printf("%d close device fail!\n", cnt);
            err ++;
        }
    }
    return err;    
}

static unsigned long vmx_test_set_pid(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;

    int err = 0;
    int i;
    int cnt = 0;
    aui_hdl hdl;
    AUI_RTN_CODE ret;

    aui_vmx_dsc_attr vmx_attr;

    aui_vmx_service_index service_index[] = {
        AUI_VMX_SERVICE_DVB0,
        AUI_VMX_SERVICE_DVB1,
        /*
        AUI_VMX_SERVICE_DVB2,
        AUI_VMX_SERVICE_IPTV0,
        AUI_VMX_SERVICE_IPTV1,
        AUI_VMX_SERVICE_IPTV2,
        AUI_VMX_SERVICE_DVR0,
        AUI_VMX_SERVICE_DVR1,
        AUI_VMX_SERVICE_DVR2,
        AUI_VMX_SERVICE_OTT0,
        AUI_VMX_SERVICE_OTT1,
        */
    };    
    memset(&vmx_attr, 0, sizeof(aui_vmx_dsc_attr));

    aui_vmx_pid_info pid_info;
    memset(&pid_info, 0, sizeof(pid_info));
    pid_info.pid_number = 13;
    pid_info.pid_table[12] = 12;
    for (i = 0; i < (int)(sizeof(service_index)/sizeof(int)); i ++)  
    {
        vmx_attr.uc_dev_idx = i;
        vmx_attr.service_index = service_index[i];
        if(aui_vmx_open(&vmx_attr, &hdl)){
            printf("%d open device fail!\n", cnt);
            err ++;
        }
        ret = aui_vmx_pid_set(hdl, &pid_info);
        if (ret){
            printf("aui_vmx_pid_set() fail!\n");
            err ++;
        }
        else
            printf("aui_vmx_pid_set() OK!\n");
        
        if(aui_vmx_close(hdl)){
            printf("%d close device fail!\n", cnt);
            err ++;
        }
    }
    return err;    
}

static unsigned long vmx_test_get_status(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;

    int err = 0;
    int i;
    int cnt = 0;
    aui_hdl hdl;
    AUI_RTN_CODE ret;

    aui_vmx_dsc_attr vmx_attr;

    aui_vmx_service_index service_index[] = {
        AUI_VMX_SERVICE_DVB0,
        AUI_VMX_SERVICE_DVB1,
        /*
        AUI_VMX_SERVICE_DVB2,
        AUI_VMX_SERVICE_IPTV0,
        AUI_VMX_SERVICE_IPTV1,
        AUI_VMX_SERVICE_IPTV2,
        AUI_VMX_SERVICE_DVR0,
        AUI_VMX_SERVICE_DVR1,
        AUI_VMX_SERVICE_DVR2,
        AUI_VMX_SERVICE_OTT0,
        AUI_VMX_SERVICE_OTT1,
        */
    };    
    memset(&vmx_attr, 0, sizeof(aui_vmx_dsc_attr));

    aui_vmx_dsc_status dsc_status;

    memset(&dsc_status, 0, sizeof(dsc_status));
    dsc_status.rec_block_count = 0;
    for (i = 0; i < (int)(sizeof(service_index)/sizeof(int)); i ++)  
    {
        vmx_attr.uc_dev_idx = i;
        vmx_attr.service_index = service_index[i];
        if(aui_vmx_open(&vmx_attr, &hdl)){
            printf("%d open device fail!\n", cnt);
            err ++;
        }
        ret = aui_vmx_dsc_status_get(hdl, &dsc_status);
        if (ret){
            printf("aui_vmx_dsc_status_get fail!\n");
            err ++;
        }
        else{
            printf("aui_vmx_dsc_status_get OK!, block count:%d\n", dsc_status.rec_block_count);
            printf("rec block_count = %d\n", dsc_status.rec_block_count);
        }
        
        if(aui_vmx_close(hdl)){
            printf("%d close device fail!\n", cnt);
            err ++;
        }
    }
    return err;    
}

static unsigned long test_vmx_help()
{
    aui_print_help_header("\nVMX Test Help");

   /* STREAM_4_HELP */
    #define     STREAM_4_HELP_PART1         "Play the TS stream with DSC from storage medium."
    #define     STREAM_4_HELP_PART2         "Format:    5   [path],[vpid],[apid],[ppid],[video format],[audio format],[dis format],[tsg clk sel]"
    #define     STREAM_4_HELP_PART3         "               [path]: the stream file path."
    #define     STREAM_4_HELP_PART4         "               [vpid]: video pid"
    #define     STREAM_4_HELP_PART5         "               [apid]: audio pid"
    #define     STREAM_4_HELP_PART6         "               [ppid]: pcr pid"
    #define     STREAM_4_HELP_PART7         "               [video format]:0: MPEG    1: AVC(H.264) 10: HEVC(H.265)"
    #define     STREAM_4_HELP_PART8         "               [audio format]:0:  AUI_DECA_STREAM_TYPE_MPEG1"
    #define     STREAM_4_HELP_PART9         "                              1:  AUI_DECA_STREAM_TYPE_MPEG2"
    #define     STREAM_4_HELP_PART10        "                              2:  AUI_DECA_STREAM_TYPE_AAC_LATM"
    #define     STREAM_4_HELP_PART11        "                              3:  AUI_DECA_STREAM_TYPE_AC3"
    #define     STREAM_4_HELP_PART12        "                              4:  AUI_DECA_STREAM_TYPE_DTS"
    #define     STREAM_4_HELP_PART13        "                              5:  AUI_DECA_STREAM_TYPE_PPCM"
    #define     STREAM_4_HELP_PART14        "                              6:  AUI_DECA_STREAM_TYPE_LPCM_V"
    #define     STREAM_4_HELP_PART15        "                              7:  AUI_DECA_STREAM_TYPE_LPCM_A"
    #define     STREAM_4_HELP_PART16        "                              8:  AUI_DECA_STREAM_TYPE_PCM"
    #define     STREAM_4_HELP_PART17        "                              9:  AUI_DECA_STREAM_TYPE_WMA"
    #define     STREAM_4_HELP_PART18        "                              10: AUI_DECA_STREAM_TYPE_RA8"
    #define     STREAM_4_HELP_PART19        "                              11: AUI_DECA_STREAM_TYPE_MP3"
    #define     STREAM_4_HELP_PART20        "                              12: AUI_DECA_STREAM_TYPE_AAC_ADTS"
    #define     STREAM_4_HELP_PART21        "                              13: AUI_DECA_STREAM_TYPE_OGG"
    #define     STREAM_4_HELP_PART22        "                              14: AUI_DECA_STREAM_TYPE_EC3"
    #define     STREAM_4_HELP_PART23        "              [dis format]: 0: HD(High Definition)    1: SD(Standard Definition)"
    #define     STREAM_4_HELP_PART24        "              [tsg clk sel]: integer > 0, tsg clock select, when the stream with high bitrate can not play smoothly, decrease the select value, default 8"
    #define     STREAM_4_HELP_PART25        "Example:   If the stream file storing the U disk that the file path is /mnt/usb/sda1/vmx_as/vmx_as_cctv2_encrypt.ts whose vpid is 513, apid is 660, "
    #define     STREAM_4_HELP_PART26        "           ppid is 8190 is played, the video format is MPEG, the audio format is MPEG2, the dis format is high definition is played, the input is:"
    #define     STREAM_4_HELP_PART27        "           5 /mnt/usb/sda1/vmx_as/vmx_as_cctv2_encrypt.ts,513,660,8190,0,1,0\n"
    aui_print_help_command("\'5\'");
    aui_print_help_instruction_newline(STREAM_4_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART3);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART4);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART5);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART6);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART7);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART8);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART9);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART10);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART11);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART12);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART13);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART14);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART15);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART16);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART17);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART18);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART19);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART20);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART21);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART22);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART23);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART24);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART25);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART26);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART27);

    /* STREAM_5_HELP */
    #define     STREAM_5_HELP_PART1         "Play the TS stream with VMX function from storage medium."
    #define     STREAM_5_HELP_PART2         "The inputting parameter of the command is the same as the \"5\" command except the first one which needs \"6\" instead of  \"5\"."
    #define     STREAM_5_HELP_PART3         "Please see the help of the \"5\" command for more details for the inputting parameter\n"

    aui_print_help_command("\'6\'");
    aui_print_help_instruction_newline(STREAM_5_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_5_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_5_HELP_PART3);
    
    return 0;
}


struct crypto_param {
    int mode;
    struct aui_cfg_kl kl_attr;
    struct aui_st_attr_dsc dsc_attr;
    unsigned char iv[16 * 2]; /* odd and even */
};
static struct crypto_param crypto_param;

static unsigned char vmx_clear_key[] = {0x1a,0x3c,0x16,0xf8,0xd7,0x66,0x43,0xc3,0x5c,0x9e,
                    0x64,0x23,0x67,0x07,0x60,0x06}; /* even */


static unsigned char vmx_iv[] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
        0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};

static unsigned short vmx_pid[] = {513,660};

static int ali_app_dsc_init(unsigned short dsc_id, aui_hdl *p_dsc_hdl, aui_hdl *p_kl_hdl)
{
    int i;

    /*
     * Descrambler initialization
     */
    crypto_param.dsc_attr.uc_dev_idx = dsc_id;
    crypto_param.dsc_attr.uc_algo = AUI_DSC_ALGO_AES;
    crypto_param.dsc_attr.csa_version = AUI_DSC_CSA1;
    crypto_param.dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;

//

    if (aui_dsc_open(&crypto_param.dsc_attr, p_dsc_hdl)) {
        VMX_TEST_PRINT("dsc open error\n");
        return 1;
    }

    if (crypto_param.mode == STREAM_DSC) {
        crypto_param.dsc_attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
        /* odd and even */
        /*
        crypto_param.dsc_attr.puc_key = (unsigned char *)clear_key;
        crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
        crypto_param.dsc_attr.ul_key_len = KEY_LEN*8;
        crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
        */

        crypto_param.dsc_attr.puc_key = (unsigned char *)vmx_clear_key;
        crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_EVEN;
        crypto_param.dsc_attr.ul_key_len = KEY_LEN*8;
        crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE;
        
    } else {
        /* odd and even */
        crypto_param.dsc_attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;

        crypto_param.dsc_attr.puc_key = NULL;
        crypto_param.dsc_attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD_EVEN;
        crypto_param.dsc_attr.ul_key_len = KEY_LEN*8;
        crypto_param.dsc_attr.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0;
    }

    crypto_param.dsc_attr.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
    crypto_param.dsc_attr.puc_iv_ctr = vmx_iv;
    crypto_param.dsc_attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

    crypto_param.dsc_attr.ul_pid_cnt = 2;
    crypto_param.dsc_attr.pus_pids = vmx_pid;

    if (aui_dsc_attach_key_info2dsc(*p_dsc_hdl, &crypto_param.dsc_attr)) {
        VMX_TEST_PRINT("dsc attach key error\n");
        return 1;
    }
    return 0;
}

static int ali_app_vmx_play_init(unsigned short dsc_id, aui_hdl *p_dsc_hdl, aui_hdl *p_kl_hdl)
{
    int ret = 0;
    aui_vmx_dsc_attr vmx_attr;
    aui_vmx_msg_callback_attr callback_attr;    
    int err = 0;
    aui_hdl hdl = NULL;
    int vmx_fd = -1;
	SEC_TEST_PARAM sec_test;
    
    int service_index[] = {
        AUI_VMX_SERVICE_DVB0,
        /*
        AUI_VMX_SERVICE_DVB1,
        AUI_VMX_SERVICE_DVB2,
        AUI_VMX_SERVICE_IPTV0,
        AUI_VMX_SERVICE_IPTV1,
        AUI_VMX_SERVICE_IPTV2,
        AUI_VMX_SERVICE_DVR0,
        AUI_VMX_SERVICE_DVR1,
        AUI_VMX_SERVICE_DVR2,
        AUI_VMX_SERVICE_OTT0,
        AUI_VMX_SERVICE_OTT1,
        */
    };    
    memset(&vmx_attr, 0, sizeof(aui_vmx_dsc_attr));

////////////////////////////            
    /* open vmx device */
    vmx_attr.uc_dev_idx = dsc_id;
    vmx_attr.service_index = service_index[0];
    if(aui_vmx_open(&vmx_attr, &hdl)){
        printf("%d open device fail!\n");
        err ++;
        return err;
    }
    else{
        printf("%d open device OK!\n");
        *p_dsc_hdl = hdl;
        callback_attr.pv_user_data = NULL;
        callback_attr.p_fn_vmx_msg_cb = vmx_msg_callback_hdl;
        aui_vmx_register_message_callback(hdl, &callback_attr);

        if (aui_dsc_is_vmx_module(hdl)){
            printf("aui handle:0x%x is an VMX handle!\n", hdl);
        }
        else{
            printf("aui handle:0x%x is NOT an VMX handle!\n", hdl);
        }
        if (aui_vmx_fd_get(hdl, &vmx_fd)){
            VMX_TEST_PRINT("aui_vmx_fd_get error!\n");
            err ++;
            goto err_vmx_play_exit;
        }else {
            VMX_TEST_PRINT("aui_vmx_fd_get fd=%d\n!", vmx_fd);            
        }
    }

// init vmxsec module
    memset(&sec_test, 0, sizeof(sec_test));
	sec_test.bServiceIdx = service_index[0];
	sec_test.cmd = 3;
	ret = ioctl(vmx_fd,IO_SEC_TEST_DSC,&sec_test);
	if (ret != 0){
        VMX_TEST_PRINT("init vmxsec module fail!\n");
        err ++;
        goto err_vmx_play_exit;
	}else {
		VMX_TEST_PRINT("init vmxsec module OK!\n");
	}


   //set ca_algo_param, DVB, IPTV service (sevice index < 0x80)must set algo; 
   //other services should not set
    aui_vmx_dsc_algo dsc_algo;
    dsc_algo.ca_algo = AUI_VMX_ALGO_AES_128;
    dsc_algo.ca_mode = AUI_VMX_CRYPTO_MODE_ECB;
    memcpy(dsc_algo.iv_value, vmx_iv, sizeof(vmx_iv));
    
    ret = aui_vmx_dsc_algo_set(hdl, &dsc_algo);
    if (ret){
        printf("aui_vmx_dsc_algo_set error.\n");
        err ++;
        goto err_vmx_play_exit;
    }
    
    //set TS pid, DVR service(0x8x) must set PID.
    //other services should not set
    aui_vmx_pid_info pid_info;
    memset(&pid_info, 0, sizeof(pid_info));
    pid_info.pid_number = 2;
    memcpy(pid_info.pid_table, vmx_pid, sizeof(unsigned short) * pid_info.pid_number);
    
    ret = aui_vmx_pid_set(hdl, &pid_info);
    if (ret){
        printf("aui_vmx_pid_set error.\n");
        //err ++;
        //goto err_vmx_play_exit;
    }


    /* set ca_key */
    //unsigned char *even_key[8]={0x70,0x28,0xE4,0x7C,0x73,0x0C,0x2F,0xAE};
    //unsigned char *odd_key[8]={0x70,0x6D,0x59,0x36,0x60,0x0B,0x00,0x6B};
    memset(&sec_test, 0, sizeof(sec_test));
    sec_test.bServiceIdx = service_index[0];
    sec_test.cmd = 1;
    sec_test.dsc_algo = AES; //CSA
    sec_test.data_type=(1<<24); //TS_MODE
    sec_test.key_size=KEY_LEN;
    sec_test.key_pattern=1;
    sec_test.pid_num=2;
    sec_test.en_de_crypt=1;
    sec_test.pid_list[0]=vmx_pid[0];
    sec_test.pid_list[1]=vmx_pid[1];
    sec_test.root_key_pos=0xff;
    memcpy(&sec_test.content_key[0], vmx_clear_key, KEY_LEN);
    memcpy(&sec_test.iv_ctr[0], vmx_iv, KEY_LEN);
    //memcpy(&sec_test.content_key[1], odd_key, 8);
    VMX_TEST_PRINT("IO_SEC_TEST_DSC, vmx_fd:%d\n", vmx_fd);
    ret = ioctl(vmx_fd,IO_SEC_TEST_DSC,&sec_test);
    if (ret != 0){
        err ++;
        printf("IO_SEC_TEST_DSC error.\n");
        goto err_vmx_play_exit;
    }

	// get dsc_status
    aui_vmx_dsc_status dsc_status;
    memset(&dsc_status, 0, sizeof(dsc_status));
    dsc_status.rec_block_count = 0;
    ret = aui_vmx_dsc_status_get(hdl, &dsc_status);
    if (ret){
        VMX_TEST_PRINT("aui_vmx_dsc_status_get fail!\n");
        err ++;
        goto err_vmx_play_exit;
    }
    else{
        VMX_TEST_PRINT("aui_vmx_dsc_status_get OK!, block count:%d\n", dsc_status.rec_block_count);
        VMX_TEST_PRINT("rec block_count = %d\n", dsc_status.rec_block_count);
    }
    
    return 0;
err_vmx_play_exit:
    //aui_vmx_close(hdl);
    
    return ret;
}


#ifdef AUI_LINUX
static int ali_app_dsc_deinit(aui_hdl hdl)
{
    if(aui_dsc_close(hdl)) {
        VMX_TEST_PRINT("\r\n aui_dsc_close error \n");
        return 1;
    }
    return 0;
}
static int ali_app_vmx_deinit(aui_hdl hdl)
{
    int vmx_fd = -1;
    int err = 0;
    int service_index = -1;
    int ret;
    
    if (aui_vmx_fd_get(hdl, &vmx_fd)){
        VMX_TEST_PRINT("aui_vmx_fd_get error!\n");
        return 1;
    }else {
        VMX_TEST_PRINT("aui_vmx_fd_get fd=%d\n!", vmx_fd);            
    }

    if (aui_vmx_service_index_get(hdl, &service_index)){
        VMX_TEST_PRINT("aui_vmx_service_index_get error!\n");
        return 1;
    }else {
        VMX_TEST_PRINT("aui_vmx_service_index_get service_index=%d!\n", service_index);            
    }

	// release key resource
    SEC_TEST_PARAM sec_test;
    memset(&sec_test, 0, sizeof(sec_test));
    sec_test.bServiceIdx = service_index;
    sec_test.cmd = 2;
    ret = ioctl(vmx_fd,IO_SEC_TEST_DSC,&sec_test);
    if (ret != 0){
        err ++;
        VMX_TEST_PRINT("release key resource error!\n");
    }
    else {
        printf("release key resource OK!\n");
    }
    
    if(aui_vmx_close(hdl)) {
        VMX_TEST_PRINT("\r\n aui_dsc_close error \n");
        err ++;
    }
    return err;
}

#endif

static unsigned long vmx_test_play_with_dsc(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
    struct ali_aui_hdls aui_hdls;
    struct aui_attr_tsg tsg_attr;
    struct ali_app_modules_init_para init_para;
    unsigned long pkt_empty,pkt_in_tsg,pkt_cnt;
    int len, ret;
    FILE *file;

    int err = 0;
    unsigned long i;
    
    *sz_output = 0;    
    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    
    VMX_TEST_PRINT("*argc = %d,argv: ",*argc);
    for(i = 0;i < *argc;i++){
        VMX_TEST_PRINT("%s,",argv[i]);
    }
    
    if (7 != *argc) {
        test_vmx_help();
        return 0;
    }

    len = (PKT_SIZE * RD_PKT_NUM) / RD_BLK_SIZE;  //94K 
    pkt_cnt = RD_PKT_NUM;

    /*init init_para variable*/
    ali_aui_init_para_for_test_tsg(argc,argv,&init_para);
	
    //init tsg device
    if(ali_app_tsg_init(&init_para.init_para_tsg, &aui_hdls.tsg_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_tsg_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI TSG opened\n");

    //init tsi device
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI TSI opened\n");


    //init dsc device device
    MEMSET(&crypto_param, 0, sizeof(crypto_param));
    crypto_param.mode = STREAM_DSC;
    if (crypto_param.mode != STREAM) {
        if (ali_app_dsc_init(1 ,// dsc id
                    &aui_hdls.dsc_hdl, &aui_hdls.kl_hdl)){
            VMX_TEST_PRINT("\r\n ali_app_dsc_init failed!");
            return -1;
        }
        /*
        if (ali_app_crypto_init(1 ,// dsc id
                    &aui_hdls.dsc_hdl, &aui_hdls.kl_hdl)){
            VMX_TEST_PRINT("\r\n ali_app_dsc_init failed!");
            return -1;
        }
        */
        VMX_TEST_PRINT("AUI DSC/KL opened %p/%p\n", aui_hdls.dsc_hdl, aui_hdls.kl_hdl);
    }
#ifdef AUI_LINUX
    //init dis device
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_dis_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI DIS opened\n");
#endif


    //init decv device
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_decv_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI DECV opened\n");

   // init deca device
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_audio_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    /*set video and audio  synchronous ways,TS from nim,set to AUI_DECV_SYNC_PTS.
     *TS from tsg,set to AUI_DECV_SYNC_AUDIO*/
    if(aui_decv_sync_mode(aui_hdls.decv_hdl,AUI_DECV_SYNC_AUDIO)){
        VMX_TEST_PRINT("Set AUI_DEVC_SYNC_AUDIO fail\n");
        goto err_tsg;
    }

    /*init dmx devce*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
        VMX_TEST_PRINT("\r\n set dmx failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;
    MEMSET(&path,0,sizeof(aui_dmx_data_path));
    path.p_hdl_de_dev = aui_hdls.dsc_hdl;
    path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        VMX_TEST_PRINT("\r\n aui_dmx_data_path_set failed\n");
        goto err_tsg;
    }
    VMX_TEST_PRINT("dmx data path set %d\n", path.data_path_type);


    /*set TS of dmx device from local media player*/
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,(void *)AUI_AVSYNC_FROM_HDD_MP)) {
        VMX_TEST_PRINT("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
        goto err_tsg;
    }
    
    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices 
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            VMX_TEST_PRINT("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            VMX_TEST_PRINT("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    else{
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL))
        {
            VMX_TEST_PRINT("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }

    
    VMX_TEST_PRINT("tsg file %s\n", argv[0]);
    file = fopen(argv[0], "r");
    if (!file) {
        VMX_TEST_PRINT("tsg file open errno\n");
        goto err_tsg;
    }
    void *buffer = malloc((RD_BLK_SIZE*len));
    if(NULL == buffer)
    {
        VMX_TEST_PRINT("tsg malloc errno\n");
        goto err_tsg;
    }


    while (1) {

        ret = fread(buffer, RD_BLK_SIZE, len, file);
        if (0 == ret) {
            printf("\n\n");
            VMX_TEST_PRINT("tsg file read errno \n");
            fclose(file);
            goto err_tsg;
        }
        printf(".");
    #ifdef AUI_LINUX
        if (ret != len) {
            VMX_TEST_PRINT("file read %d block instead of %d\n",ret, len);
            break;
        }
    #else
        if (ret != len*RD_BLK_SIZE) {
            VMX_TEST_PRINT("file read %d block instead of %d\n",ret, len*RD_BLK_SIZE);
            break;
        }
    #endif


        if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
            VMX_TEST_PRINT("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
        if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
            VMX_TEST_PRINT("aui_dmx_get error\n");

        while(pkt_empty < (pkt_cnt + pkt_in_tsg)) {
            AUI_SLEEP(10);
            if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
                VMX_TEST_PRINT("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                goto err_tsg;
            }
            if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
                VMX_TEST_PRINT("aui_dmx_get error\n");
        }
        /* push data in TSG */
        tsg_attr.ul_addr = (unsigned char *)buffer;
        tsg_attr.ul_pkg_cnt = pkt_cnt;
        tsg_attr.uc_sync_mode = 0;
        ret = aui_tsg_send(aui_hdls.tsg_hdl, &tsg_attr);
        if (ret) {
            VMX_TEST_PRINT("\naui_tsg_send error 0x%x\n", ret);
            fclose(file);
            goto err_tsg;
        }
    }
    fclose(file);

err_tsg:
#ifdef AUI_LINUX
    VMX_TEST_PRINT("\r\n close dsi aui");
    if (aui_hdls.dsc_hdl && ali_app_dsc_deinit(aui_hdls.dsc_hdl))
        VMX_TEST_PRINT("\r\n ali_app_dsc_deinit failed!");
#endif

    ali_app_deinit(&aui_hdls);

    return err;
}


static unsigned long vmx_test_play_with_vmx(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
    struct ali_aui_hdls aui_hdls;
    struct aui_attr_tsg tsg_attr;
    struct ali_app_modules_init_para init_para;
    unsigned long pkt_empty,pkt_in_tsg,pkt_cnt;
    int len, ret;
    FILE *file;

    int err = 0;
    unsigned long i;
    
    *sz_output = 0;    
    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    
    VMX_TEST_PRINT("*argc = %d,argv: ",*argc);
    for(i = 0;i < *argc;i++){
        VMX_TEST_PRINT("%s,",argv[i]);
    }
    
    if (7 != *argc) {
        test_vmx_help();
        return 0;
    }

    len = (PKT_SIZE * RD_PKT_NUM) / RD_BLK_SIZE;  //94K 
    pkt_cnt = RD_PKT_NUM;

    /*init init_para variable*/
    ali_aui_init_para_for_test_tsg(argc,argv,&init_para);
	
    //init tsg device
    if(ali_app_tsg_init(&init_para.init_para_tsg, &aui_hdls.tsg_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_tsg_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI TSG opened\n");

    //init tsi device
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI TSI opened\n");


    //init dsc device device
    MEMSET(&crypto_param, 0, sizeof(crypto_param));
    crypto_param.mode = STREAM_DSC;
    if (crypto_param.mode != STREAM) {
        if (ali_app_vmx_play_init(1 ,// dsc id
                    &aui_hdls.dsc_hdl, &aui_hdls.kl_hdl)){
            VMX_TEST_PRINT("\r\n ali_app_vmx_play_init failed!");
            goto err_tsg;
        }
        VMX_TEST_PRINT("AUI vmx opened %p/%p\n", aui_hdls.dsc_hdl, aui_hdls.kl_hdl);
    }
#ifdef AUI_LINUX
    //init dis device
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_dis_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI DIS opened\n");
#endif


    //init decv device
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_decv_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI DECV opened\n");

   // init deca device
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        VMX_TEST_PRINT("\r\n ali_app_audio_init failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    /*set video and audio  synchronous ways,TS from nim,set to AUI_DECV_SYNC_PTS.
     *TS from tsg,set to AUI_DECV_SYNC_AUDIO*/
    if(aui_decv_sync_mode(aui_hdls.decv_hdl,AUI_DECV_SYNC_AUDIO)){
        VMX_TEST_PRINT("Set AUI_DEVC_SYNC_AUDIO fail\n");
        goto err_tsg;
    }

    /*init dmx devce*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
        VMX_TEST_PRINT("\r\n set dmx failed!");
        goto err_tsg;
    }
    VMX_TEST_PRINT("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;
    MEMSET(&path,0,sizeof(aui_dmx_data_path));
    path.p_hdl_de_dev = aui_hdls.dsc_hdl;
    path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        VMX_TEST_PRINT("\r\n aui_dmx_data_path_set failed\n");
        goto err_tsg;
    }
    VMX_TEST_PRINT("dmx data path set %d\n", path.data_path_type);


    /*set TS of dmx device from local media player*/
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,(void *)AUI_AVSYNC_FROM_HDD_MP)) {
        VMX_TEST_PRINT("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
        goto err_tsg;
    }
    
    /*enabel dmx device,
     *NOTE: video and audio formats must be set to DECV and DECA devices 
     *before enabling DMX AV stream*/
    if(0==s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            VMX_TEST_PRINT("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
    }else if(1 == s_play_mode){
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL))
        {
            VMX_TEST_PRINT("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }
    else{
        if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL))
        {
            VMX_TEST_PRINT("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
            goto err_tsg;
        }
    }

    
    VMX_TEST_PRINT("tsg file %s\n", argv[0]);
    file = fopen(argv[0], "r");
    if (!file) {
        VMX_TEST_PRINT("tsg file open errno\n");
        goto err_tsg;
    }
    void *buffer = malloc((RD_BLK_SIZE*len));
    if(NULL == buffer)
    {
        VMX_TEST_PRINT("tsg malloc errno\n");
        goto err_tsg;
    }


    while (1) {

        ret = fread(buffer, RD_BLK_SIZE, len, file);
        if (0 == ret) {
            printf("\n\n");
            VMX_TEST_PRINT("tsg file read errno \n");
            fclose(file);
            goto err_tsg;
        }
         printf(".");
    #ifdef AUI_LINUX
        if (ret != len) {
            VMX_TEST_PRINT("file read %d block instead of %d\n",ret, len);
            break;
        }
    #else
        if (ret != len*RD_BLK_SIZE) {
            VMX_TEST_PRINT("file read %d block instead of %d\n",ret, len*RD_BLK_SIZE);
            break;
        }
    #endif


        if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
            VMX_TEST_PRINT("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
        if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
            VMX_TEST_PRINT("aui_dmx_get error\n");

        while(pkt_empty < (pkt_cnt + pkt_in_tsg)) {
            AUI_SLEEP(10);
            if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
                VMX_TEST_PRINT("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                goto err_tsg;
            }
            if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
                VMX_TEST_PRINT("aui_dmx_get error\n");
        }
        /* push data in TSG */
        tsg_attr.ul_addr = (unsigned char *)buffer;
        tsg_attr.ul_pkg_cnt = pkt_cnt;
        tsg_attr.uc_sync_mode = 0;
        ret = aui_tsg_send(aui_hdls.tsg_hdl, &tsg_attr);
        if (ret) {
            VMX_TEST_PRINT("\naui_tsg_send error 0x%x\n", ret);
            fclose(file);
            goto err_tsg;
        }
    }
    fclose(file);

err_tsg:
#ifdef AUI_LINUX
    VMX_TEST_PRINT("\r\n close dsi aui\n");
    if (aui_hdls.dsc_hdl && ali_app_vmx_deinit(aui_hdls.dsc_hdl))
        VMX_TEST_PRINT("\r\n ali_app_dsc_deinit failed!");
#endif

    ali_app_deinit(&aui_hdls);

    return err;
}

void vmx_test_reg()
{
	aui_tu_reg_group("vmx", "vmx as plus simple test");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, vmx_test_open, "open and close vmx device!");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, vmx_test_ca_algo, "test aui_vmx_dsc_algo_set()!");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, vmx_test_set_pid, "test aui_vmx_pid_set()!");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, vmx_test_get_status, "test aui_vmx_dsc_status_get()!");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, vmx_test_play_with_dsc, "play decrypted stream with dsc!");
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, vmx_test_play_with_vmx, "play decrypted stream with vmx AS!");
    
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_vmx_help, "help informtion!");
}

