#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#else
#include <api/libfs2/stdio.h>
#endif

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>

#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_nim.h"

#define NIM_TIMEOUT_MS 1000

#ifdef LNB_CFG_KU_BAND
/* Astra channel "Arte"
 * clear file : stream_arte.ts
 * csa1 file : stream_arte_csa1.ts
 * aes file : stream_arte_aes.ts
 */
#define FREQ 10743
#define SYMB 22000
unsigned short pids[] = { 401 /* video */, 402 /* audio */, 401 /* pcr */ };
#elif LNB_CFG_C_BAND
/* cctv1 */
#define FREQ 3840
#define SYMB 27500
unsigned short pids[] = { 512 /* video */, 650 /* audio */, 8190 /* pcr */ };
#else
#define FREQ 0
#define SYMB 0
#endif
extern unsigned long s_play_mode;
int nim_connect(struct ali_app_nim_init_para *para,aui_hdl *aui_nim_hdl)
{
    aui_hdl hdl = 0;
    struct aui_nim_attr nim_attr;
    struct aui_nim_connect_param param;
    int as_high_band = 0;
        
    /*init nim_attr and param variable*/
    MEMSET(&param, 0, sizeof(struct aui_nim_connect_param));
    MEMSET(&nim_attr, 0, sizeof(struct aui_nim_attr));

    nim_attr.ul_nim_id = para->ul_device;
    nim_attr.en_dmod_type = para->ul_nim_type;
    nim_attr.en_ter_std = para->ul_nim_std; //added by vedic.fu
    AUI_PRINTF("\nul_nim_std = %ld\n",para->ul_nim_std);
#if 0
    /*open nim device*/
    if (aui_nim_open(&nim_attr, &hdl)) {
        AUI_PRINTF("open error\n");
        return -1;
    }
    AUI_PRINTF("nim device %d opened\n", (int)nim_attr.ul_nim_id);
#endif

#ifdef AUI_LINUX
        if (aui_nim_open(&nim_attr, &hdl)) {
            AUI_PRINTF("open error\n");
            return -1;
        }
#else
        if (aui_nim_handle_get_byid(para->ul_device, &hdl))
        {
            AUI_PRINTF("aui_nim_handle_get_byid error\n");
            return -1;
        }
#endif
    AUI_PRINTF("nim device %d opened\n", (int)nim_attr.ul_nim_id);

    switch (para->ul_nim_type) {/*signal modulation types:DVB-S,DVB-C,DVB-T*/
    case AUI_NIM_QPSK:/*DVB-S*/
#ifdef LNB_CFG_KU_BAND
        if (para->ul_freq> 11700) {
            as_high_band = 1;
            param.ul_freq = para->ul_freq - 10600;
        } else {
            as_high_band = 0;
            param.ul_freq = para->ul_freq - 9750;
        }
#endif
#ifdef LNB_CFG_C_BAND
        param.ul_freq = 5150 - para->ul_freq;
        as_high_band = 0;
#endif
        param.en_demod_type = AUI_NIM_QPSK;
        param.connect_param.sat.ul_freq_band = as_high_band;
        param.connect_param.sat.ul_symrate = para->ul_symb_rate;
        param.connect_param.sat.fec = AUI_NIM_FEC_AUTO;
        param.connect_param.sat.ul_polar = para->ul_polar;
        param.connect_param.sat.ul_src = para->ul_src;
        break;

    case AUI_NIM_QAM:/*DVB-C*/
        param.ul_freq = para->ul_freq;
        param.en_demod_type = AUI_NIM_QAM;
        param.connect_param.cab.ul_symrate = para->ul_symb_rate;
        param.connect_param.cab.en_qam_mode = AUI_NIM_QAM256;
        break;

    case AUI_NIM_OFDM:/*DVB-T*/
        param.ul_freq = para->ul_freq;
        param.en_demod_type = AUI_NIM_OFDM;
        switch(para->ul_freq_band)
        {
        case 6: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; break; //added by vedic.fu
        case 7: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_7_MHZ; break;
        case 8: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_8_MHZ; break;
        default : AUI_PRINTF("wrong bandwidth %ld\n", para->ul_freq_band); return -1;
        }
        param.connect_param.ter.std = para->ul_nim_std;
        param.connect_param.ter.fec = 0;
        param.connect_param.ter.spectrum = 0;
        break;

    default:
        AUI_PRINTF("not supported NIM type %d\n", (int)para->ul_nim_type);
        aui_nim_close(hdl);
        return -1;
    }

    if (aui_nim_connect(hdl, &param)) {
        AUI_PRINTF("connect error\n");
        aui_nim_close(hdl);
        return -1;
    }

    int lock_status = AUI_NIM_STATUS_UNKNOWN;
    int timeout = NIM_TIMEOUT_MS / 10;
    while (lock_status != AUI_NIM_STATUS_LOCKED && timeout) {
        if (aui_nim_is_lock(hdl, &lock_status)) {
            AUI_PRINTF("is_lock error\n");
            aui_nim_disconnect(hdl);
            aui_nim_close(hdl);
            return -1;
        }
        AUI_SLEEP(10);
        timeout--;
    }

    if (lock_status != AUI_NIM_STATUS_LOCKED) {
        AUI_PRINTF("channel is not locked\n");
        aui_nim_disconnect(hdl);
        aui_nim_close(hdl);
        return 0;
    }

    AUI_PRINTF("channel is locked on freq %ld in %d ms\n", para->ul_freq, NIM_TIMEOUT_MS - timeout*10);

    aui_signal_status info;
    if (aui_nim_signal_info_get(hdl, &info)) {/*getting the infomation of current tp*/
        AUI_PRINTF("info_get error\n");
        aui_nim_disconnect(hdl);
        aui_nim_close(hdl);
        return -1;
    }

    if (para->ul_nim_type == AUI_NIM_QPSK) {/*DVB-S*/
#ifdef LNB_CFG_KU_BAND
        if (as_high_band)
            info.ul_freq += 10600;
        else
            info.ul_freq += 9750;
#endif
#ifdef LNB_CFG_C_BAND
        info.ul_freq = 5150 - para->ul_freq;
#endif
    }
    AUI_PRINTF("info: freq %d locked in %d ms , strength %d, quality %d, ber %d, rf_level %d, signal_cn %d\n",
           (int)info.ul_freq, NIM_TIMEOUT_MS - timeout*10, (int)info.ul_signal_strength,
           (int)info.ul_signal_quality, (int)info.ul_bit_error_rate, (int)info.ul_rf_level,
           (int)info.ul_signal_cn);

    *aui_nim_hdl = hdl;
    return 0;
}

int ali_app_tsi_init(struct ali_app_tsi_init_para *para,aui_hdl *p_hdl)
{
    aui_attr_tsi attr_tsi;
    MEMSET(&attr_tsi, 0, sizeof(aui_attr_tsi));
    attr_tsi.ul_init_param = para->ul_hw_init_val;

    if(aui_find_dev_by_idx(AUI_MODULE_TSI, 0, p_hdl))
    {
         if (aui_tsi_open(p_hdl)) {
            AUI_PRINTF("\r\n aui_tsi_open error \n");
            return -1;
        }
	}
    if (aui_tsi_src_init(*p_hdl, para->ul_input_src, &attr_tsi)) {
        AUI_PRINTF("\r\n aui_tsi_src_init error \n");
        return -1;
    }
    if (aui_tsi_route_cfg(*p_hdl, para->ul_input_src,
                 para->ul_tis_port_idx,
                 para->ul_dmx_idx)) {
        AUI_PRINTF("\r\n aui_tsi_route_cfg error \n");
        return -1;
    }
    return 0;
}


int set_dmx_for_create_av_stream(int dmx_id,
             unsigned short video_pid,
             unsigned short audio_pid,
             unsigned short audio_desc_pid,
             unsigned short pcr_pid,
             aui_hdl *dmx_hdl)
{
   	aui_hdl hdl = NULL;
	aui_attr_dmx attr_dmx;
	aui_dmx_stream_pid pid_list;

	MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
	MEMSET(&pid_list,0,sizeof(aui_dmx_stream_pid));

	attr_dmx.uc_dev_idx = dmx_id;
	if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl))
	{
		if (aui_dmx_open(&attr_dmx, &hdl)) {
			AUI_PRINTF("\r\n dmx open fail\n");
			return -1;
		}
	}
	if(NULL == hdl) return -1;
	if (aui_dmx_start(hdl, &attr_dmx)) {
        aui_dmx_stop(hdl,NULL);/*starting fail may not be close dmx*/
        if (aui_dmx_start(hdl, &attr_dmx)) {/*start again*/
		    AUI_PRINTF("\r\n aui_dmx_start fail\n");
#ifdef AUI_LINUX
    		aui_dmx_close(hdl);    		      
#endif 
            return AUI_RTN_FAIL; 
        }
	}

	pid_list.ul_pid_cnt=4;
	pid_list.stream_types[0]=AUI_DMX_STREAM_VIDEO;
	pid_list.stream_types[1]=AUI_DMX_STREAM_AUDIO;
	pid_list.stream_types[2]=AUI_DMX_STREAM_PCR;
	pid_list.stream_types[3]=AUI_DMX_STREAM_AUDIO_DESC;

	pid_list.aus_pids_val[0]=video_pid;
	pid_list.aus_pids_val[1]=audio_pid;
	pid_list.aus_pids_val[2]=pcr_pid;
	pid_list.aus_pids_val[3]=audio_desc_pid;
	AUI_PRINTF("\r\nhdl:video_pid,audio_pid,pcr_pid,audio_desc_pid=[%08x][%d][%d][%d][%d]",
		   (int)hdl,video_pid,audio_pid,pcr_pid,audio_desc_pid);

  	if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list)) {
		AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
		aui_dmx_close(hdl);
		return -1;
	}
    
	*dmx_hdl= hdl;
	return 0;
}

#if 0
int set_dmx_for_create_av_stream(int dmx_id,
             unsigned short video_pid,
             unsigned short audio_pid,
             unsigned short audio_desc_pid,
             unsigned short pcr_pid,
             aui_hdl *dmx_hdl)
{
    aui_hdl hdl = NULL;
    aui_attr_dmx attr_dmx;
    aui_dmx_stream_pid pid_list;

    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&pid_list,0,sizeof(aui_dmx_stream_pid));

    /*if search DMX device with uc_dev_idx index open or not*/
    attr_dmx.uc_dev_idx = dmx_id;
    if(aui_find_dev_by_idx(AUI_MODULE_DMX, attr_dmx.uc_dev_idx, &hdl))
    {
        if (aui_dmx_open(&attr_dmx, &hdl)) {
            AUI_PRINTF("\r\n dmx open fail\n");
            return AUI_RTN_FAIL;
        }
    }
    if (aui_dmx_start(hdl, &attr_dmx)) {
        AUI_PRINTF("\r\n aui_dmx_start fail\n");
#ifdef AUI_LINUX
        aui_dmx_close(hdl);
#endif
        return AUI_RTN_FAIL;

    }

    pid_list.ul_pid_cnt=4;
    pid_list.stream_types[0]=AUI_DMX_STREAM_VIDEO;
    pid_list.stream_types[1]=AUI_DMX_STREAM_AUDIO;
    pid_list.stream_types[2]=AUI_DMX_STREAM_PCR;
    pid_list.stream_types[3]=AUI_DMX_STREAM_AUDIO_DESC;

    pid_list.aus_pids_val[0]=video_pid;
    pid_list.aus_pids_val[1]=audio_pid;
    pid_list.aus_pids_val[2]=pcr_pid;
    pid_list.aus_pids_val[3]=audio_desc_pid;
    AUI_PRINTF("\r\nhdl:video_pid,audio_pid,pcr_pid,audio_desc_pid=[%08x][%d][%d][%d][%d]\n",
           (int)hdl,video_pid,audio_pid,pcr_pid,audio_desc_pid);
    if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list)) {
        AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
#ifdef AUI_LINUX
        aui_dmx_close(hdl);
#endif
        return AUI_RTN_FAIL;
    }

    *dmx_hdl= hdl;
    return 0;
}
#endif
int ali_app_dis_init(struct ali_app_dis_init_para init_para_dis,aui_hdl *p_hdl)
{
    aui_hdl dis_hdl=0;

    aui_attr_dis attr_dis;
    MEMSET(&attr_dis, 0, sizeof(aui_attr_dis));

    attr_dis.uc_dev_idx = init_para_dis.ul_dis_type;
    if(aui_find_dev_by_idx(AUI_MODULE_DIS, 0, &dis_hdl)) {/*if dis device has opened,return dis device handle*/
        if (aui_dis_open(&attr_dis, &dis_hdl)) {/*if dis hasn't opened,open dis device and return dis device handle*/
            AUI_PRINTF("\n aui_dis_open fail\n");
            return -1;
        }
    }

    if (aui_dis_video_enable(dis_hdl, 0)) {
        AUI_PRINTF("\n aui_dis_video_enable fail\n");
        return -1;
    }

    *p_hdl=dis_hdl;
    return 0;
}


int ali_app_decv_init(struct ali_app_decv_init_para init_para_decv,aui_hdl *p_hdl)
{
    aui_hdl decv_hdl=0;
    (void) init_para_decv;

    aui_attr_decv attr_decv;
    enum aui_en_dec_format decv_type=init_para_decv.ul_video_type;
    MEMSET(&attr_decv,0,sizeof(aui_attr_decv));

    if (init_para_decv.callback != NULL)
    {
        attr_decv.callback_nodes[0].type= AUI_DECV_CB_FIRST_SHOWED;
        attr_decv.callback_nodes[0].callback = init_para_decv.callback;
        attr_decv.callback_nodes[0].puser_data = init_para_decv.puser_data;
        AUI_PRINTF("DEBUG trace %d %p\n", __LINE__,init_para_decv.puser_data);
    }
    // DECV
    if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
        if (aui_decv_open(&attr_decv,&decv_hdl)) {
            AUI_PRINTF("\n aui_decv_open fail\n");
            return -1;
        }
    }

    AUI_PRINTF("DEBUG trace %d init_para_decv.ul_video_type %ld\n", __LINE__, init_para_decv.ul_video_type);
    if (aui_decv_decode_format_set(decv_hdl,decv_type)) {
        AUI_PRINTF("\n aui_decv_decode_format_set fail\n");
        return -1;
    }

    AUI_PRINTF("DEBUG trace %d init_para_decv.ul_chg_mode %d\n", __LINE__, init_para_decv.ul_chg_mode);
    if (init_para_decv.ul_chg_mode)
    {
        AUI_PRINTF("DEBUG trace %d init_para_decv.ul_chg_mode %d\n", __LINE__, init_para_decv.ul_chg_mode);
        /*We have two modes for changing programs,one is AUI_CHG_BLACK which means it will show black screen
         *when changing programs.The other is AUI_CHG_STILL which means it will show the last picture on the 
         *screen when changing programs.*/
        if (aui_decv_chg_mode_set(decv_hdl, init_para_decv.ul_chg_mode)) 
        {
            AUI_PRINTF("\n aui_decv_chg_mode_set fail\n");
            return -1;
        }
    }
    if (aui_decv_start(decv_hdl)) {
        AUI_PRINTF("\n aui_decv_start fail\n");
        return -1;
    }
   
    *p_hdl=decv_hdl;
    return 0;
}


int ali_app_audio_init(struct ali_app_audio_init_para init_para_audio,aui_hdl *p_hdl_deca,aui_hdl *p_hdl_snd)
{
    aui_hdl deca_hdl=0;
    aui_hdl snd_hdl=0;
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;

    MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
    MEMSET(&attr_snd,0,sizeof(aui_attr_snd));

    // SND
    if(aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl)) {
        if (aui_deca_open(&attr_deca,&deca_hdl)) {
            AUI_PRINTF("\n aui_deca_open fail\n");
            return -1;
        }
    }

    if (aui_deca_type_set(deca_hdl,init_para_audio.ul_audio_type)) {
        AUI_PRINTF("\n aui_deca_start fail\n");
        return -1;
    }
    

    if (aui_deca_start(deca_hdl,&attr_deca)) {
        AUI_PRINTF("\n aui_deca_start fail\n");
        return -1;
    }


    if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl)) {
        if (aui_snd_open(&attr_snd,&snd_hdl)) {
            AUI_PRINTF("\n aui_snd_open fail\n");
            return -1;
        }
    }
#if 0
    if (0!= aui_snd_start(snd_hdl,&attr_snd))
    {
        AUI_PRINTF("\n aui_snd_start fail\n");
        return -1;
    }
#endif
    aui_snd_mute_set(snd_hdl,0);
    aui_snd_vol_set(snd_hdl,init_para_audio.ul_volume);
    *p_hdl_deca=deca_hdl;
    *p_hdl_snd=snd_hdl;
    return 0;

}

int ali_app_av_init(struct ali_aui_hdls *p_handles,aui_hdl *p_hdl)
{
    aui_hdl av_hdl=0;
    aui_attrAV attr_av;
    MEMSET(&attr_av,0,sizeof(aui_attrAV));

    attr_av.st_av_info.b_audio_enable = 1;
    attr_av.st_av_info.b_dmx_enable = 1;
    attr_av.st_av_info.b_pcr_enable = 1;
    attr_av.st_av_info.b_video_enable = 1;
    attr_av.st_av_info.en_audio_stream_type = AUI_DECA_STREAM_TYPE_MPEG1;
    attr_av.st_av_info.en_spdif_type = AUI_SND_OUT_MODE_DECODED;
    attr_av.st_av_info.en_video_stream_type = AUI_DECV_FORMAT_MPEG;
    attr_av.st_av_info.ui_audio_pid = pids[1];
    attr_av.st_av_info.ui_video_pid = pids[0];
    attr_av.st_av_info.ui_pcr_pid = pids[2];

    aui_attr_decv attr_decv;
    aui_attr_deca attr_deca;
    aui_attr_dmx attr_dmx;
    aui_attr_snd attr_snd;
    MEMSET(&attr_decv,0,sizeof(aui_attr_decv));
    MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&attr_snd,0,sizeof(aui_attr_snd));
    if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &p_handles->decv_hdl)) {
        if (aui_decv_open(&attr_decv,&p_handles->decv_hdl)) {
            AUI_PRINTF("\n aui_decv_open fail\n");
            return -1;
        }
    }
    if(aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &p_handles->deca_hdl)) {
        if (aui_deca_open(&attr_deca,&p_handles->deca_hdl)) {
            AUI_PRINTF("\n aui_deca_open fail\n");
            return -1;
        }
    }
    if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &p_handles->dmx_hdl)) {
        if (aui_dmx_open(&attr_dmx,&p_handles->dmx_hdl)) {
            AUI_PRINTF("\n aui_dmx_open fail\n");
            return -1;
        }
    }
    if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &p_handles->snd_hdl)) {
        if (aui_snd_open(&attr_snd,&p_handles->snd_hdl)) {
            AUI_PRINTF("\n aui_snd_open fail\n");
            return -1;
        }
    }

    attr_av.pv_hdl_decv = p_handles->decv_hdl;
    attr_av.pv_hdl_dmx = p_handles->dmx_hdl;
    attr_av.pv_hdl_deca = p_handles->deca_hdl;
    attr_av.pv_hdl_snd = p_handles->snd_hdl;

    if (aui_av_open(&attr_av,&av_hdl)) {
        AUI_PRINTF("\n aui_av_open fail\n");
        return -1;
    }

    if (aui_av_start(av_hdl)) {
        AUI_PRINTF("\n aui_av_start fail\n");
        return -1;
    }
    aui_snd_mute_set(p_handles->snd_hdl, 0);
    aui_snd_vol_set(p_handles->snd_hdl, 30);
#if 0
    if (aui_av_pause(av_hdl)) {
        AUI_PRINTF("\n aui_hdmi_open fail\n");
        return -1;
    }
    if (aui_av_resume(av_hdl)) {
        AUI_PRINTF("\n aui_hdmi_open fail\n");
        return -1;
    }
#endif
    *p_hdl=av_hdl;
    return 0;
}

int ali_app_tsg_init(struct ali_app_tsg_init_para *para, aui_hdl *p_hdl)
{
    aui_attr_tsg attr;

    attr.ul_tsg_clk = para->ul_tsg_clk;
    if (aui_tsg_open(&attr, p_hdl)) {
        AUI_PRINTF("aui_tsg_init error\n");
        aui_tsg_de_init(NULL, NULL);
        return -1;
    }

    attr.ul_bit_rate =  para->ul_bit_rate;
    if (aui_tsg_start(*p_hdl, &attr)) {
        AUI_PRINTF("aui_tsg_start error\n");
        aui_tsg_close(*p_hdl);
        return 1;
    }
    return 0;
}
int ali_aui_init_para_for_test_nim(int argc,char **argv,
                struct ali_app_modules_init_para *init_para)
{
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
    enum aui_dis_format dis_format = AUI_DIS_HD;/*display format*/
    unsigned long  nim_dev = 0;
    unsigned long  nim_demod_type = 0;
    unsigned long  freq = FREQ;
    unsigned long  symb = SYMB;
    unsigned long  v_type = 0;
    unsigned long  a_type = 0;
    unsigned long  band = 0;
    aui_ter_std nim_std=0; /*choose ISDBT default*/

    /* extract para from argv*/
    if (argc > 0) {
        nim_dev = atoi(argv[0]);/*nim device index,generally have tow device:0 and 1*/
    }
    if (argc > 1) {
        nim_demod_type = atoi(argv[1]);/*signal demodulation type:DVB-S,DVB-C,DVB-T */
    }
    if (argc > 2) {
        freq = atoi(argv[2]);/* channel frequency*/
    }
    if (argc > 3) {
        symb = atoi(argv[3]);/* channel symbal*/
        band = symb;/*channel bandwidth*/
    }
    if (argc > 4) {
        pids[0] = atoi(argv[4]);/*video pid*/
    }
    if (argc > 5) {
        pids[1] = atoi(argv[5]);/*audio pid*/
    }
    if (argc > 6) {
        pids[2] = atoi(argv[6]);/*pcr(personal clock reference) pid*/
    }
    if (argc > 7) {
        v_type = atoi(argv[7]);/*video format*/
    }
    if (argc > 8) {
        a_type = atoi(argv[8]);/*audio format*/
    }
    if (argc > 9) {
        dis_format = (enum aui_dis_format)atoi(argv[9]);/*display format*/
    }
    if (argc > 10) {
        nim_std = (aui_ter_std)atoi(argv[10]);/*DVBT type*/
    }
    MEMSET(init_para,0,sizeof(struct ali_app_modules_init_para));

    init_para->init_para_nim.ul_device = nim_dev;
    init_para->init_para_nim.ul_freq = freq;
    init_para->init_para_nim.ul_symb_rate = symb;
    init_para->init_para_nim.ul_nim_type = nim_demod_type;
    init_para->init_para_nim.ul_freq_band = band;
    init_para->init_para_nim.ul_nim_std = nim_std;

    init_para->init_para_tsi.ul_dmx_idx = AUI_TSI_OUTPUT_DMX_0;

    init_para->init_para_tsi.ul_tsi_id = 0;
    init_para->init_para_tsi.ul_tis_port_idx = AUI_TSI_CHANNEL_0;

    aui_tsi_config(tsi_cfg);
    init_para->init_para_tsi.ul_hw_init_val = tsi_cfg[nim_dev].ul_hw_init_val;
    init_para->init_para_tsi.ul_input_src = tsi_cfg[nim_dev].ul_input_src;

//  if (nim_dev == AUI_TSG_DEV) {
        /* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz
         * 24 -> TSG clock 4.16 MHz
         * 32 -> TSG clock 3.12 MHz
         * 48 -> TSG clock 2.08 MHz
         */
    //  init_para.init_para_tsg.ul_tsg_clk = 8;
    //  init_para.init_para_tsg.ul_bit_rate = 0; /* 0 for default bitrates */
    //}

    init_para->dmx_create_av_para.dmx_id=0;
    init_para->dmx_create_av_para.video_encode_type=v_type;
    init_para->dmx_create_av_para.video_pid=pids[0];
    init_para->dmx_create_av_para.audio_pid=pids[1];
    init_para->dmx_create_av_para.audio_desc_pid=0x1fff;
    init_para->dmx_create_av_para.pcr_pid=pids[2];
    AUI_PRINTF("\r\n pid=[%d][%d][%d][%d]",init_para->dmx_create_av_para.video_pid,
           init_para->dmx_create_av_para.audio_pid,init_para->dmx_create_av_para.audio_desc_pid,
           init_para->dmx_create_av_para.pcr_pid);
    
    init_para->init_para_dis.ul_dis_type = dis_format;

    init_para->init_para_decv.ul_video_type = v_type;
    init_para->init_para_decv.ul_preview_enable = 0;

    init_para->init_para_audio.ul_volume = 50;
    init_para->init_para_audio.ul_audio_type = a_type;

    return 0;
}

int ali_aui_init_para_for_test_tsg(int argc,char **argv,
    struct ali_app_modules_init_para *init_para)
{
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
    unsigned short pids[3] = { 401 /* video */, 402 /* audio */, 401 /* pcr */ };
    unsigned long  v_type = 0;
    unsigned long  a_type = 0;
    enum aui_dis_format dis_format = AUI_DIS_HD;
    MEMSET(&tsi_cfg,0,sizeof(struct aui_tsi_config));

    MEMSET(init_para,0,sizeof(struct ali_app_modules_init_para));

    /* extract para from argv*/
    pids[0] = atoi(argv[1]); //vpid
    pids[1] = atoi(argv[2]); //apid
    pids[2] = atoi(argv[3]); //ppid
    v_type = atoi(argv[4]);
    a_type = atoi(argv[5]);
    dis_format = atoi(argv[6]);

    AUI_PRINTF("video_pid=%d,audio_pid=%d,pcr_pid=%d\n",pids[0],pids[1],pids[2]);

    init_para->init_para_nim.ul_device = AUI_TSG_DEV;

    init_para->init_para_tsi.ul_dmx_idx = AUI_TSI_OUTPUT_DMX_0;
    init_para->init_para_tsi.ul_tsi_id = 0;
    init_para->init_para_tsi.ul_tis_port_idx = AUI_TSI_CHANNEL_0;

    aui_tsi_config(tsi_cfg);
    init_para->init_para_tsi.ul_hw_init_val = tsi_cfg[AUI_TSG_DEV].ul_hw_init_val;
    init_para->init_para_tsi.ul_input_src = tsi_cfg[AUI_TSG_DEV].ul_input_src;

    /* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz
     * 24 -> TSG clock 4.16 MHz
     * 32 -> TSG clock 3.12 MHz
     * 48 -> TSG clock 2.08 MHz
     */
    init_para->init_para_tsg.ul_tsg_clk = 8;
    init_para->init_para_tsg.ul_bit_rate = 0; /* 0 for default bitrates */

    init_para->dmx_create_av_para.dmx_id=0;
    init_para->dmx_create_av_para.video_encode_type=v_type;
    init_para->dmx_create_av_para.video_pid=pids[0];
    init_para->dmx_create_av_para.audio_pid=pids[1];
    init_para->dmx_create_av_para.audio_desc_pid=0x1fff;
    init_para->dmx_create_av_para.pcr_pid=pids[2];
    AUI_PRINTF("\r\n pid=[%d][%d][%d][%d]",init_para->dmx_create_av_para.video_pid,
           init_para->dmx_create_av_para.audio_pid,init_para->dmx_create_av_para.audio_desc_pid,
           init_para->dmx_create_av_para.pcr_pid);
    
    init_para->init_para_dis.ul_dis_type = dis_format;

    init_para->init_para_decv.ul_video_type = v_type;
    init_para->init_para_decv.ul_preview_enable = 0;

    init_para->init_para_audio.ul_volume = 50;
    init_para->init_para_audio.ul_audio_type = a_type;

    return 0;

}

int ali_app_nim_deinit(aui_hdl hdl)
{
    int err = 0;
    if (aui_nim_close(hdl)) {
        AUI_PRINTF("\r\n aui_nim_close error \n");
        err = 1;
    }
    if (aui_nim_de_init(NULL)) {
        AUI_PRINTF("\r\n aui_nim_de_init error \n");
        err = 1;
    }
    return err;
}

int ali_app_tsi_deinit(aui_hdl hdl)
{
    if(aui_tsi_close(hdl)) {
        AUI_PRINTF("\r\n aui_tsi_close error \n");
        return 1;
    }
    return 0;
}


int ali_app_tsg_deinit(aui_hdl hdl)
{

    if(aui_tsg_close(hdl)) {
        AUI_PRINTF("\r\n aui_tsg_close error \n");
        return 1;
    }
    return 0;
}

int ali_app_dmx_deinit(aui_hdl hdl)
{
    int err = 0;
    if(aui_dmx_stop(hdl, NULL)) {
        AUI_PRINTF("\n aui_dmx_stop error \n");
        err = 1;
    }
    if(aui_dmx_close(hdl)) {
        AUI_PRINTF("\n aui_dmx_close error \n");
        err = 1;
    }
    return err;
}

int ali_app_snd_deinit(aui_hdl hdl_deca,aui_hdl hdl_snd)
{
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;
    int err = 0;

    MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
    MEMSET(&attr_snd,0,sizeof(aui_attr_snd));

    if (aui_deca_stop(hdl_deca,&attr_deca)) {
        AUI_PRINTF("\n aui_deca_close error \n");
        err = 1;
    }
#if 0
    if(aui_snd_stop(hdl_snd,&attr_snd)) {
        AUI_PRINTF("\n aui_snd_stop error \n");
        err = 1;
    }
#endif
    AUI_PRINTF("\r\n close deca aui");
    if (aui_deca_close(hdl_deca)) {
        AUI_PRINTF("\n aui_deca_close error \n");
        err = 1;
    }
    AUI_PRINTF("\r\n close snd aui");
    if (aui_snd_close(hdl_snd)) {
        AUI_PRINTF("\n aui_deca_close error \n");
        err = 1;
    }
    return err;
}

int ali_app_dis_deinit(aui_hdl hdl)
{
    int err = 0;
    if (aui_dis_video_enable(hdl, 0)) {
        AUI_PRINTF("\n aui_dis_video_enable fail\n");
        err = 1;
    }

    if(aui_dis_close(NULL,&hdl)) {
        AUI_PRINTF("\n aui_dis_close error \n");
        err = 1;
    }
    return err;
}

int ali_app_decv_deinit(aui_hdl hdl)
{
    int err = 0;

    if(aui_decv_stop(hdl)) {
        AUI_PRINTF("\n aui_decv_stop error \n");
        err = 1;
    }
    if(aui_decv_close(NULL,&hdl)) {
        AUI_PRINTF("\n aui_decv_close error \n");
        err = 1;
    }
    return err;
}


int ali_app_deinit(struct ali_aui_hdls *handles)
{
#ifdef AUI_LINUX
    AUI_PRINTF("\r\n close nim aui");
    if (handles->nim_hdl && ali_app_nim_deinit(handles->nim_hdl))
        AUI_PRINTF("\r\n ali_app_nim_deinit failed!");

    AUI_PRINTF("\r\n close tsg aui");
    if (handles->tsg_hdl && ali_app_tsg_deinit(handles->tsg_hdl))
        AUI_PRINTF("\r\n ali_app_tsg_deinit failed!");

    AUI_PRINTF("\r\n close tsi aui");
    if (handles->tsi_hdl && ali_app_tsi_deinit(handles->tsi_hdl))
        AUI_PRINTF("\r\n ali_app_tsi_deinit failed!");

    AUI_PRINTF("\r\n close dmx aui");
    if (handles->dmx_hdl && ali_app_dmx_deinit(handles->dmx_hdl))
        AUI_PRINTF("\r\n ali_app_dmx_deinit failed!");

    AUI_PRINTF("\r\n close snd aui");
    if (handles->snd_hdl &&
        ali_app_snd_deinit(handles->deca_hdl,handles->snd_hdl))
        AUI_PRINTF("\r\n ali_app_snd_deinit failed!");

    AUI_PRINTF("\r\n close decv aui");
    if (handles->decv_hdl && ali_app_decv_deinit(handles->decv_hdl))
        AUI_PRINTF("\r\n ali_app_decv_deinit failed!");

    AUI_PRINTF("\r\n close dis aui");
    if (handles->dis_hdl && ali_app_dis_deinit(handles->dis_hdl))
        AUI_PRINTF("\r\n ali_app_dis_deinit failed!");
#else
    aui_attr_deca attr_deca;
    aui_attr_decv attr_decv;

    MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
    MEMSET(&attr_decv,0,sizeof(aui_attr_decv));

    AUI_PRINTF("\r\n handles.deca_hdl=[%08x];handles.dmx_hdl=[%08x]",handles->deca_hdl,handles->dmx_hdl);
    
    if(0==s_play_mode)
    {
        if (aui_dmx_set(handles->dmx_hdl, AUI_DMX_STREAM_DISABLE, 0)) 
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_DISABLE failed\n");
        }
    }
    else if (1 == s_play_mode)
    {
        if (aui_dmx_set(handles->dmx_hdl, AUI_DMX_STREAM_DISABLE_VIDEO, 0)) 
        {
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_DISABLE_VIDEO failed\n");
        }

        
    }else{
        if (aui_dmx_set(handles->dmx_hdl, AUI_DMX_STREAM_DISABLE_AUDIO, 0)){
            AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_DISABLE_AUDIO failed\n");
        }
    }
    aui_deca_stop(handles->deca_hdl,&attr_deca);
    aui_decv_stop(handles->decv_hdl);
#endif
    return AUI_RTN_SUCCESS;
}


int ali_app_tsi_init_extend(struct ali_app_tsi_init_para *para,aui_hdl *p_hdl)
{
    aui_attr_tsi attr_tsi;
    MEMSET(&attr_tsi, 0, sizeof(aui_attr_tsi));
    attr_tsi.ul_init_param = para->ul_hw_init_val;

    AUI_PRINTF("DEBUG trace %d para->ul_input_src = %ld,ul_tis_port_idx = %ld\n", __LINE__,para->ul_input_src,para->ul_tis_port_idx);

    if(*p_hdl == NULL)
    {
        AUI_PRINTF("%d aui tsi open \n",__LINE__);
        if (aui_tsi_open(p_hdl)) {
            AUI_PRINTF("\r\n aui_tsi_open error \n");
            return -1;
        }
    }

    if (aui_tsi_src_init(*p_hdl, para->ul_input_src, &attr_tsi)) {
        AUI_PRINTF("\r\n aui_tsi_src_init error \n");
        return -1;
    }
    if (aui_tsi_route_cfg(*p_hdl, para->ul_input_src,
                para->ul_tis_port_idx,
                para->ul_dmx_idx)) {
        AUI_PRINTF("\r\n aui_tsi_route_cfg error \n");
        return -1;
    }
    return 0;
}
