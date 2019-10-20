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
Current ALi author: Wesley.He
Last update:        2017.02.25
-->

@file   aui_otp.h

@brief  One Time Program (OTP) Module

        <b> One Time Program (OTP) Module </b> is simply used to support OTP
        content and AUI read/write Functions for Advanced Security (AS) users.\n\n

        The OTP Module stores some fixed information such as
        - Manufacture ID
        - AS Configuration
        - Root Key
        - etc.

        which should be programmed during the production.\n\n

        If it is necessary to read/write OTP content from/into certain chips,
        please refer to ALi chip data sheet: generally the OTP data bit is
        different for each ALi chips.

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_OTP_H

#define _AUI_OTP_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List******************************/

/**
Macro to specify the <b> version number </b> of OTP Module, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_OTP  (0X00010000)

/**
Macro to specify that presently the structs contained in this header file are
designed to support up to one (1) OTP device
*/
#define AUI_OTP_DEV_CNT_MAX         (1)

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C"

{

#endif

/**
@brief        Function used to get the version number of the OTP Module as
              defined in the macro #AUI_MODULE_VERSION_NUM_OTP

@param[out]   pul_version        = Pointer to the version number of the OTP Module

@return       @b AUI_RTN_SUCCESS = Getting of the version number of OTP Module
                                   performed successfully
@return       @b AUI_RTN_EINVAL  = The output parameter (i.e. [out]) is invalid
@return       @b Other_Values    = Getting of the version number of OTP Module
                                   failed for some reason
*/
AUI_RTN_CODE aui_otp_version_get (

    unsigned long *pul_version

    );

/**
@brief        Function used to perform the initialization of the OTP Module
              before reading and writing OTP Data by, respectively, the
        functions #aui_otp_read and #aui_otp_write

@param[in]    p_call_back_init   = Callback function used for the initialization
                                   of the OTP Module, as per comment for the
                                   function pointer #p_fun_cb
@param[in]    pv_param           = Input parameter of the callback function
                                   @b p_call_back_init

@return       @b AUI_RTN_SUCCESS = OTP module initialized successfully
@return       @b AUI_RTN_EINVAL  = At least one input parameter (i.e. [in]) is
                                   invalid
@return       @b Other_Values    = Initializing of the OTP Module failed for
                                   some reasons
*/
AUI_RTN_CODE aui_otp_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief        Function used to perform the de-initialization of the OTP Module
              after reading and writing OTP Data by, respectively, the functions
              #aui_otp_read and #aui_otp_write.

@param[in]    p_call_back_init   = Callback function used to de-initialize the
                                   OTP Module, as per comment for the function
                                   pointer #p_fun_cb
@param[in]    pv_param           = Input parameter of the callback function
                                   @b p_call_back_init

@return       @b AUI_RTN_SUCCESS = OTP Module de-initialized successfully
@return       @b AUI_RTN_EINVAL  = At least one input parameter (i.e. [in]) is
                                   invalid
@return       @b Other_Values    = De-initializing of the OTP Module failed for
                                   some reasons
*/
AUI_RTN_CODE aui_otp_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief        Function used to read data from the OTP device

@param[in]    ul_addr            =  Address of the OTP device to be read
@param[out]   puc_data           =  Pointer to the buffer containing the OTP
                                    data just read
@param[in]    ul_data_len        =  Length of the OTP data to be read

@return       @b AUI_RTN_SUCCESS = Reading of the data from the OTP device
                                   performed successfully
@return       @b AUI_RTN_EINVAL  = At least one parameter (i.e. [in], [out])
                                   is invalid
@return       @b Other_Values    = Reading of the data from OTP device failed
                                   for some reason

@note         When reading rootkeys, the parameter @b ul_data_len must be 16 bytes
*/
AUI_RTN_CODE aui_otp_read (

    unsigned long ul_addr,

    unsigned char *puc_data,

    unsigned long ul_data_len

    );

/**
@brief        Function used to write data into the OTP device

@param[in]    ul_addr            = Address of the OTP device to be written
@param[in]    puc_data           = Pointer to the buffer containing the OTP
                                   data to be written
@param[in]    ul_data_len        = Length of the OTP data to be written

@return       @b AUI_RTN_SUCCESS = Writing of the data into the OTP device
                                   performed successfully
@return       @b AUI_RTN_EINVAL  = A least one input parameter (i.e. [in]) is
                                   invalid
@return       @b Other_Values    = Writing of the data into the OTP device
                                   failed for some reason

@note         When writing rootkeys, the parameter @b ul_data_len must be 16 bytes
*/
AUI_RTN_CODE aui_otp_write (

    unsigned long ul_addr,

    unsigned char *puc_data,

    unsigned long ul_data_len

    );

#ifdef __cplusplus

}

#endif

#endif

/*END Of FILE*/

