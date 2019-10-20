/**
 *    @brief         header file of TRNG
 *    @author        Oscar shi
 *    @date          2015-5-8
 *    @version       1.0.0
 *    @note          ALi corp. all rights reserved. 2013~2020 copyright (C)\n
 *    This module will support TRNG(True Random Number Generator) functions.
 */

/****************************INCLUDE HEAD FILE*******************************/
#include "aui_common_priv.h"
#include <aui_trng.h>
#include <hld/trng/trng.h>

/****************************LOCAL MACRO*************************************/

AUI_MODULE(TRNG)

/****************************LOCAL TYPE**************************************/
typedef struct aui_st_trng_module
{
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
}aui_trng_module_s, *aui_p_trng_module_s;

/****************************LOCAL VAR***************************************/

static aui_trng_module_s trng_mod_op;

/****************************LOCAL FUNC DECLEAR******************************/

/****************************MODULE IMPLEMENT********************************/

/****************************Function****************************************/

AUI_RTN_CODE aui_trng_open(aui_trng_attr *attr, aui_hdl *handle)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	(void)attr;
	if (!handle) {
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}

	//rtn_code = aui_dev_reg(AUI_MODULE_TRNG, &trng_mod_op);
	*handle = (aui_hdl)&trng_mod_op;

	return rtn_code;
}

AUI_RTN_CODE aui_trng_generate(aui_hdl handle, aui_trng_param *param)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	unsigned char *buf = NULL;
	int len = 0;
	int ret = 0;

	if ((!handle) || (!param)) {
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}

	if (param->ul_rand_bytes == 1) {
		ret = trng_generate_byte(param->puc_rand_output_buffer);
	} else {
		len = 8 + param->ul_rand_bytes / 8;
		buf = (unsigned char *)malloc(len * 8);
		if (!buf) {
			return 1;
		}
		ret = trng_get_64bits(buf, len);
		memcpy(param->puc_rand_output_buffer, buf, param->ul_rand_bytes);
		free(buf);
	}
	if (ret) {
        AUI_ERR("gernerate failed.");
		rtn_code = AUI_RTN_FAIL;
	}
	return rtn_code;
}

AUI_RTN_CODE aui_trng_close(aui_hdl handle)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    AUI_ERR("test error");
    AUI_WARN("test warning");
    AUI_INFO("test info");
    AUI_DBG("test dbg");

	if (!handle) {
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	//rtn_code = aui_dev_unreg(AUI_MODULE_TRNG, &trng_mod_op);
	return rtn_code;
}

