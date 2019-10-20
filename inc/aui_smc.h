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
Current ALi Author: Oscar.Shi
Last update:        2017.02.25
-->

@file   aui_smc.h

@brief  Smart Card (SMC) Module

        <b> Smart Card (SMC) Module </b> is used for information exchange
        negotiated between the interface device and the integrated card
        (Smart Card).\n
        SMC Module mainly provides the following functions:
        - Activate/Deactivate the Smart Card
        - Indication of the status of the Smart Card
        - Send/Receive data to/from the Smart Card
        - Set/Get communication parameters into/from the Smart Card

@note   For further details, please refer to ALi document
        <b><em>
        ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Smart Card (SMC) Module"
        </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
*/

#ifndef _AUI_SMC_H_

#define _AUI_SMC_H_

/*************************Included Header File List***************************/

#include "aui_common.h"

#ifdef __cplusplus

extern "C"

{

#endif

/*******************************Global Macro List*****************************/

/**
Macro to specify the maximum length (in @a byte unit) of an ISO command used
for a Smart Card
*/
#define AUI_SMC_ISOCMD_LEN  (9)

/**
Macro to specify the maximum number of interface devices supported in this
module.\n
That number starts from 0 and SMC Module can support up to two (2) interface
devices
*/
#define AUI_MAX_SMARTCARD_NUM   (1)

/**
Macro to specify that the T1 Protocol is not implemented in the AUI layer

@note   In the past, the Smart Card driver did not support the T1 Protocol
        which, therefore, it was implemented in AUI layer.\n
        Now the Smart Card driver supports the T1 Protocol then this macro
        is necessary.
*/
#define APDU_SMC_DISABLE

/**
Macro to specifies the maximum size (in @a byte unit) of the Smart Card Clock
Array
*/
#define AUI_SMC_CLK_ARRAY_MAX   16

/*******************************Global Type List*******************************/

/**
Function pointer to specify the type of callback function
- Registered with the function #aui_smc_open
- Filled in the struct #aui_smc_attr
- Used to indicate the status of the smart card, i.e. whether plugged in or
  plugged out

The input parameter @b n_card_index specifies the smart card slot, the second
parameter @b e_status, instead, specifies the status of the smart card (i.e.
plugged in or plugged out)
*/
typedef void (*aui_smc_p_fun_cb) (

    int n_card_index,

    unsigned int e_status

    );

/**
Enum to specify all supported <b> Smart Card Standards </b>
*/
typedef enum aui_smc_standard {

    /**
    Value to specify the <b> ISO/IEC 7816 </b> Standard
    */
    AUI_SMC_STANDARD_ISO,

    /**
    Value to specify the <b> EMV 2000 </b> Standard
    */
    AUI_SMC_STANDARD_EMV2000,

    /**
    Value to specify the <b> IRDETO </b> Standard
    */
    AUI_SMC_STANDARD_IRDETO,

    /**
    Value to specify the maximum number of supported Smart Card Standards
    */
    AUI_SMC_STANDARD_MAX

} aui_smc_standard, aui_smc_standard_t;

/**
Enum to specify all supported <b> Protocol ISO/IEC 7816 Standards </b>
*/
typedef enum aui_smc_protocol {

    /**
    Value to specify an @b Unknown Protocol
    */
    AUI_SMC_PROTOCOL_UNKNOWN,

    /**
    Value to specify the @b T0 Protocol
    */
    AUI_SMC_PROTOCOL_T0,

    /**
    Value to specify the @b T1 Protocol
    */
    AUI_SMC_PROTOCOL_T1,

    /**
    Value to specify the @b T14 Protocol
    */
    AUI_SMC_PROTOCOL_T14,

    /**
    Value to specify the <b> T1 ISO </b> Protocol

    @note   In this Protocol, the information to be exchanged contains the
            header and check code
    */
    AUI_SMC_PROTOCOL_T1_ISO,

    /**
    Value to specify the maximum number of supported Protocol ISO/IEC 7816
    Standards
    */
    AUI_SMC_PROTOCOL_MAX

} aui_smc_protocol, aui_smc_protocol_t;

/**
Enum to specify the number of stop bits used for the communication between
the interface device and the smart card
*/
typedef enum aui_smc_stopbits {

    /**
    Value to specify using of @b 0 stop bits
    */
    AUI_SMC_STOP_0,

    /**
    Value to specify using of @b 1 stop bits
    */
    AUI_SMC_STOP_1,

    /**
    Value to specify using of @b 2 stop bits
    */
    AUI_SMC_STOP_2,

    /**
    Value to specify using of @b 3 stop bits
    */
    AUI_SMC_STOP_3,

    /**
    Value to specify the total number of items of this enum
    */
    AUI_SMC_STOP_MAX

} aui_smc_stopbits, aui_smc_stopbits_t;

/**
Enum to specify all supported <b> Parity Modes </b> for the communication
*/
typedef enum aui_smc_databits {

    /**
    Value to specify that <b> no parity mode </b> is used
    */
    AUI_SMC_8BITS_PARITY_NONE,

    /**
    Value to specify using of the <b> Odd Parity Mode </b>
    */
    AUI_SMC_8BITS_PARITY_ODD,

    /**
    Value to specify using of the <b> Even Parity Mode </b>
    */
    AUI_SMC_8BITS_PARITY_EVEN,

    /**
    Value to specify the maximum number of all supported parity mode a character
    frame
    */
    AUI_SMC_8BITS_PARITY_MAX

} aui_smc_databits, aui_smc_databits_t;

/**
Enum to specify miscellaneous setting supported by the Smart Card Module

@note   This enum is used by the function #aui_smc_set to perform a specific
        setting where
        - The parameter @b cmd takes the item related to the specific setting
          to perform
        - The parameter @b pv_param takes the pointer as per the description
          of the specific setting to perform
*/
typedef enum aui_smc_cmd {

    /**
    This value is to set the <b> Work Waiting Time (WWT) </b> (also known as <b>
    Waiting Time (WI) </b> in the standard specifications)

    @note   The parameter @b pv_param takes the pointer to the buffer intended
            to save the WWT (in @a milliseconds (ms) unit)
    */
    AUI_SMC_CMD_SET_WWT,

    /**
    This value specify whether PPS is set or not during the reset process.
    The reset process is done by function @aui_smc_reset

    @note   The parameter @b pv_param takes the pointer to an integer, in particular:
     - @b 0 = PPS will be set during the reset process
     - @b 1 = PPS will be not set during the reset process

    @note   This command affect the function @aui_smc_reset only. It does not affect 
            function @aui_smc_setpps

    */
    AUI_SMC_CMD_DISABLE_PPS_ON_RESET

} aui_smc_cmd, aui_smc_cmd_t;

/**

@brief  @htmlonly <div class="details">
        Struct of the <b> Smart Card (SMC) Module </b> to specify the
        Communication Parameters
        </div> @endhtmlonly

        Struct to specify the <b> Communication Parameters </b> to be used by
        the Smart Card Module
*/
typedef struct aui_smc_param {

    /**
    Member to specify the <b> Elementary Time Unit (ETU) </b>
    */
    int m_nETU;

    /**
    Member to specify <b> Communication Baud Rate </b>

    @note   This member is reserved for future use then user can ignore it at now
    */
    int m_n_baud_rate;

    /**
    Member to specify the <b> Clock Frequency </b>

    @note   This member is reserved for future use then user can ignore it at now
    */
    int m_n_frequency;

    /**
    Member to specify the <b> Communication Standard </b>, as defined in the enum
    #aui_smc_standard_t
    */
    aui_smc_standard m_e_standard;

    /**
    Member to specify the <b> Communication Protocol </b>, as defined in the enum
    #aui_smc_protocol_t
    */
    aui_smc_protocol m_e_protocol;

    /**
    Member to specify the number of <b> stop bits </b> of a character frame, as
    defined in the enum #aui_smc_stopbits_t

    @note   This member is reserved for future use then user can ignore it at now
    */
    aui_smc_stopbits m_e_stop_bit;

    /**
    Member to specify the <b> parity mode </b> of the character frame, as defined
    in the enum #aui_smc_databits_t

    @note   This member is reserved for future use then user can ignore it at now
    */
    aui_smc_databits m_e_check_bit;

} aui_smc_param, aui_smc_param_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Smart Card (SMC) Module </b> to specify the parameters
        of the Smart Card Driver
        </div> @endhtmlonly

        Struct to specify the parameters of the Smart Card interface device

@note   @b 1. This struct is used by the function #aui_smc_init where the
              parameter @b psmc_cb_init is a #p_fun_cb callback function which
              parameter @b pv_param can take one pointer of this struct array
@note   @b 2. This struct is used @a only in projects based on <b> Linux OS </b>
*/
typedef struct aui_smc_device_cfg {

    /**
    Member as a @a flag to specify
    - @b 0 = Default initial clock, i.e. <b> 3.579545 MHz </b>
    - @b 1 = Configured clock as initial clock
    */
    unsigned int init_clk_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = Hardware detected ETU as initial ETU
    - @b 1 = Configured ETU as initial ETU
    */
    unsigned int def_etu_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = Default System Clock
    - @b 1 = System Clock configured by the member #smc_sys_clk of this struct

    @attention  At present, this member is not used then user can ignore it
    */
    unsigned int sys_clk_trigger : 1;

    /**
    Member as a @a flag to specify the use of GPIO to detect smart card in or
    out, in particular
    - @b 0 = Disabled
    - @b 1 = Enabled
    */
    unsigned int gpio_cd_trigger : 1;

    /**
    @warning    At present, this member is not used then user can ignore it
    */
    unsigned int gpio_cs_trigger : 1;

    /**
    Member as a @a flag to specify the use of dynamic switching of transferring/
    receiving data
    - @b 0 = Disabled
    - @b 1 = Enabled
    */
    unsigned int force_tx_rx_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = Parity Mode while get ATR enabled
    - @b 1 = Parity Mode while get ATR disabled
    */
    unsigned int parity_disable_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = Even parity while get ATR
    - @b 1 = Odd parity while get ATR
    */
    unsigned int parity_odd_trigger : 1;

    /**
    Member as a @a flag to specify the use of auto pull down function when
    detecting parity error
    - @b 0 = Disabled
    - @b 1 = Enabled
    */
    unsigned int apd_disable_trigger : 1;

    /**
    Member as a @a flag to specify whether check the Class of Smart Card or not,
    in particular
    - @b 0 = Don't check
    - @b 1 = Check (i.e. either A, B or AB class) according to interface device
             setting

    @attention  At present, this member is not used then user can ignore it
    */
    unsigned int type_chk_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = All resets are are cold reset type
    - @b 1 = All resets are warm reset type except the first one
    */
    unsigned int warm_reset_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = Don't use a GPIO PIN to provide the Vpp signal
    - @b 1 = Use a GPIO PIN to provide the Vpp signal
    */
    unsigned int gpio_vpp_trigger : 1;

    /**
    Member as a @a flag to specify
    - @b 0 = PPS enabled
    - @b 1 = PPS disabled
    */
    unsigned int disable_pps : 1;

    /**
    Member as a @a flag to specify whether invert the Card POWENJ output or not,
    in particular
    - @b 0 = Don't invert
    - @b 1 = Invert
    */
    unsigned int invert_power : 1;

    /**
    Member as a @a flag to specify whether invert the card detect input from
    card socket or not, in particular
    - @b 0 = Don't invert
    - @b 1 = Invert
    */
    unsigned int invert_detect : 1;

    /**
    Member as a @a flag to specify This element indicates current board support
    more than one class of operating conditions or not
    - @b 0 = Current board supports only one class of operating conditions
    - @b 1 = Current board supports two or more class of operating conditions
    */
    unsigned int class_selection_supported : 1;

    /**
    Member to specify that the current board supports all classes of operating
    conditions
    */
    unsigned int board_supported_class : 6;

    /**
    @attention  This member is reserved to ALi R&D Dept. then user can ignore it
    */
    unsigned int reserved : 10;

    /**
    Member to specify the number of initialized clock saved in the array
    @b init_clk_array[#AUI_SMC_CLK_ARRAY_MAX] as member of this struct
    */
    unsigned int init_clk_number;

    /**
    Member to specify an array which saves all user specified initialized clock
    */
    unsigned int init_clk_array[AUI_SMC_CLK_ARRAY_MAX];

    /**
    Member to specify the user specified ETU
    */
    unsigned int default_etu;

    /**
    Member to specify the user specified system clock of the Smart Card
    */
    unsigned int smc_sys_clk;

    /**
    Member as a @a flag to specify the polarity of the card detect GPIO,
    in particular
    - @b 0 = Low Level Active
    - @b 1 = High Level Active
    */
    unsigned int gpio_cd_pol : 1;

    /**
    Member as a @a flag to specify the direction of the card detect GPIO,
    in particular
    - @b HAL_GPIO_I_DIR = Input
    - @b HAL_GPIO_O_DIR = Output
    as defined in the file @b hal_gpio.h
    */
    unsigned int gpio_cd_io : 1;

    /**
    Member to specify the number of the card detect GPIO
    */
    unsigned int gpio_cd_pos : 14;

    /**
    @attention  At present, this member is not used then user can ignore it
    */
    unsigned int gpio_cs_pol : 1;

    /**
    @attention  At present, this member is not used then user can ignore it
    */
    unsigned int gpio_cs_io : 1;

    /**
    @attention  At present, this member is not used then user can ignore it
    */
    unsigned int gpio_cs_pos : 14;

    /**
    Member to specify the first byte of the forced TX/RX command
    */
    unsigned char force_tx_rx_cmd;

    /**
    Member to specify the the length (in @a byte unit) of the forced TX/RX
    command
    */
    unsigned char force_tx_rx_cmd_len;

    /**
    @attention  At present, this member is not used then user can ignore it
    */
    unsigned char intf_dev_type;

    /**
    Member as a @a flag to specify the polarity of the Vpp GPIO, in particular
    - @b 0 = Low Level Active
    - @b 1 = High Level Active
    */
    unsigned int gpio_vpp_pol : 1;

    /**
    Member as a @a flag to specify the direction of the Vpp GPIO, in particular
    - @b HAL_GPIO_I_DIR = Input
    - @b HAL_GPIO_O_DIR = Output
    as defined in the file @b hal_gpio.h
    */
    unsigned int gpio_vpp_io : 1;

    /**
    Member to specify the number of the powered down Vpp
    */
    unsigned int gpio_vpp_pos : 14;

    /**
    @attention  This member is reserved to ALi R&D Dept. then user can ignore it
    */
    unsigned int reserved2: 16;

    /**
    Member as a @a flag to specify whether using the external configuration
    specified by the member @b ext_cfg_pointer of this struct or not,
    in particular
    - @b 0 = Use
    - @b 1 = Don't use

    @attention  At present, this member is not used then user can ignore it
    */
    unsigned int ext_cfg_tag;

    /**
    Member as a pointer to the external configuration of Smart Card.

    @warning    Using of this member depends of the member @b ext_cfg_tag of
                this struct

    @attention  At present, this member is not used then user can ignore it
    */
    void     *ext_cfg_pointer;

    /**
    Member as a @a flag to specify whether using of the default configuration
    of Smart Card or not, in particular
    - @b 0 = Use
    - @b 1 = Don't use

    @attention  At present, this member is not used then user can ignore it
    */
    int      use_default_cfg;

} aui_smc_device_cfg, aui_smc_device_cfg_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Smart Card (SMC) Module </b> to specify the
        Configuration Attributes
        </div> @endhtmlonly

        Struct to specify the Smart Card Configuration Attributes
*/
typedef struct aui_smc_attr {

    /**
    Member to specify the <b> Slot Number </b> of the Smart Card

    @note   The slot number starts from 0 and SMC Module can support up to two
            (2) interface devices
    */
    unsigned long ul_smc_id;

    /**
    Member to specify a callback function to be used for indicating whether the
    Smart Card has been plugged in or plugged out, as per comment of the function
    pointer #aui_smc_p_fun_cb
    */
    aui_smc_p_fun_cb p_fn_smc_cb;

} aui_smc_attr, *aui_psmc_attr;

/*****************************Global Function List*****************************/

/**
@brief          Function used to initialize the Smart Card Module

@warning        This function should be used before calling any other function
                of Smart Card Module

@param[in]      psmc_cb_init         = Callback function used for the
                                       initialization of the Smart Card Module
                                       configuration, as per comment of the
                                       function pointer #p_fun_cb
                                       - @b Caution: \n\n

                                            In <b> Linux OS </b>, that callback
                                            function takes a pointer to the
                                            #aui_smc_device_cfg_t struct array as
                                            input parameter to assign all proper
                                            data according to the different ALi
                                            board.\n
                                            The #aui_smc_device_cfg_t will be
                                            used to configure the SMC Module
                                            when opening the SMC Module, i.e.
                                            when using the function #aui_smc_open.
                                            \n
                                            Please refer to the sample code of
                                            the initialization of SMC Module for
                                            more clarifications.\n\n
                                            In <b> TDS OS </b>, instead, this
                                            member can be set to @b NULL. The
                                            SMC Module will be configured in the
                                            function @b smc_attach, please refer
                                            to the sample code for more
                                            clarifications.

@return         @b AUI_RTN_SUCCESS   = Initializing of the Smart Card instance
                                       performed successfully
@return         @b Other_Values      = Initializing of the Smart Card instance
                                       failed for some reasons, as defined in
                                       the enum #aui_smc_errno_type and in the
                                       <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_init (

    p_fun_cb psmc_cb_init

    );

/**
@brief          Function used to de-initialize the Smart Card Module

@param[in]      psmc_call_back_init  = Callback function used for the
                                       initialization of the Smart Card Module,
                                       as per comment of the function pointer
                                       #p_fun_cb
                                       - @b Caution: For both <b> TDS OS </b>
                                            and <b> Linux OS </b> this member
                                            can be set to @b NULL

@return         @b AUI_RTN_SUCCESS   = De-Initializing of the Smart Card
                                       instance performed successfully
@return         @b Other_Values      = Initializing of the Smart Card instance
                                       failed for some reasons, as defined in
                                       the enum #aui_smc_errno_type and in the
                                       <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_de_init (

    p_fun_cb psmc_call_back_init

    );

/**
@brief          Function used to open a Smart Card instance after its
                initialization by the function #aui_smc_init

@warning        This function should be used before calling any other function
                of Smart Card Module except the function #aui_smc_init

@param[in]      p_smc_attr              = Pointer to a struct #aui_smc_attr
                                          which collects the configuration
                                          attributes for a Smart Card interface
                                          device

@param[out]     p_smc_handle            = #aui_hdl pointer to the handle of the
                                          Smart Card instance just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the Smart Card instance
                                          performed successfully
@return         @b Other_Values         = Opening of the Smart Card instance
                                          failed for some reasons, as defined
                                          in the enum #aui_smc_errno_type and
                                          in the <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_open (

    aui_smc_attr *p_smc_attr,

    aui_hdl *p_smc_handle

    );

/**
@brief          Function used to close a Smart Card instance already opened by
                the function #aui_smc_open

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance

@return         @b AUI_RTN_SUCCESS      = Closing of the Smart Card instance
                                          performed successfully
@return         @b Other_Values         = Closing of the Smart Card instance
                                          failed for some reasons. as defined
                                          in the enum #aui_smc_errno_type and
                                          in the <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_close (

    aui_hdl smc_handle

    );

/**
@brief          Function used to reset the Smart Card and get the Answer to
                Reset (ATR)

@warning        This function should be used before transferring data to the
                Smart Card

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance
@param[in]      b_cold_rst              = This parameter is used in <b> TDS OS
                                          </b> to set the member
                                          @b warm_reset_trigger of the struct
                                          #aui_smc_device_cfg_t \n
                                          In <b> Linux OS </b>, instead, the
                                          same setting should be performed by
                                          the function #aui_smc_init, as
                                          defined in the struct
                                          #aui_smc_device_cfg_t

@param[out]     puc_atr                 = Pointer to the buffer intended to
                                          store the ATR
@param[out]     ps_atr_length           = Length (in @a byte unit) of the ATR

@return         @b AUI_RTN_SUCCESS      = Resetting of the Smart Card instance
                                          performed successfully
@return         @b Other_Values         = Resetting of the Smart Card instance
                                          failed for some reasons, as defined
                                          in the enum #aui_smc_errno_type and
                                          in the <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_reset (

    aui_hdl smc_handle,

    unsigned char *puc_atr,

    unsigned short *ps_atr_length,

    int b_cold_rst

    );

/**
@brief          Function used to get the communication parameters from the
                interface device

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance

@param[out]     p_smc_param             = Pointer to a struct #aui_smc_param which
                                          collect the communication parameters
                                          of the Smart Card interface device
                                          just gotten

@return         @b AUI_RTN_SUCCESS      = Getting of the configuration parameters
                                          of the Smart Card instance performed
                                          successfully
@return         @b Other_Values         = Getting of the configuration parameters
                                          of the Smart Card instance failed for
                                          some reasons, as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_param_get (

    aui_hdl smc_handle,

    aui_smc_param *p_smc_param

    );

/**
@brief          Function used to set the communication parameters to the interface
                device

@note           After a successful PPS exchange, user needs to set interface device
                according to the PPS

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card instance
@param[in]      p_smc_param             = Pointer to a struct #aui_smc_param which
                                          collect the communication parameters
                                          of the Smart Card interface device to
                                          be set

@return         @b AUI_RTN_SUCCESS      = Setting of the configuration parameters
                                          of the Smart Card interface device
                                          performed successfully
@return         @b Other_Values         = Setting of the configuration parameters
                                          of the Smart Card interface device failed
                                          for some reasons, as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_param_set (

    aui_hdl smc_handle,

    const aui_smc_param *p_smc_param

    );

/**
@brief          Function used to send a <b> Transmission Protocol Data Unit (TPDU) </b>
                command to the Smart Card and get the response from it according
                to the <b> ISO/IEC 7816 </b> Standard

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance
@param[in]      puc_write_data          = Pointer to the buffer intended to store
                                          the TPDU command to be sent to the
                                          Smart Card
@param[in]      n_number_to_write       = Length (in @a byte unit) of the TPDU
                                          command to be sent to the Smart Card

@param[out]     puc_response_data       = Pointer to the buffer intended to store
                                          the response to the TPDU command just
                                          sent. The response contains:
                                          - The response body (Data field)
                                          - The response trailer (SW1 SW2)
@param[in,out]  pn_number_read          = When [in], it specifies the expected
                                          length (in @a byte unit) of data to
                                          read \n
                                          When [out], it specifies the actual
                                          length (in @a byte unit) of TPDU
                                          response data

@return         @b AUI_RTN_SUCCESS      = Sending of the TPDU Command to the
                                          Smart Card performed successfully
@return         @b Other_Values         = Sending of the TPDU Command to the
                                          Smart Card failed for some reasons,
                                          as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_transfer (

    aui_hdl smc_handle,

    unsigned char *puc_write_data,

    int n_number_to_write,

    unsigned char *puc_response_data,

    int *pn_number_read

    );

/**
@brief          Function used to send a <b> Transmission Protocol Data Unit (TPDU) </b>
                Command to the Smart Card and get the response from it according
                to the <b> ISO/IEC 7816 </b>

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance
@param[in]      puc_hdr_body_buf        = Pointer to the buffer intended to
                                          store the TPDU command to be sent to
                                          the Smart Card
@param[in]      n_number_to_write       = Length (in @a byte unit) of the TPDU
                                          command to be sent to the Smart Card
@param[in]      n_timeout               = Maximum time for waiting for the
                                          response data
                                          - @b Caution: This parameter is not
                                               used currently then user can
                                               ignore it

@param[out]     pn_number_write         = Actual length (in @a byte unit) of
                                          data received from the Smart Card
@param[out]     puc_status_word         = Pointer to the buffer intended to
                                          store the response to the TPDU command
                                          just sent
                                          - @b Caution: The expected response
                                               just contains the response trailer
                                               (SW1 SW2)

@return         @b AUI_RTN_SUCCESS      = Sending of the TPDU Command to the
                                          Smart Card performed successfully
@return         @b Other_Values         = Sending of the TPDU Command to the
                                          Smart Card failed for some reasons,
                                          as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>

@note           Currently this function @a only support <b> T0 Protocol </b> of
                <b> ISO/IEC </b> Standard
*/
AUI_RTN_CODE aui_smc_send (

    aui_hdl smc_handle,

    unsigned char *puc_hdr_body_buf,

    int n_number_to_write,

    int *pn_number_write,

    unsigned char *puc_status_word,

    int n_timeout

    );

/**
@brief          Function used to receive the TPDU responses from the Smart Card
                according to the <b> ISO/IEC 7816 </b> Standard

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance
@param[in]      puc_readcmd_hdr         = Pointer to the buffer intended to store
                                          the TPDU command to be sent to the
                                          Smart Card
                                          - @b Caution: The expected TPDU
                                               command is just contains the
                                               Command header
@param[in]      n_number_to_read        = Size (in @a byte unit) of the buffer
                                          intended to store the response data
                                          from the Smart Card
@param[in]      n_timeout               = Maximum time for waiting for the
                                          response data
                                          - @b Caution: This parameter is not
                                               uses currently then user can
                                               ignore it

@param[out]     puc_read_buf            = Pointer to the buffer intended to store
                                          the response data from the Smart Card.
                                          The response contains:
                                          - The response body (Data field)
                                          - The response trailer (SW1 SW2)
@param[out]     pn_number_read          = Actual length (in @a byte unit) of the
                                          response data received from the Smart
                                          Card
@param[out]     puc_status_word         = Pointer to the buffer intended to
                                          store the response to the TPDU command
                                          just sent to the Smart Card

@return         @b AUI_RTN_SUCCESS      = Sending of the TPDU Command to the
                                          Smart Card performed successfully
@return         @b Other_Values         = Sending of the TPDU Command to the Smart
                                          Card failed for some reasons, as defined
                                          in the enum #aui_smc_errno_type and in
                                          the <b> ERRNO_SYS Module </b>

@note           Currently this function @a only support <b> T0 Protocol </b> of
                <b> ISO/IEC </b> Standard
*/
AUI_RTN_CODE aui_smc_receive (

    aui_hdl smc_handle,

    unsigned char *puc_readcmd_hdr,

    unsigned char *puc_read_buf,

    int n_number_to_read,

    int *pn_number_read,

    unsigned char *puc_status_word,

    int n_timeout

    );

/**
@brief          Function used to detect the Smart Card status

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance

@return         @b AUISMC_ERROR_IN      = A Smart Card has been plugged in
@return         @b AUISMC_ERROR_OUT     = No Smart Card has been plugged in
@return         @b AUISMC_ERROR_READY   = A Smart Card has been plugged in and
                                          is ready to transfer data
*/
AUI_RTN_CODE aui_smc_detect (

    aui_hdl smc_handle

    );

/**
@brief          Function used to active the Smart Card in order to start work
                with it

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance

@return         @b AUI_RTN_SUCCESS      = Activating of the Smart Card performed
                                          successfully
@return         @b Other_Values         = Activating of the Smart Card failed
                                          for some reasons, as defined in the
                                          enum #aui_smc_errno_type and in the
                                          <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_active (

    aui_hdl smc_handle

    );

/**
@brief          Function used to de-active the Smart Card in order to stop work
                with it

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance

@return         @b AUI_RTN_SUCCESS      = De-Activating of the Smart Card
                                          performed successfully
@return         @b Other_Values         = De-Activating of the Smart Card
                                          failed for some reasons, as defined
                                          in the enum #aui_smc_errno_type and
                                          in the <b> ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_deactive (

    aui_hdl smc_handle

    );

/**
@brief          Function used to perform a Protocol & Parameter Selection (PPS)
                exchange

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance
@param[in]      puc_write_data          = Pointer to the buffer intended to
                                          store PPS command
@param[in]      number_to_writelen      = Length (in @a byte unit) of the data
                                          to be sent to the Smart Card

@param[out]     puc_response_data       = Pointer to the buffer intended to store
                                          the response data from the Smart Card
@param[out]     pn_response_datalen     = Length (in @a byte unit) of the response
                                          data

@return         @b AUI_RTN_SUCCESS      = PPS exchange performed successfully
@return         @b Other_Values         = PPS exchange failed for some reasons,
                                          as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_setpps (

    aui_hdl smc_handle,

    unsigned char *puc_write_data,

    int number_to_writelen,

    unsigned char *puc_response_data,

    int *pn_response_datalen

    );

/**
@brief          Function used to check the real presence of the Smart Card in the
                card slot

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance

@return         @b AUI_RTN_SUCCESS      = The Smart Card is present in the card
                                          slot
@return         @b AUI_RTN_FAIL         = The Smart Card in not present in the
                                          card slot
*/
AUI_RTN_CODE aui_smc_isexist (

    aui_hdl smc_handle

    );

/**
@brief          Function used to perform miscellaneous setting to the Smart Card

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card
                                          instance
@param[in]      cmd                     = The command to be executed for
                                          performing a specific setting, as
                                          defined in the struct #aui_smc_cmd_t
@param[in]      pv_param                = Pointer to the parameter of the command
                                          for performing a specific setting, as
                                          defined in the struct #aui_smc_cmd_t

@return         @b AUI_RTN_SUCCESS      = Specific setting performed successfully
@return         @b Other_Values         = Specific setting failed for some
                                          reasons, as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_set (

    aui_hdl smc_handle,

    aui_smc_cmd cmd,

    void *pv_param

    );

/**
@brief          Function used to read data from the Smart Card in raw mode.\n

@note           The application should manages the communication protocol.

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card instance
@param[in]      puc_data                = Specific the buffer intended to store the
                                          bytes received from the Smart Card
@param[in]      size                    = Specific the size (in @a byte unit) to
                                          read from the Smart Card

@param[out]     ps_actlen               = Pointer to the actual length (in @a byte
                                          unit) of the response data received from
                                          the Smart Card

@return         @b AUI_RTN_SUCCESS      = Specific reading performed successfully
@return         @b Other_Values         = Specific reading failed for some
                                          reasons, as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_raw_read (

    aui_hdl smc_handle,

    unsigned char *puc_data,

    short size,

    short *ps_actlen

    );

/**
@brief          Function used to send data to the Smart Card in raw mode.\n

@note           The application should manages the communication protocol.

@param[in]      smc_handle              = #aui_hdl handle of the Smart Card instance
@param[in]      puc_data                = Pointer to the buffer intended to store
                                          the data to be sent to the Smart Card
@param[in]      size                    = Length (in @a byte unit) of data to be
                                          sent to the Smart Card

@param[out]     ps_actlen               = Pointer to the actual length (in @a byte
                                          unit) of data to be sent to the Smart Card

@return         @b AUI_RTN_SUCCESS      = Specific writing performed successfully
@return         @b Other_Values         = Specific writing failed for some
                                          reasons, as defined in the enum
                                          #aui_smc_errno_type and in the <b>
                                          ERRNO_SYS Module </b>
*/
AUI_RTN_CODE aui_smc_raw_write (

    aui_hdl smc_handle,

    unsigned char *puc_data,

    short size,

    short *ps_actlen

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define EM_AUISMC_STANDARD_ISO AUI_SMC_STANDARD_ISO

#define EM_AUISMC_STANDARD_EMV2000 AUI_SMC_STANDARD_EMV2000

#define EM_AUISMC_STANDARD_IRDETO AUI_SMC_STANDARD_IRDETO

#define EM_AUISMC_STANDARD_MAX AUI_SMC_STANDARD_MAX

#define EM_AUISMC_PROTOCOL_UNKNOWN AUI_SMC_PROTOCOL_UNKNOWN

#define EM_AUISMC_PROTOCOL_T0 AUI_SMC_PROTOCOL_T0

#define EM_AUISMC_PROTOCOL_T1 AUI_SMC_PROTOCOL_T1

#define EM_AUISMC_PROTOCOL_T14 AUI_SMC_PROTOCOL_T14

#define EM_AUISMC_PROTOCOL_T1_ISO AUI_SMC_PROTOCOL_T1_ISO

#define EM_AUISMC_PROTOCOL_MAX AUI_SMC_PROTOCOL_MAX

#define EM_AUISMC_STOP_0 AUI_SMC_STOP_0

#define EM_AUISMC_STOP_1 AUI_SMC_STOP_1

#define EM_AUISMC_STOP_2 AUI_SMC_STOP_2

#define EM_AUISMC_STOP_3 AUI_SMC_STOP_3

#define EM_AUISMC_STOP_MAX AUI_SMC_STOP_MAX

#define EM_AUISMC_8BITS_NO_PARITY AUI_SMC_8BITS_PARITY_NONE

#define EM_AUISMC_8BITS_ODD_PARITY AUI_SMC_8BITS_PARITY_ODD

#define EM_AUISMC_8BITS_EVEN_PARITY AUI_SMC_8BITS_PARITY_EVEN

#define EM_AUISMC_PARITY_MAX  AUI_SMC_8BITS_PARITY_MAX

#define EM_AUISMC_CMD_SET_WWT AUI_SMC_CMD_SET_WWT

#define AUISMC_ERROR_BAD_PARAMETER AUI_SMC_ERROR_BAD_PARAMETER

#define AUISMC_ERROR_ERROR_PARITY AUI_SMC_ERROR_ERROR_PARITY

#define AUISMC_ERROR_ERROR_ANSWER AUI_SMC_ERROR_ERROR_ANSWER

#define AUISMC_ERROR_TIMEOUT AUI_SMC_ERROR_TIMEOUT

#define AUISMC_ERROR_OUT AUI_SMC_ERROR_OUT

#define AUISMC_ERROR_IN AUI_SMC_ERROR_IN

#define AUISMC_ERROR_CARD_BUSY AUI_SMC_ERROR_CARD_BUSY

#define AUISMC_ERROR_NOT_LOCK_OWNER AUI_SMC_ERROR_NOT_LOCK_OWNER

#define AUISMC_ERROR_READY AUI_SMC_ERROR_READY

#define AUISMC_ERROR_MUTE AUI_SMC_ERROR_MUTE

#define AUISMC_ERROR_ERROR_CARD AUI_SMC_ERROR_ERROR_CARD

#define AUISMC_ERROR_INVALID_PROTOCOL AUI_SMC_ERROR_INVALID_PROTOCOL

#define AUISMC_ERROR_NOT_RESET AUI_SMC_ERROR_NOT_RESET

#define AUISMC_ERROR_FAILURE AUI_SMC_ERROR_FAILURE

#define AUISMC_ERROR_NOT_SUPPORT AUI_SMC_ERROR_NOT_SUPPORT

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

