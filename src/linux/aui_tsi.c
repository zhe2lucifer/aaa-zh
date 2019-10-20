/**@file
*    @brief     ALi AUI TSI function implementation
*    @author    romain.baeriswyl
*    @date      2014-02-04
*    @version   1.0.0
*    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
*/

#include "aui_common_priv.h"
#include <alisltsi.h>
#include <aui_tsi.h>
#include <string.h>

AUI_MODULE(TSI)

/* TSI module version number. */
#define AUI_MODULE_VERSION_NUM_TSI	(0X00020200)

struct tsi_handler {
	struct aui_st_dev_priv_data data;
	int input_configured[AUI_TSI_INPUT_MAX]; /* true when input configured */
	alisl_handle dev;
};


AUI_RTN_CODE aui_tsi_open(aui_hdl *handle)
{
	struct tsi_handler *hdl;

	AUI_API("enter");

	if (!handle)
		aui_rtn(AUI_RTN_EINVAL,NULL);

	hdl = calloc(sizeof(struct tsi_handler), 1);
	if (!hdl)
		aui_rtn(AUI_RTN_EINVAL,"malloc error");

	hdl->data.dev_idx = 0;
	if (aui_dev_reg(AUI_MODULE_TSI , hdl)) {
		free(hdl);
		aui_rtn(AUI_RTN_EINVAL,"aui_dev_reg error");
	}

	/* single TSI_ID_M3602_0 ? */
	if (alisltsi_open(&hdl->dev, TSI_ID_M3602_0, NULL)) {
		hdl->dev = NULL;
		aui_tsi_close(hdl);
		aui_rtn(AUI_RTN_EINVAL,
			"alisltsi_open failed");
	}
	*handle = (aui_hdl *)hdl;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsi_src_init(aui_hdl handle, enum aui_tsi_input_id input_id,
			      struct aui_attr_tsi *attr)
{
	struct tsi_handler *hdl = (struct tsi_handler *)handle;

	if(!hdl || !attr || input_id >= AUI_TSI_INPUT_MAX)
		aui_rtn(AUI_RTN_EINVAL,NULL);

	AUI_API("input_id %d attr.ul_init_param 0x%x",
		    input_id, attr->ul_init_param);

	if (alisltsi_setinput(hdl->dev, input_id, attr->ul_init_param))
		aui_rtn(AUI_RTN_EINVAL,"tsi_src_init setinput error");

	hdl->input_configured[input_id] = true;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsi_route_cfg(aui_hdl handle, enum aui_tsi_input_id input_id,
			       enum aui_tsi_channel_id tsi_channel_id, enum aui_tsi_output_id dmx_id)
{
	struct tsi_handler *hdl = (struct tsi_handler *)handle;

	if (!hdl || input_id >= AUI_TSI_INPUT_MAX ||
	    tsi_channel_id > AUI_TSI_CHANNEL_3 || dmx_id > AUI_TSI_OUTPUT_DMX_3)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	if (!hdl->input_configured[input_id])
		aui_rtn(AUI_RTN_EINVAL,
			"aui_tsi_route_cfg input not configured");

	AUI_API("input_id %d tsi_channel_id %d dmx_id %d",
		input_id, tsi_channel_id, dmx_id);

	/* select TSI input */
	AUI_DBG("[aui_tsi] set channel %d on input %d\n", tsi_channel_id, input_id);
	if (alisltsi_setchannel(hdl->dev, input_id, tsi_channel_id))
		aui_rtn(AUI_RTN_EINVAL,
			"aui_tsi_route_cfg setchannel error");

	/* select TSI output */
	AUI_DBG("[aui_tsi] set channel %d on output %d\n", tsi_channel_id, dmx_id);
	if (alisltsi_setoutput(hdl->dev, dmx_id, tsi_channel_id))
		aui_rtn(AUI_RTN_EINVAL,
			"aui_tsi_route_cfg setoutput error");

	return AUI_RTN_SUCCESS;
}

extern AUI_RTN_CODE aui_tsi_close(aui_hdl handle)
{
	struct tsi_handler *hdl = (struct tsi_handler *)handle;
	int err = AUI_RTN_SUCCESS;

	if (!hdl)
		return AUI_RTN_SUCCESS;

	if (hdl->dev)
		if (alisltsi_close(hdl->dev)) {
			err = AUI_RTN_FAIL;
			AUI_DBG("[aui tsi] sl close error\n");
		}

	if (aui_dev_unreg(AUI_MODULE_TSI , hdl)) {
		err = AUI_RTN_FAIL;
		AUI_DBG("[aui tsi] unreg error\n");
	}
	hdl->dev = 0;
	free(hdl);
	return err;
}

#if 1
/*
 * DEPRECATED API. It is not public in the header file.
 * Keep it in the code for compatibility, just in case some customer have used it.
*/
AUI_RTN_CODE aui_tsi_CIConnect_mode_set(aui_hdl handle, unsigned long ul_mode)
{
	struct tsi_handler *hdl = (struct tsi_handler *)handle;
	
	if (!hdl)
			aui_rtn(AUI_RTN_EINVAL, NULL);
	
	if (alisltsi_set_parallel_mode(hdl->dev, ul_mode))
		aui_rtn(AUI_RTN_EINVAL,
			"Set TS stream chain or parallel CI device error!");
	return AUI_RTN_SUCCESS;
}
#endif

AUI_RTN_CODE aui_tsi_ci_card_bypass_set(aui_hdl handle, unsigned long card_id, unsigned char bypass)
{
	struct tsi_handler *hdl = (struct tsi_handler *)handle;
	
	if (!hdl)
			aui_rtn(AUI_RTN_EINVAL, NULL);
    // TASK #44551, do not support ci chain mode in driver.
    // Just hard code to parallel mode(1) in AUI.
	if (alisltsi_set_parallel_mode(hdl->dev, AUI_TSI_CI_MODE_PARALLEL))
		aui_rtn(AUI_RTN_EINVAL,
			"Set TS stream chain or parallel CI device error!");
	if (alisltsi_set_cibypass(hdl->dev, card_id, bypass))
		aui_rtn(AUI_RTN_EINVAL,
			"Set TS stream pass or bypass CI device error!");
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsi_version_get(unsigned long *version)
{
	if(!version)
		aui_rtn(AUI_RTN_EINVAL,NULL);

	*version = AUI_MODULE_VERSION_NUM_TSI;
	return AUI_RTN_SUCCESS;
}
