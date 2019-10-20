/**@file
 *    @brief     ALi AUI I2C function implementation
 *    @author    Alfa.Shang
 *    @date      2016-10-25
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2016-2999 copyright (C)
 */


/* linux lib head files */
#include "aui_common.h"
#include "aui_common_priv.h"
#include <aui_errno_sys.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

/* aui head files */
#include <aui_i2c.h>

/* aliplatform lib head files */
#include <alisli2c.h>

AUI_MODULE(I2C)

typedef struct i2c_device{
	aui_dev_priv_data       dev_priv_data;   //used to store device information           
    alisl_handle            handle;          //used to point to the opened I2C bus device            
	alisli2c_attr           attr;            //used to configure the I2C bus device
}i2c_device;

AUI_RTN_CODE aui_i2c_open(aui_attr_i2c *p_attr, aui_hdl *p_i2c_handle)
{
	//Error detection
	if ((NULL == p_attr) || (NULL == p_i2c_handle)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	alisli2c_attr sl_attr;
	sl_attr.uc_dev_idx = p_attr->uc_dev_idx;	
	struct i2c_device *dev = NULL;

	//Malloc dev
	dev = malloc(sizeof(i2c_device));
	if (NULL == dev) {
		aui_rtn(AUI_RTN_ENOMEM, "fail to malloc memory!\n");
	}
	
	//init dev
	memset(dev, 0, sizeof(i2c_device));
	memcpy(&dev->attr, &sl_attr, sizeof(alisli2c_attr));
	
    //open i2c bus device
	if (alisli2c_open(&sl_attr, &dev->handle)) {
		free(dev);
		aui_rtn(AUI_RTN_FAIL, "open failed!\n");
	}

 	*p_i2c_handle = dev;
	dev->dev_priv_data.dev_idx = sl_attr.uc_dev_idx;
	aui_dev_reg(AUI_MODULE_I2C, *p_i2c_handle);
  
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_write(aui_hdl i2c_handle, unsigned long chip_addr,unsigned long sub_addr,unsigned char *buff, unsigned long buf_len)
{	
	//Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}
	
	int ret = 0;
	unsigned int subaddr_flag = 1;
	struct i2c_device *dev = (i2c_device *)i2c_handle;

	//Write data
	ret = alisli2c_write_data(dev->handle, chip_addr, sub_addr, buff, buf_len, subaddr_flag);
	if (ret < 0){
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}
	
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_write_no_subaddr(aui_hdl i2c_handle, unsigned long chip_addr, unsigned char *buff, unsigned long buf_len)
{
       //Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	unsigned int subaddr_flag = 0;
	int ret = 0;
	struct i2c_device *dev = (i2c_device *)i2c_handle;
	unsigned long sub_addr = 0xFF;
	//Write data

	ret = alisli2c_write_data(dev->handle, chip_addr, sub_addr, buff, buf_len, subaddr_flag);
	if (ret < 0){
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}
	
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_i2c_read(aui_hdl i2c_handle, unsigned long chip_addr,unsigned long sub_addr,unsigned char *buff, unsigned long buf_len)
{
      //Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	unsigned int subaddr_flag = 1;
	int ret = 0;
	struct i2c_device *dev = (i2c_device *)i2c_handle;

	//Read data
	ret = alisli2c_read_data(dev->handle, chip_addr, sub_addr, buff, buf_len, subaddr_flag);
	if (ret < 0){
		aui_rtn(AUI_RTN_FAIL, "fail to read data!\n");
	}
	
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_read_no_subaddr(aui_hdl i2c_handle, unsigned long chip_addr, unsigned char *buff, unsigned long buf_len)
{
       //Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

       unsigned int subaddr_flag = 0;
	int ret = 0;
	struct i2c_device *dev = (i2c_device *)i2c_handle;
       unsigned long sub_addr = 0xFF;
	
	//Read data
	ret = alisli2c_read_data(dev->handle, chip_addr, sub_addr, buff, buf_len, subaddr_flag);
	if (ret < 0){
		aui_rtn(AUI_RTN_FAIL, "fail to read data!\n");
	}
	
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_write_read(aui_hdl i2c_handle, unsigned long chip_addr,unsigned long sub_addr,
	                                                    unsigned char *write_buff, unsigned long write_buf_len, unsigned char *read_buff,
	                                                    unsigned long read_buf_len)
{
    //Error detection
	if ((NULL == i2c_handle) || (NULL == write_buff) || (MAX_DATA_LENGTH < write_buf_len) || (NULL == read_buff)
		|| (MAX_DATA_LENGTH < read_buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	unsigned int subaddr_flag = 1;
	int ret = 0;
	struct i2c_device *dev = (i2c_device *)i2c_handle;

	//Write read data
	ret = alisli2c_write_read(dev->handle, chip_addr, sub_addr, write_buff, write_buf_len, read_buff, read_buf_len, subaddr_flag);
	if (ret < 0){
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}
	
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_write_read_no_subaddr(aui_hdl i2c_handle, unsigned long chip_addr,
	                                                    unsigned char *write_buff, unsigned long write_buf_len, unsigned char *read_buff,
	                                                    unsigned long read_buf_len)
{
       //Error detection
	if ((NULL == i2c_handle) || (NULL == write_buff) || (MAX_DATA_LENGTH < write_buf_len) || (NULL == read_buff) ||
		(MAX_DATA_LENGTH < read_buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	unsigned int subaddr_flag = 0;
	int ret = 0;
	struct i2c_device *dev = (i2c_device *)i2c_handle;
       unsigned long sub_addr = 0xFF;

	//Write read data
	ret = alisli2c_write_read(dev->handle, chip_addr, sub_addr, write_buff, write_buf_len, read_buff, read_buf_len, subaddr_flag);
	if (ret < 0){
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}
	
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_close (aui_hdl i2c_handle)
{
	//Error detection
	if (NULL == i2c_handle) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	//Close i2c bus device
	i2c_device *dev = (i2c_device *)i2c_handle;
	if (alisli2c_close(dev->handle)) {
		aui_rtn(AUI_RTN_FAIL, "close failed!\n");
	}

	if (aui_dev_unreg(AUI_MODULE_I2C, i2c_handle)) {
		aui_rtn(AUI_RTN_FAIL, "aui_dev_unreg AUI_MODULE_I2C fail\n");
	}

	free(dev);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_init (p_fun_cb p_init_callback, void* user_data)
{
	if (p_init_callback) {
        return p_init_callback(user_data);
    }
	
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_deinit (p_fun_cb p_deinit_callback)
{
	    
	if (p_deinit_callback) {
        return p_deinit_callback(NULL);
    }
    return AUI_RTN_SUCCESS;
	
}

    

