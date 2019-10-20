/**@file
*    @brief 		UDI uart module public data struct and interface declear
*    @author		smith.shi
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_CA_TEST_H
#define _AUI_CA_TEST_H
/****************************INCLUDE FILE************************************/
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

void test_ca_reg();

extern unsigned long ca_test_init(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long ca_test_deinit(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long ca_test_register(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long ca_test_hmac_key_write(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long ca_test_hmac_key_read(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef __cplusplus
}
#endif

#endif //_AUI_PVR_TEST_H


