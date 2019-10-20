/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               aui_conaxvsc.c
 *  @brief              Conax Virtual Smart Card API
 *
 *  @version            1.0
 *  @date               25/11/2016 02:40:25 PM
 *  @revision           none
 *
 *  @authors            Ezio.Yang <ezio.yang@alitech.com>
 */


#include "aui_common_priv.h"
#include "aui_conaxvsc.h"
#include "aui_kl.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <vsc/vsc.h>
#include <api/libchunk/chunk.h>

AUI_MODULE(CONAXVSC)

typedef struct vsc_dev {
	struct vsc_dev  *next;  /*next device */
	int type;
	char name[HLD_MAX_NAME_SIZE];
	void *store_user_data;
}conaxvsc_dev;

struct vsc_handler
{
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	conaxvsc_dev* dev;
};

aui_conaxvsc_store *aui_vsc_data=0;
aui_conaxvsc_store_fun_cb aui_conaxvsc_store_cb = 0;
void *user_data = 0;
aui_conaxvsc_store vsc_store_data;

static RET_CODE nvram_data_fetch(unsigned char* mem_addr_main, VSC_STORE_CONFIG *config, VSC_STORE_CONFIG *backup_config)
{
	if(aui_vsc_data)
	{
		memcpy(&(config->random_key[0][0]),aui_vsc_data->p_uc_key, AUI_CONAXVSC_STORE_KEY_LEN);
		memcpy(&(config->hash[0][0]),aui_vsc_data->p_uc_hash,AUI_CONAXVSC_STORE_HASH_LEN);
		memcpy(&(backup_config->random_key[0][0]),aui_vsc_data->p_uc_key, AUI_CONAXVSC_STORE_KEY_LEN);
		memcpy(&(backup_config->hash[0][0]),aui_vsc_data->p_uc_hash, AUI_CONAXVSC_STORE_HASH_LEN);
		memcpy(mem_addr_main,aui_vsc_data->p_uc_data, AUI_CONAXVSC_STORE_DATA_LEN);

		config->index = 0;
		config->tag = 0x56534300; //VSC_TAG
		backup_config->index = 0;
		backup_config->tag = 0x56534300; //VSC_TAG
		AUI_DBG("Read callback aui_vsc_data\n");
	}
	return 0;
}

static RET_CODE nvram_write_null(unsigned char* mem_addr_main, VSC_STORE_CONFIG config)
{
	(void) mem_addr_main;
	(void) config;

	AUI_DBG("Null callback\n");
	return 0;
}

static RET_CODE nvram_write_cb(unsigned char* mem_addr_main, VSC_STORE_CONFIG config)
{
	AUI_DBG("Write callback\n");
	memset(&vsc_store_data, 0, sizeof(aui_conaxvsc_store));
	memcpy(vsc_store_data.p_uc_key,&(config.random_key[0][0]), AUI_CONAXVSC_STORE_KEY_LEN);
	memcpy(vsc_store_data.p_uc_hash, &(config.hash[0][0]), AUI_CONAXVSC_STORE_HASH_LEN);
	memcpy(vsc_store_data.p_uc_data, mem_addr_main, AUI_CONAXVSC_STORE_DATA_LEN);
	
	aui_conaxvsc_store_cb(&vsc_store_data,user_data);
	return 0;
}

AUI_RTN_CODE aui_conaxvsc_open(aui_conaxvsc_attr *p_attr,
	aui_hdl *p_handle)
{
	struct vsc_handler *hdl = NULL;
	unsigned long vsc_lib_addr = __MM_VSC_CODE_START_ADDR + 0x200;
	unsigned long vsc_lib_len = 0;
	unsigned long block_addr = 0;
	unsigned long chunk_id = 0x0CF30101;

	if(!p_handle || !p_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	hdl = (struct vsc_handler*)MALLOC(sizeof(struct vsc_handler));
	if(!hdl)
	{
		AUI_ERR("Handler allocate memory fail\n");
		return AUI_RTN_FAIL;
	}

	hdl->data.dev_idx = p_attr->uc_dev_idx;

	if(p_attr->p_vsc_store)
	{
		aui_vsc_data = (aui_conaxvsc_store*)malloc(sizeof(aui_conaxvsc_store));
		memcpy((unsigned char*)aui_vsc_data, (unsigned char*)p_attr->p_vsc_store, sizeof(aui_conaxvsc_store));
		AUI_DBG("Allocate aui_vsc_data\n");
	}
	vsc_vdev_init((RET_CODE*)nvram_data_fetch,(RET_CODE*)nvram_write_null);

	if(p_attr->p_vsc_store)
		vsc_nvram_read_flash ();

	sto_get_chunk_len (chunk_id, &block_addr, &vsc_lib_len);
	if(vsc_lib_init(&vsc_lib_addr,&vsc_lib_len))
	{
		AUI_ERR("Conax VSC Lib initial fail\n");
		aui_conaxvsc_close(hdl);
		FREE(hdl);
		return AUI_RTN_FAIL;
	}

	if(aui_dev_reg(AUI_MODULE_CONAXVSC, hdl))
	{
		AUI_ERR("Conax VSC handler register fail\n");
		aui_conaxvsc_close(hdl);
		FREE(hdl);
		return AUI_RTN_FAIL;
	}

	*p_handle = (aui_hdl) hdl;
	AUI_DBG("vsc_lib_addr:%lx,vsc_lib_len:%lx,block_addr:%lx,chunk_id:%lx\n",vsc_lib_addr,vsc_lib_len,block_addr,chunk_id);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_close(aui_hdl handle)
{
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if(!hdl)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	if(aui_vsc_data)
	{
		FREE(aui_vsc_data);
		aui_vsc_data = 0;
		AUI_DBG("Free aui_vsc_data\n");
	}

	vsc_vdev_init((RET_CODE*)nvram_data_fetch,(RET_CODE*)nvram_write_null);

	if(aui_dev_unreg(AUI_MODULE_CONAXVSC, hdl))
	{
		AUI_ERR("Conax VSC handler unregister fail\n");
		hdl->dev = NULL;
		FREE(hdl);
		return AUI_RTN_FAIL;
	}

	hdl->dev = NULL;
	FREE(hdl);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_register_store_callback(aui_hdl handle,
	aui_conaxvsc_store_callback_attr *pv_attr)
{	
	(void) handle;

	if (!pv_attr || !pv_attr->p_fn_conaxvsc_store_cb)
	{
		AUI_ERR("No callback function or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	if(pv_attr->pv_user_data)
	{
		user_data = pv_attr->pv_user_data;
		AUI_WARN("Current not support pv_user_data\n");
	}

	aui_conaxvsc_store_cb = pv_attr->p_fn_conaxvsc_store_cb;
	vsc_vdev_init((RET_CODE*)nvram_data_fetch,(RET_CODE*)nvram_write_cb);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_unregister_store_callback(aui_hdl handle)
{
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if(!hdl)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	vsc_vdev_init((RET_CODE*)nvram_data_fetch,(RET_CODE*)nvram_write_null);
	
	if(user_data)
	{
		FREE(user_data);
		AUI_WARN("Free unsupported pv_user_data memory\n");
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_conaxvsc_cmd_transfer(aui_hdl handle, int n_session_id,
	aui_conaxvsc_tx_buf *pv_command, aui_conaxvsc_tx_buf *pv_response,
	int *sw_1, int *sw_2)
{
	int ret = 0;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;
	unsigned short status_word = 0;
	unsigned short cmd_size = (unsigned short)pv_command->n_size;
	unsigned short res_size = 0;

	if(!hdl)
	{
		AUI_ERR("No handler\n");
		return AUI_RTN_FAIL;
	}

	if (!pv_command || !pv_response || !sw_1 || !sw_2)
	{
		AUI_ERR("Some null pointer:%d,%d,%d,%d\n",(pv_command==0),(pv_response==0),(sw_1==0),(sw_2==0));
		return AUI_RTN_FAIL;
	}
	
	ret = vsc_lib_dispatch_cmd(n_session_id,pv_command->p_uc_data,
	cmd_size,&pv_response->p_uc_data[0],&res_size,&status_word);

	pv_response->n_size = (int)res_size;
	*sw_2 = (int) (status_word>>8);
	*sw_1 = (int) (status_word&0x00ff);

	if(ret)
	{
		AUI_ERR("Conax VSC command dispatch fail\n");
		return AUI_RTN_FAIL;
	}
	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_conaxvsc_decrypt_cw(aui_hdl handle,
	aui_conaxvsc_decw_attr *pv_attr)
{
	unsigned long kl_fd = 0xFF;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;
	CE_DATA_INFO ce_data_info ;
	MEMSET(&ce_data_info, 0, sizeof(CE_DATA_INFO));

	if (!hdl || !pv_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	if(aui_kl_get(pv_attr->kl_handle, AUI_KL_GET_KEY_POS, &kl_fd))
	{
		AUI_ERR("Get key position fail\n");
		return AUI_RTN_FAIL;
	}

	ce_data_info.des_aes_info.des_low_or_high = (pv_attr->key_parity==AUI_CONAXVSC_KEY_ODD)? 1: 0;
	ce_data_info.key_info.cw_pos  = kl_fd;

	if(vsc_lib_get_key_ext(&pv_attr->key_id))
	{
		AUI_ERR("Conax VSC Lib get key fail\n");
		return AUI_RTN_FAIL;
	}

	if(vsc_lib_decrypt_decw_ext(pv_attr->decw,&ce_data_info))
	{
		AUI_ERR("Conax VSC Lib Decrypt DECW fail\n");
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_conaxvsc_set_en_key(aui_hdl handle,
	aui_conaxvsc_en_key_attr *pv_attr)
{
	unsigned long kl_fd = -1;
	struct vsc_handler *hdl = (struct vsc_handler *) handle;

	if (!hdl || !pv_attr)
	{
		AUI_ERR("No handler or attribute pointer\n");
		return AUI_RTN_FAIL;
	}

	if(aui_kl_get(pv_attr->kl_handle, AUI_KL_GET_KEY_POS, &kl_fd))
	{
		AUI_ERR("Get key position fail\n");
		return AUI_RTN_FAIL;
	}

	if(ce_generate_vsc_rec_key(&kl_fd, pv_attr->en_key))
	{
		AUI_ERR("Reencrypt key fail\n");
		return AUI_RTN_FAIL;
	}
	return AUI_RTN_SUCCESS;	
}

