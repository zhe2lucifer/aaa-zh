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
Last update:        2018.02.28
-->

@file   aui_dsc.h

@brief  Descrambler (DSC) Module

        <b> Descrambler (DSC) Module </b> is used to <b> encrypt/decrypt Raw
        Data </b> (also known as <b> RAM2RAM Data </b>) or <b> Transport Stream
        (TS)</b>. The supported algorithms are:
        - DES/TDES
        - AES
        - CSA
        - SHA

        About @b DES/TDES and @b AES algorithms, they have different working
        modes such as
        - ECB
        - CBC
        - OFB
        - CFB
        - CTR

        When the data source is <b> TS Stream </b> from <b> DMX Module </b>,
        <em> no need </em> to call the functions
        - #aui_dsc_encrypt
        - #aui_dsc_decrypt

        since those are responsible for generating the <b> clear/encrypt data
        </b> when the data source is <b>Raw Data</b> to DSC Device.

@note For further details, please refer to ALi document
      <b><em> ALi_AUI_Porting_Guide_Modules.pdf - Chapter: "Descrambler (DSC)
      Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_DSC_H

#define _AUI_DSC_H

/**************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List*******************************/

/**
Macro to indicate the <b> version number of DSC Module </b>, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 = <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_DSC  (0X00010000)

/**
Macro to specify the lenght of a HMAC message to be generated
*/
#define HMAC_OUT_LENGTH   32

/**
Macro to specify the lenght of the first key
*/
#define FIRST_KEY_LENGTH   16

/*******************************Global Type List*******************************/

/**
Enum to specify all available @b algorithm supported by DSC device to
@b encrypt/decrypt data.
*/
typedef enum aui_dsc_algo {

    /**
    Value to indicate the <b> Data Encryption Standard (DES) </b> algorithm
    */
    AUI_DSC_ALGO_DES=0,

    /**
    Value to indicate the <b> Advanced Encryption Standard (AES) </b> algorithm
    */
    AUI_DSC_ALGO_AES=1,

    /**
    Value to indicate the <b> Secure Hash Algorithm (SHA) </b> algorithm
    */
    AUI_DSC_ALGO_SHA=2,

    /**
    Value to indicate the <b> Triple Data Encryption Standard (TDES) </b>
    algorithm
    */
    AUI_DSC_ALGO_TDES=3,

    /**
    Value to indicate the <b> Common Scramble Algorithm (CSA) </b>
    */
    AUI_DSC_ALGO_CSA=4,

    /**
    Value to indicate an invalid algorithm, and is used for checking the
    input/output parameters
    */
    AUI_DSC_ALGO_NB

} aui_dsc_algo;

/**
Enum to specify which <b> Work Modes </b> can support @a only the algorithms
@b DES/TDES and @b AES.
*/
typedef enum aui_dsc_work_mode {

    /**
    Value to indicate the @b CBC work mode
    */
    AUI_DSC_WORK_MODE_IS_CBC = 0,

    /**
    Value to indicate the @b ECB work mode
    */
    AUI_DSC_WORK_MODE_IS_ECB = (1<<4),

    /**
    Value to indicate the @b OFB work mode
    */
    AUI_DSC_WORK_MODE_IS_OFB = (2<<4),

    /**
    Value to indicate the @b CFB work mode
    */
    AUI_DSC_WORK_MODE_IS_CFB = (3<<4),

    /**
    Value to indicate the @b CTR work mode
    */
    AUI_DSC_WORK_MODE_IS_CTR = (4<<4),

    /**
    Value to indicate an invalid work mode used for checking the
    input/output parameters
    */
    AUI_DSC_WORK_MODE_NB = 5

} aui_dsc_work_mode;

/**
Enum to specify the
- <b> Key parity attributes </b>
- <b> Parity mode </b>
which can @a only be used in <b> TS Stream mode </b>
*/
typedef enum aui_dsc_parity_mode {

    /**
    Value to indicate that the <b> even parity </b> is forced then
    - All output TS packets are scrambled with the <b> even key </b>
    - The scrambling control flag in the output packets is @b even

    @note  This mode cannot be used for TS decryption but @a only for TS
           encryption
    */
    AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE  =0,

    /**
    Value to indicate that the <b> odd parity </b> is forced then
    - All output TS packets are scrambled with the <b> odd key </b>
    - The scrambling control flag in the output packets is @b odd

    @note  This mode cannot be used for descryption but @a only for TS
           encryption
    */
    AUI_DSC_PARITY_MODE_ODD_PARITY_MODE =1,

    /**
    Value to indicate <b> automatic parity </b> then the parity is detected
    from the TS packet header

    @note  This mode cannot be used for TS encryption but @a only TS decryption
    */
    AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0= 2,

    /**
    @attention This enum value is @a reserved to ALi R&D Dept. then user can
               ignore it
    */
    AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE1=3,

    /**
    Value to indicate a <b> key from OTP position 0x68 </b> which is usually
    used by the boot loader
    */
    AUI_DSC_PARITY_MODE_OTP_KEY_FROM_68 = 4,

    /**
    Value to indicate a <b> key from OTP position 0x6C </b>, which is usually
    used by the boot loader
    */
    AUI_DSC_PARITY_MODE_OTP_KEY_FROM_6C = 5,

    /**
    Value to indicate an invalid parity mode, and is used for checking the
    input/output parameters
    */
    AUI_DSC_PARITY_MODE_NB

} aui_dsc_parity_mode;

/**
Enum to specify all available <b> modes to process the residual blocks </b>
valid just for the <b> CBC work mode </b>.
*/
typedef enum aui_dsc_residue_block {

    /**
    Value to indicate that the <b> residual blocks are not processed </b>
    */
    AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE = 0,

    /**
    Value to indicate the @b ATSC mode (i.e. <b> ANSI SCT 52 Standard </b>)
    */
    AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC = (1 << 12),

    /**
    Value to indicate the @b CTS mode (the residual block handling uses cipher
    stealing method)
    */
    AUI_DSC_RESIDUE_BLOCK_IS_HW_CTS = (2 << 12),

    /**
    @attention  This value is current reserved to ALi R&D Dept. then user can
                ignore it
    */
    AUI_DSC_RESIDUE_BLOCK_IS_RESERVED = (3 << 12),

    /**
    Value to indicate an invalid mode to process the residual blocks used for
    checking the input/output parameters
    */
    AUI_DSC_RESIDUE_BLOCK_NB = 4

} aui_dsc_residue_block;

/**
Enum to specify the different <b> Crypt Mode </b> (i.e. encryption/decryption)
that the DSC Device can use for the input data source from TS Stream or Raw Data
*/
typedef enum aui_dsc_crypt_select {

    /**
    Value to indicate that DSC device decrypts the input data source from TS
    Stream or Raw Data
    */
    AUI_DSC_DECRYPT=0,

    /**
    Value to indicate the DSC Device encrypts the input data source from TS
    Stream or Raw Data
    */
    AUI_DSC_ENCRYPT=1

} aui_dsc_crypt_select;

/**
Enum to specify where the key is stored
*/
typedef enum aui_en_dsc_key_type {

    /**
    Value to indicate that the key is stored in the @b register
    */
    AUI_DSC_HOST_KEY_REG=0,

    /**
    Value to indicate that the key is stored in @b S-RAM
    */
    AUI_DSC_HOST_KEY_SRAM,

    /**
    Value to indicate that the key is generated by the KL (Key Ladder) Module
    */
    AUI_DSC_CONTENT_KEY_KL,

    /**
    Value ti indicate that the key is stored in OTP which is a special type of
    Key Ladder that belong to the first level of KL Module
    */
    AUI_DSC_CONTENT_KEY_OTP,

    /**
    Value to indicate an invalid key storage type,and is used for checking the
    input/output parameters
    */
    AUI_DSC_KEY_TYPE_NB

} aui_dsc_key_type, *aui_p_dsc_key_type;

/**
Enum to specify the <b> type of data source </b> which is the input of the DSC
Device
*/
typedef enum aui_en_dsc_data_type {

    /**
    Value to indicate that <b> TS Stream </b> is the input source of the DSC
    Device
    */
    AUI_DSC_DATA_TS = 0,

    /**
    Value to indicate that <b> Raw Data </b> is the input source of the DSC
    Device
    */
    AUI_DSC_DATA_PURE,

    /**
    Value to indicate an invalid type of data source, and is used for checking
    the input/output parameters
    */
    AUI_DSC_DATA_TYPE_NB

} aui_dsc_data_type, *aui_p_dsc_data_type;

/**
Enum to specify the <b> Key Pattern </b> which indicates the pattern of the
<b> Control Word (CW) keys </b> provided either in DRAM or in the key ladder

@note When both the value #AUI_DSC_KEY_PATTERN_ODD and #AUI_DSC_KEY_PATTERN_EVEN
      are set, it means that both odd and even keys are provided
*/
typedef enum aui_dsc_key_pattern {

    /**
    Value to indicate that the <b> Single Key </b> is provided.

    @note When using this value all key storage type defined in the enum
          #aui_dsc_key_type will be supported.
    */
    AUI_DSC_KEY_PATTERN_SINGLE = 0,

    /**
    Value to indicate that the <b> Odd Key </b> is provided.

    @note  This value can be used @a only with key from DRAM.
    */
    AUI_DSC_KEY_PATTERN_ODD = (1<<0),

    /**
    Value to indicate that the <b> Even Key </b> is provided.

    @note  This value can be used @a only with key from DRAM.
    */
    AUI_DSC_KEY_PATTERN_EVEN = (1<<1),

    /**
    Value to indicate that <b> Odd & Even Keys </b> are provided.

    @note  When using this value all key storage type defined in the enum
           #aui_dsc_key_type will be supported
    */
    AUI_DSC_KEY_PATTERN_ODD_EVEN = AUI_DSC_KEY_PATTERN_ODD | AUI_DSC_KEY_PATTERN_EVEN

} aui_dsc_key_pattern;

/// @coding

/**
Enum to specify the different <b> version number </b> for <b> CSA algorithm </b>
*/
typedef enum csa_version {

   /**
    Value to indicate version number @b V1.0 for the CSA algorithm
    */
    AUI_DSC_CSA1 = 1,

    /**
    Value to indicate version number @b V2.0 for the CSA algorithm
    */
    AUI_DSC_CSA2 = 0,

    /**
    Value to indicate version number @b V3.0 for the CSA algorithm
    */
    AUI_DSC_CSA3 = 2

} aui_dsc_csa_version, *aui_p_dsc_csa_version;

/// @endcoding

/**
Enum to specify the different output length (in @b bit unit) for different SHA
algorithm
*/
typedef enum aui_sha_mode {

    /**
    Value to indicate the output length <b> 160 bits </b> for
    <b> SHA-1 algorithm </b>
    */
    AUI_SHA_1,

    /**
    Value to indicate the output length <b> 224 bits </b> for
    <b> SHA-224 algorithm </b>
    */
    AUI_SHA_224,

    /**
    Value to indicate the output length <b> 256 bits </b> for
    <b> SHA-256 algorithm </b>
    */
    AUI_SHA_256,

    /**
    Value to indicate the output length <b> 384 bits </b> for
    <b> SHA-384 algorithm </b>
    */
    AUI_SHA_384,

    /**
    Value to indicate the output length <b> 512 bits </b> for
    <b> SHA-512 algorithm </b>
    */
    AUI_SHA_512

} aui_sha_mode, aui_dsc_sha_mode;

/**
Enum to specify where the SHA data source are stored when encrypting/decrypting
*/
typedef enum aui_sha_source {

    /**
    Value to indicate that the SHA data source are stored in DRAM
    */
    AUI_SHA_SRC_FROM_DRAM,

    /**
    Value to indicate that the SHA data source are stored in Flash Memory
    */
    AUI_SHA_SRC_FROM_FLASH

} aui_sha_source, aui_dsc_sha_src;

/**
Enum to specify the different DSC IV modes.
*/
typedef enum aui_dsc_iv_mode {

    /**
    Value to indicate that in the cryptographic functions
    - #aui_dsc_encrypt
    - #aui_dsc_decrypt

    the DSC IV value will be reset to the initial value, i.e. the one set by
    the function #aui_dsc_attach_key_info2dsc.

    @note This DSC IV mode is set by default.
    */
    AUI_DSC_IV_MODE_RESET = 0,

    /**
    Value to indicate that in the cryptographic functions
    - #aui_dsc_encrypt
    - #aui_dsc_decrypt

    the DSC IV value will be calculated automatically in the DSC driver.
    */
    AUI_DSC_IV_MODE_AUTO,

    /**
    Value to indicate that the DSC IV value will be passed by the function
    #aui_dsc_iv_attr_set to be updated in the DSC driver.
    */
    AUI_DSC_IV_MODE_UPDATE

} aui_dsc_iv_mode;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Descrambler (DSC) Module </b> to specify the DSC Device
       attributes
       </div> @endhtmlonly

       Struct to specify the attributes of DSC Device
*/
typedef struct aui_st_attr_dsc {

    /**
    Member as @a Index which integer values (from zero (0)) refers to different
    DSC Devices

    @note @b 1. This member is @a only used by the function #aui_dsc_open
    @note @b 2. The function #aui_dsc_attach_key_info2dsc doesn't need to use
                this member, then its usage will not affect the function
    */
    unsigned char uc_dev_idx;

    /**
    Member to specify the type of data source which is the input of the DSC
    Device, as defined in the enum #aui_dsc_data_type.

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then its usage will not affect the function.
    */
    aui_dsc_data_type dsc_data_type;

    /**
    Member to specify where the key is stored, as defined in the enum
    #aui_dsc_key_type

    @note @b 1. This member is @a only used by the function #aui_dsc_open
    @note @b 2. The function #aui_dsc_attach_key_info2dsc doesn't need to use
                this member, then its usage will not affect the function
    */
    aui_dsc_key_type dsc_key_type;

    /**
    Member to specify the algorithm supported by DSC device to encrypt/decrypt
    data, as defined in the enum #aui_dsc_algo

    @note @b 1. This member is @a only used by the function #aui_dsc_open.
    @note @b 2. The function #aui_dsc_attach_key_info2dsc doesn't need to use
                this member, then its usage will not affect the function
    */
    aui_dsc_algo uc_algo;

    /**
    Member to specify the work mode that supports @a only the algorithms
    @b DES/TDES and @b AES

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't not need to use this member,
                then its usage will not affect the function.
    */
    aui_dsc_work_mode uc_mode;

    /**
    Member to specify an pointer to an array used for the protecting keys and
    the protected CW keys in DRAM.
    In case both odd and even CW keys are provided, the odd key is first.\n
    In case key is from KL Module, this member is not used.

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member, then
                is usage will not affect the function
    */
    unsigned char *puc_key;

    /**
    Member to specify the size (in @a bit unit) of the provided keys
    (i.e. <b> protecting keys + protected CW keys </b>).\n
    About the total length of the key, it is
    <b> odd key length + even key length </b>.\n

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    unsigned long ul_key_len;

    /**
    Member to specify the CW keys pattern
    (i.e. <b> Single, Odd, Even or Odd|Even </b>).

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc.
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function.
    */
    unsigned long ul_key_pattern;

    /**
    Member to specify the Initial Vector (IV) values, and its length depends
    of the used algorithm. In CTR mode, it used as a counter

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    const unsigned char *puc_iv_ctr;

    /**
    Member to specify the Key Parity and the Key Source attributes

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    aui_dsc_parity_mode en_parity;

    /**
    Member to specify the mode to process the residual blocks valid just for
    the CBC work mode

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    aui_dsc_residue_block en_residue;

    /**
    Member to specify the key position returned by the KL device.

    @warning  This member @a must be filled if KL Module is used

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    unsigned long ul_key_pos;

    /**
    Member to specify the different Crypt Mode (i.e. encryption/decryption)
    that the DSC Device can use for the input data source from TS Stream or
    Raw Data, as defined in the enum #aui_dsc_crypt_select

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    aui_dsc_crypt_select en_en_de_crypt;

    /**
    Member to specify the version number for CSA algorithm

    @warning   This member is ignored by other algorithms

    @note @b 1. This member is @a only used by the function
                #aui_dsc_attach_key_info2dsc
    @note @b 2. The function #aui_dsc_open doesn't need to use this member,
                then is usage will not affect the function
    */
    aui_dsc_csa_version csa_version;

    /**
    Member to specify the PID list for TS Streams

    @warning @b 1. This member is valid @a only with the value
                   #AUI_DSC_DATA_TS of the enum #aui_en_dsc_data_type
    @warning @b 2. This member is @a only used by the function
                   #aui_dsc_attach_key_info2dsc

    @note @b 1. The function #aui_dsc_open doesn't need to use this member
                which, however, will not affect it if used.
    @note @b 2. The function #aui_dsc_attach_key_info2dsc supports binding
                a set of key to multiple PIDs as long as these multple PIDs
                keep sharing the same keys.\n
                If some PIDs need to be added or removed at run-time, setting
                keys to multiple PIDs in one call of the function
                #aui_dsc_attach_key_info2dsc will make things being complicated.\n
                So it's recommended that only one PID is set when calling the
                function #aui_dsc_attach_key_info2dsc. Setting keys to multiple
                PIDs needs to call #aui_dsc_attach_key_info2dsc multiple times.\n
                That's why this member should point to a sigle PID.
    */
    unsigned short *pus_pids;

    /**
    Member to specify the total amount of PIDs

    @warning  @b 1. This member is @a only used by the function
                  #aui_dsc_attach_key_info2dsc

    @note @b 1. The function #aui_dsc_open doesn't need to use this member
                which, however, will not affect it if used.

    @note @b 2. The function #aui_dsc_attach_key_info2dsc supports binding
                a set of key to multiple PIDs as long as these multple PIDs
                keep sharing the same keys.\n
                If some PIDs need to be added or removed at run-time, setting
                keys to multiple PIDs in one call of the function
                #aui_dsc_attach_key_info2dsc will make things being complicated.\n
                So it's recommended that only one PID is set when calling the
                function #aui_dsc_attach_key_info2dsc. Setting keys to multiple
                PIDs needs to call #aui_dsc_attach_key_info2dsc multiple times.\n
                That's why this member should take the value @b 1.
    */
    unsigned long ul_pid_cnt;

} aui_attr_dsc, *aui_p_attr_dsc;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Descrambler (DSC) Module </b> to specify the setting
       of DSC IV.
       </div> @endhtmlonly

       Struct to specify the setting of DSC IV
*/
typedef struct aui_attr_dsc_iv {

    /**
    Member to specify the updated DSC IV mode, as defined in the enum
    #aui_dsc_iv_mode.
    */
    aui_dsc_iv_mode dsc_iv_mode;

    /**
    Member as pointer to the buffer of ODD IV to be updated in the DSC driver.

    @warning  This pointer is @a only valid when the DSC IV mode is set as
              #AUI_DSC_IV_MODE_UPDATE as defined in the enum #aui_dsc_iv_mode

    @note @b 1.  If this pointer is set to NULL value, ODD IV will not be updated
    @note @b 2.  This pointer should be set to NULL value if
                 - The DSC IV mode is set as #AUI_DSC_IV_MODE_UPDATE as defined in
                   the enum #aui_dsc_iv_mode and the data
                 - The DSC input data is set as #AUI_DSC_DATA_PURE as defined in
                   the enum #aui_en_dsc_data_type
    */
    unsigned char *iv_odd;

    /**
    Member as pointer to the buffer of EVEN IV to be updated in the DSC driver.

    @warning  This pointer is @a only valid when the DSC IV mode is set as
              #AUI_DSC_IV_MODE_UPDATE as defined in the enum #aui_dsc_iv_mode

    @note @b 1. If this pointer is set to NULL value, EVEN IV will not be updated
    @note @b 2. This pointer should be properly set if
                - The DSC IV mode is set as #AUI_DSC_IV_MODE_UPDATE as defined
                  in the enum #aui_dsc_iv_mode and the data
                - The DSC input data is set as #AUI_DSC_DATA_PURE as defined in
                  the enum #aui_en_dsc_data_type
    */
    unsigned char *iv_even;

    /**
    Member as pointer to the PID list for TS Stream that will be updated to DSC
    IV values

    @warning  This member is valid @a only with the value
              - #AUI_DSC_DATA_TS of the enum #aui_en_dsc_data_type
              - #AUI_DSC_IV_MODE_UPDATE of the enum #aui_dsc_iv_mode
    @warning  in the function #aui_dsc_attach_key_info2dsc

    @note     The function #aui_dsc_attach_key_info2dsc supports binding a set of
              keys (DSC IV values) to multiple PID as long as these multiple PID
              keep sharing the same keys (DSC IV values).\n
              So the function #aui_dsc_iv_attr_set also updates the corresponding
              DSC IV values through the PID list, which @a must have been attached
              by the function #aui_dsc_attach_key_info2dsc first.
    */
    unsigned short *pids;

    /**
    Member to specify the total amount of PIDs in PID list

    @warning  This member is valid
              @a only with the value
              - #AUI_DSC_DATA_TS of the enum #aui_en_dsc_data_type
              - #AUI_DSC_IV_MODE_UPDATE of the enum #aui_dsc_iv_mode.
              in the function #aui_dsc_attach_key_info2dsc
    */
    unsigned long pid_cnt;

} aui_attr_dsc_iv, *aui_p_attr_dsc_iv;


/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Descrambler (DSC) Module </b> to specify the process
        status of the the descrambler
        </div> @endhtmlonly

        Struct to specify the process status of the the descrambler. the status
        information can be used by PVR module to know the position where key
        change have been made.

@note   This struct is intended @a only for projects based on
        <b> Linux OS </b>.
*/
typedef struct aui_dsc_process_status {

    /**
    Member to specify the number of blocks encrypted by the scrambler using the
    old DSC configuration.\n
    In particular, this member is a continous counter of processed data blocks
    in the scrambler and is increased by one every time one block of TS data
    is encrypted.\n

    @attention  Setting new encryption key will not reset this counter

    @note  The default data block size is 188 x 256 bytes
    */
    unsigned long ul_block_count;

} aui_dsc_process_status;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Descrambler (DSC) Module </b> to specify the key
       parameters on changing key ladder for PVR encryption
       </div> @endhtmlonly

       Struct to specify the key parameter on changing key ladder for PVR
       encryption
*/
typedef struct aui_dsc_encrypt_kl_param {

    /**
    Member to specify the <b> Root key index </b>
    */
    int rootkey_index;

    /**
    Member to specify the <b> KL type </b> as per the enum values
    - #AUI_KL_TYPE_ALI
    - #AUI_KL_TYPE_ETSI
    */
    int key_ladder_type;

    /**
    Member to specify the <b> KL algorithm </b> as per the enum values
    - #AUI_KL_ALGO_TDES
    - #AUI_KL_ALGO_AES
    */
    int kl_algo;

    /**
    Member to specify the <b> KL run level </b>, which can be equal to the
    values @b 1, @b 2, @b 3 or @b 5
    */
    int kl_level;

    /**
    Member to specify the <b> Three level control word </b> which lenght is
    48 byte

    @note  @a Only odd or even control word
    */
    unsigned char *control_word;

} aui_dsc_encrypt_kl_param;

/**
Enum to specify the DSC command list to be used to configure the DSC Device,

@attention  This enum is @a reserved to ALi R&D Dept. then user can ignore it
*/
typedef enum aui_dsc_item_set {

    /**
    Value to indicate an Invalid Configuration Command, and is used for checking
    input/output parameters
    */
    AUI_DSC_SET_CMD_NB

} aui_dsc_item_set;

/**
Enum to specify the DSC command list to be used to get DSC Device Information
*/
typedef enum aui_dsc_item_get {

    /**
    Value to indicate the command used to get the type of the data source that
    is under processing by the DSC Device
    */
    AUI_DSC_GET_DATA_TYPE,

    /**

    Value to indicate an Invalid getting command, and is used for checking the
    input/output parameters
    */
    AUI_DSC_GET_CMD_NB

} aui_dsc_item_get;

/**
Enum to specify the process mode of a DSC device
*/
typedef enum aui_dsc_process_mode {

    /**
    Value to indicate that the process mode is undefined.\n\n

    When a DSC device is opened and configured,
    - It  can be used for RAM-to-RAM pure data processing
    - It can be used by a DMX device to process TS data
    */
    AUI_DSC_PROCESS_MODE_UNDEFINED,

    /**
    Value to indicate that the DSC device will be used to encrypt data in
    block mode.\n\n

    In this mode, the type of data source as input of DSC Module @b can be
    - Either #AUI_DSC_DATA_PURE
    - Or #AUI_DSC_DATA_TS

    as defined in the enum #aui_dsc_key_type. The DSC module will be used by
    the DMX device to encrypt data for recording. The key for DSC device should
    be configured by the function #aui_dsc_update_pvr_encrypt_key_info

    @warning  This mode @a must be set to DSC device before to be set to the
              DMX device by the function #aui_dmx_data_path_set
    */
    AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT,

    /**
    Value to indicate that the DSC device will be used to decrypt data in block
    mode. \n\n

    This mode is used to decrypt the data encrypted when the process mode is set
    to #AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT as defined in the present enum.

    In this mode, the type of data source as input of DSC Module @b can be
    - Either #AUI_DSC_DATA_PURE
    - Or #AUI_DSC_DATA_TS

    as defined in the enum #aui_dsc_key_type. The DSC module will be used by
    the DMX device to decrypt data for AV playback. The key for DSC device should
    be configured by the function #aui_dsc_attach_key_info2dsc

    @warning  This mode @a must be set to the DSC device before to be set to the
              DMX device by the function #aui_dmx_data_path_set
    */
    AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT,

    /**
    Value to indicate that the DSC device will be used to encrypt data in TS
    mode.

    In this mode, the type of data source as input of DSC Module @b must be
    #AUI_DSC_DATA_TS as defined in the enum #aui_dsc_key_type. The DSC module
    will be used by the DMX device to encrypt data for recording. The key for
    DSC device should be configured by the function #aui_dsc_attach_key_info2dsc

    @warning  This mode @b must be set to the DSC device before to be set to the
              DMX device by the function #aui_dmx_data_path_set
    */
    AUI_DSC_PROCESS_MODE_TS_ENCRYPT,

    /**
    Value to indicate that the DSC device will be used to decrypt data in TS
    mode.\n\n

    This mode is used to decrypt the data encrypted when the process mode is set
    to #AUI_DSC_PROCESS_MODE_TS_ENCRYPT as defined in the present enum.

    In this mode, the type of data source as input of DSC Module @b must be
    #AUI_DSC_DATA_TS as defined in the enum #aui_dsc_key_type. The DSC module
    will be used by the DMX device to decrypt TS data. The key for DSC should
    be configured by the function #aui_dsc_attach_key_info2dsc

    @warning  This mode @b must be set to the DSC device before to be set to the
              DMX device by the function #aui_dmx_data_path_set
    */
    AUI_DSC_PROCESS_MODE_TS_DECRYPT

} aui_dsc_process_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Descrambler (DSC) Module </b> to specify the
        attributes for processing TS data for the descrambler.
        </div> @endhtmlonly

        Struct to specify the process attributes of the the descrambler.\n\n

        The descrambler will encrypt and decrypt TS data in blocks.
        This struct specify how many TS packet need to be grouped into a block.

@note   This struct is intended @a only for projects based on
        <b> Linux OS </b>
*/
typedef struct aui_dsc_process_attr {

    /**
    Member to specify the process mode of the DSC device.

    @note  This value @b must be selected correctly based on CAS.
    */
    aui_dsc_process_mode process_mode;

    /**
    Member to specify the size of a block (in bytes unit).\n\n

    The DSC device will encrypt and decrypt TS data in blocks.
    The scrambler will return in the structure #aui_dsc_process_status the
    total number of blocks have been processed

    @note  For block encryption, the default block size is <b> 188 x 256 bytes </b>
           (the block size @b must be a multiple of 188 bytes).\n\n

    @note  For pure data decription in CBC mode
           - If the <b> block size > 0 </b>,
             DSC device will reset IV to the initial value before decrypting
             @b ul_block_size bytes of data automatically
           - if the <b> block size = 0 </b>,
             DSC will not reset IV to th initial value automatically.
             The application needs to set new IV manually.
    */
    unsigned long ul_block_size;

} aui_dsc_process_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Descrambler (DSC) Module </b> to specify
        the identifier of DSC resources attached to AV Module
        </div> @endhtmlonly

        Struct to specify the identifier of DSC resources attached to
        AV Module by the function #aui_av_dsc_context_set.

@note   In multi-process environment, the DSC #aui_hdl handle cannot be passed
        from a process to another one so this struct is used to represent the
        DSC resources for AV Module. And this structure can be recognized by
        the AV Module in another process.
*/
typedef struct aui_dsc_resource_id {

    /**
    Member as an array of DSC identifiers
    */
    unsigned char identifier[16];

} aui_dsc_resource_id;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the version number of DSC Module as defined
                in the macro #AUI_MODULE_VERSION_NUM_DSC

@warning        This function can be used at any time

@param[out]     pul_version         = Pointer to the version number of DSC Module
                                                               .
@return         @b AUI_RTN_SUCCESS  = Getting of the version number of DSC Module
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = The output parameter (i.e. [out]) is
                                      invalid
@return         @b Other_Values     = Getting of the version number of DSC Module
                                      failed for some reasons
*/
AUI_RTN_CODE aui_dsc_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to open the DSC Device and configure the desired
                attributes then get the related handle

@warning        This function can @a only be used in the <b> Pre-run Stage </b>
                of the DSC device, in particular after performing the
                initialization of the DSC Module by the function #aui_dsc_init
                for the first opening of the DSC device

@param[in]      *p_attr_dsc             = Pointer to the struct #aui_attr_dsc
                                          which collects the attribute of the
                                          DSC Device

@param[out]     *pp_hdl_dsc             = #aui_hdl pointer to the handle of
                                          the DSC device just opened

@return         @b AUI_RTN_SUCCESS      = DSC device opened successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Opening of the DSC device failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dsc_open (

    const aui_attr_dsc *p_attr_dsc,

    aui_hdl* const pp_hdl_dsc

    );

/**
@brief          Function used to close the DSC module already opened by function
                #aui_dsc_open and then the related device handle will be released
                (i.e. the related resources such as memory, device).

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of the DSC Device in pair with its opening by the function
                #aui_dsc_open. After closing the DSC Device, user can
                - Either perform the de-initialization of the DSC Module by the
                  function #aui_dsc_de_init
                - Or open again the DSC device by function #aui_dsc_open,
                  considering the initialization of DSC Module has been
                  performed previously by the function #aui_dsc_init

@param[in]      p_hdl_dsc               = #aui_hdl handle of the DSC Module
                                          already opened and to be closed.

@return         @b AUI_RTN_SUCCESS      = DSC Device closed successfully
@return         @b AUI_RTN_EINVAL       = The input parameters (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Closing of the DSC Device failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dsc_close (

    aui_hdl p_hdl_dsc

    );

/**
@brief          Function used to configure the key attributes and attach the
                related keys to the opened DSC Device

@param[in]      p_hdl_dsc               = #aui_hdl handle of the DSC Device
                                          already opened
@param[in]      *p_attr_dsc             = Pointer to the struct #aui_attr_dsc
                                          which collects the DSC attributes of
                                          the keys to be attached to opened DSC
                                          Device

@return         @b AUI_RTN_SUCCESS      = Configuration of the key attribute
                                          and attachment performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Configuration of the key attribute
                                          and attachment failed for some reasons

@note           Each key needs to be independently attached, and a key can be
                updated by calling this same function with the updated parameters
                (i.e. the keys are re-attached).\n\n

                @b Example: One (1) program/channel (P1) of a TS stream with four
                            (4) PIDs and two (2) set of keys (key1, key2):

                - @b Attaching of the keys:
                  - @b key1 for decrypting <b> PID 0x10 </b> and <b> PID 0x11 </b>
                  - @b key2 for decrypting <b> PID 0x20 </b> and <b> PID 0x21 </b>
                - @b Updating of the keys as below:
                  - #aui_dsc_attach_key_info2dsc (key1, pids[0x10,0x11])
                  - #aui_dsc_attach_key_info2dsc (key2, pids[0x20,0x21])
*/
AUI_RTN_CODE aui_dsc_attach_key_info2dsc (

    aui_hdl p_hdl_dsc,

    const aui_attr_dsc *p_attr_dsc

    );

/**
@brief          Function used to de-attach the key resources away from DSC Devices
                by PID.\n
                When user do not play current encrypted program/channel and want
                to play another one with a different PID, the current encrypted
                program/channel key attached by the function
                #aui_dsc_attach_key_info2dsc should de-attached away from the
                DSC Device.\n
                Then the function #aui_dsc_attach_key_info2dsc need to be called
                to attach the new program/channel key to DSC device to play the
                new encrypted program/channel.

@param[in]      p_hdl_dsc               = Pointer to the handle of the
                                          DSC Module already opened
@param[in]      us_pid                  = PID of the current encrypted program/
                                          channel that user doesn't want to
                                          continue playing

@return         @b AUI_RTN_SUCCESS      = Current encrypted program/channel key
                                          de-attached successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = De-attaching of the current encrypted
                                          program/channel failed for some reasons

@note           When changing a program, the previous keys can be detached.\n\n
                @b Example: Two (2) program/channel (P1 and P2) of a TS Stream
                            with four (4) PIDs and two (2) set of keys.

                1. @b Attaching of the keys of @b P1:
                    - @b key1 for decrypting <b> PID 0x10 </b> and <b> PID 0x11 </b>
                    - @b key2 for decrypting <b> PID 0x20 </b> and <b> PID 0x21 </b>
                2. @b Attaching of the keys of @b P2:
                   - @b key3 for decrypting <b> PID 0x30 </b> and <b> PID 0x31 </b>
                   - @b key4 for decrypting <b> PID 0x40 </b> and <b> PID 0x41 </b>
                3. <b> Playing P1 </b> (two attachments are required):
                   - #aui_dsc_attach_key_info2dsc (key1, pids[0x10,0x11])
                   - #aui_dsc_attach_key_info2dsc (key2, pids[0x20,0x21])
                4. <b> Switching to @b P2 </b> (key1 & key2 are detached and the
                   new keys can be attached):
                   - #aui_dsc_deattach_key_by_pid ([0x10])\n
                     @b Note: The PID 0x11 will be deleted automatically as well,
                              as it is set together with the PID 0x10 in the same
                              call of the function #aui_dsc_attach_key_info2dsc
                   - #aui_dsc_deattach_key_by_pid ([0x20])\n
                     @b Note: The PID 0x21 will be deleted automatically as well,
                              as it is set togather with the PID 0x20 in the same
                              call of the function #aui_dsc_attach_key_info2dsc
                   - #aui_dsc_attach_key_info2dsc (key3, pids[0x30,0x31])
                   - #aui_dsc_attach_key_info2dsc (key4, pids[0x40,0x41])
*/
AUI_RTN_CODE aui_dsc_deattach_key_by_pid (

    aui_hdl p_hdl_dsc,

    unsigned short us_pid

    );

/**
@brief          Function used to encrypt Raw Data


@pre            The following operations need to be performed before calling
                this function:
                - Open a DSC Device and configure its attributes by the function
                  #aui_dsc_open
                - Configure the key attributes and attach the related keys to
                  the opened DSC Device by the function #aui_dsc_attach_key_info2dsc

@param[in]      p_hdl_dsc               = #aui_hdl pointer to the handle of the
                                          DSC Module already opened
@param[in]      *puc_data_in            = Pointer to the clear Raw Data to be
                                          encrypted by DSC Device
@param[in]      ul_data_len             = Length (in @a byte unit) of the clear
                                          Raw Data to be encrypted by DSC Device

@param[out]     *puc_data_out           = Pointer to the just encrypted Raw Data

@return         @b AUI_RTN_SUCCESS      = Encrypting of the clear Raw Data
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Encrypting of the clear Raw Data
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dsc_encrypt (

    aui_hdl p_hdl_dsc,

    unsigned char *puc_data_in,

    unsigned char *puc_data_out,

    unsigned long ul_data_len

    );

/**
@brief          Function used to de-crypt Raw Data

@pre            The following operations need to be performed before calling
                this function:
                - Open a DSC Device and configure its attributes by the
                  function #aui_dsc_open
                - Configure the key attributes and attach the related keys to
                  the opened DSC Device by the function #aui_dsc_attach_key_info2dsc

@param[in]      p_hdl_dsc               = #aui_hdl pointer to the handle of the
                                          DSC Module already opened
@param[in]      *puc_data_in            = Pointer to the encrypted Raw Data to
                                          be decrypted by DSC Device
@param[in]      ul_data_len             = Length (in @a byte unit) of the
                                          encrypted Raw Data to be decrypted
                                          by DSC device

@param[out]     *puc_data_out           = Pointer to the just decrypted Raw Data
                                          (i.e. Clear Raw Data)

@return         @b AUI_RTN_SUCCESS      = De-crypting of the encrypted Raw Data
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = De-crypting of the encrypted Raw Data
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dsc_decrypt (

    aui_hdl p_hdl_dsc,

    unsigned char *puc_data_in,

    unsigned char *puc_data_out,

    unsigned long ul_data_len

    );

/**
@brief          Function used for SHA processing of the input Raw Data

@attention      This function could be used after the initialization by the
                function #aui_dsc_init so not need to open a independent DSC
                Device by the function #aui_dsc_open

@param[in]      ul_source               = Flag to denote the data source, in
                                          particular:
                                          - @b 0 = SDRAM
                                          - @b 1 = Flash Memory
@param[in]      puc_data_in             = Pointer to the input Raw Data to be
                                          SHA processed
@param[in]      ul_data_len             = Length (in @a byte unit) of the input
                                          Raw Data to be SHA processed by DSC
                                          Device. The max length is 64M bytes.
@param[in]      ul_sha_mode             = The SHA mode, as defined in the enum
                                          #aui_dsc_sha_mode

@param[out]     puc_data_out            = Pointer to the output of the SHA process

@return         @b AUI_RTN_SUCCESS      = SHA process performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = SHA process failed for some reasons

@note           For projects based on <b> Linux OS </b>, this function is preceded/
                followed by other functions as below:
                - #aui_dsc_get_buffer
                - #aui_dsc_sha_digest
                - #aui_dsc_release_buffer
@note           For project based on <b> TDS OS </b>, instead, this function is
                not preceded/followed by other functions, i.e.
                - #aui_dsc_sha_digest
*/
AUI_RTN_CODE aui_dsc_sha_digest (

    unsigned long ul_source,

    unsigned char *puc_data_in,

    unsigned long ul_data_len,

    unsigned long ul_sha_mode,

    unsigned char *puc_data_out

    );

/**
@brief          Function used to perform the initialization of the DSC Module
                before its opening by the function #aui_dsc_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the DSC Module

@param[in]      p_call_back_init        = Callback function used to initialize
                                          the DSC Module, as per comment for
                                          the function pointer #p_fun_cb
@param[in]      pv_param                = The input parameter of the callback
                                          function @b p_call_back_init which is
                                          the first parameter [in] of this
                                          function

@return         @b AUI_RTN_SUCCESS      = DSC Module initialized successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Initializing of the DSC Module failed
                                          for some reasons

@note           About the callback function @b p_call_back_init as input
                parameter:
                - In projects based on <b> Linux OS </b>, it is suggested to
                  set as @b NULL
                - In projects based on <b> TDS OS </b>, it will perform some
                  DSC Device attachments and configurations, please refer to
                  the sample code of the initialization for more clarifications
*/
AUI_RTN_CODE aui_dsc_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the DSC Module,
                after its closing by the function #aui_dsc_close.

@param[in]      p_call_back_init        = Callback function used to de-initialize
                                          the DSC Module as per comment for the
                                          function pointer #p_fun_cb
@param[in]      pv_param                = The input parameter of the callback
                                          function @b p_call_back_init which
                                          is the first parameter [in] of this
                                          function

@return         @b AUI_RTN_SUCCESS      = DSC Module de-initialized successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = De-initializing of the DSC Module
                                          failed for some reasons

@note           This function can be used @a only in projects based on
                <b> TDS OS </b>
*/
AUI_RTN_CODE aui_dsc_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform a specific setting of the DSC Module,
                as defined in the enum #aui_en_dsc_item_set, after its opening
                and starting by, respectively. the functions #aui_dsc_open and
                #aui_dsc_start

@attention      This function is currently reserved to ALi R&D Dept. then user
                can ignore it

@param[in]      p_hdl_dsc               = #aui_hdl handle of the DSC Module
                                          already opened and to be managed to
                                          perform a specific setting
@param[in]      ul_item                 = The item related to the specific
                                          setting of the DSC Module to be
                                          performed, as defined in the enum
                                          #aui_dsc_item_set
@param[in]      *pv_param               = The pointer as per the description of
                                          the specific setting of the DSC Module
                                          to be performed, as defined in the
                                          enum #aui_dsc_item_set

@return         @b AUI_RTN_SUCCESS      = Specific setting of the DSC Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Specific setting of the DSC Module
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dsc_set (

    aui_hdl p_hdl_dsc,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific information of the DSC Module,
                as defined in the enum #aui_dsc_item_get, after its opening and
                starting by, respectively, the functions #aui_dsc_open and
                #aui_dsc_start

@param[in]      p_hdl_dsc               =  #aui_hdl handle of the DSC Module
                                           already opened and to be managed to
                                           get a specific information, as defined
                                           in the enum #aui_dsc_item_get
@param[in]      ul_item                 =  The item related to the specific
                                           information of the DSC Module to be
                                           gotten, as defined in the enum
                                           #aui_dsc_item_get

@param[out]     *pv_param               =  The pointer as per the description of
                                           the specific information of the DSC
                                           Module received, as defined in the
                                           enum #aui_dsc_item_get

@return         @b AUI_RTN_SUCCESS      =  Getting of the specific information
                                           of the DSC Module performed successfully
@return         @b AUI_RTN_EINVAL       =  At least one parameter (i.e. [in],
                                           [out]) is invalid
@return         @b Other_Values         =  Getting of the specific information
                                           of the DSC Module failed for some
                                           reasons
*/
AUI_RTN_CODE aui_dsc_get (

    aui_hdl p_hdl_dsc,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to generate HMAC Messages. The intention of this function
                is to provide an API to generate HMAC message that is bond to the SOC
                unique chip ID. The same data will have different HMAC on different platforms.

@note           This function is not the standard HMAC algorithm.

@param[in]      puc_input               = Pointer to the Input Data
@param[in]      ul_length               = Length (in @a byte unit) of Input
                                          Data
@param[in]      puc_output              = Pointer to the Output Data (i.e. HMAC
                                          Message which length is 32 bytes)

@param[out]     puc_key                 = Pointer to the Input HMAC Key (which
                                          length is 16 bytes) used for generating
                                          the HMAC message
                                          - @b Caution: Different keys will output
                                               different HMAC messages

@return         @b AUI_RTN_SUCCESS      = HMAC Message generated successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Generating of the HMAC Message failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dsc_generate_HMAC (

    unsigned char *puc_input,

    unsigned long ul_length,

    unsigned char *puc_output,

    unsigned char *puc_key

    );

/**
@brief          Function used to encrypt the Boot Loader Universal Key

@param[in]      handle                  = #aui_hdl handle of the DSC Module
                                          already opened
@param[in]      puc_input_key           = Pointer to the Clear Boot Loader
                                          Universal Key to be encrypted (which
                                          length is fixed at 128 bits)
@param[in]      puc_r_key               = Pointer to the Random Key to be
                                          encrypted (which length is fixed at
                                          128 bits)
@param[in]      root_key_index          = Type of encryption to be used, such as:
                                          - @b 0 = Use key 6
                                                   (i.e. #AUI_KL_ROOT_KEY_0_6)
                                          - @b 1 = Use key 6_r
                                                   (i.e. #AUI_KL_ROOT_KEY_0_6_R)
                                          - @b 2 = Use key 7
                                                   (i.e. #AUI_KL_ROOT_KEY_0_7)

@param[out]     puc_output_key          = Pointer to the encrypted Boot Loader
                                          Universal Key (which length is fixed
                                          at 128bits)

@return         @b AUI_RTN_SUCCESS      = Boot Loader Universal Key encrypted
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Encrypting of the Boot Loader Universal
                                          Key failed for some reasons

@note           After performing the initialization by the function
                #aui_dsc_init, below steps could be followed to encrypt the Boot
                Loader Universal Key:
                - Call the function #aui_dsc_open to open a new DSC Device,
                  where the member @b dsc_key_type and @b uc_algo of the struct
                  #aui_attr_dsc as its input parameter can be any value
                - Call the function #aui_dsc_encrypt_bl_uk to encrypt the Clear
                  Boot Loader Universal Key
                - Call the function #aui_dsc_close to close the DSC Device
*/
AUI_RTN_CODE aui_dsc_encrypt_bl_uk (

    aui_hdl handle,

    unsigned char *puc_input_key,

    unsigned char *puc_r_key,

    unsigned char *puc_output_key,

    unsigned int root_key_index

    );

/**
@brief          Function used to get an user space buffer reserved for SHA process

@warning        This function can @a only be used @b before the function
                #aui_dsc_sha_digest and @a only in projects based on <b> Linux
                OS </b>

@param[in]      ul_size                 = Size of the user space to get
                                          - @b Caution: The size @a must be less
                                                        than @b __G_ALI_MM_VIDEO_SIZE,
                                                        which value depends of
                                                        the targeted ALi board
                                                        and user can get it from
                                                        kernel's configuration

@param[in]      **pp_buf                = Pointer to the user space buffer to get

@return         @b AUI_RTN_SUCCESS      = Getting of the user space buffer
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameters (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Getting of the user space buffer
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dsc_get_buffer (

    unsigned long ul_size,

    void **pp_buf

    );

/**
@brief          Function used to release the user space buffer already gotten
                by the function #aui_dsc_get_buffer.

@warning        This function can @a only be used @b after the function
                #aui_dsc_sha_digest and @a only in projects based on <b> Linux
                OS </b>

@param[in]      ul_size                 = Size of the user space to be released
                                          - @b Caution: The size @a must be the
                                                        same as the size declared
                                                        in the function
                                                        #aui_dsc_get_buffer
@param[in]      *p_buf                  = Pointer to the user space buffer to be
                                          release

@return         @b AUI_RTN_SUCCESS      = Releasing of the user space buffer
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Releasing of the user space buffer
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dsc_release_buffer (

    unsigned long ul_size,

    void *p_buf

    );

/**
@brief          Function used to configue and update the key attributes as well
                as update the related keys to the opened DSC and KL Device.
                This function allows changing the key and algorithm on-the-fly
                without closing and reopening the DSC and KL device.

@note           This function returns (as output parameter) the process status
                of the descrambler which can be used by PVR to know the position
                of the key change.

@param[in]      handle                  = #aui_hdl handle of the DSC Device
                                          already opened
@param[in]      p_dsc_attr              = Pointer to the struct #aui_attr_dsc
                                          which collects the DSC attributes of
                                          the keys to be attached to the opened
                                          DSC Device
@param[in]      p_kl_param              = Pointer to the struct
                                          #aui_dsc_encrypt_kl_param which
                                          collects the KL attributes of the keys

@param[out]     p_encrypt_status        = Pointer to the struct
                                          #aui_dsc_process_status which specify
                                          the process status of the descrambler

@return         @b AUI_RTN_SUCCESS      = Configuration and updating of the key
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Configuration and updating of the key
                                          failed for some reasons

@warning        This function is used for PVR re-encryption only. \n
                Normal live TS stream decryption/encryption should call the
                function #aui_dsc_attach_key_info2dsc to update the keys
*/
AUI_RTN_CODE aui_dsc_update_pvr_encrypt_key_info (

    aui_hdl handle,

    aui_attr_dsc *p_dsc_attr,

    aui_dsc_encrypt_kl_param *p_kl_param,

    aui_dsc_process_status *p_encrypt_status

    );

/**
@brief          Function used to configure the attributes for processing TS
                data.

@note           This function should be called <b> only once </b> before
                starting processing the data.

@param[in]      handle                  = #aui_hdl handle of the DSC Device
                                          already opened

@param[in]      p_process_attr          = Pointer to the struct
                                          #aui_dsc_process_attr which specify
                                          the attributes for TS processing.

@return         @b AUI_RTN_SUCCESS      = Setting the attribute performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting the attribute failed for some reasons
*/
AUI_RTN_CODE aui_dsc_process_attr_set (

    aui_hdl handle,

    aui_dsc_process_attr *p_process_attr

    );

/**
@brief          Function used to get the structure #aui_dsc_resource_id, which is to
                identify the DSC resources attached to AV Module, for setting up
                the descrambling context of AV Module.\n
                The structure #aui_dsc_resource_id then can be passed to AV Module
                in another process (in multi-process environment).

@warning        This function can only be called after the current DSC device is
                set by the function #aui_dsc_process_attr_set

@param[in]      handle                    = #aui_hdl handle of the specific DSC
                                            device already opened and configured
                                            by the function
                                            #aui_dsc_attach_key_info2dsc

@param[out]     p_resource_id             = Pointer to a struct
                                            #aui_dsc_resource_id which
                                            collects the identifiers of the DSC
                                            resources attached to AV Module

@return         @b AUI_RTN_SUCCESS        = Getting of the structure
                                            #aui_dsc_resource_id performed
                                            successfully
@return         @b AUI_RTN_EINVAL         = At least one parameter (i.e. [in],
                                            [out]) is invalid
@return         @b Others_Values          = Getting of the structure
                                            #aui_dsc_resource_id failed for some
                                            reasons

@note           This function is used @a only in projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_dsc_resource_id_get (

    aui_hdl handle,

    aui_dsc_resource_id *p_resource_id

    );

/**
@brief          Function used to update the setting of DSC IV.

@warning        This function can @a only be used after setting the current
                DSC device, i.e. after the function #aui_dsc_attach_key_info2dsc

@note           As general user scenario, the function
                #aui_dsc_attach_key_info2dsc is used to configure all
                attributes of DSC then also IV.\n
                This function can be used to update only DSC IV.\n
                Otherwise, user can opt for
                - <b> Default Mode </b>: DSC IV will be reset to the initial
                                         value, i.e. the one set by the function
                                         #aui_dsc_attach_key_info2dsc
                - <b> Automatic Mode </b>: DSC IV will be calculated automatically
                                           by DSC Driver

@param[in]      handle                    = #aui_hdl handle of the specific DSC
                                            device already opened and configured
                                            by the function
                                            #aui_dsc_attach_key_info2dsc
@param[in]      p_iv_attr                 = Pointer to a struct #aui_attr_dsc_iv
                                            which collects the setting of DSC IV

@note           This function is used @a only in projects based on <b> Linux OS </b>
*/
AUI_RTN_CODE aui_dsc_iv_attr_set (

    aui_hdl handle,

    aui_attr_dsc_iv *p_iv_attr

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_dsc_algo_em aui_dsc_algo

#define aui_dsc_work_mode_em aui_dsc_work_mode

#define aui_dsc_parity_mode_em aui_dsc_parity_mode

#define aui_dsc_residue_block_em aui_dsc_residue_block

#define aui_dsc_crypt_select_em aui_dsc_crypt_select

// #define csa_version aui_csa_version

/**
enum csa_version {

    AUI_CSA1=1,

    AUI_CSA2=0,

    AUI_CSA3=2
};

*/

#define AUI_CSA1 AUI_DSC_CSA1

#define AUI_CSA2 AUI_DSC_CSA2

#define AUI_CSA3 AUI_DSC_CSA3

/**
@brief          Function used to set the key changing for performing the
                re-encryption in SRAM mode

@param[in]      p_hdl_dsc               = #aui_hdl handle of the DSC Module
                                          already opened
@param[in]      puc_keys_start_ptr      = Pointer to the start key
@param[in]      keys_num                = Total numeber of keys
@param[in]      key_length_bit          = Length of a key (in @a bits unit)
@param[in]      quantum_num_per_key     = Quantum (47K) number per key

@return         @b AUI_RTN_SUCCESS      = Setting of the key changing performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = Setting of the key changing failed for
                                          some reasons

@note           @b 1. Setting a set of keys for performing the re-encryption in
                      SRAM mode, each key will encrypt TS data
                      (considering @b quantum_num_per_key * 47K) and then will
                      change to next key.\n
                      The first key should be the same as the key attached

@note           @b 2. This function is @a only avalable for projects based on
                      <b> Linux OS </b>

@attention      This function is not ready to be used yet, please contact
                ALi R&D Dept. for further information
*/
AUI_RTN_CODE aui_dsc_set_sram_change_key_param (

    aui_hdl p_hdl_dsc,

    unsigned char *puc_keys_start_ptr,

    unsigned int keys_num,

    unsigned int key_length_bit,

    unsigned int quantum_num_per_key

    );

#endif

/// @endcond

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


