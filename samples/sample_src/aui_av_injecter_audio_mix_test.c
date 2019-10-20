/**@file
*    @brief 		The test file of AUI AV INJECTER module
*    @author		Christian.xie
*    @date			2014-3-18
*	 @version 		1.0.0
*    @note			Copyright(C) ALi Corporation. All rights reserved.  
	    			AV INJECTER is used to directly inject auido/video ES/PES data to A/V decoder.\n
	    			The injecter support PES and ES mode,which can used for up-layer media player.
	    		
*/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <signal.h>

#include <aui_av_injecter.h>
#include <aui_snd.h>
#include <aui_deca.h>

#include "aui_av_injecter_audio_mix_test.h"
#include "aui_av_injecter_test.h"
#include "aui_test_app_cmd.h"

static int snd_end = 0;
static int snd_pause = 0;
#define DECODE_SAMPLE_MAX   990//fixed

static void show_mix_usage()
{
	AUI_PRINTF("mix a pcm to the main audio\n");
	AUI_PRINTF("mix audio requirement: pcm with 32 bits per sample, 2 channel and sample rate 8192~96000\n");
	AUI_PRINTF("usage: mix [filepath],[samplerate]\n");
}

static void snd_mix_end_cb(void *pv_param) 
{
    enum aui_snd_cbtype cb_type=(enum aui_snd_cbtype)pv_param; 
     
    printf("\033[22;31m%s -> type: 0x%x.\033[0m\n",__FUNCTION__, cb_type); 
    if(AUI_SND_CB_MONITOR_MIX_DATA_END == cb_type) 
    { 
		snd_end = 1;
		//the end callback will be called when mix ending or main ending
        printf ("%s: AUI_SND_CB_MONITOR_MIX_DATA_END!\n", __FUNCTION__);
    } 
}

static void snd_mix_pause_cb(void *pv_param) 
{
    enum aui_snd_cbtype cb_type=(enum aui_snd_cbtype)pv_param; 
     
    printf("\033[22;31m%s -> type: 0x%x.\033[0m\n",__FUNCTION__, cb_type); 

    if (AUI_SND_CB_MONITOR_MIX_PAUSED == cb_type) 
    {
    	snd_pause = 1;
    	printf ("%s: AUI_SND_CB_MONITOR_MIX_PAUSED!\n", __FUNCTION__);
    }
}

static int reg_snd_end_cb(aui_hdl aui_snd_hd, enum aui_snd_cbtype cb_type) 
{ 
	int ret;
	struct aui_st_snd_io_reg_callback_para cb_para;
	memset(&cb_para, 0, sizeof(cb_para));
    cb_para.e_cbtype = cb_type;    
    cb_para.pv_param =(void*)cb_type; 
    
    if (cb_type == AUI_SND_CB_MONITOR_MIX_DATA_END) {
		snd_end = 0;
		cb_para.p_cb = snd_mix_end_cb; 
	}
	
	if (cb_type == AUI_SND_CB_MONITOR_MIX_PAUSED) {
		snd_pause = 0;
		cb_para.p_cb = snd_mix_pause_cb; 
	}
	
	AUI_PRINTF("reg_snd_end_cb: %d\n", cb_type);
	ret = aui_snd_set(aui_snd_hd,AUI_SND_REG_CB,&cb_para); 
	
	if (ret) {
		printf ("reg_snd_end_cb failed\n");
	}	
	return ret;
}

static int test_aui_av_inject_mix_data_eos(aui_hdl mix_hnd)
{
	int ret = 0;
	struct aui_av_packet header;
	memset(&header, 0, sizeof(header));
	header.flag = AUI_AV_PACKET_EOS;
	ret = aui_audio_decoder_write_header(mix_hnd, &header);
	return ret;
}

static char kbhit(void)
{

	struct timeval tv;
	fd_set read_fd;

	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);

	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
		return 0;

	if (FD_ISSET(0,&read_fd)){
	    char c = getchar();
	    if (c == 0xa)
	    {
	        return 0;
	    }
		return c;
	}
	return 0;
}

static int test_aui_av_inject_mix_data_write(
	aui_hdl mix_hnd, const char *src_file_name,  
	unsigned int ch_num, unsigned short bits_per_sample)
{
    FILE        *src_fp = NULL;
    unsigned int   src_file_size = 0;
    unsigned int    rd_size = 0;
    int         ret = 0;
	unsigned char    *audio_frm_buff = NULL;
	unsigned int    frm_buff_size = 0;
	struct aui_av_packet header;
	unsigned char key_press = -1;
	int retry = 0;

    src_fp = fopen(src_file_name, "rb");
    if(src_fp == NULL) {
		printf ("Open file %s fail\n", src_file_name);
        ret = -1;
        goto exit;
	}
    fseek(src_fp, 0, SEEK_END);
	src_file_size = ftell(src_fp);
	fseek(src_fp, 0, SEEK_SET);
	if(src_file_size <= 0) {
		printf ("File size error %u\n", src_file_size);
        ret = -1;
        goto exit;
	}
	fflush(src_fp);
    printf ("SRC File size %u\n", src_file_size);

    frm_buff_size = 3*DECODE_SAMPLE_MAX*ch_num*(bits_per_sample>>3);//fixed size

    audio_frm_buff = (unsigned char *)malloc(frm_buff_size);
    if(audio_frm_buff == NULL) {
        printf ("Malloc(%u) frame buffer fail\n", frm_buff_size);
        ret = -1;
        goto exit;
    }
	memset(&header, 0, sizeof(header));
	printf("If mix is paused as main audio paused you can press 'q' to force stop\n");
	printf("Press 'p' to pause mix audio and 'r' to remuse mix audio\n");
    while(src_file_size > 0) {
        memset(audio_frm_buff, 0, frm_buff_size);
        rd_size = frm_buff_size;
        if (src_file_size < rd_size)
            rd_size = src_file_size;
        if (fread(audio_frm_buff, 1, rd_size, src_fp) != rd_size) {
            printf ("fread error \n");
            ret = -1;
            goto exit;
        }
        src_file_size -= rd_size;

        printf ("%s->%s.%u, send size(%u) to sbm. read size %u\n",
            __FILE__, __FUNCTION__, __LINE__, frm_buff_size, rd_size);
        header.pts = AUI_AV_NOPTS_VALUE;
        header.dts = AUI_AV_NOPTS_VALUE;
        header.size = rd_size;//must fix ----> support dynamic lenth right now 
    	do {
    		key_press = kbhit();
        	if (key_press == 'q') {
        		ret = -1;
        		goto exit;
        	} else if (key_press == 'p') {
        	    aui_audio_decoder_pause(mix_hnd, 1);
        	    snd_pause = 1;
        	} else if (key_press == 'r') {
        	    aui_audio_decoder_pause(mix_hnd, 0);
        	    snd_pause = 0;
        	}
    		ret = test_av_inject_write_audio_data((void*)mix_hnd, &header, 
        		audio_frm_buff, rd_size);
        	//printf("ret: %d, retry: %d\n", ret, retry);	
        	if (ret < 0) {//may buffer full, need wait
        		usleep(20*1000);
        		if (snd_pause) {
        			usleep(200*1000);       			
        		} else {
        			retry++;
        		}
        	} else if (ret == 0) {
        		retry = 0;
        		break;
        	}
    	} while(!snd_end);
        if (0 >= src_file_size) {
            printf("\033[40;31m%s->%s.%u, tell see that sending ended.\033[0m\n", __FILE__, __FUNCTION__, __LINE__);
            ret = test_aui_av_inject_mix_data_eos(mix_hnd);
            if (ret)
            {
                printf ("call aui_snd_set_mix_info error\n\n");
                ret = -1;
                goto exit;
            }
        }
    }
	printf("write data end!\n");
    ret = 0;
exit:
    if (audio_frm_buff)
    {
        free(audio_frm_buff);
        audio_frm_buff = NULL;
    }
    if (src_fp)
    	fclose(src_fp);

    return ret;
}


unsigned long test_aui_av_inject_audio_mix(char* filename, int sample_rate_in)
{
	unsigned long ret = AUI_RTN_FAIL;
	aui_audio_info_init mix_audio_info;
	aui_hdl mix_hnd_out = 0;
	aui_hdl deca_hdl = NULL, snd_hdl = NULL;
	aui_audio_decoder_status status;
	int retry = 10;
	unsigned last_valid_size = 0;

//==========================================================================
    if (sample_rate_in<8000 || sample_rate_in>=96000)
    {
        printf ("argument invalid\n");
		return -1;
    }

	//open snd
	aui_attr_snd attr_snd;
	if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl))
	{
		memset(&attr_snd,0,sizeof(aui_attr_snd));
		aui_snd_open(&attr_snd, &snd_hdl);
	}
	//register callback
	ret = reg_snd_end_cb(snd_hdl, AUI_SND_CB_MONITOR_MIX_DATA_END);
	ret = reg_snd_end_cb(snd_hdl, AUI_SND_CB_MONITOR_MIX_PAUSED);
	
    if(aui_find_dev_by_idx(AUI_MODULE_DECA, 1, &deca_hdl))
	{
		aui_attr_deca attr_deca;
		memset(&attr_deca,0,sizeof(attr_deca));
		attr_deca.uc_dev_idx = 1;
		aui_deca_open(&attr_deca, &deca_hdl);
	}

    memset(&mix_audio_info, 0, sizeof(mix_audio_info));
    mix_audio_info.channels = 2;//fixed
    mix_audio_info.sample_rate = sample_rate_in;//8096~96000
    mix_audio_info.nb_bits_per_sample = 32;//fixed
    mix_audio_info.deca_handle = deca_hdl;
	ret = aui_audio_decoder_open(&mix_audio_info, &mix_hnd_out);
    if( AUI_RTN_SUCCESS != ret)
    {
        printf("open sbm for audio mix fail!\n");
        return ret;
    }
//==========================================================================
	printf ("send data here.\n");
	ret = test_aui_av_inject_mix_data_write(mix_hnd_out, filename, 
		mix_audio_info.channels, mix_audio_info.nb_bits_per_sample);
    if(ret)
    {
        printf("file_trans fail!\n");
        goto quit;
    }
    printf("wait mix end...\n");
//==========================================================================
	do {
	    usleep(50*1000);//sleep 50ms
#if 0 //use mix end callback to confirm the audio mix is end instead
		memset(&status, 0, sizeof(status));
		aui_audio_decoder_get_status(mix_hnd_out, &status);
		printf("retry: %d, audio_valid_size: %ld\n", retry, status.buffer_used);	
		if (last_valid_size == status.buffer_used) {
			/*
			 * please use mix end callback to make sure mix is end
			 * if not use mix end, when buffer size turn to 0, wait 500ms
			 */
			retry--;
		} else {
			retry = 10;
		}
		last_valid_size = status.buffer_used;
#endif	
	} while(retry >=0 && snd_end == 0);/*status.buffer_used >= 0 && */

quit:	
//==========================================================================	
	ret = aui_audio_decoder_close(mix_hnd_out);
    if( AUI_RTN_SUCCESS != ret)
    {
        printf("close audio decoder fail!\n");
        return ret;
    }
	aui_deca_close(deca_hdl);
	printf("mix finished!\n");
	return ret;
}

unsigned long test_aui_av_inject_audio_mix_item(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = AUI_RTN_FAIL;
	aui_audio_info_init mix_audio_info;
	char* filename;
	aui_hdl mix_hnd_out = 0;
	int sample_rate_in = 0;
	char filepath[256];
	aui_hdl snd_hdl;

//==========================================================================
	if (1 >= *argc) {
		show_mix_usage();
		return -1;
	}
	
    if(aui_find_dev_by_idx(AUI_MODULE_SND, 0, &snd_hdl)) {
       aui_attr_snd attr_snd;
           memset(&attr_snd, 0, sizeof(attr_snd));
           aui_snd_open(&attr_snd, &snd_hdl);
    }
    //set audio volume
    aui_snd_mute_set(snd_hdl, 0);
    aui_snd_vol_set(snd_hdl, 100);
    
	sample_rate_in = atoi(argv[1]);
    if (sample_rate_in<8000 || sample_rate_in>=96000)
    {
        printf ("argument invalid\n");
		return -1;
    }
    memset(filepath, 0, 256);
    strcpy(filepath, argv[0]);

	ret = test_aui_av_inject_audio_mix(filepath, sample_rate_in);
	return ret;
}
