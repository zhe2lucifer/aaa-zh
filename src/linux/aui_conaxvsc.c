/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               aui_conaxvsc.c
 *  @brief              Conax Virtual Smart Card API
 *
 *  @version            1.0
 *  @date               01/09/2016 09:50:28 AM
 *  @revision           none
 *
 *  @authors            Deji Aribuki <deji.aribuki@alitech.com>
 */


#include "aui_common_priv.h"
#include "aui_conaxvsc.h"
 #include "aui_kl.h"
#include "alipltfretcode.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include "alislconaxvsc.h"

AUI_MODULE(CONAXVSC)

#define FD_INVALID -1

struct vsc_handler
{
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	alisl_handle dev;
	aui_conaxvsc_store_callback_attr store_cb;
};

AUI_RTN_CODE aui_conaxvsc_version_get(unsigned long *pul_version)
{
	if (!pul_version)
	{
		AUI_ERR("No input version pointer\n");
		return AUI_RTN_FAIL;
	}

	*pul_version = AUI_MODULE_VERSION_NUM_CONAXVSC;

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_conaxvsc_init(p_fun_cb p_call_back_init, void *pv_param)
{
	if (p_call_back_init) {
		p_call_back_init(pv_param);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_de_init(p_fun_cb p_call_back_init, void *pv_param)
{
	if (p_call_back_init) {
		p_call_back_init(pv_param);
	}

	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_conaxvsc_open(aui_conaxvsc_attr *p_attr,
	aui_hdl *p_handle)
{
	int ret = 0;
	struct vsc_handler *hdl = NULL;

	if (!p_handle || !p_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	hdl = calloc(1, sizeof(struct vsc_handler));
	if(!hdl)
	{
		AUI_ERR("Handler allocate memory fail\n");
		return AUI_RTN_FAIL;
	}

	hdl->data.dev_idx = p_attr->uc_dev_idx;

	ret = alislconaxvsc_open(&hdl->dev);
	if (ret || !hdl->dev) {
		free(hdl);
		AUI_ERR("Conax VSC open fail\n");
		return AUI_RTN_FAIL;
	}

	if (p_attr->p_vsc_store)
		ret = alislconaxvsc_lib_init(hdl->dev, p_attr->p_vsc_store->p_uc_data,
			p_attr->p_vsc_store->p_uc_key, p_attr->p_vsc_store->p_uc_hash);
	else
		ret = alislconaxvsc_lib_init(hdl->dev, NULL, NULL, NULL);

	if (ret) {
		free(hdl);
		AUI_ERR("Conax VSC Lib initial fail\n");
		return AUI_RTN_FAIL;
	}

	ret = aui_dev_reg(AUI_MODULE_CONAXVSC, hdl);
	if (ret) {
		free(hdl);
		AUI_ERR("Conax VSC handler register fail\n");
		return AUI_RTN_FAIL;
	}

	*p_handle = (aui_hdl) hdl;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_close(aui_hdl handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	ret = alislconaxvsc_close(hdl->dev);
	if (ret) {
		ret = AUI_RTN_FAIL;
		goto exit;
	}

	ret = aui_dev_unreg(AUI_MODULE_CONAXVSC, hdl);
	if (ret) {
		ret = AUI_RTN_FAIL;
		goto exit;
	}

exit:
	hdl->dev = NULL;
	free(hdl);
	return ret;
}

static void aui_conaxvsc_store_cb_fun(
	struct alislconaxvsc_store *pv_store, void *user_data)
{
	aui_conaxvsc_store *store;
	struct vsc_handler *hdl = (struct vsc_handler *) user_data;

	if (!hdl || !pv_store) {
		AUI_DBG("null input value(s)\n");
		return;
	}

	if (!hdl->store_cb.p_fn_conaxvsc_store_cb) {
		AUI_DBG("conaxvsc_store_cb not set\n");
		return;
	}

	if (!pv_store->data || !pv_store->key || !pv_store->hash) {
		AUI_DBG("store parameter(s) not set\n");
		return;
	}

	if (pv_store->data_len != AUI_CONAXVSC_STORE_DATA_LEN) {
		AUI_DBG("wrong store data_len=%d\n",pv_store->data_len);
		return;
	}

	store = calloc(sizeof(aui_conaxvsc_store), 1);
	if (!store) {
		AUI_DBG("calloc failed\n");
		return;		
	}

	memcpy(store->p_uc_data, pv_store->data, AUI_CONAXVSC_STORE_DATA_LEN);
	memcpy(store->p_uc_key, pv_store->key, AUI_CONAXVSC_STORE_KEY_LEN);
	memcpy(store->p_uc_hash, pv_store->hash, AUI_CONAXVSC_STORE_HASH_LEN);

	/* call user callback function */
	hdl->store_cb.p_fn_conaxvsc_store_cb(store, hdl->store_cb.pv_user_data);

	free(store);

	return;	
}

AUI_RTN_CODE aui_conaxvsc_register_store_callback(aui_hdl handle,
	aui_conaxvsc_store_callback_attr *pv_attr)
{
	int ret = 0;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	if (!pv_attr || !pv_attr->p_fn_conaxvsc_store_cb)
	{
		AUI_ERR("No callback function or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	hdl->store_cb.pv_user_data = pv_attr->pv_user_data;
	hdl->store_cb.p_fn_conaxvsc_store_cb = pv_attr->p_fn_conaxvsc_store_cb;

	ret = alislconaxvsc_register_callback(
		hdl->dev, hdl, aui_conaxvsc_store_cb_fun);
	if (ret)
	{
		AUI_ERR("alislconaxvsc_register_callback failed\n");
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_unregister_store_callback(aui_hdl handle)
{
	int ret = 0;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	ret = alislconaxvsc_unregister_callback(hdl->dev);
	if (ret)
	{
		AUI_ERR("alislconaxvsc_unregister_callback failed\n");
		return AUI_RTN_FAIL;
	}

	hdl->store_cb.pv_user_data = NULL;
	hdl->store_cb.p_fn_conaxvsc_store_cb = NULL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_cmd_transfer(aui_hdl handle, int n_session_id,
	aui_conaxvsc_tx_buf *pv_command, aui_conaxvsc_tx_buf *pv_response,
	int *sw_1, int *sw_2)
{
	int ret = 0;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	if (!pv_command || !pv_response || !sw_1 || !sw_2)
	{
		AUI_ERR("Some null pointer:%d,%d,%d,%d\n",(pv_command==0),(pv_response==0),(sw_1==0),(sw_2==0));
		return AUI_RTN_FAIL;
	}

	ret = alislconaxvsc_cmd_dispatch(hdl->dev, n_session_id,
		pv_command->p_uc_data, pv_command->n_size, pv_response->p_uc_data,
		&pv_response->n_size, sw_1, sw_2);
	
	if (ret)
	{
		AUI_ERR("alislconaxvsc_cmd_dispatch fail\n");
		return AUI_RTN_FAIL;
	}
	
	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_conaxvsc_decrypt_cw(aui_hdl handle,
	aui_conaxvsc_decw_attr *pv_attr)
{
	int ret = 0;
	int kl_fd = FD_INVALID;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev || !pv_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	ret = aui_kl_get(pv_attr->kl_handle, AUI_KL_GET_KEY_POS, 
		(unsigned long *) &kl_fd);
	if (ret)
	{
		AUI_ERR("ui_kl_get KEY_POS failed\n");
		return AUI_RTN_FAIL;
	}

	ret = alislconaxvsc_set_decw_key(hdl->dev, kl_fd, pv_attr->key_id,
		pv_attr->decw, pv_attr->key_parity);
	
	if (ret)
	{
		AUI_ERR("alislconaxvsc_set_decw_key fail\n");
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_conaxvsc_set_en_key(aui_hdl handle,
	aui_conaxvsc_en_key_attr *pv_attr)
{
	int ret = 0;
	int kl_fd = FD_INVALID;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev || !pv_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	ret = aui_kl_get(pv_attr->kl_handle, AUI_KL_GET_KEY_POS, 
		(unsigned long *) &kl_fd);
	
	if (ret)
	{
		AUI_ERR("ui_kl_get KEY_POS failed\n");
		return AUI_RTN_FAIL;
	}

	ret = alislconaxvsc_set_rec_en_key(
		hdl->dev, kl_fd,
		pv_attr->en_key,
		(pv_attr->key_parity == AUI_CONAXVSC_KEY_ODD) ?
			SL_CONAXVSC_KL_PARITY_ODD : SL_CONAXVSC_KL_PARITY_EVEN);
	
	if (ret)
	{
		AUI_ERR("aui_conaxvsc_set_en_key fail\n");
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_conaxvsc_set_uk(aui_hdl handle,
	aui_conaxvsc_uk_attr *pv_attr)
{
	int ret = 0;
	int kl_fd = FD_INVALID;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !hdl->dev || !pv_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	ret = aui_kl_get(pv_attr->kl_handle, AUI_KL_GET_KEY_POS, 
		(unsigned long *) &kl_fd);
	
	if (ret)
	{
		AUI_ERR("ui_kl_get KEY_POS failed\n");
		return AUI_RTN_FAIL;
	}

	ret = alislconaxvsc_set_uk_en_key(hdl->dev, kl_fd, pv_attr->uk);
	
	if (ret)
	{
		AUI_ERR("aui_conaxvsc_set_en_key fail\n");
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;	
}
