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

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_tsg.h>
#include <aui_nim.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_av_injecter.h>
#include <aui_stc.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_pip_test.h"
#include "aui_pip_test_live.h"
#include "aui_pip_test_media.h"

enum pip_video {
	PIP_0=0,
	PIP_1,
	PIP_MAX
};

enum pip_source_type {
	PIP_FROM_NIM,
	PIP_FROM_MEDIA
};

enum pip_audio_source {
	PIP_AUDIO_NONE = 0,
	PIP_AUDIO_0 = 1,
	PIP_AUDIO_1 = 1<<2
};

enum pip_status {
	PIP_STATUS_NONE,
	PIP_STATUS_INITIALIZED,
	PIP_STATUS_PLAYED,
	PIP_STATUS_STOPED
};

struct ali_app_pip_init_para {
	
	unsigned char play_audio;
	
	
};

struct ali_app_pip_live_ts {
	struct ali_app_modules_init_para init_para_nim_play;
	struct ali_aui_hdls pip_hdls;
};

struct ali_app_pip_es {
	struct ali_app_pip_media_init_para init_para_media;
	struct pip_test_media  media;
};

struct ali_app_pip {
	enum pip_source_type source_type;
	unsigned char play_audio;
	struct ali_app_pip_live_ts *live_info;
	struct ali_app_pip_es *media_info;
	enum pip_status status;//current status
};

struct ali_app_pip_dis_rect {
	aui_dis_zoom_rect src_rect;
	aui_dis_zoom_rect dst_rect; 
};

/*In sample code: 
  video 0: shown MP 
  video 1: shown in PIP
  Not mandatory */
static struct ali_app_pip s_pip[PIP_MAX];
static struct ali_app_pip_dis_rect s_dis_rect[PIP_MAX];
static aui_hdl s_dis_hdl;//only one dis device
static aui_hdl s_snd_hdl;//only one snd device
static aui_hdl s_stc_hdl;//only one stc device
static unsigned char s_play_audio = PIP_AUDIO_NONE;

static unsigned long test_aui_pip_stop_mp();
static unsigned long test_aui_pip_stop_pip();
static unsigned long test_aui_pip_audio_swap(unsigned long *argc,char **argv,char *sz_out_put);
	
static void show_pip_usage()
{
	AUI_PRINTF("arguments for DVBS and DVBC:\n");
 	AUI_PRINTF("usage:[Test Items] [nim_id],[nim_type],[freq],[symb],[vpid],[apid],[ppid],[vformat],[aformat],[polar],[play audio]\n");	    
    AUI_PRINTF("       such as input: m 1,0,3840,27500,513,660,8190,0,1,H,1\n");
    AUI_PRINTF("                      p 1,0,3840,27500,514,670,8190,0,1,H,0\n");
    AUI_PRINTF("arguments for DVBT:\n");
 	AUI_PRINTF("usage:[Test Items] [nim_id],[nim_type],[freq],[symb],[vpid],[apid],[ppid],[vformat],[aformat],[DVBT type],[play audio]\n");
	AUI_PRINTF(" ISDBT such as input: m 0,2,682000,8,2111,2121,2111,0,0,0,1\n");
 	AUI_PRINTF("  DVBT such as input: m 0,2,682000,8,2111,2121,2111,0,0,1,1\n");
 	AUI_PRINTF(" DVBT2 such as input: m 0,2,682000,8,2111,2121,2111,0,0,2,1\n");
 	AUI_PRINTF("  DVBC such as input: m 0,1,58600,6875,851,852,851,1,3,0,1\n");
 	
	
    AUI_PRINTF("\narguments for av inject:\n");
	AUI_PRINTF("usage:[Test Items] [File path],[vformat],[aformat],[play audio],[Video frame rate]\n");
	AUI_PRINTF("[play audio]: 0: disable audio, 1: enable audio\n");
	AUI_PRINTF("\nvformat: 0: mpeg2, 1: h264\n");
	AUI_PRINTF("aformat: 0: mpeg1, 1: mpeg2, 2: AAC(LATM, media), 3: AC3, 4: DTS 13: AAC(ADTS), 16: MP3\n");
	AUI_PRINTF("      video frame rate: Video frame rate,[frame rate equal 23976, while 23.976 fps]\n");
	AUI_PRINTF("       such as input: m /root/testfiles/test.mp4,1,2,1,25000\n");
	AUI_PRINTF("                      p /root/testfiles/Threshold.ts,0,1,0,25000\n");
	AUI_PRINTF("for av inject, it needs files like xxx.video0, xxx.audio0 and etc.");
}

static unsigned long test_aui_pip_init(unsigned long *argc,char **argv, unsigned char dis_layer,
	unsigned char decv_id, struct ali_app_pip *pip)
{
	if (0 == *argc) {
		show_pip_usage();
		return -1;
	}
	if (pip->status != PIP_STATUS_PLAYED) {
		if (strlen(argv[0]) == 1){
			AUI_PRINTF("--- play nim ---\n");
			pip->source_type = PIP_FROM_NIM;
			pip->live_info = malloc(sizeof(struct ali_app_pip_live_ts));
			if (pip->live_info == NULL) {
				return -1;
			}
			MEMSET(pip->live_info, 0, sizeof(struct ali_app_pip_live_ts));
			test_aui_pip_init_para_for_nim(argc, argv, dis_layer, decv_id, &pip->live_info->init_para_nim_play);
			if (*argc > 10)
				pip->play_audio = atoi(argv[10]);
		} else {
			AUI_PRINTF("--- play media ---\n");
			pip->source_type = PIP_FROM_MEDIA;
			pip->media_info = malloc(sizeof(struct ali_app_pip_es));
			if (pip->media_info == NULL) {
				return -1;
			}
			MEMSET(pip->media_info, 0, sizeof(struct ali_app_pip_es));
			test_aui_pip_init_para_for_media(argc, argv, dis_layer, decv_id, &pip->media_info->init_para_media);
			if (*argc > 3)
				pip->play_audio = atoi(argv[3]);
		}
		pip->status = PIP_STATUS_INITIALIZED;
	}
	return 0;
}

int test_aui_pip_set_avsync(aui_hdl *p_hdl_stc, 
	enum aui_av_data_type data_type, aui_stc_avsync_mode sync_mode, 
	aui_hdl decv_hdl)
{
    aui_hdl stc_hdl = 0;
    aui_stc_avsync_attr avsync_attr;
    if(aui_find_dev_by_idx(AUI_MODULE_STC, 0, &stc_hdl)) {
        if (aui_stc_open(&stc_hdl)) {
            AUI_PRINTF("\n aui_stc_open fail\n");
            return -1;
        }
    }
    
	AUI_PRINTF("aui stc avsync set data type, data_type: %d\n", data_type);
	memset(&avsync_attr, 0, sizeof(aui_stc_avsync_attr));
	avsync_attr.data_type = data_type;
	avsync_attr.sync_mode = sync_mode;
	avsync_attr.decv_handle = decv_hdl;
    if (aui_stc_avsync_set(stc_hdl, &avsync_attr)) {
        AUI_PRINTF("\n aui_stc_avsync_set fail\n");
        return -1;
    }
    AUI_PRINTF("aui stc avsync set data type, data_type: %d, decv_hdl: %p\n", data_type, decv_hdl);
    if (p_hdl_stc) {
    	*p_hdl_stc = stc_hdl;
    }
    return 0;

}

static unsigned long test_aui_pip_stop(struct ali_app_pip *pip)
{
	unsigned long ret = 0;
	
	if (pip->source_type == PIP_FROM_NIM) {
		ret = test_aui_pip_stop_nim(&pip->live_info->pip_hdls);		
		memset(&pip->live_info->pip_hdls, 0, sizeof(struct ali_aui_hdls));
	} else if (pip->source_type == PIP_FROM_MEDIA) {
		ret = test_aui_pip_stop_media(&pip->media_info->media);
	}
	pip->status = PIP_STATUS_STOPED;
	return ret;
}

static unsigned long test_aui_pip_stop_audio(struct ali_app_pip *stop_pip) 
{
	/* 
	 * need to stop deca and disable audio stream in dmx((optional for swapping between ts and es))
	 */
	if (stop_pip->status == PIP_STATUS_PLAYED) {
		AUI_PRINTF("audio stop source type: %d\n", stop_pip->source_type);
		if (stop_pip->source_type == PIP_FROM_NIM) {
			test_aui_pip_stop_dmx_audio(&stop_pip->live_info->pip_hdls.deca_hdl, 
				stop_pip->live_info->pip_hdls.dmx_hdl);
			stop_pip->live_info->pip_hdls.deca_hdl = 0;	
		} else if (stop_pip->source_type == PIP_FROM_MEDIA) {		    
		    test_aui_pip_stop_media_audio(&stop_pip->media_info->media);		    
		}
	}
	return 0;
}

int test_aui_pip_swap_video(aui_hdl decv_main, aui_hdl decv_pip)
{
	int ret = -1;
	aui_dis_video_attr main_video, aux_video;
	memset(&main_video, 0, sizeof(aui_dis_video_attr));
	main_video.video_source = decv_main;
	main_video.p_src_rect = &s_dis_rect[AUI_DIS_LAYER_VIDEO].src_rect;
	main_video.p_dst_rect = &s_dis_rect[AUI_DIS_LAYER_VIDEO].dst_rect;
	memset(&aux_video, 0, sizeof(aui_dis_video_attr));
	aux_video.video_source = decv_pip;
	aux_video.p_src_rect = &s_dis_rect[AUI_DIS_LAYER_AUXP].src_rect;
	aux_video.p_dst_rect = &s_dis_rect[AUI_DIS_LAYER_AUXP].dst_rect;
	ret = aui_dis_video_attr_set(s_dis_hdl, &main_video, &aux_video);
	return ret;
}

static unsigned long test_aui_pip_swap_video_test(struct ali_app_pip *stop_pip, struct ali_app_pip *start_pip)
{
	unsigned long ret = -1;
	
	aui_hdl decv_main = NULL;
	aui_hdl decv_pip = NULL;

	if (stop_pip->status == PIP_STATUS_PLAYED) {
		if (stop_pip->source_type == PIP_FROM_NIM) {
			decv_pip = stop_pip->live_info->pip_hdls.decv_hdl;
		}
		else if (stop_pip->source_type == PIP_FROM_MEDIA) {
			decv_pip = stop_pip->media_info->media.decv_hdl;
		}
	}

	if (start_pip->status == PIP_STATUS_PLAYED) {	
		if (start_pip->source_type == PIP_FROM_NIM) {
			decv_main = start_pip->live_info->pip_hdls.decv_hdl;
		}
		else if (start_pip->source_type == PIP_FROM_MEDIA) {
			decv_main = start_pip->media_info->media.decv_hdl;
		}
	}
	ret = test_aui_pip_swap_video(decv_main, decv_pip);

	return ret;
}

static unsigned long test_aui_pip_swap_audio_test(struct ali_app_pip *stop_pip, struct ali_app_pip *start_pip)
{	
	/* swap between MP and PIP 
	 * 1. stop deca
	 * 2. disable current audio stream(optional for swapping between ts and es)
	 * 3. set avsync
	 * 4. start deca
	 * 5. the other audio: change audio track(optional for swapping between ts and es)
	 */
	test_aui_pip_stop_audio(stop_pip);

	if (start_pip->status == PIP_STATUS_PLAYED) {
		if (start_pip->source_type == PIP_FROM_NIM) {
			aui_attr_deca attr_deca;
			MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
			
			//set sync mode
			test_aui_pip_set_avsync(&s_stc_hdl, AUI_AV_DATA_TYPE_NIM_TS, 
				AUI_STC_AVSYNC_PCR, start_pip->live_info->pip_hdls.decv_hdl);
				
			AUI_PRINTF("deca start and set audio pid to dmx\n");
			test_aui_pip_start_dmx_audio(&start_pip->live_info->pip_hdls.deca_hdl,
				start_pip->live_info->init_para_nim_play.init_para_audio.ul_audio_type,
				start_pip->live_info->pip_hdls.dmx_hdl,
				start_pip->live_info->init_para_nim_play.dmx_create_av_para.audio_pid,
				start_pip->live_info->init_para_nim_play.dmx_create_av_para.audio_desc_pid);		
		} else if (start_pip->source_type == PIP_FROM_MEDIA) {
			//set sync mode and restart in the thread
		    start_pip->media_info->media.play_audio = 1;    
		}
	}

	return 0;		
}

int test_aui_pip_set_dis_rect(enum aui_dis_layer layer, 
	aui_dis_zoom_rect* p_src_rect, aui_dis_zoom_rect* p_dst_rect)
{
	int ret = -1;
	aui_attr_dis attr_dis;
 	memset(&attr_dis, 0, sizeof(aui_attr_dis));
	attr_dis.uc_dev_idx = AUI_DIS_HD;
	if(aui_find_dev_by_idx(AUI_MODULE_DIS, 0, &s_dis_hdl)) {/*if dis device has opened,return dis device handle*/
		if (aui_dis_open(&attr_dis, &s_dis_hdl)) {/*if dis hasn't opened,open dis device and return dis device handle*/
			AUI_PRINTF("\n aui_dis_open fail\n");
			return -1;
		}
    }
	if (p_src_rect == NULL || p_dst_rect == NULL) {
		AUI_PRINTF("use last display rect for layer %d\n", layer);
	} else {
		memcpy(&s_dis_rect[layer].src_rect, p_src_rect, sizeof(aui_dis_zoom_rect));
		memcpy(&s_dis_rect[layer].dst_rect, p_dst_rect, sizeof(aui_dis_zoom_rect));
	}
	if (s_dis_rect[layer].src_rect.ui_startX == 0 && s_dis_rect[layer].src_rect.ui_startY == 0 
		&& s_dis_rect[layer].src_rect.ui_width == 0 && s_dis_rect[layer].src_rect.ui_height == 0) {
		AUI_PRINTF("use default display rect for layer %d\n", layer);
		if (layer == AUI_DIS_LAYER_VIDEO) {
			//Main default fullview
			s_dis_rect[layer].src_rect.ui_startX = 0;
			s_dis_rect[layer].src_rect.ui_startY = 0;
			s_dis_rect[layer].src_rect.ui_width = 720;
			s_dis_rect[layer].src_rect.ui_height = 2880;
			s_dis_rect[layer].dst_rect.ui_startX = 0;
			s_dis_rect[layer].dst_rect.ui_startY = 0;
			s_dis_rect[layer].dst_rect.ui_width = 720;
			s_dis_rect[layer].dst_rect.ui_height = 2880;
		} else if (layer == AUI_DIS_LAYER_AUXP) {
			//PIP default preview
			s_dis_rect[layer].src_rect.ui_startX = 0;
			s_dis_rect[layer].src_rect.ui_startY = 0;
			s_dis_rect[layer].src_rect.ui_width = 720;
			s_dis_rect[layer].src_rect.ui_height = 2880;
			s_dis_rect[layer].dst_rect.ui_startX = 720/2;
			s_dis_rect[layer].dst_rect.ui_startY = 0;
			s_dis_rect[layer].dst_rect.ui_width = 720/2;
			s_dis_rect[layer].dst_rect.ui_height = 2880/2;

		}
	}
	
	ret = aui_dis_video_display_rect_set(s_dis_hdl, layer, &s_dis_rect[layer].src_rect, &s_dis_rect[layer].dst_rect);
	return ret;
}

static int test_aui_pip_play(struct ali_app_pip *pip) 
{
	int ret = -1;
	unsigned char first_play = 0;
	first_play = (s_play_audio == PIP_AUDIO_NONE?1:0);
	if (pip->source_type == PIP_FROM_NIM) {
		printf("\nplay from nim(audio: %d)...\n", pip->play_audio);
		pip->status = PIP_STATUS_PLAYED;		
		ret = test_aui_pip_play_nim(&pip->live_info->init_para_nim_play,
			&pip->live_info->pip_hdls, pip->play_audio, first_play);
	} else if (pip->source_type == PIP_FROM_MEDIA) {
		AUI_PRINTF("media status: %d\n", pip->status);
		printf("\nplay media(audio: %d)...\n", pip->play_audio);
		pip->status = PIP_STATUS_PLAYED;	
		pip->media_info->media.play_audio = pip->play_audio;
		pip->media_info->media.first_play = first_play;
		
		ret = test_aui_pip_play_media(&pip->media_info->init_para_media,
			&pip->media_info->media);
	}
	return ret;
}

/* 
 * default: display layer 0, decv id 0
 */
static unsigned long test_aui_pip_play_mp(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = -1;
	unsigned char decv_id = 0;
	
	struct ali_app_pip *pip = &s_pip[PIP_0];

	if (0 == *argc) {
		show_pip_usage();
		return -1;
	}
	
	if (pip->status == PIP_STATUS_PLAYED) {
		ret = test_aui_pip_stop_mp();
	}
	ret = test_aui_pip_init(argc, argv, 0, 0, pip);
	if (ret) {
		printf("init failed\n");
		return ret;
	}
	AUI_PRINTF("current audio source %d\n", s_play_audio);
		
	if (pip->play_audio) {
		if (s_play_audio == PIP_AUDIO_1) {
			// only support one audio, play audio in Main Picture as default
			test_aui_pip_stop_audio(&s_pip[PIP_1]);
		}
		s_play_audio = PIP_AUDIO_0;
	}
	
	test_aui_pip_play(pip);
	
	return ret;
}

/* 
 * default: display layer 1, decv id 1
 */
static unsigned long test_aui_pip_play_pip(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = -1;
	struct ali_app_pip *pip = &s_pip[PIP_1];
	
	if (0 == *argc) {
		show_pip_usage();
		return -1;
	}
	
	if (pip->status == PIP_STATUS_PLAYED) {
		ret = test_aui_pip_stop_pip();
	}
	ret = test_aui_pip_init(argc, argv, 1, 1, pip);
	if (ret) {
		printf("init failed\n");
		return ret;
	}
		
	AUI_PRINTF("current audio source %d\n", s_play_audio);
	if (pip->play_audio) {
		if (s_play_audio == PIP_AUDIO_0) {
			pip->play_audio = 0;
		} else {
			s_play_audio = PIP_AUDIO_1;
		}	
	}
	
	test_aui_pip_play(pip);
	return ret;
}

static unsigned long test_aui_pip_swap(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = -1;
	struct ali_app_pip *stop_pip = NULL, *start_pip = NULL;

	if (s_play_audio == PIP_AUDIO_0 && s_pip[PIP_1].status == PIP_STATUS_PLAYED) {
		AUI_PRINTF("play PIP audio\n");
		s_play_audio = PIP_AUDIO_1;
	} else if (s_play_audio == PIP_AUDIO_1 && s_pip[PIP_0].status == PIP_STATUS_PLAYED) {
		AUI_PRINTF("play MP audio\n");
		s_play_audio = PIP_AUDIO_0;
	} else {
		AUI_PRINTF("the target audio can not play!\n");
		if (s_play_audio == PIP_AUDIO_NONE) {
			if (s_pip[PIP_0].status == PIP_STATUS_PLAYED) {
				AUI_PRINTF("no audio playing: play MP audio\n");
				s_play_audio = PIP_AUDIO_0;
			} else if (s_pip[PIP_1].status == PIP_STATUS_PLAYED) {
				AUI_PRINTF("no audio playing: play PIP audio\n");
				s_play_audio = PIP_AUDIO_1;
			} 
		} else {
			AUI_PRINTF("no need swap audio!\n");
			//return ret;
		}
	}
	if (s_play_audio == PIP_AUDIO_1) {
		stop_pip = &s_pip[PIP_0];
		start_pip = &s_pip[PIP_1];
	} else if (s_play_audio == PIP_AUDIO_0) {
		stop_pip = &s_pip[PIP_1];
		start_pip = &s_pip[PIP_0];
	}
	ret = test_aui_pip_swap_audio_test(stop_pip, start_pip);
	ret = test_aui_pip_swap_video_test(stop_pip, start_pip);
	return ret;
}

static void test_aui_pip_stop_common(struct ali_app_pip *pip, struct ali_app_pip *pip_other)
{
	if (pip->status == PIP_STATUS_PLAYED) {
		if (pip_other->source_type == PIP_FROM_NIM &&
			pip->source_type == PIP_FROM_NIM &&
			pip_other->status == PIP_STATUS_PLAYED) {
			if (s_play_audio == PIP_AUDIO_1) {
				//deca is using by PIP, don't stop here
				pip->live_info->pip_hdls.deca_hdl = 0;
			}
			if (pip->live_info->init_para_nim_play.init_para_nim.ul_device
			 == pip_other->live_info->init_para_nim_play.init_para_nim.ul_device &&
			 pip_other->status == PIP_STATUS_PLAYED){
				//nim is using by PIP, don't stop here
				pip->live_info->pip_hdls.nim_hdl = 0;
			}
			pip->live_info->pip_hdls.tsi_hdl = 0;//only one tsi dev
		}
		test_aui_pip_stop(pip);
	}

	if(pip->source_type == PIP_FROM_NIM) {
		if (pip->live_info) {
			free(pip->live_info);
			pip->live_info = NULL;
		}
	} else if (pip->source_type == PIP_FROM_MEDIA) {
		if (pip->media_info) {
			free(pip->media_info);
			pip->media_info = NULL;
		}
	}
}

static unsigned long test_aui_pip_stop_mp()
{
	unsigned long ret = 0;
	struct ali_app_pip *pip = &s_pip[PIP_0];
	struct ali_app_pip *pip_other = &s_pip[PIP_1];
	s_play_audio &= ~PIP_AUDIO_0;
	test_aui_pip_stop_common(pip, pip_other);
	return ret;
}

static unsigned long test_aui_pip_stop_pip()
{
	unsigned long ret = 0;
	struct ali_app_pip *pip = &s_pip[PIP_1];
	struct ali_app_pip *pip_other = &s_pip[PIP_0];
	s_play_audio &= ~PIP_AUDIO_1;
	test_aui_pip_stop_common(pip, pip_other);
	return ret;
}

static unsigned int test_aui_pip_get_layer(struct ali_app_pip *pip)
{
	unsigned int ret = -1;
	aui_decv_info pst_info;
	aui_hdl decv_hdl = NULL;
	memset(&pst_info, 0, sizeof(aui_decv_info));
	if (pip->status == PIP_STATUS_PLAYED) {
		if (pip->source_type == PIP_FROM_NIM) {
			decv_hdl = pip->live_info->pip_hdls.decv_hdl;
		} else if (pip->source_type == PIP_FROM_MEDIA) {
			decv_hdl = pip->media_info->media.decv_hdl;
		}	
	}
	if (decv_hdl) {
		aui_decv_get(decv_hdl, AUI_DECV_GET_INFO, &pst_info);
		ret = pst_info.st_info.display_layer;
	}
	return ret;
}

static unsigned long test_aui_pip_stop_by_layer(enum aui_dis_layer display_layer)
{
	if (test_aui_pip_get_layer(&s_pip[PIP_0]) == display_layer) {
		test_aui_pip_stop_mp();
	}
	if (test_aui_pip_get_layer(&s_pip[PIP_1]) == display_layer) {
		test_aui_pip_stop_pip();
	}
	return 0;
}

static unsigned long test_aui_pip_stop_mp_layer(unsigned long *argc,char **argv,char *sz_out_put)
{	
	test_aui_pip_stop_by_layer(AUI_DIS_LAYER_VIDEO);

	return 0;
}

static unsigned long test_aui_pip_stop_pip_layer(unsigned long *argc,char **argv,char *sz_out_put)
{
	test_aui_pip_stop_by_layer(AUI_DIS_LAYER_AUXP);

	return 0;
}

static unsigned long test_aui_pip_set_mp_dis_rect(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = -1;
	aui_dis_zoom_rect src_rect, dst_rect;
	memset(&src_rect, 0, sizeof(aui_dis_zoom_rect));
	memset(&dst_rect, 0, sizeof(aui_dis_zoom_rect));
    if(*argc >= 8) {
    	src_rect.ui_startX = strtoul(argv[0], 0, 0);
    	src_rect.ui_startY = strtoul(argv[1], 0, 0);
    	src_rect.ui_width = strtoul(argv[2], 0, 0);
    	src_rect.ui_height = strtoul(argv[3], 0, 0);
    	dst_rect.ui_startX = strtoul(argv[4], 0, 0);
    	dst_rect.ui_startY = strtoul(argv[5], 0, 0);
    	dst_rect.ui_width = strtoul(argv[6], 0, 0);
    	dst_rect.ui_height = strtoul(argv[7], 0, 0);
    } else {
    	AUI_PRINTF("\narguments for set display rect:\n");
	 	AUI_PRINTF("usage:[Test Items] [src x],[src y],[src w],[src h],[des x],[des y],[des w],[des h]\n");	    
        AUI_PRINTF("such as input:\n");
        AUI_PRINTF("      preview: mo 0,0,720,2880,0,0,360,1440\n");
        AUI_PRINTF("     fullview: mo 0,0,720,2880,0,0,720,2880\n");
        return -1;
    }
	ret = test_aui_pip_set_dis_rect(AUI_DIS_LAYER_VIDEO, &src_rect, &dst_rect);
	return ret;
}

static unsigned long test_aui_pip_set_pip_dis_rect(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = -1;
	aui_dis_zoom_rect src_rect, dst_rect;
	memset(&src_rect, 0, sizeof(aui_dis_zoom_rect));
	memset(&dst_rect, 0, sizeof(aui_dis_zoom_rect));
    if(*argc >= 8) {
    	src_rect.ui_startX = strtoul(argv[0], 0, 0);
    	src_rect.ui_startY = strtoul(argv[1], 0, 0);
    	src_rect.ui_width = strtoul(argv[2], 0, 0);
    	src_rect.ui_height = strtoul(argv[3], 0, 0);
    	dst_rect.ui_startX = strtoul(argv[4], 0, 0);
    	dst_rect.ui_startY = strtoul(argv[5], 0, 0);
    	dst_rect.ui_width = strtoul(argv[6], 0, 0);
    	dst_rect.ui_height = strtoul(argv[7], 0, 0);
    } else {
    	AUI_PRINTF("\narguments for set display rect:\n");
	 	AUI_PRINTF("usage:[Test Items] [src x],[src y],[src w],[src h],[des x],[des y],[des w],[des h]\n");
        AUI_PRINTF("such as input:\n");
        AUI_PRINTF("      preview: po 0,0,720,2880,360,0,180,720\n");
        AUI_PRINTF("     fullview: po 0,0,720,2880,0,0,720,2880\n");
        return -1;
    }
	ret = test_aui_pip_set_dis_rect(AUI_DIS_LAYER_AUXP, &src_rect, &dst_rect);
	return ret;
}

static unsigned long test_aui_pip_de_init(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ret = 0;
	int i = 0;
	struct ali_app_pip *pip;
	test_aui_pip_stop_mp();
	test_aui_pip_stop_pip();
	for (i = 0; i < PIP_MAX; i++) {
		pip = &s_pip[i];
		if (pip->source_type == PIP_FROM_NIM) {
			free(pip->live_info);
		} else if (pip->source_type == PIP_FROM_MEDIA) {
			free(pip->media_info);
		}
		memset(pip, 0, sizeof(struct ali_app_pip));
	}
    AUI_PRINTF("\r\n close dis aui");
    if (s_dis_hdl) {
	    if(aui_dis_close(NULL, &s_dis_hdl)) {
        	AUI_PRINTF("\n aui_dis_close error \n");
        	ret = 1;
    	}
    }
    if (s_snd_hdl) {
	    if(aui_snd_close(s_snd_hdl)) {
        	AUI_PRINTF("\n aui_snd_close error \n");
        	ret = 1;
    	}
    	s_snd_hdl = NULL;
    }
    if (s_stc_hdl) {
	    if(aui_stc_close(s_stc_hdl)) {
        	AUI_PRINTF("\n aui_stc_close error \n");
        	ret = 1;
    	}
    	s_stc_hdl = NULL;
    }
	return ret;
}

static unsigned long test_aui_pip_set_dis_off(unsigned long *argc,char **argv,char *sz_out_put)
{
	enum aui_dis_layer dislayer = AUI_DIS_LAYER_VIDEO;
	unsigned char onoff = 0;
	unsigned long ret = 0;
	aui_attr_dis attr_dis;
    if(*argc < 2) {
    	AUI_PRINTF("usage: off [dis layer][onoff]\n");
    	AUI_PRINTF("            dis layer: 0: main, 1: pip\n");
    	AUI_PRINTF("                onoff: 0: off,  1: on\n");
    	return -1;
    }
    if (*argc > 1) {
    	dislayer = (enum aui_dis_layer)strtoul(argv[0], 0, 0);
    	onoff = strtoul(argv[1], 0, 0);
    }
    memset(&attr_dis, 0, sizeof(aui_attr_dis));
	attr_dis.uc_dev_idx = AUI_DIS_HD;
	if (s_dis_hdl == NULL) {
		aui_dis_open(&attr_dis,&s_dis_hdl);
	}
    if(aui_dis_layer_enable(s_dis_hdl, dislayer, onoff))
    	ret = -1;
    return ret;
}
#if 0
static unsigned long test_aui_swap_video_test(unsigned long *argc,char **argv,char *sz_out_put)
{
	int main_to_aux = 1;
    if (*argc > 0) {
    	main_to_aux = strtoul(argv[0], 0, 0);
    }
    if (main_to_aux) {
		test_aui_pip_change_video(&s_pip[PIP_0].pip_hdls, &s_pip[PIP_1].pip_hdls);
	} else {
		test_aui_pip_change_video(&s_pip[PIP_1].pip_hdls, &s_pip[PIP_0].pip_hdls);
	}
	return 0;
}
#endif

void test_pip_reg()
{
	aui_tu_reg_group("pip", "PIP test cases");
    //aui_tu_reg_item(2, "tt", AUI_CMD_TYPE_API, test_aui_pip_play_live_live, "simple test for MP live ts, PIP live ts");
    //aui_tu_reg_item(2, "te", AUI_CMD_TYPE_API, test_aui_pip_play_live_media, "simple test for MP live ts, PIP media(es)");
    //aui_tu_reg_item(2, "ee", AUI_CMD_TYPE_API, test_aui_pip_play_media_media, "simple test for MP media(es), PIP media(es)");
    aui_tu_reg_item(2, "m", AUI_CMD_TYPE_API, test_aui_pip_play_mp, "play stream display on MP, use decv 0 display on Main Picture as default");
    aui_tu_reg_item(2, "p", AUI_CMD_TYPE_API, test_aui_pip_play_pip, "play stream display on PIP, use decv 1 display on Picture In Picture as default");
    aui_tu_reg_item(2, "swap", AUI_CMD_TYPE_API, test_aui_pip_swap, "swap audio and video");
    aui_tu_reg_item(2, "ms", AUI_CMD_TYPE_API, test_aui_pip_stop_mp_layer, "stop playing on MP layer");
    aui_tu_reg_item(2, "ps", AUI_CMD_TYPE_API, test_aui_pip_stop_pip_layer, "stop playing on PIP layer");
    aui_tu_reg_item(2, "mo", AUI_CMD_TYPE_API, test_aui_pip_set_mp_dis_rect, "set MP dis rect");
    aui_tu_reg_item(2, "po", AUI_CMD_TYPE_API, test_aui_pip_set_pip_dis_rect, "set PIP dis rect");
    aui_tu_reg_item(2, "dis", AUI_CMD_TYPE_API, test_aui_pip_set_dis_off, "for all in one: set dis off");
    aui_tu_reg_item(2, "stop", AUI_CMD_TYPE_API, test_aui_pip_de_init, "for all in one: stop testing");
	    
}
