
#ifndef _AUI_QT_PVR_BOARD_TEST_H
#define _AUI_QT_PVR_BOARD_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_pvr.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
unsigned long qt_board_test_pvr_test_record(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long qt_board_test_pvr_test_timeshift(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long sample_config_tsi_route_for_2nd_pvr_rec(unsigned long *argc,char **argv,char *sz_out_put);
#ifdef __cplusplus
}
#endif

#endif //_AUI_PVR_TEST_H
