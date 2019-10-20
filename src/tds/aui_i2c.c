/**@file
 *    @brief     ALi AUI I2C function implementation
 *    @author   Alfa.Shang
 *    @date      2016-10-28
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2016-2999 copyright (C)
 */


#include "aui_common.h"
#include "aui_common_priv.h"
#include <aui_errno_sys.h>
#include <string.h>
#include <stdlib.h>
#include <bus/i2c/i2c.h>
 
 /* aui head files */
#include <aui_i2c.h>

AUI_MODULE(I2C)

//According to the current specifications, this value should be no
//more than 16 (bytes). The latest kernel version doesn't have this limit.                                        
#define MAX_DATA_LENGTH         16

typedef struct aui_i2c_device{
	aui_dev_priv_data       dev_priv_data;   //used to store device information              
	aui_attr_i2c                attr;            //used to configure the I2C bus device
}aui_i2c_device;

/*
	The function is used to translate AUI_I2C ID to TDS driver's GPIO ID.
*     param[in]	The user input id that the user want to write or read.

*	param[out]	The actual operated gpio i2c devce id of driver layer.  
*/
static unsigned long get_gpio_i2c_id(unsigned long id)
{
	unsigned long idx = 0;
	if (4 == id) {
		idx = id & I2C_TYPE_GPIO0;
	}
	else if (5 == id) {
		idx = id & I2C_TYPE_GPIO1;
	}

	return idx;
}

AUI_RTN_CODE aui_i2c_open(aui_attr_i2c *p_attr, aui_hdl *p_i2c_handle)
{
	//Error detection
	if ((NULL == p_attr) || (NULL == p_i2c_handle)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	struct aui_i2c_device *dev = NULL;

	//Malloc dev
	dev = (struct aui_i2c_device *)malloc(sizeof(struct aui_i2c_device));
	if (NULL == dev) {
		aui_rtn(AUI_RTN_ENOMEM, "fail to malloc memory!\n");
	}
	
	//init dev
	memset(dev, 0, sizeof(aui_i2c_device));
	memcpy(&dev->attr, p_attr, sizeof(aui_attr_i2c));

	//register the opened i2c device in system.
 	*p_i2c_handle = dev;
	dev->dev_priv_data.dev_idx = p_attr->uc_dev_idx;
	aui_dev_reg(AUI_MODULE_I2C, *p_i2c_handle);
  
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_write(aui_hdl i2c_handle, unsigned long chip_addr,unsigned long sub_addr,unsigned char *buff, unsigned long buf_len)
{	
	//Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}
	
	int i, ret = 0;
	struct aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;
	unsigned long idx = (unsigned long) dev->attr.uc_dev_idx;
	
	//Write data
	unsigned char *send_data = NULL;
       unsigned int data_len = 0;

    //if the slave device has some data register, copy sub_addr to send_data[0],
    //and copy the data in the buf to send_data[i](i>0).
	send_data = (unsigned char *)malloc((buf_len+1) * sizeof(unsigned char));
	if (NULL == send_data) {
            	AUI_ERR("fail to malloc the memory for send_data!\n");
		return AUI_RTN_FAIL;
	}  
	
       memset(send_data, 0, buf_len+1);
	send_data[0] = (unsigned char)sub_addr;
	for(i=1;i<buf_len+1;i++){
             send_data[i] = buff[i-1];
	 }
	 
	 data_len = buf_len +1;	

	//if the user input id is less than 4, call the function for hardware i2c device to write or read
	//if the user input id is more than 4, call the function for gpio i2c device to write or read
	if (idx < 4) {
	//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
	//by user is low 7bit, so shifting 1 bit to the left here to get right address for writing or reading
	//to or from the chip.
		ret =  i2c_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	else {
		idx = get_gpio_i2c_id(idx);
		ret = i2c_gpio_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	
	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}

	free(send_data);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_write_no_subaddr(aui_hdl i2c_handle, unsigned long chip_addr, unsigned char *buff, unsigned long buf_len)
{
       //Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	int ret = 0;
	struct aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;	
	unsigned long idx = (unsigned long) dev->attr.uc_dev_idx;

	//Write data
	unsigned char *send_data = NULL;
       unsigned int data_len = 0;

      //if the slave device has no data register, copy data in @buf to @send_data
      //and the @data_len is @sizeof(data_len).
        send_data = (unsigned char *)malloc(buf_len * sizeof(unsigned char));	 
	 if (NULL == send_data) {
            	AUI_ERR("fail to malloc the memory for send_data!\n");
		return AUI_RTN_FAIL;
	 }
	 
        memset(send_data, 0, buf_len);
	 memcpy(send_data, buff, buf_len);
	 data_len = buf_len;

	//if the user input id is less than 4, call the function for hardware i2c device to write or read
	//if the user input id is more than 4, call the function for gpio i2c device to write or read	
	if (idx < 4) {
		//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
		//by user is low 7bit, so shifting 1 bit to the left here.
		ret =  i2c_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	else {
		idx = get_gpio_i2c_id(idx);
		ret = i2c_gpio_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	
	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}

	free(send_data);
	return AUI_RTN_SUCCESS;

}


AUI_RTN_CODE aui_i2c_read(aui_hdl i2c_handle, unsigned long chip_addr,unsigned long sub_addr,unsigned char *buff, unsigned long buf_len)
{
      //Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	int ret = 0;
	struct aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;	
	unsigned long idx = (unsigned long) dev->attr.uc_dev_idx;
	
	//Read data
	unsigned char *send_data = NULL;
       unsigned int data_len = 0;

    //if the slave device has some data register,  copy sub_addr to send_data[0],
    //and copy the data in the buf to send_data[i](i>0).
	send_data = (unsigned char *)malloc(buf_len * sizeof(unsigned char));
	if (NULL == send_data) {
            	AUI_ERR("fail to malloc the memory for send_data!\n");
		return AUI_RTN_FAIL;
	      }
	  
        memset(send_data, 0, buf_len);
	 send_data[0] = (unsigned char)sub_addr;	 
	 data_len = 1;

	//if the user input id is less than 4, call the function for hardware i2c device to write or read
	//if the user input id is more than 4, call the function for gpio i2c device to write or read
	if (idx < 4) {
		//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
		//by user is low 7bit, so shifting 1 bit to the left here.
		ret = i2c_write_read(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len, (int)buf_len);
	}
	else {		
		idx = get_gpio_i2c_id(idx);
		ret = i2c_gpio_write_read(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len, (int)buf_len);
	}

	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to read data!\n");
	}

	memcpy(buff, send_data, buf_len);

	free(send_data);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_i2c_read_no_subaddr(aui_hdl i2c_handle, unsigned long chip_addr, unsigned char *buff, unsigned long buf_len)
{
	//Error detection
	if ((NULL == i2c_handle) || (NULL == buff) || (MAX_DATA_LENGTH < buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	   }

	int ret = 0;
	struct aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;  
	unsigned long idx = (unsigned long) dev->attr.uc_dev_idx; 
	
	//Read data
	unsigned char *read_data = NULL;
	unsigned int data_len = 0;
	

    //if the slave device has no data register, copy data in @buf to @send_data
    //and the @data_len is @sizeof(data_len).
	read_data = (unsigned char *)malloc(buf_len * sizeof(unsigned char));	
	if (NULL == read_data) {
		AUI_ERR("fail to malloc the memory for send_data!\n");
		return AUI_RTN_FAIL;
	}
		
	memset(read_data, 0, buf_len);
	data_len = 0;
	
	
	//if the user input id is less than 4, call the function for hardware i2c device to write or read
	//if the user input id is more than 4, call the function for gpio i2c device to write or read	
	if (idx < 4) {
		//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
		//by user is low 7bit, so shifting 1 bit to the left here.
		ret = i2c_write_read(idx, (unsigned char)chip_addr << 1, read_data, (int)data_len, (int)buf_len);
	}
	else {		
		idx = get_gpio_i2c_id(idx);
		ret = i2c_gpio_write_read(idx, (unsigned char)chip_addr << 1, read_data, (int)data_len, (int)buf_len);
	}


	   if (ret < 0){
		   free(read_data);
		   aui_rtn(AUI_RTN_FAIL, "fail to read data!\n");
	   }
	
	   memcpy(buff, read_data, buf_len);
	
	   free(read_data);
	   return AUI_RTN_SUCCESS;

}

AUI_RTN_CODE aui_i2c_write_read(aui_hdl i2c_handle, unsigned long chip_addr,unsigned long sub_addr,
	                                                    unsigned char *write_buff, unsigned long write_buf_len, unsigned char *read_buff,
	                                                    unsigned long read_buf_len)
{
       //Error detection
	if ((NULL == i2c_handle) || (NULL == write_buff) || (MAX_DATA_LENGTH < write_buf_len) || (NULL == read_buff) ||
		(MAX_DATA_LENGTH < read_buf_len)) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	int i, ret = 0;
	struct aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;	
	unsigned long idx = (unsigned long)dev->attr.uc_dev_idx;

	//Write data
	unsigned char *send_data = NULL;
       unsigned int data_len = 0;
	unsigned int wr_len = 0;

    //if the slave device has some data register, copy sub_addr to send_data[0],
    //and copy the data in the buf to send_data[i](i>0).
	send_data = (unsigned char *)malloc((write_buf_len+1) * sizeof(unsigned char));
	if (NULL == send_data) {
		AUI_ERR("fail to malloc the memory for send_data!\n");
		return AUI_RTN_FAIL;
	 	}
	  
       memset(send_data, 0, write_buf_len+1);
	send_data[0] = (unsigned char)sub_addr;
	for(i = 1; i < write_buf_len + 1; i++){
            	send_data[i] = write_buff[i-1];
	 	}
	 
	data_len = write_buf_len +1;
	wr_len = 1;
	


	//write data to the data register
	//if the user input id is less than 4, call the function for hardware i2c device to write or read
	//if the user input id is more than 4, call the function for gpio i2c device to write or read
	if (idx < 4) {
		//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
		//by user is low 7bit, so shifting 1 bit to the left here.
		ret =  i2c_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	else {
		idx = get_gpio_i2c_id(idx);
		ret =  i2c_gpio_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	
	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}

	//write sub_addr to the salve device and read data from the data register
	if (idx < 4) {
		//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
		//by user is low 7bit, so shifting 1 bit to the left here.
		ret = i2c_write_read(idx, (unsigned char)chip_addr << 1, send_data, (int)wr_len, (int)read_buf_len);
	}
	else {
		idx = get_gpio_i2c_id(idx);
		ret = i2c_gpio_write_read(idx, (unsigned char)chip_addr << 1, send_data, (int)wr_len, (int)read_buf_len);
	}
	
	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}

	memcpy(read_buff, send_data, read_buf_len);
	
	free(send_data);
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

	int ret = 0;
	struct aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;
	unsigned long idx = (unsigned long)dev->attr.uc_dev_idx;

	//Write data
	unsigned char *send_data = NULL;
       unsigned int data_len = 0;
	unsigned int wr_len = 0;

    //if the slave device has no data register, copy data in @buf to @send_data
    //and the @write_data_len is @sizeof(data_len).
      send_data = (unsigned char *)malloc(write_buf_len * sizeof(unsigned char));
	 
	if (NULL == send_data) {
            	AUI_ERR("fail to malloc the memory for send_data!\n");
		return AUI_RTN_FAIL;
	 	}
	 
       memset(send_data, 0, write_buf_len);
	memcpy(send_data, write_buff, write_buf_len);
	data_len = write_buf_len;
	wr_len = 0;
       

	//write data to the data register
	//if the user input id is less than 4, call the function for hardware i2c device to write or read
	//if the user input id is more than 4, call the function for gpio i2c device to write or read
	if (idx < 4) {
		//The tds system uses 8bit in a byte to store the address of chip, but the chip_addr input
		//by user is low 7bit, so shifting 1 bit to the left here.
		ret =  i2c_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	else {
		idx = get_gpio_i2c_id(idx);
		ret =  i2c_gpio_write(idx, (unsigned char)chip_addr << 1, send_data, (int)data_len);
	}
	
	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}

	//write sub_addr to the salve device and read data from the data register
	if (idx < 4) {
		ret = i2c_write_read(idx, (unsigned char)chip_addr << 1, send_data, (int)wr_len, (int)read_buf_len);
	}
	else {
		idx = get_gpio_i2c_id(idx);
		ret = i2c_gpio_write_read(idx, (unsigned char)chip_addr << 1, send_data, (int)wr_len, (int)read_buf_len);
	}

	if (ret < 0){
		free(send_data);
		aui_rtn(AUI_RTN_FAIL, "fail to write data!\n");
	}

	memcpy(read_buff, send_data, read_buf_len);
	
	free(send_data);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_i2c_close (aui_hdl i2c_handle)
{
	//Error detection
	if (NULL == i2c_handle) {
		aui_rtn(AUI_RTN_EINVAL, "fail to get the right input data!\n");
	}

	//Close i2c bus device
	aui_i2c_device *dev = (aui_i2c_device *)i2c_handle;

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

