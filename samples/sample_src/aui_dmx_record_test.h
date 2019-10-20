/**@file
*    @brief           AUI dmx module record public data struct and interface declear
*    @author        nick.li
*    @date           2016-8-15
*    @version       1.0.0
*    @note           ali corp. all rights reserved. 2016~2999 copyright (C)
*/
#ifndef __AUI_DMX_RECORD_TEST_H_
#define __AUI_DMX_RECORD_TEST_H__

/****************************INCLUDE FILE************************************/
#include <aui_dmx.h>
#include "aui_test_app.h"


/****************************GLOBAL MACRO************************************/
#define TS_PACKAGE_SIZE     188
#define RECORD_BUFFER_LEN   (384 * TS_PACKAGE_SIZE)


/****************************GLOBAL TYPE************************************/
// The attribute of record TS data PID
struct attr_record_ts_data_pid{
    unsigned short *us_pid;   // All of the PID , in all received TS package
    unsigned short *us_pid_num;  // These different kinds of PID, in all received TS package    
    unsigned short *us_pid_diff;  // All of the different PID, in all received TS package
    unsigned long ul_pid_diff_count;  // The count of different PID
    unsigned short us_pid_diff_flag;  //The different PID flag
    unsigned short *us_pid_continuity_counter;  // All of the PID corresponding continuity_counter, in all received TS package
    unsigned short *us_pid_diff_continuity_counter;  // All of the different PID corresponding continuity_counter, in all received TS package
};


/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

unsigned long sample_dmx_record_ts_data(unsigned long *argc,char **argv,char *sz_out_put);


#ifdef __cplusplus
}
#endif

#endif

