/**@file
 *	(c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *	All rights reserved
 *
 *	@file				aui_vbi.c
 *	@brief
 *
 *	@version			2.0
 *	@date				02/28/2014 10:04:22 AM
 *	@modify		      niker.li @ 20161015
 *	@revision			none
 *
 *	@author 			Summer Xia <summer.xia@alitech.com>
 */

#include "aui_common_priv.h"
#include <aui_vbi.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include "alipltfretcode.h"
#include "alislvbi.h"
#include "alisldmx.h"
#include "alisldis.h"

#include <pthread.h>
#include <aui_dmx.h>

AUI_MODULE(VBI)

#define VBI_WRITE_STOP        (0)
#define VBI_WRITE_RUN         (1)
#define TTX_PES_HEADER_LENGTH (45)
#define VBI_BUFFER_SIZE  (24 * 1024)

typedef struct vbi_filter_token {
	unsigned int channelid;         
    aui_pes_data_callback callback;  //the callback function to get ttx data
	void* callback_param;  //pointer to the parameter to be passed to a callback function
    unsigned char     *buffer;
    unsigned long     buffer_length;  
}vbi_filter_token;

typedef struct aui_handle_vbi {
    aui_dev_priv_data data; 		  /* struct for dev reg/unreg */
    alisl_handle slhdl;          /* share lib device handle */	
	alisl_handle dmx_hdl;
	vbi_filter_token vbi_token;
    int thread_flag;				  /* ttx thread flag */
	pthread_t vbi_thread;
	pthread_mutex_t mutex;  
    pthread_cond_t  cond; 
	aui_ring_buf vbi_rbuf; /* buf for vbi data */
	unsigned int ttx_pid; 
} aui_handle_vbi;

typedef enum cc_type_state {
    CC_PREANY = 0,
	CC_ANY, //for internal processing, user cannot call this.
	CC_GA94,
	CC_SCTE20,
	/*the below types are not supported*/
	CC_SCTE21,
	CC_DVD,
	CC_REPLAYTV,
	CC_OTHER,
}cc_type_state_t;

static void *aui_vbi_write_thread(void *p_param)
{
	aui_handle_vbi  *dev_hdl  = NULL;
	aui_vbi_ttx_line line_data;
	unsigned long read_len = 0;
	unsigned long data_len = 0;
	
	if (NULL == p_param) {
		AUI_ERR("aui_vbi_write_thread Fail!\n");
		goto vbi_thread_error;
	}
	
	dev_hdl = (aui_handle_vbi *)p_param;
	while(VBI_WRITE_RUN == dev_hdl->thread_flag) { 
		aui_common_ring_buf_data_len(&(dev_hdl->vbi_rbuf), &data_len);
		if (data_len == 0) {
			//Thread sleeps when no data comes 
			pthread_mutex_lock(&(dev_hdl->mutex));
			pthread_cond_wait(&(dev_hdl->cond), &(dev_hdl->mutex));
			pthread_mutex_unlock(&(dev_hdl->mutex));
		}
		//Send a line data
		aui_common_ring_buf_rd(&(dev_hdl->vbi_rbuf),VBI_TTX_LINE_LENGTH,&read_len,(unsigned char *)&line_data);
		if(0 == read_len) {
			AUI_INFO("No data is not written.\n");
			continue;
		}
		if (alislvbi_write(dev_hdl->slhdl, (char *)&line_data, VBI_TTX_LINE_LENGTH)) {
		    AUI_ERR("\n vbi write line fail! \n");
		}
	}
vbi_thread_error:
	pthread_exit(NULL); 
}

static unsigned long aui_vbi_thread_create(aui_hdl vbi_handle)
{
	AUI_DBG("\n\naui_vbi_thread_create\n\n");

	aui_handle_vbi  *dev_hdl  = NULL;
	dev_hdl  = (aui_handle_vbi *)(vbi_handle);

    if(pthread_mutex_init(&(dev_hdl->mutex), NULL) != 0) {
		AUI_ERR("pthread_mutex_init fail!\n");
		return AUI_RTN_FAIL;	
	} 
	if(pthread_cond_init(&(dev_hdl->cond), NULL) != 0) {
		AUI_ERR("pthread_cond_init fail!\n");
		return AUI_RTN_FAIL;	
	} 
	if(pthread_create(&(dev_hdl->vbi_thread), NULL, aui_vbi_write_thread, (void *)dev_hdl)	 
			!= 0) {
		AUI_ERR("aui_vbi_thread_create fail!\n");
		dev_hdl->thread_flag = VBI_WRITE_STOP;
		return AUI_RTN_FAIL;	
	} 
	
	return AUI_RTN_SUCCESS;
}

static long int aui_vbi_data_callback(void *p_user_hdl,unsigned char* pbuf,unsigned long ul_size,void* usrdata)
{
    unsigned char data_identifier;
    unsigned short i = 0,line_sum;
	unsigned long data_len = 0;
	aui_vbi_ttx_line line_data;
	aui_handle_vbi  *dev_hdl  = NULL;

	if (NULL == usrdata) {
		AUI_ERR("\aui_vbi_data_callback Fail!\n");
		return AUI_RTN_FAIL;
	}
	dev_hdl = (aui_handle_vbi *)usrdata;
    //Whether it is EBU Teletext Data
    data_identifier = pbuf[TTX_PES_HEADER_LENGTH];
    if((data_identifier < 0x10) ||(data_identifier > 0x1f)) {
		AUI_ERR("\n Incorrect data type !\n"); 
		return AUI_RTN_FAIL;
    }
    ////A PES package contains lines number of Teletext Data
	line_sum = (ul_size - TTX_PES_HEADER_LENGTH - 1)/VBI_TTX_LINE_LENGTH;
    for(i = 0;i < line_sum;i++) {	
 		if(VBI_WRITE_STOP == dev_hdl->thread_flag) { 
			AUI_INFO("\n VBI write done!\n"); 
			return AUI_RTN_SUCCESS;
		}
		//TTX PES Header length is 45 bytes, data_identifier is 1 byte, a line of data is 46 bytes
		MEMCPY(&line_data, (unsigned char*)&pbuf[TTX_PES_HEADER_LENGTH + 1 + i * VBI_TTX_LINE_LENGTH], VBI_TTX_LINE_LENGTH);
		if(aui_common_ring_buf_wt(&(dev_hdl->vbi_rbuf),VBI_TTX_LINE_LENGTH,(unsigned char *)&line_data)) {
            AUI_ERR("ring buf write fail.\n");
            return AUI_RTN_FAIL;
        }  
		aui_common_ring_buf_data_len(&(dev_hdl->vbi_rbuf), &data_len);
        //Wake up the thread when the data comes. 
        if (data_len > 0) {
			pthread_mutex_lock(&(dev_hdl->mutex));
			pthread_cond_signal(&(dev_hdl->cond)); 
			pthread_mutex_unlock(&(dev_hdl->mutex));
		} 
	}

	return AUI_RTN_SUCCESS;
	/* unused */
	(void)p_user_hdl;
}

static void vbi_requestbuf_callback(void *priv,
								unsigned long channelid,
								unsigned long filterid,
								unsigned long length,
								unsigned char  **buffer,
								unsigned long *actlen)
{
    vbi_filter_token *filter = priv;
    if(!filter) {
		AUI_DBG("filter_token can`t be null\n");
        return;
    }
	if (!filter->buffer || filter->buffer_length < length) {
		if (!filter->buffer)
			filter->buffer = (unsigned char *) malloc(length);
		else
			filter->buffer = (unsigned char *) realloc(filter->buffer, length);
	
		if (!filter->buffer) {
		    AUI_DBG("error filter token id  malloc failed for pes buffer\n");
		    return;
		}
		    filter->buffer_length = length;
	}
	
	*buffer = filter->buffer;
	*actlen = length;
	return ;
	/* unused */
	(void)channelid;
	(void)filterid;
}

static void vbi_updatebuf_callback(void *priv,
							   unsigned long channelid,
							   unsigned long filterid,
							   unsigned long valid_len)
{
    vbi_filter_token *filter = priv;
    if(!filter) {
		AUI_DBG("filter_token can`t be null\n");
        return;
    }

	if (NULL == filter->callback) {
	    AUI_DBG(" warning filter token id pes record cb is not defined\n");
	    return;
	}
	filter->callback(NULL, filter->buffer, valid_len, filter->callback_param);
	return ;
	/* unused */
	(void)channelid;
	(void)filterid;
}

AUI_RTN_CODE aui_vbi_init(p_fun_cb p_callback_init, void *pv_param)
{
	if (p_callback_init) {
		p_callback_init(pv_param);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_de_init(p_fun_cb p_callback_init, void *pv_param)
{
	if (p_callback_init) {
		p_callback_init(pv_param);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_open(aui_vbi_open_param *open_param,aui_hdl *vbi_handle)
{ 
	alisl_retcode	sl_ret;
	aui_handle_vbi *dev_hdl = NULL;

	if (( NULL == open_param) || ( NULL == vbi_handle)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL!");
	}
	*vbi_handle = NULL;
	 
	/* check vbi if already opened */
	if (!aui_find_dev_by_idx(AUI_MODULE_VBI, open_param->vbi_index, (void *)&dev_hdl)) {
		*vbi_handle = (void *)dev_hdl;
		return AUI_RTN_SUCCESS;
	}

	dev_hdl = (aui_handle_vbi *)malloc(sizeof(aui_handle_vbi));
	if(NULL == dev_hdl) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL!");
	}  
	memset(dev_hdl, 0, sizeof(aui_handle_vbi));
	 
	sl_ret = alislvbi_open(&(dev_hdl->slhdl));
	if (DIS_ERR_NONE != sl_ret) {
		FREE(dev_hdl);
		aui_rtn(AUI_RTN_FAIL, "\n vbi open line fail! \n");
	}
	dev_hdl->data.dev_idx = open_param->vbi_index;
	if (aui_dev_reg(AUI_MODULE_VBI, dev_hdl)) {
		FREE(dev_hdl);
		aui_rtn(AUI_RTN_EINVAL, "aui_dev_reg AUI_MODULE_VBI fail\n");
	}	
	*vbi_handle = dev_hdl;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_close(aui_hdl vbi_handle)
{
	aui_handle_vbi *dev_hdl  = NULL;

	if (!vbi_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL!");
	}
	dev_hdl  = (aui_handle_vbi *)(vbi_handle);
	if (DIS_ERR_NONE != alislvbi_close(dev_hdl->slhdl)) {
		aui_rtn(AUI_RTN_FAIL, "\n vbi close fail! \n");
	}
	if (aui_dev_unreg(AUI_MODULE_VBI, dev_hdl)) {
		aui_rtn(AUI_RTN_EINVAL,"aui_dev_unreg fail!");
	}
	dev_hdl->data.dev_idx = -1;
	free(vbi_handle);
	vbi_handle = NULL;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_ttx_write_line(aui_hdl vbi_handle, aui_vbi_ttx_line *ttx_data)
{
	alisl_retcode	sl_ret;
	aui_handle_vbi  *dev_hdl  = NULL;
	if (NULL == vbi_handle || NULL == ttx_data) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	dev_hdl  = (aui_handle_vbi *)(vbi_handle);
	sl_ret = alislvbi_write(dev_hdl->slhdl, (char *)ttx_data, VBI_TTX_LINE_LENGTH);
	if (DIS_ERR_NONE != sl_ret) {
		aui_rtn(AUI_RTN_FAIL, "\n vbi write line fail! \n");
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_ttx_start(aui_hdl vbi_handle, aui_ttx_start_param *start_param)
{	
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_handle_vbi	*dev_hdl  = NULL;
	struct dmx_channel_callback cb;
	struct dmx_channel_attr attr;

	if (( NULL == vbi_handle) || ( NULL == start_param)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL!");
	}
	
	dev_hdl = (aui_handle_vbi *)(vbi_handle);
	ret = alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_CHECK_TTX_TASK_START, TRUE);
	
	if (0 != ret) {
		aui_rtn(AUI_RTN_FAIL, "\n vbi teletext start fail! \n");
	}
	
	dev_hdl->ttx_pid = start_param->ttx_pid;
	
	//ttx_pid valid ,get ttx data from dmx
	if ((dev_hdl->ttx_pid > 0) && (dev_hdl->ttx_pid < AUI_INVALID_PID) ) {
		dev_hdl->thread_flag = VBI_WRITE_STOP;
		AUI_DBG("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
		
		//Initialize the ring buffer
		if(AUI_RTN_SUCCESS!=aui_common_init_ring_buf(
			VBI_BUFFER_SIZE,&(dev_hdl->vbi_rbuf))) {
			AUI_ERR("aui_common_init_ring_buf fail!\n");
			return AUI_RTN_FAIL;
		}
		
		aui_common_rst_ring_buf(&(dev_hdl->vbi_rbuf));
		
		//Start the dmx device
		if (alisldmx_open(&(dev_hdl->dmx_hdl), start_param->dmx_index, 0)) {
			aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
			aui_rtn(AUI_RTN_EINVAL,"alisldmx_open fail!");
		}
		
		if (alisldmx_start(dev_hdl->dmx_hdl)) {
			aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
			aui_rtn(AUI_RTN_EINVAL,"alisldmx_start fail!");
		}
		
		//Allocate a channel with specific channel type
		if (alisldmx_allocate_channel(dev_hdl->dmx_hdl, DMX_CHANNEL_RECORD, &(dev_hdl->vbi_token.channelid))) {
			aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
			aui_rtn(AUI_RTN_EINVAL,"alisldmx_allocate_channel fail!");
		}
		
		//Set channel attribute
		memset(&attr, 0, sizeof(attr));
		attr.output_format = DMX_OUTPUT_FORMAT_PES;
		
		if (alisldmx_set_channel_attr(dev_hdl->dmx_hdl, dev_hdl->vbi_token.channelid, &attr)) {
			aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
			aui_rtn(AUI_RTN_EINVAL,"alisldmx_set_channel_attr fail!");
		}
		
		//Set pid to a channel
		if (alisldmx_set_channel_pid(dev_hdl->dmx_hdl, dev_hdl->vbi_token.channelid, start_param->ttx_pid)) {
			aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
			aui_rtn(AUI_RTN_EINVAL,"alisldmx_set_channel_pid fail!");
		}
		
		//Register callback functions to a channel
		memset(&cb, 0, sizeof(struct dmx_channel_callback));
	 	cb.request_buffer = (alisldmx_channel_requestbuf_callback)vbi_requestbuf_callback;
	 	cb.update_buffer = (alisldmx_channel_updatebuf_callback)vbi_updatebuf_callback;
		dev_hdl->vbi_token.callback = aui_vbi_data_callback;
		dev_hdl->vbi_token.callback_param = dev_hdl;
		cb.priv = &(dev_hdl->vbi_token);
		alisldmx_register_channel_callback(dev_hdl->dmx_hdl, dev_hdl->vbi_token.channelid, &cb);
		
	    //Enable a channel
		if (alisldmx_control_channel(dev_hdl->dmx_hdl, dev_hdl->vbi_token.channelid, DMX_CTRL_ENABLE)) {
			aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
			aui_rtn(AUI_RTN_EINVAL,"alisldmx_control_channel fail!");
		}
		
	    dev_hdl->thread_flag = VBI_WRITE_RUN;
	    //Create task or thread to write data 
	    aui_vbi_thread_create(dev_hdl);	
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_ttx_stop(aui_hdl vbi_handle)
{
    aui_handle_vbi  *dev_hdl  = NULL;
    void *tret = NULL;

    if (NULL == vbi_handle) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL!");
    }
	
    dev_hdl  = (aui_handle_vbi *)(vbi_handle);
	
    //close vbi task  
    if (alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_CHECK_TTX_TASK_START, FALSE) != 0) {
        aui_rtn(AUI_RTN_FAIL, "\n vbi stop fail! \n");
    }
	
    if (alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_TTX_STOP, FALSE) != 0) {
        aui_rtn(AUI_RTN_FAIL, "\n vbi teletext stop fail! \n");
    }
	
    //ttx_pid valid ,stop get ttx data from dmx
    if ((dev_hdl->ttx_pid > 0) && (dev_hdl->ttx_pid < AUI_INVALID_PID)) {
        if (alisldmx_control_channel(dev_hdl->dmx_hdl, dev_hdl->vbi_token.channelid, DMX_CTRL_DISABLE)) {
            aui_rtn(AUI_RTN_EINVAL,"alisldmx_control_channel fail!");
        }
		
        if (alisldmx_free_channel(dev_hdl->dmx_hdl, dev_hdl->vbi_token.channelid)) {
            aui_rtn(AUI_RTN_EINVAL,"alisldmx_free_channel fail!");
        }
		
        dev_hdl->thread_flag = VBI_WRITE_STOP;
        //Make sure the vbi thread exits. 
        pthread_mutex_lock(&(dev_hdl->mutex));
        pthread_cond_signal(&(dev_hdl->cond)); 
        pthread_mutex_unlock(&(dev_hdl->mutex));
		
        if(pthread_join(dev_hdl->vbi_thread, &tret) != 0) {
            AUI_ERR("pthread_join fail!\n");
            return AUI_RTN_FAIL;	
        } 	
		
        if(pthread_mutex_destroy(&(dev_hdl->mutex)) != 0) {
            AUI_ERR("pthread_mutex_destroy fail!\n");
            return AUI_RTN_FAIL;	
        } 
		
        if(pthread_cond_destroy(&(dev_hdl->cond)) != 0) {
            AUI_ERR("pthread_cond_destroy fail!\n");
            return AUI_RTN_FAIL;	
        } 
		
        aui_common_un_init_ring_buf(&(dev_hdl->vbi_rbuf));
		
        if(alisldmx_close(dev_hdl->dmx_hdl)) {
            aui_rtn(AUI_RTN_EINVAL,"alisldmx_close fail!");
        }
    }
    return AUI_RTN_SUCCESS;	
}

AUI_RTN_CODE aui_vbi_cc_start(aui_hdl vbi_handle, aui_vbi_cc_start_param *start_param)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_handle_vbi *dev_hdl;
	unsigned char cctype = CC_OTHER;
	
	if (( NULL == start_param) || (NULL == start_param->cc_decv_hdl)) {
		AUI_ERR("please make sure the decv device is running\n");
		aui_rtn(AUI_RTN_FAIL, "aui_vbi_cc_start fail\n");
	}
	
	if (!vbi_handle) {
		AUI_ERR("vbi_handle invalid\n");
		aui_rtn(AUI_RTN_FAIL, "aui_vbi_cc_start fail\n");
	}
	
	switch(start_param->user_data_type)
	{			
		case AUI_DECV_USER_DATA_ANY:			
			cctype = CC_PREANY;
			break;
			
		case AUI_DECV_USER_DATA_SCTE20: 			
			cctype = CC_SCTE20;
			break;
		case AUI_DECV_USER_DATA_ALL:
			aui_rtn(AUI_RTN_FAIL, "cc data type invalit!!\n"); 
		case AUI_DECV_USER_DATA_ATSC53: 		
		case AUI_DECV_USER_DATA_DEFAULT:
		default: 
			cctype = CC_GA94;
			break;
	}
	
	dev_hdl = (aui_handle_vbi *)vbi_handle;

	//set data type of VBI,VBI data type is CC_VBI_TYPE
	ret = alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_SET_SOURCE_TYPE, VBI_TYPE_CC);

	if (0 != ret){
		aui_rtn(AUI_RTN_FAIL, "set VBI source data type fail\n");
	}

	ret = alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_SET_CC_TYPE, cctype);

	if (0 != ret){
		aui_rtn(AUI_RTN_FAIL, "SL_IO_VBI_SET_CC_TYPE fail\n");
	}
	

	//open VBI IO,start play closed caption 
	ret = alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_CHECK_TTX_TASK_START, TRUE);

	if (0 != ret){
		aui_rtn(AUI_RTN_FAIL, "SL_IO_VBI_CHECK_TTX_TASK_START fail,CC start fail\n");
	}

	return ret;
	
}

AUI_RTN_CODE aui_vbi_cc_stop(aui_hdl vbi_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_handle_vbi *dev_hdl;
	
	if (NULL == vbi_handle) {
		aui_rtn(AUI_RTN_EINVAL, "handle is NULL!");
	}

	dev_hdl = (aui_handle_vbi *)vbi_handle;

	ret = alislvbi_io_control(dev_hdl->slhdl, SL_IO_VBI_CHECK_TTX_TASK_START, FALSE);

   //Driver need some time to execute the stop command,Make sure the CC subtitles disappear
    AUI_SLEEP(100);

	if (0 != ret){
		aui_rtn(AUI_RTN_FAIL, "closed caption stop fail\n");
	}
	
	return ret;
}



AUI_RTN_CODE aui_vbi_select_output_dev(aui_hdl vbi_handle,aui_vbi_output_device output_device)
{
	(void)output_device;
    if(NULL == vbi_handle) {
		;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vbi_set_wss_aspect_ratio(unsigned int aspect_ratio)
{
	(void)aspect_ratio;
	return AUI_RTN_SUCCESS;
}


