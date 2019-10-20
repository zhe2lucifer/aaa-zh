#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aui_ini_config.h"
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

#include "aui_help_print.h"
#include "aui_test_app_cmd.h"
#include "aui_test_app.h"

#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_nim.h"
#include "aui_test_stream_nim_dsc.h"
#include "aui_test_stream_nim_dsc_kl.h"
#include "aui_test_stream_tsg.h"
#include "aui_test_stream_tsg_dsc.h"
#include "aui_test_stream_tsg_dsc_kl.h"
#include "aui_test_stream_play_live_stream.h"
#include "aui_test_stream_scan_plp.h"
// In abnormal situations DVB-C auto QAM mode need about 3s to connect 256 QAM signal.
#define NIM_TIMEOUT_MS (10 * 1000)

// C Band LNB L.O. frequency
#define LO_5150 (5150)
#define LO_5750 (5750)
// Ku Band LNB L.O. frequency
#define LO_9750 (9750)
#define LO_10600 (10600)

/* cctv2 */
#define FREQ 3840
#define SYMB 27500
unsigned short pids[4] = { 513 /* video */, 660 /* audio */, 8190 /* pcr */, 8190 /*ad pid */ };
extern unsigned long s_play_mode;
int nim_connect(struct ali_app_nim_init_para *para,aui_hdl *aui_nim_hdl)
{
#ifndef CONFIG_ALI_EMULATOR        
    aui_hdl hdl = 0;
    struct aui_nim_attr nim_attr;
    struct aui_nim_connect_param param;
    int as_high_band = AUI_NIM_LOW_BAND;
    int lock_status = AUI_NIM_STATUS_UNKNOWN;
    int timeout = NIM_TIMEOUT_MS / 10;
    int connect_time = 0;
    int lock_time = 0;
#ifdef _RD_DEBUG_
    unsigned long temp_freq = 0;
#endif
#ifndef AUI_TDS
    struct timeval begin_timeval = {0, 0};
    struct timeval end_timeval = {0, 0};
    struct timeval all_timeval = {0, 0};
#endif
#ifndef AUI_LINUX
    unsigned long begin_time = 0;
    unsigned long end_time = 0;
#endif

    /*init nim_attr and param variable*/
    MEMSET(&param, 0, sizeof(struct aui_nim_connect_param));
    MEMSET(&nim_attr, 0, sizeof(struct aui_nim_attr));

    nim_attr.ul_nim_id = para->ul_device;
    nim_attr.en_dmod_type = para->ul_nim_type;
    nim_attr.en_ter_std = para->ul_nim_std; //added by vedic.fu
    AUI_PRINTF("\nul_nim_std = %d, ul_nim_id = %d\n",para->ul_nim_std, nim_attr.ul_nim_id);
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
            AUI_PRINTF("%d aui_nim_handle_get_byid %d error\n",
                    __LINE__, para->ul_device);
            return -1;
        }
#endif
    AUI_PRINTF("nim device %d opened\n", (int)nim_attr.ul_nim_id);

    switch (para->ul_nim_type) {/*signal modulation types:DVB-S,DVB-C,DVB-T*/
    case AUI_NIM_QPSK:/*DVB-S*/

// All C-band LNB have a local oscillator (L.O.) frequency of 5.150 GHz
// but Ku-band LNB may come in many different frequencies 
// typically 9.750 to 12.75 GHz.

// C-band dual polarity LNB (One-Cable-Solution)
// Different polarizations use different  L.O Frequency, So users can receive
// different polarizations signal at same time.
// Polarization  Frequency band  L.O Frequency  I.F.
// Horizontal    3.7-4.2 GHz     5.15 GHz       950-1,450 MHz
// Vertical      3.7-4.2 GHz     5.75 GHz       1,550-2,050 MHz

// Ku-band LNB
// Europe Universal LNB ("Astra" LNB)
// Voltage  Tone    Polarization  Frequency        band  L.O Frequency  I.F.
// 13 V     0 kHz   Vertical      10.70-11.70 GHz  low   9.75 GHz       950-1,950 MHz
// 18 V     0 kHz   Horizontal    10.70-11.70 GHz  low   9.75 GHz       950-1,950 MHz
// 13 V     22 kHz  Vertical      11.70-12.75 GHz  high  10.60 GHz      1,100-2,150 MHz
// 18 V     22 kHz  Horizontal    11.70-12.75 GHz  high  10.60 GHz      1,100-2,150 MHz

#ifdef _RD_DEBUG
        temp_freq = para->ul_freq;
#endif
        if (para->ul_freq > 10700) {
            if (para->ul_freq> 11700) {
                as_high_band = AUI_NIM_HIGH_BAND;
                param.ul_freq = para->ul_freq - LO_10600;
            } else {
                as_high_band = AUI_NIM_LOW_BAND;
                param.ul_freq = para->ul_freq - LO_9750;
            }
        } else {
            as_high_band = AUI_NIM_LOW_BAND;
            if (para->ul_polar)
                param.ul_freq = LO_5750 - para->ul_freq;
            else
                param.ul_freq = LO_5150 - para->ul_freq;
        }
        param.en_demod_type = AUI_NIM_QPSK;
        param.connect_param.sat.ul_freq_band = as_high_band;
        param.connect_param.sat.ul_symrate = para->ul_symb_rate;
        param.connect_param.sat.fec = AUI_NIM_FEC_AUTO;
        param.connect_param.sat.ul_polar = para->ul_polar;
        param.connect_param.sat.ul_src = para->ul_src;
        param.connect_param.sat.ul_stream_id = para->plp_index;
        AUI_PRINTF("DVB-S freq:%d (I.F. %ld), sym:%ld, polar:%d, band:%d, src:%d\n",
                temp_freq, param.ul_freq,
                param.connect_param.sat.ul_symrate,
                param.connect_param.sat.ul_polar,
                param.connect_param.sat.ul_freq_band,
                param.connect_param.sat.ul_src,
				param.connect_param.sat.ul_stream_id);
        break;

    case AUI_NIM_QAM:/*DVB-C*/
        param.ul_freq = para->ul_freq;
        param.en_demod_type = AUI_NIM_QAM;
        param.connect_param.cab.ul_symrate = para->ul_symb_rate;
        param.connect_param.cab.en_qam_mode = AUI_NIM_NOT_DEFINED; // auto QAM
        //param.connect_param.cab.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; // J83A with 6MHz test
        break;

    case AUI_NIM_OFDM:/*DVB-T*/
        param.ul_freq = para->ul_freq;
        param.en_demod_type = AUI_NIM_OFDM;
        switch(para->ul_freq_band)
        {
        case 6: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; break; //added by vedic.fu
        case 7: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_7_MHZ; break;
        case 8: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_8_MHZ; break;
        default : AUI_PRINTF("wrong bandwidth %d\n", para->ul_freq_band); return -1;
        }
        param.connect_param.ter.std = para->ul_nim_std;
        param.connect_param.ter.fec = AUI_NIM_FEC_AUTO;
        param.connect_param.ter.spectrum = AUI_NIM_SPEC_AUTO;
        param.connect_param.ter.u.dvbt2.plp_id = para->plp_id;
        param.connect_param.ter.u.dvbt2.plp_index = para->plp_index;

        AUI_PRINTF("[NIM CONNECT] freq: %d demod: %d bandwidth: %d fec: %d "
                "fft: %d guard: %d modulation: %d spectrum: %d std: %d "
                "plp_id: %d plp_index: %d\n",
                param.ul_freq, param.en_demod_type,
                param.connect_param.ter.bandwidth,
                param.connect_param.ter.fec,
                param.connect_param.ter.fft_mode,
                param.connect_param.ter.guard_interval,
                param.connect_param.ter.modulation,
                param.connect_param.ter.spectrum,
                param.connect_param.ter.std,
                param.connect_param.ter.u.dvbt2.plp_id,
                param.connect_param.ter.u.dvbt2.plp_index);
        break;

    default:
        AUI_PRINTF("not supported NIM type %d\n", (int)para->ul_nim_type);
#ifdef AUI_LINUX
        aui_nim_close(hdl);
#endif
        return -1;
    }
#ifndef AUI_TDS
        gettimeofday(&begin_timeval, NULL);
#endif
#ifndef AUI_LINUX
        begin_time = osal_get_tick();
#endif
    if (aui_nim_connect(hdl, &param)) {
        AUI_PRINTF("%d connect error\n", __LINE__);
#ifdef AUI_LINUX
        aui_nim_close(hdl);
#endif
        return -1;
    }

#ifndef AUI_TDS
    gettimeofday(&end_timeval, NULL);
    timersub(&end_timeval, &begin_timeval, &all_timeval);
    connect_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
    end_time = osal_get_tick();
    connect_time = end_time - begin_time;
#endif

    lock_status = AUI_NIM_STATUS_UNKNOWN;
    timeout = NIM_TIMEOUT_MS / 10;
    while (lock_status != AUI_NIM_STATUS_LOCKED && timeout > 0) {
        if (aui_nim_is_lock(hdl, &lock_status)) {
            AUI_PRINTF("%d is_lock error\n", __LINE__);
            aui_nim_disconnect(hdl);
#ifdef AUI_LINUX
            aui_nim_close(hdl);
#endif
            return -1;
        }
        AUI_SLEEP(10);
        timeout--;
    }

    if (lock_status != AUI_NIM_STATUS_LOCKED) {
        AUI_PRINTF("[channel is not locked]\n");
        aui_nim_disconnect(hdl);
#ifdef AUI_LINUX
        aui_nim_close(hdl);
#endif
        return -1;
    }

#ifndef AUI_TDS
    gettimeofday(&end_timeval, NULL);
    timersub(&end_timeval, &begin_timeval, &all_timeval);
    lock_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
    end_time = osal_get_tick();
    lock_time = end_time - begin_time;
#endif

    AUI_PRINTF("[NIM %ld]: Locking takes %d MS (aui_nim_connect takes %d MS)\n",
            para->ul_device, lock_time, connect_time);
    (void)connect_time;
    (void)lock_time;

    if (para->ul_nim_type == AUI_NIM_OFDM){
        AUI_PRINTF("After connect DVB-T std: %d\n", param.connect_param.ter.std);
   }

    *aui_nim_hdl = hdl;
#endif    
    return 0;
}

int nim_get_signal_info(struct ali_app_nim_init_para *para)
{
    aui_hdl hdl = 0;
    unsigned int dev_id = para->ul_device;
    if (aui_nim_handle_get_byid(dev_id, &hdl)) {
        printf("%d %s NIM %d not found\n", __LINE__, __FUNCTION__, index);
        return -1;
    }

    int as_high_band = AUI_NIM_LOW_BAND;

    // Driver need at lease 50MS to prepare the signal info after locked
    AUI_SLEEP(100); // wait 100MS

    int tmp_cnt = 10; //
    while (tmp_cnt-- > 0) {
        aui_signal_status info;
        if (aui_nim_signal_info_get(hdl, &info)) {/*getting the infomation of current tp*/
            AUI_PRINTF("%d info_get error\n", __LINE__);
            aui_nim_disconnect(hdl);
#ifdef AUI_LINUX
            aui_nim_close(hdl);
#endif
            return -1;
        }
        int temp_freq = 0;
        temp_freq = info.ul_freq;
        if (para->ul_nim_type == AUI_NIM_QPSK) {/*DVB-S*/
            if (para->ul_freq > 10700) {
                if (as_high_band == AUI_NIM_HIGH_BAND)
                    temp_freq += LO_10600;
                else
                    temp_freq += LO_9750;
            } else {
                if (para->ul_polar)
                    temp_freq = LO_5750 - info.ul_freq;
                else
                    temp_freq = LO_5150 - info.ul_freq;
            }
        }
        AUI_PRINTF("[NIM %ld]: IF %ld(freq %ld) strength %ld, quality %ld, "
                "ber %ld, rf_level %ld, signal_cn %ld, mer: %ld, per:%ld, pre_ber:%ld\n",
                para->ul_device,
                info.ul_freq, temp_freq,
                info.ul_signal_strength, info.ul_signal_quality,
                info.ul_bit_error_rate, info.ul_rf_level,
                info.ul_signal_cn, info.ul_mer, info.ul_per, info.ul_pre_ber);

        if (para->ul_nim_type == AUI_NIM_OFDM) {
            AUI_PRINTF("plp_num: %d plp_id: %d\n",
                    info.u.dvbt2.plp_num, info.u.dvbt2.plp_id);
            AUI_PRINTF("info.std: %d\n", info.std);
        } else if (para->ul_nim_type == AUI_NIM_QPSK) {
            if (info.std == AUI_NIM_STD_DVBS) {
                AUI_PRINTF("work mode: DVBS\n");
            } else if (info.std == AUI_NIM_STD_DVBS2){
                AUI_PRINTF("work mode: DVBS2\n");
            }
		}
        AUI_SLEEP(200); // MS
    }
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

#ifdef AUI_LINUX
int ali_app_tsi_init_cic(struct ali_app_tsi_init_para *para,aui_hdl *p_hdl)
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
#if 0
    // TASK #44551, do not call aui_tsi_CIConnect_mode_set here,
    // the aui_tsi_ci_card_bypass_set would set the cic connect mode.
    printf("Enter aui_tsi_CIConnect_mode_set\n");
    if(aui_tsi_CIConnect_mode_set(*p_hdl,1)) {
        printf("aui_tsi_CIConnect_mode_set fail\n");
    }
#endif
    if (aui_tsi_route_cfg(*p_hdl, para->ul_input_src,
                 para->ul_tis_port_idx,
                 para->ul_dmx_idx)) {
        AUI_PRINTF("\r\n aui_tsi_route_cfg error \n");
        return -1;
    }
#if 1
    //printf("Leave aui_tsi_CIConnect_mode_set\n");
    printf("Enter aui_tsi_ci_card_bypass_set\n");

    if(aui_tsi_ci_card_bypass_set(*p_hdl, 0, 0)) {
        printf("aui_tsi_ci_card_bypass_set fail\n");
    }
    printf("Leave aui_tsi_ci_card_bypass_set\n");
#endif
    return 0;
}
#endif
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

    if(0==s_play_mode)
    {
        if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list)) {
            AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
            aui_dmx_close(hdl);
            return -1;
        }
    }else if(1 == s_play_mode)
    {
        if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AUDIO, &pid_list)) {
            AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AUDIO fail\n");
            aui_dmx_close(hdl);
            return -1;
        }
    }else{
        if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_VIDEO, &pid_list)) {
            AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_VIDEO fail\n");
            aui_dmx_close(hdl);
            return -1;
        }
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
    if(aui_find_dev_by_idx(AUI_MODULE_DIS, attr_dis.uc_dev_idx, &dis_hdl)) {/*if dis device has opened,return dis device handle*/
        if (aui_dis_open(&attr_dis, &dis_hdl)) {/*if dis hasn't opened,open dis device and return dis device handle*/
            AUI_PRINTF("\n aui_dis_open fail\n");
            return -1;
        }
    }
    /*Because logo will be showed after boot, but Linux does not have the last frame of backup at present. 
    If there is no closing the layer when the first playing stream, the blurred screen will appear*/
    #ifdef AUI_LINUX
    if (aui_dis_layer_enable(dis_hdl, init_para_dis.ul_dis_layer, 0)) {
        AUI_PRINTF("\n aui_dis_video_enable fail\n");
        return -1;
    }
    #else
    if (aui_dis_video_enable(dis_hdl, 0)) {
        AUI_PRINTF("\n aui_dis_video_enable fail\n");
        return -1;
    }
    #endif
    *p_hdl=dis_hdl;
    return 0;
}


int ali_app_decv_init(struct ali_app_decv_init_para init_para_decv,aui_hdl *p_hdl)
{
    aui_hdl decv_hdl=0;
    (void) init_para_decv;

    aui_attr_decv attr_decv;
    enum aui_decv_format decv_type=init_para_decv.ul_video_type;
    MEMSET(&attr_decv,0,sizeof(aui_attr_decv));
    attr_decv.uc_dev_idx =  init_para_decv.video_id;
    attr_decv.dis_layer = init_para_decv.ul_dis_layer;
    if (init_para_decv.callback != NULL)
    {
        attr_decv.callback_nodes[0].type= AUI_DECV_CB_FIRST_SHOWED;
        attr_decv.callback_nodes[0].callback = init_para_decv.callback;
        attr_decv.callback_nodes[0].puser_data = init_para_decv.puser_data;
        AUI_PRINTF("DEBUG trace %d %p\n", __LINE__,init_para_decv.puser_data);
    }
    // DECV
    if(aui_find_dev_by_idx(AUI_MODULE_DECV, attr_decv.uc_dev_idx, &decv_hdl)) {
        if (aui_decv_open(&attr_decv,&decv_hdl)) {
            AUI_PRINTF("\n aui_decv_open fail\n");
            return -1;
        }
    }

    AUI_PRINTF("DEBUG trace %d init_para_decv.ul_video_type %d\n", __LINE__, init_para_decv.ul_video_type);
    if (aui_decv_decode_format_set(decv_hdl,decv_type)) {
        AUI_PRINTF("\n aui_decv_decode_format_set fail\n");
        return -1;
    }

    AUI_PRINTF("DEBUG trace %d init_para_decv.ul_chg_mode %d\n", __LINE__, init_para_decv.ul_chg_mode);
    if (init_para_decv.ul_chg_mode)
    {
        AUI_PRINTF("DEBUG trace %d init_para_decv.ul_chg_mode %d\n", __LINE__, init_para_decv.ul_chg_mode);
        /*We have two modes for changing programs,one is AUI_DECV_CHG_BLACK which means it will show black screen
         *when changing programs.The other is AUI_DECV_CHG_STILL which means it will show the last picture on the
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
    AUI_PRINTF("set volume to : %d\n", init_para_audio.ul_volume);
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
    
    MEMSET(&attr, 0, sizeof(aui_attr_tsg));    
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
int ali_aui_init_para_for_test_nim(unsigned long *argc,char **argv,
                struct ali_app_modules_init_para *init_para)
{
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
    enum aui_dis_format dis_format = AUI_DIS_HD;/*display format*/
    enum aui_en_decv_chg_mode change_mode = AUI_DECV_CHG_BLACK;/*the channel change mode*/
    unsigned long  nim_dev = 0;
    unsigned long  nim_demod_type = 0;
    unsigned long  freq = FREQ;
    unsigned long  symb = SYMB;
    unsigned long  v_type = 0;
    unsigned long  a_type = 0;
    unsigned long  band = 0;
    aui_nim_std nim_std=0; /*choose ISDBT default*/
    unsigned long  polar = AUI_NIM_POLAR_HORIZONTAL;
    long plp_index = 0;
    long plp_id = 0;

    /* extract para from argv*/
    if (*argc > 0) {
        nim_dev = atoi(argv[0]);/*nim device index,generally have tow device:0 and 1*/
    }
    if (*argc > 1) {
        nim_demod_type = atoi(argv[1]);/*signal demodulation type:DVB-S,DVB-C,DVB-T */
    }
    if (*argc > 2) {
        freq = atoi(argv[2]);/* channel frequency*/
    }
    if (*argc > 3) {
        symb = atoi(argv[3]);/* channel symbal*/
        band = symb;/*channel bandwidth*/
    }
    if (*argc > 4) {
        pids[0] = atoi(argv[4])&AUI_INVALID_PID;/*video pid*/
    }
    if (*argc > 5) {
    	pids[1] = atoi(argv[5])&AUI_INVALID_PID; /*audio pid*/
    }
    if (*argc > 6) {
        pids[2] = atoi(argv[6])&AUI_INVALID_PID;/*pcr(personal clock reference) pid*/
        pids[3] = AUI_INVALID_PID;
        get_audio_description_pid(pids+3);
    }
    if (*argc > 7) {
        v_type = atoi(argv[7]);/*video format*/
    }
    if (*argc > 8) {
        a_type = atoi(argv[8]);/*audio format*/
    }
    if (*argc > 9) {
        change_mode = (enum aui_en_decv_chg_mode)atoi(argv[9]);/*the channel change mode*/
    }
    if (*argc > 10) {
        // DVB-S polar
        if (argv[10][0] == 'V' || argv[10][0] == 'v'
                || argv[10][0] == 'R' || argv[10][0] == 'r') {
            polar = AUI_NIM_POLAR_VERTICAL;
        } else {
            polar = AUI_NIM_POLAR_HORIZONTAL;
        }
        nim_std = (aui_nim_std)atoi(argv[10]);/*DVBT type*/
    }
    if (*argc > 11) {
        plp_index = atoi(argv[11]);
        AUI_PRINTF("PLP index: %ld\n", plp_index);
        (void)plp_index;//just for build
    }
    if (*argc > 12) {
        plp_id = atoi(argv[12]);
        AUI_PRINTF("PLP id: %ld\n", plp_id);
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
    init_para->dmx_create_av_para.audio_desc_pid=pids[3];
    init_para->dmx_create_av_para.pcr_pid=pids[2];
    AUI_PRINTF("\r\n pid=[%d][%d][%d][%d]",init_para->dmx_create_av_para.video_pid,
           init_para->dmx_create_av_para.audio_pid,init_para->dmx_create_av_para.audio_desc_pid,
           init_para->dmx_create_av_para.pcr_pid);

    init_para->init_para_dis.ul_dis_type = dis_format;

    init_para->init_para_decv.ul_video_type = v_type;
    init_para->init_para_decv.ul_preview_enable = 0;
    init_para->init_para_decv.ul_chg_mode = change_mode;

    init_para->init_para_audio.ul_volume = 50;
    init_para->init_para_audio.ul_audio_type = a_type;
    init_para->init_para_nim.ul_polar = polar;

    init_para->init_para_nim.plp_id  = plp_id;
    init_para->init_para_nim.plp_index  = plp_index;
    return 0;
}

int ali_aui_init_para_for_test_tsg(unsigned long *argc,char **argv,
    struct ali_app_modules_init_para *init_para)
{
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
    unsigned short pids[3] = { 401 /* video */, 402 /* audio */, 401 /* pcr */ };
    unsigned long  v_type = 0;
    unsigned long  a_type = 0;
    enum aui_dis_format dis_format = AUI_DIS_HD;
    enum aui_en_decv_chg_mode change_mode = AUI_DECV_CHG_BLACK;/*the channel change mode*/
    unsigned short tsg_clk_sel = 8;
    MEMSET(&tsi_cfg,0,sizeof(struct aui_tsi_config));

    MEMSET(init_para,0,sizeof(struct ali_app_modules_init_para));

    /* extract para from argv*/
    pids[0] = atoi(argv[1]); //vpid
    pids[1] = atoi(argv[2]); //apid
    pids[2] = atoi(argv[3]); //ppid
    v_type = atoi(argv[4]);
    a_type = atoi(argv[5]);
    change_mode = atoi(argv[6]);
    if (*argc >= 8) {
        tsg_clk_sel = atoi(argv[7]);
    }

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
    init_para->init_para_tsg.ul_tsg_clk = tsg_clk_sel;
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
    init_para->init_para_decv.ul_chg_mode = change_mode;

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
    //if (aui_nim_de_init(NULL)) {
    //    AUI_PRINTF("\r\n aui_nim_de_init error \n");
    //    err = 1;
    //}
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

    if (hdl_snd != NULL) {
        if (aui_snd_close(hdl_snd)) {
            AUI_PRINTF("\n aui_deca_close error \n");
            err = 1;
        }
    }
    return err;
}

int ali_app_dis_deinit(aui_hdl hdl)
{
    int err = 0;

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

    AUI_PRINTF("DEBUG trace %d para->ul_input_src = %d,ul_tis_port_idx = %d\n", __LINE__,para->ul_input_src,para->ul_tis_port_idx);

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

void test_stream_with_nim_help(char* item)
{
    if (item == NULL) {
        item = "item";
    }
    #define     STREAM_NIM_HELP_PART1         "Format: %s [nim_id],[nim_type],[freq],[symb/bandwidth],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[DVBT(T2,ISDBT) type/DVBS polar]"
    #define     STREAM_NIM_HELP_PART2         "            [nim_id]: the index that the tune in"
    #define     STREAM_NIM_HELP_PART3         "            [nim_type]:  0: DVB-S    1: DVB-C    2: DVB-T(T2/IDSBT)"
    #define     STREAM_NIM_HELP_PART4         "            [freq]: tune frequency "
    #define     STREAM_NIM_HELP_PART5         "            [symb/bandwidth]: symbol rate for DVB-S and DVB-C / bandwidth for DVBT(T2/ISDBT)"
    #define     STREAM_NIM_HELP_PART6         "            [vpid]: video pid"
    #define     STREAM_NIM_HELP_PART7         "            [apid]: audio pid"
    #define     STREAM_NIM_HELP_PART8         "            [ppid]: pcr pid"
    #define     STREAM_NIM_HELP_PART9         "            [video format]:0: MPEG    1: AVC(H.264)  10: HEVC(H.265)"
    #define     STREAM_NIM_HELP_PART10        "            [audio format]:0:  AUI_DECA_STREAM_TYPE_MPEG1"
    #define     STREAM_NIM_HELP_PART11        "                           1:  AUI_DECA_STREAM_TYPE_MPEG2"
    #define     STREAM_NIM_HELP_PART12        "                           2:  AUI_DECA_STREAM_TYPE_AAC_LATM"
    #define     STREAM_NIM_HELP_PART13        "                           3:  AUI_DECA_STREAM_TYPE_AC3"
    #define     STREAM_NIM_HELP_PART14        "                           4:  AUI_DECA_STREAM_TYPE_DTS"
    #define     STREAM_NIM_HELP_PART15        "                           5:  AUI_DECA_STREAM_TYPE_PPCM"
    #define     STREAM_NIM_HELP_PART16        "                           6:  AUI_DECA_STREAM_TYPE_LPCM_V"
    #define     STREAM_NIM_HELP_PART17        "                           7:  AUI_DECA_STREAM_TYPE_LPCM_A"
    #define     STREAM_NIM_HELP_PART18        "                           8:  AUI_DECA_STREAM_TYPE_PCM"
    #define     STREAM_NIM_HELP_PART19        "                           9:  AUI_DECA_STREAM_TYPE_WMA"
    #define     STREAM_NIM_HELP_PART20        "                           10: AUI_DECA_STREAM_TYPE_RA8"
    #define     STREAM_NIM_HELP_PART21        "                           11: AUI_DECA_STREAM_TYPE_MP3"
    #define     STREAM_NIM_HELP_PART22        "                           12: AUI_DECA_STREAM_TYPE_AAC_ADTS"
    #define     STREAM_NIM_HELP_PART23        "                           13: AUI_DECA_STREAM_TYPE_OGG"
    #define     STREAM_NIM_HELP_PART24        "                           14: AUI_DECA_STREAM_TYPE_EC3"
    #define     STREAM_NIM_HELP_PART25        "           [change_mode]: 0: API_CHG_BLACK(show black screen when change program) 1: API_CHG_STILL(show the last picture of the last program on screen before the new program begins to play)"
    #define     STREAM_NIM_HELP_PART26        "           [DVBT(T2/ISDBT) type/DVBS polar]:"
    #define     STREAM_NIM_HELP_PART27        "                       DVBS polar: H: Horizontal"
    #define     STREAM_NIM_HELP_PART28        "                                   V: Vertical"
    #define     STREAM_NIM_HELP_PART29        "                        DVBT(T2/ISDBT) type: 0:ISDBT"
    #define     STREAM_NIM_HELP_PART30        "                                   1:DVBT"
    #define     STREAM_NIM_HELP_PART31        "                                   2:DVBT2"
    #define     STREAM_NIM_HELP_PART32        "Example:   CCTV2,If the stream that the NIM type is DVB-S, the NIM device index is 1, the tune frequency is 3840MHz, the symbol rate is 27500, "
    #define     STREAM_NIM_HELP_PART33        "           the vpid is 513, the apid is 660, the ppid is 8190, the video format is MPEG, the audio format is MPEG2, the dis format is high definition is played, the input is"
    #define     STREAM_NIM_HELP_PART34        "           %s 1,0,3840,27500,512,650,8190,0,1,0\n"
    #define     STREAM_NIM_HELP_PART35        "           DVB-S,CCTV2,input: %s 1,0,3840,27500,513,660,8190,0,1,0,H,0\n"
    #define     STREAM_NIM_HELP_PART36        "           DVB-S,CCTV7,input: %s 1,0,3840,27500,514,670,8190,0,1,0,H,0\n"
    #define     STREAM_NIM_HELP_PART37        "           DVB-T, Example,input: %s 0,2,850000,8,1029,1028,1029,0,0,0,1\n"
    #define     STREAM_NIM_HELP_PART38        "           DVB-T2,Example,input: %s 0,2,722000,8,141,142,141,1,2,0,2,3 (Play certain PLP index)\n"
    #define     STREAM_NIM_HELP_PART39        "           ISDBT, Example,input: %s 0,2,587143,6,481,482,481,1,1,0,0\n"
    #define     STREAM_NIM_HELP_PART40        "           DVB-C, Example,input: %s 0,1,49800,6875,2001,2002,2001,1,2,0\n"
	#define     STREAM_NIM_HELP_PART41        "If you want to play with audio description, first configure audio description pid with ad_set command, then use play command\n"
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART1, item);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART3);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART4);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART5);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART6);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART7);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART8);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART9);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART10);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART11);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART12);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART13);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART14);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART15);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART16);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART17);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART18);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART19);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART20);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART21);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART22);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART23);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART24);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART25);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART26);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART27);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART28);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART29);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART30);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART31);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART32);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART33);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART34, item);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART35, item);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART36, item);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART37, item);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART38, item);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART39, item);
    AUI_PRINTF("\r\n\t      "STREAM_NIM_HELP_PART40, item);
    aui_print_help_instruction_newline(STREAM_NIM_HELP_PART41);
}

unsigned long test_stream_help(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_print_help_header("\nStream Test Help");

    /* STREAM_1_HELP */
    #define     STREAM_1_HELP_PART1         "Play the stream from the NIM. The NIM driver supports DVB-S/S2, DVB-C and DVB-T systems."

    aui_print_help_command("\'1\'");
    aui_print_help_instruction_newline("NIM stream test\n");
    test_stream_with_nim_help("1");

    /* STREAM_2_HELP */
    #define     STREAM_2_HELP_PART1         "Play the stream from the NIM with DSC function. The NIM driver supports DVB-S/S2, DVB-C and DVB-T systems."
    #define     STREAM_2_HELP_PART2         "The inputting parameter of the command is the same as the \"1\" command except the first one which needs \"2\" instead of  \"1\"."
    #define     STREAM_2_HELP_PART3         "Please see the help of the \"1\" command for more details for the inputting parameter\n "
    aui_print_help_command("\'2\'");
    aui_print_help_instruction_newline(STREAM_2_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_2_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_2_HELP_PART3);

    /* STREAM_3_HELP */
    #define     STREAM_3_HELP_PART1         "Play the stream from the NIM with DSC function and KL function. The NIM driver supports DVB-S/S2, DVB-C and DVB-T systems."
    #define     STREAM_3_HELP_PART2         "The inputting parameter of the command is the same as the \"1\" command except the first one which needs \"3\" instead of  \"1\"."
    #define     STREAM_3_HELP_PART3         "Please see the help of the \"1\" command for more details for the inputting parameter\n"
    aui_print_help_command("\'3\'");
    aui_print_help_instruction_newline(STREAM_3_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_3_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_3_HELP_PART3);

    /* STREAM_4_HELP */
    #define     STREAM_4_HELP_PART1         "Play the TS stream from storage medium."
    #define     STREAM_4_HELP_PART2         "Format:    4   [path],[vpid],[apid],[ppid],[video format],[audio format],[change_mode],[tsg clk sel]"
    #define     STREAM_4_HELP_PART3         "               [path]: the stream file path."
    #define     STREAM_4_HELP_PART4         "               [vpid]: video pid"
    #define     STREAM_4_HELP_PART5         "               [apid]: audio pid"
    #define     STREAM_4_HELP_PART6         "               [ppid]: pcr pid"
    #define     STREAM_4_HELP_PART7         "               [video format]:0: MPEG    1: AVC(H.264) 10: HEVC(H.265)"
    #define     STREAM_4_HELP_PART8         "               [audio format]:0:  AUI_DECA_STREAM_TYPE_MPEG1"
    #define     STREAM_4_HELP_PART9         "                              1:  AUI_DECA_STREAM_TYPE_MPEG2"
    #define     STREAM_4_HELP_PART10        "                              2:  AUI_DECA_STREAM_TYPE_AAC_LATM"
    #define     STREAM_4_HELP_PART11        "                              3:  AUI_DECA_STREAM_TYPE_AC3"
    #define     STREAM_4_HELP_PART12        "                              4:  AUI_DECA_STREAM_TYPE_DTS"
    #define     STREAM_4_HELP_PART13        "                              5:  AUI_DECA_STREAM_TYPE_PPCM"
    #define     STREAM_4_HELP_PART14        "                              6:  AUI_DECA_STREAM_TYPE_LPCM_V"
    #define     STREAM_4_HELP_PART15        "                              7:  AUI_DECA_STREAM_TYPE_LPCM_A"
    #define     STREAM_4_HELP_PART16        "                              8:  AUI_DECA_STREAM_TYPE_PCM"
    #define     STREAM_4_HELP_PART17        "                              9:  AUI_DECA_STREAM_TYPE_WMA"
    #define     STREAM_4_HELP_PART18        "                              10: AUI_DECA_STREAM_TYPE_RA8"
    #define     STREAM_4_HELP_PART19        "                              11: AUI_DECA_STREAM_TYPE_MP3"
    #define     STREAM_4_HELP_PART20        "                              12: AUI_DECA_STREAM_TYPE_AAC_ADTS"
    #define     STREAM_4_HELP_PART21        "                              13: AUI_DECA_STREAM_TYPE_OGG"
    #define     STREAM_4_HELP_PART22        "                              14: AUI_DECA_STREAM_TYPE_EC3"
    #define     STREAM_4_HELP_PART23        "              [change_mode]: 0: API_CHG_BLACK(show black screen when change program) 1: API_CHG_STILL(show the last picture of the last program on screen before the new program begins to play)"
    #define     STREAM_4_HELP_PART24        "              [tsg clk sel]: integer > 0, tsg clock select, when the stream with high bitrate can not play smoothly, decrease the select value, default 8"
    #define     STREAM_4_HELP_PART25        "Example:   If the stream file storing the U disk that the file path is /mnt/uda1/tvstream.ts whose vpid is 234, apid is 235, "
    #define     STREAM_4_HELP_PART26        "           ppid is 234 is played, the video format is MPEG, the audio format is MPEG2, the dis format is high definition is played, the input is:"
    #define     STREAM_4_HELP_PART27        "           4 /mnt/uda1/tvstream.ts,234,235,234,0,1,0\n"
    aui_print_help_command("\'4\'");
    aui_print_help_instruction_newline(STREAM_4_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART3);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART4);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART5);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART6);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART7);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART8);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART9);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART10);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART11);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART12);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART13);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART14);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART15);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART16);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART17);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART18);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART19);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART20);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART21);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART22);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART23);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART24);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART25);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART26);
    aui_print_help_instruction_newline(STREAM_4_HELP_PART27);

    /* STREAM_5_HELP */
    #define     STREAM_5_HELP_PART1         "Play the TS stream with DSC function from storage medium."
    #define     STREAM_5_HELP_PART2         "The inputting parameter of the command is the same as the \"5\" command except the first one which needs \"5\" instead of  \"4\"."
    #define     STREAM_5_HELP_PART3         "Please see the help of the \"4\" command for more details for the inputting parameter\n"

    aui_print_help_command("\'5\'");
    aui_print_help_instruction_newline(STREAM_5_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_5_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_5_HELP_PART3);

    /* STREAM_6_HELP */
    #define     STREAM_6_HELP_PART1         "Play the TS stream with DSC function and KL function from storage medium."
    #define     STREAM_6_HELP_PART2         "The inputting parameter of the command is the same as the \"4\" command except the first one which needs \"6\" instead of \"4\"."
    #define     STREAM_6_HELP_PART3         "Please see the help of the \"4\" command for more details for the inputting parameter\n"
    aui_print_help_command("\'6\'");
    aui_print_help_instruction_newline(STREAM_6_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_6_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_6_HELP_PART3);

    /* STREAM_7HELP */
    #define     STREAM_7_HELP_PART1         "Play the clear stream that supports DVB-S/S2, DVB-C and DVB-T format."

    aui_print_help_command("\'play\'");
    aui_print_help_instruction_newline(STREAM_7_HELP_PART1);
    test_stream_with_nim_help("play");

    /* STREAM_8_HELP */
    #define     STREAM_8_HELP_PART1         "Stop the playing stream.\n"
    aui_print_help_command("\'stop\'");
    aui_print_help_instruction_newline(STREAM_8_HELP_PART1);

    /* STREAM_9_HELP */
    #define     STREAM_9_HELP_PART1       "no sound or no video pictures when play stream."
    #define     STREAM_9_HELP_PART2       "Format:    modeset   [index]"
    #define     STREAM_9_HELP_PART3       "                     [index]:0:have sound and video picture when playing stream"
    #define     STREAM_9_HELP_PART4       "                             1:have sound and no video picture when playing stream"
    #define     STREAM_9_HELP_PART5       "                             2:have video picture and no sound when playing stream"

    aui_print_help_command("\'modeset\'");
    aui_print_help_instruction_newline(STREAM_9_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_9_HELP_PART2);
    aui_print_help_instruction_newline(STREAM_9_HELP_PART3);
    aui_print_help_instruction_newline(STREAM_9_HELP_PART4);
    aui_print_help_instruction_newline(STREAM_9_HELP_PART5);

    /* STREAM_10_HELP */
    #define     STREAM_10_HELP_PART1       "play HOSTKEY encrypt stream"
    #define     STREAM_10_HELP_PART2       "Format:    playhk"

    aui_print_help_command("\'playhk\'");
    aui_print_help_instruction_newline(STREAM_10_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_10_HELP_PART2);

    /* STREAM_11_HELP */
    #define     STREAM_11_HELP_PART1       "play CONTENT KEY encrypt stream"
    #define     STREAM_11_HELP_PART2       "Format:    playck"

    aui_print_help_command("\'playck\'");
    aui_print_help_instruction_newline(STREAM_11_HELP_PART1);
    aui_print_help_instruction_newline(STREAM_11_HELP_PART2);

    return AUI_RTN_HELP;

}

/*====================================================*/
/*===============End of add===========================*/
/*====================================================*/

void test_live_reg()
{
    aui_tu_reg_group("stream", "streaming");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_nim, "stream with nim");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_nim_dsc, "stream with nim and dsc");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_nim_dsc_kl, "stream with nim, dsc and kl");
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_tsg, "stream with tsg, only for ALi internal test.");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_tsg_dsc, "stream with tsg and dsc, only for ALi internal test.");
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_tsg_dsc_kl, "stream with tsg, dsc and kl, only for ALi internal test.");
    aui_tu_reg_item(2, "play", AUI_CMD_TYPE_API, test_aui_av_live_play_clear_stream, "play clear stream");
    aui_tu_reg_item(2, "stop", AUI_CMD_TYPE_API, test_aui_av_live_stop_clear_stream, "stop play clear stream");
    aui_tu_reg_item(2, "modeset", AUI_CMD_TYPE_API, test_aui_live_play_mode_set, "no sound or no video pictures when play stream");
    aui_tu_reg_item(2, "playhk", AUI_CMD_TYPE_API, test_aui_av_live_play_encrypt_stream, "play HOSTKEY encrypt stream");
    aui_tu_reg_item(2, "playck", AUI_CMD_TYPE_API, test_aui_av_live_play_contentkey_stream, "play CONTENT KEY encrypt stream");
    aui_tu_reg_item(2, "audio", AUI_CMD_TYPE_API, test_aui_change_audio_track, "change audio track");
    aui_tu_reg_item(2, "scan_plp", AUI_CMD_TYPE_API, test_aui_nim_scan_plp, "Scan DVB-T2 DATA PLP ID");
    aui_tu_reg_item(2, "ad_set", AUI_CMD_TYPE_API, test_aui_cfg_ad_pid, "configure audio description pid");
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_stream_help, "stream test help");
}
