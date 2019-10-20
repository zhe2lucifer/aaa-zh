/**
 * @file
 * @brief      Function used in AUI DECV for internal using
 * 
 * @author     Wendy He
 * @date       2015-12-02
 * @version    1.0.0
 * @note       Copyright(C) ALi Corporation. All rights reserved.
 */

#ifndef __AUI_DECA_PRIV_H__
#define __AUI_DECA_PRIV_H__

#include <alipltfretcode.h>

AUI_RTN_CODE aui_deca_get_sl_handle(aui_hdl pv_hdl_deca, alisl_handle * pv_hdl_sl_snd);
AUI_RTN_CODE aui_deca_get_audio_id(aui_hdl pv_hdl_deca, unsigned char *audio_id);

#endif
