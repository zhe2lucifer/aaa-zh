/****************************INCLUDE HEAD FILE************************************/
#include "aui_hdmi_test.h"
#include "aui_kl.h"
#include "aui_help_print.h"
#include <aui_common.h>
#include <stdio.h>
#include<string.h>

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/
//static void *hdmi_handle = NULL;
aui_hdl g_p_hdl_hdmi;

/****************************LOCAL FUNC DECLEAR***********************************/

/* customer should fill the  key in those array */
unsigned char test_plain_key[286]={0};
unsigned char test_plain_key_invalid[286] = {0};
unsigned char test_plain_hdcp22_key[402]={0};
unsigned char test_plain_hdcp22_key_invalid[402] = {0};
unsigned char test_plain_hdcp22_ce_key[416]={0};
unsigned char test_plain_hdcp22_ce_key_invalid[416] = {0};

unsigned char test_otp_key[288]= {0};
unsigned char test_otp_key_invalid[288] = {0};

#ifdef AUI_LINUX
#define HDCP_CE_KEY_FILE_PATH	"/mnt/usb/sda1/hdcp_ce_key.bin"
#define HDCP_SW_KEY_FILE_PATH	"/mnt/usb/sda1/hdcp_sw_key.bin"
#define HDCP22_CE_KEY_FILE_PATH	"/mnt/usb/sda1/hdcp22_ce_key.bin"  //only support S3922.
#define HDCP22_SW_KEY_FILE_PATH	"/mnt/usb/sda1/hdcp22_sw_key.bin"  //only support S3922.
#else
#define HDCP_CE_KEY_FILE_PATH	"/mnt/uda1/hdcp_ce_key.bin"
#define HDCP_SW_KEY_FILE_PATH	"/mnt/uda1/hdcp_sw_key.bin"
#define HDCP22_CE_KEY_FILE_PATH	"/mnt/uda1/hdcp22_ce_key.bin"  //only support S3922
#define HDCP22_SW_KEY_FILE_PATH	"/mnt/uda1/hdcp22_sw_key.bin" //only support Linux
#endif

/****************************EXTERN VAR*******************************************/
static aui_hdl aui_hdmi_handle = NULL;

static unsigned char *plugout_usrdata;
static unsigned char *plugin_usrdata;
static unsigned char *cec_usrdata;

/****************************EXTERN FUNC *****************************************/
//extern unsigned long get_num_by_string(char *str);
//extern aui_hdl g_p_hdl_hdmi;

void  HDCP_DUMP(unsigned char *data, unsigned int len, unsigned char *name);
/****************************TEST MODULE IMPLEMENT********************************/
#if 0
////////////////////////////HDCP-2.2 key start///////////////////////////////////////////
static unsigned char test_plain22_key[402] = {
//Header
	0x0,

//Public key Modulus N
	0xb0, 0xe9, 0xaa, 0x45, 0xf1, 0x29, 0xba, 0x0a, 0x1c, 0xbe, 0x17, 0x57, 0x28, 0xeb, 0x2b, 0x4e,
    0x8f, 0xd0, 0xc0, 0x6a, 0xad, 0x79, 0x98, 0x0f, 0x8d, 0x43, 0x8d, 0x47, 0x04, 0xb8, 0x2b, 0xf4,
    0x15, 0x21, 0x56, 0x19, 0x01, 0x40, 0x01, 0x3b, 0xd0, 0x91, 0x90, 0x62, 0x9e, 0x89, 0xc2, 0x27,
    0x8e, 0xcf, 0xb6, 0xdb, 0xce, 0x3f, 0x72, 0x10, 0x50, 0x93, 0x8c, 0x23, 0x29, 0x83, 0x7b, 0x80,
    0x64, 0xa7, 0x59, 0xe8, 0x61, 0x67, 0x4c, 0xbc, 0xd8, 0x58, 0xb8, 0xf1, 0xd4, 0xf8, 0x2c, 0x37,
    0x98, 0x16, 0x26, 0x0e, 0x4e, 0xf9, 0x4e, 0xee, 0x24, 0xde, 0xcc, 0xd1, 0x4b, 0x4b, 0xc5, 0x06,
    0x7a, 0xfb, 0x49, 0x65, 0xe6, 0xc0, 0x00, 0x83, 0x48, 0x1e, 0x8e, 0x42, 0x2a, 0x53, 0xa0, 0xf5,
    0x37, 0x29, 0x2b, 0x5a, 0xf9, 0x73, 0xc5, 0x9a, 0xa1, 0xb5, 0xb5, 0x74, 0x7c, 0x06, 0xdc, 0x7b,
    0x7c, 0xdc, 0x6c, 0x6e, 0x82, 0x6b, 0x49, 0x88, 0xd4, 0x1b, 0x25, 0xe0, 0xee, 0xd1, 0x79, 0xbd,
    0x39, 0x85, 0xfa, 0x4f, 0x25, 0xec, 0x70, 0x19, 0x23, 0xc1, 0xb9, 0xa6, 0xd9, 0x7e, 0x3e, 0xda,
    0x48, 0xa9, 0x58, 0xe3, 0x18, 0x14, 0x1e, 0x9f, 0x30, 0x7f, 0x4c, 0xa8, 0xae, 0x53, 0x22, 0x66,
    0x2b, 0xbe, 0x24, 0xcb, 0x47, 0x66, 0xfc, 0x83, 0xcf, 0x5c, 0x2d, 0x1e, 0x3a, 0xab, 0xab, 0x06,
    0xbe, 0x05, 0xaa, 0x1a, 0x9b, 0x2d, 0xb7, 0xa6, 0x54, 0xf3, 0x63, 0x2b, 0x97, 0xbf, 0x93, 0xbe,
    0xc1, 0xaf, 0x21, 0x39, 0x49, 0x0c, 0xe9, 0x31, 0x90, 0xcc, 0xc2, 0xbb, 0x3c, 0x02, 0xc4, 0xe2,
    0xbd, 0xbd, 0x2f, 0x84, 0x63, 0x9b, 0xd2, 0xdd, 0x78, 0x3e, 0x90, 0xc6, 0xc5, 0xac, 0x16, 0x77,
    0x2e, 0x69, 0x6c, 0x77, 0xfd, 0xed, 0x8a, 0x4d, 0x6a, 0x8c, 0xa3, 0xa9, 0x25, 0x6c, 0x21, 0xfd,
    0xb2, 0x94, 0x0c, 0x84, 0xaa, 0x07, 0x29, 0x26, 0x46, 0xf7, 0x9b, 0x3a, 0x19, 0x87, 0xe0, 0x9f,
    0xeb, 0x30, 0xa8, 0xf5, 0x64, 0xeb, 0x07, 0xf1, 0xe9, 0xdb, 0xf9, 0xaf, 0x2c, 0x8b, 0x69, 0x7e,
    0x2e, 0x67, 0x39, 0x3f, 0xf3, 0xa6, 0xe5, 0xcd, 0xda, 0x24, 0x9b, 0xa2, 0x78, 0x72, 0xf0, 0xa2,
    0x27, 0xc3, 0xe0, 0x25, 0xb4, 0xa1, 0x04, 0x6a, 0x59, 0x80, 0x27, 0xb5, 0xda, 0xb4, 0xb4, 0x53,
    0x97, 0x3b, 0x28, 0x99, 0xac, 0xf4, 0x96, 0x27, 0x0f, 0x7f, 0x30, 0x0c, 0x4a, 0xaf, 0xcb, 0x9e,
    0xd8, 0x71, 0x28, 0x24, 0x3e, 0xbc, 0x35, 0x15, 0xbe, 0x13, 0xeb, 0xaf, 0x43, 0x01, 0xbd, 0x61,
    0x24, 0x54, 0x34, 0x9f, 0x73, 0x3e, 0xb5, 0x10, 0x9f, 0xc9, 0xfc, 0x80, 0xe8, 0x4d, 0xe3, 0x32,
    0x96, 0x8f, 0x88, 0x10, 0x23, 0x25, 0xf3, 0xd3, 0x3e, 0x6e, 0x6d, 0xbb, 0xdc, 0x29, 0x66, 0xeb,

//Public key Exponent E
	0x03,

// Lisenced constant
	0xb5, 0xd8, 0xe9, 0xab, 0x5f, 0x8a, 0xfe, 0xca, 0x38, 0x55, 0xb1, 0xa5, 0x1e, 0xc9, 0xbc, 0x0f
};


static unsigned char test_plain22_key_invalid[402] = {
//Header
	0x0,

//Public key Modulus N
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xba, 0x0a, 0x1c, 0xbe, 0x17, 0x57, 0x28, 0xeb, 0x2b, 0x4e,
    0x8f, 0xd0, 0xc0, 0x6a, 0xad, 0x79, 0x98, 0x0f, 0x8d, 0x43, 0x8d, 0x47, 0x04, 0xb8, 0x2b, 0xf4,
    0x15, 0x21, 0x56, 0x19, 0x01, 0x40, 0x01, 0x3b, 0xd0, 0x91, 0x90, 0x62, 0x9e, 0x89, 0xc2, 0x27,
    0x8e, 0xcf, 0xb6, 0xdb, 0xce, 0x3f, 0x72, 0x10, 0x50, 0x93, 0x8c, 0x23, 0x29, 0x83, 0x7b, 0x80,
    0x64, 0xa7, 0x59, 0xe8, 0x61, 0x67, 0x4c, 0xbc, 0xd8, 0x58, 0xb8, 0xf1, 0xd4, 0xf8, 0x2c, 0x37,
    0x98, 0x16, 0x26, 0x0e, 0x4e, 0xf9, 0x4e, 0xee, 0x24, 0xde, 0xcc, 0xd1, 0x4b, 0x4b, 0xc5, 0x06,
    0x7a, 0xfb, 0x49, 0x65, 0xe6, 0xc0, 0x00, 0x83, 0x48, 0x1e, 0x8e, 0x42, 0x2a, 0x53, 0xa0, 0xf5,
    0x37, 0x29, 0x2b, 0x5a, 0xf9, 0x73, 0xc5, 0x9a, 0xa1, 0xb5, 0xb5, 0x74, 0x7c, 0x06, 0xdc, 0x7b,
    0x7c, 0xdc, 0x6c, 0x6e, 0x82, 0x6b, 0x49, 0x88, 0xd4, 0x1b, 0x25, 0xe0, 0xee, 0xd1, 0x79, 0xbd,
    0x39, 0x85, 0xfa, 0x4f, 0x25, 0xec, 0x70, 0x19, 0x23, 0xc1, 0xb9, 0xa6, 0xd9, 0x7e, 0x3e, 0xda,
    0x48, 0xa9, 0x58, 0xe3, 0x18, 0x14, 0x1e, 0x9f, 0x30, 0x7f, 0x4c, 0xa8, 0xae, 0x53, 0x22, 0x66,
    0x2b, 0xbe, 0x24, 0xcb, 0x47, 0x66, 0xfc, 0x83, 0xcf, 0x5c, 0x2d, 0x1e, 0x3a, 0xab, 0xab, 0x06,
    0xbe, 0x05, 0xaa, 0x1a, 0x9b, 0x2d, 0xb7, 0xa6, 0x54, 0xf3, 0x63, 0x2b, 0x97, 0xbf, 0x93, 0xbe,
    0xc1, 0xaf, 0x21, 0x39, 0x49, 0x0c, 0xe9, 0x31, 0x90, 0xcc, 0xc2, 0xbb, 0x3c, 0x02, 0xc4, 0xe2,
    0xbd, 0xbd, 0x2f, 0x84, 0x63, 0x9b, 0xd2, 0xdd, 0x78, 0x3e, 0x90, 0xc6, 0xc5, 0xac, 0x16, 0x77,
    0x2e, 0x69, 0x6c, 0x77, 0xfd, 0xed, 0x8a, 0x4d, 0x6a, 0x8c, 0xa3, 0xa9, 0x25, 0x6c, 0x21, 0xfd,
    0xb2, 0x94, 0x0c, 0x84, 0xaa, 0x07, 0x29, 0x26, 0x46, 0xf7, 0x9b, 0x3a, 0x19, 0x87, 0xe0, 0x9f,
    0xeb, 0x30, 0xa8, 0xf5, 0x64, 0xeb, 0x07, 0xf1, 0xe9, 0xdb, 0xf9, 0xaf, 0x2c, 0x8b, 0x69, 0x7e,
    0x2e, 0x67, 0x39, 0x3f, 0xf3, 0xa6, 0xe5, 0xcd, 0xda, 0x24, 0x9b, 0xa2, 0x78, 0x72, 0xf0, 0xa2,
    0x27, 0xc3, 0xe0, 0x25, 0xb4, 0xa1, 0x04, 0x6a, 0x59, 0x80, 0x27, 0xb5, 0xda, 0xb4, 0xb4, 0x53,
    0x97, 0x3b, 0x28, 0x99, 0xac, 0xf4, 0x96, 0x27, 0x0f, 0x7f, 0x30, 0x0c, 0x4a, 0xaf, 0xcb, 0x9e,
    0xd8, 0x71, 0x28, 0x24, 0x3e, 0xbc, 0x35, 0x15, 0xbe, 0x13, 0xeb, 0xaf, 0x43, 0x01, 0xbd, 0x61,
    0x24, 0x54, 0x34, 0x9f, 0x73, 0x3e, 0xb5, 0x10, 0x9f, 0xc9, 0xfc, 0x80, 0xe8, 0x4d, 0xe3, 0x32,
    0x96, 0x8f, 0x88, 0x10, 0x23, 0x25, 0xf3, 0xd3, 0x3e, 0x6e, 0x6d, 0xbb, 0xdc, 0x29, 0x66, 0xeb,

//Public key Exponent E
	0x03,

// Lisenced constant
	0xb5, 0xd8, 0xe9, 0xab, 0x5f, 0x8a, 0xfe, 0xca, 0x38, 0x55, 0xb1, 0xa5, 0x1e, 0xc9, 0xbc, 0x0f
};
#endif
//////////////////////////////HDCP-2.2 key end////////////////////////////////////////

typedef enum
{
	CEC_LA_TV			= 0x0,
	CEC_LA_RECORD_1			= 0x1,
	CEC_LA_RECORD_2			= 0x2,
	CEC_LA_TUNER_1			= 0x3,
	CEC_LA_PLAYBACK_1		= 0x4,
	CEC_LA_AUDIO_SYSTEM		= 0x5,
	CEC_LA_TUNER_2			= 0x6,
	CEC_LA_TUNER_3			= 0x7,
	CEC_LA_PLAYBACK_2		= 0x8,
	CEC_LA_RECORD_3			= 0x9,
	CEC_LA_TUNER_4			= 0xA,
	CEC_LA_PLAYBACK_3		= 0xB,
	CEC_LA_RESERVED_1		= 0xC,
	CEC_LA_RESERVED_2		= 0xD,
	CEC_LA_FREE_USE			= 0xE,
	CEC_LA_BROADCAST		= 0xF,
	CEC_LA_NUMBER           = 0x10
}E_CEC_LOGIC_ADDR;
unsigned long test_hdmi_cec_transmit_test(unsigned long *argc, char **argv, char *sz_out_put)
{
	aui_hdl hdmi_hdl;
	aui_attr_hdmi attr_hdmi;
	unsigned char cec_status;
	int cmd = 0x8F;
	unsigned int msg ;
	unsigned int logical_addr = 0x03;
	unsigned int poll_addr = 0x3F;
	int exist;
	int ret;
	int i;
	memset(&attr_hdmi, 0, sizeof(aui_attr_hdmi));
	if(aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_hdl)) {
		if (aui_hdmi_open(&attr_hdmi, &hdmi_hdl)) {
			AUI_PRINTF("\n aui_hdmi_open fail\n");
			return -1;
		}
	}

	AUI_TEST_CHECK_RTN(aui_hdmi_start(hdmi_hdl));
	AUI_TEST_CHECK_RTN(aui_hdmi_on(hdmi_hdl));
	AUI_TEST_CHECK_RTN(aui_hdmi_cec_set_onoff(hdmi_hdl, 1));//enable cec
	AUI_TEST_CHECK_RTN(aui_hdmi_cec_get_onoff(hdmi_hdl,&cec_status));//check cec status
	if (!cec_status){
		AUI_PRINTF("set cec on fail\n");
		return AUI_RTN_FAIL;
	}
	AUI_TEST_CHECK_RTN(aui_hdmi_cec_set_logical_address(hdmi_hdl, logical_addr));
	for (i = 0; i < CEC_LA_RESERVED_1; i++) {
		poll_addr = (logical_addr << 4) | i;
		ret = aui_hdmi_cec_transmit(hdmi_hdl,(unsigned char*)&poll_addr, 1);//cmd is 0x30--0x3A,length is 1
		if (!ret){
			exist = poll_addr;
			AUI_PRINTF("addr:0x%x,exist\n",exist);
			break;
		}else{
			AUI_PRINTF("addr:0x%x,is not exist\n",poll_addr);
		}
	}
	if (i == CEC_LA_RESERVED_1){
		AUI_PRINTF("There is not cec tv exist\n");
		return AUI_RTN_FAIL;
	}
	msg = (exist << 8) | cmd;
	AUI_PRINTF("msg:0x%x\n",msg);
	AUI_TEST_CHECK_RTN(aui_hdmi_cec_transmit(hdmi_hdl,(unsigned char*)&msg, 2));//cmd is 0x3X8F,length is 2
	return AUI_RTN_SUCCESS;
}

void aui_load_tu_hdmi()
{
    aui_tu_reg_group("hdmi", "hdmi test help info");
    {
        aui_tu_reg_item(2, "1", AUI_CMD_TYPE_UNIT, test_hdmi_start, "finish hdmi init, now you can test hdmi");
        aui_tu_reg_item(2, "2", AUI_CMD_TYPE_SYS, test_hdmi_on, "hdmi is on?");
        aui_tu_reg_item(2, "3", AUI_CMD_TYPE_SYS, test_hdmi_off, "hdmi is off?");
        aui_tu_reg_item(2, "4", AUI_CMD_TYPE_SYS, test_hdmi_audio_on, "hdmi audio is on?");
        aui_tu_reg_item(2, "5", AUI_CMD_TYPE_SYS, test_hdmi_audio_off, "hdmi audio is off?");
        aui_tu_reg_item(2, "6", AUI_CMD_TYPE_SYS, test_hdmi_para_get, "hdmi get para ok?");
        aui_tu_reg_item(2, "7", AUI_CMD_TYPE_SYS, test_hdmi_cec_on, "hdmi cec on ok?");
        aui_tu_reg_item(2, "8", AUI_CMD_TYPE_SYS, test_hdmi_cec_off, "hdmi cec off ok?");
        aui_tu_reg_item(2, "9", AUI_CMD_TYPE_SYS, test_hdmi_cec_allocate_logic_address, "hdmi allocate address ok?");
        aui_tu_reg_item(2, "10", AUI_CMD_TYPE_SYS, test_hdmi_cec_receive, "register cec receive callback.");
        aui_tu_reg_item(2, "11", AUI_CMD_TYPE_SYS, test_hdmi_reg_callback_plug_in_sample, "plug in receive callback.");
        aui_tu_reg_item(2, "12", AUI_CMD_TYPE_SYS, test_hdmi_reg_callback_plug_out, "plug out receive callback.");
        aui_tu_reg_item(2, "13", AUI_CMD_TYPE_SYS, test_hdmi_del_callback_plug_in, "plug in delete callback.");
        aui_tu_reg_item(2, "14", AUI_CMD_TYPE_SYS, test_hdmi_del_callback_plug_out, "plug out delete callback.");
        aui_tu_reg_item(2, "15", AUI_CMD_TYPE_SYS, test_hdmi_hdcp_on, "test HDCP mem/ce key");
        aui_tu_reg_item(2, "16", AUI_CMD_TYPE_SYS, test_hdmi_hdcp_off, "turn off HDCP test");
        aui_tu_reg_item(2, "17", AUI_CMD_TYPE_SYS, test_hdmi_reg_callback_hdcp_error, "hdcp error receive callback.");
        aui_tu_reg_item(2, "18", AUI_CMD_TYPE_SYS, test_hdmi_del_callback_hdcp_fail, "delete hdcp fail callback.");
        aui_tu_reg_item(2, "19", AUI_CMD_TYPE_SYS, test_hdmi_av_blank_mute, "test hdmi avblank/avmute");
		aui_tu_reg_item(2, "20", AUI_CMD_TYPE_SYS, test_hdmi_set_color_space, "test_hdmi_set_color_space");
		aui_tu_reg_item(2, "21", AUI_CMD_TYPE_SYS, test_hdmi_set_deep_color, "test_hdmi_set_deep_color");
		aui_tu_reg_item(2, "22", AUI_CMD_TYPE_SYS, test_hdmi_cec_transmit_test, "test_hdmi_cec_transmit_test");
		aui_tu_reg_item(2, "h", AUI_CMD_TYPE_SYS, test_hdmi_help, "hdmi test help");
    }
}


static int aui_testapp_hdmi_init()
{
    aui_hdl hdmi_hdl = 0;
    aui_attr_hdmi attr_hdmi;
    MEMSET(&attr_hdmi, 0, sizeof(aui_attr_hdmi));



    attr_hdmi.uc_dev_idx = 0;
    if(aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_hdl)) {
        if (aui_hdmi_open(&attr_hdmi, &hdmi_hdl)) {
            AUI_PRINTF("\n aui_hdmi_open HD fail\n");
            return -1;
        }
    }

    if (0 != aui_hdmi_start(hdmi_hdl)) {
        //printf("\n aui_hdmi_start fail\n", __FUNCTION__, __LINE__);
        return -1;
    } else {
        //printf("\n success aui_hdmi_start\n", __FUNCTION__, __LINE__);
    }

    aui_hdmi_handle = hdmi_hdl;
    return 0;
}

unsigned long test_hdmi_start(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) argc;
	(void) argv;
	(void) sz_out_put;
    AUI_TEST_CHECK_RTN(aui_testapp_hdmi_init( ));
#ifndef _AUI_OTA_
    // OTA compile no FS, can not open file
/*
*this code is only to convenient to debug. so return value should not be AUI_RTN_FAIL.
*/
	FILE* hdcp_key_fp = NULL;
	int hdcp_key_len=0;

	// Reset all keys
	memset(test_otp_key, 0, 288);
	memset(test_otp_key_invalid, 0, 288);
	memset(test_plain_key, 0, 286);
	memset(test_plain_key_invalid, 0, 286);

	memset(test_plain_hdcp22_key, 0, 402);
	memset(test_plain_hdcp22_key_invalid, 0, 402);
	memset(test_plain_hdcp22_ce_key, 0, 416);
	memset(test_plain_hdcp22_ce_key_invalid, 0, 416);

	hdcp_key_fp = fopen(HDCP_CE_KEY_FILE_PATH, "rb");
	if(!hdcp_key_fp){
		AUI_PRINTF("open %s failed!\n", HDCP_CE_KEY_FILE_PATH);
	} else {
		AUI_PRINTF("read HDCP 1.4 encrypted key from hdcp_ce_key.bin\n");
		hdcp_key_len = fread(test_otp_key, 1, 288, hdcp_key_fp);
		HDCP_DUMP(test_otp_key, 288, "HDCP 1.4 encrypted key");
		if(hdcp_key_len!=288)
			AUI_PRINTF("HDCP 1.4 CE key read err!\n");
		fclose(hdcp_key_fp);

		// init the invalid key,
		// byte 1 need to be 0 or 1,
		// byte2-6 need to be corrrect vector,and have 20 ONE and 20 ZERO,
		// otherwise, the driver do not do HDCP authen.
		memcpy(test_otp_key_invalid, test_otp_key, 6);
    }
	hdcp_key_fp = fopen(HDCP_SW_KEY_FILE_PATH, "rb");
	if(hdcp_key_fp) {
		AUI_PRINTF("read HDCP 1.4 plain key from hdcp_sw_key.bin\n");
		hdcp_key_len = fread(test_plain_key, 1, 286, hdcp_key_fp);
		HDCP_DUMP(test_plain_key, 286, "HDCP 1.4 plain key");
		if(hdcp_key_len!=286)
		AUI_PRINTF("HDCP 1.4 plain key read err!\n");
		fclose(hdcp_key_fp);
		// init the invalid key,
		// byte 1 need to be 0 or 1,
		// byte2-6 need to be corrrect vector,and have 20 ONE and 20 ZERO,
		// otherwise, the driver do not do HDCP authen.
		memcpy(test_plain_key_invalid, test_plain_key, 6);
	} else {
		AUI_PRINTF("open %s failed!\n", HDCP_SW_KEY_FILE_PATH);
	}

	hdcp_key_fp = fopen(HDCP22_SW_KEY_FILE_PATH, "rb");
	if(hdcp_key_fp) {
		AUI_PRINTF("\n read HDCP 22 plain key from hdcp22_sw_key.bin\n");
		hdcp_key_len = fread(test_plain_hdcp22_key, 1, 402, hdcp_key_fp);
		HDCP_DUMP(test_plain_hdcp22_key, 402, "\n HDCP 2.2 SW key");
		if(hdcp_key_len!=402)
			AUI_PRINTF("HDCP 22 plain key read err!\n");
		fclose(hdcp_key_fp);
		memcpy(test_plain_hdcp22_key_invalid+1, test_plain_hdcp22_key+1, 6);
	} else {
		AUI_PRINTF("open %s failed!\n", HDCP22_SW_KEY_FILE_PATH);
		//return AUI_RTN_SUCCESS;
	}

	hdcp_key_fp = fopen(HDCP22_CE_KEY_FILE_PATH, "rb");
	if(hdcp_key_fp) {
		AUI_PRINTF("\n read HDCP 22 encrypted key from hdcp22_ce_key.bin\n");
		hdcp_key_len = fread(test_plain_hdcp22_ce_key, 1, 416, hdcp_key_fp);
		HDCP_DUMP(test_plain_hdcp22_ce_key, 416, "\n HDCP 22 encrypted key");
		if(hdcp_key_len!=416)
			AUI_PRINTF("HDCP 22 encrypted key read err!\n");
		fclose(hdcp_key_fp);
		memcpy(test_plain_hdcp22_ce_key_invalid+1, test_plain_hdcp22_ce_key+1, 6);
	} else {
		AUI_PRINTF("open %s failed!\n", HDCP22_CE_KEY_FILE_PATH);
		//return AUI_RTN_SUCCESS;
	}

    AUI_PRINTF("finish start hdmi, you can test aui hdmi \n");
#endif

    return AUI_RTN_SUCCESS;
}


unsigned long test_hdmi_on(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_hdmi_on(aui_hdmi_handle));
    /*
    if(!aui_test_user_confirm("do you have success start play the video?"))
    {
       return AUI_RTN_FAIL;
    }
    */
    return AUI_RTN_SUCCESS;
}


unsigned long test_hdmi_off(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_hdmi_off(aui_hdmi_handle));
    /*
        if(!aui_test_user_confirm("do you have success stop the video?"))
        {
            return AUI_RTN_FAIL;
        }
    */
    return AUI_RTN_SUCCESS;

}

unsigned long test_hdmi_audio_on(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_hdmi_audio_on(aui_hdmi_handle));
    /*
        if(!aui_test_user_confirm("do you have success start play the video?"))
        {
            return AUI_RTN_FAIL;
        }
    */
    return AUI_RTN_SUCCESS;
}


unsigned long test_hdmi_audio_off(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_TEST_CHECK_RTN(aui_hdmi_audio_off(aui_hdmi_handle));
    /*
    if(!aui_test_user_confirm("do you have success stop the video?"))
    {
       return AUI_RTN_FAIL;
    }
    */
    return AUI_RTN_SUCCESS;

}

unsigned long test_hdmi_para_get(unsigned long *argc, char **argv, char *sz_out_put)
{
    //get all raw edid data
    unsigned int edid_len = 0;
    unsigned char *edid_buf = 0;
	unsigned int i=0;

    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_LEN_GET, &edid_len, NULL));

    AUI_PRINTF("\n  .............edid_len is : %d\n", edid_len);

    //edid_len = 512;
    //edid_buf =MALLOC(256*2);
    edid_buf = MALLOC(edid_len);
	if(edid_buf==NULL)
	{
		AUI_PRINTF("test_hdmi_para_get malloc error\n");
		return AUI_RTN_FAIL;
	}
	MEMSET(edid_buf, 0 ,edid_len);
    AUI_TEST_CHECK_RTN(aui_hdmi_ediddata_read(aui_hdmi_handle, (unsigned long *)edid_buf, &edid_len , AUI_HDMI_RAW_EDID_ALL));
	AUI_PRINTF("all edid data:\n");
	for(i=0; i<edid_len; i++)
		AUI_PRINTF("%x-",*(edid_buf+i));
	AUI_PRINTF("\n");
	FREE(edid_buf);
	edid_buf = NULL;

    //get one block raw edid data
    unsigned int blk_idx = 0;// which block will be get
    unsigned int edid_len2 = 0;
    unsigned char *edid_buf2 = 0;
    edid_buf2 = MALLOC(128);
	if(edid_buf2==NULL)
	{
		AUI_PRINTF("test_hdmi_para_get malloc error\n");
		return AUI_RTN_FAIL;
	}
	MEMSET(edid_buf2, 0 ,128);
    AUI_PRINTF("edid block %d data:\n",blk_idx);
    AUI_TEST_CHECK_RTN(aui_hdmi_ediddata_read(aui_hdmi_handle, (unsigned long *)edid_buf2, &edid_len2 , blk_idx));
	for(i=0; i<edid_len2; i++)
		AUI_PRINTF("%x-",*(edid_buf2+i));
	AUI_PRINTF("\n");
	FREE(edid_buf2);
	edid_buf2 = NULL;

    //get preferred video format
    aui_hdmi_edid_video_format_e prefered_video_fmt;
    MEMSET(&prefered_video_fmt, 0, sizeof(aui_hdmi_edid_video_format_e));
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_PREFER_VIDEO_FMT_GET, &prefered_video_fmt, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_PREFER_VIDEO_FMT_GET (%d,%d,%d)\n",prefered_video_fmt.video_format,prefered_video_fmt.field_rate,prefered_video_fmt.aspect_ratio);

    //get hdmi link status
    aui_hdmi_link_status link_status = AUI_HDMI_STATUS_UNLINK;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_CONNECT_INFO_GET, &link_status, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_CONNECT_INFO_GET 0x%x \n",link_status);

    //get hdmi or dvi
    aui_hdmi_type_e hdmi_dvi = AUI_HDMI_TYPE_DVI;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_TYPE_GET, &hdmi_dvi, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_TYPE_GET %d\n",hdmi_dvi);


    //get current hdcp verification result
    aui_hdmi_link_status hdcp_link_status = AUI_HDMI_STATUS_UNLINK;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_HDCP_STATUS_GET, &hdcp_link_status, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_HDCP_STATUS_GET 0x%x\n",hdcp_link_status);

	//get hdcp status
	aui_hdmi_link_status hdcp_status = 0;
	AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_HDCP_ENABLE_GET, &hdcp_status, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_HDCP_ENABLE_GET 0x%x\n",hdcp_status);

    //get edid logical address
    unsigned char hdmi_logical_addr=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_LOGICAL_ADDR_GET, &hdmi_logical_addr, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_LOGICAL_ADDR_GET 0x%x\n",hdmi_logical_addr);

    //get edid phy address
    unsigned short hdmi_phy_addr=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_PHYSICAL_ADDR_GET, &hdmi_phy_addr, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_PHYSICAL_ADDR_GET 0x%x\n",hdmi_phy_addr);

    //get  edid 3D format.
    aui_hdmi_3d_descriptor_t edid_3d_fmt;
    MEMSET(&edid_3d_fmt, 0, sizeof(aui_hdmi_3d_descriptor_t));
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_3D_DESC_GET, &edid_3d_fmt, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_3D_DESC_GET (%d,%d,%d)\n",edid_3d_fmt.hdmi_3d_multi_present,edid_3d_fmt.hdmi_vic_len,edid_3d_fmt.hdmi_3d_len);

    //get edid video format number
    unsigned short hdmi_edid_video_num=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_VIC_NUM_GET, &hdmi_edid_video_num, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_VIC_NUM_GET %d\n",hdmi_edid_video_num);

    //get edid audio format number
    unsigned short hdmi_edid_audio_num=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_AUD_NUM_GET, &hdmi_edid_audio_num, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_AUD_NUM_GET %d\n",hdmi_edid_audio_num);

    //get all edid video format, should use for(i=0;i<hdmi_edid_video_num;i++),
    //just get format 2 for example.
    aui_short_video_desc video_desc;
    //int num = 2;
    for(i=0;i<hdmi_edid_video_num;i++) {
        MEMSET(&video_desc, 0, sizeof(aui_short_video_desc));
        AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_VIDEO_FMT_ID_GET, &video_desc, &i));
        AUI_PRINTF("\n	.............AUI_HDMI_VIDEO_FMT_ID_GET %d,%d\n",video_desc.native_indicator,video_desc.video_id_code);
    }
    //get all edid audio format, should use for(i=0;i<hdmi_edid_audio_num;i++),
    //just get format 1 for example.
    aui_short_audio_desc audio_desc;
    //int aud_num = 0;// 1;
    for(i=0;i<hdmi_edid_audio_num;i++) {
        MEMSET(&audio_desc, 0, sizeof(aui_short_audio_desc));
        AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_AUDIO_FMT_ID_GET, &audio_desc, &i));
        AUI_PRINTF("\n	.............AUI_HDMI_AUDIO_FMT_ID_GET (%d,%d,%d,%d)\n",audio_desc.audio_format_code,audio_desc.max_num_audio_channels,audio_desc.audio_sampling_rate,audio_desc.max_audio_bit_rate);
    }

    //get  edid manufacturer name.
    unsigned char m_name[14]={0};
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_MANNAME_GET, &m_name, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_MANNAME_GET %s\n",m_name);

    //get   edid product id.
    unsigned short productid=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_PROID_GET, &productid, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_PROID_GET 0x%x\n",productid);

    //get   edid serial number.
    unsigned int s_number=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_SENUM_GET, &s_number, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_SENUM_GET 0x%x\n",s_number);

    //get   edid moniter name.
    unsigned char monitorname[14]={0};
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_MONNAME_GET, monitorname, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_MONNAME_GET %s\n",monitorname);

    //get   edid manufacturer week.
    unsigned char manufweek=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_MANWEEK_GET, &manufweek, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_MANWEEK_GET %d\n",manufweek);

    //get   edid manufacturer year.
    unsigned short manufyear=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_MANYEAR_GET, &manufyear, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_MANYEAR_GET %d\n",manufyear);


    //get   edid manufacturer week.
    unsigned short edidversion2=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_VERSION_GET, &edidversion2, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_VERSION_GET %d\n",edidversion2);

    //get   edid manufacturer week.
    unsigned short edidrevision2=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_REVISION_GET, &edidrevision2, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_REVISION_GET %d\n",edidrevision2);

    //get   edid first video id.
    unsigned char first_vid=0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_EDID_FIRSTVID_GET, &first_vid, 0));
    AUI_PRINTF("\n	.............AUI_HDMI_EDID_FIRSTVID_GET %d\n",first_vid);
	//get	sinkcapability.
	aui_hdmi_sink_capability video_caps;
    AUI_TEST_CHECK_RTN(aui_hdmi_sinkcapability_get(aui_hdmi_handle, &video_caps));
    AUI_PRINTF("\n	aui_hdmi_sinkcapability_get:\n");
	AUI_PRINTF("	.............hdmi_prefer_video_resolution %d\n",video_caps.hdmi_prefer_video_resolution);
	AUI_PRINTF("	.............hdmi_supported_video_mode ");
	for(i = 0; i < AUI_HDMI_RES_NUM; i++)
		AUI_PRINTF("0x%x ",*(video_caps.hdmi_supported_video_mode + i));
	AUI_PRINTF("\n");
	AUI_PRINTF("	.............hdmi_prefer_audio_mode %d\n",video_caps.hdmi_prefer_audio_mode);
	AUI_PRINTF("	.............hdmi_supported_audio_mode ");
	for(i = 0; i < AUI_EDID_AUDIO_NUM; i++)
		AUI_PRINTF("0x%x ",*(video_caps.hdmi_supported_audio_mode + i));
	AUI_PRINTF("\n");
	AUI_PRINTF("	.............hdmi_hdcp_support %d\n",video_caps.hdmi_hdcp_support);

	//get color space.
    aui_hdmi_color_space color_space = 0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_IOCT_GET_COLOR_SPACE, (void *)&color_space, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_IOCT_GET_COLOR_SPACE %d\n",color_space);

	//get deep color.
    aui_hdmi_deepcolor deep_color = 0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_IOCT_GET_DEEP_COLOR, (void *)&deep_color, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_IOCT_GET_DEEP_COLOR %d\n",deep_color);

	//get edid deep color.
    unsigned int edid_code = 0;
    AUI_TEST_CHECK_RTN(aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_IOCT_GET_EDID_DEEP_COLOR, (void *)&edid_code, NULL));
    AUI_PRINTF("\n	.............AUI_HDMI_IOCT_GET_EDID_DEEP_COLOR %d\n",edid_code);

    return AUI_RTN_SUCCESS;

}

unsigned long test_hdmi_cec_on(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = AUI_RTN_SUCCESS;
    aui_hdl hdmi_handle = NULL;
    unsigned char onoff_status = 0;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }
    ret = aui_hdmi_cec_set_onoff(hdmi_handle, 1);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("set hdmi cec on fail!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_cec_get_onoff(hdmi_handle, &onoff_status);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("get hdmi cec onoff fail!\n");
        return AUI_RTN_FAIL;
    }

    if(onoff_status != 1) {
        AUI_PRINTF("hdmi cec onoff status not match!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;

}

unsigned long test_hdmi_cec_off(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = AUI_RTN_SUCCESS;
    aui_hdl hdmi_handle = NULL;
    unsigned char onoff_status = 0;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }
    ret = aui_hdmi_cec_set_onoff(hdmi_handle, 0);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("set hdmi cec off fail!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_cec_get_onoff(hdmi_handle, &onoff_status);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("get hdmi cec onoff fail!\n");
        return AUI_RTN_FAIL;
    }

    if(onoff_status != 0) {
        AUI_PRINTF("hdmi cec onoff status not match!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

static unsigned long cec_laddr_polling(aui_hdl hdmi_handle, unsigned char dest_address)
{
    unsigned char buf = 0;
    unsigned char l_addr = 0x0;
    unsigned long ret = AUI_RTN_SUCCESS;

    ret = aui_hdmi_cec_get_logical_address(hdmi_handle, &l_addr);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("get logical address fail\n");
        return AUI_RTN_FAIL;
    }

    buf = ( l_addr << 4) | dest_address;

    ret = aui_hdmi_cec_transmit(hdmi_handle, &buf, 1);

    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("not accept logic address 0x%x\n", dest_address);
        return AUI_RTN_FAIL;
    } else {
        AUI_PRINTF("accept logic address 0x%x\n", dest_address);
    }

    return AUI_RTN_SUCCESS;
}


unsigned long test_hdmi_cec_allocate_logic_address(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = AUI_RTN_SUCCESS;
    unsigned short physical_address = 0xffff;
    unsigned long wait_count = 10;
    unsigned long i = 0;
    aui_hdl hdmi_handle = NULL;
    unsigned char stb_logicaddress[4] = {AUI_CEC_LA_TUNER_1, AUI_CEC_LA_TUNER_2, AUI_CEC_LA_TUNER_3, AUI_CEC_LA_TUNER_4};
    unsigned long try_index = 0;
    unsigned char logic_address = 0;
    unsigned long found = 0;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }
    ret = aui_hdmi_cec_set_onoff(hdmi_handle, 1);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("set hdmi cec off fail!\n");
        return AUI_RTN_FAIL;
    }

    while(i < wait_count) {
        ret = aui_hdmi_get(hdmi_handle, AUI_HDMI_PHYSICAL_ADDR_GET, &physical_address, NULL);
        if(ret != AUI_RTN_SUCCESS) {
            AUI_PRINTF("get hdmi physical address fail!\n");
            return AUI_RTN_FAIL;
        }
        if(physical_address == 0xffff) {
            AUI_PRINTF("please confirm hdmi connect!\n");
            AUI_SLEEP(1000);
            i++;
        } else {
            break;
        }
    }

    if(i == wait_count) {
        AUI_PRINTF("get edid error!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_cec_get_logical_address(hdmi_handle, &logic_address);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("get logical address fail\n");
        return AUI_RTN_FAIL;
    }

    switch(logic_address) {
        case AUI_CEC_LA_TUNER_1:
            try_index = 0;
            break;
        case AUI_CEC_LA_TUNER_2:
            try_index = 1;
            break;
        case AUI_CEC_LA_TUNER_3:
            try_index = 2;
            break;
        case AUI_CEC_LA_TUNER_4:
            try_index = 3;
            break;
    }

    for(i = 0; i < 4; i++) {
        ret = aui_hdmi_cec_set_logical_address(hdmi_handle, stb_logicaddress[(try_index + i) % 4]);
        if(ret != AUI_RTN_SUCCESS) {
            AUI_PRINTF("set stb logic address fail\n");
            return AUI_RTN_FAIL;
        }
        ret = cec_laddr_polling(hdmi_handle, stb_logicaddress[(try_index + i) % 4]);
        if(ret != AUI_RTN_SUCCESS) {
            found = 1;
            break;
        }
    }

    if(found) {
        ret = aui_hdmi_cec_get_logical_address(hdmi_handle, &logic_address);
        if(ret != AUI_RTN_SUCCESS) {
            AUI_PRINTF("get logical address fail\n");
            return AUI_RTN_FAIL;
        }
        AUI_PRINTF("allocate logic address success, logic address is 0x%x\n", logic_address);
    } else {
        ret = aui_hdmi_cec_set_logical_address(hdmi_handle, AUI_CEC_LA_BROADCAST);
        if(ret != AUI_RTN_SUCCESS) {
            AUI_PRINTF("set stb logic address fail\n");
            return AUI_RTN_FAIL;
        }
    }

    return AUI_RTN_SUCCESS;
}

static int cec_receive_callback(unsigned char *buf, unsigned char length,void *userdata)
{
    unsigned long i = 0;
	if( NULL == userdata) {
		return 0;
	}
	unsigned char *pdata = (unsigned char *)userdata;
	AUI_PRINTF(" usrdata = %s\n",pdata );

    if(buf != NULL) {
        AUI_PRINTF("receive cec data:");
        for(i = 0; i < length; i++) {
            AUI_PRINTF(" 0x%x", buf[i]);
        }
        AUI_PRINTF(".\n");
    }
    return 0;
}

unsigned long test_hdmi_cec_receive(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }
    cec_usrdata = (unsigned char *)MALLOC(sizeof("test callback cec receive."));
	if( NULL == cec_usrdata) {
		AUI_PRINTF("mem fail!\n");
		return AUI_RTN_FAIL;
	}
	cec_usrdata = "test callback cec receive.";
    ret = aui_hdmi_callback_reg(hdmi_handle, AUI_HDMI_CB_CEC_MESSAGE, (void *)cec_receive_callback, (void *)cec_usrdata);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("register cec receive callback fail!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

static int plug_in_callback(void *userdata)
{
    AUI_PRINTF("\n>>>>>>>>>>>>>>>>>\n");
    AUI_PRINTF("HDMI plug in\n");
    AUI_PRINTF(">>>>>>>>>>>>>>>>>\n");

	if( NULL == userdata) {
		return 0;
	}
	unsigned char *pdata = (unsigned char *)userdata;
	AUI_PRINTF(" usrdata = %s\n",pdata );

	return 0;
}

static int plug_out_callback(void *userdata)
{
    AUI_PRINTF("\n>>>>>>>>>>>>>>>>>\n");
    AUI_PRINTF("HDMI plug out\n");
    AUI_PRINTF(">>>>>>>>>>>>>>>>>\n");

	if( NULL == userdata) {
		return 0;
	}
	unsigned char *pdata = (unsigned char *)userdata;
	AUI_PRINTF(" usrdata = %s\n",pdata );

	return 0;
}
static void hdcp_fail_callback(unsigned char *buf, unsigned char length,void *userdata)
{
    AUI_PRINTF("\n>>>>>>>>>>>>>>>>>\n");
    AUI_PRINTF("hdcp_fail_callback\n");

    if(buf != NULL) {
        AUI_PRINTF("receive hdcp massage: %s\n", buf);
        /*for(i = 0; i < length; i++) {
            AUI_PRINTF(" 0x%x", buf[i]);
        }
        AUI_PRINTF(".\n");*/
    }
    AUI_PRINTF(">>>>>>>>>>>>>>>>>\n");
}

unsigned long test_hdmi_reg_callback_plug_in_sample(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }
    plugin_usrdata = (unsigned char *)MALLOC(sizeof("test callback plug in."));
	if( NULL == plugin_usrdata) {
		AUI_PRINTF("mem fail!\n");
		return AUI_RTN_FAIL;
	}
	plugin_usrdata = "test callback plug in.";

    ret = aui_hdmi_callback_reg(hdmi_handle, AUI_HDMI_CB_HOT_PLUG_IN, (void *)plug_in_callback, (void *)plugin_usrdata);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("register plug in callback fail!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_reg_callback_plug_out(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }
    plugout_usrdata = (unsigned char *)MALLOC(sizeof("test callback plug out."));
	if( NULL == plugout_usrdata) {
		AUI_PRINTF("mem fail!\n");
		return AUI_RTN_FAIL;
	}
	plugout_usrdata = "test callback plug out.";

    ret = aui_hdmi_callback_reg(hdmi_handle, AUI_HDMI_CB_HOT_PLUG_OUT, (void *)plug_out_callback, (void *)plugout_usrdata);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("register plug in callback fail!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_reg_callback_hdcp_error(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_callback_reg(hdmi_handle, AUI_HDMI_CB_HDCP_FAIL, (void *)hdcp_fail_callback, NULL);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("register hdcp_fail_callback fail!\n");
        return AUI_RTN_FAIL;
    }
    AUI_PRINTF("register hdcp_fail_callback ok!\n");
    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_del_callback_plug_in(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_callback_del(hdmi_handle, AUI_HDMI_CB_HOT_PLUG_IN, (void *)plug_in_callback, NULL);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("delete plug in callback fail!\n");
        return AUI_RTN_FAIL;
    }
	if( NULL == plugin_usrdata) {
		AUI_PRINTF("There is no parameter input!\n");
		return AUI_RTN_SUCCESS;
	}
	AUI_PRINTF("free mem!\n");
	FREE(plugin_usrdata);
	plugin_usrdata = NULL;

    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_del_callback_plug_out(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_callback_del(hdmi_handle, AUI_HDMI_CB_HOT_PLUG_OUT, (void *)plug_out_callback, NULL);
	if( NULL == plugout_usrdata) {
		AUI_PRINTF("There is no parameter input!\n");
		return AUI_RTN_SUCCESS;
	}
	AUI_PRINTF("free mem!\n");
	FREE(plugout_usrdata);
	plugout_usrdata = NULL;

    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("delete plug in callback fail!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_del_callback_hdcp_fail(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret;
    aui_hdl hdmi_handle = NULL;

    ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("find hdmi handle fail!\n");
        return AUI_RTN_FAIL;
    }

    ret = aui_hdmi_callback_del(hdmi_handle, AUI_HDMI_CB_HDCP_FAIL, (void *)hdcp_fail_callback, NULL);
	if( NULL == cec_usrdata) {
		AUI_PRINTF("There is no parameter input!\n");
		return AUI_RTN_SUCCESS;
	}
	AUI_PRINTF("free mem!\n");
	FREE(cec_usrdata);
	cec_usrdata = NULL;
    if(ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("delete hdcp fail callback fail!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

static void enable_hdcp(unsigned char *key1,unsigned char *key2, int mode)
{
    if (mode) { // CE mode
        AUI_PRINTF("\n>>>HDCP CE solution\n");
        //Make sure the key is valid
        if (key1 != NULL) {
            if ((0 != key1[1]) || (0 != key1[2])) {
                HDCP_DUMP(key1, 288, "1.4 CE key");
                if (aui_kl_load_hdcp_key(key1, 288) != AUI_RTN_SUCCESS) {
                    AUI_PRINTF("aui_kl_load_hdcp_key err \n");
                }
            }
        }
        /*
         *Key2 is hdcp22 solution, only support S3922.
         */
        if (key2 != NULL) {
            if ((0 != key2[1]) || (0 != key2[2])) {
                AUI_PRINTF("aui_hdcp_params_set hdcp22 begin\n");
                HDCP_DUMP(key2, 416, "2.2 CE key");
                if(aui_hdcp_params_set(aui_hdmi_handle, key2, 416) != AUI_RTN_SUCCESS) {
                    AUI_PRINTF("aui_hdcp_params_set hdcp22 err\n");
                }
                AUI_PRINTF("aui_hdcp_params_set hdcp22 end\n");
            }
        }
    } else { // MEM mode
        AUI_PRINTF("\n>>>HDCP MEM solution\n");
        /*
         * Please set one of the keys or both keys are set.
         * You can set two kinds of key, the system will automatically choose to support the way.
         * If TV support hdcp1.4 and hdcp2.2, the system will use hdcp2.2 by default.
         */
        if (key1 != NULL) {
            if ((0 != key1[1]) || (0 != key1[2])) {
                HDCP_DUMP(key1, 286, "1.4 MEM key");
                if(aui_hdcp_params_set(aui_hdmi_handle, key1, 286) != AUI_RTN_SUCCESS) {
                    AUI_PRINTF("aui_hdcp_params_set hdcp14 err\n");
                }
            }
        }
        /*
         *Key2 is hdcp22 solution, only support S3922.
         */
        if (key2 != NULL) {
            if ((0 != key2[1]) || (0 != key2[2])) {
                HDCP_DUMP(key2, 402, "2.2 MEM key");
                if(aui_hdcp_params_set(aui_hdmi_handle, key2, 402) != AUI_RTN_SUCCESS) {
                    AUI_PRINTF("aui_hdcp_params_set hdcp22 err\n");
                }
            }
        }
    }
    if (aui_hdmi_mem_sel(aui_hdmi_handle , mode) != AUI_RTN_SUCCESS) {
        AUI_PRINTF("aui_hdmi_mem_sel err \n");
    }
    if (aui_hdmi_start( aui_hdmi_handle) != AUI_RTN_SUCCESS) {
        AUI_PRINTF("aui_hdmi_start err \n");
    }
    if (aui_hdmi_hdcp_on(aui_hdmi_handle) != AUI_RTN_SUCCESS) {
        AUI_PRINTF("aui_hdmi_hdcp_on failed!\n");
    }

}
void  HDCP_DUMP(unsigned char *data, unsigned int len, unsigned char *name)
{
    (void) data;
	(void) name;
    const int l = (len);
    int i;
    AUI_PRINTF("\ndump--[%s]\n", name);
    for(i = 0 ; i < l ; i++) {
        AUI_PRINTF("0x%02x,", (*(data + i)));
        if((i + 1) % 16 == 0)
            AUI_PRINTF("\n");
    }
    AUI_PRINTF("\n");
}

unsigned long test_hdmi_hdcp_on(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	aui_attr_hdmi attr_hdmi;
	unsigned char *key1_4 = NULL;
	unsigned char *key2_2 = NULL;
	int mode = 0;

	AUI_PRINTF("========import indicator:========\n");
	AUI_PRINTF("if HDCP is configed as DISABLE when system setup.HDCP will not work!\n");
	AUI_PRINTF("if HDCP is configed as ENABLE when system setup.and if HDCP_FROM_CE MACRO is defined, then only HDCP CE mode can be used,or only HDCP MEM mode can be used!\n");

	AUI_PRINTF("First, Run test 1 to reload the keys at the root directory of USB stick\n");
	AUI_PRINTF("HDCP 1.4 encrypted key%s\n", HDCP_CE_KEY_FILE_PATH);
	AUI_PRINTF("HDCP 1.4 plain key%s\n", HDCP_SW_KEY_FILE_PATH);
	AUI_PRINTF("HDCP 2.2 encrypted key%s\n", HDCP22_CE_KEY_FILE_PATH);
	AUI_PRINTF("HDCP 2.2 plain key%s\n", HDCP22_SW_KEY_FILE_PATH);

    if (*argc <= 1) {
        AUI_PRINTF("\nUsage:\n");
        AUI_PRINTF("15 ce,valid_key\n");
        AUI_PRINTF("15 ce,invalid_key\n");
        AUI_PRINTF("15 mem,valid_key\n");
        AUI_PRINTF("15 mem,invalid_key\n");
        AUI_PRINTF("\n");
        AUI_PRINTF("Set only HDCP 1.4 key sample:\n");
        AUI_PRINTF("15 ce,valid_key,1\n");
        AUI_PRINTF("\n");
        AUI_PRINTF("Set only HDCP 2.2 key sample:\n");
        AUI_PRINTF("15 ce,valid_key,2\n");
        return AUI_RTN_FAIL;
    }

    MEMSET(&attr_hdmi, 0, sizeof(aui_attr_hdmi));
    if(NULL == aui_hdmi_handle) {
        attr_hdmi.uc_dev_idx = 0;
        if(aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &aui_hdmi_handle)) {
            if (AUI_RTN_SUCCESS != aui_hdmi_init(NULL)) {
                AUI_PRINTF("aui_hdmi_init failed!\n");
                return AUI_RTN_FAIL;
            }
            if (aui_hdmi_open(&attr_hdmi, &aui_hdmi_handle)) {
                AUI_PRINTF("\n aui_hdmi_open  fail\n");
                return AUI_RTN_FAIL;
            }
        }
        AUI_PRINTF("\nopen hdmi handle.......\n");
    } else {
        AUI_PRINTF("\n!!! hdmi handel already opened\n");
    }

    // make sure HDCP off status
    if(aui_hdmi_hdcp_off(aui_hdmi_handle)) {
        AUI_PRINTF("\n aui_hdmi_hdcp_off fail\n");
        return AUI_RTN_FAIL;
    }
    AUI_PRINTF("aui_hdmi_hdcp_off done.........\n");

    if(MEMCMP("ce", argv[0], 2) == 0) {
        mode = 1;
        if(MEMCMP("valid_key", argv[1], 9) == 0) {
            AUI_PRINTF("ce mode :valid_key\n");
            if (*argc >= 3 && argv[2][0] == '1') {
                key1_4 = test_otp_key;
                key2_2 = NULL;
            } else if (*argc >= 3 && argv[2][0] == '2') {
                key1_4 = NULL;
                key2_2 = test_plain_hdcp22_ce_key;
            } else {
                key1_4 = test_otp_key;
               key2_2 = test_plain_hdcp22_ce_key;
            }
            //enable_hdcp(test_otp_key, test_plain_hdcp22_ce_key, 1);
            //HDCP_DUMP(test_otp_key, 288, "1.4 ce mode :valid_key");
            //HDCP_DUMP(test_plain_hdcp22_ce_key, 416, "2.2 ce mode :valid_key");
        } else if(MEMCMP("invalid_key", argv[1], 11) == 0) {
            AUI_PRINTF("ce mode :invalid_key\n");
            if (*argc >= 3 && argv[2][0] == '1') {
                key1_4 = test_otp_key_invalid;
                key2_2 = NULL;
            } else if (*argc >= 3 && argv[2][0] == '2') {
                key1_4 = NULL;
                key2_2 = test_plain_hdcp22_ce_key_invalid;
            } else {
                key1_4 = test_otp_key_invalid;
               key2_2 = test_plain_hdcp22_ce_key_invalid;
            }
            //enable_hdcp(test_otp_key_invalid, test_plain_hdcp22_ce_key_invalid, 1);
            //HDCP_DUMP(test_otp_key_invalid, 288, "1.4 ce mode :invalid_key");
            //HDCP_DUMP(test_plain_hdcp22_ce_key_invalid, 416, "2.2 ce mode :invalid_key");
        } else {
            AUI_PRINTF(" Error!! argv[1] should be valid_key/invalid_key\n");
            return AUI_RTN_FAIL;
        }
    } else if(MEMCMP("mem", argv[0], 3) == 0) {
        mode = 0;
        if(MEMCMP("valid_key", argv[1], 9) == 0) {
            AUI_PRINTF("mem mode :valid_key\n");
            if (*argc >= 3 && argv[2][0] == '1') {
                key1_4 = test_plain_key;
                key2_2 = NULL;
            } else if (*argc >= 3 && argv[2][0] == '2') {
                key1_4 = NULL;
                key2_2 = test_plain_hdcp22_key;
            } else {
                key1_4 = test_plain_key;
               key2_2 = test_plain_hdcp22_key;
            }
            //enable_hdcp(test_plain_key, test_plain_hdcp22_key, 0);
            //HDCP_DUMP(test_plain_key, 286, "mem mode :hdcp14 valid_key");
            //HDCP_DUMP(test_plain_hdcp22_key, 402, "2.2 mem mode :valid_key");
        } else if(MEMCMP("invalid_key", argv[1], 11) == 0) {
            AUI_PRINTF("mem mode :invalid_key\n");
            if (*argc >= 3 && argv[2][0] == '1') {
                key1_4 = test_plain_key_invalid;
                key2_2 = NULL;
            } else if (*argc >= 3 && argv[2][0] == '2') {
                key1_4 = NULL;
                key2_2 = test_plain_hdcp22_key_invalid;
            } else {
                key1_4 = test_plain_key_invalid;
               key2_2 = test_plain_hdcp22_key_invalid;
            }
            //enable_hdcp(test_plain_key_invalid, test_plain_hdcp22_key_invalid, 0);
            //HDCP_DUMP(test_plain_key_invalid, 286, "mem mode :hdcp14 invalid_key");
            //HDCP_DUMP(test_plain_hdcp22_key_invalid, 402, "2.2 mem mode :invalid_key");
        } else {
            AUI_PRINTF(" Error!! argv[1] should be valid_key/invalid_key\n");
            return AUI_RTN_FAIL;
        }
    } else {
        AUI_PRINTF("Error!! argv[0] should be ce/mem\n");
        return AUI_RTN_FAIL;
    }

    // Enable HDCP
    enable_hdcp(key1_4, key2_2, mode);

    AUI_PRINTF("\nenable_hdcp done\n");

    if(aui_hdmi_on(aui_hdmi_handle)) {
        AUI_PRINTF("\n aui_hdmi_on fail\n");
        return AUI_RTN_FAIL;
    }
    AUI_PRINTF("\n aui_hdmi_on donen......");

    // HDCP state get ready may take some time in TDS system
    // TDS takes about 2s, Linux takes about 5s
    aui_hdmi_link_status hdcp_link_status = AUI_HDMI_STATUS_UNLINK;
    int i = 10;
    do {
        if (aui_hdmi_get(aui_hdmi_handle, AUI_HDMI_HDCP_STATUS_GET,
                &hdcp_link_status, NULL) != AUI_RTN_SUCCESS) {
            AUI_PRINTF("\n aui_hdmi_get AUI_HDMI_HDCP_STATUS_GET fail\n");
            break;
        } else {
            if (hdcp_link_status != AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED) {
                AUI_PRINTF("\n HDCP state not OK yet, %d\n", hdcp_link_status);
            } else {
                AUI_PRINTF("\n HDCP state OK, %d\n", hdcp_link_status);
                break;
            }
        }
        if (i > 0) {
            AUI_SLEEP(1000);
        }
    } while (i-- >= 0);
    if (hdcp_link_status != AUI_HDMI_STATUS_LINK_HDCP_SUCCESSED) {
        AUI_PRINTF("\n HDCP state FAIL %d\n", hdcp_link_status);
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;

}

unsigned long test_hdmi_hdcp_off(unsigned long *argc, char **argv, char *sz_out_put)
{
    // after hdcp off, the driver do not stop hdcp actually,
    // we need to delete the callback function to ignore the hdcp message from driver.
    if(aui_hdmi_hdcp_off(aui_hdmi_handle)) {
        AUI_PRINTF("\n aui_hdmi_hdcp_off fail\n");
        return AUI_RTN_FAIL;
    }
    AUI_PRINTF("\n test_hdmi_hdcp_off donen......");
    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_av_blank_mute(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned int on_off=0;
	aui_attr_hdmi attr_hdmi;

	if(NULL == aui_hdmi_handle) {
        attr_hdmi.uc_dev_idx = 0;
        if(aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &aui_hdmi_handle)) {
            if (AUI_RTN_SUCCESS != aui_hdmi_init(NULL)) {
                AUI_PRINTF("aui_hdmi_init failed!\n");
                return AUI_RTN_FAIL;
            }
            if (aui_hdmi_open(&attr_hdmi, &aui_hdmi_handle)) {
                AUI_PRINTF("\n aui_hdmi_open  fail\n");
                return AUI_RTN_FAIL;
            }
        }
        AUI_PRINTF("\nopen hdmi handle.......\n");
    } else {
        AUI_PRINTF("\n!!! hdmi handel already opened\n");
    }
	if(*argc != 2) {
        AUI_PRINTF("\nUsage:\n");
        AUI_PRINTF("19 avmute,on\n");
        AUI_PRINTF("19 avmute,off\n");
        AUI_PRINTF("19 avblank,on\n");
        AUI_PRINTF("19 avblank,off\n");
        return AUI_RTN_FAIL;
    }

    if(MEMCMP("avmute", argv[0], 6) == 0) {
        if(MEMCMP("on", argv[1], 2) == 0) {
            AUI_PRINTF("avmute on\n");
            AUI_TEST_CHECK_RTN(aui_hdmi_set(aui_hdmi_handle, AUI_HDMI_AV_MUTE_SET, NULL, NULL));
        } else if(MEMCMP("off", argv[1], 3) == 0) {
            AUI_PRINTF("avmute off\n");
			AUI_TEST_CHECK_RTN(aui_hdmi_set(aui_hdmi_handle, AUI_HDMI_AV_UNMUTE_SET, NULL, NULL));
        } else {
            AUI_PRINTF(" Error!! argv[1] should be on/off\n");
            return AUI_RTN_FAIL;
        }
    } else if(MEMCMP("avblank", argv[0], 7) == 0) {
        if(MEMCMP("on", argv[1], 2) == 0) {
			on_off= 1;
            AUI_PRINTF("avblank on\n");
        } else if(MEMCMP("off", argv[1], 3) == 0) {
			on_off= 0;
            AUI_PRINTF("avblank off\n");
        } else {
            AUI_PRINTF(" Error!! argv[1] should be on/off\n");
            return AUI_RTN_FAIL;
        }
		AUI_TEST_CHECK_RTN(aui_hdmi_set(aui_hdmi_handle, AUI_HDMI_IOCT_SET_AV_BLANK, NULL, &on_off));
    } else {
        AUI_PRINTF("Error!! argv[0] should be avmute/avblank\n");
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_set_color_space(unsigned long *argc, char **argv, char *sz_out_put)
{
	 unsigned long ret;
	 unsigned long color_space;
	 aui_hdl hdmi_handle = NULL;

	 /* Usually the color space will automatically set up.
	  ** AUI_HDMI_YCBCR_420 set by DE*/

	 ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
	 if(ret != AUI_RTN_SUCCESS) {
		 AUI_PRINTF("find hdmi handle fail!\n");
		 return AUI_RTN_FAIL;
	 }

	 if(*argc == 1) {
		 color_space = ATOI(argv[0]);
	 } else {
		 AUI_PRINTF("%s -> you can input :\n",__FUNCTION__);
		 AUI_PRINTF("20 [num]\n");
		 AUI_PRINTF("The testing process is described in the help.");
		 AUI_SLEEP(2000);
		 return AUI_RTN_FAIL;
	 }

	 if(color_space > 2) {
	     AUI_PRINTF("There is no such value.");
		 return AUI_RTN_FAIL;
	 }

	 AUI_TEST_CHECK_RTN(aui_hdmi_set_color(hdmi_handle, (aui_hdmi_color_space)color_space));
	 AUI_PRINTF("hdmi_color_space = %d\n",color_space);

     return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_set_deep_color(unsigned long *argc, char **argv, char *sz_out_put)
{
   unsigned long ret;
   unsigned long deep_color;
   aui_hdl hdmi_handle = NULL;

   ret = aui_find_dev_by_idx(AUI_MODULE_HDMI, 0, &hdmi_handle);
   if(ret != AUI_RTN_SUCCESS) {
     AUI_PRINTF("find hdmi handle fail!\n");
     return AUI_RTN_FAIL;
   }

   if(*argc == 1) {
     deep_color = ATOI(argv[0]);
   } else {
     AUI_PRINTF("%s -> you can input :\n",__FUNCTION__);
     AUI_PRINTF("21 [num]\n");
     AUI_PRINTF("The testing process is described in the help.");
     AUI_SLEEP(2000);
     return AUI_RTN_FAIL;
   }

   if(deep_color > 3) {
       AUI_PRINTF("There is no such value.");
     return AUI_RTN_FAIL;
   }
   //refer to struct aui_hdmi_deepcolor to set the parameters
   enum aui_hdmi_deepcolor aui_deep_color = (1<<deep_color);
   AUI_TEST_CHECK_RTN(aui_hdmi_set(hdmi_handle, AUI_HDMI_IOCT_SET_DEEP_COLOR, NULL, (void *)(&aui_deep_color)));
   AUI_PRINTF("hdmi_deep_color = %d\n",(1<<deep_color));

   return AUI_RTN_SUCCESS;
}

unsigned long test_hdmi_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nHDMI Test Help");

	/* HDMI_1_HELP */
	#define 	HDMI_1_HELP		"Before doing the other HDMI test items, the step of init the HDMI module should be executed first.\n"
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Init the HDMI module.\n");
	aui_print_help_instruction_newline(HDMI_1_HELP);


	/* HDMI_2_HELP */
	#define 	HDMI_2_HELP		"Make the HDMI output audio/video signal.\n"
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("HDMI On function.\n");
	aui_print_help_instruction_newline(HDMI_2_HELP);


	/* HDMI_3_HELP */
	#define 	HDMI_3_HELP		"Make the HDMI not output audio/video signal.\n"
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline("HDMI Off function.\n");
	aui_print_help_instruction_newline(HDMI_3_HELP);


	/* HDMI_4_HELP */
	#define 	HDMI_4_HELP		"Make the HDMI can output audio.\n"
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline("HDMI audio on function.\n");
	aui_print_help_instruction_newline(HDMI_4_HELP);

	/* HDMI_5_HELP */
	#define 	HDMI_5_HELP		"Make the HDMI cannot output audio.\n"
	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline("HDMI audio off function.\n");
	aui_print_help_instruction_newline(HDMI_5_HELP);

	/* HDMI_6_HELP */
	#define 	HDMI_6_HELP		"Get HDMI status info.such as EDID,HDMI Link status,CEC addr.etc"

	aui_print_help_command("\'6\'");
	aui_print_help_instruction_newline("HDMI get param function.\n");
	aui_print_help_instruction_newline(HDMI_6_HELP);

	/* HDMI_7_HELP */
	#define 	HDMI_7_HELP		"Make the HDMI CEC enable.\n"
	aui_print_help_command("\'7\'");
	aui_print_help_instruction_newline("HDMI CEC on function.\n");
	aui_print_help_instruction_newline(HDMI_7_HELP);

	/* HDMI_8_HELP */
	#define 	HDMI_8_HELP		"Make the HDMI CEC disable.\n"

	aui_print_help_command("\'8\'");
	aui_print_help_instruction_newline("HDMI CEC off function.\n");
	aui_print_help_instruction_newline(HDMI_8_HELP);

	/* HDMI_9_HELP */
	#define 	HDMI_9_HELP		"Get HDMI CEC logical address.\n"
	aui_print_help_command("\'9\'");
	aui_print_help_instruction_newline("HDMI CEC Get logical address function.\n");
	aui_print_help_instruction_newline(HDMI_9_HELP);

	/* HDMI_10_HELP */
	#define 	HDMI_10_HELP		"Register HDMI CEC msg receive callback.\n"

	aui_print_help_command("\'10\'");
	aui_print_help_instruction_newline(HDMI_10_HELP);

	/* HDMI_11_HELP */
	#define 	HDMI_11_HELP		"Register HDMI plug in callback.\n"

	aui_print_help_command("\'11\'");
	aui_print_help_instruction_newline(HDMI_11_HELP);

	/* HDMI_12_HELP */
	#define 	HDMI_12_HELP		"Register HDMI plug out callback.\n"

	aui_print_help_command("\'12\'");
	aui_print_help_instruction_newline(HDMI_12_HELP);

	/* HDMI_13_HELP */
	#define 	HDMI_13_HELP		"Delete HDMI plug in callback.\n"

	aui_print_help_command("\'13\'");
	aui_print_help_instruction_newline(HDMI_13_HELP);

	/* HDMI_14_HELP */
	#define 	HDMI_14_HELP		"Delete HDMI plug out callback.\n"

	aui_print_help_command("\'14\'");
	aui_print_help_instruction_newline(HDMI_14_HELP);

	/* HDMI_15_HELP */

	#define 	HDMI_15_HELP			"Before test HDMI HDCP.On TDS platfrom please sure hdcp is enable.\n"

	#define 	HDMI_15_HELP_PART1		"Format:	   15 [mode],[key],[version]"
	#define 	HDMI_15_HELP_PART2		"			   mode:ce/mem, key:valid_key/invalid_key, version(Optional):1/2\n"
	#define 	HDMI_15_HELP_PART3		"Example:	   If use hdcp 1.4/2.2 ce mode by valid_key:"
	#define 	HDMI_15_HELP_PART4		"			   15 ce,valid_key\n"
	#define 	HDMI_15_HELP_PART5		"			   If use hdcp 1.4 mem mode by valid_key:"
	#define 	HDMI_15_HELP_PART6		"			   15 mem,valid_key,1\n"

	aui_print_help_command("\'15\'");
	aui_print_help_instruction_newline(HDMI_15_HELP);
	aui_print_help_instruction_newline(HDMI_15_HELP_PART1);
	aui_print_help_instruction_newline(HDMI_15_HELP_PART2);
	aui_print_help_instruction_newline(HDMI_15_HELP_PART3);
	aui_print_help_instruction_newline(HDMI_15_HELP_PART4);
	aui_print_help_instruction_newline(HDMI_15_HELP_PART5);
	aui_print_help_instruction_newline(HDMI_15_HELP_PART6);

    /* HDMI_16_HELP */
	#define 	HDMI_16_HELP		"Stop HDCP that start by test case 15.\n"

	aui_print_help_command("\'16\'");
	aui_print_help_instruction_newline(HDMI_16_HELP);

    /* HDMI_17_HELP */
	#define 	HDMI_17_HELP_PART1		"Register HDMI hdcp fail callback.When HDCP fail, would print "
    #define     HDMI_17_HELP_PART2     "\n\">>>>>>>>>>>>>>>>>\nhdcp_fail_callback\n>>>>>>>>>>>>>>>>>\"\n"

	aui_print_help_command("\'17\'");
	aui_print_help_instruction_newline(HDMI_17_HELP_PART1);
    aui_print_help_instruction_newline(HDMI_17_HELP_PART2);

	/* HDMI_18_HELP */
	#define 	HDMI_18_HELP		"Delete HDMI plug in callback.\n"

	aui_print_help_command("\'18\'");
	aui_print_help_instruction_newline(HDMI_18_HELP);


	/* HDMI_19_HELP */
	#define 	HDMI_19_HELP			"HDMI avmute/avblank function used to make audio slient and make screen black.\n"
	#define 	HDMI_19_HELP_PART1		"Format:	   16 [method],[value]."
	#define 	HDMI_19_HELP_PART2		"			   method:avmute/avblank, value:on/off.\n"
	#define 	HDMI_19_HELP_PART3		"Example:	   If want to use HDMI avmute on:"
	#define 	HDMI_19_HELP_PART4		"			   16 avmute,on\n"
	#define 	HDMI_19_HELP_PART5		"			   If want to use HDMI avblank on:"
	#define 	HDMI_19_HELP_PART6		"			   16 avblank,on\n"

	aui_print_help_command("\'19\'");
	aui_print_help_instruction_newline(HDMI_19_HELP);
	aui_print_help_instruction_newline(HDMI_19_HELP_PART1);
	aui_print_help_instruction_newline(HDMI_19_HELP_PART2);
	aui_print_help_instruction_newline(HDMI_19_HELP_PART3);
	aui_print_help_instruction_newline(HDMI_19_HELP_PART4);
	aui_print_help_instruction_newline(HDMI_19_HELP_PART5);
	aui_print_help_instruction_newline(HDMI_19_HELP_PART6);

	/* HDMI_20_HELP */
	#define 	HDMI_20_HELP			"set color space\n"
	#define 	HDMI_20_HELP_PART1		"Format:	   20 [value]."
	#define 	HDMI_20_HELP_PART2		"			   [value] \n"
	#define 	HDMI_20_HELP_PART3		"              0 : AUI_HDMI_YCBCR_422\n"
	#define 	HDMI_20_HELP_PART4		"			   1 : AUI_HDMI_YCBCR_444\n"
	#define 	HDMI_20_HELP_PART5		"			   2 : AUI_HDMI_RGB_MODE1\n"

	aui_print_help_command("\'20\'");
	aui_print_help_instruction_newline(HDMI_20_HELP);
	aui_print_help_instruction_newline(HDMI_20_HELP_PART1);
	aui_print_help_instruction_newline(HDMI_20_HELP_PART2);
	aui_print_help_instruction_newline(HDMI_20_HELP_PART3);
	aui_print_help_instruction_newline(HDMI_20_HELP_PART4);
	aui_print_help_instruction_newline(HDMI_20_HELP_PART5);

	/* HDMI_21_HELP */
	#define   HDMI_21_HELP		"set deep color\n"
	#define   HDMI_21_HELP_PART1	"Format:	 21 [value]."
	#define   HDMI_21_HELP_PART2	"		  [value] \n"
	#define   HDMI_21_HELP_PART3	"	      0 : AUI_HDMI_DEEPCOLOR_24\n"
	#define   HDMI_21_HELP_PART4	"		  1 : AUI_HDMI_DEEPCOLOR_30\n"
	#define   HDMI_21_HELP_PART5	"		  2 : AUI_HDMI_DEEPCOLOR_36\n"
	#define   HDMI_21_HELP_PART6	"		  3 : AUI_HDMI_DEEPCOLOR_48\n"

	aui_print_help_command("\'21\'");
	aui_print_help_instruction_newline(HDMI_21_HELP);
	aui_print_help_instruction_newline(HDMI_21_HELP_PART1);
	aui_print_help_instruction_newline(HDMI_21_HELP_PART2);
	aui_print_help_instruction_newline(HDMI_21_HELP_PART3);
	aui_print_help_instruction_newline(HDMI_21_HELP_PART4);
	aui_print_help_instruction_newline(HDMI_21_HELP_PART5);
	aui_print_help_instruction_newline(HDMI_21_HELP_PART6);

	/* HDMI_22_HELP */
	#define 	HDMI_22_HELP 	"Test cec transmit"

	aui_print_help_command("\'22\'");
	aui_print_help_instruction_newline("Test cec transmit,if there connect a cec tv,it should pass.\n");
	aui_print_help_instruction_newline(HDMI_22_HELP);

	return AUI_RTN_HELP;
}


