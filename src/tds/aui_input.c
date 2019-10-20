/**  @file
*	 @brief 	Implement of AUI key moudle
*	 @author	smith.shi
*	 @date		2013-6-18
*	 @version	1.0.0
*	 @note		ali corp. all rights reserved. 2013-2020 copyright (C)
*/

/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <hal/hal_gpio.h>
#include <aui_input.h>
#include <aui_os.h>
#include <aui_common.h>

AUI_MODULE(INPUT)

extern struct pan_key * pan_get_key(struct pan_device *dev, UINT32 timeout);
/****************************LOCAL MACRO******************************************/
#define MAX_KEY_DEV_ID 1

#define KEY_TASK_CMD_STOP		0x00000001
#define KEY_TASK_CMD_EXITED 	0x00000002

#define PAN_KEY_INVALID			0xFFFFFFFF

#define LOG_INFO_0	1
#define ERR_INFO	2

#define KEY_FUNCTION_ENTER() AUI_INFO("enter \n")
#define KEY_FUNCTION_LEAVE() AUI_INFO("leave \n")


#define KEY_LOCK()		 do \
						  {  \
							if(INVALID_ID!=s_key_mutexID) \
								osal_mutex_lock(s_key_mutexID,OSAL_WAIT_FOREVER_TIME); \
						  }while(0)
									
#define KEY_UNLOCK()	 do \
						  {  \
							if(INVALID_ID!=s_key_mutexID) \
								osal_mutex_unlock(s_key_mutexID); \
						  }while(0)

/****************************LOCAL TYPE*******************************************/

enum dev_status
{
	AUI_KEY_DEV_IDLE=0,
	AUI_KEY_DEV_OPEN,
};

/** key  hendle*/
typedef struct aui_st_key_dev
{
	/** panel device object */
	struct pan_device*	p_dev_obj;
	/** open configure */
	aui_cfg_key 		 cfg_key;
	/** on/off  */
	unsigned long		dev_status;
	/** syn flag */
	OSAL_ID 			flagKDev;
}aui_key_dev,*aui_p_key_dev;

//typedef unsigned long aui_handle_key ;
typedef aui_key_dev aui_handle_key ;


/****************************LOCAL VAR********************************************/
//static aui_key_dev key_dev[MAX_KEY_DEV_ID];
static unsigned char s_key_mod_inited = FALSE;
static OSAL_ID s_key_mutexID = INVALID_ID;

/****************************LOCAL FUNC DECLEAR***********************************/
static void key_dispatch_task(void *p_param);
/****************************LOCAL FUNC IMPLEMENT********************************/							  
static BOOL pan_key2key_info(struct pan_key *p_pan_key,aui_key_info* p_key_info)
{
	if((NULL == p_pan_key) || (NULL == p_key_info))
		return FALSE;

	MEMSET(p_key_info,0,sizeof(aui_key_info));

	p_key_info->n_key_code = p_pan_key->code;
	p_key_info->n_count = p_pan_key->count;

	if((PAN_KEY_PRESSED == p_pan_key->state)
        &&(p_pan_key->count <= 1)) {
		if((PAN_KEY_TYPE_REMOTE == p_pan_key->type) 
           && (p_pan_key->count < 1)) {
			p_key_info->e_type = AUI_KEY_TYPE_REMOTECTRL;
		} else if(PAN_KEY_TYPE_PANEL == p_pan_key->type) {
			p_key_info->e_type = AUI_KEY_TYPE_FRONTPANEL;
		} else {
			p_key_info->e_type = 0;
		}
		p_key_info->e_status = AUI_KEY_STATUS_PRESS;
	} else if((PAN_KEY_PRESSED == p_pan_key->state)
	            &&(p_pan_key->count >= 3)){
		if(PAN_KEY_TYPE_REMOTE == p_pan_key->type)
			p_key_info->e_type = AUI_KEY_TYPE_REMOTECTRL;
		else if(PAN_KEY_TYPE_PANEL == p_pan_key->type)
			p_key_info->e_type = AUI_KEY_TYPE_FRONTPANEL;

		p_key_info->e_status = AUI_KEY_STATUS_REPEAT;
	} else if(PAN_KEY_RELEASE == p_pan_key->state) {
		if(PAN_KEY_TYPE_REMOTE == p_pan_key->type)
			p_key_info->e_type = AUI_KEY_TYPE_REMOTECTRL;
		else if(PAN_KEY_TYPE_PANEL == p_pan_key->type)
			p_key_info->e_type = AUI_KEY_TYPE_FRONTPANEL;

		p_key_info->e_status = AUI_KEY_STATUS_RELEASE;
	} else {
        AUI_ERR("unknown key state\n");
        return FALSE;
	}

	return TRUE;
}

static unsigned long s_ul_key_reg_val=0;
static unsigned long s_ul_key_press_count=0;
AUI_RTN_CODE read_front_power_key(void *pv_para1,void *pv_para2)
{
    unsigned long ul_key_reg_val=0;
    aui_key_dev* p_dev_obj = (aui_key_dev*)pv_para1;
    struct pan_key p_pan_key;
    aui_key_info key_info;
    (void) pv_para2;
    p_pan_key.type = PAN_KEY_TYPE_PANEL;
    
    while(1)
    {
        ul_key_reg_val = HAL_GPIO_BIT_GET(135);
        if(s_ul_key_reg_val!=ul_key_reg_val)
        {           
            p_pan_key.code = 0;     //The key code is given by client(Arion).
            s_ul_key_reg_val=ul_key_reg_val;
            if(ul_key_reg_val)
            {
                //AUI_DBG("\r\n Press power key,gpio[135]=[%08x]",ul_key_reg_val);
                s_ul_key_press_count++;
                p_pan_key.state = PAN_KEY_PRESSED;
                p_pan_key.count = s_ul_key_press_count;
            }
            else
            {
                s_ul_key_press_count--;
                p_pan_key.state = PAN_KEY_RELEASE;
                p_pan_key.count = s_ul_key_press_count;
                //AUI_DBG("\r\n Release power key,gpio[135]=[%08x]",ul_key_reg_val);

            }

            MEMSET((UINT8 *)&key_info, 0, sizeof(key_info));
            
            if(FALSE == pan_key2key_info(&p_pan_key,&key_info))
            {
                continue;
            }

            if(NULL != p_dev_obj->cfg_key.fn_callback)
            {
                p_dev_obj->cfg_key.fn_callback(&key_info,p_dev_obj->cfg_key.pv_user_data);
            }
            
        }
        aui_os_task_sleep(10);
    }
}

AUI_RTN_CODE aui_key_version_get(unsigned long *pul_version)
{
	(void) pul_version;
	return AUI_RTN_SUCCESS;    
}

AUI_RTN_CODE aui_key_init(p_fun_cb p_call_back_init,void *pv_param)
{
	KEY_FUNCTION_ENTER();

	if(FALSE == s_key_mod_inited) {
		if(NULL == p_call_back_init) {
			aui_rtn(AUI_RTN_EINVAL,NULL);
 		} else {
			p_call_back_init(pv_param);
		}

		s_key_mutexID = osal_mutex_create();
		if(INVALID_ID == s_key_mutexID) {
			aui_rtn(AUI_RTN_NO_RESOURCE,NULL);
		}
		s_key_mod_inited = TRUE;
	} else {
		AUI_WARN("already inited\n");
	}

	KEY_FUNCTION_LEAVE();
	return AUI_RTN_SUCCESS;    
}

AUI_RTN_CODE aui_key_de_init(p_fun_cb p_call_back_de_init,void *pv_param)
{
	KEY_FUNCTION_ENTER();

	if(TRUE == s_key_mod_inited) {
		if(NULL == p_call_back_de_init) {
			aui_rtn(AUI_RTN_EINVAL,NULL);
		} else {
			p_call_back_de_init(pv_param);
		}
		
		if(INVALID_ID != s_key_mutexID) {
			osal_mutex_delete(s_key_mutexID);
			s_key_mutexID = INVALID_ID;
		}
	} else {
		AUI_WARN("have not inited\n");
	}
	
	KEY_FUNCTION_LEAVE();
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_key_open(unsigned long ul_devID,
						aui_cfg_key *p_key_param,
						aui_hdl *pp_hdl_key)
{
	KEY_LOCK() ;
	(void) ul_devID;
	if(NULL == pp_hdl_key) {
        AUI_ERR("pp_hdl_key is NULL!");
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	aui_handle_key *p_hdl_key= (aui_handle_key *)MALLOC(sizeof(aui_handle_key));
	if(NULL == p_hdl_key) {
        AUI_ERR("malloc fail!");
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	MEMSET(p_hdl_key,0,sizeof(aui_handle_key));
	p_hdl_key->p_dev_obj = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);
	p_hdl_key->dev_status = AUI_KEY_DEV_OPEN;
	
	if(NULL != p_key_param) {
		p_hdl_key->cfg_key = *p_key_param;
	}

	if(NULL == p_hdl_key->p_dev_obj) {
        AUI_ERR("get HLD_DEV_TYPE_PAN fail!");
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if(NULL != dev_get_by_name("PAN_DIRECT_GPIO"))
	{
    	aui_hdl entry_task_hdl=NULL;

    	aui_attr_task attr_task;
        memset(&attr_task, 0, sizeof(aui_attr_task));
    	strncpy(attr_task.sz_name, "panelpowerkey", strlen("panelpowerkey"));
    	attr_task.ul_priority = 20;
    	attr_task.p_fun_task_entry = read_front_power_key;
    	attr_task.para1 = (unsigned long)p_hdl_key;
    	attr_task.ul_quantum = 10;
    	attr_task.ul_stack_size = 0x1000;
    	aui_os_task_create(&attr_task, &entry_task_hdl);	
	}
	p_hdl_key->flagKDev =  OSAL_INVALID_ID;
	*(aui_handle_key **)pp_hdl_key = p_hdl_key ;  

	KEY_UNLOCK() ;
	return SUCCESS;
}

AUI_RTN_CODE aui_key_close(aui_hdl pv_hdl_key)
{
	KEY_LOCK() ;
	
	unsigned long flgptn = 0;
	OSAL_ER result = OSAL_E_OK;    
	aui_handle_key *p_hdl_key = (aui_handle_key *)pv_hdl_key;
	
	if((NULL == p_hdl_key) ||(AUI_KEY_DEV_OPEN != p_hdl_key->dev_status) )
	{
	    AUI_ERR("pv_hdl_key is NULL or dev not opened!");
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	
    if (OSAL_INVALID_ID != p_hdl_key->flagKDev)
	    osal_flag_set(p_hdl_key->flagKDev,KEY_TASK_CMD_STOP);
	p_hdl_key->dev_status = AUI_KEY_DEV_IDLE;
	KEY_UNLOCK() ;
	
	if (OSAL_INVALID_ID != p_hdl_key->flagKDev)
    	result = osal_flag_wait(&flgptn,p_hdl_key->flagKDev, KEY_TASK_CMD_EXITED, 
    		OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
	if(OSAL_E_OK == result) // || (flgptn&KEY_TASK_CMD_EXITED) == KEY_TASK_CMD_EXITED
	{
		KEY_LOCK() ;
        if (OSAL_INVALID_ID != p_hdl_key->flagKDev) {
		    osal_flag_delete(p_hdl_key->flagKDev);
            p_hdl_key->flagKDev = OSAL_INVALID_ID;
        }
		FREE(p_hdl_key);
        p_hdl_key = NULL;
		KEY_UNLOCK() ;
		return AUI_RTN_SUCCESS;
	}
	
	return AUI_RTN_FAIL;
}


static void key_dispatch_task(void *p_param)
{
	aui_key_dev* p_dev_obj = (aui_key_dev*)p_param;
	OSAL_ER result = OSAL_E_OK;
	struct pan_key *p_pankey = NULL;
	struct pan_key key_struct;
	
	while(1)
	{
		unsigned long flgptn = 0;
		result = osal_flag_wait(&flgptn,p_dev_obj->flagKDev, KEY_TASK_CMD_STOP, OSAL_TWF_ANDW, 0);
		if(OSAL_E_OK == result)
		{
			osal_flag_clear(p_dev_obj->flagKDev,KEY_TASK_CMD_STOP);
			break;
		}

		KEY_LOCK() ;
		p_pankey = (struct pan_key *)pan_get_key(p_dev_obj->p_dev_obj, 10); 	   
		if (p_pankey == NULL)
		{
			KEY_UNLOCK() ;
			continue;
		}
		else
		{
			MEMCPY(&key_struct, p_pankey, sizeof(struct pan_key));
			KEY_UNLOCK() ;
		}

		//AUI_DBG("---------------------key code[%d] ddddstate[%d] count[%d]!\n",key_struct.code,key_struct.state,key_struct.count);
		if ((PAN_KEY_INVALID == key_struct.code) 
			|| ((PAN_KEY_PRESSED != key_struct.state) 
			     && (PAN_KEY_RELEASE != key_struct.state)) )
		{
			continue;
		}

		aui_key_info key_info;
		if(FALSE == pan_key2key_info(&key_struct,&key_info))
		{
			continue;
		}

		if( NULL != p_dev_obj->cfg_key.fn_callback)
		{
			p_dev_obj->cfg_key.fn_callback(&key_info,p_dev_obj->cfg_key.pv_user_data);
		}

	}
	osal_flag_set(p_dev_obj->flagKDev,KEY_TASK_CMD_EXITED);    
}

/**
*	 @brief 	register a callback function to the driver, if the driver received key fn_callback will call to report
*	 @author		smith.shi
*	 @date			2013-7-5
*	 @param[in]		pv_hdl_key key device handle
*	 @param[in]		fn_callback  callback function£¬fn_callback = NULL is deregister 
*	 @return		returns AUI_RTN_SUCCESS by success£¬return to other error code failed
*	 @note		
*
*/
AUI_RTN_CODE aui_key_callback_register(aui_hdl pv_hdl_key,aui_key_callback fn_callback)
{
	KEY_LOCK() ;
	aui_handle_key *p_hdl_key = (aui_handle_key *)pv_hdl_key;
	if((NULL == p_hdl_key) ||(AUI_KEY_DEV_OPEN != p_hdl_key->dev_status) )
	{
	    AUI_ERR("pv_hdl_key is NULL or dev not opened!");
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	p_hdl_key->cfg_key.fn_callback = fn_callback;
    if (NULL == fn_callback) {
        if (OSAL_INVALID_ID != p_hdl_key->flagKDev){
            unsigned long flgptn = 0;
            osal_flag_set(p_hdl_key->flagKDev,KEY_TASK_CMD_STOP);
            osal_flag_wait(&flgptn,p_hdl_key->flagKDev, KEY_TASK_CMD_EXITED, 
		                    OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
            osal_flag_delete(p_hdl_key->flagKDev);
            p_hdl_key->flagKDev = OSAL_INVALID_ID;
        }
        
    } else {
        p_hdl_key->flagKDev = osal_flag_create(0);
    	if(OSAL_INVALID_ID == p_hdl_key->flagKDev)
    	{
    	    AUI_ERR("osal_flag_create fail!");
    		KEY_UNLOCK() ;
    		aui_rtn(AUI_RTN_EINVAL,NULL);
    	}

        T_CTSK tsk_param;
        OSAL_ID taskID;
        tsk_param.task = (FP)key_dispatch_task;
        tsk_param.name[0] = 'A';
        tsk_param.name[1] = 'K';
        tsk_param.name[2] = 'D';
        tsk_param.quantum = 10;
        tsk_param.itskpri = OSAL_PRI_NORMAL;
        tsk_param.stksz = 0x1000;
        tsk_param.para1 = (unsigned long)p_hdl_key;

    	taskID = osal_task_create(&tsk_param); 
    	if(OSAL_INVALID_ID == taskID)
    	{
    	    AUI_ERR("osal_task_create fail!");
    		osal_flag_delete(p_hdl_key->flagKDev);    		
    		KEY_UNLOCK() ;
    		aui_rtn(AUI_RTN_EINVAL,NULL);
    	}
        AUI_INFO("<%s> run task: %d\n", __func__, taskID);
    }
    
	KEY_UNLOCK() ;
	return AUI_RTN_SUCCESS;    
}

/**
*	 @brief 		Get key,by the user access
*	 @author		smith.shi
*	 @date			2013-7-5
*	 @param[in]		pv_hdl_key key device handle
*	 @param[out]	pkey_info get key information
*	 @return		returns AUI_RTN_SUCCESS by success£¬return to other error code failed
*	 @note		
*
*/
AUI_RTN_CODE aui_key_key_get(aui_hdl pv_hdl_key,aui_key_info* pkey_info)
{	 
	KEY_LOCK() ;
	
	aui_handle_key *p_hdl_key = (aui_handle_key *)pv_hdl_key;
	if((NULL == p_hdl_key) 
       ||(AUI_KEY_DEV_OPEN != p_hdl_key->dev_status) 
       || (NULL == pkey_info) ) {
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	struct pan_key *p_pankey = NULL;
	struct pan_key key_struct;

	MEMSET(pkey_info,0,sizeof(aui_key_info));
	
	p_pankey = (struct pan_key *)pan_get_key(p_hdl_key->p_dev_obj,0);
	if (p_pankey == NULL)
	{
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	MEMCPY(&key_struct, p_pankey, sizeof(struct pan_key));

	if ((PAN_KEY_INVALID == key_struct.code) 
		|| ((PAN_KEY_PRESSED != key_struct.state) 
		     && (PAN_KEY_RELEASE != key_struct.state)) )
	{
	    //AUI_ERR("invalid key.");
		KEY_UNLOCK() ;
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	aui_key_info key_info;
	MEMSET(&key_info,0,sizeof(aui_key_info));

	if(FALSE == pan_key2key_info(&key_struct,&key_info))
	{
		KEY_UNLOCK() ;
		return AUI_RTN_FAIL; 
	}

	MEMCPY(pkey_info, &key_info, sizeof(aui_key_info));

	KEY_UNLOCK() ;
	return AUI_RTN_SUCCESS;  
}

AUI_RTN_CODE aui_pan_ir_set_endian(struct aui_pan_ir_endian aui_ir_endian)
{
    pan_ir_set_endian(*(struct pan_ir_endian*) &aui_ir_endian);
    return AUI_RTN_SUCCESS;  
}
