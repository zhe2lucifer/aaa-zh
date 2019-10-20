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

@file   aui_tsi.h

@brief  Transport Stream Switch Interface (TSI) Module

        <b> Transport Stream Switch Interface (TSI) Module </b> is used for TS
        Data Input from NIM or TSG Module to output to DMX Module

@note   For further details, please refer to the ALi document
        <b><em>
        ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Transport Stream Switch
        Interface (TSI) Module"
        </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_TSI_H

#define _AUI_TSI_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Type List******************************/

/**
Enum to specify the <b> TSI Input Data Bus Attribute </b> which is related to
the circuit of the user board.
*/
typedef enum aui_tsi_in_conf {

    /**
    Value to specify the @b polarity of the <b> input data valid signal </b>
    */
    AUI_TSI_IN_CONF_VALID_SIG_POL = (1<<0),

    /**
    Value to specify the @b polarity of the <b> input synchronization signal </b>
    */
    AUI_TSI_IN_CONF_SYNC_SIG_POL = (1<<1),

    /**
    Value to specify the @b polarity of the <b> input error signal </b>
    */
    AUI_TSI_IN_CONF_ERR_SIG_POL = (1<<2),

    /**
    Value to specify the @b order of the <b> input data </b>
    */
    AUI_TSI_IN_CONF_SSI_BIT_ORDER = (1<<3),

    /**
    Value to specify the @b polarity of the <b> input clock signal </b>
    */
    AUI_TSI_IN_CONF_SSI_CLOCK_POL = (1<<4),

    /**
    Value to specify the @b enabling of the <b> Synchronous Serial Interface
    (SSI) data bus </b>
    */
    AUI_TSI_IN_CONF_SSI_ENABLE = (1<<5),

    /**
    Value to specify the @b enabling of the <b> SSI2B data bus </b>

    @note SSI2B is a data bus defined by ALi, please contact ALi R&D Dept.
          for more clarifications
    */
    AUI_TSI_IN_CONF_SSI2B_ENABLE = (1<<6),

    /**
    Value to specify the @b enabling of the <b> Serial Peripheral Interface
    (SPI) data bus </b>
    */
    AUI_TSI_IN_CONF_SPI_ENABLE = (1<<7)

} aui_tsi_in_conf;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Transport Stream Switch Interface (TSI) Module </b>
        to specify the TSI input data bus attributes
        </div> @endhtmlonly

        Struct to specify the TSI input data bus attributes
*/
typedef struct aui_attr_tsi {

    /**
    Member variable whose least significant 8 bits are valid to specify the
    value of the TSI input data bus attributes. In particular, each bit refers
    to a value of the enum #aui_tsi_in_conf value as below:

    - <b> bit 0 </b> refers to #AUI_TSI_IN_CONF_VALID_SIG_POL and can take the
      value
      - @b 1 = Active high
      - @b 0 = Active low

    - <b> bit 1 </b> refers to #AUI_TSI_IN_CONF_SYNC_SIG_POL and can take the
      value
      - @b 1 = Active high
      - @b 0 = Active low

    - <b> bit 2 </b> refers to #AUI_TSI_IN_CONF_ERR_SIG_POL and can take the
      value
      - @b 1 = Active high
      - @b 0 = Active low

      @b Caution: This bit is @a only valid when the data bus is not TSG SPI

    - <b> bit 3 </b> refers to #AUI_TSI_IN_CONF_SSI_BIT_ORDER and can take the
      value
      - @b 1 = The first bit of the 8 bits in serial TS is the LSB of the byte
      - @b 0 = The first bit of the 8 bits in serial TS is the MSB of the byte
               (i.e. normal order)

      @b Caution: This bit is @a only valid when the data bus is SSI or SSI2B

    - <b> bit 4 </b> refers to #AUI_TSI_IN_CONF_SSI_CLOCK_POL and can take the
      value
      - @b 1 = Clock polarity will be inverted,
      - @b 0 = Clock polarity will not be changed

      @b Caution: This bit is @a only valid when the data bus  is SSI or SSI2B

    - <b> bit 5 </b> refers to #AUI_TSI_IN_CONF_SSI_ENABLE and can take the
      value
       - @b 1 = Enable SSI data bus
       - @b 0 = Disable SSI data bus

      @b Caution: Do not set this bit as @b 1 if you are not using SSI data bus

    - <b> bit 6 </b> refers to #AUI_TSI_IN_CONF_SSI2B_ENABLE and can take the
      value
       - @b 1 = Enable SSI2B data bus
       - @b 0 = Disable SSI2B data bus

      @b Caution: Do not set this this bit as @b 1 if SSI2B data bus is not
                  under using

    - <b> bit 7 </b> refers to #AUI_TSI_IN_CONF_SPI_ENABLE and can take the
      value
       - @b 1 = Enable SPI data bus
       - @b 0 = Disable SPI data bus

       @b Caution: Do not set this bit as @b 1 if SPI data bus is not under
                   using
    */
    unsigned long ul_init_param;

} aui_attr_tsi, *aui_p_attr_tsi;

/**
Enum to specify the type of TSI input data bus and related index, which is
related to the circuit of the user board.

@note  Five (5) data bus types can be used in TSI:
       - @b SPI     = Standard SPI data bus type, which data width is 8 bits
       - @b TSG SPI = SPI data bus that can be only used in TSG
       - @b SSI     = Standard serial data bus, which data width is 1 bits
       - @b SSI2B   = Data bus defined by ALi, which data width is 2 bits
                      (please contact ALi R&D Dept. for more clarifications)
       - @b SSI4B   = Data bus defined by ALi, which data width is 4 bits
                      (currently not used,  please contact ALi R&D Dept. for
                      more clarifications)
*/
typedef enum aui_tsi_input_id {

    /**
    Value to specify the @b SPI data bus and <b> index 0 </b>
    */
    AUI_TSI_INPUT_SPI_0,

    /**
    Value to specify the @b SPI data bus and <b> index 1 </b>
    */
    AUI_TSI_INPUT_SPI_1,

    /**
    Value to specify the <b> TSG SPI </b> data bus
    */
    AUI_TSI_INPUT_TSG,

    /**
    Value to specify the @b SPI data bus and <b> index 3 </b>
    */
    AUI_TSI_INPUT_SPI_3,

    /**
    Value to specify the @b SSI data bus and <b> index 0 </b>
    */
    AUI_TSI_INPUT_SSI_0,

    /**
    Value to specify the @b SSI data bus and <b> index 1 </b>
    */
    AUI_TSI_INPUT_SSI_1,

    /**
    @warning  This value is not used currently, user can ignore it
    */
    AUI_TSI_INPUT_PARA,

    /**
    @warning  This value is not used currently, user can ignore it
    */
    AUI_TSI_INPUT_RESERVED,

    /**
    Value to specify the @b SSI2B data bus and <b> index 0 </b>
    */
    AUI_TSI_INPUT_SSI2B_0,

    /**
    Value to specify the @b SSI2B data bus and <b> index 1 </b>
    */
    AUI_TSI_INPUT_SSI2B_1,

    /**
    Value to specify the @b SSI4B data bus and <b> index 0 </b> (currently not
    use, please contact ALi R&D Dept. for more clarifications)
    */
    AUI_TSI_INPUT_SSI4B_0,

    /**
    Value to specify the @b SSI4B data bus and <b> index 1 </b> (currently not
    use, please contact ALi R&D Dept. for more clarifications)
    */
    AUI_TSI_INPUT_SSI4B_1,

    /**
    Value to specify the @b SSI data bus and <b> index 2 </b>
    */
    AUI_TSI_INPUT_SSI_2,

    /**
    Value to specify the @b SSI data bus and <b> index 3 </b>
    */
    AUI_TSI_INPUT_SSI_3,

    /**
    Value to specify the @b SSI2 data bus and <b> index 2 </b>
    */
    AUI_TSI_INPUT_SSI2B_2,

    /**
    Value to specify the @b SSI2B data bus and <b> index 3 </b>
    */
    AUI_TSI_INPUT_SSI2B_3,

    /**
    Value to specify the @b SSI4B data bus and <b> index 2 </b> (currently not
    use, please contact ALi R&D Dept. for more clarifications)
    */
    AUI_TSI_INPUT_SSI4B_2,

    /**
    Value to specify the @b SSI4B data bus and <b> index 3 </b> (currently not
    use, please contact ALi R&D Dept. for more clarifications)
    */
    AUI_TSI_INPUT_SSI4B_3,

    /**
    Value to specify the total number of maximum data bus available
    */
    AUI_TSI_INPUT_MAX

} aui_tsi_input_id;

/**
Enum to specify which DMX device is the recipient of the TSI output data
*/
typedef enum aui_tsi_output_id {

    /**
    Value to specify that <b> DMX 0 </b> is the recipient of the TSI output
    data
    */
    AUI_TSI_OUTPUT_DMX_0 = 0,

    /**
    Value to specify that <b> DMX 1 </b> is the recipient of the TSI output
    data
    */
    AUI_TSI_OUTPUT_DMX_1,

    /**
    Value to specify that <b> DMX 2 </b> is the recipient of the TSI output
    data
    */
    AUI_TSI_OUTPUT_DMX_2,

    /**
    Value to specify that <b> DMX 3 </b> is the recipient of the TSI output
    data
    */
    AUI_TSI_OUTPUT_DMX_3

} aui_tsi_output_id;

/**
Enum to specify the TSI channel in an ALi SOC

@note  There are four (4) TSI channels an ALi SOC which can can work at the
       same time
*/
typedef enum aui_tsi_channel_id {

    /**
    Value to specify the @b first TSI channel in an ALi SOC
    */
    AUI_TSI_CHANNEL_0 = 1,

    /**
    Value to specify the @b second TSI channel in an ALi SOC
    */
    AUI_TSI_CHANNEL_1,

    /**
    Value to specify the @b third TSI channel in an ALi SOC
    */
    AUI_TSI_CHANNEL_2,

    /**
    Value to specify the @b fourth TSI channel in an ALi SOC
    */
    AUI_TSI_CHANNEL_3

} aui_tsi_channel_id;

#ifdef __cplusplus

extern "C" {

#endif

/*****************************Global Function List*****************************/

/**
@brief          Function used to get the AUI TSI module version number

@param[out]     pul_version         = Pointer to the version number of TSI Module

@return         @b AUI_RTN_SUCCESS  = Getting of the version number of TSI Module
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = The output parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Getting of the version number of TSI Module
                                      failed for some reason
@note
*/
AUI_RTN_CODE aui_tsi_version_get (

    unsigned long *pul_version

    );

/**
@brief          Function used to open the TSI module then get the related handle

@warning        This function can @a only be used in the <b> Pre-Run Stage </b>
                of the TSI Module, in particular after closing the TSI Module
                by the function aui_tsi_close

@param[out]     p_handle            = #aui_hdl pointer to the handle of the TSI
                                      Module just opened

@return         @b AUI_RTN_SUCCESS  = TSI Module opened successfully
@return         @b Other_Values     = Opening of the TSI Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_tsi_open (

    aui_hdl *p_handle

    );

/**
@brief          Function used to set the specific TSI input data bus attribute

@param[in]      handle              = #aui_hdl handle of the TSI Module already
                                      opened
@param[in]      input_id            = The specific type of TSI input data bus
                                      and related index to be set, as defined
                                      in the enum #aui_tsi_input_id
@param[in]      p_attr_tsi          = The specific TSI input data bus attribute
                                      to be set, as defined in the struct
                                      #aui_attr_tsi

@return         @b AUI_RTN_SUCCESS  = Setting of the specific TSI input data
                                      bus attribute performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_values     = Setting of the specific TSI input data
                                      bus attribute failed for some reasons
*/
AUI_RTN_CODE aui_tsi_src_init (

    aui_hdl handle,

    aui_tsi_input_id input_id,

    aui_attr_tsi *p_attr_tsi

    );

/**
@brief          Function used to set the route of the TS data path, in particular
                it will set the specific TSI input data bus to a TSI channel,
                and then the TSI channel output data to a DMX device \n\n

                @b Example: SPI 0 -> TSI channel 0 -> DMX 0

@pre            Precondition to use this function is set the specific TSI input
                data bus attribute by the function #aui_tsi_src_init

@warning        The TS input data bus can connect to any TSI channel but not all
                the TS channel can connect to any DMX device. With reference to
                the enum #aui_tsi_channel_id, the limitation is summarized below:
                - #AUI_TSI_CHANNEL_0 and #AUI_TSI_CHANNEL_1 can output data to
                  <b> DMX 0,1,2,3 </b>
                - #AUI_TSI_CHANNEL_2 can @a only output data to <b> DMX 2 </b>
                - #AUI_TSI_CHANNEL_3 can @a only output data to <b> DMX 3 </b>
                - @a Only #AUI_TSI_CHANNEL_0 can output data to <b> CI
                  controller </b>

@param[in]      handle              = #aui_hdl handle of the TSI Module already
                                      opened

@param[in]      input_id            = The specific type of TSI input data bus
                                      and related index to be set, as defined
                                      in the enum #aui_tsi_input_id

@param[in]      tsi_channel_id      = The TSI channel in ALi SOC to be connected
                                      to the specific TSI input data bus, please
                                      refer to enum #aui_tsi_channel_id

@param[in]      dmx_id              = The specific DMX device ID to be the
                                      recipient of the TSI output data, as
                                      defined in the enum #aui_tsi_output_id

@return         @b AUI_RTN_SUCCESS  = Setting of the route of TS data path
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = The input parameters (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Setting of the route of the TS data path
                                      failed for some reasons
*/
AUI_RTN_CODE aui_tsi_route_cfg (

    aui_hdl handle,

    aui_tsi_input_id input_id,

    aui_tsi_channel_id tsi_channel_id,

    aui_tsi_output_id dmx_id

    );

/**
@brief          Function used to close the TSI Module already opened by the
                function aui_tsi_open then the related handle will be released
                (i.e. the related resources such as memory, device)

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of the TSI Module in pair with its opening by the function
                #aui_tsi_open

@param[in]      handle               = #aui_hdl handle of the TSI Module already
                                       opened and to be closed

@return         @b AUI_RTN_SUCCESS   = TSI Module closed successfully
@return         @b AUI_RTN_EINVAL    = THe input parameters (i.e. [in]) is invalid
@return         @b Other_Values      = Closing of the TSI Module failed for some
                                       reasons
*/
AUI_RTN_CODE aui_tsi_close (

    aui_hdl handle

    );

/**
@brief          Function used to set whether the TS data will be bypassed to CI
                card or not

@param[in]      handle               = #aui_hdl handle of the TSI Module already
                                       opened
@param[in]      ul_card_id           = The ID of the CI card as bypass of the TS
                                       data
                                       - @b Caution: Since ALi SOC @a only
                                            support one (1) CI card, this input
                                            parameter can @a only be set as @a 0
                                            currently
@param[in]      uc_bypass            = Flag to enable/disable the bypass of TS
                                       data to the selected CI card, in particular
                                       - @b 0 = Pass TS data to CI card
                                       - @b 1 = Do not pass TS data to CI card,
                                                then TS data will be bypassed
                                                from the CI controller

@return         @b AUI_RTN_SUCCESS   = Bypassing of the TS data to CI Card
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameters (i.e. [in])
                                       is invalid
@return         @b Other_Values      = Bypassing of the TS data to CI Card failed
                                       for some reasons

@note       This function works only when the CI card is connected correctly.\n
            When the CI card is removed, the TS data will be bypassed to the DMX
            device directly without going through CI card, regardless of the
            bypass setting of this function
*/
AUI_RTN_CODE aui_tsi_ci_card_bypass_set (

    aui_hdl handle,

    unsigned long ul_card_id,

    unsigned char uc_bypass

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define AUI_TSI_IN_CONF_SSI_ORDER     (1<<4)

#define AUI_TSI_IN_CONF_SSI_CLK_POL   (1<<3)

#define AUI_TSI_IN_CONF_ENABLE (1<<7)

enum aui_tsi_ciconnect_mode {

    AUI_TSI_CI_MODE_CHAIN,

    AUI_TSI_CI_MODE_PARALLEL,

};

enum aui_en_tsi_item_set {

    AUI_TSI_CI_PARALL_MODE_SET,

    AUI_TSI_CI_CARD_B_SRC_SET
};

enum aui_en_tsi_item_get {

    AUI_TSI_CI_PARALL_MODE_GET,

    AUI_TSI_CI_CARD_B_SRC_GET

};

#define aui_tsi_CICard_bypass_set aui_tsi_ci_card_bypass_set

#define aui_tsi_cicard_bypass_set(card_id, bypass)  aui_tsi_ci_card_bypass_set(0, card_id, bypass)

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

