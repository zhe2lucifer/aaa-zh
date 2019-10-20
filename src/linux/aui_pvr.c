/**  @file
*    @brief     AUI PVR module interface implement
*    @author    summic.huang
*    @date      2013-8-21
*    @version   1.0.0
*    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
*/

/****************************INCLUDE HEAD FILE************************************/

#include "aui_common_priv.h"

#include <basic_types.h>
/* remove compile warning */
#ifdef SUPPORT_MPEG4_TEST
#undef SUPPORT_MPEG4_TEST
#endif
#include <osal/osal.h>
#include <bus/otp/otp.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/crypto/crypto.h>
#include <hld/trng/trng.h>
#include <hld/decv/decv.h>
#include <hld/deca/deca.h>
#include <hld/dmx/dmx.h>
#include <hld/snd/snd.h>
#include <hld/dis/vpo.h>
#include <hld/avsync/avsync.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <api/libpvr/lib_pvr.h>
#include <alisltrng.h>
#include <aui_deca.h>
#include <aui_pvr.h>
#include <aui_ca_pvr.h>
#include "cas9_pvr.h"
#include "c200a_pvr.h"
#include "gen_ca_pvr.h"


AUI_MODULE(PVR)

/****************************LOCAL MACRO******************************************/

#define SET_VALUE(x,y,default) x=y==0?default:y

#define aui_pvr_return_if_fail(expr)        do{     \
        if (expr) \
        {\
        }\
        else                        \
        {                           \
            AUI_ERR(" assertion `%s' failed\n",    \
                        #expr);                     \
            return AUI_RTN_FAIL;                     \
        };      \
    }while(0)

/****************************LOCAL TYPE*******************************************/

typedef struct aui_pvr_handle {
    PVR_HANDLE pvr_handler; /* PVR handler */
    void *user_data;    //user data
    fn_aui_pvr_callback fn_callback; /* pvr callback */
} aui_pvr_handle, *p_aui_pvr_handle;

typedef struct aui_pvr_event_msg_t {
    PVR_HANDLE pvr_handle;
    unsigned int  msg_type;
    unsigned int  msg_code;
} aui_pvr_event_msg, *p_aui_pvr_event_msg;


/****************************LOCAL VAR********************************************/

OSAL_ID event_callback_id;
OSAL_ID event_task_id;

static unsigned char s_pvr_mod_inited = FALSE;
static BOOL init_once = TRUE;
static aui_pvr_handle *p_st_pvr_handle = NULL;
static UINT8 s_max_pvr_record_number = 0;
//static OSAL_ID s_pvr_mutexID = INVALID_ID;
static pthread_mutex_t s_pvr_mutexID = PTHREAD_MUTEX_INITIALIZER;
static aui_ply_param g_ply_param;


/****************************LOCAL FUNC DECLEAR***********************************/

void aui_pvr_timer_callback(struct PVR_DTM *tm);
INT8 aui_pvr_evnt_callback(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code);


/****************************EXTERN FUNC DECLEAR**********************************/

extern void av_get_audio_pid(enum aui_deca_stream_type type, unsigned short pid, unsigned short *new_pid);

extern RET_CODE dmx_m36_attach(struct dmx_feature_config *config);
extern void HLD_vdec_attach(void);

extern RET_CODE dmx_m36_dettach();
extern void HLD_vdec_detach(void);
extern void deca_m36_dettach(struct deca_device *dev);
extern void snd_m36_dettach(struct snd_device *dev);

extern BOOL pvr_p_close_ext(PVR_HANDLE *handle, struct playback_stop_param *stop_param);
extern PVR_STATE _recorder_get_state(PVR_HANDLE handle);
extern UINT8 _recorder_get_dmx_id(PVR_HANDLE handle);
extern UINT32 _recorder_get_time(PVR_HANDLE handle);

extern RET_CODE calculate_hmac(unsigned char *input, unsigned long length, unsigned char *output, unsigned char *key);

extern RET_CODE pvr_set_uri(PVR_HANDLE handle, conax6_uri_item *URI_item);
extern RET_CODE pvr_get_uri(PVR_HANDLE handle, conax6_uri_item *URI_item);
extern RET_CODE pvr_get_rl_idx_by_path(char *path, UINT16 *idx);
extern RET_CODE pvr_get_uri_cnt(UINT32 index, UINT32 *wcnt);
extern RET_CODE pvr_get_uri_sets(UINT32 index, UINT32 base, UINT32 cnt, conax6_uri_item *URI_sets);
extern RET_CODE pvr_set_finger_info(PVR_HANDLE handle, pvr_finger_info *finger_info);
extern RET_CODE pvr_get_finger_info(PVR_HANDLE handle, pvr_finger_info *finger_info);

extern RET_CODE pvr_r_set_tms_max_time(UINT32 seconds, UINT32 rec_special_mode);
extern BOOL pvr_r_add_pid(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids);
BOOL pvr_r_change_pid(PVR_HANDLE handle, UINT16 pid_nums, UINT16 *pids);
extern RET_CODE     pvr_set_mat_rating(PVR_HANDLE handle, UINT32 ptm, UINT32 rating);
extern RET_CODE     pvr_get_mat_rating(PVR_HANDLE handle, UINT32 ptm, UINT32 *rating);
extern RET_CODE     pvr_get_mat_rating_by_idx(UINT16 record_idx, UINT32 ptm, UINT32 *rating);
extern UINT32 pvr_set_free_size_low_threshold(UINT32 low);
extern UINT32 pvr_get_free_size_low_threshold();
extern void pvr_set_block_mode(UINT8 enable);
extern UINT32 pvr_set_block_size(UINT32 size);
extern UINT32 pvr_get_block_size(void);
extern RET_CODE pvr_set_nv_uri(PVR_HANDLE handle, nasc_uri_item *uri_item);
extern RET_CODE pvr_get_nv_uri(PVR_HANDLE handle, nasc_uri_item *uri_item);
extern RET_CODE pvr_get_nv_uri_sets(UINT32 index, UINT32 base, UINT32 cnt, nasc_uri_item *uri_sets);
extern RET_CODE pvr_get_nv_uri_count(UINT32 handle, UINT32 index, UINT32 *wcnt);
extern RET_CODE pvr_set_nv_uri_ext(PVR_HANDLE handle, nasc_uri_item_ext *uri_item);
extern RET_CODE pvr_get_nv_uri_ext(PVR_HANDLE handle, nasc_uri_item_ext *uri_item);
extern RET_CODE pvr_get_nv_uri_sets_ext(UINT32 index, UINT32 base, UINT32 cnt, nasc_uri_item_ext *uri_sets);
extern RET_CODE pvr_get_nv_uri_count_ext(UINT32 handle, UINT32 index, UINT32 *wcnt);
extern RET_CODE _pvr_mgr_get_cridential_data(PVR_HANDLE handle, char *path, void *buffer, UINT32 len);
extern RET_CODE _pvr_mgr_set_cridential_data(PVR_HANDLE handle, char *path, void *buffer, UINT32 len);
extern int aui_ca_get_pvr_special_mode(void);

#define AC3_DES_EXIST                   (1)//0x2000
#define AAC_DES_EXIST                   (2)//0x4000//LATM_AAC
#define EAC3_DES_EXIST                  (3)//0x0001//EAC3
#define ADTS_AAC_DES_EXIST              (4)//0x8000//ADTS_AAC

/**************************LOCAL FUNC IMPLEMENT**********************************/

static void aui_pvr_msg_callback(aui_pvr_event_msg *p_aui_pvr_msg)
{
    int i = 0;
    unsigned int index = 0;
    aui_pvr_stop_ply_param p_stop_param;
    aui_ply_param st_app;
    //struct list_info pvr_info;
    //unsigned short pvr_index=0;

    switch (p_aui_pvr_msg->msg_type) {
        case AUI_EVNT_PVR_END_DATAEND:
        case AUI_EVNT_PVR_END_REVS:     // in the backward mode, the player gets to the beginning of a file
            if ((g_ply_param.start_mode & AUI_PVR_PB_STOP_MODE) == 0) {
                pthread_mutex_lock(&s_pvr_mutexID);

                if (p_aui_pvr_msg->pvr_handle == 0) {
                    pthread_mutex_unlock(&s_pvr_mutexID);
                    return;
                }

                if (AUI_RTN_SUCCESS != aui_pvr_get((aui_hdl)p_aui_pvr_msg->pvr_handle, AUI_PVR_PLAY_INDEX, &index, NULL, NULL)) {
                    index = 2;
                }

                memset(&st_app, 0, sizeof(aui_ply_param));
                memset(&p_stop_param, 0, sizeof(aui_pvr_stop_ply_param));

                for (i = 0; i < s_max_pvr_record_number; i++) {
                    if (p_st_pvr_handle[i].pvr_handler == p_aui_pvr_msg->pvr_handle) {
                        st_app.user_data = p_st_pvr_handle[i].user_data;
                        st_app.fn_callback = p_st_pvr_handle[i].fn_callback;
                    }
                }

                p_stop_param.stop_mode = AUI_PVR_STOPPED_ONLY;
                p_stop_param.sync = 1;
                p_stop_param.vpo_mode = TRUE;

                if (AUI_RTN_SUCCESS != aui_pvr_ply_close((aui_hdl)p_aui_pvr_msg->pvr_handle, &p_stop_param)) {
                    AUI_ERR("aui_pvr_msg_callback aui_pvr_ply_close fail\n");
                }

                st_app.dmx_id = 2;
                st_app.index = index;
                st_app.live_dmx_id = 0;
                st_app.preview_mode = 0;
                st_app.speed = AUI_PVR_PLAY_SPEED_1X;
                st_app.start_mode = 0;
                st_app.start_pos = 0 ;
                st_app.start_time = 0;
                st_app.state = AUI_PVR_PLAY_STATE_PLAY;

                AUI_PRINTF("playback reach head or end ,replay record item !! \n");

                if (AUI_RTN_SUCCESS != aui_pvr_ply_open(&st_app, (aui_hdl)&p_aui_pvr_msg->pvr_handle)) {
                    AUI_ERR("aui_pvr_msg_callback aui_pvr_ply_open fail\n");
                }

                if (NV_PLAY != pvr_p_get_state(p_aui_pvr_msg->pvr_handle)) {
                    if (RET_SUCCESS != pvr_p_play(p_aui_pvr_msg->pvr_handle)) {
                        AUI_ERR("pvr_p_play failed!\r\n");
                        pthread_mutex_unlock(&s_pvr_mutexID);
                        return;
                    }
                }

                pthread_mutex_unlock(&s_pvr_mutexID);
            }

            break;

        default:
            break;
    }

    for (i = 0; i < s_max_pvr_record_number; i ++) {
        if (p_st_pvr_handle[i].pvr_handler != 0 && p_st_pvr_handle[i].pvr_handler == p_aui_pvr_msg->pvr_handle) {
            if (p_st_pvr_handle[i].fn_callback != NULL) {
                p_st_pvr_handle[i].fn_callback((aui_hdl)p_aui_pvr_msg->pvr_handle,
                                               p_aui_pvr_msg->msg_type, p_aui_pvr_msg->msg_code, p_st_pvr_handle[i].user_data);
                break;
            }
        }
    }

}

static void aui_pvr_event_task(void)
{
    INT cmd_size;
    aui_pvr_event_msg aui_event_in_queue;

    while (1) {
        if ((event_callback_id != OSAL_INVALID_ID)
                && (osal_msgqueue_receive((VP)&aui_event_in_queue, &cmd_size, event_callback_id, 10) == E_OK)) {

            aui_pvr_msg_callback(&aui_event_in_queue);
        } else {
            AUI_SLEEP(100);
        }

        if (OSAL_INVALID_ID == event_callback_id) {
            break;
        }
    }
}

static int aui_pvr_check_disk_size(int mode)
{
    struct dvr_hdd_info partition_info;

    MEMSET(&partition_info, 0, sizeof(struct dvr_hdd_info));

    if (TRUE != pvr_get_hdd_info(&partition_info)) {
        return 1;
    }

    if (partition_info.total_size > 0) {
        switch (mode) {
            case 0:  //normal record
                if (partition_info.rec_size > 0) {
                    return 0;
                }

                break;

            case 1:
                if (partition_info.tms_size > 0) {
                    return 0;
                }

                break;

            default:
                return 1;
        }
    }

    return 1;
}
static int hld_init(void)
{
    struct deca_feature_config deca_config;
    struct snd_feature_config snd_config;

    memset(&deca_config, 0, sizeof(struct deca_feature_config));
    memset(&snd_config, 0, sizeof(struct snd_feature_config));

    if (TRUE == init_once) {
        dmx_m36_attach(NULL);
        AUI_DBG("DMX Attached!\n");

        HLD_vdec_attach();
        AUI_DBG("VDEC Attached!\n");
        AUI_DBG("modified!!!\n");

        deca_m36_attach(&deca_config);
        AUI_DBG("DECA Attached!\n");
        void *g_deca_dev = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);

        if (deca_open(g_deca_dev, AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0)) {
            AUI_ERR("DECA Open Error!!!\n");
        }

        snd_m36_attach(&snd_config);
        AUI_DBG("SND Attached!\n");
        void *g_snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);

        if (snd_open(g_snd_dev)) {
            AUI_ERR("SND Open Error!!!\n");
        }

        HLD_vpo_attach();
        AUI_DBG("VPO Attached!\n");

        avsync_attach();
        AUI_DBG("AVSYNC Attached!\n");
        void *g_avsync_dev = (struct avsync_device *)dev_get_by_id(HLD_DEV_TYPE_AVSYNC, 0);

        if (avsync_open(g_avsync_dev)) {
            AUI_ERR("AVSYNC Open Error!!!\n");
        }

#ifdef AUI_TDS
        dsc_api_attach();
        ce_api_attach();
#endif
        trng_api_attach();

        init_once = FALSE;
    }

    dev_list_all(NULL);

    return 0;
}

static void hld_deinit()
{
    if (0) {
        dmx_m36_dettach();
        AUI_DBG("DMX dettached!\n");

        HLD_vdec_detach();
        AUI_DBG("VDEC dettached!\n");

        void *g_deca_dev = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);

        if (0 == deca_close(g_deca_dev)) {
            AUI_DBG("DECA close!!!\n");
            deca_m36_dettach(g_deca_dev);
            AUI_DBG("DECA dettached!\n");
        }

        void *g_snd_dev = dev_get_by_id(HLD_DEV_TYPE_SND, 0);

        if (0 == snd_close(g_snd_dev)) {
            AUI_DBG("SND close!!!\n");
            snd_m36_dettach(g_snd_dev);
            AUI_DBG("SND dettached!\n");
        }

        HLD_vpo_dettach();
        AUI_DBG("VPO dettach!\n");

        void *g_avsync_dev = (struct avsync_device *)dev_get_by_id(HLD_DEV_TYPE_AVSYNC, 0);

        if (0 == avsync_close(g_avsync_dev)) {
            AUI_DBG("AVSYNC close!!!\n");
            avsync_dettach(g_avsync_dev);
            AUI_DBG("AVSYNC dettached!!!\n");
        }

        dsc_api_detach();
        ce_api_detach();
        trng_api_detach();
    }

    dev_list_all(NULL);
}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief         init PVR module
*    @author        Raynard.wang
*    @date          2013-10-15
*    @param[in]     aui_pvr_init_param init parameter
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_init(const aui_pvr_init_param *p_init_cfg)
{
    int i = 0;
    PVR_MGR_CFG mgr_cfg;
    PVR_CFG ini_param;
    T_CMBF t_cmbf;
    T_CTHD task;
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    RET_CODE ret = SUCCESS;

    /* init hld */
    hld_init();

    if (TRUE == s_pvr_mod_inited) {
        return aui_ret;
    }

    /* init event_callback */
    if ((p_init_cfg->max_rec_number < 1) || (p_init_cfg->max_rec_number > 2) || (p_init_cfg->max_play_number > 1)) {
        return AUI_RTN_FAIL;
    }

    s_max_pvr_record_number = p_init_cfg->max_rec_number + p_init_cfg->max_play_number; //the num of callback should be rec + ply

    p_st_pvr_handle = MALLOC(sizeof(aui_pvr_handle) * s_max_pvr_record_number);

    if (NULL == p_st_pvr_handle) {
        return AUI_RTN_FAIL;
    }

    for (i = 0; i < s_max_pvr_record_number; i ++) {
        p_st_pvr_handle[i].pvr_handler = 0;
        p_st_pvr_handle[i].fn_callback = NULL;
    }

    /* set pvr param */
    MEMSET(&mgr_cfg, 0, sizeof(mgr_cfg));
    mgr_cfg.pvr_mode = PVR_REC_SUB_FOLDER;
    mgr_cfg.pvr_name_in_unicode = 0;
    mgr_cfg.debug_enable = 0;

    MEMSET(&ini_param, 0, sizeof(ini_param));
    ini_param.max_rec_number = p_init_cfg->max_rec_number;
//    SET_VALUE(ini_param.max_play_number,p_init_cfg->max_play_number,1);
    ini_param.max_play_number = p_init_cfg->max_play_number;
    ini_param.ac3_decode_support = p_init_cfg->ac3_decode_support == 0 ? 0 : 1;
    ini_param.continuous_tms_en = p_init_cfg->continuous_tms_en;
    ini_param.debug_level   = p_init_cfg->debug_level;
    STRCPY(ini_param.dvr_path_prefix, p_init_cfg->dvr_path_prefix);
    STRCPY(ini_param.info_file_name, p_init_cfg->info_file_name);
    STRCPY(ini_param.info_file_name_new, p_init_cfg->info_file_name_new);
    STRCPY(ini_param.ts_file_format, p_init_cfg->ts_file_format);
    STRCPY(ini_param.ts_file_format_new, "ts");
    STRCPY(ini_param.ps_file_format, p_init_cfg->ps_file_format);
    STRCPY(ini_param.test_file1, p_init_cfg->test_file1);
    STRCPY(ini_param.test_file2, p_init_cfg->test_file2);
    /*Default  use Trick mode 3.1*/
    ini_param.new_trick_mode = 1;
    ini_param.trim_record_ptm = p_init_cfg->trim_record_ptm;
    ini_param.encrypt_callback    = NULL;
    ini_param.event_callback      = aui_pvr_evnt_callback;
    ini_param.name_callback       = NULL;
    ini_param.local_time_callback = aui_pvr_timer_callback;
    ini_param.play_task_loop      = NULL;
    ini_param.play_task_set_packet_num = NULL;

    ini_param.ply_state_update_freq  =  1000;
    ini_param.rec_state_update_freq    = 1000;
    ini_param.rec_size_update_callback = NULL;

    ini_param.h264_vobu_time_len = p_init_cfg->h264_vobu_time_len == 0 ? 500 : p_init_cfg->h264_vobu_time_len;
    SET_VALUE(ini_param.scramble_vobu_time_len, p_init_cfg->scramble_vobu_time_len, 600);
    SET_VALUE(ini_param.file_header_save_dur, p_init_cfg->file_header_save_dur, 30);
    SET_VALUE(ini_param.record_min_len, p_init_cfg->record_min_len, 15);
    /* in seconds, recomment to 2h(7200); */
    SET_VALUE(ini_param.tms_time_max_len, p_init_cfg->tms_time_max_len, 7200);
    /* in MBytes,  recomment to 10M */
    SET_VALUE(ini_param.tms_file_min_size, p_init_cfg->tms_file_min_size, 2);

    ini_param.cache_addr = p_init_cfg->cache_addr;
    ini_param.cache_size = p_init_cfg->cache_size;

    ini_param.info_saving = NULL;
    ini_param.ps_packet_size = 188 * 16;
    ini_param.prj_mode  = p_init_cfg->prj_mode;
    ini_param.recover_enable = 0;

    /* init mutex */
    /*
    s_pvr_mutexID = osal_mutex_create();
    if (INVALID_ID == s_pvr_mutexID) {
        AUI_DBG("init mutex failed!\n");
        aui_ret = AUI_RTN_FAIL;
        return aui_ret;
    }
    */

    /* pvr attach */
    ret = pvr_attach(&mgr_cfg, &ini_param);

    if (SUCCESS != ret) {
        AUI_ERR("pvr attached failed!\n");
        aui_ret = AUI_RTN_FAIL;
        return aui_ret;
    }

    /* set init flag */
    s_pvr_mod_inited = TRUE;

    /* init msg queue */
    MEMSET(&t_cmbf, 0, sizeof(T_CMBF));
    t_cmbf.bufsz = sizeof(aui_pvr_event_msg) * 20;
    t_cmbf.maxmsz = sizeof(aui_pvr_event_msg);

    event_callback_id = osal_msgqueue_create(&t_cmbf);

    if (OSAL_INVALID_ID == event_callback_id) {
        aui_ret = AUI_RTN_FAIL;
        return aui_ret;
    }

    /* init task */
    MEMSET(&task, 0, sizeof(T_CTHD));
    task.task = (FP)aui_pvr_event_task;
    task.stksz = 0x4000;
    task.quantum = 10;
    task.itskpri = OSAL_PRI_NORMAL;
    task.name[0] = 'A';
    task.name[1] = 'P';
    task.name[2] = 'E';
    event_task_id = osal_task_create(&task);

    if (OSAL_INVALID_ID == event_task_id) {
        aui_ret = AUI_RTN_FAIL;
        return aui_ret;
    }

    return aui_ret;

}

/**
*    @brief         deinit PVR module
*    @author        Raynard.wang
*    @date          2013-10-12
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_deinit()
{
#ifdef AUI_LINUX
    hld_deinit();

    if (s_pvr_mod_inited) {
        s_pvr_mod_inited = FALSE;
        pvr_mgr_detach();
    }

#else
    osal_task_delete(event_task_id);
    osal_msgqueue_delete(event_callback_id);
    osal_mutex_delete(s_pvr_mutexID);
#endif

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         attach disk
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     p_apart_param attach parameter
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_disk_attach(const aui_pvr_disk_attach_info *p_apart_param)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    RET_CODE r_ret = SUCCESS;
    struct pvr_register_info pvr_reg_info;
    MEMSET(&pvr_reg_info, 0, sizeof(struct pvr_register_info));
    STRCPY(pvr_reg_info.mount_name, p_apart_param->mount_name);
    pvr_reg_info.disk_usage = p_apart_param->disk_usage;
    pvr_reg_info.sync = p_apart_param->sync;
    pvr_reg_info.init_list = p_apart_param->init_list;
    pvr_reg_info.check_speed = p_apart_param->check_speed;
    r_ret = pvr_register((UINT32)&pvr_reg_info, p_apart_param->vbh_len);

    if (SUCCESS != r_ret) {
        ret = AUI_RTN_FAIL;
    }

    AUI_DBG("attach success\n");
    return ret;
}


/**
*    @brief         detach disk
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     depart_param detach parameter
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_disk_detach(const aui_pvr_disk_detach_info *depart_param)
{
    //UNUSED(depart_param);
    struct pvr_clean_info clean_info;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (depart_param == NULL) {
        ret = pvr_cleanup_all();
    } else {
        memset(&clean_info, 0, sizeof(clean_info));
        clean_info.disk_mode = 0;
        strncpy(clean_info.mount_name, depart_param->mount_name, AUI_PVR_MOUNT_NAME_LEN_MAX - 1);
        ret = pvr_cleanup_partition(&clean_info);
    }

    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("[%s]L[%d] fail \n", __FUNCTION__, __LINE__);
    }

    return ret;
}


AUI_RTN_CODE aui_pvr_set_disk_use(unsigned char disk_usage, const char *mount_name)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    UINT8          inner_use = 0xFF;

    if (NULL == mount_name || '\0' == mount_name[0]) {
        AUI_ERR("invalid mount name\n");
        ret = AUI_RTN_FAIL;
        return ret;
    }

    inner_use = disk_usage;

    if (inner_use != 0xFF) {
        ret = pvr_set_disk_use(inner_use, mount_name);
    }

    return ret;
}
static void aui_pid_info_cp_to_pvr(struct pvr_pid_info *pvr_pid_info, aui_pvr_pid_info *pid_info)
{
    unsigned char i = 0;
    //if((NULL!=pvr_pid_info)&&(NULL != pid_info))
    {
        pvr_pid_info->audio_count = pid_info->audio_count;

        for (i = 0; i < AUI_MAX_PVR_AUDIO_PID; i++) {
            pvr_pid_info->audio_lang[i] = pid_info->audio_lang[i];
            pvr_pid_info->audio_pid[i] = pid_info->audio_pid[i];
            pvr_pid_info->audio_lang[i] = pid_info->audio_lang[i];
        }

        pvr_pid_info->cat_pid = pid_info->cat_pid;
        pvr_pid_info->cur_audio_pid_sel = pid_info->cur_audio_pid_sel;
        pvr_pid_info->isdbtcc_pid_count = pid_info->isdbtcc_pid_count;

        for (i = 0; i < AUI_MAX_PVR_ISDBTCC_PID; i++) {
            pvr_pid_info->isdbtcc_pids[i] = pid_info->isdbtcc_pids[i];
        }

        pvr_pid_info->ecm_pid_count = pid_info->ecm_pid_count;

        for (i = 0; i < AUI_MAX_PVR_ECM_PID; i++) {
            pvr_pid_info->ecm_pids[i] = pid_info->ecm_pids[i];
        }

        pvr_pid_info->eit_pid = pid_info->eit_pid;
        pvr_pid_info->emm_pid_count = pid_info->emm_pid_count;

        for (i = 0; i < AUI_MAX_PVR_EMM_PID; i++) {
            pvr_pid_info->emm_pids[i] = pid_info->emm_pids[i];
        }

        pvr_pid_info->nit_pid = pid_info->nit_pid;
        pvr_pid_info->pat_pid = pid_info->pat_pid;
        pvr_pid_info->pcr_pid = pid_info->pcr_pid;
        pvr_pid_info->pmt_pid = pid_info->pmt_pid;
        pvr_pid_info->sdt_pid = pid_info->sdt_pid;
        pvr_pid_info->subt_pid_count = pid_info->subt_pid_count;

        for (i = 0; i < AUI_MAX_PVR_SUBT_PID; i++) {
            pvr_pid_info->subt_pids[i] = pid_info->subt_pids[i];
        }

        pvr_pid_info->total_num = pid_info->total_num;
        pvr_pid_info->ttx_pid_count = pid_info->ttx_pid_count;

        for (i = 0; i < AUI_MAX_PVR_TTX_PID; i++) {
            pvr_pid_info->ttx_pids[i] = pid_info->ttx_pids[i];
        }

        pvr_pid_info->ttx_subt_pid_count = pid_info->ttx_subt_pid_count;

        for (i = 0; i < AUI_MAX_PVR_TTX_SUBT_PID; i++) {
            pvr_pid_info->ttx_subt_pids[i] = pid_info->ttx_subt_pids[i];
        }

        pvr_pid_info->video_pid = pid_info->video_pid;
    }
}

/**
*    @brief         start record
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     p_rec_param open record parameter
*    @param[in]     handle      PVR record handler
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_rec_open(const aui_record_prog_param *p_rec_param, aui_hdl *handle)
{
    //set up record basic info:type,mode
    struct record_prog_param prog_info;
    int i = 0;
    PVR_HANDLE pvr_handle = 0;

    MEMSET(&prog_info, 0, sizeof(prog_info));

    if (AUI_REC_MODE_NORMAL == p_rec_param->is_tms_record) {
        prog_info.mode = RECORDING_NORMAL;

        if (aui_pvr_check_disk_size(0) != 0) {
            return AUI_RTN_FAIL;
        }
    }

    if (AUI_REC_MODE_TMS == p_rec_param->is_tms_record) {
        prog_info.mode = RECORDING_TIMESHIFT;

        if (aui_pvr_check_disk_size(1) != 0) {
            return AUI_RTN_FAIL;
        }

        if (AUI_REC_MODE_TP == p_rec_param->is_tms_record) {
            prog_info.mode = RECORDING_TIMESHIFT;
        }
    }

    prog_info.rec_type = p_rec_param->rec_type;
    prog_info.full_path = p_rec_param->full_path;
    prog_info.channel_id = p_rec_param->channel_id;
    prog_info.audio_channel = p_rec_param->audio_channel;
    prog_info.continuous_tms = p_rec_param ->continuous_tms;
    STRCPY(prog_info.folder_name, p_rec_param->folder_name);
    prog_info.record_file_size = 4 * 1024 * 1024; //1G bytes
    prog_info.tms_total_size = 4 * 1024 * 1024; //1G bytes
    prog_info.h264_flag = p_rec_param->h264_flag;
    prog_info.av_flag = p_rec_param->av_flag;
    aui_pid_info_cp_to_pvr((struct pvr_pid_info *)(&(prog_info.pid_info)), (aui_pvr_pid_info *)(&(p_rec_param->pid_info)));
    //MEMCPY(&prog_info.pid_info,&p_rec_param->pid_info,sizeof(prog_info.pid_info));
    prog_info.pid_info.pat_pid = 0;
    //prog_info.pid_info.pmt_pid = 0;
    prog_info.pid_info.sdt_pid = 17;
    prog_info.is_reencrypt = p_rec_param->is_reencrypt;
    prog_info.ca_mode  = p_rec_param->ca_mode;
    prog_info.rec_special_mode = p_rec_param->rec_special_mode;
    prog_info.is_scrambled = p_rec_param->is_scrambled;
    prog_info.dmx_id = p_rec_param->dmx_id;
    prog_info.prog_number =  p_rec_param->prog_number;
    prog_info.name_len = 12;
    prog_info.append_to_exist_file = p_rec_param->append_to_exist_file;
    prog_info.record_whole_tp_data = p_rec_param->record_whole_tp_data;
    strncpy(prog_info.service_name, p_rec_param->service_name, AUI_PVR_SERVICE_NAME_LEN_MAX - 1);
    strncpy(prog_info.folder_name, p_rec_param->folder_name, AUI_PVR_FILE_PATH_LEN_MAX - 1);

    if (1) {
        unsigned short  pid_list[64];
        unsigned int pid_num = 0;
        int i = 0;
        MEMSET(pid_list, 0, PVR_MAX_PID_NUM * sizeof(unsigned short));
        //prog_info.pid_info.video_pid = p_rec_param->pid_info.video_pid;
        pid_list[pid_num++]  = prog_info.pid_info.video_pid;

        if (p_rec_param->pid_info.audio_count <= AUI_MAX_PVR_AUDIO_PID && p_rec_param->pid_info.audio_count > 0) {
            //prog_info.pid_info.audio_count = p_rec_param->pid_info.audio_count;
            for (i = 0; i < p_rec_param->pid_info.audio_count; i++) {
                AUI_DBG("%s %d before aui_get_pul_deca_cur_type:%d prog_info.pid_info.audio_pid[i]:%08x\r\n",
                        __FUNCTION__, __LINE__, p_rec_param->pid_info.audio_type[i], prog_info.pid_info.audio_pid[i]);
                av_get_audio_pid(p_rec_param->pid_info.audio_type[i], p_rec_param->pid_info.audio_pid[i], &prog_info.pid_info.audio_pid[i]);
                AUI_DBG("%s %d after aui_get_pul_deca_cur_type:%d prog_info.pid_info.audio_pid[i]:%08x\r\n", __FUNCTION__, __LINE__, p_rec_param->pid_info.audio_type[i], prog_info.pid_info.audio_pid[i]);
                pid_list[pid_num++] = prog_info.pid_info.audio_pid[i];
            }
        }

        if (p_rec_param->pid_info.ttx_pid_count <= AUI_MAX_PVR_TTX_PID && p_rec_param->pid_info.ttx_pid_count > 0) {
            //prog_info.pid_info.ttx_pid_count = p_rec_param->pid_info.ttx_pid_count;
            for (i = 0; i < p_rec_param->pid_info.ttx_pid_count; i++) {
                //prog_info.pid_info.ttx_pids[i] = p_rec_param->pid_info.ttx_pids[i];
                pid_list[pid_num++] = prog_info.pid_info.ttx_pids[i];
            }
        }

        if (p_rec_param->pid_info.ttx_subt_pid_count <= AUI_MAX_PVR_TTX_SUBT_PID && p_rec_param->pid_info.ttx_subt_pid_count > 0) {
            //prog_info.pid_info.ttx_subt_pid_count = p_rec_param->pid_info.ttx_subt_pid_count;
            for (i = 0; i < p_rec_param->pid_info.ttx_subt_pid_count; i++) {
                //prog_info.pid_info.ttx_subt_pids[i] = p_rec_param->pid_info.ttx_subt_pids[i];
                pid_list[pid_num++] = prog_info.pid_info.ttx_subt_pids[i];
            }
        }

        if (p_rec_param->pid_info.subt_pid_count <= AUI_MAX_PVR_SUBT_PID && p_rec_param->pid_info.subt_pid_count > 0) {
            //prog_info.pid_info.subt_pid_count = p_rec_param->pid_info.subt_pid_count;
            for (i = 0; i < p_rec_param->pid_info.subt_pid_count; i++) {
                //prog_info.pid_info.subt_pids[i] = p_rec_param->pid_info.subt_pids[i];
                pid_list[pid_num++] = prog_info.pid_info.subt_pids[i];
            }
        }

        if (p_rec_param->pid_info.ecm_pid_count <= AUI_MAX_PVR_ECM_PID && p_rec_param->pid_info.ecm_pid_count > 0) {
            //prog_info.pid_info.ecm_pid_count = p_rec_param->pid_info.ecm_pid_count;
            for (i = 0; i < p_rec_param->pid_info.ecm_pid_count; i++) {
                prog_info.pid_info.ecm_pids[i] = p_rec_param->pid_info.ecm_pids[i];
                //pid_list[pid_num++] = prog_info.pid_info.ecm_pids[i];
            }
        }

        if (p_rec_param->pid_info.emm_pid_count <= AUI_MAX_PVR_EMM_PID && p_rec_param->pid_info.emm_pid_count > 0) {
            //prog_info.pid_info.emm_pid_count = p_rec_param->pid_info.emm_pid_count;
            for (i = 0; i < p_rec_param->pid_info.emm_pid_count; i++) {
                prog_info.pid_info.emm_pids[i] = p_rec_param->pid_info.emm_pids[i];
                //pid_list[pid_num++] = prog_info.pid_info.emm_pids[i];
            }
        }

        //prog_info.pid_info.cat_pid = p_rec_param->pid_info.cat_pid;
        //pid_list[pid_num++] = prog_info.pid_info.cat_pid;
        //prog_info.pid_info.pcr_pid = p_rec_param->pid_info.pcr_pid;
        //pid_list[pid_num++] = prog_info.pid_info.pcr_pid;
        //prog_info.pid_info.pat_pid = p_rec_param->pid_info.pat_pid;
        //pid_list[pid_num++] = prog_info.pid_info.pat_pid;
        //prog_info.pid_info.pmt_pid = p_rec_param->pid_info.pmt_pid;
        //pid_list[pid_num++] = prog_info.pid_info.pmt_pid;

        //prog_info.pid_info.sdt_pid = p_rec_param->pid_info.sdt_pid;
        //pid_list[pid_num++] = prog_info.pid_info.sdt_pid;

        //prog_info.pid_info.nit_pid = p_rec_param->pid_info.nit_pid;
        //pid_list[pid_num++] = prog_info.pid_info.nit_pid;

        //prog_info.pid_info.eit_pid = p_rec_param->pid_info.eit_pid;
        //pid_list[pid_num++] = prog_info.pid_info.eit_pid;

        if (pid_num > 32)
            pid_num = 32;

        aui_ca_set_dsc_pid_multi_des(pid_list, pid_num, prog_info.channel_id);
    }

    pvr_handle = pvr_get_free_record_handle(prog_info.mode);

    if (pvr_handle == 0) {
        return AUI_RTN_FAIL;
    }

    aui_pvr_callback_register((aui_hdl)pvr_handle, p_rec_param->fn_callback);

    for (i = 0; i < s_max_pvr_record_number; i++) {
        if (p_st_pvr_handle[i].pvr_handler == pvr_handle) {
            p_st_pvr_handle[i].user_data = p_rec_param->user_data;
        }
    }

    pvr_handle = pvr_r_open(&prog_info);
    *handle = (aui_hdl)pvr_handle;

    //should check if pvr_handle equal to INVALID_HANDLE ,otherwise if pvr_handle == INVALID_HANDLE will return success
    if (0 == pvr_handle || INVALID_HANDLE == pvr_handle) {
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         change record state
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pvr_handle  PVR record handler
*    @param[in]     new_state   new state to set
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_rec_state_change(aui_hdl handle, aui_pvr_rec_state new_state)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    RET_CODE pvr_ret = SUCCESS;
    BOOL bret = TRUE;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;

    switch (new_state) {
        case AUI_REC_STATE_RECORDING:
            pvr_ret = pvr_r_resume(pvr_handle);

            if (pvr_ret != RET_SUCCESS) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_REC_STATE_PAUSE:
            pvr_ret = pvr_r_pause(pvr_handle);

            if (pvr_ret != RET_SUCCESS) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_REC_STATE_STOP:
            bret = pvr_r_close(&pvr_handle, 0);

            if (bret != TRUE && pvr_handle != 0) {
                ret = AUI_RTN_FAIL;
            }

            break;

        default:
            ret = AUI_RTN_FAIL;
            break;
    }

    return ret;
}

/**
*    @brief         stop record
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     handle  PVR record handler
*    @param[in]     sync    sync flag  0 -- async, 1 -- sync
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_rec_close(aui_hdl handle, int sync)
{
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;
    BOOL ret = 0;
    int i;
    unsigned int prog_id;
    prog_id = pvr_r_get_channel_id(pvr_handle);
    ret = pvr_r_close(&pvr_handle, sync);
    aui_ca_set_dsc_pid_multi_des(NULL, 0, prog_id);

    /****  unregister recoder callback if exist  ****/
    for (i = 0; i < s_max_pvr_record_number; i ++) {
        if (p_st_pvr_handle[i].pvr_handler == (PVR_HANDLE)handle) {
            if (p_st_pvr_handle[i].fn_callback) {
                aui_pvr_callback_unregister(handle, p_st_pvr_handle[i].fn_callback);
                break;
            }
        }
    }

    if (pvr_handle == 0) {
        return AUI_RTN_SUCCESS;
    }

    if (ret == 0) {
        return AUI_RTN_SUCCESS;
    } else if (ret == 2) {
        ret = pvr_r_close(&pvr_handle, sync);

        if (0 == ret) {
            return AUI_RTN_SUCCESS;
        }

        return AUI_RTN_FAIL;
    } else {
        return AUI_RTN_FAIL;
    }
}

/**
*    @brief         start playback
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     handle     PVR playback handler
*    @param[in]     ply_param  playback parameter
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_ply_open(aui_ply_param *p_ply_param, aui_hdl *handle)
{
    UINT16 i = 0;
    UINT16 index = 0;
    PVR_HANDLE pvr_handle = 0;
    struct playback_param st_play_param;
    MEMSET(&st_play_param, 0, sizeof(st_play_param));
    MEMSET(&g_ply_param, 0, sizeof(aui_ply_param));
    MEMCPY(&g_ply_param, p_ply_param, sizeof(aui_ply_param));
    st_play_param.dmx_id = p_ply_param->dmx_id;

    if (p_ply_param->index == 0) {
        st_play_param.index = index;
    } else {
        st_play_param.index = p_ply_param->index;
    }

    st_play_param.live_dmx_id = p_ply_param->live_dmx_id;
    MEMCPY(st_play_param.path, p_ply_param->path, sizeof(st_play_param.path) - 1);
    st_play_param.path[sizeof(st_play_param.path) - 1] = 0;
    st_play_param.preview_mode = p_ply_param->preview_mode;

    switch (p_ply_param->speed) {
        case AUI_PVR_PLAY_SPEED_1X:
            st_play_param.speed = 1;
            break;

        case AUI_PVR_PLAY_SPEED_2X:
            st_play_param.speed = 2;
            break;

        case AUI_PVR_PLAY_SPEED_4X:
            st_play_param.speed = 4;
            break;

        case AUI_PVR_PLAY_SPEED_8X:
            st_play_param.speed = 8;
            break;

        case AUI_PVR_PLAY_SPEED_16X:
            st_play_param.speed = 16;
            break;

        case AUI_PVR_PLAY_SPEED_32X:
            st_play_param.speed = 32;
            break;
    }

    st_play_param.start_mode = (p_ply_param->start_mode & 0x7F);
    st_play_param.start_pos = p_ply_param->start_pos;
    //st_play_param.is_support_variable_resolution = 1;
    st_play_param.start_time = p_ply_param->start_time;
    st_play_param.stop_mode = P_STOPPED_ONLY;

    if (p_ply_param->state == AUI_PVR_PLAY_STATE_PAUSE) {
        st_play_param.state = AUI_PVR_PLAY_STATE_PLAY;
    } else {
        st_play_param.state = p_ply_param->state;
    }

    pvr_handle = pvr_get_free_player_handle();

    if (pvr_handle == 0) {
        AUI_ERR("FAIL");
        return AUI_RTN_FAIL;
    }

    aui_pvr_callback_register((aui_hdl)pvr_handle, p_ply_param->fn_callback);

    for (i = 0; i < s_max_pvr_record_number; i++) {
        if (p_st_pvr_handle[i].pvr_handler == pvr_handle) {
            p_st_pvr_handle[i].user_data = p_ply_param->user_data;
        }
    }

    pvr_handle = pvr_p_open_ext(&st_play_param);
    AUI_DBG("aui_pvr_ply_open pvr_HANDLER:%08x\n", (unsigned int)pvr_handle);

    if (0 == pvr_handle) {
        AUI_ERR("FAIL");
        return AUI_RTN_FAIL;
    }

    p_ply_param->index = st_play_param.index;

    if (AUI_PVR_PLAY_STATE_PAUSE == p_ply_param->state) {
        pvr_p_pause(pvr_handle);
    }

    *handle = (aui_hdl)pvr_handle;
    return AUI_RTN_SUCCESS;
}

/**
*    @brief         change playback state
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pvr_handle  PVR playback handler
*    @param[in]     new_state   new state to set
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_ply_state_change(aui_hdl handle, AUI_PVR_PLAY_STATE new_state,
                                      int state_param)
{
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;
    BOOL ret = TRUE;

    AUI_DBG("aui_pvr_ply_state_change pvr_HANDLER:%d\n", (int)pvr_handle);

    if (0 == pvr_handle) {
        return AUI_RTN_SUCCESS;
    }

    switch (new_state) {
        /* playing */
        case AUI_PVR_PLAY_STATE_PLAY:
            ret = pvr_p_play(pvr_handle);
            break;

        /* fast forward */
        case AUI_PVR_PLAY_STATE_FF:
            ret = pvr_p_fast_forward(pvr_handle, 2);
            break;

        /** fast backward**/
        case AUI_PVR_PLAY_STATE_FB:
            ret = pvr_p_fast_backward(pvr_handle, 2);
            break;

        /* play slow */
        case AUI_PVR_PLAY_STATE_SLOW:
            pvr_p_slow(pvr_handle, 2);
            break;

        /* play revslow */
        case AUI_PVR_PLAY_STATE_REVSLOW:
            pvr_p_revslow(pvr_handle, 2);
            break;

        /* play pause */
        case AUI_PVR_PLAY_STATE_PAUSE:
            pvr_p_pause(pvr_handle);
            break;

        /* play stop */
        case AUI_PVR_PLAY_STATE_STOP:
            pvr_p_stop(pvr_handle);
            break;

        /* play step */
        case AUI_PVR_PLAY_STATE_STEP:
            pvr_p_step(pvr_handle);
            break;

        case AUI_PVR_PLAY_STATE_SPEED:
            switch (state_param) {
                case AUI_PLAYER_SPEED_NORMAL:
                    pthread_mutex_lock(&s_pvr_mutexID);
                    ret = pvr_p_play(pvr_handle);
                    pthread_mutex_unlock(&s_pvr_mutexID);
                    break;

                case AUI_PLAYER_SPEED_FASTREWIND_64:
                    ret = pvr_p_fast_backward(pvr_handle, 64);
                    break;

                case AUI_PLAYER_SPEED_FASTREWIND_32:
                    ret = pvr_p_fast_backward(pvr_handle, 32);
                    break;

                case AUI_PLAYER_SPEED_FASTREWIND_16:
                    ret = pvr_p_fast_backward(pvr_handle, 16);
                    break;

                case AUI_PLAYER_SPEED_FASTREWIND_8:
                    ret = pvr_p_fast_backward(pvr_handle, 8);
                    break;

                case AUI_PLAYER_SPEED_FASTREWIND_4:
                    ret = pvr_p_fast_backward(pvr_handle, 4);
                    break;

                case AUI_PLAYER_SPEED_FASTREWIND_2:
                    ret = pvr_p_fast_backward(pvr_handle, 2);
                    break;
                
                case AUI_PLAYER_SPEED_FASTREWIND_1:
                    ret = pvr_p_fast_backward(pvr_handle,1);
                    break;			
                
                case AUI_PLAYER_SPEED_SLOWREWIND_2:
                    ret = pvr_p_revslow(pvr_handle, 2);
                    break;

                case AUI_PLAYER_SPEED_SLOWREWIND_4:
                    ret = pvr_p_revslow(pvr_handle, 4);
                    break;

                case AUI_PLAYER_SPEED_SLOWREWIND_8:
                    ret = pvr_p_revslow(pvr_handle, 8);
                    break;

                case AUI_PLAYER_SPEED_SLOWFORWARD_2:
                    ret = pvr_p_slow(pvr_handle, 2);
                    break;

                case AUI_PLAYER_SPEED_SLOWFORWARD_4:
                    ret = pvr_p_slow(pvr_handle, 4);
                    break;

                case AUI_PLAYER_SPEED_SLOWFORWARD_8:
                    ret = pvr_p_slow(pvr_handle, 8);
                    break;

                case AUI_PLAYER_SPEED_FASTFORWARD_2:
                    ret = pvr_p_fast_forward(pvr_handle, 2);
                    break;

                case AUI_PLAYER_SPEED_FASTFORWARD_4:
                    ret = pvr_p_fast_forward(pvr_handle, 4);
                    break;

                case AUI_PLAYER_SPEED_FASTFORWARD_8:
                    ret = pvr_p_fast_forward(pvr_handle, 8);
                    break;

                case AUI_PLAYER_SPEED_FASTFORWARD_16:
                    ret = pvr_p_fast_forward(pvr_handle, 16);
                    break;

                case AUI_PLAYER_SPEED_FASTFORWARD_32:
                    ret = pvr_p_fast_forward(pvr_handle, 32);
                    break;

                case AUI_PLAYER_SPEED_FASTFORWARD_64:
                    ret = pvr_p_fast_forward(pvr_handle, 64);
                    break;

                default:
                    break;
            }

            break;

        default:
            return AUI_RTN_FAIL;
    }

    if (ret != TRUE) {
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         seek
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     aui_handle      PVR playback handler
*    @param[in]     pos             seek position in second
*    @param[in]     ePlayPosFlag    seek mode, refer to ePlayPosFlag
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_ply_seek(aui_hdl handle, const int nPosInSec,
                              const aui_pvr_play_position_flag ePlayPosFlag)
{
    AUI_RTN_CODE a_ret = AUI_RTN_SUCCESS;
    BOOL bret = TRUE;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;
    struct list_info pvr_info;
    unsigned short pvr_index = 0;

    //AUI_DBG("aui_pvr_ply_seek pvr_HANDLER:%d\n",pvr_handle);

    if (0 == pvr_handle) {
        return AUI_RTN_SUCCESS;
    }

    switch (ePlayPosFlag) {
        case AUI_PVR_PLAY_POSITION_FROM_HEAD:
            bret = pvr_p_timesearch(pvr_handle, nPosInSec);
            break;

        case AUI_PVR_PLAY_POSITION_FROM_CURRENT:
            bret = pvr_p_timesearch(pvr_handle, pvr_p_get_time(pvr_handle) + nPosInSec);
            break;

        case AUI_PVR_PLAY_POSITION_FROM_END:
            pvr_index = pvr_get_index(pvr_handle);
            pvr_get_rl_info(pvr_index, &pvr_info);
            bret = pvr_p_timesearch(pvr_handle, pvr_info.duration + nPosInSec);
            break;

        default:
            break;
    }

    if (TRUE != bret) {
        AUI_ERR("aui_pvr_seek failed!");
        a_ret = AUI_RTN_FAIL;
    }

    return a_ret;
}


/**
*    @brief         stop playback
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     handle      PVR playback handler
*    @param[in]     stop_param  stop parameter
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_ply_close(aui_hdl handle, const aui_pvr_stop_ply_param *p_stop_param)
{
    int i;
    BOOL ret = TRUE;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;
    struct playback_stop_param stop_param;
    struct dmx_device *dmx_dev = NULL;

    //AUI_DBG("aui_pvr_ply_close pvr_HANDLER:%d\n",pvr_handle);
    if (pvr_handle == 0) {
        return AUI_RTN_SUCCESS;
    }

    dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, pvr_r_get_dmx_id(pvr_handle));

    if ((NULL != dmx_dev) && (RET_SUCCESS == dmx_io_control(dmx_dev, DMX_IS_IN_TIMESHIFT, 0))) {
        if (RET_SUCCESS != dmx_io_control(dmx_dev, DO_TIME_SHIFT, 0)) {
            return RET_FAILURE;
        }
    }

    MEMSET(&stop_param, 0, sizeof(struct playback_stop_param));
    stop_param.stop_mode = P_STOPPED_ONLY;
    stop_param.sync = p_stop_param ->sync;
    stop_param.vpo_mode = p_stop_param ->vpo_mode;
    ret = pvr_p_close_ext(&pvr_handle, &stop_param);

    /****  unregister player callback if exist  ****/
    for (i = 0; i < s_max_pvr_record_number; i ++) {
        if (p_st_pvr_handle[i].pvr_handler == (PVR_HANDLE)handle) {
            if (p_st_pvr_handle[i].fn_callback) {
                aui_pvr_callback_unregister(handle, p_st_pvr_handle[i].fn_callback);
                break;
            }
        }
    }

    if (ret == 2) {
        ret = pvr_p_close_ext(&pvr_handle, &stop_param);

        if (ret == 2)
            aui_rtn(AUI_RTN_EBUSY, "PVR close failed");
    }

    if (TRUE == ret) {
        return AUI_RTN_SUCCESS;
    } else {
        return AUI_RTN_FAIL;
    }

    //Stop Adec
    //Stop Vdec
    //Sopt DMX2,display av str on DMX2

    //reset Stream path

    //Delete VOD read cache

    //close playback record dir and files

    //reset current channel conditionallly

    //No more data,read fail

    //send NODATA MSG by callback
    return AUI_RTN_SUCCESS;
}
static enum aui_decv_format aui_pvr_get_vedio_type(unsigned char h264_flag)
{
    enum aui_decv_format vedio_type = AUI_DECV_FORMAT_MPEG;

    switch (h264_flag) {
        case 0:
            vedio_type = AUI_DECV_FORMAT_MPEG;
            break;

        case 1:
            vedio_type = AUI_DECV_FORMAT_AVC;
            break;

        case 2:
            vedio_type = AUI_DECV_FORMAT_AVS;
            break;

        case 3:
            vedio_type = AUI_DECV_FORMAT_HEVC;
            break;

        default:
            break;
    }

    return vedio_type;
}

/**
*    @brief         PVR get command
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pvr_handle     PVR handler
*    @param[in]     cmd  PVR command
*    @param[in]     param_1 1st parameter of PVR command
*    @param[in]     param_2 2nd parameter of PVR command
*    @param[in]     param_3 3rd parameter of PVR command
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_get(aui_hdl handle, aui_pvr_cmd cmd,
                         unsigned int *p_param_1, unsigned int *p_param_2, unsigned int *p_param_3)
{
    (void)p_param_3;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;
    unsigned int RecTime = 0, tms_cap = 0, tmp_start_time = 0;
    UINT16 index = 0;
    struct list_info rec_item_info;
    struct list_info pb_item_info;
    int i = 0;
    aui_pvr_pid_info *pid_info = NULL;
    UINT32 ptm = 0;
    UINT16 record_idx = 0;
    UINT16 pos = 0;
    struct store_info_data *storeinfodata = NULL;
    struct store_info_header *sheader = NULL;
    unsigned int audio_type;
    aui_pvr_rec_item_info *pvr_info = NULL;

    /*
    if(0 == pvr_handle) {
        return AUI_RTN_SUCCESS;
    }
    */
    switch (cmd) {
        case AUI_PVR_REC_STATES:
            *p_param_1 = pvr_r_get_state(pvr_handle);

            break;

        case AUI_PVR_REC_DMX_ID:
            *p_param_1 = pvr_r_get_dmx_id(pvr_handle);
            break;

        case AUI_PVR_REC_TIME_S:
            *p_param_1 = pvr_r_get_time(pvr_handle);
            break;

        case AUI_PVR_REC_TIME_MS:
            *p_param_1 = pvr_r_get_ms_time(pvr_handle);
            break;

        case AUI_PVR_REC_TIME_TMS:
            RecTime = pvr_r_get_time(pvr_handle);
            tms_cap = pvr_get_tms_capability();
            tmp_start_time = (RecTime >= tms_cap) ? (RecTime - tms_cap) : 0;
            *p_param_1 = tmp_start_time;
            break;

        case AUI_PVR_PLAY_BITRATE:
            index = pvr_get_index(pvr_handle);
            MEMSET(&pb_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &pb_item_info);
            *p_param_1 = pb_item_info.ts_bitrate;
            break;

        case AUI_PVR_REC_MODE_T:
            break;

        case AUI_PVR_REC_SIZE_KB:
            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);
            *p_param_1 = rec_item_info.size;
            break;

        case AUI_PVR_REC_CAPBILITY:
            *p_param_1 = pvr_get_tms_capability();
            break;

        case AUI_PVR_PLAY_STATES:
            *p_param_1 = pvr_p_get_state(pvr_handle);
            break;

        case AUI_PVR_PLAY_SPEED:
            *p_param_1 = pvr_p_get_speed(pvr_handle);
            break;

        case AUI_PVR_PLAY_DIRECTION:
            *p_param_1 = pvr_p_get_direct(pvr_handle);
            break;

        case AUI_PVR_PLAY_TIME_S:
            *p_param_1 = pvr_p_get_time(pvr_handle);
            break;

        case AUI_PVR_PLAY_TIME_MS:
            *p_param_1 = pvr_p_get_ms_time(pvr_handle);
            break;

        case AUI_PVR_PLAY_POS:
            *p_param_1 = pvr_p_get_pos(pvr_handle);
            break;

        case AUI_PVR_PLAY_DMX_ID:
            break;

        case AUI_PVR_PLAY_PATH:
            index = pvr_get_index(pvr_handle);

            if (p_param_1 == NULL || *p_param_2 <= 0 || *p_param_2  >= 256)
                return AUI_RTN_FAIL;

            pvr_get_path(index, (char *)p_param_1, *p_param_2);
            *p_param_2 = strlen((char *)p_param_1);
            break;

        case AUI_PVR_PLAY_INDEX:
            index = pvr_get_index(pvr_handle);
            *p_param_1 = index;
            break;

        case AUI_PVR_TS_ROUTE:
            break;

        case AUI_PVR_DEBUG_MODE:
            break;

        case AUI_PVR_ITEM_INFO:
            break;

        case AUI_PVR_PID_INFO:
            if ((pid_info = (aui_pvr_pid_info *)p_param_1) == NULL) {
                return AUI_RTN_FAIL;
            }

            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);
            pid_info->video_pid = (rec_item_info.pid_info.video_pid & 0x1fff);
            pid_info->audio_count = rec_item_info.pid_info.audio_count;

            for (i = 0; i < pid_info->audio_count; i++) {
                pid_info->audio_pid[i] = (rec_item_info.pid_info.audio_pid[i] & 0x1fff);
                //fix  Bug #72072  aui_pvr_get audio_type error
                audio_type = (rec_item_info.pid_info.audio_pid[i] & 0xe000) >> 13;

                switch (audio_type) {
                    case AC3_DES_EXIST:
                        pid_info->audio_type[i] = AUI_DECA_STREAM_TYPE_AC3;
                        break;

                    case AAC_DES_EXIST:
                        pid_info->audio_type[i] = AUI_DECA_STREAM_TYPE_AAC_LATM;
                        break;

                    case EAC3_DES_EXIST:
                        pid_info->audio_type[i] = AUI_DECA_STREAM_TYPE_EC3;
                        break;

                    case ADTS_AAC_DES_EXIST:
                        pid_info->audio_type[i] = AUI_DECA_STREAM_TYPE_AAC_ADTS;
                        break;

                    default:
                        pid_info->audio_type[i] = AUI_DECA_STREAM_TYPE_MPEG1;
                        break;
                }
            }

            pid_info->ecm_pid_count = rec_item_info.pid_info.ecm_pid_count;

            for (i = 0; i < pid_info->ecm_pid_count; i++) {
                pid_info->ecm_pids[i] = rec_item_info.pid_info.ecm_pids[i];
            }

            pid_info->emm_pid_count = rec_item_info.pid_info.emm_pid_count;

            for (i = 0; i < pid_info->emm_pid_count; i++) {
                pid_info->emm_pids[i] = rec_item_info.pid_info.emm_pids[i];
            }

            pid_info->ttx_pid_count = rec_item_info.pid_info.ttx_pid_count;

            for (i = 0; i < pid_info->ttx_pid_count; i++) {
                pid_info->ttx_pids[i] = rec_item_info.pid_info.ttx_pids[i];
            }

            pid_info->subt_pid_count = rec_item_info.pid_info.subt_pid_count;

            for (i = 0; i < pid_info->subt_pid_count; i++) {
                pid_info->subt_pids[i] = rec_item_info.pid_info.subt_pids[i];
            }

            pid_info->ttx_subt_pid_count = rec_item_info.pid_info.ttx_subt_pid_count;

            for (i = 0; i < pid_info->ttx_subt_pid_count; i++) {
                pid_info->ttx_subt_pids[i] = rec_item_info.pid_info.ttx_subt_pids[i];
            }

            pid_info->cat_pid = rec_item_info.pid_info.cat_pid;
            pid_info->pcr_pid = rec_item_info.pid_info.pcr_pid;
            pid_info->cat_pid = rec_item_info.pid_info.cat_pid;
            pid_info->pcr_pid = rec_item_info.pid_info.pcr_pid;
            pid_info->pat_pid = rec_item_info.pid_info.pat_pid;
            pid_info->pmt_pid = rec_item_info.pid_info.pmt_pid;
            pid_info->sdt_pid = rec_item_info.pid_info.sdt_pid;
            pid_info->nit_pid = rec_item_info.pid_info.nit_pid;
            pid_info->eit_pid = rec_item_info.pid_info.eit_pid;
            break;

        case AUI_PVR_ITEM_KEY_INFO:
            break;

        case AUI_PVR_ITEM_FINGER_INFO:
            break;

        case AUI_PVR_ITEM_RATING_INFO:
            break;

        case AUI_PVR_ITEM_GM_INFO:
            break;

        case AUI_PVR_ITEMS_NUM:
            *p_param_1 = pvr_get_rl_count();
            break;

        case AUI_PVR_ITEM_DURATION:
            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);
            *p_param_1 = rec_item_info.duration;
            break;

        case AUI_PVR_AUDIO_PID_COUNT:
            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);
            *p_param_1 = rec_item_info.audio_count;
            break;

        case AUI_PVR_AUDIO_PID_LIST:
            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index, &rec_item_info);

            for (i = 0; i < rec_item_info.audio_count; i ++) {
                p_param_1[i] = rec_item_info.multi_audio_pid[i];
            }

            break;

        case AUI_PVR_RECORD_INDEX_BY_POS:
            if (p_param_1 == NULL || p_param_2 == NULL) {
                ret = AUI_RTN_FAIL;
                break;
            }

            pos = *p_param_1;
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            ret = pvr_get_rl_info_by_pos(pos, &rec_item_info);

            if (RET_SUCCESS != ret) {
                AUI_ERR("get index [%d]\n", rec_item_info.index);
                ret = AUI_RTN_FAIL;
                break;
            }

            *p_param_2 = rec_item_info.index;
            break;

        case AUI_PVR_ITEM_DURATION_BY_INDEX:
            if (p_param_1 == NULL || p_param_2 == NULL) {
                ret = AUI_RTN_FAIL;
                break;
            }

            index = *p_param_1;
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);
            *p_param_2 = rec_item_info.duration;
            break;

        case AUI_PVR_FREE_SIZE_LOW_THRESHOLD:
            *p_param_1 = pvr_get_free_size_low_threshold();
            break;

        case AUI_PVR_INFO_HEADER:
            sheader = (struct store_info_header *)p_param_1;

            if (pvr_get_info_header(pvr_handle, sheader) == FALSE) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_INFO_HEADER_BY_INDEX:
            sheader = (struct store_info_header *)p_param_1;
            record_idx = *p_param_2;

            if (pvr_get_info_header_by_idx(record_idx, sheader) == FALSE) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_INFO_DATA:
            storeinfodata = (struct store_info_data *)p_param_1;
            ptm = *p_param_2;

            if (pvr_get_store_info(pvr_handle, storeinfodata, ptm) == FALSE) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_INFO_DATA_BY_INDEX:
            storeinfodata = (struct store_info_data *)p_param_1;
            record_idx = *p_param_2;
            ptm = *p_param_3;

            if (pvr_get_store_info_by_idx(record_idx, storeinfodata, ptm) == FALSE) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_NV_URI: {
            if (RET_SUCCESS != pvr_get_nv_uri(pvr_handle, (nasc_uri_item *)p_param_2)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_NV_URI_COUNT: {
            if (RET_SUCCESS != pvr_get_nv_uri_count(pvr_handle, (UINT32)p_param_2, (UINT32 *)p_param_3)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_NV_URI_SET: {
            if (RET_SUCCESS != pvr_get_nv_uri_sets((UINT32)pvr_get_index(pvr_handle), (UINT32)p_param_1, (UINT32)p_param_2, (nasc_uri_item *)p_param_3)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_NV_URI_EXT: {
            if (RET_SUCCESS != pvr_get_nv_uri_ext(pvr_handle, (nasc_uri_item_ext *)p_param_2)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_NV_URI_COUNT_EXT: {
            if (RET_SUCCESS != pvr_get_nv_uri_count_ext(pvr_handle, (UINT32)p_param_2, (UINT32 *)p_param_3)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_NV_URI_SET_EXT: {
            if (RET_SUCCESS != pvr_get_nv_uri_sets_ext((UINT32)pvr_get_index(pvr_handle), (UINT32)p_param_1, (UINT32)p_param_2, (nasc_uri_item_ext *)p_param_3)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_NV_CREDENTIAL_DATA: {
            if (RET_SUCCESS != _pvr_mgr_get_cridential_data(pvr_handle, (char *)p_param_1, (void *)p_param_2, (UINT32)p_param_3)) {
                ret =  AUI_RTN_FAIL;;
            }
        }
        break;

        case AUI_PVR_BLOCK_SIZE:
            *p_param_1 = pvr_get_block_size();
            break;

        case AUI_PVR_REC_ITEM_INFO:
            pvr_info = (aui_pvr_rec_item_info *)p_param_2;
            index = *p_param_1;

            if (pvr_info == NULL) {
                ret = AUI_RTN_FAIL;
                break;
            }

            memset(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index, &rec_item_info);
            pvr_info->rec_mode = rec_item_info.rec_mode;
            pvr_info->rec_type = rec_item_info.rec_type;
            pvr_info->prog_number = rec_item_info.prog_number;
            pvr_info->av_type = 1 - rec_item_info.channel_type;
            pvr_info->video_type = aui_pvr_get_vedio_type(rec_item_info.h264_flag);
            pvr_info->lock_flag = rec_item_info.lock_flag;
            strncpy(pvr_info->service_name, (char *)rec_item_info.txti, AUI_PVR_SERVICE_NAME_LEN_MAX - 1);
            strncpy(pvr_info->event_name, (char *)rec_item_info.event_txti, AUI_PVR_EVENT_NAME_LEN_MAX - 1);
            pvr_info->scrambled_rec = rec_item_info.is_scrambled;
            pvr_info->rec_special_mode = rec_item_info.rec_special_mode;
            pvr_info->index = rec_item_info.index;
            pvr_info->start_time.tm_hour = rec_item_info.tm.hour;
            pvr_info->start_time.tm_mday = rec_item_info.tm.day;
            pvr_info->start_time.tm_min = rec_item_info.tm.min;
            pvr_info->start_time.tm_mon = rec_item_info.tm.month;
            pvr_info->start_time.tm_sec = rec_item_info.tm.sec;
            pvr_info->start_time.tm_wday = rec_item_info.tm.weekday;
            pvr_info->start_time.tm_year = rec_item_info.tm.year;
            pvr_info->start_time.tm_isdst = rec_item_info.tm.mjd;
            pvr_info->duration = rec_item_info.duration;
            pvr_info->size_in_KB = rec_item_info.size;
            pvr_info->is_recording = rec_item_info.is_recording;
            break;

        default:
            break;
    }

    return ret;
}

/**
*    @brief         PVR set command
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pvr_handle     PVR handler
*    @param[in]     cmd  PVR command
*    @param[in]     param_1 1st parameter of PVR command
*    @param[in]     param_2 2nd parameter of PVR command
*    @param[in]     param_3 3rd parameter of PVR command
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_set(aui_hdl handle, aui_pvr_cmd cmd,
                         unsigned int param_1, unsigned int param_2, unsigned int param_3)
{
    PVR_HANDLE pvr_handle = (PVR_HANDLE)handle;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned int i = 0;
    int flag = 0;
    unsigned short pid_list[32] = {0};
    unsigned short *pid_input_list = NULL;
    AUI_PVR_PID_TYPE *pid_type = NULL;
    UINT8 rec_special_mode = 0;
    struct list_info rec_item_info;
    unsigned short  index = 0;
    int pid_num = 0;
    UINT32 ptm = 0;
    unsigned char lock_flag = 0;
    unsigned int valid_time = 0;
    unsigned short new_pid = 0;
    struct store_info_header *sheader = NULL;
    struct store_info_data_single *storeinfodata = NULL;

    switch (cmd) {
        case AUI_PVR_REC_STATES:
            break;

        case AUI_PVR_REC_DMX_ID:
            break;

        case AUI_PVR_REC_TIME_S:
            break;

        case AUI_PVR_REC_MODE_T:
            break;

        case AUI_PVR_REC_SIZE_KB:
            break;

        case AUI_PVR_REC_CAPBILITY:
            ret = pvr_set_tms_size(param_1);
            break;

        case AUI_PVR_PLAY_STATES:
            break;

        case AUI_PVR_PLAY_SPEED:
            break;

        case AUI_PVR_PLAY_DIRECTION:
            break;

        case AUI_PVR_PLAY_TIME_S:
            break;

        case AUI_PVR_PLAY_DMX_ID:
            break;

        case AUI_PVR_PLAY_PATH:
            break;

        case AUI_PVR_PLAY_INDEX:
            break;

        case AUI_PVR_TS_ROUTE:
            break;

        case AUI_PVR_DEBUG_MODE:
            break;

        case AUI_PVR_ITEM_INFO:
            break;

        case AUI_PVR_ITEM_KEY_INFO:
            break;

        case AUI_PVR_ITEM_FINGER_INFO:
            break;

        case AUI_PVR_ITEM_RATING_INFO:
            break;

        case AUI_PVR_ITEM_GM_INFO:
            break;

        case AUI_PVR_REC_MAX_TMS_LENGTH:
            if (pvr_r_get_rec_special_mode(pvr_handle, &rec_special_mode) == RET_SUCCESS) {
                ret = pvr_r_set_tms_max_time(param_1, rec_special_mode);
            }

            break;

        case AUI_PVR_AUDIO_PID_CHANGE:
            if (pvr_handle == 0) {
                return AUI_RTN_SUCCESS;
            }

            //fix  Bug #72072  get pid_type from rec_item_info and not need pass param_2
            new_pid = param_1;
            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);

            for (i = 0; i < rec_item_info.pid_info.audio_count; i++) {
                if (param_1 == (rec_item_info.pid_info.audio_pid[i] & 0x1fff)) {
                    new_pid = rec_item_info.pid_info.audio_pid[i];
                    break;
                }
            }

            //av_get_audio_pid(param_2-AUI_PVR_AUDIO_MPEG1,param_1,&new_pid);
            pvr_p_switch_audio_pid(pvr_handle, new_pid);
            break;

        case AUI_PVR_AUDIO_CHANNEL_CHANGE:
            aui_pvr_return_if_fail(pvr_handle != 0);
            pvr_p_switch_audio_channel(pvr_handle, param_1);
            break;

        case AUI_PVR_PID_CHANGE://as now only support AVC and audio
            aui_pvr_return_if_fail(pvr_handle != 0);
            index = pvr_get_index(pvr_handle);

            if (param_1 == 0 || param_2 == 0 || param_3 == 0) {
                return AUI_RTN_FAIL;
            }

            if (param_1 > 3) {
                return AUI_RTN_FAIL;
            }

            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pid_input_list = (unsigned short *)param_2;
            pid_type = (AUI_PVR_PID_TYPE *)param_3;

            switch (pid_type[0]) {
                case AUI_PVR_VIDEO_MPEG:
                    pid_list[i] = pid_input_list[i];
                    pid_num++;
                    break;

                case AUI_PVR_VIDEO_AVC:
                    pid_list[i] = pid_input_list[i] | 0x2000;

                    if (TRUE != pvr_r_change_pid(pvr_handle, 1, &pid_list[i])) {
                        return AUI_RTN_FAIL;
                    }

                    pid_num++;
                    break;

                default:
                    return AUI_RTN_FAIL;
            }

            for (i = 1; i < param_1; i++) {
                switch (pid_type[i]) {
                    case AUI_PVR_AUDIO_MPEG1:
                    case AUI_PVR_AUDIO_MPEG2:       ///< MPEG II
                    case AUI_PVR_AUDIO_MPEG_AAC:
                    case AUI_PVR_AUDIO_AC3:         ///< AC-3
                    case AUI_PVR_AUDIO_DTS:     ///<DTS audio for DVD-Video
                    case AUI_PVR_AUDIO_PPCM:        ///<Packet PCM for DVD-Audio
                    case AUI_PVR_AUDIO_LPCM_V:      ///<Linear PCM audio for DVD-Video
                    case AUI_PVR_AUDIO_LPCM_A:      ///<Linear PCM audio for DVD-Audio
                    case AUI_PVR_AUDIO_PCM:         ///<PCM audio
                    case AUI_PVR_AUDIO_BYE1:        ///<BYE1 audio
                    case AUI_PVR_AUDIO_RA8:     ///<Real audio 8
                    case AUI_PVR_AUDIO_MP3:         ///<MP3 audio
                    case AUI_PVR_AUDIO_INVALID:
                    case AUI_PVR_AUDIO_MPEG_ADTS_AAC:
                    case AUI_PVR_AUDIO_OGG:
                    case AUI_PVR_AUDIO_EC3:
                    case AUI_PVR_AUDIO_MP3_L3:
                    case AUI_PVR_AUDIO_RAW_PCM:
                    case AUI_PVR_AUDIO_MP3_2:
                        av_get_audio_pid(pid_type[i] - AUI_PVR_AUDIO_MPEG1, pid_input_list[i], &pid_list[i]);
                        rec_item_info.multi_audio_pid[rec_item_info.pid_info.audio_count] = pid_list[i];
                        rec_item_info.pid_info.audio_count++;
                        rec_item_info.audio_count ++;
                        rec_item_info.pid_info.total_num ++;
                        rec_item_info.record_pids[rec_item_info.record_pid_num] = pid_list[i];
                        rec_item_info.record_pid_num ++;
                        pid_num++;
                        flag |= 1;
                        break;

                    default:
                        break;
                }

                if (pid_num > 3) {
                    break;
                }
            }

            if ((flag) && (TRUE != pvr_r_change_pid(pvr_handle, pid_num, pid_list))) {
                return AUI_RTN_FAIL;
            }

            pvr_set_rl_info(index , &rec_item_info);
            pvr_save_rl(index);
            break;

        case AUI_PVR_PID_ADD://not support video add , when driver add it ,check again
            aui_pvr_return_if_fail(pvr_handle != 0);

            if (param_1 == 0 && param_2 == 0 && param_3 == 0) {
                return AUI_RTN_FAIL;
            }

            index = pvr_get_index(pvr_handle);
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index , &rec_item_info);
            pid_input_list = (unsigned short *)param_2;
            pid_type = (AUI_PVR_PID_TYPE *)param_3;

            for (i = 0; i < param_1; i++) {
                switch (pid_type[i]) {
                    case AUI_PVR_VIDEO_MPEG:
                    case AUI_PVR_VIDEO_AVC:
                        break;

                    case AUI_PVR_AUDIO_MPEG1:
                    case AUI_PVR_AUDIO_MPEG2:       ///< MPEG II
                    case AUI_PVR_AUDIO_MPEG_AAC:
                    case AUI_PVR_AUDIO_AC3:         ///< AC-3
                    case AUI_PVR_AUDIO_DTS:     ///<DTS audio for DVD-Video
                    case AUI_PVR_AUDIO_PPCM:        ///<Packet PCM for DVD-Audio
                    case AUI_PVR_AUDIO_LPCM_V:      ///<Linear PCM audio for DVD-Video
                    case AUI_PVR_AUDIO_LPCM_A:      ///<Linear PCM audio for DVD-Audio
                    case AUI_PVR_AUDIO_PCM:         ///<PCM audio
                    case AUI_PVR_AUDIO_BYE1:        ///<BYE1 audio
                    case AUI_PVR_AUDIO_RA8:     ///<Real audio 8
                    case AUI_PVR_AUDIO_MP3:         ///<MP3 audio
                    case AUI_PVR_AUDIO_INVALID:
                    case AUI_PVR_AUDIO_MPEG_ADTS_AAC:
                    case AUI_PVR_AUDIO_OGG:
                    case AUI_PVR_AUDIO_EC3:
                    case AUI_PVR_AUDIO_MP3_L3:
                    case AUI_PVR_AUDIO_RAW_PCM:
                    case AUI_PVR_AUDIO_MP3_2:
                        av_get_audio_pid(pid_type[i] - AUI_PVR_AUDIO_MPEG1, pid_input_list[i], &pid_list[i]);
                        AUI_DBG(">>>>>>>>>>>>>>>>>>>>>>>> pid_list[%d] : %d\n", i, pid_list[i]);

                        if (TRUE != pvr_r_add_pid(pvr_handle, 1, &pid_list[i])) {
                            return AUI_RTN_FAIL;
                        }

                        rec_item_info.multi_audio_pid[rec_item_info.pid_info.audio_count] = pid_list[i];
                        rec_item_info.pid_info.audio_count++;
                        rec_item_info.audio_count ++;
                        rec_item_info.pid_info.total_num ++;
                        rec_item_info.record_pids[rec_item_info.record_pid_num] = pid_list[i];
                        rec_item_info.record_pid_num ++;
                        flag |= 1;
                        break;

                    case AUI_PVR_AUDIO_TTX_PID:
                        if (TRUE != pvr_r_add_pid(pvr_handle, 1, &pid_input_list[i])) {
                            return AUI_RTN_FAIL;
                        }

                        rec_item_info.pid_info.ttx_pids[rec_item_info.pid_info.ttx_pid_count] = pid_input_list[i];
                        rec_item_info.pid_info.ttx_pid_count ++;
                        rec_item_info.pid_info.total_num ++;
                        rec_item_info.record_pids[rec_item_info.record_pid_num] = pid_input_list[i];
                        rec_item_info.record_pid_num ++;
                        flag |= 1;
                        break;

                    case AUI_PVR_AUDIO_SUBTITLE_PID:
                        if (TRUE != pvr_r_add_pid(pvr_handle, 1, &pid_input_list[i])) {
                            return AUI_RTN_FAIL;
                        }

                        rec_item_info.pid_info.subt_pids[rec_item_info.pid_info.subt_pid_count] = pid_input_list[i];
                        rec_item_info.pid_info.subt_pid_count++;
                        rec_item_info.pid_info.total_num ++;
                        rec_item_info.record_pids[rec_item_info.record_pid_num] = pid_input_list[i];
                        rec_item_info.record_pid_num ++;
                        flag |= 1;
                        break;

                    default:
                        break;
                }
            }

            if (1 == flag) {
                pvr_set_rl_info(index , &rec_item_info);
                pvr_save_rl(index);
            }

            break;

        case AUI_PVR_FREE_SIZE_LOW_THRESHOLD:
            pvr_set_free_size_low_threshold(param_1);
            break;

        case AUI_PVR_DELETE_RECORD_BY_INDEX:
            index = param_1;
            MEMSET(&rec_item_info, 0, sizeof(struct list_info));
            ret = pvr_get_rl_info_by_pos(index , &rec_item_info);

            if (ret != RET_SUCCESS) {
                ret = AUI_RTN_FAIL;
                break;
            }

            //printf("audio_pid:%d\n",rec_item_info.pid_info.audio_pid[0]);
            rec_item_info.del_flag = 1;
            pvr_set_rl_info(index , &rec_item_info);
            pvr_save_rl(index);
            break;

        case AUI_PVR_SET_LOCK_FLAG:
            index = param_1;
            lock_flag = param_2;
            memset(&rec_item_info, 0, sizeof(struct list_info));
            pvr_get_rl_info(index, &rec_item_info);
            rec_item_info.lock_flag = lock_flag;
            pvr_set_rl_info(index , &rec_item_info);
            pvr_save_rl(index);
            break;

        case AUI_PVR_INFO_HEADER:
            sheader = (struct store_info_header *)param_1;
            aui_pvr_return_if_fail(pvr_handle != 0);

            if (pvr_save_info_header(pvr_handle, sheader) == FALSE) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_INFO_DATA:
            storeinfodata = (struct store_info_data_single *)param_1;
            ptm = param_2;
            aui_pvr_return_if_fail(pvr_handle != 0);

            if (pvr_save_store_info(pvr_handle, storeinfodata, ptm) == FALSE) {
                ret = AUI_RTN_FAIL;
            }

            break;

        case AUI_PVR_BLOCK_MODE:
            pvr_set_block_mode((UINT8)param_1);
            ret = AUI_RTN_SUCCESS;
            break;

        case AUI_PVR_BLOCK_SIZE:
            ret = pvr_set_block_size(param_1);
            break;

        case AUI_PVR_NV_URI:
            aui_pvr_return_if_fail(pvr_handle != 0);

            if (RET_SUCCESS != pvr_set_nv_uri(pvr_handle, (nasc_uri_item *)param_2)) {
                ret =  AUI_RTN_FAIL;;
            }

            break;

        case AUI_PVR_NV_URI_EXT:
            aui_pvr_return_if_fail(pvr_handle != 0);

            if (RET_SUCCESS != pvr_set_nv_uri_ext(pvr_handle, (nasc_uri_item_ext *)param_2)) {
                ret =  AUI_RTN_FAIL;;
            }

            break;

        case AUI_PVR_NV_CREDENTIAL_DATA:
            aui_pvr_return_if_fail(pvr_handle != 0);

            if (RET_SUCCESS != _pvr_mgr_set_cridential_data(pvr_handle, (char *)param_1, (void *)param_2, (UINT32)param_3)) {
                ret =  AUI_RTN_FAIL;;
            }

            break;

        case AUI_PVR_REC_TMS_TO_REC:
            if (0 == param_1) {
                ret = AUI_RTN_FAIL;
                break;
            }

            if (FALSE == pvr_check_tms2rec()) {
                ret = AUI_RTN_FAIL;
                break;
            }

            valid_time = pvr_r_trans(pvr_handle);

            if (0 == valid_time) {
                ret = AUI_RTN_FAIL;
                break;
            }

            *(UINT32 *)param_1 = valid_time;
            ret = AUI_RTN_SUCCESS;
            break;

        default:
            break;
    }

    return ret;
}
AUI_RTN_CODE aui_pvr_rename_rec_item(unsigned short index, char *new_name, int len)
{
    struct list_info pvr_info;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (new_name == NULL || len == 0) {
        return AUI_RTN_FAIL;
    }

    memset(&pvr_info, 0, sizeof(pvr_info));
    pvr_get_rl_info(index, &pvr_info);
    strncpy((char *)pvr_info.txti, new_name, sizeof(pvr_info.txti) - 1);
    pvr_set_rl_info(index, &pvr_info);
    pvr_save_rl(index);

    return ret;
}

AUI_RTN_CODE aui_pvr_get_parition_info(char *mount_name, aui_pvr_partition_info *pvr_partition_info)
{
    struct dvr_hdd_info partition_info;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (mount_name == NULL || pvr_partition_info == NULL) {
        return AUI_RTN_FAIL;
    }

    memset(&partition_info, 0, sizeof(partition_info));

    if (AUI_RTN_SUCCESS != pvr_mgr_ioctl(0, PVR_MGRIO_PARTITION_GETINFO, (UINT32)&partition_info, (UINT32)mount_name)) {
        return AUI_RTN_FAIL;
    }

    strncpy(pvr_partition_info->mount_name, partition_info.mount_name, AUI_PVR_MOUNT_NAME_LEN_MAX - 1);
    /* PVR library, 1 - FAT, 2 - NTFS */
    pvr_partition_info->type = partition_info.type;
    pvr_partition_info->disk_usage = partition_info.disk_usage;
    pvr_partition_info->init_list = partition_info.init_list;
    pvr_partition_info->check_speed = partition_info.check_speed;
    pvr_partition_info->total_size_in_KB = partition_info.total_size;
    pvr_partition_info->free_size_in_KB = partition_info.free_size;
    pvr_partition_info->rec_free_size_in_KB = partition_info.rec_size;
    pvr_partition_info->tms_size_in_KB = partition_info.tms_size;
    pvr_partition_info->status = partition_info.status;
    return ret;
}
/**
*    @brief         register callback function
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pv_hdl_key  PVR handler
*    @param[in]     fn_callback callback function
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/

void aui_pvr_timer_callback(struct PVR_DTM *tm)
{
    time_t timer;
    struct tm *local_time;

    timer = time(NULL);
    local_time = localtime(&timer);

    tm->year = local_time->tm_year + 1900;
    tm->month = local_time->tm_mon;
    tm->day = local_time->tm_mday;
    tm->weekday = local_time->tm_wday;
    tm->hour = local_time->tm_hour;
    tm->min = local_time->tm_min;
    tm->sec = local_time->tm_sec;
}

AUI_RTN_CODE aui_pvr_callback_register(aui_hdl pv_hdl_key, fn_aui_pvr_callback fn_callback)
{
    int i = 0;
    PVR_HANDLE p_handle = (PVR_HANDLE)pv_hdl_key;

    if (0 == p_handle && NULL == fn_callback) {
        return AUI_RTN_FAIL;
    }

    for (i = 0; i < s_max_pvr_record_number; i ++) {
        if (p_st_pvr_handle[i].pvr_handler == p_handle) {
            p_st_pvr_handle[i].fn_callback = fn_callback;
            return AUI_RTN_SUCCESS;
        }
    }

    for (i = 0; i < s_max_pvr_record_number; i++) {
        if (0 == p_st_pvr_handle[i].pvr_handler && NULL == p_st_pvr_handle[i].fn_callback) {
            p_st_pvr_handle[i].pvr_handler = p_handle;
            p_st_pvr_handle[i].fn_callback = fn_callback;
            break;
        }
    }

    return AUI_RTN_SUCCESS;
}


/**
*    @brief         unregister callback function
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pv_hdl_key  PVR handler
*    @param[in]     fn_callback callback function
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_callback_unregister(aui_hdl pv_hdl_key, fn_aui_pvr_callback fn_callback)
{
    int i = 0;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)pv_hdl_key;

    if ((NULL == fn_callback) || (0 == pvr_handle)) {
        return AUI_RTN_FAIL;
    }

    for (i = 0; i < s_max_pvr_record_number; i++) {
        if (0 != p_st_pvr_handle[i].pvr_handler && p_st_pvr_handle[i].pvr_handler == pvr_handle) {
            p_st_pvr_handle[i].pvr_handler = 0;
            p_st_pvr_handle[i].user_data = NULL;

            if (NULL == p_st_pvr_handle[i].fn_callback) {
                aui_rtn(AUI_RTN_CALLBACK_UNEXIST, "Callback not exsit.");
            } else {
                p_st_pvr_handle[i].fn_callback = NULL;
                return AUI_RTN_SUCCESS;
            }
        }
    }

    return AUI_RTN_SUCCESS;
}

static INT8 aui_pvr_evnt_push(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    INT8 ret = 0;
    aui_pvr_event_msg aui_event_in_queue;

    aui_event_in_queue.pvr_handle = handle;
    aui_event_in_queue.msg_type = msg_type;
    aui_event_in_queue.msg_code = msg_code;

    if (event_callback_id != OSAL_INVALID_ID) {
        ret = osal_msgqueue_send(event_callback_id, &aui_event_in_queue, sizeof(aui_event_in_queue), 10);
    }

    return ret;
}

INT8 aui_pvr_evnt_callback(PVR_HANDLE handle, UINT32 msg_type, UINT32 msg_code)
{
    int i = 0;
    aui_hdl aui_handle = (aui_hdl)handle;
    INT8 ret = 0;
    UINT8 special_mode = 0;
    AUI_EVNT_PVR eEvent = 0xff;
    unsigned short int pid_num;
    unsigned int prog_id;
    aui_pvr_crypto_general_param aui_rec_param;
    aui_pvr_crypto_general_param aui_play_param;

    AUI_DBG("%s %d msg_type:%ld\r\n", __FUNCTION__, __LINE__, msg_type);

    switch (msg_type) {
        case PVR_MSG_REC_START_OP_STARTDMX: {
            pvr_crypto_general_param *rec_param = (pvr_crypto_general_param *)msg_code;

            memset(&aui_rec_param, 0x0, sizeof(aui_pvr_crypto_general_param));

            if (pvr_r_get_rec_special_mode(handle, &special_mode) == RET_SUCCESS) {
                if ((special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) || special_mode == RSM_BC_MULTI_RE_ENCRYPTION) {
                    memset(&aui_rec_param, 0x0, sizeof(aui_pvr_crypto_general_param));
                    aui_rec_param.pid_list = rec_param->pid_list;
                    aui_rec_param.pid_num = rec_param->pid_num;
                    aui_rec_param.dmx_id = rec_param->dmx_id;
                    msg_code = (UINT32)&aui_rec_param;
                    eEvent = AUI_EVNT_PVR_MSG_REC_START_OP_STARTDMX;
                    break;
                } else if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                    if (aui_gen_ca_pvr_rec_config(rec_param) != 0) {
                        AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                        return -1;
                    }
                } else {
                    if (aui_cas9_pvr_rec_config(rec_param) != 0) {
                        AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                        return -1;
                    }
                }
            } else {
                AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                return -1;
            }

            AUI_DBG("%s %d out rec config\r\n", __FUNCTION__, __LINE__);
            return 0;
        }

        case PVR_MSG_PLAY_START_OP_STARTDMX: {
            pvr_crypto_general_param *play_param = (pvr_crypto_general_param *)msg_code;

            //special_mode = aui_ca_get_pvr_special_mode();
            if (RET_SUCCESS != pvr_p_get_ply_special_mode(handle, &special_mode)) {
                AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                return -1;
            }

            if ((special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) || (special_mode == RSM_BC_MULTI_RE_ENCRYPTION)) {
                memset(&aui_play_param, 0x0, sizeof(aui_pvr_crypto_general_param));
                aui_play_param.pid_list = play_param->pid_list;
                aui_play_param.pid_num = play_param->pid_num;
                aui_play_param.dmx_id = play_param->dmx_id;
                msg_code = (UINT32)&aui_play_param;
                eEvent = AUI_EVNT_PVR_MSG_PLAY_START_OP_STARTDMX;
                break;
            } else if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                if (aui_gen_ca_pvr_playback_config(play_param, 0)) {
                    AUI_ERR("%s %d play config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                    return -1;
                }
            } else {
                if (aui_cas9_pvr_playback_config(play_param, 0)) {
                    AUI_ERR("%s %d play config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                    return -1;
                }
            }

            return 0;
        }

        case PVR_MSG_PLAY_STOP_OP_STOPDMX: {
            pvr_crypto_general_param *play_param = (pvr_crypto_general_param *)msg_code;

            //special_mode = aui_ca_get_pvr_special_mode();
            if (RET_SUCCESS != pvr_p_get_ply_special_mode(handle, &special_mode)) {
                AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                return -1;
            }

            if ((special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) || special_mode == RSM_BC_MULTI_RE_ENCRYPTION) {
                memset(&aui_play_param, 0x0, sizeof(aui_pvr_crypto_general_param));
                aui_play_param.pid_list = play_param->pid_list;
                aui_play_param.pid_num = play_param->pid_num;
                aui_play_param.dmx_id = play_param->dmx_id;
                msg_code = (UINT32)&aui_play_param;
                eEvent = AUI_EVNT_PVR_MSG_PLAY_STOP_OP_STOPDMX;
                break;
            } else if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                if (aui_gen_ca_pvr_playback_stop(play_param)) {
                    AUI_ERR("%s %d play stop errr!!!!! \r\n", __FUNCTION__, __LINE__);
                    return -1;
                }
            } else {
                if (aui_cas9_pvr_playback_stop(play_param)) {
                    AUI_ERR("%s %d play stop errr!!!!! \r\n", __FUNCTION__, __LINE__);
                    return -1;
                }
            }

            return 0;
        }

        case PVR_MSG_REC_STOP_OP_STOPDMX: {
            pvr_crypto_general_param *rec_param = (pvr_crypto_general_param *)msg_code;

            if (pvr_r_get_rec_special_mode(handle, &special_mode) == RET_SUCCESS) {
                if ((special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) || special_mode == RSM_BC_MULTI_RE_ENCRYPTION) {
                    memset(&aui_rec_param, 0x0, sizeof(aui_pvr_crypto_general_param));
                    aui_rec_param.pid_list = rec_param->pid_list;
                    aui_rec_param.pid_num = rec_param->pid_num;
                    aui_rec_param.dmx_id = rec_param->dmx_id;
                    msg_code = (UINT32)&aui_rec_param;
                    eEvent = AUI_EVNT_PVR_MSG_REC_STOP_OP_STOPDMX;
                    break;
                } else if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                    if (aui_gen_ca_pvr_rec_stop(rec_param)) {
                        AUI_ERR("%s %d rec stop errr!!!!! \r\n", __FUNCTION__, __LINE__);
                        return -1;
                    }
                } else {
                    if (aui_cas9_pvr_rec_stop(rec_param)) {
                        AUI_ERR("%s %d rec stop errr!!!!! \r\n", __FUNCTION__, __LINE__);
                        return -1;
                    }
                }
            } else {
                AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                return -1;
            }

            return 0;
        }

        case PVR_MSG_REC_GET_KREC: {
            special_mode = aui_ca_get_pvr_special_mode();

            if (special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) {
                return (INT8)aui_c200a_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
            } else if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                return (INT8)aui_gen_ca_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
            } else {
                return (INT8)aui_cas9_pvr_generate_keys((pvr_crypto_key_param *)msg_code);
            }
        }

        case PVR_MSG_CRYPTO_DATA: {
            special_mode = aui_ca_get_pvr_special_mode();

            if (special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) {
                return (INT8)aui_c200a_crypto_data((pvr_crypto_data_param *)msg_code);
            } else if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                return (INT8)aui_gen_ca_crypto_data((pvr_crypto_data_param *)msg_code);
            } else {
                return (INT8)aui_cas9_crypto_data((pvr_crypto_data_param *)msg_code);
            }
        }

        case PVR_MSG_REC_SET_REENCRYPT_PIDS: {
            pvr_crypto_pids_param *pids_param = (pvr_crypto_pids_param *)msg_code;
            aui_pvr_crypto_pids_param *aui_pids_param = NULL;

            if (pvr_r_get_rec_special_mode(handle, &special_mode) == RET_SUCCESS) {
                if ((special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) || special_mode == RSM_BC_MULTI_RE_ENCRYPTION) {
                    aui_pids_param = (aui_pvr_crypto_pids_param *)pids_param;
                    msg_code = (UINT32)aui_pids_param;
                    eEvent = AUI_EVNT_PVR_MSG_REC_SET_REENCRYPT_PIDS;
                    break;
                } else {
                    pid_num = pids_param->pid_num;
                    prog_id = pvr_r_get_channel_id(handle);
                    pid_num = aui_ca_get_dsc_pid_multi_des(pids_param->pid_list, pid_num, prog_id);

                    if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                        pids_param->pid_num = aui_gen_ca_pvr_check_reencrypt_pids(pids_param->pid_list, pid_num);
                    } else {
                        pids_param->pid_num = aui_cas9_pvr_check_reencrypt_pids(pids_param->pid_list, pid_num);
                    }

                    return 0;
                }
            } else {
                AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                return -1;
            }
        }

        case PVR_MSG_PLAY_SET_REENCRYPT_PIDS: {
            pvr_crypto_pids_param *pids_param = (pvr_crypto_pids_param *)msg_code;
            aui_pvr_crypto_pids_param *aui_pids_param = NULL;

            //special_mode = aui_ca_get_pvr_special_mode();
            if (RET_SUCCESS != pvr_p_get_ply_special_mode(handle, &special_mode)) {
                AUI_ERR("%s %d rec config errr!!!!! \r\n", __FUNCTION__, __LINE__);
                return -1;
            }

            if ((special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) || special_mode == RSM_BC_MULTI_RE_ENCRYPTION) {
                aui_pids_param = (aui_pvr_crypto_pids_param *)pids_param;
                msg_code = (UINT32)aui_pids_param;
                eEvent = AUI_EVNT_PVR_MSG_PLAY_SET_REENCRYPT_PIDS;
                break;
            } else {
                if (special_mode == RSM_GEN_CA_MULTI_RE_ENCRYPTION) {
                    pids_param->pid_num = aui_gen_ca_pvr_set_reencrypt_pids(pids_param->pid_info,
                                                                            pids_param->pid_list, pids_param->pid_num);
                } else {
                    pids_param->pid_num = aui_cas9_pvr_set_reencrypt_pids(pids_param->pid_info,
                                                                          pids_param->pid_list, pids_param->pid_num);
                }

                return 0;
            }
        }

        case PVR_MSG_CAL_CHUNK_HASH: {
            pvr_crypto_data_param *cp = (pvr_crypto_data_param *)msg_code;
            return calculate_hmac((unsigned char *)cp->input, (unsigned long)cp->data_len, (unsigned char *)cp->output, (unsigned char *)cp->key_ptr);
        }

        case PVR_MSG_BLOCK_MODE_DECRYPT:
            eEvent = AUI_EVNT_PVR_MSG_BLOCK_MODE_DECRYPT;
            special_mode = aui_ca_get_pvr_special_mode();
            break;

        case PVR_END_DATAEND:   // the player playback to the file EOF
            eEvent = AUI_EVNT_PVR_END_DATAEND;
            aui_pvr_evnt_push(handle, eEvent, msg_code);
            return 0;

        case PVR_END_DISKFULL:  // the HDD is full!
            eEvent = AUI_EVNT_PVR_END_DISKFULL;
            break;

        case PVR_END_TMS:       // in the time shifting mode, the player catch up the recorder
            eEvent = AUI_EVNT_PVR_END_TMS;
            break;

        case PVR_END_REVS:      // in the backward mode, the player gets to the beginning of a file
            eEvent = AUI_EVNT_PVR_END_REVS;
            aui_pvr_evnt_push(handle, eEvent, msg_code);
            return 0;

        case PVR_END_WRITEFAIL: // write failed
            eEvent = AUI_EVNT_PVR_END_WRITEFAIL;
            break;

        case PVR_END_READFAIL:  // read failed
            eEvent = AUI_EVNT_PVR_END_READFAIL;
            break;

        case PVR_TMS_OVERLAP:  // when time-shifting overlapped, notice this msg
            eEvent = AUI_EVNT_PVR_TMS_OVERLAP;
            break;

        case PVR_STATUS_UPDATE: // the status of recorder/player updated, the msg is PREC_INFO.
            eEvent = AUI_EVNT_PVR_STATUS_UPDATE;
            break;

        case PVR_STATUS_FILE_CHG:  // the status of recorder/player file changed
            eEvent = AUI_EVNT_PVR_STATUS_FILE_CHG;
            break;

        case PVR_STATUS_PID_CHG:  // the status of PID changed when play file
            eEvent = AUI_EVNT_PVR_STATUS_PID_CHG;
            break;

        case PVR_SPEED_LOW:     // Speed too low.
            eEvent = AUI_EVNT_PVR_SPEED_LOW;
            break;

        case PVR_STATUS_CHN_CHG: // channel changed when playing for one record.
            eEvent = AUI_EVNT_PVR_STATUS_CHN_CHG;
            break;

    //====== other message type =====
        case PVR_MSG_REC_START:
            aui_pvr_evnt_push(handle, eEvent, msg_code);
            eEvent = AUI_EVNT_PVR_MSG_REC_START;
            break;

        case PVR_MSG_REC_STOP:
            eEvent = AUI_EVNT_PVR_MSG_REC_STOP;
            break;

        case PVR_MSG_PLAY_START:
            aui_pvr_evnt_push(handle, eEvent, msg_code);
            eEvent = AUI_EVNT_PVR_MSG_PLAY_START;
            break;

        case PVR_MSG_PLAY_STOP:
            eEvent = AUI_EVNT_PVR_MSG_PLAY_STOP;
            break;

        case PVR_MSG_UPDATE_KEY:
            eEvent = AUI_EVNT_PVR_MSG_UPDATE_KEY;
            break;

        case PVR_MSG_UPDATE_CW:
            eEvent = AUI_EVNT_PVR_MSG_UPDATE_CW;
            break;

        case PVR_MSG_TMS_CAP_UPDATE:    // timeshift capability update
            eEvent = AUI_EVNT_PVR_MSG_TMS_CAP_UPDATE;
            break;

        case PVR_MSG_REC_START_GET_HANDLE:
            eEvent = AUI_EVNT_PVR_MSG_REC_START_GET_HANDLE;
            break;

        case PVR_MSG_PLAY_START_GET_HANDLE:
            eEvent = AUI_EVNT_PVR_MSG_PLAY_START_GET_HANDLE;
            break;

        case PVR_MSG_PLAY_PTM_UPDATE:
            eEvent = AUI_EVNT_PVR_MSG_PLAY_PTM_UPDATE;
            break;

        case PVR_MSG_PLAY_URI_NOTIFY:
            eEvent = AUI_EVNT_PVR_MSG_PLAY_URI_NOTIFY;
            break;

        case PVR_MSG_PLAY_RL_SHIFT:
            eEvent = AUI_EVNT_PVR_MSG_PLAY_RL_SHIFT;
            break;

        case PVR_MSG_PLAY_RL_RESET: //need adjust to RL A point normal play since the play time become invalid
            eEvent = AUI_EVNT_PVR_MSG_PLAY_RL_RESET;
            break;

        case PVR_MSG_PLAY_RL_INVALID:
            eEvent = AUI_EVNT_PVR_MSG_PLAY_RL_INVALID;
            break;

        case PVR_READ_SPEED_LOW:
            eEvent = AUI_EVNT_PVR_READ_SPEED_LOW;
            break;

        case PVR_WRITE_SPEED_LOW:
            eEvent = AUI_EVNT_PVR_WRITE_SPEED_LOW;
            break;

        case PVR_MSG_REC_STOP_URICHANGE:
            eEvent = AUI_EVNT_PVR_MSG_REC_STOP_URICHANGE;
            break;

        case PVR_MSG_TMS_STOP_URICHANGE:
            eEvent = AUI_EVNT_PVR_MSG_TMS_STOP_URICHANGE;
            break;

        case PVR_MSG_TMS_PAUSE_URICHANGE:
            eEvent = AUI_EVNT_PVR_MSG_TMS_PAUSE_URICHANGE;
            break;

        case PVR_MSG_TMS_RESUME_URICHANGE:
            eEvent = AUI_EVNT_PVR_MSG_TMS_RESUME_URICHANGE;
            break;

        case PVR_MSG_REC_TOO_SHORT:
            eEvent = AUI_EVNT_PVR_MSG_REC_TOO_SHORT;
            break;

        default:
            return ret;
    }

    for (i = 0; i < s_max_pvr_record_number; i++) {
        if (0 == handle || (0 != p_st_pvr_handle[i].pvr_handler && p_st_pvr_handle[i].pvr_handler == handle)) {
            if (p_st_pvr_handle[i].fn_callback != NULL) {
                p_st_pvr_handle[i].fn_callback(aui_handle, eEvent, msg_code, p_st_pvr_handle[i].user_data);
            }
        }
    }

    return ret;
}

/**
*    @brief         set URI information to the record which is being recorded
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     aui_pvr_handler PVR device handler
*    @param[in]     p_uri           address of URI information data
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_set_uri(aui_hdl aui_pvr_handler, const aui_pvr_uri_t *p_uri)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)aui_pvr_handler;
    conax6_uri_item c_item;

    if (0 == pvr_handle || NULL == p_uri) {
        return AUI_RTN_FAIL;
    }

    MEMSET(&c_item, 0, sizeof(c_item));
    c_item.dt.year = p_uri->dt.tm_year;
    c_item.dt.month = p_uri->dt.tm_mon;
    c_item.dt.day = p_uri->dt.tm_mday;
    c_item.dt.hour = p_uri->dt.tm_hour;
    c_item.dt.minute = p_uri->dt.tm_min;
    c_item.dt.sec = p_uri->dt.tm_sec;
    MEMCPY(&c_item.ptm, &(p_uri->ptm), sizeof(aui_pvr_uri_t) - sizeof(struct tm));
    AUI_DBG("%s %d pvr_handle:%d p_uri->ptm:%d c_item.ptm:%d emi:%d\r\n", __FUNCTION__, __LINE__,
            (int)pvr_handle, (int)p_uri->ptm, (int)c_item.ptm, (int)c_item.buri_emi);
    pthread_mutex_lock(&s_pvr_mutexID);;

    if (RET_SUCCESS != pvr_set_uri(pvr_handle, &c_item)) {
        AUI_ERR("set failed\r\n");
        ret = AUI_RTN_FAIL;
    }

    pthread_mutex_unlock(&s_pvr_mutexID);
    return ret;
}

/**
*    @brief         get URI information to the record which is being recorded
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     aui_pvr_handler PVR device handler
*    @param[in]     p_uri           address of URI information data
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_get_uri(aui_hdl aui_pvr_handler, aui_pvr_uri_t *p_uri)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)aui_pvr_handler;
    conax6_uri_item c_item;
    RET_CODE ret_code = RET_SUCCESS;

    if (0 == pvr_handle || NULL == p_uri) {
        return AUI_RTN_FAIL;
    }

    MEMSET(&c_item, 0, sizeof(c_item));
    c_item.ptm = p_uri->ptm;
    AUI_DBG("%s %d pvr_handle:%d p_uri->ptm:%d c_item.ptm:%d\r\n", __FUNCTION__, __LINE__,
            (int)pvr_handle, (int)p_uri->ptm, (int)c_item.ptm);
    pthread_mutex_lock(&s_pvr_mutexID);;
    ret_code = pvr_get_uri(pvr_handle, &c_item);

    if (RET_SUCCESS == ret_code) {
        p_uri->dt.tm_year = c_item.dt.year;
        p_uri->dt.tm_mon = c_item.dt.month;
        p_uri->dt.tm_mday = c_item.dt.day;
        p_uri->dt.tm_hour = c_item.dt.hour;
        p_uri->dt.tm_min = c_item.dt.minute;
        p_uri->dt.tm_sec = c_item.dt.sec;
        MEMCPY(&(p_uri->ptm), &c_item.ptm, sizeof(aui_pvr_uri_t) - sizeof(struct tm));
        AUI_DBG("%s %d p_uri->ptm:%d c_item.ptm:%d em:%d len:%d\r\n", __FUNCTION__, __LINE__,
                (int)p_uri->ptm, (int)c_item.ptm, (int)c_item.buri_emi,
                (int)(sizeof(aui_pvr_uri_t) - sizeof(struct tm)));
    } else if (3 == ret_code) {
        AUI_ERR("URI verify failed\n");
        ret = AUI_RTN_URI_VERIFY_FAILED;
    } else {
        AUI_ERR("get failed\n");
        ret = AUI_RTN_FAIL;
    }

    pthread_mutex_unlock(&s_pvr_mutexID);
    return ret;
}

/**
*    @brief         get record index by path name
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     path            path of PVR record
*    @param[out]    p_index         index address of PVR record
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_get_index_by_path(char *path, unsigned int *p_index)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (NULL == path || NULL == p_index) {
        ret = AUI_RTN_FAIL;
    } else {
        if (RET_SUCCESS != pvr_get_rl_idx_by_path(path, (UINT16 *)p_index)) {
            ret = AUI_RTN_FAIL;
        }
    }

    return ret;
}

/**
*    @brief         get count of URI information in the record
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     index           index of PVR record
*    @param[out]    p_cnt           pointer to store count of URI information data
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_get_uri_cnt(unsigned int index, unsigned int *p_cnt)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    RET_CODE ret_code = RET_SUCCESS;

    pthread_mutex_lock(&s_pvr_mutexID);;
    ret_code = pvr_get_uri_cnt(index, (UINT32 *)p_cnt);

    if (RET_FAILURE == ret_code) {
        AUI_ERR("get failed\n");
        ret = AUI_RTN_FAIL;
    } else if (3 == ret_code) {
        AUI_ERR("URI verify failed\n");
        ret = AUI_RTN_URI_VERIFY_FAILED;
    }

    if (ret != RET_SUCCESS) {
        AUI_ERR("get failed\r\n");
    }

    pthread_mutex_unlock(&s_pvr_mutexID);
    return ret;
}

/**
*    @brief         get a set of URI information in the record
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     index           index of PVR record
*    @param[in]     base            URI base number
*    @param[out]    cnt             count of URI information data, the count is how many URI data it will return
*    @param[out]    p_uri           address of buffer for storing of URI information data
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_get_uri_sets(unsigned int index, unsigned int base,
                                  unsigned int cnt, aui_pvr_uri_t *p_uri)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned int i = 0;
    RET_CODE ret_code = RET_SUCCESS;

    if (cnt < 1 || NULL == p_uri) {
        return AUI_RTN_FAIL;
    }

    conax6_uri_item *p_item = (conax6_uri_item *)malloc(sizeof(conax6_uri_item) * cnt);

    if (NULL == p_item) {
        return AUI_RTN_FAIL;
    }

    MEMSET(p_item, 0, sizeof(conax6_uri_item) * cnt);
    pthread_mutex_lock(&s_pvr_mutexID);
    ret_code = pvr_get_uri_sets(index, base, cnt, p_item);

    if (RET_FAILURE == ret_code) {
        AUI_ERR("get failed\n");
        pthread_mutex_unlock(&s_pvr_mutexID);
        free(p_item);
        return AUI_RTN_FAIL;
    } else if (3 == ret_code) {
        AUI_ERR("URI verify failed\n");
        pthread_mutex_unlock(&s_pvr_mutexID);
        free(p_item);
        return AUI_RTN_URI_VERIFY_FAILED;
    }

    pthread_mutex_unlock(&s_pvr_mutexID);

    for (i = 0; i < cnt; i ++) {
        p_uri[i].dt.tm_year = p_item[i].dt.year;
        p_uri[i].dt.tm_mon = p_item[i].dt.month;
        p_uri[i].dt.tm_mday = p_item[i].dt.day;
        p_uri[i].dt.tm_hour = p_item[i].dt.hour;
        p_uri[i].dt.tm_min = p_item[i].dt.minute;
        p_uri[i].dt.tm_sec = p_item[i].dt.sec;
        MEMCPY(&(p_uri[i].ptm), &p_item[i].ptm, sizeof(aui_pvr_uri_t) - sizeof(struct tm));
    }

    free(p_item);
    return ret;
}

/**
*    @brief         set fingerprint information in the record
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     aui_pvr_handler PVR device handler
*    @param[in]     p_fingerprint   address of fingerprint information
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_set_finger_print(aui_hdl aui_pvr_handler, aui_pvr_finger_info *p_fingerprint)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    PVR_HANDLE pvr_handle = (PVR_HANDLE)aui_pvr_handler;
    pvr_finger_info finger_info;

    if (0 == pvr_handle || NULL == p_fingerprint) {
        return AUI_RTN_FAIL;
    }

    MEMSET(&finger_info, 0, sizeof(pvr_finger_info));
    finger_info.ptm         = p_fingerprint->ptm;
    finger_info.fp_duration = p_fingerprint->fp_duration;
    finger_info.fp_height   = p_fingerprint->fp_height;
    MEMCPY(finger_info.fp_id, p_fingerprint->fp_id, 64);
    finger_info.fp_pos_x    = p_fingerprint->fp_pos_x;
    finger_info.fp_pos_y    = p_fingerprint->fp_pos_y;
    finger_info.fp_priority = p_fingerprint->fp_priority;
    pthread_mutex_lock(&s_pvr_mutexID);

    if (0 != pvr_set_finger_info(pvr_handle, &finger_info)) {
        ret = AUI_RTN_FAIL;
    }

    pthread_mutex_unlock(&s_pvr_mutexID);
    return ret;
}

/**
*    @brief         get fingerprint information in the record
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     aui_pvr_handler PVR device handler
*    @param[out]    p_fingerprint   address of fingerprint information
*    @return        successful -- AUI_RTN_SUCCESS, failed -- non-zero error code
*    @note
*
*/
AUI_RTN_CODE aui_pvr_get_finger_print(aui_hdl aui_pvr_handler, aui_pvr_finger_info *p_fingerprint)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    PVR_HANDLE pvr_handle = (PVR_HANDLE)aui_pvr_handler;
    pvr_finger_info finger_info;

    if (0 == pvr_handle || NULL == p_fingerprint) {
        return AUI_RTN_FAIL;
    }

    MEMSET(&finger_info, 0, sizeof(pvr_finger_info));
    finger_info.ptm = p_fingerprint ->ptm;
    AUI_DBG("%s %d p_fingerprint ->ptm:%d finger_info.ptm:%d\r\n", __FUNCTION__, __LINE__,
            (int)p_fingerprint->ptm, (int)finger_info.ptm);
    pthread_mutex_lock(&s_pvr_mutexID);

    if (0 == pvr_get_finger_info(pvr_handle, &finger_info)) {
        p_fingerprint->ptm          = finger_info.ptm;
        p_fingerprint->fp_duration  = finger_info.fp_duration;
        p_fingerprint->fp_height    = finger_info.fp_height;
        MEMCPY(p_fingerprint->fp_id, finger_info.fp_id, 64);
        p_fingerprint->fp_pos_x     = finger_info.fp_pos_x;
        p_fingerprint->fp_pos_y     = finger_info.fp_pos_y;
        p_fingerprint->fp_priority  = finger_info.fp_priority;
    } else {
        ret = AUI_RTN_FAIL;
    }

    pthread_mutex_unlock(&s_pvr_mutexID);
    return ret;
}

AUI_RTN_CODE aui_pvr_set_mat_rating(aui_hdl aui_pvr_handler,
                                    unsigned int play_time,
                                    unsigned int rating)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)aui_pvr_handler;

    if (SUCCESS != pvr_set_mat_rating(pvr_handle, play_time, rating)) {
        return AUI_RTN_FAIL;
    }

    return ret;
}

/**
*    @brief         get maturity rating information by PVR handler.
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     pvr_handler         PVR device handler.
*    @param[in]     play_time           time stamp of maturity,step in ms.
*    @param[in]     p_rating            address of memory to save maturity rating.
*    @return        return AUI_RTN_SUCCESS,if success. Or return others.
*    @note          none.
*
*/
AUI_RTN_CODE aui_pvr_get_mat_rating(aui_hdl aui_pvr_handler,
                                    unsigned int play_time,
                                    unsigned int *p_rating)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    PVR_HANDLE pvr_handle = (PVR_HANDLE)aui_pvr_handler;

    if (p_rating == NULL) {
        return AUI_RTN_FAIL;
    }

    if (SUCCESS != pvr_get_mat_rating(pvr_handle, play_time, (UINT32 *)p_rating)) {
        return AUI_RTN_FAIL;
    }

    return ret;
}

/**
*    @brief         get maturity rating information by index of PVR record.
*    @author        Raynard.wang
*    @date          2013-10-12
*    @param[in]     index               index of PVR record.
*    @param[in]     play_time           time stamp of maturity,step in ms.
*    @param[in]     p_rating            address of memory to save maturity rating.
*    @return        return AUI_RTN_SUCCESS,if success. Or return others.
*    @note          none.
*
*/
AUI_RTN_CODE aui_pvr_get_mat_rating_by_index(unsigned short index,
                                             unsigned int play_time,
                                             unsigned int *p_rating)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (p_rating == NULL) {
        return AUI_RTN_FAIL;
    }

    if (SUCCESS != pvr_get_mat_rating_by_idx(index, play_time, (UINT32 *)p_rating)) {
        return AUI_RTN_FAIL;
    }

    return ret;
}



/* No need to care about this function */
void aui_link_sl()
{
    unsigned char buf[8];
    alisltrng_get_rand_bytes(buf, 1);
}

/* try to get a valid record index */
unsigned short aui_pvr_get_record_idx()
{
    UINT16 rl_cnt = 0;
    UINT16 i = 0;
    UINT16 ret = 0;
    struct list_info rl_info;
    MEMSET(&rl_info, 0, sizeof(rl_info));
    rl_cnt = pvr_get_rl_count();

    if (0 == rl_cnt) {
        AUI_DBG("empty record list\n");
        /* no record list, try to do timeshift playback */
        return AUI_TMS_INDEX;
    }

    for (i = 0; i < rl_cnt; i ++) {
        ret = pvr_get_rl_info_by_pos(i, &rl_info);

        if (RET_SUCCESS == ret) {
            AUI_DBG("get index [%d]\n", rl_info.index);
            return rl_info.index;
        }
    }

    AUI_DBG("empty record list\n");
    /* no record list, try to do timeshift playback */
    return AUI_TMS_INDEX;
}
