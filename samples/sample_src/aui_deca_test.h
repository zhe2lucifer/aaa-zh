/**@file
*    @brief 		UDI deca module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_DECA_TEST_H
#define _AUI_DECA_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_deca.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
extern void aui_load_tu_deca();
unsigned long test_deca_init(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_pause(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_resume(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_data_info_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_status_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_type_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_type_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_sync_mode_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_sync_mode_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_sync_level_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_sync_level_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_aud_pid_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_aud_pid_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_dev_cnt_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_r(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_p(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_wt(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_file_name_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_file_type_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_wt(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_play_dat(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_inject_play_file(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_been_tone_start(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_been_tone_stop(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_pcm_inject(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_audio_mode(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_deca_beep(unsigned long *argc,char **argv,char *sz_out_put);
#ifdef __cplusplus
}
#endif

#endif
