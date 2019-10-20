/**@file
*    @brief 		AUI decv module interface implement
*    @author		Nick.Chiu
*    @date			2015-7-14
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"

#include <aui_mp.h>
#include <string.h>
#include "goplayer.h"

AUI_MODULE(MPLAY)

typedef struct
{
    aui_mp_message_callback msgCb; /**< Message callback function*/
    void *user_data;    /**< User private data for message callback.*/
}stCALLBACK;

//static pn_message_callback pe_callback[AUI_MP_ERROR_NONE] = {0};  //a legacy way
//static aui_mp_message_callback g_msgCb;  // new way
typedef enum
{
    GO_STATE_OPEN,
    GO_STATE_CLOSE    
}eGO_STATE; 

typedef struct
{
    char *uri;
//    aui_mp_message_callback msgCb; /**< Message callback function*/
//    void *user_data;    /**< User private data for message callback.*/
    stCALLBACK callback; /**< callback stuff.*/
    int start_time;  /**< The media time when start to play.*/
    eSTREAM_PROTOCOL stream_protocol; /**< VOD or LIVE stream.*/
    pn_message_callback pe_callback[AUI_MP_ERROR_NONE];  /** < a legacy callback way. */
    eGO_STATE go_state; /**< open/close state of goplayer.  */
}stPLAYER_HANDLE;

//I know our API try to meet multi-instance, but actually our SW stack cannot support multi-instance currently.
//Use a global player handle for now.
static stPLAYER_HANDLE *g_pPlayer_handle = NULL;
static aui_mp_stream_info *tmp_info = NULL;

void _free_audio_track_info(aui_mp_audio_track_info* audio_track_info, unsigned int cnt)
{
    if(audio_track_info)
    {
        unsigned int i;
        for(i = 0; i < cnt; i++)
        {
            if(audio_track_info[i].audDetailInfo)
            {
                free(audio_track_info[i].audDetailInfo);
            }
        }
        free(audio_track_info);
    }
}

//new callback way is prior to legacy one.
void cb_func(eGOPLAYER_CALLBACK_TYPE type, void *data)
{
    switch (type)
    {
        case eGOPLAYER_CBT_STATE_CHANGE:
        {
            eGOPLAYER_STATE state = (eGOPLAYER_STATE)(int)data;
            /*
            * Nick: haven't finished yet. For AUI_MP_PLAY_BEGIN:
                ¡Estart to play at beginning.
                ¡Eafter seek done.
                ¡Enormal playback from trick mode.
            */
            if(state == eGOPLAYER_STATE_PLAY)
            {
                if(g_pPlayer_handle->callback.msgCb)
                {
                    g_pPlayer_handle->callback.msgCb(AUI_MP_PLAY_BEGIN, data, g_pPlayer_handle->callback.user_data);
                }                
                else if(g_pPlayer_handle->pe_callback[AUI_MP_PLAY_BEGIN] != NULL)
                {
                    (g_pPlayer_handle->pe_callback[AUI_MP_PLAY_BEGIN])();
                }
            }
            break;
        }
        case eGOPLAYER_CBT_WARN_TRICK_BOS:
        case eGOPLAYER_CBT_WARN_TRICK_EOS:
        case eGOPLAYER_CBT_FINISHED:
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_PLAY_END, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_PLAY_END] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_PLAY_END])();
            }
            break;
        }
        case eGOPLAYER_CBT_WARN_UNSUPPORT_VIDEO:
        
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_VIDEO_CODEC_NOT_SUPPORT, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_VIDEO_CODEC_NOT_SUPPORT] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_VIDEO_CODEC_NOT_SUPPORT])();
            }
            break;
        }
        case eGOPLAYER_CBT_WARN_UNSUPPORT_AUDIO:
        
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_AUDIO_CODEC_NOT_SUPPORT, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT])();
            }
            break;
        }
        case eGOPLAYER_CBT_WARN_DECODE_ERR_VIDEO:
        case eGOPLAYER_CBT_WARN_DECODE_ERR_AUDIO:
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_DECODE_ERROR, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_DECODE_ERROR] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_DECODE_ERROR])();
            }
            break;
        }
        case eGOPLAYER_CBT_ERR_UNDEFINED:
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_ERROR_UNKNOWN, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_ERROR_UNKNOWN] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_ERROR_UNKNOWN])();
            }
            break;
        }
        case eGOPLAYER_CBT_BUFFERING:
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_BUFFERING, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_BUFFERING] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_BUFFERING])();
            }
            break;
        }
        case eGOPLAYER_CBT_ERR_SOUPHTTP:
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_ERROR_SOUPHTTP, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_ERROR_SOUPHTTP] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_ERROR_SOUPHTTP])();
            }
            break;
        }
        case eGOPLAYER_CBT_FRAME_CAPTURE:
        {
            if(g_pPlayer_handle->callback.msgCb)
            {
                g_pPlayer_handle->callback.msgCb(AUI_MP_FRAME_CAPTURE, data, g_pPlayer_handle->callback.user_data);
            }
            else if(g_pPlayer_handle->pe_callback[AUI_MP_FRAME_CAPTURE] != NULL)
            {
                (g_pPlayer_handle->pe_callback[AUI_MP_FRAME_CAPTURE])();
            }
            break;
        }
        default:
            //Should not be here
            AUI_ERR("Cannot match registered callback!!\n");
            break;
    }
}
AUI_RTN_CODE aui_mp_open(aui_attr_mp *pst_mp_attr,void **pp_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
        //stPLAYER_HANDLE *pPlayer_handle;
    if(g_pPlayer_handle == NULL)
    {
        if(goplayer_open(cb_func) == 0)
        {//success
            g_pPlayer_handle = (stPLAYER_HANDLE *)malloc(sizeof(stPLAYER_HANDLE));
            if(g_pPlayer_handle)
            {
                g_pPlayer_handle->uri = strdup((char*)pst_mp_attr->uc_file_name);
                g_pPlayer_handle->callback.msgCb = pst_mp_attr->aui_mp_stream_cb;
                g_pPlayer_handle->callback.user_data = pst_mp_attr->user_data;
                //pPlayer_handle->msgCb = pst_mp_attr->aui_mp_stream_cb;
                //pPlayer_handle->user_data = pst_mp_attr->user_data;
                g_pPlayer_handle->start_time = pst_mp_attr->start_time;
                g_pPlayer_handle->go_state = GO_STATE_OPEN;
                memset(g_pPlayer_handle->pe_callback, 0, sizeof(g_pPlayer_handle->pe_callback));
                switch(pst_mp_attr->stream_protocol)
                {
                    case AUI_MP_STREAM_PROTOCOL_LIVE:
                    {
                        g_pPlayer_handle->stream_protocol = eSTREAM_PROTOCOL_LIVE;
                        break;
                    }
                    case AUI_MP_STREAM_PROTOCOL_VOD:
                    {
                        g_pPlayer_handle->stream_protocol = eSTREAM_PROTOCOL_VOD;
                        break;
                    }
                    default:
                        g_pPlayer_handle->stream_protocol = eSTREAM_PROTOCOL_UNKNOW;
                        break;
                }
                *pp_handle_mp = (void*)g_pPlayer_handle;
                //reset registered callback function
                //memset(pe_callback, 0, sizeof(pe_callback));
                ret = AUI_RTN_SUCCESS;                 
            }
            else
            {
                goplayer_close();
                AUI_ERR("allocation of player handle failed!!\n");
            }
        }    
        else
            AUI_ERR("goplayer_open() failed!!\n");
    }
    else
    {//somebody forget to call aui_mp_close.
        AUI_ERR("player handle exist!!\n");
    }

    return ret;
}

AUI_RTN_CODE aui_mp_close(aui_attr_mp *pst_mp_attr,void **pp_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;

    if(pst_mp_attr != NULL)
    {
    	AUI_ERR("pst_mp_attr:%p may not be used!\n", pst_mp_attr);
    }
    
    if(pp_handle_mp && *pp_handle_mp)
    {
        stPLAYER_HANDLE *pPlayer_handle = (stPLAYER_HANDLE *)*pp_handle_mp;
        if(g_pPlayer_handle == pPlayer_handle)
        {//yes, it's right handle
            aui_mp_stop(*pp_handle_mp);
            free(pPlayer_handle->uri);
            free(pPlayer_handle);
            g_pPlayer_handle = *pp_handle_mp = NULL;
            ret = AUI_RTN_SUCCESS;
        }
        else
        {
            AUI_ERR("player handle doesn't match!!\n");
        }
    }
    else
        AUI_ERR("invalid parameter, pst_mp_attr:%p, *pp_handle_mp:%p\n", pst_mp_attr, *pp_handle_mp);

    return ret;
}

AUI_RTN_CODE aui_mp_start(void *pv_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        stPLAYER_HANDLE *pPlayer_handle = (stPLAYER_HANDLE *)pv_handle_mp;

        if(g_pPlayer_handle->go_state == GO_STATE_CLOSE)
        {
            if(goplayer_open(cb_func) != 0)
            {
                AUI_ERR("media player restart failed!!\n");
                goto Rome;
            }
        }

        if(goplayer_set_source_uri(pPlayer_handle->uri, 
            pPlayer_handle->start_time, 
            pPlayer_handle->stream_protocol))
        {
            ret = AUI_RTN_SUCCESS;
            //start time only for first time start to play.
            pPlayer_handle->start_time = 0;            
        }
    }
Rome:
    return ret;
}

AUI_RTN_CODE aui_mp_stop(void *pv_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        if(goplayer_close() == 0)
        {
            g_pPlayer_handle->go_state = GO_STATE_CLOSE;
            ret = AUI_RTN_SUCCESS;
        }
    }

    return ret;
}

AUI_RTN_CODE aui_mp_pause(void *pv_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        if(goplayer_play(0))
            ret = AUI_RTN_SUCCESS;
    }

    return ret;
}

AUI_RTN_CODE aui_mp_resume(void *pv_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        if(goplayer_play(1))
            ret = AUI_RTN_SUCCESS;
    }
    return ret;
}

AUI_RTN_CODE aui_mp_init(aui_func_mp_init fn_mp_init)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL == fn_mp_init)
    {
        aui_rtn(AUI_RTN_EINVAL,"fn_mp_init is null !\n");
    }
    fn_mp_init();
    return rtn_code;
}

AUI_RTN_CODE aui_mp_de_init(aui_func_mp_init fn_mp_de_init)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL == fn_mp_de_init)
    {
        aui_rtn(AUI_RTN_EINVAL,"fn_mp_de_init is null!\n");
    }
    fn_mp_de_init();
    return rtn_code;
}

AUI_RTN_CODE aui_mp_seek(void *pv_handle_mp, unsigned long ul_time_in_ms)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        if(goplayer_seek(ul_time_in_ms))
        {
            ret = AUI_RTN_SUCCESS;
        }
    }   
    
    return ret;
}

#define TRANS_RATE_UNKNOWN 255
// translate enum aui_mp_speed to goplayer play rate.
static int transRateFromAui2Go(enum aui_mp_speed en_speed)
{
    int rate = TRANS_RATE_UNKNOWN;
    switch(en_speed)
    {
        case AUI_MP_SPEED_1:
            rate = 1;
            break;
        case AUI_MP_SPEED_0:
            rate = 0;
            break;    
        case AUI_MP_SPEED_FASTREWIND_24:
            rate = -24;
            break;
        case AUI_MP_SPEED_FASTREWIND_16:
            rate = -16;
            break;
        case AUI_MP_SPEED_FASTREWIND_8:
            rate = -8;
            break;
        case AUI_MP_SPEED_FASTREWIND_4:
            rate = -4;
            break;
        case AUI_MP_SPEED_FASTREWIND_2:
            rate = -2;
            break;
        case AUI_MP_SPEED_FASTFORWARD_2:
            rate = 2;
            break;
        case AUI_MP_SPEED_FASTFORWARD_4:
            rate = 4;
            break;
        case AUI_MP_SPEED_FASTFORWARD_8:
            rate = 8;
            break;
        case AUI_MP_SPEED_FASTFORWARD_16:
            rate = 16;
            break;
        case AUI_MP_SPEED_FASTFORWARD_24:
            rate = 24;
            break;
        default:

            break;
    }

    return rate;    
}
// translate play rate from goplayer style to AUI mp style
//return 0:Failed. Otherwise: success.
static int TransRateFromGo2Aui(int goRate, enum aui_mp_speed *pSpeed)
{
    int ret = TRUE;
    switch(goRate)
    {
        case 0:
        {
            *pSpeed = AUI_MP_SPEED_0;
            break;
        }
        case 1:
        {
            *pSpeed = AUI_MP_SPEED_1;
            break;
        }
        case 2:
        {
            *pSpeed = AUI_MP_SPEED_FASTFORWARD_2;
            break;
        }
        case 4:
        {
            *pSpeed = AUI_MP_SPEED_FASTFORWARD_4;
            break;
        }
        case 8:
        {
            *pSpeed = AUI_MP_SPEED_FASTFORWARD_8;
            break;
        }
        case 16:
        {
            *pSpeed = AUI_MP_SPEED_FASTFORWARD_16;
            break;
        }
        case 24:
        {
            *pSpeed = AUI_MP_SPEED_FASTFORWARD_24;
            break;
        }
        case -2:
        {
            *pSpeed = AUI_MP_SPEED_FASTREWIND_2;
            break;
        }
        case -4:
        {
            *pSpeed = AUI_MP_SPEED_FASTREWIND_4;
            break;
        }
        case -8:
        {
            *pSpeed = AUI_MP_SPEED_FASTREWIND_8;
            break;
        }
        case -16:
        {
            *pSpeed = AUI_MP_SPEED_FASTREWIND_16;
            break;
        }
        case -24:
        {
            *pSpeed = AUI_MP_SPEED_FASTREWIND_24;
            break;
        }
        default:
            ret = FALSE;
            break;
    }

    return ret;
}
AUI_RTN_CODE aui_mp_speed_set(void *pv_handle_mp, enum aui_mp_speed en_speed)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        int rate = transRateFromAui2Go(en_speed);
        if(rate != TRANS_RATE_UNKNOWN)
        {
            if(goplayer_play(rate))
            {
                ret = AUI_RTN_SUCCESS;
            }
        }
        else
        {
            AUI_ERR("Invalid play rate!!\n");
        }
    }   
    
    return ret;
}

AUI_RTN_CODE aui_mp_total_time_get (void *pv_handle_mp, unsigned int *pui_total_time)
{
    AUI_RTN_CODE ret = AUI_RTN_EINVAL;
    if(pv_handle_mp && pui_total_time)
    {
        int total_time = goplayer_get_total_time();
        if(total_time == -1)
        {// total time unknown
            ret = AUI_RTN_HELP;
        }
        else
        {
            *pui_total_time = (unsigned int)total_time/1000;
            ret = AUI_RTN_SUCCESS;
        }
    }
    
    return ret;
}

AUI_RTN_CODE aui_mp_cur_time_get (void *pv_handle_mp, unsigned int *pui_cur_time)
{
   AUI_RTN_CODE ret = AUI_RTN_EINVAL;
    if(pv_handle_mp && pui_cur_time)
    {
        int curl_time, i = 0;
        while((curl_time = goplayer_get_current_time()) == -1)
        {
            usleep(500000);
            if(++i >= 4)
                break;
        }

        if(curl_time == -1)
        {// current time unknown
            ret = AUI_RTN_FAIL;
        }
        else
        {
            *pui_cur_time = (unsigned int)curl_time/1000;
            ret = AUI_RTN_SUCCESS;
        }
    }

    return ret;
}

AUI_RTN_CODE aui_mp_get_total_time(void *pv_handle_mp, unsigned int *pui_total_time_in_ms)
{
    AUI_RTN_CODE ret = AUI_RTN_EINVAL;
    if(pv_handle_mp && pui_total_time_in_ms)
    {
        int total_time = goplayer_get_total_time();
        if(total_time == -1)
        {// total time unknown
            ret = AUI_RTN_HELP;
        }
        else
        {
            *pui_total_time_in_ms = (unsigned int)total_time;
            ret = AUI_RTN_SUCCESS;
        }
    }
    
    return ret;
}

AUI_RTN_CODE aui_mp_get_cur_time(void *pv_handle_mp, unsigned int *pui_cur_time_in_ms)
{
    AUI_RTN_CODE ret = AUI_RTN_EINVAL;
    if(pv_handle_mp && pui_cur_time_in_ms)
    {
        int curl_time, i = 0;
        while((curl_time = goplayer_get_current_time()) == -1)
        {
            usleep(500000);
            if(++i >= 4)
                break;
        }

        if(curl_time == -1)
        {// current time unknown
            ret = AUI_RTN_FAIL;
        }
        else
        {
            *pui_cur_time_in_ms = (unsigned int)curl_time;
            ret = AUI_RTN_SUCCESS;
        }
    }

    return ret;
}

AUI_RTN_CODE aui_mp_set_message_callback(void *pv_handle_mp, enum aui_mp_message msg, pn_message_callback func)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
//        stPLAYER_HANDLE *pPlayer_handle = (stPLAYER_HANDLE *)pv_handle_mp;
//        pPlayer_handle->pe_callback[msg] = func;
        switch(msg)
        {//supported msg
            case AUI_MP_PLAY_BEGIN:
            case AUI_MP_PLAY_END:
            case AUI_MP_VIDEO_CODEC_NOT_SUPPORT:
            case AUI_MP_AUDIO_CODEC_NOT_SUPPORT:
            case AUI_MP_DECODE_ERROR:
            case AUI_MP_ERROR_UNKNOWN:
            case AUI_MP_BUFFERING:
            case AUI_MP_ERROR_SOUPHTTP:
            case AUI_MP_FRAME_CAPTURE:
                g_pPlayer_handle->pe_callback[msg] = func;
                ret = AUI_RTN_SUCCESS;

                break;
            default:
                ret = AUI_RTN_EINVAL;
                break;
        }
    }

    return ret;
}

AUI_RTN_CODE aui_mp_set(void *pv_hdl_mp,unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;

    if(pv_hdl_mp && pv_param)
    {
        switch(ul_item)
        {
            case AUI_MP_SET_AUDIO_ID:
                ret = aui_mp_change_audio(pv_hdl_mp, *(int*)pv_param);
                break;

            case AUI_MP_SET_SUBTITLE_ID:
                ret = aui_mp_change_subtitle(pv_hdl_mp, *(int*)pv_param);
                break;
            default:
                AUI_ERR("Invalid value!! Value=%lu\n", ul_item);
                break;
        }
    }

    return ret;
}

AUI_RTN_CODE aui_mp_change_audio(void *pv_handle_mp, int trackIdx)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_change_audio(trackIdx);
        ret = AUI_RTN_SUCCESS;
    }

    return ret;
}

AUI_RTN_CODE aui_mp_change_subtitle(void *pv_handle_mp, int trackIdx)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_change_subtitle(trackIdx);
        ret = AUI_RTN_SUCCESS;
    }

    return ret;
}

AUI_RTN_CODE aui_mp_get(void *pv_handle_mp, unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    AUI_ERR("The function is deprecated. Please use aui_mp_get_stream_info() and aui_mp_get_playrate()!!\n");
    if(pv_handle_mp && pv_param)
    {
        switch(ul_item)
        {
            case AUI_MP_GET_STREAM_INFO:
                break;
            case AUI_MP_GET_STREAM_INFO_SPEED:
                break;
        }
    }
    
    return ret;
}
AUI_RTN_CODE aui_mp_get_playrate(void *pv_handle_mp, enum aui_mp_speed *speed)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        if(TransRateFromGo2Aui(goplayer_get_cur_play_rate(), speed))
        {
            ret = AUI_RTN_SUCCESS;
        }
        else
            AUI_ERR("no matched play ragte!!\n");
    }

    return ret;
}

#define FOURCC_ARGS(fourcc) \
((char) ((fourcc)     &0xff)), \
((char) (((fourcc)>>8 )&0xff)), \
((char) (((fourcc)>>16)&0xff)), \
((char) (((fourcc)>>24)&0xff))
            
#define MAKE_FOURCC(a,b,c,d)        ((unsigned long)((a)|(b)<<8|(c)<<16|(d)<<24))

//H264
#define FOURCC_avc1     MAKE_FOURCC('a','v','c','1')
#define FOURCC_h264     MAKE_FOURCC('h','2','6','4')
//H263
#define FOURCC_h263     MAKE_FOURCC('h','2','6','3')
//MPEG
#define FOURCC_mpg1     MAKE_FOURCC('m','p','g','1')
#define FOURCC_mpg2     MAKE_FOURCC('m','p','g','2')
#define FOURCC_mpeg     MAKE_FOURCC('m','p','e','g')
#define FOURCC_mp4v  MAKE_FOURCC ('m', 'p', '4', 'v')
#define FOURCC_3ivx MAKE_FOURCC ('3', 'i', 'v', 'x')
//xvid
#define FOURCC_xvid  MAKE_FOURCC ('x','v','i','d')
#define FOURCC_vp8 MAKE_FOURCC ('v', 'p', '8', ' ')
//wvc1
#define FOURCC_wvc1 MAKE_FOURCC ('w','v','c','1')

static aui_decv_format fourCC2VidFormat(unsigned long fourCC)
{
    aui_decv_format vidCodecFmt = AUI_DECV_FORMAT_INVALID;
    switch(fourCC)
    {
        case FOURCC_avc1:
        case FOURCC_h264:
        {
            vidCodecFmt = AUI_DECV_FORMAT_AVC;
            break;
        }
        case FOURCC_mpg1:
        case FOURCC_mpg2:
        case FOURCC_mpeg:
        case FOURCC_mp4v:
        case FOURCC_3ivx:
        {
            vidCodecFmt = AUI_DECV_FORMAT_MPEG;
            break;
        }
        case FOURCC_xvid:
        {
            vidCodecFmt = AUI_DECV_FORMAT_XVID;
            break;
        }
        //According from Vis, h263 classify to FLV, I think it's weird but just follow it.
        case FOURCC_h263: 
        {
            vidCodecFmt = AUI_DECV_FORMAT_FLV1;
            break;
        }
        case FOURCC_vp8:
        {
            vidCodecFmt = AUI_DECV_FORMAT_VP8;
            break;
        }
        case FOURCC_wvc1:
        {
            vidCodecFmt = AUI_DECV_FORMAT_WVC1;
        }
        default:
        {
            AUI_ERR("unknow fourCC:%c%c%c%c\n", FOURCC_ARGS(fourCC));
            break;
        }    
    }

    return vidCodecFmt;
}
//transform stream info from goplayer style to AUI MP style.
//return 0: failed. Otherwise: success.
static int transStreamInfo(aui_mp_stream_info_type type, goplayer_stream_info *goStreamInfo, aui_mp_stream_info *auiMpStreamInfo)
{
    int ret = FALSE;
    if(goStreamInfo && auiMpStreamInfo)
    {
        unsigned int i;
        auiMpStreamInfo->count = 0; //initialize
        switch(type)
        {
            case AUI_MP_STREAM_INFO_TYPE_AUDIO:
            {
                char *decName;
                aui_mp_audio_track_info* audio_track_info;
                stGOPLAYER_AUDIO_TRACK_INFO * go_audio_track_info = goStreamInfo->stream_info.audio_track_info;
                auiMpStreamInfo->count = goStreamInfo->count;
                auiMpStreamInfo->stream_info.audio_track_info = (aui_mp_audio_track_info*)malloc(sizeof(aui_mp_audio_track_info) * auiMpStreamInfo->count);
                if(auiMpStreamInfo->stream_info.audio_track_info == NULL)
                {
                    AUI_ERR("allocation of audio_track_info failed!\n");
                    goto Neuschwanstein;
                }                
                audio_track_info = auiMpStreamInfo->stream_info.audio_track_info;
                for(i = 0; i < auiMpStreamInfo->count; i++)
                {
                    audio_track_info[i].track_index = go_audio_track_info[i].track_index;
                    strncpy(audio_track_info[i].lang_code, go_audio_track_info[i].lang_code, 5);
                    audio_track_info[i].audDetailInfo = NULL;
                    if(go_audio_track_info[i].audDetailInfo)
                    {
                        audio_track_info[i].audDetailInfo  = (aui_mp_audio_detail_info*)malloc(sizeof(aui_mp_audio_detail_info));
                        if(audio_track_info[i].audDetailInfo)
                        {
#if 0
                            AUI_DBG("audio_track_info[i].audDetailInfo:%p, go_audio_track_info[i].audDetailInfo:%p\n", 
                                audio_track_info[i].audDetailInfo, go_audio_track_info[i].audDetailInfo);//Nick debug
#endif
                            audio_track_info[i].audDetailInfo->channels = go_audio_track_info[i].audDetailInfo->channels;
                            audio_track_info[i].audDetailInfo->depth = go_audio_track_info[i].audDetailInfo->depth;
                            audio_track_info[i].audDetailInfo->samplerate = go_audio_track_info[i].audDetailInfo->samplerate;
                            decName = go_audio_track_info[i].audDetailInfo->decName;
                            if(strcmp(decName, "mp3") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_MP3;
                            }
                            else if(strcmp(decName, "aac") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_AAC_LATM; 
                            }
                            else if(strcmp(decName, "amr") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_AMR; 
                            }
                            else if(strcmp(decName, "ac3") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_AC3; 
                            }
                            //According to Vis, wma is mapping to bye1
                            else if(strcmp(decName, "wma") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_BYE1; 
                            }
                            else if(strcmp(decName, "pcm") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_PCM; 
                            }
                            else if(strcmp(decName, "adpcm") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_ADPCM; 
                            }
                            else if(strcmp(decName, "ra") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_RA8; 
                            }
                            else if(strcmp(decName, "ogg") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_OGG; 
                            }
                            else if(strcmp(decName, "flac") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_FLAC; 
                            }
                            else if(strcmp(decName, "ape") == 0)
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_APE; 
                            }
                            else
                            {
                                audio_track_info[i].audDetailInfo->audioCodecType = AUI_DECA_STREAM_TYPE_INVALID;
                                AUI_ERR("unknow codec:%s\n", decName);
                            }                              
                        }
                        else
                        {
                            AUI_ERR("allocation of audDetailInfo failed!\n");
                            _free_audio_track_info(audio_track_info, i);
                            goto Neuschwanstein;
                        }
                    }
                }
                ret = TRUE;
                break;
            }
            case AUI_MP_STREAM_INFO_TYPE_SUBTITLE:
            {
                aui_mp_subtitle_info *subtitle_info;
                auiMpStreamInfo->count = goStreamInfo->count;
                auiMpStreamInfo->stream_info.subtitle_info = (aui_mp_subtitle_info*)malloc(sizeof(aui_mp_subtitle_info) * auiMpStreamInfo->count);
                if(auiMpStreamInfo->stream_info.subtitle_info)
                {
                    subtitle_info = auiMpStreamInfo->stream_info.subtitle_info;
                    for(i = 0; i < auiMpStreamInfo->count; i++)
                    {
                        subtitle_info[i].track_index = goStreamInfo->stream_info.subtitle_info[i].track_index;
                        strncpy(subtitle_info[i].lang_code, goStreamInfo->stream_info.subtitle_info[i].lang_code, 5);
                    }
                    ret = TRUE;
                }
                else
                {
                    AUI_ERR("allocation of subtitle_info failed!\n");
                    goto Neuschwanstein;
                }
                break;
            }
            case AUI_MP_STREAM_INFO_TYPE_VIDEO:
            {
                aui_mp_video_track_info *video_track_info;
                stGOPLAYER_VIDEO_TRACK_INFO *go_video_track_info = goStreamInfo->stream_info.video_track_info;
                auiMpStreamInfo->count = goStreamInfo->count;
                auiMpStreamInfo->stream_info.video_track_info = (aui_mp_video_track_info*)malloc(sizeof(aui_mp_video_track_info) * auiMpStreamInfo->count);
                if(auiMpStreamInfo->stream_info.video_track_info)
                {
                    video_track_info = auiMpStreamInfo->stream_info.video_track_info;
                    for(i = 0; i < auiMpStreamInfo->count; i++)
                    {
                        video_track_info[i].vidCodecFmt = fourCC2VidFormat(go_video_track_info[i].fourCC);
                        video_track_info[i].framerate = go_video_track_info[i].framerate;
                        video_track_info[i].height = go_video_track_info[i].height;
                        video_track_info[i].width = go_video_track_info[i].width;
                    }
                    ret = TRUE;
                }
                else
                {
                    AUI_ERR("allocation of video_track_info failed!\n");
                    goto Neuschwanstein;
                }
                break;
            }
            default:
            {
                break;
            }
            
        }
    }
    
Neuschwanstein:
    return ret;
    
}
AUI_RTN_CODE aui_mp_get_stream_info(void *pv_handle_mp, aui_mp_stream_info_type type, aui_mp_stream_info **info)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_stream_info *goStreamInfo;
        int transRet;
        //aui_mp_stream_info *tmp_info = (aui_mp_stream_info *)malloc(sizeof(aui_mp_stream_info));
        tmp_info = (aui_mp_stream_info *)malloc(sizeof(aui_mp_stream_info));
        if(tmp_info)
        {
            tmp_info->type = type;
            switch(type)
            {
                case AUI_MP_STREAM_INFO_TYPE_AUDIO:
                {
                    if(goplayer_get_stream_info(GOPLAYER_STREAM_INFO_TYPE_AUDIO, &goStreamInfo))
                    {
                        if((transRet = transStreamInfo(AUI_MP_STREAM_INFO_TYPE_AUDIO, goStreamInfo, tmp_info)))
                        {
                            *info = tmp_info;
                        }
                        
                        if(!(goplayer_free_stream_info(goStreamInfo) && transRet))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }
                    break;
                }
                case AUI_MP_STREAM_INFO_TYPE_SUBTITLE:
                {
                    if(goplayer_get_stream_info(GOPLAYER_STREAM_INFO_TYPE_SUBTITLE, &goStreamInfo))
                    {
                        if((transRet = transStreamInfo(AUI_MP_STREAM_INFO_TYPE_SUBTITLE, goStreamInfo, tmp_info)))
                        {
                            *info = tmp_info;
                        }
                    
                        if(!(goplayer_free_stream_info(goStreamInfo) && transRet))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }
                    break;
                }
                    
                case AUI_MP_STREAM_INFO_TYPE_VIDEO:
                {
                    if(goplayer_get_cur_stream_info(GOPLAYER_STREAM_INFO_TYPE_VIDEO, &goStreamInfo))
                    {
                        if((transRet = transStreamInfo(AUI_MP_STREAM_INFO_TYPE_VIDEO, goStreamInfo, tmp_info)))
                        {
                            *info = tmp_info;
                        }
                    
                        if(!(goplayer_free_stream_info(goStreamInfo) && transRet))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }

                    break;
                }
                case AUI_MP_STREAM_INFO_TYPE_MEDIA_SIZE:
                {
                    if(goplayer_get_stream_info(GOPLAYER_STREAM_INFO_TYPE_MEDIA_SIZE, &goStreamInfo))
                    {
                        tmp_info->stream_info.mediaSize = goStreamInfo->stream_info.mediaSize;
                        *info = tmp_info;
                        if(!goplayer_free_stream_info(goStreamInfo))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }
                    break;
                }
                default:
                    AUI_ERR("no matched aui_mp_stream_info_type, type:%d!!\n", type);
                    free(tmp_info);
                    break;
            }        
        }
        else
        {
            AUI_ERR("allocation of temp aui_mp_stream_info failed!\n");
        }
    }

Hawaii:
    
    return ret;
GOPLAYER_FREE_STREAM_IFNO_ERR:
    //shuld not be here. 
    AUI_ERR("goplayer_free_stream_info() failed\n");
    aui_mp_free_stream_info(pv_handle_mp, *info);
    goto Hawaii;
}

AUI_RTN_CODE aui_mp_get_cur_stream_info(void *pv_handle_mp, aui_mp_stream_info_type type, aui_mp_stream_info **info)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_stream_info *goStreamInfo;
        int transRet;
        //aui_mp_stream_info *tmp_info = (aui_mp_stream_info *)malloc(sizeof(aui_mp_stream_info));
        tmp_info = (aui_mp_stream_info *)malloc(sizeof(aui_mp_stream_info));
        if(tmp_info)
        {
            tmp_info->type = type;        
            switch(type)
            {
                case AUI_MP_STREAM_INFO_TYPE_AUDIO:
                {
                    if(goplayer_get_cur_stream_info(GOPLAYER_STREAM_INFO_TYPE_AUDIO, &goStreamInfo))
                    {
                        if((transRet = transStreamInfo(AUI_MP_STREAM_INFO_TYPE_AUDIO, goStreamInfo, tmp_info)))
                        {
                            *info = tmp_info;
                        }
                        
                        if(!(goplayer_free_stream_info(goStreamInfo) && transRet))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }
                    break;
                }
                case AUI_MP_STREAM_INFO_TYPE_SUBTITLE:
                {
                    if(goplayer_get_cur_stream_info(GOPLAYER_STREAM_INFO_TYPE_SUBTITLE, &goStreamInfo))
                    {
                        if((transRet = transStreamInfo(AUI_MP_STREAM_INFO_TYPE_SUBTITLE, goStreamInfo, tmp_info)))
                        {
                            *info = tmp_info;
                        }
                    
                        if(!(goplayer_free_stream_info(goStreamInfo) && transRet))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }
                    break;
                }
                case AUI_MP_STREAM_INFO_TYPE_VIDEO:
                {
                    if(goplayer_get_cur_stream_info(GOPLAYER_STREAM_INFO_TYPE_VIDEO, &goStreamInfo))
                    {
                        if((transRet = transStreamInfo(AUI_MP_STREAM_INFO_TYPE_VIDEO, goStreamInfo, tmp_info)))
                        {
                            *info = tmp_info;
                        }
                    
                        if(!(goplayer_free_stream_info(goStreamInfo) && transRet))
                        {
                            goto GOPLAYER_FREE_STREAM_IFNO_ERR;
                        }
                        ret = AUI_RTN_SUCCESS;
                    }
                    break;
                }
                default:
                    AUI_ERR("no matched aui_mp_stream_info_type, type:%d!!\n", type);
                    free(tmp_info);
                    break;
            }        
        }
        else
        {
            AUI_ERR("allocation of temp aui_mp_stream_info failed!\n");
        }

    }
Barcelona:
    return ret;
    
GOPLAYER_FREE_STREAM_IFNO_ERR:
    //shuld not be here. 
    AUI_ERR("goplayer_free_stream_info() failed\n");
    aui_mp_free_stream_info(pv_handle_mp, *info);
    goto Barcelona;
}

AUI_RTN_CODE aui_mp_free_stream_info(void *pv_handle_mp, aui_mp_stream_info *info)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp && info)
    {
        switch(info->type)
        {
            case AUI_MP_STREAM_INFO_TYPE_AUDIO:
            {
#if 1
                _free_audio_track_info(info->stream_info.audio_track_info, info->count);
#else
                if(info->stream_info.audio_track_info)
                {
                    unsigned int i;
                    for(i = 0; i < info->count; i++)
                    {
                        if(info->stream_info.audio_track_info[i].audDetailInfo)
                        {
                            free(info->stream_info.audio_track_info[i].audDetailInfo);
                        }
                    }
                    free(info->stream_info.audio_track_info);
                }
#endif
                break;
            }
            case AUI_MP_STREAM_INFO_TYPE_SUBTITLE:
            {
                if(info->stream_info.subtitle_info)
                {
                    free(info->stream_info.subtitle_info);
                }
                break;
            }
            case AUI_MP_STREAM_INFO_TYPE_VIDEO:
            {
                if(info->stream_info.video_track_info)
                {
                    free(info->stream_info.video_track_info);
                }
                break;
            }
            case AUI_MP_STREAM_INFO_TYPE_MEDIA_SIZE:
                //do nothing here
                break; 
            default:
                ret = AUI_RTN_EINVAL;
                AUI_ERR("unknow stream info type:%d!!\n", info->type);
                free(info);
                goto Auckland;
        }
        free(info);
        ret = AUI_RTN_SUCCESS;
    }
    else
    {
        ret = AUI_RTN_EINVAL;
        AUI_ERR("invalid parameter, pv_handle_mp:%p, info:%p!!\n", pv_handle_mp, info);
        if(info)
        {
            free(info);
            AUI_ERR("But info still be freed!!\n");
        }
    }
Auckland:
    return ret;
}

AUI_RTN_CODE aui_mp_set_buffering_time(void *pv_handle_mp,unsigned int time, unsigned int low_percent, unsigned int high_percent)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_set_buffering_time(time, low_percent, high_percent);
        ret = AUI_RTN_SUCCESS;
    }
    return ret;
}

AUI_RTN_CODE aui_mp_set_start2play_percent(void *pv_handle_mp, int percent)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_set_start2play_percent(percent);
        ret = AUI_RTN_SUCCESS;
    }
    return ret;
}

AUI_RTN_CODE aui_mp_get_snapshot(void *pv_handle_mp)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        goplayer_set_frame_capture_mode();
        ret = aui_mp_start(pv_handle_mp);
    }
    
    return ret;
}
AUI_RTN_CODE aui_mp_is_seekable(void *pv_handle_mp, int *is_seekable)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        *is_seekable = goplayer_is_seekable();
        ret = AUI_RTN_SUCCESS;
    }
    
    return ret;
}
AUI_RTN_CODE aui_mp_get_download_speed(void *pv_handle_mp, unsigned long long *dlSpeed)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
        if(goplayer_get_download_speed(dlSpeed))
        ret = AUI_RTN_SUCCESS;
    }
    
    return ret;
}
#if 0 //There is a aui sound module for volume setting.
AUI_RTN_CODE aui_mp_set_volume(void *pv_handle_mp, int volume)
{
#define VOL_MIN 0
#define VOL_MAX 100

    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    
    if(pv_handle_mp)
    {

        aui_hdl audio_hdl = NULL;
        int sndOpenByMe = FALSE;
        
        if (volume < VOL_MIN)
            volume = VOL_MIN;
        else if (volume > VOL_MAX)
            volume = VOL_MAX;

        if (0 != aui_find_dev_by_idx(AUI_MODULE_SND, 0, &audio_hdl))
        {
            aui_attr_snd attr_snd;
            memset(&attr_snd, 0, sizeof(attr_snd));
            if (0 != aui_snd_open(&attr_snd, &audio_hdl))
                goto Florida;
            sndOpenByMe = TRUE;
        }

        ret = aui_snd_vol_set(audio_hdl, (unsigned char)volume);
        if(sndOpenByMe)
            aui_snd_close(audio_hdl);
        ret = AUI_RTN_SUCCESS;

    }

Florida:
    return ret;
}

AUI_RTN_CODE aui_mp_get_volume(void *pv_handle_mp, unsigned int *volume)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    if(pv_handle_mp)
    {
    }

    return ret;
}
#endif
