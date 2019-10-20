/**@file
*    @brief 		UDI dmx module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_DMX_TEST_H
#define _AUI_DMX_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_dmx.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long test_dmx_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_pause(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_resume(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_data_sync_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_data_async_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_filter_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_filter_close(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_filter_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_filter_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_channel_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_channel_close(unsigned long *argc,char **argv,char *sz_out_put);

extern unsigned long test_dmx(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_dmx_with_nim(unsigned long *argc,char **argv,char *sz_out_put);

void aui_load_tu_dmx(void);

#ifdef __cplusplus
}
#endif

#endif
