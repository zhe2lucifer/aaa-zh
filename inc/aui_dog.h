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

@file aui_dog.h

@brief  Watchdog (DOG) Module.

    <b> Watchdog (DOG) Module </b> is a independent chip control unit used
    to automatically detect software anomalies and reset the processor if
    any error occurs.\n\n
    Once user open the DOG device, a timer will start the counting then
    user should make sure to write data to the DOG Device within a timeout
    interval, considering that each time the writing operation causes the
    reset of the timer
    (as per the member @b ul_time_us of the struct #aui_attr_dog). If user
    does not write data within the timeout interval then the timer
    - Either will reboot the system (when <b> Watchdog Mode </b> is set)
    - Or will call the registered function and will be reset
      (when <b> Timer Mode </b> is set)

    About the <b> Timer Mode </b>:
    - It is @a only supported in project based on <b> TDS OS </b>.
    - It is used @a only to do repeated action: when the timeout interval
      occurs the system will call the function that is register, and in
      this function a corrective action is initiated
      (as per the callback function @b dog_cb which is a member of the
      struct #aui_attr_dog)

    About the <b> Watchdog Mode </b>:
    - It is supported by both <b> Linux OS </b> and <b> TDS Os </b>
    - It is used to monitor the system status: once system fail to write
      date to DOG device within the timeout interval, the DOG Device will
      think the system is abnormal then send a reset signal to the CPU to
      reset the system.

    The @a essential usage stepwise of DOG Module is described below:
    - Open the DOG Device
    - Configure the members of the struct #aui_attr_dog
    - Start the DOG Device

@note For further details, please refer to the ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Watchdog (DOG) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_DOG_H

#define _AUI_DOG_H

/**************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List*******************************/

/**
Macro to indicate the <b> version number of DOG Module </b>, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left to
right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_DOG  (0X00010000)

/**
Macro to denote that presently the structs contained in this header file are
designed to support up to one (1) Watchdog Device.
*/
#define AUI_DOG_DEV_CNT_MAX     (1)

/*******************************Global Type List*******************************/

/**
Enum to specify the different <b> Watchdog Work Mode </b> that can occur when
no data is written within the timeout interval.\n
The desired mode can be set in different way depending on the operating system
under running as below:
- In project based on <b> TDS OS </b>, please use the function #aui_dog_config
- In project based on <b> Linux OS </b>, please use the function #aui_dog_open
*/
typedef enum aui_dog_mode {

    /**
    Value to specify the <b> Watchdog Mode </b> with which the system will be
    rebooted
    */
    AUI_DOG_MODE_WATCHDOG = 0,

    /**
    Value to specify the <b> Timer Mode </b> with which the previous registered
    function will be called and the timer will be reset as well
    (as per comment for the function pointer @b dog_cb)
    */
    AUI_DOG_MODE_TIMER=1

} aui_dog_mode;


typedef void (*aui_dog_callback) (

    unsigned long ul_context

    );


/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Watchdog (DOG) Module </b> to specify a list of
        configuration attributes
        </div> @endhtmlonly

        Struct to specify a list of configuration attributes for DOG Module.
*/
typedef struct aui_attr_dog {

    /**
    Member as @a Index which integer values (from the value <b> zero (0) </b>)
    refer to different DOG Devices.
    */
    unsigned char uc_dev_idx;

    /**
    Member as <em> Watchdog Work Mode </em> (i.e. as the enum #AUI_DOG_MODE) to
    contain information related to the current Watchdog Work Mode.
    */
    aui_dog_mode ul_work_mode;

    /**
    Member to contain the interval time as the timeout interval after which the
    set Watchdog Work Mode will occur, as defined in the enum #AUI_DOG_MODE
    */
    unsigned long ul_time_us;

    /**
    Function pointer to specify the callback function registered by the function
    #aui_dog_config and to be called when the set Timer Mode will occur

    @note  This function pointer can be used @a only in projects based on
           <b> TDS OS </b>
    */
    aui_dog_callback dog_cb;

} aui_attr_dog;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief        Function used to get the version of the DOG Module as defined
              in the macro #AUI_MODULE_VERSION_NUM_DOG

@warning      This function can be used at any time

@param[out]   pul_version           = Pointer to the version number of DOG Module

@return       @b AUI_RTN_SUCCESS    = Getting of the version number of DOG Module
                                      performed successfully
@return       @b AUI_RTN_EINVAL     = The output parameter (i.e. [in]) is invalid
@return       @b Other_Values       = Getting of the version number of DOG Module
                                      failed for some reason
*/
AUI_RTN_CODE aui_dog_version_get (

    unsigned long *pul_version

    );

/**
@brief        Function used to perform the initialization of the DOG Module
              before its opening by the function #aui_dog_open

@warning      This function can be used @a only in the <b> Pre-Run Stage </b>
              of the DOG Module

@param[in]    p_call_back_init      = Callback function used for the initialization
                                      of the DOG Module, as per comment for the
                                      function pointer #p_fun_cb
@param[in]    pv_param              = The input parameter of the callback function
                                      @b p_call_back_init, which is the @a first
                                      parameter [in] of this function

@return       @b AUI_RTN_SUCCESS    = Watchdog Module initialized successfully
@return       @b AUI_RTN_EINVAL     = Either one or both input parameter (i.e.
                                      [in]) is invalid
@return       @b Other_Values       = Initializing of the DOG Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_dog_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief        Function used to perform the de-initialization of the DOG Module
              after its closing by the function #aui_dog_close

@param[in]    p_call_back_init      = #p_fun_cb callback function used for the
                                      de-initialization of the DOG Module, as
                                      per comment for the function pointer
                                      #p_fun_cb
@param[in]    pv_param              = Input parameter of the callback function
                                      @b p_call_back_init, which is the @a first
                                      parameter [in] of this function

@return       @b AUI_RTN_SUCCESS    = DOG Module de-initialized successfully
@return       @b AUI_RTN_EINVAL     = Either one or both input parameter (i.e.
                                      [in]) is invalid
@return       @b Other_Values       = De-initializing of the DOG Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_dog_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief        Function used to open and register the DOG Module and configure
              the desired attributes, then get the related handle

@warning      This function can @a only be used in the <b> Pre-Run Stage </b>
              of the DOG Module, in particular:
              - Either after performing the initialization of the DOG Module
                by the function #aui_dog_init for the first opening of the
                DOG Module
              - Or after closing the DOG Module by the function #aui_dog_close,
                considering the initialization of the DOG Module has been
                performed previously by the function #aui_dog_init

param[in]     *p_attr_dog           = Pointer to a struct #aui_attr_dog, which
                                      collects the configuration attributes for
                                      the DOG Module

param[out]    *pp_hdl_dog           = #aui_hdl pointer to the handle of the DOG
                                      Module just opened

@return       @b AUI_RTN_SUCCESS    = DOG Module opened successfully then user
                                      can start to configure the DOG device
@return       @b AUI_RTN_EINVAL     = Either one or both of the parameter (i.e.
                                      [in], [out]) are invalid
@return       @b Other_Values       = Opening of the DOG Module failed for some
                                      reasons
*/
AUI_RTN_CODE aui_dog_open (

    aui_attr_dog *p_attr_dog,

    aui_hdl *pp_hdl_dog

    );

/**
@brief        Function used to close the DOG Module already opened by the
              function #aui_dog_open then the related handle will be released
              (i.e. the related resources such as memory, device)

@warning      This function can @a only be used in the <b> Post-Run Stage </b>
              of the DOG Module in pair with its the opening by the function
              #aui_dog_open. After closing the DOG Module user can:
              - Either perform the de-initialization of the DOG Module by the
                function #aui_dog_de_init
              - Or open again the DOG Module by the function #aui_dog_open,
                considering the initialization of the DOG Module has been
                performed previously by the function #aui_dog_init

param[in]     *p_hdl_dog            = #aui_hdl pointer to the handle of the DOG
                                      Module already opened and to be closed

@return       @b AUI_RTN_SUCCESS    = DOG Module closed successfully
@return       @b AUI_RTN_EINVAL     = Either one or both of the input parameters
                                      (i.e. [in]) is invalid
@return       @b Other_Values       = Closing of the DOG Module failed for some
                                      reasons
*/
AUI_RTN_CODE aui_dog_close (

    aui_hdl p_hdl_dog

    );

/**
@brief        Function used to start the DOG Module after its opening by the
              function #aui_dog_open. After calling this function the timer
              will start the counting

@param[in]    p_hdl_dog             = Pointer to the handle of the DOG Module
                                      already opened then to be started
@param[in]    p_attr_dog            = Pointer to a struct #aui_attr_dog which
                                      collects the configuration attributes of
                                      the DOG Device to be started

@return       @b AUI_RTN_SUCCESS    = DOG Module started successfully
@return       @b AUI_RTN_EINVAL     = The input parameter (i.e. param[in])
                                      is invalid
@return       @b Other_Values       = Starting of the DOG Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_dog_start (

    void *p_hdl_dog,

    aui_attr_dog *p_attr_dog

    );

/**
@brief        Function used to stop DOG Device after its opening and starting
              by, respectively, the functions #aui_dog_open and #aui_dog_start

@param[in]    p_hdl_dog             = #aui_hdl handle of the DOG Module already
                                      opened and started then to be stopped
@param[in]    p_attr_dog            = At present the value of this parameter is
                                      reserved to ALi R&D Dept. then user can
                                      set it as @b NULL

@return       @b AUI_RTN_SUCCESS    = DOG Module stopped successfully.
@return       @b AUI_RTN_EINVAL     = The input parameter (i.e. [in]) is invalid
@return       @b Other_Values       = Stopping of the DOG Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_dog_stop (

    aui_hdl p_hdl_dog,

    aui_attr_dog *p_attr_dog

    );

/**
@brief        Function used to perform a specific setting of the DOG Device
              after its opening by the function #aui_dog_open

@attention    This function is still under developing so at present user can
              ignore it

@param[in]    p_hdl_dog             = #aui_hdl handle of the DOG Module already
                                      opened and to be managed to perform a
                                      specific setting
@param[in]    ul_item               = The item related to the specific setting
                                      of DOG Module to be performed
@param[in]    pv_param              = The pointer to the description of the
                                      specific setting of the DOG Module to be
                                      performed

@return       @b AUI_RTN_SUCCESS    = Specific setting of the DOG Module performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = At least one of the input parameter (i.e.
                                     [in]) is invalid
@return       @b Other_Values       = Specific setting of the DOG Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_dog_set (

    aui_hdl p_hdl_dog,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief        Function used to get information about a specific setting of
              the DOG Device performed by the function #aui_dog_set

@attention    This function is still under developing so at present user can
              ignore it

@param[in]    p_hdl_dog            = #aui_hdl handle of the DOG Module already
                                     opened and to be managed to get information
                                     about a specific setting.
@param[in]    ul_item              = The item related to the specific setting
                                     information of the DOG Module to be gotten
@param[in]    pv_param             = The pointer to the description of the
                                     specific setting of the DOG Module to be
                                     gotten

@return       @b AUI_RTN_SUCCESS   = Getting of DOG Device setting information
                                     performed successfully
@return       @b AUI_RTN_EINVAL    = At least one of the input parameter (i.e.
                                     [in]) is invalid
@return       @b Other_Values      = Getting of DOG Device setting information
                                     failed for some reasons
*/
AUI_RTN_CODE aui_dog_get (

    void *p_hdl_dog,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief        Function used to set the timeout interval for the timer to go
              into a Watchdog Mode

@param[in]    p_hdl_dog             = #aui_hdl handle of the DOG Module already
                                      opened and to be managed to set a timeout
                                      for the timer
@param[in]    ul_time_us            = Timeout interval to be set

@return       @b AUI_RTN_SUCCESS    = Setting of the timeout interval performed
                                      successfully
@return       @b AUI_RTN_EINVAL     = Either one or both the input parameter
                                      (i.e. [in]) is invalid.
@return       @b Other_Values       = Settings of the timeout interval failed
                                      for some reason
*/
AUI_RTN_CODE aui_dog_time_set (

    aui_hdl p_hdl_dog,

    unsigned long ul_time_us

    );

/**
@brief        Function used to get the timeout interval for the timer to go
              into a Watchdog Mode set by the function #aui_dog_time_set

@param[in]    p_hdl_dog            = #aui_hdl handle of the DECV Module already
                                     opened and to be managed to get a specific
                                     information

@param[out]   pul_time_us          = Pointer to a buffer to store the timeout
                                     interval just gotten

@return       @b AUI_RTN_SUCCESS   = Getting of the timeout interval performed
                                     successfully
@return       @b AUI_RTN_EINVAL    = Either one or both the parameters (i.e.
                                     [in], [out]) are invalid
@return       @b Other_Values      = Getting of the timeout interval failed
                                     for some reasons
*/
AUI_RTN_CODE aui_dog_time_get (

    aui_hdl p_hdl_dog,

    unsigned long *pul_time_us

    );

/**
@brief        Function used to configure the Watchdog Work Mode that will
              occur after the timeout interval set by the function
              #aui_dog_time_set.

@param[in]    p_hdl_dog             = #aui_hdl handle of the DECV Module already
                                      opened and to be configured for the Watchdog
                                      Work Mode
@param[in]    p_attr_dog            = Watchdog Work Mode to be set

@return       @b AUI_RTN_SUCCESS    = Configuration of the Watchdog Work Mode
                                      performed successfully
@return       @b AUI_RTN_EINVAL     = Either one or both the input parameter
                                      (i.e. [in]) is invalid
@return       @b Other_Values       = Configuration of the Watchdog Work Mode
                                      failed for some reason
*/
AUI_RTN_CODE aui_dog_config (

    aui_hdl p_hdl_dog,

    aui_attr_dog *p_attr_dog

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define AUI_DOG_MODE aui_dog_mode

#define aui_st_attr_dog aui_sttr_dog

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

