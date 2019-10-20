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
Current ALi Author: Steven.Zhang
Last update:        2017.02.25
-->

@file       aui_uart.h

@brief      Universal Asynchronous Receiver/Transmitter (UART) Module

            <b> Universal Asynchronous Receiver/Transmitter (UART) Module </b>
            is used to control and read/write raw data.
            It is an independent module which can be controlled by a device
            handle: when an UART device is opened, a handle is returned then
            used to read/write/clear UART device.
            The essential work flow of this module is summarized below:
            - Initialization (#aui_uart_init)
            - Open (#aui_uart_open)
            - Read / Write data (#aui_uart_read / #aui_uart_write)
            - Close (#aui_uart_close)
            - De-Initialization (#aui_uart_de_init)

@note For further details, please refer to the ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Universal Asynchronous
      Receiver/Transmitter (UART) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_UART_H

#define _AUI_UART_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version number </b> of UART Module, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_UART (0X00010000)

/**
Macro to indicate the <b> maximum length </b> acceptable (in characters) for
the <b> name </b> of a UART Module.

@attention  This macro is @a reserved to ALi R&D Dept.then users can ignore it
*/
#define AUI_FULL_FILE_NAME_LEN_MAX  (256)

/*******************************Global Type List*******************************/

/**
Enum to specify which feature (i.e. either read or write or read-write) of the
UART Device can enabled/disabled
*/
typedef enum aui_uart_io_mode {

    /**
    Value to specify that the @b read feature can be enabled/disabled
    */
    AUI_UART_READ = 0x1,

    /**
    Value to specify that the @b write feature can be enabled/disabled
    */
    AUI_UART_WRITE = 0x2,

    /**
    Value to specify that the @b read-write features can be enabled/disabled
    */
    AUI_UART_READ_WRITE = 0x3

} aui_uart_io_mode;

/**
Enum to specify the @b status of a UART Device
*/
typedef enum aui_uart_dev_status {

    /**
    Value to specify that the UART device is in <b> Free/Idle State </b>
    */
    AUI_UART_IDLE,

    /**
    Value to specify that the UART device is in <b> Open State </b>
   (i.e. under using)
    */
    AUI_UART_OPEN

} aui_uart_dev_status;

/**
Enum to specify the <b> number of bits </b> used for in an UART communication
*/
typedef enum aui_uart_data_bit {

    /**
    Value to specify <b> 5 bits </b>
    */
    AUI_UART_DATA_5BITS = 5,

    /**
    Value to specify <b> 6 bits </b>
    */
    AUI_UART_DATA_6BITS,

    /**
    Value to specify <b> 7 bits </b>
    */
    AUI_UART_DATA_7BITS,

    /**
    Value to specify <b> 8 bits </b>
    */
    AUI_UART_DATA_8BITS

} aui_uart_data_bit;

/**
Enum to specify the <b> number of stop bits </b> sent after each character
during an UART communication
*/
typedef enum aui_uart_stop_bit {

    /**
    Value to specify <b> one (1) stop bit </b>
    */
    AUI_UART_STOP_1_0,

    /**
    Value to specify <b> one and half (1.5) stop bit </b>
    */
    AUI_UART_STOP_1_5,

    /**
    Value to specify <b> two (2) stop bit </b>
    */
    AUI_UART_STOP_2_0

} aui_uart_stop_bit;

/**
Enum to specify the <b> parity mode </b> used during an UART communication
*/
typedef enum aui_uart_parity_mode {

    /**
    Value to specify that <b> no parity </b> mode is used
    */
    AUI_UART_PARITY_NONE,

    /**
    Value to specify the <b> odd parity </b> mode
    */
    AUI_UART_PARITY_ODD,

    /**
    Value to specify the <b> even parity </b> mode
    */
    AUI_UART_PARITY_EVEN

} aui_uart_parity_mode;

/**
Enum to specify the <b> flow control mode </b> used during an UART communication
*/
typedef enum aui_uart_flow_control {

    /**
    Value to specify that <b> no flow control </b> mode is used
    */
    AUI_UART_FLOW_CONTROL_NONE

} aui_uart_flow_control;

/**
Enum to specify a list of all commands available to be used for getting the
configuration information of the UART device
*/
typedef enum aui_en_uart_item_get {

    /**
    Value to specify the command to get all configuration parameters of an
    UART device
    */
    AUI_UART_GET_CFGPARAM=0,

    /**
    Value to specify the maximum number of commands which are supported by an
    UART device

    @attention  This value is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    AUI_UART_GET_CMD_LAST

} aui_uart_item_get;

/**
Enum to specify a list of all commands available to be used for setting the
attributes of an UART device
*/
typedef enum aui_en_uart_item_set {

    /**
    Value to specify the command to set all attributes of an UART device
    */
    AUI_UART_SET_CFGPARAM=0,

    /**
    Value to specify the maximum number of commands which are supported by an
    UART device

    @attention  This value is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    AUI_UART_SET_CMD_LAST

} aui_uart_item_set;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> UART Module </b> to specify all configuration details
        of an UART Device
        </div> @endhtmlonly

        Struct to specify all configuration details of an UART Device
*/
typedef struct aui_attr_uart {

    /**
    Member to specify the <b> Baud Rate </b> of an UART communication

    @note   The default value is set to <b> 115200 baud </b>
    */
    unsigned long   ul_baudrate;

    /**
    Member to specify the <b> number of bits </b> used for in an UART
    communication, as defined in the enum #aui_uart_data_bit
    */
    unsigned long   ul_data_bits;

    /**
    Member to specify the <b> number of stop bit </b> sent after each
    character during an UART communication, as defined in the enum
    #aui_uart_stop_bit
    */
    unsigned long   ul_stop_bits;

    /**
    Member to to specify the <b> parity mode </b> used during an UART
    communication, as defined in the enum #aui_uart_parity_mode
    */
    unsigned long   ul_parity;

    /**
    Member to specify the <b> flow control mode </b> used during an UART
    communication, as defined in the enum #aui_uart_flow_control
    */
    unsigned long   ul_flow_control;

} aui_attr_uart;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief        Function used to get the UART device handle known the UART
              Device ID

@param[in]    ul_uart_id            = UART device ID to be used to get the UART
                                      device handle

@param[out]   pv_handle_uart        = #aui_hdl pointer to the UART device handle
                                      just gotten

@return       @b AUI_RTN_SUCCESS    = Getting of the UART device handle performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one parameter (i.e. [in], [out])
                                      is invalid
@return       @b Other_Values       = Getting of the UART device handle failed
                                      for some reasons
*/
AUI_RTN_CODE aui_uart_handle_get_by_id (

    unsigned int ul_uart_id,

    aui_hdl *pv_handle_uart

    );

/**
@brief        Function used to get the UART device ID known the UART device handle

@param[in]    uart_handle           = #aui_hdl pointer to the UART device handle
                                      to be used to get the UART device ID

@param[out]   pul_uartid            = Pointer to the UART device ID just gotten

@return       @b AUI_RTN_SUCCESS    = Getting of the UART device ID performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one parameter (i.e. [in], [out])
                                      is invalid
@return       @b Other_Values       = Getting of the UART device ID failed for
                                      some reasons
*/
AUI_RTN_CODE aui_uart_id_get_by_handle (

    aui_hdl uart_handle,

    unsigned int *pul_uartid

    );

/**
@brief        Function used to get the version number of the UART Module, as
              defined in the macro #AUI_MODULE_VERSION_NUM_UART

@param[out]   pul_version           = Pointer to the version number of the
                                      UART Module just gotten

@return       @b AUI_RTN_SUCCESS    = Getting of the version number of UART
                                      Module performed successfully
@return       @b AUI_RTN_EINVAL     = The output parameter (i.e. [out]) is
                                      invalid
@return       @b Other_Values       = Getting of the version number of UART
                                      Module failed for some reasons
*/
AUI_RTN_CODE aui_uart_version_get (

    unsigned long *pul_version

    );

/**
@brief        Function used to initialize the UART device before its opening
              by the function #aui_uart_open

@param[in]    p_call_back_init      = Callback function used to initialize the
                                      UART device, as per comment for the
                                      function pointer #p_fun_cb
@param[in]    pv_param              = Input parameter of the callback function
                                      @b p_call_back_init

@return       @b AUI_RTN_SUCCESS    = Initializing of the UART device performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one input parameter (i.e. [in])
                                      is invalid
@return       @b Other_Values       = Initializing of the UART device failed
                                      for some reasons
*/
AUI_RTN_CODE aui_uart_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief        Function used to perform the de-initialization of the UART device
              after its closing by the function #aui_uart_close

@param[in]    p_call_back_de_init   = Callback function used to de-initialize
                                      the UART device, as per comment for the
                                      function pointer #p_fun_cb
@param[in]    pv_param              = The input parameter of the callback
                                      function @b p_call_back_de_init

@return       @b AUI_RTN_SUCCESS    = De-initializing of the UART device
                                      performed successfully
@return       @b AUI_RTN_EINVAL     = At least one input parameter (i.e. [in])
                                      is invalid
@return       @b Other_Values       = De-initializing of the UART module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_uart_de_init (

    p_fun_cb p_call_back_de_init,

    void *pv_param

    );

/**
@brief        Function used to open the UART device know the UART device ID then
              perform the configuration of it

@param[in]    ul_uartID             = UART ID device to be opened
                                      - @b Caution: It can be set to @b 0 or @b 1
@param[in]    p_uart_param          = Pointer to the struct #aui_attr_uart, which
                                      specify all configuration details of the
                    UART Device

@param[out]   pp_hdl_uart           = #aui_hdl pointer to the handle of the UART
                                      Device just opened.

@return       @b AUI_RTN_SUCCESS    = Opening of the UART device performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one parameter (i.e. [in], [out])
                                      is invalid
@return       @b Other_Values       = Opening of the UART device failed for some
                                      reasons
*/
AUI_RTN_CODE aui_uart_open (

    unsigned long ul_uartID,

    aui_attr_uart *p_uart_param,

    aui_hdl *pp_hdl_uart

    );

/**
@brief        Function used to close the UART device already opened by the
              function #aui_uart_open then the related handle will be released
        (i.e. the related resources such as memory, device)

@param[in]    pv_hdl_uart           = #aui_hdl handle of the UART device already
                                      opened and to be closed

@return       @b AUI_RTN_SUCCESS    = Closing of the UART device performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = The input parameter (i.e. [in]) is
                                      invalid
@return       @b Other_Values       = Closing of the UART device failed for
                                      some reasons
*/
AUI_RTN_CODE aui_uart_close (

    aui_hdl pv_hdl_uart

    );

/**
@brief        Function used to read UART data from the buffer

@param[in]    pv_hdl_uart           = #aui_hdl pointer to the handle of the
                                      UART device already opened and to be
                                      managed to read data from the buffer
@param[in]    ul_read_len           = Desired number of bytes to read from
                                      the buffer
@param[in]    ul_time_out           = Read timeout
                                      - @b Caution: The value @b 0 means this
                                           function will read indefinitely

@param[out]   pul_readed_len        = Pointer to the member of byte actually
                                      read
@param[out]   puc_buf               = Pointer to the buffer to be read

@return       @b AUI_RTN_SUCCESS    = Reading of the UART data performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one parameter (i.e. [in], [out])
                                      are invalid
@return       @b Other_Values       = Reading of the UART data failed for some
                                      reasons
*/
AUI_RTN_CODE aui_uart_read (

    aui_hdl pv_hdl_uart,

    unsigned char *puc_buf,

    unsigned long ul_read_len,

    unsigned long *pul_readed_len,

    unsigned long ul_time_out

    );

/**
@brief        Function used to write UART data into the buffer

@param[in]    pv_hdl_uart           = #aui_hdl pointer to the handle of the UART
                                      device already opened and to be managed to
                                      write data into the buffer
@param[in]    puc_buf               = Pointer to the buffer to be written
@param[in]    ul_write_len          = Desired number of bytes to write in the
                                      buffer
@param[in]    ul_time_out           = Write timeout
                                      - @b Caution: The value @b 0 means this
                                           function will write indefinitely

@param[out]   pul_written_len       = Pointer to the number of byte actually
                                      written

@return       @b AUI_RTN_SUCCESS    = Writing of the UART data performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one parameter (i.e. [in], [out])
                                      is invalid
@return       @b Other_Values       = Writing of the UART data failed for some
                                      reasons
*/
AUI_RTN_CODE aui_uart_write (

    aui_hdl pv_hdl_uart,

    unsigned char *puc_buf,

    unsigned long ul_write_len,

    unsigned long *pul_written_len,

    unsigned long ul_time_out

    );

/**
@brief        Function used to clear UART data from the buffer

@param[in]    pv_hdl_uart           = #aui_hdl pointer to the handle of the UART
                                      device already opened and to be managed to
                                      clear data from the buffer

@return       @b AUI_RTN_SUCCESS    = Clearing of the UART data performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = The input parameter (i.e. [in]) is invalid
@return       @b Other_Values       = Clearing of the UART data failed for some
                                      reasons
*/
AUI_RTN_CODE aui_uart_clear (

    aui_hdl pv_hdl_uart

    );

/**
@brief        Function used to enable a feature (i.e. either read or write or
              read-write) of the UART device known the UART handle

@param[in]    pv_hdl_uart           = #aui_hdl handle of the UART device to be
                                      used to enable a specific feature of the
                                      UART device
@param[in]    io_mode               = Specific feature of the UART device to be
                                      enabled, as defined in the enum
                                      #aui_uart_io_mode.

@return       @b AUI_RTN_SUCCESS    = Specific UART feature enabled successfully
@return       @b AUI_RTN_EINVAL     = At least one input parameter (i.e. [in]) is
                                      invalid
@return       @b Other_Values       = Specific UART feature not enabled for some
                                      reasons
*/
AUI_RTN_CODE aui_uart_enable (

    aui_hdl pv_hdl_uart,

    aui_uart_io_mode io_mode

    );

/**
@brief        Function used to disable a feature (i.e. either read or write or
              read-write) of the UART device known the UART handle

@param[in]    pv_hdl_uart           = #aui_hdl handle of the UART device to be
                                      used to disable a specific feature of the
                                      UART device
@param[in]    io_mode               = Specific feature of the UART device to be
                                      disabled, as defined in the enum
                                      #aui_uart_io_mode.

@return       @b AUI_RTN_SUCCESS    = Specific UART feature disabled successfully
@return       @b AUI_RTN_EINVAL     = At least one input parameter (i.e. [in])
                                      is invalid
@return       @b Other_Values       = Specific UART feature not disabled for
                                      some reasons
*/
AUI_RTN_CODE aui_uart_disable (

    aui_hdl pv_hdl_uart,

    aui_uart_io_mode io_mode

    );

/**
@brief        Function used to get a specific configuration information from the
              UART device after its opening by the functions #aui_uart_open

@param[in]    pv_hdl_uart           = #aui_hdl handle of the UART Module already
                                      opened and to be managed to get a specific
                                      configuration information
@param[in]    ul_item               = The command used to get a specific UART
                                      configuration information, as defined in
                                      the enum #aui_en_uart_item_get

@param[out]   pv_param              = The pointer to the specific UART
                                      configuration information just gotten, as
                                      defined in the enum #aui_en_uart_item_get

@return       @b AUI_RTN_SUCCESS    = Getting of the specific UART configuration
                                      information performed successfully
@return       @b AUI_RTN_EINVAL     = At least one parameters (i.e. [in] and
                                      [out]) is invalid
@return       @b Other_Values       = Getting of the specific UART configuration
                                      information failed for some reasons
*/
AUI_RTN_CODE aui_uart_get (

    aui_hdl pv_hdl_uart,

    aui_uart_item_get ul_item,

    void *pv_param

    );

/**
@brief        Function used to set a specific configuration information into the
              UART device after its opening by the functions #aui_uart_open

@param[in]    pv_hdl_uart           = #aui_hdl handle of the UART Module already
                                      opened and to be managed to set a specific
                                      configuration information
@param[in]    ul_item               = The command used to set a specific UART
                                      configuration information, as defined in
                                      the enum #aui_en_uart_item_set

@param[out]   pv_param              = The pointer to the specific UART
                                      configuration information to be set, as
                                      defined in the enum #aui_en_uart_item_set

@return       @b AUI_RTN_SUCCESS    = Setting of the specific UART configuration
                                      information performed successfully
@return       @b AUI_RTN_EINVAL     = At least one parameters (i.e. [in] and
                                      [out]) is invalid
@return       @b Other_Values       = Setting of the specific UART configuration
                                      information failed for some reasons
*/
AUI_RTN_CODE aui_uart_set (

    aui_hdl pv_hdl_uart,

    aui_uart_item_set  ul_item,

    void *pv_param

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_en_dev_enable aui_uart_io_mode

#define aui_st_cfg_uart aui_attr_uart

#define aui_cfg_uart aui_attr_uart

#define aui_p_cfg_uart aui_attr_uart*

#define aui_en_data_bit aui_uart_data_bit

#define aui_en_stop_bit aui_uart_stop_bit

#define aui_en_parity_mode aui_uart_parity_mode

#define aui_en_flow_control aui_uart_flow_control

#define AUI_ENUM_UART_DATA_5BITS AUI_UART_DATA_5BITS

#define AUI_ENUM_UART_DATA_6BITS AUI_UART_DATA_6BITS

#define AUI_ENUM_UART_DATA_7BITS AUI_UART_DATA_7BITS

#define AUI_ENUM_UART_DATA_8BITS AUI_UART_DATA_8BITS

#define AUI_ENUM_UART_STOP_1_0 AUI_UART_STOP_1_0

#define AUI_ENUM_UART_STOP_1_5 AUI_UART_STOP_1_5

#define AUI_ENUM_UART_STOP_2_0 AUI_UART_STOP_2_0

#define AUI_ENUM_UART_NO_PARITY AUI_UART_PARITY_NONE

#define AUI_ENUM_UART_ODD_PARITY AUI_UART_PARITY_ODD

#define AUI_ENUM_UART_EVEN_PARITY AUI_UART_PARITY_EVEN

#define AUI_ENUM_UART_NO_FLOW_CONTROL AUI_UART_FLOW_CONTROL_NONE

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


