#ifndef __AUI_PIP_TEST_LIVE_H_
#define __AUI_PIP_TEST_LIVE_H_

#include "aui_test_stream.h"

int test_aui_pip_init_para_for_nim(unsigned long *argc,char **argv, 
				unsigned short dis_layer, unsigned short decv_id, struct ali_app_modules_init_para *init_para);
int test_aui_pip_play_nim(struct ali_app_modules_init_para* init_para, struct ali_aui_hdls *pip_hdls,
	int play_audio, int first_play);
int test_aui_pip_stop_nim(struct ali_aui_hdls *handles);

int test_aui_pip_start_dmx_audio(aui_hdl *deca_hdl, int audio_type, aui_hdl dmx_hdl, 
	unsigned short audio_pid, unsigned short audio_desc_pid);

int test_aui_pip_stop_dmx_audio(aui_hdl *deca_hdl, aui_hdl dmx_hdl);

#endif
