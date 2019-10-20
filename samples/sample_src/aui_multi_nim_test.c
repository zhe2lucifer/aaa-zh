#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aui_ini_config.h"

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <aui_kl.h>
#include <aui_dsc.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_test_stream.h"
#include "aui_multi_nim_test.h"
#include "aui_nim_init.h"

#define LOG_PREFIX_MNTEST "[mn_test]"

#define MAX_TSI_CNT 4
#define NB_CHANNELS 3

// Channel info + corresponding NIM+DMX handlers.
typedef struct st_channel_info {
    char                          *name;
    unsigned long                  nim_index;
    unsigned long                  dmx_index;
    enum aui_decv_format         video_type;
    enum aui_deca_stream_type  audio_type;
    unsigned long                  frequence;
    unsigned long                  symrate;
    unsigned long                  video_pid;
    unsigned long                  audio_pid;
    unsigned long                  pcr_pid;
    aui_hdl                        nim_hdl;
    aui_hdl                        dmx_hdl;
    aui_hdl                        dmx_chan_hdl;
    aui_hdl                        dmx_filter_hdl;
    unsigned long                  record_buffer_size;
    void                          *record_buffer;
    int                            is_recorded;
} channel_attr_info;
channel_attr_info channel_info[NB_CHANNELS] = {
                                                {"Astra Eins festival SD",                         // Name
                                                 1,                                                // NIM index
                                                 AUI_DMX_ID_DEMUX1,                                // DMX index
                                                 AUI_DECV_FORMAT_MPEG, AUI_DECA_STREAM_TYPE_MPEG2, // Audio + video types
                                                 993, 22000,                                       // Freq + symbol rate
                                                 201, 202, 201,                                    // video + audio + pcr PIDs
                                                 NULL, NULL, NULL, NULL,                           // NIM + DMX + DMXchan + DMXfilter handlers
                                                 0, NULL, -1},                                     // Record buffer size + pointer, and record check.
                                                {"Astra ZDF HD - H.264",
                                                 2,
                                                 AUI_DMX_ID_DEMUX2,
                                                 AUI_DECV_FORMAT_AVC, AUI_DECA_STREAM_TYPE_MPEG2,
                                                 1611, 22000,
                                                 6110, 6120, 6110,
                                                 NULL, NULL, NULL, NULL,
                                                 0, NULL, -1},
                                                {"Astra ZDF_neo HD - H.264",
                                                 3,
                                                 AUI_DMX_ID_DEMUX3,
                                                 AUI_DECV_FORMAT_AVC, AUI_DECA_STREAM_TYPE_MPEG2,
                                                 1611, 22000,
                                                 6310, 6320, 6310,
                                                 NULL, NULL, NULL, NULL,
                                                 0, NULL, -1}
                                               };

// General modules handlers (DIS, DECV, DECA, SND, TSI).
// For the purpose of this example, common handlers are simply stored as global variables.
// However it is also possible to get them with aui_find_dev_by_idx()
static aui_hdl dis_hdl_HD = 0;
static aui_hdl dis_hdl_SD = 0;
static aui_hdl decv_hdl   = 0;
static aui_hdl deca_hdl   = 0;
static aui_hdl snd_hdl    = 0;
static aui_hdl tsi_hdl    = 0;

// Init module DISPLAY (SD and HD).
static int init_dis()
{
    aui_attr_dis attr_dis;
    memset(&attr_dis, 0, sizeof(attr_dis));

    printf("%s Init DISPLAY...\n", LOG_PREFIX_MNTEST);

    // Open DISPLAY #0 (HD support).
    if (aui_dis_open(&attr_dis, &dis_hdl_HD) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_open #0 (HD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if (aui_dis_video_enable(dis_hdl_HD, 0) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_video_enable #0 (HD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Open DISPLAY #1 (SD support).
    if (aui_dis_open(&attr_dis, &dis_hdl_SD) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_open #1 (SD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if (aui_dis_video_enable(dis_hdl_SD, 0) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_video_enable #1 (SD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Release module DISPLAY (SD and HD).
static int deinit_dis()
{
    printf("%s Deinit DISPLAY...\n", LOG_PREFIX_MNTEST);

    // Close DISPLAY #0 (HD support).
    if (aui_dis_video_enable(dis_hdl_HD, 0) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_video_enable #0 (HD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if(aui_dis_close(NULL, &dis_hdl_HD) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_close #0 (HD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Close DISPLAY #1 (SD support).
    if (aui_dis_video_enable(dis_hdl_SD, 0) != AUI_RTN_SUCCESS) {
        printf("error%s aui_dis_video_enable #1 (SD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if(aui_dis_close(NULL, &dis_hdl_SD) != AUI_RTN_SUCCESS) {
        printf("%s aui_dis_close #1 (SD) fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Init module VIDEO DECODER.
static int init_decv()
{
    aui_attr_decv attr_decv;
    memset(&attr_decv, 0, sizeof(attr_decv));

    printf("%s Init DECV...\n", LOG_PREFIX_MNTEST);

    // Open the VIDEO decoder.
    if (aui_decv_open(&attr_decv, &decv_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_open fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Set the video frame freeze when channel change.
    if (aui_decv_chg_mode_set(decv_hdl, AUI_DECV_CHG_STILL) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_chg_mode_set fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Release module VIDEO DECODER.
static int deinit_decv()
{
    printf("%s Deinit DECV...\n", LOG_PREFIX_MNTEST);

    // Close the VIDEO decoder.
    if (aui_decv_close(NULL, &decv_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_close fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Init module AUDIO DECODER.
static int init_deca()
{
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;
    memset(&attr_deca, 0, sizeof(attr_deca));
    memset(&attr_snd, 0, sizeof(attr_snd));

    printf("%s Init DECA...\n", LOG_PREFIX_MNTEST);

    // Open the AUDIO decoder.
    if (aui_deca_open(&attr_deca, &deca_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_deca_open fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Open the SOUND module.
    if (aui_snd_open(&attr_snd, &snd_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_snd_open fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Unmute sound.
    if (aui_snd_mute_set(snd_hdl, 0) != AUI_RTN_SUCCESS) {
        printf("%s aui_snd_mute_set fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Set volume to 80%.
    if (aui_snd_vol_set(snd_hdl, 80) != AUI_RTN_SUCCESS) {
        printf("%s aui_snd_vol_set fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Release module AUDIO DECODER.
static int deinit_deca()
{
    aui_attr_deca attr_deca;
    memset(&attr_deca, 0, sizeof(attr_deca));

    printf("%s Deinit DECA...\n", LOG_PREFIX_MNTEST);

    // Close the AUDIO decoder.
    if (aui_deca_close(deca_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_deca_close fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Close the SND module.
    if (aui_snd_close(snd_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_snd_close fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Init NET INTERFACE MODULE.
static int init_nim()
{
    printf("%s Init NIM...\n", LOG_PREFIX_MNTEST);

    // Init NIM module.
    if (aui_nim_init(nim_init_cb) != AUI_RTN_SUCCESS) {
        printf("%s aui_nim_init fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Release NET INTERFACE MODULE.
static int deinit_nim()
{
    printf("%s Deinit NIM...\n", LOG_PREFIX_MNTEST);

    // Clear the NIM module.
    if (aui_nim_de_init(NULL) != AUI_RTN_SUCCESS) {
        printf("%s aui_nim_de_init fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Init module DEMUX.
static int init_dmx()
{
    printf("%s Init DMX...\n", LOG_PREFIX_MNTEST);

    // Init DMX module.
    if (aui_dmx_init(NULL, NULL) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_init fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;

}

// Release module DEMUX.
static int deinit_dmx()
{
    printf("%s Deinit DMX...\n", LOG_PREFIX_MNTEST);

    // Clear the DMX module.
    if (aui_dmx_de_init(NULL, NULL) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_de_init fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Init module TRANSPORT STREAM INTERFACE.
static int init_tsi()
{
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
    int i = 0;
    aui_attr_tsi attr_tsi;
    enum aui_tsi_output_id dmx_id[MAX_TSI_CNT] = {AUI_TSI_OUTPUT_DMX_0,
                                                  AUI_TSI_OUTPUT_DMX_1,
                                                  AUI_TSI_OUTPUT_DMX_2,
                                                  AUI_TSI_OUTPUT_DMX_3};
    enum aui_tsi_channel_id tsi_channel_id[MAX_TSI_CNT] = {AUI_TSI_CHANNEL_0,
                                                           AUI_TSI_CHANNEL_1,
                                                           AUI_TSI_CHANNEL_2,
                                                           AUI_TSI_CHANNEL_3};

    printf("%s Init TSI...\n", LOG_PREFIX_MNTEST);

    // Init the TSI module.
    if (aui_tsi_open(&tsi_hdl) != AUI_RTN_SUCCESS) {
        printf("%s init_tsi fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Config route for each TSI.
    memset(tsi_cfg, 0, sizeof(tsi_cfg));
    aui_tsi_config(tsi_cfg);
    for (i = 0; i < MAX_TSI_CNT; i++) {
        if (tsi_cfg[i].ul_input_src < AUI_TSI_INPUT_MAX) {
            memset(&attr_tsi, 0, sizeof(attr_tsi));
            attr_tsi.ul_init_param = tsi_cfg[i].ul_hw_init_val;
            if (aui_tsi_src_init(tsi_hdl, tsi_cfg[i].ul_input_src, &attr_tsi) != AUI_RTN_SUCCESS) {
                printf("%s aui_tsi_src_init[%d] fail\n", LOG_PREFIX_MNTEST, i);
                return -1;
            }
            if (aui_tsi_route_cfg(tsi_hdl, tsi_cfg[i].ul_input_src,
                         tsi_channel_id[i],
                         dmx_id[i]) != AUI_RTN_SUCCESS) {
                printf("%s aui_tsi_route_cfg[%d] fail", LOG_PREFIX_MNTEST, i);
                return -1;
            }
        }
    }

    return 0;
}

// Release module TRANSPORT STREAM INTERFACE.
static int deinit_tsi()
{
    // Clear the TSI module.
    if (aui_tsi_close(tsi_hdl) != AUI_RTN_SUCCESS) {
        printf("%s deinit_tsi fail\n", LOG_PREFIX_MNTEST);
        return 1;
    }
    return 0;
}

// Init all modules necessary for live channel display.
static int init_platform()
{
    if (init_dis() != 0) {
        return -1;
    }
    if (init_decv() != 0) {
        return -1;
    }
    if (init_deca() != 0) {
        return -1;
    }
    if (init_nim() != 0) {
        return -1;
    }
    if (init_dmx() != 0) {
        return -1;
    }
    if (init_tsi() != 0) {
        return -1;
    }

    return 0;
}

// Clear all the modules.
static int deinit_platform()
{
    if (deinit_dis() != 0) {
        return -1;
    }
    if (deinit_decv() != 0) {
        return -1;
    }
    if (deinit_deca() != 0) {
        return -1;
    }
    if (deinit_nim() != 0) {
        return -1;
    }
    if (deinit_dmx() != 0) {
        return -1;
    }
    if (deinit_tsi() != 0) {
        return -1;
    }

    return 0;
}

// Tune on a channel: open and configure NIM and DMX.
static int tune_on_channel(int channel_idx)
{
    // Open NIM.
    aui_nim_attr nim_attr;
    printf("%s Open NIM #%d (%s)...\n", LOG_PREFIX_MNTEST,
           channel_info[channel_idx].nim_index, channel_info[channel_idx].name);
    memset(&nim_attr, 0, sizeof(nim_attr));
    nim_attr.ul_nim_id = channel_info[channel_idx].nim_index;
    nim_attr.en_dmod_type = AUI_NIM_QPSK;
    if (aui_nim_open(&nim_attr, &(channel_info[channel_idx].nim_hdl)) != AUI_RTN_SUCCESS) {
        printf("%s aui_nim_open [#%d] fail\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].nim_index);
        return -1;
    }

    // Lock on frequency.
    struct aui_nim_connect_param param;
    printf("%s NIM: lock frequency (%s)...\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].name);
    memset(&param, 0, sizeof(struct aui_nim_connect_param));
    param.ul_freq = channel_info[channel_idx].frequence;
    param.en_demod_type = AUI_NIM_QPSK;
    param.connect_param.sat.ul_freq_band = AUI_NIM_LOW_BAND;
    param.connect_param.sat.ul_symrate = channel_info[channel_idx].symrate;
    param.connect_param.sat.fec = AUI_NIM_FEC_AUTO;
    param.connect_param.sat.ul_polar = AUI_NIM_POLAR_HORIZONTAL;
    if (aui_nim_connect(channel_info[channel_idx].nim_hdl, &param) != AUI_RTN_SUCCESS) {
        printf("%s aui_nim_connect [#%d] fail\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].nim_index);
        return -1;
    }

    // For the purpose of the test, wait 1 sec and check we lock properly the frequency.
    sleep(1);
    int lock_status = AUI_NIM_STATUS_UNKNOWN;
    if (aui_nim_is_lock(channel_info[channel_idx].nim_hdl, &lock_status)) {
        printf("%s is_lock [#%d] fail\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].nim_index);
        aui_nim_disconnect(channel_info[channel_idx].nim_hdl);
        return -1;
    }
    if (lock_status != AUI_NIM_STATUS_LOCKED) {
        printf("%s ERROR: NIM is not locked against %s\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].name);
        return -1;
    }

    // Open DMX.
    aui_attr_dmx attr_dmx;
    printf("%s Open DMX #%d (%s)...\n", LOG_PREFIX_MNTEST,
           channel_info[channel_idx].dmx_index, channel_info[channel_idx].name);
    memset(&attr_dmx, 0, sizeof(attr_dmx));
    attr_dmx.uc_dev_idx = channel_info[channel_idx].dmx_index;
    if (aui_dmx_open(&attr_dmx, &channel_info[channel_idx].dmx_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_open [#%d] fail\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index);
        return -1;
    }
    if (aui_dmx_start(channel_info[channel_idx].dmx_hdl, &attr_dmx) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_start [#%d] fail\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index);
        return -1;
    }

    return 0;
}

// Close all channels (close NIM and DMX).
static int close_all_channels()
{
    int i = 0;

    // Close all NIMs and DMXs.
    for (i = 0; i < NB_CHANNELS; i++) {
        printf("%s Close NIM #%d [%s]...\n", LOG_PREFIX_MNTEST, channel_info[i].nim_index, channel_info[i].name);
        if (channel_info[i].nim_hdl != 0) {
            if (aui_nim_close(channel_info[i].nim_hdl) != AUI_RTN_SUCCESS) {
                printf("%s aui_nim_close [%d] fail\n", LOG_PREFIX_MNTEST, channel_info[i].nim_index);
                return -1;
            }
            channel_info[i].nim_hdl = 0;
        }

        // Stop DMX.
        printf("%s Close DMX #%d (%s)...\n", LOG_PREFIX_MNTEST, channel_info[i].nim_index, channel_info[i].name);
        if (aui_dmx_stop(channel_info[i].dmx_hdl, NULL) != AUI_RTN_SUCCESS) {
            printf("%s aui_dmx_stop fail\n", LOG_PREFIX_MNTEST);
            return -1;
        }
        if (channel_info[i].dmx_hdl != 0) {
            if (aui_dmx_close(channel_info[i].dmx_hdl) != AUI_RTN_SUCCESS) {
                printf("%s aui_dmx_close [%d] fail\n", LOG_PREFIX_MNTEST, channel_info[i].nim_index);
                return -1;
            }
            channel_info[i].dmx_hdl = 0;
        }
    }

    return 0;
}

// Create an audio/video playback task before showing a channel (= set the DMX data path for playing clear content).
static int create_playback_task(int channel_idx)
{
    aui_dmx_stream_pid pid_list;
    aui_dmx_data_path path;

    // Create a DMX Audio/Video task.
    printf("%s Create an Audio/Video task on DMX #%d (%s)...\n",
           LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index, channel_info[channel_idx].name);
    memset(&pid_list, 0x0, sizeof(pid_list));
    pid_list.ul_pid_cnt = 3;
    pid_list.stream_types[0] = AUI_DMX_STREAM_VIDEO;
    pid_list.stream_types[1] = AUI_DMX_STREAM_AUDIO;
    pid_list.stream_types[2] = AUI_DMX_STREAM_PCR;
    pid_list.aus_pids_val[0] = channel_info[channel_idx].video_pid;
    pid_list.aus_pids_val[1] = channel_info[channel_idx].audio_pid;
    pid_list.aus_pids_val[2] = channel_info[channel_idx].pcr_pid;
    if (aui_dmx_set(channel_info[channel_idx].dmx_hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_set [#%d] AUI_DMX_STREAM_CREATE_AV fail\n",
               LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index);
        return -1;
    }
    memset(&path, 0, sizeof(path));
    path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    if (aui_dmx_data_path_set(channel_info[channel_idx].dmx_hdl, &path) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_data_path_set [#%d] AUI_DMX_DATA_PATH_CLEAR_PLAY failed\n",
               LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index);
        return -1;
    }
    if (aui_dmx_set(channel_info[channel_idx].dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,
                    (void *)AUI_AVSYNC_FROM_TUNER) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_set [#%d] AUI_DMX_SET_AVSYNC_MODE fail\n",
               LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index);
        return -1;
    }

	return 0;
}

// Start the video and audio decoders.
static int start_audio_video_dec(int channel_idx)
{
    aui_attr_deca attr_deca;

    printf("%s Start Audio/Video decoder for %s...\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].name);

    // Start video decoder DECV.
    if (aui_decv_decode_format_set(decv_hdl, channel_info[channel_idx].video_type) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_decode_format_set fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if (aui_decv_start(decv_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_start fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if (aui_decv_sync_mode(decv_hdl, AUI_STC_AVSYNC_PCR) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_sync_mode fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Start audio decoder DECA.
    if (aui_deca_type_set(deca_hdl, channel_info[channel_idx].audio_type) != AUI_RTN_SUCCESS) {
        printf("%s aui_deca_type_set fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if (aui_deca_sync_mode_set(deca_hdl, AUI_DECA_DEOCDER_SYNC) != AUI_RTN_SUCCESS) {
        printf("%s aui_deca_sync_mode_set fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    memset(&attr_deca, 0, sizeof(attr_deca));
    if (aui_deca_start(deca_hdl, &attr_deca) != AUI_RTN_SUCCESS) {
        printf("%s aui_deca_start fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

	return 0;
}

// Stop the video and audio decoders.
static int stop_audio_video_decs()
{
    printf("%s Stop Audio/Video decoders...\n", LOG_PREFIX_MNTEST);

    // Stop audio decoder DECA.
    if (aui_deca_stop(deca_hdl, NULL) != AUI_RTN_SUCCESS) {
        printf("%s aui_deca_stop fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Stop video decoder DECV.
    if (aui_decv_stop(decv_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_decv_stop fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

	return 0;
}

// Show a specific channel on the screen (actually enable the "stream AV" task for the channel).
static int show_channel(int channel_idx)
{
    printf("%s Show channel %s...\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].name);

    // Enable Audio/Video DMX task.
    if (aui_dmx_set(channel_info[channel_idx].dmx_hdl, AUI_DMX_STREAM_ENABLE, 0) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Remove channel from screen (disable the "stream AV" task).
static int hide_channel(int channel_idx)
{
    int i = 0;

    printf("%s Hide channel %s...\n", LOG_PREFIX_MNTEST, channel_info[channel_idx].name);

    // Disable all Audio/Video DMX tasks.
    if (aui_dmx_set(channel_info[channel_idx].dmx_hdl, AUI_DMX_STREAM_DISABLE, 0) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_set AUI_DMX_STREAM_DISABLE fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// This callback is called by the system: it request that the application return
// a buffer which will be filled with recorded data.
static long request_record_buffer_callback(void *filter_handle,
                                           unsigned long ul_req_size,
                                           void **pp_req_buf,
                                           unsigned long *req_buf_size,
                                           struct aui_avsync_ctrl *pst_ctrl_blk)
{
    (void)pst_ctrl_blk;

    if (filter_handle != NULL) {
        // Get user data if needed.
        aui_attr_dmx_filter *filter_attr = NULL;
        if (aui_dmx_filter_get(filter_handle, AUI_DMX_FILTER_ATTR_GET, &filter_attr) != AUI_RTN_SUCCESS) {
            printf("%s aui_dmx_filter_get fail\n", LOG_PREFIX_MNTEST);
            return -1;
        }

        if ((filter_attr != NULL) && (filter_attr->usr_data != NULL)) {
            channel_attr_info *chan_info = (channel_attr_info *)(filter_attr->usr_data);

            // Allocate the application buffer.
            if (chan_info->record_buffer == NULL)
            {
                printf("%s Requesting a buffer to record data from %s...\n", LOG_PREFIX_MNTEST, chan_info->name);
                chan_info->record_buffer = malloc(ul_req_size);
                chan_info->record_buffer_size = ul_req_size;
            }

            // Return adress and size of the record data buffer.
            *pp_req_buf   = chan_info->record_buffer;
            *req_buf_size = chan_info->record_buffer_size;

        } else {
            printf("%s ERROR: can not get handle in request callback\n", LOG_PREFIX_MNTEST);
            return -1;
        }
    } else {
        printf("%s ERROR: bad dmx filter handle\n", LOG_PREFIX_MNTEST);
    }

    return 0;
}

// This callback is called by the system after when new recorded data are ready
// in the buffer given by the application in request_record_buffer_callback().
static long record_buffer_updated_callback(void *filter_handle, unsigned long ul_size)
{
    // Get user data if needed.
    aui_attr_dmx_filter *filter_attr = NULL;
    aui_dmx_filter_get(filter_handle, AUI_DMX_FILTER_ATTR_GET, &filter_attr);
    channel_attr_info *chan_info = (channel_attr_info *)(filter_attr->usr_data);

    // Consume the buffer containing recorded data.
    printf("%s => Received %d bytes of recorded data from %s\n", LOG_PREFIX_MNTEST, ul_size, chan_info->name);
    chan_info->is_recorded = 1;

    return 0;
}

// Record a specific channel (= set the data path for recording and open a DMX channel+filter).
static int record_channel(int channel_idx)
{
    // Reset the recording check.
    channel_info[channel_idx].is_recorded = -1;

    // We want to record data incoming into the demux.
    aui_dmx_data_path path;
    memset(&path, 0, sizeof(path));
    path.data_path_type = AUI_DMX_DATA_PATH_REC;
    if (aui_dmx_data_path_set(channel_info[channel_idx].dmx_hdl, &path) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_data_path_set [#%d] AUI_DMX_DATA_PATH_REC failed\n",
               LOG_PREFIX_MNTEST, channel_info[channel_idx].dmx_index);
        return -1;
    }

    // Open DMX channel for recording.
    aui_attr_dmx_channel attr_channel;
    printf("%s Open channel on DMX #%d for record (%s)...\n", LOG_PREFIX_MNTEST,
           channel_info[channel_idx].dmx_index, channel_info[channel_idx].name);
    memset(&attr_channel, 0, sizeof(attr_channel));
    attr_channel.us_pid = AUI_INVALID_PID;
    attr_channel.ul_pid_cnt = 3;
    attr_channel.us_pid_list[0] = channel_info[channel_idx].video_pid;
    attr_channel.us_pid_list[1] = channel_info[channel_idx].audio_pid;
    attr_channel.us_pid_list[2] = channel_info[channel_idx].pcr_pid;
    attr_channel.dmx_data_type = AUI_DMX_DATA_REC;
    if (aui_dmx_channel_open(channel_info[channel_idx].dmx_hdl, &attr_channel,
                             &channel_info[channel_idx].dmx_chan_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_channel_open fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Open a DMX filter.
    aui_attr_dmx_filter attr_filter;
    printf("%s Open filter on DMX #%d (%s)...\n", LOG_PREFIX_MNTEST,
           channel_info[channel_idx].dmx_index, channel_info[channel_idx].name);
    memset(&attr_filter, 0, sizeof(attr_filter));
    attr_filter.usr_data = &channel_info[channel_idx];
    attr_filter.p_fun_data_req_wtCB = request_record_buffer_callback;
    attr_filter.p_fun_data_up_wtCB  = record_buffer_updated_callback;
    if (aui_dmx_filter_open(channel_info[channel_idx].dmx_chan_hdl, &attr_filter,
                            &channel_info[channel_idx].dmx_filter_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_filter_open fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Start recording.
    if (aui_dmx_filter_start(channel_info[channel_idx].dmx_filter_hdl, &attr_filter) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_filter_start fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Stop recording channel.
static int stop_record_channel(int channel_idx)
{
    printf("%s Stop recording channel on DMX #%d (%s)...\n", LOG_PREFIX_MNTEST,
           channel_info[channel_idx].dmx_index, channel_info[channel_idx].name);

    // Stop recording (=> stop DMX filter and close it).
    if (aui_dmx_filter_stop(channel_info[channel_idx].dmx_filter_hdl, NULL) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_filter_stop fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }
    if (aui_dmx_filter_close(&channel_info[channel_idx].dmx_filter_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_filter_close fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    // Close the DMX channel.
    if (aui_dmx_channel_close(&channel_info[channel_idx].dmx_chan_hdl) != AUI_RTN_SUCCESS) {
        printf("%s aui_dmx_channel_close fail\n", LOG_PREFIX_MNTEST);
        return -1;
    }

    return 0;
}

// Multi NIM test: tune on hardcoded channels and ask the user to select the one he wants to display on the screen.
static unsigned long test_multi_disp(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned int cmd = 0;
    unsigned short int do_exit = 0;
    int i = 0;
    int res = 0;

    // Init all modules necessary to display live channels (DIS, DECV, DECA, DMX, NIM)
    init_platform();

    // Tune on each channels (1 channel => 1 NIM => DMX) and create an audio/video task.
    for (i = 0; i < NB_CHANNELS; i++) {
        if (tune_on_channel(i) != 0) {
            res = -1;
        }
        create_playback_task(i);
    }

    // Ask the user to select which channel to display (the other one will be hidden).
    if (res == 0) {
        fflush(stdin);
        while (do_exit == 0) {
            printf("----> PLEASE input the channel to display, from 0 to %d: ", NB_CHANNELS - 1);
            if(0 == scanf("%d", &cmd)) {
				printf("scanf failed\n");
				do_exit = 1;
			}
            for (i = 0; i < NB_CHANNELS; i++) {
                hide_channel(i);
            }
            stop_audio_video_decs();
            if (cmd < NB_CHANNELS) {
                start_audio_video_dec(cmd);
                show_channel(cmd);
            }
            else {
                // Nothing to show, quit.
                do_exit = 1;
            }
        }
    }

    // Cleanup.
    close_all_channels();
    deinit_platform();

    return res;
}

// Recording test: tune on hardcoded channels and ask the user to select the one he wants
// to display on the screen and record the others.
static unsigned long test_multi_rec(unsigned long *argc, char **argv, char *sz_out_put)
{
    int cmd = 0;
    unsigned short int do_exit = 0;
    int i = 0;
    int res = 0;

    // Init all modules necessary to display live channels (DIS, DECV, DECA, DMX, NIM)
    init_platform();

    // Tune on each channels (1 channel => 1 NIM => DMX) and create an audio/video task.
    for (i = 0; i < NB_CHANNELS; i++) {
        if (tune_on_channel(i) != 0) {
            res = -1;
        }
    }

    // Ask the user to select which channel to display.
    if (res == 0) {
        fflush(stdin);
        while (do_exit == 0) {
            printf("----> PLEASE input the channel to display, from 0 to %d (the other ones will be recorded): ", NB_CHANNELS - 1);
            if(0 == scanf("%d", &cmd)) {
				printf("scanf failed\n");
				do_exit = 1;
			}
            // Show the selected channel and record the others.
            if (cmd < NB_CHANNELS) {
                for (i = 0; i < NB_CHANNELS; i++) {
                    if (i == cmd) {
                        // Create a playback task for the channel which will be displayed (and show it).
                        create_playback_task(i);
                        start_audio_video_dec(i);
                        show_channel(i);
                    }
                    else {
                        // Open a DMX channel for recording (for the other TV channels).
                        record_channel(i);
                    }
                }
                sleep(1); // Do the recording for 1 second.
                for (i = 0; i < NB_CHANNELS; i++) {
                    if (i == cmd) {
                        // Hide channel that was shown to the TV.
                        hide_channel(i);
                        stop_audio_video_decs();
                    }
                    else {
                        // Stop the recording for all the other channels.
                        stop_record_channel(i);
                        // Check if recording was done.
                        if (channel_info[i].is_recorded == -1) {
                            res = -1;
                        }
                    }
                }

            }
            else {
                // Nothing to show and record, quit.
                do_exit = 1;
            }
        }
    }

    // Cleanup.
    close_all_channels();
    deinit_platform();

    return res;
}

// Test framework: register sub-tests.
void test_multi_nim_reg()
{
    aui_tu_reg_group("mn", "multi nim test");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_multi_disp, "test multi display");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_multi_rec,  "test multi recording");
}

