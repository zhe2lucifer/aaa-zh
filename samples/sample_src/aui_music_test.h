#ifndef _AUI_MUSIC_TEST_H
#define _AUI_MUSIC_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_music.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

AUI_RTN_CODE test_aui_music_open(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_music_close(unsigned long *argc,char **argv,char *sz_out_put);

AUI_RTN_CODE test_aui_music_start(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_music_stop(unsigned long *argc,char **argv,char *sz_out_put);

AUI_RTN_CODE test_aui_music_pause(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_music_resume(unsigned long *argc,char **argv,char *sz_out_put);

AUI_RTN_CODE aui_test_music_seek(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE aui_test_total_time_get(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE aui_test_cur_time_get(unsigned long *argc,char **argv,char *sz_out_put);


AUI_RTN_CODE test_aui_music_init(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_music_de_init(unsigned long *argc,char **argv,char *sz_out_put);


AUI_RTN_CODE test_aui_music_set(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_music_get(unsigned long *argc,char **argv,char *sz_out_put);
void music_test_reg();




#ifdef __cplusplus
}
#endif

#endif


