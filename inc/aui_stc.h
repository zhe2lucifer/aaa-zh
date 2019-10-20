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
Current ALi author: Wendy.He, Amu.tu
Last update:        2017.02.25
-->

@file aui_stc.h

@brief  System Time Clock (STC) Module

        <b> System Time Clock (STC) Module </b> is used to get the current time
        of the playing audio and/or video.

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_AV_STC_H

#define _AUI_AV_STC_H

/*************************Included Header File List***************************/

#include "aui_common.h"


#ifdef __cplusplus

extern "C" {

#endif

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> STC Module Module </b> to specify the attributes
        available to be configured for the different synchronization of video
        and audio for PiP scenario
        </div> @endhtmlonly

        Struct to specify the attributes available to be configured for
        synchronization Module by the function #aui_stc_avsync_set.
*/
typedef struct aui_stc_avsync_attr {
    /**
    Member to specify the type of data to be injected into audio and video as
    defined in the enum #aui_av_data_type
    */
    aui_av_data_type data_type;

    /**
    Member to specify the different synchronization mode of video and audio as
    defined in the enum #aui_stc_avsync_mode
    */
    aui_stc_avsync_mode sync_mode;

    /**
    Member to specify the video handle of which video will do the synchronization
    */
    aui_hdl decv_handle;
	
    /**
    Member to specify the video free run threadhold, the unit is a millisecond.
    If the difference between video pts and STC is greater than this threadhold,
    video will free run.
    If 0 is set, the driver will use the default value 15 second.
    */
    unsigned long video_freerun_threadhold;
} aui_stc_avsync_attr;


/*****************************Global Function List*****************************/

/**
@brief        Function used to perform the initialization of the STC Module
              before its opening by the function #aui_stc_open

@warning      This function can be used @a only in the <b> Pre-Run stage </b>
              of the STC Module

@param[in]    p_callback_init        = Callback function used to initialize the
                                       STC Module, as per comment for the function
                                       pointer #p_fun_cb
@param[in]    *pv_param              = Input parameter of the callback function
                                       @b p_callback_init
                                       - @b Caution: The callback function
                                            @b p_call_back_init @a must take a
                                            input parameter, anyway it may take
                                            @b NULL value as well

@return       @b AUI_RTN_SUCCESS     = STC Module initialized successfully
@return       @b AUI_RTN_FAIL        = Initialization of STC module failed

@note         The return value of this function is the return value of the
              callback function @b p_callback_init, which is the @a first
              parameter [in] of this function.\n
              Furthermore, if the @a first parameter [in] of this function
              is @b NULL then the return value is always @b AUI_RTN_SUCCESS.
*/
AUI_RTN_CODE aui_stc_init (

    p_fun_cb p_callback_init,

    void *pv_param

    );

/**
@brief        Function used to perform the de-initialization of the STC Module
              after its closing by the function #aui_stc_close

@param[in]    p_callback_de_init     = Callback function used to de-initialize
                                       the STC Module, as per comment for the
                                       function pointer #p_fun_cb
@param[in]    *pv_param              = Input parameter of the callback function
                                       @b p_callback_de_init
                                       - @b Caution: The callback function
                                            @b p_call_back_de_init @a must
                                            take a input parameter, anyway it
                                             may take @b NULL value as well

@return       @b AUI_RTN_SUCCESS     = STC Module de-initialized successfully
@return       @b AUI_RTN_FAIL        = De-Initialization of STC Module failed

@note         The return value of this function is the return value of the
              callback function @b p_callback_de_init, which is the @a first
              parameter [in] of this function.\n
              Furthermore, if the @a first parameter [in] of this function
              is @b NULL then the return value is always @b AUI_RTN_SUCCESS.
*/
AUI_RTN_CODE aui_stc_de_init (

    p_fun_cb p_callback_de_init,

    void *pv_param

    );

/**
@brief        Function used to open the STC Module then get the related handle

@warning      This function can @a only be used in the <b> Pre-Run stage </b>
              of the STC Module, in particular after performing the
              initialization of the STC Module by the function #aui_stc_init

@param[out]   p_handle               = #aui_hdl pointer to the handle of the STC
                                       Module just opened

@return       @b AUI_RTN_SUCCESS     = STC Module opened successfully then user
                                       can start to get the current STC
                                       information
@return       @b AUI_RTN_FAIL        = Opening of the STC Module failed
*/
AUI_RTN_CODE aui_stc_open (

    aui_hdl *p_handle

    );

/**
@brief        Function used to close the STC Module already opened by the
              function #aui_stc_open then the related handle will be released
              (i.e. the related resources such as memory, device).\n
              After closing the STC Module, user can perform the
              de-initialization of the STC Module by the function
              #aui_stc_de_init

@warning      This function can @a only be used in the <b> Post-Run stage </b>
              of the STC Module in pair with its the opening by the function
              #aui_stc_open.\n


@param[in]    handle                 = The handle of the STC Module already
                                       opened and to be closed

@return       @b AUI_RTN_SUCCESS     = STC Module closed successfully
@return       @b AUI_RTN_FAIL        = Closing of STC Module failed
*/
AUI_RTN_CODE aui_stc_close (

    aui_hdl handle

    );

/**
@brief          Function used to get the current time of the playing audio and/or
                video, after opening STC Module by the function #aui_stc_open

@param[in]      handle               = #aui_hdl handle of STC Module already
                                       opened and to be checked for the current
                                       time of the playing audio and/or video

@param[out]     pul_val_out          = The value of the current time with unit:
                                       - Either @b 45KHZ for playing <b> TS
                                         Stream </b>
                                       - Or <b> ms (millisecond) </b> for playing
                                         stream by <b> AV-Injector Module </b>
                                         (@a only for projects based on <b> Linux
                                         OS</b>)

@return         @b AUI_RTN_SUCCESS   = Checking for the current time performed
                                       successfully
@return         @b AUI_RTN_FAIL      = Checking for the current time failed
*/
AUI_RTN_CODE aui_stc_get (

    aui_hdl handle,

    unsigned long *pul_val_out

    );


/**
@brief          Function used to configure avsync when playing for swapping for
                PiP, after opening STC Module by the function #aui_stc_open

@param[in]      stc                 = #aui_hdl handle of STC Module already
                                      opened and to be checked for the current
                                      time of the playing audio and/or video

@param[in]      p_avsync_attr       = The pointer as per the description of the
                                      desired synchronization attributes to be
                                      set

@return         @b AUI_RTN_SUCCESS  = Setting of the desired synchronization
                                      attributes performed successfully
successfully
@return         @b AUI_RTN_FAIL     = Setting of the desired synchronization
                                      attributes failed

@note           @b 1. User @a must use this function to configure synchronization when
                      playing PiP

@note           @b 2. This function is used @a only in projects based on <b> LINUX OS </b>
*/
AUI_RTN_CODE aui_stc_avsync_set (

    aui_hdl stc,

    aui_stc_avsync_attr *p_avsync_attr

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

/**
@warning    Since this function in no longer implemented then will be
            deprecated soon, no detail is provided therefore user can
            definitely ignore it
*/
AUI_RTN_CODE aui_stc_set (

    aui_hdl stc,

    unsigned long long val

    );

/**
@warning    Since this function in no longer implemented then will be
            deprecated soon, no detail is provided therefore user can
            definitely ignore it
*/
AUI_RTN_CODE aui_stc_pause (

  aui_hdl stc,

  int pause

  );

/**
@warning    Since this function in no longer implemented then will be
            deprecated soon, no detail is provided therefore user can
            definitely ignore it
*/
AUI_RTN_CODE aui_stc_change_speed (

  aui_hdl stc,

  int ppm

  );

#define aui_stc_sync_mode aui_stc_avsync_mode

#define AUI_STC_SYNC_PTS  AUI_STC_AVSYNC_PCR

#define AUI_STC_SYNC_AUDIO  AUI_STC_AVSYNC_AUDIO

#define AUI_STC_SYNC_FREERUN  AUI_STC_AVSYNC_FREERUN

#define aui_stc_av_data_type aui_av_data_type

#define AUI_STC_AV_DATA_TYPE_NIM_TS AUI_AV_DATA_TYPE_NIM_TS

#define AUI_STC_AV_DATA_TYPE_RAM_TS AUI_AV_DATA_TYPE_RAM_TS

#define AUI_STC_AV_DATA_TYPE_ES AUI_AV_DATA_TYPE_ES

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

