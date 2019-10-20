/**@file
*    @brief 		UDI KEY module public data struct and interface declear
*    @author		smith.shi
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_INPUT_TEST_H
#define _AUI_INPUT_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_input.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

void test_key_reg();

extern unsigned long test_key_callback_test(unsigned long *argc,char **argv,char *sz_out_put);
#ifdef __cplusplus
}
#endif

#endif //_AUI_KEY_TEST_H

