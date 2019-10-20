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
#include <sys/select.h>

#include <aui_av_injecter.h>
#include "aui_av_injecter_test.h"
#include "aui_av_injecter_audio_mix_test.h"
#include "aui_test_app_cmd.h"
#include "aui_dis.h"
#include "aui_stc.h"
#include "aui_snd.h"
#include "aui_decv.h"
#include "aui_help_print.h"
//#define PTS_TEST

int g_first_frame_showed = 0;

/* Command <-> Audo codec map list */
struct av_inject_audio_codec audio_codec_arry[] =
{
    /* audio codecs */
    {"CODEC_ID_MP2",    AUI_DECA_STREAM_TYPE_MPEG2},
    {"CODEC_ID_MP3",    AUI_DECA_STREAM_TYPE_MP3}, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
    {"CODEC_ID_AAC_LATM",    AUI_DECA_STREAM_TYPE_AAC_LATM},
    {"CODEC_ID_AAC_ADTS",    AUI_DECA_STREAM_TYPE_AAC_ADTS},
    {"CODEC_ID_AC3",    AUI_DECA_STREAM_TYPE_AC3},
    {"CODEC_ID_DTS",    AUI_DECA_STREAM_TYPE_DTS},
    {"CODEC_ID_VORBIS", AUI_DECA_STREAM_TYPE_OGG},
    {"CODEC_ID_WMAV1",  AUI_DECA_STREAM_TYPE_BYE1},
    {"CODEC_ID_FLAC",   AUI_DECA_STREAM_TYPE_FLAC},
    {"CODEC_ID_APE",    AUI_DECA_STREAM_TYPE_APE},
    {"CODEC_ID_EAC3",   AUI_DECA_STREAM_TYPE_EC3},
    {"CODEC_ID_RAW_PCM",AUI_DECA_STREAM_TYPE_RAW_PCM}
};

/* Command <-> video codec map list */
struct av_inject_video_codec video_codec_arry[] =
{
    {"h264",  AUI_DECV_FORMAT_AVC},    /**< h264 */
    {"xvid",  AUI_DECV_FORMAT_XVID},    /**< xvid */
    {"mpg2",  AUI_DECV_FORMAT_MPEG},    /**< mpg2 */
    {"flv1",  AUI_DECV_FORMAT_FLV1},    /**< flv1 */
    {"vp8",   AUI_DECV_FORMAT_VP8},     /**< vp8 */     
    {"wvc1",  AUI_DECV_FORMAT_WVC1},    /**< wvc1 */
    {"wmv3",  AUI_DECV_FORMAT_WX3},    /**< wmv3 */ 
    //{"wx3",   AUI_DECV_FORMAT_WX3},     /**< wx3 */ same as wx3
    {"rmvb",  AUI_DECV_FORMAT_RMVB},    /**< rmvb */
    {"mjpg",  AUI_DECV_FORMAT_MJPG},    /**< mjpg */
    //{"xd",    AUI_DECV_FORMAT_XVID},       /**< XD */same as xvid
	{"h265",  AUI_DECV_FORMAT_HEVC}    /**< h265 */
};

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

void aui_get_video_codec(char *src, aui_decv_format*codec_id)
{    
    int i = 0;

    for (i = 0; i < (int)(sizeof(video_codec_arry)/sizeof(video_codec_arry[0])); i++)
    {
		if (strcmp(video_codec_arry[i].command, src) == 0)
		{
			*codec_id = video_codec_arry[i].video_codec_id;
			break;
		}
	}

    if(i >= (int)(sizeof(video_codec_arry)/sizeof(video_codec_arry[0])))
    {
        printf("Cannot get match video codec, use default one");
        *codec_id = AUI_DECV_FORMAT_MPEG;
    }
}

void aui_get_audio_codec(char *src, aui_deca_stream_type *codec_id)
{
    int i = 0;

    for (i = 0; i < (int)(sizeof(audio_codec_arry)/sizeof(audio_codec_arry[0])); i++)
    {
		if (strcmp(audio_codec_arry[i].command, src) == 0)
		{
			*codec_id = audio_codec_arry[i].audio_codec_id;
			break;
		}
	}

    if(i >= (int)(sizeof(audio_codec_arry)/sizeof(audio_codec_arry[0])))
    {
        printf("Cannot get match audio codec, use default one");
        *codec_id = AUI_DECA_STREAM_TYPE_MPEG2;
    }
}

int test_av_inject_write_audio_data(void* handle, struct aui_av_packet* header,
	char* buffer, unsigned int len)
{
	int ret = 0;
	unsigned int audio_free_size = 0, audio_valid_size = 0, audio_total_size = 0;
	aui_hdl audio_decoder_handle = (aui_hdl)handle;
	aui_decoder_buffer_status buffer_status;
	
	aui_audio_decoder_get_buffer_status(audio_decoder_handle, &buffer_status);
	audio_total_size = buffer_status.total_size;
	audio_valid_size = buffer_status.valid_size;
	audio_free_size = buffer_status.free_size;
	if(audio_free_size > len && audio_valid_size < audio_total_size*4/5) 
	{
	    do {
	        ret = aui_audio_decoder_write_header(audio_decoder_handle, header);
	        if( AUI_RTN_SUCCESS != ret)
	        {
	            usleep(10*1000);
	        } else {
	            do {
	                ret = aui_audio_decoder_write(audio_decoder_handle, buffer, len);
	                if( AUI_RTN_SUCCESS != ret)
	                {
	                    usleep(10*1000);
	                }
	            } while(ret != AUI_RTN_SUCCESS);  
	        }
	    } while(ret != AUI_RTN_SUCCESS);
	} else {
		ret = -1;
	}
	return ret;
}

int test_av_inject_write_video_data(void* handle, struct aui_av_packet* header,
	char* buffer, unsigned int len)
{
	int ret = 0;
	unsigned int video_free_size = 0, video_valid_size = 0, video_total_size = 0;
	aui_hdl video_decoder_handle = (aui_hdl)handle;
	aui_decoder_buffer_status buffer_status;

	aui_video_decoder_get_buffer_status(video_decoder_handle, &buffer_status);
	video_total_size = buffer_status.total_size;
	video_valid_size = buffer_status.valid_size;
	video_free_size = buffer_status.free_size;
	if(video_free_size > len && video_valid_size < video_total_size*4/5) 
	{
	    do {
	        ret = aui_video_decoder_write_header(video_decoder_handle, header);
	        if( AUI_RTN_SUCCESS != ret)
	        {
	            usleep(10*1000);
	        } else {
	            do {
	                ret = aui_video_decoder_write(video_decoder_handle, buffer, len);
	                if( AUI_RTN_SUCCESS != ret)
	                {
	                    usleep(10*1000);
	                }
	            } while(ret != AUI_RTN_SUCCESS);  
	        }
	    } while(ret != AUI_RTN_SUCCESS);
	} else {
		ret = -1;
	}
	return ret;
}

int test_av_inject_get_file_info(char* file, char* suffix, FILE **f_data, 
	FILE **f_pktsize, FILE **f_pts, unsigned int *pktsize_size, 
	unsigned char ** extra_data, unsigned int *extra_data_size)
{
	FILE *data=NULL, *pktsize=NULL, *pts=NULL, *extra=NULL;
	unsigned int data_size=0, pkt_size=0, pts_size=0, extra_size=0;
	unsigned char *extra_data_buf=NULL;
	char* filename;
	char filepath[256];
    memset(filepath, 0, 256);
    strcpy(filepath, file);
    strcat(filepath, suffix);
    filename = filepath;
    data = fopen(filename, "rb");
    if(data == NULL) {
		log_info("Open file %s fail\n", filename);
		goto err;
	} else {
	    fseek(data, 0, SEEK_END);
		data_size = ftell(data);
		if(data_size <= 0) {
			log_info("%s data file size error %d\n", filepath, data_size);
			goto err;
		} else {
		    fseek(data, 0, SEEK_SET);
			fflush(data);
		    log_info("data size %d", data_size);
		    if (f_data != NULL)
				*f_data = data;
		}
	}
    memset(filepath, 0, 256);
    strcpy(filepath, file);
    strcat(filepath, suffix);
    strcat(filepath, ".pktsize");
    filename = filepath;
    pktsize = fopen(filename, "rb");
    if(pktsize == NULL) {
		log_info("Open file %s fail\n", filename);
		if (f_pktsize != NULL)
			goto err;
	} else {
	    fseek(pktsize, 0, SEEK_END);
		pkt_size = ftell(pktsize);
		if(pkt_size <= 0) {
			log_info("%s pkt size error %d\n", filepath, pkt_size);
			goto err;
		} else {
		    fseek(pktsize, 0, SEEK_SET);
			fflush(pktsize);
		    log_info("pkt frame size %ld", pkt_size);
		    if (f_pktsize != NULL && pktsize_size != NULL) {
				*f_pktsize = pktsize;
				*pktsize_size = pkt_size;
			}
		}
	}
    memset(filepath, 0, 256);
    strcpy(filepath, file);
    strcat(filepath, suffix);
    strcat(filepath, ".pts");
    filename = filepath;
    pts = fopen(filename, "rb");
    if(pts == NULL) {
		log_info("Open file %s fail\n", filename);
		if (f_pts != NULL)
			goto err;
	} else {
	    fseek(pts, 0, SEEK_END);
		pts_size = ftell(pts);
		if(pts_size <= 0) {
			log_info("%s pts size error %d\n", filename, pts_size);
			goto err;
		} else {
		    fseek(pts, 0, SEEK_SET);
			fflush(pts);
		    log_info("pts size %ld", pts_size);
		    if (f_pts != NULL)
		    	*f_pts = pts;
	    }
    }
    memset(filepath, 0, 256);
    strcpy(filepath, file);
    strcat(filepath, suffix);
    strcat(filepath, ".extradata");
    filename = filepath;
    extra = fopen(filename, "rb");

	if(extra == NULL) {
		log_info("Open file %s fail\n", filename);
	} else {
        fseek(extra, 0, SEEK_END);
    	extra_size = ftell(extra);
    	if(extra_size <= 0) {        	
    		log_info("%s extra data size error %d\n", filename, extra_size);
    		fclose(extra);
    		extra = NULL;
    	} else {
            fseek(extra, 0, SEEK_SET);
        	fflush(extra);
        	log_info("extra data size %ld", extra_size);
        	if (extra_data_size != NULL && extra_data != NULL) {
	        	*extra_data_size = extra_size;  
		    	extra_data_buf = (uint8_t *)malloc(extra_size);
		    	if(extra_data_buf == NULL) {
		        	log_info("Malloc frame buffer fail\n");
		        	goto err;
		    	} else {
				    if(extra_size != fread(extra_data_buf, 1, extra_size, extra)) {
				        log_info("read extra data fail\n");
				        goto err;
				    }
				    *extra_data = extra_data_buf;
			    }
		    }
		    fclose(extra);
        }
    }
    return 0;
err:
	if (data) {
		fclose(data);
		data = NULL;
	}
	if (pktsize) {
		fclose(pktsize);
		pktsize = NULL;
	}
	if (pts) {
		fclose(pts);
		pts = NULL;
	}
	if (extra) {
		fclose(extra);
		extra = NULL;
	}
	if (extra_data_buf) {
		free(extra_data_buf);
		extra_data_buf = NULL;
	}
	*f_data = NULL;
	*f_pktsize = NULL;
	*f_pts = NULL;
	*extra_data_size = 0;
	*extra_data = NULL;
	return -1;
}

unsigned int test_av_inject_seek2pts(FILE *f_data, FILE *f_pkt, 
	FILE *f_pts, unsigned int seek_pts)
{
	unsigned int frm_size, frm_pts;
	unsigned int last_pts = 0;
	unsigned int seek_frame_size = 0;
	unsigned int current_frame_index = 0;
	unsigned int read_size = 0;
	
	printf("seek to %d\n",seek_pts);

	fseek(f_data, 0, SEEK_SET);
	fflush(f_data);
	fseek(f_pkt, 0, SEEK_SET);
	fflush(f_pkt);
	fseek(f_pts, 0, SEEK_SET);
	fflush(f_pts);
	for(;;)
	{
		read_size =  fread(&frm_pts, 1, sizeof(uint32_t), f_pts);
		if (read_size != sizeof(uint32_t)){
			printf("pts file goto the end!\n");
			break;			
		}
		read_size = fread(&frm_size, 1, sizeof(uint32_t), f_pkt);
		if (read_size != sizeof(uint32_t)){
			printf("pkt file goto the end!\n");
			break;			
		}
		seek_frame_size += frm_size;	
		current_frame_index++;
		if((seek_pts <= frm_pts) && (seek_pts >=  last_pts))
		{
			printf("find the seek pts:%d, frame index: %d, seek to: %d\n", frm_pts, current_frame_index, seek_frame_size);
            fseek(f_data, seek_frame_size, SEEK_SET);
			break;
		}		
		last_pts = frm_pts;
	}
	
	return current_frame_index;
}

static void inject_audio_es_usages(void)
{
	printf("usage:[Test Items] [Audio codec id],[File path],[Audio sample rate],[Width],[Bit rate],[Block align],[Endian],[Sign flag]\n");
	printf("      Test Items:  1: audio inject, 2:video inject 3:video stream inject 4:AV sync inject\n");
	printf("      Audio codec: [CODEC_ID_MP2 CODEC_ID_MP3 CODEC_ID_AAC_LATM CODEC_ID_AAC_ADTS CODEC_ID_AC3 CODEC_ID_DTS CODEC_ID_VORBIS]\n");
	printf("                   [CODEC_ID_WMAV1 CODEC_ID_FLAC CODEC_ID_APE CODEC_ID_EAC3 CODEC_ID_RAW_PCM]\n");
    printf("      File path:   audio file name\n");
    printf("      Audio sampling rate: Audo sampling rate\n");
    printf("      Width: bits per sample \n");
    printf("      Bit rate: Audo bit rate for WMA\n");
    printf("      Block align: Audo block align for WMA\n");
    printf("      Endian: Endian for PCM, 0: little, 1: big\n");
    printf("      Sign flag: Sign flag for PCM, 0: unsigned, 1: signed\n");
    printf("      Channel: Channel number for example 1: Mono, 2: stereo\n");
}

static unsigned long test_audio_playback(unsigned long *argc, char **argv,char *sz_out_put)
{
    aui_deca_stream_type codec_id;
    aui_hdl audio_decoder_out = 0;
    aui_audio_decoder_status status;
    aui_decoder_buffer_status buffer_status;
    struct aui_av_packet pkt;
    aui_audio_info_init audio_info_init;
    aui_attr_snd attr_snd;
    aui_hdl	aui_snd_hd = NULL;
    uint32_t key_press = -1;

    FILE *f_audio_data=NULL, *f_audio_pkt=NULL,*f_audio_pts=NULL;
    uint32_t audio_pkt_size, audio_extra_data_size=0;
    uint32_t j = 0, audio_free_size = 0, audio_valid_size = 0, audio_total_size = 0;
    uint32_t audio_frm_size, audio_frm_pts;
    uint32_t audio_read_size = 0,  audio_need_read = 1, ret;
    uint8_t *audio_frame_buf = NULL, *audio_extra_data_buf = NULL;
    // static int current_display_pts;
    aui_audio_decoder_status audio_decoder_status;
    int print_cnt =250;

    // current_display_pts = 0;
    if(*argc < 3) {
        inject_audio_es_usages();
        return -1;
    }

    aui_get_audio_codec(argv[0], &codec_id);
    log_info("get audio file info\n");
    ret = test_av_inject_get_file_info(argv[1], ".audio0", &f_audio_data, &f_audio_pkt, 
        &f_audio_pts, &audio_pkt_size, &audio_extra_data_buf, &audio_extra_data_size);
    printf("audio: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
        f_audio_data, f_audio_pkt, f_audio_pts, audio_pkt_size, audio_extra_data_size);		
    
    if (ret != 0){
        return ret;
    }

    //set audio volume
    memset(&attr_snd,0,sizeof(aui_attr_snd));	
    aui_snd_open(&attr_snd, &aui_snd_hd);
    aui_snd_mute_set(aui_snd_hd, 0);
    aui_snd_vol_set(aui_snd_hd, 100);

    memset(&pkt, 0, sizeof(pkt));
    audio_frame_buf = (uint8_t *)malloc(0x100000);
    if(audio_frame_buf == NULL) {
        log_info("Malloc frame buffer fail\n");
        return -1;
    }
    
    MEMSET(&audio_info_init, 0, sizeof (aui_audio_info_init));
    audio_info_init.channels = 2;
    audio_info_init.codec_id = codec_id;
    audio_info_init.sample_rate = strtoul(argv[2], 0, 0);
    if (*argc > 3)
        audio_info_init.nb_bits_per_sample = strtoul(argv[3], 0, 0);
    if (*argc > 4)
        audio_info_init.bit_rate = strtoul(argv[4], 0, 0);
    if (*argc > 5)    
        audio_info_init.block_align = strtoul(argv[5], 0, 0);
        
    audio_info_init.extradata = audio_extra_data_buf;
    audio_info_init.extradata_size = audio_extra_data_size;

    if (*argc > 6)
        audio_info_init.endian = strtoul(argv[6], 0, 0);
    if (*argc > 7)   
        audio_info_init.sign_flag = strtoul(argv[7], 0, 0); 
    
    if (*argc > 8)   
        audio_info_init.channels = strtoul(argv[8], 0, 0);  

    if(AUI_RTN_SUCCESS != aui_audio_decoder_open(&audio_info_init, &audio_decoder_out)) {
        log_info("open audio decoder fail\n");
        return -1;
    }

    print_cnt = 250;
to_begin:
    audio_read_size = 0;
    audio_frm_size = 0;
    audio_frm_pts = 0;
    audio_need_read = 1;
    log_info("Press p key to pause audio, press r key to un-pause \n");
    log_info("Press s key to seek audio, press q key to exit \n");

    while(1)  {	
        if(audio_need_read) {           
            /* parse a complete audio frame */
            if(fread(&audio_frm_size, 1, sizeof(uint32_t), f_audio_pkt) <= 0){
				AUI_PRINTF("failed to read audio_frm_size\n");
				break;
			}
            audio_read_size = fread(audio_frame_buf, 1, audio_frm_size, f_audio_data);
            if(fread(&audio_frm_pts, 1, sizeof(uint32_t), f_audio_pts) <= 0) {
				AUI_PRINTF("failed to read audio_frame_buf\n");
				break;
			}
        }
        //printf("audio_read_size: %d\n", audio_read_size);
        if((audio_read_size > 0) && (audio_read_size == audio_frm_size)) {
            pkt.pts = audio_frm_pts;
            pkt.dts = AUI_AV_NOPTS_VALUE;
            pkt.size = audio_read_size;

            ret = test_av_inject_write_audio_data((void *)audio_decoder_out, &pkt, audio_frame_buf, audio_read_size);

            if (ret != 0) {
                //write fail, retry
                //log_info("write fail\n");
                audio_need_read = 0;
                continue;
            } else {
                if(++j >= audio_pkt_size/4) {
                    log_info("write data done\n");
                    break;
                } else {
                    audio_need_read = 1;
                }
                audio_read_size = 0;
            }

            memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
            aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);
            //if(current_display_pts != audio_decoder_status.last_pts)
            {
                if(print_cnt < 300) {
                    printf("begin,current display time is %dms,audio_frm_pts=%dms\n",audio_decoder_status.last_pts,audio_frm_pts);	
                }
                print_cnt++;
            // current_display_pts = audio_decoder_status.last_pts;
            }
        } 

        key_press = kbhit();
        if (key_press)  {
                int has_audio = 1;

            if(key_press == 'p') {//pause
                printf("\n pause\n");
                aui_audio_decoder_pause(audio_decoder_out,1);	
                memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
                aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);
                printf("pause ,current display time is %dms\n",audio_decoder_status.last_pts);	
            } else if(key_press == 'r') {
                printf("\n un-pause\n");
                aui_audio_decoder_pause(audio_decoder_out,0);

                for(print_cnt = 0;print_cnt < 100;print_cnt++) {
                    memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
                    aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);
                    printf("resume ,current display time is %dms\n",audio_decoder_status.last_pts);	

                }
            } else if(key_press == 's') {		
                unsigned int seek_pts = 0;
                aui_audio_decoder_flush(audio_decoder_out);
                sleep(1);
                memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
                aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);

                printf("flush ,current display time is %dms,sent to :%dms\n",audio_decoder_status.last_pts,audio_frm_pts);	
                print_cnt = 0;

                //seek to pts
                fflush(stdin);
                printf("pls input seek pts \n");
                if( 0 == scanf("%d", &seek_pts)) {
					printf("scanf failed\n");
					 goto to_begin;
				}
                printf("seek to %d\n",seek_pts);

                j = test_av_inject_seek2pts(f_audio_data, f_audio_pkt, f_audio_pts, seek_pts);

                goto to_begin;
            } else if(key_press == 'q') {
                goto exit;
            }
        }        
    }
    
    int i = 0;
    do {
        sleep(1);
        memset(&buffer_status, 0, sizeof(buffer_status));
        aui_audio_decoder_get_buffer_status(audio_decoder_out, &buffer_status);
        log_info("wait %d\n", buffer_status.valid_size);
        aui_audio_decoder_get_status(audio_decoder_out, &status);
        log_info("status: frames_decoded: %d, last_pts: %d\n", 
        status.frames_decoded, status.last_pts);
        i++;
    } while(buffer_status.valid_size > 0 || i < 3);

exit:    
    aui_audio_decoder_close(audio_decoder_out);
    audio_decoder_out = 0;
    sleep(1);
    aui_snd_close(aui_snd_hd);
    if(f_audio_data)
        fclose(f_audio_data);
    if(f_audio_pkt)
        fclose(f_audio_pkt);
    if (audio_extra_data_buf)
        free(audio_extra_data_buf);
    if (audio_frame_buf)    
        free(audio_frame_buf);

    log_info("Audio file: %s play success!\n", argv[1]);

    return 0;
}

static void video_info_change_cb(void * p_user_hld, unsigned int parm1, unsigned parm2)
{
	struct aui_decv_info_cb *new_info = (struct aui_decv_info_cb *)parm1;
	printf("video_info_change_cb,width =%d,height =%d ,frame_rate =%d \n",new_info->pic_width,new_info->pic_height,new_info->frame_rate);
}
static void first_i_frame_deocded(void * p_user_hld, unsigned int parm1, unsigned parm2)
{
    g_first_frame_showed = 1;
	printf("first_i_frame_deocded\n");
}

static void inject_video_es_usages(void)
{
	printf("usage:[Test Items] [Video codec id],[Pic width],[Pic height],[Video frame rate],[File path]\n");
	printf("      Test Items:  1: audio inject, 2:video inject 3:video stream inject 4:AV sync inject\n");
	printf("      Video codec: [h264 xvid mpg2 flv1 vp8 wvc1 wmv3 rmvb mjpg h265]\n");
	printf("      Pic width:  video picture width\n");
    printf("      Pic_height: video picture height\n");
    printf("      video frame rate: Video frame rate,[frame rate equal 23976, while 23.976 fps]\n");
    printf("      File path:   Video ES file name(like $PATH/xx.mp4.video0)\n");
}

/* Inject video ES stream by a interate frame
 * the package size in header is the frame size
 * frame to frame send the data, this can support 
 * All video codec in mars
*/
static unsigned long test_video_playback(unsigned long *argc, char **argv,char *sz_out_put)
{
	FILE *f_video_data, *f_video_pkt,*f_video_pts;
	uint32_t video_data_size, video_pkt_size, video_extra_data_size, video_pts_size;
    uint8_t *frame = NULL, *video_extra_data_buf = NULL;;
    uint32_t video_read_size = 0, video_frm_size = 0, video_frm_pts = 0;
    uint32_t video_frame_cnt = 0, video_need_read = 1;
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    aui_video_decoder_init decoder_init;
    aui_decv_format codec_id = AUI_DECV_FORMAT_MPEG;
    aui_hdl decoder_out = 0;
    aui_decoder_buffer_status buffer_status;

    aui_video_decoder_status video_status;
    unsigned int frames_displayed_last = 0;

    struct aui_av_packet pkt;
    #ifdef PTS_TEST    
    int test_pts = 0;
    #endif
	aui_hdl	aui_decv_hd = NULL;
	aui_attr_decv attr_decv;	
	struct aui_decv_callback_node first_iframe_callback;
	struct aui_decv_callback_node info_change_callback;
	uint32_t key_press = -1;
	
	if(*argc < 5)
	{
		inject_video_es_usages();
		return -1;
	}
	
    aui_get_video_codec(argv[0], &codec_id);
    log_info("get video file info\n");
    ret = test_av_inject_get_file_info(argv[4], ".video0", &f_video_data, &f_video_pkt, 
    		&f_video_pts, &video_pkt_size, &video_extra_data_buf, &video_extra_data_size);
    printf("video: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
    	f_video_data, f_video_pkt, f_video_pts, video_pkt_size, video_extra_data_size);		
    if (ret != 0){
    	return ret;
    }
	 
    frame = (uint8_t *)malloc(0x100000);
    if(frame == NULL) {
        log_info("Malloc frame buffer fail\n");
        return -1;
    }
	memset(&attr_decv,0,sizeof(aui_attr_decv));   	
	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &aui_decv_hd)) {
		if (aui_decv_open(&attr_decv,&aui_decv_hd)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return AUI_RTN_FAIL;
		}
	}
    /* init decoder */
    memset(&decoder_init, 0, sizeof(decoder_init));
    decoder_init.decode_mode = AUI_VIDEO_DEC_MODE_FRAME;
    decoder_init.codec_id    = codec_id;
    decoder_init.pic_width   = strtoul(argv[1], 0, 0);
    decoder_init.pic_height  = strtoul(argv[2], 0, 0);
    decoder_init.sar_width   = 1;
    decoder_init.sar_height  = 1;
    decoder_init.frame_rate  = strtoul(argv[3], 0, 0);
    decoder_init.extradata_size = video_extra_data_size;
    decoder_init.extradata = video_extra_data_buf;
    if( AUI_RTN_SUCCESS != aui_video_decoder_open(&decoder_init, &decoder_out))
    {
        log_info("open video decoder fail\n");
        goto exit;
    }

    if(AUI_RTN_SUCCESS != aui_video_decoder_set_sync(decoder_out, 0)) {
        log_info("aui_video_decoder_set_sync fail\n");
        return -1;
    }

	//register first I frame decocded callback
	memset(&first_iframe_callback,0,sizeof(struct aui_decv_callback_node));
	first_iframe_callback.callback = first_i_frame_deocded;
	first_iframe_callback.type = AUI_DECV_CB_FIRST_I_FRAME_DECODED;
	aui_decv_set(aui_decv_hd,AUI_DECV_SET_REG_CALLBACK,&first_iframe_callback); 
	//register info change callback
	memset(&info_change_callback,0,sizeof(struct aui_decv_callback_node));
	info_change_callback.callback = video_info_change_cb;
	info_change_callback.type = AUI_DECV_CB_INFO_CHANGED;
	aui_decv_set(aui_decv_hd,AUI_DECV_SET_REG_CALLBACK,&info_change_callback);    
    //ali_video_ioctl(decoder_out, VDECIO_MP_SET_QUICK_MODE, 1);        
    while(1) {

        if(video_need_read) {
            /* parse a complete video frame(frame size and frame pts) */           
           if(fread(&video_frm_size, 1, sizeof(uint32_t), f_video_pkt)<=0) {
				AUI_PRINTF("failed to read video_frm_size\n");
				break;
		   }
            video_read_size = fread(frame, 1, video_frm_size, f_video_data);
           if(fread(&video_frm_pts, 1, sizeof(uint32_t), f_video_pts)<=0){
				AUI_PRINTF("failed to read video_frm_pts\n");
				break;
		   }
        }

        if((video_read_size > 0) && (video_read_size == video_frm_size)) {
        	pkt.pts = video_frm_pts;
	        pkt.dts = AUI_AV_NOPTS_VALUE;
	        pkt.size = video_read_size;
	    	ret = test_av_inject_write_video_data((void *)decoder_out, &pkt, frame, video_read_size);

			if (ret != 0) {
				//write fail, retry
				//printf("write video fail retry\n");
				video_need_read = 0;
				usleep(25*1000);
			} else {              
	            if(++video_frame_cnt >= video_pkt_size/4) {
					video_need_read = 0;
					break;
	            } else {
					video_need_read = 1;
				}
				video_read_size = 0;
			}
        }

        key_press = kbhit();
        if(key_press == 'q'){
			    goto exit;
		}
    }

    do {       
        sleep(1);
        memset(&buffer_status, 0, sizeof(buffer_status));
        aui_video_decoder_get_buffer_status(decoder_out, &buffer_status);
        aui_video_decoder_get_status(decoder_out, &video_status);
        printf("frames displayed %d\n", video_status.frames_displayed);
        if(video_status.frames_displayed != frames_displayed_last){
            frames_displayed_last = video_status.frames_displayed;
        }else{
            break;
        }
        log_info("wait %d\n", buffer_status.valid_size);
    } while(1);

exit:
    aui_video_decoder_close(decoder_out);
    aui_decv_close(NULL,&aui_decv_hd);
    #ifdef STC_TEST 
    aui_stc_close(stc_id);
    #endif
    decoder_out = 0;
    sleep(1);
    if (f_video_data)
		fclose(f_video_data);
	if (f_video_pkt)	
    	fclose(f_video_pkt);
    if (f_video_pts)	
    	fclose(f_video_pts);
    if (frame)
        free(frame);
    if (video_extra_data_buf) 
        free(video_extra_data_buf);
    log_info("Exit playing file %s\n", argv[4]);
    (void)sz_out_put;
    return 0;
}


/* Inject video ES stream by length, not a interate frame
 * the package size in header is not a frame size, it may 
 * be a fixed length or a valitile length, this only can 
 * support m2v/h264 codec.
*/
static unsigned long test_stream_playback(unsigned long *argc, char **argv, char *sz_out_put)
{
    FILE *fp=NULL,*extra=NULL;
    uint32_t file_size = 0, extra_size = 0;
    uint32_t i = 0, free_size = 0, valid_size = 0, total_size = 0;
    uint32_t last_buffer_used = 0, codec_id;
    uint8_t *frame;
    unsigned char *extra_data_buf=NULL;
    uint32_t size = 10 * 1024, read_size = 0, need_read = 1; 
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    aui_video_decoder_init decoder_init;
    aui_hdl decoder_out = 0;
    aui_video_decoder_status status;
    aui_decoder_buffer_status buffer_status;
    aui_video_decoder_status decore_status;
    struct aui_av_packet pkt;
    const char* filename = NULL;
    char filepath[256];

	if(*argc < 5)
	{
		inject_video_es_usages();
		return -1;
	}
    
	aui_get_video_codec(argv[0], &codec_id);

    strcpy(filepath, argv[4]);
	filename = filepath;
    log_info("Enter playing file %s\n", filename);

    memset(&pkt, 0, sizeof(pkt));
    memset(&decoder_init, 0, sizeof(decoder_init));
    memset(&buffer_status, 0, sizeof(buffer_status));

    fp = fopen(filename, "rb");
    if(fp == NULL) {
		log_info("Open file %s fail\n", filename);
		return -1;
	}

    fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if(file_size <= 0) {
		log_info("File size error %lld\n", file_size);
		return -1;
	}
	fflush(fp);
    log_info("File size %lld", file_size);

    frame = (uint8_t *)malloc(0x100000);
    if(frame == NULL) {
        log_info("Malloc frame buffer fail\n");
        return -1;
    }

    memset(filepath, 0, 256);
    strcpy(filepath, argv[4]);
    strcat(filepath, ".extradata");
    filename = filepath;
    extra = fopen(filename, "rb");

	if(extra == NULL) {
		log_info("Open file %s fail\n", filename);
	} else {
        fseek(extra, 0, SEEK_END);
    	extra_size = ftell(extra);
    	if(extra_size <= 0) {        	
    		log_info("%s extra data size error %d\n", filename, extra_size);
    		fclose(extra);
    		extra = NULL;
    	} else {
            fseek(extra, 0, SEEK_SET);
        	fflush(extra);
        	log_info("extra data size %ld", extra_size); 
	    	extra_data_buf = (uint8_t *)malloc(extra_size);
	    	if(extra_data_buf == NULL) {
	        	log_info("Malloc frame buffer fail\n");
	    	} else {
			    if(extra_size != fread(extra_data_buf, 1, extra_size, extra)) {
			        log_info("read extra data fail\n");
			        free(extra_data_buf);
			        extra_data_buf = NULL;
			        extra_size = 0;
			    }
		    }
		    fclose(extra);
        }
    }

    /* init decoder */
    decoder_init.decode_mode = AUI_VIDEO_DEC_MODE_STREAM;
    decoder_init.codec_id    = codec_id;
    decoder_init.pic_width   = strtoul(argv[1], 0, 0);
    decoder_init.pic_height  = strtoul(argv[2], 0, 0);;
    decoder_init.sar_width   = 1;
    decoder_init.sar_height  = 1;
    decoder_init.frame_rate  = strtoul(argv[3], 0, 0);
    decoder_init.extradata   = extra_data_buf;
    decoder_init.extradata_size = extra_size;
    if( AUI_RTN_SUCCESS != aui_video_decoder_open(&decoder_init, &decoder_out))
    {
        log_info("open video decoder fail\n");
        return -1;
    }
	if(AUI_RTN_SUCCESS != aui_video_decoder_set_sync(decoder_out, 0)) {
        log_info("aui_video_decoder_set_sync fail\n");
        return -1;
    }
    //ali_video_ioctl(decoder_out, VDECIO_MP_SET_QUICK_MODE, 1);
    while(1) {
        aui_video_decoder_get_buffer_status(decoder_out, &buffer_status);
        total_size = buffer_status.total_size;
        valid_size = buffer_status.valid_size;
        free_size = buffer_status.free_size;

        if(need_read) {
            read_size = fread(frame, 1, size, fp);
        }

        if(read_size > 0) {
            if(free_size > read_size && valid_size < total_size*4/5) {
                pkt.pts = AUI_AV_NOPTS_VALUE;
                pkt.dts = AUI_AV_NOPTS_VALUE;
                pkt.size = read_size;
                //log_info("read %d %d\n", i, read_size);

                do {
                        ret = aui_video_decoder_write_header(decoder_out, &pkt);
                        if (AUI_RTN_SUCCESS != ret)
                        {
                            usleep(100*1000);
                        }
                } while(ret != AUI_RTN_SUCCESS);

                do {
                        ret = aui_video_decoder_write(decoder_out, frame, read_size);
                        if (AUI_RTN_SUCCESS != ret)
                        {
                            usleep(100*1000);
                        }
                } while(ret != AUI_RTN_SUCCESS);

                i++;
                need_read = 1;
            } else {
                memset(&decore_status, 0, sizeof(decore_status));
                usleep(100*1000);
                need_read = 0;
            }
        } else {
            log_info("read frame error %d %d\n", read_size, size);
            break;
        }
    }

    i = 0;

    while(1)
    {
        memset(&status, 0, sizeof(decore_status));
        memset(&buffer_status, 0, sizeof(buffer_status));

        aui_video_decoder_get_status(decoder_out, &decore_status);
        aui_video_decoder_get_buffer_status(decoder_out, &buffer_status);

        if(buffer_status.valid_size == 0
           && decore_status.buffer_used == last_buffer_used)
        {
            if(++i > 5)
                break;
        }
        else
        {
            i = 0;
        }
        last_buffer_used = decore_status.buffer_used;

        //log_info("wait %lu/%lu\n", decore_status.buffer_used, decore_status.buffer_size);
        sleep(1);
    }

    aui_video_decoder_close(decoder_out);
    decoder_out = 0;

	fclose(fp);
    free(frame);

    log_info("Exit playing file %s", filename);

    return 0;
}
        
static void avsync_usages(void)
{
	printf("usage:[Test Items] [Video codec id],[Audio codec id],[File path],[pic_width],[pic_height],[video frame rate],[audio sampling rate],");
	printf("[select source video x offset],[select source video y offset],[select source video width],[select source video height],");
	printf("[video display x coordinate],[video display y coordinate],[vidoe display width],[video display height]\n");
	printf("    Test Items:  1: audio inject, 2:video inject 3:video stream inject 4:AV sync inject\n");
	printf("    Video codec: [h264 xvid mpg2 flv1 vp8 wvc1 wmv3 rmvb mjpg h265]\n");
    printf("    Audio codec: [CODEC_ID_MP1 CODEC_ID_MP2 CODEC_ID_MP3 CODEC_ID_AAC CODEC_ID_AC3 CODEC_ID_DTS CODEC_ID_EAC3 etc]\n");
	printf("    pic_width:  video picture width\n");
    printf("    pic_height: video picture height\n");
    printf("    video frame rate: Video frame rate,[frame rate equal 23976, while 23.976 fps]\n");
    printf("    audio sampling rate: Audo sampling rate\n");
    printf("    File path:   mdiea player file name(like $PATH/xx.mp4)\n");
    printf("                 audio data: xx.mp4.audio0\n");
    printf("                 audio extra data: xx.mp4.audio0.extradata\n");
    printf("                 audio pktsize: xx.mp4.audio0.pktsize\n");
    printf("                 audio pts: xx.mp4.audio0.pts\n");
    printf("                 video data: xx.mp4.video0\n");
    printf("                 video extra data: xx.mp4.video0.extradata\n");
    printf("                 video pktsize: xx.mp4.video0.pktsize\n");
    printf("                 video pts: xx.mp4.video0.pts\n");
	printf("    select source video: select the source video rect that want to be display on screen. e.g,if you want to select the full source video ,just input 0,0,720,2880.\n");
	printf("                 select source video x offset: the source video x offset that will be select\n");
	printf("                 select source video y offset: the source video y offset that will be select\n");
	printf("                 select source video width : the source video width that will be select\n");
	printf("                 select source video height: the source video height that will be select\n");
	printf("    video display rect: select the source video coordinate that want to be display on screen.e.g,if you want to display the video on full screen,just input 0,0,720,2880.\n");
	printf("                 video display x coordinate: the video x coordinate that will be display\n");
	printf("                 video display x coordinate: the video y coordinate that will be display\n");
	printf("                 video display x coordinate: the video width that will be display\n");
	printf("                 video display x coordinate: the video height that will be display\n");	
}

static int set_video_output_rect(aui_hdl aui_dis_hdl, aui_dis_zoom_rect *src_rect,aui_dis_zoom_rect *dst_rect)
{
    AUI_RTN_CODE ret;
    aui_view_mode mode = AUI_VIEW_MODE_FULL;

    if (dst_rect->ui_height >= src_rect->ui_height
        && dst_rect->ui_width >= src_rect->ui_width) {
        mode = AUI_VIEW_MODE_FULL;
    } else {
        mode = AUI_VIEW_MODE_PREVIEW;
    }
    
    ret = aui_dis_mode_set(aui_dis_hdl, mode, src_rect, dst_rect);
    if(0!= ret) {
        log_info("aui_dis_mode_set failed \n");
        return -1;
    }
    //aui_dis_close(NULL,&aui_dis_hdl);
    return 0;
}

/* Audio and Video sync test */
static unsigned long test_avsync_playback(unsigned long *argc, char **argv, char *sz_out_put)
{
	FILE *f_video_data, *f_video_pkt,*f_video_pts;
	FILE *f_audio_data, *f_audio_pkt,*f_audio_pts;
	uint32_t video_data_size, video_pkt_size, video_extra_data_size, video_pts_size;
	uint32_t audio_data_size, audio_pkt_size, audio_extra_data_size=0, audio_pts_size;
	uint32_t video_frame_cnt = 0, video_free_size = 0, video_valid_size = 0, video_total_size = 0;
	uint32_t audio_frame_cnt = 0, audio_free_size = 0, audio_valid_size = 0, audio_total_size = 0;
	uint32_t video_read_size = 0, audio_read_size = 0, video_need_read = 1, audio_need_read = 1, ret;
	uint32_t video_frm_size, audio_frm_size, video_frm_pts, audio_frm_pts;
	uint8_t *video_frame_buf = NULL, *video_extra_data_buf = NULL;
	uint8_t *audio_frame_buf = NULL, *audio_extra_data_buf = NULL;

	
    aui_video_decoder_init video_decoder_init;    
    aui_hdl video_decoder_out = 0;
    aui_decoder_buffer_status video_buffer_status;
    aui_decoder_buffer_status audio_buffer_status;
    aui_decv_format video_codec_id;
    aui_deca_stream_type audio_codec_id;
    aui_hdl audio_decoder_out = 0;
    aui_audio_info_init audio_info_init;
    
    struct aui_av_packet pkt;
    aui_dis_zoom_rect src_rect;
	aui_dis_zoom_rect dst_rect;
	aui_hdl stc_hd;
	uint32_t key_press = -1;
	aui_hdl	aui_snd_hd = NULL;
	aui_hdl	aui_decv_hd = NULL;
    aui_attr_snd attr_snd;
	aui_attr_decv attr_decv;	
	struct aui_decv_callback_node decv_callback;
    aui_hdl aui_dis_hdl=0;
    aui_attr_dis attr_dis;
	unsigned char  p_enable_diplay;
	int test_pts_a = 150;
	int last_pts = 0;
	if(*argc < 7)
	{
		avsync_usages();
		return -1;
	}
	
    aui_get_video_codec(argv[0], &video_codec_id);
    aui_get_audio_codec(argv[1], &audio_codec_id);
    
    memset(&pkt, 0, sizeof(pkt));
    memset(&video_decoder_init, 0, sizeof(video_decoder_init));
    memset(&audio_info_init, 0, sizeof(audio_info_init));
    memset(&video_buffer_status, 0, sizeof(video_buffer_status));
    memset(&audio_buffer_status, 0, sizeof(audio_buffer_status));
    log_info("get video file info\n");
    ret = test_av_inject_get_file_info(argv[2], ".video0", &f_video_data, &f_video_pkt, 
    		&f_video_pts, &video_pkt_size, &video_extra_data_buf, &video_extra_data_size);
    printf("video: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
    	f_video_data, f_video_pkt, f_video_pts, video_pkt_size, video_extra_data_size);		
    if (ret != 0){
    	return ret;
    }
    log_info("get audio file info\n");
    ret = test_av_inject_get_file_info(argv[2], ".audio0", &f_audio_data, &f_audio_pkt, 
    		&f_audio_pts, &audio_pkt_size, &audio_extra_data_buf, &audio_extra_data_size);
    printf("audio: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
    	f_audio_data, f_audio_pkt, f_audio_pts, audio_pkt_size, audio_extra_data_size);		
    if (ret != 0){
    	return ret;
    }
    //Video frame buffer
    video_frame_buf = (uint8_t *)malloc(0x100000);
    if(video_frame_buf == NULL) 
    {
        log_info("Malloc video frame buffer fail\n");
        return -1;
    }

    //Audio frame buffer
    audio_frame_buf = (uint8_t *)malloc(0x100000);
    if(audio_frame_buf == NULL) 
    {
        log_info("Malloc audio frame buffer fail\n");
        return -1;
    }
    
    //open stc
	aui_stc_open(&stc_hd);

	//set audio volume
	memset(&attr_snd,0,sizeof(aui_attr_snd));	
	aui_snd_open(&attr_snd, &aui_snd_hd);
	aui_snd_mute_set(aui_snd_hd, 0);
	aui_snd_vol_set(aui_snd_hd, 100);
	//open vpo device
	memset(&attr_dis, 0, sizeof(aui_attr_dis));
	attr_dis.uc_dev_idx = AUI_DIS_HD;
	aui_dis_open(&attr_dis,&aui_dis_hdl);
    aui_dis_video_enable(aui_dis_hdl, 0);
	//register first I frame decocded callback
	memset(&attr_decv,0,sizeof(aui_attr_decv));
	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &aui_decv_hd)) {
		if (aui_decv_open(&attr_decv,&aui_decv_hd)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return AUI_RTN_FAIL;
		}
	}
    /* init decoder */
    video_decoder_init.decode_mode = AUI_VIDEO_DEC_MODE_FRAME;
    video_decoder_init.codec_id    = video_codec_id;
    video_decoder_init.pic_width   = strtoul(argv[3], 0, 0);
    video_decoder_init.pic_height  = strtoul(argv[4], 0, 0);
    video_decoder_init.sar_width   = 1;
    video_decoder_init.sar_height  = 1;
    video_decoder_init.frame_rate  = strtoul(argv[5], 0, 0);
    video_decoder_init.extradata_size = video_extra_data_size;
    video_decoder_init.extradata = video_extra_data_buf;

    if( AUI_RTN_SUCCESS != aui_video_decoder_open(&video_decoder_init, &video_decoder_out))
    {
        log_info("open video decoder fail\n");
        return -1;
    }
	memset(&decv_callback,0,sizeof(struct aui_decv_callback_node));
	decv_callback.callback = first_i_frame_deocded;
	decv_callback.type = AUI_DECV_CB_FIRST_I_FRAME_DECODED;
	aui_decv_set(aui_decv_hd,AUI_DECV_SET_REG_CALLBACK,&decv_callback);
    //ali_video_ioctl(decoder_out, VDECIO_MP_SET_QUICK_MODE, 1);
    // Audio decorder init
    audio_info_init.channels = 2;
    audio_info_init.codec_id = audio_codec_id;
    audio_info_init.nb_bits_per_sample = 16;
    audio_info_init.sample_rate = strtoul(argv[6], 0, 0);
    audio_info_init.extradata = audio_extra_data_buf;
    audio_info_init.extradata_size = audio_extra_data_size;

    if( AUI_RTN_SUCCESS != aui_audio_decoder_open(&audio_info_init, &audio_decoder_out))
    {
        log_info("open video decoder fail\n");
        ret = -1;
    }
   
    if(*argc >= 15) {
    	src_rect.ui_startX = strtoul(argv[7], 0, 0);
    	src_rect.ui_startY = strtoul(argv[8], 0, 0);
    	src_rect.ui_width = strtoul(argv[9], 0, 0);
    	src_rect.ui_height = strtoul(argv[10], 0, 0);
    	dst_rect.ui_startX = strtoul(argv[11], 0, 0);
    	dst_rect.ui_startY = strtoul(argv[12], 0, 0);
    	dst_rect.ui_width = strtoul(argv[13], 0, 0);
    	dst_rect.ui_height = strtoul(argv[14], 0, 0);

    	if(set_video_output_rect(aui_dis_hdl, &src_rect,&dst_rect) != 0)
    	{
    		log_info("set_video_output_rect failed! \n");
    	}
    }
	if(AUI_RTN_SUCCESS != aui_audio_decoder_set_sync(audio_decoder_out, 1)) {
        log_info("aui_audio_decoder_set_sync fail\n");
        return -1;
    }
	test_pts_a = 0;
to_begin:
	log_info("Press p key to pause video/audio,press r key to un-pause \n");
	log_info("Press s key to seek video/audio, press q key to exit \n");
	test_pts_a = 150;
	int first_test = 1;
    while(1) {

        if(video_need_read) {
            /* parse a complete video frame(frame size and frame pts) */           
            if(fread(&video_frm_size, 1, sizeof(uint32_t), f_video_pkt) <= 0){
				AUI_PRINTF("failed to read video_frm_size\n");
				break;
			}
            video_read_size = fread(video_frame_buf, 1, video_frm_size, f_video_data);
            if(fread(&video_frm_pts, 1, sizeof(uint32_t), f_video_pts) <= 0){
				AUI_PRINTF("failed to read video_frm_pts\n");
				break;
			}
        }

        if((video_read_size > 0) && (video_read_size == video_frm_size)) {
        	pkt.pts = video_frm_pts;
	        pkt.dts = AUI_AV_NOPTS_VALUE;
	        pkt.size = video_read_size;
	    	ret = test_av_inject_write_video_data((void *)video_decoder_out, &pkt, video_frame_buf, video_read_size);

			if (ret != 0) {
				//write fail, retry
				//printf("write video fail retry\n");
				video_need_read = 0;
				usleep(25*1000);
			} else {              
	            if(++video_frame_cnt >= video_pkt_size/4) {
					video_need_read = 0;
	            } else {
					video_need_read = 1;
				}
				video_read_size = 0;
			}
        } 

        if(audio_need_read) {           
            /* parse a complete audio frame */
           if(fread(&audio_frm_size, 1, sizeof(uint32_t), f_audio_pkt) <= 0) {
				AUI_PRINTF("failed to read audio_frm_size\n");
				break;
		   }
            audio_read_size = fread(audio_frame_buf, 1, audio_frm_size, f_audio_data);
           if(fread(&audio_frm_pts, 1, sizeof(uint32_t), f_audio_pts) <= 0){
				AUI_PRINTF("failed to read audio_frame_buf\n");
				break;
		   }
        }

        if((audio_read_size > 0) && (audio_read_size == audio_frm_size)) {
            pkt.pts = audio_frm_pts;
	        pkt.dts = AUI_AV_NOPTS_VALUE;
	        pkt.size = audio_read_size;
	    	ret = test_av_inject_write_audio_data((void *)audio_decoder_out, &pkt, audio_frame_buf, audio_read_size);

			if (ret != 0) {
				//write fail, retry
				audio_need_read = 0;
				usleep(5*1000);
			} else {
				if(++audio_frame_cnt >= audio_pkt_size/4) {
	                audio_need_read = 0;
				} else {
					audio_need_read = 1;
				}
				audio_read_size = 0;
			}
        } 
        if((audio_frame_cnt >= audio_pkt_size/4) && (video_frame_cnt >= video_pkt_size/4)) {
        	//printf("*** a:%d, v: %d\n", audio_frame_cnt, video_frame_cnt);
            break;
        }

		aui_audio_decoder_status audio_decoder_status;
		aui_video_decoder_status video_decoder_status;
		if(test_pts_a < 200 ) {
			memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
			aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);
			memset(&video_decoder_status,0,sizeof(aui_video_decoder_status));
			aui_video_decoder_get_status(video_decoder_out, &video_decoder_status);
			
			//if(audio_decoder_status.last_pts != 0)
			{
				printf("display time is %ldms,snd time:%d,video_frm_pts%d,audio_frm_pts:%d\n",video_decoder_status.last_pts,audio_decoder_status.last_pts,video_frm_pts,audio_frm_pts);	
				test_pts_a++;
			}
		}		
		key_press = kbhit();
        if (key_press) {
			
			int has_audio = 1;
			
			unsigned long current_display_pts = 0;
			if(key_press == 'p') {//pause
			
				printf("\n pause\n");
				aui_video_decoder_pause(video_decoder_out,1);
				aui_audio_decoder_pause(audio_decoder_out,1);		
			} else if(key_press == 'r') {
				printf("\n un-pause\n");
				aui_video_decoder_pause(video_decoder_out,0);
				aui_audio_decoder_pause(audio_decoder_out,0);
			} else if (key_press == 'm') {	
				int sample_rate = 0;
				char mix_filepath[256];
				printf("pls input mix file path\n");
				memset(mix_filepath, 0, 256);
				if(0 == scanf("%s", mix_filepath)){
					printf("scanf failed\n");
					goto to_begin;
				}
				printf("pls input mix audio sample rate\n");
				if(0 == scanf("%d", &sample_rate)) {
					printf("scanf failed\n");
					goto to_begin;
				}
				test_aui_av_inject_audio_mix(mix_filepath, sample_rate);
			} else if(key_press == 's') {
				unsigned int seek_pts = 0;
				
				//if the stream has audio,the current display pts is audio pts,else if it only has video ,the current display pts is video pts
				if(has_audio) {
					memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
					aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);
					current_display_pts = audio_decoder_status.last_pts;
				} else {
					memset(&video_decoder_status,0,sizeof(aui_video_decoder_status));
					aui_video_decoder_get_status(video_decoder_out, &video_decoder_status);
					current_display_pts = video_decoder_status.last_pts;
				}
				
				log_info("current display time is %ldms\n",current_display_pts);				
				aui_video_decoder_get_status(video_decoder_out, &video_decoder_status);
				log_info("sar_width:%d sar_height:%d\n",video_decoder_status.sar_width,video_decoder_status.sar_height);
				log_info("width:%d height:%d\n",video_decoder_status.width,video_decoder_status.height);
	            log_info("flush\n");
	     
	            aui_audio_decoder_flush(audio_decoder_out);
	            aui_video_decoder_flush(video_decoder_out, AUI_VDEC_PLAYBACK_MODE_NORMAL);
				
				memset(&audio_decoder_status,0,sizeof(aui_audio_decoder_status));
				aui_audio_decoder_get_status(audio_decoder_out, &audio_decoder_status);
				printf("after flush current audio time is %ldms\n",audio_decoder_status.last_pts);				
				memset(&video_decoder_status,0,sizeof(aui_video_decoder_status));
				aui_video_decoder_get_status(video_decoder_out, &video_decoder_status);
				printf("after flush current video time is %ldms\n",video_decoder_status.last_pts);	            
	            
				video_need_read = 1;
				audio_need_read = 1;
				fflush(stdin);
				printf("pls input seek pts \n");
				if(0 == scanf("%d", &seek_pts)) {
					printf("scanf failed\n");
					goto to_begin;
				}
				/*****************/
				unsigned int last_pts = 0;
				unsigned int seek_frame_size = 0;
				
				printf("seek to %d\n",seek_pts);
				//seek video
				video_frame_cnt = test_av_inject_seek2pts(f_video_data, f_video_pkt, f_video_pts, seek_pts);
				//seek audio
				audio_frame_cnt = test_av_inject_seek2pts(f_audio_data, f_audio_pkt, f_audio_pts, seek_pts);				
				/*****************/
	            goto to_begin;
			} else if (key_press == 'q') {
                goto exit;
            }
        }       
    }
    do {
        sleep(1);
        memset(&video_buffer_status, 0, sizeof(video_buffer_status));
        aui_video_decoder_get_buffer_status(video_decoder_out, &video_buffer_status);
        log_info("wait %ld\n", video_buffer_status.valid_size);
    } while(video_buffer_status.valid_size > 0);
exit:
    aui_audio_decoder_close(audio_decoder_out);
    aui_video_decoder_close(video_decoder_out);
    aui_stc_close(stc_hd);
	aui_snd_close(aui_snd_hd);
    aui_dis_close(NULL, &aui_dis_hdl);
	aui_decv_set(aui_decv_hd,AUI_DECV_SET_UNREG_CALLBACK, &decv_callback.type);
	aui_decv_close(NULL,&aui_decv_hd);
    audio_decoder_out = 0;
    video_decoder_out = 0;
    sleep(1);
	fclose(f_video_data);
    fclose(f_video_pkt);
    fclose(f_video_pts);
    fclose(f_audio_data);
    fclose(f_audio_pkt);
    fclose(f_audio_pts);
    if (video_frame_buf)
        free(video_frame_buf);
    if (audio_frame_buf)    
        free(audio_frame_buf);
    if (video_extra_data_buf) 
        free(video_extra_data_buf);
    if (audio_extra_data_buf)
        free(audio_extra_data_buf); 
    log_info("Media player: %s AV sync success!\n", argv[2]);

    return 0;
}

static unsigned long test_dynamic_adaptive_video_playback(unsigned long *argc, char **argv, char *sz_out_put)
{
	FILE *f_video_data, *f_video_pkt,*f_video_pts;
	unsigned int video_data_size = 0, video_pkt_size = 0, video_extra_data_size = 0, video_pts_size = 0;
	unsigned int video_frame_cnt = 0, video_free_size = 0, video_valid_size = 0, video_total_size = 0;
	unsigned int video_read_size = 0, video_need_read = 1,  ret;
	unsigned int video_frm_size, video_frm_pts;
	unsigned char *video_frame_buf = NULL, *video_extra_data_buf = NULL;
	unsigned int file_index = 0;
    aui_video_decoder_init video_decoder_init;    
    aui_hdl video_decoder_out = 0;
    aui_decoder_buffer_status video_buffer_status;
    aui_decv_format video_codec_id;
    struct aui_av_packet pkt;
	const char* filename = NULL;
    char filepath[256];
	aui_hdl stc_hd;
	uint32_t key_press = -1;
	aui_hdl	aui_decv_hd = NULL;
	aui_attr_decv attr_decv;	
	struct aui_decv_callback_node decv_callback;
    aui_hdl aui_dis_hdl=0;
    aui_attr_dis attr_dis;
	unsigned char  p_enable_diplay;
	unsigned char dash = 1;
	unsigned char write_done = 0;
	if(*argc < 4)
	{
		printf("usage:[Test Items] [Video codec id],[Video Frame rate],[File path 0],[File path 1]\n");
		printf("    Video codec id: [h264 xvid mpg2 flv1 vp8 wvc1 wmv3 rmvb mjpg h265]\n");
		printf("    Video Frame rate: Video frame rate,[frame rate equal 23976, while 23.976 fps]\n");
		return -1;
	}
    aui_get_video_codec(argv[0], &video_codec_id);
    memset(&pkt, 0, sizeof(pkt));
    memset(&video_decoder_init, 0, sizeof(video_decoder_init));
    memset(&video_buffer_status, 0, sizeof(video_buffer_status));
    log_info("get video file info\n");
    ret = test_av_inject_get_file_info(argv[2], ".video0", &f_video_data, &f_video_pkt, 
    		&f_video_pts, &video_pkt_size, &video_extra_data_buf, &video_extra_data_size);
    printf("video: data: %x, pkt: %x, pts: %x,pkt size: %d, extradata size: %d\n",
    	f_video_data, f_video_pkt, f_video_pts, video_pkt_size, video_extra_data_size);		
    if (ret != 0){
    	return ret;
    }
    video_frame_buf = (uint8_t *)malloc(0x100000);
    if(video_frame_buf == NULL) 
    {
        log_info("Malloc video frame buffer fail\n");
        return -1;
    }
    
	//open vpo device
	memset(&attr_dis, 0, sizeof(aui_attr_dis));
	attr_dis.uc_dev_idx = AUI_DIS_HD;
	aui_dis_open(&attr_dis,&aui_dis_hdl);
    aui_dis_video_enable(aui_dis_hdl, 0);
	//register first I frame decocded callback
	memset(&attr_decv,0,sizeof(aui_attr_decv));
	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &aui_decv_hd)) {
		if (aui_decv_open(&attr_decv,&aui_decv_hd)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return AUI_RTN_FAIL;
		}
	}
    /* init decoder */
    video_decoder_init.decode_mode = AUI_VIDEO_DEC_MODE_FRAME;
    video_decoder_init.codec_id    = video_codec_id;
    video_decoder_init.sar_width   = 1;
    video_decoder_init.sar_height  = 1;
    video_decoder_init.frame_rate  = strtoul(argv[1], 0, 0);
    video_decoder_init.extradata_size = video_extra_data_size;
    video_decoder_init.extradata = video_extra_data_buf;

    if( AUI_RTN_SUCCESS != aui_video_decoder_open(&video_decoder_init, &video_decoder_out))
    {
        log_info("open video decoder fail\n");
        return -1;
    }
	memset(&decv_callback,0,sizeof(struct aui_decv_callback_node));
	decv_callback.callback = video_info_change_cb;
	decv_callback.type = AUI_DECV_CB_INFO_CHANGED;
	aui_decv_set(aui_decv_hd, AUI_DECV_SET_REG_CALLBACK, &decv_callback);
	aui_decv_set(aui_decv_hd, AUI_DECV_SET_VARIABLE_RESOLUTION, &dash);


	if(AUI_RTN_SUCCESS != aui_video_decoder_set_sync(video_decoder_out, 0))
	{
        log_info("aui_video_decoder_set_sync fail\n");
        return -1;
    }
    while(1) {

        if(video_need_read) {
            /* parse a complete video frame(frame size and frame pts) */           
            if(fread(&video_frm_size, 1, sizeof(uint32_t), f_video_pkt) <= 0) {
				printf("failed to read video_frm_size\n");
				break;
			}
            video_read_size = fread(video_frame_buf, 1, video_frm_size, f_video_data);
            if(fread(&video_frm_pts, 1, sizeof(uint32_t), f_video_pts) <= 0) {
				printf("failed to read video_frame_buf\n");
				break;
			}
        }

        if((video_read_size > 0) && (video_read_size == video_frm_size)) {
        	pkt.pts = video_frm_pts;
	        pkt.dts = AUI_AV_NOPTS_VALUE;
	        pkt.size = video_read_size;
	    	ret = test_av_inject_write_video_data((void *)video_decoder_out, &pkt, video_frame_buf, video_read_size);

			if (ret != 0) {
				//write fail, retry
				//printf("write video fail retry\n");
				video_need_read = 0;
				usleep(25*1000);
			} else {              
	            if(++video_frame_cnt >= video_pkt_size/4) {
					video_need_read = 0;
					write_done = 1;
	            } else {
					video_need_read = 1;
				}
				video_read_size = 0;
			}
        }

        if(video_need_read == 0 && file_index == 0 && write_done == 1) {
        	fclose(f_video_data);
        	fclose(f_video_pkt);
        	fclose(f_video_pts);
        	if (video_extra_data_buf) {
        		free(video_extra_data_buf);
        		video_extra_data_buf = NULL;
        	}
	        ret = test_av_inject_get_file_info(argv[3], ".video0", &f_video_data, &f_video_pkt, 
    		&f_video_pts, &video_pkt_size, &video_extra_data_buf, &video_extra_data_size);
   			printf("video: data: %x, pkt: %x, pts: %x, pkt size: %d, extradata size: %d\n",
    		f_video_data, f_video_pkt, f_video_pts, video_pkt_size, video_extra_data_size);		
    		if (ret != 0){
    			return ret;
    		} 
		    video_need_read = 1;
		    video_frame_cnt = 0;
		    file_index++;
		    //write video extra data		    

	      	pkt.pts = 0;
            pkt.dts = AUI_AV_NOPTS_VALUE;
            pkt.size = video_extra_data_size;
            pkt.flag = AUI_AV_PACKET_EXTRA_DATA;
            do {
	    		ret = test_av_inject_write_video_data((void *)video_decoder_out, &pkt, video_extra_data_buf, video_extra_data_size);
	    		if (ret)
	    			usleep(25*1000);
	    	}while (ret != 0);

		    pkt.flag = AUI_AV_PACKET_ES_DATA;
		    write_done = 0;
        }
        if(video_frame_cnt >= video_pkt_size/4) {
            break;
        }
		key_press = kbhit();
        if (key_press) {	
			if(key_press == 'p') {//pause
			
				printf("\n pause\n");
				aui_video_decoder_pause(video_decoder_out,1);
			} else if(key_press == 'r') {
				printf("\n un-pause\n");
				aui_video_decoder_pause(video_decoder_out,0);
			} else if (key_press == 'q') {
                goto exit;
            }
        }       
    }
    do {
        sleep(1);
        memset(&video_buffer_status, 0, sizeof(video_buffer_status));
        aui_video_decoder_get_buffer_status(video_decoder_out, &video_buffer_status);
        log_info("wait %ld\n", video_buffer_status.valid_size);
    } while(video_buffer_status.valid_size > 0);
exit:
    aui_video_decoder_close(video_decoder_out);
    aui_dis_close(NULL, &aui_dis_hdl);
	aui_decv_set(aui_decv_hd,AUI_DECV_SET_UNREG_CALLBACK, &decv_callback.type);
	aui_decv_close(NULL,&aui_decv_hd);
    video_decoder_out = 0;
    sleep(1);
    if (f_video_data)
		fclose(f_video_data);
	if (f_video_pkt)	
    	fclose(f_video_pkt);
    if (f_video_pts)	
    	fclose(f_video_pts);
    if (video_frame_buf)
        free(video_frame_buf);
    if (video_extra_data_buf) 
        free(video_extra_data_buf);
    log_info("Media player play success!\n");
    return 0;
}

static unsigned long test_av_inject_help(unsigned long *argc, char **argv, char *sz_out_put)
{
    aui_print_help_header("\nAV inject Help");
    #define AV_INJECT_HELP_PART1  "Need to extract the ES data from the stream firstly, user can generate test files by tools"
    #define AV_INJECT_HELP_PART2  "The test file xxx may use files as following:"
    #define AV_INJECT_HELP_PART3  "             xxx.audio0: audio frames"
    #define AV_INJECT_HELP_PART4  "     xxx.audio0.pktsize: size of each audio frame, every 4 bytes take one value"
    #define AV_INJECT_HELP_PART5  "         xxx.audio0.pts: PTS of each audio frame, unit is ms, every 4 bytes take one value"
    #define AV_INJECT_HELP_PART6  "   xxx.audio0.extradata: extra data which is helpful for audio decoding"
    #define AV_INJECT_HELP_PART7  "             xxx.video0: video frames"
    #define AV_INJECT_HELP_PART8  "     xxx.video0.pktsize: size of each video frame, every 4 bytes take one value"
    #define AV_INJECT_HELP_PART9  "         xxx.video0.pts: PTS of each video frame, unit is ms, every 4 bytes take one value"
    #define AV_INJECT_HELP_PART10 "   xxx.video0.extradata: extra data which is helpful for video decoding"

    aui_print_help_instruction_newline(AV_INJECT_HELP_PART1);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART2);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART3);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART4);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART5);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART6);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART7);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART8);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART9);
    aui_print_help_instruction_newline(AV_INJECT_HELP_PART10);
    /* AV_AV_INJECT_1_HELP */
    #define AV_INJECT_1_HELP_PART1  "usage: [Test Items] [Audio codec id],[File path],[Audio sample rate],[Width],[Bit rate],[Block align],[Endian],[Sign flag]"
    #define AV_INJECT_1_HELP_PART2  "       [Audio codec id]: CODEC_ID_MP2"
    #define AV_INJECT_1_HELP_PART3  "                         CODEC_ID_MP3"
    #define AV_INJECT_1_HELP_PART4  "                         CODEC_ID_AAC_LATM"
    #define AV_INJECT_1_HELP_PART5  "                         CODEC_ID_AAC_ADTS"
    #define AV_INJECT_1_HELP_PART6  "                         CODEC_ID_AC3"
    #define AV_INJECT_1_HELP_PART7  "                         CODEC_ID_DTS"
    #define AV_INJECT_1_HELP_PART8  "                         CODEC_ID_VORBIS"
    #define AV_INJECT_1_HELP_PART9  "                         CODEC_ID_WMAV1"
    #define AV_INJECT_1_HELP_PART10  "                         CODEC_ID_FLAC"
    #define AV_INJECT_1_HELP_PART11  "                         CODEC_ID_APE"
    #define AV_INJECT_1_HELP_PART12  "                         CODEC_ID_EAC3"
    #define AV_INJECT_1_HELP_PART13  "                         CODEC_ID_RAW_PCM"
    #define AV_INJECT_1_HELP_PART14  "       [File path]: test file(xxx) path, the related files should be in the same path"	
    #define AV_INJECT_1_HELP_PART15 "                    test files: xxx.audio0, xxx.audio0.pktsize, xxx.audio0.pts, xxx.audio0.extradata(if any)"
    #define AV_INJECT_1_HELP_PART16 "       [Audio sampling rate]: audio sampling rate"
    #define AV_INJECT_1_HELP_PART17 "       [Width]: bits per sample"
    #define AV_INJECT_1_HELP_PART18 "       [Bit rate]: audio bit rate for WMA"
    #define AV_INJECT_1_HELP_PART19 "       [Block align]: audio block align for WMA"
    #define AV_INJECT_1_HELP_PART20 "       [Endian]: endian for PCM, 0: little, 1: big"
    #define AV_INJECT_1_HELP_PART21 "       [Sign flag]: Sign flag for PCM, 0: unsigned, 1: signed"
    #define AV_INJECT_1_HELP_PART22 "       [Channel]: Channel number for example 1: Mono, 2: Stereo"
    #define AV_INJECT_1_HELP_PART23 "example: 1 CODEC_ID_AAC,xxx,48000,0,0,0\n"

    aui_print_help_command("\'1\'");
    aui_print_help_instruction_newline("inject audio ES by frame");
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART1);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART2);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART3);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART4);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART5);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART6);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART7);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART8);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART9);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART10);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART11);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART12);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART13);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART14);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART15);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART16);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART17);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART18);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART19);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART20);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART21);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART22);
    aui_print_help_instruction_newline(AV_INJECT_1_HELP_PART23);

    #define AV_INJECT_2_HELP_PART1  "usage: [Test Items] [Video codec id],[Pic width],[Pic height],[Video frame rate],[File path]"
    #define AV_INJECT_2_HELP_PART2  "       [Video codec id]: h264 xvid mpg2 flv1 vp8 wvc1 wmv3 rmvb mjpg h265"
    #define AV_INJECT_2_HELP_PART3  "       [Pic width]: width of video picture"
    #define AV_INJECT_2_HELP_PART4  "       [Pic height]: height of video picture"
    #define AV_INJECT_2_HELP_PART5  "       [Video frame rate]: frame rate equal 23976, while 23.976 fps"
    #define AV_INJECT_2_HELP_PART6  "       [File path]: test file(xxx) path, the related files should be in the same path"	
    #define AV_INJECT_2_HELP_PART7  "                    test files: xxx.video0, xxx.video0.pktsize, xxx.video0.pts, xxx.video0.extradata(if any)"
    #define AV_INJECT_2_HELP_PART8 "example: 2 h264,1920,800,25000,xxx\n"

    /* AV_AV_INJECT_2_HELP */
    aui_print_help_command("\'2\'");
    aui_print_help_instruction_newline("inject video ES by frame: inject an entire frame to decoder");
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART1);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART2);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART3);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART4);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART5);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART6);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART7);
    aui_print_help_instruction_newline(AV_INJECT_2_HELP_PART8);

    #define AV_INJECT_3_HELP_PART1  "usage: [Test Items] [Video codec id],[Pic width],[Pic height],[Video frame rate],[File path]"
    #define AV_INJECT_3_HELP_PART2  "       [Video codec id]: h264 mpg2"
    #define AV_INJECT_3_HELP_PART3  "       [Pic width]: width of video picture"
    #define AV_INJECT_3_HELP_PART4  "       [Pic height]: height of video picture"
    #define AV_INJECT_3_HELP_PART5  "       [Video frame rate]: frame rate equal 23976, while 23.976 fps"
    #define AV_INJECT_3_HELP_PART6  "       [File path]: test file(xxx.video0) path, the related files should be in the same path"	
    #define AV_INJECT_3_HELP_PART7  "                    test files: xxx.video0.extradata(if any)"
    #define AV_INJECT_3_HELP_PART8  "example: 3 h264,1920,800,25000,xxx\n"

    /* AV_AV_INJECT_3_HELP */
    aui_print_help_command("\'3\'");
    aui_print_help_instruction_newline("inject video ES by size: inject the ES data to decoder in fixed size, default 10K");
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART1);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART2);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART3);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART4);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART5);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART6);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART7);
    aui_print_help_instruction_newline(AV_INJECT_3_HELP_PART8);

    #define AV_INJECT_4_HELP_PART1  "usage: [Test Items] [Video codec id],[Audio codec id],[File path],[pic_width],[pic_height],[video frame rate],[audio sampling rate],"
    #define AV_INJECT_4_HELP_PART2  "       [select source video x offset],[select source video y offset],[select source video width],[select source video height],"
    #define AV_INJECT_4_HELP_PART3  "       [video display x coordinate],[video display y coordinate],[vidoe display width],[video display height]"
    #define AV_INJECT_4_HELP_PART4  "Please see the help of the \"1\" and \"2\" command for more details for the inputting parameters of following:"
    #define AV_INJECT_4_HELP_PART5  "       [Video codec id], [Audio codec id],[pic_width],[pic_height],[video frame rate],[audio sampling rate]" 
    #define AV_INJECT_4_HELP_PART6  "       [File path]: test file(xxx) path, the related files should be in the same path"
    #define AV_INJECT_4_HELP_PART7  "                    test files: xxx.audio0, xxx.audio0.pktsize, xxx.audio0.pts, xxx.audio0.extradata(if any)"
    #define AV_INJECT_4_HELP_PART8  "                                xxx.video0, xxx.video0.pktsize, xxx.video0.pts, xxx.video0.extradata(if any)"
    #define AV_INJECT_4_HELP_PART9  "Select source video: optional for preview or fullview, select the source video rect that want to be display on screen. e.g,if you want to select the full source video ,just input 0,0,720,2880.\n"
    #define AV_INJECT_4_HELP_PART10  "       [select source video x offset]: the source video x offset that will be select\n"
    #define AV_INJECT_4_HELP_PART11 "       [select source video y offset]: the source video y offset that will be select\n"
    #define AV_INJECT_4_HELP_PART12 "       [select source video width]: the source video width that will be select\n"
    #define AV_INJECT_4_HELP_PART13 "       [select source video height]: the source video height that will be select\n"
    #define AV_INJECT_4_HELP_PART14 "Video display rect: optional for preview or fullview, select the source video coordinate that want to be display on screen.e.g,if you want to display the video on full screen,just input 0,0,720,2880.\n"
    #define AV_INJECT_4_HELP_PART15 "       [video display x coordinate]: the video x coordinate that will be display\n"
    #define AV_INJECT_4_HELP_PART16 "       [video display x coordinate]: the video y coordinate that will be display\n"
    #define AV_INJECT_4_HELP_PART17 "       [video display x coordinate]: the video width that will be display\n"
    #define AV_INJECT_4_HELP_PART18 "       [video display x coordinate]: the video height that will be display\n"	
    #define AV_INJECT_4_HELP_PART19 "example:" 
    #define AV_INJECT_4_HELP_PART20 "       normal: 4 h264,CODEC_ID_AAC,xxx,640,360,23900,48000"
    #define AV_INJECT_4_HELP_PART21 "       preview: 4 h264,CODEC_ID_AAC,xxx,640,360,23900,48000,0,0,720,2880,0,0,360,2880"
    #define AV_INJECT_4_HELP_PART22 "       fullview: 4 h264,CODEC_ID_AAC,xxx,640,360,23900,48000,0,0,720,2880,0,0,720,2880\n"
    /* AV_INJECT_4_HELP_PART */
    aui_print_help_command("\'4\'");
    aui_print_help_instruction_newline("inject AV ES and do avsync");
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART1);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART2);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART3);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART4);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART5);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART6);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART7);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART8);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART9);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART10);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART11);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART12);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART13);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART14);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART15);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART16);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART17);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART18);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART19);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART20);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART21);
    aui_print_help_instruction_newline(AV_INJECT_4_HELP_PART22);

    #define AV_INJECT_5_HELP_PART1  "usage: [Test Items] [Video codec id],[Video Frame rate],[File path 0],[File path 1]"
    #define AV_INJECT_5_HELP_PART2  "       [Video codec id]: h264 xvid mpg2 flv1 vp8 wvc1 wx3 rmvb mjpg h265"
    #define AV_INJECT_5_HELP_PART3  "       [Video frame rate]: frame rate equal 23976, while 23.976 fps"
    #define AV_INJECT_5_HELP_PART4  "       [File path 0]: test file(xxx) path, the related files should be in the same path"	
    #define AV_INJECT_5_HELP_PART5  "                      test files: xxx.video0, xxx.video0.pktsize, xxx.video0.pts, xxx.video0.extradata(if any)"
    #define AV_INJECT_5_HELP_PART6  "       [File path 1]: test file(xxxxxx) path, the related files should be in the same path"	
    #define AV_INJECT_5_HELP_PART7  "                      test files: xxxxxx.video0, xxxxxx.video0.pktsize, xxxxxx.video0.pts, xxxxxx.video0.extradata(if any)"
    #define AV_INJECT_5_HELP_PART8  "example: 5 h264,25000,xxx,xxxxxx"

    /* AV_AV_INJECT_3_HELP */
    aui_print_help_command("\'5\'");
    aui_print_help_instruction_newline("inject AV ES for dynamic resolution");
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART1);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART2);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART3);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART4);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART5);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART6);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART7);
    aui_print_help_instruction_newline(AV_INJECT_5_HELP_PART8);
    return AUI_RTN_HELP;
}

void av_injecter_tests_reg(void)
{
	aui_tu_reg_group("av_inject", "av inject test");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_audio_playback,  "inject audio ES by frame");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_video_playback,  "inject video ES by frame");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_stream_playback, "inject video ES by size");
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_avsync_playback, "inject AV ES and do avsync");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_dynamic_adaptive_video_playback, 
    	"inject AV ES for dynamic resolution");
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_av_inject_help, "AV inject help");
	aui_tu_reg_item(2, "mix", AUI_CMD_TYPE_API, test_aui_av_inject_audio_mix_item, "mix pcm audio to main audio");	
}
