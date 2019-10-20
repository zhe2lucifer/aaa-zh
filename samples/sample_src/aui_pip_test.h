#ifndef __AUI_PIP_TEST_H_
#define __AUI_PIP_TEST_H_

#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_stc.h>

int test_aui_pip_set_avsync(aui_hdl *p_hdl_stc, 
	enum aui_av_data_type data_type, aui_stc_avsync_mode sync_mode, 
	aui_hdl decv_hdl);

int test_aui_pip_set_dis_rect(enum aui_dis_layer layer, 
	aui_dis_zoom_rect* src_rect, aui_dis_zoom_rect* dst_rect);

int test_aui_pip_swap_video(aui_hdl decv_main, aui_hdl decv_pip);

void test_pip_reg();

#endif

