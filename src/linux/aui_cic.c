/* linux lib head files */
#include "aui_common_priv.h"
#include <aui_cic.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>


/* aliplatform lib head files */
#include <alislcic.h>


AUI_MODULE(CIC)

enum cic_signal_type {
    CIC_ENSTREAM = 0,   /* Emanciption stream (bypass stream to CAM) */
    CIC_ENSLOT,     /* Enable slot */
    CIC_RSTSLOT,        /* Reset slot */
    CIC_IOMEM,      /* Enable IO & memory space */
    CIC_SLOTSEL,        /* Select slot */
    CIC_CARD_DETECT,    /* CD information */
    CIC_CARD_READY      /* Ready/busy information */
};

enum cic_msg_type {
    CIC_DATA = 0,   /* CI data register */
    CIC_CSR,    /* CI command/stauts register */
    CIC_SIZELS, /* CI size register low bytes */
    CIC_SIZEMS, /* CI size register high bytes */
    CIC_MEMORY, /* CI memory space*/
    CIC_BLOCK,  /* CI block data Read/Write */
};

/* define for CI command register */
enum ci_command_register {
    CI_REG_RE               = 0x01,
    CI_REG_WE               = 0x02,
    CI_REG_IIR              = 0x10, /* CI+ */
    CI_REG_FR               = 0x40,
    CI_REG_DA               = 0x80,
    CI_REG_HC               = 0x01,
    CI_REG_SW               = 0x02,
    CI_REG_SR               = 0x04,
    CI_REG_RS               = 0x08
};
/****************************LOCAL VAR********************************************/

//aui_cic_hw_status_cb aui_hw_callback;

/****************************LOCAL FUNC ***********************************/

/*select slot*/
/*
TS control Register (0xB801A00C)
 CI_CESEL[bit-1] :
    1 : Select CI card B control signal
        into CI controller
    0 : Select CI card A control signal
        into CI controller (*)
*/
static  int cam_select_slot(aui_hdl *cic_handle, int slot)
{
    int ret = AUI_RTN_SUCCESS;

    ret = alislcic_set_signal(cic_handle, slot, CIC_SLOTSEL, 1);
    return ret;
}

/****************************MODULE IMPLEMENT*************************************/
/**
*    @brief         CIC module initialize
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     p_callback_init optional callback
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_init(p_fun_cb p_callback_init)
{
    if(p_callback_init) {
        return p_callback_init(NULL);
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         CIC module de-initialize
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     p_callback_deinit   optional callback
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_de_init(p_fun_cb p_callback_deinit)
{

    if(p_callback_deinit) {
        return p_callback_deinit(NULL);
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         open CIC device
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     aui_cic_hw_status_cb     called when Card detect status changed
*    @param[out]    pp_cic_handle   device handle
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          called only once for both slot 0 and slot 1.
*
*/
AUI_RTN_CODE aui_cic_open(aui_cic_hw_status_cb cic_hw_cb , aui_hdl *pp_cic_handle)
{
    int ret = AUI_RTN_SUCCESS;

    if(!pp_cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid pp_cic_handle parameter!");
    }

    /*contruct cic_handle*/
    ret = alislcic_construct(pp_cic_handle);
    if(ret || !(*pp_cic_handle)) {
        aui_rtn(AUI_RTN_ENOMEM,"Malloc cic device handle error!");
    }

    ret = alislcic_open(*pp_cic_handle);
    if(ret) {
        aui_rtn(AUI_RTN_FAIL,"Open cic device error!");
    }

    if(cic_hw_cb) {
        //if cam already in the slot, delay callback until open finished
        /* now driver only support slot 0 for CI*/
		
        /* register callback */
        if(alislcic_register_callback(cic_hw_cb)) {
            aui_rtn(AUI_RTN_FAIL,"aui_cic register callback error!");
        }
        // Use KUMSG instead of netlink, do not CIC_DRIVER_SETPORT
//        /* set socket communication port,third param = 0,auto allocate port*/
//        if(alislcic_ioctl(*pp_cic_handle, CIC_DRIVER_SETPORT, 0)) {
//            aui_rtn(AUI_RTN_FAIL, "Set socket communication port error!");
//        }
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         close CIC device
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle    device handle
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_close(aui_hdl cic_handle)
{

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }

    if(alislcic_close(cic_handle)) {
        aui_rtn(AUI_RTN_FAIL,"Close cic device error!");
    }

    if(alislcic_destruct(cic_handle)) {
        aui_rtn(AUI_RTN_FAIL,"Destruct cic device handle fail!");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         enable ci slot after detecting cam inserted
*    @author        Adolph.Liu
*    @date          2014-11-25
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_enable_cam(aui_hdl cic_handle,int slot)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }
    ret = cam_select_slot(cic_handle, slot);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Select CI slot fail!");
    }

    /* Enable Slot 0 */
    ret = alislcic_set_signal(cic_handle, slot, CIC_ENSLOT, 1);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Enable CI slot fail!");
    }


    /* Enable memory and io space*/
    ret = alislcic_set_signal(cic_handle, slot, CIC_IOMEM, 0);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Enable memory and io space fail!");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         disable ci slot after detecting cam removed
*    @author        Adolph.Liu
*    @date          2014-11-25
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_disable_cam(aui_hdl cic_handle,int slot)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    ret = cam_select_slot(cic_handle, slot);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Select CI slot fail!");
    }

    /* Enable Slot 0*/
    ret = alislcic_set_signal(cic_handle, slot, CIC_ENSLOT, 0);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Disable CI slot fail!");
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         read one byte from io register
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     offset          io register offset
*    @param[out]    value           read one byte
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_read_io_reg(aui_hdl cic_handle,int slot,aui_cic_reg offset,
                                 unsigned char *value)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }
    if(AUI_CIC_DATA != offset && AUI_CIC_DATA != offset && AUI_CIC_CSR != offset && \
       AUI_CIC_SIZELS != offset && AUI_CIC_SIZEMS !=offset) {
        aui_rtn(AUI_RTN_EINVAL,"offset invalid!");
    }

    ret = alislcic_read_io_reg(cic_handle, slot, offset, value);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Disable CI slot fail!");
    }

    return  AUI_RTN_SUCCESS;
}

/**
*    @brief         write one byte to io register
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     offset          io register offset
*    @param[in]     value           write one byte
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_write_io_reg(aui_hdl cic_handle,int slot,aui_cic_reg offset,
                                  unsigned char value)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"handle is NULL");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }
    if(AUI_CIC_DATA != offset && AUI_CIC_DATA != offset && AUI_CIC_CSR != offset && \
       AUI_CIC_SIZELS != offset && AUI_CIC_SIZEMS !=offset) {
        aui_rtn(AUI_RTN_EINVAL,"offset invalid!");
    }

    ret = alislcic_write_io_reg(cic_handle, slot, offset, &value);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Disable CI slot fail!");
    }
    return  AUI_RTN_SUCCESS;
}

/**
*    @brief         read io data register consecutively
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     size            size to read
*    @param[out]    buf             read buffer
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          user should check or clear command/status register
*
*/
AUI_RTN_CODE aui_cic_read_io_data(aui_hdl cic_handle,int slot,unsigned short size,
                                  unsigned char *buf)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle || !buf) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameters");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    ret = alislcic_read_io_data(cic_handle, slot, size, buf);
    if(ret) {
        aui_rtn(AUI_RTN_EIO,"Read block data fail!");
    }

    return  AUI_RTN_SUCCESS;
}

/**
*    @brief         write io data register consecutively
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     size            size to read
*    @param[in]     buf             write buffer
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          user should check or clear command/status register
*
*/
AUI_RTN_CODE aui_cic_write_io_data(aui_hdl cic_handle,int slot,unsigned short size,
                                   unsigned char *buf)
{
    int ret = AUI_RTN_SUCCESS;
    if(!cic_handle || NULL == buf) {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    ret = alislcic_write_io_data(cic_handle, slot, size, buf);
    if(ret) {
        aui_rtn(AUI_RTN_EIO,"Read block data fail!");
    }

    return  AUI_RTN_SUCCESS;
}

/**
*    @brief         read attribute memory
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     size            size to read, driver limit to 0x2000
*    @param[in]     addr            offset to start reading
*    @param[out]    buf             read buffer
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          driver read values from even addresses and writes consecutivly to the buffer.
*
*/
AUI_RTN_CODE aui_cic_read_mem(aui_hdl cic_handle,int slot,unsigned short size,
                              unsigned short addr,unsigned char *buf)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle || NULL == buf) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameters!");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    ret = alislcic_read_mem(cic_handle, slot, size, addr, buf);
    if (ret) {
	    aui_rtn(AUI_RTN_FAIL, "Read cic memory error!");
    }

    return  AUI_RTN_SUCCESS;
}

/**
*    @brief         write attribute memory
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     size            size to write, driver limit to 0x2000
*    @param[in]     addr            offset to start writing
*    @param[in]     buf             write buffer
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          driver read values from the buffer consecutively and writes to even addresses.
*
*/
AUI_RTN_CODE aui_cic_write_mem(aui_hdl cic_handle,int slot,unsigned short size,
                               unsigned short addr,unsigned char *buf)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle || NULL == buf) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameters!");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    ret = alislcic_write_mem(cic_handle, slot, size, addr, buf);
    if (ret) {
        aui_rtn(AUI_RTN_FAIL,"Write cic memory error!");
    }

    return  AUI_RTN_SUCCESS;
}

/**
*    @brief         detect cam
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[out]    detected        cam detected(1) or not(0)
*    @return        return AUI_RTN_SUCCESS, if success.
*    @return        return  AUI_ERR, cam not inserted well
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_detect_cam(aui_hdl cic_handle,int slot,int *detected)
{
    int ret = AUI_RTN_SUCCESS;
    unsigned char tmp;

    if(!cic_handle || (NULL==detected)) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameters!");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    ret = alislcic_test_signal(cic_handle, slot, CIC_CARD_DETECT, &tmp);
    if(ret) {
        aui_rtn(AUI_RTN_FAIL, "Detect CI error!");
    }
    AUI_DBG("detect status = %d\n",tmp);

    if(tmp == 0)
        *detected = 0;
    else if(tmp == 3)
        *detected = 1;
    else {
        aui_rtn(AUI_RTN_FAIL,NULL);
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         check CARD status(power on and ready)
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[out]    detected        cam ready(1) or not(0)
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_cam_is_ready(aui_hdl cic_handle,int slot,int *ready)
{
    int ret = AUI_RTN_SUCCESS;
    unsigned char tmp;
    tmp = 0;

    if(!cic_handle|| (NULL==ready)) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameters!");
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

	*ready = 0;

    ret = alislcic_test_signal(cic_handle, slot, CIC_CARD_READY, &tmp);
    if(ret) {
        aui_rtn(AUI_RTN_FAIL, "Detect CI is or not ready!");
    }
    AUI_DBG("detect ready status = %d\n",tmp);

    if(tmp)
        *ready= 1;
    else
        *ready = 0;

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         PCMCIA reset ¨C The Host sets the RESET signal active then inactive
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_reset_cam(aui_hdl cic_handle,int slot)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n"); 
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    /* Reset Slot */
    ret = alislcic_set_signal(cic_handle, slot, CIC_RSTSLOT, 1);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Reset slot fail!");
    }
	sleep(2);
    /* Reset Slot */
    ret = alislcic_set_signal(cic_handle, slot, CIC_RSTSLOT, 0);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Reset slot fail!");
    }

    //sleep(2);

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         CI Plus CAM reset ¨C Host sets the RS flag and begins interface initialisation
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_rs_reset_cam(aui_hdl cic_handle,int slot)
{
    int ret = AUI_RTN_SUCCESS;
    unsigned char value = CI_REG_RS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n"); 
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }


    ret = alislcic_write_io_reg(cic_handle, slot, CIC_CSR, &value);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Disable CI slot fail!");
    }

    usleep(200);

    value = 0;
    ret = alislcic_write_io_reg(cic_handle, slot, CIC_CSR, &value);
    if(ret) {
        aui_rtn( AUI_RTN_EIO, "Disable CI slot fail!");
    }

    return  AUI_RTN_SUCCESS;
}

#if 1
/**
*    @brief         set ts pass or bypass CAM
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     slot            slot index, 0 or 1
*    @param[in]     pass            0(bypass) or 1(pass)
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          none
*
*/
AUI_RTN_CODE aui_cic_pass_stream(aui_hdl cic_handle,int slot,unsigned char pass)
{
    int ret = AUI_RTN_SUCCESS;

    if(!cic_handle) {
        aui_rtn(AUI_RTN_EINVAL,"\n handle is NULL!\n"); 
    }
    if(0 != slot) {
        aui_rtn(AUI_RTN_EINVAL,"Only suppot slot 0 for the present");
    }

    /* Enable Slot 0 */
    ret = alislcic_set_signal(cic_handle, slot, CIC_ENSTREAM, pass);
    if(ret) {
        aui_rtn(AUI_RTN_EIO, "Set TS stream pass or bypass mode error!");
    }

    return AUI_RTN_SUCCESS;
}
#endif

/**
*    @brief         for other module to request mutex
*    @author        Adolph.Liu
*    @date          2015-07-22
*    @param[in]     cic_handle      CIC device handle
*    @param[in]     p_req           get mutex functions from driver.
*    @return        return AUI_RTN_SUCCESS, if success. or others.
*    @note          only used for handling hareware share pin conflict
*
*/
AUI_RTN_CODE aui_cic_req_mutex(aui_hdl cic_handle, aui_cic_mutex *p_req)
{
    (void )cic_handle;
    if(p_req->cic_enter_mutex)
        p_req->cic_enter_mutex();

    return AUI_RTN_SUCCESS;
}
