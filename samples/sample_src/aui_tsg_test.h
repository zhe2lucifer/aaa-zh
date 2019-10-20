/**@file
*    @brief 		UDI snd module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_TSG_TEST_H
#define _AUI_TSG_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_tsg.h>
#include <aui_tsi.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long test_tsg_play_back_by_tsg(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_tsg_open_file(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_tsg_close_file(unsigned long *argc,char **argv,char *sz_out_put);


#ifdef __cplusplus
}
#endif

#endif
