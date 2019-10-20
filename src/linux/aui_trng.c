/**
 *    @brief         header file of TRNG
 *    @author        Oscar Shi
 *    @date          2015-5-8
 *    @version       1.0.0
 *    @note          ALi corp. all rights reserved. 2013~2020 copyright (C)\n
 *    This module will support TRNG(True Random Number Generator) functions.
 */

/****************************INCLUDE HEAD FILE*******************************/
#include "aui_common_priv.h"
#include <aui_trng.h>
#include <alisltrng.h>

AUI_MODULE(TRNG)

/****************************LOCAL MACRO*************************************/

/****************************LOCAL TYPE**************************************/
typedef struct aui_st_trng_module
{
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	alisltrng_dev_t dev;
} aui_trng_module_s, *aui_p_trng_module_s;

/****************************LOCAL VAR***************************************/

#define TRNG_DEV_MAX 1
static aui_trng_module_s trng_mod_op[TRNG_DEV_MAX];

/****************************LOCAL FUNC DECLEAR******************************/

/****************************MODULE IMPLEMENT********************************/

/****************************Function****************************************/

AUI_RTN_CODE aui_trng_open(aui_trng_attr *attr, aui_hdl *handle)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	alisltrng_param_t param;
	alisl_retcode sl_rtn = 0;

	if (!handle || !attr || attr->uc_dev_idx >= TRNG_DEV_MAX) {
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}

	memset(&param, 0x00, sizeof(alisltrng_param_t));
	param.series = true;
	memset(&trng_mod_op[attr->uc_dev_idx].dev, 0x00, sizeof(alisltrng_dev_t));
	sl_rtn = alisltrng_construct(&trng_mod_op[attr->uc_dev_idx].dev, &param);
	//rtn_code = aui_dev_reg(AUI_MODULE_TRNG, &trng_mod_op);

	if (!sl_rtn) {
		*handle = (aui_hdl)&trng_mod_op[attr->uc_dev_idx];
	} else {
		rtn_code = AUI_RTN_FAIL;
        AUI_ERR("open fail");
	}

	return rtn_code;
}

AUI_RTN_CODE aui_trng_generate(aui_hdl handle, aui_trng_param *param)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	alisl_retcode sl_rtn = 0;
	unsigned char *buf = NULL;
	int len = 0;
	aui_trng_module_s *trng_op;

	if (!handle || !param) {
		return AUI_RTN_EINVAL;
	}
	trng_op = (aui_trng_module_s *)handle;
	if (param->ul_rand_bytes == 1 || !(param->ul_rand_bytes % 8)) {
		sl_rtn = alisltrng_get_rand(&trng_op->dev,
				param->puc_rand_output_buffer, param->ul_rand_bytes);
	} else {
		len = 8 - (param->ul_rand_bytes % 8) + param->ul_rand_bytes;
		buf = malloc(len);
		if (!buf) {
			aui_rtn(AUI_RTN_ENOMEM, "malloc buffer failed");
		}
		sl_rtn = alisltrng_get_rand(&(trng_op->dev), buf, len);
		memcpy(param->puc_rand_output_buffer, buf, param->ul_rand_bytes);
		free(buf);
	}

	if (sl_rtn) {
        AUI_ERR("Generate failed.");
		rtn_code = AUI_RTN_FAIL;
	}
	return rtn_code;
}

AUI_RTN_CODE aui_trng_close(aui_hdl handle)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	aui_trng_module_s *trng_op;
	if (!handle) {
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	trng_op = (aui_trng_module_s *)handle;
	alisltrng_destruct(&(trng_op->dev));
	//rtn_code = aui_dev_unreg(AUI_MODULE_TRNG, &trng_mod_op);
	return rtn_code;
}

