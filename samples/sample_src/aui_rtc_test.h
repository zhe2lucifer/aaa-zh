/**@file
*    @brief 		UDI kl module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_RTC_TEST_H
#define _AUI_RTC_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_rtc.h>
#include "aui_test_app.h"
#ifdef AUI_TDS 
#include <api/libc/printf.h>
#include <api/libc/string.h>
#endif

/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

void test_rtc_reg();

extern unsigned long test_rtc_time_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_rtc_time_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_rtc_alm_config(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_rtc_alm_on_off(unsigned long *argc,char **argv,char *sz_out_put);
#ifdef __cplusplus
}
#endif

#endif
