/**  @file
*	 @brief         implement the AUI's UART
*	 @author	 smith.shi
*	 @date		   2013-6-18
*	 @version	  1.0.0
*	 @note		   ali corp. all rights reserved. 2013-2020 copyright (C)
         UART module is a dependent module, you can read & write it
         by the handle, the UDI is the simple package for the obj of
         device, you can get the handle by open interface, then can read,
         write, clear, enable and disable it.
	 the follow is:
	 init->open->read/write->close->deinit

*/

/****************************INCLUDE HEAD FILE************************************/

#include "aui_common_priv.h"
#include <aui_uart.h>
#include <bus/sci/sci.h>


extern void sci_16550uart_set_mode(UINT32 id, UINT32 bps, int parity);
extern UINT8 sci_16550uart_read(UINT32 id);
extern INT32 sci_16550uart_read_tm(UINT32 id, UINT8* data, UINT32 timeout);
extern void sci_16550uart_write(UINT32 id, UINT8 ch);
extern void sci_16550uart_clear_buff(UINT32 id);


/***************	  LOCAL MACRO		   *******************/
#define MAX_UART_ID 2
#define LOG_INFO_0	1
#define ERR_INFO	2

//#define INVALID_HANDLE 0xFFFFFFFF
//#define WAIT_FOR_EVER 0x0FFFFFFF

AUI_MODULE(UART)

#define UART_LOCK() 	  do \
						  {  \
							if(INVALID_ID!=s_uart_mutex_id) \
								osal_mutex_lock(s_uart_mutex_id,\
								OSAL_WAIT_FOREVER_TIME); \
						  }while(0)

#define UART_UNLOCK()	  do \
						  {  \
							if(INVALID_ID!=s_uart_mutex_id) \
								osal_mutex_unlock(s_uart_mutex_id); \
						  }while(0)

/*****************	  LOCAL TYPE	 ************/
/** attribute of uart device */
typedef struct aui_st_uart_dev
{
	/** device id */
	unsigned long	ul_dev_id;
	/** uart configuration */
	aui_attr_uart	 cfg_uart;
	/** status of r/w */
	unsigned char	uc_enable;
	/** used or free */
	unsigned long	ul_status;
}aui_st_uart_dev,*aui_p_uart_dev;

//typedef unsigned long aui_handle_uart ;
typedef aui_st_uart_dev aui_handle_uart ;

/*************		LOCAL VAR	   ********************/
static aui_attr_uart st_default_uart_cfg=
{
	115200,
	AUI_UART_DATA_8BITS,
	AUI_UART_STOP_1_0,
	AUI_UART_PARITY_NONE,
	AUI_UART_FLOW_CONTROL_NONE ,
};

//static aui_st_uart_dev* uart_dev[MAX_UART_ID]={0};
typedef struct aui_st_uart_info
{
	aui_handle_uart* ap_uart_hnd[MAX_UART_ID];
}aui_uart_info;
static aui_uart_info s_uart_info;

static unsigned char s_uart_mod_inited = FALSE;
static OSAL_ID s_uart_mutex_id = INVALID_ID;

/****************************LOCAL FUNC DECLEAR***********************************/
/**************************LOCAL FUNC IMPLEMENT**********************************/

static int find_uart_handle(aui_handle_uart* hnd)
{
	int i = 0;
	for(i=0 ; i<MAX_UART_ID; i++)
	{
		if(hnd == s_uart_info.ap_uart_hnd[i])
		{
			return i;
		}
	}
	return -1;
}

static int find_empty_slot()
{
	int i = 0;
	for(i=0 ; i<MAX_UART_ID; i++)
	{
		if(0 == s_uart_info.ap_uart_hnd[i])
		{
			return i;
		}
	}
	return -1;
}

static int add_uart_handle(aui_handle_uart* hnd)
{
	int slot = find_empty_slot();
	if(slot >=0 )
	{
		s_uart_info.ap_uart_hnd[slot] = hnd;
		return slot;
	}
	return -1;
}

static int del_uart_handle(aui_handle_uart* hnd)
{
	int slot = find_uart_handle(hnd);
	if(slot >=0 )
	{
		s_uart_info.ap_uart_hnd[slot] = 0;
		return slot;
	}
	return -1;
}

static int is_valid_uart_handle(aui_handle_uart* hnd)
{
	int slot = find_uart_handle(hnd);
	if(slot >=0 )
	{
		return TRUE;
	}
	return FALSE;
}

static int is_uart_dev_opened(unsigned long ul_uart_id)
{
	int i = 0;
	for(i=0 ; i<MAX_UART_ID; i++)
	{
		if(0 != s_uart_info.ap_uart_hnd[i])
		{
			if(ul_uart_id == s_uart_info.ap_uart_hnd[i]->ul_dev_id)
				return TRUE;
		}
	}
	return FALSE;
}

static unsigned long uart_cfg2sci_parity(aui_attr_uart* p_cfg)
{
	unsigned long ul_parity = 0;
	if(NULL == p_cfg)
	{
		AUI_ERR("bad parameter \n");
		return 0;
	}

	if(AUI_UART_PARITY_NONE == p_cfg->ul_parity)
		ul_parity |= SCI_PARITY_NONE;
	else if(AUI_UART_PARITY_EVEN == p_cfg->ul_parity)
		ul_parity |= SCI_PARITY_EVEN;
	else if(AUI_UART_PARITY_ODD == p_cfg->ul_parity)
		ul_parity |= SCI_PARITY_ODD;
	else
		ul_parity |= SCI_PARITY_NONE;

	//stop bit
	if(AUI_UART_STOP_1_0 == p_cfg->ul_stop_bits)
		ul_parity |= SCI_STOPBIT_1;
	else
		ul_parity |= SCI_STOPBIT_2;

	//data bit
	if(AUI_UART_DATA_8BITS== p_cfg->ul_data_bits)
		ul_parity |= SCI_WORDLEN_8;
	else if(AUI_UART_DATA_7BITS== p_cfg->ul_data_bits)
		ul_parity |= SCI_WORDLEN_7;
	else if(AUI_UART_DATA_6BITS== p_cfg->ul_data_bits)
		ul_parity |= SCI_WORDLEN_6;
	else if(AUI_UART_DATA_5BITS== p_cfg->ul_data_bits)
		ul_parity |= SCI_WORDLEN_5;
	else
		ul_parity |= SCI_WORDLEN_8;

	return ul_parity;
}



/****************************MODULE IMPLEMENT*************************************/
/**
*	 @brief 		get the handle of UART by id
*	 @author		Comer.Wei
*	 @date			2014-11-27
*	 @param[in]	        ul_uart_id: id of input, type is unsigned int
*	 @param[out]	        pv_handle_uart: handle of UART for output
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_handle_get_by_id(unsigned int ul_uart_id, aui_hdl *pv_handle_uart)
{
	int ret = AUI_RTN_SUCCESS;
	if(ul_uart_id >= MAX_UART_ID)
	{
		aui_rtn(AUI_RTN_EINVAL,"ul_uart_id >= MAX_UART_ID");
	}

	*pv_handle_uart = (aui_hdl)(s_uart_info.ap_uart_hnd[ul_uart_id]);

	if(NULL == *pv_handle_uart)
	{
		aui_rtn(AUI_RTN_EINVAL,"pv_handle_uart is NULL");
	}
	
	return ret;
}

/****************************MODULE IMPLEMENT*************************************/
/**
*	 @brief 		get the handle of UART by id
*	 @author		Comer.Wei
*	 @date			2014-11-27
*	 @param[in]             uart_handle:the UART handle input
*	 @param[out]	        p_uart_id: the pointer of UART id for output
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_id_get_by_handle(aui_hdl uart_handle, unsigned int *p_uart_id)
{
	int ret = AUI_RTN_SUCCESS;
	
	if(NULL == uart_handle)
	{
		aui_rtn(AUI_RTN_EINVAL,"uart_handle is NULL");
	}

	*p_uart_id = ((aui_handle_uart *)uart_handle)->ul_dev_id;
	
	return ret;
}

/**
*	 @brief 	        to get the version number of this module	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[out]            pul_version: the version number of module, type is unsigned long
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @return		
*	 @note
*
*/
AUI_RTN_CODE aui_uart_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL,"pul_version is NULL");
	}

	*pul_version = AUI_MODULE_VERSION_NUM_UART;
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        initial the UART module, It must be initial before use	
*	 @author		smith.shi
*	 @date			  2013-6-18
*	 @param[in] 	   p_call_back_init the callback function for initial
*	 @param[in] 	   pv_param: the parameters for callback function 
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_init(p_fun_cb p_call_back_init,void *pv_param)
{
    AUI_API("ENTER");

	if(FALSE == s_uart_mod_inited)
	{
		if(NULL == p_call_back_init)
		{
			aui_rtn(AUI_RTN_EINVAL,"p_call_back_init is NULL");
		}
		else
		{
			p_call_back_init(pv_param);
		}

		s_uart_mutex_id = osal_mutex_create();
		if(INVALID_ID == s_uart_mutex_id)
		{
			aui_rtn(AUI_RTN_EIO,"INVALID_ID == s_uart_mutex_id");
		}
		s_uart_mod_inited = TRUE;
	}
	else
	{
            AUI_INFO("already inited\n");
	}

	AUI_API("LEAVE");
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        destruct the UART's initial	
*	 @author		smith.shi
*	 @date			  2013-6-18
*	 @param[in] 	   p_call_back_de_init: the callback of deinitial
*	 @param[in] 	   pv_param: the parameters for callback function 
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_de_init(p_fun_cb p_call_back_de_init,void *pv_param)
{
    AUI_API("ENTER");

	if(TRUE == s_uart_mod_inited)
	{
		if(NULL == p_call_back_de_init)
		{
			aui_rtn(AUI_RTN_EINVAL,"NULL== p_call_back_de_init");
		}
		else
		{
			p_call_back_de_init(pv_param);
		}

		if(INVALID_ID != s_uart_mutex_id)
		{
			osal_mutex_delete(s_uart_mutex_id);
			s_uart_mutex_id = INVALID_ID;
		}
		s_uart_mod_inited = FALSE;
	}
	else
	{
            AUI_INFO("have not inited\n");
	}

	AUI_API("LEAVE");
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        to get the handle by id and parameters	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        ul_uartID: the id of UART device, base by 0
*	 @param[in] 	        p_uart_param:parameters of UARD device
*	 @param[out]	        pp_hdl_uart: the handle of device for output
*	 @return                AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_open(unsigned long ul_uart_id,
						aui_attr_uart *p_uart_param,
						aui_hdl *pp_hdl_uart)
{
	UART_LOCK();
	if((NULL == pp_hdl_uart) || (NULL != *(aui_handle_uart **)pp_hdl_uart)
		||(ul_uart_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"pp_hdl_uart is NULL");
	}
	if(TRUE == is_uart_dev_opened(ul_uart_id))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_NO_RESOURCE,"uart_dev_opene is ture");
	}

	if(find_empty_slot() < 0)
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_NO_RESOURCE,"find_empty_slot() < 0");
	}

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)MALLOC(\
											sizeof(aui_handle_uart));
	if(NULL == p_hdl_uart)
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_ENOMEM,"NULL == p_hdl_uart");
	}
	MEMSET(p_hdl_uart,0,sizeof(aui_handle_uart));

	unsigned long ul_parity = 0;
	aui_attr_uart* p_uart_cfg = (p_uart_param == NULL) ? \
								&st_default_uart_cfg : p_uart_param;

	//parity
	ul_parity = uart_cfg2sci_parity(p_uart_cfg);

	sci_mode_set(ul_uart_id, p_uart_cfg->ul_baudrate, ul_parity);


	p_hdl_uart->ul_dev_id=ul_uart_id;
	p_hdl_uart->uc_enable=AUI_UART_READ_WRITE;
	p_hdl_uart->cfg_uart= (p_uart_param == NULL) ? \
			st_default_uart_cfg : (*p_uart_param);
	p_hdl_uart->ul_status=AUI_UART_OPEN;

	*(aui_handle_uart **)pp_hdl_uart = p_hdl_uart;
	add_uart_handle(p_hdl_uart);

	UART_UNLOCK();
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        close the device	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart: the handle of uart for close 
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_close(aui_hdl pv_hdl_uart)
{
	UART_LOCK();

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;
	if((NULL == p_hdl_uart) ||(FALSE == is_valid_uart_handle(p_hdl_uart))
		||(p_hdl_uart->ul_dev_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"p_hdl_uart is NULL");
	}

	if(AUI_UART_OPEN == p_hdl_uart->ul_status)
	{
		p_hdl_uart->ul_status = AUI_UART_IDLE;
	}


	del_uart_handle(p_hdl_uart);
	FREE(p_hdl_uart);
	p_hdl_uart=NULL;

	UART_UNLOCK();
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        the buffer for read with length from UART	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart: handle of UART
*	 @param[in] 	        pu_buf: the destination address
*	 @param[in] 	        ul_read_len: the length, it must less then buffer's length
*	 @param[in] 	        ul_time_out: timeout, 0 mean wait forever
*	 @param[out]	        pul_readed_len: the length have be read.
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_read(aui_hdl pv_hdl_uart,unsigned char* pu_buf,
					unsigned long ul_read_len, unsigned long* pul_readed_len,
					unsigned long ul_time_out)
{
	UART_LOCK();

		unsigned long ul_readed= 0;
	unsigned char ch= 0;
	unsigned long ul_err_cnt= 0;
	unsigned long ul_timed= 0;
	unsigned long ul_ret_code = AUI_RTN_SUCCESS;
	unsigned long	ul_dev_id = 0;
	unsigned long timeout_perread= 1000;//us
	ul_time_out *= 1000;//ms to us
	
	unsigned long error_time = ul_time_out / timeout_perread;

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;
	if((NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart))
		|| (p_hdl_uart->ul_dev_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"p_hdl_uart is NULL");
	}

	if((AUI_UART_OPEN!= p_hdl_uart->ul_status)
		|| ((p_hdl_uart->uc_enable & AUI_UART_READ)!=AUI_UART_READ)
		|| (NULL == pu_buf))
	{
		if(NULL!=pul_readed_len)
			*pul_readed_len = 0;

		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"AUI_UART_OPEN!= p_hdl_uart->ul_status");
	}

	ul_dev_id = p_hdl_uart->ul_dev_id;

	UART_UNLOCK();

	while(ul_readed < ul_read_len)
	{
		if(ul_time_out == 0)
		{
			if((ch = sci_read(ul_dev_id))>0)
			{
				pu_buf[ul_readed] = ch;
				ul_readed++;
			}
		}
		else
		{
			if(SUCCESS == sci_read_tm(ul_dev_id, &ch, timeout_perread))//us
			{
				pu_buf[ul_readed] = ch;
				ul_readed++;
				ul_err_cnt=0;

				ul_timed += timeout_perread;
				if(ul_timed > ul_time_out)
				{
					ul_ret_code = AUI_RTN_FAIL;
					break;
				}
			}
			else
			{
				ul_err_cnt++;
				if(ul_err_cnt > error_time)
				{
					ul_ret_code = AUI_RTN_FAIL;
					break;
				}
			}
		}
	}

	if(NULL!=pul_readed_len)
		*pul_readed_len = ul_readed;

	return ul_ret_code;

}

/**
*	 @brief 	        write data to device with length	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart: the handle for UART
*	 @param[in] 	        pu_buf: the buffer address of data
*	 @param[in] 	        ul_write_len: the length for write, must len then the length of buffer
*	 @param[out]	        pul_written_len:the length for write
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_write(aui_hdl pv_hdl_uart,unsigned char* pu_buf,
						unsigned long ul_write_len,unsigned long* pul_written_len,
						unsigned long ul_time_out)
{
	UART_LOCK();
	(void)ul_time_out;//for remove warning 
	unsigned long send_cnt = 0;
	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;
	unsigned long	ul_dev_id = 0;

	if((NULL == p_hdl_uart) ||(FALSE == is_valid_uart_handle(p_hdl_uart))
		|| (p_hdl_uart->ul_dev_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"p_hdl_uart is NULL");
	}

	if((AUI_UART_OPEN!= p_hdl_uart->ul_status)
		|| ((p_hdl_uart->uc_enable & AUI_UART_WRITE)!=AUI_UART_WRITE)
		|| (NULL == pu_buf))
	{
		if(NULL!= pul_written_len)
			*pul_written_len = 0;
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"AUI_UART_OPEN!= p_hdl_uart->ul_status");
	}

	ul_dev_id = p_hdl_uart->ul_dev_id;

	UART_UNLOCK();

	while(send_cnt < ul_write_len)
	{
		sci_write(ul_dev_id,*(pu_buf+send_cnt));
		send_cnt++;
	}
	if(NULL != pul_written_len)
		*pul_written_len = send_cnt;

	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        clean the data of UART	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart:the handle of UART
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_clear(aui_hdl pv_hdl_uart)
{
	UART_LOCK();

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;
	if((NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart))
		|| (p_hdl_uart->ul_dev_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"(NULL == p_hdl_uart )||(FALSE == is_valid_uart_handle(p_hdl_uart)");
	}

	if(AUI_UART_OPEN!= p_hdl_uart->ul_status)
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"AUI_UART_OPEN!= p_hdl_uart->ul_status");
	}
	sci_clear_buff(p_hdl_uart->ul_dev_id);

	UART_UNLOCK();
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		enable to read & write
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart: the handle of UART
*	 @param[in] 	        pv_hdl_uart the data from aui_uart_io_mode
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_enable(aui_hdl pv_hdl_uart,enum aui_uart_io_mode ul_en_val)
{
	UART_LOCK();

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;
	if((NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart))
		|| (p_hdl_uart->ul_dev_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"(NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart)");
	}

	if((AUI_UART_OPEN!= p_hdl_uart->ul_status) || (ul_en_val > AUI_UART_READ_WRITE))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"(AUI_UART_OPEN!= p_hdl_uart->ul_status || ul_en_val > AUI_UART_READ_WRITE)");
	}

	p_hdl_uart->uc_enable |= ul_en_val;

	UART_UNLOCK();
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        disable to read & write	
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart: the handle of UART
*	 @param[in] 	        pv_hdl_uart: the data from aui_uart_io_mode
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_disable(aui_hdl pv_hdl_uart,enum aui_uart_io_mode ul_en_val)
{
	UART_LOCK();

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;
	if((NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart))
		|| (p_hdl_uart->ul_dev_id >= MAX_UART_ID))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart)");
	}

	if((AUI_UART_OPEN!= p_hdl_uart->ul_status) || (ul_en_val > AUI_UART_READ_WRITE))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"AUI_UART_OPEN!= p_hdl_uart->ul_status || ul_en_val > AUI_UART_READ_WRITE");
	}

	p_hdl_uart->uc_enable &= ~ul_en_val;

	UART_UNLOCK();
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 	        to get the attribute of UART
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart:the handle of UART 
*	 @param[in] 	        ul_item:the item of UART attribute,it from aui_en_uart_item_get
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @note
*
*/
AUI_RTN_CODE aui_uart_get(aui_hdl pv_hdl_uart,
						aui_uart_item_get ul_item,
						void* pv_param)
{
	UART_LOCK();

	AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;

	if((NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart))
		 ||(NULL == pv_param))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"NULL == p_hdl_uart");
	}

	if((p_hdl_uart->ul_dev_id >= MAX_UART_ID)
		||(AUI_UART_OPEN!= p_hdl_uart->ul_status))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"p_hdl_uart->ul_dev_id >= MAX_UART_ID");
	}

	switch(ul_item)
	{
		case AUI_UART_GET_CFGPARAM:
		{
			//if(NULL != pv_param )
			{
				MEMCPY(pv_param,&p_hdl_uart->cfg_uart,sizeof(aui_attr_uart)) ;
				rtn_code = AUI_RTN_SUCCESS;
			}
			break;
		}
		default:
		{
			UART_UNLOCK();
			aui_rtn(AUI_RTN_EINVAL,"ul_item's default");
			break;
		}
	}

	UART_UNLOCK();
	return rtn_code;
}

/**
*	 @brief 		to config the UART
*	 @author		smith.shi
*	 @date			2013-6-18
*	 @param[in] 	        pv_hdl_uart:the handle of UART
*	 @param[in] 	        ul_item:the item of UART,it fromaui_en_uart_item_get
*	 @param[in] 	        pv_param:the data be input
*	 @return		AUI_RTN_SUCCESS for success, wrong number for failed 
*	 @notes
*
*/
AUI_RTN_CODE aui_uart_set(aui_hdl pv_hdl_uart,
						aui_uart_item_set ul_item,
						void* pv_param)
{
	UART_LOCK();

	AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;

	aui_handle_uart *p_hdl_uart = (aui_handle_uart *)pv_hdl_uart;

	if((NULL == p_hdl_uart)||(FALSE == is_valid_uart_handle(p_hdl_uart))
		||(ul_item > AUI_UART_SET_CMD_LAST) ||(NULL == pv_param))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"NULL == p_hdl_uart)");
	}

	if((p_hdl_uart->ul_dev_id >= MAX_UART_ID)
		||(AUI_UART_OPEN!= p_hdl_uart->ul_status))
	{
		UART_UNLOCK();
		aui_rtn(AUI_RTN_EINVAL,"p_hdl_uart->ul_dev_id >= MAX_UART_ID");
	}

	switch(ul_item)
	{
		case AUI_UART_SET_CFGPARAM:
		{
			unsigned long ul_parity = 0;
			//if(NULL != pv_param)
			{
				p_hdl_uart->cfg_uart = *(aui_attr_uart*)pv_param;
				ul_parity = uart_cfg2sci_parity(&p_hdl_uart->cfg_uart);
                sci_mode_set(p_hdl_uart->ul_dev_id, \
					p_hdl_uart->cfg_uart.ul_baudrate, ul_parity);
				rtn_code = AUI_RTN_SUCCESS;
			}
			break;
		}
		default:
		{
			rtn_code = AUI_RTN_FAIL;
			break;
		}
	}

	if(AUI_RTN_SUCCESS != rtn_code)
	{
		UART_UNLOCK();
		return rtn_code;
	}

	UART_UNLOCK();
	return AUI_RTN_SUCCESS;
}

