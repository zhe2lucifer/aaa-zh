#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "aui_ini_config.h"
#else
#include <api/libfs2/stdio.h>
#endif

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_stc.h>
#include <aui_av_injecter.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_test_stream.h"
#include "aui_pip_test.h"
#include "aui_pip_test_media.h"
#include "aui_av_injecter_test.h"

struct pip_test_media_info {
    struct pip_test_media *media;
    struct ali_app_pip_media_init_para* init_para;
    struct ali_aui_hdls *pip_hdls;
};

struct pip_test_media_info media_info[2];
static pthread_mutex_t m_play_audio = PTHREAD_MUTEX_INITIALIZER;

int test_aui_pip_init_para_for_media(unsigned long *argc,char **argv, 
                unsigned char dis_layer, unsigned char decv_id, struct ali_app_pip_media_init_para *init_para)
{
    if (init_para == NULL) {
        return -1;
    }
    if (*argc > 0) {
        init_para->filepath = strdup(argv[0]);
    }
    if (*argc > 1) {
        init_para->video_format = atoi(argv[1]);
    } 
    if (*argc > 2) {
        init_para->audio_format = atoi(argv[2]);
    }
	
    if (*argc > 4) {
        init_para->video_frame_rate = atoi(argv[4]);
    }
	
    init_para->video_id = decv_id;
    init_para->dis_layer = dis_layer;
    return 0;
}

static void *test_aui_pip_media_play_thread(void *argv)
{
    struct pip_test_media_info *media_info = (struct pip_test_media_info *)argv;
    struct pip_test_media * media = (struct pip_test_media *)media_info->media;
    struct ali_app_pip_media_init_para *init_para = media_info->init_para;
    int first_play = media->first_play;
    FILE *f_video_data = NULL, *f_video_pkt = NULL,*f_video_pts = NULL;
    FILE *f_audio_data = NULL, *f_audio_pkt = NULL,*f_audio_pts = NULL;
    unsigned int video_data_size=0, video_pkt_size=0, video_extra_data_size=0, video_pts_size=0;
    unsigned int audio_data_size=0, audio_pkt_size=0, audio_extra_data_size=0, audio_pts_size=0;
    unsigned int video_frame_cnt = 0, video_free_size = 0, video_valid_size = 0, video_total_size = 0;
    unsigned int audio_frame_cnt = 0, audio_free_size = 0, audio_valid_size = 0, audio_total_size = 0;
    unsigned int video_read_size = 0, audio_read_size = 0, video_need_read = 1, audio_need_read = 1, ret=0;
    unsigned int video_frm_size=0, audio_frm_size=0, video_frm_pts=0, audio_frm_pts=0;
    unsigned char *video_frame_buf = NULL, *video_extra_data_buf = NULL;
    unsigned char *audio_frame_buf = NULL, *audio_extra_data_buf = NULL;
    
    aui_video_decoder_init video_decoder_init;    
    aui_hdl video_decoder_out = 0;
    aui_decv_format video_codec_id = init_para->video_format;
    aui_deca_stream_type audio_codec_id = init_para->audio_format;
    aui_hdl audio_decoder_out = 0;
    aui_audio_info_init audio_info_init;
    
    struct aui_av_packet pkt;
    aui_hdl decv_hdl = NULL, stc_hdl = NULL, snd_hdl = NULL, dis_hdl = NULL;
    aui_attr_snd attr_snd;
    aui_attr_decv attr_decv;    
    aui_attr_dis attr_dis;

    aui_video_decoder_status v_status;
    unsigned int frames_displayed_last = 0;
    
    memset(&pkt, 0, sizeof(pkt));
    memset(&video_decoder_init, 0, sizeof(video_decoder_init));
    printf("get video file info\n");
    ret = test_av_inject_get_file_info(init_para->filepath, ".video0", &f_video_data, &f_video_pkt, 
            &f_video_pts, &video_pkt_size, &video_extra_data_buf, &video_extra_data_size);
    printf("video: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
        f_video_data, f_video_pkt, f_video_pts, video_pkt_size, video_extra_data_size);     
    if (ret != 0){
        goto exit;
    }
    
    printf("get audio file info\n");
    ret = test_av_inject_get_file_info(init_para->filepath, ".audio0", &f_audio_data, &f_audio_pkt, 
            &f_audio_pts, &audio_pkt_size, &audio_extra_data_buf, &audio_extra_data_size);
    printf("audio: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
        f_audio_data, f_audio_pkt, f_audio_pts, audio_pkt_size, audio_extra_data_size);     
    if (ret != 0){
        goto exit;
    }
    //Video frame buffer
    video_frame_buf = (unsigned char *)malloc(0x100000);
    if(video_frame_buf == NULL) 
    {
        printf("Malloc video frame buffer fail\n");
        goto exit;;
    }

    //Audio frame buffer
    audio_frame_buf = (unsigned char *)malloc(0x100000);
    if(audio_frame_buf == NULL) 
    {
        printf("Malloc audio frame buffer fail\n");
        goto exit;;
    }

    memset(&attr_dis, 0, sizeof(aui_attr_dis));
    attr_dis.uc_dev_idx = AUI_DIS_HD;
    
    MEMSET(&attr_dis, 0, sizeof(aui_attr_dis));
    if(aui_find_dev_by_idx(AUI_MODULE_DIS, 0, &dis_hdl)) {/*if dis device has opened,return dis device handle*/
        if (aui_dis_open(&attr_dis, &dis_hdl)) {/*if dis hasn't opened,open dis device and return dis device handle*/
            AUI_PRINTF("\n aui_dis_open fail\n");
             goto exit;
        }
    }
    aui_dis_layer_enable(dis_hdl, init_para->dis_layer, 0);
    
    // open DECV for av inject and set it to av inject
    memset(&attr_decv,0,sizeof(aui_attr_decv));
    attr_decv.uc_dev_idx =  init_para->video_id;
    attr_decv.dis_layer = init_para->dis_layer;
    if(aui_find_dev_by_idx(AUI_MODULE_DECV, attr_decv.uc_dev_idx, &media->decv_hdl)) {
        printf("===== aui_decv_open ======\n");
        if (aui_decv_open(&attr_decv,&media->decv_hdl)) {
            AUI_PRINTF("\n aui_decv_open fail\n");
            goto exit;
        }
    }
    /* init decoder */
    video_decoder_init.decv_handle = media->decv_hdl;
    video_decoder_init.decode_mode = AUI_VIDEO_DEC_MODE_FRAME;
    video_decoder_init.codec_id    = video_codec_id;
    video_decoder_init.extradata_size = video_extra_data_size;
    video_decoder_init.extradata = video_extra_data_buf;
    video_decoder_init.frame_rate = init_para->video_frame_rate;

    if( AUI_RTN_SUCCESS != aui_video_decoder_open(&video_decoder_init, 
        &video_decoder_out)) {
        printf("open video decoder fail\n");
        goto exit;
    }

    test_aui_pip_set_dis_rect(attr_decv.dis_layer, NULL, NULL);
    //} else {
    //  video_need_read = 0;
    //}
init_audio:    
    // Audio decorder init
    pthread_mutex_lock(&m_play_audio);
    printf("====media init audio %d===\n", media->play_audio);

    //for no audio playing
    if (first_play) {
        test_aui_pip_set_avsync(&stc_hdl, AUI_AV_DATA_TYPE_ES,
            AUI_STC_AVSYNC_AUDIO, media->decv_hdl);     
    }
    if (media->play_audio) {
        //set sync mode
        test_aui_pip_set_avsync(&stc_hdl, AUI_AV_DATA_TYPE_ES,
            AUI_STC_AVSYNC_AUDIO, media->decv_hdl); 
        if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl))
        {
            memset(&attr_snd,0,sizeof(aui_attr_snd));
            aui_snd_open(&attr_snd, &snd_hdl);
        }
        //set audio volume
        aui_snd_mute_set(snd_hdl, 0);
        aui_snd_vol_set(snd_hdl, 30);

        memset(&audio_info_init, 0, sizeof(audio_info_init));
        audio_info_init.channels = 2;
        audio_info_init.codec_id = audio_codec_id;
        audio_info_init.nb_bits_per_sample = 16;
        audio_info_init.extradata = audio_extra_data_buf;
        audio_info_init.extradata_size = audio_extra_data_size;
        audio_info_init.sample_rate = 48000;

        if( AUI_RTN_SUCCESS != aui_audio_decoder_open(&audio_info_init, (aui_hdl*)&media->audio_hdl))
        {
            printf("open audio decoder fail\n");
            ret = -1;
            pthread_mutex_unlock(&m_play_audio);
            goto exit;
        }
        audio_decoder_out = (aui_hdl)media->audio_hdl;
        if(AUI_RTN_SUCCESS != aui_audio_decoder_set_sync(audio_decoder_out, 1)) {
            printf("aui_audio_decoder_set_sync fail\n");
            pthread_mutex_unlock(&m_play_audio);
            goto exit;;
        }
    }
    pthread_mutex_unlock(&m_play_audio);
 
    while(media->loop) {

        if(video_need_read) {
            /* parse a complete video frame(frame size and frame pts) */
            ret = fread(&video_frm_size, 1, sizeof(unsigned int), f_video_pkt);
            if (ret <= 0){
                printf("%s,%d,ret:%d,f_video_pkt end or err.\n", __FUNCTION__, __LINE__, ret);
            }

            video_read_size = fread(video_frame_buf, 1, video_frm_size, f_video_data);
            if (video_read_size <= 0){
                printf("%s,%d,ret:%d,f_video_data end or err.\n", __FUNCTION__, __LINE__, video_read_size);
            }
            
            ret = fread(&video_frm_pts, 1, sizeof(unsigned int), f_video_pts);
            if (ret <= 0){
                printf("%s,%d,ret:%d,f_video_pts end or err.\n", __FUNCTION__, __LINE__, ret);
            }   
        }
        
        if((video_read_size > 0) && (video_read_size == video_frm_size)) {
            pkt.pts = video_frm_pts;
            pkt.dts = AUI_AV_NOPTS_VALUE;
            pkt.size = video_read_size;
            ret = test_av_inject_write_video_data((void *)video_decoder_out, &pkt, video_frame_buf, video_read_size);

            if (ret != 0) {
                //write fail, retry
                //printf("write video fail retry\n");
                video_need_read = 0;
                usleep(25*1000);
            } else {              
                if(++video_frame_cnt >= video_pkt_size/4) {
                    video_need_read = 0;
                } else {
                    video_need_read = 1;
                }
                video_read_size = 0;
            }
        } 
        
        pthread_mutex_lock(&m_play_audio);
        if (media->play_audio) {
            if (media->audio_hdl == NULL) {
                pthread_mutex_unlock(&m_play_audio);
                //audio seek to the pos of current video pts
                aui_video_decoder_status video_status;
                aui_video_decoder_get_status(video_decoder_out, &video_status);
                audio_frame_cnt = test_av_inject_seek2pts(f_audio_data, f_audio_pkt, f_audio_pts, video_status.last_pts + 800);
                AUI_PRINTF("change audio track find the seek pts\n");
                goto init_audio;
            }
            
            if(audio_need_read) {      
                //printf("%s,%d\n", __FUNCTION__, __LINE__);
                /* parse a complete audio frame */
                ret = fread(&audio_frm_size, 1, sizeof(unsigned int), f_audio_pkt);
                if (ret <= 0){
                    printf("%s,%d,ret:%d,f_audio_pkt end or err.\n", __FUNCTION__, __LINE__, ret);
                }
                
                audio_read_size = fread(audio_frame_buf, 1, audio_frm_size, f_audio_data);
                if (audio_read_size <= 0){
                    printf("%s,%d,ret:%d,f_audio_data end or err.\n", __FUNCTION__, __LINE__, audio_read_size);
                }   
                
                ret = fread(&audio_frm_pts, 1, sizeof(unsigned int), f_audio_pts);
                if (ret <= 0){
                    printf("%s,%d,ret:%d,f_audio_pts end or err.\n", __FUNCTION__, __LINE__, ret);
                }   
            }

            if((audio_read_size > 0) && (audio_read_size == audio_frm_size)) {
                pkt.pts = audio_frm_pts;
                pkt.dts = AUI_AV_NOPTS_VALUE;
                pkt.size = audio_read_size;
                ret = test_av_inject_write_audio_data((void *)audio_decoder_out, &pkt, audio_frame_buf, audio_read_size);

                if (ret != 0) {
                    //write fail, retry
                    printf("write audio fail\n");
                    audio_need_read = 0;
                    usleep(5*1000);
                } else {
                    if(++audio_frame_cnt >= audio_pkt_size/4) {
                        audio_need_read = 0;
                    } else {
                        audio_need_read = 1;
                    }
                    audio_read_size = 0;
                }
            } 
        }
        pthread_mutex_unlock(&m_play_audio);

        if (video_frame_cnt >= video_pkt_size/4) {
            
            if(media->play_audio == 0) {
                break;
            } else if ((audio_frame_cnt >= audio_pkt_size/4) && (media->play_audio == 1)){
                break;
            }
        }
    }

    do {
        usleep(200*1000);       
        aui_video_decoder_get_status(video_decoder_out, &v_status);
        printf("frames displayed %d\n", v_status.frames_displayed);

        if(v_status.frames_displayed != frames_displayed_last){
            frames_displayed_last = v_status.frames_displayed;
        }else{
            break;
        }
        
        pthread_mutex_lock(&m_play_audio);
        if (media->play_audio) {
            if (media->audio_hdl == NULL) {
                pthread_mutex_unlock(&m_play_audio);
                //audio seek to the pos of current video pts
                aui_video_decoder_status video_status;
                aui_video_decoder_get_status(video_decoder_out, &video_status);
                audio_frame_cnt = test_av_inject_seek2pts(f_audio_data, f_audio_pkt, f_audio_pts, video_status.last_pts + 800);
                AUI_PRINTF("change audio track find the seek pts\n");
                goto init_audio;
            }
        }   
        pthread_mutex_unlock(&m_play_audio);
    } while(media->loop);
exit:
    pthread_mutex_lock(&m_play_audio);
    if (media->loop) {
        media->loop = 0;
    }
    if (video_decoder_out) {
        printf("===== aui_video_decoder_close =====\n");
        aui_video_decoder_close(video_decoder_out);    
        video_decoder_out = 0;
        printf("===== aui_decv_close ======\n");
        aui_decv_close(NULL, &media->decv_hdl);     
        media->decv_hdl = NULL;
    }

       
    if (media->play_audio) {
        if (media->audio_hdl) {
            printf("===== aui_audio_decoder_close =====\n");
            aui_audio_decoder_close((aui_hdl)media->audio_hdl);
            media->audio_hdl = 0;
        }
    }
    if (f_video_data)
        fclose(f_video_data);
    if (f_video_pkt)    
        fclose(f_video_pkt);
    if (f_video_pts)    
        fclose(f_video_pts);
    if (f_audio_data)   
        fclose(f_audio_data);
    if (f_audio_pkt)    
        fclose(f_audio_pkt);
    if (f_audio_pts)    
        fclose(f_audio_pts);
    if (video_frame_buf)
        free(video_frame_buf);
    if (audio_frame_buf)    
        free(audio_frame_buf);
    if (video_extra_data_buf) 
        free(video_extra_data_buf);
    if (audio_extra_data_buf)
        free(audio_extra_data_buf); 
    pthread_mutex_unlock(&m_play_audio);
    printf("media play done!");
    return NULL;
}

int test_aui_pip_play_media(struct ali_app_pip_media_init_para* init_para, 
    pip_test_media *media)
{
    printf("start to create media thread !\n");
    if (media == NULL) {
        AUI_PRINTF("invalid media info\n");
    }
    media_info[media->id].media = media;
    media_info[media->id].init_para = init_para;
    if (media->media_thread) {
        media->loop = 0;
        pthread_join(media->media_thread, NULL);
        media->media_thread = 0;
    }
    media->loop = 1;
    if (pthread_create(&media->media_thread, NULL, test_aui_pip_media_play_thread, &media_info[media->id])) {
        AUI_PRINTF("create thread fail\n");
        return -1;
    }

    return 0;
}

int test_aui_pip_stop_media(pip_test_media *media)
{
    if (media == NULL) {
        AUI_PRINTF("invalid media info\n");
    }
    media->loop = 0;
    if (media->media_thread) {
        pthread_join(media->media_thread, NULL);
        media->media_thread = 0;
    }

    return 0;
}

int test_aui_pip_start_media_audio(pip_test_media *media)
{
    if (media == NULL) {
        AUI_PRINTF("invalid media info\n");
    }
    media->play_audio = 1;
    return 0;
}


int test_aui_pip_stop_media_audio(pip_test_media *media)
{
    if (media == NULL) {
        AUI_PRINTF("invalid media info\n");
        return -1;
    }
    pthread_mutex_lock(&m_play_audio);
    media->play_audio = 0;
    if (media->audio_hdl) {
        aui_audio_decoder_close((aui_hdl)media->audio_hdl);
        media->audio_hdl = 0;
    }
    pthread_mutex_unlock(&m_play_audio);
    return 0;
}
