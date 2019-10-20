/**@file
*    @brief 		UDI kl module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_KL_TEST_H
#define _AUI_KL_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_kl.h>
#include <aui_dsc.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long test_kl_gen_key_by_cfg(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef __cplusplus
}
#endif

#endif
