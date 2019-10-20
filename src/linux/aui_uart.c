/**@file
*	  @brief	 ALi AUI UART function implementation
*	  @date 	 2014-02-24
*	  @version	 1.0.0
*	  @note 	 ali corp. all rights reserved. 2013-2999 copyright (C)
*/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <aui_common.h>
#include <ali_uart_io_common.h>
#include "aui_common_priv.h"
#include <aui_uart.h>

#include <termios.h>

AUI_MODULE(UART)

#define UART_SFU_MODE_DISABLE	(0)
#define UART_SFU_MODE_ENABLE	(1)

static int uartfd = 0;

struct aui_uart {
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	int fd;
};
/* linux-PDK1.11.0-20170405/include/uapi/asm-generic/termbits.h
#define  B0     0000000
#define  B50    0000001
#define  B75    0000002
#define  B110   0000003
#define  B134   0000004
#define  B150   0000005
#define  B200   0000006
#define  B300   0000007
#define  B600   0000010
#define  B1200  0000011
#define  B1800  0000012
#define  B2400  0000013
#define  B4800  0000014
#define  B9600  0000015
#define  B19200 0000016
#define  B38400 0000017
#define    B57600 0010001
#define   B115200 0010002
#define   B230400 0010003
#define   B460800 0010004
#define   B500000 0010005
#define   B576000 0010006
#define   B921600 0010007
#define  B1000000 0010010
#define  B1152000 0010011
#define  B1500000 0010012
#define  B2000000 0010013
#define  B2500000 0010014
#define  B3000000 0010015
#define  B3500000 0010016
#define  B4000000 0010017
*/
int speed_arr[] = {B4000000,B3500000,B3000000,B2500000,B2000000,B1500000,B1152000,B1000000,B921600,B576000,B500000,\
                B460800,B230400,B115200, B57600,B38400, B19200, B9600, B4800, B2400,B1800,B1200, B300,B200,B150,B134,B110,B75,B50,B0};
int name_arr[] = {4000000,3500000,3000000,2500000,2000000,1500000,1152000,1000000,921600,576000,500000,\
                460800,230400,115200, 57600, 38400, 19200, 9600, 4800, 2400, 1800, 1200, 300,200,150,134,110,75,50,0};

/**
*@brief  set the baudrate of uart
*@param  fd    : int type  , the handle of uart device
*@param  speed : int type  , the baudrate
*@return  int  : int type  , 0: success, other:  fail
*/
static int set_speed(int fd, int speed){
	unsigned int   i;
	int   status=-1;
	struct termios   Opt;
	AUI_DBG("the speed_arr index is %d\n",speed);
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) {
		if  (speed == name_arr[i]) {
			status = tcgetattr(fd, &Opt);
			if  (status != 0) {
				AUI_ERR("set_speed tcgetattr fd");
				return -1;
			}
		//	AUI_DBG("the speed_arr index is %d\n",i);
			status |= tcflush(fd, TCIOFLUSH);
			if  (status != 0) {
				AUI_ERR("set_speed tcflush fd1");
				return -1;
			}
			status |=  cfsetispeed(&Opt, speed_arr[i]);
			if  (status != 0) {
				AUI_ERR("set_speed cfsetispeed fd1");
				return -1;
			}
			status |= cfsetospeed(&Opt, speed_arr[i]);
			if  (status != 0) {
				AUI_ERR("set_speed cfsetospeed fd1");
				return -1;
			}
			status |= tcsetattr(fd, TCSANOW, &Opt);
			if  (status != 0) {
				AUI_ERR("set_speed tcsetattr fd2");
				return -1;
			}
		}
	}
	return status;
}



AUI_RTN_CODE aui_uart_version_get(unsigned long *pul_version)
{
	(void) pul_version;
	return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_uart_init(p_fun_cb p_call_back_init,
						   void *pv_param)
{
	return (p_call_back_init) ? p_call_back_init(pv_param) :
		   AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_uart_de_init(p_fun_cb p_call_back_de_init,
							  void *pv_param)
{
	return (p_call_back_de_init) ? p_call_back_de_init(pv_param) :
		   AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_uart_open(unsigned long ul_uartID,
						   aui_attr_uart *p_uart_param,
						   aui_hdl *pp_hdl_uart)
{
	struct aui_uart *uart;
	//(void) p_uart_param;

	uart = (struct aui_uart *)malloc(sizeof(struct aui_uart));
	if (uart == NULL) {
		AUI_DBG("uart malloc fail\n");
		return AUI_RTN_FAIL;
	}
	uart->fd = open("/dev/ali_uart_io", O_RDWR);
	if (uart->fd == -1) {
		AUI_DBG("open /dev/ali_uart_io fail");
		free(uart);
		return AUI_RTN_FAIL;
	}

	uart->data.dev_idx = ul_uartID;
	aui_dev_reg(AUI_MODULE_UART, uart);

	*pp_hdl_uart = uart;

	if( (NULL != p_uart_param)&&(p_uart_param->ul_baudrate !=0) ){
		if(uartfd ==0){
		    uartfd = open("/dev/ttyS0", O_RDWR|O_NOCTTY);
		}
		if (uartfd == -1) {
			AUI_ERR("open /dev/ttyS0 fail");
			free(uart);
			return AUI_RTN_FAIL;
		}
//		AUI_DBG("the uartfd is %d, attr_uart->ul_baudrate is %lu\n",uartfd,p_uart_param->ul_baudrate);
		if(0 != set_speed(uartfd,p_uart_param->ul_baudrate)){
			AUI_ERR("Set Speed Error\n");
			return AUI_RTN_FAIL;
		}
	}

	return AUI_RTN_SUCCESS;

}

AUI_RTN_CODE aui_uart_close(aui_hdl pv_hdl_uart)
{
	struct aui_uart *uart;

	uart = (struct aui_uart *)pv_hdl_uart;
	if (uart == NULL) {
		return AUI_RTN_FAIL;
	}
	if(uartfd !=0)
		close(uartfd);
	uartfd = 0;

	close(uart->fd);
	aui_dev_unreg(AUI_MODULE_UART, uart);
	free(uart);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_uart_read(aui_hdl pv_hdl_uart,
						   unsigned char *pu_buf,
						   unsigned long ul_read_len,
						   unsigned long *pul_readed_len,
						   unsigned long ul_time_out)
{
	struct aui_uart *uart;
	unsigned long i;
	struct uart_pars pars;

	if(NULL == pv_hdl_uart) {
		AUI_ERR("pv_hdl_uart is NULL!\n");
		return AUI_RTN_FAIL;
	}
	uart = (struct aui_uart *)pv_hdl_uart;

	for (i = 0; i < ul_read_len; i++) {
		pars.tm = ul_time_out;
		if (ioctl(uart->fd, ALI_UART_IO_READ_TM, &pars)) {
			AUI_ERR("ioctl ALI_UART_IO_READ_TIMEOUT fail!\n");
			return AUI_RTN_FAIL;
		}

		pu_buf[i] = pars.ch;
	}

//	AUI_DBG("have read i = %lu\n", i);
	if (NULL != pul_readed_len) {
		*pul_readed_len = i;
	} 
	
	return (i == ul_read_len) ? AUI_RTN_SUCCESS : AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_uart_write(aui_hdl pv_hdl_uart,
							unsigned char *pu_buf,
							unsigned long ul_write_len,
							unsigned long *pul_written_len,
							unsigned long ul_time_out)
{
	struct aui_uart *uart;
	unsigned long i;
	(void) ul_time_out;

	if(NULL == pv_hdl_uart) {
		AUI_ERR("pv_hdl_uart is NULL!\n");
		return AUI_RTN_FAIL;
	}
	uart = (struct aui_uart *)pv_hdl_uart;

	for (i = 0; i < ul_write_len; i++) {
		if (ioctl(uart->fd, ALI_UART_IO_WRITE, &pu_buf[i])) {
			AUI_ERR("ioctl ALI_UART_IO_WRITE fail!\n");
			return AUI_RTN_FAIL;
		}
	}

	if (NULL != pul_written_len) {
		*pul_written_len = i;
	} 
	
	return (i == ul_write_len) ? AUI_RTN_SUCCESS : AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_uart_clear(aui_hdl pv_hdl_uart)
{
	(void) pv_hdl_uart;
	return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_uart_enable(aui_hdl pv_hdl_uart,
							 enum aui_uart_io_mode ul_en_val)
{
	struct aui_uart *uart;
	int ret;

	uart = (struct aui_uart *)pv_hdl_uart;
	if (uart == NULL) {
		return AUI_RTN_FAIL;
	}

	if (AUI_UART_READ_WRITE != ul_en_val) {
		AUI_ERR("aui_uart_enable  ul_en_val is not AUI_UART_READ_WRITE\n");
		return AUI_RTN_EINVAL;
	}

	ret = ioctl(uart->fd, ALI_UART_IO_SET_SCI, NULL);

	return ((ret == 0) ? AUI_RTN_SUCCESS : AUI_RTN_FAIL);
}

AUI_RTN_CODE aui_uart_disable(aui_hdl pv_hdl_uart,
							  enum aui_uart_io_mode ul_en_val)
{
	struct aui_uart *uart;
	int ret;

	uart = (struct aui_uart *)pv_hdl_uart;
	if (uart == NULL) {
		return AUI_RTN_FAIL;
	}

	if (AUI_UART_READ_WRITE != ul_en_val) {
		AUI_ERR("aui_uart_disable  ul_en_val is not AUI_UART_READ_WRITE\n");
		return AUI_RTN_EINVAL;
	}

	ret = ioctl(uart->fd, ALI_UART_IO_CLEAR_SCI, NULL);

	return ((ret == 0) ? AUI_RTN_SUCCESS : AUI_RTN_FAIL);
}

AUI_RTN_CODE aui_uart_get(aui_hdl pv_hdl_uart,
						  aui_uart_item_get ul_item,
						  void* pv_param)
{
	(void) pv_hdl_uart;
	(void) ul_item;
	(void) pv_param;
	return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_uart_set(aui_hdl pv_hdl_uart,
						  aui_uart_item_set	ul_item,
						  void* pv_param)
{
	(void) pv_hdl_uart;
	(void) ul_item;
	(void) pv_param;
	return AUI_RTN_FAIL;
}
