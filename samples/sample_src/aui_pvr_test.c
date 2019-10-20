/****************************INCLUDE HEAD FILE************************************/
#include "aui_pvr_test.h"
#include <aui_ca_pvr.h>
#include <aui_dsc.h>
#include <api/libsi/si_tdt.h>
#ifndef AUI_LINUX
#include <api/libc/string.h>
#include <api/libc/printf.h>
#endif
#include "aui_test_stream.h"
#include "aui_qt_pvr_board_test.h"
#include <aui_dmx.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include "aui_help_print.h"
#include <bus/sci/sci.h>
#include "aui_pvr_reencrypt_test.h"

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/


/****************************LOCAL VAR********************************************/

static int g_aui_pvr_reencrypt_mode = AUI_PVR_NONE;

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************EXTERN VAR*******************************************/
//extern aui_handle_deca *g_p_hdl_deca;
extern aui_hdl ali_qt_recorder;
extern aui_hdl ali_qt_player;
extern int qt_test_timeshift_flag; //value to indicate qt timeshift burn-in test
/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/

#define TEST_PVR_FUNC 1
aui_hdl g_p_hdl_pvr;
int t_index = 0;
unsigned int max_index = 0;
unsigned char *pvr_buffer = NULL;
unsigned int pvr_buffer_len = 20 * 1024 * 1024;
static int dsc_dev = 0;
static unsigned int dsc_algo = 4; /* CSA */
static int dsc_type = 0; /* TS */
static int need_close = 0;

#ifdef AUI_TDS
extern void get_local_time(date_time *dt);
#else
void get_local_time(date_time *dt)
{
    (void)dt;
}
#endif


extern unsigned long pvr_pre(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    t_index--;

    if (t_index <= 1) {
        t_index = 1;
    }

#endif
    return ret;
}

extern unsigned long pvr_next(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    t_index++;
    //max_index = pvr_get_rl_count();
    aui_pvr_get(NULL, AUI_PVR_ITEMS_NUM, &max_index, NULL, NULL);
    /*
    if(index > max_index+1){
        index = max_index+1;
    }
    */
#endif
    return ret;
}

extern unsigned long test_pvr_init(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_init_param ini_param;
    MEMSET(&ini_param, 0, sizeof(ini_param));
#ifdef ONE_RECODER_PVR
    ini_param.max_rec_number = 1;   // max recorder number
#else
    ini_param.max_rec_number = 2;       // max recorder number
#endif
    ini_param.max_play_number = 1;
    //ini_param.max_rec_number=init_cfg.max_rec_number;
    ini_param.ac3_decode_support = 1;
    ini_param.continuous_tms_en = 0;
    //ini_param.debug_level   = init_cfg.debug_level;
    ini_param.debug_level   = AUI_PVR_DEBUG_NONE;
    STRCPY(ini_param.dvr_path_prefix, "ALIDVRS2");
    //STRCPY(ini_param.dvr_path_prefix,init_cfg.dvr_path_prefix);

    STRCPY(ini_param.info_file_name, "info.dvr");
    //STRCPY(ini_param.info_file_name,init_cfg.info_file_name);

    STRCPY(ini_param.info_file_name_new, "info3.dvr");
    //STRCPY(ini_param.info_file_name_new,init_cfg.info_file_name_new);
    STRCPY(ini_param.ts_file_format, "dvr");
    //STRCPY(ini_param.ts_file_format,ini_param.ts_file_format);

    STRCPY(ini_param.ts_file_format_new, "ts");
    //STRCPY(ini_param.ts_file_format_new, "ts");

    STRCPY(ini_param.ps_file_format, "mpg");
    //STRCPY(ini_param.ps_file_format,init_cfg.ps_file_format);

    STRCPY(ini_param.test_file1, "test_write1.dvr");
    //STRCPY(ini_param.test_file1,init_cfg.test_file1);
    STRCPY(ini_param.test_file2, "test_write2.dvr");
    //STRCPY(ini_param.test_file2,init_cfg.test_file2);
    STRCPY(ini_param.storeinfo_file_name, "storeinfo.dvr");

    ini_param.h264_vobu_time_len = 500; // in ms, scope: 500-2000ms, recommend to 600ms
    //ini_param.h264_vobu_time_len = init_cfg.h264_vobu_time_len;   // in ms, scope: 500-2000ms, recommend to 600ms
    ini_param.scramble_vobu_time_len = 600; //in ms, scope: 500-2000ms, recommend to 600ms
    //ini_param.scramble_vobu_time_len = init_cfg.scramble_vobu_time_len;   //in ms, scope: 500-2000ms, recommend to 600ms
    ini_param.file_header_save_dur = 30;// in seconds, recomment to 30s, file head save cycle, min duration is 15s.
    //ini_param.file_header_save_dur = init_cfg.file_header_save_dur;// in seconds, recomment to 30s, file head save cycle, min duration is 15s.
    ini_param.record_min_len = 15;      // in seconds, recomment to 15s, the record will be deleted if shorter that this limit
    //ini_param.record_min_len = init_cfg.record_min_len;       // in seconds, recomment to 15s, the record will be deleted if shorter that this limit
    ini_param.tms_time_max_len = 7200;  // in seconds, recomment to 2h(7200);
    //ini_param.tms_time_max_len = init_cfg.tms_time_max_len;   // in seconds, recomment to 2h(7200);
    ini_param.tms_file_min_size = 2;    // in MBytes,  recomment to 10M
    //ini_param.tms_file_min_size = init_cfg.tms_file_min_size; // in MBytes,  recomment to 10M
    ini_param.prj_mode  = AUI_PVR_DVBS2;  //8.why is PVR_DVBS
    ret = aui_pvr_init(&ini_param);
#endif
    return ret;
}

unsigned long test_pvr_disk_attach(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_disk_attach_info p_apart_param;
    MEMSET(&p_apart_param, 0, sizeof(aui_pvr_disk_attach_info));
    STRCPY(p_apart_param.mount_name, "/mnt/uda1");

    p_apart_param.disk_usage = AUI_PVR_REC_AND_TMS_DISK;
    //pvr_reg_info.disk_usage = p_apart_param->disk_usage;
    p_apart_param.sync = 1;
    //pvr_reg_info.sync = p_apart_param->sync;
    p_apart_param.init_list = 1;
    //pvr_reg_info.init_list = p_apart_param->init_list;
    p_apart_param.check_speed = 1;
    //pvr_reg_info.check_speed = p_apart_param->check_speed;
    ret = aui_pvr_disk_attach(&p_apart_param);
    AUI_SLEEP(1000);
    //max_index = pvr_get_rl_count();
    aui_pvr_get(NULL, AUI_PVR_ITEMS_NUM, &max_index, NULL, NULL);
    t_index = 1;
#endif
    return ret;
}


void test_aui_pvr_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void *user_data)
{
    /*for timeshift burn-in test : pvr recover live play when receive event PVR_END_DATAEND*/
    if ((msg_type == AUI_EVNT_PVR_END_DATAEND || msg_type == AUI_EVNT_PVR_END_READFAIL) && qt_test_timeshift_flag == 1) {
        unsigned int index;
        aui_pvr_pid_info aui_pid_info;
        aui_pvr_rec_item_info rec_info;

        if (ali_qt_recorder != NULL && ali_qt_player != NULL) {
            AUI_PRINTF("APP receive msg: %d ,recorder hdl: 0x%x  player hdl: 0x%x \n", (unsigned int)ali_qt_recorder, (unsigned int)ali_qt_player);

            /*get current play item's idx and pid info*/
            if (AUI_RTN_SUCCESS != aui_pvr_get((aui_hdl)ali_qt_player, AUI_PVR_PLAY_INDEX, &index, NULL, NULL)) {
                AUI_PRINTF("test_aui_pvr_callback get index fail\n");
            }

            if (AUI_RTN_SUCCESS != aui_pvr_get((aui_hdl)ali_qt_player, AUI_PVR_PID_INFO, (unsigned int *)&aui_pid_info, 0, 0)) {
                AUI_PRINTF("test_aui_pvr_callback get pid info fail\n");
            }

            if (AUI_RTN_SUCCESS != aui_pvr_get((aui_hdl)ali_qt_player, AUI_PVR_REC_ITEM_INFO, &index, (unsigned int *)&rec_info, 0)) {
                AUI_PRINTF("test_aui_pvr_callback get record item info fail\n");
            }

            if ((index == 1) && (msg_type == AUI_EVNT_PVR_END_DATAEND || msg_type == AUI_EVNT_PVR_END_READFAIL)) { // index = 1 means the record item is tms
                if (AUI_RTN_SUCCESS != ali_pvr_play_close(ali_qt_player)) {
                    AUI_PRINTF("test_aui_pvr_callback close player fail\n");
                }

                if (AUI_RTN_SUCCESS != ali_pvr_record_close(&ali_qt_recorder)) {
                    AUI_PRINTF("test_aui_pvr_callback close recorder fail\n");
                }

                AUI_PRINTF("tms reach end or read file fail,recover live play\n");
                AUI_PRINTF("vpid: %d ,apid: %d ,vtype: %d ,atype: %d ,ppid: %d \n", aui_pid_info.video_pid, aui_pid_info.audio_pid[0], \
                           rec_info.video_type, aui_pid_info.audio_type[0], aui_pid_info.pcr_pid);
                ali_recover_live_play(aui_pid_info.video_pid, aui_pid_info.audio_pid[0], rec_info.video_type, aui_pid_info.audio_type[0], aui_pid_info.pcr_pid);
                qt_test_timeshift_flag = 0;
            }
        }
    }

    if (AUI_PVR_VMX_MULTI_RE_ENCRYPTION == g_aui_pvr_reencrypt_mode) {
        pvr_reencrypt_callback(handle, msg_type, msg_code, user_data);
    }

}

extern unsigned long test_pvr_tms_open(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_record_prog_param st_arp;
    date_time start_ptm;
    MEMSET(&st_arp, 0, sizeof(st_arp));
    st_arp.av_flag = 1;
    //st_arp.ca_mode = 1;
    st_arp.ca_mode = 0;
    st_arp.dmx_id = 0;
    get_local_time(&start_ptm);

    sprintf(st_arp.folder_name, "/[TS]%4d-%02d-%02d.%02d.%02d.%02d-%s-%2d",
            start_ptm.year,
            start_ptm.month,
            start_ptm.day,
            start_ptm.hour,
            start_ptm.min,
            start_ptm.sec,
            "JMSKTY",
#ifndef AUI_LINUX
            (int)RAND(100));
#else
            rand() % 100);
#endif

    //STRCPY(st_arp.folder_name,"/[TS]2013-11-01-13-32-30");
    st_arp.full_path = 0;
    //st_arp.h264_flag = 1;
    st_arp.h264_flag = 0;
    //st_arp.is_reencrypt = 1;
    st_arp.is_reencrypt = 0;
    //st_arp.is_tms_record = AUI_REC_MODE_NORMAL;AUI_REC_MODE_TMS
    st_arp.is_tms_record = AUI_REC_MODE_TMS;
    st_arp.live_dmx_id = 0;
    st_arp.pid_info.total_num = 32;
    //st_arp.pid_info
    st_arp.pid_info.total_num = 0;
    st_arp.pid_info.audio_count = 1;
    //st_arp.pid_info.addition_pid_count =1;
    st_arp.pid_info.video_pid = 408;
    st_arp.pid_info.audio_pid[0] = 356;
    st_arp.pid_info.audio_lang[0] = 19491;
    st_arp.pid_info.pcr_pid = 7190;
    st_arp.pid_info.pmt_pid = 32;
    st_arp.pid_info.sdt_pid = 17;
    st_arp.pid_info.eit_pid = 18;
    st_arp.pid_info.cat_pid = 8191;
    st_arp.pid_info.nit_pid = 16;
    //st_arp.pid_info.addition_pid[0]= 20;
    st_arp.channel_id = 0x21e00c;
    st_arp.audio_channel = 2;
    //st_arp.rec_special_mode =RSM_CAS9_MULTI_RE_ENCRYPTION;RSM_NONE
    st_arp.rec_special_mode = AUI_PVR_NONE;
    st_arp.rec_type = AUI_PVR_REC_TYPE_TS;
    STRCPY(st_arp.service_name, "DVT");
    ret = aui_pvr_rec_open(&st_arp, &g_p_hdl_pvr);

#endif
    return ret;
}

unsigned long test_pvr_ts_open(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_record_prog_param st_arp;
    date_time start_ptm;
    MEMSET(&st_arp, 0, sizeof(st_arp));
    st_arp.av_flag = 1;
    //st_arp.ca_mode = 1;
    st_arp.ca_mode = 0;
    st_arp.dmx_id = 0;
    get_local_time(&start_ptm);

    sprintf(st_arp.folder_name, "/[TS]%4d-%02d-%02d.%02d.%02d.%02d-%s-%2d",
            start_ptm.year,
            start_ptm.month,
            start_ptm.day,
            start_ptm.hour,
            start_ptm.min,
            start_ptm.sec,
            "DVT",
#ifndef AUI_LINUX
            (int)RAND(100));
#else
            rand() % 100);
#endif

    //STRCPY(st_arp.folder_name,"/[TS]2013-11-01-13-32-30");
    st_arp.full_path = 0;
    //st_arp.h264_flag = 1;
    st_arp.h264_flag = 0;
    //st_arp.is_reencrypt = 1;
    st_arp.is_reencrypt = 0;
    //st_arp.is_tms_record = AUI_REC_MODE_NORMAL;AUI_REC_MODE_TMS
    st_arp.is_tms_record = AUI_REC_MODE_NORMAL;
    st_arp.live_dmx_id = 0;
    st_arp.pid_info.total_num = 32;
    //st_arp.pid_info
    st_arp.pid_info.total_num = 0;
    st_arp.pid_info.audio_count = 1;
    //st_arp.pid_info.addition_pid_count =1;
    st_arp.pid_info.video_pid = 408;
    st_arp.pid_info.audio_pid[0] = 356;
    st_arp.pid_info.audio_lang[0] = 19491;
    st_arp.pid_info.pcr_pid = 7190;
    st_arp.pid_info.pmt_pid = 32;
    st_arp.pid_info.sdt_pid = 17;
    st_arp.pid_info.eit_pid = 18;
    st_arp.pid_info.cat_pid = 8191;
    st_arp.pid_info.nit_pid = 16;
    //st_arp.pid_info.addition_pid[0]= 20;
    st_arp.channel_id = 0x21e00c;
    st_arp.audio_channel = 2;
    //st_arp.rec_special_mode =RSM_CAS9_MULTI_RE_ENCRYPTION;RSM_NONE
    st_arp.rec_special_mode = AUI_PVR_NONE;
    st_arp.rec_type = AUI_PVR_REC_TYPE_TS;
    STRCPY(st_arp.service_name, "DVT");
    ret = aui_pvr_rec_open(&st_arp, &g_p_hdl_pvr);

#endif
    return ret;
}

extern unsigned long test_pvr_e_ts_open(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_record_prog_param st_arp;
    date_time start_ptm;
    MEMSET(&st_arp, 0, sizeof(st_arp));
    st_arp.av_flag = 1;
    //st_arp.ca_mode = 1;
    st_arp.ca_mode = 0;
    st_arp.dmx_id = 0;
    get_local_time(&start_ptm);

    sprintf(st_arp.folder_name, "/[TS]%4d-%02d-%02d.%02d.%02d.%02d-%s-%2d",
            start_ptm.year,
            start_ptm.month,
            start_ptm.day,
            start_ptm.hour,
            start_ptm.min,
            start_ptm.sec,
            "DVT",
#ifndef AUI_LINUX
            (int)RAND(100));
#else
            rand() % 100);
#endif

    //STRCPY(st_arp.folder_name,"/[TS]2013-11-01-13-32-30");
    st_arp.full_path = 0;
    //st_arp.h264_flag = 1;
    st_arp.h264_flag = 0;
    //st_arp.is_reencrypt = 1;

    st_arp.is_reencrypt = 1;
    st_arp.rec_special_mode = AUI_PVR_COMMON_RE_ENCRYPTION;
    st_arp.is_scrambled = 0;

    //st_arp.is_tms_record = AUI_REC_MODE_NORMAL;AUI_REC_MODE_TMS
    st_arp.is_tms_record = AUI_REC_MODE_NORMAL;
    st_arp.pid_info.total_num = 0;
    st_arp.pid_info.audio_count = 1;
    //st_arp.pid_info.addition_pid_count =1;
    st_arp.pid_info.video_pid = 544;
    st_arp.pid_info.audio_pid[0] = 545;
    st_arp.pid_info.audio_lang[0] = 19491;
    st_arp.pid_info.pcr_pid = 544;
    st_arp.pid_info.pmt_pid = 34;
    st_arp.pid_info.sdt_pid = 17;
    st_arp.pid_info.eit_pid = 18;
    st_arp.pid_info.cat_pid = 8191;
    st_arp.pid_info.nit_pid = 16;
    //st_arp.pid_info.addition_pid[0]= 20;
    //st_arp.channel_id = 0x21e00c;
    st_arp.channel_id = 0x242c02;
    st_arp.audio_channel = 2;
    //st_arp.rec_special_mode =RSM_CAS9_MULTI_RE_ENCRYPTION;RSM_NONE

    st_arp.rec_type = AUI_PVR_REC_TYPE_TS;
    STRCPY(st_arp.service_name, "DVT");
    ret = aui_pvr_rec_open(&st_arp, &g_p_hdl_pvr);

#endif
    return ret;
}
extern unsigned long test_pvr_e_tms_open(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_record_prog_param st_arp;
    date_time start_ptm;
    MEMSET(&st_arp, 0, sizeof(st_arp));
    st_arp.av_flag = 1;
    //st_arp.ca_mode = 1;
    st_arp.ca_mode = 0;
    st_arp.dmx_id = 0;
    get_local_time(&start_ptm);

    sprintf(st_arp.folder_name, "/[TS]%4d-%02d-%02d.%02d.%02d.%02d-%s-%2d",
            start_ptm.year,
            start_ptm.month,
            start_ptm.day,
            start_ptm.hour,
            start_ptm.min,
            start_ptm.sec,
            "DVT",
#ifndef AUI_LINUX
            (int)RAND(100));
#else
            rand() % 100);
#endif

    //STRCPY(st_arp.folder_name,"/[TS]2013-11-01-13-32-30");
    st_arp.full_path = 0;
    //st_arp.h264_flag = 1;
    st_arp.h264_flag = 0;
    //st_arp.is_reencrypt = 1;

    st_arp.is_reencrypt = 1;
    st_arp.rec_special_mode = AUI_PVR_COMMON_RE_ENCRYPTION;
    st_arp.is_scrambled = 0;

    //st_arp.is_tms_record = AUI_REC_MODE_NORMAL;AUI_REC_MODE_TMS
    st_arp.is_tms_record = AUI_REC_MODE_TMS;
    st_arp.live_dmx_id = 0;
    /*
    st_arp.pid_info.total_num =0;
    st_arp.pid_info.audio_count =1;
    st_arp.pid_info.addition_pid_count =1;
    st_arp.pid_info.video_pid =408;
    st_arp.pid_info.audio_pid[0]=356;
    st_arp.pid_info.audio_lang[0] = 19491;
    st_arp.pid_info.pcr_pid =7190;
    st_arp.pid_info.pmt_pid = 32;
    st_arp.pid_info.sdt_pid =17;
    st_arp.pid_info.eit_pid =18;
    st_arp.pid_info.cat_pid =8191;
    st_arp.pid_info.nit_pid =16;
    st_arp.pid_info.addition_pid[0]= 20;
    */
    st_arp.pid_info.total_num = 0;
    st_arp.pid_info.audio_count = 1;
    //st_arp.pid_info.addition_pid_count =1;
    st_arp.pid_info.video_pid = 544;
    st_arp.pid_info.audio_pid[0] = 545;
    st_arp.pid_info.audio_lang[0] = 19491;
    st_arp.pid_info.pcr_pid = 544;
    st_arp.pid_info.pmt_pid = 34;
    st_arp.pid_info.sdt_pid = 17;
    st_arp.pid_info.eit_pid = 18;
    st_arp.pid_info.cat_pid = 8191;
    st_arp.pid_info.nit_pid = 16;
    //st_arp.pid_info.addition_pid[0]= 20;
    //st_arp.channel_id = 0x21e00c;
    st_arp.channel_id = 0x242c02;
    st_arp.audio_channel = 2;
    //st_arp.rec_special_mode =RSM_CAS9_MULTI_RE_ENCRYPTION;RSM_NONE

    st_arp.rec_type = AUI_PVR_REC_TYPE_TS;
    STRCPY(st_arp.service_name, "DVT");
    ret = aui_pvr_rec_open(&st_arp, &g_p_hdl_pvr);

#endif
    return ret;
}

static void pvr_test_print_speed(unsigned int nspeed)
{
    char speedbuf[64] = {0};

    switch (nspeed) {
    case AUI_PLAYER_SPEED_FASTFORWARD_2:
        strcpy(speedbuf, "FFX2");
        break;

    case AUI_PLAYER_SPEED_FASTFORWARD_4:
        strcpy(speedbuf, "FFX4");
        break;

    case AUI_PLAYER_SPEED_FASTFORWARD_8:
        strcpy(speedbuf, "FFX8");
        break;

    case AUI_PLAYER_SPEED_FASTFORWARD_16:
        strcpy(speedbuf, "FFX16");
        break;

    case AUI_PLAYER_SPEED_FASTFORWARD_32:
        strcpy(speedbuf, "FFX32");
        break;

    case AUI_PLAYER_SPEED_FASTFORWARD_64:
        strcpy(speedbuf, "FFX64");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_1:
        strcpy(speedbuf,"FBX1");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_2:
        strcpy(speedbuf, "FBX2");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_4:
        strcpy(speedbuf, "FBX4");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_8:
        strcpy(speedbuf, "FBX8");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_16:
        strcpy(speedbuf, "FBX16");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_32:
        strcpy(speedbuf, "FBX32");
        break;

    case AUI_PLAYER_SPEED_FASTREWIND_64:
        strcpy(speedbuf, "FBX64");
        break;

    case AUI_PLAYER_SPEED_SLOWREWIND_2:
        strcpy(speedbuf, "RESLOWX2");
        break;

    case AUI_PLAYER_SPEED_SLOWREWIND_4:
        strcpy(speedbuf, "RESLOWX4");
        break;

    case AUI_PLAYER_SPEED_SLOWREWIND_8:
        strcpy(speedbuf, "RESLOWX8");
        break;

    case AUI_PLAYER_SPEED_SLOWFORWARD_2:
        strcpy(speedbuf, "SLOWX2");
        break;

    case AUI_PLAYER_SPEED_SLOWFORWARD_4:
        strcpy(speedbuf, "SLOWX4");
        break;

    case AUI_PLAYER_SPEED_SLOWFORWARD_8:
        strcpy(speedbuf, "SLOWX8");
        break;

    default:
        return ;

    }

    AUI_PRINTF("**************play speed is :%s**************\n", speedbuf);
}

unsigned long test_pvr_rec_change(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_rec_state_change(g_p_hdl_pvr, AUI_REC_STATE_PAUSE);
    aui_pvr_rec_state_change(g_p_hdl_pvr, AUI_REC_STATE_RECORDING);
    aui_pvr_rec_state_change(g_p_hdl_pvr, AUI_REC_STATE_STOP);
#endif
    return ret;
}

extern unsigned long test_pvr_rec_pause(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_rec_state_change(g_p_hdl_pvr, AUI_REC_STATE_PAUSE);
#endif
    return ret;
}

extern unsigned long test_pvr_rec_resume(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_rec_state_change(g_p_hdl_pvr, AUI_REC_STATE_RECORDING);
#endif
    return ret;
}


unsigned long test_pvr_rec_close(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    ret = aui_pvr_rec_close(g_p_hdl_pvr, 1);

    if (ret == AUI_RTN_SUCCESS) {
        g_p_hdl_pvr = 0;
    }

#endif
    return ret;
}

unsigned long test_pvr_play_open(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_ply_param st_app;
    MEMSET(&st_app, 0, sizeof(st_app));
    st_app.dmx_id = 2;
    st_app.index = t_index;
    st_app.live_dmx_id = 0;
    //STRCPY(st_app.path,"/mnt/uda1/ALIDVRS2/[TS]2013-11-01-13-32-30");
    st_app.preview_mode = 0;
    st_app.speed  = AUI_PVR_PLAY_SPEED_1X;
    st_app.start_mode = 4;
    st_app.start_pos = 0 ;
    st_app.start_time = 0;
    st_app.state = AUI_PVR_PLAY_STATE_STEP;
    ret = aui_pvr_ply_open(&st_app, &g_p_hdl_pvr);
#endif
    return ret;
}

extern unsigned long test_pvr_play_play(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_ply_state_change(g_p_hdl_pvr, AUI_PVR_PLAY_STATE_PLAY, 0);
#endif
    return ret;
}

extern unsigned long test_pvr_play_pause(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_ply_state_change(g_p_hdl_pvr, AUI_PVR_PLAY_STATE_PAUSE, 0);
#endif
    return ret;
}

extern unsigned long test_pvr_play_FF(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_ply_state_change(g_p_hdl_pvr, AUI_PVR_PLAY_STATE_FF, 0);
#endif
    return ret;
}

extern unsigned long test_pvr_play_FB(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_ply_state_change(g_p_hdl_pvr, AUI_PVR_PLAY_STATE_FB, 0);
#endif
    return ret;
}

extern unsigned long test_pvr_play_stop(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_ply_state_change(g_p_hdl_pvr, AUI_PVR_PLAY_STATE_STOP, 0);
#endif
    return ret;
}

unsigned long test_pvr_play_close(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    aui_pvr_stop_ply_param st_apsp;
    st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
    st_apsp.sync = TRUE;
    st_apsp.vpo_mode = 0;
    ret = aui_pvr_ply_close(g_p_hdl_pvr, &st_apsp);
#endif
    return ret;
}

unsigned long test_pvr_play_change(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC

#endif
    return ret;
}

unsigned long test_pvr_get(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC

#endif
    return ret;
}

unsigned long test_pvr_set(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC

#endif
    return ret;
}

unsigned long test_pvr_register(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    ret = aui_pvr_callback_register(g_p_hdl_pvr, test_aui_pvr_callback);
#endif
    return ret;
}

unsigned long test_pvr_unregister(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
#if TEST_PVR_FUNC
    ret = aui_pvr_callback_unregister(g_p_hdl_pvr, test_aui_pvr_callback);
#endif
    return ret;
}


int ali_pvr_init()
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_pvr_init_param ini_param;
    pvr_buffer = (unsigned char *)malloc(pvr_buffer_len);

    if (pvr_buffer == NULL) {
        AUI_PRINTF("pvr_buffer is %p\n", pvr_buffer);
        return 1;
    } else {
        AUI_PRINTF("pvr_buffer is %p\n", pvr_buffer);
    }

    memset(&ini_param, 0, sizeof(ini_param));
    ini_param.max_rec_number = 2;
    ini_param.max_play_number = 1;
    ini_param.ac3_decode_support = 1;
    ini_param.trim_record_ptm = 0;
    ini_param.continuous_tms_en = 0;
    ini_param.debug_level   = AUI_PVR_DEBUG_NONE;
    STRCPY(ini_param.dvr_path_prefix, "ALIDVRS2");
    STRCPY(ini_param.info_file_name, "info.dvr");
    STRCPY(ini_param.info_file_name_new, "info3.dvr");
    STRCPY(ini_param.ts_file_format, "dvr");
    STRCPY(ini_param.ts_file_format_new, "ts");
    STRCPY(ini_param.ps_file_format, "mpg");
    STRCPY(ini_param.test_file1, "test_write1.dvr");
    STRCPY(ini_param.test_file2, "test_write2.dvr");
    STRCPY(ini_param.storeinfo_file_name, "storeinfo.dvr");
    ini_param.record_min_len = 15;      // in seconds, recomment to 15s, the record will be deleted if shorter that this limit
    ini_param.tms_time_max_len = 7200;  // in seconds, recomment to 2h(7200);
    ini_param.tms_file_min_size = 2;    // in MBytes,  recomment to 10M
    ini_param.prj_mode  = AUI_PVR_DVBS2;
    ini_param.cache_addr = (unsigned int)pvr_buffer;
    ini_param.cache_size = pvr_buffer_len;


    ret = aui_pvr_init(&ini_param);

    if (ret != AUI_RTN_SUCCESS) {
        return 1;
    }

    return 0;
}

unsigned long ali_pvr_attach_disk(char *diskname)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_pvr_disk_attach_info p_apart_param;
    MEMSET(&p_apart_param, 0, sizeof(aui_pvr_disk_attach_info));
    STRCPY(p_apart_param.mount_name, diskname);
    p_apart_param.disk_usage = AUI_PVR_REC_AND_TMS_DISK;
    p_apart_param.sync = 1;
    p_apart_param.init_list = 1;
    p_apart_param.check_speed = 1;
    ret = aui_pvr_disk_attach(&p_apart_param);
    return ret;
}

/* get TS DSC (virtual) device handle*/
static unsigned short ali_pvr_get_ts_dsc_handle_callback(unsigned short program_id, aui_hdl *p_dsc_handler)
{
    aui_hdl hdl;

    if (aui_find_dev_by_idx(AUI_MODULE_DSC, dsc_dev, &hdl)) {
        AUI_PRINTF("aui_find_dev_by_idx fault\n");
        return -1;
    }

    *p_dsc_handler = hdl;
    return 0;
}

/* open CSA sub device by index 0 */
static int ali_pvr_open_csa_sub_device()
{
    aui_attr_dsc attr;
    aui_hdl hdl;

    attr.uc_dev_idx = dsc_dev;
    attr.uc_algo = dsc_algo;
    attr.dsc_data_type = dsc_type;

    if (aui_find_dev_by_idx(AUI_MODULE_DSC, attr.uc_dev_idx, &hdl)) {
        AUI_PRINTF("cannot find dsc device, ready to open it\n");

        if (aui_dsc_open(&attr, &hdl)) {
            AUI_PRINTF("dsc open error\n");
            return 1;
        }

        AUI_PRINTF("it is a FTA stream, need close CSA device when stop record\n");
        need_close = 1;
    }

    AUI_PRINTF("find the used CSA device\n");
    return 0;
}

static int ali_pvr_close_csa_sub_device()
{
    aui_hdl hdl;

    if (0 == need_close) {
        return 0;
    }

    if (aui_find_dev_by_idx(AUI_MODULE_DSC, dsc_dev, &hdl)) {
        AUI_PRINTF("aui_find_dev_by_idx fault\n");
        return 1;
    }

    need_close = 0;
    return aui_dsc_close(hdl);
}


unsigned long ali_pvr_record_open(aui_hdl *aui_pvr_handler, unsigned int dmx_id, unsigned int video_pid, unsigned int video_type,
                                  unsigned int audio_pid_count, unsigned int *audio_pid, unsigned int *audio_type, unsigned int pcr_pid,
                                  aui_pvr_rec_mode rec_type, int encrypt, int is_ca_mode, unsigned char *record_path)
{
    unsigned int i = 0;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_record_prog_param st_arp;
    aui_ca_pvr_callback ca_pvr_callback;
#ifdef AUI_LINUX
    aui_ca_pvr_config config;
    MEMSET(&config, 0, sizeof(aui_ca_pvr_config));
#endif
    MEMSET(&st_arp, 0, sizeof(st_arp));

    if (video_pid == AUI_INVALID_PID) {
        st_arp.av_flag = 0;
    } else {
        st_arp.av_flag = 1;
    }

    st_arp.dmx_id = dmx_id;
    st_arp.is_tms_record = rec_type;
    //st_arp.channel_id = 37780486;
    //st_arp.audio_channel  = 2;
    //st_arp.pid_info.total_num =0;
    st_arp.pid_info.audio_count = audio_pid_count;
    st_arp.h264_flag = video_type;
    st_arp.fn_callback = test_aui_pvr_callback;
    st_arp.pid_info.video_pid = video_pid;
    st_arp.pid_info.pcr_pid = pcr_pid;

    for (i = 0; i < audio_pid_count; i++) {
        st_arp.pid_info.audio_pid[i] = audio_pid[i];
        st_arp.pid_info.audio_type[i] = audio_type[i];
    }

    st_arp.rec_type = AUI_PVR_REC_TYPE_TS;

    switch (encrypt) {
    case 0: //raw_fta_2_fta
        st_arp.is_reencrypt = 0;
        st_arp.rec_special_mode = AUI_PVR_NONE;
        st_arp.is_scrambled = 0;
        break;

    case 1: //for FTA to re-encrypt, fixed ali internal key, en_fta_2_encrypt_ali_key
        st_arp.is_reencrypt = 1;
        st_arp.rec_special_mode = AUI_PVR_CAS9_RE_ENCRYPTION;
        st_arp.is_scrambled = 0;
        break;

    case 2:  //FOR conax, fixed ali internal key , en_ali_key
        st_arp.is_reencrypt = 1;
        st_arp.rec_special_mode = AUI_PVR_CAS9_RE_ENCRYPTION;
        st_arp.is_scrambled = 0;
        break;

    case 3://for Nagra
        st_arp.is_reencrypt = 1;
        st_arp.rec_special_mode = AUI_RSM_C0200A_MULTI_RE_ENCRYPTION;
        st_arp.is_scrambled = 0;
        break;

    case 4: //for en_raw_fta_2_en_raw_fta
        st_arp.is_reencrypt = 0;
        st_arp.rec_special_mode = AUI_PVR_NONE;
        st_arp.is_scrambled = 1;
        break;

    case 5://for genca
        st_arp.is_reencrypt = 1;
        st_arp.rec_special_mode = AUI_RSM_GEN_CA_MULTI_RE_ENCRYPTION;
        st_arp.is_scrambled = 0;
        break;

    case 6:
        st_arp.is_reencrypt = 1;
        st_arp.rec_special_mode = AUI_PVR_VMX_MULTI_RE_ENCRYPTION;
        st_arp.is_scrambled = 0;
        break;

    default:
        return 1;
    }

    st_arp.ca_mode = is_ca_mode == 1 ? 1 : 0;
    pvr_reencrypt_mode_init(AUI_PVR_NONE);

    if (6 == encrypt) {
        st_arp.fn_callback = test_aui_pvr_callback;
        g_aui_pvr_reencrypt_mode = AUI_PVR_VMX_MULTI_RE_ENCRYPTION;
        pvr_reencrypt_mode_init(AUI_PVR_VMX_MULTI_RE_ENCRYPTION);

        pvr_sample_mode_set(REC_MODE);

        if (0 != pvr_reencrypt_module_init()) {
            AUI_PRINTF("pvr_reencrypt_module_init failed\n");
            return 1;
        }
    }

    else if (encrypt) {
        if (0 != ali_pvr_open_csa_sub_device()) {
            AUI_PRINTF("open_csa_sub_device failed\n");
            return 1;
        }

#ifndef AUI_LINUX
        aui_ca_pvr_init();
#else
        config.special_mode = st_arp.rec_special_mode;
        aui_ca_pvr_init_ext(&config);
#endif
        ca_pvr_callback.fp_pure_data_callback = NULL;
        ca_pvr_callback.fp_ts_callback = ali_pvr_get_ts_dsc_handle_callback;
        aui_ca_register_callback(&ca_pvr_callback);
    }


    if (record_path == NULL) {
        date_time start_ptm;
        get_local_time(&start_ptm);
        sprintf(st_arp.folder_name, "/%4d-%02d-%02d.%02d.%02d.%02d-%s-%2d",
                start_ptm.year,
                start_ptm.month,
                start_ptm.day,
                start_ptm.hour,
                start_ptm.min,
                start_ptm.sec,
                "DVT",
#ifndef AUI_LINUX
                (int)RAND(100));
#else
                rand() % 100);
#endif
    } else {
        sprintf(st_arp.folder_name, "/%s", record_path);
    }

    AUI_PRINTF("pvr recorder filename:%s\n", st_arp.folder_name);
    ret = aui_pvr_rec_open(&st_arp, aui_pvr_handler);
    AUI_PRINTF("pvr recorder ret:%d\n", ret);

    if (ret != AUI_RTN_SUCCESS) {
        *aui_pvr_handler = 0;
        return 1;
    }

    return 0;
}

unsigned long ali_pvr_record_close(aui_hdl *aui_pvr_handler)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    ret = aui_pvr_rec_close(*aui_pvr_handler, 1);

    pvr_reencrypt_module_deinit();

    if (ret == AUI_RTN_SUCCESS) {
        *aui_pvr_handler = 0;
        return 1;
    }

    if (0 != ali_pvr_close_csa_sub_device()) {
        AUI_PRINTF("close_csa_sub_device failed\n");
        return 1;
    }

    return 0;
}

unsigned long ali_pvr_play_open(aui_hdl *player_handler, int record_index, char *filename, unsigned char start_mode)
{
    int ret = 0;

    aui_ply_param st_app;
    MEMSET(&st_app, 0, sizeof(st_app));

    st_app.fn_callback = test_aui_pvr_callback;

    pvr_sample_mode_set(PLAYBACK_MODE);

    if (0 != pvr_reencrypt_module_init()) {
        AUI_PRINTF("pvr_reencrypt_module_init failed\n");
        return -1;
    };

    st_app.dmx_id = 2;

    st_app.index = record_index;

    st_app.live_dmx_id = 0;

    if (filename != NULL) {
        STRCPY(st_app.path, filename);
    }

    st_app.preview_mode = 0;
    st_app.speed  = AUI_PVR_PLAY_SPEED_1X;
    AUI_PRINTF("start_mode :0x%x\n", start_mode);
    st_app.start_mode = start_mode;
    st_app.start_pos = 0 ;
    st_app.start_time = 0;
    st_app.state = AUI_PVR_PLAY_STATE_PLAY;

    //ret = aui_pvr_ply_open(&st_app,&player_handler);
    if (AUI_RTN_SUCCESS != aui_pvr_ply_open(&st_app, player_handler)) {
        ret = -1;
    }

    return ret;
}

unsigned long ali_pvr_play_play(aui_hdl player_handler)
{
    int ret = 0;

    if (AUI_RTN_SUCCESS != aui_pvr_ply_state_change(player_handler, AUI_PVR_PLAY_STATE_PLAY, 0)) {
        ret = -1;
    }

    return ret;
}

unsigned long ali_pvr_play_pause(aui_hdl player_handler)
{
    int ret = 0;

    if (AUI_RTN_SUCCESS != aui_pvr_ply_state_change(player_handler, AUI_PVR_PLAY_STATE_PAUSE, 0)) {
        ret = -1;
    }

    return ret;
}

unsigned long ali_pvr_play_close(aui_hdl player_handler)
{
    int ret = 0;

    aui_pvr_stop_ply_param st_apsp;
    st_apsp.stop_mode = AUI_PVR_STOPPED_ONLY;
    st_apsp.sync = TRUE;
    st_apsp.vpo_mode = 0;

    if (AUI_RTN_SUCCESS != aui_pvr_ply_close(player_handler, &st_apsp)) {
        ret = -1;
    }

    pvr_reencrypt_module_deinit();

    return ret;
}

unsigned long ali_pvr_deinit()
{
    aui_pvr_deinit();

    return 0;
}

unsigned long ali_stop_live_play(void)
{
    aui_hdl dmx_hdl;
    aui_hdl deca_hdl;
    aui_hdl decv_hdl;

    //disable dmx
    if (aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &dmx_hdl) == AUI_RTN_SUCCESS) {
        if (AUI_RTN_SUCCESS != aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_DISABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_DISABLE fail\n");
            return -1;
        }
    }

    //stop deca
    if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl) == AUI_RTN_SUCCESS) {
        if (AUI_RTN_SUCCESS != aui_deca_stop(deca_hdl, NULL)) {
            AUI_PRINTF("aui_deca_stop fail\n");
            return -1;
        }
    }

    //stop decv
    if (aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl) == AUI_RTN_SUCCESS) {
        if (AUI_RTN_SUCCESS != aui_decv_stop(decv_hdl)) {
            AUI_PRINTF("aui_decv_stop fail\n");
            return -1;
        }
    }

    return 0;
}

unsigned long ali_recover_live_play(int vedio_pid, int auido_pid, unsigned int vedio_type, unsigned int audio_type, unsigned int pcr_pid)
{
    aui_hdl dmx_hdl;
    aui_hdl deca_hdl;
    aui_hdl decv_hdl;
    //aui_attr_dis attr_dis;
    //aui_hdl dis_hdl=0;
    //unsigned char uc_dev_idx = 0;
    aui_dmx_stream_pid pid_list;
    aui_dmx_data_path path;

    MEMSET(&pid_list, 0, sizeof(aui_dmx_stream_pid));
#if 0//def AUI_LINUX

    //able dmx
    if (aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &dmx_hdl) == AUI_RTN_SUCCESS) {
        if (AUI_RTN_SUCCESS != aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            return -1;
        }
    }

    //start deca
    if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl) == AUI_RTN_SUCCESS) {
        if (AUI_RTN_SUCCESS != aui_deca_start(deca_hdl, NULL)) {
            AUI_PRINTF("aui_deca_start fail\n");
            return -1;
        }
    }

    //start decv
    if (aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl) == AUI_RTN_SUCCESS) {
        if (AUI_RTN_SUCCESS != aui_decv_start(decv_hdl)) {
            AUI_PRINTF("aui_deca_start fail\n");
            return -1;
        }
    }

#else
#if 0
    MEMSET(&attr_dis, 0, sizeof(aui_attr_dis));

    attr_dis.uc_dev_idx = vedio_type;

    if (aui_dis_open(&attr_dis, &dis_hdl)) {/*if dis hasn't opened,open dis device and return dis device handle*/
        AUI_PRINTF("\n aui_dis_open fail\n");
        return -1;
    }

#ifdef AUI_LINUX

    if (aui_dis_layer_enable(dis_hdl, 0, 0)) {
        AUI_PRINTF("\n aui_dis_video_enable fail\n");
        return -1;
    }

#else

    if (aui_dis_video_enable(dis_hdl, 0)) {
        AUI_PRINTF("\n aui_dis_video_enable fail\n");
        return -1;
    }

#endif
#endif
    pid_list.ul_pid_cnt = 4;
    pid_list.stream_types[0] = AUI_DMX_STREAM_VIDEO;
    pid_list.stream_types[1] = AUI_DMX_STREAM_AUDIO;
    pid_list.stream_types[2] = AUI_DMX_STREAM_PCR;
    pid_list.stream_types[3] = AUI_DMX_STREAM_AUDIO_DESC;

    pid_list.aus_pids_val[0] = vedio_pid;
    pid_list.aus_pids_val[1] = auido_pid;
    pid_list.aus_pids_val[2] = pcr_pid;
    pid_list.aus_pids_val[3] = 0x1fff;
    AUI_PRINTF("\r\nhdl:video_pid,audio_pid,pcr_pid,audio_desc_pid=[%d][%d][%d][%d]", vedio_pid, auido_pid, pcr_pid, 0x1fff);

    //start deca
    if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, &deca_hdl) == AUI_RTN_SUCCESS) {
        aui_deca_type_set(deca_hdl, audio_type);

        if (AUI_RTN_SUCCESS != aui_deca_start(deca_hdl, NULL)) {
            AUI_PRINTF("aui_deca_start fail\n");
            return -1;
        }
    }

    if (vedio_type == 3) {
        vedio_type = 10;
    }

    //start decv
    if (aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl) == AUI_RTN_SUCCESS) {
        aui_decv_decode_format_set(decv_hdl, vedio_type);

        if (AUI_RTN_SUCCESS != aui_decv_start(decv_hdl)) {
            AUI_PRINTF("aui_deca_start fail\n");
            return -1;
        }
    }

    if (aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &dmx_hdl) == AUI_RTN_SUCCESS) {
        if (vedio_pid != 0x1fff && auido_pid != 0x1fff) {
            if (aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list)) {
                AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
                aui_dmx_close(dmx_hdl);
                return -1;
            }

            AUI_PRINTF("\r\n aui dmx set create av\n");
        } else if (auido_pid != 0x1fff) {
            if (aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_CREATE_AUDIO, &pid_list)) {
                AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AUDIO fail\n");
                aui_dmx_close(dmx_hdl);
                return -1;
            }

            AUI_PRINTF("\r\n aui dmx set create audio\n");
        } else {
            if (aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_CREATE_VIDEO, &pid_list)) {
                AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_VIDEO fail\n");
                aui_dmx_close(dmx_hdl);
                return -1;
            }

            AUI_PRINTF("\r\n aui dmx set create video\n");
        }

        //if live is screambled ts,custumer need to set data path to AUI_DMX_DATA_PATH_DE_PLAY
        //sample code as blew:
        /*memset(&path, 0, sizeof(aui_dmx_data_path));
        path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
        path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
         path.p_hdl_de_dev = hdl_dsc_dec;//aui dsc handle
         //path.p_hdl_en_dev = hdl_dsc_enc;
         if (aui_dmx_data_path_set(hdl_dmx, &path)) {
         printf("aui_dmx_data_path_set failed\n");
         return -1;
         }else {
         printf("aui_dmx_data_path_set success\n");
         }
         */
        MEMSET(&path, 0, sizeof(aui_dmx_data_path));
        path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;

        if (aui_dmx_data_path_set(dmx_hdl, &path)) {
            AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
            return -1;
        }

        AUI_PRINTF("dmx data path set %d\n", path.data_path_type);

        if (vedio_pid != 0x1fff && auido_pid != 0x1fff) {
            if (aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
                AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                return -1;
            }

            AUI_PRINTF("\r\n aui dmx set enable av");
        } else if (auido_pid != 0x1fff) {
            if (aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_ENABLE_AUDIO, NULL)) {
                AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
                return -1;
            }

            AUI_PRINTF("\r\n aui dmx set enable audio");
        } else {
            if (aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL)) {
                AUI_PRINTF("aui_dmx_set  AUI_DMX_STREAM_ENABLE failed\n");
                return -1;
            }

            AUI_PRINTF("\r\n aui dmx set enable video");
        }
    }

#endif
    return 0;
}

unsigned long pvr_test_init(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = 0;

    if (0 != ali_pvr_init()) {
        AUI_PRINTF("ali_pvr_init failed\n");
        ret = 1;
        aui_pvr_deinit();
    }

    return ret;
}

unsigned long pvr_test_deinit(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = 0;

    if (0 != aui_pvr_deinit()) {
        AUI_PRINTF("ali_pvr_deinit failed\n");
        ret = 1;
    }

    if (pvr_buffer != NULL) {
        free(pvr_buffer);
        pvr_buffer = NULL;
    }


    return ret;
}

unsigned long pvr_test_attach(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = 0;
    unsigned char *mount_name = NULL;
    AUI_TEST_CHECK_NULL(argc);
    AUI_TEST_CHECK_NULL(argv);
    AUI_TEST_CHECK_NULL(sz_out_put);

    if (*argc < 1) {
        AUI_PRINTF("\r\nattach command format:attach usb_disk_mount_path.\r\n");
#ifdef AUI_LINUX
        AUI_PRINTF("\r\nFor example:attach /mnt/usb/sda1.\r\n");
#else
        AUI_PRINTF("\r\nFor example:attach /mnt/uda1.\r\n");
#endif
        return 1;
    }

    mount_name = argv[0];
    AUI_PRINTF("\r\n disk:%s will be attached!\r\n", mount_name);

    if (0 != ali_pvr_attach_disk(mount_name)) {
        AUI_PRINTF("attached disk failed\n");
        ret = 1;
    }

    return ret;
}

unsigned long pvr_test_record(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    unsigned int pos = 0;
    unsigned int i = 0;
    unsigned char *filename = NULL;
    unsigned int vcount = 0;
    unsigned int vpid = 0x1fff;
    unsigned int vtype = 0;
    unsigned int acount = 0;
    unsigned int apids[10] = {0x1fff,};
    unsigned int atypes[10] = {0,};
    unsigned int pcr_pid = 0;
    unsigned int is_reencrypt = 0;
    unsigned int duration = 0;

    AUI_TEST_CHECK_NULL(argc);
    AUI_TEST_CHECK_NULL(argv);
    AUI_TEST_CHECK_NULL(sz_out_put);

    if (*argc < 7) {
        AUI_PRINTF("\r\n record command format:record filename,video_count,video_pid,video_type,audio_count,audio_pid0,audio_type0,audio_pid1,audio_type1,...,pcr_pid,ecnrypt_mode.\r\n");
        AUI_PRINTF("\r\n For example:record pvr_001,1,513,0,1,660,0,8190,0.\r\n");
        AUI_PRINTF("\r\n video_type:\r\n0:MPEG2;\r\n 1:H264;\r\n2:ACS;\r\n3:H265\r\n");
        AUI_PRINTF("\r\n encrypt mode:\r\n0:FTA;\r\n 1:FTA to re-ecrypt;\r\n2:for conax 6;\r\n3:for nagura\r\n4:for raw ts record\r\n5:for gen ca ts record\r\n");
        return 1;
    }


    filename = argv[0];
    AUI_PRINTF("\r\n filename:%s\r\n", filename);
    //get video info
    vcount = ATOI(argv[1]);
    AUI_PRINTF("\r\n vcount:%d\r\n", vcount);

    if (vcount == 0) {
        AUI_PRINTF("\r\n Record no video!\r\n");
        pos = 2;
    } else if (vcount == 1) {
        vpid = ATOI(argv[2]);
        vtype = ATOI(argv[3]);
        AUI_PRINTF("\r\n vpid:%d vpid:%d\r\n", vpid, vtype);
        pos = 4;
    } else {
        AUI_PRINTF("\r\n Video count is error!\r\n");
        return 1;
    }

    //get audio info
    acount = ATOI(argv[pos++]);

    if (*argc != 5 + 2 * vcount + 2 * acount) {
        AUI_PRINTF("\r\n the video count & auidio count error \r\n");
        return 1;
    }

    AUI_PRINTF("\r\n acount : %d \r\n", acount);

    if ((acount <= 0) || (AUI_MAX_PVR_AUDIO_PID <= acount)) {
        AUI_PRINTF("\r\n audio count is error!\r\n");
        return 1;
    } else {
        for (i = 0; i < acount; i++) {
            apids[i] = ATOI(argv[pos++]);
            atypes[i] = ATOI(argv[pos++]);
            AUI_PRINTF("\r\n apids[%d] : %d \r\n", i, apids[i]);
            AUI_PRINTF("\r\n atypes[%d] : %d \r\n", i, atypes[i]);
        }
    }

    //get pcr info
    pcr_pid = ATOI(argv[pos++]);
    //get reencrypt info
    is_reencrypt = ATOI(argv[pos]);
    AUI_PRINTF("\r\n pcr_pid : %d \r\n", pcr_pid);
    AUI_PRINTF("\r\n is_reencrypt : %d \r\n", is_reencrypt);

    AUI_PRINTF("********************pvr record start********************************\n");
    aui_hdl aui_pvr_handler = NULL;
    AUI_PRINTF("start recording....\n");

    if (0 != ali_pvr_record_open(&aui_pvr_handler, 0 /* dmx_id */, vpid /* video pid*/, vtype /* video type*/,
                                 acount /* audio count*/, apids /* audio pid*/, atypes /* audio type*/, pcr_pid /* pcr pid*/,
                                 AUI_REC_MODE_NORMAL /* rec mode*/, is_reencrypt /* is reencrypt*/, 0 /* ca mode*/, filename /* file name*/)) {
        AUI_PRINTF("ali_pvr_record_open failed\n");
        ret = 1;
        return ret;
    }

    if (is_reencrypt == 6) {
        AUI_PRINTF("************************PVR recod is configured****************************\n");
        AUI_PRINTF("PVR re-encrypt is changing key each 10s automaticlly\n Press 's' to stop streaming\n ");
        pvr_crypto_key_change_record_run(aui_pvr_handler);

    } else {
        AUI_PRINTF("************************PVR recod is configured****************************\n");
        AUI_PRINTF("Press 's' to stop streaming\n 'p' to pause the record\n 'r' resume to record\n 't' to get record duration\n");
    }

    char ch = 0;

    while (1) {
        aui_test_get_user_str_input(&ch);
        AUI_PRINTF("ch is %c\n", ch);

        //AUI_PRINTF("ch is %d %02x\n",ch,ch);
        if (ch == 's') { /* stop stream */
            if (is_reencrypt == 6) {
                pvr_crypto_key_change_record_stop();
            }

            break;
        }

        if (is_reencrypt != 6) {
            if (ch == 'p') { /* pause record */
                aui_pvr_rec_state_change(aui_pvr_handler, AUI_PVR_REC_STATE_PAUSE);
                aui_pvr_get(aui_pvr_handler, AUI_PVR_REC_TIME_S, &duration, 0, 0);
                AUI_PRINTF("************pvr reocrd pause at [%d]\n", duration);
                continue;
            }

            if (ch == 'r') { /* resume record */
                aui_pvr_rec_state_change(aui_pvr_handler, AUI_PVR_REC_STATE_RECORDING);
                aui_pvr_get(aui_pvr_handler, AUI_PVR_REC_TIME_S, &duration, 0, 0);
                AUI_PRINTF("************pvr reocrd resume at [%d]\n", duration);
                continue;
            }

            if (ch == 't') {
                aui_pvr_get(aui_pvr_handler, AUI_PVR_REC_TIME_S, &duration, 0, 0);
                AUI_PRINTF("*******PVR recorder current duration is [%d]*******\n", duration);
                continue;
            }

            if (ch == 'k') {
                pvr_reencrypt_key_change();
                continue;
            }
        }

    }

    aui_pvr_get(aui_pvr_handler, AUI_PVR_REC_TIME_S, &duration, 0, 0);
    AUI_PRINTF("******PVR recorder duration[%d]********\n record is close!\n", duration);

    if (aui_pvr_handler != NULL) {
        ali_pvr_record_close(&aui_pvr_handler);
    }

    return ret;
}

unsigned long pvr_test_playback(unsigned long *argc, char **argv, char *sz_out_put)
{
    int i = 0;
    int ret = 0;
    int index = 2 ;
    int cur_apid_idx = 0 ;
    unsigned char *path = NULL;
    aui_pvr_pid_info aui_pid_info;
    unsigned int duarion = 0;
    int ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;
    int fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;
    int slow_speed = AUI_PLAYER_SPEED_SLOWFORWARD_2;

    MEMSET(&aui_pid_info, 0, sizeof(aui_pvr_pid_info));
    AUI_PRINTF("********************pvr play start********************************\n");
    aui_hdl aui_pvr_handler = NULL;

    if (*argc < 2) {
        AUI_PRINTF("\r\n play command format:play index,path.\r\n");
        AUI_PRINTF("\r\nFor example:play 2,0.\r\n");
#ifdef AUI_LINUX
        AUI_PRINTF("\r\nFor example:play 0,/mnt/usb/sda1/ALIDVRS2/pvr_001.\r\n");
#else
        AUI_PRINTF("\r\nFor example:play 0,/mnt/uda1/ALIDVRS2/pvr_001.\r\n");
#endif
        return 1;
    }

    //AUI_TEST_CHECK_VAL(*argc,2);
    if (2 != *argc && 3 != *argc) {
        AUI_PRINTF("%s:%d %s() value missmatch %d != 2(3)\n", __FILE__, __LINE__,
                   __FUNCTION__, *argc);
        return AUI_RTN_EINVAL;
    }

    //stop live play as live dmx and soft dmx send data to see at the same time will cause mosaic
    ali_stop_live_play();

    index = ATOI(argv[0]);
    path = argv[1];

    if (index == 0 && path != NULL) {
        if (AUI_RTN_SUCCESS != aui_pvr_get_index_by_path(path, &index)) {
            AUI_PRINTF("\r\nThe file %s doesn't exist!\r\n", path);
            return 1;
        }
    }

    int encrypt = 0;

    if (3 == *argc) {
        encrypt = ATOI(argv[2]);
    }

    if (6 == encrypt) {
        g_aui_pvr_reencrypt_mode = AUI_PVR_VMX_MULTI_RE_ENCRYPTION;
        pvr_reencrypt_mode_init(AUI_PVR_VMX_MULTI_RE_ENCRYPTION);
    } else {
        g_aui_pvr_reencrypt_mode = AUI_PVR_NONE;
        pvr_reencrypt_mode_init(AUI_PVR_NONE);
    }

    if (0 != ali_pvr_play_open(&aui_pvr_handler, index,
                               path, AUI_P_OPEN_FROM_HEAD)) {
        AUI_PRINTF("ali_pvr_play_open failed\n");
        ret = 1;
        goto exit;
    }

    AUI_PRINTF("handler[%08x] opend!\n", aui_pvr_handler);

    if (encrypt == 6) {
        AUI_PRINTF("PVR re-encrypt is changing key each 10s automaticlly.\n Press 's' to stop streaming\n");
        pvr_crypto_key_change_playback_run(aui_pvr_handler);
    } else {
        AUI_PRINTF("Press 's' to stop streaming\n");
        AUI_PRINTF("Press 'p' to play \n");
        AUI_PRINTF("Press 'd' to pause\n");
        AUI_PRINTF("Press 't' to get current play time\n");
        AUI_PRINTF("Press 'f' for forward (press again for multip FF)\n");
        AUI_PRINTF("Press 'b' for backward (press again for multip FB)\n");
        AUI_PRINTF("Press 'w' for slow (press again for multip slow)\n");
        AUI_PRINTF("Press 'r' for reslow (press again for multip reslow)\n");
        AUI_PRINTF("Press 'x' play step\n");
        AUI_PRINTF("Press 'j' seek from head position\n");
        AUI_PRINTF("Press 'g' for get pid info\n");
        AUI_PRINTF("Press 'c' for change audio pid\n");
    }

    char ch = 0;

    while (1) {
        aui_test_get_user_str_input(&ch);
        AUI_PRINTF("ch is %c\n", ch);

        if (ch == 's')  { /* stop stream */
            if (encrypt == 6) {
                pvr_crypto_key_change_playbck_stop();
            }

            break;
        }

        if (encrypt != 6) {
            if (ch == 'p') {
                AUI_PRINTF("\n****************************** handler[%08x] start playing **********************\n", aui_pvr_handler);
                ali_pvr_play_play(aui_pvr_handler);
                ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;
                fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;
                slow_speed = AUI_PLAYER_SPEED_SLOWFORWARD_2;
                continue;
            }

            if (ch == 'd') {
                AUI_PRINTF("\n****************************** pause ***************************\n");
                ali_pvr_play_pause(aui_pvr_handler);
                continue;
            }

            if (ch == 't') {
                AUI_PRINTF("\n****************************** get play time **********************\n");
                aui_pvr_get(aui_pvr_handler, AUI_PVR_PLAY_TIME_S, &duarion, 0, 0);
                AUI_PRINTF("get current playback time :%d s\n", duarion);
                continue;
            }

            if (ch == 'f') {
                AUI_PRINTF("\n****************************** fast forward ***************************\n");
                pvr_test_print_speed(ff_speed);
                aui_pvr_ply_state_change(aui_pvr_handler, AUI_PLY_STATE_SPEED, ff_speed);

                if (ff_speed < AUI_PLAYER_SPEED_FASTFORWARD_64)
                    ff_speed++;
                else
                    ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;

                continue;
            }

            if (ch == 'b') {
                AUI_PRINTF("\n****************************** backward ***************************\n");
                pvr_test_print_speed(fb_speed);
                aui_pvr_ply_state_change(aui_pvr_handler, AUI_PLY_STATE_SPEED, fb_speed);

                if (fb_speed > AUI_PLAYER_SPEED_FASTREWIND_64)
                    fb_speed--;
                else
                    fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;

                continue;
            }

            if (ch == 'w') {
                AUI_PRINTF("\n****************************** slow ***************************\n");
                pvr_test_print_speed(slow_speed);
                aui_pvr_ply_state_change(aui_pvr_handler, AUI_PLY_STATE_SPEED, slow_speed);

                if (slow_speed < AUI_PLAYER_SPEED_SLOWFORWARD_8)
                    slow_speed++;
                else
                    slow_speed = AUI_PLAYER_SPEED_SLOWFORWARD_2;

                continue;
            }

            if (ch == 'x') {
                AUI_PRINTF("\n****************************** play step ***************************\n");
                aui_pvr_ply_state_change(aui_pvr_handler, AUI_PVR_PLAY_STATE_STEP, 0);
                continue;
            }

            if (ch == 'j') {
                AUI_PRINTF("\n****************************** seek from head position ***************************\n");
                unsigned long nPosInSec = 0;
                AUI_PRINTF("pls input seek time(s):\n");
                aui_test_get_user_dec_input(&nPosInSec);
                AUI_PRINTF("seek time is %ds\n", nPosInSec);
                aui_pvr_ply_seek(aui_pvr_handler, nPosInSec, AUI_PVR_PLAY_POSITION_FROM_HEAD);
                continue;
            }

            if (ch == 'g') {
                AUI_PRINTF("\n****************************** get pids info ***************************\n");
                aui_pvr_get(aui_pvr_handler, AUI_PVR_PID_INFO, (unsigned int *)&aui_pid_info, 0, 0);

                for (i = 0; i < aui_pid_info.audio_count; i++) {
                    AUI_PRINTF("\r\nv a_pid[%d]:%d \r\n", i, aui_pid_info.audio_pid[i]);
                }

                AUI_PRINTF("\r\nv_pid:%d a_pid:%d p_pid:%d \r\n", aui_pid_info.video_pid, aui_pid_info.audio_pid[0], aui_pid_info.pcr_pid);
                continue;
            }

            if (ch == 'c') {
                AUI_PRINTF("\n****************************** change audio pid ***************************\n");

                if (0 == aui_pid_info.audio_count) {
                    AUI_PRINTF("\r\nv Please input g to get pids info first.\r\n");
                } else if (1 == aui_pid_info.audio_count) {
                    AUI_PRINTF("\n****************************** get pids info ***************************\n");
                    aui_pvr_get(aui_pvr_handler, AUI_PVR_PID_INFO, (unsigned int *)&aui_pid_info, 0, 0);

                    for (i = 0; i < aui_pid_info.audio_count; i++) {
                        AUI_PRINTF("\r\nv a_pid[%d]:%d \r\n", i, aui_pid_info.audio_pid[i]);
                    }

                    AUI_PRINTF("\r\nv Current program has only one audio pid cann't change.\r\n");
                }

                cur_apid_idx++;
                cur_apid_idx = (cur_apid_idx == aui_pid_info.audio_count) ? 0 : cur_apid_idx ;
                aui_pvr_set(aui_pvr_handler, AUI_PVR_AUDIO_PID_CHANGE, aui_pid_info.audio_pid[cur_apid_idx], aui_pid_info.audio_type[cur_apid_idx], 0);
                AUI_PRINTF("\r\n cur_apid_idx: %d nv_pid:%d a_pid:%d a_type: %d p_pid:%d \r\n", cur_apid_idx, aui_pid_info.video_pid, aui_pid_info.audio_pid[cur_apid_idx], aui_pid_info.audio_type[cur_apid_idx], aui_pid_info.pcr_pid);
                continue;
            }

            if (ch == 'k') {
                pvr_reencrypt_key_change();
                continue;
            }

        }
    }

    if (aui_pvr_handler != NULL) {
        ali_pvr_play_close(aui_pvr_handler);
    }

exit:
    //recover live play when playback finish
    //ali_recover_livr_play();

    return ret;
}

unsigned long pvr_test_timeshift(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_PRINTF("********************pvr timeshift start********************************\n");
    int ret = 0;
    unsigned int pos = 0;
    unsigned int i = 0;
    unsigned int duration = 0;
    unsigned long nPosInSec = 0;
    unsigned char *filename = NULL;
    unsigned int vcount = 0;
    unsigned int vpid = 0x1fff;
    unsigned int vtype = 0;
    unsigned int acount = 0;
    unsigned int apids[10] = {0x1fff,};
    unsigned int atypes[10] = {0,};
    unsigned int pcr_pid = 0;
    unsigned int is_reencrypt = 0;
    unsigned int duarion = 0;
    aui_hdl aui_recorder = NULL;
    aui_hdl aui_player = NULL;
    aui_hdl p_hdl_deca = NULL;
    aui_hdl p_hdl_snd = NULL;
    unsigned long rec_time = 0;
    unsigned int ply_time = 0;
    int ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;
    int fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;
    char ch = 0;

    AUI_TEST_CHECK_NULL(argc);
    AUI_TEST_CHECK_NULL(argv);
    AUI_TEST_CHECK_NULL(sz_out_put);

    if (*argc < 7) {
        AUI_PRINTF("\r\n timeshift command format:timeshift filename,video_count,video_pid,video_type,audio_count,audio_pid0,audio_type0,audio_pid1,audio_type1,...,pcr_pid,ecnrypt_mode.\r\n");
        AUI_PRINTF("\r\n For example:timeshift pvr_001,1,513,0,1,660,0,8190,0.\r\n");
        AUI_PRINTF("\r\n video_type:\r\n0:MPEG2;\r\n 1:H264;\r\n2:ACS;\r\n3:H265\r\n");
        AUI_PRINTF("\r\n encrypt mode:\r\n0:FTA;\r\n 1:FTA to re-ecrypt;\r\n2:for conax 6;\r\n3:for nagura\r\n4:for raw ts record\r\n");
        return 1;
    }

    filename = argv[0];
    //get video info
    vcount = ATOI(argv[1]);

    if (vcount == 0) {
        AUI_PRINTF("\r\n Record no video!\r\n");
        pos = 2;
    } else if (vcount == 1) {
        vpid = ATOI(argv[2]);
        vtype = ATOI(argv[3]);
        AUI_PRINTF("\r\n vpid:%d vpid:%d\r\n", vpid, vtype);
        pos = 4;
    } else {
        AUI_PRINTF("\r\n Video count is error!\r\n");
        return 1;
    }

    //get audio info
    acount = ATOI(argv[pos++]);
    AUI_TEST_CHECK_VAL(*argc, 5 + 2 * vcount + 2 * acount);

    if (acount <= 0) {
        AUI_PRINTF("\r\n audio count is error!\r\n");
        return 1;
    } else {
        for (i = 0; i < acount; i++) {
            apids[i] = ATOI(argv[pos++]);
            atypes[i] = ATOI(argv[pos++]);
        }
    }

    //get pcr info
    pcr_pid = ATOI(argv[pos++]);
    //get reencrypt info
    is_reencrypt = ATOI(argv[pos]);

    AUI_PRINTF("input the duration between record and playback: (>5s)\n");
    aui_test_get_user_dec_input(&rec_time);
    AUI_PRINTF("start recording....\n");

    if (0 != ali_pvr_record_open(&aui_recorder, 0 /* dmx_id */, vpid /* video pid*/, vtype /* video type*/,
                                 acount /* audio count*/, apids /* audio pid*/, atypes /* audio type*/, pcr_pid /* pcr pid*/,
                                 AUI_REC_MODE_TMS /* rec mode*/, is_reencrypt /* is reencrypt*/, 0 /* ca mode*/, filename /* file name*/)) {
        AUI_PRINTF("ali_pvr_record_open failed\n");
        aui_recorder = NULL;
        ret = 1;
        goto exit;
    }

    AUI_PRINTF("************************PVR recorder start****************************\n");

    //For low bitrate stream,if the start play time is too fast maybe no enough data to send to dmx,
    //and may case block for a while,to avoid this can set a longer start play time.
    while (duarion < rec_time) {
        aui_pvr_get(aui_recorder, AUI_PVR_REC_TIME_S, &duarion, 0, 0);
        AUI_PRINTF("************************PVR recorder duarion[%d]****************************\n", duarion);
        AUI_SLEEP(1 * 1000);
    }

    ali_stop_live_play();
    AUI_PRINTF("************************PVR player playback****************************\n");

    if (0 != ali_pvr_play_open(&aui_player, 1,
                               NULL, AUI_P_OPEN_FROM_HEAD)) {
        AUI_PRINTF("ali_pvr_play_open failed\n");
        aui_player = NULL;
        ret = 1;
        goto exit;
    }

    //AUI_SLEEP(60);
    AUI_PRINTF("Press 's' to stop streaming\n");
    AUI_PRINTF("Press 'd' to pause\n");
    AUI_PRINTF("Press 'p' to play\n");
    AUI_PRINTF("Press 't' to get current play time\n");
    AUI_PRINTF("Press 'f' to fast forward (press again for multip FF)\n");
    AUI_PRINTF("Press 'b' to  fast backward (press again for multip FB)\n");
    AUI_PRINTF("Press 'a' to input the seconds to seek\n");

    while (1) {
        aui_test_get_user_str_input(&ch);

        if (ch == 's') /* stop stream */
            break;

        if (ch == 'd') { /* pause stream */
            AUI_PRINTF("\n****************************** pause ***************************\n");
            ali_pvr_play_pause(aui_player);
            continue;
        }

        if (ch == 't') {
            AUI_PRINTF("\n****************************** get play time **********************\n");
            aui_pvr_get(aui_player, AUI_PVR_PLAY_TIME_S, &ply_time, 0, 0);
            AUI_PRINTF("get current playback time :%d s\n", ply_time);
            continue;
        }

        if (ch == 'p') { /* play stream */
            AUI_PRINTF("\n****************************** play ***************************\n");
            ali_pvr_play_play(aui_player);
            ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;
            fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;
            continue;
        }

        if (ch == 'f') {
            AUI_PRINTF("\n****************************** fast forward ***************************\n");
            pvr_test_print_speed(ff_speed);
            aui_pvr_ply_state_change(aui_player, AUI_PLY_STATE_SPEED, ff_speed);

            if (ff_speed < AUI_PLAYER_SPEED_FASTFORWARD_64)
                ff_speed++;
            else
                ff_speed = AUI_PLAYER_SPEED_FASTFORWARD_2;

            continue;
        }

        if (ch == 'b') {
            AUI_PRINTF("\n******************************  fast backward  ***************************\n");
            pvr_test_print_speed(fb_speed);
            aui_pvr_ply_state_change(aui_player, AUI_PLY_STATE_SPEED, fb_speed);

            if (fb_speed > AUI_PLAYER_SPEED_FASTREWIND_64)
                fb_speed--;
            else
                fb_speed = AUI_PLAYER_SPEED_FASTREWIND_1;

            continue;
        }

        if (ch == 'a') {
            AUI_PRINTF("\n****************************** seek to time ******************************\n");
            aui_pvr_get(aui_recorder, AUI_PVR_REC_TIME_S, &duration, 0, 0);
            AUI_PRINTF("Current record duration: %d,input the time(s) to seek:\n", duration);
            aui_test_get_user_dec_input(&nPosInSec);

            if (nPosInSec > duration) {
                AUI_PRINTF("time(s) larger than record duration:\n");
                continue;
            }

            aui_pvr_ply_state_change(aui_player, AUI_PVR_PLAY_STATE_PLAY, 0); //fix bug: After FF and seek to end ,the frame stop
            aui_pvr_ply_seek(aui_player, nPosInSec, AUI_PVR_PLAY_POSITION_FROM_HEAD);
            continue;
        }

    }

    AUI_PRINTF("record is close!\n");

exit:

    if (aui_player != NULL) {
        ali_pvr_play_close(aui_player);
    }

    if (aui_recorder != NULL) {
        ali_pvr_record_close(&aui_recorder);
    }

    ali_recover_live_play(vpid, apids[0], vtype, atypes[0], pcr_pid);
    return ret;

    if (p_hdl_deca && p_hdl_snd)
        ali_app_snd_deinit(p_hdl_deca, p_hdl_snd);

    return ret;
}

unsigned long pvr_test_fta_or_encrypt(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 1;
    int param = 2;
    aui_attr_dmx attr_dmx;
    aui_hdl aui_dmx_handler;
    AUI_TEST_CHECK_NULL(argc);
    AUI_TEST_CHECK_NULL(argv);
    AUI_TEST_CHECK_NULL(sz_out_put);

    if (*argc < 1) {
        AUI_PRINTF("\r\nconfig command format:config [fta|encrypt].\r\n");
        AUI_PRINTF("\r\nFor example:config fta.\r\n");
        return 1;
    }

    AUI_TEST_CHECK_VAL(*argc, 1);


    memset(&attr_dmx, 0, sizeof(attr_dmx));
    attr_dmx.uc_dev_idx = 0;

    if (aui_find_dev_by_idx(AUI_MODULE_DMX, attr_dmx.uc_dev_idx , &aui_dmx_handler)) {
        if (aui_dmx_open(&attr_dmx, &aui_dmx_handler)) {
            AUI_PRINTF("dmx open failed\n");
            ret = 1;
            return ret;
        }

        AUI_PRINTF("aui_dmx_open success\n");
    } else {
        AUI_PRINTF("dmx %d has already been open\n", attr_dmx.uc_dev_idx);
    }

    char *config = argv[0];

    if (STRCMP(config, "fta") == 0) {
        param = 0;

    }

    if (STRCMP(config, "encrypt") == 0) {
        param = 1;
    }

    if (AUI_RTN_SUCCESS != aui_dmx_set(aui_dmx_handler, AUI_DMX_SET_IO_REC_MODE, &param)) {
        AUI_PRINTF("aui_dmx_set AUI_IO_REC_MODE %s failed!\r\n", param == 0 ? "FTA" : "Encrypt");
        ret = 1;
    } else {
        AUI_PRINTF("aui_dmx_set set %s successfuly!\r\n", param == 0 ? "FTA" : "Encrypt");
        ret = 0;
    }

    if (AUI_RTN_SUCCESS != aui_dmx_get(aui_dmx_handler, AUI_DMX_GET_IO_REC_MODE, &param)) {
        AUI_PRINTF("aui_dmx_get AUI_IO_REC_MODE %s failed!\r\n", param == 0 ? "FTA" : "Encrypt");
        ret = 1;
    } else {
        AUI_PRINTF("aui_dmx_get rec_mode:%s successfuly!\r\n", param == 0 ? "FTA" : "Encrypt");
        ret = 0;
    }

    return ret;
}

unsigned long pvr_test_set(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned long ret = 0;
    int param1 = 0;

    if (*argc < 2) {
        AUI_PRINTF("\r\ncommand format:set threshold,10  //10means 10MB\r\n");
        AUI_PRINTF("\r\nsupport command:threshold,10  //10 means 10MB\r\n");
        return 1;
    }

    char *config = argv[0];

    if (STRCMP(config, "threshold") == 0) {
        param1 = ATOI(argv[1]);

        if (AUI_RTN_SUCCESS != aui_pvr_set((aui_hdl)1, AUI_PVR_FREE_SIZE_LOW_THRESHOLD, param1 * 1024, 0, 0)) {
            return 1;
        }

        param1 = 0;

        if (AUI_RTN_SUCCESS != aui_pvr_get((aui_hdl)1, AUI_PVR_FREE_SIZE_LOW_THRESHOLD, &param1, 0, 0)) {
            return 1;
        }

        AUI_PRINTF("\r\nGet threshold value from PVR:%d\r\n", param1 / 1024);
        return 0;
    }

    return ret;
}

unsigned long pvr_test_help(unsigned long *argc, char **argv, char *sz_out_put)
{
    aui_print_help_header("\nPVR Test Help");

    /* PVR_1_HELP */
    #define     PVR_1_HELP_PART1        "For storing the PVR media file, the storage path must be set firstlly. In general, the storage medium is U Flash Disk, Mobile Hard disk and so on.\n"
    #define     PVR_1_HELP_PART2        "Format:       attach [storage path]\n"
#ifdef AUI_LINUX
    #define     PVR_1_HELP_PART3        "Example:      If the storage path is /usb/sda1/uda1, the input is"
    #define     PVR_1_HELP_PART4        "              attach /usb/sda1/uda1\n"
    #define     PVR_1_HELP_PART5        "Note: In addition to the storage medium, the string \033[1m\"ALIDVRS2\"\033[0m is appended to the path. So the storage full paht is \033[1m\"/mnt/usb/sda1/ALIDVRS2\"\033[0m\n"
#else
    #define     PVR_1_HELP_PART3        "Example:      If the storage path is /mnt/uda1, the input is"
    #define     PVR_1_HELP_PART4        "              attach /mnt/uda1\n"
    #define     PVR_1_HELP_PART5        "Note: In addition to the storage medium, the string \033[1m\"ALIDVRS2\"\033[0m is appended to the path. So the storage full paht is \033[1m\"/mnt/uda1/ALIDVRS2\"\033[0m\n"
#endif
    aui_print_help_command("\'attach\'");
    aui_print_help_instruction_newline("Attach the pvr storage path.\n");
    aui_print_help_instruction_newline(PVR_1_HELP_PART1);
    aui_print_help_instruction_newline(PVR_1_HELP_PART2);
    aui_print_help_instruction_newline(PVR_1_HELP_PART3);
    aui_print_help_instruction_newline(PVR_1_HELP_PART4);
    aui_print_help_instruction_newline(PVR_1_HELP_PART5);

    /* PVR_2_HELP */
    #define     PVR_2_HELP_PART1        "Set the PVR mode to FTA or encrypt.\n"
    #define     PVR_2_HELP_PART2        "Format:       config [record mode]"
    #define     PVR_2_HELP_PART3        "              [record mode]: \"fta\" or \"encrypt\".\n"
    #define     PVR_2_HELP_PART4        "Example:      If the record mode is set to \"encrypt\", the input is"
    #define     PVR_2_HELP_PART5        "              config encrypt\n"

    aui_print_help_command("\'config\'");
    aui_print_help_instruction_newline("Configurate the PVR mode.\n");
    aui_print_help_instruction_newline(PVR_2_HELP_PART1);
    aui_print_help_instruction_newline(PVR_2_HELP_PART2);
    aui_print_help_instruction_newline(PVR_2_HELP_PART3);
    aui_print_help_instruction_newline(PVR_2_HELP_PART4);
    aui_print_help_instruction_newline(PVR_2_HELP_PART5);

    /* PVR_3_HELP */
    #define     PVR_3_HELP_PART1        "To record the video, please follow the below steps:"
    #define     PVR_3_HELP_PART2        "1. Use \"attach\" command to specify the PVR storage path."
    #define     PVR_3_HELP_PART3        "2. Use \"stream\" -> \"play\" command to play a stream"
    #define     PVR_3_HELP_PART4        "3. Use \"pvr\" -> \"record\" command to record the playing stream."
    #define     PVR_3_HELP_PART5        "4. During the recorded, the operation can be "
    #define     PVR_3_HELP_PART6        "        stopped by inputting \'s\', "
    #define     PVR_3_HELP_PART7        "        paused by inputting \'d\', "
    #define     PVR_3_HELP_PART8        "        resumed by inputting \'r\'\n"
    #define     PVR_3_HELP_PART9        "Format:       record [filename],[video count],[video_pid],[video_type],[audio count],[audio_pid0],[audio_type0],[audio_pid1],[audio_type1],...,[pcr_pid],[is_encrypt]"
    #define     PVR_3_HELP_PART10       "              [filename]: the name of the recording file."
    #define     PVR_3_HELP_PART11       "              [video_pid],[audio_pid],[audio_type],[pcr_pid]: The playing stream information can be got from the \"streamXpress\" or internet"
    #define     PVR_3_HELP_PART12       "              [video count]: only support 1  [audio count]: max support 10\n"
    #define     PVR_3_HELP_PART13       "              [video_type]: 0-not h264 or not h265  1-h264 3-h265\n"
    #define     PVR_3_HELP_PART14       "              [is_ecnrypt]: 0-nonencrypt    1-encrypt    6-VMX_MULTI mode encrypt \n"
    #define     PVR_3_HELP_PART15       "Example:      If the playing stream whose video pid is 213, video_type is 0(not h264), audio pid is 214, audio_type is 2(AAC), pcr_id is 213 is recorded non-encrypt to the file named \"record1\", the input is"
    #define     PVR_3_HELP_PART16       "              record record1,1,213,0,1,214,1,213,0\n"

    aui_print_help_command("\'record\'");
    aui_print_help_instruction_newline("Record the video.\n");
    aui_print_help_instruction_newline(PVR_3_HELP_PART1);
    aui_print_help_instruction_newline(PVR_3_HELP_PART2);
    aui_print_help_instruction_newline(PVR_3_HELP_PART3);
    aui_print_help_instruction_newline(PVR_3_HELP_PART4);
    aui_print_help_instruction_newline(PVR_3_HELP_PART5);
    aui_print_help_instruction_newline(PVR_3_HELP_PART6);
    aui_print_help_instruction_newline(PVR_3_HELP_PART7);
    aui_print_help_instruction_newline(PVR_3_HELP_PART8);
    aui_print_help_instruction_newline(PVR_3_HELP_PART9);
    aui_print_help_instruction_newline(PVR_3_HELP_PART10);
    aui_print_help_instruction_newline(PVR_3_HELP_PART11);
    aui_print_help_instruction_newline(PVR_3_HELP_PART12);
    aui_print_help_instruction_newline(PVR_3_HELP_PART13);
    aui_print_help_instruction_newline(PVR_3_HELP_PART14);
    aui_print_help_instruction_newline(PVR_3_HELP_PART15);
    aui_print_help_instruction_newline(PVR_3_HELP_PART16);

    /* PVR_4_HELP */
    #define     PVR_4_HELP_PART1        "There are two method to play record.  1. by recoding index.  2. by storage path.\n"
    #define     PVR_4_HELP_PART2        "Format:       play [index],[path]\n"
    #define     PVR_4_HELP_PART3        "Example:      If the record that the index is 2 and play it, the input is (set [path] to 0)"
    #define     PVR_4_HELP_PART4        "              play 2,0\n"
#ifdef AUI_LINUX
    #define     PVR_4_HELP_PART5        "              If the record that the storage path is /mnt/usb/sda1/ALIDVRS2/record1 and play it, the input is (set [index] to 0)"
    #define     PVR_4_HELP_PART6        "              play 0,/mnt/usb/sda1/ALIDVRS2/record1\n"
#else
    #define     PVR_4_HELP_PART5        "              If the record that the storage path is /mnt/uda1/ALIDVRS2/record1 and play it, the input is (set [index] to 0)"
    #define     PVR_4_HELP_PART6        "              play 0,/mnt/uda1/ALIDVRS2/record1\n"
#endif
    aui_print_help_command("\'play\'");
    aui_print_help_instruction_newline("Play the recorded stream.\n");
    aui_print_help_instruction_newline(PVR_4_HELP_PART1);
    aui_print_help_instruction_newline(PVR_4_HELP_PART2);
    aui_print_help_instruction_newline(PVR_4_HELP_PART3);
    aui_print_help_instruction_newline(PVR_4_HELP_PART4);
    aui_print_help_instruction_newline(PVR_4_HELP_PART5);
    aui_print_help_instruction_newline(PVR_4_HELP_PART6);

    /* PVR_5_HELP */
    #define     PVR_5_HELP_PART1        "To record the playing stream and play the recorded stream, please follow the below steps:"
    #define     PVR_5_HELP_PART2        "1. Use \"attach\" command to specify the PVR storage path."
    #define     PVR_5_HELP_PART3        "2. Use \"stream\" -> \"play\" command to play a stream"
    #define     PVR_5_HELP_PART4        "3. Use \"pvr\" -> \"timeshift\" command to record the playing stream and play the recorded stream."
    #define     PVR_5_HELP_PART5        "4. During the recorded, the operation can be"
    #define     PVR_5_HELP_PART6        "        stopped by inputting \'s\', "
    #define     PVR_5_HELP_PART7        "        paused by inputting \'d\', "
    #define     PVR_5_HELP_PART8        "        played(play the recorded stream after the \"paused\" being inputted ) by inputting \'p\'\n"
    #define     PVR_5_HELP_PART9        "Format:       timeshift [filename],[video count],[video_pid],[video type],[audio count],[audio_pid0],[audio_type0],[audio_pid1],[audio_type1],...,[pcr_pid],[is_encrypt]"
    #define     PVR_5_HELP_PART10       "              [filename]: the name of the recording file."
    #define     PVR_5_HELP_PART11       "              [video_pid],[audio_pid],[audio_type],[pcr_pid]: The playing stream information can be got from the \"streamXpress\" or internet"
    #define     PVR_5_HELP_PART12       "              [video count]: only support 1  [audio count]: max support 10\n"
    #define     PVR_5_HELP_PART13       "              [video ]: 0-not h264 or not h265  1-h264 3-h265\n"
    #define     PVR_5_HELP_PART14       "              [is_ecnrypt]: 0-nonencrypt    1-encrypt\n"
    #define     PVR_5_HELP_PART15       "Example:      If the playing stream whose video pid is 213, video_type is 0(not h264), audio pid is 214, audio_type is 2(AAC), pcr_id is 213 is recorded(timeshift) non-encrypt to the file named \"record1\", the input is"
    #define     PVR_5_HELP_PART16       "              timeshift record1,1,213,0,1,214,1,213,0\n"

    aui_print_help_command("\'timeshift\'");
    aui_print_help_instruction_newline("Record and play the video.\n");
    aui_print_help_instruction_newline(PVR_5_HELP_PART1);
    aui_print_help_instruction_newline(PVR_5_HELP_PART2);
    aui_print_help_instruction_newline(PVR_5_HELP_PART3);
    aui_print_help_instruction_newline(PVR_5_HELP_PART4);
    aui_print_help_instruction_newline(PVR_5_HELP_PART5);
    aui_print_help_instruction_newline(PVR_5_HELP_PART6);
    aui_print_help_instruction_newline(PVR_5_HELP_PART7);
    aui_print_help_instruction_newline(PVR_5_HELP_PART8);
    aui_print_help_instruction_newline(PVR_5_HELP_PART9);
    aui_print_help_instruction_newline(PVR_5_HELP_PART10);
    aui_print_help_instruction_newline(PVR_5_HELP_PART11);
    aui_print_help_instruction_newline(PVR_5_HELP_PART12);
    aui_print_help_instruction_newline(PVR_5_HELP_PART13);
    aui_print_help_instruction_newline(PVR_5_HELP_PART14);
    aui_print_help_instruction_newline(PVR_5_HELP_PART15);
    aui_print_help_instruction_newline(PVR_5_HELP_PART16);


    /* PVR_6_HELP */
#define     PVR_6_HELP      "Delete the PVR running tast.\n"

    aui_print_help_command("\'deinit\'");
    aui_print_help_instruction_newline(PVR_6_HELP);

    return AUI_RTN_HELP;
}

void test_pvr_reg()
{
    aui_tu_reg_group("pvr", "pvr record and playback");
#ifdef AUI_LINUX
    aui_tu_reg_item(2, "init", AUI_CMD_TYPE_API, pvr_test_init, "pvr init");
#endif
    aui_tu_reg_item(2, "attach", AUI_CMD_TYPE_API, pvr_test_attach, "pvr attach disk");
    aui_tu_reg_item(2, "config", AUI_CMD_TYPE_API, pvr_test_fta_or_encrypt, "config pvr FTA or Encrypt");
    aui_tu_reg_item(2, "record", AUI_CMD_TYPE_API, pvr_test_record, "pvr recorder");
    aui_tu_reg_item(2, "play", AUI_CMD_TYPE_API, pvr_test_playback, "pvr play back");
    aui_tu_reg_item(2, "timeshift", AUI_CMD_TYPE_API, pvr_test_timeshift, "pvr timeshift");
    aui_tu_reg_item(2, "qt_record", AUI_CMD_TYPE_API, qt_board_test_pvr_test_record, "pvr recorder for ALI internal QT board test");
    aui_tu_reg_item(2, "qt_timeshift", AUI_CMD_TYPE_API, qt_board_test_pvr_test_timeshift, "pvr timeshift for QT board test");
    aui_tu_reg_item(2, "set", AUI_CMD_TYPE_API, pvr_test_set, "pvr set disk threshold value");
    aui_tu_reg_item(2, "deinit", AUI_CMD_TYPE_API, pvr_test_deinit, "pvr deinit");
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, pvr_test_help, "pvr test help");
}

