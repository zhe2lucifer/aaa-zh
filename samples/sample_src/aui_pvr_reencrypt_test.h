/*****************************************************************************
*    Copyright (c) 2017 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: aui_pvr_reencrypt_test.h
*
*    Description: 

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __AUI_PVR_REENCRYPT_TEST_H__
#define __AUI_PVR_REENCRYPT_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <osal/osal.h>
#include <api/libc/string.h>
#include <bus/sci/sci.h>
#include "aui_common.h"
#include "aui_help_print.h"
#include "encrypt_pvr.h"

#define GEN_CA_TS_PACKAGE_SIZE 188 //bytes

typedef enum sample_mode {

    /**
    Value to specify that user set key in record mode
    */
    REC_MODE,

    /**
    Value to specify that user set key in playback mode
    */
    PLAYBACK_MODE

} sample_mode;

void pvr_reencrypt_key_set(struct pvr_crypto_key* p_key);
void pvr_reencrypt_mode_init(int mode);
void pvr_sample_mode_set(int mode);

int pvr_reencrypt_module_init();
void pvr_reencrypt_module_deinit();

void pvr_reencrypt_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data);

void pvr_reencrypt_key_change();

void pvr_crypto_key_change_record_run(aui_hdl handler);
void pvr_crypto_key_change_record_stop();
void pvr_crypto_key_change_playback_run(aui_hdl handler);
void pvr_crypto_key_change_playbck_stop();


#ifdef __cplusplus
}
#endif

#endif

