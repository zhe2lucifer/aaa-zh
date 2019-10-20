/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: c200a_pvr.h
*
*    Description: This file contains the callback functions for re-encryption of transport stream
                 and encryption and HMAC of related metadata
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __C200A_PVR_H__
#define __C200A_PVR_H__

#include <sys_config.h>
#include <api/libpvr/lib_pvr_eng.h>
#include "aui_common.h"
#include <alidefinition/adf_pvr.h>

int aui_c200a_pvr_generate_keys(pvr_crypto_key_param *key_param);


void aui_c200a_set_dsc_for_live_play(UINT16 dmx_id);

int aui_c200a_pvr_init(void);
int aui_c200a_pvr_vsc_init(void);

int aui_c200a_crypto_data(pvr_crypto_data_param *cp);
#if 0 ///this function is no used.Because callback is abolished.custermor process all the function in upper layer.nnn
int aui_c200a_pvr_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data);
int c200a_pvr_rec_config(pvr_crypto_general_param *rec_param);
int c200a_pvr_block_decrypt(UINT32 handle, UINT8 *buffer, UINT32 length, UINT32 block_idx);
#endif

#endif /* __C200A_PVR_H__ */
