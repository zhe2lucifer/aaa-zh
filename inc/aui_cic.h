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
Current ALi Author: Niker.Li, Fawn.Fu
Last update:        2017.02.25
-->

@file       aui_cic.h

@brief      Common Interface Controller (CIC) Module

            <b> Common Interface Controller (CIC) Module </b> is used to
            separate the Conditional Access Functionality from a Digital
            TV Receiver/Decoder (Host) into a removable Conditional Access
            Module (CAM).\n
            This module @a only support the communication of the HOST with CAM
            Card or Common Interface (CI) plus CAM Card, but does not include
            CI Stack. \n
            To use this module to communicate with CAM Card, user @a must
            develop CI Stack by himself.

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_CIC_H_

#define _AUI_CIC_H_

/*************************Included Header File List***************************/

#include "aui_common.h"

#ifdef __cplusplus

extern "C" {

#endif

/*******************************Global Type List*******************************/

/**
Enum to specify the registers available to be read/written in CIC Module
*/
typedef enum aui_cic_reg {

    /**
    Value to specify the <b> Data Register </b>
    */
    AUI_CIC_DATA    = 0,

    /**
    Value to specify the <b> Command/Status Register </b>
    */
    AUI_CIC_CSR,

    /**
    Value to specify the <b> Size Register </b> in  particular its <b> Less
    Significant Byte (LSB) </b>
     */
    AUI_CIC_SIZELS,

    /**
    Value to specify the <b> Size Register </b> in  particular its <b> Most
    Significant Byte (MSB) </b>
    */
    AUI_CIC_SIZEMS

} aui_cic_reg;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Common Interface Controller (CIC) Module </b>
        reserved to ALi R&D Dept. then user can ignore it
        </div> @endhtmlonly

@attention  This struct is reserved to ALi R&D Dept. then user can ignore it
*/
typedef struct aui_cic_io_param {

    int     slot;

    unsigned short  reg;

    unsigned char  *buf;

} aui_cic_io_param;

typedef void (*aui_cic_mutex_callback) (

    void

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Common Interface Controller (CIC) Module </b> to
        specify the callback functions used by other modules to lock/unlock
        the mutex
        </div> @endhtmlonly

        Struct to specify the callback functions (without user data) used by
        other modules to lock/unlock the mutex
*/
typedef struct aui_cic_mutex {

    /**
    Member to specify the callback function (without user data) used by other
    modules to lock the mutex.

    @note   When using this callback function, the callback function
            #cic_exit_mutex cannot be set at the same time
    */
    aui_cic_mutex_callback cic_enter_mutex;

    /**
    Member to specify the callback function (without user data) used by other
    modules to unlock the mutex

    @note   When using this callback function, the callback function
            #cic_enter_mutex cannot be set at the same time
    */
    aui_cic_mutex_callback cic_exit_mutex;

} aui_cic_mutex;

/**
Funtion pointer to specify the callback function registered to get the status
of a CAM Card Slot.\n

@note   The parameter @b slot is to specify the specific CAM Card Slot, and it
        can take the values 0 or 1 since ALi boards can support up to two (2)
        CAM Card Slots
*/
typedef void (*aui_cic_hw_status_cb) (

    int slot

    );

/*****************************Global Function List*****************************/

/**
@brief          Function used to perform the initialization of the CIC Module,
                before its opening by the function #aui_cic_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the CIC Module

@param[in]      p_callback_init     = Callback function used for the
                                      initialization of the CIC Module, as per
                                      comment for the function pointer #p_fun_cb
                                      - @b Caution: It suggested to set to NULL

@return         @b AUI_RTN_SUCCESS  = Initializing of the CIC Module performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Initializing of the CIC Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_init (

    p_fun_cb p_callback_init

    );

/**
@brief          Function used to perform the de-initialization of the CIC Module,
                after its closing by the function #aui_cic_close

@param[in]      p_callback_deinit   = Callback function used to de-initialize
                                      the CIC Module, as per comment for the
                                      function pointer #p_fun_cb
                                      - @b Caution: It suggested to set to NULL

@return         @b AUI_RTN_SUCCESS  = De-Initializing of the CIC Module performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = De-initializing of the CIC Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_cic_de_init (

    p_fun_cb p_callback_deinit

    );

/**
@brief          Function used to open the CIC Module and configure the desired
                attributes, then get the related handle

@warning        This function can @a only be used in the <b> Pre-Run Stage </b>
                of the CIC Module

@param[in]      cic_hw_cb           = Callback function used to get the CAM
                                      Card Slot Status

@param[out]     *pp_cic_handle      = #aui_hdl pointer to the handle of the CIC
                                      Module just opened

@return         @b AUI_RTN_SUCCESS  = Opening of the CIC Module performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Opening of the CIC Module failed for some
                                      reasons
*/
AUI_RTN_CODE aui_cic_open (

    aui_cic_hw_status_cb cic_hw_cb,

    aui_hdl *pp_cic_handle

    );

/**
@brief          Function used to close the CIC Module already opened by the
                function #aui_cic_open, then the related handle will be released
                (i.e. the related resources such as memory, device)

@param[in]      cic_handle          = #aui_hdl pointer to the handle of the CIC
                                      Module already opened and to be closed

@return         @b AUI_RTN_SUCCESS  = Closing of the CIC Module performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Closing of the CIC Module failed for same
                                      reasons
*/
AUI_RTN_CODE aui_cic_close (

    aui_hdl cic_handle

    );

/**
@brief          Function used to enable the CAM Card after its inserting in the
                CAM Card Slot has been detected

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened

@param[in]      slot                = ID Slot of the CAM Card to be enabled
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots

@return         @b AUI_RTN_SUCCESS  = Enabling of the CAM Card in the slot
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Enabling of the CAM Card in the slot
                                      failed for some reasons

*/
AUI_RTN_CODE aui_cic_enable_cam (

    aui_hdl cic_handle,

    int slot

    );

/**
@brief          Function used to disable the CAM Card in the slot

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened

@param[in]      slot                = ID Slot of the CAM Card to be disabled
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots

@return         @b AUI_RTN_SUCCESS  = Disabling of the CAM Card in the slot
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Disabling of the CAM Card in the slot
                                      failed for some reasons

*/
AUI_RTN_CODE aui_cic_disable_cam (

    aui_hdl cic_handle,

    int slot

    );

/**
@brief          Function used to read one byte from a specific register

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be read
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots
@param[in]      offset              = Specific register to be read, as defined
                                      in the enum #aui_cic_reg

@param[out]     puc_value           = Pointer to the data just read from the
                                      specific register

@return         @b AUI_RTN_SUCCESS  = Reading of the data from a specific
                                      register performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Reading of the data from a specific
                                      register failed for some reasons
*/
AUI_RTN_CODE aui_cic_read_io_reg (

    aui_hdl cic_handle,

    int slot,

    aui_cic_reg offset,

    unsigned char *puc_value

    );

/**
@brief          Function used to write one byte into a specific register

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be written
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots
@param[in]      offset              = Specific register to be written, as defined
                                      in the enum #aui_cic_reg
@param[in]      uc_value            = Data to be written in the specific register

@return         @b AUI_RTN_SUCCESS  = Writing of the data into a specific
                                      register performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Writing of the data into a specific register
                                      failed for some reasons
*/
AUI_RTN_CODE aui_cic_write_io_reg (

    aui_hdl cic_handle,

    int slot,

    aui_cic_reg offset,

    unsigned char uc_value

    );

/**
@brief          Function used to read data from a specific register consecutively

@warning        User should check or clear the Command/Status Register before/
                after reading data

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be read
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots
@param[in]      us_size             = Size of the data to be read from a specific
                                      register

@param[out]     puc_buf             = Pointer to the buffer intended to store
                                      the data just read from a specific register

@return         @b AUI_RTN_SUCCESS  = Reading of the data from a specific
                                      register performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Reading of the data from a specific
                                      register failed for some reasons.
*/
AUI_RTN_CODE aui_cic_read_io_data (

    aui_hdl cic_handle,

    int slot,

    unsigned short us_size,

    unsigned char *puc_buf

    );

/**
@brief          Function used to write data into a specific register consecutively

@warning        User should check or clear the Command/Status Register before/
                after writing data

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be written
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots
@param[in]      us_size             = Size of the data to be written into a
                                      specific register
@param[out]     puc_buf             = Pointer to the buffer intended to store
                                      the data to be written into a specific
                                      register

@return         @b AUI_RTN_SUCCESS  = Writing of the data into a specific
                                      register performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Writing of the data into a specific
                                      register failed for some reasons
*/
AUI_RTN_CODE aui_cic_write_io_data (

    aui_hdl cic_handle,

    int slot,

    unsigned short us_size,

    unsigned char *puc_buf

    );

/**
@brief          Function used to read the memory attributes of the CAM Card

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened

@param[in]      slot                = ID Slot of the CAM Card of which read the
                                      memory attributes
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots

@param[in]      us_size             = Size of the memory attributes to be read
                                      - @b Caution: ALi driver is limited to
                                           read up to 0x2000 byte

@param[in]      us_addr             = Start address of the memory attributes
                                      to read

@param[out]     puc_buf             = Pointer to the buffer intended to store
                                      the memory attribute just read

@return         @b AUI_RTN_SUCCESS  = Reading of the memory attributes memory
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Reading of the memory attribute failed
                                      for some reasons

@note           ALi driver reads the parameter @b addr and writes into the
                buffer consecutively
*/
AUI_RTN_CODE aui_cic_read_mem (

    aui_hdl cic_handle,

    int slot,

    unsigned short us_size,

    unsigned short us_addr,

    unsigned char *puc_buf

    );

/**
@brief          Function used to write the memory attributes of the CAM Card

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card into which write
                                      the memory attributes
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots
@param[in]      us_size             = Size of the memory attributes to be written
                                      - @b Caution: Ali driver is limited to write
                                           up to 0x2000 byte
@param[in]      us_addr             = Start address of the memory attributes
                                      to write
@param[in]      puc_buf             = Pointer to the buffer intended to store
                                      the memory attributes to be written

@return         @b AUI_RTN_SUCCESS  = Writing of the memory attributes performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Writing of the memory attributes failed
                                      for some reasons
*/
AUI_RTN_CODE aui_cic_write_mem (

    aui_hdl cic_handle,

    int slot,

    unsigned short us_size,

    unsigned short us_addr,

    unsigned char *puc_buf

    );

/**
@brief          Function used to detect the CAM Card in a slot

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be detected
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots

@param[out]     p_detected           = Pointer to result of the detecting process
                                       saved by a driver, in particular:
                                      - @b 1 = CAM Card detected
                                      - @b 0 = CAM Card not detected

@return         @b AUI_RTN_SUCCESS  = Detecting of the CAM Card performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Detecting of the CAM Card failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_detect_cam (

    aui_hdl cic_handle,

    int slot,

    int *p_detected

    );

/**
@brief          Function used to check whether the detected CAM Card in a slot
                is ready to work or not

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be checked if
                                      ready to work
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots

@param[out]     p_ready             =  Pointer to result of the checking process
                                       saved by a driver, in particular:
                                      - @b 1 = CAM Card ready to work
                                      - @b 0 = CAM Card not ready to work

@return         @b AUI_RTN_SUCCESS  = Checking of the CAM Card performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Checking of the CAM Card failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_cam_is_ready (

    aui_hdl cic_handle,

    int slot,

    int *p_ready

    );

/**
@brief          Function used to reset the CAM Card in a slot

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = Slot index (which value can be 0 or 1)

@return         @b AUI_RTN_SUCCESS  = Resetting of the CAM card performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. param[in])
                                      is invalid
@return         @b Other_Values     = Resetting of the CAM card failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_reset_cam (

    aui_hdl cic_handle,

    int slot

    );

/**
@brief          Function used to reset CI Plus CAM card. Host sets the RS flag
                and starts interface initialization

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      slot                = ID Slot of the CAM Card to be reset
                                      - @b Caution: It can take the values @b 0
                                           or @b 1 since ALi boards can support
                                           up to two (2) CAM Card Slots

@return         @b AUI_RTN_SUCCESS  = Resetting of the CAM Card performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Resetting of the CAM Card failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_rs_reset_cam (

    aui_hdl cic_handle,

    int slot

    );

/**
@brief          Function used by other modules to lock/unlock the mutex

@param[in]      cic_handle          = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]      *p_req              = Specific action to be performed on the
                                      mutex, i.e. either lock or unlock as
                                      defined in the struct #aui_cic_mutex

@return         @b AUI_RTN_SUCCESS  = Locking/Unlocking of the mutex performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Locking/Unlocking of the mutex failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_req_mutex (

    aui_hdl cic_handle,

    aui_cic_mutex *p_req

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                 START                                     */
/*****************************************************************************/

///@cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

/**
@brief         Function used for setting whether the TS Stream will pass or
               bypass CAM Card

@warning       This function is replaced by the function #aui_tsi_ci_card_bypass_set

@param[in]     cic_handle           = #aui_hdl handle of the CIC Module already
                                      opened
@param[in]     slot                 = Slot index (which value can be 0 or 1)
@param[in]     uc_pass              = Flag to specify the desired setting, i.e.
                                      - @b 0 = Bypass Cam Card
                                      - @b 1 = Pass Cam Card

@return         @b AUI_RTN_SUCCESS  = Setting of the CAM Card performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Setting of the CAM Card failed for
                                      some reasons
*/
AUI_RTN_CODE aui_cic_pass_stream (

    aui_hdl cic_handle,

    int slot,

    unsigned char uc_pass

    );

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

