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
Current ALi Author: Wesley.He
Last update:        2017.02.25
-->

@file aui_trng.h

@brief  True Random Number Generator (TRNG) Module.

        <b> True Random Number Generator (TRNG) Module </b> is used to support
        devices which need to generate random numbers from a physical process

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_TRNG_H

#define _AUI_TRNG_H

/*************************Included Header File List***************************/

#include "aui_common.h"

#ifdef __cplusplus

extern "C" {

#endif

/*******************************Global Type List******************************/

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> True Random Number Generator (TRNG) Module </b> to
        specify the TRNG attribution
        </div> @endhtmlonly

        Struct to specify the <b> TRNG attribution </b>
*/
typedef struct aui_trng_attr {

    /**
    Member as @a Index which integer values (from the value <b> zero (0) </b>)
    refer to different TRNG Devices

    @note At present @a only one (1) device is supported then that index can
          take @a only the value @b 0
    */
    unsigned char uc_dev_idx;

} aui_trng_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> True Random Number Generator (TRNG) Module </b> to
        specify the TRNG output attribution
        </div> @endhtmlonly

        Struct to specify the TRNG output attribution
*/
typedef struct aui_trng_param {

    /**
    Member as pointer to the <b> TRNG output buffer </b>
    */
    unsigned char* puc_rand_output_buffer;

    /**
    Member to specify the <b> TRNG output length </b>
    */
    unsigned long ul_rand_bytes;

} aui_trng_param;

/*****************************Global Function List*****************************/

/**
@brief         Function used to open a TRNG device

@param[in]     p_attr               = Pointer to a struct #aui_trng_attr, which
                                      specifies the TRNG attribution

@param[out]    p_handle             = #aui_hdl pointer to the handle of the
                                      TRNG Device just opened

@return        @b AUI_RTN_SUCCESS   = Opening of the TRNG device performed
                                      successfully
@return        @b Other_Values      = Opening of the TRNG device failed for
                                      some reasons
*/
AUI_RTN_CODE aui_trng_open (

    aui_trng_attr *p_attr,

    aui_hdl *p_handle

    );

/**
@brief          Function used to generate the true random numbers after opening
                the TRNG device by the function #aui_trng_open

@param[in]      handle               = #aui_hdl handle of the TRNG device already
                                       opened and to be managed for generating
                                       the true random numbers

@param[out]     p_param              = Pointer to a struct #aui_trng_param, which
                                       specifies the TRNG output attribution

@return         @b AUI_RTN_SUCCESS   = Generating of the true random numbers
                                       performed successfully
@return         @b Other_Values      = Generating of the TRNG device failed
                                       for some reasons
*/
AUI_RTN_CODE aui_trng_generate (

    aui_hdl handle,

    aui_trng_param *p_param

    );

/**
@brief          Function used to close the TRNG device already opened by the
                function #aui_trng_open then the related handle will be
                released.(i.e. the related resources such as memory, device)

@param[in]      handle               = #aui_hdl handle of the TRNG device
                                       already opened then to be closed

@return         @b AUI_RTN_SUCCESS   = Closing of the TRNG device performed
                                       successfully
@return         @b Other_Values      = Closing of the TRNG device failed for
                                       some reasons
*/
AUI_RTN_CODE aui_trng_close (

    aui_hdl handle

    );

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */

