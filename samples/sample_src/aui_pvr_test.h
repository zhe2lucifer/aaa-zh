/**@file
*    @brief 		UDI uart module public data struct and interface declear
*    @author		smith.shi
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_PVR_TEST_H
#define _AUI_PVR_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_pvr.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long pvr_pre(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long pvr_next(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_init(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_disk_attach(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_ts_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_tms_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_e_ts_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_e_tms_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_rec_pause(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_rec_resume(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_rec_change(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_rec_close(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_tms_open(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_play(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_pause(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_FF(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_FB(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_stop(unsigned long *argc,char **argv,char *sz_out_put);extern unsigned long test_pvr_play_close(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_play_change(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_get(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_set(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_register(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_pvr_unregister(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long ali_stop_live_play(void);
extern unsigned long ali_pvr_play_open(aui_hdl *player_handler,int record_index,char *filename,unsigned char start_mode);
extern unsigned long ali_pvr_play_close(aui_hdl player_handler);
extern unsigned long ali_pvr_record_close(aui_hdl *aui_pvr_handler);
extern unsigned long ali_recover_live_play(int vedio_pid,int auido_pid,unsigned int vedio_type,unsigned int audio_type, unsigned int pcr_pid);
extern unsigned long ali_pvr_record_open(aui_hdl *aui_pvr_handler,unsigned int dmx_id,unsigned int video_pid,unsigned int video_type,\
							unsigned int audio_pid_count,unsigned int *audio_pid,unsigned int *audio_type,unsigned int pcr_pid,\
							AUI_PVR_REC_MODE rec_type,int encrypt,int is_ca_mode,unsigned char *record_path);
void test_pvr_reg();


#ifdef __cplusplus
}
#endif

#endif //_AUI_PVR_TEST_H
