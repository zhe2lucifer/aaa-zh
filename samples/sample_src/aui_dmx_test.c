/****************************INCLUDE HEAD FILE************************************/
#include <aui_dmx.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_nim.h>
#include <aui_tsi.h>
//#include "unity_fixture.h"
#include "aui_test_app.h"
#include "aui_test_stream.h"
#include "aui_nim_init.h"
#include "aui_dmx_test.h"
#include "aui_test_stream_play_live_stream.h"
#include "aui_dmx_record_test.h"

#ifdef AUI_LINUX
#include <pthread.h>
#endif

//static void *dmx_handle = NULL;
//static unsigned char dev_idx = 0;

//static int DmxFilterGetData_usr_data;
static long fun_sectionCB
(
    aui_hdl filter_handle,
    unsigned char *p_section_data_buf_addr,
    unsigned long ul_section_data_len,
    void *usr_data,
    void *reserved
)
{
    AUI_PRINTF("\r\nfilter[%08x] Got data length[%d], first 5 bytes:\n[%02x][%02x][%02x][%02x][%02x]\n",
            filter_handle,ul_section_data_len,
            p_section_data_buf_addr[0],p_section_data_buf_addr[1],p_section_data_buf_addr[2],
            p_section_data_buf_addr[3],p_section_data_buf_addr[4]);
	return 0;
}

#define BUF_SIZE 2 * 188
unsigned char g_auc_test_buf[BUF_SIZE]={0};
long aui_req_wtCB(void *p_user_hdl, unsigned long ul_req_size, void ** pp_req_buf, 
        unsigned long *req_buf_size, struct aui_avsync_ctrl *pst_ctrl_blk)
{
    AUI_PRINTF("\r\n req wt!");
    *pp_req_buf = g_auc_test_buf;
    *req_buf_size = BUF_SIZE;
    return 0;
}
long aui_update_wtCB(void *p_user_hdl, unsigned long ul_size)
{
    int i=0;
    AUI_PRINTF("\r\n up wt[%d]!",ul_size);
    for(i=0;i<(int)ul_size;i++)
    {
        AUI_PRINTF("[%02x]-",g_auc_test_buf[i]);
    }
	return 0;
}

unsigned long test_dmx_help(unsigned long *argc,char **argv,char *sz_out_put)
{    
    AUI_PRINTF("\n\n--------------------NOTE---------------------------------------------------\n");
    AUI_PRINTF("Use play command of stream menu to play a channel before testing dmx module,\n or else nothing could be got!\n");
    AUI_PRINTF("--------------------NOTE---------------------------------------------------\n\n");

    #define     DMX_1_HELP      "Get pat table.\n"
    AUI_PRINTF("1\n");
    AUI_PRINTF(DMX_1_HELP);
    AUI_PRINTF("To get pat data, you just input: 1\n\n");

    #define     DMX_2_HELP      "Get pes data.\n"
    AUI_PRINTF("2\n");
    AUI_PRINTF(DMX_2_HELP);
    AUI_PRINTF("To get PES data, you can input :\n");
    AUI_PRINTF("2 pid\n");
    AUI_PRINTF("For example to get video pes data of cctv2\n");
    AUI_PRINTF("2 513\n\n");

    #define DMX_3_HELP          "Get stc information\n"
    AUI_PRINTF("3\n");
    AUI_PRINTF(DMX_3_HELP);
    AUI_PRINTF("To get stc, you just input: 3\n\n");

#ifdef AUI_TDS
    #define DMX_4_HELP          "Get pcr data\n"
    AUI_PRINTF("4\n");
    AUI_PRINTF(DMX_4_HELP);
    AUI_PRINTF("To get pcr data, you can input :\n");
    AUI_PRINTF("4 pcr_pid\n");
    AUI_PRINTF("For example to get pcr data of cctv2\n");
    AUI_PRINTF("4 8190\n\n");
#endif
    #define DMX_5_HELP          "Get section in sync mode\n"
    AUI_PRINTF("5\n");
    AUI_PRINTF(DMX_5_HELP);
    AUI_PRINTF("To get section in sync mode, you can input :\n");
    AUI_PRINTF("5 pid,table id\n");
    AUI_PRINTF("For example to get PAT table in sync mode:\n");
    AUI_PRINTF("5 0,0\n\n");

    #define DMX_6_HELP          "Get raw data\n"
    AUI_PRINTF("6\n");
    AUI_PRINTF(DMX_6_HELP);
    AUI_PRINTF("To get raw data, you can input :\n");
    AUI_PRINTF("6\n\n");
    
    #define DMX_7_HELP          "Record TS data\n"
    AUI_PRINTF("7\n");
    AUI_PRINTF(DMX_7_HELP);
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
    AUI_PRINTF("<2> [record_time]   --  1:1s, 2:2s, 10:10s, 30:30s, unit is seconds\n");
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
    
    return AUI_RTN_HELP;
}

unsigned long sample_dmx_get_pat_table(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_attr_dmx attr_dmx;
    aui_attr_dmx_channel attr_channel;
    aui_attr_dmx_filter attr_filter;
    aui_hdl hdl_dmx=NULL;
    aui_hdl hdl_channel=NULL;
    aui_hdl hdl_filter=NULL;
    // user want to get all the table that the first byte equal 0x00 
    unsigned char mask[] = {0xff};
    unsigned char value[] = {0x00};
    unsigned char reverse[] = {0x0};
    // Just care for the first BYTE
    unsigned long ul_mask_val_len=1;
    // Dont do CRC check for the section data
    unsigned char uc_crc_check=0;
    // Muti-times capture the section,if uc_continue_capture_flag==0,then just the first section will call the user callback
    unsigned char uc_continue_capture_flag=1;
    int dmx_id = 0;
    int table_id = 0;
    int pid = 0;
	if (*argc > 0) {
		dmx_id = atoi(argv[0]);
		AUI_PRINTF("%s dmx_id: %d\n", __func__, dmx_id);
	}
	if (*argc > 1) {
		pid = atoi(argv[1])&AUI_INVALID_PID;
		AUI_PRINTF("%s pid: %d\n", __func__, pid);
	}
	if (*argc > 2) {
		table_id = atoi(argv[2]);
		value[0] = table_id;
		AUI_PRINTF("%s table_id: %d\n", __func__, table_id);
	}
    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&attr_channel,0,sizeof(aui_attr_dmx_channel));
    MEMSET(&attr_filter,0,sizeof(aui_attr_dmx_filter));
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, dmx_id, &hdl_dmx))
    {
        AUI_PRINTF("dmx device is close!!\n");
        attr_dmx.uc_dev_idx = dmx_id;
        if(aui_dmx_open(&attr_dmx, &hdl_dmx))
        {
            AUI_PRINTF("%s -> aui_dmx_open fail!!\n", __FUNCTION__);
            return AUI_RTN_FAIL;
        }
    }
    //Start the dmx0 device
    aui_dmx_start(hdl_dmx, &attr_dmx);
    //Capture the channel are bind with PID(value is 0) 
    attr_channel.us_pid = pid;
    //Config the channel data type are section data
    attr_channel.dmx_data_type = AUI_DMX_DATA_SECT;
    //Open the dmx channel device
    aui_dmx_channel_open(hdl_dmx, &attr_channel, &hdl_channel);
    //Start the dmx channel device    
    aui_dmx_channel_start(hdl_channel, &attr_channel);
    //Open the dmx filter device
    aui_dmx_filter_open(hdl_channel,&attr_filter,&hdl_filter);
    //The fun_sectionCB are the callback for app and implement by user
    aui_dmx_reg_sect_call_back(hdl_filter,fun_sectionCB);    
    //Configure the filter:user want to continue get all the table that the first byte equal 0x00
    aui_dmx_filter_mask_val_cfg(hdl_filter,mask,value,reverse, ul_mask_val_len,uc_crc_check,uc_continue_capture_flag);
    //Start the dmx filter device
    aui_dmx_filter_start(hdl_filter, &attr_filter);
    AUI_PRINTF("%s -> wait for 10 seconds\n", __FUNCTION__);
    AUI_SLEEP(10000);
    aui_dmx_filter_stop(hdl_filter,NULL);
    aui_dmx_filter_close(&hdl_filter);
    aui_dmx_channel_stop(hdl_channel, NULL);    
    aui_dmx_channel_close(&hdl_channel);
	
	return AUI_RTN_SUCCESS;
}

unsigned long  sample_dmx_get_raw_data(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_attr_dmx attr_dmx;
    aui_attr_dmx_channel attr_channel;
    aui_attr_dmx_filter attr_filter;
    aui_hdl hdl_dmx=NULL;
    aui_hdl hdl_channel=NULL;
    aui_hdl hdl_filter=NULL;

    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&attr_channel,0,sizeof(aui_attr_dmx_channel));
    MEMSET(&attr_filter,0,sizeof(aui_attr_dmx_filter));
    
    //Open the first dmx device
    //attr_dmx.uc_dev_idx=0;
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl_dmx))
    {
        AUI_PRINTF("dmx device is close!!\n");
        if(aui_dmx_open(&attr_dmx, &hdl_dmx))
        {
            AUI_PRINTF("%s -> aui_dmx_open fail!!\n", __FUNCTION__);
            return AUI_RTN_FAIL;
        }
    }
    //Start the dmx0 device
    aui_dmx_start(hdl_dmx, &attr_dmx);
    //Capture the channel are bind with PID(value is 0) 
    attr_channel.us_pid = 0;//AUI_FULL_TS_PID;//0;
    //Config the channel data type are TS record data
    attr_channel.dmx_data_type = AUI_DMX_DATA_RAW;//AUI_DMX_DATA_REC;
    AUI_PRINTF("dmx_data_type: %d\n", attr_channel.dmx_data_type);
    
    //Open the dmx channel device
    aui_dmx_channel_open(hdl_dmx, &attr_channel, &hdl_channel);
    //Start the dmx channel device    
    aui_dmx_channel_start(hdl_channel, &attr_channel);
    //Open the dmx filter device
    aui_dmx_filter_open(hdl_channel,&attr_filter,&hdl_filter);
    //The fun_sectionCB are the callback for app and implement by user
    aui_dmx_reg_data_call_back(hdl_filter,aui_req_wtCB,aui_update_wtCB);    
    //Start the dmx filter device
    aui_dmx_filter_start(hdl_filter, &attr_filter);   

    AUI_SLEEP(2000);
    
    aui_dmx_reg_data_call_back(hdl_filter, NULL, NULL);
    aui_dmx_filter_stop(hdl_filter,NULL);   
    aui_dmx_filter_close(&hdl_filter);
    hdl_filter = NULL;    
    aui_dmx_channel_stop(hdl_channel, NULL);
    aui_dmx_channel_close(&hdl_channel);

	return AUI_RTN_SUCCESS;
}

//#ifdef AUI_TDS
aui_hdl g_hdl_dmx_channel = NULL;
unsigned g_section_cnt = 0;
unsigned g_filter_cnt = 0;
unsigned g_filter_cnt_ddb = 0;

#ifdef AUI_TDS
void *g_p_hdl_ch_read_task=NULL;
void *g_p_hdl_ch_read_task2=NULL;
void *g_p_hdl_overflow_proc_task=NULL;
#else
pthread_t g_p_hdl_ch_read_task;
pthread_t g_p_hdl_ch_read_task2;
pthread_t g_p_hdl_overflow_proc_task;
static pthread_mutex_t mutex_read_channel = PTHREAD_MUTEX_INITIALIZER;
#endif

aui_hdl g_hdl_filter=NULL;
aui_hdl g_hdl_filter_ddb=NULL;

#define RCV_BUF_LEN  65536
int ch_read_task_quit_flg = 0;
unsigned char g_buf[RCV_BUF_LEN];
unsigned char g_buf_ddb[RCV_BUF_LEN];
#ifdef AUI_TDS
void channel_read_task()
#else
static void * channel_read_task()
#endif
{   
    unsigned int  read_size = 0;
	//int i =0;
    while(1)
    {
        if(ch_read_task_quit_flg != 0)
        {
            AUI_PRINTF("%s -> it is time to go out\n", __FUNCTION__);
            break;
        }
        read_size = 0;
        if(g_hdl_dmx_channel!= 0)
        {
            
            aui_dmx_channel_sync_get_section_data_ext(
                g_hdl_dmx_channel,
                NULL,
                RCV_BUF_LEN,
                g_buf,
                &read_size,
                6);

            if(read_size)
            {
                g_section_cnt++;
                
                AUI_PRINTF("chl:0x%x,read_size:%d\n",g_hdl_dmx_channel,read_size);
                AUI_SLEEP(4000);

                #if 0
                for(i = 0;i< read_size;i++)
                {
                    AUI_PRINTF("0x%x ",g_buf[i]);
                }
                AUI_PRINTF("\n");
                #endif
            }
            
            read_size = 0;
            if(g_hdl_filter)
            {
                aui_dmx_channel_sync_get_section_data_ext(
                NULL,
                g_hdl_filter,
                RCV_BUF_LEN,
                g_buf,
                &read_size,
                10);
                if(read_size)
                {
                    g_filter_cnt++;
                    
                    AUI_PRINTF("flt:0x%x,read_size:%d\n",g_hdl_filter,read_size);
                    AUI_SLEEP(6000);
                    #if 0
                    for(i = 0;i< read_size;i++)
                    {
                        AUI_PRINTF("0x%x ",g_buf[i]);
                    }
                    AUI_PRINTF("\n");
                    #endif
                
                }
            }
            else
            {
                AUI_SLEEP(20);
            }
        }
        else
        {
            AUI_SLEEP(20);
        }
    }
#ifdef AUI_TDS
    aui_os_task_crit_start();
#else
    pthread_mutex_lock(&mutex_read_channel);
#endif
    ch_read_task_quit_flg++;
    AUI_PRINTF("%s -> ch_read_task_quit_flg: %d\n", __FUNCTION__, ch_read_task_quit_flg);
#ifdef AUI_TDS
    aui_os_task_crit_stop();
#else
    pthread_mutex_unlock(&mutex_read_channel);
	return NULL;
#endif
}

#ifdef AUI_TDS
void channel_read_task2()
#else
static void *channel_read_task2()
#endif
{   
    unsigned int  read_size = 0;
	//int i =0;
    while(1)
    {
        if(ch_read_task_quit_flg !=0)
        {
            AUI_PRINTF("%s -> it is time to go out\n", __FUNCTION__);
            break;
        }
        read_size = 0;
        if(g_hdl_dmx_channel!= 0)
        {

            if(g_hdl_filter_ddb)
            {
                #if 0
                (void)read_size;
                printf("just do not read section data, to trick overflow cb.\n");
                #else
                aui_dmx_channel_sync_get_section_data(
                NULL,
                g_hdl_filter_ddb,
                RCV_BUF_LEN,
                g_buf_ddb,
                &read_size,
                10);
                if(read_size)
                {
                    g_filter_cnt_ddb++;
                    AUI_PRINTF("flt:0x%x ,read_size:%d\n",g_hdl_filter_ddb,read_size);

                    #if 0
                    for(i = 0;i< read_size;i++)
                    {
                        AUI_PRINTF("0x%x ",g_buf_ddb[i]);
                    }
                    AUI_PRINTF("\n");
                    #endif
                }
                #endif
            }
            else
            {
                AUI_SLEEP(20);
            }
        }
        else
        {
            AUI_SLEEP(20);
        }
    }
#ifdef AUI_TDS
    aui_os_task_crit_start();
#else
    pthread_mutex_lock(&mutex_read_channel);
#endif
    ch_read_task_quit_flg++;
    AUI_PRINTF("%s -> ch_read_task_quit_flg: %d\n", __FUNCTION__, ch_read_task_quit_flg);
#ifdef AUI_TDS
    aui_os_task_crit_stop();
#else
    pthread_mutex_unlock(&mutex_read_channel);
	return NULL;
#endif
}

aui_hdl g_hdl_dmx=NULL;
unsigned int  g_delay_ms = 20000;
#define MASK_MAX_LEN (32)
#define PAT_TBID (0)
#define DSI_TBID (0x3b)
#define DDB_TBID (0x3c)
unsigned short g_test_tb_pid = 0;
unsigned char g_test_tbid = PAT_TBID;
unsigned char g_test_tbid2 = PAT_TBID;

static long fool_sectioncb
(
    aui_hdl filter_handle,
    unsigned char *p_section_data_buf_addr,
    unsigned long ul_section_data_len,
    void *usr_data,
    void *reserved
)
{
    AUI_PRINTF("\nfil:%d,buf:0x%x,len:%d\n",
    filter_handle,p_section_data_buf_addr,ul_section_data_len);
	return 0;
}

unsigned char dst_buf[0x10000];
long test_aui_dmx_event_proc(void *p_user_hdl,aui_dmx_handle_type type,
    aui_dmx_event_type event_id,void* rptdata,void* usr_param)
{
    aui_p_st_dmx_data_overflow_rpt  p_rpt_data = rptdata;

    unsigned char* data = dst_buf;

    if(!p_rpt_data)
    {
        return -1;
    }

    if((p_rpt_data->len_right + p_rpt_data->len_left) <=0x10000)
    {
        if(p_rpt_data->len_right)
        {
            memcpy(data,(unsigned char*)(p_rpt_data->ringbuf_right),p_rpt_data->len_right);
            data+= p_rpt_data->len_right;
        }

        if(p_rpt_data->len_left)
        {
            memcpy(data,(unsigned char*)(p_rpt_data->ringbuf_left),p_rpt_data->len_left);
        }
        return 0;   
    }
    else
    {   
        return -1;
    }
    

}

static int g_double_check_flg = 0;
static  void * g_p_user_hdl = NULL;
static  aui_dmx_handle_type g_type = AUI_DMX_HANDLE_FILTER;
static  aui_dmx_event_type g_event_id = AUI_DMX_EVENT_SYNC_GET_DATA_OVERFLOW;
static  void * g_rptdata = NULL;
static  void * g_usr_param = NULL;

long test_aui_dmx_event_report(void *p_user_hdl,aui_dmx_handle_type type,
    aui_dmx_event_type event_id,void* rptdata,void* usr_param)
{
    //send  a msg packed with p_user_hdl ,type ,event_id ,rptdata
    //because for serialized proc model ,globle var is ok.
    g_double_check_flg = 1;

    g_p_user_hdl = p_user_hdl;
    g_type = type;
    g_event_id = event_id;
    g_rptdata = rptdata;
    g_usr_param= usr_param;
    
    while(g_double_check_flg!=2)
    {
        AUI_SLEEP(1);
    }

    g_double_check_flg = 0;
    //make sure msg received  and then return

#if 0
    int i=0;
    aui_st_dmx_data_overflow_rpt *p_rpt=rptdata;
    unsigned char *p_buf_tmp=p_rpt->ringbuf_right;
    AUI_PRINTF("\r\n right:");
    for(i=0;i<p_rpt->len_right;i++)
    {
        AUI_PRINTF("[%02x]-",p_buf_tmp[i]);
    }
    p_buf_tmp=p_rpt->ringbuf_left;
    AUI_PRINTF("\r\n left:");
    for(i=0;i<p_rpt->len_left;i++)
    {
        AUI_PRINTF("[%02x]-",p_buf_tmp[i]);
    }
#endif
        
    return 0;
}

//monitor thread 
#ifdef AUI_TDS
void test_overflow_monitor()
#else
static void *test_overflow_monitor()
#endif
{
    //wait a msg if ok
    while(1)
    {
        if(ch_read_task_quit_flg !=0)
        {
            AUI_PRINTF("%s -> it is time to go out\n", __FUNCTION__);
            break;
        }
        if(g_double_check_flg == 1)
        {
            test_aui_dmx_event_proc(g_p_user_hdl, g_type, g_event_id, g_rptdata,g_usr_param);   
            g_double_check_flg = 2;
        }
        else
        {
            AUI_SLEEP(1);
        }   
    }
#ifdef AUI_TDS
    aui_os_task_crit_start();
#else
    pthread_mutex_lock(&mutex_read_channel);
#endif
    ch_read_task_quit_flg++;
    AUI_PRINTF("%s -> ch_read_task_quit_flg: %d\n", __FUNCTION__, ch_read_task_quit_flg);
#ifdef AUI_TDS
    aui_os_task_crit_stop();
#else
    pthread_mutex_unlock(&mutex_read_channel);
	return NULL;
#endif
}

void* g_test_event_cb_param  = NULL;
unsigned long sample_dmx_sync_get_section(unsigned long *argc,char **argv,char *sz_out_put)
{   
    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
    if(*argc==2)
    {
        //s_ulfreq=ATOI(argv[0]);
        //s_ulsymbo=ATOI(argv[1]);
        g_test_tb_pid = ATOI(argv[0]);  
        g_test_tbid = ATOI(argv[1]);
        g_test_tbid2 = g_test_tbid;
    }
    else
    {
        AUI_PRINTF("%s -> To get section in sync mode, you can input :\n",__FUNCTION__);
        AUI_PRINTF("5 pid,table id\n");
        AUI_PRINTF("For example to get PAT table:\n");
        AUI_PRINTF("5 0,0\n");
        AUI_SLEEP(2000);
        return -1;
    }

    AUI_PRINTF("%s -> pid:%d, table id:%d\n", __FUNCTION__, g_test_tb_pid, g_test_tbid);    
#ifdef AUI_TDS
    aui_attr_task attr_task;
    MEMSET(&attr_task,0,sizeof(aui_attr_task));
    strcpy(attr_task.sz_name,"chread");
    attr_task.ul_priority=17;
    attr_task.p_fun_task_entry=(aui_p_fun_task_entry)channel_read_task;
    attr_task.ul_quantum=10;
    attr_task.ul_stack_size=0x4000;
    AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_ch_read_task));

    attr_task.p_fun_task_entry=(aui_p_fun_task_entry)channel_read_task2;
    AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_ch_read_task2));

    strcpy(attr_task.sz_name,"ofck");
    attr_task.ul_priority=17;
    attr_task.p_fun_task_entry=(aui_p_fun_task_entry)test_overflow_monitor;
    attr_task.ul_quantum=10;
    attr_task.ul_stack_size=0x4000;
    AUI_TEST_CHECK_RTN(aui_os_task_create(&attr_task,&g_p_hdl_overflow_proc_task)); 
#else
    AUI_PRINTF("%s, create thread for channel_read_task\n", __func__);
    if(pthread_create(&g_p_hdl_ch_read_task, NULL, channel_read_task, NULL))
    {
        AUI_PRINTF("%s - %d: ->Create thread fail\n", __FUNCTION__, __LINE__);        
        return -1;
    }
    AUI_PRINTF("%s, create thread for channel_read_task2\n", __func__);
    if(pthread_create(&g_p_hdl_ch_read_task2, NULL, channel_read_task2, NULL))
    {
        AUI_PRINTF("%s - %d: ->Create thread fail\n", __FUNCTION__, __LINE__);        
        return -1;
    }
    AUI_PRINTF("%s, create thread for test_overflow_monitor\n", __func__);
    if(pthread_create(&g_p_hdl_overflow_proc_task, NULL, test_overflow_monitor, NULL))
    {
        AUI_PRINTF("%s - %d: ->Create thread fail\n", __FUNCTION__, __LINE__);        
        return -1;
    }
#endif
    g_section_cnt = g_filter_cnt = g_filter_cnt_ddb=0;
    g_hdl_dmx_channel = g_hdl_filter = g_hdl_filter_ddb = NULL;

    ch_read_task_quit_flg = 0;

    aui_attr_dmx attr_dmx;
    aui_attr_dmx_channel attr_channel;
    aui_attr_dmx_filter attr_filter;

    unsigned char mask[MASK_MAX_LEN] = {0xff,0x80};
    unsigned char value[MASK_MAX_LEN] = {g_test_tbid,0x80};
    unsigned char reverse[MASK_MAX_LEN] = {0};

    unsigned char mask_ddb[MASK_MAX_LEN] = {0xff,0x80};
    unsigned char value_ddb[MASK_MAX_LEN] = {g_test_tbid2,0x80};

    // Just care for the first BYTE
    unsigned long ul_mask_val_len = 2;

    unsigned long ul_mask_val_len_ddb = 2;
    // Dont do CRC check for the section data
    unsigned char uc_crc_check=0;
    // Muti-times capture the section,if uc_continue_capture_flag==0,then just the first section will call the user callback
    unsigned char uc_continue_capture_flag=1;
    
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&attr_channel,0,sizeof(aui_attr_dmx_channel));
    MEMSET(&attr_filter,0,sizeof(aui_attr_dmx_filter));
    
    //Open the first dmx device
    attr_dmx.uc_dev_idx=0;
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, attr_dmx.uc_dev_idx, &g_hdl_dmx))
    {
        aui_dmx_open(&attr_dmx, &g_hdl_dmx);    
        aui_dmx_start(g_hdl_dmx, &attr_dmx);
    }

    if(g_hdl_dmx==NULL)
    {
        AUI_PRINTF("%s - %d: get dmx handle fail!\n", __FUNCTION__, __LINE__);
        goto FUNC_OUT;
    }
    
    attr_channel.us_pid = g_test_tb_pid;
    //Config the channel data type are section data
    attr_channel.dmx_data_type = AUI_DMX_DATA_SECT;
    attr_channel.dmx_channel_sec_sync_get_data_support = 1;
    attr_channel.event_cb = test_aui_dmx_event_report;
    attr_channel.event_cb_param = g_test_event_cb_param;
    //Open the dmx channel device
    aui_dmx_channel_open(g_hdl_dmx, &attr_channel, &g_hdl_dmx_channel);
    //Start the dmx channel device    
    aui_dmx_channel_start(g_hdl_dmx_channel, &attr_channel);

    //Open the dmx filter device
    attr_filter.dmx_fil_sec_data_sync_get_support = 1;
    attr_filter.event_cb=test_aui_dmx_event_report;
    aui_dmx_filter_open(g_hdl_dmx_channel,&attr_filter,&g_hdl_filter);

    attr_filter.event_cb=NULL;
    aui_dmx_filter_open(g_hdl_dmx_channel,&attr_filter,&g_hdl_filter_ddb);

    //The fun_sectionCB are the callback for app and implement by user
    aui_dmx_reg_sect_call_back(g_hdl_filter,fool_sectioncb);

    aui_dmx_reg_sect_call_back(g_hdl_filter_ddb,fool_sectioncb);
    
    //Configure the filter:user want to continue get all the table that the first byte equal 0x80
    aui_dmx_filter_mask_val_cfg(g_hdl_filter,mask,value,reverse, ul_mask_val_len,uc_crc_check,uc_continue_capture_flag);

    aui_dmx_filter_mask_val_cfg(g_hdl_filter_ddb,mask_ddb,value_ddb,reverse, ul_mask_val_len_ddb,uc_crc_check,uc_continue_capture_flag);
    //Start the dmx filter device
    aui_dmx_filter_start(g_hdl_filter, &attr_filter);

    aui_dmx_filter_start(g_hdl_filter_ddb, &attr_filter);
    
    AUI_PRINTF("\nPlease wait about 20s to receive section\n");
    AUI_SLEEP(g_delay_ms);

    ch_read_task_quit_flg = 1;

    while(ch_read_task_quit_flg != 4)
    {
        AUI_SLEEP(1);
    }

    ch_read_task_quit_flg = 0;
FUNC_OUT:
    aui_dmx_filter_stop(g_hdl_filter,NULL); 
    aui_dmx_filter_stop(g_hdl_filter_ddb,NULL);
    aui_dmx_filter_close(&g_hdl_filter);
    aui_dmx_filter_close(&g_hdl_filter_ddb);
    g_hdl_filter = NULL;
    g_hdl_filter_ddb = NULL;
    aui_dmx_channel_stop(g_hdl_dmx_channel, NULL);
    aui_dmx_channel_close(&g_hdl_dmx_channel);
    g_hdl_dmx_channel = NULL;   
    if(g_section_cnt||g_filter_cnt||g_filter_cnt_ddb)
    {
        AUI_PRINTF("\nGet %d channel section in %d ms\n",g_section_cnt,g_delay_ms);
        AUI_PRINTF("\nGet %d filter  section in %d ms\n",g_filter_cnt,g_delay_ms);
        AUI_PRINTF("\nGet %d filter  section in %d ms\n",g_filter_cnt_ddb,g_delay_ms);
        return 0;
    }
    else
    {
        return -1;
    }
    
}

#ifdef AUI_TDS
static void ali_get_pcr_cb(aui_dmx_pcr_param *pcr_info)
{
    if(pcr_info == NULL)
    {
        AUI_PRINTF("Enter %s: pcr_info == NULL \n",pcr_info);
        return;
    }
    
    AUI_PRINTF("1msb: %d ,32lsb : %d,extension_9b:%d\n",pcr_info->pcr_base_1msb,pcr_info->pcr_base_32lsb,\
        pcr_info->pcr_extension_9b);
    
    return;
}

unsigned long sample_dmx_get_pcr(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_attr_dmx attr_dmx;
    aui_hdl hdl_dmx=NULL;
    aui_attr_dmx attr_dmx_pcr; 
    unsigned short s_pcr_pid = 0x1fff;
    AUI_RTN_CODE ret = AUI_RTN_FAIL;

    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
    if(*argc==1)
    {
        s_pcr_pid = ATOI(argv[0]);
    }
    else
    {
        AUI_PRINTF("%s -> To get pcr data, you can input :\n",__FUNCTION__);
        AUI_PRINTF("4 pcr_pid\n");
        AUI_PRINTF("For example to get pcr data of cctv2\n");
        AUI_PRINTF("4 8190\n");
        AUI_SLEEP(2000);
        return -1;
    }
    memset(&attr_dmx, 0, sizeof(attr_dmx));
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl_dmx))
    {
        AUI_PRINTF("dmx device has been closed!!\n");
        aui_dmx_open(&attr_dmx, &hdl_dmx);  
    }
    AUI_SLEEP(1000);
    memset(&attr_dmx_pcr,0,sizeof(aui_attr_dmx));   
    attr_dmx_pcr.uc_dev_idx = 0;
    attr_dmx_pcr.dmx_pcr_info.get_pcr_cb = ali_get_pcr_cb;
    attr_dmx_pcr.dmx_pcr_info.param = NULL;//can input usr param
    attr_dmx_pcr.dmx_pcr_info.pid = s_pcr_pid;
    
    ret = aui_dmx_pcr_reg(hdl_dmx,&attr_dmx_pcr);

    if(ret != AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("pcr register failed!\n");
        asm(".word 0x7000003f; nop");
        return AUI_RTN_FAIL;
    }

    AUI_SLEEP(30000);

    ret = aui_dmx_pcr_unreg(hdl_dmx,&attr_dmx_pcr);

    if(ret != AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("pcr unregister failed!\n");
        asm(".word 0x7000003f; nop");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}
#endif

unsigned short g_test_pes_pid = 513;

long int test_aui_pes_data_callback(void *p_user_hdl,unsigned char* pbuf,unsigned long ul_size,void* usrdata)
{
    int i=0;
    AUI_PRINTF("\r\n pbuf is [0x%x],size is [%d]!\n",pbuf,ul_size);
    AUI_PRINTF("The latter 20 byte is \n");
    for(i=0;i<20;i++)
    {
        AUI_PRINTF("[%02x]-",pbuf[i]);
    }   
    AUI_PRINTF("\n");
	return 0;
}

unsigned long sample_dmx_get_pes_data(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_attr_dmx attr_dmx;
    aui_attr_dmx_channel attr_channel;
    aui_attr_dmx_filter attr_filter;
    aui_hdl hdl_dmx=NULL;
    aui_hdl hdl_channel=NULL;
    aui_hdl hdl_filter=NULL;
    
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&attr_channel,0,sizeof(aui_attr_dmx_channel));
    MEMSET(&attr_filter,0,sizeof(aui_attr_dmx_filter));

    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
    if(*argc==1)
    {
        g_test_pes_pid = ATOI(argv[0]); 
    }
    else
    {
        AUI_PRINTF("%s -> To get PES data, you can input :\n",__FUNCTION__);
        AUI_PRINTF("2 pid\n");
        AUI_PRINTF("For example to get video pes data of cctv2\n");
        AUI_PRINTF("2 513\n");
        AUI_SLEEP(2000);
        return AUI_RTN_FAIL;
    }   
    attr_dmx.uc_dev_idx=0;
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, attr_dmx.uc_dev_idx, &hdl_dmx))
    {
        if(aui_dmx_open(&attr_dmx, &hdl_dmx))
        {
            AUI_PRINTF("%s -> aui_dmx_open fail\n", __FUNCTION__);
            return AUI_RTN_FAIL;
        }
    }
    //Start the dmx0 device
    aui_dmx_start(hdl_dmx, &attr_dmx);
    //Capture the channel are bind with PID(value is 0) 
    attr_channel.us_pid = g_test_pes_pid;
    //Config the channel data type are TS raw data
    attr_channel.dmx_data_type = AUI_DMX_DATA_PES;
    attr_channel.dmx_channel_pes_callback_support = 1;
    //Open the dmx channel device
    aui_dmx_channel_open(hdl_dmx, &attr_channel, &hdl_channel);
    //Start the dmx channel device    
    aui_dmx_channel_start(hdl_channel, &attr_channel);
    //Open the dmx filter device
    aui_dmx_filter_open(hdl_channel,&attr_filter,&hdl_filter);
    aui_dmx_reg_pes_call_back(hdl_filter,test_aui_pes_data_callback,NULL);   
    //Start the dmx filter device
    aui_dmx_filter_start(hdl_filter, &attr_filter);
    AUI_SLEEP(30000);

    aui_dmx_filter_stop(hdl_filter,NULL);
    aui_dmx_filter_close(&hdl_filter);
    aui_dmx_channel_stop(hdl_channel, NULL);    
    aui_dmx_channel_close(&hdl_channel);

	return AUI_RTN_SUCCESS;
}

unsigned long  sample_dmx_get_stc(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    aui_pts pts_cur_h = {0,0};
    unsigned long  stc_cur = 0;
    unsigned int i =0;
    aui_attr_dmx attr_dmx;
    aui_hdl hdl_dmx=NULL;

    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n\n");
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));    
    if(AUI_RTN_SUCCESS!=aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl_dmx))
    {
        AUI_PRINTF("dmx device is close!!\n");
        if(aui_dmx_open(&attr_dmx, &hdl_dmx))
        {
            AUI_PRINTF("%s -> aui_dmx_open fail!!\n", __FUNCTION__);
            return AUI_RTN_FAIL;
        }
    }
    AUI_SLEEP(1000);
    while(i<20)
    {   
        ret = aui_dmx_get(hdl_dmx,AUI_DMX_GET_PTS,(void*)(&pts_cur_h));
        
        if(ret != AUI_RTN_SUCCESS)
        {
            AUI_PRINTF("********get cur pts err!!**********\n");
            return AUI_RTN_FAIL;
        }
        else
        {
            AUI_PRINTF("^^^^^^^cur pts: msb: 0x%x,lsb: 0x%x^^^^^^^\n",pts_cur_h.pts_1msb,pts_cur_h.pts_32lsb);
        }

        ret = aui_dmx_get(hdl_dmx,AUI_DMX_GET_CUR_STC,(void*)(&stc_cur));
        if(ret != AUI_RTN_SUCCESS)
        {
            AUI_PRINTF("********get cur stc err!!**********\n");
            return AUI_RTN_FAIL;
        }
        else
        {
            AUI_PRINTF("-------cur stc: 0x%x-------\n",stc_cur);
        }

        i++;
        AUI_SLEEP(2000);
    }
    return AUI_RTN_SUCCESS;
}

//reg start 
void aui_load_tu_dmx(void)
{
    aui_tu_reg_group("dmx", "Dmx test cases");
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_dmx_help, "DMX help");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, sample_dmx_get_pat_table, "Get pat table");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, sample_dmx_get_pes_data, "Get pes data");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, sample_dmx_get_stc, "Dmx get stc");   
#ifdef AUI_TDS
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, sample_dmx_get_pcr, "Dmx get pcr data");
#endif
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, sample_dmx_sync_get_section, "Get section in sync mode");
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, sample_dmx_get_raw_data, "Get raw data");
    aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, sample_dmx_record_ts_data, "Record ts data");
}
