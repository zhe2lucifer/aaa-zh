/*****************************************************************************
*    Copyright (c) 2017 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: Encrypt_pvr.h
*
*    Description: 

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#ifndef __ENCRYPT_PVR_H__
#define __ENCRYPT_PVR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "aui_common.h"
#include "aui_pvr.h"
#include "aui_kl.h"
#include "aui_dsc.h"
#include "aui_dmx.h"


extern int g_is_pvr_reencrypt_status_inited;
struct pvr_crypto_key
{

    /**
      *    @brief               User can set this value to choose key source.
      *
      *    @how to set      AUI_KL_KEY_SKIP_LEVEL   -> key from S-RAM.
      *                           AUI_KL_KEY_ONE_LEVEL    -> key from KL and level 1;
      *                           AUI_KL_KEY_TWO_LEVEL   -> key from KL and level 2;
      *                           AUI_KL_KEY_THREE_LEVEL -> key from KL and level 3;
      *                           AUI_KL_KEY_FIVE_LEVEL    -> key from KL and level 5.
      */ 
    aui_kl_crypt_key_level kl_level;

    /**
      *    @brief              Member as an Index that refers to the KL root key position.
      *
      *    @how to set      Users can set it according to specific need.
      */ 
    aui_kl_root_key_idx kl_root_key_idx;

    /**
      *    @brief              User can set key KL algo to generate keys if key from KL.
      *
      *    @how to set      AUI_KL_ALGO_TDES ->  Triple Data Encryption Standard (DES)
      *                           AUI_KL_ALGO_AES   ->  Advanced Encryption Standard (AES)
      */ 
    aui_kl_algo kl_algo;

    /**
      *    @brief              User can set key parity to use this aui interface instead.
      *                           In this sample case, the parity is set up and users does not require to modify it.
      *
      *    @how to set      AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE  ->  record mode & even key
      *                           AUI_DSC_PARITY_MODE_ODD_PARITY_MODE   ->  record mode & odd key
      *                           AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 2  -> playback mode & even key 
      *                           AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 1  -> playback mode & odd key 
      */ 
    aui_dsc_parity_mode parity;

    /**
      *    @brief              User can set key DSC algo here.
      *
      *    @how to set      AUI_DSC_ALGO_DES   ->  Data Encryption Standard (DES)
      *                           AUI_DSC_ALGO_AES   ->  Advanced Encryption Standard (AES)
      *                           AUI_DSC_ALGO_TDES  ->  Triple Data Encryption Standard (TDES)
      *                           AUI_DSC_ALGO_CSA   ->  Common Scramble Algorithm (CSA)
      */
    aui_dsc_algo dsc_algo;

    /**
      *    @brief              User can set key DSC specific algo mode, but just when dsc_algo 
      *                           is set DES/TDES and AES.
      *
      *    @how to set      choose the mode user's want from #aui_dsc_work_mode.
      */     
    aui_dsc_work_mode dsc_mode;
    
    /**
      *    @brief              User can set DSC block size, the value should be set only in BLOCK MODE.
      *    
      *    @unit               bytes
      *
      *    @how to set     For block encryption, the default block size is 188 x 256 bytes.
      *                          (the block size must be set a multiple of 188 bytes).
      */  
    unsigned long dsc_block_size;
    
    /**
      *    @brief              User set the key length.
      *    
      *    @unit               bytes
      *
      *    @how to set     choose from 8 or 16.
      */ 
    int key_length;

    /**
      *    @brief              User set the key value & key iv below.
      *    
      *    @attention       Number of protect key should match the kl_level when set.
      */ 
    unsigned char* p_key;
    unsigned char* p_iv;
    unsigned char* p_default_iv;
    unsigned char* p_protected_keys;
    
};


/**
*    @brief       Pvr Key Re-encryption Handlers
*
*                    h_dsc: handler of DSC  
*                    h_kl: handler of KL 
*                    h_dmx: handler of DMX  
*/
struct pvr_set_crypto_key_hd
{
    aui_hdl h_dsc;
    
    aui_hdl h_kl;

    aui_hdl h_dmx;
};

struct pvr_crypto_pids
{
    int count;
    unsigned short int* p_pids;
};


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             set or modify pvr crypto key for encrypt or decrypt.
*
*    @call time       This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          It is called just after the user-set stage.
*    
*    @param[in]        *p_key         =     p_key->kl_level > 0, use kl mode, otherwise use clear key mode
*                                                      p_key->p_iv, if NULL, use default iv(all zero)
*
*    @param[in/out]  *p_key_hd    =     p_key_hd->h_dmx must not NULL;
*                                                      p_key_hd->h_kl, if NULL, dsc use clear key mode, otherwise use kl mode;
*                                                      p_key_hd->h_dsc, if NULL, will invoke aui_dsc_open(), and return the dsc_hd by this para.
*                                                      (param[out] because KL&DSC open when init)
*
*    @param[in]         *p_pids       =     if not NULL, will record pids from this para.
*    
*    @return         AUI_RTN_SUCCESS      = Getting of the desired set of URI information.
*    @return         AUI_RTN_FAIL            = The input parameters (i.e. [in]) is invalid or function which called return failed
*/

AUI_RTN_CODE pvr_crypto_key_set(const struct pvr_crypto_key* p_key, struct pvr_set_crypto_key_hd* p_key_hd, struct pvr_crypto_pids* p_pids);

/**
*    @brief      when error is found, recommend for clear pvr encrpty/edcrypt status and resource.  
*/
void pvr_crypto_key_free();

AUI_RTN_CODE pvr_encrypt_start(aui_pvr_crypto_general_param *p_param);
AUI_RTN_CODE pvr_encrypt_stop(aui_pvr_crypto_general_param *p_param);
AUI_RTN_CODE pvr_decrypt_start(aui_pvr_crypto_general_param *p_param);
AUI_RTN_CODE pvr_decrypt_stop(aui_pvr_crypto_general_param *p_param);

AUI_RTN_CODE pvr_crypto_pids_set(aui_pvr_crypto_pids_param *p_pids_param);



#ifdef __cplusplus
}
#endif

#endif /* __C200A_PVR_H__ */

