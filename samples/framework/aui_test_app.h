#ifndef __AUI_TEST_APP_HEAD__
#define __AUI_TEST_APP_HEAD__

#ifdef AUI_LINUX 
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#else
#include <api/libfs2/stdio.h>
#endif
#include "aui_test_app_cmd.h"
#include "aui_common.h"
#include "unity.h"
#define SCPI_MBF_SIZE	(2048)

#ifndef NULL
#define NULL							0
#endif
#define AUI_RTN_CODE_SUCCESS		0
#define AUI_RTN_CODE_ERR		-1

#define CHECK_AUI_RET(ret) do {						\
	if(!ret) {						\
		AUI_PRINTF("%s:%d %s failed!\n", __FILE__,__LINE__, __FUNCTION__); \
		return AUI_RTN_FAIL;					\
	}								\
} while (0);

#define AUI_TEST_CHECK_RTN(func_name) do {				\
	AUI_RTN_CODE rtn = func_name;					\
	if(rtn != AUI_RTN_SUCCESS) {					\
		AUI_PRINTF("%s:%d %s() failed with err %d\n", __FILE__, __LINE__, __FUNCTION__, rtn); \
		return rtn;						\
	}								\
} while (0);

#define AUI_TEST_CHECK_NULL( param_name ) do {	\
	if(NULL==param_name) {					\
		AUI_PRINTF("%s:%d %s() param null\n", __FILE__, __LINE__, __FUNCTION__); \
		return AUI_RTN_EINVAL;		\
	}								\
} while (0);

#define AUI_TEST_CHECK_BOOL( param_name ) do {			\
	if(0==param_name)						\
	{								\
		AUI_PRINTF("%s:%d %s() check failed\n", __FILE__, __LINE__, __FUNCTION__); \
		return AUI_RTN_EINVAL;					\
	}								\
} while (0);

#define AUI_TEST_CHECK_VAL( param_name,value ) do {			\
	if(value!=param_name)						\
	{								\
		AUI_PRINTF("%s:%d %s() value missmatch %d != %d\n", __FILE__, __LINE__, \
		       __FUNCTION__, value, param_name);		\
		return AUI_RTN_EINVAL;					\
	}								\
} while (0);

#define AUI_TEST_CHECK_NOT_EQU_VAL( param_name,value ) do {		\
	if(value==param_name)						\
	{								\
		AUI_PRINTF("%s:%d %s() value equal %d == %d\n", __FILE__, __LINE__, \
		       __FUNCTION__, value, param_name);		\
		return AUI_RTN_EINVAL;					\
	}								\
} while (0);

typedef struct st_cmd_dev
{
	int bl_aui_scpi_task_flag;
	unsigned char *pby_main_cmd_buf;
	unsigned char *pby_ask_cmd_buf;
	unsigned long ui_cmd_status;
	unsigned long ui_main_cmd_rcv_cur_idx;
	unsigned long ui_ask_cmd_rcv_cur_idx;
}st_cmd_dev,*pst_cmd_dev;

//extern st_cmd_dev g_st_cmd_dev;

char *get_ask_cmd_buf_addr();
unsigned long drv_compare_ask_cmd(char *sz_rcv_cmd);
void rst_ask_cmd_status();
void rst_main_cmd_status();
void drv_process_cmd();
unsigned char auiscpi_init();
unsigned char aui_test_confirm_result(char *sz_info);
unsigned long drv_get_aui_cmd_task_status();
unsigned char aui_test_user_confirm(char *sz_info);

AUI_RTN_CODE aui_test_get_user_hex_input(unsigned long *pul_user_in);
AUI_RTN_CODE aui_test_get_user_key_input(char *pul_user_in);
AUI_RTN_CODE aui_test_get_user_dec_input(unsigned long *pul_user_in);
AUI_RTN_CODE aui_test_get_user_str_input(char *pul_user_in);
AUI_RTN_CODE str2ulong(unsigned char *psz_in, unsigned long ul_str_len, unsigned long *pul_out);

#endif
