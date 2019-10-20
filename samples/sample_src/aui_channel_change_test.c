#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include "aui_ini_config.h"
#include <sys/time.h>
#else
#include <api/libfs2/stdio.h>
#endif
#include <string.h>
#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>

#include "aui_channel_change_test.h"
#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_test_stream.h"
#include "aui_nim_init.h"


#define TEST_MODE_MAX 1
#define AVSYNC_MODE_MAX 2
#define CHANNEL_NUM_MAX 4
#define CYCLE_NUM_MAX 100
#define MAX_VALUE_LEN 64
#define MAX_NIM_CNT 4
#define MAX_DMX_CNT 4  /* hardware dmx max count */
#define MAX_TSI_CNT 4
#define CONFIG_FILE_PATH_MAX 255

typedef enum channel_change_time_type {
    CHANNEL_CHANGE_TIME,
    PLAY_AV_TIME,
    NIM_LOCK_FREQ_TIME,
    FIRST_I_FRAME_DECODE_TIME,
    FIRST_I_FRAME_SHOW_TIME,
    STOP_AV_TIME
}channel_change_time_type;

typedef enum channel_change_mode {
    CHANNEL_CHANGE_NORMAL_MODE = 0,
    CHANNEL_CHANGE_FAST_MODE,   
}channel_change_mode;

typedef struct channel_change_config_info {
    enum channel_change_mode ch_ch_mode;
    unsigned int nim_index;
    enum aui_nim_demod_type nim_type;
    unsigned int hwdmx_id;
    enum aui_decv_format video_type;
    enum aui_deca_stream_type audio_type;
    enum aui_nim_qam_mode qam_mode;  /* DVBC */
    unsigned int frequence;
    unsigned int symrate;
    unsigned int video_pid;
    unsigned int audio_pid;
    unsigned int pcr_pid;
    enum aui_nim_polar polar;  /* DVBS */
    unsigned int freq_band;  /* DVBT */
}channel_change_config_info;

typedef struct channel_change_module_handle {
    aui_hdl tsi_hdl;
    aui_hdl decv_hdl;
    aui_hdl deca_hdl;
    aui_hdl snd_hdl;
    aui_hdl nim_hdl[MAX_NIM_CNT];
    aui_hdl dmx_hdl[MAX_DMX_CNT];
#ifdef AUI_LINUX
    aui_hdl dis_hdl_HD;
    aui_hdl dis_hdl_SD;
#endif
}channel_change_module_handle;

typedef enum channel_change_test_mode {
    CHANNEL_CHANGE_ONE_STEP_TEST_MODE = 0,
    CHANNEL_CHANGE_CYCLE_TEST_MODE = 1
}channel_change_test_mode;

typedef struct channel_change_param {
    unsigned int test_mode;
    unsigned int delay_time;
    unsigned int chan_idx;
    unsigned int pre_chan_idx;
    unsigned int test_count;
    unsigned int test_num;
    unsigned int cycle_num;
    unsigned int channel_num;
    unsigned int avsync_mode;
    unsigned int freq_pre;
    unsigned int audio_type_pre;
    unsigned int video_type_pre;
    unsigned int nim_index_pre;
    char key;
    char config_file_path[CONFIG_FILE_PATH_MAX];
}channel_change_param;

typedef struct channel_change_time_point {
    unsigned int start_test_time_point;
    unsigned int play_av_start_time_point;
    unsigned int nim_lock_freq_end_time_point;
    unsigned int play_av_end_time_point;
    unsigned int first_I_frame_decode_time_point;
    unsigned int first_I_frame_show_time_point;
    unsigned int stop_av_start_time_point;
    unsigned int stop_av_end_time_point;
}channel_change_time_point;

typedef struct channel_change_time_interval {
    unsigned int channel_change_time;
    unsigned int play_av_time;
    unsigned int nim_lock_freq_time;
    unsigned int first_I_frame_decode_time;
    unsigned int first_I_frame_show_time;
    unsigned int stop_av_time;
}channel_change_time_interval;


static struct channel_change_config_info change_info[CHANNEL_NUM_MAX] = {{0}};
static struct channel_change_module_handle ch_module_hdl;
static struct channel_change_time_point ch_time_point;
static struct channel_change_time_interval ch_time_interval[CHANNEL_NUM_MAX * CYCLE_NUM_MAX] = {{0}};  /* test ten times */

/* 
 * 0: use config file (Linux), use the default config but do not modify (TDS);
 * 1: usr set all config params;
 * other value: wrong param
 */
static unsigned short g_config_info_flag = 0;

static void first_I_frame_decode_cb(void *p_user_hld, unsigned int paras1, unsigned int paras2);
static void first_I_frame_show_cb(void *p_user_hld, unsigned int paras1, unsigned int paras2);


static enum aui_tsi_output_id hwdmx_id_to_tsi_output_id(enum aui_dmx_id hwdmx_id)
{
    enum aui_tsi_output_id tsi_output_id = -1;
    switch (hwdmx_id) {
        case AUI_DMX_ID_DEMUX0: {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_0;
            break;
        }
        case AUI_DMX_ID_DEMUX1: {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_1;
            break;
        }
        case AUI_DMX_ID_DEMUX2: {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_2;
            break;
        }
        case AUI_DMX_ID_DEMUX3: {
            tsi_output_id = AUI_TSI_OUTPUT_DMX_3;
            break;
        }
        default : {
            AUI_PRINTF("Not support hwdmx_id [%d]\n", hwdmx_id);
            break;
        }
    }
    return tsi_output_id;
}

static unsigned long hwdmx_id_to_index(enum aui_dmx_id hwdmx_id)
{
    unsigned long dmx_index = 0;
    switch(hwdmx_id) {
        case AUI_DMX_ID_DEMUX0: {
            dmx_index = 0;
            break;
        }
        case AUI_DMX_ID_DEMUX1: {
            dmx_index = 1;
            break;
        }
        case AUI_DMX_ID_DEMUX2: {
            dmx_index = 2;
            break;
        }
        case AUI_DMX_ID_DEMUX3: {
            dmx_index = 3;
            break;
        }
        default: {
            dmx_index = 0;
            AUI_PRINTF("hwdmx_id %d is not support, set dmx_index to %ld !\n", hwdmx_id, dmx_index);
            break;
        }
    }

    return dmx_index;
}

/* find max value */
static unsigned int find_max_value(unsigned int num, enum channel_change_time_type time_type)
{
    unsigned int i = 0;
    unsigned int value = 0;
    unsigned int max = 0;

    switch (time_type) {
        case CHANNEL_CHANGE_TIME: {
            max = ch_time_interval[0].channel_change_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].channel_change_time;
                if (value > max) {
                    max = value;
                }
            }
            break;
        }
        case PLAY_AV_TIME: {
            max = ch_time_interval[0].play_av_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].play_av_time;
                if (value > max) {
                    max = value;
                }
            }
            break;
        }
        case NIM_LOCK_FREQ_TIME: {
            max = ch_time_interval[0].nim_lock_freq_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].nim_lock_freq_time;
                if (value > max) {
                    max = value;
                }
            }
            break;
        }
        case FIRST_I_FRAME_DECODE_TIME: {
            max = ch_time_interval[0].first_I_frame_decode_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].first_I_frame_decode_time;
                if (value > max) {
                    max = value;
                }
            }
            break;
        }
        case FIRST_I_FRAME_SHOW_TIME: {
            max = ch_time_interval[0].first_I_frame_show_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].first_I_frame_show_time;
                if (value > max) {
                    max = value;
                }
            }
            break;
        }
        case STOP_AV_TIME: {
            max = ch_time_interval[0].stop_av_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].stop_av_time;
                if (value > max) {
                    max = value;
                }
            }
            break;
        }
        default : {
            AUI_PRINTF("%s %d: time type %d is not support\n", __FUNCTION__, __LINE__, time_type);
            break;
        }
        
    }
    
    return max;
}

/* find min value */
static unsigned int find_min_value(unsigned int num, enum channel_change_time_type time_type)
{
    unsigned int i = 0;
    unsigned int value = 0;
    unsigned int min = 0;

    switch (time_type) {
        case CHANNEL_CHANGE_TIME: {
            min = ch_time_interval[0].channel_change_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].channel_change_time;
                if (value < min) {
                    min = value;
                }
            }
            break;
        }
        case PLAY_AV_TIME: {
            min = ch_time_interval[0].play_av_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].play_av_time;
                if (value < min) {
                    min = value;
                }
            }
            break;
        }
        case NIM_LOCK_FREQ_TIME: {
            min = ch_time_interval[0].nim_lock_freq_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].nim_lock_freq_time;
                if (value < min) {
                    min = value;
                }
            }
            break;
        }
        case FIRST_I_FRAME_DECODE_TIME: {
            min = ch_time_interval[0].first_I_frame_decode_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].first_I_frame_decode_time;
                if (value < min) {
                    min = value;
                }
            }
            break;
        }
        case FIRST_I_FRAME_SHOW_TIME: {
            min = ch_time_interval[0].first_I_frame_show_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].first_I_frame_show_time;
                if (value < min) {
                    min = value;
                }
            }
            break;
        }
        case STOP_AV_TIME: {
            min = ch_time_interval[0].stop_av_time;
            for (i = 0; i < num; i++) {
                value = ch_time_interval[i].stop_av_time;
                if (value < min) {
                    min = value;
                }
            }
            break;
        }
        default : {
            AUI_PRINTF("%s %d: time type %d is not support\n", __FUNCTION__, __LINE__, time_type);
            break;
        }
        
    }
    
    return min;
}


/* find average value */
static unsigned int find_average_value(unsigned int num, enum channel_change_time_type time_type)
{
    unsigned int i = 0;
    unsigned int sum = 0;
    unsigned int average = 0;

    switch (time_type) {
        case CHANNEL_CHANGE_TIME: {
            for (i = 0; i < num; i++) {
                sum += ch_time_interval[i].channel_change_time;
            }
            average = sum / num;
            break;
        }
        case PLAY_AV_TIME: {
            for (i = 0; i < num; i++) {
                sum += ch_time_interval[i].play_av_time;
            }
            average = sum / num;
            break;
        }
        case NIM_LOCK_FREQ_TIME: {
            for (i = 0; i < num; i++) {
                sum += ch_time_interval[i].nim_lock_freq_time;
            }
            average = sum / num;
            break;
        }
        case FIRST_I_FRAME_DECODE_TIME: {
            for (i = 0; i < num; i++) {
                sum += ch_time_interval[i].first_I_frame_decode_time;
            }
            average = sum / num;
            break;
        }
        case FIRST_I_FRAME_SHOW_TIME: {
            for (i = 0; i < num; i++) {
                sum += ch_time_interval[i].first_I_frame_show_time;
            }
            average = sum / num;
            break;
        }
        case STOP_AV_TIME: {
            for (i = 0; i < num; i++) {
                sum += ch_time_interval[i].stop_av_time;
            }
            average = sum / num;
            break;
        }
        default : {
            AUI_PRINTF("%s %d: time type %d is not support\n", __FUNCTION__, __LINE__, time_type);
            break;
        }
        
    }
    
    return average;
}


/* Statistics of average, maximum, minimum time for each phase */
static void statistic_time_info(struct channel_change_param *p_ch_param)
{
    unsigned int test_num = p_ch_param->test_num;

    AUI_PRINTF("\n-----------------------------------------------------------------------\n");
    AUI_PRINTF("time_info (test_num=%-d)         average(ms)    max(ms)     min(ms)    \n", test_num);
    AUI_PRINTF("-----------------------------------------------------------------------\n");
    AUI_PRINTF("[0]channel_change_time           %-4d           %-4d        %-4d       \n",
        find_average_value(test_num, CHANNEL_CHANGE_TIME),
        find_max_value(test_num, CHANNEL_CHANGE_TIME),
        find_min_value(test_num, CHANNEL_CHANGE_TIME));
    AUI_PRINTF("-----------------------------------------------------------------------\n");
    AUI_PRINTF("[1]play_av_time                  %-4d           %-4d        %-4d       \n",
        find_average_value(test_num, PLAY_AV_TIME),
        find_max_value(test_num, PLAY_AV_TIME),
        find_min_value(test_num, PLAY_AV_TIME));
    AUI_PRINTF("[2]nim_lock_freq_time            %-4d           %-4d        %-4d       \n",
        find_average_value(test_num, NIM_LOCK_FREQ_TIME),
        find_max_value(test_num, NIM_LOCK_FREQ_TIME),
        find_min_value(test_num, NIM_LOCK_FREQ_TIME));
    AUI_PRINTF("[3]first_I_frame_decode_time     %-4d           %-4d        %-4d       \n",
        find_average_value(test_num, FIRST_I_FRAME_DECODE_TIME),
        find_max_value(test_num, FIRST_I_FRAME_DECODE_TIME),
        find_min_value(test_num, FIRST_I_FRAME_DECODE_TIME));
    AUI_PRINTF("[4]first_I_frame_show_time       %-4d           %-4d        %-4d       \n",
        find_average_value(test_num, FIRST_I_FRAME_SHOW_TIME),
        find_max_value(test_num, FIRST_I_FRAME_SHOW_TIME),
        find_min_value(test_num, FIRST_I_FRAME_SHOW_TIME));
    AUI_PRINTF("[5]stop_av_time                  %-4d           %-4d        %-4d       \n",
        find_average_value(test_num, STOP_AV_TIME),
        find_max_value(test_num, STOP_AV_TIME),
        find_min_value(test_num, STOP_AV_TIME));
    AUI_PRINTF("-----------------------------------------------------------------------\n");
    
    return;
}

static unsigned int get_time_ms()
{
    unsigned int time_ms = 0;
#ifdef AUI_LINUX
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    gettimeofday(&tv, NULL);
    time_ms = ((tv.tv_sec)*1000000 + tv.tv_usec) / 1000;
#else
    time_ms = osal_get_tick();
#endif
    return time_ms;
}


/*
 * Linux: Get the channel info from channel_change_info.ini
 * TDS : Get the channel info from channel0~channel3
 */
static int init_channel_info(struct channel_change_param *p_ch_param)
{
    unsigned int i = 0;
    unsigned int channel_num = p_ch_param->channel_num;
#ifdef AUI_LINUX
    /*
        * The configuration parameters of TS file to /usr/share/auiconfig/channel_change_info.ini
        * For example:
        *               change_mode,nim_index,nim_type,hwdmx_id,video_type,audio_type,qam_mode,frequence,symrate,video_pid,audio_pid,pcr_pid
        * channel0=0,0,0,0,0,1,3,3840,27500,513,660,8190  // CCTV-2
        * channel1=0,0,0,0,0,1,3,3840,27500,514,670,8190  // CCTV-7
        * channel2=0,0,0,0,0,1,3,3880,27500,516,690,8190  // CCTV-14
        * channel3=0,0,0,0,0,1,3,3880,27500,517,700,8190  // CCTV-13
        */
    char *config_file_path = p_ch_param->config_file_path;
    char fsc_group[32] = {0};
    char value[MAX_VALUE_LEN] = {0};
    char array_name[MAX_VALUE_LEN] = {0};
    unsigned int tmp_change_info[sizeof(struct channel_change_config_info) / 4] = {0};
    int j = 0;
    char *p = NULL;

    AUI_PRINTF("get channel info from %s\n", config_file_path);
    
    memset(change_info, 0, sizeof(struct channel_change_config_info));

    /* check the config file */
    if (0 != access(config_file_path, F_OK)) {
        AUI_PRINTF("ERROR: there is no config file! File path = %s\n", config_file_path);
        return -1;
    }
    if (0 != access(config_file_path, R_OK)) {
        AUI_PRINTF("ERROR: the config file NOT read! File path = %s\n", config_file_path);
        return -1;
    }
    
    strcat(fsc_group, "[channel_array]");
    
    for (i = 0; i < channel_num; i++) { 
        sprintf(array_name, "channel%d", i);
        read_conf_value_ex(array_name, value, fsc_group, MAX_VALUE_LEN, config_file_path);
        j = 0;
        p = strtok(value, ","); 
        while (p) {
            tmp_change_info[j] = atoi(p);
            j++;
            p = strtok(NULL, ","); 
        }
        memcpy(&change_info[i], tmp_change_info, sizeof(struct channel_change_config_info));
    }
#else
    /*
        * default play channel is DVB-S ZX6B
        *               change_mode,nim_index,nim_type,  hwdmx_id,video_type,audio_type,  qam_mode,frequence,symrate,  video_pid,audio_pid,pcr_pid
        * channel0=0,0,0,0,0,1,3,3840,27500,513,660,8190  // CCTV-2
        * channel1=0,0,0,0,0,1,3,3840,27500,514,670,8190  // CCTV-7
        * channel2=0,0,0,0,0,1,3,3880,27500,516,690,8190  // CCTV-14
        * channel3=0,0,0,0,0,1,3,3880,27500,517,700,8190  // CCTV-13
        */
    AUI_PRINTF("Use the default configuration parameters, but we can not modify them directly.\n");
    AUI_PRINTF("If the users want to set these configuration parameters by themselves, they can refer the command '2'.\n");
    static const unsigned long channel0[12] = {0,0,0,0,0,1,3,3840,27500,513,660,8190};
    static const unsigned long channel1[12] = {0,0,0,0,0,1,3,3840,27500,514,670,8190};
    static const unsigned long channel2[12] = {0,0,0,0,0,1,3,3880,27500,516,690,8190};
    static const unsigned long channel3[12] = {0,0,0,0,0,1,3,3880,27500,517,700,8190};
    memcpy(&change_info[0], &channel0[0], sizeof(struct channel_change_config_info));
    memcpy(&change_info[1], &channel1[0], sizeof(struct channel_change_config_info));
    memcpy(&change_info[2], &channel2[0], sizeof(struct channel_change_config_info));
    memcpy(&change_info[3], &channel3[0], sizeof(struct channel_change_config_info));
#endif
    /* print channel info */
    AUI_PRINTF("\nchan_idx=ch_mode,nim_idx,nim_type,hwdmx_id,vtype,atype,qam,freq,symrate,vpid,apid,ppid\n");
    AUI_PRINTF("----------------------------------------------------------------------------------------\n");
    for (i = 0; i < channel_num; i++) {
        AUI_PRINTF("channel%d=%7d,%7d,%8d", i, change_info[i].ch_ch_mode, change_info[i].nim_index, change_info[i].nim_type);
        AUI_PRINTF(",%8d,%5d,%5d", change_info[i].hwdmx_id, change_info[i].video_type, change_info[i].audio_type);
        AUI_PRINTF(",%3d,%4d,%7d", change_info[i].qam_mode, change_info[i].frequence, change_info[i].symrate);
        AUI_PRINTF(",%4d,%4d,%4d\n", change_info[i].video_pid, change_info[i].audio_pid, change_info[i].pcr_pid);
    }
    AUI_PRINTF("\n\n");
    
    return 0;
}

#ifdef AUI_LINUX
static int init_dis()
{
    aui_attr_dis attr_dis;
    MEMSET(&attr_dis, 0, sizeof(aui_attr_dis));

    if(aui_find_dev_by_idx(AUI_MODULE_DIS, 0, &ch_module_hdl.dis_hdl_HD)) {
        if (aui_dis_open(&attr_dis, &ch_module_hdl.dis_hdl_HD)) {
            AUI_PRINTF("\n%s -> aui_dis_open #0 HD fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    if (aui_dis_video_enable(ch_module_hdl.dis_hdl_HD, 0)) {
        AUI_PRINTF("\n%s -> aui_dis_video_enable #0 HD fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if(aui_find_dev_by_idx(AUI_MODULE_DIS, 1, &ch_module_hdl.dis_hdl_SD)) {
        if (aui_dis_open(&attr_dis, &ch_module_hdl.dis_hdl_SD)) {
            AUI_PRINTF("\n%s -> aui_dis_open #1 HD fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    if (aui_dis_video_enable(ch_module_hdl.dis_hdl_SD, 0)) {
        AUI_PRINTF("\n%s -> aui_dis_video_enable #1 HD fail\n", __FUNCTION__);
        goto EXIT_FAIL; 
    }
    return 0;
EXIT_FAIL:
    return -1;
}

static int deinit_dis()
{
    int ret = 0;
    
    if (aui_dis_video_enable(ch_module_hdl.dis_hdl_HD, 0)) {
        AUI_PRINTF("\n%s -> aui_dis_video_enable #0 HD fail\n", __FUNCTION__);
        ret |= -1;
    }
    if(aui_dis_close(NULL,&ch_module_hdl.dis_hdl_HD)) {
        AUI_PRINTF("\n%s -> aui_dis_close #0 HD fail\n", __FUNCTION__);
        ret |= -1;
    }
    if (aui_dis_video_enable(ch_module_hdl.dis_hdl_SD, 0)) {
        AUI_PRINTF("\n%s -> aui_dis_video_enable #1 HD fail\n", __FUNCTION__);
        ret |= -1;
    }
    if(aui_dis_close(NULL,&ch_module_hdl.dis_hdl_SD)) {
        AUI_PRINTF("\n%s -> aui_dis_close #1 HD fail\n", __FUNCTION__);
        ret |= -1;
    }
    
    return ret;
}

static int deinit_nim(struct channel_change_param *p_ch_param)
{
    unsigned int i = 0;
    int ret = 0;
    if (MAX_NIM_CNT < p_ch_param->channel_num) {
        return -1;
    }
    for (i = 0; i < p_ch_param->channel_num; i++) {
        if (0 != ch_module_hdl.nim_hdl[change_info[i].nim_index]) {
            if (aui_nim_close(ch_module_hdl.nim_hdl[change_info[i].nim_index])) {
                AUI_PRINTF("%s -> close nim[%d] fail\n", __FUNCTION__, i);
                ret |= -1;
            }
            ch_module_hdl.nim_hdl[change_info[i].nim_index] = NULL;
        }
    }
    
    if (aui_nim_de_init(NULL)) {
        AUI_PRINTF("%s -> aui_nim_de_init fail\n", __FUNCTION__);
        ret |= -1;
    }
    return ret;
}

static int deinit_tsi()
{
    if(aui_tsi_close(ch_module_hdl.tsi_hdl)) {
        AUI_PRINTF("%s -> aui_tsi_close fail\n", __FUNCTION__);
        return -1;
    }
    return 0;
}
#endif

static int init_decv(struct channel_change_param *p_ch_param)
{
    aui_attr_decv attr_decv;
    aui_decv_callback_node first_I_frame_show_callback;
    aui_decv_callback_node first_I_frame_decode_callback;
    unsigned int avsync_mode = p_ch_param->avsync_mode;

    memset(&ch_module_hdl.decv_hdl, 0, sizeof(ch_module_hdl.decv_hdl));
    memset(&attr_decv, 0, sizeof(aui_attr_decv));
    memset(&first_I_frame_show_callback, 0, sizeof(aui_decv_callback_node));
    memset(&first_I_frame_decode_callback, 0, sizeof(aui_decv_callback_node));

    if (aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &ch_module_hdl.decv_hdl)) {
        if (aui_decv_open(&attr_decv, &ch_module_hdl.decv_hdl)) {
            AUI_PRINTF("\n%s -> aui_decv_open fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }

    /* set the video frame black or freeze when channel change */
    enum aui_en_decv_chg_mode en_chg_mode = AUI_DECV_CHG_BLACK;
    if (aui_decv_chg_mode_set(ch_module_hdl.decv_hdl, en_chg_mode)) {
        AUI_PRINTF("\n%s -> aui_decv_chg_mode_set fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    /* register callback first_I_frame_show_cb */
    first_I_frame_show_callback.puser_data = (void *)p_ch_param;
    first_I_frame_show_callback.callback = (aui_decv_callback)first_I_frame_show_cb;
    first_I_frame_show_callback.type = AUI_DECV_CB_FIRST_SHOWED;
    if (aui_decv_set(ch_module_hdl.decv_hdl, AUI_DECV_SET_REG_CALLBACK, &first_I_frame_show_callback)) {
        AUI_PRINTF("%s -> aui_decv_set AUI_DECV_SET_REG_CALLBACK fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    
    /* register callback first_I_frame_decode_cb */
    first_I_frame_decode_callback.puser_data = (void *)p_ch_param;
    first_I_frame_decode_callback.callback = (aui_decv_callback)first_I_frame_decode_cb;
    first_I_frame_decode_callback.type = AUI_DECV_CB_FIRST_I_FRAME_DECODED;
    if (aui_decv_set(ch_module_hdl.decv_hdl, AUI_DECV_SET_REG_CALLBACK, &first_I_frame_decode_callback)) {
        AUI_PRINTF("%s -> aui_decv_set AUI_DECV_SET_REG_CALLBACK fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    /* set sync mode */
    if (aui_decv_sync_mode(ch_module_hdl.decv_hdl, avsync_mode)) {
        AUI_PRINTF("%s -> aui_decv_sync_mode fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    
    return 0;
EXIT_FAIL:
    return -1;
}

static int deinit_decv()
{
    if (aui_decv_close(NULL, &ch_module_hdl.decv_hdl)) {
        AUI_PRINTF("\n%s -> aui_decv_close fail\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

static int init_deca()
{
    aui_attr_deca attr_deca;
    aui_attr_snd attr_snd;

    //aui_hdl snd_hdl = ch_module_hdl.snd_hdl;

    memset(&ch_module_hdl.deca_hdl, 0, sizeof(ch_module_hdl.deca_hdl));
    memset(&attr_deca, 0, sizeof(aui_attr_deca));
    memset(&attr_snd, 0, sizeof(aui_attr_snd));

    if(aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &ch_module_hdl.deca_hdl)) {
        if (aui_deca_open(&attr_deca, &ch_module_hdl.deca_hdl)) {
            AUI_PRINTF("\n%s -> aui_deca_open fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &ch_module_hdl.snd_hdl)) {
        if (aui_snd_open(&attr_snd, &ch_module_hdl.snd_hdl)) {
            AUI_PRINTF("\n%s -> aui_snd_open fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    if (aui_snd_mute_set(ch_module_hdl.snd_hdl, 0)) {
        AUI_PRINTF("\n%s -> aui_snd_mute_set fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (aui_snd_vol_set(ch_module_hdl.snd_hdl, 30)) {
        AUI_PRINTF("\n%s -> aui_snd_vol_set fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    return 0;
EXIT_FAIL:
    return -1;
}

static int deinit_deca()
{
    //aui_hdl snd_hdl = ch_module_hdl.snd_hdl;
    
    if (aui_deca_close(ch_module_hdl.deca_hdl)) {
        AUI_PRINTF("\n%s -> aui_deca_close fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (aui_snd_close(ch_module_hdl.snd_hdl)) {
        AUI_PRINTF("\n%s -> aui_snd_close fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    
    return 0;
EXIT_FAIL:
    return -1;
}

static int init_nim(struct channel_change_param *p_ch_param)
{
    unsigned int i = 0, j = 0;   
    unsigned int same_nim = 0;
    aui_nim_attr nim_attr;
    //memset(ch_module_hdl.nim_hdl, 0, sizeof(ch_module_hdl.nim_hdl));
#ifdef AUI_LINUX
    if (aui_nim_init(nim_init_cb)) {
        AUI_PRINTF("\n%s -> aui_nim_init fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
#endif
    if (MAX_NIM_CNT < p_ch_param->channel_num) {
        goto EXIT_FAIL;
    }
    for (i = 0; i < p_ch_param->channel_num; i++) {
        same_nim = 0;       
        /* Each nim open once,so need check if current nim index same as before */
        for (j = 0; j < i; j++) {
            if (change_info[i].nim_index == change_info[j].nim_index) {
                same_nim = 1;  /* Current dmx index same as before */
            }
        }
        if (0 == same_nim) {
            memset(&nim_attr, 0, sizeof(aui_nim_attr));
            nim_attr.ul_nim_id = change_info[i].nim_index;
            nim_attr.en_dmod_type = change_info[i].nim_type;
            if (nim_attr.en_dmod_type == AUI_NIM_OFDM) {
                nim_attr.en_std = AUI_STD_DVBT2_COMBO;
            } else if (nim_attr.en_dmod_type == AUI_NIM_QAM) {
                nim_attr.en_std = AUI_NIM_STD_DVBC_ANNEX_AC;
            } else {
                nim_attr.en_std = AUI_NIM_STD_OTHER;
            }
            if (aui_find_dev_by_idx(AUI_MODULE_NIM, change_info[i].nim_index, &ch_module_hdl.nim_hdl[change_info[i].nim_index])) {
                if (aui_nim_open(&nim_attr, &ch_module_hdl.nim_hdl[change_info[i].nim_index])) {
                    AUI_PRINTF("%s -> open nim[%d] fail\n", __FUNCTION__, change_info[i].nim_index);
                    goto EXIT_FAIL;
                }
            }
        }
    }
    return 0;
EXIT_FAIL:
    return -1;
}

static int init_dmx()
{
    int i = 0;
    int j = 0;
    unsigned int same_dmx = 0;
    aui_attr_dmx attr_dmx;
    memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
    
    for (i = 0; i < MAX_DMX_CNT; i++) {
        same_dmx = 0;
        /* Each dmx open once,so need check if current dmx index same as before */
        for (j = 0; j < i; j++) {
            if (change_info[i].hwdmx_id == change_info[j].hwdmx_id) {
                same_dmx = 1; /* Current dmx index same as before */
            }
        }
        if (0 == same_dmx) {
            MEMSET(&attr_dmx, 0, sizeof(aui_attr_dmx));
            attr_dmx.uc_dev_idx = change_info[i].hwdmx_id;
            if (aui_find_dev_by_idx(AUI_MODULE_DMX, change_info[i].hwdmx_id, &ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[i].hwdmx_id)])) {
                if (aui_dmx_open(&attr_dmx, &ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[i].hwdmx_id)])) {
                    AUI_PRINTF("%s -> open dmx[%d] fail\n", __FUNCTION__, change_info[i].hwdmx_id);
                    goto EXIT_FAIL;
                }
            }
        }
    }
    return 0;
EXIT_FAIL:
    return -1;
}

static int deinit_dmx()
{
    int i = 0;
    for (i = 0; i < MAX_DMX_CNT; i++) {
        if (0 != ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[i].hwdmx_id)]) {
            if (aui_dmx_close(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[i].hwdmx_id)])) {
                AUI_PRINTF("%s -> close dmx[%d] fail\n", __FUNCTION__, i);
                return -1;
            }
            ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[i].hwdmx_id)] = 0;
        }
    }
    return 0;
}

static int init_tsi(struct channel_change_param *p_ch_param)
{
    /* open tsi */
    MEMSET(&ch_module_hdl.tsi_hdl, 0, sizeof(ch_module_hdl.tsi_hdl));
    if(aui_find_dev_by_idx(AUI_MODULE_TSI, 0, &ch_module_hdl.tsi_hdl)) {
        if (aui_tsi_open(&ch_module_hdl.tsi_hdl)) {
            AUI_PRINTF("%s -> aui_tsi_open fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    return 0;
EXIT_FAIL:
    return -1;
}

static int init_platform(struct channel_change_param *p_ch_param)
{
#ifdef AUI_LINUX
    if (init_dis()) {
        AUI_PRINTF("%s -> init_dis fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
#endif
    if (init_nim(p_ch_param)) {
        AUI_PRINTF("%s -> init_nim fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (init_tsi(p_ch_param)) {
        AUI_PRINTF("%s -> init_tsi fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (init_decv(p_ch_param)) {
        AUI_PRINTF("%s -> init_decv fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (init_deca()) {
        AUI_PRINTF("%s -> init_deca fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (init_dmx()) {
        AUI_PRINTF("%s -> init_dmx fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    return 0;
EXIT_FAIL:
    return -1;
}

static int deinit_platform(struct channel_change_param *p_ch_param)
{
    int ret = 0;
#ifdef AUI_LINUX
    if (deinit_dis()) {
        AUI_PRINTF("%s -> deinit_dis fail\n", __FUNCTION__);
        ret |= -1;
    }
    /* 
        * In TDS,  when the card power on, the NIM and TSI are already open in the aui_root.c. 
        * when the card power off, they will be closed. 
        * They don't need to be opened and closed again when in each test.
        */
   if (deinit_nim(p_ch_param)) {
        AUI_PRINTF("%s -> deinit_nim fail\n", __FUNCTION__);
        ret |= -1;
    }
    if (deinit_tsi()) {
        AUI_PRINTF("%s -> deinit_tsi fail\n", __FUNCTION__);
        ret |= -1;
    }
#endif
    if (deinit_decv()) {
        AUI_PRINTF("%s -> deinit_decv fail\n", __FUNCTION__);
        ret |= -1;
    }
    if (deinit_deca()) {
        AUI_PRINTF("%s -> deinit_deca fail\n", __FUNCTION__);
        ret |= -1;
    }
    if (deinit_dmx()) {
        AUI_PRINTF("%s -> deinit_dmx fail\n", __FUNCTION__);
        ret |= -1;
    }
    
    return ret;
}

enum { BAND_KU, BAND_C };

static int dvbs_freq2inter(int freq, int lnb_mode, int polar,int *band)
{
    if (lnb_mode == BAND_KU) {
        if (freq > 11700) {
            *band = AUI_NIM_HIGH_BAND;
            return freq - 10600;
        } else {
            *band = AUI_NIM_LOW_BAND;
            return freq - 9750;
        }
    } else {
        // C band Cause:
        // If the band is not AUI_NIM_LOW_BAND/AUI_NIM_HIGH_BAND,
        // polar/band setting will not be do in NIM connect and auto scan.
        *band = AUI_NIM_LOW_BAND;
        if (polar == AUI_NIM_POLAR_VERTICAL) {
            return 5750 - freq;
        } else {
            return 5150 - freq;
        }
    }
}

static int nim_lock_freq(unsigned int channel_idx)
{
    struct aui_nim_connect_param param;
    memset(&param, 0, sizeof(struct aui_nim_connect_param));
    int high_band = 0;
    int lnb_mode = (change_info[channel_idx].frequence > 5150) ? BAND_KU : BAND_C;
    
    param.en_demod_type = change_info[channel_idx].nim_type;
    
    if (AUI_NIM_QAM == change_info[channel_idx].nim_type) {  /* DVB-C */
        param.ul_freq = change_info[channel_idx].frequence;
        param.connect_param.cab.ul_symrate = change_info[channel_idx].symrate;
        param.connect_param.cab.en_qam_mode = change_info[channel_idx].qam_mode;
    } else if (AUI_NIM_QPSK == change_info[channel_idx].nim_type) {  /* DVB-S */
        param.ul_freq = dvbs_freq2inter(change_info[channel_idx].frequence, lnb_mode, AUI_NIM_POLAR_HORIZONTAL, &high_band);
        param.connect_param.sat.ul_freq_band = AUI_NIM_LOW_BAND; /* chinasat6b */
        param.connect_param.sat.ul_symrate = change_info[channel_idx].symrate;
        param.connect_param.sat.fec = AUI_NIM_FEC_AUTO;
        param.connect_param.sat.ul_polar = AUI_NIM_POLAR_HORIZONTAL;
    } else if (AUI_NIM_OFDM == change_info[channel_idx].nim_type) {  /* DVB-T */
        param.ul_freq = change_info[channel_idx].frequence;
        param.connect_param.ter.bandwidth = change_info[channel_idx].freq_band;
        param.en_demod_type = AUI_NIM_OFDM;
        switch (change_info[channel_idx].freq_band) {
            case 6: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; break;
            case 7: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_7_MHZ; break;
            case 8: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_8_MHZ; break;
            default : AUI_PRINTF("wrong bandwidth %d\n", change_info[channel_idx].freq_band); goto EXIT_FAIL;;
        }
        param.connect_param.ter.std = AUI_STD_DVBT2_COMBO; /* automatic detection between DVBT and DVBT2, preferred DVBT2 */
        param.connect_param.ter.fec = AUI_NIM_FEC_AUTO;
        param.connect_param.ter.spectrum = AUI_NIM_SPEC_AUTO;
    } else {
        AUI_PRINTF("%s %d -> nim type %d is wrong !\n", __FUNCTION__, __LINE__, change_info[channel_idx].nim_type);
        goto EXIT_FAIL;
    }
    
    if (aui_nim_connect(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index], &param)) {
        AUI_PRINTF("%s -> aui_nim_connect fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    int lock_status = AUI_NIM_STATUS_UNKNOWN;
    #define NIM_TIMEOUT_MS 1000
    int timeout = NIM_TIMEOUT_MS / 10;
    while ((lock_status != AUI_NIM_STATUS_LOCKED) && timeout) {
        if (aui_nim_is_lock(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index], &lock_status)) {
            AUI_PRINTF("%s -> aui_nim_is_lock fail\n", __FUNCTION__);
            aui_nim_disconnect(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
            //aui_nim_close(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
            //return 0;
            //goto EXIT_FAIL;
        }
        AUI_SLEEP(10);
        timeout--;
    }
    
    /* print nim signal info */
    aui_signal_status info;
    memset(&info, 0, sizeof(aui_signal_status));
	if (aui_nim_signal_info_get(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index], &info)) {
		AUI_PRINTF("aui_nim_signal_info_get return fail\n");
		aui_nim_disconnect(&ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
		//aui_nim_close(&ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
		goto EXIT_FAIL;
	}
	AUI_PRINTF("nim info: freq %ld, strength %d, quality %d, ber %d, rf_level %d, signal_cn %d\n", 
            info.ul_freq, (int)info.ul_signal_strength,
            (int)info.ul_signal_quality, (int)info.ul_bit_error_rate,
            (int)info.ul_rf_level, (int)info.ul_signal_cn);
    
    if (lock_status != AUI_NIM_STATUS_LOCKED) {
        AUI_PRINTF("%s -> channel is not locked\n", __FUNCTION__);
        aui_nim_disconnect(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
        //aui_nim_close(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
        //return 0;
        goto EXIT_FAIL;
    }
    AUI_PRINTF("channel is locked on freq %d in %d ms\n", param.ul_freq, NIM_TIMEOUT_MS - timeout*10);

#if 0 /* because do not need nim info, only need to connect nim success */
    /* Driver need at lease 50MS to prepare the signal info after locked */
    AUI_SLEEP(100);  /* wait 100MS */

    aui_signal_status info;
    MEMSET(&info, 0, sizeof(struct aui_signal_status));
    if (aui_nim_signal_info_get(ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index], &info)) {/*getting the infomation of current tp*/
        AUI_PRINTF("%s -> aui_nim_signal_info_get\n", __FUNCTION__);
        aui_nim_disconnect(&ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
        aui_nim_close(&ch_module_hdl.nim_hdl[change_info[channel_idx].nim_index]);
        goto EXIT_FAIL;
    }
#endif
    return 0;
EXIT_FAIL:
    return -1;
}

static int config_tsi(struct channel_change_param *p_ch_param)
{
    if (MAX_TSI_CNT < p_ch_param->chan_idx) {
        AUI_PRINTF("chan_idx %d is more than %d !\n", p_ch_param->chan_idx, MAX_TSI_CNT);
        goto EXIT_FAIL;
    }
    unsigned int start = get_time_ms();
    unsigned int nim_index = change_info[p_ch_param->chan_idx].nim_index;
    AUI_PRINTF("nim_index: %d\n", nim_index);
    aui_attr_tsi attr_tsi;
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];  /* If use MAX_TSI_CNT, array bounds will happen */
    enum aui_tsi_channel_id tsi_channel_id[MAX_TSI_CNT] = {AUI_TSI_CHANNEL_0, AUI_TSI_CHANNEL_1, AUI_TSI_CHANNEL_2, AUI_TSI_CHANNEL_3};

    MEMSET(&attr_tsi, 0, sizeof(aui_attr_tsi));
    MEMSET(tsi_cfg, 0xff, sizeof(tsi_cfg));  /* not 0x00 */

    aui_tsi_config(tsi_cfg);

    if (AUI_TSI_INPUT_MAX > tsi_cfg[nim_index].ul_input_src) {
        attr_tsi.ul_init_param = tsi_cfg[nim_index].ul_hw_init_val;
        if (aui_tsi_src_init(ch_module_hdl.tsi_hdl, tsi_cfg[nim_index].ul_input_src, &attr_tsi)) {
            AUI_PRINTF("%s -> aui_tsi_src_init fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
        if (aui_tsi_route_cfg(ch_module_hdl.tsi_hdl,
                tsi_cfg[nim_index].ul_input_src,
                tsi_channel_id[nim_index],
                hwdmx_id_to_tsi_output_id(change_info[p_ch_param->chan_idx].hwdmx_id))) {
            AUI_PRINTF("%s -> aui_tsi_route_cfg fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    AUI_PRINTF("config tsi time: %dms\n", get_time_ms() - start);

    return 0;
EXIT_FAIL:
    return -1;
}

static void first_I_frame_decode_cb(void *p_user_hld, unsigned int paras1, unsigned int paras2)
{
    struct channel_change_param *p_ch_param = NULL;
    if (p_user_hld != NULL) {
        p_ch_param = (struct channel_change_param *)p_user_hld;
    }
    
    ch_time_point.first_I_frame_decode_time_point = get_time_ms();
    ch_time_interval[p_ch_param->test_count].first_I_frame_decode_time = ch_time_point.first_I_frame_decode_time_point - ch_time_point.start_test_time_point;
    AUI_PRINTF("[%d]start --> first_I_frame_decode = %d ms\n", p_ch_param->test_count, ch_time_interval[p_ch_param->test_count].first_I_frame_decode_time);

    (void)paras1;
    (void)paras2;

    return;
}

static void first_I_frame_show_cb(void *p_user_hld, unsigned int paras1, unsigned int paras2)
{
    struct channel_change_param *p_ch_param = NULL;
    if (p_user_hld != NULL) {
        p_ch_param = (struct channel_change_param *)p_user_hld;
    }
    
    ch_time_point.first_I_frame_show_time_point = get_time_ms();
    ch_time_interval[p_ch_param->test_count].first_I_frame_show_time = ch_time_point.first_I_frame_show_time_point - ch_time_point.start_test_time_point;
    AUI_PRINTF("[%d]start --> first_I_frame_show = %d ms\n", p_ch_param->test_count, ch_time_interval[p_ch_param->test_count].first_I_frame_show_time);

    if (CHANNEL_CHANGE_CYCLE_TEST_MODE == p_ch_param->test_mode) {
        AUI_PRINTF("playing %ds ...\n", p_ch_param->delay_time);
    }
    
    (void)paras1;
    (void)paras2;

    return;
}

static int play_av(struct channel_change_param *p_ch_param)
{
    aui_dmx_stream_pid pid_list;
    aui_dmx_data_path path;

    memset(&pid_list, 0, sizeof(aui_dmx_stream_pid));
    memset(&path, 0, sizeof(aui_dmx_data_path));

    unsigned int channel_idx = p_ch_param->chan_idx;

    ch_time_point.play_av_start_time_point = get_time_ms();

    /******************** config tsi ********************/
    if (p_ch_param->nim_index_pre != change_info[channel_idx].nim_index) {
        if (config_tsi(p_ch_param)) {
            AUI_PRINTF("config_tsi return failed!\n");
            goto EXIT_FAIL;
        }
    } else {
        AUI_PRINTF("nim_index %d is same to last test\n", change_info[channel_idx].nim_index);
    }
    
    /******************** lock freqence ********************/
    /* Attention:
        * If the current frequency is the same to the previous, not need to lock it. If they are different, we should be lock it.
        * OR If the nim index changed, we should lock again.
        */
    if ((p_ch_param->freq_pre != change_info[channel_idx].frequence) || (p_ch_param->nim_index_pre != change_info[channel_idx].nim_index)) {
        if (nim_lock_freq(channel_idx)) {
            AUI_PRINTF("%s -> nim_lock_freq fail\n", __FUNCTION__);
            //goto EXIT_FAIL; /* when we do the weak signal test, need to lock nim every time */
        }
    } else {
        AUI_PRINTF("freq %d has been locked\n", change_info[channel_idx].frequence);
    }
    
    ch_time_point.nim_lock_freq_end_time_point = get_time_ms();
    ch_time_interval[p_ch_param->test_count].nim_lock_freq_time = ch_time_point.nim_lock_freq_end_time_point - ch_time_point.play_av_start_time_point;
    AUI_PRINTF("[%d]nim_lock_freq = %d ms\n", p_ch_param->test_count, ch_time_interval[p_ch_param->test_count].nim_lock_freq_time);

    /********************   start decv   ********************/
    if (p_ch_param->video_type_pre != change_info[channel_idx].video_type) {
        if (aui_decv_decode_format_set(ch_module_hdl.decv_hdl, change_info[channel_idx].video_type)) {
            AUI_PRINTF("%s -> aui_decv_decode_format_set fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    if (aui_decv_start(ch_module_hdl.decv_hdl)) {
        AUI_PRINTF("%s -> aui_decv_start fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    
    /********************   start deca   ********************/
    if (p_ch_param->audio_type_pre != change_info[channel_idx].audio_type) {
        if (aui_deca_type_set(ch_module_hdl.deca_hdl, change_info[channel_idx].audio_type)) {
            AUI_PRINTF("%s -> aui_deca_type_set fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    /*
        * we can comment out this part, because we have set avsync mode by aui_decv_sync_mode.
        * If we set a wrong avsync mode before in the decv module by aui_decv_sync_mode, we can
        * set a right avsync mode by aui_deca_sync_mode_set. Otherwise, we not need set again. 
        */
#if 0
    if (aui_deca_sync_mode_set(ch_module_hdl.deca_hdl, AUI_DECA_DEOCDER_SYNC)) {
        AUI_PRINTF("%s -> aui_deca_sync_mode_set AUI_DECA_DEOCDER_SYNC fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
#endif
    if (aui_deca_start(ch_module_hdl.deca_hdl, NULL)) {
        AUI_PRINTF("%s -> aui_deca_start fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    /********************   start dmx   ********************/
    if (aui_dmx_start(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], NULL)) {
        AUI_PRINTF("%s -> aui_dmx_start fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    pid_list.ul_pid_cnt = 3;
    pid_list.stream_types[0] = AUI_DMX_STREAM_VIDEO;
    pid_list.stream_types[1] = AUI_DMX_STREAM_AUDIO;
    pid_list.stream_types[2] = AUI_DMX_STREAM_PCR;
    pid_list.aus_pids_val[0] = change_info[channel_idx].video_pid;
    pid_list.aus_pids_val[1] = change_info[channel_idx].audio_pid;
    pid_list.aus_pids_val[2] = change_info[channel_idx].pcr_pid;
    //AUI_PRINTF("\n video_pid:%d,audio_pid:%d,pcr_pid:%d\n",
    //    change_info[channel_idx].video_pid,change_info[channel_idx].audio_pid,change_info[channel_idx].pcr_pid);
    if (aui_dmx_set(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], AUI_DMX_STREAM_CREATE_AV, &pid_list)) {
        AUI_PRINTF("%s -> aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    
    path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    if (aui_dmx_data_path_set(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], &path)) {
        AUI_PRINTF("%s -> aui_dmx_data_path_set fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (aui_dmx_set(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], AUI_DMX_SET_AVSYNC_SOURCE_TYPE, (void *)AUI_AVSYNC_FROM_TUNER)) {
        AUI_PRINTF("%s -> aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    if (aui_dmx_set(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], AUI_DMX_STREAM_ENABLE, NULL)) {
        AUI_PRINTF("%s -> aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    /* save params for comparing it with the next test params */
    p_ch_param->freq_pre = change_info[channel_idx].frequence;
    p_ch_param->video_type_pre = change_info[channel_idx].video_type;
    p_ch_param->audio_type_pre = change_info[channel_idx].audio_type;
    p_ch_param->nim_index_pre = change_info[channel_idx].nim_index;

    ch_time_point.play_av_end_time_point = get_time_ms();
    ch_time_interval[p_ch_param->test_count].play_av_time = ch_time_point.play_av_end_time_point - ch_time_point.play_av_start_time_point;
    AUI_PRINTF("[%d]play_av_time = %d ms\n", p_ch_param->test_count, ch_time_interval[p_ch_param->test_count].play_av_time);

    return 0;
EXIT_FAIL:
    return -1;
}

static int stop_av(struct channel_change_param *p_ch_param)
{
    int ret = 0;
    unsigned int channel_idx = p_ch_param->chan_idx;

    ch_time_point.stop_av_start_time_point = get_time_ms();

#if 0
    if (aui_dmx_stop(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], NULL)) {
        AUI_PRINTF("%s -> aui_dmx_stop fail\n", __FUNCTION__);
        ret |= 1;
    }
#else
   /*
    * Can NOT call aui_dmx_stop. If call it, dmx will close all channels. It will affect the other place of using the channel. 
    * we only need to disable stream.
    */
    if (aui_dmx_set(ch_module_hdl.dmx_hdl[hwdmx_id_to_index(change_info[channel_idx].hwdmx_id)], AUI_DMX_STREAM_DISABLE, NULL)) {
        AUI_PRINTF("%s -> aui_dmx_set AUI_DMX_STREAM_DISABLE fail\n", __FUNCTION__);
        ret |= -1;
    }
#endif
    if (aui_deca_stop(ch_module_hdl.deca_hdl, NULL)) {
        AUI_PRINTF("%s -> aui_deca_stop fail\n", __FUNCTION__);
        ret |= -1;
    }
    if (aui_decv_stop(ch_module_hdl.decv_hdl)) {
        AUI_PRINTF("%s -> aui_decv_stop fail\n", __FUNCTION__);
        ret |= -1;
    }

    ch_time_point.stop_av_end_time_point = get_time_ms();
    ch_time_interval[p_ch_param->test_count].stop_av_time = ch_time_point.stop_av_end_time_point - ch_time_point.stop_av_start_time_point;
    AUI_PRINTF("[%d]stop_av_time = %d ms\n", p_ch_param->test_count, ch_time_interval[p_ch_param->test_count].stop_av_time);
    ch_time_interval[p_ch_param->test_count].channel_change_time = ch_time_interval[p_ch_param->test_count].stop_av_time
                                                                   + ch_time_interval[p_ch_param->test_count].first_I_frame_show_time;
    AUI_PRINTF("[%d]channel_change_time = %d ms\n", p_ch_param->test_count, ch_time_interval[p_ch_param->test_count].channel_change_time);

    return ret;
}

static void channel_change_1_help()
{
    AUI_PRINTF("Please type the right command format.\n");
    AUI_PRINTF(RED_STR"Default value instructions:\n"DEFAULT_STR);
    AUI_PRINTF("delay_time        =  5 s\n");
    AUI_PRINTF("channel_num       =  4\n");
    AUI_PRINTF("cycle_num         =  2\n");
    AUI_PRINTF("avsync_mode       =  2 (0: AVSYNC_PCR, 1: AVSYNC_AUDIO, 2: AVSYNC_FREERUN)\n");
    AUI_PRINTF("config_file_path  =  /usr/share/auiconfig/channel_change_info.ini\n\n");

    
    AUI_PRINTF(RED_STR"One step test\n"DEFAULT_STR);
    AUI_PRINTF("situation [1]: all params are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode]\n");
    AUI_PRINTF(GREEN_STR"1 0\n"DEFAULT_STR);
    
    AUI_PRINTF("situation [2]: avsync_mode is the value specified by user, the others are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[avsync_mode]\n");
    AUI_PRINTF(GREEN_STR"1 0,0\n"DEFAULT_STR);

    AUI_PRINTF("situation [3]: channel_num and config_file_path are the values specified by user, the others are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[channel_num],[config_file_path]\n");
    AUI_PRINTF(GREEN_STR"1 0,2,/usr/mnt_app/channel_change_info.ini\n"DEFAULT_STR);
         
    AUI_PRINTF("situation [4]: Use the config file, all params are the values specified by user.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[avsync_mode],[channel_num],[config_file_path]\n");
    AUI_PRINTF(GREEN_STR"1 0,0,2,/usr/mnt_app/channel_change_info.ini\n"DEFAULT_STR);
    
    AUI_PRINTF("situation [5]: Do not use the config file, all the parameters specified by user directly.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[avsync_mode],\n");
    AUI_PRINTF("             [change_mode],[nim_index],[nim_type],[hwdmx_id],\n");
    AUI_PRINTF("             [video_type],[audio_type],[qam_mode],[frequence],\n");
    AUI_PRINTF("             [symrate],[video_pid],[audio_pid],[pcr_pid]\n");
    AUI_PRINTF(GREEN_STR"1 0,0,0,0,0,0,0,1,3,3840,27500,513,660,8190\n\n"DEFAULT_STR);


    AUI_PRINTF(RED_STR"Cycle test\n"DEFAULT_STR);
    AUI_PRINTF("situation [1]: all params are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode]\n");
    AUI_PRINTF(GREEN_STR"1 1\n"DEFAULT_STR);
    
    AUI_PRINTF("situation [2]: delay_time is the value specified by user, the others are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[delay_time]\n");
    AUI_PRINTF(GREEN_STR"1 1,5\n"DEFAULT_STR);

    AUI_PRINTF("situation [3]: channel_num and config_file_path are the values specified by user, the others are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[channel_num],[config_file_path]\n");
    AUI_PRINTF(GREEN_STR"1 1,2,/usr/mnt_app/channel_change_info.ini\n"DEFAULT_STR);
    
    AUI_PRINTF("situation [4]: delay_time channel_num and config_file_path are the values specified by user, the others are the default values.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[delay_time],[channel_num],[config_file_path]\n");
    AUI_PRINTF(GREEN_STR"1 1,5,2,/usr/mnt_app/channel_change_info.ini\n"DEFAULT_STR);
    
    AUI_PRINTF("situation [5]: all params are the values specified by user.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[delay_time],[channel_num],[cycle_num],[avsync_mode],[config_file_path]\n");
    AUI_PRINTF("             [cycle_num]:  0       -- Eternal cycle test, do stress test, not statistics the channel change time.\n");
    AUI_PRINTF("                          (0,100]  -- Normal cycle number, do cycle test, and statistics the channel change time.\n");
    AUI_PRINTF("                          (100,++) -- Invalid cycle number.\n");
    AUI_PRINTF(GREEN_STR"1 1,5,2,3,0,/usr/mnt_app/channel_change_info.ini\n"DEFAULT_STR);

    AUI_PRINTF("situation [6]: all params are the values specified by user, use these channel config info by command '2'.\n");
    AUI_PRINTF("[Test_Items] [test_mode],[delay_time],[channel_num],[cycle_num],[avsync_mode]\n");
    AUI_PRINTF("             [cycle_num]:  0       -- Eternal cycle test, do stress test, not statistics the channel change time.\n");
    AUI_PRINTF("                          (0,100]  -- Normal cycle number, do cycle test, and statistics the channel change time.\n");
    AUI_PRINTF("                          (100,++) -- Invalid cycle number.\n");
    AUI_PRINTF(GREEN_STR"1 1,5,2,3,0\n\n"DEFAULT_STR);
    
    return;
}

static void channel_change_2_help()
{
    AUI_PRINTF(RED_STR"set the flag of config info source\n"DEFAULT_STR);
    AUI_PRINTF("situation [1] use config file (Linux), use the default config but do not modify (TDS)\n");
    AUI_PRINTF("[Test_Items] [config_info_flag]\n");
    AUI_PRINTF(GREEN_STR"2 0\n"DEFAULT_STR);
    AUI_PRINTF("situation [2] the users set these configuration parameters by themselves\n");
    AUI_PRINTF("[Test_Items] [config_info_flag]\n");
    AUI_PRINTF(GREEN_STR"2 1\n"DEFAULT_STR);
    AUI_PRINTF("Note: If config_info_flag is the other value, it will be a wrong parameter\n\n");

    AUI_PRINTF(RED_STR"set channel change config info\n"DEFAULT_STR);
    AUI_PRINTF("the users can set these channel change config info by themselves.\n");
    AUI_PRINTF("[Test_Items] [chan_idx],\n");
    AUI_PRINTF("             [change_mode],[nim_index],[nim_type],[hwdmx_id],\n");
    AUI_PRINTF("             [video_type],[audio_type],[qam_mode],[frequence],\n");
    AUI_PRINTF("             [symrate],[video_pid],[audio_pid],[pcr_pid]\n");
    AUI_PRINTF(GREEN_STR"2 0,0,0,0,0,0,1,3,3840,27500,513,660,8190  #CCTV-2\n"DEFAULT_STR);
    AUI_PRINTF(GREEN_STR"2 1,0,0,0,0,0,1,3,3840,27500,514,670,8190  #CCTV-7\n"DEFAULT_STR);
    AUI_PRINTF(GREEN_STR"2 2,0,0,0,0,0,1,3,3880,27500,516,690,8190  #CCTV-14\n"DEFAULT_STR);
    AUI_PRINTF(GREEN_STR"2 3,0,0,0,0,0,1,3,3880,27500,517,700,8190  #CCTV-13\n"DEFAULT_STR);

    return;
}

static unsigned long test_channel_change_help(unsigned long *argc,char **argv,char *sz_out_put)
{
    *sz_out_put = 0;
    
    #define    CHANNEL_CHANGE_1_HELP      "test channel change.\n"
    AUI_PRINTF("1\n");
    AUI_PRINTF(CHANNEL_CHANGE_1_HELP);
    channel_change_1_help();

    #define    CHANNEL_CHANGE_2_HELP      "set channel change config info.\n"
    AUI_PRINTF("2\n");
    AUI_PRINTF(CHANNEL_CHANGE_2_HELP);
    channel_change_2_help();
    
    return AUI_RTN_HELP;
}

static int check_1_input_param(struct channel_change_param *p_ch_param)
{
    if (TEST_MODE_MAX < p_ch_param->test_mode) {
        AUI_PRINTF("%d NOT support! Please specify test_mode in [0, 1].\n", p_ch_param->test_mode);
        goto EXIT_FAIL;
    }
    if (AVSYNC_MODE_MAX < p_ch_param->avsync_mode) {
        AUI_PRINTF("%d NOT support! Please specify avsync_mode in [0, 2].\n", p_ch_param->avsync_mode);
        goto EXIT_FAIL;
    }
    if (CHANNEL_NUM_MAX < p_ch_param->channel_num) {
        AUI_PRINTF("%d NOT support! Please specify channel_num in [0, 4].\n", p_ch_param->channel_num);
        goto EXIT_FAIL;
    }
    if (CYCLE_NUM_MAX < p_ch_param->cycle_num) {
        AUI_PRINTF("%d NOT support! Please specify cycle_num in [0, 100].\n", p_ch_param->cycle_num);
        goto EXIT_FAIL;
    }
    
    return 0;
EXIT_FAIL:
    return -1;
}

static int get_1_input_param(unsigned long *argc, char **argv, struct channel_change_param *p_ch_param)
{
    char default_config_file_path[CONFIG_FILE_PATH_MAX] = "/usr/share/auiconfig/channel_change_info.ini";
    unsigned int default_delay_time = 5;  /* uint is seconds */
    unsigned int default_channel_num = 4;
    unsigned int default_cycle_num = 2;
    unsigned int default_avsync_mode = 2;  /* 0: AVSYNC_PCR, 1: AVSYNC_AUDIO, 2: AVSYNC_FREERUN */
    
    if ((0 == ATOI(argv[0])) && (1 == *argc)){ /* One step test, Default avsync mode and config file */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->avsync_mode = default_avsync_mode;
        p_ch_param->channel_num = default_channel_num;
        strcpy(p_ch_param->config_file_path, default_config_file_path);
    } else if ((0 == ATOI(argv[0])) && (2 == *argc)){ /* One step test, Usr specified avsync mode, Default config file */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->avsync_mode = ATOI(argv[1]);
        p_ch_param->channel_num = default_channel_num;
        strcpy(p_ch_param->config_file_path, default_config_file_path);
    } else if ((0 == ATOI(argv[0])) && (3 == *argc)){ /* One step test, Usr specified config file, Default avsync mode */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->avsync_mode = default_avsync_mode;
        p_ch_param->channel_num = ATOI(argv[1]);
        strcpy(p_ch_param->config_file_path, argv[2]);
    } else if ((0 == ATOI(argv[0])) && (4 == *argc)){ /* One step test, Usr specified avsync mode and config file */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->avsync_mode = ATOI(argv[1]);
        p_ch_param->channel_num = ATOI(argv[2]);
        strcpy(p_ch_param->config_file_path, argv[3]);
    } else if ((0 == ATOI(argv[0])) && (14 == *argc)){ /* One step test, Usr specified all params */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->avsync_mode = ATOI(argv[1]);
        p_ch_param->chan_idx = 0;
        p_ch_param->channel_num = 1; /* only use channel0 */
        g_config_info_flag = 1;      /* usr set all config params */
        change_info[0].ch_ch_mode = ATOI(argv[2]);
        change_info[0].nim_index = ATOI(argv[3]);
        change_info[0].nim_type = ATOI(argv[4]);
        change_info[0].hwdmx_id = ATOI(argv[5]);
        change_info[0].video_type = ATOI(argv[6]);
        change_info[0].audio_type = ATOI(argv[7]);
        change_info[0].qam_mode = ATOI(argv[8]);
        change_info[0].frequence = ATOI(argv[9]);
        change_info[0].symrate = ATOI(argv[10]);
        change_info[0].freq_band = ATOI(argv[10]);//DVBT band and DVBC symrate share
        change_info[0].video_pid = ATOI(argv[11]);
        change_info[0].audio_pid = ATOI(argv[12]);
        change_info[0].pcr_pid = ATOI(argv[13]);
    } else if ((1 == ATOI(argv[0])) && (1 == *argc)) { /* Cycle test, Default all params */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->delay_time = default_delay_time;
        p_ch_param->channel_num = default_channel_num;
        p_ch_param->cycle_num = default_cycle_num;
        p_ch_param->avsync_mode = default_avsync_mode;
        strcpy(p_ch_param->config_file_path, default_config_file_path);
    } else if ((1 == ATOI(argv[0])) && (2 == *argc)) { /* Cycle test, Usr specified delay time, Default other params */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->delay_time = ATOI(argv[1]);
        p_ch_param->channel_num = default_channel_num;
        p_ch_param->cycle_num = default_cycle_num;
        p_ch_param->avsync_mode = default_avsync_mode;
        strcpy(p_ch_param->config_file_path, default_config_file_path);
    } else if ((1 == ATOI(argv[0])) && (3 == *argc)) { /* Cycle test, Usr specified config file, Default other params */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->delay_time = default_delay_time;
        p_ch_param->channel_num = ATOI(argv[1]);
        p_ch_param->cycle_num = default_cycle_num;
        p_ch_param->avsync_mode = default_avsync_mode;
        strcpy(p_ch_param->config_file_path, argv[2]);
    } else if ((1 == ATOI(argv[0])) && (4 == *argc)) { /* Cycle test, Usr specified delay time and config file, Default other params */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->delay_time = ATOI(argv[1]);
        p_ch_param->channel_num = ATOI(argv[2]);
        p_ch_param->cycle_num = default_cycle_num;
        p_ch_param->avsync_mode = default_avsync_mode;
        strcpy(p_ch_param->config_file_path, argv[3]);
    } else if ((1 == ATOI(argv[0])) && (6 == *argc)) { /* Cycle test, Usr specified all params */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->delay_time = ATOI(argv[1]);
        p_ch_param->channel_num = ATOI(argv[2]);
        p_ch_param->cycle_num = ATOI(argv[3]);
        p_ch_param->avsync_mode = ATOI(argv[4]);
        strcpy(p_ch_param->config_file_path, argv[5]);
    } else if ((1 == ATOI(argv[0])) && (5 == *argc)) { /* Cycle test, Usr specified all params, but not use config file */
        p_ch_param->test_mode = ATOI(argv[0]);
        p_ch_param->delay_time = ATOI(argv[1]);
        p_ch_param->channel_num = ATOI(argv[2]);
        p_ch_param->cycle_num = ATOI(argv[3]);
        p_ch_param->avsync_mode = ATOI(argv[4]);
        if (1 != g_config_info_flag) {
            AUI_PRINTF("Firstly, please use the command '2' to set these channel config info.\n");
            channel_change_2_help();
            return -1;
        }
    } else {
        goto EXIT_FAIL;
    }

    /* check input param */
    if (check_1_input_param(p_ch_param)) {
        goto EXIT_FAIL;
    }

    return 0;
EXIT_FAIL:
    channel_change_1_help();
    return -1;
}

static unsigned long channel_change_one_step_test_mode(struct channel_change_param *p_ch_param)
{
    unsigned short play_flag = 0;
    
    AUI_PRINTF("\nThe current is one step test mode...\n");
    /* when channel num is 1, we not need to input chan_idx. */
    AUI_PRINTF("Please press the 'q' key, quit the test.\n");
    while (1 == p_ch_param->channel_num) {
        p_ch_param->chan_idx = 0;
        /* store start test time point */
        ch_time_point.start_test_time_point = get_time_ms();

        if (1 == play_flag) {
            play_flag = 0;
            if (stop_av(p_ch_param)) {
                AUI_PRINTF ("%s -> stop_av fail\n", __FUNCTION__);
                goto EXIT_FAIL;
            }
        }
        if (play_av(p_ch_param)) {
            play_flag = 0;
            AUI_PRINTF ("%s -> play_av fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        } else {
            play_flag = 1;
        }
        /* Press the 'q' key, out of the while loop */
        if (aui_test_get_user_str_input(&p_ch_param->key)){
            AUI_PRINTF("%s -> aui_test_get_user_str_input fail\n", __FUNCTION__);
            break;
        } else {
            if (p_ch_param->key == 'q') {
                AUI_PRINTF("To quit the test...\n");
                break;
            }
        }
    }
    /* when channel num is  greater than 1, we should be input chan_idx. */
    while(1 < p_ch_param->channel_num) {
        AUI_PRINTF("\nPlease input the channel, from 0 to %d: \n", (p_ch_param->channel_num - 1));
        if (aui_test_get_user_str_input(&p_ch_param->key)){
            AUI_PRINTF ("%s -> aui_test_get_user_str_input fail\n", __FUNCTION__);
            break;
        }
        p_ch_param->chan_idx = ATOI(&p_ch_param->key);
        if (p_ch_param->channel_num > p_ch_param->chan_idx) {
            AUI_PRINTF("play channel %d \n", p_ch_param->chan_idx);
            /* store start test time point */
            ch_time_point.start_test_time_point = get_time_ms();
                        
            if (1 == play_flag) {
                play_flag = 0;
                if (stop_av(p_ch_param)) {
                    AUI_PRINTF ("%s -> stop_av fail\n", __FUNCTION__);
                    goto EXIT_FAIL;
                }
            }
            if (play_av(p_ch_param)) {
                play_flag = 0;
                AUI_PRINTF ("%s -> play_av fail\n", __FUNCTION__);
                goto EXIT_FAIL;
            } else {
                play_flag = 1;
            }
            p_ch_param->pre_chan_idx = p_ch_param->chan_idx;
        } else {
            AUI_PRINTF("Please input the channel, from 0 to %d.\n", (p_ch_param->channel_num - 1));
            p_ch_param->chan_idx = p_ch_param->pre_chan_idx;
            break;
        }
        /* Press the 'q' key, out of the while loop */
        if (p_ch_param->key == 'q') {
            AUI_PRINTF("To quit the test...\n");
            break;
        }
    }
    /* If not stop_av after quiting, we should be stop_av. */
    if (1 == play_flag) {
        play_flag = 0;
        if (stop_av(p_ch_param)) {
            AUI_PRINTF ("%s -> stop_av fail", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }
    
    return 0;
EXIT_FAIL:
    return -1;
}

static unsigned long channel_change_cycle_test_mode(struct channel_change_param *p_ch_param)
{
    unsigned short play_flag = 0;

    AUI_PRINTF("\nThe current is cycle test model...\n");
    AUI_PRINTF("Please press the 'q' key, quit the test.\n");
    while(1){
        AUI_PRINTF("\nplay channel %d \n", p_ch_param->chan_idx);
        /* store start test time point */
        ch_time_point.start_test_time_point = get_time_ms();

        /* play av */
        if (play_av(p_ch_param)) {
            play_flag = 0;
            AUI_PRINTF("%s -> play_av fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        } else {
            play_flag = 1;
        }
        
        AUI_SLEEP(p_ch_param->delay_time*1000);
        if (1 == play_flag) {
            play_flag = 0;
            if (stop_av(p_ch_param)) {
                AUI_PRINTF("%s -> stop_av fail\n", __FUNCTION__);
                goto EXIT_FAIL;
            }
        }
        /* check channel number */
        p_ch_param->chan_idx++;
        if (p_ch_param->channel_num == p_ch_param->chan_idx) {
            p_ch_param->chan_idx = 0;
        }

        /* The test_num is equal to 0 when the cycle_num is equal to 0. */
        /* when the cycle_num is set to 0, always do cycle test. */
        if (0 != p_ch_param->test_num) {
            /* check test number */
            p_ch_param->test_count++;
            if (p_ch_param->test_num == p_ch_param->test_count) {
                p_ch_param->test_count = 0;
                AUI_PRINTF("Test number [%d] have reached, quit the test...\n", p_ch_param->test_num);
                break;
            }
        }
        /* Press the 'q' key, out of the while loop */
        if (aui_test_get_user_key_input(&p_ch_param->key)){
            AUI_PRINTF("%s -> aui_test_get_user_key_input fail\n", __FUNCTION__);
            break;
        } else {
            if (p_ch_param->key == 'q') {
                AUI_PRINTF("To quit the test...\n");
                break;
            }
        }
    }
    if (1 == play_flag) {
        play_flag = 0;
        if (stop_av(p_ch_param)) {
            AUI_PRINTF ("%s -> stop_av fail", __FUNCTION__);
            goto EXIT_FAIL;
        }
    }

    /* when the cycle_num is not set to 0, statistic channel change time. */
    if (0 != p_ch_param->test_num) {
        /* statistic time info: average value, max value, min value */
        statistic_time_info(p_ch_param);
    }

    return 0;
EXIT_FAIL:
    return -1;
}

static unsigned long test_channel_change(unsigned long *argc, char **argv, char *sz_out_put)
{
    if ((NULL == argc) || (NULL == *argv) || (NULL == sz_out_put)) {
        AUI_PRINTF("The input parameter of %s is illegal\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    *sz_out_put = 0;
    memset(&ch_module_hdl, 0, sizeof(struct channel_change_module_handle));
    memset(&ch_time_point, 0, sizeof(struct channel_change_time_point));

    struct channel_change_param ch_param;
    memset(&ch_param, 0, sizeof(struct channel_change_param));

    ch_param.freq_pre = 0;
    ch_param.audio_type_pre = AUI_DECA_STREAM_TYPE_INVALID;
    ch_param.video_type_pre = AUI_DECV_FORMAT_INVALID;
    ch_param.nim_index_pre = -1;

    /* get the input param */
    if (get_1_input_param(argc, argv, &ch_param)) {
        goto EXIT_FAIL;
    }
    
    /* get the channel info */
    if (0 == g_config_info_flag) {  /* use config file (Linux), use the default config but do not modify (TDS) */
        if (init_channel_info(&ch_param)) {
            AUI_PRINTF("%s -> init_channel_info fail\n", __FUNCTION__);
            goto EXIT_FAIL;
        }
    } else if (1 == g_config_info_flag) {  /* usr set all config params */
        AUI_PRINTF("Use these configuration parameters by users themselves.\n");
        AUI_PRINTF("Firstly, Please config these parameters of all channels by the command '2'.\n");
    }
    
    /* init platform */
    if (init_platform(&ch_param)) {
        AUI_PRINTF("%s -> init_platform fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    if (1 == ch_param.test_mode) {
        ch_param.test_num = ch_param.cycle_num * ch_param.channel_num;
    } else {
        ch_param.test_num = 1;
    }
    /* select the test mode, and do the test */
    switch (ch_param.test_mode) {
        case CHANNEL_CHANGE_ONE_STEP_TEST_MODE : {
            if (channel_change_one_step_test_mode(&ch_param)) {
                goto EXIT_DEINIT;
            }
            break;
        }
        case CHANNEL_CHANGE_CYCLE_TEST_MODE : {
            if (channel_change_cycle_test_mode(&ch_param)) {
                goto EXIT_DEINIT;
            }
            break;
        }
        default : {
            AUI_PRINTF("Please select right test mode.\n");
            break;
        }
    }
    
    /* deinit platform */
    if (deinit_platform(&ch_param)) {
        AUI_PRINTF("%s -> init_platform fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    /* print prompt information */
    if (1 == g_config_info_flag) {  /* usr set all config params */
        AUI_PRINTF("If the users don't want to configure the parameters by themselves,\n");
        AUI_PRINTF("Please cancel these configuration by the command '2 0'.\n");
    }
    
    return 0;

EXIT_DEINIT:
    if (deinit_platform(&ch_param)) {
        AUI_PRINTF("%s -> init_platform fail\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
EXIT_FAIL:
    return 1;
}

static int get_2_input_param(unsigned long *argc, char **argv)
{
    unsigned int chan_idx = 0;
    unsigned int i = 0;
    
    if ((CHANNEL_NUM_MAX > ATOI(argv[0])) && (13 == *argc)) {
        g_config_info_flag = 1;  /* usr set all config params */
        chan_idx = ATOI(argv[0]);
        change_info[chan_idx].ch_ch_mode = ATOI(argv[1]);
        change_info[chan_idx].nim_index = ATOI(argv[2]);
        change_info[chan_idx].nim_type = ATOI(argv[3]);
        change_info[chan_idx].hwdmx_id = ATOI(argv[4]);
        change_info[chan_idx].video_type = ATOI(argv[5]);
        change_info[chan_idx].audio_type = ATOI(argv[6]);
        change_info[chan_idx].qam_mode = ATOI(argv[7]);
        change_info[chan_idx].frequence = ATOI(argv[8]);
        change_info[chan_idx].symrate = ATOI(argv[9]);
        change_info[chan_idx].freq_band = ATOI(argv[9]);
        change_info[chan_idx].video_pid = ATOI(argv[10]);
        change_info[chan_idx].audio_pid = ATOI(argv[11]);
        change_info[chan_idx].pcr_pid = ATOI(argv[12]);
        /* print channel change config info */
        AUI_PRINTF("chan_idx=ch_mode,nim_idx,nim_type,hwdmx_id,vtype,atype,qam,freq,symrate,vpid,apid,ppid\n");
        AUI_PRINTF("----------------------------------------------------------------------------------------\n");
        for (i = 0; i < CHANNEL_NUM_MAX; i++) {
            if (0 != change_info[i].frequence) {
                AUI_PRINTF("channel%d=%7d,%7d,%8d", i, change_info[i].ch_ch_mode, change_info[i].nim_index, change_info[i].nim_type);
                AUI_PRINTF(",%8d,%5d,%5d", change_info[i].hwdmx_id, change_info[i].video_type, change_info[i].audio_type);
                AUI_PRINTF(",%3d,%4d,%7d", change_info[i].qam_mode, change_info[i].frequence, change_info[i].symrate);
                AUI_PRINTF(",%4d,%4d,%4d\n", change_info[i].video_pid, change_info[i].audio_pid, change_info[i].pcr_pid);
            }
        }

        /* check frequence  */
        if (0 == change_info[chan_idx].frequence) {
            AUI_PRINTF("%d NOT support! Please specify the correct frequence.\n", change_info[chan_idx].frequence);
            goto EXIT_FAIL;
        }
    } else if (1 == *argc) {
        /* 
              * 0: use config file (Linux), use the default config but do not modify (TDS);
              * 1: usr set all config params;
              * other value: wrong param
              */
        g_config_info_flag = ATOI(argv[0]);

        /* check g_config_info_flag */
        if (1 < g_config_info_flag) {
            AUI_PRINTF("%d NOT support! Please specify g_config_info_flag in [0, 1].\n", g_config_info_flag);
            goto EXIT_FAIL;
        }
    } else {
        goto EXIT_FAIL;
    }

    return 0;
EXIT_FAIL:
    channel_change_2_help();
    return 1;
}

static unsigned long set_channel_change_config_info(unsigned long *argc, char **argv, char *sz_out_put)
{
    if ((NULL == argc) || (NULL == *argv) || (NULL == sz_out_put)) {
        AUI_PRINTF("The input parameter of %s is illegal\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    *sz_out_put = 0;

    /* get the input param */
    if (get_2_input_param(argc, argv)) {
        goto EXIT_FAIL;
    }

    return 0;
EXIT_FAIL:
    return 1;
}

void test_channel_change_reg()
{
    aui_tu_reg_group("ch", "channel change test");
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_channel_change_help, "channel change help");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_channel_change, "test channel change");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, set_channel_change_config_info, "set channel change config info");
}

