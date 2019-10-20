/**
<!--
Notice:             This file mainly contains Doxygen-style comments to generate
                    the HTML help file (.chm) provided by ALi Corporation to
                    give ALi Customers a quick support for using ALi AUI API
                    Interface.\n
                    Furthermore, the comments have been wrapped at about 80 bytes
                    in order to avoid the horizontal scrolling when splitting
                    the display in two part for coding, testing, debugging
                    convenience
Current ALi author: Davy.Wu
Last update:        2017.02.25
-->

@file  aui_ca_pvr.h

@brief Conditional Access Personal Video Recorder (CA-PVR) Module

       <b> Conditional Access Personal Video Recorder (CA-PVR) Module </b> is
       used to process <b> Transport Stream (TS) data </b> and @a pure data
       in memory.\n
       The main tasks of CA-PVR Module are summarized below:
       - Get the ID of Descrambler (DSC) Device.
       - Set/Get pids for DSC Device.
       - Generate the random keys for performing the re-encryption.
       - Perform the HMAC calculation for meta data protection.
       - Encryption/Decryption of meta data protection.

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Personal Video Record (PVR) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef __AUI_CA_PVR__

#define __AUI_CA_PVR__

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Type List*******************************/

///@coding

/**
Function pointer for getting the handle of the DSC Device in TS Data Mode where:
- The input parameter @b program_id is the ID of the operating channel.
- The input/output parameter @b p_dsc_handler is the pointer to the handle of
  the DSC device.
- The return value can be
  - @b 0 = Getting of the the handle of DSC device performed successfully
  - <b> Other Values </b> = Getting of the handle of the DSC device failed
    cause either input/output parameter are invalid or the ID of the channel
    has not found in the DSC Module
*/
typedef unsigned short (*aui_pvr_get_ts_dsc_handle_callback) (

    unsigned short program_id,

    aui_hdl *p_dsc_handler

    );

/**
Function pointer for getting the handle of DSC Device in Pure Data Mode where:
- The input parameter @b program_id is the ID of the operating channel.
- The input/output parameter @b p_dsc_handler is the pointer to the handle of
  the DSC device.
- The return value can be
  - @b 0 = Getting of the the handle of DSC device performed successfully
  - <b> Other Values </b> = Getting of the handle of the DSC device failed
    cause either input/output parameter are invalid or the ID of the channel
    has not found in the DSC Module
*/
typedef unsigned short (*aui_pvr_get_pure_data_dsc_handle_callback) (

    unsigned short program_id,

    aui_hdl *p_dsc_handler

    );

///@endcoding

typedef void (*aui_pvr_c200a_callback_fun) (

    void *p_user_data,

    void *p_enc_info,

    void *p_dec_info

    );

/**
@brief @htmlonly <div class="details">
       Struct of the <b> CA-PVR Module </b> to get the handle of the DSC Device
       to perform the re-encryption for PVR
       </div> @endhtmlonly

       Struct to get the handle of the DSC Device by the channel ID to perform
       the re-encryption for PVR
*/
typedef struct aui_ca_pvr_callback {

    /**
    Member as the callback function used to get the handle of the DSC Device
    for decrypting TS stream.
    When re-encrypting TS stream, the DSC device to decrypt the stream is opened
    and configured out side of PVR module.
    AUI PVR use this callback to get the DSC for decrypting the recording stream
    */
    aui_pvr_get_ts_dsc_handle_callback fp_ts_callback;

    /**
    This member is not used anymore
    */
    aui_pvr_get_pure_data_dsc_handle_callback fp_pure_data_callback;


    /**
    Member as pointer to <em> p_user_data </em> for the callback function with
    the identifier #callback_fun

    @note    This member is required for Nagra projects only
    */
    void *user_data;

    /**
    Member as the callback function used to get the encryption and decryption
    info from Nagra PRM module.

    @note    This member is required for Nagra projects only
    */
    aui_pvr_c200a_callback_fun callback_fun;

} aui_ca_pvr_callback;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> CA-PVR Module </b> used for configuring CA-PVR Module
       </div> @endhtmlonly

       Struct used to configure the CA-PVR Module
*/
typedef struct aui_ca_pvr_config {

    /**
    Member to specify different encryption modes for PVR re-encryption
    */
    int special_mode;

} aui_ca_pvr_config, *p_aui_ca_pvr_config;


#ifdef __cplusplus

extern "C" {

#endif

/*****************************Global Function List*****************************/

/**
@brief          Function used to initialize the CA-PVR Module before its opening
                and starting

@warning        This function @a must be used before starting to record or
                playback for the re-encryption

@return         @b AUI_RTN_SUCCESS  = CA-PVR Module opened successfully then
                                      user can start to register the callback
                                      function and perform the re-encryption
@return         @b Other_Values     = Initializing of CA-PVR failed for some
                                      reasons
*/
AUI_RTN_CODE aui_ca_pvr_init (

    void

    );

/**
@brief          Function used to initialize the CA-PVR Module with configuration
                (i.e. selection of the CA system for PVR re-encryption)

@param[in]      p_config             = Pointer to the struct which collects
                                       the configuration of the CA-PVR Module

@return         @b AUI_RTN_SUCCESS   = Initializing of the CA-PVR Module
                                       performed successfully

@return         @b AUI_RTN_EINVAL    = The parameter (i.e. [in]) is invalid

@return         @b Other_Values      = Initializing of CA-PVR failed for some
                                       reasons
*/
AUI_RTN_CODE aui_ca_pvr_init_ext (

    aui_ca_pvr_config *p_config

    );

/**
@brief          Function used to register the DSC callback function for CA-PVR
                Module

@warning        This function @a must be used after performing the initialization
                by the function #aui_ca_pvr_init

@param[in]      p_aui_ca_pvr_callback   = Pointer to the struct which collects
                                          the callback functions used to get the
                                          handle of DSC Device

@return         @b AUI_RTN_SUCCESS      = The callback function is registered
                                          successfully
@return         @b AUI_RTN_EINVAL       = The parameter (i.e. [in]) is invalid
@return         @b Other_Values         = Registering of the callback function
                                          failed for some reasons
*/
unsigned short aui_ca_register_callback (

    aui_ca_pvr_callback *p_aui_ca_pvr_callback

    );

/**
@brief          Function used to get the stream ID of DSC Device for <b> TS data
                decryption </b> in the operating channel.

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module

@param[in]      program_id  = The ID of the operating channel

@return         @b 1. The stream ID of DSC Device for TS data decryption in the
                      operating channel
@return         @b 2. <b> INVALID_DSC_STREAM_ID (0xFF) </b> = Getting of the
                      stream ID of DSC Device failed for some reasons
*/
unsigned int aui_ca_pvr_get_ts_stream_id (

    unsigned int program_id

    );

/**
@brief          Function used to get the CSA Device ID of DSC Device for TS data
                decryption in the operating channel

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module

@param[in]      program_id  = The ID of the operating channel

@return         @b 1. The CSA Device ID of DSC Device for TS data decryption in
                      the operating channel
@return         @b 2. <b> INVALID_DEVICE_ID (0xff) </b> = Getting of the CSA
                      Device ID of DSC Device failed for some reasons
*/
unsigned int aui_ca_pvr_get_ts_csa_device_id (

    unsigned int program_id

    );

/**
@brief          Function used to get the stream ID of DSC Device for <b> Pure
                data </b> decryption in the operating channel

@warning        This function can be used @a only after performing the
                initialization of CA-PVR Module

@param[in]      program_id  = The ID of the operating channel

@return         @b 1. The stream ID of DSC Device for Pure data decryption in
                      the operating channel
@return         @b 2. <b> INVALID_DSC_STREAM_ID (0xFF) </b> = Getting of the
                      stream ID of DSC Device failed for some reasons
*/
unsigned int aui_ca_pvr_get_pure_data_stream_id (

    unsigned int program_id

    );

/**
@brief          Function used to set the TS stream PID list for re-encryption
                in the operating channel

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module.

@param[in]      pus_pid_list        = The pointer to the TS stream PID list
@param[in]      us_pid_number       = The total amount of TS stream PID in
                                      @b pid_list.
                                      - @b Caution: If @b pid_number = 0 then
                                           all information in DSC Module of the
                                           operating channel will be reset
@param[in]      prog_id             = The ID of the operating channel

@return         @b 1. The total amount of TS stream PID has been set into the
                      DSC Module in one time.
@return         @b 2. @b 0 when
                      - Either if the @b pid_number = 0 then all information
                        in DSC Module of the operating channel will be reset
                      - Or the channel ID has not been found.
*/
unsigned short aui_ca_set_dsc_pid_multi_des (

    unsigned short *pus_pid_list,

    unsigned short us_pid_number,

    unsigned int prog_id

    );

/**
@brief          Function used to get the TS stream PID list for re-encryption
                in the operating channel

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module

@param[in,out]  pus_pid_list        = The pointer to the TS stream PID list
@param[in]      us_pid_number       = The maximum number of the PID the
                                      @b pid_list in the DSC Module of the
                                      operating channel
@param[in]      prog_id             = The ID of the operating channel

@return         @b 1. The total amount of TS stream PID has been gotten from
                      the DSC Module in one time
@return         @b 2. @b 0 = The channel ID has not been found
*/
unsigned short aui_ca_get_dsc_pid_multi_des (

    unsigned short *pus_pid_list,

    unsigned short us_pid_number,

    unsigned int prog_id

    );

///@endcond

/**
@brief          This function uses the HMAC arithmetic to verify the meta data.
                With the help of this function
                - Before the encryption of PVR meta data while recording, the
                  HMAC of the PVR meta data will be calculated and saved
                - After the decryption of PVR meta data while playback, the
                  HMAC of the PVR meta data will be calculated and compared
                  with the saved one.

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module

@param[in]      puc_input           = Pointer to the meta data memory address
@param[in]      ul_length           = The length of the meta data memory address
@param[in]      puc_key             = Pointer to the HMAC Key

@param[out]     puc_output          = The HMAC result of the input buffer
                                      - @b Caution: The length of the output is
                                           fixed in 32 bytes

@return         @b AUI_RTN_SUCCESS  = The HMAC result is calculated successfully
@return         @b AUI_RTN_FAIL     = The calculation failed for some reasons
*/
AUI_RTN_CODE aui_ca_calculate_hmac (

    unsigned char *puc_input,

    unsigned long ul_length,

    unsigned char *puc_output,

    unsigned char *puc_key

    );

/**
@brief          Function used to generate a number of random keys.\n
                While start recording, this function will be called by PVR Module
                then the random keys can be used for re-encryption recording

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module

@param[in]      key_len             = The length of a key in bit
@param[in]      key_num             = The total amount of the random keys to be
                                      generated
@param[out]     puc_key_ptr         = Pointer to the keys buffer, where the keys
                                      generated have been saved

@return         @b AUI_RTN_SUCCESS  = The keys have been generated successfully
@return         @b AUI_RTN_FAIL     = Generating of the keys failed for some
                                      reasons
*/
AUI_RTN_CODE aui_ca_pvr_generate_keys (

    unsigned int key_len,

    unsigned int key_num,

    unsigned char *puc_key_ptr

    );

/**
@brief          Function used to perform the encryption/decryption of the PVR
                meta data. With the help of this function
                - Before saving PVR meta data into the hard disk while
                  recording, the PVR meta data could be encrypted
                - After getting PVR meta data from the hard disk while
                  playback, the PVR meta data could be decrypted

@warning        Currently, the input data could @a only be processed with TDES
                algorithm in CBC mode

@warning        This function can be used @a only after performing the
                initialization and registration of CA-PVR Module

@param[out]     puc_out_data        = Pointer to the post processed data

@param[in]      puc_in_data         = Pointer to the data to be processed
@param[in]      data_len            = Length (in @a bytes unit) of @b in_data
@param[in]      puc_key_ptr         = Pointer to the key for encryption/decryption
@param[in]      puc_iv_ptr          = Pointer to Initial Vector (IV) for some
                                      specific algorithms
@param[in]      key_len             = Length (in @a bit unit) of the key
@param[in]      uc_crypto_mode      = Reserved for feature used, user can ignore
                                      currently
@param[in]      uc_encrypt          = Flag to indicate encryption/decryption,
                                      in particular:
                                      - @b 1 = Encryption
                                      - @b 0 = Decryption

@return         @b AUI_RTN_SUCCESS  = Input meta data processed successfully
@return         @b AUI_RTN_FAIL     = Processing of the input meta data failed
                                      for some reasons

@note           @b 1. Length of @b out_data @a must be the same with @b in_data
@note           @b 2. Length of @b iv_ptr is fixed to 16 bytes
*/
AUI_RTN_CODE aui_ca_pvr_crypto (

    unsigned char *puc_out_data,

    unsigned char *puc_in_data,

    unsigned int data_len,

    unsigned char *puc_key_ptr,

    unsigned char *puc_iv_ptr,

    unsigned int key_len,

    unsigned char uc_crypto_mode,

    unsigned char uc_encrypt

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                 START                                     */
/*****************************************************************************/

///@cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_ca_pvr_callback_t aui_ca_pvr_callback

#define fp_get_pure_data_dsc_handle_callback aui_pvr_get_pure_data_dsc_handle_callback

#define fp_get_ts_dsc_handle_callback aui_pvr_get_ts_dsc_handle_callback

#endif

///@endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */

