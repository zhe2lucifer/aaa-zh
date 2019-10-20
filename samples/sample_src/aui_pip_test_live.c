#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "aui_ini_config.h"
#else
#include <api/libfs2/stdio.h>
#endif

#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_tsg.h>
#include <aui_nim.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_stc.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_nim_init.h"
#include "aui_pip_test.h"
#include "aui_pip_test_live.h"

int test_aui_pip_init_para_for_nim(unsigned long *argc,char **argv, 
				unsigned short dis_layer, unsigned short decv_id, struct ali_app_modules_init_para *init_para)
{
    struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
    enum aui_dis_format dis_format = AUI_DIS_HD;/*display format*/
    unsigned long  nim_dev = 0;
    unsigned long  nim_demod_type = 0;
    unsigned long  freq = 3840;
    unsigned long  symb = 27500;
    unsigned long  v_type = 0;
    unsigned long  a_type = 0;
    unsigned long  band = 0;
    aui_ter_std nim_std=0; /*choose ISDBT default*/
    unsigned long  polar = AUI_NIM_POLAR_HORIZONTAL;
	unsigned short pids[] = {513,660,8190};
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
        pids[0] = atoi(argv[4]);/*video pid*/
    }
    if (*argc > 5) {
        pids[1] = atoi(argv[5]);/*audio pid*/
    }
    if (*argc > 6) {
        pids[2] = atoi(argv[6]);/*pcr(personal clock reference) pid*/
    }
    if (*argc > 7) {
        v_type = atoi(argv[7]);/*video format*/
    }
    if (*argc > 8) {
        a_type = atoi(argv[8]);/*audio format*/
    }
    //use HD DIS as default
    //if (*argc > 9) {
    //    dis_format = (enum aui_dis_format)atoi(argv[9]);/*display format*/
    //}
    if (*argc > 9) {
    	// DVB-S polar
    	if (argv[9][0] == 'V' || argv[9][0] == 'v'
    			|| argv[9][0] == 'R' || argv[9][0] == 'r') {
    		polar = AUI_NIM_POLAR_VERTICAL;
    	} else {
    		polar = AUI_NIM_POLAR_HORIZONTAL;
    	}
        nim_std = (aui_ter_std)atoi(argv[9]);/*DVBT type*/
    }
    MEMSET(init_para,0,sizeof(struct ali_app_modules_init_para));

    init_para->init_para_nim.ul_device = nim_dev;
    init_para->init_para_nim.ul_freq = freq;
    init_para->init_para_nim.ul_symb_rate = symb;
    init_para->init_para_nim.ul_nim_type = nim_demod_type;
    init_para->init_para_nim.ul_freq_band = band;
    init_para->init_para_nim.ul_nim_std = nim_std;

    init_para->init_para_tsi.ul_dmx_idx = (decv_id == 0) ? AUI_TSI_OUTPUT_DMX_0:AUI_TSI_OUTPUT_DMX_1;//AUI_TSI_OUTPUT_DMX_0;

    init_para->init_para_tsi.ul_tsi_id = 0;
    init_para->init_para_tsi.ul_tis_port_idx = decv_id + 1;//TSA(B)->dmx0, TSA(B)->dmx1
    aui_tsi_config(tsi_cfg);
    init_para->init_para_tsi.ul_hw_init_val = tsi_cfg[nim_dev].ul_hw_init_val;
    init_para->init_para_tsi.ul_input_src = tsi_cfg[nim_dev].ul_input_src;
	AUI_PRINTF("ul_hw_init_val: 0x%x, ul_input_src: 0x%x\n", init_para->init_para_tsi.ul_hw_init_val, init_para->init_para_tsi.ul_input_src);
//  if (nim_dev == AUI_TSG_DEV) {
        /* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz
         * 24 -> TSG clock 4.16 MHz
         * 32 -> TSG clock 3.12 MHz
         * 48 -> TSG clock 2.08 MHz
         */
    //  init_para.init_para_tsg.ul_tsg_clk = 8;
    //  init_para.init_para_tsg.ul_bit_rate = 0; /* 0 for default bitrates */
    //}

    init_para->dmx_create_av_para.dmx_id=(decv_id == 0)?0:1;
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
    //in sample code default video 0 display on MAIN, video 1 display on PIP
    init_para->init_para_decv.video_id = decv_id;
    init_para->init_para_decv.ul_dis_layer = dis_layer;

    init_para->init_para_audio.ul_volume = 50;
    init_para->init_para_audio.ul_audio_type = a_type;
    init_para->init_para_nim.ul_polar = polar;

	init_para->init_para_dis.ul_dis_layer = dis_layer;
    return 0;
}


static int test_aui_pip_set_dmx_for_create_av_stream(int dmx_id,
             unsigned short video_pid,
             unsigned short audio_pid,
             unsigned short audio_desc_pid,
             unsigned short pcr_pid,
             unsigned char play_audio,
             aui_hdl decv_hdl,
             aui_hdl *dmx_hdl)
{
   	aui_hdl hdl = NULL;
	aui_attr_dmx attr_dmx;
	aui_dmx_stream_pid pid_list;

	MEMSET(&attr_dmx,0,sizeof(aui_attr_dmx));
	MEMSET(&pid_list,0,sizeof(aui_dmx_stream_pid));

	attr_dmx.uc_dev_idx = dmx_id;
	attr_dmx.decv_handle = decv_hdl;
	if (aui_dmx_open(&attr_dmx, &hdl)) {
			AUI_PRINTF("\r\n dmx open fail\n");
			return -1;
	}
	if(NULL == hdl) 
		return -1;
	if (aui_dmx_start(hdl, &attr_dmx)) {
        aui_dmx_stop(hdl,NULL);/*starting fail may not be close dmx*/
        if (aui_dmx_start(hdl, &attr_dmx)) {/*start again*/
		    AUI_PRINTF("\r\n aui_dmx_start fail\n");
    		aui_dmx_close(hdl);
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
	//pid_list.decv_handle = decv_hdl;
	AUI_PRINTF("\r\nhdl:video_pid,audio_pid,pcr_pid,audio_desc_pid=[%08x][%d][%d][%d][%d]",
		   (int)hdl,video_pid,audio_pid,pcr_pid,audio_desc_pid);

	if (play_audio) {
		AUI_PRINTF("dmx play audio and video\n");
	  	if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_AV, &pid_list)) {
			AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_AV fail\n");
			aui_dmx_close(hdl);
			return -1;
		}
	} else {
		AUI_PRINTF("play video only\n");
		if (aui_dmx_set(hdl, AUI_DMX_STREAM_CREATE_VIDEO, &pid_list)) {
			AUI_PRINTF("\r\n aui_dmx_set AUI_DMX_STREAM_CREATE_VIDEO fail\n");
			aui_dmx_close(hdl);
			return -1;
		}
	}
	
	*dmx_hdl= hdl;
	return 0;
}


int test_aui_pip_play_nim(struct ali_app_modules_init_para* init_para, 
	struct ali_aui_hdls *pip_hdls,
	int play_audio,
	int first_play)
{
	aui_hdl snd_hdl = NULL;
    /*nim_init_cb is callback function point,to init nim device about special board
     * for example M3733,M3515B*/
    if (aui_nim_init(nim_init_cb)) {
        AUI_PRINTF("\nnim init error\n");
        goto err_live;
    }
    AUI_PRINTF("AUI NIM opened\n");/* Start streaming */

	if (ali_app_tsi_init(&init_para->init_para_tsi, &pip_hdls->tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI TSI opened\n");
	
    /*open and init display device*/
    if(ali_app_dis_init(init_para->init_para_dis,&pip_hdls->dis_hdl)) {
        AUI_PRINTF("\r\n ali_app_dis_init failed!");
        goto err_live;
    }
	
    AUI_PRINTF("AUI DIS opened\n");
	/*init decv device */
	if (ali_app_decv_init(init_para->init_para_decv,&pip_hdls->decv_hdl)) {
		AUI_PRINTF("\r\n ali_app_decv_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI DECV opened\n");

	//set display rect after DECV opened
	test_aui_pip_set_dis_rect((enum aui_dis_layer)init_para->init_para_decv.ul_dis_layer, 
		NULL, NULL);

	/*init deca device*/
	if (play_audio){
		if (ali_app_audio_init(init_para->init_para_audio,&pip_hdls->deca_hdl,&snd_hdl)) {
			AUI_PRINTF("\r\n ali_app_audio_init failed!");
			goto err_live;
		}
		AUI_PRINTF("AUI audio opened[%08x]\n",(int)pip_hdls->deca_hdl);		
	}

	/*set vpid,apid,pcr and create av stream*/
	if (test_aui_pip_set_dmx_for_create_av_stream(init_para->dmx_create_av_para.dmx_id,
						init_para->dmx_create_av_para.video_pid,
						init_para->dmx_create_av_para.audio_pid,
					 	init_para->dmx_create_av_para.audio_desc_pid,
					 	init_para->dmx_create_av_para.pcr_pid,
					 	play_audio, pip_hdls->decv_hdl,&pip_hdls->dmx_hdl)) {
		AUI_PRINTF("\r\n set dmx failed!");
		goto err_live;
	}

	if (play_audio || first_play) {		
		/* set video and audio  synchronous ways,signal from nim,set to AUI_STC_AVSYNC_PCR.
		 * signal from tsg,set to AUI_STC_AVSYNC_AUDIO
		 */	 
		aui_hdl stc_hdl;
		test_aui_pip_set_avsync(&stc_hdl, AUI_AV_DATA_TYPE_NIM_TS,
			AUI_STC_AVSYNC_PCR, pip_hdls->decv_hdl);

	} 

	AUI_PRINTF("AUI DMX opened[%08x]\n",(int)pip_hdls->dmx_hdl);

	aui_dmx_data_path path;
	memset(&path, 0, sizeof(aui_dmx_data_path));
	path.data_path_type = AUI_DMX_DATA_PATH_CLEAR_PLAY;
	if (aui_dmx_data_path_set(pip_hdls->dmx_hdl, &path)) {
		AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
		goto err_live;
	}
	AUI_PRINTF("dmx data path set %d\n", path.data_path_type);
	
	if (play_audio) {
		AUI_PRINTF("\nplay audio and video\n");
	    if (aui_dmx_set(pip_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE, NULL)) {
	        AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
	        goto err_live;
	    }
	    AUI_PRINTF("\n dmx start to play\n");
    } else {
    	AUI_PRINTF("\nplay video only\n");
    	if (aui_dmx_set(pip_hdls->dmx_hdl, AUI_DMX_STREAM_ENABLE_VIDEO, NULL)) {
	        AUI_PRINTF("aui_dmx_set AUI_DMX_STREAM_ENABLE fail\n");
	        goto err_live;
	    }
    }
	AUI_PRINTF("nim open and connect\n");
	if (aui_find_dev_by_idx(AUI_MODULE_NIM, init_para->init_para_nim.ul_device, &pip_hdls->nim_hdl)) {
		AUI_PRINTF("nim connect, dev: %d\n", init_para->init_para_nim.ul_device);
		if (nim_connect(&init_para->init_para_nim,&pip_hdls->nim_hdl)) {
			goto err_live;
		} 
	} else {
		AUI_PRINTF("the same nim\n");
	}
	AUI_PRINTF("nim connect success: %p\n", pip_hdls->nim_hdl);
	
	return AUI_RTN_SUCCESS;

err_live:
	AUI_PRINTF("play nim error!\n");
	ali_app_deinit(pip_hdls);
	return AUI_RTN_SUCCESS;
}

int test_aui_pip_stop_nim(struct ali_aui_hdls *handles)
{
    AUI_PRINTF("\r\n close nim aui: %p\n", handles->nim_hdl);
    if (handles->nim_hdl) {
    	if (ali_app_nim_deinit(handles->nim_hdl))
        	AUI_PRINTF("\r\n ali_app_nim_deinit failed!");
        
           
	} else {
		AUI_PRINTF("\r\n nim not closed\n");
	}
	AUI_PRINTF("\r\n close tsi aui: %p\n", handles->tsi_hdl);
	if (handles->tsi_hdl) {		
	    if (ali_app_tsi_deinit(handles->tsi_hdl))
	        AUI_PRINTF("\r\n ali_app_tsi_deinit failed!"); 
	} else {
		AUI_PRINTF("\r\n tsi not closed\n");
	}
    AUI_PRINTF("\r\n close tsg aui\n");
    if (handles->tsg_hdl && ali_app_tsg_deinit(handles->tsg_hdl))
        AUI_PRINTF("\r\n ali_app_tsg_deinit failed!");

    AUI_PRINTF("\r\n close dmx aui\n");
    if (handles->dmx_hdl && ali_app_dmx_deinit(handles->dmx_hdl))
        AUI_PRINTF("\r\n ali_app_dmx_deinit failed!");

    AUI_PRINTF("\r\n close snd aui\n");
    if (handles->deca_hdl) {
        if (ali_app_snd_deinit(handles->deca_hdl,NULL))
        	AUI_PRINTF("\r\n ali_app_snd_deinit failed!");
    } else {
    	AUI_PRINTF("\r\n deca not stopped\n");
    }
    handles->deca_hdl = NULL;
    AUI_PRINTF("\r\n close decv aui\n");
    if (handles->decv_hdl && ali_app_decv_deinit(handles->decv_hdl))
        AUI_PRINTF("\r\n ali_app_decv_deinit failed!");
    handles->decv_hdl = NULL;    
    return 0;
}

int test_aui_pip_start_dmx_audio(aui_hdl *deca_hdl, int audio_type, aui_hdl dmx_hdl, 
	unsigned short audio_pid, unsigned short audio_desc_pid)
{
	aui_attr_deca attr_deca;
	aui_dmx_stream_pid pid_list;
	
	MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
	if (aui_find_dev_by_idx(AUI_MODULE_DECA, 0, deca_hdl))
			aui_deca_open(&attr_deca, deca_hdl);	
	AUI_PRINTF("deca start\n");
    aui_deca_type_set(*deca_hdl, audio_type);			
    aui_deca_start(*deca_hdl,&attr_deca);
	memset(&pid_list, 0, sizeof(aui_dmx_stream_pid));
	pid_list.ul_pid_cnt=4;
	pid_list.stream_types[0]=AUI_DMX_STREAM_LAST;
	pid_list.stream_types[1]=AUI_DMX_STREAM_AUDIO;
	pid_list.stream_types[2]=AUI_DMX_STREAM_LAST;
	pid_list.stream_types[3]=AUI_DMX_STREAM_AUDIO_DESC;

	pid_list.aus_pids_val[0]=0;
	pid_list.aus_pids_val[1]=audio_pid;
	pid_list.aus_pids_val[2]=0;
	pid_list.aus_pids_val[3]=audio_desc_pid;
	AUI_PRINTF("change audio stream\n");
	aui_dmx_set(dmx_hdl, AUI_DMX_SET_CHANGE_AUD_STREM, &pid_list);

	return 0;
}


int test_aui_pip_stop_dmx_audio(aui_hdl *deca_hdl, aui_hdl dmx_hdl)
{
	if (deca_hdl && *deca_hdl && dmx_hdl) {
		aui_deca_stop(*deca_hdl, NULL);
		aui_deca_close(*deca_hdl);
		*deca_hdl = NULL;
		aui_dmx_set(dmx_hdl, AUI_DMX_STREAM_DISABLE_AUDIO, 0);
	}
	
	return 0;
}


