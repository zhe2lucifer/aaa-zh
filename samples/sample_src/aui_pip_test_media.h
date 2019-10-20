#ifndef __AUI_PIP_TEST_MEDIA_H_
#define __AUI_PIP_TEST_MEDIA_H_

struct ali_app_pip_media_init_para {
    char* filepath;
    unsigned char video_format;
    unsigned char audio_format;
    unsigned long video_frame_rate;
    unsigned char video_id;
    unsigned char dis_layer;	
};

typedef struct pip_test_media{
	pthread_t media_thread;
	int first_play;	
	int play_audio;
	int loop;
	int id;
	aui_hdl audio_hdl;//aui av inject: audio
	aui_hdl decv_hdl;//decv
}pip_test_media;

/**
*   @brief      init related parameters to test media with av inject
*/
int test_aui_pip_init_para_for_media(unsigned long *argc,char **argv, 
				unsigned char dis_layer, unsigned char decv_id, struct ali_app_pip_media_init_para *init_para);

/**
*   @brief      play media, create a thread to play es data
*/
int test_aui_pip_play_media(struct ali_app_pip_media_init_para* init_para, 
	struct pip_test_media *media);


/**
*   @brief      stop playing media, destory the thread and stop playing
*/
int test_aui_pip_stop_media(pip_test_media *media);

/**
*   @brief      restart audio playing
*/
int test_aui_pip_start_media_audio(pip_test_media *media);

/**
*   @brief      stop playing audio
*/
int test_aui_pip_stop_media_audio(pip_test_media *media);

#endif

