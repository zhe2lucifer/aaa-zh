/**@file
*  @brief           ALi AUI DMX module interface implement
*  @author          ray.gong
*  @date            2013-5-21
*  @version         1.0.0
*  @note            ali corp. all rights reserved. 2013~2999 copyright (C)\n
*/
/****************************INCLUDE HEAD FILE************************************/
#include <sys/time.h>
#include "aui_common_priv.h"
#include <aui_dmx.h>
#include <alisldmx.h>
#include <alislavsync.h>
#include <alislsnd.h>
#include <alislvdec.h>
#include <pthread.h>
#include "aui_dsc_common.h"
#include "aui_decv_priv.h"
#include "aui_dmx_priv.h"
#include <alisldsc.h>

/****************************LOCAL MACRO******************************************/
AUI_MODULE(DMX)

#define FILTER_TOKEN_CNT                (256)
#define AUI_DMX_ONE_SECTION_LEN_MAX     (4*1024)
#define AUI_DMX_FILTER_SIZE             32
#define VIDEO_VALID                     (1 << 0)
#define AUDIO_VALID                     (1 << 1)
#define AUDIO_DESC_VALID                (1 << 2)
#define AUI_DMX_RINBUF_WRITE_TRYCNT     (1)

/** here always disable the macro. because we move the ADD CA PID operation to aui dsc module */
//#define SET_PID_IN_DMX

/****************************LOCAL TYPE*******************************************/
typedef struct aui_dmx_data_path_priv {
    unsigned char dev_idx;
    unsigned char path_type;
    unsigned short dsc_process_mode;
    union dmx_pvr_hdl{
        unsigned int dmx_dsc_id; /** for encrypt record */
        unsigned int ali_pvr_de_hdl; /** for pvr playback block data */
    }dmx_pvr_hdl;
    unsigned int block_size;
}aui_dmx_data_path_priv;

typedef struct dmx_device {
    aui_dev_priv_data           dev_priv_data;
    aui_attr_dmx                attr_dmx;
    alisl_handle                handle;
    alisl_handle                avsync_hdl;
    aui_avsync_srctype_t        avsync_srctype; /** default 0: AUI_AVSYNC_FROM_TUNER */
    unsigned int                crypto_started;
    unsigned int                ttx_chid;
    unsigned int                subt_chid;
    unsigned int                audio_chid;
    unsigned int                audio_desc_chid;
    unsigned int                video_chid;
    unsigned int                pcr_chid;
    unsigned short              ttx_pid;
    unsigned short              subt_pid;
    unsigned short              audio_pid;
    unsigned short              audio_desc_pid;
    unsigned short              video_pid;
    unsigned short              pcr_pid;
    aui_hdl                     decv_hdl;
    unsigned int                front_end;
    struct dmx_dsc_fd_param     enc_para;
    struct aui_list_head        channels; /** list head of service channels */
    enum aui_dmx_cache_type     cache_type; /** fast channel change cache type */
    unsigned int                av_started;
    aui_dmx_data_path_priv      data_path_priv; /** multi-process support */
    /** only used for swdmx playback block data. only ca-process can release ali_pvr_decrypt handle. */
    aui_dmx_data_path_dsc_type  data_path_dsc_type;
    /** add for block data injection by linux mmap */
    aui_hdl                     ali_pvr_sl_hdl;
    unsigned int                mmap_addr;
    unsigned int                mmap_len;
} dmx_device_t;

typedef struct dmx_channel {
    struct aui_list_head        node;
    unsigned int                id;
    struct dmx_device          *dmx_device;
    aui_attr_dmx_channel        attr_channel;
    struct aui_list_head        filters; /** list head of service channels */
    aui_ring_buf                chan_rbuf; /** buf for section data */
} dmx_channel_t;

typedef struct dmx_filter {
    aui_attr_dmx_filter         attr_filter;
    struct aui_list_head        node;
    unsigned int                id;
    struct dmx_channel         *dmx_channel;
    unsigned char              *puc_mask;
    unsigned char              *puc_val;
    unsigned char              *puc_reverse;
    unsigned long               ul_mask_val_len;
    unsigned char               uc_crc_check;
    unsigned char               uc_continue_capture_flag;
    int                         cb_filter_token_id;
    unsigned char              *buffer;
    unsigned long               buffer_length;
    aui_ring_buf                fil_rbuf; /** the filter ringbuf */
} dmx_filter_t;

typedef struct cb_filter_token {
    int                         id;
    struct dmx_filter          *filter;
}cb_filter_token_t;

typedef enum _token_status_e {
    TOKEN_IDLE = 0,
    TOKEN_USED,
    TOKEN_PRE_USED,
    TOKEN_MAX,
}token_status_e;

/****************************LOCAL VAR********************************************/
//static unsigned char cb_filter_token_status[FILTER_TOKEN_CNT] = {0};
static token_status_e cb_filter_token_status[FILTER_TOKEN_CNT] = {TOKEN_IDLE};
static cb_filter_token_t filter_token_array[FILTER_TOKEN_CNT] = {{0,0}};
static pthread_mutex_t token_mutex = PTHREAD_MUTEX_INITIALIZER;

/****************************LOCAL FUNC DECLARE***********************************/
static int require_cb_token_id(void)
{
    int i = 0;
    pthread_mutex_lock(&token_mutex);
    for (i = 0; i < FILTER_TOKEN_CNT; i++) {
        if (TOKEN_IDLE == cb_filter_token_status[i]) {
            break;
        }
    }
    if (i >= FILTER_TOKEN_CNT) {
        AUI_ERR("error -> no enough token id\n");
        pthread_mutex_unlock(&token_mutex);
        return AUI_RTN_FAIL;
    } else {
        cb_filter_token_status[i] = TOKEN_PRE_USED;
        AUI_DBG("filter token id: %d, status: %d\n", i, cb_filter_token_status[i]);
        pthread_mutex_unlock(&token_mutex);
        return i;
    }
}

static void set_cb_token_status(int token_id, token_status_e status)
{
    pthread_mutex_lock(&token_mutex);
    if (((token_id >= FILTER_TOKEN_CNT) || (token_id < 0)) || (status >= TOKEN_MAX)) {
        AUI_WARN("warning -> token_id is out of range: %d\n", token_id);
        pthread_mutex_unlock(&token_mutex);
        return ;
    }
    AUI_DBG("set filter token id %d to status %d\n", token_id, status);
    cb_filter_token_status[token_id] = status;
    pthread_mutex_unlock(&token_mutex);
}

static void get_cb_token_status(int token_id, token_status_e *status)
{
    pthread_mutex_lock(&token_mutex);
    if (((token_id >= FILTER_TOKEN_CNT) || (token_id < 0)) || (!status)) {
        AUI_WARN("warning -> token_id is out of range: %d\n", token_id);
        pthread_mutex_unlock(&token_mutex);
        return ;
    }
    *status = cb_filter_token_status[token_id];
    pthread_mutex_unlock(&token_mutex);
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
        of_rpt.ringbuf_right = (unsigned long)(p_ringbuf->pby_ring_buf+p_ringbuf->ul_ring_buf_rd);
        of_rpt.len_right = p_ringbuf->ul_ring_buf_len - p_ringbuf->ul_ring_buf_rd;
        of_rpt.ringbuf_left = (unsigned long)p_ringbuf->pby_ring_buf;
        of_rpt.len_left = p_ringbuf->ul_ring_buf_wt;
    }
    cb(p_user_hdl, handle_type, AUI_DMX_EVENT_SYNC_GET_DATA_OVERFLOW, (void*)(&of_rpt), param);
    p_ringbuf->ul_ring_buf_rd = p_ringbuf->ul_ring_buf_wt = 0;
    aui_common_ring_buf_unlock(p_ringbuf);
    //AUI_SLEEP(1); //osal_task_sleep(1);
    return ;
}


static void aui_dmx_put_data_to_rbuf(aui_ring_buf *p_ringbuf,
                                     unsigned char *buf,
                                     unsigned int len,
                                     struct dmx_channel *p_chl_handle,
                                     struct dmx_filter *p_flt_hdl)
{
    unsigned char syn_indi = *(unsigned char*)(buf + 1);
    unsigned char sec_len_low_byte = *(unsigned char*)(buf + 2);
    aui_dmx_event_report cb = NULL;
    //AUI_DBG("enter");
    int fail_cnt = 0;
    if (p_ringbuf && (p_ringbuf->pby_ring_buf) && (p_ringbuf->ul_ring_buf_len)) {
        if (len != (unsigned int)(((unsigned short)((syn_indi&0x0f)<<8)) | (sec_len_low_byte)) + 3) {
            AUI_ERR("section len not match.\n");
            return;
        }
        while (fail_cnt <= AUI_DMX_RINBUF_WRITE_TRYCNT) {
            if (AUI_RTN_SUCCESS == aui_common_ring_buf_wt(p_ringbuf,len,buf)) {
                //AUI_DBG("%p write: %d done.\n", p_ringbuf->pby_ring_buf, len);
                return ;
            } else {
                fail_cnt++;
                AUI_DBG("notify overflow, fail count: %d.\n", fail_cnt);
                /** notify overflow */
                if ((p_chl_handle) && (!p_flt_hdl)) {
                    cb = p_chl_handle->attr_channel.event_cb;
                    if (cb) {
                        aui_dmx_rpt_overflow(p_ringbuf, AUI_DMX_HANDLE_CHANNEL, p_chl_handle, cb,
                                             p_chl_handle->attr_channel.event_cb_param);
                        p_chl_handle->attr_channel.overflow_rpt_cnt++;
                    }
                } else if ((!p_chl_handle) && (p_flt_hdl)) {
                    cb = p_flt_hdl->attr_filter.event_cb;
                    if (cb) {
                        aui_dmx_rpt_overflow(p_ringbuf, AUI_DMX_HANDLE_FILTER, p_flt_hdl, cb,
                                             p_flt_hdl->attr_filter.event_cb_param);
                        p_flt_hdl->attr_filter.overflow_rpt_cnt++;
                    }
                }
                if (!cb) {
                    //AUI_SLEEP(1); //osal_task_sleep(1);
                }
            }
        }
        /** record auto reset buffer count by driver when overflow happens */
        if ((p_chl_handle) && (!p_flt_hdl)) {
            p_chl_handle->attr_channel.overflow_auto_rst_buf_cnt++;
        } else if ((!p_chl_handle) && (p_flt_hdl)) {
            p_flt_hdl->attr_filter.overflow_auto_rst_buf_cnt++;
        }
        aui_common_rst_ring_buf(p_ringbuf); /** automatic rst */
    }
    //AUI_DBG("leave");
    return ;
    }

static void requestbuf_callback(void *priv,
                                unsigned long channelid,
                                unsigned long filterid,
                                unsigned long length,
                                unsigned char **buffer,
                                unsigned long *actlen)
{
    cb_filter_token_t *filter_token = priv;
    struct dmx_filter *filter = NULL;
    struct dmx_channel *channel = NULL;
    token_status_e status = TOKEN_IDLE;
    int ret = AUI_RTN_FAIL;

    if (!filter_token) {
        AUI_WARN("filter_token can`t be null\n");
        return ;
    }
    if ((filter_token->id < 0) || (filter_token->id >= FILTER_TOKEN_CNT)) {
        AUI_WARN("filter_token id is out of range\n");
        return ;
    }
    get_cb_token_status(filter_token->id, &status);
    if (TOKEN_IDLE == status) {
        AUI_WARN("warning filter token id %d has been closed\n", filter_token->id);
        return ;
    }
    filter = filter_token->filter;
    channel = filter->dmx_channel;

    if (AUI_DMX_DATA_SECT == channel->attr_channel.dmx_data_type) {
        if ((!filter->buffer) || (!filter->buffer_length)) {
            filter->buffer = (unsigned char *) malloc(AUI_DMX_ONE_SECTION_LEN_MAX);
            if (!filter->buffer) {
                AUI_ERR("filter token id %d malloc failed for section buffer\n", filter_token->id);
                return ;
            }
            filter->buffer_length = AUI_DMX_ONE_SECTION_LEN_MAX;
        }
        *buffer = filter->buffer;
        *actlen = (length <= filter->buffer_length) ? length : filter->buffer_length;
        return ;
    }
    if ((AUI_DMX_DATA_PES == channel->attr_channel.dmx_data_type) &&
        (!filter->attr_filter.p_fun_data_req_wtCB)) {
        if ((!filter->buffer) || (filter->buffer_length < length)) {
            if (!filter->buffer) {
                filter->buffer = (unsigned char *) malloc(length);
            } else {
                filter->buffer = (unsigned char *) realloc(filter->buffer, length);
            }
            if (!filter->buffer) {
                AUI_ERR("filter token id %d malloc failed for pes buffer\n", filter_token->id);
                return ;
            }
            filter->buffer_length = length;
        }
        *buffer = filter->buffer;
        *actlen = length;
        return ;
    }
    if (filter->attr_filter.p_fun_data_req_wtCB) {
        ret = filter->attr_filter.p_fun_data_req_wtCB(filter, length, (void**)buffer, actlen, NULL);
        if (AUI_RTN_SUCCESS != ret) {
            *buffer = NULL;
            *actlen = 0;
            return ;
        }
    } else {
        *buffer = NULL;
        *actlen = 0;
    }
    /* unused */
    (void)channelid;
    (void)filterid;
}

static void updatebuf_callback(void *priv,
                               unsigned long channelid,
                               unsigned long filterid,
                               unsigned long valid_len,
							   unsigned short offset)
{
	(void)offset;
    cb_filter_token_t *filter_token = priv;
    struct dmx_filter *filter = NULL;
    struct dmx_channel *channel = NULL;
    token_status_e status = TOKEN_IDLE;

    if (!filter_token) {
        AUI_WARN("filter_token can`t be null\n");
        return ;
    }
    if ((filter_token->id < 0) || (filter_token->id >= FILTER_TOKEN_CNT)) {
        AUI_WARN("filter_token id is out of range\n");
        return;
    }
    get_cb_token_status(filter_token->id, &status);
    if (TOKEN_IDLE == status) {
        AUI_WARN("warning filter token id %d has been closed\n", filter_token->id);
        return ;
    }
    filter = filter_token->filter;
    channel = filter->dmx_channel;

    if (AUI_DMX_DATA_SECT == channel->attr_channel.dmx_data_type) {
        if (NULL == filter->attr_filter.p_fun_sectionCB) {
            AUI_WARN("filter token id %d section record cb is not defined\n", filter_token->id);
            return ;
        }

        if (channel->attr_channel.dmx_channel_sec_sync_get_data_support == 1) {
            //AUI_DBG("to call dmx_put_data_to_rbuf\n");
            aui_dmx_put_data_to_rbuf(&(channel->chan_rbuf), filter->buffer, valid_len, channel, NULL);
            aui_dmx_put_data_to_rbuf(&(filter->fil_rbuf), filter->buffer, valid_len, NULL, filter);
        }
        // keep the same flow with TDS
        if (filter->attr_filter.p_fun_sectionCB) {
            filter->attr_filter.p_fun_sectionCB(filter, filter->buffer, valid_len,
                                                filter->attr_filter.usr_data, NULL);
        }
        return ;
    }
    if ((AUI_DMX_DATA_PES == channel->attr_channel.dmx_data_type) &&
        (!filter->attr_filter.p_fun_data_up_wtCB)) {
        if (NULL == filter->attr_filter.callback) {
            AUI_WARN("filter token id %d pes record cb is not defined\n", filter_token->id);
            return ;
        }

        filter->attr_filter.callback(filter, filter->buffer, valid_len, filter->attr_filter.callback_param);
        return ;
    }
    if (NULL == filter->attr_filter.p_fun_data_up_wtCB) {
        AUI_WARN("filter token id %d data record cb is not defined\n", filter_token->id);
        return ;
    }
    filter->attr_filter.p_fun_data_up_wtCB(filter, valid_len);

    /* unused */
    (void)channelid;
    (void)filterid;
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
    aui_hdl hdl_dmx = NULL;
    enum dmx_see_id see_id = DMX_SEE_ID_DEMUX0;
    if (alisldmx_open(&hdl_dmx, ul_dev_idx, see_id)) {
        aui_rtn(AUI_RTN_EINVAL, "DMX ID [%ld] is invalid!\n", ul_dev_idx);
    }
    alisldmx_close(hdl_dmx);
	
	return AUI_RTN_SUCCESS;
}

/****************************MODULE IMPLEMENT*************************************/
AUI_RTN_CODE aui_dmx_version_get(unsigned long *const pul_version)
{
    if (NULL == pul_version) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    *pul_version = AUI_MODULE_VERSION_NUM_DMX;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_init(p_fun_cb p_call_back_init, void *pv_param)
{
    if (p_call_back_init != NULL) {
        p_call_back_init(pv_param);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_de_init(p_fun_cb p_call_back_init, void *pv_param)
{
    if (p_call_back_init != NULL) {
        p_call_back_init(pv_param);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_open(const aui_attr_dmx *p_attr_dmx,
                          aui_hdl *const pp_hdl_dmx)
{
    struct dmx_device *dev = NULL;
    unsigned char video_id = 0;
    enum dmx_see_id see_id = DMX_SEE_ID_DEMUX0;

    if (NULL == pp_hdl_dmx || NULL == p_attr_dmx) {
        aui_rtn(AUI_RTN_EINVAL, "Input parameter is invalid!\n");
    }

    /* check DMX device ID */
    if (check_dmx_dev_id(p_attr_dmx->uc_dev_idx)) {
        aui_rtn(AUI_RTN_EINVAL, "DMX device ID [%ld] is invalid!\n", (unsigned long)(p_attr_dmx->uc_dev_idx));
    }
    
    *pp_hdl_dmx = NULL;
    dev = malloc(sizeof(*dev));
    if (NULL == dev) {
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }
    memset(dev, 0, sizeof(*dev));
    memcpy(&(dev->attr_dmx), p_attr_dmx, sizeof(aui_attr_dmx));
    dev->dev_priv_data.dev_idx = p_attr_dmx->uc_dev_idx;
    if (alislavsync_open(&dev->avsync_hdl)) {
        AUI_ERR("open avsync fail\n");
    }
    dev->decv_hdl = p_attr_dmx->decv_handle;
    if (dev->decv_hdl) {
        aui_decv_get_video_id(dev->decv_hdl, &video_id);
        if (video_id == 1) {
            see_id = DMX_SEE_ID_DEMUX1;
        }
        aui_decv_set_see_dmx_id(dev->decv_hdl, see_id);
    }

    if (alisldmx_open(&dev->handle, dev->dev_priv_data.dev_idx, see_id)) {
        free(dev);
        aui_rtn(AUI_RTN_FAIL, "alisldmx_open return FAIL!\n");
    }

    if (dev->attr_dmx.dmx_data_path.data_path_type != AUI_DMX_DATA_PATH_CLEAR_PLAY) {
        if (aui_dmx_data_path_set((aui_hdl *) dev, &dev->attr_dmx.dmx_data_path)) {
            free(dev);
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
    dev->ttx_chid = DMX_ILLEGAL_CHANNELID;
    dev->subt_chid = DMX_ILLEGAL_CHANNELID;
    dev->audio_chid = DMX_ILLEGAL_CHANNELID;
    dev->audio_desc_chid = DMX_ILLEGAL_CHANNELID;
    dev->video_chid = DMX_ILLEGAL_CHANNELID;
    dev->pcr_chid = DMX_ILLEGAL_CHANNELID;
    dev->ttx_pid = AUI_INVALID_PID;
    dev->subt_pid = AUI_INVALID_PID;
    dev->audio_pid = AUI_INVALID_PID;
    dev->audio_desc_pid = AUI_INVALID_PID;
    dev->video_pid = AUI_INVALID_PID;
    dev->pcr_pid = AUI_INVALID_PID;
    dev->enc_para.decrypt_fd = -1;
    dev->enc_para.encrypt_fd = -1;

    AUI_INIT_LIST_HEAD(&dev->channels);
    aui_dev_reg(AUI_MODULE_DMX, dev);
    *pp_hdl_dmx = dev;

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_close(aui_hdl p_hdl_dmx)
{
    struct dmx_device *dev = p_hdl_dmx;
    struct dmx_channel *ch, *tmp;

    if ((NULL == dev) ||
        (NULL == dev->handle)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if ((AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == dev->data_path_priv.dsc_process_mode)
        &&(dev->data_path_dsc_type != AUI_DMX_DATA_PATH_DSC_TYPE_ID)){
        if(aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_SRC_SET, (void *)AUI_DMX_M2S_SRC_NORMAL))
            AUI_ERR("set normal src fail\n");
        if(aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_SET, (void *)0))
            AUI_ERR("set valid size fail\n");
    } else if ((AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == dev->data_path_priv.dsc_process_mode) &&
               (AUI_DMX_DATA_PATH_DSC_TYPE_ID == dev->data_path_dsc_type)) {
        if(aui_dsc_pvr_munmap(dev->ali_pvr_sl_hdl, (unsigned int *)dev->mmap_addr, &dev->mmap_len)) {
            AUI_ERR("non-ca munmap fail\n");
            return AUI_RTN_FAIL;
        }
    }
    aui_list_for_each_entry_safe(ch, tmp, &dev->channels, node) {
        aui_dmx_channel_close((aui_hdl*)&ch);
    }

    alisldmx_close(dev->handle);
    alislavsync_close(dev->avsync_hdl);

    aui_dev_unreg(AUI_MODULE_DMX, dev);

    memset(dev, 0, sizeof(*dev));
    free(dev);

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
        aui_rtn(AUI_RTN_EINVAL, "Input parameter ERROR!\n");
    }

    aui_dmx_dev_capability *p_info = p_capability_info;
    memset(p_info, 0, sizeof(aui_dmx_dev_capability));

    p_info->ul_dev_idx = ul_dev_idx;

    switch (ul_dev_idx) {
	    case AUI_DMX_ID_DEMUX0:
	    case AUI_DMX_ID_DEMUX1:
	    case AUI_DMX_ID_DEMUX2:
	    case AUI_DMX_ID_DEMUX3: {
		    p_info->dev_type = AUI_DMX_DEV_TYPE_HARDWARE;
		    break;
		}
	    case AUI_DMX_ID_SW_DEMUX0:
	    case AUI_DMX_ID_SW_DEMUX1: {
		    p_info->dev_type = AUI_DMX_DEV_TYPE_SOFTWARE;
		    break;
		}
        default : {
		    p_info->dev_type = AUI_DMX_DEV_TYPE_LAST;
			aui_rtn(AUI_RTN_FAIL, "DMX ID [%ld] is invalid!\n", ul_dev_idx);
        }
	}

    return AUI_RTN_SUCCESS;
}

/** in ca_process, get dmx_dsc_id after calling aui_dmx_data_path_set */
AUI_RTN_CODE aui_dmx_dsc_id_get(aui_hdl p_hdl_dmx, aui_dmx_dsc_id *p_dmx_dsc_id)
{
    if ((!p_hdl_dmx) || (!p_dmx_dsc_id)) {
        aui_rtn(AUI_RTN_EINVAL, "null input arg\n");
    }

    memset(p_dmx_dsc_id->identifier, 0, sizeof(p_dmx_dsc_id->identifier));
    struct dmx_device *dev = p_hdl_dmx;

    if ((AUI_DMX_DATA_PATH_EN_REC == dev->data_path_priv.path_type) ||
        (AUI_DMX_DATA_PATH_DE_PLAY_EN_REC == dev->data_path_priv.path_type) ||
        (AUI_DMX_DATA_PATH_REEN_REC == dev->data_path_priv.path_type)) {
        struct dmx_record_deencrypt_info tmp_deen_info;
        tmp_deen_info.m_dmx_dsc_fd.decrypt_fd = dev->enc_para.decrypt_fd;
        tmp_deen_info.m_dmx_dsc_fd.encrypt_fd = dev->enc_para.encrypt_fd;
        tmp_deen_info.m_dmx_dsc_id = -1;
        if (alisldmx_en_rec_info_get(dev->handle, &tmp_deen_info)) {
            AUI_ERR("get en rec info fail\n");
            return AUI_RTN_FAIL;
        }
        dev->data_path_priv.dmx_pvr_hdl.dmx_dsc_id = tmp_deen_info.m_dmx_dsc_id;
    }
    memcpy(p_dmx_dsc_id, &(dev->data_path_priv), sizeof(dev->data_path_priv));

    AUI_DUMP("identifier: ", (char *)p_dmx_dsc_id->identifier, sizeof(p_dmx_dsc_id->identifier));
    AUI_DUMP("data_path_priv: ", (char *) &(dev->data_path_priv), sizeof(dev->data_path_priv));

    AUI_INFO("size: %u, id:%d, path: %d, hdl: 0x%x, process_mode: %d \n", sizeof(dev->data_path_priv),
        (int)dev->data_path_priv.dev_idx, (int)dev->data_path_priv.path_type,
        dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl, (int)dev->data_path_priv.dsc_process_mode);
    return AUI_RTN_SUCCESS;
}

int aui_dmx_ali_pvr_de_hdl_get(aui_hdl p_hdl_dmx,
                               unsigned int *ali_pvr_de_hdl,
                               unsigned int *block_size,
                               unsigned int *dsc_process_mode,
                               unsigned int *mmap_addr,
                               unsigned int *mmap_len,
                               aui_hdl *ali_sl_hdl)
{
    struct dmx_device *dev = p_hdl_dmx;

    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    if (ali_pvr_de_hdl) {
        *ali_pvr_de_hdl = dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl;
    }
    if (block_size) {
        *block_size = dev->data_path_priv.block_size;
    }
    if (dsc_process_mode) {
        *dsc_process_mode = dev->data_path_priv.dsc_process_mode;
    }
    if (mmap_addr) {
        *mmap_addr = dev->mmap_addr;
    }
    if (mmap_len) {
        *mmap_len = dev->mmap_len;
    }
    if (ali_sl_hdl) {
        *ali_sl_hdl = dev->ali_pvr_sl_hdl;
    }
    return AUI_RTN_SUCCESS;
}

static int dsc_type_id_data_path_set(struct dmx_device *dev,
                                     const aui_dmx_data_path *p_dmx_data_path_info)
{

    if ((NULL == dev) ||
        (NULL == p_dmx_data_path_info->p_dsc_id)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    aui_dmx_data_path_priv *data_path_priv = (aui_dmx_data_path_priv *)p_dmx_data_path_info->p_dsc_id;

    AUI_INFO("path type: %d\n", p_dmx_data_path_info->data_path_type);
    AUI_DUMP("identifier: ", (char *)p_dmx_data_path_info->p_dsc_id->identifier,
        sizeof(p_dmx_data_path_info->p_dsc_id->identifier));

    AUI_INFO("id:%d, path: %d, hdl: 0x%x, process_mode: %d \n",
        (int)data_path_priv->dev_idx, (int)data_path_priv->path_type,
        data_path_priv->dmx_pvr_hdl.ali_pvr_de_hdl, (int)data_path_priv->dsc_process_mode);
    /** if path type is AUI_DMX_DATA_PATH_DSC_TYPE_ID, the current process is non-ca process,
        we should check if the dmx id and data_path info is same with calling  aui_dmx_data_path_set in CA process. */
    if ((dev->dev_priv_data.dev_idx != data_path_priv->dev_idx) ||
        (p_dmx_data_path_info->data_path_type != data_path_priv->path_type)) {
        AUI_ERR("ERROR->non_ca_dmx_id: %d, ca_dmx_id: %d, non_ca_path: %d, ca_path:%d \n",
            (int)dev->dev_priv_data.dev_idx, (int)(data_path_priv->dev_idx),
            (int)p_dmx_data_path_info->data_path_type, (int)(data_path_priv->path_type));
        return AUI_RTN_FAIL;
    }
    switch (p_dmx_data_path_info->data_path_type) {
        case AUI_DMX_DATA_PATH_CLEAR_PLAY:
        case AUI_DMX_DATA_PATH_REC:
            AUI_ERR("is there anything special to do ?\n");
            break;
        case AUI_DMX_DATA_PATH_DE_PLAY: {
            if (((AUI_DMX_ID_SW_DEMUX0 == dev->dev_priv_data.dev_idx) ||
                (AUI_DMX_ID_SW_DEMUX1 == dev->dev_priv_data.dev_idx)) &&
                (AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == data_path_priv->dsc_process_mode)) {
                /** in non-ca process, set dsc-id to sl and let pvrlib get it. */
                dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl = data_path_priv->dmx_pvr_hdl.ali_pvr_de_hdl;
                dev->data_path_priv.block_size = data_path_priv->block_size;
                dev->data_path_priv.dsc_process_mode = data_path_priv->dsc_process_mode;
                dev->data_path_dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_ID;
                if (aui_dsc_pvr_mmap(&dev->ali_pvr_sl_hdl, &dev->mmap_addr, &dev->mmap_len)) {
                    AUI_ERR("mmap fail\n");
                    return AUI_RTN_FAIL;
                }
                AUI_INFO("hdl: 0x%x, bs: %u, %p, %u\n", dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl,
                    dev->data_path_priv.block_size,(void *)dev->mmap_addr, dev->mmap_len);
            } else {
                /** everything is done in CA process and do nothing in non-ca process. */
                AUI_ERR("everything is done in CA process and do nothing in non-ca process\n");
            }
            break;
        }
        case AUI_DMX_DATA_PATH_EN_REC: {
            /** non-ca process, just set dsc-id to sl and wait for recording. */
            unsigned int dmx_dsc_id = data_path_priv->dmx_pvr_hdl.dmx_dsc_id;
            return alisldmx_dsc_id_set(dev->handle, dmx_dsc_id);
        }
        case AUI_DMX_DATA_PATH_DE_PLAY_EN_REC: {
            /** for this data path, we just support live play, do not support software dmx playback.
                we will decrypt the scrambled service to live play and re-encrypt record it. */
            if ((AUI_DMX_ID_SW_DEMUX0 == dev->dev_priv_data.dev_idx) ||
                (AUI_DMX_ID_SW_DEMUX1 == dev->dev_priv_data.dev_idx)) {
                aui_rtn(AUI_RTN_FAIL,"not support AUI_DMX_DATA_PATH_DE_PLAY_EN_REC for software dmx\n");
            }
            /** non-ca process, just set dsc-id to sl and wait for recording.
                non-ca process, do nothing for play. */
            unsigned int dmx_dsc_id = data_path_priv->dmx_pvr_hdl.dmx_dsc_id;
            return alisldmx_dsc_id_set(dev->handle, dmx_dsc_id);
        }
        case AUI_DMX_DATA_PATH_REEN_REC: {
            /** non-ca process, just set dsc-id to sl and wait for recording. */
            unsigned int dmx_dsc_id = data_path_priv->dmx_pvr_hdl.dmx_dsc_id;
            return alisldmx_dsc_id_set(dev->handle, dmx_dsc_id);
        }
        default:
            break;
    }
    return AUI_RTN_SUCCESS;
}

static int dsc_type_fd_data_path_set(struct dmx_device *dev,
                                     const aui_dmx_data_path *p_dmx_data_path_info)
{
    struct dmx_dsc_fd_param dsc_config;

    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dsc_config.decrypt_fd = dsc_config.encrypt_fd = -1;
    AUI_DBG("path type: %d\n", p_dmx_data_path_info->data_path_type);
    AUI_DBG("dec_fd: %d, enc_fd: %d ",
        (int)p_dmx_data_path_info->p_hdl_de_dev, (int)p_dmx_data_path_info->p_hdl_en_dev);
    switch (p_dmx_data_path_info->data_path_type) {
        case AUI_DMX_DATA_PATH_CLEAR_PLAY:
        case AUI_DMX_DATA_PATH_REC:
            AUI_ERR("is there anything special to do ? \n");
            break;
        case AUI_DMX_DATA_PATH_DE_PLAY: {
            if ((AUI_DMX_ID_SW_DEMUX0 == dev->dev_priv_data.dev_idx) ||
                (AUI_DMX_ID_SW_DEMUX1 == dev->dev_priv_data.dev_idx)) {
                AUI_ERR("not support such case. Please check your design flow carefully.\n");
                return AUI_RTN_FAIL;
            } else {
                /** should this case happen? If yes, just config decrypt_fd */
                dsc_config.decrypt_fd = (int)p_dmx_data_path_info->p_hdl_de_dev;
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_DEC_HANDLE, (unsigned long)&dsc_config)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }

                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_START, 0)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                dev->crypto_started = 1;
            }
            break;
        }
        case AUI_DMX_DATA_PATH_EN_REC: {
            int rec = 1; /** 1:DMX_FTA_TO_ENCRYPT, 0:DMX_FTA_TO_FTA */
            //AUI_DBG("set io_rec_mode %d\n", rec);
            if (aui_dmx_set(dev, AUI_DMX_SET_IO_REC_MODE, (void *)&rec)) {
                AUI_ERR("FTA to ENCRYPT fail\n");
                return AUI_RTN_FAIL;
            }
            dsc_config.encrypt_fd = (int)p_dmx_data_path_info->p_hdl_en_dev;
            memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
            break;
        }
        case AUI_DMX_DATA_PATH_DE_PLAY_EN_REC: {
            /** for this data path, we just support live play, do not support software dmx playback.
                we will decrypt the scrambled service to live play and re-encrypt record it. */
            if ((AUI_DMX_ID_SW_DEMUX0 == dev->dev_priv_data.dev_idx) ||
                (AUI_DMX_ID_SW_DEMUX1 == dev->dev_priv_data.dev_idx)) {
                aui_rtn(AUI_RTN_FAIL, "not support AUI_DMX_DATA_PATH_DE_PLAY_EN_REC for software dmx\n");
            }
            /** ca process, just config decrypt_fd for live play. */
            dsc_config.decrypt_fd = (int)p_dmx_data_path_info->p_hdl_de_dev;
            /** set dsc decrypt fd */
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_DEC_HANDLE, (unsigned long)&dsc_config)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_START, 0)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dev->crypto_started = 1;
            dsc_config.encrypt_fd = (int)p_dmx_data_path_info->p_hdl_en_dev;
            memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
            break;
        }
        case AUI_DMX_DATA_PATH_REEN_REC: {
            dsc_config.decrypt_fd = (int)p_dmx_data_path_info->p_hdl_de_dev;
            dsc_config.encrypt_fd = (int)p_dmx_data_path_info->p_hdl_en_dev;
            memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
            break;
        }
        default:
            AUI_ERR("what do you want to do?\n");
            break;
    }
    /** storing the path type, maybe sending to non-ca process to check
        if setting the data path with same dmx device and path type. */
    dev->data_path_priv.path_type= p_dmx_data_path_info->data_path_type;
    dev->data_path_priv.dev_idx = dev->dev_priv_data.dev_idx;
    return AUI_RTN_SUCCESS;

}

static int decrypt_ts_data_to_live_play(struct dmx_device *dev,
                                        const aui_dmx_data_path *p_dmx_data_path_info,
                                        struct algo_info *algori_info)
{
    struct algo_info algo_info;
    struct dmx_dsc_fd_param dsc_config;

    if ((NULL == dev) ||
        (NULL == p_dmx_data_path_info) ||
        (!algori_info)) {
        aui_rtn(AUI_RTN_EINVAL, "null input arg\n");
    }

    memcpy(&algo_info, algori_info, sizeof(struct algo_info));
    dsc_config.decrypt_fd = dsc_config.encrypt_fd = -1;

    dsc_config.decrypt_fd = algo_info.dsc_fd;
    dsc_config.encrypt_fd = -1;
    memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
#ifdef SET_PID_IN_DMX
    if(aui_dsc_set_pid(p_dmx_data_path_info->p_hdl_de_dev, algo_info,
                       AUI_DSC_DECRYPT_IN, AUI_DSC_EVEN_MODE_IN)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
#endif //<--SET_PID_IN_DMX

#ifndef AUI_TA_SUPPORT
    if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_DEC_HANDLE, (unsigned long)&dsc_config)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
#else
	int session_fd = 0;
    memcpy(&session_fd, p_dmx_data_path_info->p_dsc_id, sizeof(int));
    if (alisldmx_ioctl(dev->handle, DMX_IOCMD_BIND_CRYPTO_SESSION, (unsigned long)&(session_fd))) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
	AUI_DBG("AUI session_fd = %d\n", session_fd);
#endif
    if (alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_START, 0)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    dev->crypto_started = 1;
    if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_SRC_SET, (void *)AUI_DMX_M2S_SRC_NORMAL)) {
        aui_rtn(AUI_RTN_FAIL, "set normal src fail\n");
    }
    if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_SET, (void *)0)) {
        aui_rtn(AUI_RTN_FAIL, "set valid size fail\n");
    }
    return AUI_RTN_SUCCESS;
}

static int decrypt_block_data_to_playback(struct dmx_device *dev,
                                          const aui_dmx_data_path *p_dmx_data_path_info,
                                          struct algo_info *algori_info)
{
    struct algo_info algo_info;
    struct dmx_dsc_fd_param dsc_config;
    unsigned int block_cnt = 0;

    memcpy(&algo_info, algori_info, sizeof(struct algo_info));

    /** pvr blockmode: block size is not zero, for HLS ts of vmx ott, block size is zero. */
    if (algo_info.process_attr.ul_block_size) {
        /** reset main2see buf size to origin size then get it. */
        aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_SET, (void *)0);
        unsigned int m2s_buf_size = 0;
        aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_GET, (void *)&m2s_buf_size);
        /** maximize block_cnt
            Limited to dmx driver, 2*188 is used for trick play of pvr lib. */
        unsigned long vob_indicator_buffer_size = BLOCK_VOB_BUFFER_SIZE;
        //AUI_DBG("vob_indicator_buffer_size: %lu\n", vob_indicator_buffer_size);
        block_cnt = (m2s_buf_size) / (algo_info.process_attr.ul_block_size+vob_indicator_buffer_size);
        if ((!m2s_buf_size) || (block_cnt < 4)) {
            AUI_ERR("%u, bs: %lu, cnt: %u\n", m2s_buf_size, algo_info.process_attr.ul_block_size, block_cnt);
            aui_rtn(AUI_RTN_FAIL, "block size is too big\n");
        }
        if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_SRC_SET, (void *)AUI_DMX_M2S_SRC_CRYPT_BLK)) {
            aui_rtn(AUI_RTN_FAIL, "set src to crypt fail\n");
        }
        if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_SET,
                               (void *)((algo_info.process_attr.ul_block_size+vob_indicator_buffer_size)*block_cnt))) {
            aui_rtn(AUI_RTN_FAIL, "set valid size fail\n");
        }
    } else {
        /** vmx ott ts inject */
        if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_SRC_SET, (void *)AUI_DMX_M2S_SRC_CRYPT_DYNAMIC_BLK)) {
            aui_rtn(AUI_RTN_FAIL, "set src to crypt fail\n");
        }
    }
    /** encrypted stream is decrypted by ali_pvr, ts stream is clear for dmx driver. */
    dsc_config.decrypt_fd = dsc_config.encrypt_fd = -1;
    if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_DEC_HANDLE, (unsigned long)&dsc_config)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    if (alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_STOP, 0)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    dev->crypto_started = 0;
    AUI_DBG("dsc_data_type: %d\n", algo_info.dsc_data_type);

    if (AUI_DSC_DATA_TS == algo_info.dsc_data_type) {
#ifdef SET_PID_IN_DMX
        if(aui_dsc_set_pid(p_dmx_data_path_info->p_hdl_de_dev, algo_info,
                           AUI_DSC_DECRYPT_IN, AUI_DSC_EVEN_MODE_IN)) {
            aui_rtn(AUI_RTN_FAIL, "dsc set pid fail");
        }
#endif //<--SET_PID_IN_DMX
    }

    /** not support change key now. */
    unsigned int dsc_num = 1;
    unsigned int block_size = (AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == algo_info.process_attr.process_mode)
                               ? (algo_info.process_attr.ul_block_size) : 0;
    if(aui_dsc_pvr_playback_env_init(p_dmx_data_path_info->p_hdl_de_dev, block_size,
                                     dsc_num, &dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl)) {
        aui_rtn(AUI_RTN_FAIL, "init playback block data env fail\n");
    }
    unsigned int de_index = 0;
    unsigned int switch_block = 0xFFFFFFFF - 1;
    if (aui_dsc_pvr_playback_key_set(p_dmx_data_path_info->p_hdl_de_dev, de_index, block_size,
                                     switch_block, dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl)) {
        if (aui_dsc_pvr_playback_env_deinit(p_dmx_data_path_info->p_hdl_de_dev)) {
            AUI_ERR("deinit playback block data env fail\n");
        }
        aui_rtn(AUI_RTN_FAIL, "set playback block data key fail\n");
    }
    dev->data_path_priv.block_size = block_size;
    AUI_DBG("valid_size:%u, bs: %u, dsc_num: %u, de_index: %u, de_hdl: 0x%x\n",
            block_cnt, block_size, dsc_num, de_index, dev->data_path_priv.dmx_pvr_hdl.ali_pvr_de_hdl);
    return AUI_RTN_SUCCESS;
}

static int dsc_type_hdl_data_path_set(struct dmx_device *dev,
                                      const aui_dmx_data_path *p_dmx_data_path_info)
{
    struct algo_info algo_info;
    struct dmx_dsc_fd_param dsc_config;
    int pvr_process_mode = AUI_DSC_PROCESS_MODE_UNDEFINED;

    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, "null input arg\n");
    }

    memset(&algo_info, 0, sizeof(struct algo_info));
    dsc_config.decrypt_fd = dsc_config.encrypt_fd = -1;

    switch (p_dmx_data_path_info->data_path_type) {
        case AUI_DMX_DATA_PATH_CLEAR_PLAY: {
            dsc_config.decrypt_fd = dsc_config.encrypt_fd = -1;
            memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
            dev->crypto_started = 0;
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_DEC_HANDLE, (unsigned long)&dsc_config)) {
                aui_rtn(AUI_RTN_FAIL, "set dec hanle fail\n");
            }
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_STOP, 0)) {
                aui_rtn(AUI_RTN_FAIL, "set crypto stop fail\n");
            }
            /** set see dmx source type to ts type */
            if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_SRC_SET, (void *)AUI_DMX_M2S_SRC_NORMAL)) {
                aui_rtn(AUI_RTN_FAIL, "set normal src fail\n");
            }
            /** reset see dmx buffer size to origin size */
            if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_SET, (void *)0)) {
                aui_rtn(AUI_RTN_FAIL, "set valid size fail\n");
            }
            break;
        }
        case AUI_DMX_DATA_PATH_REC: {
            int rec = 0;
            if (aui_dmx_set(dev, AUI_DMX_SET_IO_REC_MODE, (void *)&rec)) {
                AUI_ERR("fta to encrypt fail\n");
                return AUI_RTN_FAIL;
            }
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE, DMX_REC_OUT_TS)) {
                aui_rtn(AUI_RTN_FAIL, "set rec ts mode\n");
            }
            dsc_config.decrypt_fd = dsc_config.encrypt_fd = -1;
            memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
            break;
        }
        case AUI_DMX_DATA_PATH_DE_PLAY: {
            /** get dsc decrypt fd */
			#ifndef AUI_TA_SUPPORT
            if (aui_dsc_get_fd(p_dmx_data_path_info->p_hdl_de_dev,&algo_info)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
			#endif
			
            if ((AUI_DSC_PROCESS_MODE_UNDEFINED == algo_info.process_attr.process_mode) ||
                (AUI_DSC_PROCESS_MODE_TS_DECRYPT == algo_info.process_attr.process_mode)) {
                if (decrypt_ts_data_to_live_play(dev, p_dmx_data_path_info, &algo_info)) {
                    aui_rtn(AUI_RTN_FAIL, "decrypt_ts_data_to_live_play fail\n");
                }
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_TS_DECRYPT;
            } else if ((AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == algo_info.process_attr.process_mode) &&
                       ((AUI_DMX_ID_SW_DEMUX0 == dev->dev_priv_data.dev_idx) ||
                       (AUI_DMX_ID_SW_DEMUX1 == dev->dev_priv_data.dev_idx))) {
                if (decrypt_block_data_to_playback(dev, p_dmx_data_path_info, &algo_info))
                    aui_rtn(AUI_RTN_FAIL, "decrypt_block_data_to_playback fail\n");
                dev->data_path_priv.dsc_process_mode = algo_info.process_attr.process_mode;
            }
            break;
        }
        case AUI_DMX_DATA_PATH_EN_REC: {
            int rec = 1;
            if (aui_dmx_set(dev, AUI_DMX_SET_IO_REC_MODE, (void *)&rec)) {
                AUI_ERR("fta to encrypt fail\n");
                return AUI_RTN_FAIL;
            }
            /** get dsc encrypt fd, seting encrypt dsc fd */
            if (aui_dsc_get_fd(p_dmx_data_path_info->p_hdl_en_dev, &algo_info)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dsc_config.decrypt_fd = -1;
            pvr_process_mode = algo_info.process_attr.process_mode;

            if (AUI_DSC_PROCESS_MODE_UNDEFINED == pvr_process_mode) {
                if (algo_info.pvr_crypto_mode) {
                    pvr_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
                } else {
                    pvr_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
                }
            }

            //AUI_DBG("pvr_mode: %d, dsc_fd: %d, pvr_fd: %d, block_size: %lu, mode: %d, size: %lu\n",
            //  algo_info.pvr_crypto_mode, algo_info.dsc_fd, algo_info.pvr_fd,
            //  algo_info.pvr_block_size, algo_info.process_attr.process_mode, algo_info.process_attr.ul_block_size);
            if (AUI_DSC_PROCESS_MODE_TS_ENCRYPT == pvr_process_mode) {
                dsc_config.encrypt_fd = algo_info.dsc_fd;
                /** save the encoding setting, it will be used in aui_dmx_channel_open() */
                memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
                /** set ts mode record */
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE,DMX_REC_OUT_TS)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
#ifdef SET_PID_IN_DMX
                if (aui_dsc_set_pid(p_dmx_data_path_info->p_hdl_en_dev, algo_info,
                                    AUI_DSC_ENCRYPT_IN, AUI_DSC_EVEN_MODE_IN)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                if (aui_dsc_pvr_start_record(p_dmx_data_path_info->p_hdl_en_dev, &algo_info)) {
                    aui_rtn(AUI_RTN_FAIL, "start pvr record fail");
                }
#endif //<--SET_PID_IN_DMX
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;

            } else if (AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT == pvr_process_mode) { /** block mode record */
                dsc_config.encrypt_fd = algo_info.pvr_fd;
                /** save the encoding setting, it will be used in aui_dmx_channel_open() */
                memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
                /** set ts mode record */
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE, DMX_REC_OUT_BLOCK)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_BLOCKSIZE, algo_info.pvr_block_size)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                dev->data_path_priv.block_size = algo_info.pvr_block_size;
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
            }
            //AUI_DBG("encrypt fd:%d,decrypt_fd: %d,pid_count:%d,encrypt record set success!\n",
            //    dsc_config.encrypt_fd,dsc_config.decrypt_fd,pid_count);
            break;
        }
        case AUI_DMX_DATA_PATH_DE_PLAY_EN_REC: {
            /** this data path is only used for live play, set see dmx source type to ts type. */
            if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_SRC_SET, (void *)AUI_DMX_M2S_SRC_NORMAL)) {
                aui_rtn(AUI_RTN_FAIL, "set normal src fail\n");
            }
            /** reset see dmx buffer size to origin size */
            if (aui_dmx_ioctl_priv(dev, AUI_DMX_M2S_BUF_VALIDSIZE_SET, (void *)0)) {
                aui_rtn(AUI_RTN_FAIL, "set valid size fail\n");
            }

            /** get dsc decrypt fd */
            if (aui_dsc_get_fd(p_dmx_data_path_info->p_hdl_de_dev, &algo_info)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dsc_config.decrypt_fd = algo_info.dsc_fd;
            /** set dsc decrypt fd */
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_DEC_HANDLE, (unsigned long)&dsc_config)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            if (alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_START, 0)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dev->crypto_started = 1;
            memset(&algo_info, 0, sizeof(struct algo_info));
            /** get dsc encrypt fd */
            if (aui_dsc_get_fd(p_dmx_data_path_info->p_hdl_en_dev,&algo_info)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            pvr_process_mode = algo_info.process_attr.process_mode;

            if (AUI_DSC_PROCESS_MODE_UNDEFINED == pvr_process_mode) {
                if (algo_info.pvr_crypto_mode) {
                    pvr_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
                } else {
                    pvr_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
                }
            }

            if (AUI_DSC_PROCESS_MODE_TS_ENCRYPT == pvr_process_mode) {
                dsc_config.encrypt_fd = algo_info.dsc_fd;
                /** save the encoding setting, it will be used in aui_dmx_channel_open() */
                memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
                /** set ts mode record */
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE, DMX_REC_OUT_TS)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
#ifdef SET_PID_IN_DMX
                if (aui_dsc_set_pid(p_dmx_data_path_info->p_hdl_en_dev, algo_info,
                                    AUI_DSC_ENCRYPT_IN, AUI_DSC_EVEN_MODE_IN)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                if (aui_dsc_pvr_start_record(p_dmx_data_path_info->p_hdl_en_dev, &algo_info)) {
                    aui_rtn(AUI_RTN_FAIL, "start pvr record fail");
                }
#endif //<--SET_PID_IN_DMX
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
            } else if (AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT == pvr_process_mode) { /** block mode record */
                dsc_config.encrypt_fd = algo_info.pvr_fd;
                /** save the encoding setting, it will be used in aui_dmx_channel_open() */
                memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
                /** set ts mode record */
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE, DMX_REC_OUT_BLOCK)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_BLOCKSIZE, algo_info.pvr_block_size)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                dev->data_path_priv.block_size = algo_info.pvr_block_size;
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
            }
            //AUI_DBG("encrypt fd:%d,decrypt_fd: %d,pid_count:%d,encrypt record set success!\n",
            //    dsc_config.encrypt_fd,dsc_config.decrypt_fd,pid_count);
            break;
        }
        case AUI_DMX_DATA_PATH_REEN_REC: {
            /** get dsc decrypt fd */
            if (aui_dsc_get_fd(p_dmx_data_path_info->p_hdl_de_dev, &algo_info)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dsc_config.decrypt_fd = algo_info.dsc_fd;
            memset(&algo_info, 0, sizeof(struct algo_info));
            /** get dsc decrypt fd */
            if (aui_dsc_get_fd(p_dmx_data_path_info->p_hdl_en_dev, &algo_info)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            pvr_process_mode = algo_info.process_attr.process_mode;
            if (AUI_DSC_PROCESS_MODE_UNDEFINED == pvr_process_mode) {
                if (algo_info.pvr_crypto_mode) {
                    pvr_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
                } else {
                    pvr_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
                }
            }
            if (AUI_DSC_PROCESS_MODE_TS_ENCRYPT == pvr_process_mode) {
                dsc_config.encrypt_fd = algo_info.dsc_fd;
                /** save the encoding setting, it will be used in aui_dmx_channel_open() */
                memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
                /** set ts mode record */
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE, DMX_REC_OUT_TS)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
#ifdef SET_PID_IN_DMX
                if (aui_dsc_set_pid(p_dmx_data_path_info->p_hdl_en_dev,algo_info,
                                    AUI_DSC_ENCRYPT_IN, AUI_DSC_EVEN_MODE_IN)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                if (aui_dsc_pvr_start_record(p_dmx_data_path_info->p_hdl_en_dev, &algo_info)) {
                    aui_rtn(AUI_RTN_FAIL, "start pvr record fail\n");
                }
#endif //<--SET_PID_IN_DMX
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
            } else if (AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT == pvr_process_mode) { /** block mode record */
                dsc_config.encrypt_fd = algo_info.pvr_fd;
                /** save the encoding setting, it will be used in aui_dmx_channel_open() */
                memcpy(&dev->enc_para, &dsc_config, sizeof(struct dmx_dsc_fd_param));
                /** set ts mode record */
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_MODE, DMX_REC_OUT_BLOCK)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                if (alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_RECORD_BLOCKSIZE, algo_info.pvr_block_size)) {
                    aui_rtn(AUI_RTN_FAIL, NULL);
                }
                dev->data_path_priv.block_size = algo_info.pvr_block_size;
                dev->data_path_priv.dsc_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
            }
            //AUI_DBG("encrypt fd:%d,decrypt_fd: %d,pid_count:%d,encrypt record set success!\n",
            //    dsc_config.encrypt_fd,dsc_config.decrypt_fd,pid_count);
            break;
        }
        default:
            break;
    }
    dev->data_path_priv.path_type = p_dmx_data_path_info->data_path_type;
    dev->data_path_priv.dev_idx = dev->dev_priv_data.dev_idx;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_data_path_set(aui_hdl p_hdl_dmx,
                                   const aui_dmx_data_path *p_dmx_data_path_info)
{
    struct dmx_device *dev = p_hdl_dmx;

    if ((NULL == dev) || (NULL == p_dmx_data_path_info)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    AUI_DBG("path type: %d\n", p_dmx_data_path_info->data_path_type);
    if (AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE == p_dmx_data_path_info->dsc_type) {
        /** dsc type handle is used when all dsc resource is shared in one process. */
        return dsc_type_hdl_data_path_set(dev, p_dmx_data_path_info);
    } else if (AUI_DMX_DATA_PATH_DSC_TYPE_FD == p_dmx_data_path_info->dsc_type) {
        /** dsc type fd is used for ca process, CA need create/operate DSC resource */
        return dsc_type_fd_data_path_set(dev, p_dmx_data_path_info);
    } else if (AUI_DMX_DATA_PATH_DSC_TYPE_ID == p_dmx_data_path_info->dsc_type) {
        /** dsc type id is used for non-ca process, non-ca knows nothing about dsc resource. */
        return dsc_type_id_data_path_set(dev, p_dmx_data_path_info);
    } else {
        aui_rtn(AUI_RTN_EINVAL, "unknown DATA_PATH_DSC_TYPE");
    }
}

AUI_RTN_CODE aui_dmx_start(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    struct dmx_device *dev = p_hdl_dmx;

    if ((NULL == dev) ||
        (NULL == dev->handle)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (alisldmx_start(dev->handle)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    /* unused */
    (void)p_attr_dmx;
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_dmx_av_stream_stop(aui_hdl p_hdl_dmx);

AUI_RTN_CODE aui_dmx_stop(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    struct dmx_device *dev = p_hdl_dmx;
    struct dmx_channel *ch, *tmp;

    if ((!dev) || (!dev->handle)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    if ((dev->crypto_started) &&
        alisldmx_ioctl(dev->handle, DMX_IOCMD_CRYPTO_STOP, 0)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    if (alisldmx_stop(dev->handle)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    /** stop dmx channel & filter really */
    aui_list_for_each_entry_safe(ch, tmp, &dev->channels, node) {
        aui_dmx_channel_close((aui_hdl*)&ch);
    }

    if (dev->av_started) {
        aui_dmx_av_stream_stop((aui_hdl*)dev);
    }
    /* unused */
    (void)p_attr_dmx;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_pause(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    struct dmx_device *dev = p_hdl_dmx;

    if ((!dev) || (!dev->handle)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    if (alisldmx_pause(dev->handle)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    /* unused */
    (void)p_attr_dmx;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_resume(aui_hdl p_hdl_dmx, const aui_attr_dmx *p_attr_dmx)
{
    struct dmx_device *dev = p_hdl_dmx;

    if ((NULL == dev) ||
        (NULL == dev->handle)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    if (alisldmx_start(dev->handle)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    /* unused */
    (void)p_attr_dmx;
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_dmx_av_stream_stop(aui_hdl p_hdl_dmx)
{
    struct dmx_device *dev = p_hdl_dmx;

    if (DMX_ILLEGAL_CHANNELID != dev->video_chid) {
        alisldmx_free_channel(dev->handle, dev->video_chid);
        //AUI_ERR("free video channel success\n");
    }
    if (DMX_ILLEGAL_CHANNELID != dev->audio_chid) {
        alisldmx_free_channel(dev->handle, dev->audio_chid);
        //AUI_ERR("free audio channel success\n");
    }
    if (DMX_ILLEGAL_CHANNELID != dev->audio_desc_chid) {
        alisldmx_free_channel(dev->handle, dev->audio_desc_chid);
        //AUI_ERR("free audio desc channel success\n");
    }
    if (DMX_ILLEGAL_CHANNELID != dev->pcr_chid) {
        alisldmx_free_channel(dev->handle, dev->pcr_chid);
        //AUI_ERR("free pcr channel success\n");
    }

    alisldmx_avstop(dev->handle);
    alislavsync_stop(dev->avsync_hdl);
    //AUI_DBG("alisldmx_avstop success\n");
    dev->audio_chid = DMX_ILLEGAL_CHANNELID;
    dev->audio_desc_chid = DMX_ILLEGAL_CHANNELID;
    dev->video_chid = DMX_ILLEGAL_CHANNELID;
    dev->pcr_chid = DMX_ILLEGAL_CHANNELID;
    dev->av_started = 0;
    return AUI_RTN_SUCCESS;
}

static void dmx_get_audio_pid(unsigned short pid,unsigned short *new_pid)
{
    alisl_handle snd_hdl;
    enum Snd_decoder_type audio_type = SND_TYPE_MPEG2;
    unsigned short audio_pid_with_codec_flag = pid;

    if (AUI_INVALID_PID <= audio_pid_with_codec_flag) {
        goto EXIT_FAIL;
    }

    alislsnd_open(&snd_hdl);
    alislsnd_get_decoder_type(snd_hdl, &audio_type);
    alislsnd_close(snd_hdl);

    switch (audio_type) {
        case SND_TYPE_MPEG_ADTS_AAC:
            audio_pid_with_codec_flag |= 0x8000;
            break;
        case SND_TYPE_MPEG_AAC: /** LATM_AAC */
            audio_pid_with_codec_flag |= 0x4000;
            break;
        case SND_TYPE_AC3:
            audio_pid_with_codec_flag |= 0x2000;
            break;
        case SND_TYPE_EC3:
            audio_pid_with_codec_flag |= 0x6000;
            break;
        default:
            break;
    }
EXIT_FAIL:
    if (new_pid != NULL) {
        *new_pid = audio_pid_with_codec_flag;
    }
}

static AUI_RTN_CODE aui_dmx_av_stream_start(aui_hdl p_hdl_dmx, unsigned long av_valid)
{
    struct dmx_device *dev = p_hdl_dmx;
    struct dmx_channel_attr attr;
    uint32_t channelid;
    dmx_cache_retrace_param_t param_t;
    int ret = AUI_RTN_FAIL;
    int see_dmx_id = 0;
    enum avsync_sync_mode sl_sync_mode = AVSYNC_PCR;

    /** create video and audio individually,
        just stop see playing, do not use aui_dmx_av_stream_stop, it will make pid 0x1fff. */
    if (dev->av_started) {
        alisldmx_avstop(dev->handle);
        alislavsync_stop(dev->avsync_hdl);
    }

    alisldmx_get_see_dmx_id(dev->handle, &see_dmx_id);

    AUI_DBG("dev->cache_type = %d\n", dev->cache_type);

    if (dev->cache_type != AUI_DMX_CACHE_NONE) { /** only need to set if use fast channel change mode */
        memset(&param_t, 0, sizeof(param_t));
        param_t.pid_list_len = 3;
        param_t.pid_list[0] = dev->video_pid;
        param_t.pid_list[1] = dev->audio_pid;
        param_t.pid_list[2] = dev->pcr_pid;

        ret = alisldmx_cache_retrace_set(dev->handle, &param_t);
        if (ret < 0) {
            AUI_ERR("set dmx cache retrace failed!\n");
        }
    }
    memset(&attr, 0, sizeof(attr));
    if ((av_valid & VIDEO_VALID) && dev->video_pid != AUI_INVALID_PID) {
        /** Ugly code to patch the high bit of pid issue */
        alisl_handle vdec_hdl = NULL;
        unsigned char video_id = 0;

        enum vdec_decoder_type video_type = VDEC_DECODER_MPEG;
        unsigned short video_pid_with_codec_flag = dev->video_pid;

        /** decv_hdl need set for PIP
            for PIP: can not configure avsync when play video only,
                because there may be another video and audio needed to do the avsync
            for normal play: configure avsync for playing video only(avsync will
                be configured agagin if there is audio)*/
        if (dev->decv_hdl == NULL) {
            enum vdec_video_id sl_video_id = 0;
            aui_decv_get_current_opened_sl_handle(&vdec_hdl);
            /** no video opened, should open video and set format first. */
            if (vdec_hdl == NULL) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            alislvdec_get_id(vdec_hdl, &sl_video_id);
            video_id = sl_video_id;
            alislavsync_set_sourcetype(dev->avsync_hdl, dev->avsync_srctype);
            alislavsync_get_av_sync_mode(dev->avsync_hdl, &sl_sync_mode);
            if ((sl_sync_mode == AVSYNC_PCR) &&
                (dev->avsync_srctype != AUI_AVSYNC_FROM_TUNER)) {
                /** when playing local stream in PCR sync mode, it will be some avsync problem,
                    set it to audio master mode. */
                alislavsync_set_av_sync_mode(dev->avsync_hdl, AVSYNC_AUDIO);
            }
            alislavsync_set_data_type(dev->avsync_hdl, AVSYNC_DATA_TYPE_TS);
            /** set video id to avsync */
            alislavsync_set_video_id(dev->avsync_hdl, (enum avsync_video_id)video_id);
            /** set see dmx id to avsync */
            alislavsync_set_see_dmx_id(dev->avsync_hdl, see_dmx_id);
        } else {
            aui_decv_get_sl_handle(dev->decv_hdl, &vdec_hdl);
            aui_decv_get_video_id(dev->decv_hdl, &video_id);
        }
        alislvdec_get_decoder(vdec_hdl, &video_type);
        if (video_type == VDEC_DECODER_MPEG) {
            video_pid_with_codec_flag = dev->video_pid;
        } else if (video_type == VDEC_DECODER_AVC) {
            video_pid_with_codec_flag = dev->video_pid | 0x2000;
        } else if (video_type == VDEC_DECODER_AVS) {
            video_pid_with_codec_flag = dev->video_pid | 0x4000;
        } else if (video_type == VDEC_DECODER_HEVC) {
            video_pid_with_codec_flag = dev->video_pid | 0x8000;
        } else {
            AUI_ERR("unknown video type for dmx\n");
        }

        /** start injecting TS stream to SEE dmx. */
        attr.stream = DMX_STREAM_VIDEO;
        ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_STREAM, &channelid);
        if (ret) {
            AUI_ERR("allocate channel failed!\n");
            goto EXIT_FAIL;
        }
        attr.video_id = video_id;
        alisldmx_set_channel_attr(dev->handle, channelid, &attr);
        alisldmx_set_channel_pid(dev->handle, channelid, video_pid_with_codec_flag);
        alisldmx_control_channel(dev->handle, channelid, DMX_CTRL_ENABLE);
        dev->video_chid = channelid;
    }

    if ((av_valid & AUDIO_VALID) && (dev->audio_pid != AUI_INVALID_PID)) {

        /** ugly code to patch the high bit of pid issue. */
        alisl_handle snd_hdl;
        enum Snd_decoder_type audio_type = SND_TYPE_MPEG2;
        unsigned short audio_pid_with_codec_flag = dev->audio_pid;
        unsigned char video_id = 0;

        /** configure avsync when playing audio. */
        alislavsync_set_sourcetype(dev->avsync_hdl, dev->avsync_srctype);
        alislavsync_set_data_type(dev->avsync_hdl, AVSYNC_DATA_TYPE_TS);
        alislavsync_get_av_sync_mode(dev->avsync_hdl, &sl_sync_mode);
        /** set see dmx id to avsync. */
        alislavsync_set_see_dmx_id(dev->avsync_hdl, see_dmx_id);
        if ((sl_sync_mode == AVSYNC_PCR) &&
            (dev->avsync_srctype != AUI_AVSYNC_FROM_TUNER)) {
            /** when playing local stream in PCR sync mode, it will be some avsync problem,
                set it to audio master mode. */
            alislavsync_set_av_sync_mode(dev->avsync_hdl, AVSYNC_AUDIO);
        }

        /** for normal play: configure decv_hdl, need to configure the video id to avsync. */
        if (dev->decv_hdl) {
            aui_decv_get_video_id(dev->decv_hdl, &video_id);
            /** set video id to avsync */
            alislavsync_set_video_id(dev->avsync_hdl, (enum avsync_video_id)video_id);
        }
        alislsnd_open(&snd_hdl);
        alislsnd_get_decoder_type(snd_hdl, &audio_type);
        alislsnd_close(snd_hdl);

        switch (audio_type) {
            case SND_TYPE_MPEG_ADTS_AAC:
                audio_pid_with_codec_flag |= 0x8000;
                break;
            case SND_TYPE_MPEG_AAC: /** LATM_AAC */
                audio_pid_with_codec_flag |= 0x4000;
                break;
            case SND_TYPE_AC3:
                audio_pid_with_codec_flag |= 0x2000;
                break;
            case SND_TYPE_EC3:
                audio_pid_with_codec_flag |= 0x6000;
                break;
            default:
                break;
        }
        memset(&attr, 0, sizeof(attr));
        /** start injecting TS stream to SEE dmx. */
        attr.stream = DMX_STREAM_AUDIO;
        ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_STREAM, &channelid);
        if (ret) {
            AUI_ERR("allocate channel failed!\n");
            goto EXIT_FREE_VIDEO_CHID;
        }
        alisldmx_set_channel_attr(dev->handle, channelid, &attr);
        alisldmx_set_channel_pid(dev->handle, channelid, audio_pid_with_codec_flag);
        alisldmx_control_channel(dev->handle, channelid, DMX_CTRL_ENABLE);
        dev->audio_chid = channelid;
    }

    if ((av_valid & AUDIO_DESC_VALID) &&
        (dev->audio_desc_pid != AUI_INVALID_PID)) {
        memset(&attr, 0, sizeof(attr));
        /** ugly code to patch the high bit of pid issue. */
        alisl_handle snd_hdl;
        enum Snd_decoder_type audio_type = SND_TYPE_MPEG2;
        unsigned short audio_pid_with_codec_flag = dev->audio_desc_pid;

        alislsnd_open(&snd_hdl);
        alislsnd_get_decoder_type(snd_hdl, &audio_type);
        alislsnd_close(snd_hdl);

        switch (audio_type) {
            case SND_TYPE_MPEG_ADTS_AAC:
                audio_pid_with_codec_flag |= 0x8000;
                break;
            case SND_TYPE_MPEG_AAC: /** LATM_AAC */
                audio_pid_with_codec_flag |= 0x4000;
                break;
            case SND_TYPE_AC3:
                audio_pid_with_codec_flag |= 0x2000;
                break;
            case SND_TYPE_EC3:
                audio_pid_with_codec_flag |= 0x6000;
                break;
            default:
                break;
        }

        attr.stream = DMX_STREAM_AUDIO_DESCRIPTION;
        ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_STREAM, &channelid);
        if (ret) {
            AUI_ERR("allocate channel failed!\n");
            goto EXIT_FREE_AUDIO_CHID;
        }
        alisldmx_set_channel_attr(dev->handle, channelid, &attr);
        alisldmx_set_channel_pid(dev->handle, channelid, audio_pid_with_codec_flag);
        alisldmx_control_channel(dev->handle, channelid, DMX_CTRL_ENABLE);
        dev->audio_desc_chid = channelid;
    }
    /** If video pid and pcr pid is same, main dmx should not allocate pcr channel
            to decrease main driver loading.
        If video pid and pcr pid is different, main dmx need allocate pcr channel to send ts containing pcr to SEE,
            SEE dmx parses pcr for avsync.
        No matter what pcr pid and video pid is same or not, user space need set audio, video, pcr pid to see dmx
            by alisldmx_avstart api for avsync. In other word, see dmx need know what the current pcr pid is. */
    if ((av_valid & VIDEO_VALID) &&
        (dev->video_pid != AUI_INVALID_PID) &&
        ((dev->pcr_pid & AUI_INVALID_PID) != AUI_INVALID_PID)) {
        memset(&attr, 0, sizeof(attr));
        attr.stream = DMX_STREAM_PCR;
        attr.output_format = DMX_OUTPUT_FORMAT_PCR;
        ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_STREAM, &channelid);
        if (ret) {
            AUI_ERR("allocate channel failed!\n");
            goto EXIT_FREE_AUDIO_DESC_CHID;
        }
        alisldmx_set_channel_attr(dev->handle, channelid, &attr);
        alisldmx_set_channel_pid(dev->handle, channelid, dev->pcr_pid);
        /*
        For pdk1.6.x - pdk1.9, pcr is parsed by hardware on main side.
        From pdk1.10, when pcr pid is different with video pid, main dmx should allocate pcr channel
        to send ts containing pcr to SEE, then see dmx parses pcr for avsync.
        From PDK1.10a-20170119, pcr is parsed by hardware on main size, the same as pdk1.9.
        */
        alisldmx_control_channel(dev->handle, channelid, DMX_CTRL_ENABLE);
        dev->pcr_chid = channelid;
    }
    if (dev->cache_type != AUI_DMX_CACHE_NONE) { /** only need to set if use fast channel change mode. */
        ret = alisldmx_cache_retrace_start(dev->handle);
        if (ret < 0) {
            AUI_ERR("cache retrace_start failed!\n");
        }
    }
    alislavsync_start(dev->avsync_hdl);
    alisldmx_avstart(dev->handle);
    dev->av_started = 1;
    return AUI_RTN_SUCCESS;

EXIT_FREE_AUDIO_DESC_CHID:
    if ((av_valid & AUDIO_DESC_VALID) && (dev->audio_desc_pid != AUI_INVALID_PID)) {
        ret = alisldmx_free_channel(dev->handle, dev->audio_desc_chid);
        if (ret) {
            AUI_ERR("free channel failed!\n");
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
EXIT_FREE_AUDIO_CHID:
    if ((av_valid & AUDIO_VALID) && (dev->audio_pid != AUI_INVALID_PID)) {
        ret = alisldmx_free_channel(dev->handle, dev->audio_chid);
        if (ret) {
            AUI_ERR("Free channel failed!\n");
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
EXIT_FREE_VIDEO_CHID:
    if ((av_valid & VIDEO_VALID) && (dev->video_pid != AUI_INVALID_PID)) {
        ret = alisldmx_free_channel(dev->handle, dev->video_chid);
        if (ret) {
            AUI_ERR("free channel failed!\n");
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
EXIT_FAIL:
    aui_rtn(AUI_RTN_FAIL, NULL);
}

static unsigned short find_pid(struct aui_dmx_stream_pid *pid_info,
                               aui_dmx_stream_type type)
{
    unsigned long i = 0;

    for (i = 0; i < pid_info->ul_pid_cnt; i++) {
        if (pid_info->stream_types[i] == type) {
            /** effective PID is 13bits, The rang of effective PID is [0, 0x1fff),
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

AUI_RTN_CODE aui_dmx_set(aui_hdl p_hdl_dmx,
                         unsigned long ul_item,
                         void *pv_param)
{
    struct dmx_device *dev = p_hdl_dmx;
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    int ret = AUI_RTN_FAIL;

    if ((NULL == dev) ||
        (NULL == dev->handle) ||
        (ul_item >= AUI_DMX_SET_CMD_LAST)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DMX_TYPE_SET: {
            if(NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_CREATE_AV: {
            struct aui_dmx_stream_pid *p_pids_info = pv_param;

            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (p_pids_info->ul_pid_cnt > AUI_DMX_STREAM_PIDS_CNT_MAX) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            dev->video_pid = AUI_INVALID_PID;
            dev->audio_pid = AUI_INVALID_PID;
            dev->audio_desc_pid = AUI_INVALID_PID;
            dev->pcr_pid = AUI_INVALID_PID;
            dev->video_pid = find_pid(p_pids_info, AUI_DMX_STREAM_VIDEO);
            dev->audio_pid = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO);
            dev->audio_desc_pid = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO_DESC);
            if (dev->audio_desc_pid == dev->audio_pid) {
                dev->audio_desc_pid = AUI_INVALID_PID;
            }
            dev->pcr_pid = find_pid(p_pids_info, AUI_DMX_STREAM_PCR);
            dev->front_end = p_pids_info->ul_magic_num;
            if (dev->decv_hdl == NULL) {
                dev->decv_hdl = p_pids_info->decv_handle;
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_CREATE_AUDIO: {
            struct aui_dmx_stream_pid *p_pids_info = pv_param;
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (p_pids_info->ul_pid_cnt > AUI_DMX_STREAM_PIDS_CNT_MAX) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            dev->audio_pid = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO);
            dev->audio_desc_pid = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO_DESC);
            if (dev->audio_desc_pid == dev->audio_pid)
                dev->audio_desc_pid = AUI_INVALID_PID;
            dev->front_end = p_pids_info->ul_magic_num;

            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_CREATE_VIDEO: {
            struct aui_dmx_stream_pid *p_pids_info = pv_param;
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (p_pids_info->ul_pid_cnt > AUI_DMX_STREAM_PIDS_CNT_MAX) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            dev->video_pid = find_pid(p_pids_info, AUI_DMX_STREAM_VIDEO);
            dev->pcr_pid = find_pid(p_pids_info, AUI_DMX_STREAM_PCR);
            if (dev->decv_hdl == NULL) {
                dev->decv_hdl = p_pids_info->decv_handle;
            }
            dev->front_end = p_pids_info->ul_magic_num;
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_ENABLE: {
            alisldmx_set_front_end(dev->handle, dev->front_end);
            /**
             * TODO:
             *     Set nim chipid here,
             *     but how to get nim_chipid???
             */
            alisldmx_set_nim_chipid(dev->handle, 0/*nim_chipid*/);
            rtn_code = aui_dmx_av_stream_start(p_hdl_dmx,
                                               VIDEO_VALID |
                                               AUDIO_VALID |
                                               AUDIO_DESC_VALID);
            break;
        }
        case AUI_DMX_STREAM_ENABLE_AUDIO: {
            alisldmx_set_front_end(dev->handle, dev->front_end);
            /**
             * TODO:
             *     Set nim chipid here,
             *     but how to get nim_chipid???
             */
            alisldmx_set_nim_chipid(dev->handle, 0/*nim_chipid*/);
            rtn_code = aui_dmx_av_stream_start(p_hdl_dmx,
                                               AUDIO_VALID |
                                               AUDIO_DESC_VALID);
            break;
        }
        case AUI_DMX_STREAM_ENABLE_VIDEO: {
            alisldmx_set_front_end(dev->handle, dev->front_end);
            /**
             * TODO:
             *     Set nim chipid here,
             *     but how to get nim_chipid???
             */
            alisldmx_set_nim_chipid(dev->handle, 0/*nim_chipid*/);
            rtn_code = aui_dmx_av_stream_start(p_hdl_dmx,
                                               VIDEO_VALID);
            break;
        }
        case AUI_DMX_STREAM_DISABLE_AUDIO: {
            if (dev->audio_chid != DMX_ILLEGAL_CHANNELID) {
                ret = alisldmx_free_channel(dev->handle, dev->audio_chid);
                if (ret) {
                    AUI_ERR("free channel failed!\n");
                }
                dev->audio_chid = DMX_ILLEGAL_CHANNELID;
            }
            if (dev->audio_desc_chid != DMX_ILLEGAL_CHANNELID) {
                ret = alisldmx_free_channel(dev->handle, dev->audio_desc_chid);
                if (ret) {
                    AUI_ERR("free channel failed!\n");
                }
                dev->audio_desc_chid = DMX_ILLEGAL_CHANNELID;
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_DISABLE_VIDEO: {
            if (dev->video_chid != DMX_ILLEGAL_CHANNELID) {
                ret = alisldmx_free_channel(dev->handle, dev->video_chid);
                if (ret) {
                    AUI_ERR("free channel failed!\n");
                }
                dev->video_chid = DMX_ILLEGAL_CHANNELID;
            }
            if (dev->pcr_chid != DMX_ILLEGAL_CHANNELID) {
                ret = alisldmx_free_channel(dev->handle, dev->pcr_chid);
                if (ret) {
                    AUI_ERR("free channel failed!\n");
                }
                dev->pcr_chid = DMX_ILLEGAL_CHANNELID;
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_STREAM_DISABLE: {
            aui_dmx_av_stream_stop(p_hdl_dmx);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_BYPASS_CSA: {
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_AVSYNC_MODE: {
#if 1
            enum aui_dmx_avsync_mode avsync_mode;
            avsync_mode = (enum aui_dmx_avsync_mode)pv_param;
            /** default AUI_DMX_AVSYNC_NONE set src to "from tuner". */
            if ((avsync_mode == AUI_DMX_AVSYNC_NONE) ||
                (avsync_mode == AUI_DMX_AVSYNC_LIVE)){
                alislavsync_set_sourcetype(dev->avsync_hdl, AVSYNC_FROM_TUNER);
            } else if (avsync_mode == AUI_DMX_AVSYNC_LIVE){
                alislavsync_set_sourcetype(dev->avsync_hdl, AVSYNC_FROM_TUNER);
            } else {
                alislavsync_set_sourcetype(dev->avsync_hdl, AVSYNC_FROM_HDD_MP);
            }
#else
            if (alisldmx_set_avsync_mode(dev->handle, (unsigned long)pv_param))
                aui_rtn(AUI_RTN_FAIL, NULL);
#endif //<--#if 1
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_AVSYNC_SOURCE_TYPE: {
            aui_avsync_srctype_t avsync_srctype;
            avsync_srctype = (aui_avsync_srctype_t)pv_param;
            if (avsync_srctype != AUI_AVSYNC_FROM_TUNER) {
                /** driver only supports from turner and local. */
                avsync_srctype = AUI_AVSYNC_FROM_HDD_MP;
            }
            if (alislavsync_set_sourcetype(dev->avsync_hdl, avsync_srctype)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            dev->avsync_srctype = avsync_srctype;
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }

        case AUI_DMX_SET_PLAYBACK_PARAM: {
            struct dmx_playback_param *p_dmx2_param;
            p_dmx2_param = (struct dmx_playback_param *)pv_param;
            alisldmx_set_playback_param(dev->handle, p_dmx2_param);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_DIRECT_WRITE: {
            struct dmx_write_playback_param *p_dmx2_write_param;
            p_dmx2_write_param = (struct dmx_write_playback_param *)pv_param;
            alisldmx_direct_write_playback(dev->handle, p_dmx2_write_param);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_FAST_CHANNEL_CHANGE_PARAM: {
            dmx_cache_param_t param;
        
            param.type = ((aui_dmx_cache_param_t *)pv_param)->type;
            param.pid_list_len = ((aui_dmx_cache_param_t *)pv_param)->pid_list_len;
            param.pid_list = &((aui_dmx_cache_param_t *)pv_param)->pid_list[0];

            rtn_code = alisldmx_cache_set(dev->handle, &param);

            if (rtn_code) {
                AUI_ERR("alisldmx_cache_set type[%d] return fail!\n", param.type);
            } else {
                /*
                 * Need to save cache_type for aui_dmx_av_stream_start.
                 * If not, when FCC mode, the output data does not reach the maximum amount of dmx cache buffer.
                 * So the FCC speed will be slower. 
                 */
                dev->cache_type = (aui_dmx_cache_type)param.type;

                /* Open or close fast mode according to the cache type. */
                if ((aui_dmx_cache_type)param.type == AUI_DMX_CACHE_NONE) {
                    alislavsync_set_fcc_onoff(dev->avsync_hdl, 0);
                } else {
                    alislavsync_set_fcc_onoff(dev->avsync_hdl, 1);
                }
            }
            break;
        }
        case AUI_DMX_SET_PLAYBACK_SPEED: {
            long speed = (long)pv_param;
            rtn_code = alisldmx_ioctl(dev->handle, DMX_IOCMD_SET_PLAYBACK_SPEED, speed);
            break;
        }
        case AUI_DMX_SET_IO_REC_MODE: {
            if (*(int *)pv_param == 0) {
                rtn_code = alisldmx_ioctl(dev->handle, DMX_IO_SET_FTA_REC_MODE, DMX_FTA_TO_FTA);

            } else if (*(int *)pv_param == 1) {
                rtn_code = alisldmx_ioctl(dev->handle, DMX_IO_SET_FTA_REC_MODE, DMX_FTA_TO_ENCRYPT);
            }
            //AUI_DBG("set AUI_DMX_SET_IO_REC_MODE rc=%lu success!\n", rtn_code);
            break;
        }
        case AUI_DMX_SET_CHANGE_AUD_STREM: {
            struct aui_dmx_stream_pid *p_pids_info = pv_param;
            unsigned short audio_pid_with_codec_flag = AUI_INVALID_PID;
            unsigned short ad_pid_with_codec_flag = AUI_INVALID_PID;

            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (p_pids_info->ul_pid_cnt > AUI_DMX_STREAM_PIDS_CNT_MAX) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            dev->audio_pid = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO);
            dev->audio_desc_pid = find_pid(p_pids_info, AUI_DMX_STREAM_AUDIO_DESC);
            if (dev->audio_desc_pid == dev->audio_pid) {
                dev->audio_desc_pid = AUI_INVALID_PID;
            }
            dmx_get_audio_pid(dev->audio_pid, &audio_pid_with_codec_flag);
            dmx_get_audio_pid(dev->audio_desc_pid, &ad_pid_with_codec_flag);
            //AUI_DBG("dev->audio_pid: 0x%x(%d), dev->audio_desc_pid: 0x%x(%d)\n",
            //    dev->audio_pid, dev->audio_pid, dev->audio_desc_pid, dev->audio_desc_pid);
            alisldmx_change_audio_pid(dev->handle,
                                      audio_pid_with_codec_flag,
                                      ad_pid_with_codec_flag,
                                      &dev->audio_chid,
                                      &dev->audio_desc_chid);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_SET_CLEAR_DISCONTINUE_PKG_CNT: {
            rtn_code = alisldmx_clear_discontinue_av_pkt_cnt(dev->handle);
            break;
        }
        default: {
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_get(aui_hdl p_hdl_dmx,
                         unsigned long ul_item,
                         void *pv_param)
{
    struct dmx_device *dev = p_hdl_dmx;
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    if ((NULL == dev) ||
        (NULL == dev->handle) ||
        (ul_item > AUI_DMX_GET_CMD_LAST)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DMX_TYPE_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_HANDLE_SIZE_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = sizeof(struct dmx_device);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_CHANNEL_HANDLE_SIZE_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = sizeof(struct dmx_channel);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_HANDLE_SIZE_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = sizeof(struct dmx_filter);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_ATTR_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            pv_param = &(dev->attr_dmx);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_RCV_PKG_CNT_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = alisldmx_ioctl(dev->handle, DMX_IOCMD_RCV_PKG_CNT_GET, (unsigned long)pv_param);
            if (rtn_code != 0){
                *(unsigned long *)pv_param = 0;
                AUI_DBG("get AUI_DMX_RCV_PKG_CNT_GET rc=%lu \n", rtn_code);
                aui_rtn(DMX_ERR, NULL);
            } else {
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        case AUI_DMX_RCV_TS_PKG_CNT_GET_BY_PID: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            aui_dmx_get_ts_pkg_cnt_by_pid *p_get_para = pv_param;
            struct sl_dmx_get_ts_pkg_cnt_by_pid io_param;
            memset(&io_param, 0, sizeof(struct sl_dmx_get_ts_pkg_cnt_by_pid));
            io_param.ul_pid = p_get_para->ul_pid;
            rtn_code = alisldmx_ioctl(dev->handle, DMX_IOCMD_RCV_TS_PKG_CNT_GET_BY_PID, (unsigned long)&io_param);
            if (rtn_code != 0){
                *(unsigned long *)pv_param = 0;
                AUI_DBG("get AUI_DMX_RCV_TS_PKG_CNT_GET_BY_PID rc=%lu \n", rtn_code);
                aui_rtn(DMX_ERR, NULL);
            } else {
                p_get_para->ul_ts_pkg_cnt = io_param.ul_ts_pkg_cnt;
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        case AUI_DMX_GET_FREE_BUF_LEN: {
            if (alisldmx_ioctl(dev->handle,
                               DMX_IOCMD_GET_FREE_BUF_LEN,
                               (unsigned long)pv_param)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_GET_WRITE_BUF_LEN: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            alisldmx_get_write_pb_length(dev->handle, pv_param);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_GET_IO_REC_MODE: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = alisldmx_ioctl(dev->handle,DMX_IO_GET_FTA_REC_MODE,(unsigned long)pv_param);
            if (rtn_code != 0){
                AUI_ERR("get AUI_DMX_GET_IO_REC_MODE rc=%lu success!\n", rtn_code);
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            //AUI_DBG("get AUI_DMX_GET_IO_REC_MODE rc=%lu success!\n", rtn_code);
            if (*(int *)pv_param == (int)DMX_FTA_TO_FTA) {
                *(unsigned int *)pv_param = 0;
            } else if (*(int *)pv_param == (int)DMX_FTA_TO_ENCRYPT) {
                *(unsigned int *)pv_param = 1;
            } else {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            break;
        }
        case AUI_DMX_GET_PTS: {
            aui_pts* pts_tmp = NULL;
            unsigned int pts_param = 0;
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            pts_tmp = (aui_pts*)pv_param;
            if (alislavsync_get_current_pts(dev->avsync_hdl, &pts_param)) {
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
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_GET_CUR_STC: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            alislavsync_get_current_stc(dev->avsync_hdl, (unsigned int *)pv_param);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_GET_TOTAL_BUF_LEN: {
            if (alisldmx_ioctl(dev->handle,
                               DMX_IOCMD_GET_TOTAL_BUF_LEN,
                               (unsigned long)pv_param)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_GET_PROG_BITRATE: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            /** Looks strange, DMX_IOCMD_GET_AUDIO_BITRATE will get the total bitrate of audio and vidio,
                maybe pdk do not support for getting individaul pid bitrate. */
            if (alisldmx_ioctl(dev->handle,
                               DMX_IOCMD_GET_AUDIO_BITRATE,
                               (unsigned int)pv_param)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            AUI_DBG("AUI_DMX_GET_PROG_BITRATE: %lu \n", *(unsigned long *)pv_param);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_GET_STREAM_ENCRYPT_INFO: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, "The input parameter is NULL!\n");
            }
            aui_dmx_stream_encrypt_type stream_encrypt_type = AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM;
            rtn_code = alisldmx_get_scram_status(dev->handle, (unsigned int *)(&stream_encrypt_type));
            if (AUI_RTN_SUCCESS != rtn_code) {
                *(unsigned int *)pv_param = AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM;
                aui_rtn(AUI_RTN_FAIL, "alisldmx_get_scram_status return FAIL!\n");
            }
            *(unsigned int *)pv_param = stream_encrypt_type;
            AUI_DBG("aui_en_dmx_stream_encrypt_type: %d\n", *(unsigned int*)pv_param);
            break;
        }
        case AUI_DMX_GET_STREAM_PID_DECRYPT_INFO: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, "The input parameter is NULL!\n");
            }
            aui_dmx_stream_pid_decrypt_info *p_decrypt_info = pv_param;
            /* Because SEE code limitation, should be set a non-zero value to get real encrypt_type */
            unsigned int scrambling_status = AUI_DMX_STREAM_ENCRYPT_TYPE_LAST;
            AUI_DBG("ul_pids: pid[0] = %d, pid[1] = %d\n", p_decrypt_info->ul_pids[0], p_decrypt_info->ul_pids[1]);
            rtn_code = alisldmx_get_scram_status_ext(dev->handle,
                                                     p_decrypt_info->ul_pids,
                                                     2, /* driver support video_pid (ul_pids[0]) and audio_pid (ul_pids[1]) */
                                                     &scrambling_status);
            if (AUI_RTN_SUCCESS != rtn_code) {
                AUI_ERR("It is driver copy_to_user return fail, not get scrambling_status fail!\n");
                aui_rtn(AUI_RTN_FAIL, "alisldmx_get_scram_status_ext return FAIL!\n");
            } else {
                /* If the input value is not changed, may be decryption fail, may also be clear stream */
                if (AUI_DMX_STREAM_ENCRYPT_TYPE_LAST == scrambling_status) {
                    p_decrypt_info->ul_result = AUI_DMX_STREAM_DECRYPT_FAIL;
                    p_decrypt_info->ul_info = AUI_DMX_STREAM_ENCRYPT_TYPE_CLEAR_STREAM;
                } else {
                    p_decrypt_info->ul_result = AUI_DMX_STREAM_DECRYPT_SUCCESS;
                    p_decrypt_info->ul_info = scrambling_status & 0xff;
                }
            }
            AUI_DBG("ul_result = %ld, ul_info = %ld\n", p_decrypt_info->ul_result, p_decrypt_info->ul_info);
            break;
        }
        case AUI_DMX_GET_DISCONTINUE_PKG_CNT: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            if (alisldmx_get_discontinue_pkt_cnt(dev->handle, (unsigned int *)pv_param)) {
                aui_rtn(AUI_RTN_FAIL, NULL);
            } else {
                rtn_code = AUI_RTN_SUCCESS;
            }
            break;
        }
        default: {
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_channel_open(aui_hdl p_hdl_dmx,
                                  const aui_attr_dmx_channel *p_attr_channel,
                                  aui_hdl *const pp_hdl_dmx_channel)
{
    struct dmx_device *dev = p_hdl_dmx;
    struct dmx_channel *channel = NULL;
    struct dmx_channel_attr attr;
    unsigned int channelid = 0;
    int ret = AUI_RTN_FAIL;

    if ((NULL == p_attr_channel) ||
        (NULL == dev) ||
        (NULL == pp_hdl_dmx_channel)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    channel = malloc(sizeof(*channel));
    if (NULL == channel) {
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }
    memset(channel, 0, sizeof(*channel));
    memcpy(&(channel->attr_channel), p_attr_channel, sizeof(*p_attr_channel));

    channel->id = DMX_ILLEGAL_CHANNELID;
    channel->dmx_device = dev;

    memset(&attr, 0, sizeof(attr));
    switch (channel->attr_channel.dmx_data_type) {
        case AUI_DMX_DATA_SECT:
            attr.is_encrypted = channel->attr_channel.is_encrypted;
            attr.output_format = DMX_OUTPUT_FORMAT_SEC;
            ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_SECTION, &channelid);
            if (ret) {
                AUI_ERR("allocate channel failed!\n");
                free(channel);
                channel = NULL;
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            alisldmx_set_channel_attr(dev->handle, channelid, &attr);
            if (AUI_INVALID_PID != channel->attr_channel.us_pid) {
                alisldmx_set_channel_pid(dev->handle, channelid, channel->attr_channel.us_pid);
            } else if (channel->attr_channel.ul_pid_cnt > 0) {
                alisldmx_set_channel_pidlist(dev->handle, channelid, channel->attr_channel.us_pid_list,
                                             channel->attr_channel.ul_pid_cnt);
            }
            channel->id = channelid;

            /** add channel ringbuf start -->, just support section synchronous get data */
            if (channel->attr_channel.dmx_channel_sec_sync_get_data_support) {
                if (AUI_RTN_SUCCESS != aui_common_init_ring_buf(AUI_DMX_CHANNEL_BUF_LEN, &(channel->chan_rbuf))) {
                    AUI_ERR("aui_common_init_ring_buf fail!\n");
                    free(channel);
                    channel = NULL;
                    aui_rtn(AUI_RTN_EINVAL, NULL);
                }
                AUI_DBG("init ring buf done, get: %p\n", channel->chan_rbuf.pby_ring_buf);
            }
            /** <-- add channel ringbuf stop */
            break;
        case AUI_DMX_DATA_RAW:  /** Support only once for A kind of PID TS data, at the same time; not support pid_list; not support AUI_FULL_TS_PID */
            if (AUI_INVALID_PID == channel->attr_channel.us_pid) {
                free(channel);
                channel = NULL;
                return AUI_RTN_FAIL;
            }
            attr.output_format = DMX_OUTPUT_FORMAT_TS;
            attr.enc_para = NULL; /** in AUI_DMX_DATA_RAW mode, ts data will not be re-encrypted. */
            /** for AUI_DMX_DATA_RAW, set uncache_para = 1 to inform driver do not cache data,
                and call user callback immediatly. */
            attr.uncache_para = 1;
            ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_RECORD, &channelid);
            if (ret) {
                AUI_ERR("allocate channel failed!\n");
                free(channel);
                channel = NULL;
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            alisldmx_set_channel_attr(dev->handle, channelid, &attr);
            if (AUI_INVALID_PID != channel->attr_channel.us_pid) {
                alisldmx_set_channel_pid(dev->handle, channelid, channel->attr_channel.us_pid);
            }
            channel->id = channelid;
            break;
        case AUI_DMX_DATA_ES:
        case AUI_DMX_DATA_REC:
            attr.output_format = DMX_OUTPUT_FORMAT_TS;
            attr.enc_para = &dev->enc_para;
            /** if data path is AUI_DMX_DATA_REC, should set attr.enc_para NULL,
                or else can`t record raw data with dscrambled stream. */
            if (AUI_DMX_DATA_PATH_REC == dev->data_path_priv.path_type) {
                attr.enc_para = NULL;
            }
            ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_RECORD, &channelid);
            if (ret) {
                AUI_ERR("allocate channel failed!\n");
                free(channel);
                channel = NULL;
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            /** when pid_cnt is bigger than our limitation, rec_whole_tp. */
            if (channel->attr_channel.ul_pid_cnt > AUI_DMX_REC_PID_LIST_MAX_LEN ||
                    channel->attr_channel.us_pid == AUI_FULL_TS_PID) {
                attr.rec_whole_tp = 1;
            }
            alisldmx_set_channel_attr(dev->handle, channelid, &attr);
            if (AUI_INVALID_PID != channel->attr_channel.us_pid) {
                alisldmx_set_channel_pid(dev->handle, channelid, channel->attr_channel.us_pid);
            } else if ((channel->attr_channel.ul_pid_cnt > 0) &&
                       (channel->attr_channel.ul_pid_cnt <= AUI_DMX_REC_PID_LIST_MAX_LEN)) {
                alisldmx_set_channel_pidlist(dev->handle, channelid,
                                             channel->attr_channel.us_pid_list,
                                             channel->attr_channel.ul_pid_cnt);
            }
            channel->id = channelid;
            break;
        case AUI_DMX_DATA_PES:
            attr.is_encrypted = channel->attr_channel.is_encrypted;
            attr.output_format = DMX_OUTPUT_FORMAT_PES;
            attr.enc_para = &dev->enc_para;
            ret = alisldmx_allocate_channel(dev->handle, DMX_CHANNEL_RECORD, &channelid);
            if (ret) {
                AUI_ERR("allocate channel failed!\n");
                free(channel);
                channel = NULL;
                aui_rtn(AUI_RTN_FAIL, NULL);
            }
            alisldmx_set_channel_attr(dev->handle, channelid, &attr);
            if (AUI_INVALID_PID != channel->attr_channel.us_pid) {
                alisldmx_set_channel_pid(dev->handle, channelid, channel->attr_channel.us_pid);
            } else if (channel->attr_channel.ul_pid_cnt > 0) {
                alisldmx_set_channel_pidlist(dev->handle, channelid,
                                             channel->attr_channel.us_pid_list,
                                             channel->attr_channel.ul_pid_cnt);
            }
            channel->id = channelid;
            break;
        default:
            free(channel);
            channel = NULL;
            aui_rtn(AUI_RTN_FAIL, NULL);
    }
    AUI_INIT_LIST_HEAD(&channel->filters);
    pthread_mutex_lock(&token_mutex);
    aui_list_add_tail(&channel->node, &dev->channels);
    pthread_mutex_unlock(&token_mutex);
    *pp_hdl_dmx_channel = channel;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_close(aui_hdl* pp_hdl_dmx_channel)
{
    struct dmx_device *dev = NULL;
    struct dmx_channel *channel = NULL;
    struct dmx_filter *filter = NULL;
    struct dmx_filter *tmp = NULL;
    int ret = AUI_RTN_FAIL;

    if (NULL == pp_hdl_dmx_channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    channel = (struct dmx_channel *)(*pp_hdl_dmx_channel);
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    dev = channel->dmx_device;
    /** free channel ringbuf start -->*/
    if(channel->chan_rbuf.pby_ring_buf)
    {
        AUI_DBG("free rbuf: %p\n", channel->chan_rbuf.pby_ring_buf);
        aui_common_un_init_ring_buf(&(channel->chan_rbuf));
    }
    /** <-- free channel ringbuf stop */

    aui_list_for_each_entry_safe(filter, tmp, &channel->filters, node) {
        aui_dmx_filter_close((aui_hdl*)&filter);
    }
    if (DMX_ILLEGAL_CHANNELID != channel->id) {
        ret = alisldmx_free_channel(dev->handle, channel->id);
        if (ret) {
            AUI_ERR("free channel failed!\n");
        }
    }
    pthread_mutex_lock(&token_mutex);
    aui_list_del(&channel->node);
    pthread_mutex_unlock(&token_mutex);
    free(channel);
    *pp_hdl_dmx_channel = NULL;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_start(aui_hdl p_hdl_dmx_channel,
                                   const aui_attr_dmx_channel *p_attr_channel)
{
    struct dmx_channel *channel = p_hdl_dmx_channel;
    struct dmx_device *dev = NULL;

    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel->attr_channel.dmx_channel_status = AUI_DMX_CHANNEL_RUN;
    dev = channel->dmx_device;
    if (channel->id != DMX_ILLEGAL_CHANNELID) {
        alisldmx_control_channel(dev->handle, channel->id, DMX_CTRL_ENABLE);
    }

    /* unused */
    (void)p_attr_channel;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_stop(aui_hdl p_hdl_dmx_channel,
                                  const aui_attr_dmx_channel *p_attr_channel)
{
    struct dmx_channel *channel = p_hdl_dmx_channel;
    struct dmx_device *dev = NULL;

    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel->attr_channel.dmx_channel_status = AUI_DMX_CHANNEL_STOP;
    dev = channel->dmx_device;
    if (DMX_ILLEGAL_CHANNELID != channel->id) {
        alisldmx_control_channel(dev->handle, channel->id, DMX_CTRL_DISABLE);
    }

    /* unused */
    (void)p_attr_channel;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_set(aui_hdl p_hdl_dmx_channel,
                                 unsigned long ul_item,
                                 void *pv_param)
{
    struct dmx_channel *channel = p_hdl_dmx_channel;
    struct dmx_device *dev;
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    if ((NULL == channel) ||
        (ul_item >= AUI_DMX_SET_CMD_LAST)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    switch (ul_item) {
        case AUI_DMX_CHANNEL_PID_SET: {
            channel->attr_channel.us_pid = (unsigned short)((unsigned int)pv_param);
            if (DMX_ILLEGAL_CHANNELID != channel->id) {
                rtn_code = alisldmx_set_channel_pid(dev->handle, channel->id, channel->attr_channel.us_pid);
            }
            break;
        }
        default: {
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_channel_add_pid(aui_hdl hdl_dmx_channel,
                                     const aui_dmx_channel_pid_list *p_channel_pid_list)
{
    struct dmx_channel *channel = hdl_dmx_channel;
    struct dmx_device *dev = NULL;

    if ((NULL == channel) ||
        (NULL == p_channel_pid_list) ||
        (DMX_ILLEGAL_CHANNELID == channel->id) ||
        (AUI_DMX_DATA_REC != channel->attr_channel.dmx_data_type)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;

    if (alisldmx_add_channel_pidlist(dev->handle, channel->id,
                                     p_channel_pid_list->pids,
                                     p_channel_pid_list->pid_cnt)) {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_del_pid(aui_hdl hdl_dmx_channel,
                                     const aui_dmx_channel_pid_list *p_channel_pid_list)
{
    struct dmx_channel *channel = hdl_dmx_channel;
    struct dmx_device *dev = NULL;

    if ((NULL == channel) ||
        (NULL == p_channel_pid_list) ||
        (DMX_ILLEGAL_CHANNELID == channel->id) ||
        (AUI_DMX_DATA_REC != channel->attr_channel.dmx_data_type)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    dev = channel->dmx_device;
    if (alisldmx_del_channel_pidlist(dev->handle, channel->id,
                                     p_channel_pid_list->pids,
                                     p_channel_pid_list->pid_cnt)) {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_get(aui_hdl p_hdl_dmx_channel,
                                 unsigned long ul_item,
                                 void *pv_param)
{
    struct dmx_channel *channel = p_hdl_dmx_channel;
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    if ((NULL == channel) ||
        (ul_item > AUI_DMX_GET_CMD_LAST)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DMX_CHANNEL_PID_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned short *)pv_param = channel->attr_channel.us_pid;
            *(unsigned long *)pv_param = (*(unsigned long *)pv_param) & AUI_INVALID_PID;
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_CHANNEL_ATTR_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = (unsigned long) & (channel->attr_channel);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_filter_open(aui_hdl p_hdl_dmx_channel,
                                 const aui_attr_dmx_filter *p_attr_filter,
                                 aui_hdl *const pp_hdl_dmx_filter)
{
    struct dmx_channel_callback cb;
    cb_filter_token_t filter_token;
    struct dmx_channel *channel = p_hdl_dmx_channel;
    struct dmx_filter *filter = NULL;
    struct dmx_device *dev = NULL;
    unsigned long ul_mask_val_len = 0;
    unsigned long i = 0;

    if ((NULL == p_attr_filter) ||
        (NULL == channel) ||
        (NULL == pp_hdl_dmx_filter)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    filter = malloc(sizeof(*filter));
    if (NULL == filter) {
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }
    memset(filter, 0, sizeof(*filter));
    memcpy(&(filter->attr_filter), p_attr_filter, sizeof(*p_attr_filter));

    filter->id = DMX_ILLEGAL_FILTERID;
    filter->dmx_channel = channel;

    memset(&cb, 0, sizeof(cb));
    memset(&filter_token, 0, sizeof(filter_token));
    cb.request_buffer = (alisldmx_channel_requestbuf_callback)requestbuf_callback;
    cb.update_buffer = (alisldmx_channel_updatebuf_callback)updatebuf_callback;
    filter_token.filter = filter;
    filter_token.id = require_cb_token_id();
    if (filter_token.id < 0) {
        free(filter);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    memcpy(&filter_token_array[filter_token.id], &filter_token, sizeof(cb_filter_token_t));
    cb.priv = &filter_token_array[filter_token.id];
    //cb.priv = filter;

    if (AUI_DMX_DATA_SECT != channel->attr_channel.dmx_data_type) {
         if (filter->attr_filter.p_fun_data_req_wtCB && filter->attr_filter.p_fun_data_up_wtCB) {
            alisldmx_register_channel_callback(dev->handle, channel->id, &cb);
         }
    }
    /** add filter ringbuf start -->*/
    if((filter->attr_filter.dmx_fil_sec_data_sync_get_support)&&
        (AUI_DMX_DATA_SECT == channel->attr_channel.dmx_data_type))
    {
        if(AUI_RTN_SUCCESS!=aui_common_init_ring_buf(AUI_DMX_FILTER_BUF_LEN,&(filter->fil_rbuf)))
        {
            AUI_ERR("aui_common_init_ring_buf fail!\n");
            free(filter);
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
        AUI_DBG("init ring buf done, get: %p\n", filter->fil_rbuf.pby_ring_buf);
    }
    /** <-- add filter ringbuf close */

    switch (channel->attr_channel.dmx_data_type) {
        case AUI_DMX_DATA_RAW:
        case AUI_DMX_DATA_PES:
        case AUI_DMX_DATA_ES:
        case AUI_DMX_DATA_REC:
            pthread_mutex_lock(&token_mutex);
            aui_list_add_tail(&filter->node, &channel->filters);
            pthread_mutex_unlock(&token_mutex);
            *pp_hdl_dmx_filter = filter;
            filter->cb_filter_token_id = filter_token.id;
            set_cb_token_status(filter_token.id, TOKEN_USED);
            return AUI_RTN_SUCCESS;
        case AUI_DMX_DATA_SECT:
            break;
        default:
            if (filter->fil_rbuf.pby_ring_buf) {
                aui_common_un_init_ring_buf(&(filter->fil_rbuf));
            }
            free(filter);
            aui_rtn(AUI_RTN_FAIL, NULL);;
    }

    if (alisldmx_allocate_filter(dev->handle, channel->id, &filter->id)) {
        if (filter->fil_rbuf.pby_ring_buf) {
            aui_common_un_init_ring_buf(&(filter->fil_rbuf));
        }
        free(filter);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }

    /** register callback for each filter of the channel. */
    if (filter->attr_filter.p_fun_sectionCB) {
        alisldmx_register_filter_callback(dev->handle, filter->id, &cb);
    }

    filter->puc_val = NULL;
    filter->puc_mask = NULL;
    filter->puc_reverse = NULL;

    filter->ul_mask_val_len = 0;
    filter->uc_crc_check = p_attr_filter->uc_crc_check;
    alisldmx_set_filter_crc_check(dev->handle, filter->id, filter->uc_crc_check);
    filter->uc_continue_capture_flag = p_attr_filter->uc_continue_capture_flag;

    if ((NULL != p_attr_filter->puc_val) &&
        (NULL != p_attr_filter->puc_mask) &&
        (NULL != p_attr_filter->puc_reverse) &&
        (0 != p_attr_filter->ul_mask_val_len)) {
        ul_mask_val_len = p_attr_filter->ul_mask_val_len;
        filter->puc_val = malloc(AUI_DMX_FILTER_SIZE);
        if (NULL == filter->puc_val) {
            alisldmx_free_filter(dev->handle, filter->id);
            if (filter->fil_rbuf.pby_ring_buf) {
                aui_common_un_init_ring_buf(&(filter->fil_rbuf));
            }
            free(filter);
            aui_rtn(AUI_RTN_ENOMEM, NULL);
        }
        filter->puc_mask = malloc(AUI_DMX_FILTER_SIZE);
        if (NULL == filter->puc_mask) {
            free(filter->puc_val);
            alisldmx_free_filter(dev->handle, filter->id);
            if (filter->fil_rbuf.pby_ring_buf) {
                aui_common_un_init_ring_buf(&(filter->fil_rbuf));
            }
            free(filter);
            aui_rtn(AUI_RTN_ENOMEM, NULL);
        }
        filter->puc_reverse = malloc(AUI_DMX_FILTER_SIZE);
        if (NULL == filter->puc_reverse) {
            free(filter->puc_val);
            free(filter->puc_mask);
            alisldmx_free_filter(dev->handle, filter->id);
            if (filter->fil_rbuf.pby_ring_buf) {
                aui_common_un_init_ring_buf(&(filter->fil_rbuf));
            }
            free(filter);
            aui_rtn(AUI_RTN_ENOMEM, NULL);
        }
        memset(filter->puc_val, 0, AUI_DMX_FILTER_SIZE);
        memset(filter->puc_mask, 0, AUI_DMX_FILTER_SIZE);
        memset(filter->puc_reverse, 0, AUI_DMX_FILTER_SIZE);
        memcpy(filter->puc_val, p_attr_filter->puc_val, ul_mask_val_len);
        memcpy(filter->puc_mask, p_attr_filter->puc_mask, ul_mask_val_len);
        memcpy(filter->puc_reverse, p_attr_filter->puc_reverse, ul_mask_val_len);
        filter->ul_mask_val_len = p_attr_filter->ul_mask_val_len;
        for (i = 0; i < ul_mask_val_len; i++) {
            filter->puc_reverse[i] = ~(filter->puc_reverse[i]);
        }
    }

    if (alisldmx_set_filter(dev->handle,
                            filter->id, filter->ul_mask_val_len,
                            filter->puc_val, filter->puc_mask,
                            filter->puc_reverse,
                            filter->uc_continue_capture_flag)) {
        if (filter->puc_val) {
            free(filter->puc_val);
        }
        if (filter->puc_mask) {
            free(filter->puc_mask);
        }
        if (filter->puc_reverse) {
            free(filter->puc_reverse);
        }
        alisldmx_free_filter(dev->handle, filter->id);
        if (filter->fil_rbuf.pby_ring_buf) {
            aui_common_un_init_ring_buf(&(filter->fil_rbuf));
        }
        free(filter);
        aui_rtn(AUI_RTN_FAIL, NULL);
    }
    pthread_mutex_lock(&token_mutex);
    aui_list_add_tail(&filter->node, &channel->filters);
    pthread_mutex_unlock(&token_mutex);
    *pp_hdl_dmx_filter = filter;
    filter->cb_filter_token_id = filter_token.id;
    set_cb_token_status(filter_token.id, TOKEN_USED);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_close(aui_hdl *pp_hdl_dmx_filter)
{
    struct dmx_filter *filter = NULL;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;

    if (NULL == pp_hdl_dmx_filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    filter = (struct dmx_filter *)(*pp_hdl_dmx_filter);

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    /** free filter  ringbuf start -->*/
    if (filter->fil_rbuf.pby_ring_buf) {
        AUI_DBG("free rbuf: %p\n", filter->fil_rbuf.pby_ring_buf);
        aui_common_un_init_ring_buf(&(filter->fil_rbuf));
    }
    /** <-- free filter  ringbuf stop */

    if (DMX_ILLEGAL_FILTERID != filter->id) {
        alisldmx_free_filter(dev->handle, filter->id);
    }

    if (NULL != filter->puc_mask) {
        free(filter->puc_mask);
        filter->puc_mask = NULL;
    }
    if (NULL != filter->puc_val) {
        free(filter->puc_val);
        filter->puc_val = NULL;
    }
    if (NULL != filter->puc_reverse) {
        free(filter->puc_reverse);
        filter->puc_reverse = NULL;
    }

    if (NULL != filter->buffer) {
        free(filter->buffer);
        filter->buffer = NULL;
    }
    pthread_mutex_lock(&token_mutex);
    aui_list_del(&filter->node);
    pthread_mutex_unlock(&token_mutex);
    set_cb_token_status(filter->cb_filter_token_id, TOKEN_IDLE);
    free(filter);
    *pp_hdl_dmx_filter = NULL;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_channel_read(aui_hdl p_hdl_dmx_channel, unsigned char *p_uc_buffer,
                                    unsigned long n_number_to_read, unsigned long *p_n_number_read,
                                    int n_timeout)
{
    struct dmx_channel *channel = p_hdl_dmx_channel;
    struct dmx_device *dev = NULL;

    *p_n_number_read = 0;

    if (NULL == p_hdl_dmx_channel || NULL == p_n_number_read) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (channel->id == DMX_ILLEGAL_CHANNELID) {
        aui_rtn(AUI_RTN_EINVAL, "illegal channel id");
    }

    if (channel->attr_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) {
        aui_rtn(AUI_RTN_EINVAL, "dmx channel not yet started");
    }

    dev = channel->dmx_device;
    if (alisldmx_channel_read(dev->handle, channel->id, p_uc_buffer,
                                n_number_to_read, p_n_number_read, n_timeout)) {
        aui_rtn(AUI_RTN_FAIL, "alisldmx_read failed");
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_start(aui_hdl hdl_dmx_filter,
                                  const aui_attr_dmx_filter *p_attr_filter)
{
    struct dmx_filter *filter = (struct dmx_filter *)hdl_dmx_filter;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (AUI_DMX_DATA_SECT == channel->attr_channel.dmx_data_type) {
        alisldmx_control_filter(dev->handle, filter->id, DMX_CTRL_ENABLE);
    } else {
        alisldmx_control_channel(dev->handle, channel->id, DMX_CTRL_ENABLE);
    }
    filter->attr_filter.dmx_filter_status = AUI_DMX_FILTER_RUN;

    /** unused */
    (void)p_attr_filter;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_stop(aui_hdl hdl_dmx_filter,
                                 const aui_attr_dmx_filter *p_attr_filter)
{
    struct dmx_filter *filter = (struct dmx_filter *)hdl_dmx_filter;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (AUI_DMX_DATA_SECT == channel->attr_channel.dmx_data_type) {
        alisldmx_control_filter(dev->handle, filter->id, DMX_CTRL_DISABLE);
    } else {
        alisldmx_control_channel(dev->handle, channel->id, DMX_CTRL_DISABLE);
    }
    filter->attr_filter.dmx_filter_status=AUI_DMX_FILTER_STOP;

    /** unused */
    (void)p_attr_filter;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_filter_set(aui_hdl p_hdl_dmx_filter,
                                unsigned long ul_item,
                                void *pv_param)
{
    struct dmx_filter *filter = p_hdl_dmx_filter;
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;

    if ((NULL == filter) ||
        (ul_item >= AUI_DMX_SET_CMD_LAST)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DMX_FILTER_MASKVAL_SET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_CONFIG: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_filter_get(aui_hdl p_hdl_dmx_filter, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_FAIL;
    struct dmx_filter *filter = p_hdl_dmx_filter;

    if ((NULL == filter) ||
        (ul_item > AUI_DMX_GET_CMD_LAST)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DMX_FILTER_TYPE_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DMX_FILTER_ATTR_GET: {
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            *(unsigned long *)pv_param = (unsigned long) & (filter->attr_filter);
            rtn_code = AUI_RTN_SUCCESS;
            break;
        }
        default: {
            aui_rtn(AUI_RTN_FAIL, NULL);
            break;
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_dmx_filter_mask_val_cfg(aui_hdl p_hdl_dmx_filter,
                                         const unsigned char *puc_mask,
                                         const unsigned char *puc_val,
                                         const unsigned char *puc_reverse,
                                         unsigned long ul_mask_val_len,
                                         unsigned char uc_crc_check,
                                         unsigned char uc_continue_capture_flag)
{
    struct dmx_filter *filter = (struct dmx_filter *)p_hdl_dmx_filter;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;
    unsigned long i = 0;

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (ul_mask_val_len >= AUI_DMX_FILTER_SIZE) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if ((NULL == puc_mask) ||
        (NULL == puc_val)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    filter->ul_mask_val_len = ul_mask_val_len;

    if (NULL == filter->puc_val) {
        filter->puc_val = malloc(ul_mask_val_len);
    }
    if (NULL == filter->puc_val) {
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }

    if (NULL == filter->puc_mask) {
        filter->puc_mask = malloc(ul_mask_val_len);
    }
    if (NULL == filter->puc_mask) {
        free(filter->puc_val);
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }

    if (NULL == filter->puc_reverse) {
        filter->puc_reverse = malloc(ul_mask_val_len);
    }
    if (NULL == filter->puc_reverse) {
        free(filter->puc_val);
        free(filter->puc_mask);
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }

    memcpy(filter->puc_val, puc_val, ul_mask_val_len);
    memcpy(filter->puc_mask, puc_mask, ul_mask_val_len);
    memcpy(filter->puc_reverse, puc_reverse, ul_mask_val_len);
    filter->uc_crc_check = uc_crc_check;
    alisldmx_set_filter_crc_check(dev->handle, filter->id, filter->uc_crc_check);
    filter->uc_continue_capture_flag = uc_continue_capture_flag;
    for (i = 0; i < ul_mask_val_len; i++) {
        filter->puc_reverse[i] = ~(filter->puc_reverse[i]);
    }

    if (DMX_ILLEGAL_FILTERID != filter->id) {
        if (alisldmx_set_filter(dev->handle,
                                filter->id,
                                filter->ul_mask_val_len,
                                filter->puc_val,
                                filter->puc_mask,
                                filter->puc_reverse,
                                filter->uc_continue_capture_flag)) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_reg_sect_call_back(aui_hdl hdl_dmx_filter,
                                        aui_p_fun_sectionCB p_fun_sectionCB)
{
    struct dmx_filter *filter = (struct dmx_filter *)hdl_dmx_filter;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;
    struct dmx_channel_callback cb;
    cb_filter_token_t filter_token;

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (AUI_DMX_DATA_SECT != channel->attr_channel.dmx_data_type) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }

    filter->attr_filter.p_fun_sectionCB = p_fun_sectionCB;

    memset(&cb, 0, sizeof(cb));
    cb.request_buffer = (alisldmx_channel_requestbuf_callback) requestbuf_callback;
    cb.update_buffer = (alisldmx_channel_updatebuf_callback) updatebuf_callback;
    memset(&filter_token, 0, sizeof(filter_token));
    filter_token.filter = filter;
    if (filter->cb_filter_token_id > 0) {
        filter_token.id = filter->cb_filter_token_id;
    } else {
        filter_token.id = require_cb_token_id();
        if (filter_token.id < 0) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
    memcpy(&filter_token_array[filter_token.id], &filter_token, sizeof(cb_filter_token_t));
    cb.priv = &filter_token_array[filter_token.id];
    //cb.priv = filter;
    alisldmx_register_filter_callback(dev->handle, filter->id, &cb);
    filter->cb_filter_token_id = filter_token.id;
    set_cb_token_status(filter_token.id, TOKEN_USED);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_reg_data_call_back(aui_hdl hdl_dmx_filter,
                                        aui_p_fun_data_req_wtCB p_fun_data_req_wtCB,
                                        aui_p_fun_data_up_wtCB p_fun_data_up_wtCB)
{
    struct dmx_filter *filter = (struct dmx_filter *)hdl_dmx_filter;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;
    struct dmx_channel_callback cb;
    cb_filter_token_t filter_token;

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    filter->attr_filter.p_fun_data_req_wtCB = p_fun_data_req_wtCB;
    filter->attr_filter.p_fun_data_up_wtCB = p_fun_data_up_wtCB;

    memset(&cb, 0, sizeof(cb));
    cb.request_buffer = (alisldmx_channel_requestbuf_callback)requestbuf_callback;
    cb.update_buffer = (alisldmx_channel_updatebuf_callback) updatebuf_callback;
    memset(&filter_token, 0, sizeof(filter_token));
    filter_token.filter = filter;
    if (filter->cb_filter_token_id > 0) {
        filter_token.id = filter->cb_filter_token_id;
    } else {
        filter_token.id = require_cb_token_id();
        if (filter_token.id < 0) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
    memcpy(&filter_token_array[filter_token.id], &filter_token, sizeof(cb_filter_token_t));
    cb.priv = &filter_token_array[filter_token.id];
    //cb.priv = filter;
    alisldmx_register_channel_callback(dev->handle, channel->id, &cb);
    filter->cb_filter_token_id = filter_token.id;
    set_cb_token_status(filter_token.id, TOKEN_USED);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_reg_pes_call_back(aui_hdl hdl_dmx_filter,
                                       aui_pes_data_callback callback,
                                       void *callback_param)
{
    struct dmx_filter *filter = (struct dmx_filter *)hdl_dmx_filter;
    struct dmx_channel *channel = NULL;
    struct dmx_device *dev = NULL;
    struct dmx_channel_callback cb;
    cb_filter_token_t filter_token;

    if (NULL == filter) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    channel = filter->dmx_channel;
    if (NULL == channel) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = channel->dmx_device;
    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    filter->attr_filter.p_fun_data_req_wtCB = NULL;
    filter->attr_filter.p_fun_data_up_wtCB = NULL;

    filter->attr_filter.callback = callback;
    filter->attr_filter.callback_param = callback_param;

    memset(&cb, 0, sizeof(cb));
    cb.request_buffer = (alisldmx_channel_requestbuf_callback)requestbuf_callback;
    cb.update_buffer = (alisldmx_channel_updatebuf_callback) updatebuf_callback;
    memset(&filter_token, 0, sizeof(filter_token));
    filter_token.filter = filter;
    if (filter->cb_filter_token_id > 0) {
        filter_token.id = filter->cb_filter_token_id;
    } else {
        filter_token.id = require_cb_token_id();
        if (filter_token.id < 0) {
            aui_rtn(AUI_RTN_FAIL, NULL);
        }
    }
    memcpy(&filter_token_array[filter_token.id], &filter_token, sizeof(cb_filter_token_t));
    cb.priv = &filter_token_array[filter_token.id];
    //cb.priv = filter;
    alisldmx_register_channel_callback(dev->handle, channel->id, &cb);
    filter->cb_filter_token_id = filter_token.id;
    set_cb_token_status(filter_token.id, TOKEN_USED);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_cache_set_by_index(unsigned int dmx_index,
                                        aui_dmx_cache_param_t* param)
{
    if (alisldmx_cache_set((alisl_handle)dmx_index, (dmx_cache_param_t *)param)) {
        AUI_ERR("failed!\n");
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_hw_buffer_clean(unsigned int dmx_index)
{
    if (alisldmx_hw_buffer_clean(dmx_index)) {
        AUI_ERR("failed!\n");
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dmx_set_vmx_de_play(aui_hdl p_hdl_dmx, struct dec_parse_param *param)
{
    struct dmx_device *dev = p_hdl_dmx;

    if ((NULL == dev) ||
        (NULL == param)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (alisldmx_ioctl(dev->handle,
                        DMX_IOCMD_SET_DEC_HANDLE,
                        (unsigned long)param)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }

    if (alisldmx_ioctl(dev->handle,
                       DMX_IOCMD_CRYPTO_START,
                       0)) {
        aui_rtn(AUI_RTN_FAIL, NULL);
    }

    dev->crypto_started = 1;
    return AUI_RTN_SUCCESS;
}

int aui_dmx_ioctl_priv(aui_hdl p_hdl_dmx, unsigned long ul_item, void *param)
{
    if (!p_hdl_dmx) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    struct dmx_device *dev = p_hdl_dmx;
    int ret = AUI_RTN_FAIL;

    switch (ul_item) {
        case AUI_DMX_M2S_BUF_VALIDSIZE_SET:
            ret = alisldmx_ioctl(dev->handle,
                                 DMX_IOCMD_MAIN2SEE_BUF_VALIDSIZE_SET,
                                 (unsigned long)param);
            break;
        case AUI_DMX_M2S_SRC_SET:
            ret = alisldmx_ioctl(dev->handle,
                                 DMX_IOCMD_MAIN2SEE_SRC_SET,
                                 (unsigned long)param);
            break;
        case AUI_DMX_M2S_BUF_REQ_SIZE:
            if (param) {
                ret = alisldmx_ioctl(dev->handle,
                                     DMX_IOCMD_MAIN2SEE_BUF_REQ_SIZE,
                                     (unsigned long)param);
            } else {
                AUI_ERR("item %lu fail for null pointer\n", ul_item);
            }
            break;
        case AUI_DMX_M2S_BUF_RET_SIZE:
            ret = alisldmx_ioctl(dev->handle,
                                 DMX_IOCMD_MAIN2SEE_BUF_RET_SIZE,
                                 (unsigned long)param);
            break;
        case AUI_DMX_REC_MODE_GET:
            if (param) {
                ret = alisldmx_ioctl(dev->handle,
                                     DMX_IOCMD_GET_RECORD_MODE,
                                     (unsigned long)param);
            } else {
                AUI_ERR("item %lu fail for null pointer\n", ul_item);
            }
            break;
        case AUI_DMX_REC_BLOCK_SIZE_GET:
            if (param) {
                ret = alisldmx_ioctl(dev->handle,
                                     DMX_IOCMD_GET_RECORD_BLOCKSIZE,
                                     (unsigned long)param);
            } else {
                AUI_ERR(" item %lu fail for null pointer\n", ul_item);
            }
            break;
        case AUI_DMX_M2S_BUF_VALIDSIZE_GET:
            if (param) {
                ret = alisldmx_ioctl(dev->handle,
                                     DMX_IOCMD_MAIN2SEE_BUF_VALIDSIZE_GET,
                                     (unsigned long)param);
            } else {
                AUI_ERR("item %lu fail for null pointer\n", ul_item);
            }
            break;
        case AUI_DMX_M2S_SRC_GET:
            if (param) {
                ret = alisldmx_ioctl(dev->handle,
                                     DMX_IOCMD_MAIN2SEE_SRC_GET,
                                     (unsigned long)param);
            } else {
                AUI_ERR("item %lu fail for null pointer\n", ul_item);
            }
            break;
        default:
            break;
    }
    if (ret) {
        AUI_ERR("err %lu\n", ul_item);
    }
    return ret;
}

static unsigned int get_tick_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

/** for section sync get special proc,one time on section
    to get current section length. */
static int aui_common_get_rbuf_rdlen(aui_ring_buf *p_ring_buf)
{
    unsigned long ul_data_len = 0;
    int sec_len = 0;
    unsigned char syn_indi = 0;
    unsigned char sec_len_low_byte = 0;
    if (NULL == p_ring_buf->pby_ring_buf) {
        AUI_ERR("ring buf is null!\n");
        return 0;
    }
    AUI_DBG("ring_buf: %p\n", p_ring_buf->pby_ring_buf);
    if (AUI_RTN_SUCCESS != aui_common_ring_buf_data_len(p_ring_buf, &ul_data_len)) {
        AUI_ERR("ring buf data len is 0!\n");
        return 0;
    }
    if (ul_data_len > 0) {
        //AUI_TAKE_SEM(p_ring_buf->sem_ring);
        aui_common_ring_buf_lock(p_ring_buf);
        if ((p_ring_buf->ul_ring_buf_rd + 2) == p_ring_buf->ul_ring_buf_len) {
            syn_indi = *(unsigned char *)((unsigned char *)(p_ring_buf->pby_ring_buf) +
                                           p_ring_buf->ul_ring_buf_rd + 1);
            sec_len_low_byte = *(unsigned char *)(p_ring_buf->pby_ring_buf);
        } else if ((p_ring_buf->ul_ring_buf_rd + 1) == p_ring_buf->ul_ring_buf_len) {
            syn_indi = *(unsigned char *)((p_ring_buf->pby_ring_buf));
            sec_len_low_byte = *(unsigned char *)((p_ring_buf->pby_ring_buf) + 1);
        } else {
            syn_indi = *(unsigned char *)((p_ring_buf->pby_ring_buf) +
                                           p_ring_buf->ul_ring_buf_rd + 1);
            sec_len_low_byte = *(unsigned char *)((p_ring_buf->pby_ring_buf) +
                                                   p_ring_buf->ul_ring_buf_rd + 2);
        }
        sec_len = (((unsigned short)((syn_indi&0x0f)<<8)) | (sec_len_low_byte)) + 3;
        AUI_DBG("sec_len: %d\n", sec_len);
        //AUI_GIVE_SEM(p_ring_buf->sem_ring);
        aui_common_ring_buf_unlock(p_ring_buf);
        return sec_len;
    } else {
        AUI_DBG("ring buf data len(ul_data_len) is 0!\n");
        return 0;
    }
}

AUI_RTN_CODE aui_dmx_channel_sync_get_section_data(aui_hdl p_hdl_dmx_channel,
                                                   aui_hdl pst_hdl_dmx_filter,
                                                   const unsigned int req_size,
                                                   unsigned char *const p_buf,
                                                   unsigned int *const p_data_size,
                                                   const unsigned int timeout_ms)
{
    struct dmx_channel *p_hdl_dmx_chl = p_hdl_dmx_channel;
    struct dmx_filter *p_hdl_dmx_filter = pst_hdl_dmx_filter;
    unsigned int  tick = 0;
    unsigned char *dst = p_buf;
    int left = req_size;
    unsigned long read_len = 0;

    if ((p_buf == NULL) ||
        (p_data_size == NULL) ||
        (left == 0)) {
        if (p_data_size) {
            *p_data_size = 0;
        }
        return AUI_RTN_FAIL;
    }
    *p_data_size = 0;
    tick = get_tick_ms();

    if (p_hdl_dmx_chl) {
        while (left > 0) {
GET_CHANNEL_BUF_SECTION_LEN:
            if (p_hdl_dmx_chl->attr_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) {
                /** channel not ready */
                AUI_ERR("the channel not ready! status: %d\n",
                p_hdl_dmx_chl->attr_channel.dmx_channel_status);
                return AUI_RTN_FAIL;
            }
            if ((p_hdl_dmx_chl->attr_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_chl->chan_rbuf.pby_ring_buf == NULL)) {
                //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                AUI_ERR("the dmx_data_type is not AUI_DMX_DATA_SECT or the ring_buf is NULL!\n");
                return AUI_RTN_FAIL;
            }
            left = aui_common_get_rbuf_rdlen(&(p_hdl_dmx_chl->chan_rbuf));
            if (left == 0) {
                //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                if (((get_tick_ms() - tick ) >= timeout_ms) || (get_tick_ms() < tick)) {
                    AUI_ERR("time out!\n");
                    break;
                }
                AUI_SLEEP(5); //osal_task_sleep(1);
                //AUI_DBG("read len is 0,retry!\n");
                goto GET_CHANNEL_BUF_SECTION_LEN;
            }
            if ((unsigned int)left > req_size) {
                //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                //AUI_DBG("left > req_size!\n");
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_rd(&(p_hdl_dmx_chl->chan_rbuf), left, &read_len, dst);
            //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
            left = 0;
        }
        *p_data_size = read_len;
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            //AUI_DBG("");
            return AUI_RTN_FAIL;
        }
    } else if (p_hdl_dmx_filter){
        while (left > 0) {
GET_FLT_BUF_SECTION_LEN:
            if (p_hdl_dmx_filter->attr_filter.dmx_filter_status != AUI_DMX_FILTER_RUN) {
                /** filter not ready */
                AUI_ERR("filter not ready! status: %d\n",p_hdl_dmx_filter->attr_filter.dmx_filter_status);
                return AUI_RTN_FAIL;
            }
            p_hdl_dmx_chl = p_hdl_dmx_filter->dmx_channel;
            if ((p_hdl_dmx_chl == NULL) ||
                (p_hdl_dmx_chl->attr_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) ||
                (p_hdl_dmx_chl->attr_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_filter->fil_rbuf.pby_ring_buf == NULL)) {
                //if (p_hdl_dmx_filter->dev_mutex_id) {
                //    osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                //}
                AUI_ERR("something error!\n");
                return AUI_RTN_FAIL;
            }
            left = aui_common_get_rbuf_rdlen(&(p_hdl_dmx_filter->fil_rbuf));
            if (left == 0) {
                //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                if (((get_tick_ms() - tick ) >= timeout_ms) || (get_tick_ms() < tick)) {
                    AUI_ERR("time out! %dms\n", timeout_ms);
                    break;
                }
                AUI_SLEEP(5);//osal_task_sleep(1);
                //AUI_DBG("section len is 0,retry!\n");
                goto GET_FLT_BUF_SECTION_LEN;
            }
            /** req buf not enough. */
            if ((unsigned int)left > req_size) {
                //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                //AUI_DBG("req buf not enough.! req: %d, left: %d.\n", req_size, left);
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_rd(&(p_hdl_dmx_filter->fil_rbuf),left,&read_len,dst);
            //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
            left = 0;
        }
        *p_data_size = read_len;
        //AUI_DBG("read_len: %ld\n", read_len);
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            AUI_ERR("read_len: %ld!\n", read_len);
            return AUI_RTN_FAIL;
        }
    } else {
        AUI_ERR("filter and channel are NULL!\n");
        return AUI_RTN_FAIL;
    }
}

AUI_RTN_CODE aui_dmx_channel_sync_get_section_data_ext(aui_hdl p_hdl_dmx_channel,
                                                       aui_hdl pst_hdl_dmx_filter,
                                                       const unsigned int req_size,
                                                       unsigned char *const p_buf,
                                                       unsigned int *const p_data_size,
                                                       const unsigned int timeout_ms)
{
    struct dmx_channel *p_hdl_dmx_chl = p_hdl_dmx_channel;
    struct dmx_filter *p_hdl_dmx_filter = pst_hdl_dmx_filter;
    unsigned int tick = 0;
    unsigned char *dst = p_buf;
    unsigned long left = req_size;
    unsigned long read_len = 0;

    if ((p_buf == NULL) ||
        (p_data_size == NULL) ||
        (left == 0)) {
        if (p_data_size) {
            *p_data_size = 0;
        }
        return AUI_RTN_FAIL;
    }
    *p_data_size = 0;
    tick = get_tick_ms();
    if (p_hdl_dmx_chl) {
        while (left > 0) {
GET_CHANNEL_BUF_SECTION_LEN:
            if (p_hdl_dmx_chl->attr_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) {
                /** channel not ready */
                AUI_ERR("the channel not ready! status: %d\n", p_hdl_dmx_chl->attr_channel.dmx_channel_status);
                return AUI_RTN_FAIL;
            }
            if ((p_hdl_dmx_chl->attr_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_chl->chan_rbuf.pby_ring_buf == NULL)) {
                //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                AUI_ERR("the dmx_data_type is not AUI_DMX_DATA_SECT or the ring_buf is NULL!\n");
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_data_len(&(p_hdl_dmx_chl->chan_rbuf),&left);

            if (left == 0) {
                //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                if (((get_tick_ms() - tick ) >= timeout_ms) || (get_tick_ms() < tick)) {
                    AUI_ERR("time out!\n");
                    break;
                }
                AUI_SLEEP(1); //osal_task_sleep(1);
                //AUI_DBG("read len is 0,retry!\n");
                goto GET_CHANNEL_BUF_SECTION_LEN;
            }
            if ((unsigned int)left > req_size) {
                //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
                //AUI_DBG("left > req_size!\n");
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_rd(&(p_hdl_dmx_chl->chan_rbuf), left, &read_len, dst);
            //osal_mutex_unlock(p_hdl_dmx_chl->dev_mutex_id);
            left = 0;
        }
        *p_data_size = read_len;
        //AUI_DBG("read_len: %ld\n", read_len);
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            return AUI_RTN_FAIL;
        }
    } else if (p_hdl_dmx_filter) {
        while (left > 0) {
GET_FLT_BUF_SECTION_LEN:
            if (p_hdl_dmx_filter->attr_filter.dmx_filter_status != AUI_DMX_FILTER_RUN) {
                /** filter already deleted */
                return AUI_RTN_FAIL;
            }
            p_hdl_dmx_chl = p_hdl_dmx_filter->dmx_channel;
            if ((p_hdl_dmx_chl == NULL) ||
                (p_hdl_dmx_chl->attr_channel.dmx_channel_status != AUI_DMX_CHANNEL_RUN) ||
                (p_hdl_dmx_chl->attr_channel.dmx_data_type != AUI_DMX_DATA_SECT) ||
                (p_hdl_dmx_filter->fil_rbuf.pby_ring_buf == NULL)) {
                //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_data_len(&(p_hdl_dmx_filter->fil_rbuf), &left);
            if (left == 0) {
                //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                if (((get_tick_ms() - tick ) >= timeout_ms) || (get_tick_ms() < tick)) {
                    //AUI_DBG("time out!\n");
                    break;
                }
                AUI_SLEEP(1);//osal_task_sleep(1);
                //AUI_DBG("read len is 0,retry!\n");
                goto GET_FLT_BUF_SECTION_LEN;
            }
            if ((unsigned int)left > req_size) {
                //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
                //AUI_DBG("left > req_size!\n");
                return AUI_RTN_FAIL;
            }
            aui_common_ring_buf_rd(&(p_hdl_dmx_filter->fil_rbuf), left, &read_len, dst);
            //osal_mutex_unlock(p_hdl_dmx_filter->dev_mutex_id);
            left = 0;
        }
        *p_data_size = read_len;
        //AUI_DBG("read_len: %ld\n", read_len);
        if ((read_len > 0) && (read_len <= req_size)) {
            return AUI_RTN_SUCCESS;
        } else {
            return AUI_RTN_FAIL;
        }
    } else {
        AUI_ERR("filter and channel are NULL!\n");
        return AUI_RTN_FAIL;
    }
}

