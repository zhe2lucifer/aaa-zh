/**@file
 *    @brief         AUI AV INJECTER module interface implement
 *    @author        christian.xie
 *    @date            2014-3-12
 *     @version         1.0.0
 *    @note            ali corp. all rights reserved. 2013~2999 copyright (C)
 */
/****************************INCLUDE HEAD FILE************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <signal.h>

#include "aui_common_priv.h"
#include <aui_av_injecter.h>
#include <aui_decv.h>
#include <aui_snd.h>
#include <aui_av.h>

#include <alislsnd.h>
#include <alislvdec.h>
#include <alislavsync.h>
#include <alislsbm.h>
#include <alisldis.h>

#include "aui_decv_priv.h"
#include "aui_deca_priv.h"


AUI_MODULE(AV_INJECT)

//AV_PKT_FLAG_CODEC_DATA=0x10000000
#define AUI_AV_PKT_FLAG_CODEC_DATA 0x10000000

typedef enum ES_DATA_TYPE {
    ES_DATA_TYPE_CLEAR = 0,
    ES_DATA_TYPE_FULL_SAMP,
    ES_DATA_TYPE_SUB_SAMP,
}ES_DATA_TYPE;
#define VBV_BUF_FOR_OTT_VIDEO_SIZE  (0x380000) //3.5M
#define VBV_BUF_FOR_OTT_AUDIO_SIZE     (0x80000) //0.5M

/****************************LOCAL MACRO******************************************/
struct aui_av_packet_complete {
    /**
     * Presentation timestamp in AVStream->time_base units; the time at which
     * the decompressed packet will be presented to the user.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     * pts MUST be larger or equal to dts as presentation cannot happen before
     * decompression, unless one wants to view hex dumps. Some formats misuse
     * the terms dts and pts/cts to mean something different. Such timestamps
     * must be converted to true pts/dts before they are stored in AVPacket.
     */
    long long pts;
    /**
     * Decompression timestamp in AVStream->time_base units; the time at which
     * the packet is decompressed.
     * Can be AV_NOPTS_VALUE if it is not stored in the file.
     */
    long long dts;
    unsigned char *data;
    long size;
    long stream_index;
    long flags;
    unsigned short width, height;
    long param_change_flags;

    /**
     * Duration of this packet in AVStream->time_base units, 0 if unknown.
     * Equals next_pts - this_pts in presentation order.
     */
    long duration;
    void (*destruct)(struct aui_av_packet_complete *);
    void *priv;
    long long pos; ///< byte position in stream, -1 if unknown
    long long convergen;
    unsigned char nalu_num;
};

typedef struct aui_av_packet_complete aui_av_packet_complete_t;

struct aui_decore_buffer {
    int vpkt_hdr_sbm;   // video(0 or 1) header sbm
    unsigned int vpkt_hdr_addr;
    unsigned int vpkt_hdr_size;
    unsigned int vpkt_hdr_reserve_size; // buffer size to keep empty in the sbm ring buffer, to avoid overlap
    int vpkt_data_sbm;  // video(0 or 1) data sbm
    unsigned int vpkt_data_addr;
    unsigned int vpkt_data_size;
    unsigned int vpkt_data_reserve_size;
    int vdec_out_sbm;  // no used anymore
    unsigned int vdec_out_addr;
    unsigned int vdec_out_size;
    unsigned int vdec_out_reserve_size;
    int vsink_in_sbm;  // no used anymore
    unsigned int vsink_in_addr;
    unsigned int vsink_in_size;
    unsigned int vsink_in_reserve_size;
    int apkt_hdr_sbm;   // audio header sbm, can be the same with apkt_data_sbm
    unsigned int apkt_hdr_addr;
    unsigned int apkt_hdr_size;
    unsigned int apkt_hdr_reserve_size;
    int apkt_data_sbm;  // audio data sbm, can be the same with apkt_hdr_sbm
    unsigned int apkt_data_addr;
    unsigned int apkt_data_size;
    unsigned int apkt_data_reserve_size;
    int adec_out_sbm;  // no used anymore
    unsigned int adec_out_addr;
    unsigned int adec_out_size;
    unsigned int adec_out_reserve_size;
    int asink_in_sbm;  // no used anymore
    unsigned int asink_in_addr;
    unsigned int asink_in_size;
    unsigned int asink_in_reserve_size;
    unsigned int frm_buf_addr;
    unsigned int frm_buf_size;
    unsigned int pcm_buf_addr;
    unsigned int pcm_buf_size;
    int mix_data_sbm;   // for audio mix
    unsigned int mix_data_addr;
    unsigned int mix_data_size;
    unsigned int mix_data_reserve_size;
};

struct aui_private_audio {
    //#73437 get av inject audio handle by aui_find_dev_by_idx
    aui_dev_priv_data dev_priv_data; 
    alisl_handle pkt_hdr_hdl;
    alisl_handle pkt_data_hdl;
    alisl_handle dec_out_hdl;
    alisl_handle sink_in_hdl;
    alisl_handle audio_hdl;
    unsigned int codec_id;
    //when we get audio_hdl from init info, we can't close it
    unsigned char audio_hdl_need_close;
    alisl_handle avsync_hdl;
    unsigned char mix;
};

struct aui_private_video {
    //#73437 get av inject video handle by aui_find_dev_by_idx
    aui_dev_priv_data dev_priv_data;
    alisl_handle pkt_hdr_hdl;
    alisl_handle pkt_data_hdl;
    alisl_handle dec_out_hdl;
    alisl_handle disp_in_hdl;
    alisl_handle video_hdl;
    unsigned int codec_id;
    //when we get video_hdl from init info, we can't close it
    unsigned char video_hdl_need_close;
    alisl_handle avsync_hdl;
};

static const unsigned int av_inject_codec_pcm[2][2][4] = {
    {
        {SND_CODEC_ID_PCM_U8, SND_CODEC_ID_PCM_U16LE, SND_CODEC_ID_PCM_U24LE, SND_CODEC_ID_PCM_U32LE},
        {SND_CODEC_ID_PCM_U8, SND_CODEC_ID_PCM_U16BE, SND_CODEC_ID_PCM_U24BE, SND_CODEC_ID_PCM_U32BE}
    },
    {
        {SND_CODEC_ID_PCM_S8, SND_CODEC_ID_PCM_S16LE, SND_CODEC_ID_PCM_S24LE, SND_CODEC_ID_PCM_S32LE},
        {SND_CODEC_ID_PCM_S8, SND_CODEC_ID_PCM_S16BE, SND_CODEC_ID_PCM_S24BE, SND_CODEC_ID_PCM_S32BE}
    }
};

/****************************LOCAL TYPE*******************************************/

/**
    case: User know the ES data going to play maybe is clear

    Index   Used        Content                     Buffer
    ------------------------------------------------------
    SBM0    video(0)    ES data(Clear)           PE
    SBM0    video(0)    header                   PE

    SBM1    video(1)    ES data(Clear)           PE
    SBM1    video(1)    header                   PE

    SBM4    audio       ES data(Clear)           PE
    SBM4    audio       header                   PE

   case: User know the ES data going to play maybe is encrypted,or clear+encrypted mixed
    
    SBM0    video(0)    ES data(Encrypted or Clear+Encrypted)    VBV
    SBM1    video(1)    ES data(Encrypted or Clear+Encrypted)    VBV  
    
    SBM2    video(0)    header                      PE
    SBM3    video(1)    header                      PE

    SBM4    audio        header                     PE
    SBM5    audio        ES data(Encrypted)                      VBV
    SBM6    -           -                           -
    SBM7    -           -                           -
    SBM8    -           -                           -
    SBM9    audio mix   header+data                 PE
    SBM10   -           -                           -
    SBM11   -           -                           -
*/
static AUI_RTN_CODE aui_decore_alloc_buffer(alisl_handle video_hdl,
    struct aui_decore_buffer *decore_buffer, ES_DATA_TYPE data_type)
{
    struct vdec_mem_info video_mem_info;
    unsigned int pe_mem_addr, pe_mem_size;
    //unsigned int video_priv_addr, video_priv_size;
    unsigned int video_frm_addr, video_frm_size;
    unsigned int vbv_addr, vbv_size;
    unsigned int run_addr, run_size;
    unsigned int run_addr_pe_en = 0, run_size_pe_en = 0;//for encrypted ES
    //unsigned int av_frame_size;
    unsigned char need_close = 0;
    unsigned int total_data_size = 0;
    /* PIP video0: sbm0(header+data), video1: sbm1(header+data), 
        audio: sbm4(header+data), audio mix: sbm9(header+data)
        In vmx ott project, avc data has two parts, encrypted es data and clear NALU header.
        encrypted es data will be decrypted by ali_pvr module. sbm0 is used to store es data 
        decrypted by ali_pvr in driver. sbm2 is used for user to write AVC vdec_av_packet and 
        clear NALU header.
        
    */
    int video_sbm = SBM_ID_SBM0;
    enum vdec_video_id video_id = VDEC_ID_VIDEO_0;

    if (decore_buffer == NULL) {
        return AUI_RTN_EINVAL;
    }

    memset(decore_buffer, 0, sizeof(struct aui_decore_buffer));

    if (video_hdl == NULL) {
        /* default use meminfo get from video 0 
            for audio decoder, or video decoder without aui decv_hdl
        */
        if (alislvdec_open(&video_hdl, 0)) {
            aui_rtn(AUI_RTN_EINVAL, "decoder open failed\n");
            return AUI_RTN_EINVAL;
        }
        need_close = 1;
    } else {
        alislvdec_get_id(video_hdl, &video_id);
        
        if (video_id == VDEC_ID_VIDEO_0) {
            video_sbm = SBM_ID_SBM0;
        } else if (video_id == VDEC_ID_VIDEO_1) {
            video_sbm = SBM_ID_SBM1;
        }
    }
    
    alislvdec_get_mem_info(video_hdl, &video_mem_info);
    //alislvdec_get_av_frame_struct_size(&av_frame_size);
    //video_priv_addr = (unsigned int) video_mem_info.priv_mem_start;
    //video_priv_size = video_mem_info.priv_mem_size;
    video_frm_addr = (unsigned int) video_mem_info.mem_start;
    video_frm_size = video_mem_info.mem_size;
    pe_mem_addr = (unsigned int) video_mem_info.mp_mem_start;
    pe_mem_size = video_mem_info.mp_mem_size;
    vbv_addr = (unsigned int) video_mem_info.vbv_mem_start;
    vbv_size = (unsigned int)video_mem_info.vbv_mem_size;
    
    AUI_DBG(
        "decore use mem: cpu 0x%x 0x%x, see 0x%x 0x%x, mp 0x%x 0x%x\n",
        video_frm_addr, video_frm_size, video_mem_info.priv_mem_start,
    video_mem_info.priv_mem_size, pe_mem_addr, pe_mem_size);
        
    if (need_close)        
        alislvdec_close(video_hdl);

    decore_buffer->vdec_out_size = 0; //for AS ,last: 256 * av_frame_size;
    decore_buffer->vsink_in_size = 0; //for AS ,last: 256 * av_frame_size;
    decore_buffer->apkt_hdr_size = 0; //for AS ,last: 4096 * sizeof(struct vdec_av_packet);
    decore_buffer->adec_out_size = 0; //for AS ,last: 1024 * sizeof(struct snd_audio_frame);
    decore_buffer->asink_in_size = 0; //for AS ,last: 1024 * sizeof(struct snd_audio_frame);

    decore_buffer->vpkt_hdr_reserve_size = sizeof(struct vdec_av_packet);
    decore_buffer->vpkt_data_reserve_size = 1024;
    decore_buffer->vdec_out_reserve_size = 0; //av_frame_size;
    decore_buffer->vsink_in_reserve_size = 0; //av_frame_size;

    decore_buffer->apkt_hdr_reserve_size = 10 * sizeof(struct vdec_av_packet);
    decore_buffer->apkt_data_reserve_size = 128 * 1024;
    decore_buffer->adec_out_reserve_size = 10 * sizeof(struct snd_audio_frame);
    decore_buffer->asink_in_reserve_size = 10 * sizeof(struct snd_audio_frame);

    decore_buffer->mix_data_reserve_size = 0;
        
    if(ES_DATA_TYPE_CLEAR == data_type) {
        run_size = pe_mem_size;
        run_addr = pe_mem_addr;
        if (run_size >= 0xE00000) {
            decore_buffer->vpkt_data_size = 0xA00000;
            decore_buffer->apkt_data_size = 0x300000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0xC00000) {
            decore_buffer->vpkt_data_size = 0x800000;
            decore_buffer->apkt_data_size = 0x300000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0xA00000) {
            decore_buffer->vpkt_data_size = 0x700000;
            decore_buffer->apkt_data_size = 0x200000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0x900000) {
            decore_buffer->vpkt_data_size = 0x600000;
            decore_buffer->apkt_data_size = 0x200000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0x800000) {
            decore_buffer->vpkt_data_size = 0x500000;
            decore_buffer->apkt_data_size = 0x200000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0x700000) {
            decore_buffer->vpkt_data_size = 0x400000;
            decore_buffer->apkt_data_size = 0x200000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0x600000) {
            decore_buffer->vpkt_data_size = 0x380000;
            decore_buffer->apkt_data_size = 0x180000-0x30000;
            decore_buffer->mix_data_size  = 0x30000;
        } else if (run_size >= 0x500000) {
            decore_buffer->vpkt_data_size = 0x300000;
            decore_buffer->apkt_data_size = 0x100000-0x20000;
            decore_buffer->mix_data_size  = 0x20000;
        } else if (run_size >= 0x400000) {
            decore_buffer->vpkt_data_size = 0x200000;
            decore_buffer->apkt_data_size = 0x100000-0x20000;
            decore_buffer->mix_data_size  = 0x20000;
        } else if (run_size >= 0x300000) {
            decore_buffer->vpkt_data_size = 0x180000;
            decore_buffer->apkt_data_size = 0x80000-0x20000;
            decore_buffer->mix_data_size  = 0x20000;
        } else {
            decore_buffer->vpkt_data_size = 0x100000;
            decore_buffer->apkt_data_size = 0x80000-0x20000;
            decore_buffer->mix_data_size  = 0x20000;
        }

        decore_buffer->vpkt_hdr_size = 0;
        total_data_size = decore_buffer->vpkt_hdr_size + decore_buffer->vpkt_data_size
            + decore_buffer->vdec_out_size + decore_buffer->vsink_in_size
            + decore_buffer->apkt_hdr_size + decore_buffer->apkt_data_size
            + decore_buffer->adec_out_size + decore_buffer->asink_in_size
            + decore_buffer->mix_data_size;
        if (total_data_size > run_size) {
            AUI_DBG(
                "decore not enough memory for clear ES %d, %d\n", run_size, total_data_size);

            return AUI_RTN_FAIL;           
        } 
    } else {
        //for vmx ott project, 4M vbv buffer, 3.5M for video, 0.5M for audio
        run_size = vbv_size;
        run_addr = vbv_addr;
        decore_buffer->vpkt_data_size = VBV_BUF_FOR_OTT_VIDEO_SIZE;
        decore_buffer->apkt_data_size = VBV_BUF_FOR_OTT_AUDIO_SIZE;
        decore_buffer->mix_data_size  = 0x30000;
        run_addr_pe_en = pe_mem_addr;
        run_size_pe_en = pe_mem_size;
        
        if ((ES_DATA_TYPE_SUB_SAMP == data_type) 
            || (ES_DATA_TYPE_FULL_SAMP == data_type)) {//used to config sbm for header from user
            decore_buffer->vpkt_hdr_size = 4096 * sizeof(struct vdec_av_packet); //for AS ,last: 4096 * sizeof(struct vdec_av_packet);
            //es data use vbv, es header and audio mix use pe
            total_data_size = decore_buffer->vpkt_data_size + decore_buffer->vdec_out_size + decore_buffer->vsink_in_size
                + decore_buffer->apkt_hdr_size + decore_buffer->apkt_data_size
                + decore_buffer->adec_out_size + decore_buffer->asink_in_size;
            if ((decore_buffer->vpkt_hdr_size + decore_buffer->mix_data_size > run_size_pe_en)
                || (total_data_size > run_size)) {
                AUI_DBG(
                    "decore not enough memory for encrypt ES1 %d, %d, %d, %d\n", 
                    run_size, total_data_size, run_size_pe_en, 
                    decore_buffer->vpkt_hdr_size + decore_buffer->mix_data_size);

                return AUI_RTN_FAIL;           
            }
            
        } else {
            decore_buffer->vpkt_hdr_size = 0; 
            //es header/data use vbv, audio mix use pe
            total_data_size = decore_buffer->vpkt_hdr_size + decore_buffer->vpkt_data_size + decore_buffer->vdec_out_size + decore_buffer->vsink_in_size
                + decore_buffer->apkt_hdr_size + decore_buffer->apkt_data_size
                + decore_buffer->adec_out_size + decore_buffer->asink_in_size;
            if ((decore_buffer->mix_data_size > run_size_pe_en)
                || (total_data_size > run_size)) {
                AUI_DBG(
                    "decore not enough memory for encrypt ES0 %d, %d, %d, %d\n", 
                    run_size, total_data_size, run_size_pe_en, decore_buffer->mix_data_size);

                return AUI_RTN_FAIL;           
            }           
        }     
    }

    decore_buffer->pcm_buf_size = 0; //0x200000; /* do not exceed max vbv buffer size of mpeg2 and avc */
    decore_buffer->pcm_buf_addr = 0; //video_priv_addr + video_priv_size - decore_buffer->pcm_buf_size; /* share with video vbv buffer */

    decore_buffer->frm_buf_addr = video_frm_addr;
    decore_buffer->frm_buf_size = video_frm_size;

    decore_buffer->vpkt_data_addr = run_addr;
    decore_buffer->vpkt_data_sbm = video_sbm;
    run_addr += decore_buffer->vpkt_data_size;
    run_size -= decore_buffer->vpkt_data_size;

    //used to config sbm for header from user
    if ((ES_DATA_TYPE_SUB_SAMP == data_type) 
        || (ES_DATA_TYPE_FULL_SAMP == data_type)) {
        decore_buffer->vpkt_hdr_addr = run_addr_pe_en; //pe_mem_addr;
        if (decore_buffer->vpkt_data_sbm == SBM_ID_SBM0) {
            decore_buffer->vpkt_hdr_sbm = SBM_ID_SBM2;
        } else {
            decore_buffer->vpkt_hdr_sbm = SBM_ID_SBM3;
        }
        run_addr_pe_en += decore_buffer->vpkt_hdr_size;
        run_size_pe_en -= decore_buffer->vpkt_hdr_size;
    } else { //when es_data_type_v_sub_samp != data_type, video data and header use the same sbm
        decore_buffer->vpkt_hdr_addr = 0;
        decore_buffer->vpkt_hdr_sbm = decore_buffer->vpkt_data_sbm;
        run_addr += decore_buffer->vpkt_hdr_size;
        run_size -= decore_buffer->vpkt_hdr_size;
    }
    
    //    decore_buffer->vdec_out_addr =
    //        (decore_buffer->vdec_out_size == 0)? 0:run_addr;
    //    decore_buffer->vdec_out_sbm=
    //        (decore_buffer->vdec_out_size == 0)? -1:SBM_ID_SBM2;
    decore_buffer->vdec_out_addr = 0;
    decore_buffer->vdec_out_sbm = -1;
    run_addr += decore_buffer->vdec_out_size;
    run_size -= decore_buffer->vdec_out_size;

    //    decore_buffer->vsink_in_addr =
    //        (decore_buffer->vsink_in_size == 0)? 0:run_addr;
    //    decore_buffer->vsink_in_sbm =
    //        (decore_buffer->vsink_in_size == 0)? -1:SBM_ID_SBM3;
    decore_buffer->vsink_in_addr = 0;
    decore_buffer->vsink_in_sbm = -1;
    run_addr += decore_buffer->vsink_in_size;
    run_size -= decore_buffer->vsink_in_size;

    if ((ES_DATA_TYPE_SUB_SAMP == data_type) 
        || (ES_DATA_TYPE_FULL_SAMP == data_type)) {
        //for AS ,last: 4096 * sizeof(struct vdec_av_packet);
        decore_buffer->apkt_hdr_size = 4096 * sizeof(struct vdec_av_packet);
        decore_buffer->apkt_hdr_addr = run_addr_pe_en;
        decore_buffer->apkt_hdr_sbm = SBM_ID_SBM4;
        run_addr_pe_en += decore_buffer->apkt_hdr_size;
        run_size_pe_en -= decore_buffer->apkt_hdr_size;
        
        decore_buffer->apkt_data_addr = run_addr;     
        decore_buffer->apkt_data_sbm = SBM_ID_SBM5;
        run_addr += decore_buffer->apkt_data_size;
        run_size -= decore_buffer->apkt_data_size;
    } else {
        
        /*audio data and header use the same sbm*/
        decore_buffer->apkt_data_addr = run_addr;     
        decore_buffer->apkt_data_sbm = SBM_ID_SBM4;
        run_addr += decore_buffer->apkt_data_size;
        run_size -= decore_buffer->apkt_data_size;
        
        //    decore_buffer->apkt_hdr_addr =
        //        (decore_buffer->apkt_hdr_size == 0)? 0:run_addr;
        //    decore_buffer->apkt_hdr_sbm =
        //        (decore_buffer->apkt_hdr_size == 0)? decore_buffer->apkt_data_sbm:SBM_ID_SBM5;
        decore_buffer->apkt_hdr_addr = 0;
        decore_buffer->apkt_hdr_sbm = decore_buffer->apkt_data_sbm;
        run_addr += decore_buffer->apkt_hdr_size;//current value 0
        run_size -= decore_buffer->apkt_hdr_size;//current value 0
    }
    //    decore_buffer->adec_out_addr =
    //        (decore_buffer->adec_out_size == 0)? 0:run_addr;
    //    decore_buffer->adec_out_sbm =
    //        (decore_buffer->adec_out_size == 0)? -1:SBM_ID_SBM6;
    decore_buffer->adec_out_addr = 0;
    decore_buffer->adec_out_sbm = -1;
    run_addr += decore_buffer->adec_out_size;//current value 0
    run_size -= decore_buffer->adec_out_size;//current value 0

    //    decore_buffer->asink_in_addr =
    //        (decore_buffer->asink_in_size == 0)? 0:run_addr;
    //    decore_buffer->asink_in_sbm =
    //        (decore_buffer->asink_in_size == 0)? -1:SBM_ID_SBM7;
    decore_buffer->asink_in_addr = 0;
    decore_buffer->asink_in_sbm = -1;
    run_addr += decore_buffer->asink_in_size;//current value 0
    run_size -= decore_buffer->asink_in_size;//current value 0

    //add for audio mixing
    decore_buffer->mix_data_sbm = SBM_ID_SBM9;  
    if (ES_DATA_TYPE_CLEAR == data_type) {
        decore_buffer->mix_data_addr = run_addr;
        run_addr += decore_buffer->mix_data_size;
        run_size -= decore_buffer->mix_data_size;
    } else {
        decore_buffer->mix_data_addr = run_addr_pe_en;
        run_addr_pe_en += decore_buffer->mix_data_size;
        run_addr_pe_en -= decore_buffer->mix_data_size;
    }

    return AUI_RTN_SUCCESS;
}

static enum Snd_decoder_codec_id aui_get_pcm_code_id(aui_audio_info_init *audio_info) 
{
    enum Snd_decoder_codec_id  codec_id = SND_CODEC_ID_LAST;
    int bps = audio_info->nb_bits_per_sample/8 - 1;
    
    if (bps >= 0 && bps <= 3 && audio_info->sign_flag <= 1 && audio_info->endian <= 1) {
        codec_id = av_inject_codec_pcm[audio_info->sign_flag][audio_info->endian][bps];
    }

    return codec_id;
}

static enum Snd_decoder_codec_id aui_to_sl_audio_format(aui_audio_info_init *audio_info)
{
    aui_deca_stream_type format = audio_info->codec_id;
    enum Snd_decoder_codec_id  codec_id = SND_CODEC_ID_LAST;
    
    /* Deprecated value, need to convert them to match with correct codec value of decv */
    if (format <= AUI_DECA_STREAM_TYPE_LAST) {
        switch (format) {
            case AUI_DECA_STREAM_TYPE_MPEG1: 
                codec_id = SND_CODEC_ID_MP2;//not support CODEC_ID_MP1 right now, both use CODEC_ID_MP2
                break;
            case AUI_DECA_STREAM_TYPE_MPEG2: 
                codec_id = SND_CODEC_ID_MP2;
                break;
            case AUI_DECA_STREAM_TYPE_AAC_LATM: 
                codec_id = SND_CODEC_ID_AAC_LATM;
                break;
            case AUI_DECA_STREAM_TYPE_AC3: 
                codec_id = SND_CODEC_ID_AC3;
                break;
            case AUI_DECA_STREAM_TYPE_DTS: 
                codec_id = SND_CODEC_ID_DTS;
                break;           
            case AUI_DECA_STREAM_TYPE_RAW_PCM:
                codec_id = aui_get_pcm_code_id(audio_info);
                break;            
            case AUI_DECA_STREAM_TYPE_BYE1: 
                codec_id = SND_CODEC_ID_BYE1V2;
                break;
            case AUI_DECA_STREAM_TYPE_RA8: 
                codec_id = SND_CODEC_ID_RA_144;
                break;
            case AUI_DECA_STREAM_TYPE_MP3: 
                codec_id = SND_CODEC_ID_MP3;
                break;
            case AUI_DECA_STREAM_TYPE_AAC_ADTS: 
                codec_id = SND_CODEC_ID_AAC;//mpeg aac and adts aac both use AAC in MP
                break;
            case AUI_DECA_STREAM_TYPE_OGG: 
                codec_id = SND_CODEC_ID_VORBIS;
                break;
            case AUI_DECA_STREAM_TYPE_EC3: 
                codec_id = SND_CODEC_ID_EAC3;
                break;
            case AUI_DECA_STREAM_TYPE_MP3_L3: 
                codec_id = SND_CODEC_ID_MP3;
                break; 
            case AUI_DECA_STREAM_TYPE_ADPCM: 
            case AUI_DECA_STREAM_TYPE_PPCM:     
            case AUI_DECA_STREAM_TYPE_LPCM_V: 
            case AUI_DECA_STREAM_TYPE_LPCM_A: 
            case AUI_DECA_STREAM_TYPE_PCM: //need more info to support adpcm
                codec_id = SND_CODEC_ID_LAST;
                break;
            case AUI_DECA_STREAM_TYPE_BYE1PRO: 
                codec_id = SND_CODEC_ID_BYE1PRO;
                break;
            case AUI_DECA_STREAM_TYPE_FLAC: 
                codec_id = SND_CODEC_ID_FLAC;
                break;
            case AUI_DECA_STREAM_TYPE_APE: 
                codec_id = SND_CODEC_ID_APE;
                break;
            case AUI_DECA_STREAM_TYPE_MP3_2: 
                codec_id = SND_CODEC_ID_MP3;
                break;
            case AUI_DECA_STREAM_TYPE_AMR: 
                codec_id = SND_CODEC_ID_AMR_NB;
                break;
            default:
                codec_id = SND_CODEC_ID_LAST;
                break;
        }
    } else {
        codec_id = format;
    }
    return codec_id;
}

static AUI_RTN_CODE aui_audio_decoder_mix_sbm_config(struct aui_private_audio *p_mix, 
    struct aui_decore_buffer *decore_buf)
{
    struct sbm_buf_config sbm_info;
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if (alislsbm_open(&p_mix->pkt_data_hdl, decore_buf->mix_data_sbm)) {
        AUI_DBG( "open SBMDEV_APKT_DATA fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto mix_cfg_exit;
    }
    
    memset(&sbm_info, 0, sizeof(sbm_info));
    sbm_info.buffer_size = decore_buf->mix_data_size;
    sbm_info.buffer_addr = decore_buf->mix_data_addr;
    sbm_info.block_size = 0x6000;
    sbm_info.reserve_size = decore_buf->mix_data_reserve_size;
    sbm_info.wrap_mode = SBM_WRAP_MODE_PACKET;
    sbm_info.lock_mode = SBM_LOCK_MODE_MUTEX_LOCK;

    if (alislsbm_create(p_mix->pkt_data_hdl, &sbm_info)) {
        AUI_DBG( "decore create sbm9 fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto mix_cfg_exit;
    }

    p_mix->pkt_hdr_hdl = p_mix->pkt_data_hdl;
    return rtn_code;

mix_cfg_exit:
    if (p_mix->pkt_data_hdl) {
        alislsbm_close(p_mix->pkt_data_hdl);
        p_mix->pkt_data_hdl = NULL;
    }

    return rtn_code;
}


/** init audio decoder with given codec.
*   \param enc_flag : if es is encrypted, set enc_flag 1, or else set enc_flag 0.
 *  \param codec_id  codec to be used
 *  \param[out] decoder_out returned decoder instance
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_audio_decoder_open_internal(aui_audio_info_init* audio_info,
        aui_hdl* decoder_out, unsigned char enc_flag) 
{
    struct aui_private_audio *decp = NULL;
    struct snd_audio_config audio_config;
    struct sbm_buf_config sbm_info;
    enum Snd_decoder_codec_id codec_id = SND_CODEC_ID_LAST;
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    aui_av_packet_complete_t extra_pkt_info;
    struct aui_decore_buffer decore_buf;
    ES_DATA_TYPE data_type = ES_DATA_TYPE_CLEAR;
    unsigned char audio_id = 0;
    
    memset(&extra_pkt_info, 0, sizeof(aui_av_packet_complete_t));

    if (decoder_out == NULL || audio_info == NULL) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (*decoder_out) {
        AUI_DBG( "decoder has been inited\n");
        return AUI_RTN_SUCCESS;
    }

    codec_id = aui_to_sl_audio_format(audio_info);
    decp = malloc(sizeof(struct aui_private_audio));

    if (decp == NULL) {
        aui_rtn(AUI_RTN_ENOMEM,
            "malloc decoder private fail\n");
    }
    
    memset(decp, 0, sizeof(struct aui_private_audio));
    decp->codec_id = codec_id;

    memset(&sbm_info, 0, sizeof(sbm_info));
    memset(&audio_config, 0, sizeof(audio_config));
    
    if (enc_flag) {
        data_type = ES_DATA_TYPE_FULL_SAMP;
    }

    if ( AUI_RTN_SUCCESS != aui_decore_alloc_buffer(NULL, &decore_buf, data_type)) {
        AUI_DBG("aui_decore_alloc_buffer failed!\n");
        rtn_code = AUI_RTN_FAIL;
        goto audio_init_exit;
    }
    
    if (audio_info->deca_handle) {
        aui_deca_get_audio_id(audio_info->deca_handle, &audio_id);
        aui_deca_get_sl_handle(audio_info->deca_handle, &decp->audio_hdl);
        decp->audio_hdl_need_close = 0;
        if (audio_id == 1) {
            struct snd_audio_mix_info mix_info;
            decp->mix = 1;
            
            if (aui_audio_decoder_mix_sbm_config(decp, &decore_buf)) {
                AUI_DBG("aui_audio_decoder_mix_sbm_config failed!\n");
                rtn_code = AUI_RTN_FAIL;
                goto audio_init_exit;
            }    
        
            memset(&mix_info, 0, sizeof(mix_info));
            //mix_info.enable = 1;
            mix_info.ch_num = audio_info->channels;
            mix_info.sample_rate = audio_info->sample_rate;
            mix_info.bit_per_samp = audio_info->nb_bits_per_sample;
            mix_info.sbm_id = decore_buf.mix_data_sbm;
            
            if (alislsnd_set_audio_mix_info(decp->audio_hdl, &mix_info)) {
                AUI_DBG("set audio mix info failed!\n");
                rtn_code = AUI_RTN_FAIL;
                goto audio_init_exit;
            }

            *decoder_out = (aui_hdl) decp;
            //When audio mix, we use deca 1, the av inject audio index equals the deca index
            decp->dev_priv_data.dev_idx = 1;
            aui_dev_reg(AUI_MODULE_AV_INJECT_AUDIO, *decoder_out);
            return AUI_RTN_SUCCESS;
        }
    }

    if (decp->audio_hdl == NULL) {
        decp->audio_hdl_need_close = 1;
        
        if (alislsnd_open(&decp->audio_hdl)) {
            AUI_DBG( "open audio dev fail\n");
            rtn_code = AUI_RTN_FAIL;
            goto audio_init_exit;
        }
    }

    if (decp->avsync_hdl == NULL) {
        alislavsync_open(&decp->avsync_hdl);
    }
    
    //set avsync as default
    if (decp->avsync_hdl) {
        alislavsync_set_sourcetype(decp->avsync_hdl, AVSYNC_FROM_HDD_MP);
        alislavsync_set_data_type(decp->avsync_hdl, AVSYNC_DATA_TYPE_ES);
        alislavsync_set_av_sync_mode(decp->avsync_hdl, AVSYNC_AUDIO);    
        alislavsync_start(decp->avsync_hdl);    
    }

    if (alislsbm_open(&decp->pkt_data_hdl, decore_buf.apkt_data_sbm)) {
        AUI_DBG(
            "open SBMDEV_APKT_DATA fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto audio_init_exit;
    }
    
    sbm_info.buffer_size = decore_buf.apkt_data_size; //0x300000;
    sbm_info.buffer_addr = decore_buf.apkt_data_addr;
    sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = decore_buf.apkt_data_reserve_size;
    sbm_info.wrap_mode = SBM_WRAP_MODE_PACKET;
    sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;

    if (alislsbm_create(decp->pkt_data_hdl, &sbm_info)) {
        AUI_DBG( "decore create sbm fail\n");
        rtn_code = AUI_RTN_EIO;
        goto audio_init_exit;
    }

    if (alislsbm_open(&decp->pkt_hdr_hdl, decore_buf.apkt_hdr_sbm)) {
        AUI_DBG( "open SBMDEV_APKT_HDR fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto audio_init_exit;
    }

    if (decore_buf.apkt_hdr_sbm != decore_buf.apkt_data_sbm) {
        sbm_info.buffer_size = decore_buf.apkt_hdr_size; // 1024 num packets [input packet]
        sbm_info.buffer_addr = decore_buf.apkt_hdr_addr;
        sbm_info.reserve_size = decore_buf.apkt_hdr_reserve_size;
        sbm_info.wrap_mode = SBM_WRAP_MODE_NORMAL;
        sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;

        if (alislsbm_create(decp->pkt_hdr_hdl, &sbm_info)) {
            AUI_DBG(
                "decore create sbm fail\n");
            rtn_code = AUI_RTN_EIO;
            goto audio_init_exit;
        }
    }

    if (decore_buf.adec_out_addr && decore_buf.adec_out_size
        && decore_buf.adec_out_sbm >= 0) {
        if (alislsbm_open(&decp->dec_out_hdl, decore_buf.adec_out_sbm)) {
            AUI_DBG(
                "open SBMDEV_DECA_OUT fail\n");
            rtn_code = AUI_RTN_FAIL;
            goto audio_init_exit;
        }
        
        sbm_info.buffer_size = decore_buf.adec_out_size; // 128 num frm decoded [output frame]
        sbm_info.buffer_addr = decore_buf.adec_out_addr;
        sbm_info.reserve_size = decore_buf.adec_out_reserve_size;
        sbm_info.wrap_mode = SBM_WRAP_MODE_NORMAL;
        sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;

        if (alislsbm_create(decp->dec_out_hdl, &sbm_info)) {
            AUI_DBG(
                "decore create sbm fail\n");
            rtn_code = AUI_RTN_EIO;
            goto audio_init_exit;
        }
    }
    
    if (decore_buf.asink_in_addr && decore_buf.asink_in_size
        && decore_buf.asink_in_sbm >= 0) {
        if (alislsbm_open(&decp->sink_in_hdl, decore_buf.asink_in_sbm)) {
            AUI_DBG( "open SBM fail\n");
            rtn_code = AUI_RTN_FAIL;
            goto audio_init_exit;
        }
            
        sbm_info.buffer_size = decore_buf.asink_in_size; // 128 num frm decoded [input frm to see]
        sbm_info.buffer_addr = decore_buf.asink_in_addr;
        sbm_info.reserve_size = decore_buf.asink_in_reserve_size;
        sbm_info.wrap_mode = SBM_WRAP_MODE_NORMAL;
        sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;

        if (alislsbm_create(decp->sink_in_hdl, &sbm_info)) {
            AUI_DBG(
                "decore create sbm fail\n");
            rtn_code = AUI_RTN_EIO;
            goto audio_init_exit;
        }
    }
    
    AUI_DBG(
        "autio sbm info: hdr:%d,data:%d,out:%d,in:%d\n",
        decore_buf.apkt_hdr_sbm, decore_buf.apkt_data_sbm,
        decore_buf.adec_out_sbm, decore_buf.asink_in_sbm);
    audio_config.decode_mode = 1;
    //audio_config.sync_mode = audio_info->sync_mode;
    //audio_config.sync_unit = audio_info->sync_unit;
    audio_config.deca_input_sbm = (decore_buf.apkt_hdr_sbm << 16)
        | decore_buf.apkt_data_sbm;
    audio_config.deca_output_sbm = decore_buf.adec_out_sbm; //DECA_OUT_SBM_IDX;
    audio_config.snd_input_sbm = decore_buf.asink_in_sbm;     //SND_IN_SBM_IDX;
    audio_config.codec_id = codec_id;      //avctx->codec_id;
    audio_config.bits_per_coded_sample = audio_info->nb_bits_per_sample;
    audio_config.sample_rate = audio_info->sample_rate; // param user need set according different file
    audio_config.channels = audio_info->channels; // param user need set according different file
    audio_config.bit_rate = audio_info->bit_rate; // param user need set according different file
    //audio_config.pcm_buf_size = audio_info->pcm_buf_size;
    //audio_config.pcm_buf = audio_info->pcm_buf;
    audio_config.block_align = audio_info->block_align;
    //audio_config.codec_frame_size = audio_info->codec_frame_size;
    /*
    * This variable is always set to 0
    * For the decoder does not distinguish between encryption and non-encryption.
    */
    audio_config.encrypt_mode = ES_DATA_TYPE_CLEAR;//data_type;
    
    if (audio_info->extradata_size > 0 && audio_info->extradata) {
        AUI_DBG(
            "audio decode extradata: 0x%x %d\n",
            (int )audio_info->extradata, (int )audio_info->extradata_size);

        if (audio_info->extradata_size > 512) {
            extra_pkt_info.size = audio_info->extradata_size;
            
            if (alislsbm_write(decp->pkt_hdr_hdl,
                (unsigned char *) &extra_pkt_info,
                sizeof(aui_av_packet_complete_t))) {
                AUI_DBG(
                    "audio decode extradata size error(%ld > 512)!!!",
                    audio_info->extradata_size);
                goto audio_init_exit;
            }
            
            if (alislsbm_write(decp->pkt_data_hdl, audio_info->extradata,
                audio_info->extradata_size)) {
                AUI_DBG(
                    "audio decode extradata size error(%ld > 512)!!!",
                    audio_info->extradata_size);
                goto audio_init_exit;
            }
        } else {
            MEMCPY(audio_config.extra_data, audio_info->extradata,
            audio_info->extradata_size);
            audio_config.extradata_size = audio_info->extradata_size;
            audio_config.extradata_mode = 0;
        }
    }

    if (alislsnd_mp_decore_init(decp->audio_hdl, &audio_config)) {
        AUI_DBG( "deca decore init fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto audio_init_exit;
    }

    *decoder_out = (aui_hdl) decp;
    //When play audio, we use deca 0(audio mix deca 1), the av inject audio index equals the deca index
    decp->dev_priv_data.dev_idx = 0;
    aui_dev_reg(AUI_MODULE_AV_INJECT_AUDIO, *decoder_out);
    return AUI_RTN_SUCCESS;

audio_init_exit: 
    if (decp->pkt_hdr_hdl)
        alislsbm_close(decp->pkt_hdr_hdl);
    if (decp->pkt_data_hdl)
        alislsbm_close(decp->pkt_data_hdl);
    if (decp->dec_out_hdl)
        alislsbm_close(decp->dec_out_hdl);
    if (decp->sink_in_hdl)
        alislsbm_close(decp->sink_in_hdl);
    if (decp->audio_hdl)
        alislsnd_close(decp->audio_hdl);
    if (decp)
        free(decp);
        
    return rtn_code;
}

AUI_RTN_CODE aui_audio_decoder_open(aui_audio_info_init* audio_info, aui_hdl* decoder_out) 
{
    return aui_audio_decoder_open_internal(audio_info, decoder_out, 0);
}

/** Close given decoder instance.
 *  \param decoder  decoder instance created with \e ali_audio_decoder_open()
 */
AUI_RTN_CODE aui_audio_decoder_close(aui_hdl decoder) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;
    //int pkt_num = 0;

#if 0
    if (alislsbm_get_pkt_num(decp->pkt_data_hdl, &pkt_num))
    {
        aui_rtn(AUI_RTN_EIO, "sbm ioctl fail\n");
    }
    while(pkt_num)
    {
        if (alislsbm_get_pkt_num(decp->pkt_data_hdl, &pkt_num))
        {
            aui_rtn(AUI_RTN_EIO, "sbm ioctl fail\n");
        }
    }
#endif
    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "decoder not valid\n");
    }
    if (!decp->mix) {
        if (alislsnd_mp_decore_release(decp->audio_hdl)) {
            aui_rtn(AUI_RTN_EIO,
                    "deca decore release fail\n");
        }
    } else {
        /* stop audio mix before closing sbm, 
        or there may be an unexpected error in driver*/
        
        if (alislsnd_audio_mix_stop(decp->audio_hdl)) {
            aui_rtn(AUI_RTN_EIO,
                    "deca mix stop fail\n");
        }
    }

    if (decp->pkt_hdr_hdl)
        alislsbm_close(decp->pkt_hdr_hdl);
    if (decp->pkt_data_hdl)
        alislsbm_close(decp->pkt_data_hdl);
    if (decp->dec_out_hdl)
        alislsbm_close(decp->dec_out_hdl);
    if (decp->sink_in_hdl)
        alislsbm_close(decp->sink_in_hdl);
    if (decp->audio_hdl && decp->audio_hdl_need_close)
        alislsnd_close(decp->audio_hdl);
    if (decp->avsync_hdl) {
        alislavsync_stop(decp->avsync_hdl);
        alislavsync_close(decp->avsync_hdl);
        decp->avsync_hdl = NULL;
    }
    
    aui_dev_unreg(AUI_MODULE_AV_INJECT_AUDIO, decp);
    free(decp);

    return AUI_RTN_SUCCESS;
}

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_audio_decoder_get_status(aui_hdl decoder,
        aui_audio_decoder_status* status) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;
    unsigned long free_size;
    struct snd_audio_decore_status snd_status;
    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "decoder not valid\n");
    }

    if (status == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "decoder not valid\n");
    }

    alislsbm_get_total_size(decp->pkt_data_hdl, &status->buffer_size);
    alislsbm_get_free_size(decp->pkt_data_hdl, &free_size);
    status->buffer_used = status->buffer_size - free_size;
    if (!decp->mix) {//mix can't do following steps
        if (alislsnd_mp_decore_get_status(decp->audio_hdl, &snd_status)) {
            aui_rtn(AUI_RTN_EIO, "audio get status fail\n");
        }
        //AUI_DBG("bits_per_sample: %ld, channels: %ld, fhg: %ld, fhp: %ld, fd: %ld, sr: %ld\n",
        //    snd_status.bits_per_sample, snd_status.channels, snd_status.first_header_got,
        //    snd_status.first_header_parsed, snd_status.frames_decoded, snd_status.sample_rate);
        status->frames_decoded = snd_status.frames_decoded;
        if (decp->avsync_hdl) {
            struct avsync_cur_status avsync_status;
            if (alislavsync_get_status(decp->avsync_hdl, &avsync_status)) {
                aui_rtn(AUI_RTN_EIO,
                        "avsync get status fail\n");
            }
            status->last_pts = avsync_status.cur_apts;
        }
    }
    return AUI_RTN_SUCCESS;
}

/** Pause decoding audio frames / playing audio samples.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_audio_decoder_pause(aui_hdl decoder, int pause) 
{
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "decoder not valid\n");
    }

    if (pause) {
        if (decp->mix) {
            if (alislsnd_audio_mix_pause(decp->audio_hdl)) {
                aui_rtn(AUI_RTN_EIO,
                    "audio mix pause fail\n");
            }
        } else {
            if (alislsnd_mp_decore_pause(decp->audio_hdl)) {
                aui_rtn(AUI_RTN_EIO,
                    "deca decore pause fail\n");
            }
        }
    } else {
        if (decp->mix) {
            if (alislsnd_audio_mix_resume(decp->audio_hdl)) {
                aui_rtn(AUI_RTN_EIO,
                    "audio mix resume fail\n");
            }
        } else {
            if (alislsnd_mp_decore_resume(decp->audio_hdl)) {
                aui_rtn(AUI_RTN_EIO,
                    "deca decore resume fail\n");
            }
        }
    }

    return AUI_RTN_SUCCESS;
}

/** Push next PES packet header to ali audio decoder. ES mode need not write this header.
 *  \param p_pkt_header, contains pts, frame size, etc.
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_audio_decoder_write_header(aui_hdl decoder,
        const struct aui_av_packet* p_pkt_header) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;

    aui_av_packet_complete_t av_packet;
    memset(&av_packet, 0, (sizeof(aui_av_packet_complete_t)));

    if ((decp == NULL) || (NULL == p_pkt_header)) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    /* Due to drivers spec, i need to pass the entire struct size, just customer
     * parameters are really considered */

    av_packet.dts = p_pkt_header->dts;
    av_packet.pts = p_pkt_header->pts;
    av_packet.size = p_pkt_header->size;

    if (p_pkt_header->flag != AUI_AV_PACKET_EOS) {
        if (alislsbm_write(decp->pkt_hdr_hdl, (unsigned char *) &av_packet,
                sizeof(aui_av_packet_complete_t)))
            return AUI_RTN_EIO;        
    } else {
        if (decp->mix) {
            //when received eos header, set mix end to driver
            alislsnd_set_audio_mix_end(decp->audio_hdl);
        }
    }

    return AUI_RTN_SUCCESS;
}

/** Push fragment of PES packet to audio decoder, or ES data (see \e ali_audio_decoder_set_pes_mode).
 *  \param buf  buffer with fragment of one PES packet, or some ES data.
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_audio_decoder_write(aui_hdl decoder,
        const unsigned char * buf, unsigned long size) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;
    
    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (alislsbm_write(decp->pkt_data_hdl, (unsigned char *) buf, size))
        return AUI_RTN_EIO;

    return AUI_RTN_SUCCESS;
}

/**
 *   @brief      Flush audio decoder
 *   @author     christian.xie
 *   @date       2014-3-11
 *   @param[in]  decoder             decoder instance
 *   @return     AUI_RTN_SUCCESS     Return successful
 *   @return     AUI_RTN_FAIL        Return error
 *   @note
 *
 */
AUI_RTN_CODE aui_audio_decoder_flush(aui_hdl decoder) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "decoder not valid\n");
    }

    if (decp->mix) {
        return AUI_RTN_SUCCESS;//mix can not do flush
    }
    
    if (decp->avsync_hdl) {
        if (alislavsync_flush(decp->avsync_hdl)) {
            aui_rtn(AUI_RTN_EIO,
                    "--> set av sync mode fail\n");
        }
    }
    if (alislsnd_mp_decore_pause(decp->audio_hdl))
        aui_rtn(AUI_RTN_EINVAL,
                "decoder pause failed\n");
    if (alislsnd_mp_decore_flush(decp->audio_hdl))
        aui_rtn(AUI_RTN_EINVAL,
                "decoder flush failed\n");

    if (alislsnd_mp_decore_resume(decp->audio_hdl))
        aui_rtn(AUI_RTN_EINVAL,
                "decoder resume failed\n");

    return AUI_RTN_SUCCESS;
}

/**
 *   @brief      Get current buffer status.
 *   @author     christian.xie
 *   @date       2014-3-11
 *   @param[in]  decoder             decoder instance
 *   @param[out] buffer_status       returned current buffer status
 *   @return     AUI_RTN_SUCCESS     Return successful
 *   @return     AUI_RTN_FAIL        Return error
 *   @note
 *
 */
AUI_RTN_CODE aui_audio_decoder_get_buffer_status(aui_hdl decoder,
        aui_decoder_buffer_status *buffer_status) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;
    // AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    alislsbm_get_total_size(decp->pkt_data_hdl, &buffer_status->total_size);
    alislsbm_get_valid_size(decp->pkt_data_hdl, &buffer_status->valid_size);
    alislsbm_get_free_size(decp->pkt_data_hdl, &buffer_status->free_size);

    return AUI_RTN_SUCCESS;
}

/** Enable/disable STC sync.
 *  With sync enabled, decoder plays audio frames according to pts/stc difference.
 *  With sync disabled, decoder plays audio frames according to sample rate.
 *  \param enable  enable/disable STC sync
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_audio_decoder_set_sync(aui_hdl decoder,
        int enable) {
    struct aui_private_audio *decp = (struct aui_private_audio *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (decp->mix) {
        return AUI_RTN_SUCCESS;//mix can not set avsync
    }

    if (enable) {
        if (decp->avsync_hdl) {
            if (alislavsync_set_av_sync_mode(decp->avsync_hdl, AVSYNC_AUDIO)) {
                aui_rtn(AUI_RTN_EIO,
                        "--> set av sync mode fail\n");
            }
        }
    } else {
        if (decp->avsync_hdl) {        
            if (alislavsync_set_av_sync_mode(decp->avsync_hdl, AVSYNC_AV_FREERUN)) {
                aui_rtn(AUI_RTN_EIO,
                        "--> set av sync mode fail\n");
            }
        }
    }
    return AUI_RTN_SUCCESS;
}
/*
 AUI_RTN_CODE aui_video_decoder_get_memory(aui_video_decoder_mem *decoder_mem)
 {
 struct vdec_mem_info video_mem_info;
 alisl_handle video_hdl = NULL;

 if (alislvdec_open(&video_hdl))
 {
 aui_rtn(AUI_RTN_EINVAL,"decoder has not been opened\n");
 }
 if (alislvdec_get_mem_info(video_hdl, &video_mem_info))
 {
 alislvdec_close(video_hdl);
 aui_rtn(AUI_RTN_EINVAL,"VDECIO GET MEMORY FAILED\n");
 }

 decoder_mem->main_mem_addr = (unsigned long)video_mem_info.mem_start;
 decoder_mem->main_mem_size = video_mem_info.mem_size;
 decoder_mem->priv_mem_addr = (unsigned long)video_mem_info.priv_mem_start;
 decoder_mem->priv_mem_size = video_mem_info.priv_mem_size;
 decoder_mem->mp_mem_addr = (unsigned long)video_mem_info.mp_mem_start;
 decoder_mem->mp_mem_size = video_mem_info.mp_mem_size;
 AUI_DBG( "decoder use mem: cpu 0x%x 0x%x, see 0x%x 0x%x, mp 0x%x 0x%x\n",
 (unsigned int )decoder_mem->main_mem_addr, (unsigned int )decoder_mem->main_mem_size,
 (unsigned int )decoder_mem->priv_mem_addr, (unsigned int )decoder_mem->priv_mem_size,
 (unsigned int )decoder_mem->mp_mem_addr, (unsigned int )decoder_mem->mp_mem_size);

 alislvdec_close(video_hdl);

 return AUI_RTN_SUCCESS;
 }
 */

static enum vdec_id aui_to_sl_video_format(unsigned int format)
{
    enum vdec_id codec_id = VDEC_ID_LAST;
    /* Deprecated value, need to convert them to match with correct codec value of decv */
    if (format <= AUI_DECV_FORMAT_NUM) {
        switch (format) {
            case AUI_DECV_FORMAT_MPEG: 
                codec_id = VDEC_ID_MPG2;
                break;
            case AUI_DECV_FORMAT_AVC: 
                codec_id = VDEC_ID_H264;
                break;
            case AUI_DECV_FORMAT_AVS: 
                codec_id = VDEC_ID_LAST;//not support AVS
                break;
            case AUI_DECV_FORMAT_XVID: 
                codec_id = VDEC_ID_XVID;
                break;
            case AUI_DECV_FORMAT_FLV1: 
                codec_id = VDEC_ID_FLV1;
                break;
            case AUI_DECV_FORMAT_VP8: 
                codec_id = VDEC_ID_VP8;
                break;
            case AUI_DECV_FORMAT_WVC1: 
                codec_id = VDEC_ID_WVC1;
                break;
            case AUI_DECV_FORMAT_WX3: 
                codec_id = VDEC_ID_WX3;
                break;
            case AUI_DECV_FORMAT_RMVB: 
                codec_id = VDEC_ID_RMVB;
                break;
            case AUI_DECV_FORMAT_MJPG: 
                codec_id = VDEC_ID_MJPG;
                break;
            case AUI_DECV_FORMAT_HEVC: 
                codec_id = VDEC_ID_HEVC;
                break;    
            default:
                codec_id = VDEC_ID_LAST;
                break;
        }
    } else {
        codec_id = format - 0x10000;
    }
    return codec_id;
}

/** Open video decoder with given codec.
 *  \param[in] enc_flag  if es is encrypted, set enc_flag 1, or else set enc_flag 0.
 *  \param video_info  codec to be used
 *  \param[out] decoder_out returned decoder instance
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_video_decoder_open_internal(aui_video_decoder_init *video_info,
        aui_hdl *decoder_out, unsigned char enc_flag) 
{
    struct aui_private_video *decp = NULL;
    struct sbm_buf_config sbm_info;
    struct vdec_mp_init_config init_param;
    struct vdec_mp_sbm_config sbm_param;
    struct vdec_mp_extra_data_config extra_data_param;
    struct aui_decore_buffer decore_buf;
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    enum vdec_id codec_id = 0;
    ES_DATA_TYPE data_type = ES_DATA_TYPE_CLEAR;
    unsigned char video_id = 0;

    if (video_info == NULL || decoder_out == NULL)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    if (*decoder_out) {
        AUI_DBG( "decoder has been inited\n");
        return AUI_RTN_SUCCESS;
    }

    decp = malloc(sizeof(struct aui_private_video));
    
    if (decp == NULL) {
        aui_rtn(AUI_RTN_ENOMEM,
            "malloc decoder private fail\n");
    }

    memset(decp, 0, sizeof(struct aui_private_video));
    codec_id = aui_to_sl_video_format(video_info->codec_id);
    decp->codec_id = codec_id;

    if (decp->avsync_hdl == NULL) {
        alislavsync_open(&decp->avsync_hdl);
    }
    
    if (video_info->decv_handle != NULL) {
        aui_decv_get_video_id(video_info->decv_handle, &video_id);
        aui_decv_get_sl_handle(video_info->decv_handle, &decp->video_hdl);
        decp->video_hdl_need_close = 0;
    } else {
        if (alislvdec_open(&decp->video_hdl, 0)) {
            AUI_DBG( "open video device failed\n");
            rtn_code = AUI_RTN_FAIL;
            goto video_open_fail;
        }
        
        decp->video_hdl_need_close = 1;
        //set avsync for default using
        if (decp->avsync_hdl) {
            alislavsync_set_sourcetype(decp->avsync_hdl, AVSYNC_FROM_HDD_MP);
            alislavsync_set_data_type(decp->avsync_hdl, AVSYNC_DATA_TYPE_ES);
            alislavsync_set_av_sync_mode(decp->avsync_hdl, AVSYNC_AUDIO);
            alislavsync_set_video_id(decp->avsync_hdl, 0);
        }
    }

    alislavsync_start(decp->avsync_hdl);

    if (enc_flag) {
        if (AUI_DECV_FORMAT_AVC == video_info->codec_id)
            data_type = ES_DATA_TYPE_FULL_SAMP;
        else
            data_type = ES_DATA_TYPE_FULL_SAMP;
    }
    
    if ( AUI_RTN_SUCCESS != aui_decore_alloc_buffer(decp->video_hdl, &decore_buf, data_type)) {
        rtn_code = AUI_RTN_FAIL;
        goto video_open_fail;
    }

    memset(&sbm_info, 0, sizeof(sbm_info));
    memset(&init_param, 0, sizeof(init_param));

    if (alislsbm_open(&decp->pkt_data_hdl, decore_buf.vpkt_data_sbm)) {
        AUI_DBG(
            "open vdeo pkt data sbm fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto video_open_fail;
    }

    sbm_info.buffer_addr = decore_buf.vpkt_data_addr;
    sbm_info.buffer_size = decore_buf.vpkt_data_size;
    sbm_info.block_size = 0x20000;
    sbm_info.reserve_size = decore_buf.vpkt_data_reserve_size;
    sbm_info.wrap_mode = SBM_WRAP_MODE_PACKET;
    sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;    //SBM_MUTEX_LOCK;

    if (alislsbm_create(decp->pkt_data_hdl, &sbm_info)) {
        AUI_DBG( "decore create sbm fail\n");
        rtn_code = AUI_RTN_EIO;
        goto video_open_fail;
    }

    if (alislsbm_open(&decp->pkt_hdr_hdl, decore_buf.vpkt_hdr_sbm)) {
        AUI_DBG(
            "open vdec pkt hdr sbm fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto video_open_fail;
    }

    if (decore_buf.vpkt_hdr_sbm != decore_buf.vpkt_data_sbm) {
        sbm_info.buffer_addr = decore_buf.vpkt_hdr_addr;
        sbm_info.buffer_size = decore_buf.vpkt_hdr_size;
        sbm_info.reserve_size = decore_buf.vpkt_hdr_reserve_size;
        sbm_info.wrap_mode = SBM_WRAP_MODE_NORMAL;
        sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;    //SBM_MUTEX_LOCK;

        if (alislsbm_create(decp->pkt_hdr_hdl, &sbm_info)) {
            AUI_DBG(
                "decore create sbm fail\n");
            rtn_code = AUI_RTN_EIO;
            goto video_open_fail;
        }
    }

    if (decore_buf.vdec_out_addr && decore_buf.vdec_out_size
        && decore_buf.vdec_out_sbm >= 0) {
        if (alislsbm_open(&decp->dec_out_hdl, decore_buf.vdec_out_sbm)) {
            AUI_DBG(
                "open vdec out sbm fail\n");
            rtn_code = AUI_RTN_FAIL;
            goto video_open_fail;
        }
        
        sbm_info.buffer_addr = decore_buf.vdec_out_addr;
        sbm_info.buffer_size = decore_buf.vdec_out_size;
        sbm_info.reserve_size = decore_buf.vdec_out_reserve_size;
        sbm_info.wrap_mode = SBM_WRAP_MODE_NORMAL;
        sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;

        if (alislsbm_create(decp->dec_out_hdl, &sbm_info)) {
            AUI_DBG(
                "decore create sbm fail\n");
            rtn_code = AUI_RTN_EIO;
            goto video_open_fail;
        }
    }

    if (decore_buf.vsink_in_addr && decore_buf.vsink_in_size
        && decore_buf.vsink_in_sbm >= 0) {
        if (alislsbm_open(&decp->disp_in_hdl, decore_buf.vsink_in_sbm >= 0)) {
            AUI_DBG("open vdec disp in sbm fail\n");
            rtn_code = AUI_RTN_FAIL;
            goto video_open_fail;
        }
        
        sbm_info.buffer_addr = decore_buf.vsink_in_addr;
        sbm_info.buffer_size = decore_buf.vsink_in_size;
        sbm_info.reserve_size = decore_buf.vsink_in_reserve_size;
        sbm_info.wrap_mode = SBM_WRAP_MODE_NORMAL;
        sbm_info.lock_mode = SBM_LOCK_MODE_SPIN_LOCK;

        if (alislsbm_create(decp->disp_in_hdl, &sbm_info)) {
            AUI_DBG(
                "decore create sbm fail\n");
            rtn_code = AUI_RTN_EIO;
            goto video_open_fail;
        }
    }

    init_param.decode_mode = video_info->decode_mode;
    init_param.codec_id = codec_id;
    init_param.frame_rate = video_info->frame_rate; /* param parser from container */
    init_param.pic_width = video_info->pic_width;
    init_param.pic_height = video_info->pic_height;
    init_param.pixel_aspect_x = video_info->sar_width;
    init_param.pixel_aspect_y = video_info->sar_height;
    init_param.dec_buf_addr = decore_buf.frm_buf_addr;
    init_param.dec_buf_size = decore_buf.frm_buf_size;
    init_param.encrypt_mode = data_type;

    if (video_info->codec_id == AUI_DECV_FORMAT_AVC
        && video_info->extradata_size > 0 && video_info->extradata) {
        init_param.decoder_flag |= VDEC_MP_FLAG_AVC1_FORMAT;
    }

    if (alislvdec_mp_init(decp->video_hdl, &init_param)) {
        AUI_DBG( "decore init fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto video_open_fail;
    }

    sbm_param.packet_header = decore_buf.vpkt_hdr_sbm;
    sbm_param.packet_data = decore_buf.vpkt_data_sbm;
    sbm_param.decode_output = decore_buf.vdec_out_sbm;
    sbm_param.display_input = decore_buf.vsink_in_sbm;
    AUI_DBG(
        "video sbm info: hdr:%d,data:%d,out:%d,in:%d\n",
        decore_buf.vpkt_hdr_sbm, decore_buf.vpkt_data_sbm,
        decore_buf.vdec_out_sbm, decore_buf.vsink_in_sbm);

    if (alislvdec_mp_set_sbm(decp->video_hdl, &sbm_param)) {
        AUI_DBG( "decore set video sbm fail\n");
        rtn_code = AUI_RTN_FAIL;
        goto video_open_fail;
    }

    if (video_info->extradata_size > 0 && video_info->extradata) {
        AUI_DBG(
            "video decode extradata: 0x%x %d\n",
            (int )video_info->extradata, (int )video_info->extradata_size);
            extra_data_param.extra_data = video_info->extradata;
            extra_data_param.extra_data_size = video_info->extradata_size;
        if (alislvdec_mp_set_extra_data(decp->video_hdl, &extra_data_param)) {
            AUI_DBG( "decore extradata fail\n");
            rtn_code = AUI_RTN_FAIL;
            goto video_open_fail;

        }
    }

    /* if(video_info->preview)
    {
    struct vdec_rect srect, drect;

    srect.x = video_info->src_rect.x;
    srect.y = video_info->src_rect.y;
    srect.w = video_info->src_rect.w;
    srect.h = video_info->src_rect.h;
    drect.x = video_info->dst_rect.x;
    drect.y = video_info->dst_rect.y;
    drect.w = video_info->dst_rect.w;
    drect.h = video_info->dst_rect.h;

    if (alislvdec_mp_set_output_rect(decp->video_hdl, &srect, &drect))
    {
    AUI_DBG( "decore set display rect fail\n");
    rtn_code = AUI_RTN_FAIL;
    goto video_open_fail;
    }
    AUI_DBG( "set display rect, src<%ld %ld %ld %ld>, dst<%ld %ld %ld %ld>\n",
    video_info->src_rect.x, video_info->src_rect.y, video_info->src_rect.w, video_info->src_rect.h,
    video_info->dst_rect.x, video_info->dst_rect.y, video_info->dst_rect.w, video_info->dst_rect.h);
    }*/
    
    *decoder_out = (aui_hdl) decp;
    /* When PIP, there may be two av injecter video, the index equals 
     * the decv index. When no PIP, use default 0
     */
    decp->dev_priv_data.dev_idx = video_id;
    aui_dev_reg(AUI_MODULE_AV_INJECT_VIDEO, *decoder_out);
    return AUI_RTN_SUCCESS;

video_open_fail: 
    if (decp->pkt_hdr_hdl)
        alislsbm_close(decp->pkt_hdr_hdl);
    if (decp->pkt_data_hdl)
        alislsbm_close(decp->pkt_data_hdl);
    if (decp->dec_out_hdl)
        alislsbm_close(decp->dec_out_hdl);
    if (decp->disp_in_hdl)
        alislsbm_close(decp->disp_in_hdl);
    if (decp->video_hdl)
        alislvdec_close(decp->video_hdl);
    if (decp)
        free(decp);
        
    *decoder_out = 0;
    return rtn_code;
}

AUI_RTN_CODE aui_video_decoder_open(aui_video_decoder_init *video_info,
        aui_hdl *decoder_out) {
    return aui_video_decoder_open_internal(video_info, decoder_out, 0);
}

/** Close given decoder instance.
 *  \param decoder  decoder instance created with \e ali_video_decoder_open()
 */
AUI_RTN_CODE aui_video_decoder_close(aui_hdl decoder) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;
    alisl_handle sl_dis_hdl = NULL;
    enum vdec_dis_layer dis_layer = VDEC_DIS_MAIN_LAYER;
    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decore has been released\n");
    }

    if (alislvdec_mp_release(decp->video_hdl)) {
        aui_rtn(AUI_RTN_EINVAL, "decore release fail\n");
    }

    if (decp->pkt_hdr_hdl)
        alislsbm_close(decp->pkt_hdr_hdl);
    if (decp->pkt_data_hdl)
        alislsbm_close(decp->pkt_data_hdl);
    if (decp->dec_out_hdl)
        alislsbm_close(decp->dec_out_hdl);
    if (decp->disp_in_hdl)
        alislsbm_close(decp->disp_in_hdl);
    
    if (0 == alisldis_open(DIS_HD_DEV, &sl_dis_hdl)) {
        //close VPO to avoid blurred screen
        alislvdec_get_display_layer(decp->video_hdl, &dis_layer);
        
        if (dis_layer == VDEC_DIS_AUX_LAYER) {
            alisldis_win_onoff_by_layer(sl_dis_hdl, 0, DIS_LAYER_AUXP);
        } else {
            alisldis_win_onoff_by_layer(sl_dis_hdl, 0, DIS_LAYER_MAIN);
        }
        
        alisldis_close(sl_dis_hdl);
    }
    
    if (decp->video_hdl_need_close){
        alislvdec_close(decp->video_hdl);
    }

    if (decp->avsync_hdl) {    
        alislavsync_stop(decp->avsync_hdl);
        alislavsync_close(decp->avsync_hdl);
        decp->avsync_hdl = NULL;
    }
    
    aui_dev_unreg(AUI_MODULE_AV_INJECT_VIDEO, decp);
    free(decp);

    return AUI_RTN_SUCCESS;
}

/** Push Video ES packet header to ali video decoder.
 *  \param pkt_hdr, contains pts, dts, packet size, etc.
 *  \returns 0 on success, negative on error
 */
/*AUI_RTN_CODE aui_video_decoder_write_header(aui_video_decoder_id decoder, const struct aui_av_packet *pkt_hdr)
 {
 struct aui_private_video *decp = (struct aui_private_video *)decoder;

 if(decp == NULL)
 {
 aui_rtn(AUI_RTN_EINVAL, "decoder has not been opened\n");
 }

 if (alislsbm_write(decp->pkt_hdr_hdl, (unsigned char *)pkt_hdr, sizeof(struct aui_av_packet)))
 {
 return AUI_RTN_FAIL;
 }
 return AUI_RTN_SUCCESS;
 }*/

AUI_RTN_CODE aui_video_decoder_write_header(aui_hdl decoder,
        const struct aui_av_packet *pkt_hdr) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;
    aui_av_packet_complete_t av_packet;
    memset(&av_packet, 0, (sizeof(aui_av_packet_complete_t)));

    if ((decp == NULL) || (NULL == pkt_hdr)) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    /* Due to drivers spec, i need to pass the entire struct size, just customer
     * parameters are really considered */

    av_packet.dts = pkt_hdr->dts;
    av_packet.pts = pkt_hdr->pts;
    av_packet.size = pkt_hdr->size;
    //initialize avc sub_sample count 
    av_packet.nalu_num = (unsigned char)pkt_hdr->subsampe_enc_info.ul_subsample_count;
    
    if (pkt_hdr->flag == AUI_AV_PACKET_EXTRA_DATA) {
        av_packet.flags = AUI_AV_PKT_FLAG_CODEC_DATA;
    }

    if (alislsbm_write(decp->pkt_hdr_hdl, (unsigned char *) &av_packet,
            sizeof(aui_av_packet_complete_t))) {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

/** Push next fragment of PES packet to video decoder.
 *  \param buf  buffer with fragment of one PES packet
 *  \param size size of data in \a buf
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_video_decoder_write(aui_hdl decoder,
        const unsigned char *buf, unsigned long size) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (alislsbm_write(decp->pkt_data_hdl, (unsigned char *) buf, size)) {
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_video_decoder_flush(aui_hdl decoder,
        const aui_vdec_playback_mode vdec_playback_mode) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (decp->avsync_hdl) {
        if (alislavsync_flush(decp->avsync_hdl)) {
            aui_rtn(AUI_RTN_EIO,
                    "--> set av sync mode fail\n");
        }
    }
    if (alislvdec_mp_pause(decp->video_hdl)) {
        aui_rtn(AUI_RTN_FAIL, "decoder pause failed\n");
    }
    if (alislvdec_mp_flush(decp->video_hdl, vdec_playback_mode)) {
        aui_rtn(AUI_RTN_FAIL, "decoder flush failed\n");
    }

    if (alislvdec_mp_resume(decp->video_hdl)) {
        aui_rtn(AUI_RTN_FAIL, "decoder resume failed\n");
    }

    return AUI_RTN_SUCCESS;
}

/** Pause decoding/displaying video frames.
 *  \param pause pause/unpause
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_video_decoder_pause(aui_hdl decoder, int pause) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (pause) {
        if (alislvdec_mp_pause(decp->video_hdl)) {
            aui_rtn(AUI_RTN_FAIL,
                    "decore pause video fail\n");
        }
    } else {
        if (alislvdec_mp_resume(decp->video_hdl)) {
            aui_rtn(AUI_RTN_FAIL,
                    "decore resume video fail\n");
        }
    }

    return AUI_RTN_SUCCESS;
}

/** Enable/disable STC sync.
 *  With sync enabled, decoder displays video frames according to pts/stc difference.
 *  With sync disabled, decoder displays video frames according to vsync.
 *  \param enable  enable/disable STC sync
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_video_decoder_set_sync(aui_hdl decoder,
        int enable) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (enable) {
        if (decp->avsync_hdl) {
            if (alislavsync_set_av_sync_mode(decp->avsync_hdl, AVSYNC_AUDIO)) {
                aui_rtn(AUI_RTN_EIO,
                        "--> set av sync mode fail\n");
            }
        }
    } else {
        if (decp->avsync_hdl) {
            if (alislavsync_set_av_sync_mode(decp->avsync_hdl, AVSYNC_AV_FREERUN)) {
                aui_rtn(AUI_RTN_EIO,
                        "--> set av sync mode fail\n");
            }
        }
    }

    return AUI_RTN_SUCCESS;
}

/** Choose what frames are to be decoded.
 *  \param types see \e ali_video_frame_types
 *  \returns 0 on success, negative on error
 */
/*AUI_RTN_CODE aui_video_decoder_set_frame_types(aui_video_decoder_id decoder, aui_video_frame_types types)
 {
 struct aui_private_video *decp = (struct aui_private_video *)decoder;

 if(decp == NULL)
 {
 aui_rtn(AUI_RTN_EINVAL, "decoder has not been opened\n");
 }

 if (alislvdec_mp_set_frame_type(decp->video_hdl, types))
 {
 aui_rtn(AUI_RTN_FAIL, "decore set decode mode fail\n");
 }
 return AUI_RTN_SUCCESS;
 }*/

/** Get current status of decoder.
 *  \param[out] status  returned status
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_video_decoder_get_status(aui_hdl decoder,
        aui_video_decoder_status *status) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;
    struct vdec_mp_info decore_status;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    memset(&decore_status, 0, sizeof(decore_status));

    if (alislvdec_mp_get_info(decp->video_hdl, &decore_status)) {
        aui_rtn(AUI_RTN_FAIL,
                "decore get status fail\n");
    }
    status->width = decore_status.pic_width;
    status->height = decore_status.pic_height;
    status->sar_width = decore_status.sar_width;
    status->sar_height = decore_status.sar_heigth;
    status->fps = decore_status.frame_rate;
    status->interlaced = decore_status.interlaced_frame;
    status->frames_decoded = decore_status.frames_decoded;
    status->frames_displayed = decore_status.frames_displayed;
    status->last_pts = decore_status.frame_last_pts;
    status->buffer_size = decore_status.buffer_size;
    status->buffer_used = decore_status.buffer_used;
    status->display_layer = decore_status.layer;
    if (decore_status.layer == VDEC_DIS_AUX_LAYER) {
        status->display_layer = AUI_DIS_LAYER_AUXP;    
    } else {
        status->display_layer = AUI_DIS_LAYER_VIDEO;
    }
    return AUI_RTN_SUCCESS;
}

/** Get current buffer status of decoder.
 *  \param[out] buffer_status  returned buffer status
 *  \returns 0 on success, negative on error
 */
AUI_RTN_CODE aui_video_decoder_get_buffer_status(aui_hdl decoder,
        aui_decoder_buffer_status *buffer_status) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;
    struct vdec_info vdec_stat;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    memset(&vdec_stat, 0, sizeof(vdec_stat));

    alislsbm_get_total_size(decp->pkt_data_hdl, &buffer_status->total_size);
    alislsbm_get_valid_size(decp->pkt_data_hdl, &buffer_status->valid_size);
    alislsbm_get_free_size(decp->pkt_data_hdl, &buffer_status->free_size);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_video_decoder_write_nal_header(aui_hdl decoder,
        const unsigned char *buf, unsigned long size) {
    struct aui_private_video *decp = (struct aui_private_video *) decoder;

    if (decp == NULL) {
        aui_rtn(AUI_RTN_EINVAL,
                "decoder has not been opened\n");
    }

    if (alislsbm_write(decp->pkt_hdr_hdl, (unsigned char *) buf, size)) {
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

