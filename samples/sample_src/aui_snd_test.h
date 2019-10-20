/**@file
*    @brief 		UDI snd module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_SND_TEST_H
#define _AUI_SND_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_snd.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
extern void aui_load_tu_snd();
extern unsigned long test_snd_init(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_pause(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_resume(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_type_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_type_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_sync_level_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_sync_level_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_vol_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_vol_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_mute_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_mute_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_channel_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_channel_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_out_interface_type_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_out_interface_type_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_out_data_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_out_data_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_reg_cb(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_spdif_delay_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_snd_spdif_delay_get(unsigned long *argc,char **argv,char *sz_out_put);
#ifdef __cplusplus
}
#endif

#endif
