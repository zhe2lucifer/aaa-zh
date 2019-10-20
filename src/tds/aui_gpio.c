/**@file
 *    @brief     ALi AUI GPIO function implementation
 *    @author    leon.peng
 *    @date      2015-09-26
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <aui_os.h>
#include <aui_common.h>
#include <aui_gpio.h>
#include <hal/hal_gpio.h>
#include "aui_common_priv.h"


#define ARRAY_SIZE(a)     (sizeof(a) / sizeof((a)[0]))

#define GPIO_CTRL_ENABLE 			1
#define GPIO_CTRL_DISABLE 			0

typedef struct aui_gpio_handler {
	/** internal data */
	aui_dev_priv_data dev_priv_data;
	/** gpio attr */
	aui_gpio_attr attr;
	/** gpio inter attr */	
	aui_gpio_interrupt_attr irq_gpio_attr;	//added by steven
	/** save the last tick */
    unsigned long last_tick;  				//added by steven
    /** save regcb flag */
	int regcb_set_flag; 					//added by steven
}aui_handle_gpio,*aui_p_handle_gpio;


#define gpio_handle_list_num  128
aui_handle_gpio* handle_gpio_list[gpio_handle_list_num];

AUI_MODULE(GPIO)

int verify_gpio_index()
{
	int i, index = -1;

	//need to search the pos of current interrupt.
	for (i = 0; i < gpio_handle_list_num; i++) {
		if (1 == HAL_GPIO_INT_STA_GET(i)) {
			index = i;
			break;
		}
	}
	
	return index;
}

int check_interval_timeout(unsigned long last_tick, unsigned long debounce_interval)
{
    return (osal_get_tick() - last_tick) > debounce_interval ? 1 : 0;
}


void gpio_irq(DWORD param)
{
	(void)param;
	aui_handle_gpio* gpio_handle;
	int index = verify_gpio_index();
	if ((index < 0) && (index >=  gpio_handle_list_num)) {
		AUI_ERR("get wrong index!\n");
		return;
	}

	gpio_handle = handle_gpio_list[index];
	if (gpio_handle == NULL) {
		AUI_ERR("can't get gpio_handle!\n");
		return;
	}

	UINT32 gpio_index = gpio_handle->attr.uc_dev_idx;

	//the gpio_index must equivalent the INDEX.
	if (gpio_index == index) {
		AUI_ERR("check the index is OK!\n");
	} else {
		AUI_ERR("check the index is ERR!\n");
		return;
	}

	int int_type = gpio_handle->irq_gpio_attr.interrupt_type;
	//clear the interrupt flag.
	HAL_GPIO_INT_CLEAR(gpio_index);

	//call the user's function.
	if (NULL == gpio_handle->irq_gpio_attr.p_callback) {
		AUI_ERR("hdl is NULL or handle->irq_gpio_attr.p_callback is NULL!\n");
		return;
	}
	
	if (gpio_handle->irq_gpio_attr.debounce_interval < 0) {
		AUI_ERR("debounce_interva is smaller than zero!\n");
		return;
	}
	//if user press the button so quickly, and we should ignore it.
	if (check_interval_timeout(gpio_handle->last_tick, (unsigned long)gpio_handle->irq_gpio_attr.debounce_interval)){
		//call the user callback function.
		gpio_handle->irq_gpio_attr.p_callback(gpio_index, int_type, gpio_handle->irq_gpio_attr.pv_user_data);
		gpio_handle->last_tick = osal_get_tick();
	} else {
		AUI_DBG("It in the interval,don't call user function!\n");
	}
}

static void gpio_get_config(UINT32* reg_addr , int *count){
	if(ALI_S3281==sys_ic_get_chip_id()){
		*reg_addr = 0xb8000040;
		*(reg_addr + 1) = 0xb80000a0;
		*(reg_addr + 2) = 0xb80000D0;
		*(reg_addr + 3) = 0xb8000150;
		*count = 4;
	}
	else{
		*reg_addr = 0xb8000430;
		*(reg_addr + 1) = 0xb8000434;
		*(reg_addr + 2) = 0xb8000438;
		*(reg_addr + 3) = 0xb800043c;
		*(reg_addr + 4) = 0xb8000440;
		*count = 5;
	}
}

static AUI_RTN_CODE gpio_ctrl_enable(aui_gpio_attr *attr, unsigned long enable)
{
	int i = 0;
	int count = 0;
	unsigned long value = 0;
	UINT32 reg_addr[5] = {0,0,0,0,0};
	AUI_RTN_CODE ret = AUI_RTN_FAIL;

	gpio_get_config(reg_addr,&count);

	for(i = 0; i < count; i++)
	{
		if((attr->uc_dev_idx >= (i*32)) && (attr->uc_dev_idx < ((i+1)*32)))
		{
			value = *((volatile unsigned long *)reg_addr[i]);

			if(enable == GPIO_CTRL_ENABLE)
			{
				value |= enable << (attr->uc_dev_idx - (i*32));
				//set gpio able status
				*((volatile unsigned long *)reg_addr[i]) = value;
				//set gpio direction
				HAL_GPIO_BIT_DIR_SET(attr->uc_dev_idx, attr->io);
			}
			else if(enable == GPIO_CTRL_DISABLE)
			{
				value &= ~(1 << (attr->uc_dev_idx - (i*32)));
				//set gpio able status
				*((volatile unsigned long *)reg_addr[i]) = value;
			}
			else
			{
				ret = AUI_RTN_EINVAL;
				return ret;
			}

			ret = AUI_RTN_SUCCESS;
			break;
		}
	}

	return ret;
}

AUI_RTN_CODE aui_gpio_set_value(aui_hdl handle, aui_gpio_value value)
{
	if(NULL==(aui_handle_gpio *)handle)
	{
		aui_rtn(AUI_RTN_EINVAL," hanle is NULL");
	}

	HAL_GPIO_BIT_SET(((aui_handle_gpio *)handle)->attr.uc_dev_idx,(unsigned char)value);


	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gpio_get_value(aui_hdl handle, aui_gpio_value *value)
{
	if((NULL == (aui_handle_gpio *)handle) || (NULL == value))
	{
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}

	*value = HAL_GPIO_BIT_GET(((aui_handle_gpio *)handle)->attr.uc_dev_idx);

	aui_gpio_dir gpio_dir;
	gpio_dir = HAL_GPIO_BIT_DIR_GET(((aui_handle_gpio *)handle)->attr.uc_dev_idx);
	UINT32  interrupt_type;
	interrupt_type = HAL_GPIO_INT_STA_GET((UINT32)((aui_handle_gpio *)handle)->attr.uc_dev_idx);//HAL_GPIO_INT_STA_GET
	AUI_INFO("value:%x, dir:%x, interrup:%x\n", *value, gpio_dir, interrupt_type);

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_gpio_open(aui_gpio_attr *attr, aui_hdl *handle)
{
	AUI_RTN_CODE ret = AUI_RTN_FAIL;
	aui_handle_gpio* gpio_handle = NULL;

	gpio_handle=(aui_handle_gpio *)MALLOC(sizeof(aui_handle_gpio));

	if(NULL==gpio_handle)
	{
		aui_rtn(AUI_RTN_ENOMEM,"gpio_handle is NULL");
	}
	MEMSET(gpio_handle,0,sizeof(aui_handle_gpio));

	if(NULL==attr)
	{
		aui_rtn(AUI_RTN_EINVAL,"attr is NULL");
	}
	else
	{
		MEMCPY(&(gpio_handle->attr),attr,sizeof(aui_gpio_attr));
		ret = gpio_ctrl_enable(attr,GPIO_CTRL_ENABLE);
	}
	gpio_handle->dev_priv_data.dev_idx = attr->uc_dev_idx;
	*handle = (aui_hdl)gpio_handle;

	ret |= aui_dev_reg(AUI_MODULE_GPIO, (aui_hdl)gpio_handle);

	return ret;
}


AUI_RTN_CODE aui_gpio_close(aui_hdl handle)
{
	AUI_RTN_CODE ret = AUI_RTN_FAIL;
	aui_handle_gpio* gpio_handle = (aui_handle_gpio *)handle;

	if((NULL == gpio_handle))
	{
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
	//unreg interrupt, must check it first.
	if (0 != gpio_handle->regcb_set_flag)
		aui_gpio_interrupt_unreg(gpio_handle);

	//disable gpio ctrl
	ret = gpio_ctrl_enable(&(gpio_handle->attr),GPIO_CTRL_DISABLE);

	//dev unreg
	aui_dev_unreg(AUI_MODULE_GPIO, (aui_hdl)gpio_handle);

	FREE(gpio_handle);

	return ret;
}

AUI_RTN_CODE aui_gpio_init(p_fun_cb p_call_back_init,void *pv_param)
{  

	if(p_call_back_init && pv_param )
		return p_call_back_init(pv_param);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gpio_deinit()
{
	return AUI_RTN_SUCCESS;
}

/**
 *  Function Name:      	aui_gpio_interrupt_reg
 *  @brief             	cal this function to register user's callback function that be called when event happended.
 *
 * @param[in]     		p_hdl:			gpio irq device pointer
 * @param[in]     		p_interrupt_attr:	gpio irq attribute's pointer
 *
 *  @return             	aui_retcode
 *
 *  @author             	Steven Zhang <steven.zhang@alitech.com>
 *  @date               	2017.05.26, added
 *
 *  @note
 */

AUI_RTN_CODE aui_gpio_interrupt_reg(aui_hdl p_hdl, aui_gpio_interrupt_attr *p_interrupt_attr) 
{
	aui_handle_gpio *gpio_handle = (aui_handle_gpio *)p_hdl;
	UINT32 gpio_index = 0;
	aui_gpio_interrupt_type irq_type;
	if ((NULL == p_hdl)
			|| (NULL == p_interrupt_attr)
			|| (NULL == p_interrupt_attr->p_callback)) {
		aui_rtn(AUI_RTN_EINVAL, "NULL attribute!\n");
	}

	//irq_type
	irq_type = p_interrupt_attr->interrupt_type;
	//index
	gpio_index = gpio_handle->attr.uc_dev_idx;
	if (gpio_index < 0) {
		aui_rtn(AUI_RTN_EINVAL, "gpio index is invalid!");
	}
	//interval_timeout
	if(p_interrupt_attr->debounce_interval < 0)
		gpio_handle->irq_gpio_attr.debounce_interval = 0;
	else if (p_interrupt_attr->debounce_interval == 0)
		gpio_handle->irq_gpio_attr.debounce_interval = 200;
	else
		gpio_handle->irq_gpio_attr.debounce_interval = p_interrupt_attr->debounce_interval;

	if ((AUI_GPIO_INTERRUPT_DISABLED > irq_type) || (AUI_GPIO_INTERRUPT_MAX <= irq_type)) {
		AUI_ERR("invalid irq type\n");
		return AUI_RTN_EINVAL;
	}

	AUI_INFO("will set gpio interrupt, irq_type:%d\n", irq_type);
	//riq tye
	switch (irq_type) {
		case AUI_GPIO_INTERRUPT_DISABLED:         
			HAL_GPIO_INT_EDG_SET(gpio_index, 0, 0);            
			return AUI_RTN_SUCCESS;

		case AUI_GPIO_INTERRUPT_RISING_EDGE:	//Interrupt on a 0->1 transition.            
			HAL_GPIO_INT_EDG_SET(gpio_index, 1, 0);                        
			break;
		case AUI_GPIO_INTERRUPT_FALLING_EDGE:	//Interrupt on a 1->0 transition.
			HAL_GPIO_INT_EDG_SET(gpio_index, 0, 1);                                
			break;
		case AUI_GPIO_INTERRUPT_EDGE:	//Interrupt on both a 0->1 and a 1->0 transition.
			HAL_GPIO_INT_EDG_SET(gpio_index, 1, 1);    
			gpio_handle->irq_gpio_attr.debounce_interval = 0;//if  this case debounce interval will be 0.
			break;
		default:
			break;
	}
	//set up interup type;	
	gpio_handle->irq_gpio_attr.interrupt_type = irq_type;
	//callback fuanction and user data
	gpio_handle->irq_gpio_attr.p_callback = p_interrupt_attr->p_callback;
	gpio_handle->irq_gpio_attr.pv_user_data = p_interrupt_attr->pv_user_data;

	// enable gpio interrupt
	HAL_GPIO_INT_SET(gpio_index, 1);

	//register gpio proc to isr
	if (0 != gpio_handle->regcb_set_flag)//if have set to isr, then delete it
		os_delete_isr(IRQ_NUM, (ISR_PROC)gpio_irq);
	
	os_register_isr(IRQ_NUM, (ISR_PROC)gpio_irq, (DWORD)gpio_handle);
	gpio_handle->regcb_set_flag = 1;
	
	//init the last tick;
	gpio_handle->last_tick = osal_get_tick();
	AUI_INFO("irq_type:%x\n", gpio_handle->irq_gpio_attr.interrupt_type);
	
	handle_gpio_list[gpio_index] = gpio_handle;//save this gpio_handle to the list for verify when function gpio_irq be call.

	AUI_SLEEP(1000);/* sleep 1000ms */
	return AUI_RTN_SUCCESS;
}

/**
 *  Function Name:      	aui_gpio_interrupt_unreg
 *  @brief             	unregister user's callback function
 *
 * @param[in]     		p_hdl:		gpio irq device pointer
 *
 *  @return             	aui_retcode
 *
 *  @author             	Steven Zhang <steven.zhang@alitech.com>
 *  @date               	2017.05.26, added
 *
 *  @note
 */

AUI_RTN_CODE aui_gpio_interrupt_unreg(aui_hdl p_hdl) 
{
	if (NULL == p_hdl) {
		aui_rtn(AUI_RTN_EINVAL, "NULL attribute!\n");
	}

	aui_handle_gpio *gpio_handle = p_hdl;
	UINT32 gpio_index = gpio_handle->attr.uc_dev_idx;
	handle_gpio_list[gpio_index] = NULL;          //set the handle of list to NULL;

	if (0 != gpio_handle->regcb_set_flag)
		os_delete_isr(IRQ_NUM, (ISR_PROC)gpio_irq);
	gpio_handle->regcb_set_flag = 0;
	
	HAL_GPIO_INT_CLEAR(gpio_index);
	HAL_GPIO_INT_EDG_SET(gpio_index, 0, 0);

	return AUI_RTN_SUCCESS;
}

