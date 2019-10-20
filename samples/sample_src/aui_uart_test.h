/**@file
*    @brief 		UDI uart module public data struct and interface declear
*    @author		smith.shi
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_UART_TEST_H
#define _AUI_UART_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_uart.h>
#include "aui_test_app.h"
#ifdef AUI_TDS 
#include <../inc/api/libc/string.h>
#endif
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

void test_uart_reg();

extern unsigned long test_uart_read(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_uart_write(unsigned long *argc,char **argv,char *sz_out_put);
#ifdef __cplusplus
}
#endif

#endif //_AUI_UART_TEST_H
