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
Current ALi author: Frankly.Pan
Last update:        2017.02.13
-->

@file   aui_vmx.h

@brief  Verimatrix Advanced Security (VMX) Module (for Verimatrix's users @a only)

        <b> Verimatrix Advanced Security (VMX) Module </b> is used to encrypt/decrypt
        transport stream (TS) involved in Verimatrix AS flow.\n\n

        VMX Module does not use DSC/KL Module to setup the cryptography environment:
        the configuration of the encryption/decription key is moved to VMX Library
        of SEE, so the level of security is higher. That is an important feature of
        VMX Module.

        VMX Module supports the encryption/decryptiopn algorithms below:
        - AES 128bit
        - CSA

        About AES 128bit algorithm, the two different working modes below are supported:
        - ECB
        - CBC

@note At the moment, VMX Module can be used @a only in projects based on <b> Linux OS </b>

@copyright Copyright &copy; 2016 ALi &reg; Corporation. All Rights Reserved
*/

#ifndef __AUI_VMX__H_

#define __AUI_VMX__H_

#ifdef __cplusplus

extern "C"

{

#endif

/******************************Global Macro List*******************************/

/**
Macro to specify the maximum size of a buffer intended to store IV values of
AES 128 bit algorithm with CBC work mode
*/
#define AUI_VMX_MAX_IV_SIZE     32

/**
Macro to specify the maximum size of an array intended to store TS PIDs
*/
#define AUI_VMX_MAX_PID_COUNT   32

/*******************************Global Type List*******************************/

/**
Enum to specify an index of VMX services for all which the proper VMX device
will be opened

@note   For multiple request of the same service different VMX devices might be
        opened
*/
typedef enum aui_vmx_service_index {

    /**
    Value to specify the @b first VMX device used for descrambling DVB
    stream
    */
    AUI_VMX_SERVICE_DVB0 = 0x00,

    /**
    Value to specify the @b second VMX device used for descrambling DVB
    stream
    */
    AUI_VMX_SERVICE_DVB1 = 0x01,

    /**
    Value to specify the @b third VMX device used for descrambling DVB
    stream
    */
    AUI_VMX_SERVICE_DVB2 = 0x02,

    /**
    Value to specify the @b first VMX device used for descrambling IPTV
    stream
    */
    AUI_VMX_SERVICE_IPTV0 = 0x40,

    /**
    Value to specify the @b second VMX device used for descrambling IPTV
    stream
    */
    AUI_VMX_SERVICE_IPTV1 = 0x41,

    /**
    Value to specify the @b third VMX device used for descrambling IPTV
    stream
    */
    AUI_VMX_SERVICE_IPTV2 = 0x42,

    /**
    Value to specify the @b first VMX device used for recording/descrambling
    re-encrypted DVR stream with block mode
    */
    AUI_VMX_SERVICE_DVR0  = 0x80,

    /**
    Value to specify the @b second VMX device used for recording/descrambling
    re-encrypted DVR stream with block mode
    */
    AUI_VMX_SERVICE_DVR1  = 0x81,

    /**
    Value to specify the @b third VMX device used for recording/descrambling
    re-encrypted DVR stream with block mode
    */
    AUI_VMX_SERVICE_DVR2  = 0x82,

    /**
    Value to specify the @b first VMX device used for descrambling OTT
    stream (MP4/HLS) with block mode
    */
    AUI_VMX_SERVICE_OTT0 = 0xC0,

    /**
    Value to specify the @b second VMX device used for descrambling OTT
    stream (MP4/HLS) with block mode
    */
    AUI_VMX_SERVICE_OTT1 = 0xC1,

} aui_vmx_service_index;

/**
Enum to specify the callback type used by VMX module to send the data/messages
generated by VMX Library of SEE to upper levels/applications
*/
typedef enum aui_vmx_callback_type {

    /**
    Value to specify the callback type related to the Control Word Contribution
    of Crypto Firewall of KL Module
    */
    AUI_VMX_CALLBACK_TYPE_CF_CWC = 0,

} aui_vmx_callback_type;

/**
Enum to specify the cryptographic algoritms supported by VMX module
*/
typedef enum aui_vmx_algo {

    /**
    Value to specify @b CSA algorithm to decrypt/encrypt stream
    */
    AUI_VMX_ALGO_CSA = 0,

    /**
    Value to specify <b> AES 128bit </b> to decrypt/encrypt stream
    */
    AUI_VMX_ALGO_AES_128 = 1,

    /**
    Value to specify that clear stream, i.e. no need decryption/encryption
    */
    AUI_VMX_ALGO_FTA = 0xF

} aui_vmx_algo;

/**
Enum to specify the working modes of AES 128bit algorithm supported by VMX module
*/
typedef enum aui_vmx_crypto_mode {

    /**
    Value to specify @b ECB working mode
    */
    AUI_VMX_CRYPTO_MODE_ECB = 0,

    /**
    Value to specify @b CBC working mode
    */
    AUI_VMX_CRYPTO_MODE_CBC

} aui_vmx_crypto_mode;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Verimatrix Advanced Security (VMX) Module </b> to
       specify callback function attributes
       </div> @endhtmlonly

       Struct to specify the attributes of the callback function used to send the
       data/messages generated by VMX Library of SEE to upper levels/applications

@note  This structure is passed to the callback function #aui_vmx_msg_callback as
       the input parameter @b param
*/
typedef struct aui_vmx_callback_attr {

    /**
    Member to specify the callback type as defined in the enum #aui_vmx_callback_type
    */
    aui_vmx_callback_type callback_type;

    /**
    Member to specify the buffer intended to store the messages received from the
    VMX Library of SEE
    */
    unsigned char* buffer;

    /**
    Member to specify the size of the buffer specified with the member @b buffer
    of this struct
    */
    unsigned long  buffer_size;

} aui_vmx_callback_attr;

/**
Function pointer to specify the callback used by VMX module to send the data/messages
generated by VMX Library of SEE to upper levels/applicationsfor VMX module, as
per the type defined in the enum #aui_vmx_callback_type

@note Input parameter list:
      - @b pv_user_data  = Pointer to user data which will be passed to
                           upper levels/applications when the callback function
                           is called
      - @b pv_param      = Pointer to a structure #aui_vmx_callback_attr as
                           collection of callback function attributes
*/
typedef int (*aui_vmx_msg_callback) (

    void *pv_user_data,

    void *pv_param

    );

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Verimatrix Advanced Security (VMX) Module </b> to
       specify the descrambler attributes
       </div> @endhtmlonly

       Struct to specify the attributes of the descrambler

@note  The descrambler attributes need to be set when opening a VMX device
*/
typedef struct aui_vmx_dsc_attr {

    /**
    Member as an @a Index to refer different opened VMX Devices

    @note This index takes integer values from zero (0)
    */
    unsigned char uc_dev_idx;

    /**
    Member to specify the requested VMX services as defined in the enum
    #aui_vmx_service_index
    */
    aui_vmx_service_index service_index;

    /**
    Member to specify the size of a block (in @a bytes unit) processed by the
    descrabler

    @note @b 1. The descrambler will encrypt/decrypt TS data in blocks and will
                return the related amount processed in the structure
                #aui_vmx_dsc_status

    @note @b 2  The block size must be an integer multiple of 188 bytes (the default
                value is 188 x 256 bytes)
    */
    unsigned long block_size;

} aui_vmx_dsc_attr;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Verimatrix Advanced Security (VMX) Module </b> to
       specify a PID table for DVR recording/playback
       </div> @endhtmlonly

       Struct to specify a PID table for DVR recording/playback
*/
typedef struct aui_vmx_pid_info {

    /**
    Member to specify an index to surf in a PID array
    */
    unsigned short pid_number;

    /**
    Member to specify a PID array
    */
    unsigned short pid_table[AUI_VMX_MAX_PID_COUNT];

} aui_vmx_pid_info;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Verimatrix Advanced Security (VMX) Module </b> to
       specify the descrambler status information
       </div> @endhtmlonly

       Struct to specify the descrambler status when encrypting/decrypting
       TS data
*/
typedef struct aui_vmx_dsc_status {

    /**
    Member for specify the amount of blocks encrypted/decrypted by the descrambler
    */
    unsigned int rec_block_count;

} aui_vmx_dsc_status;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Verimatrix Advanced Security (VMX) Module </b> to
       specify the algorithm information for playback
       </div> @endhtmlonly

       Struct to specify the algorithm information for playback
*/
typedef struct aui_vmx_dsc_algo {

    /**
    Member to specify the cryptographic algoritms supported by VMX module,
    as defined in the enum #aui_vmx_algo
    */
    aui_vmx_algo ca_algo;

    /**
    Member to specify the working mode of the cryptographic algorith supported
    by VMX module
    */
    aui_vmx_crypto_mode ca_mode;

    /**
    Member to specify the buffer intended to store IV values of AES 128 bit
    algorithm with CBC work mode
    */
    unsigned char iv_value[AUI_VMX_MAX_IV_SIZE];

} aui_vmx_dsc_algo;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Verimatrix Advanced Security (VMX) Module </b> to
       specify the callback function information
       </div> @endhtmlonly

       Struct to specify the information related to the callback function
       used by VMX module to send the data/messages generated by VMX Library
       of SEE to upper levels/applications
*/
typedef struct aui_vmx_msg_callback_attr {

    /**
    Member to specify the user data of the callback function passed to upper
    levels/applications when the callback function is called, as per comment
    of the function pointer #aui_vmx_msg_callback
    */
    void *pv_user_data;

    /**
    Member to specify the callback function, as per comment of the function
    pointer #aui_vmx_msg_callback
    */
    aui_vmx_msg_callback p_fn_vmx_msg_cb;

} aui_vmx_msg_callback_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> VMX Module </b> to specify
        the identifier of VMX resources attached to AV Module
        </div> @endhtmlonly

        Struct to specify the identifier of VMX resources attached to
        AV Module by the function #aui_av_dsc_context_set.

@note   In multi-process environment, the VMX #aui_hdl handle cannot be
        passed from a process to another so this struct is used to represent
        the VMX resources for AV Module. And this structure can be recognized
        by AV Module in another process.
*/
typedef struct aui_vmx_resource_id {

    /**
    Member as an array of VMX identifiers
    */
    unsigned char identifier[16];

} aui_vmx_resource_id;

/*****************************Global Function List*****************************/

/**
@brief          Function used to open a VMX device then get the related handle

@param[in]      *p_vmx_attr         = Pointer to a struct #aui_vmx_dsc_attr
                                      which collects the atributes of the VMX
                                      device to be opened

@param[out]     p_handle            = #aui_hdl pointer to the handle of the
                                      VMX device just opened

@return         @b AUI_RTN_SUCCESS  = Opening of the VMX device performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Opening of the VMX device failed for
                                      some reasons
*/
AUI_RTN_CODE aui_vmx_open (

    aui_vmx_dsc_attr *p_vmx_attr,

    aui_hdl *p_handle

    );

/**
@brief          Function used to close a VMX device already opened by the function
                #aui_vmx_open, then the related device handle will be released
                (i.e. the related resources such as memory, device).

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of a VMX device in pair with its opening by the function #aui_vmx_open

@param[in]      handle              = #aui_hdl handle of the VMX Module already
                                      opened and to be closed

@return         @b AUI_RTN_SUCCESS  = Closing of the VMX device performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameters (i.e. [in]) is invalid
@return         @b Other_Values     = Closing of the VMX sevice failed for
                                      some reasons
*/
AUI_RTN_CODE aui_vmx_close (

    aui_hdl handle

    );

/**
@brief          Function used to set a cryptographic algorithm supported by
                VMX module

@warning        With reference to the enum #aui_vmx_service_index, @a only
                the VMX devices opened for the following services @a must be set
                with a cryptographic algorithm:
                - #AUI_VMX_SERVICE_DVB0
                - #AUI_VMX_SERVICE_DVB1
                - #AUI_VMX_SERVICE_DVB2
                - #AUI_VMX_SERVICE_IPTV0
                - #AUI_VMX_SERVICE_IPTV1
                - #AUI_VMX_SERVICE_IPTV2

@param[in]      handle              = #aui_hdl handle of the VMX device already
                                      opened
@param[in]      p_vmx_algo          = Pointer to a struct #aui_vmx_dsc_algo
                                      which collect the information of the
                                      algorithm to be set

@return         @b AUI_RTN_SUCCESS  = Setting of the algorithm performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of th algorithm failed for some
                                      reasons
*/
AUI_RTN_CODE aui_vmx_dsc_algo_set (

    aui_hdl handle,

    aui_vmx_dsc_algo *p_vmx_algo

    );

/**
@brief          Function used to set a PID table for DVR recording/playback

@warning        With reference to the enum #aui_vmx_service_index , @a only
                the VMX devices opened for the following services @a must be set
                with a PID table:
                - #AUI_VMX_SERVICE_DVR0
                - #AUI_VMX_SERVICE_DVR1
                - #AUI_VMX_SERVICE_DVR2

@param[in]      handle              = #aui_hdl handle of the VMX device
                                      already opened
@param[in]      p_pid_info          = Pointer to a struct #aui_vmx_pid_info
                                      which collect the needed information
                                      to set the PID table

@return         @b AUI_RTN_SUCCESS  = Setting of the PID table performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the PID table failed for
                                      some reasons
*/
AUI_RTN_CODE aui_vmx_pid_set (

    aui_hdl handle,

    aui_vmx_pid_info *p_pid_info

    );

/**
@brief          Function used to get the descrambler status (i.e. the amout of
                the blocks encrypted/decrypted) by using the service index as
                defined in the enum #aui_vmx_service_index

@param[in]      handle                  = #aui_hdl handle of the VMX device
                                          already opened

@param[out]     p_dsc_status            = Pointer to a struct #aui_vmx_dsc_status
                                          which collects the descrambler status
                                          information

@return         @b AUI_RTN_SUCCESS      = Getting of the descrabler status
                                          information performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameters (i.e. [in], [out])
                                          is invalid
@return         @b Other_Values         = Getting of the descrabler status
                                          information failed for some reasons
*/
AUI_RTN_CODE aui_vmx_dsc_status_get (

    aui_hdl handle,

    aui_vmx_dsc_status *p_dsc_status

    );

/**
@brief          Function used to registered a callback function used by VMX module
                to send the data/messages generated by VMX Library of SEE to upper
                levels/applicationsfor VMX module, as per the type defined in the
                enum #aui_vmx_callback_type

@note           This function should be called after opening a VMX device

@param[in]      handle                  = #aui_hdl handle of the VMX device already
                                          opened
@param[in]      p_callback_attr         = Pointer to a struct #aui_vmx_msg_callback_attr
                                          which collects the attribute of the callback
                                          function to be registered

@return         @b AUI_RTN_SUCCESS      = Registering of the callback function
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameters (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Registering of the callback function
                                          failed for some reasons
*/
AUI_RTN_CODE aui_vmx_register_message_callback (

    aui_hdl handle,

    aui_vmx_msg_callback_attr *p_callback_attr

    );

/**
@brief          Function used to get the structure #aui_vmx_resource_id, which is to
                identify the VMX(DSC) resources attached to AV Module, for setting up
                the descrambling context of AV Module.\n
                That structure, therefore, can be passed to AV Module in another process
                when working in multi-process environment.

@param[in]      handle                    = #aui_hdl handle of the specific VMX
                                            device already opened

@param[out]     p_resource_id             = Pointer to a struct
                                            #aui_vmx_resource_id which
                                            collects the identifiers of the VMX
                                            resources attached to AV Module

@return         @b AUI_RTN_SUCCESS        = Getting of the structure
                                            #aui_vmx_resource_id performed
                                            successfully
@return         @b AUI_RTN_EINVAL         = At least one parameter (i.e. [in],
                                            [out]) is invalid
@return         @b Others_Values          = Getting of the structure
                                            #aui_vmx_resource_id failed for some
                                            reasons

@note           This function is used @a only in projects based on <b> Linux OS </b>
*/

AUI_RTN_CODE aui_vmx_resource_id_get (

    aui_hdl handle,

    aui_vmx_resource_id *p_resource_id

    );

#ifdef __cplusplus

}

#endif

#endif//end of #define __AUI_VMX__H_

/* END OF FILE */

