/**@file
*    @brief 			GPIO module public data struct and interface declear
*    @author			steven.zhang(steven.zhagn@alitech.com)
*    @date			2016-07-17
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_GPIO_TEST_H
#define _AUI_GPIO_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_gpio.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

extern void aui_load_tu_gpio();
extern unsigned long test_gpio_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_gpio_close(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_gpio_reg_cb(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_gpio_unreg_cb(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef __cplusplus
}
#endif

#endif
