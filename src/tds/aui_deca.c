
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_deca.h>
#include <aui_decv.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd.h>
#include <mediatypes.h>
#include <hld/sbm/sbm.h>

/****************************LOCAL MACRO******************************************/
AUI_MODULE(DECA)

#define AUI_DECA_PCM_FRAME_LEN_MAX  (1536)
#define AUI_DECA_INJECT_DATA_PER_LEN    (184)
#define AUI_DECA_RUN_STATUS_STOP    (0)
#define AUI_DECA_RUN_STATUS_PLAY    (1)
#define AUI_DECA_RUN_STATUS_PAUSE    (2)

/****************************LOCAL TYPE*******************************************/
typedef struct aui_st_handle_deca
{

    aui_dev_priv_data dev_priv_data;

    OSAL_ID dev_mutex_id;

    struct deca_device *pst_dev_deca;

    aui_attr_deca attr_deca;
    aui_deca_pcm_param pcm_param;
    aui_deca_pcm_cap pcm_cap;
    unsigned long ul_run_status;
    aui_deca_cb_func cbfuncs[AUI_DECA_CB_MAX];
    void* pv_user_data[AUI_DECA_CB_MAX];
    unsigned char ase_init;
}aui_handle_deca,*aui_p_handle_deca;

/****************************LOCAL VAR********************************************/
static OSAL_ID s_mod_mutex_id_deca=0;

/****************************LOCAL FUNC DECLARE***********************************/
static unsigned long _deca_cb_type_swith(unsigned long ul_aui_cb_type)
{
    unsigned long drv_cb_type=-1;
    
    switch(ul_aui_cb_type)
    {
       case AUI_DECA_CB_MONITOR_NEW_FRAME:
            drv_cb_type = DECA_CB_MONITOR_NEW_FRAME;
            break;
       case AUI_DECA_CB_MONITOR_START:
            drv_cb_type = DECA_CB_MONITOR_START;
            break;
       case AUI_DECA_CB_MONITOR_STOP:
            drv_cb_type = DECA_CB_MONITOR_STOP;
            break;
       case AUI_DECA_CB_MONITOR_DECODE_ERR:
            drv_cb_type = DECA_CB_MONITOR_DECODE_ERR;
            break;
       case AUI_DECA_CB_MONITOR_OTHER_ERR:
            drv_cb_type = DECA_CB_MONITOR_OTHER_ERR;
            break;
       case AUI_DECA_CB_EWS:
            drv_cb_type = DECA_CB_Announcement_Switching_Data_Field;
            break;
       case AUI_DECA_CB_STATE_CHANGED:
            drv_cb_type = DECA_CB_STATE_CHANGED;
            break;
            
        default:
            drv_cb_type=-1;
            break;
    }
    return drv_cb_type;
    
}

static unsigned long _deca_aud_type_switch_from_aui_2_drv(unsigned long ul_aui_type)
{
    unsigned long drv_deca_type=AUDIO_INVALID;
    switch(ul_aui_type)
    {
       case AUI_DECA_STREAM_TYPE_MPEG1:
            drv_deca_type = AUDIO_MPEG1;
            break;
       case AUI_DECA_STREAM_TYPE_MPEG2:
            drv_deca_type = AUDIO_MPEG2;
            break;
       case AUI_DECA_STREAM_TYPE_AAC_LATM:
            drv_deca_type = AUDIO_MPEG_AAC;
            break;
       case AUI_DECA_STREAM_TYPE_AC3:
            drv_deca_type = AUDIO_AC3;
            break;
       case AUI_DECA_STREAM_TYPE_DTS:
            drv_deca_type = AUDIO_DTS;
            break;
       case AUI_DECA_STREAM_TYPE_PPCM:
            drv_deca_type = AUDIO_PPCM;
            break;
       case AUI_DECA_STREAM_TYPE_LPCM_V:
            drv_deca_type = AUDIO_LPCM_V;
            break;
       case AUI_DECA_STREAM_TYPE_LPCM_A:
            drv_deca_type = AUDIO_LPCM_A;
            break;
       case AUI_DECA_STREAM_TYPE_PCM:
            drv_deca_type = AUDIO_PCM;
            break;
       case AUI_DECA_STREAM_TYPE_BYE1:
            drv_deca_type = AUDIO_BYE1;
            break;
       case AUI_DECA_STREAM_TYPE_RA8:
            drv_deca_type = AUDIO_RA8;
            break;
       case AUI_DECA_STREAM_TYPE_INVALID:
            drv_deca_type = AUDIO_INVALID;
            break;
       case AUI_DECA_STREAM_TYPE_AAC_ADTS:
            drv_deca_type = AUDIO_MPEG_ADTS_AAC;
            break;
       case AUI_DECA_STREAM_TYPE_OGG:
            drv_deca_type = AUDIO_OGG;
            break;
       case AUI_DECA_STREAM_TYPE_EC3:
            drv_deca_type = AUDIO_EC3;
            break;
        case AUI_DECA_STREAM_TYPE_RAW_PCM:
            drv_deca_type = AUDIO_PCM_RAW;
            break;
        case AUI_DECA_STREAM_TYPE_MP3:
        case AUI_DECA_STREAM_TYPE_MP3_L3:
        case AUI_DECA_STREAM_TYPE_MP3_2:
            drv_deca_type = AUDIO_MP3_2;
            break;
        default:
            drv_deca_type=AUDIO_STREAM_TYPE_END;
            break;
    }
    return drv_deca_type;
}

static unsigned long _deca_aud_type_switch_from_drv_2_aui(unsigned long ul_drv_type)
{
    unsigned long aui_deca_type=AUI_DECA_STREAM_TYPE_LAST;
    switch(ul_drv_type)
    {
       case AUDIO_MPEG1:
            aui_deca_type = AUI_DECA_STREAM_TYPE_MPEG1;
            break;
       case AUDIO_MPEG2:
            aui_deca_type = AUI_DECA_STREAM_TYPE_MPEG2;
            break;
       case AUDIO_MPEG_AAC:
            aui_deca_type = AUI_DECA_STREAM_TYPE_AAC_LATM;
            break;
       case AUDIO_AC3:
            aui_deca_type = AUI_DECA_STREAM_TYPE_AC3;
            break;
       case AUDIO_DTS:
            aui_deca_type = AUI_DECA_STREAM_TYPE_DTS;
            break;
       case AUDIO_PPCM:
            aui_deca_type = AUI_DECA_STREAM_TYPE_PPCM;
            break;
       case AUDIO_LPCM_V:
            aui_deca_type = AUI_DECA_STREAM_TYPE_LPCM_V;
            break;
       case AUDIO_LPCM_A:
            aui_deca_type = AUI_DECA_STREAM_TYPE_LPCM_A;
            break;
       case AUDIO_PCM:
            aui_deca_type = AUI_DECA_STREAM_TYPE_PCM;
            break;
       case AUDIO_BYE1:
            aui_deca_type = AUI_DECA_STREAM_TYPE_BYE1;
            break;
       case AUDIO_RA8:
            aui_deca_type = AUI_DECA_STREAM_TYPE_RA8;
            break;
       case AUDIO_MP3:
            aui_deca_type = AUI_DECA_STREAM_TYPE_MP3;
            break;
       case AUDIO_INVALID:
            aui_deca_type = AUI_DECA_STREAM_TYPE_INVALID;
            break;
       case AUDIO_MPEG_ADTS_AAC:
            aui_deca_type = AUI_DECA_STREAM_TYPE_AAC_ADTS;
            break;
       case AUDIO_OGG:
            aui_deca_type = AUI_DECA_STREAM_TYPE_OGG;
            break;
       case AUDIO_EC3:
            aui_deca_type = AUI_DECA_STREAM_TYPE_EC3;
            break;
        case AUDIO_MP3_L3:
            aui_deca_type = AUI_DECA_STREAM_TYPE_MP3_L3;
            break;
        case AUDIO_MP3_2:
            aui_deca_type = AUI_DECA_STREAM_TYPE_MP3_2;
            break;
        case AUDIO_PCM_RAW:
            aui_deca_type = AUI_DECA_STREAM_TYPE_RAW_PCM;
            break;
        default:
            aui_deca_type=AUI_DECA_STREAM_TYPE_LAST;
            break;
    }
    return aui_deca_type;
}

static void  _deca_cb_entrance(UINT32 u_param1, UINT32 u_param2)
{
    aui_handle_deca *pst_aui_hdl_deca=NULL;
    struct deca_io_reg_callback_para * pst_deca_para = (struct deca_io_reg_callback_para *)u_param1;
    struct aui_st_deca_io_reg_callback_para st_aui_deca_para;
    (void)u_param2;
    
    MEMSET(&st_aui_deca_para, 0, sizeof(st_aui_deca_para));
    if (AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_DECA, 0, (aui_hdl *)&pst_aui_hdl_deca)) {
        if(pst_aui_hdl_deca->cbfuncs[pst_deca_para->e_cbtype]!=NULL)
        {
            st_aui_deca_para.en_cb_type = pst_deca_para->e_cbtype;
            st_aui_deca_para.status = pst_deca_para->state_flag;
            st_aui_deca_para.pv_param = pst_aui_hdl_deca->pv_user_data[pst_deca_para->e_cbtype];
            st_aui_deca_para.p_cb = pst_aui_hdl_deca->cbfuncs[pst_deca_para->e_cbtype];
            
            ((aui_deca_cb_func)(pst_aui_hdl_deca->cbfuncs[pst_deca_para->e_cbtype]))(&st_aui_deca_para, NULL, st_aui_deca_para.pv_param);
            
        }
    }
    else
    {
        AUI_ERR("ERROR in %s \n",__FUNCTION__);
    }
    
}

AUI_RTN_CODE aui_send_one_mepg_pes_frame(unsigned long p_dev_deca,unsigned char *psrc,unsigned long ul_len_in,unsigned long *p_ul_len_real)
{
    unsigned long i=0;
    unsigned char *optv_data = psrc;
    unsigned long ul_real_write_per_len=0;
    unsigned long ul_real_write_total_len=0;
    
    for(i=0;i<ul_len_in/AUI_DECA_INJECT_DATA_PER_LEN;i++)
    {
        osal_task_sleep(2);
        if(0!=deca_copy_data(p_dev_deca,(UINT32)optv_data,AUI_DECA_INJECT_DATA_PER_LEN,&ul_real_write_per_len))
        {
            ul_real_write_per_len=AUI_DECA_INJECT_DATA_PER_LEN;
        }
        optv_data += ul_real_write_per_len;
        ul_real_write_total_len=ul_real_write_total_len+ul_real_write_per_len;
    }

    if(ul_len_in%AUI_DECA_INJECT_DATA_PER_LEN)
    {
        if(0!=deca_copy_data(p_dev_deca,(UINT32)optv_data,ul_len_in%AUI_DECA_INJECT_DATA_PER_LEN,&ul_real_write_per_len))
        {
            ul_real_write_per_len=ul_len_in%AUI_DECA_INJECT_DATA_PER_LEN;
        }
        ul_real_write_total_len=ul_real_write_total_len+ul_real_write_per_len;

    }
   *p_ul_len_real=ul_real_write_total_len;    
   return AUI_RTN_SUCCESS;
}
unsigned long s_ul_frm_len=0;
AUI_RTN_CODE aui_get_es_frame(unsigned char *psrc,unsigned long ul_len_in,unsigned char **ppdst,unsigned long *p_ul_len_dst)
{
    unsigned long i=0;
    int j=0;
    unsigned long ul_pos_tmp1=0;
    unsigned long ul_frame_len=0;
    unsigned long ul_frame_len2=0;
    
    for(i=0;i<ul_len_in;i++)
    {
        //AUI_DBG("i=[%d][%d]\n", i, ul_len_in);
        if((psrc[i]==0)&&(psrc[i+1]==0)&&(psrc[i+2]==1)&&(psrc[i+3]==0xc0))
        {
            ul_frame_len=((psrc[i+4]<<8)&0xff00)+(psrc[i+5]&0x00ff);
            s_ul_frm_len=ul_frame_len;
            AUI_DBG("es fram len=[%08x][%08x][%08x]\n", ul_frame_len, psrc[i+4], psrc[i+5]);
            //if(s_ul_frm_len!=0x1e08)
            //{
                //ul_frame_len=0x1e08;
                //AUI_DBG("fix it es fram len=[%08x][%08x][%08x]\n", ul_frame_len, psrc[i+4], psrc[i+5]);
            //}
            ul_pos_tmp1=i;
            if(ul_frame_len+6>ul_len_in)
            {
                continue;
            }
            if(ul_len_in>ul_frame_len+6)
            {
                if((psrc[ul_pos_tmp1+ul_frame_len+6]==0)&&(psrc[ul_pos_tmp1+ul_frame_len+7]==0)&&(psrc[ul_pos_tmp1+ul_frame_len+8]==1)&&(psrc[ul_pos_tmp1+ul_frame_len+9]==0xc0))
                {
                    ul_frame_len2=((psrc[ul_pos_tmp1+ul_frame_len+10]<<8)&0xff00)+(psrc[ul_pos_tmp1+ul_frame_len+11]&0x00ff);
                    AUI_DBG("es fram len2=[%08x]\n", ul_frame_len2);
                    if(ul_frame_len2!=ul_frame_len)
                    {
                        AUI_DBG("!!!!!!!!!!!!!!!! es fram len2=[%08x]\n", ul_frame_len2);
                    }
                    //ul_frame_len=ul_pos_tmp1;
                    *ppdst=psrc+ul_pos_tmp1+6;
                    *p_ul_len_dst=ul_frame_len;
                    AUI_DBG("##################pre frame head 16 byte:\n");
                    for(j=0;j<15;j++)
                    {
                        AUI_DBG("[%02x]",psrc[ul_pos_tmp1+j]);
                    }
                    AUI_DBG("\n");
                    return AUI_RTN_SUCCESS;
                }
                else
                {
                    *p_ul_len_dst=0;
                    return AUI_RTN_FAIL;
                }
            }
            else
            {
                *ppdst=psrc+ul_pos_tmp1+6;
                *p_ul_len_dst=ul_frame_len;
                return AUI_RTN_SUCCESS;
            }
                
            
        }
    }  
    *p_ul_len_dst=0;
    return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_pes2es(unsigned long p_dev_deca,unsigned char *psrc,unsigned long ul_len_in,unsigned long *p_ul_len_real)
{
    unsigned long i=0;
    unsigned long ul_frame_len=0;
    unsigned char *psrc_tmp=NULL;
    unsigned long ul_real_len_per=0;
    
    *p_ul_len_real=0;
    for(i=0;i<ul_len_in;)
    {
        //AUI_DBG("\r\n i1=[%d][%d]",i,ul_len_in);
        if(AUI_RTN_SUCCESS!=aui_get_es_frame(psrc+*p_ul_len_real,ul_len_in-*p_ul_len_real,&psrc_tmp,&ul_frame_len))
        {
            AUI_ERR("!!!!!!!!!!!!!!! LESS ONE FRAME LEN.\n");
            return AUI_RTN_SUCCESS;
        }
        aui_send_one_mepg_pes_frame(p_dev_deca,psrc_tmp,ul_frame_len,&ul_real_len_per);
        i=i+ul_frame_len+6;
        *p_ul_len_real=*p_ul_len_real+ul_frame_len+6;
        if(*p_ul_len_real>=ul_len_in)
        {
            return AUI_RTN_SUCCESS;
        }
    }
    return AUI_RTN_SUCCESS;
}

/****************************MODULE DRV IMPLEMENT*************************************/



/****************************MODULE IMPLEMENT*************************************/

AUI_RTN_CODE aui_deca_version_get(unsigned long* const pul_version)
{
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_version)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    *pul_version=AUI_MODULE_VERSION_NUM_DECA;
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_init(p_fun_cb p_call_back_init,void *pv_param)
{
    AUI_RTN_CODE aui_rtn_code=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    s_mod_mutex_id_deca=osal_mutex_create();
    if(0==s_mod_mutex_id_deca)
    {
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    if (p_call_back_init != NULL) {
        aui_rtn_code=p_call_back_init(pv_param);
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return aui_rtn_code;
}


AUI_RTN_CODE aui_deca_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    AUI_RTN_CODE aui_rtn_code=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(E_OK!=osal_mutex_delete(s_mod_mutex_id_deca))
    {
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    if (p_call_back_init != NULL) {
        aui_rtn_code=p_call_back_init(pv_param);
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return aui_rtn_code;
}

extern void snd_m36g_init_tone_voice(struct snd_device *dev);
extern void deca_m36_init_tone_voice(struct deca_device *dev);
AUI_RTN_CODE aui_deca_open(const aui_attr_deca *p_attr_deca,aui_hdl* const pp_hdl_deca)
{
    OSAL_ID dev_mutex_id;
    aui_handle_deca *pst_hdl_deca=NULL;
    /** device index*/
    unsigned char uc_dev_idx=0;
    /** stream audio type*/
    enum aui_deca_stream_type en_stream_aud_type=AUI_DECA_STREAM_TYPE_MPEG2;
    /** stream audio sample rate*/
    enum audio_sample_rate en_sample_rate=AUDIO_SAMPLE_RATE_48;
    /** audio quantization*/
    enum audio_quantization en_quan=AUDIO_QWLEN_24;
    /** stream audio channel number*/
    unsigned char uc_channel_num=2;
    /** reserved */
    unsigned long ul_info_struct=0;
    if (!pp_hdl_deca)
        aui_rtn(AUI_RTN_EINVAL,NULL);
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_deca,dev_mutex_id,AUI_MODULE_DECA,AUI_RTN_FAIL);

    pst_hdl_deca=(aui_handle_deca *)MALLOC(sizeof(aui_handle_deca));
    if(NULL==pst_hdl_deca)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM,NULL);
    }
    MEMSET(pst_hdl_deca,0,sizeof(aui_handle_deca));
    pst_hdl_deca->dev_mutex_id=INVALID_ID;
    if(NULL==p_attr_deca)
    {
        pst_hdl_deca->dev_priv_data.dev_idx = uc_dev_idx;
        pst_hdl_deca->attr_deca.uc_dev_idx=uc_dev_idx;
        pst_hdl_deca->attr_deca.en_stream_aud_type=en_stream_aud_type;
        pst_hdl_deca->attr_deca.en_sample_rate=en_sample_rate;
        pst_hdl_deca->attr_deca.en_quan=en_quan;
        pst_hdl_deca->attr_deca.uc_channel_num=uc_channel_num;
        pst_hdl_deca->attr_deca.ul_info_struct=ul_info_struct;

    }
    else
    {
        pst_hdl_deca->dev_priv_data.dev_idx = p_attr_deca->uc_dev_idx;
        MEMCPY(&(pst_hdl_deca->attr_deca),p_attr_deca,sizeof(aui_attr_deca));
    }
    // set the default channel num
    if (!pst_hdl_deca->attr_deca.uc_channel_num){
        pst_hdl_deca->attr_deca.uc_channel_num = 8;
    }

    pst_hdl_deca->pst_dev_deca=(struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA,
                                                    pst_hdl_deca->attr_deca.uc_dev_idx);
    if(NULL==pst_hdl_deca->pst_dev_deca)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }

    if(RET_SUCCESS!=deca_open(pst_hdl_deca->pst_dev_deca,
                                _deca_aud_type_switch_from_aui_2_drv(pst_hdl_deca->attr_deca.en_stream_aud_type),
                                pst_hdl_deca->attr_deca.en_sample_rate,
                                pst_hdl_deca->attr_deca.en_quan,
                                pst_hdl_deca->attr_deca.uc_channel_num,
                                pst_hdl_deca->attr_deca.ul_info_struct))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    pst_hdl_deca->dev_mutex_id=dev_mutex_id;
    aui_dev_reg(AUI_MODULE_DECA,pst_hdl_deca);
    *pp_hdl_deca=pst_hdl_deca;

    //test bee tone
    //struct deca_device *temp_deca_handle = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    //deca_init_ase(temp_deca_handle);

    deca_m36_init_tone_voice((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
    snd_m36g_init_tone_voice((struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0));

    UINT32 mute_setting = 0;//union is ms; the beep tone is continuous when mute_setting is 0;
    deca_io_control((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0), DECA_BEEP_TONE_MUTE_INTERVAL, mute_setting);
    UINT32 beep_tone_setting = 360;//union is ms, if here is 0, the default beep tone last 360ms
    deca_io_control((struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0), DECA_BEEP_TONE_INTERVAL, beep_tone_setting);
    osal_mutex_unlock(dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_close(aui_hdl p_hdl_deca)
{
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);

    if(RET_SUCCESS!=deca_close(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca))
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }

    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);

    /*if(NULL==((aui_handle_deca *)p_hdl_deca))
    {
        aui_rtn(AUI_RTN_FAIL,"\r\n_handle is NULL.");
    }*/
    if(0!=osal_mutex_delete(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        aui_rtn(AUI_RTN_FAIL,"\r\n_delete mutex err.");
    }
    aui_dev_unreg(AUI_MODULE_DECA,p_hdl_deca);
    MEMSET(p_hdl_deca,0,sizeof(aui_handle_deca));
    FREE(p_hdl_deca);
    p_hdl_deca=NULL;

    osal_mutex_unlock(s_mod_mutex_id_deca);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_start(aui_hdl p_hdl_deca, __attribute__ ((unused)) const aui_attr_deca *p_attr_deca)
{
    aui_handle_deca *tmp = NULL;
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    //AUI_DBG("!!!!!!!!!!!!!!!!!!func:%s, line:%d,\n", __func__, __LINE__);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==(void *)p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca)
        ||(INVALID_ID==((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    tmp = (aui_handle_deca *)p_hdl_deca;
    osal_mutex_lock(tmp->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);
    //important warning: 1.remove deca driver state judge,for main can get differnt value from see,although the same virtual address. 2.aui layer must not use driver structure in see side for see cache may be different from main cache.more detail refer to redmine http://prj.alitech.com/qt/issues/44340 #5
    /*if(DECA_STATE_PLAY == tmp->pst_dev_deca->flags)
    {
        tmp->ul_run_status=AUI_DECA_RUN_STATUS_PLAY;
        osal_mutex_unlock(tmp->dev_mutex_id);
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return AUI_RTN_SUCCESS; 
    }*/
    if(RET_SUCCESS!=deca_start(tmp->pst_dev_deca,tmp->attr_deca.ul_pts))
    {
        osal_mutex_unlock(tmp->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    tmp->ul_run_status=AUI_DECA_RUN_STATUS_PLAY;
    osal_mutex_unlock(tmp->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_stop(aui_hdl p_hdl_deca, __attribute__ ((unused)) const aui_attr_deca *p_attr_deca)
{
    aui_handle_deca *tmp = NULL;
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    //AUI_DBG("!!!!!!!!!!!!!!!!!!func:%s, line:%d,\n", __func__, __LINE__);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==(void *)p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca)
        ||(INVALID_ID==((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    tmp = (aui_handle_deca *)p_hdl_deca;
    osal_mutex_lock(tmp->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);
    //important warning: 1.remove deca driver state judge,for main can get differnt value from see,although the same virtual address. 2.aui layer must not use driver structure in see side for see cache may be different from main cache.more detail refer to redmine http://prj.alitech.com/qt/issues/44340 #5
    /*if(DECA_STATE_IDLE == tmp->pst_dev_deca->flags)
    {
        tmp->ul_run_status=AUI_DECA_RUN_STATUS_STOP;
        osal_mutex_unlock(tmp->dev_mutex_id);
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return AUI_RTN_SUCCESS; 
    }*/
    if(RET_SUCCESS!=deca_stop(tmp->pst_dev_deca,
                                tmp->attr_deca.ul_pts,
                                tmp->attr_deca.ul_deca_stop_mode))
    {
        osal_mutex_unlock(tmp->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    tmp->ul_run_status=AUI_DECA_RUN_STATUS_STOP;
    osal_mutex_unlock(tmp->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_pause(aui_hdl p_hdl_deca, __attribute__ ((unused)) const aui_attr_deca *p_attr_deca)
{
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==(void *)p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca)
        ||(INVALID_ID==((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);

    /*if((NULL==p_hdl_deca)||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca))
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        aui_rtn(AUI_MODULE_SYS,AUI_RTN_EINVAL,NULL);
    }*/
    if(RET_SUCCESS!=deca_pause(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca))
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,NULL);
    }
    ((aui_handle_deca *)p_hdl_deca)->ul_run_status=AUI_DECA_RUN_STATUS_PAUSE;
    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_resume(aui_hdl p_hdl_deca, const aui_attr_deca *p_attr_deca)
{
    /*
    unsigned long ul_rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca)
        ||(INVALID_ID==((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_MODULE_SYS,AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);

    ul_rtn=aui_deca_start(p_hdl_deca,p_attr_deca);
    if(AUI_RTN_SUCCESS!=ul_rtn)
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        return ul_rtn;
    }
    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    */
    unsigned long ul_rtn=AUI_RTN_FAIL;
    ul_rtn=aui_deca_start(p_hdl_deca,p_attr_deca);
    return ul_rtn;
}

AUI_RTN_CODE aui_deca_set(aui_hdl p_hdl_deca,unsigned long ul_item,void *pv_param)
{
    AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
    enum audio_stream_type deca_type=AUDIO_INVALID;
    aui_handle_deca *pst_aui_hdl_deca=NULL;
    unsigned long ul_real_write_per_len=0;
    unsigned long ul_real_write_total_len=0;
    
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca)
        ||(INVALID_ID==((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);


    if(ul_item>=AUI_DECA_SET_CMD_LAST)
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    switch(ul_item)
    {
        case AUI_DECA_DEOCDER_TYPE_SET:
        {
            if (NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            pst_aui_hdl_deca=p_hdl_deca;

            unsigned long ul_deca_status=DECA_STATE_ATTACH;

            struct deca_device *pst_dev_deca=((aui_handle_deca *)p_hdl_deca)->pst_dev_deca;

            if (RET_SUCCESS!=deca_io_control(pst_dev_deca,DECA_GET_DECA_STATE,(UINT32)&ul_deca_status))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            //Don`t permit to set audio format while decoding
            if ((DECA_STATE_PLAY == ul_deca_status) ||(DECA_STATE_PAUSE == ul_deca_status)) {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,"wrong status!\n");
            }
            deca_type=_deca_aud_type_switch_from_aui_2_drv(*(unsigned long *)pv_param);
            if ((AUDIO_STREAM_TYPE_END==deca_type)||(AUDIO_INVALID==deca_type))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,"Deca invalid type!\n");
            }

            if (RET_SUCCESS!=deca_io_control(pst_dev_deca, DECA_SET_STR_TYPE, deca_type))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            pst_aui_hdl_deca->attr_deca.en_stream_aud_type=(*(unsigned long *)pv_param);
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_AUD_SYNC_LEVEL_SET:
        {
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            //???? should add driver DECA_SET_SYNC_MODE
            if(RET_SUCCESS!=deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_SET_AV_SYNC_LEVEL, (unsigned long)(*(unsigned long *)pv_param)))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            ((aui_handle_deca *)p_hdl_deca)->attr_deca.ul_sync_level=(*(unsigned long *)pv_param);
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_AUD_SYNC_MODE_SET:
        {
            enum aui_stc_avsync_mode sync_mode = AUI_STC_AVSYNC_PCR;
            enum adec_sync_mode hld_sync_mode=ADEC_SYNC_PTS;
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            sync_mode=*(enum aui_stc_avsync_mode *)pv_param;
            if(AUI_DECA_DEOCDER_SYNC==(aui_deca_sync_mode)sync_mode)
            {
                hld_sync_mode=ADEC_SYNC_PTS;
            }
            else if(AUI_DECA_DEOCDER_ASYNC==(aui_deca_sync_mode)sync_mode)
            {
                hld_sync_mode=ADEC_SYNC_FREERUN;
            }
            else
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            if (RET_SUCCESS!=deca_set_sync_mode(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca,hld_sync_mode))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            ((aui_handle_deca *)p_hdl_deca)->attr_deca.ul_sync_mode=(*(unsigned long *)pv_param);
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_AUD_PID_SET:
        {
            ((aui_handle_deca *)p_hdl_deca)->attr_deca.us_aud_pid=*(unsigned short *)pv_param;
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_EMPTY_BS_SET:
        {
            if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_EMPTY_BS_SET, 0x00))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_ADD_BS_SET:
        {
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            enum audio_stream_type drv_audio_stream_type;
            drv_audio_stream_type=_deca_aud_type_switch_from_aui_2_drv(*(unsigned long *)pv_param);
            if((AUDIO_STREAM_TYPE_END==drv_audio_stream_type)||(AUDIO_INVALID==drv_audio_stream_type))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            
            if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_ADD_BS_SET, drv_audio_stream_type))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }

            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_INJECT_ES_DATA:
        {
            unsigned long ul_len_in_tmp=0;
            unsigned char *optv_data = NULL;
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            
            aui_deca_inject_data *pdata=pv_param;
            pst_aui_hdl_deca=p_hdl_deca;
            void  *device_hdl=pst_aui_hdl_deca->pst_dev_deca;

            if(AUI_DECA_STREAM_TYPE_RAW_PCM==pst_aui_hdl_deca->attr_deca.en_stream_aud_type)
            {
                ul_len_in_tmp=0;
                optv_data = pdata->puc_data_in;
                //AUI_DBG("aui : in len:[%d]\n",pdata->data_in_len);
                if((0==pst_aui_hdl_deca->pcm_param.sampling_rate)||(0==pst_aui_hdl_deca->pcm_param.channels))
                {
                    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                    aui_rtn(AUI_RTN_EINVAL,NULL);
                }
                while((pst_aui_hdl_deca->pst_dev_deca)&&(AUI_DECA_RUN_STATUS_PLAY==pst_aui_hdl_deca->ul_run_status))
                {
                    //osal_task_sleep(5);
                    //AUI_DBG("aa[%d][%d]\n", pdata->data_in_len, ul_real_write_total_len);
                    if(pdata->data_in_len-ul_real_write_total_len>=AUI_DECA_PCM_FRAME_LEN_MAX)
                    {
                        ul_len_in_tmp=AUI_DECA_PCM_FRAME_LEN_MAX;
                    }
                    else if((pdata->data_in_len-ul_real_write_total_len>0)
                        &&(pdata->data_in_len-ul_real_write_total_len<AUI_DECA_PCM_FRAME_LEN_MAX))
                    {
                        ul_len_in_tmp=pdata->data_in_len-ul_real_write_total_len;
                    }
                    else
                    {
                        //AUI_DBG("aui : tot len last2:[%d][%d]\n", ul_real_write_total_len, ul_real_write_per_len);
                        break;
                    }

                    deca_process_pcm_samples(ul_len_in_tmp, //union is byte, not more than 1536bytes.
                                            optv_data,//pcm data address
                                            pst_aui_hdl_deca->pcm_param.sampling_rate ,//sample rate
                                            pst_aui_hdl_deca->pcm_param.channels,//channel number(1,2)
                                            pst_aui_hdl_deca->pcm_param.sample_size//bit per sample
                                            );
                    ul_real_write_per_len=ul_len_in_tmp;
                    //AUI_DBG("ul_len_in_tmp=[%d]\n", ul_len_in_tmp);

                    optv_data += ul_real_write_per_len;

                    ul_real_write_total_len=ul_real_write_total_len+ul_real_write_per_len;
                    if(ul_real_write_total_len>=pdata->data_in_len)
                    {
                        //AUI_DBG("aui : tot len last:[%d][%d]\n", ul_real_write_total_len, ul_real_write_per_len);
                        break;
                    }
                    //AUI_DBG("aui : tot len:[%d][%d]\n", ul_real_write_total_len, ul_real_write_per_len);
                }
                pdata->data_in_len_real=ul_real_write_total_len;

            }
            else
            {
                ul_len_in_tmp=0;
                optv_data = pdata->puc_data_in;
                //AUI_DBG("aui : in len:[%d]\n", pdata->data_in_len);

                while((pst_aui_hdl_deca->pst_dev_deca)&&(AUI_DECA_RUN_STATUS_PLAY==pst_aui_hdl_deca->ul_run_status))
                {
                    if(AUI_DECA_STREAM_TYPE_AC3==pst_aui_hdl_deca->attr_deca.en_stream_aud_type)
                    {
                        //osal_task_sleep(15);
                    }
                    else
                    {
                        //osal_task_sleep(5);
                    }
                    //AUI_DBG("aa[%d][%d]\n",pdata->data_in_len,ul_real_write_total_len);
                    if(pdata->data_in_len-ul_real_write_total_len>=AUI_DECA_INJECT_DATA_PER_LEN)
                    {
                        ul_len_in_tmp=AUI_DECA_INJECT_DATA_PER_LEN;
                    }
                    else if((pdata->data_in_len-ul_real_write_total_len>0)
                        &&(pdata->data_in_len-ul_real_write_total_len<AUI_DECA_INJECT_DATA_PER_LEN))
                    {
                        ul_len_in_tmp=pdata->data_in_len-ul_real_write_total_len;
                    }
                    else
                    {
                        //AUI_DBG("aui : tot len last2:[%d][%d]\n",ul_real_write_total_len,ul_real_write_per_len);
                        break;
                    }
                    //AUI_DBG("ul_len_in_tmp=[%d]\n",ul_len_in_tmp);
                    if(0!=deca_copy_data((UINT32)device_hdl,(UINT32)optv_data,ul_len_in_tmp,&ul_real_write_per_len))
                    {
                        //AUI_DBG("deca_copy_data failed1!\n");
                        //ul_real_write_per_len=ul_len_in_tmp;
                        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                        aui_rtn(AUI_RTN_EINVAL,NULL);
                    }
                    optv_data += ul_real_write_per_len;

                    ul_real_write_total_len=ul_real_write_total_len+ul_real_write_per_len;
                    if(ul_real_write_total_len>=pdata->data_in_len)
                    {
                        //AUI_DBG("\r\n aui : tot len last:[%d][%d]",ul_real_write_total_len,ul_real_write_per_len);
                        break;
                    }
                    //AUI_DBG("\r\n aui : tot len:[%d][%d]",ul_real_write_total_len,ul_real_write_per_len);
                }
                pdata->data_in_len_real=ul_real_write_total_len;

            }

            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_PCM_PARAM_SET:
        {
            pst_aui_hdl_deca=p_hdl_deca;
            aui_deca_pcm_param *p_pcm_param=pv_param;
            if(NULL==p_pcm_param)
            {
                osal_mutex_unlock(pst_aui_hdl_deca->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            #if 1
            osal_cache_flush(p_pcm_param, sizeof(aui_deca_pcm_param));
            if(RET_SUCCESS!=deca_io_control(pst_aui_hdl_deca->pst_dev_deca, DECA_SET_PCM_DECODER_PARAMS, (unsigned long)((unsigned long *)p_pcm_param)))
            {
                osal_mutex_unlock(pst_aui_hdl_deca->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            #endif
            MEMCPY(&(pst_aui_hdl_deca->pcm_param),p_pcm_param,sizeof(aui_deca_pcm_param));
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_REG_CB:
        {
            struct aui_st_deca_io_reg_callback_para *p_cb_param=pv_param;
            struct deca_io_reg_callback_para hld_param;
            unsigned long ul_cb_type=-1;
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            MEMSET(&hld_param,0,sizeof(hld_param));
            ul_cb_type=_deca_cb_type_swith(p_cb_param->en_cb_type);
            if((unsigned long)-1==ul_cb_type)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }    
            hld_param.e_cbtype=ul_cb_type;
            hld_param.monitor_rate=p_cb_param->monitor_rate;
            hld_param.p_cb= _deca_cb_entrance;
            hld_param.pv_param=p_cb_param->pv_param;
            hld_param.state_change_timeout_threshold=p_cb_param->timeout;
            hld_param.state_flag=p_cb_param->status;
            ((aui_handle_deca *)p_hdl_deca)->cbfuncs[p_cb_param->en_cb_type]=p_cb_param->p_cb;
            ((aui_handle_deca *)p_hdl_deca)->pv_user_data[p_cb_param->en_cb_type]=p_cb_param->pv_param;
            if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_IO_REG_CALLBACK, (UINT32)&hld_param))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_PREPARE_CHANGE_AUD_TRACK:
            rtn_code=AUI_RTN_SUCCESS;
        break;
        default:
        {
            osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL,NULL);
            break;
        }
    }
    if(AUI_RTN_SUCCESS!=rtn_code)
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_get(aui_hdl p_hdl_deca,unsigned long ul_item,void *pv_param)
{
    AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
    //unsigned long ul_rtn=RET_FAILURE;
    unsigned long ul_deca_cur_type=0xff;
    aui_handle_deca *pst_aui_hdl_deca=NULL;
    
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_deca)
        ||(NULL==((aui_handle_deca *)p_hdl_deca)->pst_dev_deca)
        ||(INVALID_ID==((aui_handle_deca *)p_hdl_deca)->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);

    if(ul_item>AUI_DECA_GET_CMD_LAST)
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    switch(ul_item)
    {
        case AUI_DECA_DATA_INFO_GET:
        {
            struct cur_stream_info st_stream_info;
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_GET_PLAY_PARAM, (unsigned long)&st_stream_info))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,NULL);
            }
            ul_deca_cur_type=_deca_aud_type_switch_from_drv_2_aui(st_stream_info.str_type);
            if((AUI_DECA_STREAM_TYPE_LAST==ul_deca_cur_type)||(AUI_DECA_STREAM_TYPE_INVALID==ul_deca_cur_type))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,"Deca invalid type!\n");
            }
            
            ((aui_deca_data_info *)pv_param)->uc_deca_type=ul_deca_cur_type;
            ((aui_deca_data_info *)pv_param)->uc_bit_width=st_stream_info.bit_depth;
            ((aui_deca_data_info *)pv_param)->ul_sample_rate=st_stream_info.sample_rate;
            ((aui_deca_data_info *)pv_param)->ul_sample_cnt=st_stream_info.samp_num;
            ((aui_deca_data_info *)pv_param)->ul_channel_cnt=st_stream_info.chan_num;
            ((aui_deca_data_info *)pv_param)->ul_frame_cnt=st_stream_info.frm_cnt;

            if((AUI_DECA_STREAM_TYPE_AC3 == ul_deca_cur_type) || (AUI_DECA_STREAM_TYPE_EC3 == ul_deca_cur_type)) {
                unsigned int acmod = 0xFFFFFFFF;
                if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_GET_DDP_INMOD, (unsigned int)&acmod)){
                    ((aui_deca_data_info *)pv_param)->audio_mode.ac_mode= 0xFFFFFFFF;
                    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                    aui_rtn(AUI_RTN_FAIL,NULL);
                }
                //AUI_DBG("%s -> acmod: %u\n", __FUNCTION__, acmod);
                ((aui_deca_data_info *)pv_param)->audio_mode.ac_mode= acmod;
            }
            else if((AUI_DECA_STREAM_TYPE_MPEG1 == ul_deca_cur_type) || (AUI_DECA_STREAM_TYPE_MPEG2 == ul_deca_cur_type)) {
                struct AUDIO_INFO audio_info;
                MEMSET(&audio_info, 0, sizeof(audio_info));
                if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_GET_AUDIO_INFO, (unsigned int)&audio_info)){
                    ((aui_deca_data_info *)pv_param)->audio_mode.channel_mode = 0xFFFFFFFF;
                    /* This ioctl calling would faill after deca stopped */
                    //osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                    //aui_rtn(AUI_RTN_FAIL,NULL);
                }
                //AUI_DBG("%s -> channel_mode : %u\n", __FUNCTION__, audio_info.mode);
                ((aui_deca_data_info *)pv_param)->audio_mode.channel_mode = audio_info.mode;
            }
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_DEOCDER_TYPE_GET:
        {
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            enum aui_deca_stream_type *p_en_deca_type=pv_param;
            pst_aui_hdl_deca=p_hdl_deca;
            *p_en_deca_type=pst_aui_hdl_deca->attr_deca.en_stream_aud_type;
            #if 0
            if(RET_SUCCESS != deca_io_control(((aui_handle_deca *)p_hdl_deca)->pst_dev_deca, DECA_GET_STR_TYPE, (unsigned long)pv_param))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_MODULE_DECA,DECA_ERR,NULL);
            }
            #endif
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_DEOCDER_STATUS_GET:
        {
            unsigned long ul_deca_status=DECA_STATE_ATTACH;
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            struct deca_device *pst_dev_deca=((aui_handle_deca *)p_hdl_deca)->pst_dev_deca;

            if(RET_SUCCESS!=deca_io_control(pst_dev_deca,DECA_GET_DECA_STATE,(UINT32)&ul_deca_status))
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            
            if(DECA_STATE_DETACH==ul_deca_status)
            {
                *((unsigned long *)pv_param)=AUI_DECA_DEATTACH;
            }
            else if(DECA_STATE_ATTACH==ul_deca_status)
            {
                *((unsigned long *)pv_param)=AUI_DECA_ATTACH;
            }
            else if(DECA_STATE_IDLE==ul_deca_status)
            {
                *((unsigned long *)pv_param)=AUI_DECA_STOP;
            }
            else if(DECA_STATE_PLAY==ul_deca_status)
            {
                *((unsigned long *)pv_param)=AUI_DECA_RUN;
            }
            else if(DECA_STATE_PAUSE==ul_deca_status)
            {
                *((unsigned long *)pv_param)=AUI_DECA_PAUSE;
            }
            else
            {
                osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_AUD_SYNC_MODE_GET:
        {
            *(unsigned short *)pv_param=((aui_handle_deca *)p_hdl_deca)->attr_deca.ul_sync_mode;
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_AUD_SYNC_LEVEL_GET:
        {
            *(unsigned short *)pv_param=((aui_handle_deca *)p_hdl_deca)->attr_deca.ul_sync_level;
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_AUD_PID_GET:
        {
            *(unsigned short *)pv_param=((aui_handle_deca *)p_hdl_deca)->attr_deca.us_aud_pid;
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_BUF_DATA_PTR:
        {
            break;
        }
        case AUI_DECA_PCM_PARAM_GET:
        {
            aui_handle_deca *pst_aui_hdl_deca=p_hdl_deca;
            aui_deca_pcm_param *p_pcm_param=pv_param;
            MEMCPY(p_pcm_param,&(pst_aui_hdl_deca->pcm_param),sizeof(aui_deca_pcm_param));
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_DECA_PCM_CAP_GET:
        {
            aui_deca_pcm_cap *p_pcm_cap=pv_param;
            p_pcm_cap->channels=AUI_PCM_MONO_MASK|AUI_PCM_STEREO_RIGHT_FIRST_MASK|AUI_PCM_STEREO_LEFT_FIRST_MASK|AUI_PCM_MULTI_CHANNEL_MASK;
            p_pcm_cap->endians=AUI_PCM_BIG_ENDIAN_MASK|AUI_PCM_LITTLE_ENDIAN_MASK;
            p_pcm_cap->sample_sizes=AUI_PCM_8BIT_SAMPLE_MASK|AUI_PCM_16BIT_SAMPLE_MASK|AUI_PCM_24BIT_SAMPLE_MASK|AUI_PCM_32BIT_SAMPLE_MASK;
            p_pcm_cap->sampling_rates=AUI_PCM_8KHZ_SAMPLE_MASK|AUI_PCM_16KHZ_SAMPLE_MASK|AUI_PCM_22KHZ_SAMPLE_MASK|AUI_PCM_24KHZ_SAMPLE_MASK \
                                    |AUI_PCM_32KHZ_SAMPLE_MASK|AUI_PCM_44KHZ_SAMPLE_MASK|AUI_PCM_48KHZ_SAMPLE_MASK| \
                                    AUI_PCM_64KHZ_SAMPLE_MASK|AUI_PCM_88KHZ_SAMPLE_MASK|AUI_PCM_96KHZ_SAMPLE_MASK;
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        default:
        {
            osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL,NULL);
            break;
        }
    }
    if(AUI_RTN_SUCCESS!=rtn_code)
    {
        osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
        return rtn_code;
    }
    osal_mutex_unlock(((aui_handle_deca *)p_hdl_deca)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_data_info_get(aui_hdl p_hdl_deca,aui_deca_data_info *pul_data_info)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_data_info)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    rtn=aui_deca_get(p_hdl_deca,AUI_DECA_DATA_INFO_GET, pul_data_info);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_dev_cnt_get(unsigned long* const pul_deca_cnt)
{
    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_deca_cnt)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    *pul_deca_cnt=1;//???? need add driver
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_type_get(aui_hdl p_hdl_deca,unsigned long* const pul_deca_cur_type)
{
    //enum audio_stream_type decode_type=AUDIO_INVALID;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_deca_cur_type)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    rtn=aui_deca_get(p_hdl_deca,AUI_DECA_DEOCDER_TYPE_GET, pul_deca_cur_type);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }

    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_type_set(aui_hdl p_hdl_deca,unsigned long ul_deca_cur_type)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;


    AUI_ENTER_FUNC(AUI_MODULE_DECA);

    rtn=aui_deca_set(p_hdl_deca,AUI_DECA_DEOCDER_TYPE_SET, &ul_deca_cur_type);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    //((aui_handle_deca *)p_hdl_deca)->attr_deca.en_stream_aud_type=ul_deca_cur_type;

    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_deca_sync_mode_get(aui_hdl p_hdl_deca,unsigned long* const pul_deca_sync_mode)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_deca_sync_mode)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    rtn=aui_deca_get(p_hdl_deca,AUI_DECA_AUD_SYNC_MODE_GET, pul_deca_sync_mode);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_sync_mode_set(aui_hdl p_hdl_deca,unsigned long ul_deca_sync_mode)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    rtn=aui_deca_set(p_hdl_deca,AUI_DECA_AUD_SYNC_MODE_SET, &ul_deca_sync_mode);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_sync_level_get(aui_hdl p_hdl_deca,unsigned long *pul_deca_sync_level)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_deca_sync_level)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    rtn=aui_deca_get(p_hdl_deca,AUI_DECA_AUD_SYNC_MODE_GET, pul_deca_sync_level);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_sync_level_set(aui_hdl p_hdl_deca,unsigned long ul_deca_sync_level)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    rtn=aui_deca_set(p_hdl_deca,AUI_DECA_AUD_SYNC_LEVEL_SET, &ul_deca_sync_level);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_aud_pid_get(aui_hdl p_hdl_deca,unsigned short *pus_aud_pid)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pus_aud_pid)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    rtn=aui_deca_get(p_hdl_deca,AUI_DECA_AUD_PID_GET, pus_aud_pid);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_aud_pid_set(aui_hdl p_hdl_deca,unsigned long us_aud_pid)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    rtn=aui_deca_set(p_hdl_deca,AUI_DECA_AUD_PID_SET, &us_aud_pid);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_deca_status_get(aui_hdl p_hdl_deca,unsigned long* const pul_deca_status)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if(NULL==pul_deca_status)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    rtn=aui_deca_get(p_hdl_deca,AUI_DECA_DEOCDER_STATUS_GET, pul_deca_status);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_inject_write_data(aui_hdl p_hdl_deca,const unsigned char *puc_data,unsigned long ul_data_len,struct aui_avsync_ctrl st_ctl_blk,unsigned long* const pul_real_wt_len)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_deca_inject_data cmd_deca_snd_data;

    AUI_ENTER_FUNC(AUI_MODULE_DECA);
    if((NULL==p_hdl_deca)||(NULL==puc_data)||(NULL==pul_real_wt_len))
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    MEMSET(&cmd_deca_snd_data,0,sizeof(aui_cmd_deca_snd_data));
    cmd_deca_snd_data.puc_data_in=(unsigned char *)puc_data;
    cmd_deca_snd_data.data_in_len=ul_data_len;
    MEMCPY((cmd_deca_snd_data.p_ctrl_block),&st_ctl_blk,sizeof(struct control_block));
    rtn=aui_deca_set(p_hdl_deca,AUI_DECA_INJECT_ES_DATA, &cmd_deca_snd_data);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        AUI_LEAVE_FUNC(AUI_MODULE_DECA);
        return rtn;
    }
    *pul_real_wt_len=cmd_deca_snd_data.data_in_len_real;
    AUI_LEAVE_FUNC(AUI_MODULE_DECA);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_deca_bee_tone_start(aui_hdl p_hdl_deca, unsigned char level, int* data, unsigned int data_len)
{
    int deca_state = -1;
    struct deca_device *temp_deca_handle = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);

    (void)level;
    if((NULL==p_hdl_deca)||(NULL==data))
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    deca_io_control(temp_deca_handle, DECA_GET_DECA_STATE, (UINT32)&deca_state);
    if(DECA_STATE_PLAY == deca_state)
    {
        deca_stop(temp_deca_handle, 0, ADEC_STOP_IMM);
        //osal_task_sleep(100);
    }
    //osal_task_sleep(100);
    deca_tone_voice(temp_deca_handle, (UINT32)data, data_len);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_bee_tone_stop(aui_hdl p_hdl_deca)
{
    if(NULL==p_hdl_deca)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    struct deca_device *temp_deca_handle = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    deca_stop_tone_voice(temp_deca_handle);
    //deca_start(temp_deca_handle, 0);    
    deca_stop(temp_deca_handle, 0, ADEC_STOP_END);
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_deca_init_ase(aui_hdl p_hdl_deca)
{
    aui_handle_deca *tmp = p_hdl_deca;
    
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==tmp)
        ||(NULL==tmp->pst_dev_deca)
        ||(INVALID_ID==tmp->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(tmp->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);
    if(0 == tmp->ase_init)
    {
        deca_init_ase(tmp->pst_dev_deca);
        tmp->ase_init = 1;
    }
    osal_mutex_unlock(tmp->dev_mutex_id);
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_deca_str_play(aui_hdl p_hdl_deca, unsigned int param)
{
    aui_handle_deca *tmp = p_hdl_deca;
    unsigned long ret = AUI_RTN_SUCCESS;

    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==tmp)||(NULL==tmp->pst_dev_deca)||(INVALID_ID==tmp->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(tmp->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);
    
    if(deca_io_control(tmp->pst_dev_deca, DECA_STR_PLAY, param))
    {
        ret = !AUI_RTN_SUCCESS;
    }
    osal_mutex_unlock(tmp->dev_mutex_id);
    return ret;
}
AUI_RTN_CODE aui_deca_str_stop(aui_hdl p_hdl_deca)
{
    aui_handle_deca *tmp = p_hdl_deca;
    unsigned long ret = AUI_RTN_SUCCESS;
    
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==tmp)||(NULL==tmp->pst_dev_deca)||(INVALID_ID==tmp->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(tmp->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);
    if(deca_io_control(tmp->pst_dev_deca, DECA_STR_STOP, 0))
    {
        ret = !AUI_RTN_SUCCESS;
    }
    osal_mutex_unlock(tmp->dev_mutex_id);
    return ret;
}
AUI_RTN_CODE aui_deca_set_beep_interval(aui_hdl p_hdl_deca, unsigned int param)
{
    aui_handle_deca *tmp = p_hdl_deca;
    unsigned long ret = AUI_RTN_SUCCESS;
    
    osal_mutex_lock(s_mod_mutex_id_deca,OSAL_WAIT_FOREVER_TIME);
    if((NULL==tmp)||(NULL==tmp->pst_dev_deca)||(INVALID_ID==tmp->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_deca);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    osal_mutex_lock(tmp->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_deca);
    
    if(deca_io_control(tmp->pst_dev_deca, DECA_BEEP_INTERVAL, param))
    {
     ret = !AUI_RTN_SUCCESS;
    }
    osal_mutex_unlock(tmp->dev_mutex_id);
    return ret;
}

AUI_RTN_CODE aui_deca_audio_desc_enable (aui_hdl p_hdl_deca, unsigned char enable)
{
    //not support right now
    (void)p_hdl_deca;
    (void)enable;
    return AUI_RTN_SUCCESS;
}

