/****************************INCLUDE HEAD FILE************************************/
#include <stdlib.h>
#include <string.h>
#include "aui_dmx_record_test.h"


/****************************GLOBAL VAR*****************************************/
static unsigned char *gp_uc_record_ts_buf = NULL;


/****************************TEST MODULE IMPLEMENT*******************************/
static long aui_record_req_wtcb(void *p_user_hdl, unsigned long ul_req_size, void ** pp_req_buf, 
        unsigned long *req_buf_size, struct aui_avsync_ctrl *pst_ctrl_blk)
{
    AUI_PRINTF("\nEnter into record request call back!\n");
    
    *pp_req_buf = gp_uc_record_ts_buf;
    
    if (RECORD_BUFFER_LEN > ul_req_size) {
        *req_buf_size = ul_req_size;
    } else {
        *req_buf_size = RECORD_BUFFER_LEN;
    }
    AUI_PRINTF("Driver request buffer size: %ld bytes\n", ul_req_size);
    return AUI_RTN_SUCCESS;
}

static long aui_record_update_wtcb(void *p_user_hdl, unsigned long ul_size)
{   
    AUI_PRINTF("\nEnter into record update call back!\n");  
    AUI_PRINTF("Actual received TS packet number: %ld bytes\n", ul_size);

    unsigned long ul_ts_package_num = ul_size / TS_PACKAGE_SIZE;
    unsigned long i = 0, j = 0, k = 0;
    unsigned short us_sync_byte_error_count = 0;
    unsigned short us_transport_error_indicator_count = 0;
    unsigned short us_continuity_counter_error_count = 0;

    struct attr_record_ts_data_pid attr_pid;
    MEMSET(&attr_pid, 0, sizeof(struct attr_record_ts_data_pid));

    // Malloc memory for store pid pid_diff pid_num and pid_continuity_counter
    attr_pid.us_pid = (unsigned short *)MALLOC(ul_ts_package_num * sizeof(unsigned short));
    if (NULL == attr_pid.us_pid) {
        AUI_PRINTF("%s -> attr_pid.us_pid malloc: FAIL\n", __FUNCTION__);
        goto EXIT_FAIL;
    }
    attr_pid.us_pid_diff = (unsigned short *)MALLOC(ul_ts_package_num * sizeof(unsigned short));
    if (NULL == attr_pid.us_pid_diff) {
        AUI_PRINTF("%s -> attr_pid.us_pid_diff malloc: FAIL\n", __FUNCTION__);
        goto EXIT_PID_FREE;
    }
    attr_pid.us_pid_num = (unsigned short *)MALLOC(ul_ts_package_num * sizeof(unsigned short));
    if (NULL == attr_pid.us_pid_num) {
        AUI_PRINTF("%s -> attr_pid.us_pid_num malloc: FAIL\n", __FUNCTION__);
        goto EXIT_PID_DIFF_FREE;
    }
    attr_pid.us_pid_continuity_counter = (unsigned short *)MALLOC(ul_ts_package_num * sizeof(unsigned short));
    if (NULL == attr_pid.us_pid_continuity_counter) {
        AUI_PRINTF("%s -> attr_pid.us_pid_continuity_counter malloc: FAIL\n", __FUNCTION__);
        goto EXIT_PID_NUM_FREE;
    }
    MEMSET(attr_pid.us_pid, 0, ul_ts_package_num * sizeof(unsigned short));
    MEMSET(attr_pid.us_pid_diff, 0, ul_ts_package_num * sizeof(unsigned short));
    MEMSET(attr_pid.us_pid_num, 0, ul_ts_package_num * sizeof(unsigned short));
    MEMSET(attr_pid.us_pid_continuity_counter, 0, ul_ts_package_num * sizeof(unsigned short));
    
    // step1: Parse TS data, check sync_type and transport_error_indicator, store PID and continuity_counter
    for (i=0; i<ul_ts_package_num; i++) {
        // Check sync_byte
        if (0x47 != gp_uc_record_ts_buf[i*TS_PACKAGE_SIZE+0]) {
            us_sync_byte_error_count++;
        }
        // Check transport_error_indicator
        if (0x80 == (gp_uc_record_ts_buf[i*TS_PACKAGE_SIZE+1]&0x80)) {
            us_transport_error_indicator_count++;
        }
        // PID
        attr_pid.us_pid[i] = (((unsigned short)(gp_uc_record_ts_buf[i*TS_PACKAGE_SIZE+1]&0x1F))<<8)|((unsigned short)gp_uc_record_ts_buf[i*TS_PACKAGE_SIZE+2]&0x00FF);
        // continuity_counter
        attr_pid.us_pid_continuity_counter[i] = (unsigned short)(gp_uc_record_ts_buf[i*TS_PACKAGE_SIZE+3]&0x0F);
    }
    // Print the resoult of check
    if (0 == us_sync_byte_error_count) {
        AUI_PRINTF("All received TS package sync_byte:  -- OK\n");
    } else {
        AUI_PRINTF("All received TS package have [%d] sync_byte:  -- ERROR\n", us_sync_byte_error_count);
    }
    if (0 == us_transport_error_indicator_count) {
        AUI_PRINTF("All received TS package transport_error_indicator:  -- OK\n");
    } else {
        AUI_PRINTF("All received TS package have [%d] transport_error_indicator:  -- ERROR\n", us_transport_error_indicator_count);
    }
    
    // step2: Filter out the same PID and invalid PID (AUI_INVALID_PID: 8191 [0x1fff]), and statistics the number of different PID    
    for (i=0; i<ul_ts_package_num; i++) {
        attr_pid.us_pid_diff_flag = 1;
        for (j=0; j <attr_pid.ul_pid_diff_count; j++) {         
            if ((attr_pid.us_pid_diff[j] == attr_pid.us_pid[i]) || (AUI_INVALID_PID == attr_pid.us_pid[i])) {
                attr_pid.us_pid_diff_flag = 0;
                break;
            }
        } 
        if (1 == attr_pid.us_pid_diff_flag) {
            attr_pid.us_pid_diff[attr_pid.ul_pid_diff_count] = attr_pid.us_pid[i];
            attr_pid.ul_pid_diff_count++;
        }
    }

    // step3: Statistics the number of each PID TS packet
    for (i=0; i<attr_pid.ul_pid_diff_count; i++) {
        for (j=0; j<ul_ts_package_num; j++) {
            if (attr_pid.us_pid_diff[i] == attr_pid.us_pid[j]) {
                attr_pid.us_pid_num[i]++;
            }
        }
    }    
    AUI_PRINTF("All received TS package have [%d] different PID: [pid  num]\n", attr_pid.ul_pid_diff_count);    
    unsigned short us_pid_num_max = attr_pid.us_pid_num[0];
    for (i=0; i<attr_pid.ul_pid_diff_count; i++) {      
        AUI_PRINTF("[%d  %d]  ", attr_pid.us_pid_diff[i], attr_pid.us_pid_num[i]);
        if (us_pid_num_max <= attr_pid.us_pid_num[i]) {
            us_pid_num_max = attr_pid.us_pid_num[i];  // search the max pid_num for malloc
        }
    }
    // Malloc a memory for store all different continuity_counter, according the size of ul_pid_diff_count and us_pid_num_max
    attr_pid.us_pid_diff_continuity_counter = (unsigned short *)MALLOC(attr_pid.ul_pid_diff_count * us_pid_num_max * sizeof(unsigned short));
    if (NULL == attr_pid.us_pid_diff_continuity_counter) {
        AUI_PRINTF("%s -> us_pid_diff_continuity_counter malloc: FAIL\n", __FUNCTION__);
        goto EXIT_PID_CONT_COUNTER_FREE;
    }
    MEMSET(attr_pid.us_pid_diff_continuity_counter, 0, attr_pid.ul_pid_diff_count * us_pid_num_max * sizeof(unsigned short));
    
    // Step4: Check whether all the PID TS packet continuously
    signed s_after_before_diff = 0;
    AUI_PRINTF("\n");
    for (i=0; i<attr_pid.ul_pid_diff_count; i++) {
        k = 0;
        us_continuity_counter_error_count = 0;
        for (j=0; j<ul_ts_package_num; j++) {
            if (attr_pid.us_pid_diff[i] == attr_pid.us_pid[j]) {
                attr_pid.us_pid_diff_continuity_counter[k + i*us_pid_num_max] = attr_pid.us_pid_continuity_counter[j];
                k++;
            }
        }        
        for (j=1; j<attr_pid.us_pid_num[i]; j++) {
            s_after_before_diff = (signed)attr_pid.us_pid_diff_continuity_counter[j + i*us_pid_num_max]
                                  - (signed)attr_pid.us_pid_diff_continuity_counter[j-1 + i*us_pid_num_max];
            // continuity_counter start from 0 to the maximum 0xF (15) and then back to zero.
            //    1-0=1; 2-1=1; ... 15-14=1; 0-15=-15; 1-0=1; ...
            if ((1 != s_after_before_diff) && (-15 != s_after_before_diff)) {
                us_continuity_counter_error_count++;
            }
        }
        if (0 == us_continuity_counter_error_count) {
            AUI_PRINTF("%-2d. PID[%-4d] continuity_counter:  -- OK\n", i+1, attr_pid.us_pid_diff[i]);
        } else {
            AUI_PRINTF("%-2d. PID[%-4d] have [%d] continuity_counter:  -- ERROR\n", i+1, attr_pid.us_pid_diff[i], us_continuity_counter_error_count);
            for (j=0; j<attr_pid.us_pid_num[i]; j++) {
                AUI_PRINTF("    %d", attr_pid.us_pid_diff_continuity_counter[j + i*us_pid_num_max]);
            }
            AUI_PRINTF("\n");
        }     
    }   

    // Print all received TS package number
    AUI_PRINTF("The number of TS package all received: %ld\n\n", ul_ts_package_num);

    // Normal exit, free memory
    if (NULL != attr_pid.us_pid_diff_continuity_counter) {
        FREE(attr_pid.us_pid_diff_continuity_counter);
        attr_pid.us_pid_diff_continuity_counter= NULL;
    }
    if (NULL != attr_pid.us_pid_continuity_counter) {
        FREE(attr_pid.us_pid_continuity_counter);
        attr_pid.us_pid_continuity_counter= NULL;
    }
    if (NULL != attr_pid.us_pid_num) {
        FREE(attr_pid.us_pid_num);
        attr_pid.us_pid_num = NULL;
    }
    if (NULL != attr_pid.us_pid_diff) {
        FREE(attr_pid.us_pid_diff);
        attr_pid.us_pid_diff = NULL;
    }
    if (NULL != attr_pid.us_pid) {
        FREE(attr_pid.us_pid);
        attr_pid.us_pid = NULL;
    }    
    return AUI_RTN_SUCCESS;

    // Non-normal exit, free memory
EXIT_PID_CONT_COUNTER_FREE:
    if (NULL != attr_pid.us_pid_continuity_counter) {
        FREE(attr_pid.us_pid_continuity_counter);
        attr_pid.us_pid_continuity_counter= NULL;
    }
EXIT_PID_NUM_FREE:
    if (NULL != attr_pid.us_pid_num) {
        FREE(attr_pid.us_pid_num);
        attr_pid.us_pid_num = NULL;
    }
EXIT_PID_DIFF_FREE:
    if (NULL != attr_pid.us_pid_diff) {
        FREE(attr_pid.us_pid_diff);
        attr_pid.us_pid_diff = NULL;
    }
EXIT_PID_FREE:
    if (NULL != attr_pid.us_pid) {
        FREE(attr_pid.us_pid);
        attr_pid.us_pid = NULL;
    }
EXIT_FAIL:
    return AUI_RTN_FAIL;
}

static void show_usage()
{
    AUI_PRINTF("In AUI_DMX_DATA_REC mode, record full TS data, you can input:\n");
    AUI_PRINTF("7 [dmx_data_type],[record_time]\n");
    AUI_PRINTF("Param Spec:\n");
    AUI_PRINTF("<1> [dmx_data_type] --  0:AUI_DMX_DATA_REC\n");
    AUI_PRINTF("<2> [record_time]   --  1:1s, unit is seconds\n");
    AUI_PRINTF("For example:\n");
    AUI_PRINTF("-- record all TS data:                               7 0,1\n");
    AUI_PRINTF("In AUI_DMX_DATA_REC mode, record specified PID TS data, you can input:\n");
    AUI_PRINTF("7 [dmx_data_type],[record_time],[pid_cnt],[pid1],[pid2],[pid3]...\n");
    AUI_PRINTF("Param Spec:\n");
    AUI_PRINTF("<1> [dmx_data_type] --  0:AUI_DMX_DATA_REC\n");
    AUI_PRINTF("<2> [record_time]   --  1:1s, ,2:2s, 10:10s, 30:30s, unit is seconds\n");
    AUI_PRINTF("<3> [pid_cnt]       --  1:one PID, 2:two different kinds of PID, 3,4,5...\n");
    AUI_PRINTF("<4> [pid]           --  0:PAT PID, 1:CAT PID, 660:Audio PID, 513:Video PID, 8190:PCR PID\n");
    AUI_PRINTF("For example:\n");
    AUI_PRINTF("-- record PAT PID TS data:                           7 0,30,1,0\n");
    AUI_PRINTF("-- record CAT PID TS data:                           7 0,30,1,1\n");
    AUI_PRINTF("-- record Audio PID TS data:                         7 0,3,1,660\n");
    AUI_PRINTF("-- record Video PID TS data:                         7 0,1,1,513\n");
    AUI_PRINTF("-- record PCR PID TS data:                           7 0,10,1,8190\n");
    AUI_PRINTF("-- record PAT and CAT PID TS data:                   7 0,30,2,0,1\n");
    AUI_PRINTF("-- record Audio and Video PID TS data:               7 0,2,2,660,513\n");
    AUI_PRINTF("-- record PAT CAT Audio Video and PCR PID TS data:   7 0,2,5,0,1,660,513,8190\n");
    AUI_PRINTF("\nIn AUI_DMX_DATA_RAW mode, you can input:\n");
    AUI_PRINTF("7 [dmx_data_type],[record_time],[pid]\n");
    AUI_PRINTF("Param Spec:\n");
    AUI_PRINTF("<1> [dmx_data_type] --  1:AUI_DMX_DATA_RAW\n");
    AUI_PRINTF("<2> [record_time]   --  1:1s, 2:2s, 3:3s, unit is seconds\n");
    AUI_PRINTF("<3> [pid]           --  0:PAT PID, 1:CAT PID, 660:Audio PID, 513:Video PID, 8190:PCR PID\n");
    AUI_PRINTF("-- record PAT PID TS data:                           7 1,1,0\n");
    AUI_PRINTF("-- record CAT PID TS data:                           7 1,2,1\n");
    AUI_PRINTF("-- record Audio PID TS data:                         7 1,1,660\n");
    AUI_PRINTF("-- record Video PID TS data:                         7 1,1,513\n");
    AUI_PRINTF("-- record PCR PID TS data:                           7 1,3,8190\n");
    AUI_PRINTF("Attention:\n");
    AUI_PRINTF("[1] AUI_DMX_DATA_REC  Support for a variety of PID TS data, at the same time; support AUI_FULL_TS_PID.\n");
    AUI_PRINTF("                      If record PAT PID and CAT PID, the record_time should be set 30s(manual test) at least.\n");
    AUI_PRINTF("                      Sometimes in TS Stream, the number of PAT table will be few.\n");
    AUI_PRINTF("                      and we apply for a larg memory(47k bytes), driver need more time to cache data.\n");
    AUI_PRINTF("                      When [pid_cnt] > AUI_DMX_REC_PID_LIST_MAX_LEN(32), We use AUI_FULL_TS_PID as us_pid,\n");
    AUI_PRINTF("                      and make ul_pid_cnt equal to zero. Then we can record all TS data.\n");    
    AUI_PRINTF("[2] AUI_DMX_DATA_RAW  Support only once for A kind of PID TS data, at the same time; \n");
    AUI_PRINTF("                      not support pid_list; not support AUI_FULL_TS_PID; driver not cache data.\n");
    AUI_PRINTF("[3] Common program PID\n");
    AUI_PRINTF("                      <CCTV-2>    660:Audio  PID,  513:Video PID,  8190:PCR PID\n");
    AUI_PRINTF("                      <CCTV-11>   690:Audio  PID,  516:Video PID,  8190:PCR PID\n");
    AUI_PRINTF("                      <CCTV-7>    670:Audio  PID,  514:Video PID,  8190:PCR PID\n");
    AUI_PRINTF("                      <CCTV-10>   680:Audio  PID,  515:Video PID,  8190:PCR PID\n");
    AUI_PRINTF("                      <CCTV-13>   700:Audio  PID,  517:Video PID,  8190:PCR PID\n");
    AUI_PRINTF("                      <CCTV-NEWS> 1220:Audio PID,  1260:Video PID, 8190:PCR PID\n");
    AUI_PRINTF("                      <SHOPPING>  650:Audio  PID,  512:Video PID,  512:PCR PID\n");
    AUI_PRINTF("                      <NOTP950>   650:Audio  PID,  512:Video PID,  512:PCR PID\n");
}

unsigned long sample_dmx_record_ts_data(unsigned long *argc,char **argv,char *sz_out_put)
{
    // check whether the input parameter is illegal
    if ((NULL == argc) || (NULL == *argv) || (NULL == sz_out_put)) {
        AUI_PRINTF("The input parameter of %s is illegal\n", __FUNCTION__);
        goto EXIT_FAIL;
    }

    // config cmd param
    unsigned long i = 0;
    unsigned long ul_record_time = 0;
    unsigned long ul_pid_cnt = 0;
    unsigned short us_pid = 0;
    unsigned short us_pid_list[AUI_DMX_REC_PID_LIST_MAX_LEN] = {0};
    unsigned short us_dmx_data_type = 0;
    if (0 == *argc) {
        show_usage();
        goto EXIT_FAIL;
    } else {
        us_dmx_data_type = ATOI(argv[0]);
        if (0 == us_dmx_data_type) {  // AUI_DMX_DATA_REC
            ul_record_time = ATOI(argv[1]);
            ul_pid_cnt = ATOI(argv[2]);
            if (ul_pid_cnt > AUI_DMX_REC_PID_LIST_MAX_LEN) {
                AUI_PRINTF("record too many channels %ld > %d \n", ul_pid_cnt, AUI_DMX_REC_PID_LIST_MAX_LEN);
                AUI_PRINTF("you can record all ts data, please input:   record 0,1\n");
                goto EXIT_FAIL;
            }
            for (i=0; i<ul_pid_cnt; i++) {
                us_pid_list[i] = ATOI(argv[i+3]);
            }
            if (1 == *argc) {
                AUI_PRINTF("In AUI_DMX_DATA_REC mode, record full TS data, you can input:\n");
                AUI_PRINTF("7 [dmx_data_type],[record_time]\n\n");
                AUI_PRINTF("In AUI_DMX_DATA_REC mode, record specified PID TS data, you can input:\n");
                AUI_PRINTF("7 [dmx_data_type],[record_time],[pid_cnt],[pid1],[pid2],[pid3]...\n");
                goto EXIT_FAIL;
            }
            if (3 == *argc) {
                AUI_PRINTF("In AUI_DMX_DATA_REC mode, record specified PID TS data, you can input:\n");
                AUI_PRINTF("7 [dmx_data_type],[record_time],[pid_cnt],[pid1],[pid2],[pid3]...\n");
                goto EXIT_FAIL;
            }
        } else if (1 == us_dmx_data_type) {  // AUI_DMX_DATA_RAW
            ul_record_time = ATOI(argv[1]);
            us_pid = ATOI(argv[2]);
            if (3 != *argc) {
                AUI_PRINTF("In AUI_DMX_DATA_RAW mode, Please only input the 3 parameters:\n");
                AUI_PRINTF("7 [dmx_data_type],[record_time],[pid]\n");
                goto EXIT_FAIL;
            }
        } else {
            AUI_PRINTF("please input right first param, 0:AUI_DMX_DATA_REC  1:AUI_DMX_DATA_RAW\n");
            goto EXIT_FAIL;
        }
    }

    aui_hdl hdl_dmx = NULL;
    aui_hdl hdl_channel = NULL;
    aui_hdl hdl_filter = NULL;

    aui_attr_dmx attr_dmx;
    aui_attr_dmx_channel attr_channel;
    aui_attr_dmx_filter attr_filter;

    MEMSET(&attr_dmx, 0, sizeof(aui_attr_dmx));
    MEMSET(&attr_channel, 0, sizeof(aui_attr_dmx_channel));
    MEMSET(&attr_filter, 0, sizeof(aui_attr_dmx_filter));

    // Malloc record TS data memory
    gp_uc_record_ts_buf = (unsigned char *)MALLOC(RECORD_BUFFER_LEN * sizeof(unsigned char)); 
    if (NULL == gp_uc_record_ts_buf) {
        AUI_PRINTF("%s -> gp_uc_record_ts_buf malloc: FAIL\n", __FUNCTION__);        
        goto EXIT_FAIL;
    }
    MEMSET(gp_uc_record_ts_buf, 0, RECORD_BUFFER_LEN * sizeof(unsigned char));

    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
    
    // dmx open
    if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl_dmx)) {
        if (aui_dmx_open(&attr_dmx, &hdl_dmx)) {
            AUI_PRINTF("%s -> aui_dmx_open: FAIL\n", __FUNCTION__);
            goto EXIT_RECORD_TS_BUF_FREE;
        }
        AUI_PRINTF("aui_dmx_open: OK\n");
    }
    if(0 == us_dmx_data_type) { // only in AUI_DMX_DATA_REC mode, set dmx data path
        aui_dmx_data_path path;
        MEMSET(&path, 0, sizeof(aui_dmx_data_path));
        path.data_path_type = AUI_DMX_DATA_PATH_REC;
        if (aui_dmx_data_path_set(hdl_dmx, &path)) {
            AUI_PRINTF("%s -> aui_dmx_data_path_set: FAIL\n", __FUNCTION__);
            goto EXIT_RECORD_TS_BUF_FREE;
        }
        AUI_PRINTF("aui dmx data path set: %d\n", path.data_path_type);
    }
	
    // dmx start
    if (aui_dmx_start(hdl_dmx, &attr_dmx)) {
        AUI_PRINTF("%s -> aui_dmx_start: FAIL\n", __FUNCTION__);
        goto EXIT_RECORD_TS_BUF_FREE;
    }
    AUI_PRINTF("aui_dmx_start: OK\n");

    // config dmx channel pid and data type
    // AUI_DMX_DATA_REC, support for a variety of PID TS data, at the same time
    if(0 == us_dmx_data_type) {
        attr_channel.dmx_data_type = AUI_DMX_DATA_REC;
        if (ul_pid_cnt > 0) {
            attr_channel.us_pid = AUI_INVALID_PID;
            attr_channel.ul_pid_cnt = ul_pid_cnt;
            for (i=0; i<ul_pid_cnt; i++) {
                attr_channel.us_pid_list[i] = us_pid_list[i];
            }
            AUI_PRINTF("\n");
        } else {
            attr_channel.us_pid = AUI_FULL_TS_PID;
            attr_channel.ul_pid_cnt = 0;
        }
    }
    // AUI_DMX_DATA_RAW, support only once for A kind of PID TS data, at the same time;
    //                               not support pid_list; not support AUI_FULL_TS_PID
    else if (1 == us_dmx_data_type) {
        attr_channel.dmx_data_type = AUI_DMX_DATA_RAW;
        attr_channel.us_pid = us_pid;
    } else {
        AUI_PRINTF("please input right first param, 0:AUI_DMX_DATA_REC  1:AUI_DMX_DATA_RAW\n");
        goto EXIT_RECORD_TS_BUF_FREE;
    }
    // dmx channel open
    if(aui_dmx_channel_open(hdl_dmx, &attr_channel, &hdl_channel)) {
        AUI_PRINTF("%s -> aui_dmx_channel_open: FAIL\n", __FUNCTION__);
        goto EXIT_RECORD_TS_BUF_FREE;
    }
    AUI_PRINTF("aui_dmx_channel_open: OK\n");

    // dmx channel start
    if(aui_dmx_channel_start(hdl_channel, NULL)) {
        AUI_PRINTF("%s -> aui_dmx_channel_start: FAIL\n", __FUNCTION__);
        AUI_PRINTF("dmx data type:%d, pid:%d, pid cnt:%d\n", attr_channel.dmx_data_type, attr_channel.us_pid, attr_channel.ul_pid_cnt);
        goto EXIT_CHANNEL_CLOSE;
    }
    AUI_PRINTF("aui_dmx_channel_start: OK\n");

    // dmx filter open
    attr_filter.p_fun_data_req_wtCB = aui_record_req_wtcb;
    attr_filter.p_fun_data_up_wtCB  = aui_record_update_wtcb;
    if(aui_dmx_filter_open(hdl_channel, &attr_filter, &hdl_filter)) {
        AUI_PRINTF("%s -> aui_dmx_filter_open: FAIL\n", __FUNCTION__);
        goto EXIT_CHANNEL_STOP;
    }
    AUI_PRINTF("aui_dmx_filter_open: OK\n");
    
    // dmx filter start
    if(aui_dmx_filter_start(hdl_filter, &attr_filter)) {
        AUI_PRINTF("%s -> aui_dmx_filter_start: FAIL\n", __FUNCTION__);
        goto EXIT_FILTER_CLOSE;
    }
    AUI_PRINTF("aui_dmx_filter_start: OK\n");

    // wait while recording
    AUI_PRINTF("wait %ld seconds for record ts data\n", ul_record_time);
    AUI_SLEEP(ul_record_time*1000);

    // dmx filter stop  
    if (aui_dmx_filter_stop(hdl_filter, NULL)) {
        AUI_PRINTF("%s -> aui_dmx_filter_stop: FAIL\n", __FUNCTION__);
    }
    AUI_PRINTF("aui_dmx_filter_stop: OK\n");
    
    // dmx filter close
EXIT_FILTER_CLOSE:
    if (aui_dmx_filter_close(&hdl_filter)) {
        AUI_PRINTF("%s -> aui_dmx_filter_close: FAIL\n", __FUNCTION__);
    }
    AUI_PRINTF("aui_dmx_filter_close: OK\n");

    // dmx channel stop
EXIT_CHANNEL_STOP:
    if (aui_dmx_channel_stop(hdl_channel, NULL)) {
        AUI_PRINTF("%s -> aui_dmx_channel_stop: FAIL\n", __FUNCTION__);
    }
    AUI_PRINTF("aui_dmx_channel_stop: OK\n");

    // dmx channel close
EXIT_CHANNEL_CLOSE:
    if (aui_dmx_channel_close(&hdl_channel)) {
        AUI_PRINTF("%s -> aui_dmx_channel_close: FAIL\n", __FUNCTION__);
    }
    AUI_PRINTF("aui_dmx_channel_close: OK\n");
    // Don't need aui_dmx_stop and aui_dmx_close, because them affect the normal stream

    // Normal exit, free memory
    if (NULL != gp_uc_record_ts_buf) {
        FREE(gp_uc_record_ts_buf);
        gp_uc_record_ts_buf = NULL;
    }
     
    return AUI_RTN_SUCCESS;
    
    // Non-normal exit, free memory
EXIT_RECORD_TS_BUF_FREE:
    if (NULL != gp_uc_record_ts_buf) {
        FREE(gp_uc_record_ts_buf);
        gp_uc_record_ts_buf = NULL;
    }
EXIT_FAIL:
    return AUI_RTN_FAIL;
}

