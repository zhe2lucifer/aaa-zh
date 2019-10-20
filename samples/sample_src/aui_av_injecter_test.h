/**@file
*    @brief 		The head file of AUI AV INJECTER test
*    @author		Christian.xie
*    @date			2014-3-11
*	 @version 		1.0.0
*    @note			Copyright(C) ALi Corporation. All rights reserved.  
	    			AV INJECTER is used to directly inject auido/video ES/PES data to A/V decorder.\n
	    			The injecter support PES and ES mode,which can used for up-layer media player.
	    			

*/
#ifndef _AUI_AV_INJECTER_TEST_H
#define _AUI_AV_INJECTER_TEST_H

#define ARRAY_SIZE(a)     (sizeof(a) / sizeof((a)[0]))

#define log_info(fmt, ...)  do { fprintf(stderr, "TEST INFO-" fmt "\n", ##__VA_ARGS__); } while (0)
#define log_error(fmt, ...) do { fprintf(stderr, "TEST ERROR-" fmt "\n", ##__VA_ARGS__); } while (0)

/*!
 \brief Represents the audio or video streams between applications and the API.
*/
typedef void *GameStreamHandle;
/*!
  \brief Codec type definitions
*/

typedef enum {
    CODEC_NONE = 0, //!< None 
    CODEC_H264 = 1, //!< H264 codec
    CODEC_AAC  = 2  //!< AAC(ADTS) codec
    // other enumerators are omitted.
} GameCodecType;

typedef enum {
    OK         = 0, // success.
    UNKNOWN    = -1  // a general error occurred. 
    // other codes are omitted.
} GameErrorCode;

struct av_inject_audio_codec
{
    unsigned char *command;
    aui_deca_stream_type audio_codec_id;
};

struct av_inject_video_codec
{
    unsigned char *command;
    aui_decv_format video_codec_id;
};

int test_av_inject_write_audio_data(void* handle, struct aui_av_packet* header,
	char* buffer, unsigned int len);
	
int test_av_inject_write_video_data(void* handle, struct aui_av_packet* header,
	char* buffer, unsigned int len);

int test_av_inject_get_file_info(char* file, char* suffix, FILE **f_data, 
	FILE **f_pktsize, FILE **f_pts, unsigned int *pktsize_size, 
	unsigned char **extra_data, unsigned int *extra_data_size);

unsigned int test_av_inject_seek2pts(FILE *f_data, FILE *f_pkt, 
	FILE *f_pts, unsigned int seek_pts);

int test_av_injecter(int argc, char **argv);
    
#endif
