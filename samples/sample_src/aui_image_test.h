#ifndef _AUI_IMAGE_TEST_H
#define _AUI_IMAGE_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_image.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

AUI_RTN_CODE test_aui_image_open(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_image_close(unsigned long *argc,char **argv,char *sz_out_put);

AUI_RTN_CODE test_aui_image_start(unsigned long *argc,char **argv,char *sz_out_put);

AUI_RTN_CODE test_aui_image_set(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_image_get(unsigned long *argc,char **argv,char *sz_out_put);

AUI_RTN_CODE test_aui_image_zoom(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_image_rotate(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_image_display_mode_set(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_image_move(unsigned long *argc,char **argv,char *sz_out_put);
AUI_RTN_CODE test_aui_image_enlarge(unsigned long *argc,char **argv,char *sz_out_put);
void image_test_reg();



#ifdef __cplusplus
}
#endif

#endif

