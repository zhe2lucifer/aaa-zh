/**
 * @file
 * @brief      Function used in AUI DECV for internal using
 * 
 * @author     Wendy He
 * @date       2015-12-02
 * @version    1.0.0
 * @note       Copyright(C) ALi Corporation. All rights reserved.
 */

#ifndef __AUI_DECV_PRIV_H__
#define __AUI_DECV_PRIV_H__
#include <aui_common.h>
#include <aui_decv.h>

#include <alislvdec.h>
#include <alipltfretcode.h>
#ifdef __cplusplus
extern "C"
{
#endif
enum vbv_buf_handle_flag {
	VBV_BUF_NOT_RESET, //do not reset vbv buffer, let app inject data base on current vbv buffer.
	VBV_BUF_RESET, //reset vbv buffer
};
AUI_RTN_CODE aui_decv_buffer_reset(void *pv_handle_decv, unsigned int reset_buffer);
AUI_RTN_CODE aui_decv_trickmode_set(void *pv_handle_decv, enum aui_playback_speed speed, 
	enum aui_playback_direction direction, int mode);
AUI_RTN_CODE aui_decv_trickinfo_get(void *pv_handle_decv, int *if_cb_err, int *if_frame_dis);

AUI_RTN_CODE aui_decv_get_sl_handle(aui_hdl pv_hdl_decv, alisl_handle * pv_hdl_sl_vdec);
AUI_RTN_CODE aui_decv_get_video_id(aui_hdl pv_hdl_decv, unsigned char *video_id);
AUI_RTN_CODE aui_decv_get_current_opened_sl_handle(alisl_handle * pv_hdl_sl_vdec);
AUI_RTN_CODE aui_decv_set_see_dmx_id(aui_hdl pv_hdl_decv, int see_dmx_id);
AUI_RTN_CODE aui_decv_get_see_dmx_id(aui_hdl pv_hdl_decv, int *see_dmx_id);
#ifdef __cplusplus
}
#endif
#endif
