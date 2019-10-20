#ifndef _AUI_DECV_TEST_H
#define _AUI_DECV_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_decv.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

void aui_load_tu_decv();

unsigned long test_decv(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_decv_showlogo(unsigned long *argc,char **argv,char *sz_out_put);

#if 0
unsigned test_decv_open(unsigned long *argc,char **argv,char *sz_out_put);
unsigned test_decv_close(unsigned long *argc,char **argv,char *sz_out_put);

unsigned test_decv_start(unsigned long *argc,char **argv,char *sz_out_put);
unsigned test_decv_stop(unsigned long *argc,char **argv,char *sz_out_put);


unsigned test_decv_pause(unsigned long *argc,char **argv,char *sz_out_put);
unsigned test_decv_resume(unsigned long *argc,char **argv,char *sz_out_put);

unsigned test_decv_init(unsigned long *argc,char **argv,char *sz_out_put);
unsigned test_decv_de_init(unsigned long *argc,char **argv,char *sz_out_put);

unsigned test_decv_deode_format(unsigned long *argc,char **argv,char *sz_out_put);
unsigned test_decv_sync_mode(unsigned long *argc,char **argv,char *sz_out_put);

unsigned test_decv_change_mode(unsigned long *argc,char **argv,char *sz_out_put);
#endif

#ifdef __cplusplus
}
#endif

#endif
