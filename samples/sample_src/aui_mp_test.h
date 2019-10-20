#ifndef _AUI_MP_TEST_H
#define _AUI_MP_TEST_H
/****************************INCLUDE FILE************************************/

#include <aui_mp.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
unsigned long test_mp_open(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_close(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_start(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_stop(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_pause(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_resume(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_seek(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_speed(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_get_total_time(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_get_current_time(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_mp_get_info(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_aui_mp_set_callback(unsigned long *argc,char **argv,char *sz_out_put);




#ifdef __cplusplus
}
#endif
#endif



