/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file                        aui_dog.c
 *  @brief                      Ali aui dog API
 *
 *  @version                  1.0
 *  @date                       02/10/2014 10:33:44 PM
 *  @revision                  none
 *
 *  @author                     Alan Zhang <Alan.Zhang@alitech.com>
 */

#include "aui_common_priv.h"
#include <aui_dog.h>
#include "alislwatchdog.h"

AUI_MODULE(DOG)

// #define E_OK   0
/*  static OSAL_ID s_mod_mutex_id_dog=0; */

/** watch_dog device handler */
typedef struct aui_st_handle_dog
{
    aui_dev_priv_data dev_priv_data;
    /** attribute of watchdog device  */
    aui_attr_dog attr_dog;
} aui_handle_dog, *aui_p_handle_dog;


AUI_RTN_CODE aui_dog_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL,"NULL==pul_version");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_DOG;
	
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dog_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if (p_call_back_init != NULL) {
		p_call_back_init(pv_param);
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dog_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if (p_call_back_init != NULL) {
		p_call_back_init(pv_param);
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dog_open(aui_attr_dog *p_attr_dog,aui_hdl *pp_hdl_dog)
{
	aui_handle_dog *handle = NULL;
    handle = calloc(sizeof(aui_handle_dog), 1);
    if (NULL == handle) {
        aui_rtn(AUI_RTN_ENOMEM,"NULL == handle");
    }
    handle->dev_priv_data.dev_idx = p_attr_dog->uc_dev_idx;
    handle->attr_dog = *p_attr_dog;
    *pp_hdl_dog = handle;
	aui_dev_reg(AUI_MODULE_DOG, handle);
	
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dog_close(aui_hdl p_hdl_dog)
{
	if(NULL == p_hdl_dog) {
		aui_rtn(AUI_RTN_EINVAL,"NULL == p_hdl_dog");
	}

	aui_dev_unreg(AUI_MODULE_DOG, p_hdl_dog);
    free(p_hdl_dog);
	
	return AUI_RTN_SUCCESS;
}


/* enable watchdog */
AUI_RTN_CODE aui_dog_start(void *p_hdl_dog,aui_attr_dog *p_attr_dog)
{
	if((NULL==p_hdl_dog))
	{
		aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_dog");
	}
	alislwatchdog_start();

	(void)p_attr_dog;
	return AUI_RTN_SUCCESS;
}

/* disable watchdog */
AUI_RTN_CODE aui_dog_stop(aui_hdl p_hdl_dog,aui_attr_dog *p_attr_dog)
{
	if((NULL==p_hdl_dog))
	{
		aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_dog");
	}
	
    alislwatchdog_stop();

	(void)p_attr_dog;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_set(aui_hdl p_hdl_dog,unsigned long ul_item,void *pv_param)
{
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

	if(NULL==(aui_handle_dog *)p_hdl_dog)
	{
		aui_rtn(AUI_RTN_EINVAL,"NULL==(aui_handle_dog *)p_hdl_dog");
	}

	switch(ul_item)
	{
		default:
		{
			aui_rtn(AUI_RTN_FAIL,NULL);
			break;
		}
	}
	if(AUI_RTN_SUCCESS!=rtn_code)
	{
		return rtn_code;
	}

	(void)pv_param;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_get(void *p_hdl_dog,unsigned long ul_item,void *pv_param)
{
	AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
		
	if(NULL==(aui_handle_dog *)p_hdl_dog)
	{
		aui_rtn(AUI_RTN_EINVAL,"NULL==(aui_handle_dog *)p_hdl_dog");
	}

	switch(ul_item)
	{
		default:
		{
			aui_rtn(AUI_RTN_FAIL,NULL);
			break;
		}
	}
	if(AUI_RTN_SUCCESS!=rtn_code)
	{
		return rtn_code;
	}

	(void)pv_param;
	return AUI_RTN_SUCCESS;
}

/* feed dog */
AUI_RTN_CODE aui_dog_time_set(aui_hdl p_hdl_dog,unsigned long ul_time_us)
{
	//alislwatchdog_feed_dog(ul_time_us);
	if(0 != ul_time_us)
		alislwatchdog_set_duration_time(ul_time_us);
	alislwatchdog_start();

	(void)p_hdl_dog;
	return AUI_RTN_SUCCESS;
}

/* get watchdog left time */
AUI_RTN_CODE aui_dog_time_get(aui_hdl p_hdl_dog,unsigned long *pul_time_us)
{
	if(NULL == pul_time_us)
	{
		AUI_ERR("pul_time_us is NULL.\n");
		aui_rtn(AUI_RTN_EINVAL,"NULL == pul_time_us");
	}
    *pul_time_us = alislwatchdog_get_time_left();

	(void)p_hdl_dog;
	return AUI_RTN_SUCCESS;
}

/* set watchdog duration time */
AUI_RTN_CODE aui_dog_config(aui_hdl p_hdl_dog, aui_attr_dog *p_attr_dog)
{
	AUI_ENTER_FUNC(AUI_MODULE_DOG);
	if(NULL==p_attr_dog)
	{
		aui_rtn(AUI_RTN_EINVAL,"NULL==p_attr_dog");
	}

//	AUI_DBG("done");
    alislwatchdog_set_duration_time(p_attr_dog->ul_time_us);
	AUI_LEAVE_FUNC(AUI_MODULE_DOG);

	(void)p_hdl_dog;
	return AUI_RTN_SUCCESS;
}
