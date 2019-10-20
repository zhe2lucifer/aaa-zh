
#ifndef _AUI_AV_TEST_H
#define _AUI_AV_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_av.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
void aui_load_tu_av();

unsigned long test_av_start(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_av_stop(unsigned long *argc,char **argv,char *sz_out_put);


unsigned long test_av_pause(unsigned long *argc,char **argv,char *sz_out_put);


unsigned long test_av_resume(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_av_init(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_av_open(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_av_close(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_video_pid_set(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_audio_pid_set(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_pcr_pid_set(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_av_set(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_av_get(unsigned long *argc,char **argv,char *sz_out_put);


#ifdef __cplusplus
}
#endif

#endif

