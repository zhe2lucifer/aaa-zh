/**@file
*    @brief 		AUI AV INJECTER module interface implement
*    @author		christian.xie
*    @date			2014-3-12
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/

#include "aui_common_priv.h"
#include <aui_stc.h>
#include <aui_dmx.h>

AUI_MODULE(STC)

typedef struct {
	aui_dev_priv_data dev_priv_data;
} aui_stc_dev;

/**
*   @brief      Initialize stc module
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  p_callback_init: Callback function, which is registered to
*               initialize stc module.
*   @param[in]  pv_param: callback function parameter
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*/
AUI_RTN_CODE aui_stc_init(p_fun_cb p_callback_init, void *pv_param)
{
	AUI_RTN_CODE result = AUI_RTN_SUCCESS;

	if (p_callback_init != NULL) {
		result = p_callback_init(pv_param);
	}

	return result;
}

/**
*   @brief      De-initialize stc module
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  p_callback_de_init: Callback function, which is registered to
*               de-initialize stc module.
*   @param[in]  pv_param: callback function parameter
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*/
AUI_RTN_CODE aui_stc_de_init(p_fun_cb p_callback_de_init, void *pv_param)
{
	AUI_RTN_CODE result = AUI_RTN_SUCCESS;

	if (p_callback_de_init != NULL) {
		result = p_callback_de_init(pv_param);
	}

	return result;
}

/**
*   @brief      Open stc device
*   @author     christian.xie
*   @date       2014-3-11
*   @param[out] stc_handle          Return STC device handle
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*
*/
AUI_RTN_CODE aui_stc_open(aui_hdl *stc_handle)
{
	aui_stc_dev *p_dev = (aui_stc_dev *) MALLOC(sizeof(aui_stc_dev));
	if (NULL == p_dev) {
		 aui_rtn(AUI_RTN_ENOMEM, "out of memory!");
	}

	MEMSET(p_dev, 0, sizeof(aui_stc_dev));
	*stc_handle = p_dev;
	aui_dev_reg(AUI_MODULE_STC, p_dev);

    return AUI_RTN_SUCCESS;
}


/**
*   @brief      Close stc device
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc_handle          STC device handle
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*
*/
AUI_RTN_CODE aui_stc_close(aui_hdl stc_handle)
{
	aui_dev_unreg(AUI_MODULE_STC, stc_handle);
	FREE(stc_handle);
    return AUI_RTN_SUCCESS;
}


/**
*   @brief      Get current value of the STC timer.
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[out] val_out             stc value data
*                                   DVB mode: 45kHz,32bits
*                                   others: ms
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*
*/
AUI_RTN_CODE aui_stc_get(aui_hdl stc, unsigned long *val_out)
{
	AUI_RTN_CODE result;
    aui_hdl handle = NULL;
    (void) stc;

    if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &handle)) {
        if(aui_find_dev_by_idx(AUI_MODULE_DMX, 1, &handle)) {
        	aui_rtn(AUI_RTN_FAIL, "no dmx device is opened!\n");
    	}
	}

    aui_pts pts_tmp;
    result = aui_dmx_get(handle, AUI_DMX_GET_PTS, (void *)&pts_tmp);
    *val_out = ((pts_tmp.pts_32lsb >> 1) & 0x7FFFFFFF);
    if (pts_tmp.pts_1msb) {
    	(*val_out) = (*val_out) | 0x80000000;
    }

    return result;
}

/**
*   @brief      Set current value of the STC timer.
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[in]  val                 stc valule data
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note
*
*/
AUI_RTN_CODE aui_stc_set(aui_hdl stc, unsigned long long val)
{
    // unimplemented
    (void) stc;
    (void) val;

    return AUI_RTN_FAIL;
}

/**
*   @brief      Pause/unpause the timer.
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[in]  pause               pause/unpause
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note   When paused, \a ali_stc_get will alywas return the same value.
*           Pausing STC should also pause the video display / audio playback.
*/
AUI_RTN_CODE aui_stc_pause(aui_hdl stc, int pause)
{
    // unimplemented
    (void) stc;
    (void) pause;

    return AUI_RTN_FAIL;
}

/**
*   @brief      Change speed of the STC timer.
*   @author     christian.xie
*   @date       2014-3-11
*   @param[in]  stc                 STC device handle
*   @param[in]  ppm                 correction to be made, in 1/1000000 units.
*   @return     AUI_RTN_SUCCESS     Return successful
*   @return     AUI_RTN_FAIL        Return error
*   @note       All consecutive corrections are accumulative
*
*/
AUI_RTN_CODE aui_stc_change_speed(aui_hdl stc, int ppm)
{
    // unimplemented
    (void) stc;
    (void) ppm;

    return AUI_RTN_FAIL;
}
