/**@file
 *    @brief         AUI av module interface implement
 *    @author        henry.xie
 *    @date            2013-6-27
 *     @version         1.0.0
 *    @note            ali corp. all rights reserved. 2013~2999 copyright (C)
 */
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_tsg.h>
#include <aui_tsi.h>
#include <aui_dis.h>
#include <aui_av.h>
#include <aui_av_injecter.h>

#include <pthread.h>

#include "aui_decv_priv.h"
#include "aui_dmx_priv.h"
#include "aui_dsc_common.h"
#include "aui_av_injecter_priv.h"

AUI_MODULE(AV)

#define AUI_AV_LINUX

/****************************LOCAL MACRO******************************************/
#define MAX_PID_NUM 32

#define AUI_AC3_DES_EXIST                    (1<<13)//0x2000
#define AUI_AAC_DES_EXIST                    (2<<13)//0x4000//LATM_AAC
#define AUI_EAC3_DES_EXIST                    (3<<13)//0x0001//EAC3
#define AUI_ADTS_AAC_DES_EXIST                (4<<13)//0x8000//ADTS_AAC

#define USLEEP_TIME         (100)
#define WRITE_ERROR_MAX  (10000)

#define FAST_PLAYBACK_SPEED_FLAG (10)
/** es split package*/
#ifndef SHARE_MEMORY_LEN
#define SHARE_MEMORY_LEN (256*1024) 
#endif

typedef unsigned long aui_audio_codec_id;
// typedef unsigned long aui_audio_decoder_id;

typedef struct {

    aui_av_audio_info audio_parameters;
    aui_av_video_info video_parameters;
    aui_hdl video_decoder_out;
    aui_hdl audio_decoder_out;
    unsigned char es_have_audio;
    unsigned char es_have_video;

} av_inject_t;

typedef struct {
    /** attribute of av module */
    aui_attrAV attrAV;

    /** trickmode attributes**/
    aui_av_trickmode_params trickmode_param;
    unsigned long ul_last_frm_idx;
    int b_send_vob_packet;

    /* ott encrypted es data information*/
    aui_av_dsc_context en_es_ctx;
    aui_dsc_resource_id audio_dsc_res_id;
    aui_dsc_resource_id video_dsc_res_id;
    aui_hdl ali_pvr_hdl;
    unsigned int mmap_flag;
    unsigned int mmap_addr;
    unsigned int mmap_len;
    
} aui_handleAV, *aui_p_handleAV;

/*open other modules in av module*/
typedef enum  {
    AUI_AV_OPEN_NONE = 0,
    AUI_AV_OPEN_DECV = 1,    
    AUI_AV_OPEN_DECA = 1 << 1,    
    AUI_AV_OPEN_DMX = 1 << 2,
    AUI_AV_OPEN_SND = 1 << 3,//deprecated
    AUI_AV_OPEN_TSI = 1 << 4,
    AUI_AV_OPEN_TSG = 1 << 5,
    AUI_AV_OPEN_DIS_HD = 1 << 6,//deprecated
    AUI_AV_OPEN_DIS_SD = 1 << 7,//deprecated
    AUI_AV_OPEN_DSC = 1 << 8
} av_open_device_t;

static av_inject_t inject_param;
static int aui_av_magic_num = 0;
static unsigned aui_av_open_device = AUI_AV_OPEN_NONE;
static pthread_mutex_t es_blk_mutex  = PTHREAD_MUTEX_INITIALIZER;

/****************************MODULE DRV IMPLEMENT*********************************/
AUI_RTN_CODE aui_av_open(aui_attrAV *pst_attrAV, aui_hdl *ppv_handleAV) {
    aui_handleAV *ptr = NULL;

    if (NULL == pst_attrAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    *ppv_handleAV = (aui_handleAV *) malloc(sizeof(aui_handleAV));

    if (NULL == *ppv_handleAV) {
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }

    ptr = (aui_handleAV *) (*ppv_handleAV);

    MEMSET(ptr, 0, sizeof(aui_handleAV));
    MEMCPY(&(ptr->attrAV), pst_attrAV, sizeof(aui_attrAV));

    ptr->trickmode_param.mode = AUI_AV_TRICK_MODE_NONE;
    ptr->trickmode_param.speed = AUI_PLAYBACK_SPEED_1;
    ptr->trickmode_param.direction = AUI_PLAYBACK_FORWARD;

    AUI_DBG("dmx hdl:%p,decv hdl:%p,deca hdl:%p,snd hdl:%p, tsi:%p, tsg:%p, " \
        "dis_hd:%p, dis_sd:%p\n",
            pst_attrAV->pv_hdl_dmx, pst_attrAV->pv_hdl_decv,
            pst_attrAV->pv_hdl_deca, pst_attrAV->pv_hdl_snd,
            pst_attrAV->pv_hdl_tsi, pst_attrAV->pv_hdl_tsg,
            pst_attrAV->pv_hdl_dis_hd, pst_attrAV->pv_hdl_dis_sd);

    return AUI_RTN_SUCCESS;
}

//used by aui_pvr.c
void av_get_audio_pid(enum aui_deca_stream_type type, unsigned short pid,
        unsigned short *new_pid) {
    unsigned short aud_pid = pid;

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
    default:
        break;
    }

    if (new_pid != NULL) {
        *new_pid = aud_pid;
    }
}

static AUI_RTN_CODE cc_av_act(void *pv_handleAV) 
{
    aui_handleAV *av_handle;
    AUI_RTN_CODE _ret = AUI_RTN_SUCCESS;
    unsigned char have_audio = 0;
    unsigned char have_video = 0;
    unsigned char have_pcr = 0;
    unsigned char have_dmx = 0, have_subtitle = 0;
    unsigned int k = 0, create_stream = 0, enable_stream = 0;
    unsigned short PID_list[MAX_PID_NUM];
    aui_snd_out_mode snd_out_mode;
    enum aui_deca_stream_type type;
    enum aui_snd_data_type spdif_type;
    enum aui_decv_vbv_buf_mode vbv_buf_mode;
    aui_snd_out_type snd_out_type;

    aui_av_inject_settings_t *p_av_setting = NULL;

    aui_hdl pv_hdl_vdec = NULL;
    aui_hdl pv_hdl_deca = NULL;
    aui_hdl pv_hdl_dmx = NULL;
    aui_hdl pv_hdl_snd = NULL;
    av_info *info = NULL;

    av_handle = (aui_handleAV *) pv_handleAV;

    p_av_setting = &(av_handle->attrAV.stream_type);
    info = &av_handle->attrAV.st_av_info;
    have_audio = info->b_audio_enable;
    have_video = info->b_video_enable;

    if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)
        || (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS)) {

        pv_hdl_dmx = av_handle->attrAV.pv_hdl_dmx;
        pv_hdl_vdec = av_handle->attrAV.pv_hdl_decv;
        pv_hdl_deca = av_handle->attrAV.pv_hdl_deca;
        pv_hdl_snd = av_handle->attrAV.pv_hdl_snd;

        type = info->en_audio_stream_type;
        spdif_type = info->en_spdif_type;
        snd_out_type = info->snd_out_type;        
        have_dmx = info->b_dmx_enable;
        have_pcr = info->b_pcr_enable;
        //have_ttx = info->b_ttx_enable;
        have_subtitle = info->b_sub_enable;

        AUI_DBG("vpid:%d,apid:%d,ppid:%d\n", info->ui_video_pid,
                info->ui_audio_pid, info->ui_pcr_pid);

        if (info->b_modify) {
            if (have_audio) {
                aui_deca_stop(pv_hdl_deca, NULL);
            }
            
            if (have_video) {
                aui_decv_stop(pv_hdl_vdec);
            }
            
            if (have_dmx) {
                aui_dmx_stop(pv_hdl_dmx, NULL);
                aui_dmx_set(((aui_hdl) pv_hdl_dmx), AUI_DMX_STREAM_DISABLE, NULL);
            }
        }

        PID_list[0] = PID_list[1] = PID_list[2] = PID_list[3] = PID_list[4] = AUI_INVALID_PID;

        if (have_audio && pv_hdl_deca) {
            //dmx will add pid mask
            PID_list[1] = info->ui_audio_pid;
            aui_deca_set(pv_hdl_deca, AUI_DECA_DEOCDER_TYPE_SET, &type);
            snd_out_mode.snd_data_type = spdif_type;
            snd_out_mode.snd_out_type = snd_out_type;
            
            if (pv_hdl_snd)
                aui_snd_out_data_type_set(pv_hdl_snd, snd_out_mode);
                
            aui_deca_start(pv_hdl_deca, NULL);    
        }
        
        if (have_video && pv_hdl_vdec) {
            PID_list[0] = info->ui_video_pid;
            aui_decv_decode_format_set(pv_hdl_vdec, info->en_video_stream_type);
            aui_decv_start(pv_hdl_vdec);
            
            if (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS) {
                vbv_buf_mode = AUI_DECV_VBV_BUF_BLOCK_FULL_MODE;
                aui_decv_set(pv_hdl_vdec, AUI_DECV_SET_VBV_BUF_MODE, &vbv_buf_mode);
                aui_decv_sync_mode(pv_hdl_vdec, AUI_STC_AVSYNC_AUDIO);
            }
        }
        
        if (have_pcr) {
            PID_list[2] = info->ui_pcr_pid;
        }
        
        //pid_list[3] is for AUDIO_DESC, not support here
        //if (have_ttx) {
        //    PID_list[3] = info->ui_ttx_pid;
        //}
        
        if (have_subtitle) {
            PID_list[4] = info->ui_sub_pid;
        }

        if ((have_dmx) && (pv_hdl_dmx)) {
            /* it's better to implement before av start, 
            because the dmx start only clear buffers for playback */
            aui_dmx_start(pv_hdl_dmx, NULL);
            if((!have_audio) && (!have_video)) {
                //In ott hls playback, app needs filter pat&pmt first when sending ts to sw dmx.
                return AUI_RTN_SUCCESS;
            }

            if (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS) {            
                aui_dmx_set(pv_hdl_dmx, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,
                    (void *) AUI_AVSYNC_FROM_HDD_MP);
            } else {
                aui_dmx_set(pv_hdl_dmx, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,
                    (void *) AUI_AVSYNC_FROM_TUNER);
            }

            struct aui_dmx_stream_pid pids_info;
            memset(&pids_info, 0, sizeof(pids_info));
            pids_info.ul_pid_cnt = 5;
            
            for (k = 0; k < pids_info.ul_pid_cnt; k++) {
                pids_info.aus_pids_val[k] = PID_list[k];
            }
            
            pids_info.stream_types[0] = AUI_DMX_STREAM_VIDEO;
            pids_info.stream_types[1] = AUI_DMX_STREAM_AUDIO;
            pids_info.stream_types[2] = AUI_DMX_STREAM_PCR;
            pids_info.stream_types[3] = AUI_DMX_STREAM_AUDIO_DESC;
            pids_info.stream_types[4] = AUI_DMX_STREAM_SUBTITLE;

            pids_info.ul_magic_num = aui_av_magic_num;
            create_stream =
                    have_audio ?
                            (have_video ?
                                    AUI_DMX_STREAM_CREATE_AV :
                                    AUI_DMX_STREAM_CREATE_AUDIO) :
                            (have_video ? AUI_DMX_STREAM_CREATE_VIDEO : 0);
            _ret = aui_dmx_set(((aui_hdl) pv_hdl_dmx), create_stream,
                    (void *) &pids_info);

            if (_ret != AUI_RTN_SUCCESS) {
                goto RETURN;
            }
            
            enable_stream =
                    have_audio ?
                            (have_video ?
                                    AUI_DMX_STREAM_ENABLE :
                                    AUI_DMX_STREAM_ENABLE_AUDIO) :
                            (have_video ? AUI_DMX_STREAM_ENABLE_VIDEO : 0);
            _ret = aui_dmx_set(((aui_hdl) pv_hdl_dmx), enable_stream,
                    (void *) NULL);

            if (_ret != AUI_RTN_SUCCESS) {
                goto RETURN;
            }        
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_ES) {
        /*  This function will called twice when init av inject audio 
        *  and video independently. If the av inject video is initialized, 
        *  it cannot be initialized again
        *  The handle will be set to NULL after closing
        */
        if (have_video && (inject_param.video_decoder_out == NULL)) {
            aui_video_decoder_init av_video_init;
            MEMSET(&av_video_init, 0, sizeof(aui_video_decoder_init));
            av_video_init.pic_width = inject_param.video_parameters.pic_width;
            av_video_init.pic_height = inject_param.video_parameters.pic_height;
            av_video_init.sar_width = inject_param.video_parameters.sar_width;
            av_video_init.sar_height = inject_param.video_parameters.sar_height;
            av_video_init.frame_rate = inject_param.video_parameters.frame_rate;
            av_video_init.extradata = inject_param.video_parameters.extradata;
            av_video_init.extradata_size = inject_param.video_parameters.extradata_size;
            av_video_init.decode_mode = (aui_video_dec_mode) inject_param.video_parameters.decode_mode;

            av_video_init.codec_id = (aui_decv_format) inject_param.video_parameters.stream_type;
            //check if es stream is encrypted
            unsigned char enc_flag = av_handle->en_es_ctx.p_video_dsc_id ? 1:0;
            AUI_DBG("video enc_flag: %d\n", enc_flag);

            if ( AUI_RTN_SUCCESS != aui_video_decoder_open_internal(&av_video_init, &inject_param.video_decoder_out, enc_flag)) {
                AUI_ERR("open video decoder fail\n");
                return AUI_RTN_FAIL;
            }

            AUI_DBG("video_parameters.pic_width %d\n",
                (int) inject_param.video_parameters.pic_width);
            AUI_DBG("video_parameters.pic_height %d\n",
                (int) inject_param.video_parameters.pic_height);
            AUI_DBG("video_parameters.sar_width %d\n",
                (int) inject_param.video_parameters.sar_width);
            AUI_DBG(" pvideo_parameters.sar_height %d\n",
                (int) inject_param.video_parameters.sar_height);
            AUI_DBG("video_parameters.frame_rate %d\n",
                (int) inject_param.video_parameters.frame_rate);
            AUI_DBG("video_parameters.decode_mode %d\n",
                (int) inject_param.video_parameters.decode_mode);
            AUI_DBG("video_parameters.stream_type %d\n",
                (int) inject_param.video_parameters.stream_type);
        }

        /*  This function will called twice when init av inject audio 
        *  and video independently. If the av inject audio is initialized, 
        *  it cannot be initialized again
        *  The handle will be set to NULL after closing
        */
        if (have_audio && (inject_param.audio_decoder_out == NULL)) {

            aui_audio_info_init av_audio_init;
            MEMSET(&av_audio_init, 0, sizeof(aui_audio_info_init));
            av_audio_init.channels = inject_param.audio_parameters.channels;
            av_audio_init.nb_bits_per_sample = inject_param.audio_parameters.bits_per_sample;
            av_audio_init.sample_rate = inject_param.audio_parameters.sample_rate;
            av_audio_init.bit_rate = inject_param.audio_parameters.bit_rate;
            av_audio_init.block_align = inject_param.audio_parameters.block_align;
            av_audio_init.extradata = inject_param.audio_parameters.extradata;
            av_audio_init.extradata_size = inject_param.audio_parameters.extradata_size;
            av_audio_init.sign_flag = inject_param.audio_parameters.sign_flag;
            av_audio_init.endian = inject_param.audio_parameters.endian;

            av_audio_init.codec_id = (enum aui_deca_stream_type) inject_param.audio_parameters.stream_type;

            //check if es stream is encrypted
            unsigned char enc_flag = av_handle->en_es_ctx.p_audio_dsc_id ? 1:0;
            AUI_DBG("audio enc_flag: %d\n", enc_flag);
            
            if ( AUI_RTN_SUCCESS != aui_audio_decoder_open_internal(&av_audio_init, &inject_param.audio_decoder_out, enc_flag)) {
                AUI_ERR("open audio decoder fail\n");
                return AUI_RTN_FAIL;
            }

            AUI_DBG("audio_parameters.channels %d\n", (int) inject_param.audio_parameters.channels);
            AUI_DBG("audio_parameters.stream_typet %d\n", (int) inject_param.audio_parameters.stream_type);
            AUI_DBG("audio_parameters.bits_per_sample %d\n", (int) inject_param.audio_parameters.bits_per_sample);
            AUI_DBG("audio_parameters.sample_rate %d\n",      (int) inject_param.audio_parameters.sample_rate);
            AUI_DBG("audio_parameters.extradata_size %d\n", (int) inject_param.audio_parameters.extradata_size);
        }

        if ((inject_param.es_have_video) && (inject_param.es_have_audio)) {
            if (AUI_RTN_SUCCESS != aui_audio_decoder_set_sync(inject_param.audio_decoder_out, 1)) {
                AUI_ERR("aui_audio_decoder_set_sync fail\n");
                return AUI_RTN_FAIL;
            }
        }
    }

RETURN:
    return _ret;
}

AUI_RTN_CODE aui_av_close(aui_hdl pv_handleAV) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if (NULL == pv_handleAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    aui_handleAV *av_hdl = pv_handleAV;
    void *pv_hdl_vdec = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_decv;
    void *pv_hdl_deca = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_deca;
    void *pv_hdl_dmx = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_dmx;
    void *pv_hdl_tsi = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_tsi;
    void *pv_hdl_tsg = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_tsg;

    aui_av_inject_settings_t *p_av_setting =
            &(((aui_handleAV *) (pv_handleAV))->attrAV.stream_type);
            
    if (p_av_setting->data_type == AUI_AV_DATA_TYPE_ES) {
        if ((inject_param.video_decoder_out) 
             && (av_hdl->attrAV.st_av_info.b_video_enable)) {
            /* stop video */
            if (AUI_RTN_SUCCESS
                    != aui_video_decoder_close(
                            inject_param.video_decoder_out)) {
                free(pv_handleAV);
                aui_rtn(AUI_RTN_FAIL,
                        "\nstop video fail in av close\n");
            }
            inject_param.video_decoder_out = 0;
            //inject_param.es_have_video = 0;
            av_hdl->attrAV.st_av_info.b_video_enable = 0;
        }
        if ((inject_param.audio_decoder_out) 
            && (av_hdl->attrAV.st_av_info.b_audio_enable)) {
            /* stop audio */
            if (AUI_RTN_SUCCESS
                    != aui_audio_decoder_close(
                            inject_param.audio_decoder_out)) {
                free(pv_handleAV);
                aui_rtn(AUI_RTN_FAIL,
                        "\nstop audio fail in av close\n");
            }
            inject_param.audio_decoder_out = 0;
            //inject_param.es_have_audio = 0;
            av_hdl->attrAV.st_av_info.b_audio_enable = 0;
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS 
                || p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS) {
        if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)
            && (p_av_setting->dmx_id != AUI_DMX_ID_SW_DEMUX0)
            && (p_av_setting->dmx_id != AUI_DMX_ID_SW_DEMUX1)) {
            if (aui_av_open_device & AUI_AV_OPEN_TSG) {
                aui_tsg_close(pv_hdl_tsg);
                aui_av_open_device &= ~(AUI_AV_OPEN_TSG);
            }

            if (aui_av_open_device & AUI_AV_OPEN_TSI) {
                aui_tsi_close(pv_hdl_tsi);
                aui_av_open_device &= ~(AUI_AV_OPEN_TSI);
            }
        }

        if (aui_av_open_device & AUI_AV_OPEN_DMX) {
            aui_dmx_close(pv_hdl_dmx);
            aui_av_open_device &= ~(AUI_AV_OPEN_DMX);
        }
        if (aui_av_open_device & AUI_AV_OPEN_DECA) {
            if (AUI_RTN_SUCCESS != aui_deca_close(pv_hdl_deca)) {
                free(pv_handleAV);
                aui_rtn(AUI_RTN_FAIL, "\nstop audio fail in av close\n");
            }
            aui_av_open_device &= ~(AUI_AV_OPEN_DECA);
        }
        if (aui_av_open_device & AUI_AV_OPEN_DECV) {
            if (AUI_RTN_SUCCESS != aui_decv_close(NULL, &pv_hdl_vdec)) {
                free(pv_handleAV);
                aui_rtn(AUI_RTN_FAIL, "\nstop video fail in av close\n");
            }
            aui_av_open_device &= ~(AUI_AV_OPEN_DECV);
        }
    }
   	if(av_hdl->mmap_flag) {
	    /*
		* only for restore see resource, ,wait for to es playback codeing finish and go back.
		*/
        if(alisldsc_pvr_ioctl_ex(av_hdl->ali_pvr_hdl, SL_PVR_IO_FREE_BLOCK, 0)) {
            AUI_ERR("SL_PVR_IO_FREE_BLOCK error\n");
        }
        if(aui_dsc_pvr_munmap(av_hdl->ali_pvr_hdl, (unsigned int *)av_hdl->mmap_addr, &av_hdl->mmap_len)) {
            AUI_ERR("munmap fail\n");
        }
        AUI_DBG("munmap ok\n");
       }
    free(pv_handleAV);
    aui_av_open_device = AUI_AV_OPEN_NONE;
    pv_handleAV = NULL;
    pv_hdl_vdec = NULL;
    pv_hdl_deca = NULL;
    pv_hdl_dmx = NULL;
    return rtn_code;
}

AUI_RTN_CODE aui_av_start(aui_hdl pv_handleAV) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if (NULL == pv_handleAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (AUI_RTN_SUCCESS != cc_av_act(pv_handleAV)) {
        aui_rtn(AUI_RTN_FAIL, "\nstart fail in av start\n");
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_stop(void *pv_handleAV) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if (NULL == pv_handleAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    void *pv_hdl_vdec = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_decv;
    void *pv_hdl_deca = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_deca;
    void *pv_hdl_dmx = ((aui_handleAV *) (pv_handleAV))->attrAV.pv_hdl_dmx;
    av_info *p_info = &((((aui_handleAV *) (pv_handleAV))->attrAV).st_av_info);

    aui_av_inject_settings_t *p_av_setting =
            &(((aui_handleAV *) (pv_handleAV))->attrAV.stream_type);

    AUI_DBG("dmx hdl:%p,decv hdl:%p,deca hdl:%p\n", pv_hdl_vdec,
            pv_hdl_deca, pv_hdl_dmx);

    if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS)
            || (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)) {
        //stop audio
        if (p_info->b_audio_enable) {
            if (AUI_RTN_SUCCESS != aui_deca_stop(pv_hdl_deca, NULL)) {
                aui_rtn(AUI_RTN_FAIL,
                        "\nstop audio fail in av close\n");
            }
        }

        if (p_info->b_video_enable) {
            if (AUI_RTN_SUCCESS != aui_decv_stop(pv_hdl_vdec)) {
                aui_rtn(AUI_RTN_FAIL, "\nstop video fail in av close\n");
            }
        }

        if (p_info->b_dmx_enable) {
            aui_dmx_set(((aui_hdl) pv_hdl_dmx), AUI_DMX_STREAM_DISABLE, NULL);
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_ES) {

        /*if (AUI_RTN_SUCCESS
                != aui_video_decoder_stop(inject_param.video_decoder_out)) {
            free(pv_handleAV);
            aui_rtn(AUI_RTN_FAIL, "\nstop video fail in av close\n");
        }

        if (AUI_RTN_SUCCESS
                != aui_audio_decoder_stop(inject_param.audio_decoder_out)) {
            free(pv_handleAV);
            aui_rtn(AUI_RTN_FAIL, "\nstop audio fail in av close\n");
        }*/
        rtn_code = AUI_RTN_SUCCESS;
    }

    pv_hdl_vdec = NULL;
    pv_hdl_deca = NULL;
    pv_hdl_dmx = NULL;
    p_info = NULL;
    p_av_setting = NULL;
    return rtn_code;
}

AUI_RTN_CODE aui_av_pause(void *pv_handleAV) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    aui_handleAV *p_hdl_av = NULL;
    void *pv_hdl_vdec = NULL;
    void *pv_hdl_deca = NULL;
    void *pv_hdl_snd = NULL;

    if (NULL == pv_handleAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    p_hdl_av = (aui_handleAV *) (pv_handleAV);
    aui_av_inject_settings_t *p_av_setting = &(p_hdl_av->attrAV.stream_type);

    if (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS) {
        //for live play, should stop when pausing
        pv_hdl_deca = p_hdl_av->attrAV.pv_hdl_deca;
        if (pv_hdl_deca && p_hdl_av->attrAV.st_av_info.b_audio_enable) {
            if (AUI_RTN_SUCCESS!=aui_deca_stop(pv_hdl_deca, NULL))
            {
                aui_rtn(AUI_RTN_FAIL,"\nstop audio fail in av pause\n");
            }
        }
        pv_hdl_vdec = p_hdl_av->attrAV.pv_hdl_decv;
        if (pv_hdl_vdec && p_hdl_av->attrAV.st_av_info.b_video_enable) {
            if (AUI_RTN_SUCCESS!=aui_decv_stop(pv_hdl_vdec))
            {
                aui_rtn(AUI_RTN_FAIL,"\nstop video fail in av pause\n");
            }
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS) {
        pv_hdl_snd = p_hdl_av->attrAV.pv_hdl_snd;
        if (pv_hdl_snd) {
            if (AUI_RTN_SUCCESS != aui_snd_pause(pv_hdl_snd, NULL)) {
                aui_rtn(AUI_RTN_FAIL, "\npause snd fail in av pause\n");
            }
        }

        pv_hdl_deca = p_hdl_av->attrAV.pv_hdl_deca;
        if (pv_hdl_deca && p_hdl_av->attrAV.st_av_info.b_audio_enable) {
            if (AUI_RTN_SUCCESS != aui_deca_pause(pv_hdl_deca, NULL)) {
                aui_rtn(AUI_RTN_FAIL, "\npause audio fail in av pause\n");
            }
        }

        pv_hdl_vdec = p_hdl_av->attrAV.pv_hdl_decv;
        if (pv_hdl_vdec && p_hdl_av->attrAV.st_av_info.b_video_enable) {
            if (AUI_RTN_SUCCESS != aui_decv_pause(pv_hdl_vdec)) {
                aui_rtn(AUI_RTN_FAIL, "\npause video fail in av pause\n");
            }
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_ES) {
        if ((inject_param.audio_decoder_out) 
            && (p_hdl_av->attrAV.st_av_info.b_audio_enable)) {
            if (AUI_RTN_SUCCESS
                    != aui_audio_decoder_pause(inject_param.audio_decoder_out,
                            1))
                aui_rtn(AUI_RTN_FAIL, "\npause audio fail in av pause\n");
        }
        if ((inject_param.video_decoder_out) 
            && (p_hdl_av->attrAV.st_av_info.b_video_enable)) {
            if (AUI_RTN_SUCCESS
                    != aui_video_decoder_pause(inject_param.video_decoder_out,
                            1))
                aui_rtn(AUI_RTN_FAIL, "\npause video fail in av pause\n");
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_resume(void *pv_handleAV) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    aui_handleAV *p_hdl_av = NULL;
    void *pv_hdl_vdec = NULL;
    void *pv_hdl_deca = NULL;
    void *pv_hdl_snd = NULL;

    if (NULL == pv_handleAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    p_hdl_av = (aui_handleAV *) (pv_handleAV);
    aui_av_inject_settings_t *p_av_setting = &(p_hdl_av->attrAV.stream_type);

    if (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS) {
        //for live play, stopped when pausing, then need start again
        pv_hdl_deca = p_hdl_av->attrAV.pv_hdl_deca;
        if (pv_hdl_deca && p_hdl_av->attrAV.st_av_info.b_audio_enable) {
            if (AUI_RTN_SUCCESS != aui_deca_start(pv_hdl_deca, NULL)) {
                aui_rtn(AUI_RTN_FAIL, "\nstart audio fail in av start\n");
            }
        }

        pv_hdl_vdec = p_hdl_av->attrAV.pv_hdl_decv;
        if (pv_hdl_vdec && p_hdl_av->attrAV.st_av_info.b_video_enable) {
            if (AUI_RTN_SUCCESS != aui_decv_start(pv_hdl_vdec)) {
                aui_rtn(AUI_RTN_FAIL, "\nstart video fail in av start\n");
            }
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS) {
        pv_hdl_snd = p_hdl_av->attrAV.pv_hdl_snd;
        if (pv_hdl_snd) {
            if (AUI_RTN_SUCCESS != aui_snd_resume(pv_hdl_snd, NULL)) {
                aui_rtn(AUI_RTN_FAIL, "\nresume snd fail in av pause\n");
            }
        }

        pv_hdl_deca = p_hdl_av->attrAV.pv_hdl_deca;
        if (pv_hdl_deca && p_hdl_av->attrAV.st_av_info.b_audio_enable) {
            if (AUI_RTN_SUCCESS != aui_deca_resume(pv_hdl_deca, NULL)) {
                aui_rtn(AUI_RTN_FAIL, "\nresume audio fail in av start\n");
            }
        }

        pv_hdl_vdec = p_hdl_av->attrAV.pv_hdl_decv;
        if (pv_hdl_vdec && p_hdl_av->attrAV.st_av_info.b_video_enable) {
            if (AUI_RTN_SUCCESS != aui_decv_resume(pv_hdl_vdec)) {
                aui_rtn(AUI_RTN_FAIL, "\nresume video fail in av start\n");
            }
        }
    } else if (p_av_setting->data_type == AUI_AV_DATA_TYPE_ES) {
        if ((inject_param.audio_decoder_out) 
            && (p_hdl_av->attrAV.st_av_info.b_audio_enable)) {
            if (AUI_RTN_SUCCESS
                    != aui_audio_decoder_pause(inject_param.audio_decoder_out,
                            0))
                aui_rtn(AUI_RTN_FAIL, "\nresume audio fail\n");
        }
        if ((inject_param.video_decoder_out) 
            && (p_hdl_av->attrAV.st_av_info.b_video_enable)) {
            if (AUI_RTN_SUCCESS
                    != aui_video_decoder_pause(inject_param.video_decoder_out,
                            0))
                aui_rtn(AUI_RTN_FAIL, "\nresume video fail\n");
        }
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_init_attr(aui_attrAV *p_attrAV,
        aui_av_stream_info *av_stream_info) 
{

    enum aui_tsi_input_id input_id;
    enum aui_tsi_output_id tsi_dmx_out_id;
    enum aui_tsi_channel_id tsi_channel_id;
    enum aui_dmx_id dmx_id;

    aui_attr_dmx attr_dmx;
    aui_attr_deca attr_deca;
    aui_attr_decv attr_decv;
    //aui_attr_snd attr_snd;
    aui_attr_tsi attr_tsi;
    aui_attr_tsg attr_tsg;
    //aui_attr_dis attr_dis;
    aui_attrAV attr_av;
    aui_av_inject_settings_t * p_av_setting = NULL;
    av_info *pst_av_info = NULL;

    if (!av_stream_info || !p_attrAV) {
        AUI_ERR("invalid parameters\n");
        return AUI_RTN_FAIL;
    }

    p_av_setting = &(av_stream_info->stream_type);
    pst_av_info = &(av_stream_info->st_av_info);

    MEMSET(&attr_av, 0, sizeof(aui_attrAV));

    //#73437 for open av inject audio and video separately
    //memset(&inject_param, 0x0, sizeof(av_inject_t));
    
    if (p_av_setting->data_type == AUI_AV_DATA_TYPE_ES) {
        if (NULL != av_stream_info->p_video_parameters) {
            memcpy(&inject_param.video_parameters,
            av_stream_info->p_video_parameters,
            sizeof(aui_av_video_info));
            //inject_param.es_have_video = 1;
            attr_av.st_av_info.b_video_enable = 1;
        }
        if (NULL != av_stream_info->p_audio_parameters) {
            memcpy(&inject_param.audio_parameters,
            av_stream_info->p_audio_parameters,
            sizeof(aui_av_audio_info));
            //inject_param.es_have_audio = 1;
            attr_av.st_av_info.b_audio_enable = 1;
        }
    }
    
    dmx_id = p_av_setting->dmx_id;

    if (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS) {
        //If Inject to HW DMX, set the route path from:RAM->TSG->TSI->DMX->AV Decoder.
        //If Inject to SW DMX, Directly write data to playback Demux.
        if ((dmx_id != AUI_DMX_ID_SW_DEMUX0) && (dmx_id != AUI_DMX_ID_SW_DEMUX1)) {
            if (aui_find_dev_by_idx(AUI_MODULE_TSG, 0, &attr_av.pv_hdl_tsg)) {
                MEMSET(&attr_tsg, 0, sizeof(attr_tsg));
                attr_tsg.ul_tsg_clk = 0x18;
                if(aui_tsg_open(&attr_tsg, &attr_av.pv_hdl_tsg)) {
                    aui_rtn(AUI_RTN_FAIL, "\n open tsg failed\n");
                }
                aui_av_open_device |= AUI_AV_OPEN_TSG;
            }
            
            if (aui_find_dev_by_idx(AUI_MODULE_TSI, 0, &attr_av.pv_hdl_tsi)) {
                MEMSET(&attr_tsi, 0, sizeof(attr_tsi));
                if(aui_tsi_open(&attr_av.pv_hdl_tsi)) {
                    aui_rtn(AUI_RTN_FAIL, "\n open tsi failed\n");
                }
                aui_av_open_device |= AUI_AV_OPEN_TSI;
            }
            
            input_id = AUI_TSI_INPUT_TSG;
            tsi_dmx_out_id = dmx_id;
            tsi_channel_id = AUI_TSI_CHANNEL_0;
            attr_tsi.ul_init_param = 0x83;
            if (aui_tsi_src_init(attr_av.pv_hdl_tsi, input_id, &attr_tsi)) {
                aui_rtn(AUI_RTN_FAIL, "aui_tsi_src_init return fail!\n");
            }
            if (aui_tsi_route_cfg(attr_av.pv_hdl_tsi, input_id, tsi_channel_id, tsi_dmx_out_id)) {
                aui_rtn(AUI_RTN_FAIL, "aui_tsi_route_cfg return fail!\n");
            }
        }
    }

    if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)
        || (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS)) {
        if (aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_id, &attr_av.pv_hdl_dmx)) {
            MEMSET(&attr_dmx, 0, sizeof(attr_dmx));
            attr_dmx.uc_dev_idx = dmx_id;
            if(aui_dmx_open(&attr_dmx, &attr_av.pv_hdl_dmx)){
                aui_rtn(AUI_RTN_FAIL, "\n open dmx failed\n");
            }
            aui_av_open_device |= AUI_AV_OPEN_DMX;
        }

        if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &attr_av.pv_hdl_deca)) {
            MEMSET(&attr_deca, 0, sizeof(aui_attr_deca));
            if(aui_deca_open(&attr_deca, &attr_av.pv_hdl_deca)){
                aui_rtn(AUI_RTN_FAIL, "\n open deca failed\n");
            }
            aui_av_open_device |= AUI_AV_OPEN_DECA;
        }

        if (aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &attr_av.pv_hdl_decv)) {
            MEMSET(&attr_decv, 0, sizeof(aui_attr_decv));
            //attr_decv.uc_dev_idx = av_stream_info->st_av_info.en_video_stream_type;
            if(aui_decv_open(&attr_decv, &attr_av.pv_hdl_decv)){
                aui_rtn(AUI_RTN_FAIL, "\n open decv failed\n");
            }
            aui_av_open_device |= AUI_AV_OPEN_DECV;
        }
    }

    MEMCPY(p_attrAV, &attr_av, sizeof(aui_attrAV));
    MEMCPY(&(((aui_attrAV *) p_attrAV)->stream_type), p_av_setting,
        sizeof(aui_av_inject_settings_t));

    if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)
        || (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS))
        MEMCPY(&(((aui_attrAV *) p_attrAV)->st_av_info), pst_av_info,
            sizeof(av_info));

    pst_av_info = NULL;
    p_av_setting = NULL;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_av_init(aui_funcAVInit fnAVInit) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if (fnAVInit) {
        fnAVInit();
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_de_init(aui_funcAVInit fnAVDe_init) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if (fnAVDe_init) {
        fnAVDe_init();
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_set(void *pv_hdlAV, aui_av_item_set ul_item, void *pv_param) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if (NULL == pv_hdlAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    av_info *p_info = &((((aui_handleAV *) (pv_hdlAV))->attrAV).st_av_info);
    switch (ul_item) {
    case AUI_AV_VIDEO_PID_SET:
        p_info->b_video_enable = 1;
        p_info->ui_video_pid = *((unsigned short *) pv_param);
        break;

    case AUI_AV_AUDIO_PID_SET:
        p_info->b_audio_enable = 1;
        p_info->ui_audio_pid = *((unsigned short *) pv_param);
        break;

    case AUI_AV_PCR_PID_SET:
        p_info->b_pcr_enable = 1;
        p_info->ui_pcr_pid = *((unsigned short *) pv_param);
        break;

    case AUI_AV_TTX_PID_SET:
        p_info->b_ttx_enable = 1;
        p_info->ui_ttx_pid = *((unsigned short *) pv_param);
        break;

    case AUI_AV_SUB_PID_SET:
        p_info->b_sub_enable = 1;
        p_info->ui_sub_pid = *((unsigned short *) pv_param);
        break;

    case AUI_AV_VIDEO_TYPE_SET:
        p_info->en_video_stream_type = *((unsigned int *) pv_param);
        break;

    case AUI_AV_AUDIO_TYPE_SET:
        p_info->en_audio_stream_type = *((unsigned int *) pv_param);
        break;

    case AUI_AV_SPDIF_TYPE_SET:
        p_info->en_spdif_type = *((unsigned int *) pv_param);
        break;

    case AUI_AV_MODIFY_SET:
        p_info->b_modify = *((unsigned int *) pv_param);
        break;

    case AUI_AV_DMX_ENABLE:
        p_info->b_dmx_enable = *((unsigned int *) pv_param);
        break;

    case AUI_AV_VIDEO_ENABLE:
        p_info->b_video_enable = *((unsigned int *) pv_param);
        break;

    case AUI_AV_AUDIO_ENABLE:
        p_info->b_audio_enable = *((unsigned int *) pv_param);
        break;

    case AUI_AV_PCR_ENABLE:
        p_info->b_pcr_enable = *((unsigned int *) pv_param);
        break;

    case AUI_AV_TTX_ENABLE:
        p_info->b_ttx_enable = *((unsigned int *) pv_param);
        break;

    case AUI_AV_SUB_ENABLE:
        p_info->b_sub_enable = *((unsigned int *) pv_param);
        break;

    default:
        break;
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_get(void *pv_hdlAV, aui_av_item_get ul_item, void *pv_param) {
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if (NULL == pv_hdlAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    av_info *p_info = &((((aui_handleAV *) (pv_hdlAV))->attrAV).st_av_info);
    switch (ul_item) {
    case AUI_AV_VIDEO_PID_GET:
        if (p_info->b_video_enable) {
            *((unsigned short *) pv_param) = p_info->ui_video_pid;
        } else {
            aui_rtn(AUI_RTN_FAIL, "\nvideo disable,can't get pid\n");
        }
        break;

    case AUI_AV_AUDIO_PID_GET:
        if (p_info->b_audio_enable) {
            *((unsigned short *) pv_param) = p_info->ui_audio_pid;
        } else {
            aui_rtn(AUI_RTN_FAIL, "\naudio disable,can't get pid\n");
        }
        break;

    case AUI_AV_PCR_PID_GET:
        if (p_info->b_pcr_enable) {
            *((unsigned short *) pv_param) = p_info->ui_pcr_pid;
        } else {
            aui_rtn(AUI_RTN_FAIL, "\npcr disable,can't get pid\n");
        }
        break;

    case AUI_AV_TTX_PID_GET:
        if (p_info->b_ttx_enable) {
            *((unsigned short *) pv_param) = p_info->ui_ttx_pid;
        } else {
            aui_rtn(AUI_RTN_FAIL, "\nttx disable,can't get pid\n");
        }
        break;

    case AUI_AV_SUB_PID_GET:
        if (p_info->b_sub_enable) {
            *((unsigned short *) pv_param) = p_info->ui_sub_pid;
        } else {
            aui_rtn(AUI_RTN_FAIL, "\nsubtitle disable,can't get pid\n");
        }
        break;

    case AUI_AV_VIDEO_TYPE_GET:
        *((unsigned int *) pv_param) = p_info->en_video_stream_type;
        break;

    case AUI_AV_AUDIO_TYPE_GET:
        *((unsigned int *) pv_param) = p_info->en_audio_stream_type;
        break;

    case AUI_AV_SPDIF_TYPE_GET:
        *((unsigned int *) pv_param) = p_info->en_spdif_type;
        break;

    case AUI_AV_MODIFY_GET:
        *((unsigned int *) pv_param) = p_info->b_modify;
        break;

    default:
        break;
    }
    return rtn_code;
}

AUI_RTN_CODE aui_av_flush(void *pv_handleAV, aui_av_flush_setting *setting) 
{
    aui_handleAV *av_handle;
    struct aui_attr_tsg attr;

    if (NULL == pv_handleAV || (NULL == setting)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    av_handle = (aui_handleAV *) pv_handleAV;

    aui_av_inject_settings_t *p_av_setting =
        &(((aui_handleAV *) (pv_handleAV))->attrAV.stream_type);

    if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS)
        || (p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)) {
        //when use sw dmx to play local ts or when play live ts, there is no tsg
        if (av_handle->attrAV.pv_hdl_tsg != NULL) {
            MEMSET(&attr, 0, sizeof(attr));
            attr.ul_tsg_clk = 0x18;
            if (aui_tsg_stop(av_handle->attrAV.pv_hdl_tsg, &attr) != AUI_RTN_SUCCESS) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
        }

        if (aui_dmx_set(av_handle->attrAV.pv_hdl_dmx, AUI_DMX_STREAM_DISABLE,
            0) != AUI_RTN_SUCCESS) {
            aui_rtn(AUI_RTN_EINVAL, NULL);
        }
    } else if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_ES)) {
        if ((inject_param.audio_decoder_out) 
            && (av_handle->attrAV.st_av_info.b_audio_enable)) {
            if (AUI_RTN_SUCCESS != 
                aui_audio_decoder_flush(inject_param.audio_decoder_out))
                aui_rtn(AUI_RTN_FAIL, "\nflush audio fail\n");
            }
        if ((inject_param.video_decoder_out) 
            && (av_handle->attrAV.st_av_info.b_video_enable)) {
            if (AUI_RTN_SUCCESS
                != aui_video_decoder_flush(inject_param.video_decoder_out,
                setting->playback_mode))
                aui_rtn(AUI_RTN_FAIL, "\nflush video fail\n");
        }
    }
    
    p_av_setting = NULL;
    return AUI_RTN_SUCCESS;
}

static int write_block_es_data_by_dsc(const aui_handleAV *av_hdl, aui_av_packet *p_pkt, 
        const unsigned char *buf, const unsigned long size, enum pvr_ott_data_type type) {
    //write es data
    int ret = 0;
    unsigned int data_write_fail = 0;
    unsigned int wrote_bytes = 0;
    unsigned int to_be_wrote = 0;
    unsigned int remain_bytes = (unsigned int)size;
    ali_pvr_de_hdl_magic *de_hdl_magic = (PVR_OTT_DATA_VBV == type)?((ali_pvr_de_hdl_magic *)(&av_hdl->video_dsc_res_id)):
                                    ((ali_pvr_de_hdl_magic *)(&av_hdl->audio_dsc_res_id));
    if ((!de_hdl_magic) || (!de_hdl_magic->ali_pvr_de_hdl)) {
        AUI_ERR("fail\n");
        return -1;
    }
    if (size > av_hdl->mmap_len) {
        AUI_ERR("is too big, not support now\n");
        return -1;
    }
    pthread_mutex_lock(&es_blk_mutex);
    for(;wrote_bytes<size;) {
        data_write_fail = 0;
        remain_bytes = size-wrote_bytes;
        to_be_wrote = (remain_bytes>=av_hdl->mmap_len)?av_hdl->mmap_len:remain_bytes;
        do {
            //if need, write iv and write full sample data
            memcpy((void *)av_hdl->mmap_addr, buf+wrote_bytes, to_be_wrote);
            ret = aui_dsc_pvr_decrypt_block_data(av_hdl->ali_pvr_hdl, de_hdl_magic->ali_pvr_de_hdl, (const unsigned char *)av_hdl->mmap_addr, to_be_wrote,0,
                p_pkt->iv_ctr, p_pkt->iv_length, type);
            if ( AUI_RTN_SUCCESS != ret) {
                usleep(USLEEP_TIME);
                data_write_fail++;
            }
        } while( (ret != AUI_RTN_SUCCESS)&&(data_write_fail<=WRITE_ERROR_MAX));
        if (ret != AUI_RTN_SUCCESS) {
            AUI_ERR("fail\n");
            pthread_mutex_unlock(&es_blk_mutex);
            return -1;
        }
        wrote_bytes += to_be_wrote;
    }
    pthread_mutex_unlock(&es_blk_mutex);
    return 0;
}
#if 0
static int write_avc_block_es_data(const aui_handleAV *av_hdl, aui_av_packet *p_pkt, 
        const unsigned char *buf, const unsigned long size) 
{
    unsigned long i = 0;
    unsigned int clr_buf_len = 0;
    unsigned int enc_buf_len = 0;
    unsigned long sub_clr_len = 0;
    unsigned long sub_enc_len = 0;
    unsigned long clr_copied_len = 0;
    unsigned long enc_copied_len = 0;
    unsigned int pkt_write_fail = 0;
    unsigned int data_write_fail = 0;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    AUI_DBG("iv: %p, iv_len: %lu\n", p_pkt->iv_ctr, p_pkt->iv_length);
    AUI_DUMP("iv: ", (char *)p_pkt->iv_ctr, p_pkt->iv_length);

    AUI_DBG("cnt: %lu\n", p_pkt->subsampe_enc_info.ul_subsample_count);
    //get total clear buf length and enc buf length
    for(i=0;i<p_pkt->subsampe_enc_info.ul_subsample_count; i++) {
        AUI_DBG("%lu clear: %d, enc: %lu\n", i,
                (int)p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_clear_data,
                p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_encrypted_data);
        
        clr_buf_len += p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_clear_data;
        enc_buf_len +=p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_encrypted_data;
        
        AUI_DBG("clr_buf_len: 0x%08x, enc_buf_len: 0x%08x\n", clr_buf_len, enc_buf_len);
    }

    if((clr_buf_len+enc_buf_len) != size) {
        AUI_ERR("warning size: %lu, clr: %u, enc: %u, (clr+enc): %u\n", size, clr_buf_len,
            enc_buf_len, (clr_buf_len+enc_buf_len));
        return -1;
    }

    if (enc_buf_len > av_hdl->mmap_len) {
        AUI_ERR("es is too big, not support now\n");
        return -1;
    }

    //write packet header, size is the encrypted buf length
    p_pkt->size = enc_buf_len;
    do {
        ret = aui_video_decoder_write_header(inject_param.video_decoder_out, p_pkt);
        if ( AUI_RTN_SUCCESS != ret) {
            usleep(USLEEP_TIME);
            pkt_write_fail++;
        } 
    } while( (ret != AUI_RTN_SUCCESS)&&(pkt_write_fail<=WRITE_ERROR_MAX));
    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("fail\n");
        return -1;
    }
    //write clear NALU header.
    pthread_mutex_lock(&es_blk_mutex);
    memset((void *)av_hdl->mmap_addr, 0, av_hdl->mmap_len);
    clr_copied_len=enc_copied_len=sub_clr_len=sub_enc_len=0;
    for(i=0;i<p_pkt->subsampe_enc_info.ul_subsample_count; i++) {
        sub_clr_len = p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_clear_data;
        sub_enc_len = p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_encrypted_data;
        memcpy((void *)av_hdl->mmap_addr+clr_copied_len, buf+clr_copied_len+enc_copied_len, sub_clr_len);
        clr_copied_len += sub_clr_len;
        enc_copied_len += sub_enc_len;
    }
    do {
        ret = aui_video_decoder_write_nal_header(inject_param.video_decoder_out, 
            (const unsigned char *)av_hdl->mmap_addr, clr_copied_len);
        if ( AUI_RTN_SUCCESS != ret) {
            data_write_fail++;
        }
    } while( (ret != AUI_RTN_SUCCESS)&&(data_write_fail<=WRITE_ERROR_MAX));
    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("fail\n");
        pthread_mutex_unlock(&es_blk_mutex);
        return -1;
    }
    //write encrypted NAL es.
    unsigned int wrote_bytes = 0;
    unsigned int to_be_wrote = 0;
    //unsigned int remain_bytes = enc_buf_len;
    ali_pvr_de_hdl_magic *de_hdl_magic = (ali_pvr_de_hdl_magic *)(&av_hdl->video_dsc_res_id);
    memset((void *)av_hdl->mmap_addr, 0, av_hdl->mmap_len);
    clr_copied_len=enc_copied_len=sub_clr_len=sub_enc_len=0;
    int write_cnt = 0;
    int need_write = 0;
    for(i=0; ((i<p_pkt->subsampe_enc_info.ul_subsample_count) || (wrote_bytes<enc_buf_len)); ) {
        //There is sub nalu to be written.
        if(i<p_pkt->subsampe_enc_info.ul_subsample_count) {
            sub_clr_len = p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_clear_data;
            sub_enc_len = p_pkt->subsampe_enc_info.p_subsample_byte_info[i].bytes_of_encrypted_data;
            //check if mmap buf can store one more nalu.
            if ((av_hdl->mmap_len-to_be_wrote)>=sub_enc_len) {
                memcpy((void *)av_hdl->mmap_addr+enc_copied_len, buf+clr_copied_len+enc_copied_len+sub_clr_len, sub_enc_len);
                to_be_wrote += sub_enc_len;
                clr_copied_len += sub_clr_len;
                enc_copied_len += sub_enc_len;
                i++;
            } else {
                //mmap buf can not store one full nalu, let us write 
                need_write = 1;
            }    
        } else {
            //All NALU have been stored into mmap_buf, write the remain es data of mmap_buf . 
            need_write = 1;
        }
        if (need_write){
            //write the NALU in current mmap buf.
            data_write_fail = 0;
            do {
                if(0 == write_cnt) {
                    ret = aui_dsc_pvr_decrypt_block_data(av_hdl->ali_pvr_hdl, de_hdl_magic->ali_pvr_de_hdl, 
                        (const unsigned char *)av_hdl->mmap_addr, to_be_wrote, 0,
                        p_pkt->iv_ctr, p_pkt->iv_length, PVR_OTT_DATA_VBV);
                } else {
                    //if the iv of this full encrypted es buf has been set, do not reset iv now. 
                    ret = aui_dsc_pvr_decrypt_block_data(av_hdl->ali_pvr_hdl, de_hdl_magic->ali_pvr_de_hdl, 
                        (const unsigned char *)av_hdl->mmap_addr, to_be_wrote, 0, NULL, 0, PVR_OTT_DATA_VBV);
                }
                if ( AUI_RTN_SUCCESS != ret) {
                    usleep(USLEEP_TIME);
                    data_write_fail++;
                }
            } while( (ret != AUI_RTN_SUCCESS) && (data_write_fail<=WRITE_ERROR_MAX));
            if (ret != AUI_RTN_SUCCESS) {
                AUI_ERR("fail @ %d", write_cnt);
                pthread_mutex_unlock(&es_blk_mutex);
                return -1;
            } else {
                AUI_DBG("try %u ok", data_write_fail);
                wrote_bytes += to_be_wrote;
                need_write = 0;
                to_be_wrote = 0;                
                memset((void *)av_hdl->mmap_addr, 0, av_hdl->mmap_len);
                write_cnt++;
            }
        }
    }
    pthread_mutex_unlock(&es_blk_mutex);

	
	return ret;	
}
#endif 

static int write_video_block_es_data_subsample(aui_handleAV *av_hdl, aui_av_packet *p_pkt, 
        const unsigned char *buf, const unsigned long size) 
{
    int write_fail_cnt = 0;
    struct aui_av_packet pkt_header;
    aui_es_data_desc  desc_header;
    unsigned long desc_header_len = 0;
    unsigned char *p_dst = NULL;
    aui_av_subsample_encryption_info *p_sub_info = NULL;
    aui_av_subsample_byte_info *p_byte_info = NULL;
    aui_es_sub_pkt_info *p_sub_pkt_info = NULL;
    unsigned long i = 0, j = 0, k = 0;

    memset(&pkt_header,0x0,sizeof(struct aui_av_packet));
    memset(&desc_header, 0, sizeof(aui_es_data_desc));

    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    AUI_DBG("Enter");


    if (!p_pkt || !buf || !size) {
        aui_rtn(AUI_RTN_EINVAL, "invalid input arg\n");
    }
    
    if (av_hdl->attrAV.st_av_info.b_video_enable == 0) {
        aui_rtn(AUI_RTN_EINVAL, "av_hdl->attrAV.st_av_info.b_video_enable says no video data for this play.");
    }
	
    AUI_DBG("iv: %p, iv_len: %lu\n", p_pkt->iv_ctr, p_pkt->iv_length);
    //AUI_DUMP("iv: ", (char *)p_pkt->iv_ctr, p_pkt->iv_length);
    
    AUI_DBG("ul_subsample_count: %lu\n", p_pkt->subsampe_enc_info.ul_subsample_count);

    pkt_header.dts = p_pkt->dts;
    pkt_header.pts = p_pkt->pts;
    pkt_header.size = p_pkt->size;
    pkt_header.flag = p_pkt->flag;

    // Write header firstly, it will pass to decoder through hdr_sbm
    do {
        ret = aui_video_decoder_write_header(inject_param.video_decoder_out, &pkt_header);
    
        if (ret != AUI_RTN_SUCCESS) {
            write_fail_cnt ++;
            AUI_DBG("write_fail_cnt: %d\n", write_fail_cnt);
            usleep(20 * 1000);
        }
    } while ((ret != AUI_RTN_SUCCESS) && (write_fail_cnt <= WRITE_ERROR_MAX));

    pthread_mutex_lock(&es_blk_mutex);
    
    /** can be divided into multiple transmission data*/
    unsigned int wrote_bytes = 0; //has been written in length
    int to_be_wrote = 0;          //The current length of writing this time
    unsigned long remain_bytes = size;      //left unwritten length
    unsigned long max_write_size = SHARE_MEMORY_LEN; //The maximum length of a write data

    ali_pvr_de_hdl_magic *de_hdl_magic = (ali_pvr_de_hdl_magic *)(&av_hdl->video_dsc_res_id);
    memset((void *)av_hdl->mmap_addr, 0, av_hdl->mmap_len);
    p_dst = (unsigned char*)av_hdl->mmap_addr;
    //Whether to subcontract, get this time to write the length
    //to_be_wrote = (remain_bytes>=max_write_size)?max_write_size:remain_bytes;    

    // Sub sample case (AVC sub sample)
    
    unsigned long sub_pkt_cnt = 0; //Need to write the number of sub pkt
    unsigned long pkt_offset = 0;  //current sub pkt offset
    unsigned long sub_pkt_info_max_len = 0;  //sub pkt header information malloc maximum length
    
    p_sub_info = &p_pkt->subsampe_enc_info;
    p_byte_info = p_sub_info->p_subsample_byte_info;
    
    // Every sub sample has clear & encrypted two sub-pkt
    sub_pkt_info_max_len = 2 * p_sub_info->ul_subsample_count * sizeof(aui_es_sub_pkt_info);
    
    //put pkt info after es desc
    p_sub_pkt_info = (aui_es_sub_pkt_info *)(p_dst + sizeof(aui_es_data_desc));
    //pkt length has exceeded the maximum size
    if( sub_pkt_info_max_len> max_write_size) {
        AUI_DBG("sub_pkt_info_max_len: %d, max_write_size: %d\n",sub_pkt_info_max_len,max_write_size);  
        pthread_mutex_unlock(&es_blk_mutex);
        aui_rtn(AUI_RTN_FAIL, "es is desc too big\n");
    }  

    memset(p_sub_pkt_info, 0, sub_pkt_info_max_len);
    
    //Caculate total clear buf length and enc buf length
    for(i=0, j = 0;i<p_sub_info->ul_subsample_count; i++) {
    
        // Print every sub-sample's byte info
        AUI_DBG("%lu clear: %lu, enc: %lu\n", i,
                p_byte_info[i].bytes_of_clear_data,
                p_byte_info[i].bytes_of_encrypted_data);
        // Clear sub pkt
        if (p_byte_info[i].bytes_of_clear_data > 0) {
            p_sub_pkt_info[j].encrypted_flag = 0;
            p_sub_pkt_info[j].sub_pkt_offset = pkt_offset;
            p_sub_pkt_info[j].sub_pkt_length= (unsigned long)p_byte_info[i].bytes_of_clear_data;
            pkt_offset += (unsigned long)p_byte_info[i].bytes_of_clear_data;
  
            j++;
        }
    
        // Encrypted sub pkt
        if (p_byte_info[i].bytes_of_encrypted_data > 0) {
            p_sub_pkt_info[j].encrypted_flag = 1;
            p_sub_pkt_info[j].sub_pkt_offset = pkt_offset;
            p_sub_pkt_info[j].sub_pkt_length= p_byte_info[i].bytes_of_encrypted_data;
            pkt_offset += p_byte_info[i].bytes_of_encrypted_data; 
            
            j++;
        }
    }
    
    sub_pkt_cnt= j;
    
    //AUI_DBG("clear_data_len: 0x%08x, encrypted_data_len: 0x%08x\n", clear_data_len, encrypted_data_len);
    
    // description header total length: struct es_data_desc + struct es_sub_pkt_info array
    desc_header_len = sizeof(aui_es_data_desc) + (sub_pkt_cnt*sizeof(aui_es_sub_pkt_info));
    
    // Write Description header firstly
    desc_header.pkt_enc_type = AUI_ES_PKT_TYPE_PART_ENCRYPTED;
    desc_header.desc_header_length = desc_header_len;
    desc_header.es_data_length = size;
    desc_header.sub_pkt_count = sub_pkt_cnt;
                
    // Copy fixed desc_header
    memcpy(p_dst, &desc_header, sizeof(aui_es_data_desc));


    /** The data head is followed by data*/
    p_dst = (unsigned char*)av_hdl->mmap_addr + desc_header_len;
    for(k=0; wrote_bytes<size; k++) {
        AUI_DBG("Current packet number :%lu\n",k);
        remain_bytes = size-wrote_bytes;   
        if(0 == k) {
            /** Here you need to subtract the size of the data head*/
            max_write_size = SHARE_MEMORY_LEN - desc_header.desc_header_length;
        } else {
            max_write_size = SHARE_MEMORY_LEN;
            /** Only the first packet has desc*/
            desc_header_len = 0;  
        }
        to_be_wrote = (remain_bytes>=max_write_size)?max_write_size:remain_bytes;
        AUI_DBG("==================================================");
        AUI_DBG("pkt_enc_type %X", desc_header.pkt_enc_type);
        AUI_DBG("desc_header_len %lu", desc_header.desc_header_length);
        AUI_DBG("es_data_length %lu", desc_header.es_data_length);
        AUI_DBG("sub_count %lu", desc_header.sub_pkt_count);
        AUI_DBG("to_be_wrote %lu", to_be_wrote);
        AUI_DBG("==================================================");
        memcpy(p_dst, (unsigned char*)(buf+wrote_bytes), to_be_wrote);        
        p_pkt->size = size;
        #if 0
        unsigned long loop=0;
        unsigned long sum_check=0;
        for(loop=0;loop<desc_header_len + to_be_wrote;loop++) {
            sum_check+= (*((unsigned char *)av_hdl->mmap_addr+loop));
        }
        printf("{auilength: %lu,sum_check: %lu}\n",desc_header_len + to_be_wrote,sum_check);
        #endif
        AUI_DBG("av_hdl->mmap_addr 0x%x", (unsigned char *)av_hdl->mmap_addr);
        ret = aui_dsc_pvr_decrypt_block_data_subsample(av_hdl->ali_pvr_hdl, de_hdl_magic->ali_pvr_de_hdl, 
            (const unsigned char *)av_hdl->mmap_addr, desc_header_len + to_be_wrote, 0,
            p_pkt->iv_ctr, p_pkt->iv_length,PVR_OTT_DATA_VBV);
        /** Adjust the data pointer*/
        p_dst = (unsigned char*)av_hdl->mmap_addr;
        wrote_bytes += to_be_wrote;
    }  
    pthread_mutex_unlock(&es_blk_mutex);
    AUI_DBG("Leave");
    return ret;
}

static int write_video_block_es_data(const aui_handleAV *av_hdl, aui_av_packet *p_pkt, 
        const unsigned char *buf, const unsigned long size)
{
	struct aui_av_packet pkt_header;
	memset(&pkt_header,0x0,sizeof(struct aui_av_packet));
	unsigned int pkt_write_fail = 0;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	
	if ((!av_hdl) || (!p_pkt) || (!buf) || (size <= 0)) {
		aui_rtn(AUI_MODULE_AV, "invalid input arg\n");
	}
	
    if (av_hdl->attrAV.st_av_info.b_video_enable == 0) {
        aui_rtn(AUI_RTN_EINVAL, "av_hdl->attrAV.st_av_info.b_video_enable says no video data for this play.");
    }
	
	if(p_pkt->subsampe_enc_info.ul_subsample_count != 0) {
        //it must be avc sub_sample case.
        return write_video_block_es_data_subsample((aui_handleAV *)av_hdl, p_pkt, buf, size);
        //return write_avc_block_es_data(av_hdl, p_pkt, buf, size);
    } 
    //it must be one full sample of mpeg video
    pkt_header.dts = p_pkt->dts;
    pkt_header.pts = p_pkt->pts;
    pkt_header.size = p_pkt->size;

    //write header
    do {
        ret = aui_video_decoder_write_header(inject_param.video_decoder_out, &pkt_header);
        if ( AUI_RTN_SUCCESS != ret) {
            usleep(USLEEP_TIME);
            pkt_write_fail++;
        }
    } while( (ret != AUI_RTN_SUCCESS)&&(pkt_write_fail<=WRITE_ERROR_MAX));
    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("fail\n");
        return -1;
    }
    ret = write_block_es_data_by_dsc(av_hdl, p_pkt, buf, size, PVR_OTT_DATA_VBV);
    return ret;
}

static int write_audio_block_es_data(const aui_handleAV *av_hdl, aui_av_packet *p_pkt, 
        const unsigned char *buf, const unsigned long size)
{
    struct aui_av_packet pkt_header;
    memset(&pkt_header,0x0,sizeof(struct aui_av_packet));
    unsigned int pkt_write_fail = 0;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    pkt_header.dts = p_pkt->dts;
    pkt_header.pts = p_pkt->pts;
    pkt_header.size = p_pkt->size;

    //write header
    do {
        ret = aui_audio_decoder_write_header(inject_param.audio_decoder_out, &pkt_header);
        if ( AUI_RTN_SUCCESS != ret) {
            usleep(USLEEP_TIME);
            pkt_write_fail++;
        } 
    } while( (ret != AUI_RTN_SUCCESS) && (pkt_write_fail<=WRITE_ERROR_MAX));
    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("fail\n");
        return -1;
    }

    //write es data
    ret = write_block_es_data_by_dsc(av_hdl, p_pkt, buf, size, PVR_OTT_DATA_AUDIO);
    return ret;
}
static AUI_RTN_CODE aui_av_write_es(const aui_handleAV *av_hdl,
        aui_av_inject_packet_info_t *packet_info, const unsigned char *buf,
        const unsigned long size) {

    struct aui_av_packet pkt_header;
    memset(&pkt_header,0x0,sizeof(struct aui_av_packet));

    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (!packet_info) {
        aui_rtn(AUI_RTN_EINVAL, "invalid input arg\n");
    }

    pkt_header.dts = packet_info->buffer_info.dts;
    pkt_header.pts = packet_info->buffer_info.pts;
    pkt_header.size = packet_info->buffer_info.size;
    pkt_header.flag = packet_info->buffer_info.flag;
    
    if ((packet_info->buffer_channel == AUI_AV_BUFFER_CHANNEL_VID_DECODER)
            && (av_hdl->attrAV.st_av_info.b_video_enable)) {
        do {
            ret = aui_video_decoder_write_header(inject_param.video_decoder_out,
                    &pkt_header);

            if (ret != AUI_RTN_SUCCESS) {
                usleep(20 * 1000);
            } else {
                do {
                    ret = aui_video_decoder_write(
                            inject_param.video_decoder_out, buf, size);
                    if ( AUI_RTN_SUCCESS != ret) {
                        usleep(20 * 1000);
                    }
                } while (ret != AUI_RTN_SUCCESS);
            }
        } while (ret != AUI_RTN_SUCCESS);

    } else if ((packet_info->buffer_channel == AUI_AV_BUFFER_CHANNEL_AUD_DECODER)
            && (av_hdl->attrAV.st_av_info.b_audio_enable)) {
        do {
            ret = aui_audio_decoder_write_header(inject_param.audio_decoder_out,
                    &pkt_header);
            if ( AUI_RTN_SUCCESS != ret) {
                usleep(10 * 1000);
            } else {
                do {
                    ret = aui_audio_decoder_write(
                            inject_param.audio_decoder_out, buf, size);
                    if ( AUI_RTN_SUCCESS != ret) {
                        usleep(10 * 1000);
                    }
                } while (ret != AUI_RTN_SUCCESS);
            }
        } while (ret != AUI_RTN_SUCCESS);
    } else if(packet_info->buffer_channel == AUI_AV_BUFFER_CHANNEL_VID_DECODER_FOR_ENCRYPTED_DATA) {
        write_video_block_es_data(av_hdl, &(packet_info->buffer_info), buf, size);
    } else if(packet_info->buffer_channel == AUI_AV_BUFFER_CHANNEL_AUD_DECODER_FOR_ENCRYPTED_DATA) {
        write_audio_block_es_data(av_hdl, &(packet_info->buffer_info), buf, size);
    }
    packet_info = NULL;
    buf = NULL;
    return AUI_RTN_SUCCESS;
}

#define DE_NOMEM_ERROR (-0x1008)
static AUI_RTN_CODE aui_av_write_block_ts_data(aui_hdl hdl,  const unsigned char *buf, int size, aui_av_packet *av_pkt)
{
    aui_handleAV *av_handle = (aui_handleAV *)hdl;
    aui_hdl dmx_hdl = NULL;
    
    dmx_hdl = av_handle->attrAV.pv_hdl_dmx;
    aui_hdl ali_sl_hdl =NULL;
    unsigned int ali_pvr_de_hdl = 0;
    unsigned int block_size = 0, tmp_block_size = 0;
    unsigned int dsc_process_mode = 0;
    unsigned int mmap_addr = 0;
    unsigned int mmap_len = 0;
    aui_dmx_ali_pvr_de_hdl_get(dmx_hdl, &ali_pvr_de_hdl, &block_size, &dsc_process_mode, &mmap_addr, &mmap_len, &ali_sl_hdl);
    if (block_size) {
        //for pvr blockmode playback, app must make sure the buffer size is integer times of current block size.
        if ((0 == ali_pvr_de_hdl) || (block_size>mmap_len) ||(size%block_size)) {
            AUI_ERR("invalid env: 0x%08x, 0x%08x, 0x%08x, %d\n",
                ali_pvr_de_hdl, mmap_len, block_size, size);
            return AUI_RTN_FAIL;
        }
        tmp_block_size = block_size;
    } else {
        //for hls ts of ott, block size is the input argument size.
        if ((0 == mmap_addr) ||(0==mmap_len)) {
            AUI_ERR("invalid env: %d,%d, %d\n", ali_pvr_de_hdl, block_size, size);
            return AUI_RTN_FAIL;
        }
        //write as many as possible, avoid mmap_len is not aligned with 188 bytes.
        tmp_block_size = (mmap_len/188)*188;
        //AUI_DBG("%p, mmap_len: %u, tmp_block_size %u\n", (void *)mmap_addr, mmap_len, tmp_block_size);
    }
    unsigned int got_len = 0;
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    int req_buf_error_cnt = 0;
    int de_nomem_error_cnt = 0;
    int i=0;
    unsigned long vob_indicator_buffer_size = BLOCK_VOB_BUFFER_SIZE; //2*188;

    int wrote_bytes = 0;
    int to_be_wrote = 0;
    int remain_bytes = size;
    //AUI_DBG("block_cnt: %d\n", block_cnt);
    for(i=0; wrote_bytes<size; i++) {
        req_buf_error_cnt = 0;
        de_nomem_error_cnt = 0;
        remain_bytes = size-wrote_bytes;
        to_be_wrote = (remain_bytes>=(int)tmp_block_size)?(int)tmp_block_size:remain_bytes;
        while(de_nomem_error_cnt<WRITE_ERROR_MAX){
            ret = aui_dmx_ioctl_priv(dmx_hdl, AUI_DMX_M2S_BUF_REQ_SIZE, &got_len);
            //2*188 is used for trick play of pvrlib. Ali pvr will fill this 2 ts package
            if ((ret != AUI_RTN_SUCCESS) || (got_len <= (to_be_wrote+vob_indicator_buffer_size))){
                usleep(USLEEP_TIME);
                req_buf_error_cnt++;
                if(req_buf_error_cnt<=WRITE_ERROR_MAX)
                    continue;
                else {
                    AUI_ERR("AUI_DMX_M2S_BUF_REQ_SIZE fail, %d\n", req_buf_error_cnt);
                    return AUI_RTN_FAIL;
                }
            } else {
                //AUI_DBG("AUI_DMX_M2S_BUF_REQ_SIZE %d success\n", req_buf_error_cnt);
                //break;
            }
        
            /*
            All parameter is prepared for ali_pvr.
            */
            memcpy((void *)mmap_addr, buf+wrote_bytes, to_be_wrote);
            if(block_size)
                ret = aui_dsc_pvr_decrypt_block_data(ali_sl_hdl, ali_pvr_de_hdl, (const unsigned char *)mmap_addr, to_be_wrote, 
                                    av_pkt->pts+i, NULL, 0, PVR_OTT_DATA_DMX);
            else
                ret = aui_dsc_pvr_decrypt_block_data(ali_sl_hdl, ali_pvr_de_hdl, (const unsigned char *)mmap_addr, to_be_wrote, 
                                    0, av_pkt->iv_ctr, av_pkt->iv_length, PVR_OTT_DATA_DMX);
            if (ret != AUI_RTN_SUCCESS) {
                if(DE_NOMEM_ERROR == (int)ret) {
                    de_nomem_error_cnt++;
                    req_buf_error_cnt = 0;
                    //AUI_DBG("config block data to decrypt fail for dis_continue dsc buffer!\n");
                } else {
                    AUI_ERR("config block data %d bytes to decrypt fail, ret: %lu!\n",
                        to_be_wrote, ret);
                    return AUI_RTN_FAIL;
                }
            }
            else 
                break;
        }
        //For ott project, there may be some padding bytes in the encrypted ts data. So the size would not align with 188.
        //However, we should make sure the data sent to dmx driver should align with 188 after the data is decrypt successfully
        unsigned int real_ret_size = to_be_wrote-to_be_wrote%188;
        ret = aui_dmx_ioctl_priv(dmx_hdl, AUI_DMX_M2S_BUF_RET_SIZE, (void *)(real_ret_size+vob_indicator_buffer_size));
        if (ret != AUI_RTN_SUCCESS){
            AUI_ERR("AUI_ALI_DMX_SEE_MAIN2SEE_BUF_RET_SIZE fail!\n");
        }
        wrote_bytes += to_be_wrote;
    }
        
    return ret;
}
static AUI_RTN_CODE aui_av_write_ts(void *pv_handleAV,
        aui_av_inject_packet_info_t *packet_info, const unsigned char *buf,
        unsigned long size) 
{
    unsigned char *start_buf = NULL;
    unsigned int to_write = 0;
    unsigned int frame_size = 0;
    unsigned long writed_len = 0;
    unsigned long todo = size;
    unsigned long dmx_pkt_cnt = 0, tsg_use_pkt = 0;
    aui_attr_tsg attr;
    AUI_RTN_CODE ret;
    aui_handleAV *av_handle = NULL;
    aui_av_inject_settings_t *stream_type = NULL;
    aui_dmx_write_playback_param_t p_dmx2_write_param;

    if((packet_info)&&(AUI_AV_BUFFER_CHANNEL_DMX_FOR_BLOCK_ENCRYPTED_DATA == packet_info->buffer_channel)){
        return aui_av_write_block_ts_data(pv_handleAV, buf, size, &packet_info->buffer_info);
    }

    av_handle = (aui_handleAV *) pv_handleAV;
    stream_type = &(((aui_handleAV *) (pv_handleAV))->attrAV.stream_type);
    todo = size;

    if (!av_handle->attrAV.pv_hdl_dmx) {
        aui_rtn(AUI_RTN_EINVAL, "inject dmx handle invalid\n");
    }

    start_buf = (unsigned char *) buf;
    if ((stream_type->dmx_id == AUI_DMX_ID_SW_DEMUX0) || (stream_type->dmx_id == AUI_DMX_ID_SW_DEMUX1)) {
        while (todo) {
            to_write = todo;
            if (to_write > 1024 * 188)
                to_write = 1024 * 188;
            do {
                p_dmx2_write_param.start_buf = start_buf;
                p_dmx2_write_param.length = to_write;
                ret = aui_dmx_set(av_handle->attrAV.pv_hdl_dmx,
                        AUI_DMX_SET_DIRECT_WRITE, &p_dmx2_write_param);
                if (AUI_RTN_SUCCESS != ret) {
                    usleep(10 * 1000);
                }
                if (AUI_RTN_SUCCESS
                        != aui_dmx_get(av_handle->attrAV.pv_hdl_dmx,
                                AUI_DMX_GET_WRITE_BUF_LEN, &writed_len)) {
                    aui_rtn(AUI_RTN_EINVAL, "playback demux get length failed\n");
                }
            } while ((ret != AUI_RTN_SUCCESS) || (writed_len != to_write));

            todo -= writed_len;
            start_buf += writed_len;
        }
    } else if (stream_type->dmx_id == AUI_DMX_ID_DEMUX0
            || stream_type->dmx_id == AUI_DMX_ID_DEMUX1) {
        while (todo) {
            frame_size = 188;
            to_write = todo / frame_size;
            to_write = to_write * frame_size;
//            to_write = todo; // Remove 188 alignment
            if (to_write == 0)
                break;
            if (to_write > 1024 * 188)
                to_write = 1024 * 188;
            do {
                aui_tsg_check_remain_pkt(av_handle->attrAV.pv_hdl_tsg,
                        &tsg_use_pkt);
                aui_dmx_get(av_handle->attrAV.pv_hdl_dmx,
                        AUI_DMX_GET_FREE_BUF_LEN, (void *) &dmx_pkt_cnt);
                usleep(1000);
            } while ((dmx_pkt_cnt) < (to_write / 188 + tsg_use_pkt));

            /* push data in TSG */
            attr.ul_addr = start_buf;
            attr.ul_pkg_cnt = to_write / 188;
            attr.uc_sync_mode = 0;
            if ( AUI_RTN_SUCCESS
                    != aui_tsg_send(av_handle->attrAV.pv_hdl_tsg, &attr)) {
                to_write = 0;
            }
            todo -= to_write;
            start_buf += to_write;
        }
    }

    return AUI_RTN_SUCCESS;

}

AUI_RTN_CODE aui_av_write(aui_hdl pv_handleAV,
        aui_av_inject_packet_info_t *packet_info, const unsigned char *buf,
        unsigned long size) {

    AUI_RTN_CODE ret;
    aui_handleAV *av_handle;
    aui_av_inject_settings_t *stream_type = NULL;
    av_handle = NULL;
    ret = AUI_RTN_SUCCESS;

    if (!pv_handleAV) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    if ((packet_info) && (AUI_AV_PACKET_EOS == packet_info->buffer_info.flag)) {
        //to handle eos packet in future.
        AUI_DBG("the eos packet, ignore now\n");
        return 0;
    }
    if ((!buf) ||(size<=0)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    av_handle = (aui_handleAV *) pv_handleAV;
    stream_type =
            ((aui_av_inject_settings_t*) &(av_handle->attrAV.stream_type));

    if (stream_type->data_type == AUI_AV_DATA_TYPE_RAM_TS) {

        if (!av_handle->attrAV.pv_hdl_dmx) {
            aui_rtn(AUI_RTN_EINVAL,
                    "inject dmx handle invalid\n");
        }

        ret = aui_av_write_ts(av_handle, packet_info, buf, size);
        if (AUI_RTN_SUCCESS != ret) {
            aui_rtn(AUI_RTN_EINVAL, "aui_av_write failed\n");
        }
    } else if (stream_type->data_type == AUI_AV_DATA_TYPE_ES) {

        if (AUI_RTN_SUCCESS
                != aui_av_write_es(av_handle, packet_info, buf, size)) {
            aui_rtn(AUI_RTN_EINVAL, "aui_av_write failed\n");
        }
    }
    packet_info = NULL;
    buf = NULL;	
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_av_trickmode_status_get(aui_hdl pv_handleAV, aui_av_trickmode_status *status) {
    aui_handleAV *av_handle;
    AUI_RTN_CODE ret = 0;

    if (NULL == pv_handleAV || NULL == status) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    av_handle = (aui_handleAV *) pv_handleAV;
    
    unsigned int ali_pvr_de_hdl = 0;
    unsigned int block_size = 0;
    unsigned int dsc_process_mode = 0;
    aui_dmx_ali_pvr_de_hdl_get(av_handle->attrAV.pv_hdl_dmx, &ali_pvr_de_hdl, &block_size, &dsc_process_mode, NULL, NULL, NULL);

    if(AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == dsc_process_mode) {
        if((block_size==0) ||(block_size%188)) {
            aui_rtn(AUI_RTN_EINVAL, "wrong env\n");
        }
    } else {
        block_size = 47*1024;
    }
    status->ul_chunk_size = 1 * block_size;
    status->action = AUI_AV_TRICK_MODE_ACTION_MORE_DATA;

    if (av_handle->trickmode_param.ul_bitrate < 5 * 1000000)
        status->ul_chunk_size = 1 * block_size;
    else if (av_handle->trickmode_param.ul_bitrate < 10 * 1000000)
        status->ul_chunk_size = 2 * block_size;
    else if (av_handle->trickmode_param.ul_bitrate < 20 * 1000000)
        status->ul_chunk_size = 3 * block_size;
    else
        status->ul_chunk_size = 4 * block_size;

    if ((av_handle->trickmode_param.speed > AUI_PLAYBACK_SPEED_8)
        || (av_handle->trickmode_param.direction == AUI_PLAYBACK_BACKWARD)) {
            int if_cb_err= 0;
            int if_new_frame_dis = 0;
            aui_decv_trickinfo_get(av_handle->attrAV.pv_hdl_decv, &if_cb_err, &if_new_frame_dis);
            //AUI_DBG("if_cb_err: %d, if_new_frame_dis: %d\n", if_cb_err, if_new_frame_dis);
            /*
            if new frame is displayed or decoded error, let app seek and feed next chunk.
            */
            if((if_new_frame_dis) ||(if_cb_err)) {
                status->action = AUI_AV_TRICK_MODE_ACTION_NEW_FRAME;            
                aui_decv_buffer_reset(av_handle->attrAV.pv_hdl_decv, VBV_BUF_RESET);
            }
    }
    else {
        status->action = AUI_AV_TRICK_MODE_ACTION_MORE_DATA;
    }
    return ret;
}

AUI_RTN_CODE aui_av_trickmode_set(aui_hdl pv_handleAV, aui_av_trickmode_params *params) {
    aui_handleAV *av_handle;
    AUI_RTN_CODE ret = 0;
    enum aui_decv_vbv_buf_mode vbv_buf_mode = AUI_DECV_VBV_BUF_BLOCK_FULL_MODE;

    if ((NULL == pv_handleAV) ||(NULL == params)) {
        aui_rtn(AUI_RTN_EINVAL, "null pointer param");
    }

    av_handle = (aui_handleAV *) pv_handleAV;

    if ((av_handle->trickmode_param.speed != params->speed) ||
        (av_handle->trickmode_param.mode != params->mode) ||
        (av_handle->trickmode_param.direction != params->direction)) {

        if (params->speed > AUI_PLAYBACK_SPEED_1) {
            int dmx_speed = FAST_PLAYBACK_SPEED_FLAG;//It`s just flag what is distinct from normal playback.
            aui_dmx_set(av_handle->attrAV.pv_hdl_dmx, AUI_DMX_SET_PLAYBACK_SPEED, &dmx_speed);
            aui_decv_sync_mode(av_handle->attrAV.pv_hdl_decv, AUI_STC_AVSYNC_FREERUN);

            aui_decv_chg_mode_set(av_handle->attrAV.pv_hdl_decv, AUI_DECV_CHG_STILL);
            aui_decv_stop(av_handle->attrAV.pv_hdl_decv);
            aui_decv_start(av_handle->attrAV.pv_hdl_decv);
            aui_decv_set(av_handle->attrAV.pv_hdl_decv, AUI_DECV_SET_VBV_BUF_MODE, &vbv_buf_mode);

            if ((params->speed <= AUI_PLAYBACK_SPEED_8) && (params->direction == AUI_PLAYBACK_FORWARD)) {
                ///* stream is injected in continous chunks , speed is controlled totally by vdec driver.*/
                ret = aui_decv_trickmode_set(av_handle->attrAV.pv_hdl_decv, params->speed, params->direction, VDEC_PLAYBACK_METHOD_CONTINUOUS);
                if (AUI_RTN_SUCCESS != ret) {
                    aui_rtn(AUI_RTN_EINVAL, "set continue mode fail\n");
                }
            } else {
                ///* stream is injected in non-continous chunks */
                ret = aui_decv_trickmode_set(av_handle->attrAV.pv_hdl_decv, AUI_PLAYBACK_SPEED_8, params->direction, VDEC_PLAYBACK_METHOD_NONCONTINUOUS);
                if (AUI_RTN_SUCCESS != ret) {
                    aui_rtn(AUI_RTN_EINVAL, "set dis-continue mode fail\n");
                }
            }
        } else {
            aui_decv_chg_mode_set(av_handle->attrAV.pv_hdl_decv, AUI_DECV_CHG_STILL);
            aui_decv_stop(av_handle->attrAV.pv_hdl_decv);
            aui_decv_start(av_handle->attrAV.pv_hdl_decv);
            aui_decv_set(av_handle->attrAV.pv_hdl_decv, AUI_DECV_SET_VBV_BUF_MODE, &vbv_buf_mode);
            ret = aui_decv_trickmode_set(av_handle->attrAV.pv_hdl_decv, AUI_PLAYBACK_SPEED_1, AUI_PLAYBACK_FORWARD, VDEC_PLAYBACK_METHOD_CONTINUOUS);

            if (AUI_RTN_SUCCESS != ret) {
                aui_rtn(AUI_RTN_EINVAL, "set local play fail\n");
            }
        }
    }

    av_handle->trickmode_param.mode = params->mode;
    av_handle->trickmode_param.speed = params->speed;
    av_handle->trickmode_param.direction = params->direction;
    av_handle->trickmode_param.ul_bitrate = params->ul_bitrate;

    return 0;
}

AUI_RTN_CODE aui_av_buffer_status_get(aui_hdl pv_hdlAV,
        aui_av_buffer_channel buffer_channel, aui_av_buffer_status *status) 
{

    aui_handleAV *av_handle = NULL;
    aui_decoder_buffer_status buffer_status;
    aui_hdl pv_hdl_dmx = NULL;
    aui_av_inject_settings_t *p_av_setting = NULL;

    if ((NULL == pv_hdlAV) || (NULL == status)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
        
    av_handle = (aui_handleAV *) pv_hdlAV;
    p_av_setting = &(av_handle->attrAV.stream_type);

    if ((p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)
            || (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS)) {
        pv_hdl_dmx = av_handle->attrAV.pv_hdl_dmx;
    }

    memset(&buffer_status, 0x0, sizeof(aui_decoder_buffer_status));

    if (((buffer_channel == AUI_AV_BUFFER_CHANNEL_AUD_DECODER)
            ||(buffer_channel == AUI_AV_BUFFER_CHANNEL_AUD_DECODER_FOR_ENCRYPTED_DATA))
            && (av_handle->attrAV.st_av_info.b_audio_enable)) {
        if (AUI_RTN_SUCCESS != aui_audio_decoder_get_buffer_status(
                inject_param.audio_decoder_out, &buffer_status))
            aui_rtn(AUI_RTN_EINVAL, "fail to get a status\n");

        status->free_size = buffer_status.free_size;
        status->total_size = buffer_status.total_size;
        status->valid_size = buffer_status.valid_size;
    } else if (((buffer_channel == AUI_AV_BUFFER_CHANNEL_VID_DECODER)
                    ||(buffer_channel == AUI_AV_BUFFER_CHANNEL_VID_DECODER_FOR_ENCRYPTED_DATA))
                    && (av_handle->attrAV.st_av_info.b_video_enable)) {
        if (AUI_RTN_SUCCESS != aui_video_decoder_get_buffer_status(
                inject_param.video_decoder_out, &buffer_status))
            aui_rtn(AUI_RTN_EINVAL, "fail to get a status\n");

        status->free_size = buffer_status.free_size;
        status->total_size = buffer_status.total_size;
        status->valid_size = buffer_status.valid_size;
    } else if ((buffer_channel == AUI_AV_BUFFER_CHANNEL_DMX)
                && ((p_av_setting->data_type == AUI_AV_DATA_TYPE_RAM_TS)
                || (p_av_setting->data_type == AUI_AV_DATA_TYPE_NIM_TS))) {

        if (AUI_RTN_SUCCESS != aui_dmx_get(pv_hdl_dmx, AUI_DMX_GET_FREE_BUF_LEN,
                (unsigned long*) &buffer_status.free_size))
            aui_rtn(AUI_RTN_EINVAL,
                "fail to get a AUI_DMX_GET_FREE_BUF_LEN\n");

        if (AUI_RTN_SUCCESS != aui_dmx_get(pv_hdl_dmx, AUI_DMX_GET_TOTAL_BUF_LEN,
                (unsigned long*) &buffer_status.total_size))
            aui_rtn(AUI_RTN_EINVAL,
                "fail to get a AUI_DMX_GET_TOTAL_BUF_LEN\n");

        if ((p_av_setting->dmx_id != AUI_DMX_ID_SW_DEMUX0) && (p_av_setting->dmx_id != AUI_DMX_ID_SW_DEMUX1)) {
            //hw dmx unit is 188, note hw dmx not support total size right now
            buffer_status.free_size = buffer_status.free_size * 188;
            buffer_status.total_size = buffer_status.total_size * 188;
        }

        status->free_size = buffer_status.free_size;
        status->total_size = buffer_status.total_size;
        buffer_status.valid_size = buffer_status.total_size - buffer_status.free_size;
        status->valid_size = buffer_status.valid_size;
    } else if(AUI_AV_BUFFER_CHANNEL_DMX_FOR_BLOCK_ENCRYPTED_DATA == buffer_channel) {
        aui_dmx_ioctl_priv(pv_hdl_dmx, AUI_DMX_M2S_BUF_REQ_SIZE, &status->free_size);
        aui_dmx_ioctl_priv(pv_hdl_dmx, AUI_DMX_M2S_BUF_VALIDSIZE_GET, &status->total_size);
        status->valid_size = status->total_size-status->free_size;
    } else {
        aui_rtn(AUI_RTN_EINVAL, "buffer channel unknown ...\n");
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_av_dsc_context_set (aui_hdl av_hdl, aui_av_dsc_context *p_dsc_context) 
{
    aui_handleAV *av_handle = NULL;
    if((!av_hdl)||(!p_dsc_context)) {
        aui_rtn(AUI_RTN_EINVAL, "invalid input arg\n");
    }
    if ((NULL == p_dsc_context->p_audio_dsc_id) && (NULL == p_dsc_context->p_video_dsc_id))
        return AUI_RTN_SUCCESS;
    av_handle = av_hdl;
    memcpy(&av_handle->en_es_ctx, p_dsc_context, sizeof(aui_av_dsc_context));
    AUI_DBG("v: %p, a: %p\n", av_handle->en_es_ctx.p_video_dsc_id,
        av_handle->en_es_ctx.p_audio_dsc_id);
    //we should store aui_dsc_resource_id information to avoid local user variable.
    if (p_dsc_context->p_audio_dsc_id)
        memcpy(&av_handle->audio_dsc_res_id, p_dsc_context->p_audio_dsc_id, sizeof(aui_dsc_resource_id));
    if (p_dsc_context->p_video_dsc_id)
        memcpy(&av_handle->video_dsc_res_id, p_dsc_context->p_video_dsc_id, sizeof(aui_dsc_resource_id));
    if(aui_dsc_pvr_mmap(&av_handle->ali_pvr_hdl, &av_handle->mmap_addr, &av_handle->mmap_len)) {
        AUI_ERR("fail\n");
        return -1;
    }
    av_handle->mmap_flag = 1;
    return AUI_RTN_SUCCESS;
}

