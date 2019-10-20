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
Current ALi Author: Frankly.Pan, Wesley.He
Last update:        2017.02.25
-->

@file   aui_conaxvsc.h

@brief  Conax Virtual Smart Card (Conax VSC) Module (for CONAX's users @a only)

        <b> Conax Virtual Smart Card (Conax VSC) Module </b> is used for
        exchanging commands and negotiating a generation of keys between the
        interface device and the Conax Virtual Smart Card (Conax VSC).\n\n

        Conax Virtual Smart Card Module mainly provides the following functions:
        - Load storage data configuration to Virtual Smart Card
        - Initialize the Virtual Smart Card
        - Fetch store data configuration from Virtual Smart Card when available
        - Send/Receive data to/from the Virtual Smart Card
        - Set encrypted Code Word (DECW) for stream decryption into chipset
        - Set PVR re-encryption key for stream re-encryption into chipset
        - Decrypt encrypted Universal Key (UK) into chipset

@note   This module is for CONAX's users @a only

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
*/

#ifndef _AUI_CONAXVSC_H_

#define _AUI_CONAXVSC_H_

/*************************Included Header File List***************************/

#include "aui_common.h"

#ifdef __cplusplus

extern "C"

{

#endif

/*****************************Global Macro List*******************************/

/**
Macro to specify the <b> version number </b> of Conax Virtual Smart Card Module,
which is a hexadecimal number that consist of three (3) reading parts (sorted
from left to right) as exemplified below:\n\n

Version number 0x00010100, where
- 0x0001 = <b> main version </b>
- 01 = <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_CONAXVSC   (0X00010100)

/**
Macro to specify the maximum length (in @a byte unit) of a @b Command used
for the <b> Virtual Smart Card </b>
*/
#define AUI_CONAXVSC_CMD_MAX_LEN  (4096)

/**
Macro to specify the length (in @a byte unit) of the <b> Store Data </b>
*/
#define AUI_CONAXVSC_STORE_DATA_LEN  (0x10000)

/**
Macro to specify the length (in @a byte unit) of the <b> Store Data Key </b>
*/
#define AUI_CONAXVSC_STORE_KEY_LEN  (16)

/**
Macro to specify the length (in @a byte unit) of the <b> Store Data Hash </b>
*/
#define AUI_CONAXVSC_STORE_HASH_LEN  (32)

/**
Macro to specify the length (in @a byte unit) of the <b> Double Encrypted
Code Word (DECW) </b>
*/
#define AUI_CONAXVSC_DECW_LEN  (16)

/**
Macro to specify the length (in @a byte unit) of the <b> Re-Encryptrion Key </b>
*/
#define AUI_CONAXVSC_EN_KEY_LEN  (16)

/**
Macro to specify the length (in @a byte unit) of the <b> Universal Key </b>
*/
#define AUI_CONAXVSC_UK_LEN  (48)

/******************************Global Type List*******************************/

/**
Enum to specify the parity of the output key generates by KL device as DSC
Control Word (CW)
*/
typedef enum aui_conaxvsc_key_parity {

    /**
    Value to indicate <b> odd key parity </b>
    */
    AUI_CONAXVSC_KEY_ODD = (1<<0),

    /**
    Value to indicate <b> even key parity </b>
    */
    AUI_CONAXVSC_KEY_EVEN = (1<<1),

    /**
    Value to indicate an <b> Invalid Output Key Attribute </b> used for checking
    input and output parameters
    */
    AUI_CONAXVSC_KEY_ATTR_NB

} aui_conaxvsc_key_parity;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b>
        to specify the store data configuration attributes
        </div> @endhtmlonly

        Struct to specify the store data configuration attributes
*/
typedef struct aui_conaxvsc_store {

    /**
    Member to specify the store data field
    */
    unsigned char p_uc_data[AUI_CONAXVSC_STORE_DATA_LEN];

    /**
    Member to specify the key used to decrypt the store data field
    */
    unsigned char p_uc_key[AUI_CONAXVSC_STORE_KEY_LEN];

    /**
    Member to specify the hash value of the store data field
    */
    unsigned char p_uc_hash[AUI_CONAXVSC_STORE_HASH_LEN];

} aui_conaxvsc_store, *aui_pconaxvsc_store;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b> to
        specify the store data buffer attributes for exchanging commands
        </div> @endhtmlonly

        Struct to specify the store data buffer attributes for exchanging
        commands

@note   The size of this struct is bigger than 4KB, please allocate it from the
        system memory heap instead of the stack heap
*/
typedef struct aui_conaxvsc_tx_buf {

    /**
    Member to specify the store data field
    */
    unsigned char p_uc_data[AUI_CONAXVSC_CMD_MAX_LEN];

    /**
    Member to specify the number of valid bytes in the store data field
    */
    int n_size;

} aui_conaxvsc_tx_buf, *aui_pconaxvsc_tx_buf;

/**
Function pointer to specify the type of callback function which is
- Registered with the function #aui_conaxvsc_register_store_callback
- Filled in the struct #aui_conaxvsc_store_callback_attr
- Intended to provide an application the new available store data
  configuration from the Conax Virtual Smart Card
*/
typedef void (*aui_conaxvsc_store_fun_cb) (

    aui_conaxvsc_store *p_vsc_store,

    void *pv_user_data

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b>
        to specify the configuration attributes
        </div> @endhtmlonly

        Struct to specify the configuration attributes of the Conax Virtual
        Smart Card
*/
typedef struct aui_conaxvsc_attr {

    /**
    Member as an <b> Index </b> which integer values (from 0) refer to
    different opened VSC Devices
    */
    unsigned char uc_dev_idx;

    /**
    Member to specify the <b> Store Data Configuration </b> to be loaded to the
    Conax Virtual Smart Card before its initialization

    @note   This member can be set to NULL to skip the loading of store data
            configuration. This operation can be performed, for example, during
            the chip personalization since this data may still be not available or
            the chip has been previously configured and the application wants
            to re-configure again.
    */
    aui_conaxvsc_store *p_vsc_store;

} aui_conaxvsc_attr, *aui_pconaxvsc_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b>
        to specify the callback function attributes to provide an application
        the new store data configuration
        </div> @endhtmlonly

        Struct to specify the attributes of a callback function intended to
        provide an application the new available store data configuration from
        Conax Virtual Smart Card
*/
typedef struct aui_conaxvsc_store_callback_attr {

    /**
    Member to specify the <b> User Data </b> to be passed to the callback
    function
    */
    void *pv_user_data;

    /**
    Member to specify the callback function as per comment of the function
    pointer #aui_conaxvsc_store_fun_cb
    */
    aui_conaxvsc_store_fun_cb p_fn_conaxvsc_store_cb;

} aui_conaxvsc_store_callback_attr, *aui_pconaxvsc_store_callback_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b>
        to specify the Double Encrypted Code Word (DECW) attributes
        </div> @endhtmlonly

        Struct to specify the attributes for decrypting the Double Encrypted
        Code Word (DECW) and loading to the chipset for stream decryption
*/
typedef struct aui_conaxvsc_decw_attr {

    /**
    Member to specify the handle of the destination key ladder
    */
    aui_hdl kl_handle;

    /**
    Member to specify the key ID used to decrypt the encrypted code word
    (obtained from a previous ECM data exchange)
    */
    unsigned short key_id;

    /**
    Member to specify the encrypted code word (obtained from a previous ECM
    data exchange)
    */
    unsigned char decw[AUI_CONAXVSC_DECW_LEN];

    /**
    Member to specify the parity of the output key generates by KL device as DSC
    Control Word (CW), as defined in the enum #aui_conaxvsc_key_parity
    */
    aui_conaxvsc_key_parity key_parity;

} aui_conaxvsc_decw_attr, *aui_pconaxvsc_decw_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b>
        to specify the PVR re-encryption key attributes
        </div> @endhtmlonly

        Struct to specify the PVR attributes for decrypting recording keys
        and loading to the chipset for stream re-encryption
*/
typedef struct aui_conaxvsc_en_key_attr {

    /**
    Member to specify the handle of the destination key ladder
    */
    aui_hdl kl_handle;

    /**
    Member as an array to contain encrypted keys for PVR re-encrypting
    */
    unsigned char en_key[AUI_CONAXVSC_EN_KEY_LEN];

    /**
    Member to specify the parity of the output key generates by KL device as DSC
    Control Word (CW), as defined in the enum #aui_conaxvsc_key_parity
    */
    aui_conaxvsc_key_parity key_parity;

} aui_conaxvsc_en_key_attr, *aui_pconaxvsc_en_key_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Conax Virtual Smart Card (Conax VSC) Module </b>
        to specify the universal key attributes
        </div> @endhtmlonly

        Struct to specify the attributes for decrypting universal keys and
        loading to the chipset
*/
typedef struct aui_conaxvsc_uk_attr {

    /**
    Member to specify the handle of the destination key ladder
    */
    aui_hdl kl_handle;

    /**
    Member as an array to contain encrypted keys for PVR re-encrypting
    */
    unsigned char uk[AUI_CONAXVSC_UK_LEN];

} aui_conaxvsc_uk_attr, *aui_pconaxvsc_uk_attr;

/*****************************Global Function List*****************************/

/**
@brief          Function used to get the version number of the Conax VSC Module,
                as defined in the macro #AUI_MODULE_VERSION_NUM_CONAXVSC

@warning        This function can be used at any time

@param[out]     pul_version         = Pointer to the version number of Conax VSC
                                      Module

@return         @b AUI_RTN_SUCCESS  = Getting of the version number of Conax VSC
                                      Module performed successfully
@return         @b AUI_RTN_EINVAL   = The output parameter (i.e. [out]) is
                                      invalid
@return         @b Other_Values     = Getting of the version number of Conax VSC
                                      Module failed for some reasons
*/
AUI_RTN_CODE aui_conaxvsc_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to perform the initialization of the Conax VSC Module
                before its opening by the function #aui_conaxvsc_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the Conax VSC Module

@param[in]      p_call_back_init    = Callback function used to initialize the
                                      Conax VSC Module, as per comment for the
                                      function pointer @b p_fun_cb
@param[in]      pv_param            = The input parameter of the callback
                                      function @b p_call_back_init which is the
                                      first inout parameter of this function

@return         @b AUI_RTN_SUCCESS  = Conax VSC Module initialized successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Initializing of the Conax VSC Module
                                      failed for some reasons
*/
AUI_RTN_CODE aui_conaxvsc_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the Conax VSC
                Module after its closing by the function #aui_conaxvsc_close

@param[in]      p_call_back_init   = Callback function used to de-initialize
                                     the Conax VSC Module, as per comment for
                                     the function pointer @b p_fun_cb
@param[in]      pv_param           = The input parameter of the callback
                                     function @b p_call_back_init which is the
                                     first input parameter of this function

@return         @b AUI_RTN_SUCCESS = Conax VSC Module de-initialized successfully
@return         @b AUI_RTN_EINVAL  = At least one input parameter (i.e. [in])
                                     is invalid
@return         @b Other_Values    = De-initializing of the Conax VSC Module
                                     failed for some reasons
*/
AUI_RTN_CODE aui_conaxvsc_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to open a Conax Virtual Smart Card instance after
                its initialization by the function #aui_conaxvsc_init

@warning        This function should be used before calling any other function
                of Conax VSC Module except the function #aui_conaxvsc_init

@param[in]      p_attr                  = Pointer to a struct #aui_conaxvsc_attr
                                          which collects the configuration
                                          attributes

@param[out]     p_handle                = #aui_hdl pointer to the handle of the
                                          Conax VSC instance just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the Conax VSC instance
                                          performed successfully
@return         @b Other_Values         = Opening of the Conax VSC instance
                                          failed for some reasons, as defined
                                          in the <b> System Errors (ERRNO_SYS)
                                          Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_open (

    aui_conaxvsc_attr *p_attr,

    aui_hdl *p_handle

    );

/**
@brief          Function used to close a Conax Virtual Smart Card instance
                already opened by the function #aui_conaxvsc_open

@param[in]      handle                = #aui_hdl handle of the Conax VSC
                                        instance

@return         @b AUI_RTN_SUCCESS    = Closing of the Conax VSC instance
                                        performed successfully
@return         @b Other_Values       = Closing of the Conax VSC instance
                                        failed for some reasons. as defined
                                        in the <b> System Errors (ERRNO_SYS)
                                        Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_close (

    aui_hdl handle

    );

/**
@brief          Function used to register the callback function intended to
                provide an application the new available store data configuration

@param[in]      handle              = #aui_hdl handle of the Conax VSC instance
@param[in]      pv_attr             = Pointer to the attributes of the callback
                                      function to be registered, as defined in
                                      the struct #aui_conaxvsc_store_callback_attr

@return         @b AUI_RTN_SUCCESS  = Registering of the callback function
                                      performed successfully
@return         @b Other_Values     = Registering of the callback function failed
                                      for some reasons, as defined in the
                                      <b> System Errors (ERRNO_SYS) Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_register_store_callback (

    aui_hdl handle,

    aui_conaxvsc_store_callback_attr *pv_attr

    );

/**
@brief          Function used to unregister the callback function intended to
                provide an application the new available store data configuration
                (supposed that callback function has already been registered by
                the function #aui_conaxvsc_register_store_callback)

@param[in]      handle              = #aui_hdl handle of the Conax VSC instance

@return         @b AUI_RTN_SUCCESS  = Unregistering of the callback function
                                      performed successfully
@return         @b Other_Values     = Unregistering of the callback function
                                      failed for reasons as defined in the
                                      <b> System Errors (ERRNO_SYS) Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_unregister_store_callback (

    aui_hdl handle

    );

/**
@brief          Function used to send a command to the Conax Virtual Smart Card
                and get the response from it according to the specifications of
                the Conax Smart Card Interface

@param[in]      handle              = #aui_hdl handle of the Conax VSC instance
@param[in]      n_session_id        = Session identifier
@param[in]      pv_command          = Pointer to a store data buffer for the
                                      input command, as defined in the struct
                                      #aui_conaxvsc_tx_buf where, in particular,
                                      the member @b n_size specifies the number
                                      of valid bytes in the member @b p_uc_data
                                      (i.e. store data field) to be sent to the
                                      Conax Virtual Smart Card

@param[out]     pv_response         = Pointer to a store data buffer for the
                                      output response, as defined in the struct
                                      #aui_conaxvsc_tx_buf where, in particular,
                                      the member @b n_size specifies the actual
                                      length (in @a byte unit) of the read
                                      response from the Conax Virtual Smart Card
@param[out]     p_sw_1              = Status Word 1
@param[out]     P_sw_2              = Status Word 2

@return         @b AUI_RTN_SUCCESS  = Sending of the command to the Conax
                                      Virtual Smart Card performed successfully
@return         @b Other_Values     = Sending of the Command to the Conax
                                      Virtual Smart Card failed for some reasons,
                                      as defined in the <b> System Errors
                                      (ERRNO_SYS) Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_cmd_transfer (

    aui_hdl handle,

    int n_session_id,

    aui_conaxvsc_tx_buf *pv_command,

    aui_conaxvsc_tx_buf *pv_response,

    int *p_sw_1,

    int *p_sw_2

    );

/**
@brief          Function used to decrypt a Double Encrypted Code Word (DECW)
                and load to the chipset for stream decryption

@param[in]      handle              = #aui_hdl handle of the Conax VSC instance
@param[in]      pv_attr             = Pointer to the DECW decrypting attributes,
                                      as defined in the struct #aui_conaxvsc_decw_attr

@return         @b AUI_RTN_SUCCESS  = Decypting of the DECW performed successfully
@return         @b Other_Values     = Decrypting of the DECW failed for some
                                      reasons, as defined in the <b> System Errors
                                      (ERRNO_SYS) Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_decrypt_cw (

    aui_hdl handle,

    aui_conaxvsc_decw_attr *pv_attr

    );

/**
@brief          Function used to decrypt a recording key and load to the chipset
                for stream re-encrption

@param[in]      handle              = #aui_hdl handle of the Conax VSC instance
@param[in]      pv_attr             = Pointer to the PVR attributes for
                                      decrypting recording keys, as defined
                                      in the struct #aui_conaxvsc_en_key_attr

@return         @b AUI_RTN_SUCCESS  = Decrypting of the recording key performed
                                      successfully
@return         @b Other_Values     = Decrypting of the recording key failed
                                      for some reasons, as defined in the <b>
                                      System Errors (ERRNO_SYS) Module </b>
*/
AUI_RTN_CODE aui_conaxvsc_set_en_key (

    aui_hdl handle,

    aui_conaxvsc_en_key_attr *pv_attr

    );

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */

