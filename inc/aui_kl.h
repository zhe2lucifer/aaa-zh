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
Current ALi author: Wesley.He, Frankly.Pan
Last update:        2017.02.25
-->

@file   aui_kl.h

@brief  KL (Key Ladder) Module

        <b> Key Ladder (KL) Module </b> is used to <b> encrypt/decrypt </b> keys
        in the AUI API layer.\n\n

        The KL Module allows decryption/encryption of content keys using several
        levels of ciphered protecting keys.\n
        The cipher algorithms supported are @b AES and @b TDES, both using
        @b 128-bit key size, and output key is stored in KL Module that cannot
        be accessed by the CPU.\n

        Multiple instances of the KL Module can be opened and each one will
        allocate the necessary KL internal memory.
        Successive key generations in the same KL instance will use the same KL
        internal memory positions. The allocated memory will be freed when the
        KL instance is closed.

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Key Ladder (KL) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
*/

#ifndef _AUI_KL_H

#define _AUI_KL_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version number of KL Module </b>, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010100, where
- 0x0001 = <b> main version </b>
- 01 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_KL   (0X00010100)

/**
Macro to indicate the <b> maximum length </b> of all keys passed to key
generation function
*/
#define AUI_KL_MUTI_LEVEL_KEY_LEN_MAX   (256)

/**
Macro to indicate the <b> key length </b> (in byte unit) of @b AES algorithms
*/
#define AUI_KL_AES_KEY_LEN  16

/**
Macro to indicate the <b> key length </b> (in byte unit) of @b XOR algorithms
*/
#define AUI_KL_XOR_KEY_LEN  16

/**
Macro to indicate the <b> key length </b> (in byte unit) of @b TDES algorithms
*/
#define AUI_KL_TDES_KEY_LEN  8

/*******************************Global Type List*******************************/

/**
Enum to specify the <b> KL Crypt Mode </b> (i.e. encryption, decryption), and
is used by the struct #aui_cfg_kl
*/
typedef enum aui_kl_crypt_mode {

    /**
    This value indicates the <b> encryption mode </b>
    */
    AUI_KL_ENCRYPT=0,

    /**
    This value indicates the <b> decryption mode </b>
    */
    AUI_KL_DECRYPT,

    /**
    This value indicates an invalid crypt mode, and is used for checking input
    and output parameters
    */
    AUI_KL_NB

} aui_kl_crypt_mode;
/**
Enum to specify all different <b> KL Run Level Mode </b>. and is used by the
struct #aui_cfg_kl
*/
typedef enum aui_kl_run_level_mode {

    /**
    This value indicates the <b> All Mode </b> within which <em> several
    encryption/decryption levels key </em> will be generated in one time by the
    function #aui_kl_gen_key_by_cfg,
    in particular the number of them is decided by the user when opening KL
    Device by the function #aui_kl_open with the input parameter
    @b aui_kl_crypt_key_level
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL,

    /**
    This value indicates the <b> First Mode </b> within which <em> only the
    first encryption/decryption level key </em> will be generated in one time
    by the function #aui_kl_gen_key_by_cfg,
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST,

    /**
    This value indicates the <b> Second Mode </b> within which <em> only the
    second encryption/decryption level key </em>will be generated in one time
    by the function #aui_kl_gen_key_by_cfg
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND,

    /**
    This value indicates the <b> Third Mode </b> within which <em> only the
    third encryption/decryption level key </em> will be generated in one time
    by the function #aui_kl_gen_key_by_cfg
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD,

    /**
    This KL Run Level Mode is not supported.
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_SKIP_LEVEL,

    /**
    This value indicates the <b> Fifth Mode </b> within which <em> only the
    fifth encryption/decryption level key </em> will be generated in one time
    by the function #aui_kl_gen_key_by_cfg
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_FIVE,

    /**
    This value indicates the KL Device will perform the calculation for
    <b> CryptoFirewall </b>

    @note This value can @a only be used in the function #aui_kl_gen_key_by_cfg_ext
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_CF,

    /**
    This value indicates an <b> invalid KL Run Mode </b>, and is used for
    checking input & output parameters
    */
    AUI_KL_RUN_LEVEL_MODE_LEVEL_NB

} aui_kl_run_level_mode;

/**
Enum to specify the different <b> storage positions </b> of the <b> root key </b>
in <b> KL Memory </b>, and is used by the struct #aui_attr_kl
*/
typedef enum aui_kl_root_key_idx {

    /**
    This value specifies the first key position of the <b> root key 0 </b>
    */
    AUI_KL_ROOT_KEY_0_0=0,

    /**
    This value specifies the first key position of the <b> root key 1 </b>
    */
    AUI_KL_ROOT_KEY_0_1=1,

    /**
    This value specifies the first key position of the <b> root key 2 </b>
    */
    AUI_KL_ROOT_KEY_0_2=2,

    /**
    This value specifies the first key position of the <b> root key 3 </b>
    */
    AUI_KL_ROOT_KEY_0_3=3,

    /**
    This value specifies the first key position of the <b> root key 4 </b>
    */
    AUI_KL_ROOT_KEY_0_4=4,

    /**
    This value specifies the first key position of the <b> root key 5 </b>
    */
    AUI_KL_ROOT_KEY_0_5=5,

    /**
    This value specifies the second key position of the <b> root key 6 </b>,
    considering that it consists of two keys
    */
    AUI_KL_ROOT_KEY_0_6_R=6,

    /**
    This value specifies the first key position of the <b> root key 6 </b>,
    considering that it consists of two keys
    */
    AUI_KL_ROOT_KEY_0_6=7,

    /**
    This value specifies the first key position of the <b> root key 7 </b>
    */
    AUI_KL_ROOT_KEY_0_7=8,

    /**
    This value specifies an <b> invalid root key position </b>, and is used
    for checking input & output parameters
    */
    AUI_KL_ROOT_KEY_NB

} aui_kl_root_key_idx;

/**
Enum to specify the <b> Output Key Pattern </b> which defines the number of
bits to be allocated in KL Memory
*/
typedef enum aui_kl_output_key_pattern {

    /**
    This value indicates that the output in the last calculated encryption/
    decryption level consists of a <b> 64 bits single key </b>
    */
    AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE,

    /**
    This value indicates that the output in the last calculated encryption/
    decryption level consist of a <b> 64 bits odd key </b> and <b> 64 bits even
    key </b>
    */
    AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN,

    /**
    This value indicates that the output in the last calculated encryption/
    decryption level consists of a <b> 128 bits single key </b>
    */
    AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE,

    /**
    This value indicates that the output in the last calculated encryption/
    decryption level consist of a <b> 128 bits odd key </b> and <b> 128 bits
    even key </b>
    */
    AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN,

    /**
    This value indicates an <b> invalid output key pattern </b>, and is used
    for checking input and output parameters
    */
    AUI_KL_OUTPUT_KEY_PATTERN_NB

} aui_kl_output_key_pattern;

/**
Enum to specify the <b> Output Key Attributes </b> which are the number of the
keys and their content type present in the array of keys as output
*/
typedef enum aui_kl_cw_key_attr {

    /**
    This value is used in the <b> Pure Data Mode </b>, as defined in the enum
    #aui_en_dsc_data_type, and indicates that the KL device generate a single
    key as the DSC control word(CW)
    */
    AUI_KL_CW_KEY_SINGLE = 0,

    /**
    This value is used in the <b> TS Mode </b>, as defined in the enum
    #aui_en_dsc_data_type, and indicates that the KL device generate <b> odd
    key </b> as the DSC control word(CW)
    */
    AUI_KL_CW_KEY_ODD = (1<<0),

    /**
    This value is used in the <b> TS Mode </b>, as defined in the enum
    #aui_en_dsc_data_type, and indicates that the KL device generate <b> even
    key </b> as the DSC control word(CW)
    */
    AUI_KL_CW_KEY_EVEN = (1<<1),

    /**
    This value is used in the <b> TS Mode </b>, as defined in the enum
    #aui_en_dsc_data_type, and indicates that the KL device generate <b> odd
    key </b> and <b> even key </b> as the DSC control word(CW)
    */
    AUI_KL_CW_KEY_ODD_EVEN = AUI_KL_CW_KEY_ODD | AUI_KL_CW_KEY_EVEN,

    /**
    This value indicates an <b> Invalid Output Key Attribute </b>, and is used
    for checking input and output parameters
    */
    AUI_KL_CW_KEY_ATTR_NB

} aui_kl_cw_key_attr;

/**
Enum to specify the <b> Key Derivation Levels </b>, and is used by the struct
#aui_attr_kl
*/
typedef enum aui_kl_crypt_key_level {

    /**
    This value specifies no need to perform any encryption/decryption,
    i.e. the key is clear
    */
    AUI_KL_KEY_SKIP_LEVEL,

    /**
    This value specifies the KL Device will perform the calculation of the
    <b> level 1 </b> when calling once the function #aui_kl_gen_key_by_cfg
    */
    AUI_KL_KEY_ONE_LEVEL,

    /**
    This value specifies the KL Device will perform the calculation <b> from
    level 1 to level 2 </b> when calling once the function
    #aui_kl_gen_key_by_cfg
    */
    AUI_KL_KEY_TWO_LEVEL,

    /**
    This value specifies the KL Device will perform the calculation <b> from
    the level 1 to level 3 </b> when calling once the function
    #aui_kl_gen_key_by_cfg
    */
    AUI_KL_KEY_THREE_LEVEL,

    /**
    This Key Derivation Level is not supported
    */
    AUI_KL_KEY_SKIP_FOURLEVEL,

    /**
    This value specifies the KL Device will perform the calculation <b> from
    the level 1 to level 5 </b> when calling once the function
    #aui_kl_gen_key_by_cfg
    */
    AUI_KL_KEY_FIVE_LEVEL,

    /**
    This value indicates an <b> Invalid Key Derivation Level </b>, and is used
    for checking input and output parameters
    */
    AUI_KL_KEY_LEVEL_NB

} aui_kl_crypt_key_level;

/**
Enum to specify the <b> KL supported algorithms </b>, and is used by the struct
#aui_cfg_kl
*/
typedef enum aui_kl_algo {

    /**
    This value indicates the <b> Triple Data Encryption Standard (DES) </b>
    algorithm
    */
    AUI_KL_ALGO_TDES = 0,

    /**
    This value indicates the <b> Advanced Encryption Standard (AES) </b>
    algorithm
    */
    AUI_KL_ALGO_AES,

    /**
    This value indicates an <b> Invalid algorithms </b>, it is used for
    checking input and output parameters
    */
    AUI_KL_ALGO_NB

} aui_kl_algo;

/**
Enum to specify the <b> KL command list </b> to be used to configure the KL
Device, and is used by the #aui_kl_set
*/
typedef enum aui_kl_item_set {

    /**
    This value indicates an <b> Invalid Configuration Command </b>, and is used
    for checking input and output parameters
    */
    AUI_KL_SET_CMD_NB

} aui_kl_item_set;

/**
Enum to specify the <b> KL command list </b> to be used to get KL Device
Information, and is used by the #aui_kl_get
*/
typedef enum aui_kl_item_get {

    /**
    This value indicates the KL Memory position of the generated key
    */
    AUI_KL_GET_KEY_POS,

    /**
    This value indicates the size (in @a bit unit) of the generated key
    */
    AUI_KL_GET_KEY_SIZE,

    /**
    This value indicates an <b> Invalid Getting Command </b>, and is used for
    checking input and output parameters
    */
    AUI_KL_GET_CMD_NB

} aui_kl_item_get;

/**
Enum to specify the type of key ladder to be opened for derivation
*/
typedef enum aui_kl_type {

    /**
    Value to specify that the key ladder is @b ALi key ladder device
    */
    AUI_KL_TYPE_ALI,

    /**
    Value to specify that the key ladder is @b ETSI key ladder device

    @note   ETSI key ladder will be enabled @a only when some special OTP bits
            are set
    */
    AUI_KL_TYPE_ETSI,

    /**
    Value to specify that the key ladder is @b CONAXVSC key ladder device
    */
    AUI_KL_TYPE_CONAXVSC

} aui_kl_type;

typedef enum aui_kl_key_source {

    /**
    Value to indicate that the key is the clear key from the RAM buffer
    */
    AUI_KL_KEY_SOURCE_RAM,

    /**
    Value to indicate that the key is from the other KL (Key Ladder) Module
    */
    AUI_KL_KEY_SOURCE_KEY_LADDER

} aui_kl_key_source;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Key Ladder (KL) Module </b> to specify KL Device
        Attributes
        </div> @endhtmlonly

        Struct to specify the KL Device Attributes
*/
typedef struct aui_attr_kl {

    /**
    Member as an <b> Index </b> that refers to different opened KL Device,
    and it starts from 0
    */
    unsigned char uc_dev_idx;

    /**
    Member to specify the <b> Key Derivation Levels </b>, as defined in the
    enum #aui_kl_crypt_key_level
    */
    aui_kl_crypt_key_level en_level;

    /**
    Member to specify the size of the output keys
    */
    aui_kl_output_key_pattern en_key_pattern;

    /**
    Member as an <b> Index </b> that refers to the KL root key position

    @note  If the member @b en_key_ladder_type of this struct is set to the enum
           value #AUI_KL_TYPE_ETSI, then this member can not be set to the enum
           value #AUI_KL_ROOT_KEY_0_3
    */
    aui_kl_root_key_idx en_root_key_idx;

    /**
    Member to specify the type of key ladder to be used
    */
    aui_kl_type en_key_ladder_type;

} aui_attr_kl;


/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Key Ladder (KL) Module </b> to specify the
        configuration of KL Device
        </div> @endhtmlonly

        Struct to specify the configuration of the KL Device
*/
typedef struct aui_cfg_kl {

    /**
    Member to specify the <b> KL Run Level Mode </b> to be used, as define in
    the enum #aui_kl_run_level_mode
    */
    aui_kl_run_level_mode run_level_mode;

    /**
    Member to specify the <b> KL Crypt Mode </b> to be used, as defined enum
    #aui_kl_crypt_mode
    */
    aui_kl_crypt_mode en_crypt_mode;

    /**
    Member to specify the <b> KL supported algorithm </b> to be used, as defined
    in the enum #aui_kl_algo
    */
    aui_kl_algo en_kl_algo;

    /**
    Member to specify the <b> Output Key Attribute </b> to be used, as defined
    in the enum #aui_kl_cw_key_attr
    */
    aui_kl_cw_key_attr en_cw_key_attr;

    /**
    Member as an array used to store the two kind of keys as below
    - Ciphered Protecting Key
    - Control Word

    and configure them into KL Device.\n
    Some use cases of the key map in the memory are listed below:

    <b> Use case 1 </b>\n

     Conditions:
     - 3 level KL
     - 64 bit key length
       (i.e. #aui_kl_output_key_pattern = #AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN)
     - ODD and EVEN control word
       (i.e.  #en_cw_key_attr = #AUI_KL_CW_KEY_ODD_EVEN)

     Key map:
     - Ciphered Protecting Key Level 1 (128 bit)
     - Ciphered Protecting Key Level 2 (128 bit)
     - Ciphered Content Key Odd (64 bit)
     - Ciphered Content Key Even (64 bit)

    <b> Use case 2 </b>\n

    Conditions:
    - 3 level KL
    - 128 bit key length
      (i.e. #aui_kl_output_key_pattern = #AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN)
    - ODD and EVEN control word
      (i.e. #en_cw_key_attr = #AUI_KL_CW_KEY_ODD_EVEN)

    Key map:
    - Ciphered Protecting Key Level 1 (128 bit)
    - Ciphered Protecting Key Level 2 (128 bit)
    - Ciphered Content Key Odd (128 bit)
    - Ciphered Content Key Even (128 bit)

    <b> Use case 3 </b>\n

    Conditions:
    - 3 level KL
    - 128 bit key length
      (i.e. #aui_kl_output_key_pattern = #AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN)
    - EVEN control word
      (i.e.  #en_cw_key_attr = #AUI_KL_CW_KEY_EVEN)

    Key map:
    - Ciphered Protecting Key Level 1 (128 bit)
    - Ciphered Protecting Key Level 2 (128 bit)
    - Ciphered Content Key Even (128 bit)

    <b> Use case 4 </b>\n

    Conditions:
    - 3 level KL
    - 128 bit key length
      (i.e. #aui_kl_output_key_pattern = #AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN)
    - ODD  control word
      (i.e.  #en_cw_key_attr = #AUI_KL_CW_KEY_ODD)

    Key map:
    - Ciphered Protecting Key Level 1 (128 bit)
    - Ciphered Protecting Key Level 2 (128 bit)
    - Ciphered Content Key Odd (128 bit)

    @note @b 1. The total amount of the key type depends of the member
                #en_cw_key_attr
     .
    @note @b 2. The size of the keys of the key map must match the size decided
                by the function #aui_kl_open with input parameter the enum
                #aui_kl_output_key_pattern
    */
    unsigned char ac_key_val[AUI_KL_MUTI_LEVEL_KEY_LEN_MAX];

} aui_cfg_kl;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Key Ladder (KL) Module </b> to specify the
        CryptoFirewall attribute
        </div> @endhtmlonly

        Struct to specify the CryptoFirewall attribute
*/
typedef struct aui_kl_cf_target_attr {

    /**
    Member to specify the device index of the CryptoFirewall

    @warning  At present, it must be zero (0)
    */
    unsigned char uc_cf_dev_idx;

    /**
    Member to speciy the parity of the output from CryptioFirewall
    */
    aui_kl_cw_key_attr parity;

} aui_kl_cf_target_attr;

/// @cond

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Key Ladder (KL) Module </b> to specify the
        key source attribute
        </div> @endhtmlonly

        Struct to specify the key source attribute
*/
typedef struct aui_kl_key_source_attr {

    /**
    Member to specify the source of key. When the current
    KL run level_mode is set to #AUI_KL_RUN_LEVEL_MODE_LEVEL_CF,
    key ladder allows the key or data from clear key or other key ladder
    */
    aui_kl_key_source key_source;

    /**
    Member to specify the key for CryptoFirewall calculation
    */
    union {

        struct {

            /**
            Sub-member to specify the handle of source key ladder
            */
            aui_hdl key_ladder_handle;

            /**
            Sub-member to specify the parity of key: the key ladder may have
            odd and even keys, this member specify which key will be used for
            CF calculation.
            */
            aui_kl_cw_key_attr parity;

        };

        /**
        Sub-member to specify the clear key for CryptoFirewall calculation
        */
        unsigned char buf[16];

    } key_param;

} aui_kl_key_source_attr;

/// @endcond

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the version number of KL Module as defined
                in the macro #AUI_MODULE_VERSION_NUM_KL

@warning        This function can be used at any time

@param[out]     pul_version         = Pointer to the version number of KL Module
                                                               .
@return         @b AUI_RTN_SUCCESS  = Getting of the version number of KL Module
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = The output parameter (i.e. [out]) is
                                      invalid
@return         @b Other_Values     = Getting of the version number of KL Module
                                      failed for some reasons
*/
AUI_RTN_CODE aui_kl_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to perform the initialization of the KL Module
                before its opening by the function #aui_kl_open

@warning        This function can be used only in the <b> Pre-Run Stage </b>
                of the KL Module


@param[in]      p_call_back_init    = Callback function used to initialize the
                                      KL Module, as per comment for the function
                                      pointer @b p_fun_cb
@param[in]      pv_param            = The input parameter of the callback
                                      function @b p_call_back_init which is the
                                      first parameter [in] of this function

@return         @b AUI_RTN_SUCCESS  = KL Module initialized successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Initializing of the KL Module failed for
                                      some reasons

@note           About the callback function @b p_call_back_init as input parameter:
                - In projects based on <b> Linux OS </b>, it is suggested to
                  set as @b NULL
                - In projects based on <b> TDS OS </b>, it will perform some
                  video device attachments and configurations.please refer to
                  the sample code related to the <b> Init Stage </b> for more
                  clarifications
*/
AUI_RTN_CODE aui_kl_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the KL Module
                after its closing by the function #aui_kl_close

@param[in]      p_call_back_init    = Callback function used to de-initialize the
                                      KL Module, as per comment for the function
                                      pointer @b p_fun_cb
@param[in]      pv_param            = The input parameter of the callback
                                      function @b p_call_back_init which is the
                                      first parameter [in] of this function

@return         @b AUI_RTN_SUCCESS  = KL Module de-initialized successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = De-initializing of the KL Module failed
                                      for some reasons

@note           @a Only in projects based on <b> TDS OS </b>, it is suggested
                to @a only use this function before to go into standby mode
*/
AUI_RTN_CODE aui_kl_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function is used to open the KL device and configure the desired
                attributes then get the related handle

@warning        This function can @a only be used in the <b> Pre-Run Stage </b>
                of the KL device, in particular after performing the
                initialization of the KL Module by the function #aui_kl_init
                for the first opening of the KL device.

@param[in]      *p_attr_kl          = Pointer to the struct #aui_attr_kl which
                                      collects the attribute of the KL device

@param[out]     *pp_hdl_kl          = #aui_hdl pointer to the handle of the KL
                                      device just opened

@return         @b AUI_RTN_SUCCESS  = KL device opened successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameters
                                      (i.e. [in], [out]) are invalid
@return         @b Other_Values     = Opening of the KL device failed for some
                                      reasons
*/
AUI_RTN_CODE aui_kl_open (

    aui_attr_kl *p_attr_kl,

    aui_hdl *pp_hdl_kl

    );

/**
@brief          Function used to close the KL Module already opened by the
                function #aui_kl_open then the related device handle will be
                released (i.e. the related resources such as memory, device)

@warning        This function can @a only be used in the <b> Post-Run Stage
                </b> of the KL device in pair with its opening by the function
                #aui_kl_open. After closing the KL device, user can:
                - Either perform the de-initialization of the KL Module by the
                  function #aui_kl_de_init
                - Or open again the KL device by the function #aui_kl_open,
                  considering the initialization of the KL Module has been
                  performed previously by the function #aui_kl_init

@param[in]      *p_hdl_kl           = #aui_hdl handle of the KL Module
                                      already opened and to be closed

@return         @b AUI_RTN_SUCCESS  = KL device closed successfully
@return         @b AUI_RTN_EINVAL   = The input parameters (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Closing of the KL device failed for
                                      some reasons
*/
AUI_RTN_CODE aui_kl_close (

    aui_hdl p_hdl_kl

    );

/**
@brief          Function used to generate the encrypted/decrypted key and store
                it into the key internal memory position of KL device after its
                opening by the function #aui_kl_open

@warning        This function should not be called when the Key ladder is used
                - To store the output of CF module, then the function
                  #aui_kl_cf_target_set should be used instead
                - To run the operations as per the #AUI_KL_RUN_LEVEL_MODE_LEVEL_CF
                  run level mode, then the function #aui_kl_gen_key_by_cfg_ext
                  should be used instead

@param[in]      *p_hdl_kl           = #aui_hdl handle of the KL Device already
                                      opened
@param[in]      *p_kl_cfg           = Pointer to the struct #aui_cfg_kl which
                                      specify the configuration of KL Device

@param[out]     *pul_key_pos_dst    = Pointer to the output key position that
                                      store the key content that be encrypt/
                                      decrypt

@return         @b AUI_RTN_SUCCESS  = Generating of the encrypted/decrypted key
                                      and storing performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both parameters (i.e. [in],
                                      [out]) are invalid
@return         @b Other_Values     = Generating of the encrypted/decrypted key
                                      and/or storing failed for some reasons
*/
AUI_RTN_CODE aui_kl_gen_key_by_cfg (

    aui_hdl p_hdl_kl,

    aui_cfg_kl *p_kl_cfg,

    unsigned long *pul_key_pos_dst

    );

/**
@brief          Function used to perform a specific setting of the KL Module
                after its opening by the functions #aui_kl_open

@param[in]      p_hdl_kl            =  #aui_hdl handle of the KL Module already
                                       opened and to be managed to perform a
                                       specific setting
@param[in]      ul_item             =  The item related to the specific setting
                                       of the KL Module to be performed, as
                                       defined in the enum #aui_kl_item_set
@param[in]      *pv_param           =  The pointer as per the description of the
                                       specific setting of the KL Module to be
                                       performed, as defined in the enum
                                       #aui_kl_item_set

@return         @b AUI_RTN_SUCCESS  =  Specific setting of the KL Module performed
                                       successfully
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values     =  Specific setting of the KL Module failed
                                       for some reasons
*/
AUI_RTN_CODE aui_kl_set (

    aui_hdl p_hdl_kl,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific information of the KL Module,
                as defined in the enum #aui_kl_item_get, after its opening by
                the functions #aui_kl_open

@param[in]      p_hdl_kl            =  #aui_hdl handle of the KL Module already
                                       opened and to be managed to get a specific
                                       information
@param[in]      ul_item             =  The item related to the specific information
                                       of the KL Module to be gotten, as defined
                                       in the enum #aui_kl_item_get

@param[out]     *pv_param           =  The pointer as per the description of the
                                       specific information of the KL Module to
                                       be gotten, as defined in the enum
                                       #aui_kl_item_get

@return         @b AUI_RTN_SUCCESS  =  Getting of the specific information of
                                       the KL Module performed successfully
@return         @b AUI_RTN_EINVAL   =  Either one or both of the parameters
                                       (i.e. [in] and [out]) are invalid
@return         @b Other_Values     =  Getting of the specific information of
                                       the KL Module failed for some reasons
*/
AUI_RTN_CODE aui_kl_get (

    aui_hdl p_hdl_kl,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to load HDCP key into chip then active HDCP key

@param[in]      *puc_key            = Pointer to the HDCP key content
@param[in]      ul_byte_len         = The length of HDCP key content
                                      - @b Caution: @a Only a length of 288
                                           bytes is supported.

@return         @b AUI_RTN_SUCCESS  = Loading of the HDCP key into chip and
                                      activating HDCP key performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                      (i.e. [in]) are invalid
@return         @b Other_Values     = Loading of the HDCP key into chip and/or
                                      activating HDCP key failed failed for some
                                      reasons
*/
AUI_RTN_CODE aui_kl_load_hdcp_key (

    unsigned char *puc_key,

    unsigned long ul_byte_len

    );

/**
@brief          Function used to set a key ladder as the target for the CF
                module. The CF module will generate the key and put the key
                into the target key ladder module.

@warning        When a KL device is created for storing the output of the CF
                module, the function #aui_kl_gen_key_by_cfg should not be called.
                On the other hand, the function #aui_kl_cf_target_set should be
                called to store the key generated by CF module.

@param[in]      handle              = #aui_hdl handle of the KL Module already
                                      opened and to be used as the target of
                                      CF module.
@param[in]      p_cf_target_attr    = Pointer to the struct #aui_kl_cf_target_attr
                                      to specify the target position for the
                                      output of the CF module.

@return         @b AUI_RTN_SUCCESS  = Setting of the target of CF performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the the target of CF failed
                                      for some reasons
*/
AUI_RTN_CODE aui_kl_cf_target_set (

    aui_hdl handle,

    aui_kl_cf_target_attr *p_cf_target_attr

    );

/**
@brief          Function used to generate the encrypted/decrypted key and store
                it into the key internal memory position of KL device after its
                opening by the function #aui_kl_open

@warning        This function allows using the output of CF module as source of
                the key. And the run level mode specified in the struct #aui_cfg_kl
                by the input parameter @b p_cfg @a must be #AUI_KL_RUN_LEVEL_MODE_LEVEL_CF

@param[in]      handle              = #aui_hdl handle of the KL Device already
                                      opened
@param[in]      p_cfg               = Pointer to the struct #aui_cfg_kl which
                                      specify the configuration of KL Device

@param[in]      p_data_source       = Pointer to the struct #aui_kl_key_source_attr
                                      which specify the source of data.
                                      - @b Caution: This parameter will be used when
                                           the KL run level mode in the struct
                                           #aui_cfg_kl is set to the value
                                           #AUI_KL_RUN_LEVEL_MODE_LEVEL_CF. In
                                           other case, please set this parameter
                                           to @b NULL.

@param[in]      p_key_source        = Pointer to the struct
                                      #aui_kl_key_source_attr which specify the
                                      source of key
                                      - @b Caution: This parameter will be used
                                           when the KL run level mode in the struct
                                           #aui_cfg_kl is set to the value
                                           #AUI_KL_RUN_LEVEL_MODE_LEVEL_CF.
                                           In other case, please set this parameter
                                           to @b NULL.

@return         @b AUI_RTN_SUCCESS  = Generating of the encrypted/decrypted key
                                      and storing performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameters (i.e. [in],
                                      is invalid
@return         @b Other_Values     = Generating of the encrypted/decrypted key
                                      and/or storing failed for some reasons
*/
AUI_RTN_CODE aui_kl_gen_key_by_cfg_ext (

    aui_hdl handle,

    aui_cfg_kl *p_cfg,

    aui_kl_key_source_attr *p_data_source,

    aui_kl_key_source_attr *p_key_source

    );

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */


