#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "aui_ini_config.h"
#else
#include <api/libfs2/stdio.h>
#include <osal/osal.h>     
#endif

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <aui_common.h>

#include "aui_help_print.h"
#include "aui_vbi_test.h"
#include "aui_dmx_test.h"
#include "aui_test_stream.h"

/* TSG parameters */
#define PKT_SIZE    188
#define RD_PKT_NUM  512
#define RD_BLK_SIZE 1024

#define VBI_WRITE_STOP        0
#define VBI_WRITE_RUN         1

#define TTX_PES_HEADER_LENGTH 45
#define TEST_VBI_BUFFER_SIZE  24 * 1024

extern aui_hdl g_pHdlNim[0];
extern aui_hdl g_p_hdl_dmx0;
static aui_hdl ttx_handle = NULL;
//#ifdef AUI_TDS
static aui_hdl cc_handle = NULL;
//#endif
static int test_thread_task_flag; 

#ifdef AUI_LINUX
static pthread_t test_vbi_thread;
static pthread_mutex_t mutex_wait;  
static pthread_cond_t  cond_wait; 
#else
static OSAL_ID vbi_output_task_id = OSAL_INVALID_ID;
#endif

static void vbi_thread_task_exit()
{
#ifdef AUI_LINUX    
pthread_exit(NULL); 
#else      
osal_task_exit(0);
#endif
}

#ifdef AUI_LINUX
static void *vbi_write_task_thread(void *p_param)
#else
static void vbi_write_task_thread(unsigned long param1,unsigned long param2)
#endif
{	
#ifdef AUI_LINUX
	aui_ring_buf *rbuf  = NULL;
	aui_vbi_ttx_line line_data;
	unsigned long read_len= 0;
	unsigned long data_len = 0;

	if (NULL == p_param) {
		vbi_thread_task_exit();
	}
	rbuf = (aui_ring_buf *)p_param;
	while(VBI_WRITE_RUN == test_thread_task_flag) {
		aui_common_ring_buf_data_len(rbuf, &data_len);
		if (data_len == 0) {
			//Thread sleeps when no data comes 
			pthread_mutex_lock(&mutex_wait);
			pthread_cond_wait(&cond_wait, &mutex_wait);
			pthread_mutex_unlock(&mutex_wait);
		}
		//Send a line data
		aui_common_ring_buf_rd(rbuf,VBI_TTX_LINE_LENGTH,&read_len,(unsigned char *)&line_data);
		aui_vbi_ttx_write_line(ttx_handle,&line_data);  
	}
	vbi_thread_task_exit();
	return NULL;
#else
    vbi_thread_task_exit();
	return;
#endif
}

static unsigned long vbi_thread_task_create(void *rbuf)
{
	if (NULL == rbuf) {
		AUI_PRINTF("rbuf is NULL!\n");
		return AUI_RTN_FAIL;
	}
#ifdef AUI_LINUX
    if(pthread_mutex_init(&mutex_wait, NULL) != 0) {
		return AUI_RTN_FAIL;	
	} 
	if(pthread_cond_init(&cond_wait, NULL) != 0) {
		return AUI_RTN_FAIL;	
	} 
	if(pthread_create(&test_vbi_thread, NULL, vbi_write_task_thread, (void *)rbuf)	 
			!= 0) {
		AUI_PRINTF("vbi_thread_task_create Fail!\n");
		return AUI_RTN_FAIL;	
	} 
#else
	OSAL_T_CTSK task; 
	task.itskpri = OSAL_PRI_NORMAL;
	task.task = vbi_write_task_thread;
    task.quantum = 30;
    task.stksz = 16*1024;
    task.name[0] = 'V';
    task.name[1] = 'B';
    task.name[2] = 'I';
    vbi_output_task_id = osal_task_create(&task);
    if (vbi_output_task_id == OSAL_INVALID_ID) { 			
		return AUI_RTN_FAIL;
    }   
#endif
	return AUI_RTN_SUCCESS;
}

long int test_aui_vbi_data_callback(void *p_user_hdl,unsigned char* pbuf,unsigned long ul_size,void *usrdata)
{
    unsigned char data_identifier;
    unsigned short i = 0,line_sum;
	unsigned long data_len;
	aui_vbi_ttx_line line_data;
	aui_ring_buf *rbuf  = NULL;

	if (NULL == usrdata) {
		AUI_PRINTF("\aui_vbi_data_callback Fail!\n");
		return AUI_RTN_FAIL;
	}
	rbuf = (aui_ring_buf *)usrdata;
    //Whether it is EBU Teletext Data
    data_identifier = pbuf[TTX_PES_HEADER_LENGTH];
    if((data_identifier < 0x10) ||(data_identifier > 0x1f)) {
		AUI_PRINTF("\r\n Incorrect data type !\n"); 
		return AUI_RTN_FAIL;
    }
    //A PES package contains lines number of Teletext Data
	line_sum = (ul_size - TTX_PES_HEADER_LENGTH - 1)/VBI_TTX_LINE_LENGTH;
    for(i = 0;i < line_sum;i++) {	
 		if(VBI_WRITE_STOP == test_thread_task_flag) {
			return AUI_RTN_SUCCESS;
		}
		//TTX PES Header length is 45 bytes, data_identifier is 1 byte, a line of data is 46 bytes
		MEMCPY(&line_data, (unsigned char*)&pbuf[TTX_PES_HEADER_LENGTH + 1 + i * VBI_TTX_LINE_LENGTH], VBI_TTX_LINE_LENGTH);
		if(aui_common_ring_buf_wt(rbuf,VBI_TTX_LINE_LENGTH,(unsigned char *)&line_data)) {
            AUI_PRINTF("ring buf write fail.\n");
            return AUI_RTN_FAIL;
        }
		aui_common_ring_buf_data_len(rbuf, &data_len);
	#ifdef AUI_LINUX
       //Wake up the thread when the data comes. 
        if (data_len > 0) {
			pthread_mutex_lock(&mutex_wait);
			pthread_cond_signal(&cond_wait); 
			pthread_mutex_unlock(&mutex_wait);
		} 
	#endif
	}
	return AUI_RTN_SUCCESS;
	/*unuse*/
	(void)p_user_hdl;
}

static void show_vbi_tsg_usage() {
	AUI_PRINTF("command as fallow:\n");
    AUI_PRINTF("cmd_num [path],[vpid],[apid],[ppid],[video format],[audio format],[dis format],[ttxpid]\n");
    AUI_PRINTF("such as :1 /mnt/uda1/ch7.ts,769,770,769,0,1,0,772\n");
	AUI_PRINTF("such as :1 /mnt/usb/sda1/ch7.ts,769,770,769,0,1,0,772\n");
}

static unsigned long test_vbi_ttx(unsigned long *argc,char **argv,char *sz_out_put)
{	
	aui_ttx_start_param start_param;
	aui_vbi_open_param open_param;

	MEMSET(&open_param, 0, sizeof(open_param));
	open_param.vbi_index = 0;
	open_param.data_type = AUI_VBI_DATA_TYPE_TELETEXT;
	//aui_vbi_init(NULL,NULL);
	if (aui_vbi_open(&open_param, &ttx_handle)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
	}
	//Teletext only supports SD output
	if (aui_vbi_select_output_dev(ttx_handle, VBI_OUTPUT_SD)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
	}
	
	MEMSET(&start_param, 0, sizeof(start_param));
	start_param.ttx_pid = atoi(argv[7]);
	start_param.dmx_index = 0;

	AUI_PRINTF("%s %d ttx pid is %d\n", __FUNCTION__, __LINE__,  start_param.ttx_pid);
	if (aui_vbi_ttx_start(ttx_handle, &start_param)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
	}
	return AUI_RTN_SUCCESS;
}

/**@file
*    @note      Copyright(C) ALi Corporation. All rights reserved.
*   
*   this sample is used to test TSG playing clear TS stream from local media.
* 
*   TS route:local media file-->TSG-->TSI-->DMX->|-->DECA------->SND-->|--->HDMI
*                                                |-->DECV-->|--->DIS-->|
*                                                |-->SUBT-->|
*
**/
static unsigned long test_vbi_using_tsg(unsigned long *argc,char **argv,char *sz_out_put)
{
    struct ali_aui_hdls aui_hdls;
    struct aui_attr_tsg tsg_attr;
    struct ali_app_modules_init_para init_para;
    unsigned long pkt_empty,pkt_in_tsg,pkt_cnt;
    int len, ret;
    FILE *file;
    char ch=0;

    *sz_out_put = 0;    
    MEMSET(&aui_hdls, 0, sizeof(struct ali_aui_hdls));
    unsigned long i;
    AUI_PRINTF("*argc = %d,argv: ",*argc);
    for(i = 0;i < *argc;i++){
        AUI_PRINTF("%s,",argv[i]);
    }
    AUI_PRINTF("\n");
    if (8 != *argc) {
        show_vbi_tsg_usage();
        return 0;
    }
    len = (PKT_SIZE * RD_PKT_NUM) / RD_BLK_SIZE;  //94K 
    pkt_cnt = RD_PKT_NUM;

    /*init init_para variable*/
    ali_aui_init_para_for_test_tsg(argc,argv,&init_para);
	
    //init tsg device
    if(ali_app_tsg_init(&init_para.init_para_tsg, &aui_hdls.tsg_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsg_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSG opened\n");

    //init tsi device
    if (ali_app_tsi_init(&init_para.init_para_tsi, &aui_hdls.tsi_hdl)) {
        AUI_PRINTF("\r\n ali_app_tsi_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI TSI opened\n");

#ifdef AUI_LINUX
    //init dis device
    if(ali_app_dis_init(init_para.init_para_dis,&aui_hdls.dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DIS opened\n");
#endif

    //init decv device
    if (ali_app_decv_init(init_para.init_para_decv,&aui_hdls.decv_hdl)) {
        AUI_PRINTF("\r\n ali_app_decv_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI DECV opened\n");

    // init deca device
    if (ali_app_audio_init(init_para.init_para_audio,&aui_hdls.deca_hdl,&aui_hdls.snd_hdl)) {
        AUI_PRINTF("\r\n ali_app_audio_init failed!");
        goto err_tsg;
    }
    AUI_PRINTF("AUI audio opened[%08x]\n",(int)aui_hdls.deca_hdl);

    //set video and audio  synchronous ways,
    if(aui_decv_sync_mode(aui_hdls.decv_hdl,AUI_STC_AVSYNC_AUDIO)){
        AUI_PRINTF("Set AUI_DEVC_SYNC_AUDIO fail\n");
		goto err_tsg;
    }

    /*init dmx devce*/
    if (set_dmx_for_create_av_stream(init_para.dmx_create_av_para.dmx_id, 
                        init_para.dmx_create_av_para.video_pid,
                        init_para.dmx_create_av_para.audio_pid,
                        init_para.dmx_create_av_para.audio_desc_pid, 
                        init_para.dmx_create_av_para.pcr_pid, 
                        &aui_hdls.dmx_hdl)) {
            AUI_PRINTF("\r\n set dmx failed!");
            goto err_tsg;
        }
    AUI_PRINTF("AUI DMX opened[%08x]\n",(int)aui_hdls.dmx_hdl);

    aui_dmx_data_path path;
	MEMSET(&path, 0, sizeof(aui_dmx_data_path));
    path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
    if (aui_dmx_data_path_set(aui_hdls.dmx_hdl, &path)) {
        AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
        goto err_tsg;
    }
    AUI_PRINTF("dmx data path set %d\n", path.data_path_type);

    /*set TS of dmx device from local media player*/
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_SET_AVSYNC_SOURCE_TYPE,(void *)AUI_AVSYNC_FROM_HDD_MP)) {
        AUI_PRINTF("aui_dmx_set AUI_DMX_SET_AVSYNC_SOURCE_TYPE fail\n");
        goto err_tsg;
    }
    
    if (aui_dmx_set(aui_hdls.dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
        AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
        goto err_tsg;
    }
   
    AUI_PRINTF("tsg file %s\n", argv[0]);
    file = fopen(argv[0], "r");
    if (!file) {
        AUI_PRINTF("tsg file open errno\n");
        goto err_tsg;
    }
    void *buffer = malloc((RD_BLK_SIZE*len));
    if(NULL == buffer) {
        AUI_PRINTF("tsg malloc errno\n");
        goto err_tsg;
    }
	//Turn on the ttx test
    test_vbi_ttx(argc,argv,sz_out_put);

    AUI_PRINTF("Press 's' to stop playing\n");
    while (1) {          //Play stream
        if (AUI_RTN_SUCCESS == aui_test_get_user_key_input(&ch)){
            if (ch == 's') {
                break;
            }
        }
        ret = fread(buffer, RD_BLK_SIZE, len, file);
        if (0 == ret) {
            AUI_PRINTF("tsg file open errno \n");
            fclose(file);
            goto err_tsg;
        }
    #ifdef AUI_LINUX
        if (ret != len) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len);
            break;
        }
    #else
        if (ret != len*RD_BLK_SIZE) {
            AUI_PRINTF("file read %d block instead of %d\n",ret, len*RD_BLK_SIZE);
            break;
        }
    #endif

        if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
            AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
            goto err_tsg;
        }
        if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
            AUI_PRINTF("aui_dmx_get error\n");

        while(pkt_empty < (pkt_cnt + pkt_in_tsg)) {
            AUI_SLEEP(10);
            if (aui_tsg_check_remain_pkt(aui_hdls.tsg_hdl, &pkt_in_tsg)) {
                AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
                goto err_tsg;
            }
            if (aui_dmx_get(aui_hdls.dmx_hdl,AUI_DMX_GET_FREE_BUF_LEN,&pkt_empty))
                AUI_PRINTF("aui_dmx_get error\n");
        }
        /* push data in TSG */
        tsg_attr.ul_addr = (unsigned char *)buffer;
        tsg_attr.ul_pkg_cnt = pkt_cnt;
        tsg_attr.uc_sync_mode = 0;
        ret = aui_tsg_send(aui_hdls.tsg_hdl, &tsg_attr);
        if (ret) {
            AUI_PRINTF("\naui_tsg_send error 0x%x\n", ret);
            fclose(file);
            goto err_tsg;
        }
    }
    fclose(file);

err_tsg:
    ali_app_deinit(&aui_hdls);
	AUI_PRINTF("vbi stop  !\n");
	if (aui_vbi_ttx_stop(ttx_handle)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
	}
	AUI_PRINTF("vbi close  !\n");

	if (aui_vbi_close(ttx_handle)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
	}
    return AUI_RTN_SUCCESS;
}

static unsigned long test_vbi_using_pes(unsigned long *argc,char **argv,char *sz_out_put)
{
    aui_ttx_start_param start_param;
    unsigned short test_pes_pid;
    aui_attr_dmx attr_dmx;
    aui_attr_dmx_channel attr_channel;
    aui_attr_dmx_filter attr_filter;
    aui_hdl hdl_dmx=NULL;
    aui_hdl hdl_channel=NULL;
    aui_hdl hdl_filter=NULL;
    aui_ring_buf *vbi_rbuf = NULL;
    vbi_rbuf = MALLOC(sizeof(aui_ring_buf));	
	
    if( NULL == vbi_rbuf) {
        AUI_PRINTF("mem fail\n");
        return AUI_RTN_FAIL;
    }
	
    AUI_PRINTF("\n\nPlease make sure a channel is playing, or else you will get nothing\n");
    AUI_PRINTF("\n\nPlease make sure the format is correct before testing\n\n");
    
    if(*argc == 1) {
        test_pes_pid = ATOI(argv[0]); 
    } else {
        AUI_PRINTF("%s -> To get TTX PES data, you can input :\n",__FUNCTION__);
        AUI_PRINTF("2 pid\n");
        AUI_SLEEP(2000);
        return AUI_RTN_FAIL;
    }

    aui_vbi_open_param open_param;
    MEMSET(&open_param, 0, sizeof(open_param));
    open_param.vbi_index = 0;
    open_param.data_type = AUI_VBI_DATA_TYPE_TELETEXT;
	
    if (aui_vbi_open( &open_param, &ttx_handle)) {
        AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
        return AUI_RTN_FAIL;
    }

    MEMSET(&start_param, 0, sizeof(start_param));
    //when pid is invalid, the application should write TTX data into aui_vbi by aui_vbi_ttx_write_line
    start_param.ttx_pid = AUI_INVALID_PID;
	
    if (aui_vbi_ttx_start(ttx_handle, &start_param)) {
        AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
        return AUI_RTN_FAIL;
    }
    
    test_thread_task_flag = VBI_WRITE_STOP;
    MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
    MEMSET(&attr_channel,0,sizeof(aui_attr_dmx_channel));
    MEMSET(&attr_filter,0,sizeof(aui_attr_dmx_filter));  
    attr_dmx.uc_dev_idx = 0;
	
    if(AUI_RTN_SUCCESS != aui_find_dev_by_idx(AUI_MODULE_DMX, attr_dmx.uc_dev_idx, &hdl_dmx)) {
        if(aui_dmx_open(&attr_dmx, &hdl_dmx)) {
            AUI_PRINTF("%s -> aui_dmx_open fail\n", __FUNCTION__);
            return AUI_RTN_FAIL;
        }
    }
	
    //Initialize the ring buffer
    if(AUI_RTN_SUCCESS!=aui_common_init_ring_buf(
        TEST_VBI_BUFFER_SIZE,vbi_rbuf)) {
        AUI_PRINTF("%s, aui_common_init_ring_buf fail!\n", __func__);
        return AUI_RTN_FAIL;
    }
	
    aui_common_rst_ring_buf(vbi_rbuf);
    //Start the dmx0 device
    aui_dmx_start(hdl_dmx, &attr_dmx);
    //Capture the channel are bind with PID(value is 0) 
    attr_channel.us_pid = test_pes_pid;
    //Config the channel data type are TS raw data
    attr_channel.dmx_data_type = AUI_DMX_DATA_PES;
    attr_channel.dmx_channel_pes_callback_support = 1;
    //Open the dmx channel device
    aui_dmx_channel_open(hdl_dmx, &attr_channel, &hdl_channel);
    //Start the dmx channel device    
    aui_dmx_channel_start(hdl_channel, &attr_channel);
    //Open the dmx filter device
    aui_dmx_filter_open(hdl_channel,&attr_filter,&hdl_filter);
	test_thread_task_flag = VBI_WRITE_RUN;
    aui_dmx_reg_pes_call_back(hdl_filter,test_aui_vbi_data_callback,(void *)vbi_rbuf);   
    //Start the dmx filter device
    aui_dmx_filter_start(hdl_filter, &attr_filter);
    //Create task or thread to write data 
    vbi_thread_task_create((void *)vbi_rbuf);	
    AUI_SLEEP(10000);

    aui_dmx_filter_stop(hdl_filter,NULL);
    aui_dmx_filter_close(&hdl_filter);
    aui_dmx_channel_stop(hdl_channel, NULL);  
    aui_dmx_channel_close(&hdl_channel);
    test_thread_task_flag = VBI_WRITE_STOP;
    AUI_SLEEP(3000);
#ifdef AUI_LINUX
    //Make sure the thread exits. 
    pthread_mutex_lock(&mutex_wait);
    pthread_cond_signal(&cond_wait); 
    pthread_mutex_unlock(&mutex_wait);
	
    if(pthread_join(test_vbi_thread, NULL) != 0) {
        AUI_PRINTF("%s, pthread_join fail!\n", __func__);
        return AUI_RTN_FAIL;	
    } 	
	
    if(pthread_mutex_destroy(&mutex_wait) != 0) {
        AUI_PRINTF("%s, pthread_mutex_destroy fail!\n", __func__);
        return AUI_RTN_FAIL;	
    } 
	
    if(pthread_cond_destroy(&cond_wait) != 0) {
        AUI_PRINTF("%s, pthread_cond_destroy fail!\n", __func__);
        return AUI_RTN_FAIL;	
    } 
	
#endif
    aui_common_un_init_ring_buf(vbi_rbuf);
    FREE(vbi_rbuf);
    vbi_rbuf = NULL;
    
    if (aui_vbi_ttx_stop(ttx_handle)) {
        AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
        return AUI_RTN_FAIL;
    }
	
    if (aui_vbi_close(ttx_handle)) {
        AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
        return AUI_RTN_FAIL;
    }
	
    return AUI_RTN_SUCCESS;
}
//#ifdef AUI_TDS
static unsigned long test_vbi_cc_open(unsigned long *argc,char **argv,char *sz_out_put)
{	
	aui_vbi_open_param open_param;

	MEMSET(&open_param, 0, sizeof(open_param));

	//step1 : open vbi device,  vbi_index = 1 for CC, vbi_index = 0 for teletxt.
	open_param.vbi_index = 1; 
	open_param.data_type = AUI_VBI_DATA_TYPE_CLOSED_CAPTION;
	if (aui_vbi_open(&open_param, &cc_handle)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
		return AUI_RTN_FAIL;
	}

	//step2 : select_output
	if (aui_vbi_select_output_dev(cc_handle, VBI_OUTPUT_SD)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
		return AUI_RTN_FAIL;
	}
		
	
	return AUI_RTN_SUCCESS;
}

static unsigned long test_vbi_cc_start(unsigned long *argc,char **argv,char *sz_out_put)
{	
	aui_vbi_cc_start_param start_param;
	aui_hdl decv_hdl = NULL;
	aui_attr_decv attr_decv;
	unsigned int cc_type = AUI_DECV_USER_DATA_ANY; 

	if(*argc >= 1) {
        cc_type = ATOI(argv[0]); 
    }

	//step1 : make sure cc data from decv, and from which decv
	MEMSET(&attr_decv,0,sizeof(aui_attr_decv));
	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
		if (aui_decv_open(&attr_decv,&decv_hdl)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return AUI_RTN_FAIL;
		}
	}
	
    //step2: CC start
	MEMSET(&start_param,0,sizeof(aui_vbi_cc_start_param));
	start_param.cc_decv_hdl = decv_hdl;
	start_param.user_data_type = cc_type;
	if (aui_vbi_cc_start(cc_handle, &start_param)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
		return AUI_RTN_FAIL;
	}
	
	return AUI_RTN_SUCCESS;
}


static unsigned long test_vbi_cc_stop(unsigned long *argc,char **argv,char *sz_out_put)
{	
	if (aui_vbi_cc_stop(cc_handle)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
		return AUI_RTN_FAIL;
	}
	
	return AUI_RTN_SUCCESS;
}

static unsigned long test_vbi_cc_close(unsigned long *argc,char **argv,char *sz_out_put)
{	
	if (aui_vbi_close(cc_handle))
	{
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
	}
	
	return AUI_RTN_SUCCESS;
}

static void test_decv_cb_cc_user_data(void *p_user_hld, unsigned int size, unsigned int data)
{
    unsigned int i = 0;
    unsigned char *buf = (unsigned char *)data;
    
    printf("Closed caption data, size=%d\n", size);
    
    while (i < size)
    {
        printf("0x%.2x ", buf[i]);
        i++;
    }
    printf("\n");    
    return;
}

static unsigned long test_decv_user_data_filter(unsigned long *argc, char **argv, char *sz_out_put)
{
	aui_hdl decv_hdl = NULL;
	aui_attr_decv attr_decv;
	unsigned int user_data_type = AUI_DECV_USER_DATA_ANY; 

	if(*argc >= 1) {
		user_data_type = ATOI(argv[0]); 
	}

	MEMSET(&attr_decv,0,sizeof(aui_attr_decv));
	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
		if (aui_decv_open(&attr_decv,&decv_hdl)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return AUI_RTN_FAIL;
		}
	}

	struct aui_decv_callback_node callback_node;
	memset(&callback_node, 0, sizeof(callback_node));
	callback_node.type = AUI_DECV_CB_USER_DATA_PARSED;
	callback_node.callback = test_decv_cb_cc_user_data; 
	
	if (aui_decv_set(decv_hdl, AUI_DECV_SET_REG_CALLBACK,(void *)&callback_node)) {
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
		return AUI_RTN_FAIL;
	}

	//set user data type
	if (aui_decv_user_data_type_set(decv_hdl, user_data_type)){
		AUI_PRINTF("error %s: %d\n", __FUNCTION__, __LINE__);
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}

static unsigned long test_vbi_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nVBI Test Help");  
	aui_print_help_instruction_newline("\nPlease make sure display video format is correct before testing\n\n");

	/* VBI_1_HELP */
	#define 	VBI_1_HELP_PART1		"VBI test by tsg, Steps:\n"
	#define 	VBI_1_HELP_PART2		"	 1	Switch the display video format\n"
	#define 	VBI_1_HELP_PART3		"	 2	Start the vbi test\n"
	#define 	VBI_1_HELP_PART4		"Example :\n"
	#define 	VBI_1_HELP_PART5		"		 Step1:  Enter \"up\"->\"dis\"->\"1\"->\"7,0\" to switch the format\n"
	#define 	VBI_1_HELP_PART6		"		 Step2:  1 /mnt/usb/sda1/ch7.ts,769,770,769,0,1,0,772\n"
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Test the VBI module by tsg\n");
	aui_print_help_instruction_newline(VBI_1_HELP_PART1);
	aui_print_help_instruction_newline(VBI_1_HELP_PART2);
	aui_print_help_instruction_newline(VBI_1_HELP_PART3);
	aui_print_help_instruction_newline(VBI_1_HELP_PART4);
	aui_print_help_instruction_newline(VBI_1_HELP_PART5);
	aui_print_help_instruction_newline(VBI_1_HELP_PART6);

	/* VBI_2_HELP */
	#define 	VBI_2_HELP_PART1		"VBI test by pes, Steps:\n"
	#define 	VBI_2_HELP_PART2		"	 1	Switch the display video format\n"
	#define 	VBI_2_HELP_PART3		"	 2	Play the stream\n"
	#define 	VBI_2_HELP_PART4		"	 3	Start the vbi test\n"
	#define 	VBI_2_HELP_PART5		"Example :\n"
	#define 	VBI_2_HELP_PART6		"		 Step1:  Enter \"up\"->\"dis\"->\"1\"->\"7,0\" to switch the format\n"
	#define 	VBI_2_HELP_PART7 		"		 Step2:  Enter \"up\"->\"stream\"->\"play ...\"to play the stream\n"
	#define 	VBI_2_HELP_PART8	 	"		 Step3:  Enter \"2 772\"\n"
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("Get data from pes packet,and test the VBI module\n");
	aui_print_help_instruction_newline("Please make sure a channel is playing, or else you will get nothing\n");
	aui_print_help_instruction_newline(VBI_2_HELP_PART1);
	aui_print_help_instruction_newline(VBI_2_HELP_PART2);
	aui_print_help_instruction_newline(VBI_2_HELP_PART3);
	aui_print_help_instruction_newline(VBI_2_HELP_PART4);
	aui_print_help_instruction_newline(VBI_2_HELP_PART5);
	aui_print_help_instruction_newline(VBI_2_HELP_PART6);
	aui_print_help_instruction_newline(VBI_2_HELP_PART7);
	aui_print_help_instruction_newline(VBI_2_HELP_PART8);

//#ifdef AUI_TDS
	/* VBI_3_HELP */
	#define 	VBI_3_HELP_PART1		"CC test , Steps:\n"
	#define 	VBI_3_HELP_PART2		"	 1	Switch the display video format\n"
	#define 	VBI_3_HELP_PART3		"	 2	Play the stream\n"
	#define 	VBI_3_HELP_PART4		"	 3	Start the cc test\n"
	#define 	VBI_3_HELP_PART5		"Example :\n"
	#define 	VBI_3_HELP_PART6		"		 Step1:  Enter \"up\"->\"dis\"->\"1\"->\"8\" to switch the format\n"
	#define 	VBI_3_HELP_PART7 		"		 Step2:  Enter \"up\"->\"stream\"->\"play ...\"to play the stream\n"
	#define 	VBI_3_HELP_PART8	 	"		 Step3:  Enter \"3\"\n"
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline("open VBI device\n");
	aui_print_help_instruction_newline("Please make sure a channel is playing, or else you will get nothing\n");
	aui_print_help_instruction_newline(VBI_3_HELP_PART1);
	aui_print_help_instruction_newline(VBI_3_HELP_PART2);
	aui_print_help_instruction_newline(VBI_3_HELP_PART3);
	aui_print_help_instruction_newline(VBI_3_HELP_PART4);
	aui_print_help_instruction_newline(VBI_3_HELP_PART5);
	aui_print_help_instruction_newline(VBI_3_HELP_PART6);
	aui_print_help_instruction_newline(VBI_3_HELP_PART7);
	aui_print_help_instruction_newline(VBI_3_HELP_PART8);
//#endif

	/* VBI_4_HELP */
	
	#define 	VBI_4_HELP_PART1		"Example :\n"
	#define 	VBI_4_HELP_PART2	 	"        set cc data type as any type! Enter \"4 1\"\n"
	#define 	VBI_4_HELP_PART3	 	"        set cc data type as ATSC53! Enter \"4 2\"\n"
	#define 	VBI_4_HELP_PART4	 	"        set cc data type as SCET20! Enter \"4 3\"\n"
	
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline("Start the cc test and set cc data type\n");
	aui_print_help_instruction_newline(VBI_4_HELP_PART1);
	aui_print_help_instruction_newline(VBI_4_HELP_PART2);
	aui_print_help_instruction_newline(VBI_4_HELP_PART3);
	aui_print_help_instruction_newline(VBI_4_HELP_PART4);

	/* VBI_5_HELP */
	
	#define 	VBI_5_HELP_PART1	 	"Enter \"5\"\n"
	
	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline("stop cc play\n");
	aui_print_help_instruction_newline(VBI_5_HELP_PART1);

	/* VBI_6_HELP */
	
	#define 	VBI_6_HELP_PART1	 	"Enter \"6\"\n"
	
	aui_print_help_command("\'6\'");
	aui_print_help_instruction_newline("close VBI device\n");
	aui_print_help_instruction_newline(VBI_6_HELP_PART1);
    
	/* VBI_7_HELP */
	
	#define 	VBI_7_HELP_PART1		"Example :\n"
	#define 	VBI_7_HELP_PART2	 	"         set decv user data type as AUI_DECV_USER_DATA_DEFAULT! Enter \"7 0\"\n"
	#define 	VBI_7_HELP_PART3	 	"         set decv user data type as AUI_DECV_USER_DATA_ANY! Enter \"7 1\"\n"
	#define 	VBI_7_HELP_PART4	 	"         set decv user data type as AUI_DECV_USER_DATA_ATSC53! Enter \"7 2\"\n"
	#define 	VBI_7_HELP_PART5	 	"         set decv user data type as AUI_DECV_USER_DATA_SCTE20! Enter \"7 3\"\n"
	#define 	VBI_7_HELP_PART6	 	"         set decv user data type as AUI_DECV_USER_DATA_ALL! Enter \"7 4\"\n"
	
	aui_print_help_command("\'7\'");
	aui_print_help_instruction_newline(" set user data type and filter decv user data\n");
	aui_print_help_instruction_newline(VBI_7_HELP_PART1);
	aui_print_help_instruction_newline(VBI_7_HELP_PART2);
	aui_print_help_instruction_newline(VBI_7_HELP_PART3);
	aui_print_help_instruction_newline(VBI_7_HELP_PART4);
	aui_print_help_instruction_newline(VBI_7_HELP_PART5);
	aui_print_help_instruction_newline(VBI_7_HELP_PART6);
	
	return AUI_RTN_HELP;
}

void test_vbi_reg()
{
	aui_tu_reg_group("vbi", "vbi test");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_vbi_using_tsg, "ttx using vbi test by tsg");  //support tds and linux
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_vbi_using_pes, "ttx using vbi test by pes packet");  //Only support linux
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_vbi_cc_open, "test cc_open by vbi");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_vbi_cc_start, "test cc_start by vbi");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_vbi_cc_stop, "test cc_stop by vbi");
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_vbi_cc_close, "test cc_close by vbi");
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_decv_user_data_filter, "test decv user data filter");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_vbi_help, "vbi test help");
}




