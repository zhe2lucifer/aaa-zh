/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved 
 *
 *  @file               aui_misc_test.h
 *  @brief              head file of misc test module
 *
 *  @version            1.0
 *  @date               05/19/2014 02:05:08 PM
 *  @revision           none
 *
 *  @author             Stephen Xiao <stephen.xiao@alitech.com>
 */


#ifndef _AUI_MISC_TEST_H
#define _AUI_MISC_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_misc.h>
#include "aui_test_app.h"
#ifdef AUI_TDS 
#include <api/libc/printf.h>
#include <api/libc/string.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

void misc_test_reg(void);

unsigned long test_misc_pm_standby(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_pmu_standby(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_sys_reboot(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_wakeup_timer(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_set_pm_standby(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_set_pmu_standby(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_set_wakeup_time(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_set_panel_display(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_wakeup_timer(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_misc_help(unsigned long *argc, char **argv, char *sz_out_put);

#ifdef __cplusplus
	}
#endif
	
#endif


