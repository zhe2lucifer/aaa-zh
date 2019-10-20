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
#ifndef __GEN_CA_PVR_H__
#define __GEN_CA_PVR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <sys_config.h>
#include <api/libpvr/lib_pvr_eng.h>

/*****************************************************************************
 * Function: aui_gen_ca_pvr_rec_config
 * Description:
 *    configure crypto for re-encrypt ts, and encrypt key
 * Input:
 *        Para 1: pvr_crypto_general_param *rec_param, a sturcutre pointer of pvr crypto information
 *
 * Output:
 *      Para 1: pvr_crypto_general_param *rec_param, a sturcutre pointer of pvr crypto information
 *
 * Returns:   0: Successful, Others:Failed
 * 
*****************************************************************************/
int aui_gen_ca_pvr_rec_config(pvr_crypto_general_param *rec_param);
/*****************************************************************************
 * Function: aui_gen_ca_pvr_playback_config
 * Description:
 *    configure crypto for decrypt ts and decrypt key
 * Input:
 *        Para 1: pvr_crypto_general_param *play_param, a sturcutre pointer of pvr crypto information
 *        Para 2: INT8 timeshift_flag, Indicate timeshift or not
 *
 * Output:
 *      Para 1: pvr_crypto_general_param *play_param, a sturcutre pointer of pvr crypto information
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int aui_gen_ca_pvr_playback_config(pvr_crypto_general_param *play_param,INT8 timeshift_flag);
/*****************************************************************************
 * Function:aui_gen_ca_pvr_rec_stop
 * Description:
 *    Release recording resource when stop record
 * Input:
 *        Para 1: pvr_crypto_general_param *rec_param, a sturcutre pointer of pvr crypto information
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int aui_gen_ca_pvr_rec_stop(pvr_crypto_general_param *rec_param);
/*****************************************************************************
 * Function: aui_gen_ca_pvr_playback_stop
 * Description:
 *    Release playback resource when stop playback
 * Input:
 *        Para 1: pvr_crypto_general_param *play_param, a sturcutre pointer of pvr crypto information
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int aui_gen_ca_pvr_playback_stop(pvr_crypto_general_param *play_param);
/*****************************************************************************
 * Function: aui_gen_ca_set_dsc_for_live_play
 * Description:
 *    Set DSC for live stream
 * Input:
 *        Para 1: UINT16 dmx_id, dmx id of live stream
 *        Para 2: UINT32 stream_id, stream id of live stream
 *
 * Output:
 *      None
 *
 * Returns:   None
 *
*****************************************************************************/
void aui_gen_ca_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id);
/*****************************************************************************
 * Function: aui_gen_ca_pvr_init
 * Description:
 *    Initialize gen_ca pvr module
 * Input:
 *        None
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int aui_gen_ca_pvr_init(void);
/*****************************************************************************
 * Function: aui_gen_ca_crypto_data
 * Description:
 *    Encrypt/Decrypt meta data
 * Input:
 *        Para 1: pvr_crypto_data_param *cp, a sturcutre pointer of pvr crypto data
 *
 * Output:
 *      None
 *
 * Returns:   0: Successful, Others:Failed
 *
*****************************************************************************/
int aui_gen_ca_crypto_data(pvr_crypto_data_param *cp);
/*****************************************************************************
*    @brief         generate random numbers as encrypted keys
*    @author        Alan.Song
*    @date          2016-03-21
*    @param[in]     key_param   store key number, key length(bit), key address
*    @return        successful -- 0, failed -- -1
*    @note
*
*****************************************************************************/
int aui_gen_ca_pvr_generate_keys(pvr_crypto_key_param *key_param);
/**
*    @brief         remove the invalid and duplicate pids
*    @author        Alan.Song
*    @date          2016-03-21
*    @param[in]     pid_list    pid list to be checked
*    @param[in]     pid_num     pid number in pid list
*    @return        valid pid number
*    @note
*
*/
UINT16 aui_gen_ca_pvr_check_reencrypt_pids(UINT16 *pid_list, UINT16 pid_num);
/**
*    @brief         set re-encrypt pids to pid_list
*    @author        Alan.Song
*    @date          2016-03-21
*    @param[in]     pid_info        pids info
*    @param[out]    pid_list        pids to be re-encrypted
*    @param[in]     pid_list_size   max size of pid_list
*    @return        valid pid number
*    @note
*
*/
UINT16 aui_gen_ca_pvr_set_reencrypt_pids(struct pvr_pid_info *pid_info,
									  UINT16 *pid_list, UINT16 pid_list_size);



#ifdef __cplusplus
}
#endif

#endif /* __C200A_PVR_H__ */

