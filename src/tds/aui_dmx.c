/**@file
*  @brief          AUI DMX module interface implement
*  @author         ray.gong
*  @date           2013-5-21
*  @version        1.0.0
*  @note           ali corp. all rights reserved. 2013~2999 copyright (C)\n
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_dmx.h>
#include <aui_dsc.h>
#include <aui_decv.h>
#include "aui_dsc_inner.h"
#include <hld/dmx/dmx.h>
#include <hld/dsc/dsc.h>

/****************************LOCAL MACRO******************************************/
AUI_MODULE(DMX)

#define AUI_DMX_ONE_SECTION_LEN_MAX     (4*1024)
#define AUI_DMX_RINBUF_WRITE_TRYCNT     (10)
#define ALI_UDI_PES_BUF_SIZE            (64*1024)
#define ALI_UDI_RAW_PKT_LEN             (188)
#define ALI_UDI_RAW_BUF_SIZE            (64*1024)

/****************************LOCAL TYPE*******************************************/
typedef struct aui_st_handle_dmx
{
    aui_dev_priv_data            dev_priv_data;
    OSAL_ID                      dev_mutex_id;
    struct dmx_device           *pst_dev_dmx;
    aui_attr_dmx                 attr_dmx;
    void                        *p_channel_head;
    struct list_head             lst_channel_dev;
} aui_handle_dmx, *aui_p_handle_dmx;

typedef struct aui_st_handle_dmx_channel
{
    aui_dev_priv_data            dev_priv_data;
    OSAL_ID                      dev_mutex_id;
    unsigned long                ul_channel_id; /** dmx channel id*/
    aui_handle_dmx              *p_hdl_dmx;
    aui_ring_buf                 chan_rbuf;
    unsigned char               *bufaddr;
    unsigned int                 bufpos;
    unsigned int                 buflen;
    unsigned int                 last_pes_len;
    unsigned int                 last_pes_acc;
    aui_attr_dmx_channel         attr_dmx_channel;
    void                        *p_filter_head;
    /** it's output parameter, so just set zero when open filter.
        it output the information of internal driver */
    struct get_section_param     st_get_section;
    /** it's output parameter, so just set zero when open filter.
        it output the information of internal driver */
    struct register_service_new  st_reg_serv;
    /** it's output parameter, so just set zero when open filter.
        it output the information of internal driver */
    unsigned char               *puc_chl_buf_tmp;
    unsigned char               *puc_chl_buf;
    unsigned int                 busy:1;
    unsigned int                 try_close:1;
    unsigned int                 resv:30;    
    void                        *p_prev;
    void                        *p_next;
    struct list_head             lst_channel_dev;
} aui_handle_dmx_channel, *aui_p_handle_dmx_channel;

typedef struct aui_st_handle_dmx_filter
{
    aui_dev_priv_data            dev_priv_data;
    OSAL_ID                      dev_mutex_id;
    unsigned long                ul_filter_id;
    aui_handle_dmx_channel      *p_hdl_dmx_channel;
    aui_attr_dmx_filter          attr_dmx_filter;
    aui_ring_buf                 fil_rbuf; /**The filter ringbuf*/
    unsigned int                 busy:1;
    unsigned int                 try_close:1;
    unsigned int                 resv:30;
    void                        *p_prev;
    void                        *p_next;
} aui_handle_dmx_filter, *aui_p_handle_dmx_filter;

//The current video format is used to get the video PID mask.
extern aui_decv_format g_aui_decv_format;

/****************************LOCAL VAR********************************************/
static aui_dmx_module_attr s_dmx_attr;
static OSAL_ID s_mod_mutex_id_dmx = OSAL_INVALID_ID;
static OSAL_ID s_mutex_id_dmx = OSAL_INVALID_ID;
static OSAL_ID s_mutex_id_dmx_channel = OSAL_INVALID_ID;
static unsigned char s_ac_buffer_in[16] = {0};
static unsigned char s_ac_buffer_out[16] = {0};
static DEEN_CONFIG st_de_en_cfg;

/****************************LOCAL FUNC DECLARE***********************************/
#ifndef _AUI_OTA_
static void dmx_get_audio_pid(enum aui_deca_stream_type type, unsigned short pid, unsigned short *new_pid)
{
    unsigned short aud_pid = pid;
    if (AUI_INVALID_PID <= aud_pid) {
        goto ERR_END;
    }
    switch (type) {
        case AUI_DECA_STREAM_TYPE_AC3:
            aud_pid |= AUI_AC3_DES_EXIST;
            break;
        case AUI_DECA_STREAM_TYPE_AAC_LATM:
            aud_pid |= AUI_AAC_DES_EXIST;
            break;
        case AUI_DECA_STREAM_TYPE_AAC_ADTS:
            aud_pid |= AUI_ADTS_AAC_DES_EXIST;
            break;
        case AUI_DECA_STREAM_TYPE_EC3:
            aud_pid |= AUI_EAC3_DES_EXIST;
            break;
        default :
            break;
    }
ERR_END:
    if (new_pid != NULL) {
        *new_pid = aud_pid;
    }
}

static void dmx_get_video_pid(aui_decv_format type, unsigned short pid, unsigned short *new_pid)
{
    unsigned short video_pid = pid;
    if (AUI_INVALID_PID <= video_pid) {
        goto ERR_END;
    }
    switch (type) {
        case AUI_DECV_FORMAT_AVC:
            video_pid |= AUI_DECV_AVC_MASK;
            break;
        case AUI_DECV_FORMAT_AVS:
            video_pid |= AUI_DECV_AVS_MASK;
            break;
        case AUI_DECV_FORMAT_HEVC:
            video_pid |= AUI_DECV_HEVC_MASK;
            break;
        default :
            break;
    }
ERR_END:
    if (new_pid != NULL) {
        *new_pid = video_pid;
    }
}
#endif //<--_AUI_OTA_

static int aui_msk_val_rev_filter(unsigned char *val,
                                  unsigned char *msk,
                                  unsigned char *revers,
                                  unsigned char *src,
                                  unsigned long ul_mask_len)
{
    unsigned long i = 0;
    unsigned char bit_msk = 0;
    unsigned char bit_msk_no = 0;
    unsigned char ul_cond1 = 0;
    unsigned char ul_cond2 = 0;

    if (((val == NULL) || (msk == NULL) || (NULL == revers) || (NULL == src)) && (ul_mask_len > 0)) {
        return AUI_RTN_FAIL;
    }

    for (i = 0; i < ul_mask_len; i++) {
        bit_msk = (msk[i] & revers[i]);
        bit_msk_no = (msk[i] & (~revers[i]));
        ul_cond1 = ((bit_msk_no & val[i]) == (bit_msk_no & src[i]));
        ul_cond2 = ((bit_msk & val[i]) != (bit_msk & src[i]));
        AUI_DBG("bit_msk:bit_msk_no:ulCond1:ulCond2 = [%02x][%02x][%02x][%02x]\n",
            bit_msk, bit_msk_no, ul_cond1, ul_cond2);
        if (bit_msk) {
            if (!(ul_cond1 && ul_cond2)) {
                AUI_ERR("failed 1 cond\n");
                return AUI_RTN_FAIL;
            }
        } else {
            if (!(ul_cond1 )) {
                AUI_ERR("failed 2 cond\n");
                return AUI_RTN_FAIL;
            }
        }
    }
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_hit_flt(aui_hdl flt_hdl, unsigned char *p_section_buf, unsigned long ul_section_len)
{
    aui_handle_dmx_filter *flt_hdl_tmp = flt_hdl;
    aui_attr_dmx_filter *p_flt_attr = NULL;
    unsigned long ul_mask_len = 0;
    unsigned char *src = NULL;
    unsigned char *val = NULL;
    unsigned char *msk = NULL;
    unsigned char *mod = NULL;
    
    if ((0 == ul_section_len) || (0 == p_section_buf)) {
        return AUI_RTN_FAIL;
    }
    p_flt_attr = &(flt_hdl_tmp->attr_dmx_filter);
    /** new add for fil flt close busy detect ,remove callback mutex begin -->*/
    osal_task_dispatch_off();
    if (flt_hdl_tmp->try_close) {
        osal_task_dispatch_on();
        return AUI_RTN_FAIL;
    }
    flt_hdl_tmp->busy = 1;
    osal_task_dispatch_on();
    /** <-- new add for fil flt close busy detect ,remove callback mutex end */
    ul_mask_len = p_flt_attr->ul_mask_val_len;
    src = p_section_buf;
    val = p_flt_attr->puc_val;
    msk = p_flt_attr->puc_mask;
    mod = p_flt_attr->puc_reverse;
    if (0 != aui_msk_val_rev_filter(val, msk, mod, src, ul_mask_len)) {
        flt_hdl_tmp->busy = 0;
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_chl_dbuf_ali_req(aui_hdl chl_hdl, 
                                         unsigned long ul_req_size,
                                         void **pp_req_buf,
                                         unsigned long *req_buf_size,
                                         struct aui_avsync_ctrl *pst_ctrl_blk)
{
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = chl_hdl;    
    unsigned int left = 0;
    
    if ((NULL == chl_hdl) || (pp_req_buf == NULL) || (ul_req_size == 0)) {
        return AUI_RTN_FAIL;
    }

    if (NULL == p_hdl_dmx_channel_tmp->bufaddr) {
        return AUI_RTN_FAIL;
    }
    *pp_req_buf = p_hdl_dmx_channel_tmp->bufaddr + p_hdl_dmx_channel_tmp->bufpos;
    left = p_hdl_dmx_channel_tmp->buflen - p_hdl_dmx_channel_tmp->bufpos;
    if (left >= ul_req_size) {
        *req_buf_size = ul_req_size;
    } else {
        *req_buf_size = left;
    }
    pst_ctrl_blk->instant_update = 1;
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_chl_dbuf_ctm_req(aui_hdl chl_hdl,
                                         unsigned long ul_req_size,
                                         void **pp_req_buf,
                                         unsigned long *req_buf_size,
                                         struct aui_avsync_ctrl *pst_ctrl_blk)
{
    aui_dev_priv_data *aui_dev_priv = NULL;
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = NULL;
    struct list_head *cur = NULL;
    struct list_head *head_dev = NULL;
    aui_handle_dmx_filter *flt_hdl = NULL;

    (void)p_hdl_dmx_channel_tmp;
    if ((NULL == chl_hdl) || (pp_req_buf == NULL)) {
        return AUI_RTN_FAIL;
    }
    *pp_req_buf = NULL;
    aui_dev_priv = (aui_dev_priv_data *)chl_hdl;
    p_hdl_dmx_channel_tmp = chl_hdl;
    head_dev = (struct list_head *)aui_dev_priv->sub_list.next;
    //AUI_DBG("resume head flt [%08x]->[%08x]\n", chl_hdl,head_dev);
    if (NULL == head_dev) {
        return AUI_RTN_FAIL;
    }
    cur = head_dev;
    p_hdl_dmx_channel_tmp = chl_hdl;
    do {
        if (NULL == cur) {
            return AUI_RTN_FAIL;
        }
        flt_hdl = (aui_handle_dmx_filter *)cur;
        if ((AUI_DMX_FILTER_RUN == flt_hdl->attr_dmx_filter.dmx_filter_status) &&
            (NULL != flt_hdl->attr_dmx_filter.p_fun_data_req_wtCB)) {
            flt_hdl->attr_dmx_filter.p_fun_data_req_wtCB(flt_hdl,ul_req_size,pp_req_buf,req_buf_size,pst_ctrl_blk);        
            return AUI_RTN_SUCCESS;
        }
        cur = cur->next;
        if (NULL != cur) {
            /** because for data type channel,If customer implement buffer request and update
             we add a constraint: one channel one filter
             if we remove this constraint, then we can remove this ASSERT */
            ASSERT(0);
        }
    } while (cur != NULL);
    return AUI_RTN_FAIL;
}    

/** for section sync get special proc,one time on section */
static int aui_common_get_rbuf_rdlen(aui_ring_buf *p_ring_buf)
{
    unsigned long ul_data_len = 0;
    int sec_len = 0;
    unsigned char syn_indi = 0;
    unsigned char sec_len_low_byte = 0;
    if ((NULL == p_ring_buf->pby_ring_buf)) {
        return AUI_RTN_SUCCESS;
    }
    if (SUCCESS != aui_common_ring_buf_data_len(p_ring_buf,&ul_data_len)) {
        return AUI_RTN_SUCCESS;
    }
    if (ul_data_len > 0) {
        AUI_TAKE_SEM(p_ring_buf->sem_ring);
        if ((p_ring_buf->ul_ring_buf_rd + 2) == p_ring_buf->ul_ring_buf_len) {
            syn_indi = *(unsigned char *)((unsigned char*)(p_ring_buf->pby_ring_buf) + p_ring_buf->ul_ring_buf_rd + 1);
            sec_len_low_byte = *(unsigned char*)(p_ring_buf->pby_ring_buf);
        } else if ((p_ring_buf->ul_ring_buf_rd + 1) == p_ring_buf->ul_ring_buf_len) {
            syn_indi = *(unsigned char *)((p_ring_buf->pby_ring_buf));
            sec_len_low_byte = *(unsigned char *)((p_ring_buf->pby_ring_buf) + 1);
        } else {
            syn_indi = *(unsigned char *)((p_ring_buf->pby_ring_buf) + p_ring_buf->ul_ring_buf_rd + 1);
            sec_len_low_byte = *(unsigned char *)((p_ring_buf->pby_ring_buf)+ p_ring_buf->ul_ring_buf_rd + 2);
        }
        sec_len = (((unsigned short)((syn_indi&0x0f)<<8)) | (sec_len_low_byte)) + 3;
        AUI_GIVE_SEM(p_ring_buf->sem_ring);
        return sec_len;
    } else {
        return AUI_RTN_SUCCESS;
    }
}

extern AUI_RTN_CODE aui_common_ring_buf_lock(aui_ring_buf* p_ring_buf);
extern AUI_RTN_CODE aui_common_ring_buf_unlock(aui_ring_buf* p_ring_buf);

static void aui_dmx_rpt_overflow(aui_ring_buf *p_ringbuf,
                                 aui_dmx_handle_type handle_type,
                                 void *p_user_hdl,
                                 aui_dmx_event_report cb,
                                 void *param)
{
    aui_st_dmx_data_overflow_rpt of_rpt;    
    memset(&of_rpt, 0, sizeof(aui_st_dmx_data_overflow_rpt));
    
    if (!p_ringbuf) {
        return ;
    }    
    aui_common_ring_buf_lock(p_ringbuf);
    if (p_ringbuf->ul_ring_buf_wt >= p_ringbuf->ul_ring_buf_rd) {
        of_rpt.ringbuf_right = (unsigned long)(p_ringbuf->pby_ring_buf + p_ringbuf->ul_ring_buf_rd);
        of_rpt.len_right = p_ringbuf->ul_ring_buf_wt-p_ringbuf->ul_ring_buf_rd;    
        of_rpt.ringbuf_left = (unsigned long)p_ringbuf->pby_ring_buf;        
        of_rpt.len_left = 0;    
    } else {        
        of_rpt.ringbuf_right = (unsigned long)(p_ringbuf->pby_ring_buf + p_ringbuf->ul_ring_buf_rd);            
        of_rpt.len_right = p_ringbuf->ul_ring_buf_len - p_ringbuf->ul_ring_buf_rd;
        of_rpt.ringbuf_left = (unsigned long)p_ringbuf->pby_ring_buf;
        of_rpt.len_left = p_ringbuf->ul_ring_buf_wt;                            
    }
    cb(p_user_hdl, handle_type, AUI_DMX_EVENT_SYNC_GET_DATA_OVERFLOW, (void*)(&of_rpt), param);
    p_ringbuf->ul_ring_buf_rd = p_ringbuf->ul_ring_buf_wt = 0;    
    aui_common_ring_buf_unlock(p_ringbuf);
    osal_task_sleep(1);    
    return ;
}

static void aui_dmx_put_data_to_rbuf(aui_ring_buf *p_ringbuf,
                                     unsigned char *buf,
                                     unsigned int len,
                                     aui_handle_dmx_channel *p_chl_handle,
                                     aui_handle_dmx_filter *p_flt_hdl)
{
    unsigned char syn_indi = *(unsigned char *)(buf + 1);
    unsigned char sec_len_low_byte = *(unsigned char*)(buf + 2);
    aui_dmx_event_report cb = NULL;
    int fail_cnt = 0;
    
    if (p_ringbuf && (p_ringbuf->pby_ring_buf) && (p_ringbuf->ul_ring_buf_len)) {
        if (len != (unsigned int)(((unsigned short)((syn_indi&0x0f)<<8)) | (sec_len_low_byte)) + 3) {
            return;
        }
        while (fail_cnt <= AUI_DMX_RINBUF_WRITE_TRYCNT) {
            if (AUI_RTN_SUCCESS == aui_common_ring_buf_wt(p_ringbuf,len,buf)) {
                return ;
            } else {
                fail_cnt++;
                /** notify overflow */
                if ((p_chl_handle) && (!p_flt_hdl)) {
                    cb = p_chl_handle->attr_dmx_channel.event_cb;                    
                    if (cb) {
                        aui_dmx_rpt_overflow(p_ringbuf, AUI_DMX_HANDLE_CHANNEL,
                            p_chl_handle, cb, p_chl_handle->attr_dmx_channel.event_cb_param);
                        p_chl_handle->attr_dmx_channel.overflow_rpt_cnt++;
                    }                    
                } else if ((!p_chl_handle) && (p_flt_hdl)) {
                    cb = p_flt_hdl->attr_dmx_filter.event_cb;
                    if (cb) {
                        aui_dmx_rpt_overflow(p_ringbuf, AUI_DMX_HANDLE_FILTER,
                            p_flt_hdl, cb, p_flt_hdl->attr_dmx_filter.event_cb_param);
                        p_flt_hdl->attr_dmx_filter.overflow_rpt_cnt++;
                    }
                }            
                if (!cb) {
                    osal_task_sleep(1);
                }            
            }
        }

        /** record auto reset buffer count by driver when overflow happens */
        if ((p_chl_handle) && (!p_flt_hdl)) {
            p_chl_handle->attr_dmx_channel.overflow_auto_rst_buf_cnt++;    
        } else if ((!p_chl_handle) && (p_flt_hdl)) {
            p_flt_hdl->attr_dmx_filter.overflow_auto_rst_buf_cnt++;
        }    
        aui_common_rst_ring_buf(p_ringbuf); /** automatic rst */
    }
    return ;    
}

static int aui_get_pes_header(UINT8 *buf)
{
    if ((NULL != buf ) &&
        ((0x0 == buf[0]) && (0x0 == buf[1]) && (0x01 == buf[2]))) {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

static unsigned int aui_get_pes_len(UINT8 *buf)
{
    if (NULL != buf) {
        return (((buf[4] << 8) | buf[5]) + 6);
    }
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_pes_dispatch(aui_hdl chl_hdl, unsigned char *buf, unsigned long size)
{
    aui_dev_priv_data *aui_dev_priv = NULL;
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = NULL;
    struct list_head *cur = NULL;
    struct list_head *head_dev = NULL;
    aui_handle_dmx_filter *flt_hdl = NULL;

    (void)p_hdl_dmx_channel_tmp;
    if (NULL == chl_hdl) {
        return AUI_RTN_FAIL;
    }
    aui_dev_priv = (aui_dev_priv_data *)chl_hdl;
    p_hdl_dmx_channel_tmp = chl_hdl;
    head_dev = (struct list_head *)aui_dev_priv->sub_list.next;
    if (NULL == head_dev) {
        return AUI_RTN_FAIL;
    }
    cur = head_dev;
    p_hdl_dmx_channel_tmp = chl_hdl;
    do {
        if (NULL == cur) {
            return AUI_RTN_FAIL;
        }       
        flt_hdl = (aui_handle_dmx_filter *)cur;
        if ((AUI_DMX_FILTER_RUN == flt_hdl->attr_dmx_filter.dmx_filter_status) &&
            (NULL != flt_hdl->attr_dmx_filter.callback)) {
            flt_hdl->attr_dmx_filter.callback(flt_hdl, buf, size, flt_hdl->attr_dmx_filter.callback_param);
            return AUI_RTN_SUCCESS;
        }
        cur = cur->next;
    } while (cur != NULL);
    return AUI_RTN_FAIL;
}    

static void aui_chl_ali_update_pes(aui_hdl chl_hdl, unsigned long ul_size)
{
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = chl_hdl;
    unsigned char *buf = NULL;

    if (NULL == p_hdl_dmx_channel_tmp->bufaddr) {
        return ;
    }
    buf = &p_hdl_dmx_channel_tmp->bufaddr[p_hdl_dmx_channel_tmp->bufpos];
    p_hdl_dmx_channel_tmp->bufpos += ul_size;
    if (p_hdl_dmx_channel_tmp->bufpos >= p_hdl_dmx_channel_tmp->buflen) {
        p_hdl_dmx_channel_tmp->bufpos = 0;
        p_hdl_dmx_channel_tmp->last_pes_acc = 0;
        p_hdl_dmx_channel_tmp->last_pes_len = 0;
        return ;
    }
    if (aui_get_pes_header(buf)) {
        /** new add for pes head not in the head of internal buffer caused by any unknown err.
            This code block will not be excuted in normal case */
        if ((p_hdl_dmx_channel_tmp->bufpos != ul_size) &&
            (p_hdl_dmx_channel_tmp->last_pes_acc)) {
            /** send last pes packet */
            aui_pes_dispatch((aui_hdl)p_hdl_dmx_channel_tmp,
                             p_hdl_dmx_channel_tmp->bufaddr,
                             p_hdl_dmx_channel_tmp->last_pes_acc);
            /** copy to the head of internal buffer */
            memcpy(p_hdl_dmx_channel_tmp->bufaddr, buf, ul_size);    
        }
        p_hdl_dmx_channel_tmp->bufpos = ul_size;
        p_hdl_dmx_channel_tmp->last_pes_acc = ul_size;
        p_hdl_dmx_channel_tmp->last_pes_len = aui_get_pes_len(buf);
    } else if (p_hdl_dmx_channel_tmp->last_pes_len != 0) {
        p_hdl_dmx_channel_tmp->last_pes_acc += ul_size;            
    }

    if ((p_hdl_dmx_channel_tmp->last_pes_acc >= p_hdl_dmx_channel_tmp->last_pes_len) &&
        (p_hdl_dmx_channel_tmp->last_pes_len != 0)) {
        aui_pes_dispatch((aui_hdl)p_hdl_dmx_channel_tmp,
                         p_hdl_dmx_channel_tmp->bufaddr,
                         p_hdl_dmx_channel_tmp->last_pes_acc);
        p_hdl_dmx_channel_tmp->last_pes_acc = 0;
        p_hdl_dmx_channel_tmp->bufpos = 0;
        p_hdl_dmx_channel_tmp->last_pes_len = 0;
    }
    return ;
}

static void aui_chl_ali_update(aui_hdl chl_hdl, unsigned long ul_size)
{
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = chl_hdl;

    if (NULL == p_hdl_dmx_channel_tmp) {
        return ;
    }
    if (AUI_DMX_DATA_PES == p_hdl_dmx_channel_tmp->attr_dmx_channel.dmx_data_type) {
        if (p_hdl_dmx_channel_tmp->attr_dmx_channel.dmx_channel_pes_callback_support) {
            aui_chl_ali_update_pes(chl_hdl, ul_size);    
        }    
    }
    return ;    
}

static AUI_RTN_CODE aui_chl_ctm_update(aui_hdl chl_hdl, unsigned long ul_size)
{
    aui_dev_priv_data *aui_dev_priv = NULL;
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = NULL;
    struct list_head *cur = NULL;
    struct list_head *head_dev = NULL;
    aui_handle_dmx_filter *flt_hdl = NULL;

    (void)p_hdl_dmx_channel_tmp;
    if (NULL == chl_hdl) {
        return AUI_RTN_FAIL;
    }
    aui_dev_priv = (aui_dev_priv_data *)chl_hdl;
    p_hdl_dmx_channel_tmp = chl_hdl;
    head_dev = (struct list_head *)aui_dev_priv->sub_list.next;
    if (NULL == head_dev) {
        return AUI_RTN_FAIL;
    }
    cur = head_dev;
    p_hdl_dmx_channel_tmp = chl_hdl;
    do {
        if (NULL == cur) {
            return AUI_RTN_FAIL;
        }       
        flt_hdl = (aui_handle_dmx_filter *)cur;
        if ((AUI_DMX_FILTER_RUN == flt_hdl->attr_dmx_filter.dmx_filter_status) &&
            (NULL != flt_hdl->attr_dmx_filter.p_fun_data_up_wtCB)) {
            flt_hdl->attr_dmx_filter.p_fun_data_up_wtCB(flt_hdl, ul_size);
            return AUI_RTN_SUCCESS;
        }
        cur = cur->next;
    } while (cur != NULL);
    return AUI_RTN_FAIL;
}    

extern int mg_fcs_decoder(unsigned char *pindata, int len);

static AUI_RTN_CODE aui_chl_search_flt(aui_hdl chl_hdl,
                                       unsigned char *p_section_buf,
                                       unsigned long ul_section_len)
{
    aui_dev_priv_data *aui_dev_priv = NULL;
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = NULL;
    struct list_head *cur = NULL;
    struct list_head *head_dev = NULL;
    aui_handle_dmx_filter *flt_hdl = NULL;
    int condition1 = 0;
    int condition2 = 0;
    unsigned long ul_cb_cnt = 0;
    (void)p_hdl_dmx_channel_tmp;
    
    if (NULL == chl_hdl) {
        return AUI_RTN_FAIL;
    }
    aui_dev_priv = (aui_dev_priv_data *)chl_hdl;
    p_hdl_dmx_channel_tmp = chl_hdl;
    head_dev = (struct list_head *)aui_dev_priv->sub_list.next;
    //AUI_DBG("resume head flt [%08x]->[%08x]\n", chl_hdl, head_dev);
    if (NULL == head_dev) {
        return AUI_RTN_FAIL;
    }
    cur = head_dev;
    p_hdl_dmx_channel_tmp = chl_hdl;
    do {
        if (NULL == cur) {
            return AUI_RTN_SUCCESS;
        }
        if (0 == aui_hit_flt(cur,p_section_buf,ul_section_len)) {
            flt_hdl = (aui_handle_dmx_filter *)cur;
            if((AUI_DMX_FILTER_RUN == flt_hdl->attr_dmx_filter.dmx_filter_status) &&
               ((NULL != flt_hdl->attr_dmx_filter.p_fun_sectionCB) || 
               (0 != flt_hdl->attr_dmx_filter.dmx_fil_sec_data_sync_get_support))) {
                condition1 = ((flt_hdl->attr_dmx_filter.uc_crc_check) && 
                              (!(mg_fcs_decoder(p_section_buf, ul_section_len))));
                condition2 = (flt_hdl->attr_dmx_filter.uc_continue_capture_flag);
                ul_cb_cnt = (flt_hdl->attr_dmx_filter.uc_hit_sect_cnt);             
                if (condition1) {
                    if (condition2) {
                        flt_hdl->attr_dmx_filter.uc_hit_sect_cnt++;
                        //osal_mutex_unlock(s_callback_mutex);                
                        if (0 != flt_hdl->attr_dmx_filter.dmx_fil_sec_data_sync_get_support) {
                            aui_dmx_put_data_to_rbuf(&(flt_hdl->fil_rbuf), p_section_buf,
                                                     ul_section_len, NULL, flt_hdl);
                        }    
                        /** put data to filter ringbuf end */
                        if (NULL != flt_hdl->attr_dmx_filter.p_fun_sectionCB) {
                            flt_hdl->attr_dmx_filter.p_fun_sectionCB(flt_hdl, p_section_buf, ul_section_len,
                                                                     flt_hdl->attr_dmx_filter.usr_data, NULL); 
                        }
                        //osal_mutex_lock(s_callback_mutex,OSAL_WAIT_FOREVER_TIME);
                    } else if (ul_cb_cnt == 0) {
                        flt_hdl->attr_dmx_filter.uc_hit_sect_cnt++;
                        //osal_mutex_unlock(s_callback_mutex);            
                        if (0 != flt_hdl->attr_dmx_filter.dmx_fil_sec_data_sync_get_support) {
                            aui_dmx_put_data_to_rbuf(&(flt_hdl->fil_rbuf), p_section_buf,
                                                     ul_section_len, NULL, flt_hdl);
                        }    
                        /** put data to filter ringbuf end */
                        if (NULL != flt_hdl->attr_dmx_filter.p_fun_sectionCB) {
                            flt_hdl->attr_dmx_filter.p_fun_sectionCB(flt_hdl, p_section_buf, ul_section_len,
                                                                     flt_hdl->attr_dmx_filter.usr_data, NULL);   
                        }    
                        //osal_mutex_lock(s_callback_mutex,OSAL_WAIT_FOREVER_TIME);                     
                    } else if (ul_cb_cnt > 0) {
                        ;
                    }
                } else if (!(flt_hdl->attr_dmx_filter.uc_crc_check)) {
                    if (condition2) {
                        flt_hdl->attr_dmx_filter.uc_hit_sect_cnt++;
                        //osal_mutex_unlock(s_callback_mutex);            
                        if (0 != flt_hdl->attr_dmx_filter.dmx_fil_sec_data_sync_get_support) {
                            aui_dmx_put_data_to_rbuf(&(flt_hdl->fil_rbuf), p_section_buf,
                                                     ul_section_len, NULL, flt_hdl);
                        }    
                        /** put data to filter ringbuf end */
                        if (NULL != flt_hdl->attr_dmx_filter.p_fun_sectionCB) {
                            flt_hdl->attr_dmx_filter.p_fun_sectionCB(flt_hdl, p_section_buf, ul_section_len,
                                                                     flt_hdl->attr_dmx_filter.usr_data, NULL);   
                        }
                        //osal_mutex_lock(s_callback_mutex,OSAL_WAIT_FOREVER_TIME);
                    } else if (ul_cb_cnt == 0) {
                        flt_hdl->attr_dmx_filter.uc_hit_sect_cnt++;
                        //osal_mutex_unlock(s_callback_mutex);            
                        if (0 != flt_hdl->attr_dmx_filter.dmx_fil_sec_data_sync_get_support) {
                            aui_dmx_put_data_to_rbuf(&(flt_hdl->fil_rbuf), p_section_buf,
                                                     ul_section_len, NULL, flt_hdl);   
                        }    
                        /** put data to filter ringbuf end */
                        if (NULL != flt_hdl->attr_dmx_filter.p_fun_sectionCB) {
                            flt_hdl->attr_dmx_filter.p_fun_sectionCB(flt_hdl, p_section_buf, ul_section_len,
                                                                     flt_hdl->attr_dmx_filter.usr_data, NULL);   
                        }
                        //osal_mutex_lock(s_callback_mutex,OSAL_WAIT_FOREVER_TIME);
                    }
                } else {
                    ;
                }
            }
            osal_task_dispatch_off();
            flt_hdl->busy = 0;
            osal_task_dispatch_on();
        }
        if (NULL != cur) {
            cur = cur->next;
        }
    }while (cur != NULL);
    return AUI_RTN_SUCCESS;
}

#if 0
static AUI_RTN_CODE aui_chl_start_all_flt(aui_hdl chl_hdl)
{
    aui_dev_priv_data *aui_dev_priv = NULL;
    AUI_RTN_CODE aui_rtn_code = AUI_RTN_FAIL;
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = NULL;
    struct list_head *tmp_next = NULL;
    struct list_head *tmp_prev = NULL;
    struct list_head *cur = NULL;
    struct list_head *head_dev = NULL;

    if (NULL == chl_hdl) {
        return AUI_RTN_SUCCESS;
    }
    aui_dev_priv = (aui_dev_priv_data *)chl_hdl;
    head_dev = aui_dev_priv->sub_list.next;
    //AUI_DBG("resume head flt [%08x]->[%08x]\n", chl_hdl, head_dev);

    if (NULL == head_dev) {
        return AUI_RTN_SUCCESS;
    }
    cur = head_dev;
    tmp_next = cur->next;
    tmp_prev = cur->prev;
    do {
        if (NULL == cur) {
            return AUI_RTN_SUCCESS;
        }
        p_hdl_dmx_filter_tmp = cur;
        if (AUI_DMX_FILTER_STOP == p_hdl_dmx_filter_tmp->attr_dmx_filter.dmx_filter_status) {
            //AUI_DBG("resume flt [%08x]\n", cur);
            aui_dmx_filter_start(cur, 1);
        }
        if (NULL != cur) {
            tmp_prev = cur->next;
        }
        cur = cur->next;
        if (NULL != cur) {
            tmp_next = cur->next;
        }
    } while (cur != NULL);
FUNC_END:
    return aui_rtn_code;
}

static AUI_RTN_CODE aui_chl_stop_all_flt(aui_hdl chl_hdl)
{
    aui_dev_priv_data *aui_dev_priv = NULL;
    AUI_RTN_CODE aui_rtn_code = AUI_RTN_FAIL;
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = NULL;
    struct list_head *tmp_next = NULL;
    struct list_head *tmp_prev = NULL;
    struct list_head *cur = NULL;
    struct list_head *head_dev = NULL;

    if (NULL == chl_hdl) {
        return AUI_RTN_SUCCESS;
    }
    aui_dev_priv = (aui_dev_priv_data *)chl_hdl;
    head_dev = aui_dev_priv->sub_list.next;

    if (NULL == head_dev) {
        return AUI_RTN_SUCCESS;
    }
    cur = head_dev;
    do {
        if (NULL== cur) {
            return AUI_RTN_SUCCESS;
        }
        p_hdl_dmx_filter_tmp = cur;
        if (AUI_DMX_FILTER_RUN == p_hdl_dmx_filter_tmp->attr_dmx_filter.dmx_filter_status) {
            //AUI_DBG("pause flt [%08x]\n", cur);
            aui_dmx_filter_stop(cur, 1);
        }
        cur = cur->next;
    } while (cur != NULL);
FUNC_END:
    return aui_rtn_code;
}
#endif //<-- #if 0

static long aui_dmx_data_buf_update(void *channel_hdl, unsigned long ul_size)
{  
    if(AUI_RTN_SUCCESS == aui_chl_ctm_update(channel_hdl, ul_size)) {
        return AUI_RTN_SUCCESS;
    }
    aui_chl_ali_update(channel_hdl, ul_size);
    return AUI_RTN_SUCCESS;
}      

static long aui_dmx_data_buf_req(void *channel_hdl,
                                 unsigned long ul_req_size,
                                 void **pp_req_buf, 
                                 unsigned long *req_buf_size,
                                 struct aui_avsync_ctrl *pst_ctrl_blk)
{
    int ret = 0;
    
    if ((channel_hdl == NULL) || 
        (ul_req_size == 0) || 
        (pp_req_buf == NULL) ||
        (req_buf_size == NULL) ||
        (pst_ctrl_blk == NULL)) {
        return AUI_RTN_FAIL;
    }
    /** Must use aui_chl_dbuf_ctm_req assigned address pp_req_buf first, then can judge *pp_req_buf. */
    ret = aui_chl_dbuf_ctm_req(channel_hdl, ul_req_size, pp_req_buf, req_buf_size, pst_ctrl_blk);
    if ((AUI_RTN_SUCCESS == ret) && (*pp_req_buf)) {
        return AUI_RTN_SUCCESS;
    }
    aui_chl_dbuf_ali_req(channel_hdl, ul_req_size, pp_req_buf, req_buf_size, pst_ctrl_blk);
    return AUI_RTN_SUCCESS;
}

static void aui_dmx_sect_callback(struct get_section_param *pst_get_section)
{
    UINT8* section = NULL;
    UINT32 length = 0;
    aui_handle_dmx_channel *pst_hdl_dmx_channel = NULL;

    if ((NULL == pst_get_section) ||
        (0 == pst_get_section->priv_param) ||
        (pst_get_section->sec_tbl_len == 0)) {
        return ;
    }
    section = pst_get_section->buff;
    length = pst_get_section->sec_tbl_len;
    pst_hdl_dmx_channel = (aui_handle_dmx_channel *)pst_get_section->priv_param;
    /** new add for fil channel close busy detect ,remove callback mutex begin -->*/
    osal_task_dispatch_off();
    if (pst_hdl_dmx_channel->try_close) {
        osal_task_dispatch_on();
        return ;
    }
    pst_hdl_dmx_channel->busy = 1;
    osal_task_dispatch_on();
    /**<-- new add for fil channel close busy detect ,remove callback mutex end */
    /**put data to section type channel buf start -->*/
    if (pst_hdl_dmx_channel->attr_dmx_channel.dmx_channel_sec_sync_get_data_support != 0) {
        aui_dmx_put_data_to_rbuf(&(pst_hdl_dmx_channel->chan_rbuf), section,
                                 length, pst_hdl_dmx_channel, NULL);
    }
    /**<-- put data to section type channel buf stop */
    if (AUI_RTN_SUCCESS == aui_chl_search_flt(pst_hdl_dmx_channel, section, length)) {
        pst_hdl_dmx_channel->busy = 0;
    }
    osal_task_dispatch_off();
    pst_hdl_dmx_channel->busy = 0;
    osal_task_dispatch_on();
    return ;
}

INT32 aui_dmx_data_rec_buf_req(UINT32 channel_hdl, UINT8 **addr, INT32 length, INT32 *indicator)
{
    int ret = 0;
    
    (void)indicator;
    if ((channel_hdl == 0) || (addr == NULL) || (length == 0)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    /** Must use aui_chl_dbuf_ctm_req assigned address addr first, then can judge *addr. 
        Otherwise, DMX cannot record the data. */
    ret = aui_chl_dbuf_ctm_req((aui_hdl)channel_hdl, (unsigned long)length, (void **)addr, (unsigned long *)&length, NULL);
    if ((AUI_RTN_SUCCESS == ret) && (*addr)) {
        return length;
    } else {
        return AUI_RTN_SUCCESS;
    }
}

BOOL aui_dmx_data_rec_buf_update(UINT32 channel_hdl, UINT32 size, UINT16 offset)
{
    (void)offset;
    AUI_DBG("size:0x%x, offset:0x%x\n", size, offset);
    if ( AUI_RTN_SUCCESS == aui_chl_ctm_update((aui_hdl)channel_hdl, size)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static AUI_RTN_CODE aui_dmx_channel_data_async_get(aui_hdl p_hdl_dmx_channel)
{
    unsigned long ul_buf_len = 0;
    struct pvr_rec_io_param r_param;
    aui_handle_dmx_channel *p_hdl_dmx_channel_tmp = NULL;

    if (NULL == p_hdl_dmx_channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    //osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    p_hdl_dmx_channel_tmp = (aui_handle_dmx_channel *)p_hdl_dmx_channel;
    if (OSAL_INVALID_ID==p_hdl_dmx_channel_tmp->dev_mutex_id) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    ul_buf_len = AUI_DMX_ONE_SECTION_LEN_MAX;
    //AUI_DBG("panduan....[%08x][%08x][%08x][%08x]\n", p_hdl_dmx_channel_tmp,
    //    (p_hdl_dmx_channel_tmp->attr_dmx_channel).section_buffer, pul_data_len, p_attr_dmx_channel);
    switch ((p_hdl_dmx_channel_tmp->attr_dmx_channel).dmx_data_type) {
        case AUI_DMX_DATA_SECT: {
            if (NULL == (p_hdl_dmx_channel_tmp->puc_chl_buf)) {
                aui_rtn(AUI_RTN_EINVAL, NULL);    
            }
            p_hdl_dmx_channel_tmp->st_reg_serv.device = p_hdl_dmx_channel_tmp;
            p_hdl_dmx_channel_tmp->st_reg_serv.dmx_data_type = DMX_SEC_DATA;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_pid = (p_hdl_dmx_channel_tmp->attr_dmx_channel).us_pid;
            p_hdl_dmx_channel_tmp->st_reg_serv.pid_scrambled_flag = (p_hdl_dmx_channel_tmp->attr_dmx_channel).is_encrypted;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_id = AUI_DMX_SERV_ID_INVALID;
            p_hdl_dmx_channel_tmp->st_reg_serv.request_write = NULL;
            p_hdl_dmx_channel_tmp->st_reg_serv.update_write = NULL;            
            if (NULL != p_hdl_dmx_channel_tmp->puc_chl_buf_tmp) {
                p_hdl_dmx_channel_tmp->st_reg_serv.param = p_hdl_dmx_channel_tmp->puc_chl_buf_tmp;
            }
            if (NULL == (p_hdl_dmx_channel_tmp->st_reg_serv.param)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            AUI_DBG("st_reg_serv.param = [%08x]\n", (p_hdl_dmx_channel_tmp->st_reg_serv.param));
            MEMSET(((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param)), 0, sizeof(struct get_section_param));
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->continue_get_sec = 1;
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->retrieve_sec_fmt = RETRIEVE_SEC;
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->pid = p_hdl_dmx_channel_tmp->attr_dmx_channel.us_pid;
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->crc_flag = 0;
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->buff = (p_hdl_dmx_channel_tmp->puc_chl_buf);
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->buff_len = ul_buf_len;
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->get_sec_cb = aui_dmx_sect_callback; 
            AUI_DBG("get_sec_cb1 = [%08x][%08x]\n", ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->get_sec_cb, aui_dmx_sect_callback);
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->mask_value = p_hdl_dmx_channel_tmp->st_get_section.mask_value;
            MEMCPY(&(p_hdl_dmx_channel_tmp->st_get_section), (p_hdl_dmx_channel_tmp->st_reg_serv.param), sizeof(struct get_section_param));
            AUI_DBG("get_sec_cb2 = [%08x][%08x]\n", ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->get_sec_cb, aui_dmx_sect_callback);
            ((struct get_section_param *)(p_hdl_dmx_channel_tmp->st_reg_serv.param))->priv_param = (UINT32)p_hdl_dmx_channel_tmp;
            if (RET_SUCCESS != dmx_register_service_new(p_hdl_dmx_channel_tmp->p_hdl_dmx->pst_dev_dmx, &(p_hdl_dmx_channel_tmp->st_reg_serv))) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            break;
        }
        case AUI_DMX_DATA_RAW: {
            p_hdl_dmx_channel_tmp->st_reg_serv.device = p_hdl_dmx_channel_tmp;
            p_hdl_dmx_channel_tmp->st_reg_serv.dmx_data_type = DMX_RAW_DATA;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_pid = (p_hdl_dmx_channel_tmp->attr_dmx_channel).us_pid;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_id = AUI_DMX_SERV_ID_INVALID;
            p_hdl_dmx_channel_tmp->st_reg_serv.request_write = (request_write)aui_dmx_data_buf_req;
            p_hdl_dmx_channel_tmp->st_reg_serv.update_write = (update_write)aui_dmx_data_buf_update;
            p_hdl_dmx_channel_tmp->st_reg_serv.param = NULL;
            if (RET_SUCCESS != dmx_register_service_new(p_hdl_dmx_channel_tmp->p_hdl_dmx->pst_dev_dmx, &(p_hdl_dmx_channel_tmp->st_reg_serv))) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            break;
        }
        case AUI_DMX_DATA_PES: {
            p_hdl_dmx_channel_tmp->st_reg_serv.device = p_hdl_dmx_channel_tmp;
            p_hdl_dmx_channel_tmp->st_reg_serv.dmx_data_type = DMX_PES_DATA;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_pid = (p_hdl_dmx_channel_tmp->attr_dmx_channel).us_pid;
            p_hdl_dmx_channel_tmp->st_reg_serv.pid_scrambled_flag = (p_hdl_dmx_channel_tmp->attr_dmx_channel).is_encrypted;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_id = AUI_DMX_SERV_ID_INVALID;
            p_hdl_dmx_channel_tmp->st_reg_serv.request_write = (request_write)aui_dmx_data_buf_req;
            p_hdl_dmx_channel_tmp->st_reg_serv.update_write = (update_write)aui_dmx_data_buf_update;
            p_hdl_dmx_channel_tmp->st_reg_serv.param = NULL;
            if (RET_SUCCESS != dmx_register_service_new(p_hdl_dmx_channel_tmp->p_hdl_dmx->pst_dev_dmx, &(p_hdl_dmx_channel_tmp->st_reg_serv))) {
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            break;
        }
        case AUI_DMX_DATA_ES: {
            p_hdl_dmx_channel_tmp->st_reg_serv.device = p_hdl_dmx_channel_tmp;
            p_hdl_dmx_channel_tmp->st_reg_serv.dmx_data_type = DMX_ES_DATA;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_pid = (p_hdl_dmx_channel_tmp->attr_dmx_channel).us_pid;
            p_hdl_dmx_channel_tmp->st_reg_serv.service_id = AUI_DMX_SERV_ID_INVALID;
            p_hdl_dmx_channel_tmp->st_reg_serv.request_write = (request_write)aui_dmx_data_buf_req;
            p_hdl_dmx_channel_tmp->st_reg_serv.update_write = (update_write)aui_dmx_data_buf_update;
            p_hdl_dmx_channel_tmp->st_reg_serv.param = NULL;
            if (RET_SUCCESS != dmx_register_service_new(p_hdl_dmx_channel_tmp->p_hdl_dmx->pst_dev_dmx, &(p_hdl_dmx_channel_tmp->st_reg_serv))) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            break;
        }
        case AUI_DMX_DATA_REC: {
            MEMSET(&r_param, 0, sizeof(struct pvr_rec_io_param));
            r_param.io_buff_in = (UINT8 *)(p_hdl_dmx_channel_tmp->attr_dmx_channel).us_pid_list;
            r_param.buff_in_len = (p_hdl_dmx_channel_tmp->attr_dmx_channel).ul_pid_cnt*sizeof(UINT16);
            r_param.hnd = (UINT32)(p_hdl_dmx_channel_tmp);
            //r_param.h264_flag = 0;
            if (AUI_FULL_TS_PID == (p_hdl_dmx_channel_tmp->attr_dmx_channel).us_pid) {
                r_param.record_whole_tp = 1;
            } else {
                r_param.record_whole_tp = 0;
            }
            //r_param.buff_out_len = TRUE;
            r_param.rec_type = 0; //0: TS, 1:PS
            r_param.request = aui_dmx_data_rec_buf_req;
            r_param.update = aui_dmx_data_rec_buf_update;
            if (RET_SUCCESS != dmx_io_control(p_hdl_dmx_channel_tmp->p_hdl_dmx->pst_dev_dmx, CREATE_RECORD_STR_EXT, (UINT32)&r_param)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            break;
        }
        default:
            break;
    }
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE check_dmx_dev_id(unsigned long ul_dev_idx)
{
    /* 
     * check the defined state of the dmx dev id.
     * If support more DMX, need to add it to this judging condition.
     */
    if ((AUI_DMX_ID_DEMUX3 < ul_dev_idx) && (AUI_DMX_ID_SW_DEMUX1 != ul_dev_idx)) {
        AUI_DBG("DMX ID [%ld] is NOT support!\n", ul_dev_idx);
        return AUI_RTN_FAIL;
    }
    /* check the working state of the dmx dev id */
	struct dmx_device *p_dmx_dev = NULL;
	p_dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, ul_dev_idx);
    if (NULL == p_dmx_dev) {
        aui_rtn(AUI_RTN_EINVAL, "DMX ID [%ld] is invalid!\n", ul_dev_idx);
    }
	
	return AUI_RTN_SUCCESS;
}

/****************************MODULE IMPLEMENT*************************************/
AUI_RTN_CODE aui_dmx_version_get(unsigned long * const pul_version)
{
    if (NULL == pul_version) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    *pul_version = AUI_MODULE_VERSION_NUM_DMX;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_init(p_fun_cb p_call_back_init,void *pv_param)
{
    aui_dmx_module_attr *p_mod_attr = (aui_dmx_module_attr *)pv_param;
    
    s_mod_mutex_id_dmx = osal_mutex_create();
    if (OSAL_INVALID_ID == s_mod_mutex_id_dmx) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    s_mutex_id_dmx = osal_mutex_create();
    if (OSAL_INVALID_ID == s_mutex_id_dmx) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    s_mutex_id_dmx_channel = osal_mutex_create();
    if (OSAL_INVALID_ID == s_mutex_id_dmx_channel) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    if ((NULL != p_call_back_init) && (NULL != p_mod_attr)) {
        MEMCPY(&s_dmx_attr, p_mod_attr, sizeof(aui_dmx_module_attr));
        return p_call_back_init(pv_param);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_de_init(p_fun_cb p_call_back_init, void *pv_param)
{
    if (E_OK != osal_mutex_delete(s_mod_mutex_id_dmx)) {
        AUI_ERR("s_mod_mutex_id_dmx delete failed.\n");
    }
    if (E_OK != osal_mutex_delete(s_mutex_id_dmx)) {
        AUI_ERR("s_mutex_id_dmx delete failed.\n");
    }
    if (E_OK != osal_mutex_delete(s_mutex_id_dmx_channel)) {
        AUI_ERR("s_mutex_id_dmx_channel delete failed.\n");
    }
    if (NULL != p_call_back_init) {
        return p_call_back_init(pv_param);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_open(const aui_attr_dmx *p_attr_dmx, aui_hdl *const pp_hdl_dmx)
{
    OSAL_ID dev_mutex_id = OSAL_INVALID_ID;
    aui_handle_dmx *pst_hdl_dmx = NULL;
    
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_dmx, dev_mutex_id, AUI_MODULE_DMX, AUI_RTN_FAIL);
    if ((NULL == p_attr_dmx) || (NULL == pp_hdl_dmx)) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, "Input parameter is NULL!");
    }
    /* check DMX device ID */
    if (check_dmx_dev_id(p_attr_dmx->uc_dev_idx)) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, "DMX device ID [%ld] is invalid!", (unsigned long)(p_attr_dmx->uc_dev_idx));
    }
    pst_hdl_dmx = (aui_handle_dmx *)MALLOC(sizeof(aui_handle_dmx));
    if (NULL == pst_hdl_dmx) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM, "malloc pst_hdl_dmx FAIL!");
    }
    MEMSET(pst_hdl_dmx, 0, sizeof(aui_handle_dmx));
    MEMCPY(&(pst_hdl_dmx->attr_dmx), p_attr_dmx, sizeof(aui_attr_dmx));
    pst_hdl_dmx->pst_dev_dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, pst_hdl_dmx->attr_dmx.uc_dev_idx);
    if (NULL == pst_hdl_dmx->pst_dev_dmx) {
        FREE(pst_hdl_dmx);
        pst_hdl_dmx = NULL;
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_NOT_INIT, "pst_hdl_dmx->pst_dev_dmx is NULL!");
    }
    /** only open once, Yes it is ugly, but work */
    if (pst_hdl_dmx->pst_dev_dmx->flags == DMX_STATE_ATTACH) {
        if (RET_SUCCESS != dmx_open(pst_hdl_dmx->pst_dev_dmx)) {
            FREE(pst_hdl_dmx);
            pst_hdl_dmx = NULL;
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, "dmx_open return FAIL!");
        }
    }
    pst_hdl_dmx->dev_mutex_id = dev_mutex_id;
    (pst_hdl_dmx->dev_priv_data).dev_idx = p_attr_dmx->uc_dev_idx;
    aui_dev_reg(AUI_MODULE_DMX, pst_hdl_dmx);
    pst_hdl_dmx->dev_priv_data.en_status = AUI_DEV_STATUS_OPEN;
    *pp_hdl_dmx = pst_hdl_dmx;
    AUI_DBG("open pp_hdl_dmx = [%08x]\n", pst_hdl_dmx);
    osal_mutex_unlock(dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_close(aui_hdl p_hdl_dmx)
{
    aui_handle_dmx *pst_hdl_dmx = p_hdl_dmx;
    aui_handle_dmx_channel *pst_hdl_dmx_channel = NULL;

    AUI_DBG("dmx cls[%08x] = [%08x]\n", &p_hdl_dmx, p_hdl_dmx);
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == pst_hdl_dmx) ||
        (NULL == pst_hdl_dmx->pst_dev_dmx) ||
        (OSAL_INVALID_ID == pst_hdl_dmx->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(pst_hdl_dmx->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    if (AUI_DEV_STATUS_OPEN != pst_hdl_dmx->dev_priv_data.en_status) {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
#if 0
    // Never close DMX, driver has ugly bug, just keep dmx driver open when system running
    if (RET_SUCCESS != dmx_close(pst_hdl_dmx->pst_dev_dmx)) {
        osal_mutex_unlock(pst_hdl_dmx->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
#endif //<--#if 0
    //osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(pst_hdl_dmx->dev_mutex_id);
    pst_hdl_dmx->dev_priv_data.en_status = AUI_DEV_STATUS_CLOSED;
    if (0 != osal_mutex_delete(pst_hdl_dmx->dev_mutex_id)) {
        aui_rtn(AUI_RTN_FAIL, "delete mutex err.\n");
    }
    
    while (1) {
        pst_hdl_dmx_channel = (aui_handle_dmx_channel *)pst_hdl_dmx->dev_priv_data.sub_list.next;
        if (NULL != pst_hdl_dmx_channel) {
            if (AUI_RTN_SUCCESS != aui_dmx_channel_close((aui_hdl *)&pst_hdl_dmx_channel)) {
                //osal_mutex_unlock(s_mod_mutex_id_dmx);
                aui_rtn(AUI_RTN_FAIL, "delete filter handle list err.\n");
            }
        } else {
            break;
        }
    }
    aui_dev_unreg(AUI_MODULE_DMX, pst_hdl_dmx);
    MEMSET(p_hdl_dmx, 0, sizeof(aui_handle_dmx));
    FREE(p_hdl_dmx);
    p_hdl_dmx = NULL;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_dev_cnt_get(unsigned long* const pul_dmx_cnt)
{
    if (NULL == pul_dmx_cnt) {
        aui_rtn(AUI_RTN_EINVAL, "Input parameter ERROR!");
    }
    
    int dev_index = 0;
    int dmx_count = 0;
    
    /* check dmx */
    for (dev_index = 0; dev_index < AUI_DMX_NB_DEMUX; dev_index++) {
        if (AUI_RTN_SUCCESS == check_dmx_dev_id(dev_index)) {
            dmx_count++;
        }
    }

    *pul_dmx_cnt = dmx_count;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_capability_get(unsigned long ul_dev_idx,
                                    aui_dmx_dev_capability *const p_capability_info)
{
    if ((check_dmx_dev_id(ul_dev_idx)) || (NULL == p_capability_info)) {
        aui_rtn(AUI_RTN_EINVAL, "Input parameter ERROR!");
    }

    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    aui_dmx_dev_capability *p_info = p_capability_info;
    memset(p_info, 0, sizeof(aui_dmx_dev_capability));

    p_info->ul_dev_idx = ul_dev_idx;

    switch (ul_dev_idx) {
        case AUI_DMX_ID_DEMUX0:
        case AUI_DMX_ID_DEMUX1:
        case AUI_DMX_ID_DEMUX2:
        case AUI_DMX_ID_DEMUX3: {
            p_info->dev_type = AUI_DMX_DEV_TYPE_HARDWARE;
            rtn_code = AUI_RTN_SUCCESS;
            break;
		}
	    case AUI_DMX_ID_SW_DEMUX0:
	    case AUI_DMX_ID_SW_DEMUX1: {
            p_info->dev_type = AUI_DMX_DEV_TYPE_SOFTWARE;
            rtn_code = AUI_RTN_SUCCESS;
		    break;
		}
        default : {
            p_info->dev_type = AUI_DMX_DEV_TYPE_LAST;
            AUI_ERR("DMX ID [%ld] is invalid!\n", ul_dev_idx);
            rtn_code = AUI_RTN_FAIL;
            break;
        }
	}

    return rtn_code;
}

AUI_RTN_CODE aui_dmx_data_path_set(aui_hdl p_hdl_dmx, const aui_dmx_data_path *p_dmx_data_path_info)
{
    if ((NULL == p_hdl_dmx)||(NULL == p_dmx_data_path_info)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    aui_handle_dmx *pst_hdl_dmx = p_hdl_dmx;
    aui_handle_dsc *pst_aui_dsc_hdl = NULL;
    aui_attr_dsc attr_dsc_tmp;
    MEMSET(&attr_dsc_tmp, 0, sizeof(aui_attr_dsc));
    struct dec_parse_param st_descramble_hdl;
    MEMSET(&st_descramble_hdl, 0, sizeof(struct dec_parse_param));
    int b_encrypt = 0;    
    MEMSET(&st_de_en_cfg, 0, sizeof(DEEN_CONFIG));
    
    switch (p_dmx_data_path_info->data_path_type) {
        case AUI_DMX_DATA_PATH_CLEAR_PLAY: {
            st_descramble_hdl.dec_dev = NULL;
            st_descramble_hdl.type = AUI_DMX_DSC_ALGO_INVALID;
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx, IO_SET_DEC_HANDLE,
                                              (unsigned long)&st_descramble_hdl)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dmx_io_control(pst_hdl_dmx->pst_dev_dmx, IO_SET_DEC_STATUS, 0);
            break;
        }
        case AUI_DMX_DATA_PATH_REC: {
            break;
        }
        case AUI_DMX_DATA_PATH_DE_PLAY: {
            pst_aui_dsc_hdl = p_dmx_data_path_info->p_hdl_de_dev;
            st_descramble_hdl.type = pst_aui_dsc_hdl->attr_dsc.uc_algo;
            if ((AUI_DSC_ALGO_TDES == st_descramble_hdl.type) ||
                (AUI_DSC_ALGO_DES == st_descramble_hdl.type)) {
                AUI_DBG("de algo: DES\n");
                st_descramble_hdl.dec_dev = pst_aui_dsc_hdl->p_dev_des;
                st_descramble_hdl.type = AUI_DSC_ALGO_DES;
            } else if (AUI_DSC_ALGO_AES == st_descramble_hdl.type) {
                AUI_DBG("de algo: AES\n");
                st_descramble_hdl.dec_dev = pst_aui_dsc_hdl->p_dev_aes;
            } else if (AUI_DSC_ALGO_CSA == st_descramble_hdl.type) {
                AUI_DBG("de algo: CSA\n");
                st_descramble_hdl.dec_dev = pst_aui_dsc_hdl->p_dev_csa;
            } else {
                AUI_ERR("de algo %d: invalid\n", st_descramble_hdl.type);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx, IO_SET_DEC_HANDLE,
                                              (unsigned long)&st_descramble_hdl)) {
                AUI_ERR("IO_SET_DEC_HANDLE fail\n");
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx, IO_SET_DEC_STATUS, 1)) {
                AUI_ERR("IO_SET_DEC_STATUS fail\n");
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            pst_aui_dsc_hdl->pv_live_dsc_hdl = pst_aui_dsc_hdl;
            aui_dsc_set_playback(pst_aui_dsc_hdl);
            break;
        }
        case AUI_DMX_DATA_PATH_EN_REC:
        {
            pst_aui_dsc_hdl = p_dmx_data_path_info->p_hdl_en_dev;
            attr_dsc_tmp = pst_aui_dsc_hdl->attr_dsc;

            b_encrypt = 1;
            if (AUI_RTN_SUCCESS != aui_dmx_set(p_hdl_dmx, AUI_DMX_SET_IO_REC_MODE, (void *)&b_encrypt)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            st_de_en_cfg.do_decrypt = 0;
            st_de_en_cfg.dec_dev = NULL;
            st_de_en_cfg.decrypt_mode = (UINT8)AUI_DMX_DSC_ALGO_INVALID;
            st_de_en_cfg.dec_dmx_id = 0;
            st_de_en_cfg.do_encrypt = 1;
            st_de_en_cfg.enc_dmx_id = pst_aui_dsc_hdl->ul_stream_id;
            if (AUI_DSC_ALGO_AES == attr_dsc_tmp.uc_algo) {
                st_de_en_cfg.enc_dev = pst_aui_dsc_hdl->p_dev_aes;
                st_de_en_cfg.encrypt_mode = AUI_DSC_ALGO_AES;
            } else if (AUI_DSC_ALGO_TDES == attr_dsc_tmp.uc_algo) {
                st_de_en_cfg.enc_dev = pst_aui_dsc_hdl->p_dev_des;
                st_de_en_cfg.encrypt_mode = AUI_DSC_ALGO_TDES;
            }
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                              IO_SET_DEC_CONFIG,
                                              (unsigned long)&st_de_en_cfg)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (pst_aui_dsc_hdl->pvr_crypto_mode) { /** pvr record in block mode */
                if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                                  DMX_SET_REC_OUT_MODE,
                                                  DMX_SET_REC_OUT_BLOCK)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            } else {                                /** set record mode */
                if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                                  DMX_SET_REC_OUT_MODE,
                                                  DMX_SET_REC_OUT_TS)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            }

            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                              IO_SET_FTA_REC_MODE,
                                              FTA_TO_ENCRYPT)){
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            break;
        }
        case AUI_DMX_DATA_PATH_DE_PLAY_EN_REC: {
            pst_aui_dsc_hdl = p_dmx_data_path_info->p_hdl_de_dev;
            st_de_en_cfg.do_decrypt = 1;
            st_de_en_cfg.dec_dmx_id = pst_aui_dsc_hdl->ul_stream_id;
            st_descramble_hdl.type = pst_aui_dsc_hdl->attr_dsc.uc_algo;
            if ((AUI_DSC_ALGO_TDES == pst_aui_dsc_hdl->attr_dsc.uc_algo) ||
                (AUI_DSC_ALGO_DES == pst_aui_dsc_hdl->attr_dsc.uc_algo)) {
                AUI_DBG("de algo: DES\n");
                st_de_en_cfg.dec_dev = pst_aui_dsc_hdl->p_dev_des;
                st_de_en_cfg.decrypt_mode = AUI_DSC_ALGO_DES;
                st_descramble_hdl.dec_dev = pst_aui_dsc_hdl->p_dev_des;
                st_descramble_hdl.type = AUI_DSC_ALGO_DES;
            } else if (AUI_DSC_ALGO_AES == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("de algo: AES\n");
                st_de_en_cfg.dec_dev = pst_aui_dsc_hdl->p_dev_aes;
                st_de_en_cfg.decrypt_mode = AUI_DSC_ALGO_AES;
                st_descramble_hdl.dec_dev = pst_aui_dsc_hdl->p_dev_aes;
            } else if (AUI_DSC_ALGO_CSA == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("de algo: CSA\n");
                st_de_en_cfg.dec_dev = pst_aui_dsc_hdl->p_dev_csa;
                st_de_en_cfg.decrypt_mode = AUI_DSC_ALGO_CSA;
                st_descramble_hdl.dec_dev = pst_aui_dsc_hdl->p_dev_csa;
            } else {
                AUI_ERR("algo %d is not defined\n", pst_aui_dsc_hdl->attr_dsc.uc_algo);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }

            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx, IO_SET_DEC_HANDLE, (unsigned long)&st_descramble_hdl)) {
                AUI_ERR("IO_SET_DEC_HANDLE fail\n");
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx, IO_SET_DEC_STATUS, 1)) {
                AUI_ERR("IO_SET_DEC_STATUS fail\n");
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            pst_aui_dsc_hdl->pv_live_dsc_hdl = pst_aui_dsc_hdl;
            aui_dsc_set_playback(pst_aui_dsc_hdl);
            pst_aui_dsc_hdl = p_dmx_data_path_info->p_hdl_en_dev;
            st_de_en_cfg.do_encrypt = 1;
            st_de_en_cfg.enc_dmx_id = pst_aui_dsc_hdl->ul_stream_id;
            if (AUI_DSC_ALGO_AES == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("en algo: AES\n");
                st_de_en_cfg.enc_dev = pst_aui_dsc_hdl->p_dev_aes;
                st_de_en_cfg.encrypt_mode = AUI_DSC_ALGO_AES;
            } else if (AUI_DSC_ALGO_TDES == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("en algo: DES\n");
                st_de_en_cfg.enc_dev = pst_aui_dsc_hdl->p_dev_des;
                st_de_en_cfg.encrypt_mode = AUI_DSC_ALGO_TDES;
            } else {
                AUI_ERR("invalid encrypt algo is :%d\n", pst_aui_dsc_hdl->attr_dsc.uc_algo);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                              IO_SET_DEC_CONFIG,
                                              (unsigned long)&st_de_en_cfg)) {
                AUI_ERR("IO_SET_DEC_CONFIG fail\n");
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (pst_aui_dsc_hdl->pvr_crypto_mode) { /** pvr record in block mode */
                if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                                  DMX_SET_REC_OUT_MODE,
                                                  DMX_SET_REC_OUT_BLOCK)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            } else {                                /** set record mode */
                if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                                  DMX_SET_REC_OUT_MODE,
                                                  DMX_SET_REC_OUT_TS)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            }
            break;
        }
        case AUI_DMX_DATA_PATH_REEN_REC: {
            pst_aui_dsc_hdl = p_dmx_data_path_info->p_hdl_de_dev;
            st_de_en_cfg.do_decrypt = 1;
            st_de_en_cfg.dec_dmx_id = pst_aui_dsc_hdl->ul_stream_id;
            if ((AUI_DSC_ALGO_TDES == pst_aui_dsc_hdl->attr_dsc.uc_algo) ||
                (AUI_DSC_ALGO_DES == pst_aui_dsc_hdl->attr_dsc.uc_algo)) {
                AUI_DBG("de algo: DES\n");
                st_de_en_cfg.dec_dev = pst_aui_dsc_hdl->p_dev_des;
                st_de_en_cfg.decrypt_mode = AUI_DSC_ALGO_DES;
            } else if (AUI_DSC_ALGO_AES == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("de algo: AES\n");
                st_de_en_cfg.dec_dev = pst_aui_dsc_hdl->p_dev_aes;
                st_de_en_cfg.decrypt_mode = AUI_DSC_ALGO_AES;
            } else if (AUI_DSC_ALGO_CSA == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("de algo: CSA\n");
                st_de_en_cfg.dec_dev = pst_aui_dsc_hdl->p_dev_csa;
                st_de_en_cfg.decrypt_mode = AUI_DSC_ALGO_CSA;
            } else {
                AUI_ERR("algo %d is not defined\n", pst_aui_dsc_hdl->attr_dsc.uc_algo);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            
            pst_aui_dsc_hdl = p_dmx_data_path_info->p_hdl_en_dev;
            st_de_en_cfg.do_encrypt = 1;
            st_de_en_cfg.enc_dmx_id = pst_aui_dsc_hdl->ul_stream_id;
            if (AUI_DSC_ALGO_AES == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("en algo: AES\n");
                st_de_en_cfg.enc_dev = pst_aui_dsc_hdl->p_dev_aes;
                st_de_en_cfg.encrypt_mode = AUI_DSC_ALGO_AES;
            } else if (AUI_DSC_ALGO_TDES == pst_aui_dsc_hdl->attr_dsc.uc_algo) {
                AUI_DBG("en algo: DES\n");
                st_de_en_cfg.enc_dev = pst_aui_dsc_hdl->p_dev_des;
                st_de_en_cfg.encrypt_mode = AUI_DSC_ALGO_TDES;
            } else {
                AUI_ERR("invalid encrypt algo is :%d\n", pst_aui_dsc_hdl->attr_dsc.uc_algo);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                              IO_SET_DEC_CONFIG,
                                              (unsigned long)&st_de_en_cfg)) {
                AUI_ERR("IO_SET_DEC_CONFIG fail\n");
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (pst_aui_dsc_hdl->pvr_crypto_mode) { /** pvr record in block mode */
                if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                                  DMX_SET_REC_OUT_MODE,
                                                  DMX_SET_REC_OUT_BLOCK)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            } else {                                /** set record mode */
                if (RET_SUCCESS != dmx_io_control(pst_hdl_dmx->pst_dev_dmx,
                                                  DMX_SET_REC_OUT_MODE,
                                                  DMX_SET_REC_OUT_TS)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            }
            break;
        }
        default:
            break;
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_start(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{   
    aui_handle_dmx *p_aui_dmx = NULL;
    struct dmx_device *p_hld_dmx = NULL;
    (void)p_attr_dmx;
    
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    p_aui_dmx = (aui_handle_dmx *)p_hdl_dmx;
    p_hld_dmx = p_aui_dmx->pst_dev_dmx;
    osal_mutex_lock(p_aui_dmx->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if (DMX_STATE_PLAY == p_hld_dmx->flags) {
        osal_mutex_unlock(p_aui_dmx->dev_mutex_id);
        return AUI_RTN_SUCCESS;
    }
    if (RET_SUCCESS != dmx_start(p_hld_dmx)) {
        osal_mutex_unlock(p_aui_dmx->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    osal_mutex_unlock(p_aui_dmx->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_stop(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    aui_handle_dmx *p_aui_dmx = NULL;
    struct dmx_device *p_hld_dmx = NULL;
    (void)p_attr_dmx;
    
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    p_aui_dmx = (aui_handle_dmx *)p_hdl_dmx;
    p_hld_dmx = p_aui_dmx->pst_dev_dmx;
    
    osal_mutex_lock(p_aui_dmx->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if (DMX_STATE_IDLE == p_hld_dmx->flags) {
        osal_mutex_unlock(p_aui_dmx->dev_mutex_id);
        return AUI_RTN_SUCCESS;
    }
    if (RET_SUCCESS != dmx_stop(p_aui_dmx->pst_dev_dmx)) {
        osal_mutex_unlock(p_aui_dmx->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    osal_mutex_unlock(p_aui_dmx->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_pause(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    (void)p_attr_dmx;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if ((NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx)) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    if (RET_SUCCESS != dmx_pause(((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx)) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_resume(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    unsigned long ul_rtn = AUI_RTN_FAIL;
    (void)p_attr_dmx;
    
    osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if (RET_SUCCESS == dmx_start(((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx)) {
        ul_rtn = AUI_RTN_SUCCESS;
    }
    //ul_rtn=aui_dmx_start(p_hdl_dmx, p_attr_dmx);
    if (AUI_RTN_SUCCESS != ul_rtn) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        return ul_rtn;
    }
    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

#ifndef _AUI_OTA_
static unsigned short find_pid(struct aui_dmx_stream_pid *pid_info, aui_dmx_stream_type type)
{
    unsigned long i = 0;

    for (i = 0; i < pid_info->ul_pid_cnt; i++) {
        if (pid_info->stream_types[i] == type) {
            /** effective PID is 13bits, The rang of effective PID is [0, 0x1fff).
                add the condition to prevent the misuse of PID. */
            if (AUI_INVALID_PID < pid_info->aus_pids_val[i]) {
                return AUI_INVALID_PID;
            } else {
                return pid_info->aus_pids_val[i];
            }
        }
    }
    return AUI_INVALID_PID;
}
#endif //<--_AUI_OTA_

AUI_RTN_CODE aui_dmx_set(aui_hdl p_hdl_dmx, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
#ifndef _AUI_OTA_
    aui_hdl deca_hdl = 0;
    enum aui_deca_stream_type aui_get_pul_deca_cur_type = AUI_DECA_STREAM_TYPE_MPEG1;
    unsigned short aud_pid;
#endif //<--_AUI_OTA_
    
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);


    if (ul_item >= AUI_DMX_SET_CMD_LAST) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DMX_TYPE_SET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_CREATE_AV:
        case AUI_DMX_STREAM_CREATE_VIDEO:
        case AUI_DMX_STREAM_CREATE_AUDIO:
        {
#ifndef _AUI_OTA_
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            struct aui_dmx_stream_pid *p_pids_info = (struct aui_dmx_stream_pid *)pv_param;
            struct io_param io_parameter;
            aui_dmx_stream_pid pid_list;
            int dmx_io_cmd = IO_CREATE_AV_STREAM;
            MEMSET(&io_parameter, 0, sizeof(struct io_param));    
            MEMSET(&pid_list, 0, sizeof(aui_dmx_stream_pid));

            if (p_pids_info->ul_pid_cnt > AUI_DMX_STREAM_PIDS_CNT_MAX) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (AUI_DMX_STREAM_CREATE_AUDIO == ul_item) {
                dmx_io_cmd = IO_CREATE_AUDIO_STREAM;
            } else if (AUI_DMX_STREAM_CREATE_VIDEO == ul_item) {
                dmx_io_cmd = IO_CREATE_VIDEO_STREAM;
            }

            pid_list.ul_pid_cnt = 4;
            pid_list.ul_magic_num = p_pids_info->ul_magic_num;

            pid_list.aus_pids_val[0] = find_pid(p_pids_info, AUI_DMX_STREAM_VIDEO);
            pid_list.aus_pids_val[1] = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO);
            pid_list.aus_pids_val[2] = find_pid(p_pids_info, AUI_DMX_STREAM_PCR);
            pid_list.aus_pids_val[3] = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO_DESC);
            if (pid_list.aus_pids_val[3] == pid_list.aus_pids_val[1])
                pid_list.aus_pids_val[3] = AUI_INVALID_PID;
            if ((AUI_DMX_STREAM_CREATE_AUDIO == ul_item) ||
                (AUI_DMX_STREAM_CREATE_AV == ul_item)) {
                if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl)) {        
                    return AUI_RTN_FAIL;
                }
                if (AUI_RTN_SUCCESS != aui_deca_type_get(deca_hdl, (unsigned long *const)&aui_get_pul_deca_cur_type)) {
                    AUI_ERR("deca_open_get_type failed!\n");
                }
                dmx_get_audio_pid(aui_get_pul_deca_cur_type, pid_list.aus_pids_val[1], &aud_pid);
                pid_list.aus_pids_val[1] = aud_pid;
                
                /** support audio description
                    audio description type must be same as main audio type, or else the ts stream is wrong. */
                if (AUI_INVALID_PID != pid_list.aus_pids_val[3]) {
                    dmx_get_audio_pid(aui_get_pul_deca_cur_type,pid_list.aus_pids_val[3], &aud_pid);
                    pid_list.aus_pids_val[3] = aud_pid;
                }
                //AUI_DBG("audio pid: 0x%x, ad pid: 0x%x\n", pid_list.aus_pids_val[1], pid_list.aus_pids_val[3]);
            }

            if ((AUI_DMX_STREAM_CREATE_VIDEO == ul_item) ||
                (AUI_DMX_STREAM_CREATE_AV == ul_item)) {
                dmx_get_video_pid(g_aui_decv_format, pid_list.aus_pids_val[0], &pid_list.aus_pids_val[0]);
            }
    
            io_parameter.buff_in_len = pid_list.ul_pid_cnt;
            io_parameter.io_buff_in = (UINT8 *)pid_list.aus_pids_val;
            io_parameter.buff_out_len = pid_list.ul_magic_num;
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               dmx_io_cmd,
                                               (unsigned long)&io_parameter)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
#endif //<--_AUI_OTA_
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }        
        case AUI_DMX_STREAM_ENABLE: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               IO_STREAM_ENABLE,
                                               (unsigned long)NULL)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_ENABLE_AUDIO: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               AUDIO_STREAM_ENABLE,
                                               (unsigned long)NULL)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_ENABLE_VIDEO: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               VIDEO_STREAM_ENABLE,
                                               (unsigned long)NULL)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_DISABLE_AUDIO: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               AUDIO_STREAM_DISABLE,
                                               (unsigned long)pv_param)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_DISABLE_VIDEO: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               VIDEO_STREAM_DISABLE,
                                               (unsigned long)pv_param)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_DISABLE: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               IO_STREAM_DISABLE,
                                               (unsigned long)pv_param)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_BYPASS_CSA: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               DMX_BYPASS_CSA,
                                               (unsigned long)NULL)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_AVSYNC_MODE: {
            if (AUI_DMX_AVSYNC_TSG_LIVE == (enum aui_dmx_avsync_mode)pv_param) {
                if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                                   SET_TSG_PLAYBACK,
                                                   (unsigned long)1)) {
                    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            } else {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_CHANGE_AUD_STREM: {
#ifndef _AUI_OTA_
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            struct aui_dmx_stream_pid *p_pids_info = (struct aui_dmx_stream_pid *)pv_param;
            struct io_param io_parameter;
            aui_dmx_stream_pid pid_list;
            MEMSET(&io_parameter, 0, sizeof(struct io_param));
            MEMSET(&pid_list, 0, sizeof(aui_dmx_stream_pid));
            
            pid_list.ul_pid_cnt = 4;
            pid_list.ul_magic_num = p_pids_info->ul_magic_num;

            pid_list.aus_pids_val[0] = AUI_INVALID_PID;
            pid_list.aus_pids_val[1] = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO);
            pid_list.aus_pids_val[2] = AUI_INVALID_PID;
            pid_list.aus_pids_val[3] = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO_DESC);
            if (pid_list.aus_pids_val[3] == pid_list.aus_pids_val[1]) {
                pid_list.aus_pids_val[3] = AUI_INVALID_PID;
            }
            if (p_pids_info->ul_pid_cnt > AUI_DMX_STREAM_PIDS_CNT_MAX) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if(aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl)) {
                return -1;
            }
            if (AUI_RTN_SUCCESS != aui_deca_type_get(deca_hdl, (unsigned long* const)&aui_get_pul_deca_cur_type)) {
                AUI_ERR("deca_open_get_type failed!\n");
            }
            dmx_get_audio_pid(aui_get_pul_deca_cur_type, pid_list.aus_pids_val[1], &aud_pid);
            pid_list.aus_pids_val[1] = aud_pid;

            if (AUI_INVALID_PID != pid_list.aus_pids_val[3]) {
                dmx_get_audio_pid(aui_get_pul_deca_cur_type, pid_list.aus_pids_val[3], &aud_pid);
                pid_list.aus_pids_val[3] = aud_pid;
            }
            io_parameter.buff_in_len = pid_list.ul_pid_cnt;
            io_parameter.io_buff_in = (UINT8 *)pid_list.aus_pids_val;
            io_parameter.buff_out_len = pid_list.ul_magic_num;
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                                IO_CHANGE_AUDIO_STREAM,
                                                (unsigned long)&io_parameter)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
#endif //<--_AUI_OTA_
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_IO_REC_MODE: {
            if (*(int *)pv_param == 0) {
                if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx), 
                                                    IO_SET_FTA_REC_MODE,
                                                    FTA_TO_FTA))
                {
                    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                    aui_rtn(AUI_RTN_FAIL, NULL);
                } else {
                    rtn_code = AUI_RTN_SUCCESS;
                }
            } else if (*(int *)pv_param == 1) {
                if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx), 
                                                   IO_SET_FTA_REC_MODE,
                                                   FTA_TO_ENCRYPT)) {
                    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                    aui_rtn(AUI_RTN_FAIL, NULL);
                } else {
                    rtn_code = AUI_RTN_SUCCESS;
                }
            }
            break;
        }
        case AUI_DMX_SET_AVSYNC_SOURCE_TYPE: { /** same as AUI_DMX_SET_AVSYNC_MODE */
            if (AUI_AVSYNC_FROM_TUNER != (enum aui_avsync_srctype)pv_param) {
                if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                                   SET_TSG_PLAYBACK,
                                                   (unsigned long)1)) {
                    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            } else {   /** when is AUI_AVSYNC_FROM_TUNER, set SET_TSG_PLAYBACK to 0 */
                if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                                   SET_TSG_PLAYBACK,
                                                   (unsigned long)0)) {
                    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    if (AUI_RTN_SUCCESS != rtn_code) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_get(aui_hdl p_hdl_dmx, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    //unsigned long ul_rtn=RET_FAILURE;
    unsigned int pts_param = 0;
    aui_pts *pts_tmp = NULL;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    if (ul_item > AUI_DMX_GET_CMD_LAST) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    switch (ul_item) {
        case AUI_DMX_TYPE_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_HANDLE_SIZE_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = (unsigned long)sizeof(aui_handle_dmx);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_CHANNEL_HANDLE_SIZE_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = (unsigned long)sizeof(aui_handle_dmx_channel);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_HANDLE_SIZE_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = (unsigned long)sizeof(aui_handle_dmx_filter);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_ATTR_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            //pv_param = &(((aui_handle_dmx *)p_hdl_dmx)->attr_dmx);
            MEMCPY((aui_attr_dmx *)pv_param, &(((aui_handle_dmx *)p_hdl_dmx)->attr_dmx), sizeof(aui_attr_dmx));
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_RCV_PKG_CNT_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               IO_DMX_GET_PACKET_NUM,
                                               (unsigned long)pv_param)) {
                *(unsigned long *)pv_param = 0;
                rtn_code = AUI_RTN_FAIL;

            } else {
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        case AUI_DMX_RCV_TS_PKG_CNT_GET_BY_PID: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            aui_dmx_get_ts_pkg_cnt_by_pid *p_get_para = (aui_dmx_get_ts_pkg_cnt_by_pid *)pv_param;
            struct io_param param;
            MEMSET(&param, 0, sizeof(struct io_param));
            
            param.buff_in_len = 4;
            param.io_buff_in = s_ac_buffer_in;
            MEMSET(param.io_buff_in, 0, param.buff_in_len);
            param.buff_out_len = 4;
            param.io_buff_out = s_ac_buffer_out;
            MEMSET(param.io_buff_out, 0, param.buff_out_len);
            MEMCPY(param.io_buff_in, &(p_get_para->ul_pid), 4);
            //param.io_buff_in = &(p_get_para->ul_pid); //(unsigned char *) (&((unsigned char)(p_get_para->ul_pid)));
            //param.io_buff_out = &(p_get_para->ul_ts_pkg_cnt); //(unsigned char *) (&((unsigned char)(p_get_para->ul_ts_pkg_cnt)));
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               DMX_IO_GET_TS_PACKET_NUM_BY_PID,
                                               (unsigned long)&param)) {
                *(unsigned long *)pv_param = 0;
                rtn_code = AUI_RTN_FAIL;

            } else {
                p_get_para->ul_ts_pkg_cnt = *(unsigned int *)(param.io_buff_out);
                rtn_code = AUI_RTN_SUCCESS;
            }
            //FREE(param.io_buff_in);
            //param.io_buff_in=0;
            //FREE(param.io_buff_out);
            //param.io_buff_out=0;
            break;
        }
        case AUI_DMX_GET_FREE_BUF_LEN: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               CHECK_DMX_REMAIN_BUF,
                                               (unsigned long)pv_param)) {
                *(unsigned long *)pv_param = 0;
                rtn_code = AUI_RTN_FAIL;

            } else {
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        case AUI_DMX_GET_IO_REC_MODE: {
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx), 
                                               IO_GET_FTA_REC_MODE,
                                               (UINT32)pv_param)) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (*(int *)pv_param == FTA_TO_FTA) {
                *(int *)pv_param = 0;
                rtn_code = AUI_RTN_SUCCESS;
                break;
            }
            if (*(int *)pv_param == FTA_TO_ENCRYPT) {
                *(int *)pv_param = 1;
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        
        case AUI_DMX_GET_PTS: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            pts_tmp = (aui_pts*)pv_param;
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               DMX_GET_CUR_STC,
                                               (unsigned int)(&pts_param))) {
                pts_tmp->pts_1msb = 0;
                pts_tmp->pts_32lsb = 0;
                rtn_code = AUI_RTN_FAIL;
            } else {    
                if (pts_param >= 0x80000000) {
                    pts_tmp->pts_1msb = 1;
                } else {
                    pts_tmp->pts_1msb = 0;
                }
                pts_tmp->pts_32lsb = (pts_param<<1) & 0xffffffff;
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        case AUI_DMX_GET_STREAM_ENCRYPT_INFO: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "The input parameter is NULL!\n");
            }
            aui_dmx_stream_encrypt_type stream_encrypt_type = AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM;
            rtn_code = dmx_io_control(((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx,
                                        IS_AV_SOURCE_SCRAMBLED,
                                        (unsigned long)(&stream_encrypt_type));
            if (RET_SUCCESS != rtn_code) {
                stream_encrypt_type = AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM;
                AUI_ERR("dmx_io_control IS_AV_SOURCE_SCRAMBLED return FAIL!\n");
            }
            *(unsigned long *)pv_param = stream_encrypt_type;
            AUI_DBG("aui_en_dmx_stream_encrypt_type: %d\n", *(unsigned long *)pv_param);
            break;
        }
        case AUI_DMX_GET_STREAM_PID_DECRYPT_INFO: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "The input parameter is NULL!\n");
            }
            aui_dmx_stream_pid_decrypt_info *p_decrypt_info = (aui_dmx_stream_pid_decrypt_info *)pv_param;
            AUI_DBG("ul_pids: pid[0] = %d, pid[1] = %d\n", p_decrypt_info->ul_pids[0], p_decrypt_info->ul_pids[1]);
            
            /* Because SEE code limitation, should be set a non-zero value to get real encrypt_type */
            unsigned long ul_encrypt_type = AUI_DMX_STREAM_ENCRYPT_TYPE_LAST;
            struct io_param param;
            MEMSET(&param, 0, sizeof(param));
            param.io_buff_in = (UINT8 *)(&(p_decrypt_info->ul_pids));
            param.io_buff_out = (UINT8 *)(&ul_encrypt_type);
            dmx_io_control(((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx,
                                        IS_AV_SCRAMBLED_EXT,
                                        (unsigned long)(&param));
            /* If the input value is not changed, may be decryption fail, may also be clear stream */
            if (AUI_DMX_STREAM_ENCRYPT_TYPE_LAST == ul_encrypt_type) {
                p_decrypt_info->ul_result = AUI_DMX_STREAM_DECRYPT_FAIL;
                p_decrypt_info->ul_info = AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM;
            } else {
                p_decrypt_info->ul_result = AUI_DMX_STREAM_DECRYPT_SUCCESS;
                p_decrypt_info->ul_info = ul_encrypt_type & 0xff;
            }
            AUI_DBG("ul_result = %ld, ul_info = %ld\n", p_decrypt_info->ul_result, p_decrypt_info->ul_info);
            break;
        }
        case AUI_DMX_GET_CUR_STC: {      
#ifdef DMX_GET_CUR_STC
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               DMX_GET_CUR_STC,
                                               (unsigned long)pv_param)) {
                *(unsigned long *)pv_param = 0;
                rtn_code = AUI_RTN_FAIL;
            }
            rtn_code = AUI_RTN_SUCCESS;
#else
            rtn_code = AUI_RTN_FAIL;
#endif //<--DMX_GET_CUR_STC
            break;
        }
        case AUI_DMX_GET_PROG_BITRATE: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               DMX_GET_CUR_PROG_BITRATE,
                                               (unsigned long)pv_param)) {
                *(unsigned long *)pv_param = 0;
                rtn_code = AUI_RTN_FAIL;
            } else {
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        case AUI_DMX_GET_DISCONTINUE_PKG_CNT: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            if (RET_SUCCESS != dmx_io_control((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                               IO_GET_DISCONTINUE_COUNT,
                                               (unsigned long)pv_param)) {
                *(unsigned int *)pv_param = 0;
                rtn_code = AUI_RTN_FAIL;
            } else {
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        default: {
            osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    if (AUI_RTN_SUCCESS != rtn_code) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_add_pid(aui_hdl hdl_dmx_channel, const aui_dmx_channel_pid_list *p_channel_pid_list)
{
    (void)hdl_dmx_channel;
    (void)p_channel_pid_list;
    return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_dmx_channel_del_pid(aui_hdl hdl_dmx_channel, const aui_dmx_channel_pid_list *p_channel_pid_list)
{
    (void)hdl_dmx_channel;
    (void)p_channel_pid_list;
    return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_dmx_channel_open(aui_hdl p_hdl_dmx,
                                  const aui_attr_dmx_channel *p_attr_channel,
                                  aui_hdl* const pp_hdl_dmx_channel)
{
    OSAL_ID dev_mutex_id;
    aui_handle_dmx *pst_hdl_dmx = NULL;
    aui_handle_dmx_channel *pst_hdl_dmx_channel = NULL;
    AUI_RTN_CODE aui_rtn_val = AUI_RTN_FAIL;

    (void)aui_rtn_val;
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_dmx,dev_mutex_id,AUI_MODULE_DMX,AUI_RTN_FAIL);
    if ((NULL == p_attr_channel) ||
        (NULL == p_hdl_dmx) ||
        (NULL == pp_hdl_dmx_channel)) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
        //(*(aui_handle_dmx_channel **)pp_hdl_dmx_channel)->attr_dmx_channel.ul_crc_en=0;
        //(*(aui_handle_dmx_channel **)pp_hdl_dmx_channel)->attr_dmx_channel.section_call_back=NULL;
    }
    //AUI_DBG("section_buffer = [%08x]\n", p_attr_channel->section_buffer);
    pst_hdl_dmx_channel = (aui_handle_dmx_channel *)MALLOC(sizeof(aui_handle_dmx_channel));
    if (NULL == pst_hdl_dmx_channel) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }
    MEMSET(pst_hdl_dmx_channel, 0, sizeof(aui_handle_dmx_channel));
    MEMCPY(&(pst_hdl_dmx_channel->attr_dmx_channel), p_attr_channel, sizeof(aui_attr_dmx_channel));    
    if (AUI_DMX_DATA_SECT == pst_hdl_dmx_channel->attr_dmx_channel.dmx_data_type) {
        pst_hdl_dmx_channel->puc_chl_buf = MALLOC(AUI_DMX_ONE_SECTION_LEN_MAX);
        if (NULL == pst_hdl_dmx_channel->puc_chl_buf) {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn_val = AUI_RTN_ENOMEM;
            goto ERR_END;
        }
        MEMSET(pst_hdl_dmx_channel->puc_chl_buf, 0, AUI_DMX_ONE_SECTION_LEN_MAX);
        pst_hdl_dmx_channel->st_reg_serv.param = MALLOC(sizeof(struct get_section_param));
        if (NULL == (pst_hdl_dmx_channel->st_reg_serv.param)) {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn_val = AUI_RTN_FAIL;
            goto ERR_END;
        }
        AUI_DBG("st_reg_serv.param=[%08x]\n", (pst_hdl_dmx_channel->st_reg_serv.param));
        MEMSET(((struct get_section_param *)(pst_hdl_dmx_channel->st_reg_serv.param)), 0, sizeof(struct get_section_param));
        pst_hdl_dmx_channel->puc_chl_buf_tmp = pst_hdl_dmx_channel->st_reg_serv.param;
    } else if ((AUI_DMX_DATA_PES == pst_hdl_dmx_channel->attr_dmx_channel.dmx_data_type) &&
               (pst_hdl_dmx_channel->attr_dmx_channel.dmx_channel_pes_callback_support)) {
        pst_hdl_dmx_channel->bufaddr = MALLOC(ALI_UDI_PES_BUF_SIZE);
        if (pst_hdl_dmx_channel->bufaddr == NULL) {
            osal_mutex_unlock(dev_mutex_id);
            goto ERR_END;
        }    
        pst_hdl_dmx_channel->buflen = ALI_UDI_PES_BUF_SIZE;
        pst_hdl_dmx_channel->bufpos = 0;
    }
    //AUI_DBG("!!!!!!!!!!!![%08x][%08x][%08x]\n", pst_hdl_dmx_channel, 
    //    pst_hdl_dmx_channel->attr_dmx_channel.section_buffer, p_attr_channel->section_buffer);
    pst_hdl_dmx = (aui_handle_dmx *)p_hdl_dmx;
    pst_hdl_dmx_channel->ul_channel_id = (unsigned long)(pst_hdl_dmx_channel);
    pst_hdl_dmx_channel->p_hdl_dmx = (aui_handle_dmx *)p_hdl_dmx;
    pst_hdl_dmx_channel->dev_mutex_id = dev_mutex_id;
    //aui_handle_set_magic_num(AUI_MODULE_DMX_CHANNEL,pst_hdl_dmx_channel);
    add_dev_2_list((struct aui_list_head *)pst_hdl_dmx_channel, &(pst_hdl_dmx->dev_priv_data.sub_list));
    pst_hdl_dmx_channel->dev_priv_data.en_status = AUI_DEV_STATUS_OPEN;
    *pp_hdl_dmx_channel = pst_hdl_dmx_channel;

    /** add channel  ringbuf start ,just support section synchronous get data */
    if ((AUI_DMX_DATA_SECT == pst_hdl_dmx_channel->attr_dmx_channel.dmx_data_type) &&
        (pst_hdl_dmx_channel->attr_dmx_channel.dmx_channel_sec_sync_get_data_support)) {
        if (AUI_RTN_SUCCESS != aui_common_init_ring_buf(AUI_DMX_CHANNEL_BUF_LEN,
                                                        &(pst_hdl_dmx_channel->chan_rbuf))) {
            osal_mutex_unlock(dev_mutex_id);
            goto ERR_END;
        }    
    }
    /** add channel  ringbuf stop */
    osal_mutex_unlock(dev_mutex_id);
    return AUI_RTN_SUCCESS;
ERR_END:
    if (NULL != pst_hdl_dmx_channel) {
        if (NULL != pst_hdl_dmx_channel->puc_chl_buf) {
            FREE(pst_hdl_dmx_channel->puc_chl_buf);
            pst_hdl_dmx_channel->puc_chl_buf = NULL;
        }
        if (NULL != pst_hdl_dmx_channel->st_reg_serv.param) {
            FREE(pst_hdl_dmx_channel->st_reg_serv.param);
            pst_hdl_dmx_channel->st_reg_serv.param = NULL;
        }
        FREE(pst_hdl_dmx_channel);
        pst_hdl_dmx_channel = NULL;
    }
    return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_dmx_channel_close(aui_hdl *pp_hdl_dmx_channel)
{
    aui_handle_dmx *pst_hdl_dmx = NULL;
    aui_handle_dmx_channel *pst_hdl_dmx_channel = NULL;
    aui_handle_dmx_filter *pst_hdl_dmx_filter = NULL;
    OSAL_ID dev_mutex_id = INVALID_ID;

    AUI_DBG("pp_hdl_dmx_channel = [%08x], *pp_hdl_dmx_channel = %08x\n",
        pp_hdl_dmx_channel, *pp_hdl_dmx_channel);
    if ((OSAL_INVALID_ID == s_mutex_id_dmx)) {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    if ((NULL == pp_hdl_dmx_channel)) {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    /** new add begin --> before close channle must stop it first */
    pst_hdl_dmx_channel=*(aui_handle_dmx_channel **)pp_hdl_dmx_channel;
    aui_dmx_channel_stop(pst_hdl_dmx_channel, NULL);
    /** <-- new add end */
    osal_mutex_lock(s_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
    if ((NULL == pst_hdl_dmx_channel) || (OSAL_INVALID_ID == pst_hdl_dmx_channel->dev_mutex_id)) {
        osal_mutex_unlock(s_mutex_id_dmx);
        return AUI_RTN_SUCCESS;
    }
    osal_mutex_lock(pst_hdl_dmx_channel->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mutex_id_dmx);
TRY_CHAN_CLOSE:
    /* new add for fil channel close busy detect, remove callback mutex begin -->*/
    osal_task_dispatch_off();
    pst_hdl_dmx_channel->try_close = 1;
    if (pst_hdl_dmx_channel->busy) {
        osal_task_dispatch_on();
        osal_task_sleep(1);    
        goto TRY_CHAN_CLOSE;
    } else {
        osal_task_dispatch_on();
    }
    /** <-- new add for fil channel close busy detect, remove callback mutex end */
    if (AUI_DEV_STATUS_OPEN != pst_hdl_dmx_channel->dev_priv_data.en_status) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    pst_hdl_dmx_channel->dev_priv_data.en_status = AUI_DEV_STATUS_CLOSED;
    /** free channel ringbuf start -->*/
    if (pst_hdl_dmx_channel->chan_rbuf.pby_ring_buf) {
        aui_common_un_init_ring_buf(&(pst_hdl_dmx_channel->chan_rbuf));
    }
    /** <-- free channel ringbuf stop */
    dev_mutex_id = pst_hdl_dmx_channel->dev_mutex_id;
    pst_hdl_dmx = pst_hdl_dmx_channel->p_hdl_dmx;
    while (1) {
        pst_hdl_dmx_filter = (aui_handle_dmx_filter *)(pst_hdl_dmx_channel->dev_priv_data.sub_list.next);
        AUI_DBG("pst_hdl_dmx_filter = [%08x]\n", pst_hdl_dmx_filter);
        if (NULL != pst_hdl_dmx_filter) {
            if (AUI_RTN_SUCCESS != aui_dmx_filter_close((aui_hdl *)&pst_hdl_dmx_filter)) {
                aui_rtn(AUI_RTN_FAIL, "delete filter handle list err.");
            }
        } else {
            break;
        }
    }
    del_dev_from_list((struct aui_list_head *)pst_hdl_dmx_channel, &(pst_hdl_dmx->dev_priv_data.sub_list));

    if (AUI_DMX_DATA_SECT == pst_hdl_dmx_channel->attr_dmx_channel.dmx_data_type) {
        if (NULL != pst_hdl_dmx_channel->puc_chl_buf) {
            MEMSET(pst_hdl_dmx_channel->puc_chl_buf, 0, AUI_DMX_ONE_SECTION_LEN_MAX);
            FREE(pst_hdl_dmx_channel->puc_chl_buf);
            pst_hdl_dmx_channel->puc_chl_buf = NULL;
        }
        if (NULL != (pst_hdl_dmx_channel->st_reg_serv.param)) {
            MEMSET(((struct get_section_param *)(pst_hdl_dmx_channel->st_reg_serv.param)), 0, sizeof(struct get_section_param));
            FREE(pst_hdl_dmx_channel->st_reg_serv.param);
            pst_hdl_dmx_channel->st_reg_serv.param = NULL;
        }
    } else if (AUI_DMX_DATA_PES == pst_hdl_dmx_channel->attr_dmx_channel.dmx_data_type) {
        if (pst_hdl_dmx_channel->bufaddr) {
            FREE(pst_hdl_dmx_channel->bufaddr);
            pst_hdl_dmx_channel->bufaddr = NULL;
            pst_hdl_dmx_channel->buflen = 0;
        }
    }
    MEMSET(pst_hdl_dmx_channel, 0, sizeof(aui_handle_dmx_channel));
    pst_hdl_dmx_channel->dev_mutex_id = OSAL_INVALID_ID;
    *((aui_handle_dmx_channel**)pp_hdl_dmx_channel) = NULL;    
    if (0 != osal_mutex_delete(dev_mutex_id)) {
        AUI_ERR("fatal err.\n");
    }
    osal_task_sleep(1);
    FREE(pst_hdl_dmx_channel);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_start(aui_hdl p_hdl_dmx_channel, const aui_attr_dmx_channel *p_attr_channel)
{
    aui_handle_dmx_channel *p_st_chl_hdl = NULL;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_channel) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if (NULL != p_attr_channel) {
        if (p_attr_channel->us_pid >= AUI_INVALID_PID) {
            if (p_attr_channel->dmx_data_type != AUI_DMX_DATA_REC) {
                osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            } else {
                (&((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel)->ul_pid_cnt = p_attr_channel->ul_pid_cnt;
                memcpy((&((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel)->us_pid_list,
                    p_attr_channel->us_pid_list,
                    sizeof(p_attr_channel->us_pid_list));
            }
        } else {
            (&((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel)->us_pid = p_attr_channel->us_pid;
        }
    } else {
        if(((&((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel)->us_pid >= AUI_INVALID_PID) &&
            ((&((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel)->dmx_data_type != AUI_DMX_DATA_REC)) {
            osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL, NULL);
        }
    }
    p_st_chl_hdl = p_hdl_dmx_channel;
    if (AUI_RTN_SUCCESS != aui_dmx_channel_data_async_get(p_hdl_dmx_channel)) {
        osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    p_st_chl_hdl->attr_dmx_channel.dmx_channel_status = AUI_DMX_CHANNEL_RUN;
    osal_mutex_unlock(p_st_chl_hdl->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_stop(aui_hdl p_hdl_dmx_channel, const aui_attr_dmx_channel *p_attr_channel)
{
    aui_handle_dmx_channel *p_st_chl_hdl = NULL;
    int b_encrypt = 0;

    (void)p_attr_channel;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_channel) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    p_st_chl_hdl=p_hdl_dmx_channel;
    //AUI_DBG("aui_chl_stop_all_flt = [%08x]\n", p_st_chl_hdl);
    //aui_chl_stop_all_flt(p_st_chl_hdl);

    if (((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel.dmx_channel_status == AUI_DMX_CHANNEL_STOP) {
        /** already stopped */
        osal_mutex_unlock(p_st_chl_hdl->dev_mutex_id);
        return AUI_RTN_SUCCESS;
    }

    if (AUI_DMX_DATA_REC == p_st_chl_hdl->attr_dmx_channel.dmx_data_type) {
        b_encrypt = 0;
        if (RET_SUCCESS != aui_dmx_set((aui_hdl)p_st_chl_hdl->p_hdl_dmx, AUI_DMX_SET_IO_REC_MODE, (void *)&b_encrypt)) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
        if (RET_SUCCESS != dmx_io_control(p_st_chl_hdl->p_hdl_dmx->pst_dev_dmx, IO_SET_DEC_CONFIG, 0)) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
        if (RET_SUCCESS != dmx_io_control(p_st_chl_hdl->p_hdl_dmx->pst_dev_dmx, DELETE_RECORD_STR, (UINT32)(p_st_chl_hdl))) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }  
    if (NULL != (p_st_chl_hdl->st_reg_serv.param)) {
        if (RET_SUCCESS != dmx_unregister_service_new(p_st_chl_hdl->p_hdl_dmx->pst_dev_dmx, &(p_st_chl_hdl->st_reg_serv))) {
            osal_mutex_unlock(p_st_chl_hdl->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
    ((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel.dmx_channel_status = AUI_DMX_CHANNEL_STOP;
    osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_set_channel_pid(aui_hdl p_hdl_dmx_channel)
{
    aui_handle_dmx_channel *p_hdl_channel = p_hdl_dmx_channel;
    unsigned short restart = 0;

    if ((NULL == p_hdl_dmx_channel) ||
        (AUI_INVALID_PID == p_hdl_channel->attr_dmx_channel.us_pid)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    /** if channel is run, stop it */
    if (AUI_DMX_CHANNEL_RUN == p_hdl_channel->attr_dmx_channel.dmx_channel_status) {
        if (NULL != p_hdl_channel->st_reg_serv.param) {
            if (AUI_RTN_FAIL == dmx_unregister_service_new(p_hdl_channel->p_hdl_dmx->pst_dev_dmx, &(p_hdl_channel->st_reg_serv))) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
        }
        p_hdl_channel->attr_dmx_channel.dmx_channel_status = AUI_DMX_CHANNEL_STOP;
        restart = 1;
    }
    
    /** set channel pid */
    p_hdl_channel->st_reg_serv.service_pid = p_hdl_channel->attr_dmx_channel.us_pid;

    /**
     * Atention: In the funtion of dmx_unregister_service_new, p_hdl_channel->st_reg_serv.param = NULL.
     *           AUI_DMX_DATA_SECT mode and other model, the implementation of dmx_register_service_new is not the same.
     *           In the AUI_DMX_DATA_SECT mode, dmx_register_service_new will use the empty parameters, lead to errors.
     *           In other mode, dmx_register_service_new will mallloc a parameters again.
     */
    if (NULL != p_hdl_channel->puc_chl_buf_tmp) {
        p_hdl_channel->st_reg_serv.param = p_hdl_channel->puc_chl_buf_tmp;
    }

    /** restart channnel */
    if (1 == restart) {
        if (AUI_RTN_FAIL == dmx_register_service_new(p_hdl_channel->p_hdl_dmx->pst_dev_dmx, &(p_hdl_channel->st_reg_serv))) {
            aui_rtn(AUI_RTN_FAIL,NULL);
        }
        p_hdl_channel->attr_dmx_channel.dmx_channel_status = AUI_DMX_CHANNEL_RUN;
    }
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_set(aui_hdl p_hdl_dmx_channel, unsigned long ul_item, void *pv_param)
{
    aui_handle_dmx_channel *p_hdl_channel = p_hdl_dmx_channel;
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_channel) ||
        (OSAL_INVALID_ID == p_hdl_channel->dev_mutex_id) ||
        (ul_item >= AUI_DMX_CHANNEL_GET_CMD_LAST)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(p_hdl_channel->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    
    switch (ul_item) {
        case AUI_DMX_CHANNEL_PID_SET: {
            p_hdl_channel->attr_dmx_channel.us_pid = (unsigned short)((unsigned int)pv_param);
            rtn_code = aui_dmx_set_channel_pid(p_hdl_channel);  /** update us_pid of channel */
            break;
        }
        default: {
            osal_mutex_unlock(p_hdl_channel->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL,NULL);
            break;
        }
    }
    
    osal_mutex_unlock(p_hdl_channel->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_channel_get(aui_hdl p_hdl_dmx_channel, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    //unsigned long ul_rtn=RET_FAILURE;

    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_channel) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    if (ul_item > AUI_DMX_GET_CMD_LAST) {
        osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    switch (ul_item) {
        case AUI_DMX_CHANNEL_PID_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned short *)pv_param = (((aui_handle_dmx_channel *)p_hdl_dmx_channel)->attr_dmx_channel).us_pid;
            *(unsigned long *)pv_param = (*(unsigned long *)pv_param) & AUI_INVALID_PID;
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_CHANNEL_ATTR_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(aui_attr_dmx_channel **)pv_param = &(((aui_handle_dmx_channel*)p_hdl_dmx_channel)->attr_dmx_channel);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_CHANNEL_GET_DMX_HDL: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(aui_hdl *)pv_param = &(((aui_handle_dmx_channel*)p_hdl_dmx_channel)->p_hdl_dmx);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    if (AUI_RTN_SUCCESS != rtn_code) {
        osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_dmx_channel *)p_hdl_dmx_channel)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_open(aui_hdl p_hdl_dmx_channel,
                                 const aui_attr_dmx_filter *p_attr_filter,
                                 aui_hdl* const pp_hdl_dmx_filter)
{
    OSAL_ID dev_mutex_id;
    aui_handle_dmx_channel *pst_hdl_dmx_channel = NULL;
    aui_handle_dmx_filter *pst_hdl_dmx_filter = NULL;
    AUI_RTN_CODE aui_rtn_val = AUI_RTN_FAIL;

    (void)aui_rtn_val;
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_dmx, dev_mutex_id, AUI_MODULE_DMX, AUI_RTN_FAIL);
    if ((NULL == p_hdl_dmx_channel) ||
        (NULL == p_attr_filter) ||
        (NULL == pp_hdl_dmx_filter)) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    pst_hdl_dmx_filter = (aui_handle_dmx_filter *)MALLOC(sizeof(aui_handle_dmx_filter));
    if ((NULL == pst_hdl_dmx_filter)) {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn_val = AUI_RTN_ENOMEM;
        goto ERR_END;
    }
    MEMSET(pst_hdl_dmx_filter, 0, sizeof(aui_handle_dmx_filter));
    MEMCPY(&(pst_hdl_dmx_filter->attr_dmx_filter), p_attr_filter, sizeof(aui_attr_dmx_filter));
    pst_hdl_dmx_channel = (aui_handle_dmx_channel *)p_hdl_dmx_channel;
    pst_hdl_dmx_filter->ul_filter_id = (unsigned long)(pst_hdl_dmx_filter);
    pst_hdl_dmx_filter->p_hdl_dmx_channel = pst_hdl_dmx_channel;
    pst_hdl_dmx_filter->dev_mutex_id = dev_mutex_id;
    //AUI_DBG("flt[%08x] open = [%08x]\n", pst_hdl_dmx_filter, (pst_hdl_dmx_filter->st_get_section.mask_value));
    AUI_DBG("oepn pst_hdl_dmx_filter=[%08x]\n", pst_hdl_dmx_filter);
    AUI_DBG("p_attr_filter->ul_mask_val_len = [%08x]\n", p_attr_filter->ul_mask_val_len);
    if (p_attr_filter->ul_mask_val_len > 0) {
        pst_hdl_dmx_filter->attr_dmx_filter.puc_mask = MALLOC(p_attr_filter->ul_mask_val_len);
        AUI_DBG("pst_hdl_dmx_filter->attr_dmx_filter.puc_mask = [%08x]\n", pst_hdl_dmx_filter->attr_dmx_filter.puc_mask);
        if (NULL == pst_hdl_dmx_filter->attr_dmx_filter.puc_mask) {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn_val = AUI_RTN_ENOMEM;
            goto ERR_END;
        }
        MEMSET(pst_hdl_dmx_filter->attr_dmx_filter.puc_mask,0,p_attr_filter->ul_mask_val_len);
        if (NULL != p_attr_filter->puc_mask) {
            MEMCPY(pst_hdl_dmx_filter->attr_dmx_filter.puc_mask,p_attr_filter->puc_mask,p_attr_filter->ul_mask_val_len);
        }

        pst_hdl_dmx_filter->attr_dmx_filter.puc_val = MALLOC(p_attr_filter->ul_mask_val_len);
        AUI_DBG("pst_hdl_dmx_filter->attr_dmx_filter.puc_val = [%08x]\n", pst_hdl_dmx_filter->attr_dmx_filter.puc_val);
        if (NULL == pst_hdl_dmx_filter->attr_dmx_filter.puc_val) {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn_val = AUI_RTN_ENOMEM;
            goto ERR_END;
        }
        MEMSET(pst_hdl_dmx_filter->attr_dmx_filter.puc_val, 0, p_attr_filter->ul_mask_val_len);
        if (NULL != p_attr_filter->puc_val) {
            MEMCPY(pst_hdl_dmx_filter->attr_dmx_filter.puc_val, p_attr_filter->puc_val, p_attr_filter->ul_mask_val_len);
        }

        pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse = MALLOC(p_attr_filter->ul_mask_val_len);
        AUI_DBG("pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse = [%08x]\n", pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse);
        if (NULL == pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse) {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn_val = AUI_RTN_ENOMEM;
            goto ERR_END;
        }
        MEMSET(pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse, 0, p_attr_filter->ul_mask_val_len);
        if (NULL != p_attr_filter->puc_reverse) {
            MEMCPY(pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse,p_attr_filter->puc_reverse,p_attr_filter->ul_mask_val_len);
        }
    }
    //aui_handle_set_magic_num(AUI_MODULE_DMX_FILTER,pst_hdl_dmx_filter);
    add_dev_2_list((struct aui_list_head *)pst_hdl_dmx_filter, &(pst_hdl_dmx_channel->dev_priv_data.sub_list));
    pst_hdl_dmx_filter->dev_priv_data.en_status = AUI_DEV_STATUS_OPEN;
    
    /** add filter ringbuf start -->*/
    if ((pst_hdl_dmx_filter->attr_dmx_filter.dmx_fil_sec_data_sync_get_support) &&
        (AUI_DMX_DATA_SECT == pst_hdl_dmx_channel->attr_dmx_channel.dmx_data_type)) {
        if (AUI_RTN_SUCCESS != aui_common_init_ring_buf(AUI_DMX_FILTER_BUF_LEN,
                                                        &(pst_hdl_dmx_filter->fil_rbuf))) {
            osal_mutex_unlock(dev_mutex_id);
            goto ERR_END;
        }
    }
    /** <-- add filter ringbuf close */
    *pp_hdl_dmx_filter=pst_hdl_dmx_filter;
    AUI_DBG("open a filter = [%08x]\n", pst_hdl_dmx_filter);
    osal_mutex_unlock(dev_mutex_id);
    return AUI_RTN_SUCCESS;
ERR_END:
    if (NULL != pst_hdl_dmx_filter) {
        if (NULL != pst_hdl_dmx_filter->attr_dmx_filter.puc_mask) {
            FREE(pst_hdl_dmx_filter->attr_dmx_filter.puc_mask);
            pst_hdl_dmx_filter->attr_dmx_filter.puc_mask = NULL;
        }
        if (NULL != pst_hdl_dmx_filter->attr_dmx_filter.puc_val) {
            FREE(pst_hdl_dmx_filter->attr_dmx_filter.puc_val);
            pst_hdl_dmx_filter->attr_dmx_filter.puc_val = NULL;
        }
        if (NULL != pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse) {
            FREE(pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse);
            pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse=NULL;
        }
        FREE(pst_hdl_dmx_filter);
        pst_hdl_dmx_filter = NULL;
    }
    return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_dmx_filter_close(aui_hdl *pp_hdl_dmx_filter)
{
    aui_handle_dmx_channel *pst_hdl_dmx_channel = NULL;
    aui_handle_dmx_filter *pst_hdl_dmx_filter = NULL;
    aui_handle_dmx_filter **ppst_hdl_dmx_filter = NULL;

    OSAL_ID dev_mutex_id = 0;
    
    AUI_DBG("pp_hdl_dmx_filter_1 = [%08x], *pp_hdl_dmx_filter_1 = %08x\n",
        pp_hdl_dmx_filter, *pp_hdl_dmx_filter);
    if ((OSAL_INVALID_ID == s_mutex_id_dmx_channel)) {
        AUI_DBG("pp_hdl_dmx_filter = [%08x]\n", pp_hdl_dmx_filter);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(s_mutex_id_dmx_channel,OSAL_WAIT_FOREVER_TIME);
    if ((NULL == pp_hdl_dmx_filter)) {
        AUI_DBG("check pp_hdl_dmx_filter is NULL: [%08x]\n", pp_hdl_dmx_filter);
        osal_mutex_unlock(s_mutex_id_dmx_channel);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    AUI_DBG("pp_hdl_dmx_filter_2 = [%08x], *pp_hdl_dmx_filter_2 = %08x\n",
        pp_hdl_dmx_filter, *pp_hdl_dmx_filter);
    ppst_hdl_dmx_filter = (aui_handle_dmx_filter **)pp_hdl_dmx_filter;
    pst_hdl_dmx_filter = *ppst_hdl_dmx_filter;
    if ((NULL == pst_hdl_dmx_filter) ||
        (OSAL_INVALID_ID == pst_hdl_dmx_filter->dev_mutex_id)) {
        osal_mutex_unlock(s_mutex_id_dmx_channel);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }    
    if (AUI_DEV_STATUS_OPEN != pst_hdl_dmx_filter->dev_priv_data.en_status) {
        osal_mutex_unlock(s_mutex_id_dmx_channel);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }    
    osal_mutex_lock(pst_hdl_dmx_filter->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mutex_id_dmx_channel);
TRY_FIL_CLOSE:
    /** new add for fil channel close busy detect, remove callback mutex begin -->*/
    osal_task_dispatch_off();
    pst_hdl_dmx_filter->try_close = 1;
    if (pst_hdl_dmx_filter->busy) {
        osal_task_dispatch_on();
        osal_task_sleep(1);    
        goto TRY_FIL_CLOSE;
    } else {
        osal_task_dispatch_on();
    }
    /** <-- new add for fil channel close busy detect, remove callback mutex end */
    pst_hdl_dmx_filter->attr_dmx_filter.dmx_filter_status = AUI_DMX_FILTER_STOP;
    /** free filter ringbuf start --> */
    if (pst_hdl_dmx_filter->fil_rbuf.pby_ring_buf) {
        aui_common_un_init_ring_buf(&(pst_hdl_dmx_filter->fil_rbuf));
    }
    /** <-- free filter ringbuf stop */
    pst_hdl_dmx_filter->dev_priv_data.en_status = AUI_DEV_STATUS_CLOSED;
    dev_mutex_id = pst_hdl_dmx_filter->dev_mutex_id;
    pst_hdl_dmx_channel = pst_hdl_dmx_filter->p_hdl_dmx_channel;
    if (NULL == pst_hdl_dmx_channel) {
        /** fatal err */
        aui_rtn(AUI_RTN_FAIL, "channel_handle is NULL.\n");
    }
    del_dev_from_list((struct aui_list_head *)pst_hdl_dmx_filter, &(pst_hdl_dmx_channel->dev_priv_data.sub_list));
    if (pst_hdl_dmx_filter->attr_dmx_filter.puc_mask) {
        MEMSET(pst_hdl_dmx_filter->attr_dmx_filter.puc_mask, 0, pst_hdl_dmx_filter->attr_dmx_filter.ul_mask_val_len);
        FREE(pst_hdl_dmx_filter->attr_dmx_filter.puc_mask);
        pst_hdl_dmx_filter->attr_dmx_filter.puc_mask = NULL;
    }
    if (pst_hdl_dmx_filter->attr_dmx_filter.puc_val) {
        MEMSET(pst_hdl_dmx_filter->attr_dmx_filter.puc_val, 0, pst_hdl_dmx_filter->attr_dmx_filter.ul_mask_val_len);
        FREE(pst_hdl_dmx_filter->attr_dmx_filter.puc_val);
        pst_hdl_dmx_filter->attr_dmx_filter.puc_val = NULL;
    }
    if (pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse) {
        MEMSET(pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse, 0, pst_hdl_dmx_filter->attr_dmx_filter.ul_mask_val_len);
        FREE(pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse);
        pst_hdl_dmx_filter->attr_dmx_filter.puc_reverse = NULL;
    }
    MEMSET(&(pst_hdl_dmx_filter->dev_priv_data), 0, sizeof(aui_dev_priv_data));
    pst_hdl_dmx_filter->dev_mutex_id = OSAL_INVALID_ID;
    *((aui_handle_dmx_filter **)pp_hdl_dmx_filter) = NULL;
    if (0 != osal_mutex_delete(dev_mutex_id)) {
        AUI_ERR("fatal err.\n");
    }
    osal_task_sleep(1);    
    FREE(pst_hdl_dmx_filter);    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_start(aui_hdl p_hdl_dmx_filter, const aui_attr_dmx_filter *p_attr_filter)
{
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = (aui_handle_dmx_filter *)p_hdl_dmx_filter;
    (void)p_attr_filter;
    
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter_tmp) ||
        (OSAL_INVALID_ID == p_hdl_dmx_filter_tmp->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(p_hdl_dmx_filter_tmp->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    p_hdl_dmx_filter_tmp->attr_dmx_filter.dmx_filter_status = AUI_DMX_FILTER_RUN;
    osal_mutex_unlock(p_hdl_dmx_filter_tmp->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_stop(aui_hdl p_hdl_dmx_filter, const aui_attr_dmx_filter *p_attr_filter)
{
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = (aui_handle_dmx_filter *)p_hdl_dmx_filter;
    (void)p_attr_filter;
    
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter_tmp) ||
        (OSAL_INVALID_ID == p_hdl_dmx_filter_tmp->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(p_hdl_dmx_filter_tmp->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    p_hdl_dmx_filter_tmp->attr_dmx_filter.dmx_filter_status = AUI_DMX_FILTER_STOP;
    osal_mutex_unlock(p_hdl_dmx_filter_tmp->dev_mutex_id);
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_set(aui_hdl p_hdl_dmx_filter, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if (ul_item >= AUI_DMX_SET_CMD_LAST) {
        osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    switch (ul_item) {
        case AUI_DMX_FILTER_MASKVAL_SET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_CONFIG: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    if (AUI_RTN_SUCCESS != rtn_code) {
        osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_get(aui_hdl p_hdl_dmx_filter, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    if (ul_item > AUI_DMX_GET_CMD_LAST) {
        osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    switch (ul_item) {
        case AUI_DMX_FILTER_TYPE_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_ATTR_GET: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(aui_attr_dmx_filter **)pv_param = &(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->attr_dmx_filter);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_GET_CHANNEL_HDL: {
            if (NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(aui_hdl **)pv_param = (aui_hdl *)&(((aui_handle_dmx_filter*)p_hdl_dmx_filter)->p_hdl_dmx_channel);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    if (AUI_RTN_SUCCESS != rtn_code) {
        osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_mask_val_cfg(aui_hdl p_hdl_dmx_filter,
                                         const unsigned char *puc_mask,
                                         const unsigned char *puc_val,
                                         const unsigned char *puc_reverse,
                                         unsigned long ul_mask_val_len,
                                         unsigned char uc_crc_check,
                                         unsigned char uc_continue_capture_flag)
{
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = p_hdl_dmx_filter;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter) || (NULL == puc_mask) || (NULL == puc_val) ||
        (NULL == puc_reverse) || (0 == ul_mask_val_len) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);

    if (ul_mask_val_len == p_hdl_dmx_filter_tmp->attr_dmx_filter.ul_mask_val_len) {
        if ((NULL != p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_mask) && (NULL != puc_mask)) {
               MEMCPY(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_mask, puc_mask, ul_mask_val_len);
        }
        if ((NULL != p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_val) && (NULL != puc_val)) {
               MEMCPY(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_val, puc_val, ul_mask_val_len);
        }
        if ((NULL != p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_reverse) && (NULL != puc_reverse)) {
               MEMCPY(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_reverse, puc_reverse, ul_mask_val_len);
        }
    } else {
        p_hdl_dmx_filter_tmp->attr_dmx_filter.ul_mask_val_len = ul_mask_val_len;
        p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_mask = REALLOC(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_mask, ul_mask_val_len);
        if ((NULL != p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_mask) && (NULL != puc_mask)) {
               MEMCPY(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_mask, puc_mask, ul_mask_val_len);
        }
        p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_val=REALLOC(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_val,ul_mask_val_len);
        if ((NULL != p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_val) && (NULL != puc_val)) {
            MEMCPY(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_val, puc_val, ul_mask_val_len);
        }
        p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_reverse=REALLOC(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_reverse,ul_mask_val_len);
        if ((NULL != p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_reverse) && (NULL != puc_reverse)) {
            MEMCPY(p_hdl_dmx_filter_tmp->attr_dmx_filter.puc_reverse, puc_reverse, ul_mask_val_len);
        }
    }
    
    p_hdl_dmx_filter_tmp->attr_dmx_filter.ul_mask_val_len = ul_mask_val_len;
    p_hdl_dmx_filter_tmp->attr_dmx_filter.uc_crc_check = uc_crc_check;
    p_hdl_dmx_filter_tmp->attr_dmx_filter.uc_continue_capture_flag = uc_continue_capture_flag;
    osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_reg_sect_call_back(aui_hdl p_hdl_dmx_filter, aui_p_fun_sectionCB p_fun_section_cb)
{
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter) || 
        (OSAL_INVALID_ID==((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    ((((aui_handle_dmx_filter *)p_hdl_dmx_filter)->attr_dmx_filter).p_fun_sectionCB) = p_fun_section_cb;
    osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_reg_data_call_back(aui_hdl p_hdl_dmx_filter,
                                        aui_p_fun_data_req_wtCB p_fun_data_req_wt_cb,
                                        aui_p_fun_data_up_wtCB p_fun_data_up_wt_cb)
{
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = p_hdl_dmx_filter;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter) ||
        (OSAL_INVALID_ID == ((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    p_hdl_dmx_filter_tmp->attr_dmx_filter.p_fun_data_req_wtCB = p_fun_data_req_wt_cb;
    p_hdl_dmx_filter_tmp->attr_dmx_filter.p_fun_data_up_wtCB = p_fun_data_up_wt_cb;
    osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_reg_pes_call_back(aui_hdl p_hdl_dmx_filter, aui_pes_data_callback callback, void *callback_param)
{
    aui_handle_dmx_filter *p_hdl_dmx_filter_tmp = p_hdl_dmx_filter;
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx_filter)||(0 == ((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    p_hdl_dmx_filter_tmp->attr_dmx_filter.callback = callback;
    p_hdl_dmx_filter_tmp->attr_dmx_filter.callback_param = callback_param;
    osal_mutex_unlock(((aui_handle_dmx_filter *)p_hdl_dmx_filter)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_sync_get_section_data(aui_hdl p_hdl_dmx_channel,
                                                   aui_hdl pst_hdl_dmx_filter,
                                                   const unsigned int req_size, 
                                                   unsigned char *const p_buf, 
                                                   unsigned int *const p_data_size, 
                                                   const unsigned int timeout_ms)
{
    aui_handle_dmx_channel *p_hdl_dmx_chl = p_hdl_dmx_channel;
    aui_handle_dmx_filter *p_hdl_dmx_filter = pst_hdl_dmx_filter;
    unsigned int  tick = 0;
    unsigned char *dst = p_buf;
    int left = req_size;
    unsigned long read_len = 0;
    
    if ((p_buf == NULL) || (p_data_size == NULL) || (left == 0)) {
        if (p_data_size) {
            *p_data_size = 0;
        }
        return AUI_RTN_FAIL;
    }

    *p_data_size = 0;
    tick = osal_get_tick();
        
    if (p_hdl_dmx_chl) {    
        while (left > 0) {
GET_CHANNEL_BUF_SECTION_LEN:
            osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
            if (OSAL_INVALID_ID == p_hdl_dmx_chl->dev_mutex_id) {
                osal_mutex_unlock(s_mod_mutex_id_dmx);
                return AUI_RTN_FAIL;
            }
            
            osal_mutex_lock(p_hdl_dmx_chl->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
            osal_mutex_unlock(s_mod_mutex_id_dmx);
            if (p_hdl_dmx_chl->attr_dmx_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) {    
                /** channel already deleted */
                return AUI_RTN_FAIL;
            }            
            if ((p_hdl_dmx_chl->attr_dmx_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_chl->chan_rbuf.pby_ring_buf == NULL)) {    
                osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);    
                return AUI_RTN_FAIL;
            }            
            left = aui_common_get_rbuf_rdlen(&(p_hdl_dmx_chl->chan_rbuf));            
            if (left == 0) {
                osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                if (((osal_get_tick() - tick ) >= timeout_ms)|| (osal_get_tick() < tick)) {
                    break;
                }
                osal_task_sleep(1);
                goto GET_CHANNEL_BUF_SECTION_LEN;
            }            
            if ((unsigned int)left > req_size) {
                osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                return AUI_RTN_FAIL;    
            }            
            aui_common_ring_buf_rd(&(p_hdl_dmx_chl->chan_rbuf), left, &read_len, dst);
            osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
            left = 0;    
        }
        *p_data_size = read_len;        
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            return AUI_RTN_FAIL;
        }    
    } else if (p_hdl_dmx_filter) {    
        while (left > 0) {
GET_FLT_BUF_SECTION_LEN:
            osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
            if (OSAL_INVALID_ID == p_hdl_dmx_filter->dev_mutex_id) {
                osal_mutex_unlock(s_mod_mutex_id_dmx);
                return AUI_RTN_FAIL;
            }            
            osal_mutex_lock(p_hdl_dmx_filter->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
            osal_mutex_unlock(s_mod_mutex_id_dmx);            
            if (p_hdl_dmx_filter->attr_dmx_filter.dmx_filter_status != AUI_DMX_FILTER_RUN) {    
                /** filter already deleted */
                return AUI_RTN_FAIL;
            }
            p_hdl_dmx_chl = p_hdl_dmx_filter->p_hdl_dmx_channel;
            if ((p_hdl_dmx_chl == NULL) ||
                (p_hdl_dmx_chl->attr_dmx_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) ||
                (p_hdl_dmx_chl->attr_dmx_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_filter->fil_rbuf.pby_ring_buf == NULL)) {    
                if (p_hdl_dmx_filter->dev_mutex_id) {
                    osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);    
                }                
                return AUI_RTN_FAIL;
            }                
            left = aui_common_get_rbuf_rdlen(&(p_hdl_dmx_filter->fil_rbuf));
            if (left == 0) {
                osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                if (((osal_get_tick() - tick ) >= timeout_ms) || (osal_get_tick() < tick)) {
                    break;
                }
                osal_task_sleep(1);
                goto GET_FLT_BUF_SECTION_LEN;
            }
            /** req buf not enough */
            if ((unsigned int)left > req_size) {
                osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                return AUI_RTN_FAIL;    
            }            
            aui_common_ring_buf_rd(&(p_hdl_dmx_filter->fil_rbuf), left, &read_len, dst);
            osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
            left = 0;
        }
        *p_data_size = read_len;
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            return AUI_RTN_FAIL;
        }
    } else {
        return AUI_RTN_FAIL;
    }
}

AUI_RTN_CODE aui_dmx_pcr_reg(aui_hdl p_hdl_dmx, aui_p_attr_dmx dmx_attr)
{
    struct register_service_new reg_pcr_serv;
    struct get_pcr_param* pcr_param = NULL;
    int rtn_code = AUI_RTN_FAIL;
    //aui_handle_dmx *p_handle_dmx_temp = (aui_handle_dmx *)p_hdl_dmx;
    
    osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
    if ((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (0 == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id) ||
        (NULL == dmx_attr)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
#if 0
    MEMCPY(&(p_handle_dmx_temp->dmx_pcr_info),(struct get_pcr_param*)(&(dmx_attr->dmx_pcr_info)),sizeof(struct get_pcr_param));
    p_handle_dmx_temp->dmx_pcr_info.get_pcr_cb = aui_get_pcr_cb;
    
    reg_pcr_serv.device =  NULL;
    reg_pcr_serv.dmx_data_type = DMX_PCR_DATA;    
    reg_pcr_serv.service_pid = p_handle_dmx_temp->dmx_pcr_info.pid;
    reg_pcr_serv.param = (void*)(&(p_handle_dmx_temp->dmx_pcr_info)); 
#endif
    //MEMCPY(&(p_handle_dmx_temp->dmx_pcr_info),(struct get_pcr_param*)(&(dmx_attr->dmx_pcr_info)),sizeof(struct get_pcr_param));
    //p_handle_dmx_temp->dmx_pcr_info.get_pcr_cb = aui_get_pcr_cb;
    pcr_param = (struct get_pcr_param*)(&(dmx_attr->dmx_pcr_info));
    reg_pcr_serv.device = NULL;
    reg_pcr_serv.dmx_data_type = DMX_PCR_DATA;    
    reg_pcr_serv.service_pid = pcr_param->pid;
    reg_pcr_serv.param = (void*)(pcr_param); 
    reg_pcr_serv.request_write = NULL;
    reg_pcr_serv.update_write = NULL;
    reg_pcr_serv.str_type = UNKNOW_STR;
    
    dmx_attr->reg_pcr_serv = MALLOC(sizeof(struct register_service_new));
    if (dmx_attr->reg_pcr_serv == NULL) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }

    MEMCPY((struct register_service_new*)(dmx_attr->reg_pcr_serv),
           &reg_pcr_serv, sizeof(struct register_service_new));
    AUI_DBG("reg addr: 0x%x\n", dmx_attr->reg_pcr_serv);    
    rtn_code = dmx_register_service_new((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),
                                         (struct register_service_new*)(dmx_attr->reg_pcr_serv));    
    AUI_DBG("rnt: %d\n", rtn_code);
    if (0 != rtn_code) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_pcr_unreg(aui_hdl p_hdl_dmx, aui_p_attr_dmx dmx_attr)
{
    osal_mutex_lock(s_mod_mutex_id_dmx, OSAL_WAIT_FOREVER_TIME);
    if((NULL == p_hdl_dmx) ||
        (NULL == ((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx) ||
        (0 == ((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id) ||
        (NULL == dmx_attr) ||
        (NULL == dmx_attr->reg_pcr_serv)) {
        osal_mutex_unlock(s_mod_mutex_id_dmx);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    osal_mutex_lock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_dmx);
    if (RET_SUCCESS != dmx_unregister_service_new((((aui_handle_dmx *)p_hdl_dmx)->pst_dev_dmx),(struct register_service_new*)(dmx_attr->reg_pcr_serv))) {
        osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    FREE(dmx_attr->reg_pcr_serv);
    dmx_attr->reg_pcr_serv = NULL;
    osal_mutex_unlock(((aui_handle_dmx *)p_hdl_dmx)->dev_mutex_id);
    return AUI_RTN_SUCCESS;    
}

AUI_RTN_CODE aui_dmx_channel_sync_get_section_data_ext(aui_hdl p_hdl_dmx_channel,
                                                       aui_hdl pst_hdl_dmx_filter,
                                                       const unsigned int req_size, 
                                                       unsigned char *const p_buf, 
                                                       unsigned int *const p_data_size, 
                                                       const unsigned int timeout_ms)
{
    aui_handle_dmx_channel *p_hdl_dmx_chl = p_hdl_dmx_channel;
    aui_handle_dmx_filter *p_hdl_dmx_filter = pst_hdl_dmx_filter;    
    unsigned int  tick = 0;
    unsigned char *dst = p_buf;
    unsigned long left = req_size;
    unsigned long read_len = 0;

    if ((p_buf == NULL) || (p_data_size == NULL) || (left == 0)) {
        if (p_data_size) {
            *p_data_size = 0;
        }
        return AUI_RTN_FAIL;
    }
    *p_data_size = 0;
    tick = osal_get_tick();    
    if (p_hdl_dmx_chl) {    
        while (left > 0) {
GET_CHANNEL_BUF_SECTION_LEN:
            osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
            if (OSAL_INVALID_ID == p_hdl_dmx_chl->dev_mutex_id) {
                osal_mutex_unlock(s_mod_mutex_id_dmx);
                return AUI_RTN_FAIL;
            }            
            osal_mutex_lock(p_hdl_dmx_chl->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
            osal_mutex_unlock(s_mod_mutex_id_dmx);            
            if (p_hdl_dmx_chl->attr_dmx_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) {    
                /** channel already deleted */
                return AUI_RTN_FAIL;
            }
            if ((p_hdl_dmx_chl->attr_dmx_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_chl->chan_rbuf.pby_ring_buf == NULL)) {  
                osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);    
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_data_len(&(p_hdl_dmx_chl->chan_rbuf),&left);
            
            if (left == 0) {
                osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                if (((osal_get_tick() - tick ) >= timeout_ms) || (osal_get_tick() < tick)) {
                    break;
                }
                osal_task_sleep(1);
                goto GET_CHANNEL_BUF_SECTION_LEN;
            }        
            if ((unsigned int)left > req_size) {
                osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);                
                return AUI_RTN_FAIL;    
            }
            aui_common_ring_buf_rd(&(p_hdl_dmx_chl->chan_rbuf), left, &read_len, dst);
            osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
            left = 0;    
        }
        *p_data_size = read_len;    
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            return AUI_RTN_FAIL;
        }    
    } else if (p_hdl_dmx_filter) {    
        while (left > 0) {
GET_FLT_BUF_SECTION_LEN:
            osal_mutex_lock(s_mod_mutex_id_dmx,OSAL_WAIT_FOREVER_TIME);
            if(0==p_hdl_dmx_filter->dev_mutex_id)
            {
                osal_mutex_unlock(s_mod_mutex_id_dmx);
                return AUI_RTN_FAIL;
            }    
            osal_mutex_lock(p_hdl_dmx_filter->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
            osal_mutex_unlock(s_mod_mutex_id_dmx);        
            if (p_hdl_dmx_filter->attr_dmx_filter.dmx_filter_status != AUI_DMX_FILTER_RUN) {    
                /** filter already deleted */
                return AUI_RTN_FAIL;
            }
            p_hdl_dmx_chl = p_hdl_dmx_filter->p_hdl_dmx_channel;
            if ((p_hdl_dmx_chl == NULL) ||
                (p_hdl_dmx_chl->attr_dmx_channel.dmx_channel_status!=AUI_DMX_CHANNEL_RUN) ||
                (p_hdl_dmx_chl->attr_dmx_channel.dmx_data_type!=AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_filter->fil_rbuf.pby_ring_buf==NULL)) {    
                osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);                
                return AUI_RTN_FAIL;
            }        
            aui_common_ring_buf_data_len(&(p_hdl_dmx_filter->fil_rbuf), &left);
            if (left == 0) {
                osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                if (((osal_get_tick() - tick ) >= timeout_ms) || (osal_get_tick() < tick)) {
                    break;
                }
                osal_task_sleep(1);
                goto GET_FLT_BUF_SECTION_LEN;
            }
            if ((unsigned int)left > req_size) {
                osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);            
                return AUI_RTN_FAIL;    
            }        
            aui_common_ring_buf_rd(&(p_hdl_dmx_filter->fil_rbuf), left, &read_len, dst);
            osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
            left = 0;
        }
        *p_data_size = read_len;
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            return AUI_RTN_FAIL;
        }
    } else {
        return AUI_RTN_FAIL;
    }    
}

