/**@file
*    @brief 		UDI kl module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_DOG_TEST_H
#define _AUI_DOG_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_dog.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

extern unsigned long test_dog_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dog_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dog_time_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dog_time_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dog_config(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dog_channel_get(unsigned long *argc,char **argv,char *sz_out_put);

void test_dog_reg();

#ifdef __cplusplus
}
#endif

#endif
