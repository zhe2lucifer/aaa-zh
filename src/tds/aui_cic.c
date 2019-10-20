#include "aui_common_priv.h"
#include <sys_config.h>
#include <hld/cic/cic.h>
#include <aui_cic.h>

AUI_MODULE(CIC)

/****************************LOCAL TYPE*******************************************/
/** CIC device handle */
typedef struct aui_st_handle_cic
{
    struct cic_device *cic_dev;
}aui_handle_cic,*aui_p_handle_cic;

/****************************LOCAL VAR********************************************/
static  aui_handle_cic *aui_cic_info = NULL;
static  aui_cic_hw_status_cb    aui_hw_callback = NULL;

/****************************LOCAL FUNC ***********************************/
static int cb_slot = -1; 
static void cam_hw_callback(int slot)
{
    cb_slot = slot;
    if(NULL != aui_hw_callback)
        aui_hw_callback(slot);
}

static  void cam_select_slot(int slot)
{
    struct cic_io_command_signal signal_param;
    signal_param.slot = slot;
    signal_param.signal = CIC_SLOTSEL;
    signal_param.status = 1;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_SSIGNAL, (unsigned int)&signal_param);
}
/****************************MODULE IMPLEMENT*************************************/
/**
*	 @brief 		CIC module initialize
*	 @author		Jason.Chen
*	 @date			2014-11-21
*	 @param[in] 	p_callback_init optional callback
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_init(p_fun_cb p_callback_init)
{
    if(NULL != aui_cic_info) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
	if((aui_cic_info = MALLOC(sizeof(aui_handle_cic))) == NULL) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}    
	if(p_callback_init)
		return p_callback_init(NULL);
    
	return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		CIC module de-initialize
*	 @author		Jason.Chen
*	 @date			2014-11-21
*	 @param[in] 	p_callback_deinit   optional callback
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_de_init(p_fun_cb p_callback_deinit)
{
	if(NULL == aui_cic_info) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	} else {
		MEMSET((void*)aui_cic_info,0x00,sizeof(aui_handle_cic));
		FREE(aui_cic_info);
        aui_cic_info = NULL;

        if(p_callback_deinit)
            return p_callback_deinit(NULL);

		return AUI_RTN_SUCCESS;
	}
}

/**
*	 @brief 		open CIC device 
*	 @author		Jason.Chen
*	 @date			2014-11-21
*    @param[in]	    cic_hw_cb     called when Card detect status changed
*	 @param[out]	pp_cic_handle	device handle
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			called only once for both slot 0 and slot 1.
*
*/
AUI_RTN_CODE aui_cic_open(aui_cic_hw_status_cb cic_hw_cb , aui_hdl *pp_cic_handle)
{
	int ret = AUI_RTN_SUCCESS;

	if(NULL == pp_cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    if(NULL == aui_cic_info) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
	aui_cic_info->cic_dev = (struct cic_device *)dev_get_by_id(HLD_DEV_TYPE_CIC, 0);

	if(NULL == aui_cic_info->cic_dev) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cb_slot = -1;
    if (0 != cic_open(aui_cic_info->cic_dev, cam_hw_callback)) {
        aui_rtn(AUI_RTN_EINVAL,"cic_open is error");
    }
 	*pp_cic_handle = (aui_hdl)aui_cic_info;
    if(cic_hw_cb) {
        aui_hw_callback = cic_hw_cb;
        //if cam already in the slot, delay callback until open finished
        aui_hw_callback(cb_slot);
    }
	return ret;
}

/**
*	 @brief 		close CIC device
*	 @author		Jason.Chen
*	 @date			2014-11-21
*	 @param[in]		cic_handle    device handle
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_close(aui_hdl cic_handle)
{
	int  ret = AUI_RTN_SUCCESS;

	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}

	if(NULL == aui_cic_info->cic_dev) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}

	if(0 != cic_close(aui_cic_info->cic_dev)) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
	aui_cic_info->cic_dev = NULL;
    if(NULL != aui_hw_callback)
        aui_hw_callback = NULL;
    
	return ret;
}

/**
*	 @brief 		enable ci slot after detecting cam inserted
*	 @author		Jason.Chen
*	 @date			2014-11-25
*	 @param[in]		cic_handle		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_enable_cam(aui_hdl cic_handle,int slot)
{
    struct cic_io_command_signal signal_param;

	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}

    // Select Slot   
    cam_select_slot(slot);
    // Enable Slot
    signal_param.slot = slot;        
    signal_param.signal = CIC_ENSLOT;
    signal_param.status = 1;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_SSIGNAL, (unsigned int)&signal_param);

    return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		disable ci slot after detecting cam removed
*	 @author		Jason.Chen
*	 @date			2014-11-25
*	 @param[in]		cic_handle		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_disable_cam(aui_hdl cic_handle,int slot)
{
    struct cic_io_command_signal signal_param;

	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    // Select Slot   
    cam_select_slot(slot);
    // Enable Slot
    signal_param.slot = slot;        
    signal_param.signal = CIC_ENSLOT;
    signal_param.status = 0;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_SSIGNAL, (unsigned int)&signal_param);

    return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		read one byte from io register
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		offset 		    io register offset
*	 @param[out]	value	        read one byte
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_read_io_reg(aui_hdl cic_handle,int slot,aui_cic_reg offset,
                                unsigned char *value)
{
    struct cic_io_command_iorw iorw_param;
    
	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==value)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);
    iorw_param.slot = slot;
    iorw_param.reg = offset;
    iorw_param.buffer = value;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_READIO, (UINT32)&iorw_param);
    return  AUI_RTN_SUCCESS;    
}

/**
*	 @brief 		write one byte to io register
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		offset 		    io register offset
*	 @param[in]	    value	        write one byte
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_write_io_reg(aui_hdl cic_handle,int slot,aui_cic_reg offset,
                                unsigned char value)
{
    struct cic_io_command_iorw iorw_param;  
    
	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);
    iorw_param.slot = slot;
    iorw_param.reg = offset;
    iorw_param.buffer = &value;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_WRITEIO, (UINT32)&iorw_param);    
    return  AUI_RTN_SUCCESS;    
}

/**
*	 @brief 		read io data register consecutively
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		size 		    size to read
*	 @param[out]	buf	            read buffer
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			user should check or clear command/status register
*
*/
AUI_RTN_CODE aui_cic_read_io_data(aui_hdl cic_handle,int slot,unsigned short size,
                                unsigned char *buf)
{
	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==buf)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);
    if(0 != cic_read(aui_cic_info->cic_dev, slot, size, buf))
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    
    return  AUI_RTN_SUCCESS;    
}

/**
*	 @brief 		write io data register consecutively
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		size 		    size to read
*	 @param[in]	    buf	            write buffer
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			user should check or clear command/status register
*
*/
AUI_RTN_CODE aui_cic_write_io_data(aui_hdl cic_handle,int slot,unsigned short size, 
                                unsigned char *buf)
{
	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==buf)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);
    if(0 != cic_write(aui_cic_info->cic_dev, slot, size, buf))
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    
    return  AUI_RTN_SUCCESS;   
}

/**
*	 @brief 		read attribute memory
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		size 		    size to read, driver limit to 0x2000
*	 @param[in]		addr 		    offset to start reading
*	 @param[out]	buf	            read buffer
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			driver read values from even addresses and writes consecutivly to the buffer.
*
*/
AUI_RTN_CODE aui_cic_read_mem(aui_hdl cic_handle,int slot,unsigned short size, 
                                unsigned short addr,unsigned char *buf)
{
    struct cic_io_command_memrw     memrw_param;
    
	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==buf)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);

    memrw_param.slot = slot;
    memrw_param.addr = addr;
    memrw_param.size = size;
    memrw_param.buffer = buf;
    if (0 != cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_READMEM, (unsigned int)&memrw_param)) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
    
    return  AUI_RTN_SUCCESS;
}

/**
*	 @brief 		write attribute memory
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		size 		    size to write, driver limit to 0x2000
*	 @param[in]		addr 		    offset to start writing
*	 @param[in]	    buf	            write buffer
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			driver read values from the buffer consecutively and writes to even addresses.
*
*/
AUI_RTN_CODE aui_cic_write_mem(aui_hdl cic_handle,int slot,unsigned short size, 
                                unsigned short addr,unsigned char *buf)
{
    struct cic_io_command_memrw     memrw_param;
    
	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==buf)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);

    memrw_param.slot = slot;
    memrw_param.addr = addr;
    memrw_param.size = size;
    memrw_param.buffer = buf;
    if (0 != cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_WRITEMEM, (unsigned int)&memrw_param)) {
        aui_rtn(AUI_RTN_EINVAL,"CIC_DRIVER_WRITEMEM is error");
    }
    
    return  AUI_RTN_SUCCESS;
}

/**
*	 @brief 		detect cam
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[out]	detected	    cam detected(1) or not(0)
*	 @return		return AUI_RTN_SUCCESS, if success. 
*    @return        return  AUI_CIC_ERROR, cam not inserted well
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_detect_cam(aui_hdl cic_handle,int slot,int *detected)
{
    struct cic_io_command_signal signal_param;

	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==detected)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}

    MEMSET(&signal_param, 0, sizeof(struct cic_io_command_signal));
    signal_param.slot = slot;
    signal_param.signal = CIC_CARD_DETECT;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_TSIGNAL, (unsigned int)&signal_param);

    AUI_DBG("detect status = %d\n",signal_param.status);
    
    if(signal_param.status == 0)
        *detected = 0;
    else if(signal_param.status == 3)
        *detected = 1;
    else
        aui_rtn(AUI_RTN_EINVAL,"status is error");

    return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		check CARD status(power on and ready)
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[out]	detected	    cam ready(1) or not(0)
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_cam_is_ready(aui_hdl cic_handle,int slot,int *ready)
{
    struct cic_io_command_signal signal_param;

	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==ready)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}

    MEMSET(&signal_param, 0, sizeof(struct cic_io_command_signal));
    signal_param.slot = slot;
    signal_param.signal = CIC_CARD_READY;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_TSIGNAL, (unsigned int)&signal_param);

    if(signal_param.status)
        *ready= 1;
    else
        *ready = 0;

    return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		PCMCIA reset ¨C The Host sets the RESET signal active then inactive
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_reset_cam(aui_hdl cic_handle,int slot)
{
    struct cic_io_command_signal signal_param;
    
	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);
    // Reset Slot
    signal_param.slot = slot;    
    signal_param.signal = CIC_RSTSLOT;
    signal_param.status = 1;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_SSIGNAL, (unsigned int)&signal_param);

    /* Keep >15ms. (3nd party design: 100mS, Digisat sugest 200ms for ZetaCAM) After that, CAM Attach Again */
    osal_task_sleep(200);

    cam_select_slot(slot);
    // Clear Reset Slot  
    signal_param.signal = CIC_RSTSLOT;
    signal_param.status = 0;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_SSIGNAL, (unsigned int)&signal_param);   

    return  AUI_RTN_SUCCESS;
}

/**
*	 @brief 		CI Plus CAM reset ¨C Host sets the RS flag and begins interface initialisation
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_rs_reset_cam(aui_hdl cic_handle,int slot)
{
    struct cic_io_command_iorw     iorw_param;
    unsigned char   data[2];
    
	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cam_select_slot(slot);
    
    // Set I/O Param
    iorw_param.slot = slot;
    iorw_param.reg = CIC_CSR;    /* Command/Status Register */
    iorw_param.buffer = data;

    /* Step 1: Reset the CAM Card Interface */
    data[0] = CI_REG_RS;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_WRITEIO, (unsigned int)&iorw_param);

    osal_task_sleep(200);

    /* Additional Step for ATsky CAM: Clear Reset CAM Card Interface*/
    cam_select_slot(slot);    
    data[0] = 0x00;
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_WRITEIO, (unsigned int)&iorw_param);

    return AUI_RTN_SUCCESS;    
}

/**
*	 @brief 		set ts pass or bypass CAM
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle 		CIC device handle
*	 @param[in]		slot 		    slot index, 0 or 1
*	 @param[in]		pass 		    0(bypass) or 1(pass)
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			none
*
*/
AUI_RTN_CODE aui_cic_pass_stream(aui_hdl cic_handle,int slot,unsigned char pass)
{
    struct cic_io_command_signal signal_param;

	if((void*) aui_cic_info != (void*)cic_handle) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    signal_param.slot = slot;
    signal_param.signal = CIC_EMSTREAM;
    signal_param.status = pass;    /* Pass CAM */
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_SSIGNAL, (unsigned int)&signal_param);

    return AUI_RTN_SUCCESS;
}

/**
*	 @brief 		for other module to request mutex
*	 @author		Jason.Chen
*	 @date			2014-11-24
*	 @param[in]		cic_handle		CIC device handle
*	 @param[in]		p_req           get mutex functions from driver.
*	 @return		return AUI_RTN_SUCCESS, if success. or others.
*	 @note			only used for handling hareware share pin conflict 
*
*/
AUI_RTN_CODE aui_cic_req_mutex(aui_hdl cic_handle, aui_cic_mutex *p_req)
{
	if(((void*) aui_cic_info != (void*)cic_handle) || (NULL==p_req)) {
		aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
	}
    cic_io_control(aui_cic_info->cic_dev, CIC_DRIVER_REQMUTEX, (unsigned int)p_req);
  
    return AUI_RTN_SUCCESS;
}

