/**  @file
 *	 @brief 	Implement of AUI panel moudle
 *	 @author	 smith.shi
 *	 @date		   2013-6-18
 *	 @version	  1.0.0
 *	 @note		   ali corp. all rights reserved. 2013-2020 copyright (C)
 *     @describe       To simple encapsulation of a panel module, realize the basic display, clearing operations.	
 */

/****************************INCLUDE HEAD FILE******************************/
#include "aui_common_priv.h"
#include <aui_panel.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <aui_common.h>

AUI_MODULE(PANEL)

/************		  LOCAL MACRO		 ******************/
#define LOG_INFO_0	1
#define SEG_NUM 5//default including ':'

#define PANEL_FUNCTION_ENTER() AUI_INFO("enter \n")
#define PANEL_FUNCTION_LEAVE() AUI_INFO("leave \n")

#define PANEL_LOCK() \
		do { \
			if (INVALID_ID!=s_panel_mutex_id) \
			osal_mutex_lock(s_panel_mutex_id, OSAL_WAIT_FOREVER_TIME); \
		} while(0)

#define PANEL_UNLOCK() \
		do { \
			if(INVALID_ID!=s_panel_mutex_id) \
			osal_mutex_unlock(s_panel_mutex_id); \
		} while(0)

/****************************LOCAL TYPE*************************************/
/**  attribute of panel device */
typedef struct aui_st_panel_dev
{
	/** panel device ID */
	unsigned long		ul_dev_id;
	/** Pointer to the device object */
	struct pan_device*	p_dev_obj;
	/** configuration*/
	aui_cfg_panel		 cfg_panel;
	/** Enable */
	unsigned char		uc_enable;
	/** On/Off */
	unsigned long		ul_status;
} aui_panel_dev, *aui_p_panel_dev;

//typedef unsigned long aui_handle_panel;
typedef aui_panel_dev aui_handle_panel;

/****************************LOCAL VAR**************************************/
typedef struct aui_st_panel_info
{
	aui_handle_panel* ap_panel_hnd[MAX_PANEL_ID];
} aui_panel_info;
static aui_panel_info s_panel_info;

static unsigned char s_panel_mod_inited = FALSE;
static OSAL_ID s_panel_mutex_id = INVALID_ID;

/****************************LOCAL FUNC DECLEAR*****************************/
/*******************************LOCAL FUNC *********************************/

static int find_panel_handle(aui_handle_panel* hnd)
{
	int i = 0;
	for (i=0 ; i<MAX_PANEL_ID; i++)
	{
		if (hnd == s_panel_info.ap_panel_hnd[i])
		{
			return i;
		}
	}
	return -1;
}

static int find_empty_slot()
{
	int i = 0;
	for (i=0 ; i<MAX_PANEL_ID; i++)
	{
		if (0 == s_panel_info.ap_panel_hnd[i])
		{
			return i;
		}
	}
	return -1;
}

static int add_panel_handle(aui_handle_panel* hnd)
{
	int slot = find_empty_slot();
	if (slot >=0 )
	{
		s_panel_info.ap_panel_hnd[slot] = hnd;
		return slot;
	}
	return -1;
}

static int del_panel_handle(aui_handle_panel* hnd)
{
	int slot = find_panel_handle(hnd);
	if (slot >=0 )
	{
		s_panel_info.ap_panel_hnd[slot] = 0;
		return slot;
	}
	return -1;
}

static int is_valid_panel_handle(aui_handle_panel* hnd)
{
	int slot = find_panel_handle(hnd);
	if (slot >=0 )
	{
		return TRUE;
	}
	return FALSE;
} 

static int is_panel_dev_opened(unsigned long ul_panel_id)
{
	int i = 0;
	for (i=0 ; i<MAX_PANEL_ID; i++)
	{
		if (0 != s_panel_info.ap_panel_hnd[i])
		{
			if (ul_panel_id == s_panel_info.ap_panel_hnd[i]->ul_dev_id)
				return TRUE;
		}
	}
	return FALSE;
}

/****************************MODULE IMPLEMENT*******************************/

AUI_RTN_CODE aui_panel_version_get(unsigned long *pul_version)
{
	if (NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	*pul_version = AUI_MODULE_VERSION_NUM_PANEL;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_panel_init(p_fun_cb p_call_back_init, void *pv_param)
{
	PANEL_FUNCTION_ENTER() ;

	if (FALSE == s_panel_mod_inited)
	{
		if (NULL == p_call_back_init)
		{
			aui_rtn(AUI_RTN_EINVAL, NULL);
		}
		else
		{
			p_call_back_init(pv_param);
		}

		s_panel_mutex_id = osal_mutex_create();
		if (INVALID_ID == s_panel_mutex_id)
		{
			aui_rtn(AUI_RTN_NO_RESOURCE, NULL);
		}
		s_panel_mod_inited = TRUE;
	}
	else
	{
		AUI_WARN("already inited\n");
	}

	PANEL_FUNCTION_LEAVE() ;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_panel_de_init(p_fun_cb p_call_back_de_init, void *pv_param)
{
	PANEL_FUNCTION_ENTER() ;

	if (TRUE == s_panel_mod_inited)
	{
		if (NULL == p_call_back_de_init)
		{
			aui_rtn(AUI_RTN_EINVAL, NULL);
		}
		else
		{
			p_call_back_de_init(pv_param);
		}

		if (INVALID_ID != s_panel_mutex_id)
		{
			osal_mutex_delete(s_panel_mutex_id);
			s_panel_mutex_id = INVALID_ID;
		}
		s_panel_mod_inited = FALSE;
	}
	else
	{
		AUI_WARN("have not inited\n");
	}

	PANEL_FUNCTION_LEAVE() ;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_panel_open(unsigned long ul_panel_id,
		aui_cfg_panel *p_panel_param, aui_hdl *pp_hdl_panel)
{
	PANEL_LOCK();

	if ((NULL==pp_hdl_panel) ||(NULL!=*(aui_handle_panel **)pp_hdl_panel)
			||(ul_panel_id>=MAX_PANEL_ID))
	{
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	if (TRUE == is_panel_dev_opened(ul_panel_id))
	{
		*(aui_handle_panel **)pp_hdl_panel =
				s_panel_info.ap_panel_hnd[ul_panel_id];
		PANEL_UNLOCK();
		return AUI_RTN_SUCCESS;
	}
	if (find_empty_slot() < 0)
	{
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_NO_RESOURCE, NULL);
	}

	aui_handle_panel *p_hdl_panel =
			(aui_handle_panel *)MALLOC(sizeof(aui_handle_panel));
	if (NULL == p_hdl_panel)
	{
	    AUI_ERR("malloc fail.\n");
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_ENOMEM, NULL);
	}
	MEMSET(p_hdl_panel, 0, sizeof(aui_handle_panel));

	p_hdl_panel->ul_dev_id = ul_panel_id;
	p_hdl_panel->p_dev_obj =
			(struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	p_hdl_panel->ul_status = AUI_PANEL_OPEN;
	if (NULL == p_panel_param)
	{
		p_hdl_panel->cfg_panel.ul_led_cnt = SEG_NUM;//default including ':'
	}
	else
	{
		p_hdl_panel->cfg_panel = *p_panel_param;
	}
	if (NULL == p_hdl_panel->p_dev_obj)
	{   
	    AUI_ERR("get HLD_DEV_TYPE_PAN fail.\n");
		FREE(p_hdl_panel);
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_NO_RESOURCE, NULL);
	}
	pan_open(p_hdl_panel->p_dev_obj);

	add_panel_handle(p_hdl_panel);
	*(aui_handle_panel **)pp_hdl_panel = p_hdl_panel ;

	PANEL_UNLOCK();
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_panel_close(aui_hdl pv_hdl_panel)
{
	PANEL_LOCK();

	aui_handle_panel *p_hdl_panel = (aui_handle_panel *)pv_hdl_panel;
	if ((NULL == p_hdl_panel) || (FALSE == is_valid_panel_handle(p_hdl_panel)))
	{
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (AUI_PANEL_OPEN == p_hdl_panel->ul_status)
	{
		p_hdl_panel->ul_status = AUI_PANEL_IDLE;
	}

	pan_close(p_hdl_panel->p_dev_obj);

	del_panel_handle(p_hdl_panel);
	FREE(p_hdl_panel);
	p_hdl_panel = NULL;

	PANEL_UNLOCK();
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_panel_display(aui_hdl pv_hdl_panel,
		enum aui_panel_data_type data_type,
		unsigned char* puc_buf,
		unsigned long data_len)
{
	PANEL_LOCK();

	aui_handle_panel *p_hdl_panel = (aui_handle_panel *)pv_hdl_panel;
	if ((NULL == p_hdl_panel) ||(FALSE == is_valid_panel_handle(p_hdl_panel)))
	{
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if ((AUI_PANEL_OPEN != p_hdl_panel->ul_status)
			|| (NULL == p_hdl_panel->p_dev_obj))
	{
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	char ctrl_data[4]={0};

	if (AUI_PANEL_DATA_ANSI == data_type)
	{
		if (data_len >= p_hdl_panel->cfg_panel.ul_led_cnt)
			pan_display(p_hdl_panel->p_dev_obj, (char*)puc_buf,
					p_hdl_panel->cfg_panel.ul_led_cnt);
		else
			pan_display(p_hdl_panel->p_dev_obj, (char*)puc_buf, data_len);
	}
	else if (AUI_PANEL_CMD_LED_LOCK == data_type)
	{
		ctrl_data[0] = 27;
		ctrl_data[1] = PAN_ESC_CMD_LBD;
		ctrl_data[2] = PAN_ESC_CMD_LBD_LOCK;
		ctrl_data[3] = PAN_ESC_CMD_LBD_ON;
		pan_display(p_hdl_panel->p_dev_obj, ctrl_data, 4);
	}

	PANEL_UNLOCK();

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_panel_clear(aui_hdl pv_hdl_panel,
		enum aui_panel_data_type data_type)
{
	PANEL_LOCK();

	aui_handle_panel *p_hdl_panel = (aui_handle_panel *)pv_hdl_panel;
	if ((NULL == p_hdl_panel) || (FALSE == is_valid_panel_handle(p_hdl_panel)))
	{
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	char ctrl_data[4]={0};
	MEMSET(ctrl_data, 0, sizeof(ctrl_data));

	if (AUI_PANEL_DATA_ANSI == data_type)
	{
		pan_display(p_hdl_panel->p_dev_obj, ctrl_data,
				sizeof(ctrl_data));
	}
	else if (AUI_PANEL_CMD_LED_LOCK == data_type)
	{
		ctrl_data[0] = 27;
		ctrl_data[1] = PAN_ESC_CMD_LBD;
		ctrl_data[2] = PAN_ESC_CMD_LBD_LOCK;
		ctrl_data[3] = PAN_ESC_CMD_LBD_OFF;
		pan_display(p_hdl_panel->p_dev_obj, ctrl_data, 4);
	}

	PANEL_UNLOCK();
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_panel_get(aui_hdl pv_hdl_panel,
		aui_panel_item_get ul_item,
		void* pv_param)
{
	PANEL_LOCK();
	(void)pv_param;
	aui_handle_panel *p_hdl_panel = (aui_handle_panel *)pv_hdl_panel;
	if ((NULL == p_hdl_panel) 
        || (FALSE == is_valid_panel_handle(p_hdl_panel))){
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	switch(ul_item)
	{
	default:
		break;
	}

	PANEL_UNLOCK();
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_panel_set(aui_hdl pv_hdl_panel,
		aui_panel_item_set	ul_item,
		void* pv_param)
{
	PANEL_LOCK();
	(void)pv_param;
	aui_handle_panel *p_hdl_panel = (aui_handle_panel *)pv_hdl_panel;
	if ((NULL == p_hdl_panel)
         || (FALSE == is_valid_panel_handle(p_hdl_panel))){
		PANEL_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	switch (ul_item)
	{
	default:
		break;
	}

	PANEL_UNLOCK();
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_set_led_state(aui_hdl pv_hdl_panel, unsigned long ul_led_number,
                        unsigned char uc_led_active)
{
	PANEL_LOCK();
    if ((NULL == pv_hdl_panel) 
        || ((0 != ul_led_number) 
              && (1 != ul_led_number) 
              && (2 != ul_led_number)
              && (3 != ul_led_number)) 
        || ((1 != uc_led_active) && (0 != uc_led_active))) {
        AUI_ERR("invalid params.\n");
        PANEL_UNLOCK();
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    unsigned long wr_len = 4;
    char data[4] = { 0 }; // control data length is 4
    aui_handle_panel *p_hdl_panel = (aui_handle_panel *)pv_hdl_panel;
    data[0] = 27;
    data[1] = PAN_ESC_CMD_LBD;
    switch(ul_led_number) {
        case 0:
            data[2] = PAN_ESC_CMD_LBD_POWER;
			break;
        case 1:
            data[2] = PAN_ESC_CMD_LBD_LOCK;
            break;
        case 2:
            data[2] = PAN_ESC_CMD_LBD_FUNCA;
			break;
        case 3:
            data[2] = PAN_ESC_CMD_LBD_FUNCB;
            break;
    }
    if(1 == uc_led_active)
        data[3] = PAN_ESC_CMD_LBD_ON;
    else
        data[3] = PAN_ESC_CMD_LBD_OFF;
    pan_display(p_hdl_panel->p_dev_obj, data, wr_len);
	PANEL_UNLOCK();
    return AUI_RTN_SUCCESS;
}
