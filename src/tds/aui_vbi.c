/**  @file
*	 @brief 	aui vbi module
*	 @author	 andy.yu
*	 @date		   2013-6-26
*	 @version	  1.0.0
*	 @note		   ali corp. all rights reserved. 2013-2999 copyright (C)
*/
#include "aui_common_priv.h"
#include <aui_vbi.h>
#include <mediatypes.h>
#include <hld/hld_dev.h>
#include <hld/vbi/vbi.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/dis/vpo.h>

AUI_MODULE(VBI)

static OSAL_ID vbi_mutex = INVALID_ID;

#define VBI_SEVICE 3

#define VBI_FUNCTION_ENTER AUI_API("enter\n");
#define VBI_FUNCTION_LEAVE AUI_API("leave\n");

#define VBI_LOCK   osal_mutex_lock(vbi_mutex,OSAL_WAIT_FOREVER_TIME)
#define VBI_UNLOCK osal_mutex_unlock(vbi_mutex)

typedef struct aui_handle_vbi
{
	aui_dev_priv_data dev_priv_data;

	aui_ttx_start_param start_param;
	unsigned long vbi_date_type;
	unsigned int aspect_ratio;
	struct vbi_device *vbi_device;
	struct dmx_device *dmx_device;
	BOOL ttx_on_off;
}aui_handle_vbi;

static void ttx_upd_page(UINT16 page_id, UINT8 line_id)
{
	//vbi_ioctl(st_vbi_attr.vbi_device,IO_VBI_ENGINE_UPDATE_PAGE, (((page_id&0xffff)<<8)|(line_id&0xff)));
	(void)page_id;
	(void)line_id;
}

/**
*	 @brief 		vbi init function
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   p_call_back_init: init callback function 
*	 @param[in] 	   pv_param: callback para
*	 @return		 error code
*	 @note		  VBI module must be used before calling this function, passing a callback function, aui_vbi_init will call this callback function, 
*				Callback function is generally to achieve the contents of the attach.
*
*/
AUI_RTN_CODE aui_vbi_init(p_fun_cb p_callback_init, void *pv_param)
{
	VBI_FUNCTION_ENTER;

	if (p_callback_init) {
		p_callback_init(pv_param);
	}
	vbi_mutex = osal_mutex_create();
	if(INVALID_ID == vbi_mutex) {
		goto ERROR;
	}
	return AUI_RTN_SUCCESS;

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"INIT fail\n");
}

/**
*	 @brief 		vbi deinit function
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   p_call_back_init: deinit callback function
*	 @param[in] 	   pv_param: callback para
*	 @return		 error code
*	 @note		  which is called when exiting the VBI module, passing in a callback function that is called in aui_vbi_de_init.
*
*/
AUI_RTN_CODE aui_vbi_de_init(p_fun_cb p_callback_init,void *pv_param)
{
	VBI_FUNCTION_ENTER;

	if(p_callback_init) {
		p_callback_init(pv_param);
	}
	if(INVALID_ID != vbi_mutex) {
		osal_mutex_delete(vbi_mutex);
		vbi_mutex = INVALID_ID;
		goto ERROR;
	}
	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS;

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"DEINIT fail\n");
}


/**
*	 @brief 		open vbi device
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   open_param: open para
*	 @param[out]	vbi_handle: returns the operation handle
*	 @return		 error code
*	 @note
*
*/
AUI_RTN_CODE aui_vbi_open(aui_vbi_open_param *open_param,aui_hdl *vbi_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_handle_vbi *dev_hdl = NULL;
	struct vbi_device *vbi_dev = NULL;
	
	VBI_FUNCTION_ENTER;
    //(void) open_param;
	if (( NULL == open_param) || ( NULL == vbi_handle)) {
		AUI_ERR("aui_vbi_open failed \n");
		goto ERROR;
	}
	*vbi_handle = NULL;
	
	vbi_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, open_param->data_type);
	if(NULL == vbi_dev) {
		AUI_ERR("dev_get_by_type failed \n");
		goto ERROR;
	}
	ret = vbi_open(vbi_dev);
	if(RET_SUCCESS != ret) {
		AUI_ERR("sdec_open failed \n");
		goto ERROR;
	}
	//This is only enabled for ttx, and cc does not exist for this bit.
	if (AUI_VBI_DATA_TYPE_TELETEXT == open_param->data_type) {
		enable_vbi_transfer(TRUE);
	}
	
    dev_hdl = (aui_handle_vbi *)malloc(sizeof(aui_handle_vbi));
	if(NULL == dev_hdl) {
		AUI_ERR("dev_hdl is NULL.\n");
		goto ERROR;
	}  
	memset(dev_hdl, 0, sizeof(aui_handle_vbi));

	dev_hdl->vbi_device = vbi_dev;
	dev_hdl->vbi_date_type = open_param->data_type;
	dev_hdl->dev_priv_data.dev_idx = open_param->data_type;
	if (aui_dev_reg(AUI_MODULE_VBI, dev_hdl)) {
		free(dev_hdl);
		dev_hdl = NULL;
		AUI_ERR("aui_dev_reg failed \n");
		goto ERROR;
	}
	*vbi_handle    = dev_hdl;
	
	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS;

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"OPEN fail\n");
}

/**
*	 @brief 		close vbi decice
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   vbi_handle: handle returned by aui_vbi_open
*	 @param[out]	NULL
*	 @return		error code
*	 @note
*
*/
AUI_RTN_CODE aui_vbi_close(aui_hdl vbi_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_handle_vbi *dev_hdl = NULL;

	VBI_FUNCTION_ENTER;
	if (!vbi_handle) {
		AUI_ERR("vbi_handle invalid\n");
		goto ERROR;
	}

	dev_hdl = (aui_handle_vbi *)vbi_handle;
	ret = vbi_close(dev_hdl->vbi_device);
	if(RET_SUCCESS != ret) {
		AUI_ERR("aui_dev_unreg failed \n");
		goto ERROR;
	}
	
	if (aui_dev_unreg(AUI_MODULE_VBI, dev_hdl)) {
		AUI_ERR("aui_dev_unreg failed \n");
		goto ERROR;
	}
	dev_hdl->dev_priv_data.dev_idx = -1;
	free(vbi_handle);
	vbi_handle = NULL;

	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS;

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"CLOSE fail\n");
}


/**
*	 @brief 		start teletext
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   vbi_handle: handle returned by aui_vbi_open
*	 @param[in]    start_param: teletext start para
*	 @param[out]	NULL
*	 @return		error code
*	 @note		 Before you start teletext, you need to make sure that the DMX module is ready for use, 
*				 and you need to lock in the corresponding frequency points to get teletext data.
*/
AUI_RTN_CODE aui_vbi_ttx_start(aui_hdl vbi_handle,aui_ttx_start_param *start_param)
{
	struct dmx_device *dmx_dev = NULL;
	struct register_service vbi_sevice;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS; 	
	aui_handle_vbi *dev_hdl = NULL;

	VBI_FUNCTION_ENTER;
	if ( !vbi_handle) {
		AUI_ERR("vbi_handle is NULL\n");
		goto ERROR;
	}

	dev_hdl = (aui_handle_vbi*)vbi_handle;
	if ( AUI_VBI_DATA_TYPE_TELETEXT != dev_hdl->vbi_date_type) {
		AUI_ERR("Please set ttx data type \n");
		goto ERROR;
	}

	if(1 == dev_hdl->ttx_on_off) {
		AUI_ERR("vbi have already started \n");
		goto ERROR;
	}

	dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, start_param->dmx_index);
	if(NULL == dmx_dev)
	{
		AUI_ERR("dev_get_by_id failed \n");
		goto ERROR;
	}

	memset(&vbi_sevice,0, sizeof(struct register_service));
	vbi_sevice.device = dev_hdl->vbi_device;
	vbi_sevice.request_write = (request_write)vbi_request_write;
	vbi_sevice.update_write = (update_write)vbi_update_write;
	vbi_sevice.service_pid = start_param->ttx_pid;
	//regist it to dmx
	ret = dmx_register_service(dmx_dev,VBI_SEVICE,&vbi_sevice);
	if(ret != RET_SUCCESS) {
		AUI_ERR("dmx_register_service failed \n");
		goto ERROR;

	}
	ret = vbi_start(dev_hdl->vbi_device,ttx_upd_page);

	if(ret != RET_SUCCESS) {
		AUI_ERR("sdec_start failed \n");
		goto ERROR;
	}
	//vbi_ioctl(vbi_attr->vbi_device,IO_VBI_ENGINE_SHOW_ON_OFF, 1);
	dev_hdl->dmx_device = dmx_dev;
	dev_hdl->ttx_on_off = 1;

	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS; 

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"aui_vbi_ttx_start fail\n");
}


/**
*	 @brief 		stop teletext
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   vbi_handle: handle returned by aui_vbi_open
*	 @param[out]	NULL
*	 @return		 error code
*	 @note
*
*/
AUI_RTN_CODE aui_vbi_ttx_stop(aui_hdl vbi_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS; 	
	aui_handle_vbi * dev_hdl = NULL;

	VBI_FUNCTION_ENTER;
	if ( !vbi_handle) {
		AUI_ERR("vbi_handle invalid  \n");
		goto ERROR;
	}

	dev_hdl = (aui_handle_vbi*)vbi_handle;
	if ( AUI_VBI_DATA_TYPE_TELETEXT != dev_hdl->vbi_date_type) {
		AUI_ERR("vbi_handle error \n");
		goto ERROR;
	}

	if(0 == dev_hdl->ttx_on_off) {
		AUI_ERR("ttx status does not start \n");
		goto ERROR;
	}

	//stop dmx sevice
	ret = dmx_unregister_service(dev_hdl->dmx_device,VBI_SEVICE);
	if(ret != RET_SUCCESS)
	{
		AUI_ERR("dmx_unregister_service failed \n");
		goto ERROR;
	}
	//close osd
	ret = vbi_stop(dev_hdl->vbi_device);
	if(ret != RET_SUCCESS)
	{
		AUI_ERR("vbi_stop failed\n");
		goto ERROR;
	}
	dev_hdl->ttx_on_off = 1;
	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS; 	

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"aui_vbi_ttx_stop fail\n");
}

/**
*	 @brief 		Select the VBI data output device
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   vbi_handle: handle returned by aui_vbi_open
*	 @param[in] 	   output_device:HD / SD device
*	 @param[out]	NULL
*	 @return		error code
*	 @note		 VBI data is output to the TV through the CVBS, for the C3281 IC, because only the SD output, so output_device = VBI_OUTPUT_SD,
				 For the 3603/3811/3503 IC, due to the HD and SD devices, it is necessary to select which TV ENCODER to output the VBI data.
*
*/
AUI_RTN_CODE aui_vbi_select_output_dev(aui_hdl vbi_handle, aui_vbi_output_device output_device)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS; 	
	aui_handle_vbi * dev_hdl;
	t_vbirequest vbi_data_call_back;
	struct vpo_device * vpo_device;

	VBI_FUNCTION_ENTER;
	if ( !vbi_handle) {
		AUI_ERR("subt_handle invalid \n");
		goto ERROR;
	}

	dev_hdl = (aui_handle_vbi*)vbi_handle;
	vpo_device = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);

	vbi_setoutput(dev_hdl->vbi_device, &vbi_data_call_back);
	ret = vpo_ioctl(vpo_device, VPO_IO_SET_VBI_OUT, (UINT32)vbi_data_call_back);
	if(RET_SUCCESS != ret)
	{
		AUI_ERR("VPO_IO_SET_VBI_OUT failed\n");
		goto ERROR;
	}
	//Teletext set tv encode type
	if(AUI_VBI_DATA_TYPE_TELETEXT == dev_hdl->vbi_date_type) {
		ret = vbi_ioctl(dev_hdl->vbi_device,IO_VBI_SELECT_OUTPUT_DEVICE,(unsigned long)output_device);
	}
	if(RET_SUCCESS != ret) {
		AUI_ERR("VPO_IO_SET_VBI_OUT failed \n");
		goto ERROR;
	}
	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS; 	

ERROR:
	VBI_FUNCTION_LEAVE;
		aui_rtn(AUI_RTN_FAIL,"aui_vbi_select_output_dev fail\n");

}


AUI_RTN_CODE aui_vbi_cc_start(aui_hdl vbi_handle, aui_vbi_cc_start_param *start_param)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS; 	
	aui_handle_vbi *dev_hdl;
	struct vpo_device * vpo_device;
	//(void *)start_param;
	if( ( NULL == start_param) || (NULL == start_param->cc_decv_hdl)) {
		AUI_ERR("please make sure the decv device is running\n");
		goto ERROR;
	}
	
	VBI_FUNCTION_ENTER;
	if ( !vbi_handle) {
		AUI_ERR("vbi_handle invalid\n");
		goto ERROR;
	}

	dev_hdl = (aui_handle_vbi *)vbi_handle;
	if ( AUI_VBI_DATA_TYPE_CLOSED_CAPTION != dev_hdl->vbi_date_type) {
		AUI_ERR("vbi_handle error  \n");
		goto ERROR;
	}

	vpo_device = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	
	//Closed caption can only use sd  tv encode type
	ret = vpo_ioctl(vpo_device, VPO_IO_SD_CC_ENABLE,1); 	 
	if (RET_SUCCESS != ret) {
		AUI_ERR("vbi cc enable fail!\n");
		goto ERROR;
	}
	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS; 	

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"aui_vbi_cc_start fail\n");

}

/**
*	 @brief 		stop CC
*	 @author		niker.li
*	 @date			  2016-12-05
*	 @param[in] 	   vbi_handle: handle returned by aui_vbi_open
*	 @param[out]	NULL
*	 @return		error code
*	 @note
*
*/
AUI_RTN_CODE aui_vbi_cc_stop(aui_hdl vbi_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS; 	
	aui_handle_vbi * dev_hdl = NULL;
	struct vpo_device * vpo_device;

	VBI_FUNCTION_ENTER;
	if ( !vbi_handle) {
		AUI_ERR("vbi_handle invalid\n");
		goto ERROR;
	}

	dev_hdl = (aui_handle_vbi*)vbi_handle;
	if ( AUI_VBI_DATA_TYPE_CLOSED_CAPTION != dev_hdl->vbi_date_type) {
		AUI_ERR("vbi_handle error\n");
		goto ERROR;
	}

	ret = vbi_stop(dev_hdl->vbi_device);
	if(ret != RET_SUCCESS) {
		AUI_ERR("vbi_stop failed  \n");
		goto ERROR;
	}
	
	vpo_device = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	vbi_setoutput(dev_hdl->vbi_device, NULL);
	ret = vpo_ioctl(vpo_device, VPO_IO_SET_VBI_OUT, 0);
	if(RET_SUCCESS != ret) {
		AUI_ERR("VPO_IO_SET_VBI_OUT stop failed  \n");
		goto ERROR;
	}	

	ret = vpo_ioctl(vpo_device, VPO_IO_SD_CC_ENABLE,0); 	 
	if (RET_SUCCESS != ret) {
		AUI_ERR("vbi cc disable fail! \n");
		goto ERROR;
	}

	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS; 	

ERROR:
	VBI_FUNCTION_LEAVE;
	aui_rtn(AUI_RTN_FAIL,"aui_vbi_cc_stop fail\n");
}

/**
*	 @brief 		Set the aspect ratio of the video
*	 @author		andy.yu
*	 @date			  2013-6-26
*	 @param[in] 	   aspect_ratio: aspect ratio
*	 @param[out]	NULL
*	 @return
*	 @note		  not supported at this time
*
*/
AUI_RTN_CODE aui_vbi_set_wss_aspect_ratio(unsigned int aspect_ratio)
{
	VBI_FUNCTION_ENTER;
    (void) aspect_ratio;

	VBI_FUNCTION_LEAVE;
	return AUI_RTN_SUCCESS; 
}


