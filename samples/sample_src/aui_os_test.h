/**@file
*    @brief 		UDI dmx module public data struct and interface declear
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_OS_TEST_H
#define _AUI_OS_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_os.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long test_os_task_create(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_delete(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_self_id(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_join(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_msgQRcv(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_msgQCreate(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_msgQDelete(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_msgQSnd(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_msgQRcv(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_sem_create(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_sem_delete(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_sem_wait(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_sem_release(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_event_create(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_event_delete(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_event_wait(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_event_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_mutex_create(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_mutex_delete(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_mutex_lock(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_mutex_unlock(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_mutex_try_lock(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_timer_create(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_timer_delete(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_timer_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_timer_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_timer_run(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_create_demo1(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_create_demo2(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_create_demo3(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_create_demo4(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_task_delete_demo(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_os_interrupt(unsigned long *argc,char **argv,char *sz_out_put);
extern void aui_load_tu_os();
#ifdef __cplusplus
}
#endif

#endif
