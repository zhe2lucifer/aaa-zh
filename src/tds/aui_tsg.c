#include "aui_common_priv.h"
#include <hld/snd/snd.h>
#include <aui_tsg.h>
#include <api/libfs2/stdio.h>
#include <bus/tsg/tsg.h>
#include <bus/tsi/tsi.h>
#include <hld/dmx/dmx.h>

/* TSG module version number. */
#define AUI_MODULE_VERSION_NUM_TSG    (0X00020200)

/** Device handle */
typedef struct aui_st_handle_tsg {
    aui_dev_priv_data dev_priv_data;
    aui_attr_tsg      attr_tsg;
} aui_handle_tsg;

AUI_MODULE(TSG)

static OSAL_ID s_mod_mutex_id_tsg = 0;

AUI_RTN_CODE aui_tsg_version_get(unsigned long *pul_version) {
    if(NULL == pul_version) {
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }
    *pul_version = AUI_MODULE_VERSION_NUM_TSG;

    return SUCCESS;
}

AUI_RTN_CODE aui_tsg_init(p_fun_cb p_call_back_init, void *pv_param) {
    AUI_RTN_CODE result = AUI_RTN_SUCCESS;

    s_mod_mutex_id_tsg = osal_mutex_create();
    if(0 == s_mod_mutex_id_tsg) {
        aui_rtn(AUI_RTN_FAIL, "create mutex failed");
    }

    if(NULL != p_call_back_init) {
        result = p_call_back_init(pv_param);
    }

    return result;

}

AUI_RTN_CODE aui_tsg_de_init(p_fun_cb p_call_back_init, void *pv_param) {
    AUI_RTN_CODE result = AUI_RTN_SUCCESS;

    if(E_OK != osal_mutex_delete(s_mod_mutex_id_tsg)) {
        aui_rtn(AUI_RTN_FAIL, "delete mutex failed");
    }

    if((NULL != p_call_back_init))
    {
        result = p_call_back_init(pv_param);
    }

    return result;
}

AUI_RTN_CODE aui_tsg_open(const aui_attr_tsg *p_attr_tsg, aui_hdl* const pp_hdl_tsg) {

	if(NULL == pp_hdl_tsg) {
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }

    aui_handle_tsg *p_handle = (aui_handle_tsg *) MALLOC(sizeof(aui_handle_tsg));
    if(NULL == p_handle) {
        aui_rtn( AUI_RTN_ENOMEM, "malloc failed");
    }
    MEMSET(p_handle, 0, sizeof(aui_handle_tsg));

    if(NULL == p_attr_tsg) {
    	FREE(p_handle);
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }

    p_handle->dev_priv_data.dev_idx = p_attr_tsg->uc_dev_idx;
    MEMCPY(&p_handle->attr_tsg, p_attr_tsg, sizeof(aui_attr_tsg));
    //set default clk to tsg and tsg will use this clk when tsg start with bitrate=0
    tsg_set_default_clk(p_attr_tsg->ul_tsg_clk);
    /* sometime need to send null packet to CI card to keep CI card working, 
       now we don't send null packet as default*/
	tsg_send_null_packets_disable(0);
    
    aui_dev_reg(AUI_MODULE_TSG, p_handle);
    *pp_hdl_tsg = p_handle;

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsg_close(aui_hdl p_hdl_tsg) {

    aui_dev_unreg(AUI_MODULE_TSG, p_hdl_tsg);
    MEMSET(p_hdl_tsg, 0, sizeof(aui_handle_tsg));
    FREE(p_hdl_tsg);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsg_start(aui_hdl p_hdl_tsg, const aui_attr_tsg *p_attr_tsg) {
	aui_handle_tsg *p_handle = (aui_handle_tsg *) p_hdl_tsg;

    tsg_start(p_attr_tsg->ul_bit_rate);
    p_handle->attr_tsg.ul_bit_rate=p_attr_tsg->ul_bit_rate;

    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_stop(aui_hdl p_hdl_tsg, const aui_attr_tsg *p_attr_tsg) {
    if(NULL == p_hdl_tsg) {
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }
    tsg_stop();

    // reserved 
    (void)p_attr_tsg;
    
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_send(aui_hdl p_hdl_tsg,const aui_attr_tsg *p_attr_tsg) {
	if((NULL == p_hdl_tsg) || (NULL == p_attr_tsg)) {
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }
    osal_cache_flush(p_attr_tsg->ul_addr, p_attr_tsg->ul_pkg_cnt * 188);
    tsg_transfer(p_attr_tsg->ul_addr, \
    		p_attr_tsg->ul_pkg_cnt, p_attr_tsg->uc_sync_mode);

    return AUI_RTN_SUCCESS;
}


#define tsg_is_buf_valid  tsg_check_buf_busy
AUI_RTN_CODE aui_tsg_check_buf_status(aui_hdl p_hdl_tsg, 
		unsigned long ul_buf_addr2check, unsigned long ul_buf_len2check) {

	if(NULL == p_hdl_tsg) {
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }

	// check buffer whether it is valid or not.
	// if the buffer is not overlap with the buffers in TSG buffer, it is valid!
	// tsg_check_buf_busy is really a confused name.
	// tsg_is_buf_valid is better.
    if(!tsg_is_buf_valid(ul_buf_addr2check, ul_buf_len2check)) {
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_check_remain_pkt(aui_hdl p_hdl_tsg, unsigned long *p_pkt_cnt) {

	if((NULL == p_hdl_tsg) || (NULL==p_pkt_cnt)) {
        aui_rtn( AUI_RTN_EINVAL, "Invalid parameter");
    }
    *p_pkt_cnt = tsg_check_remain_buf();

    return AUI_RTN_SUCCESS;
}

