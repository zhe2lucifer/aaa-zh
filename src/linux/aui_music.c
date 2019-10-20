/*
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file                  aui_music.c
 *  @brief                Ali AUI Music
 *
 *  @version            1.0
 *  @date                02/25/2014 14:33:44 PM
 *  @revision           none
 *
 *  @author             Alan Zhang <Alan.Zhang@alitech.com>
 */

/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"

#include <aui_music.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <signal.h>
#include "alidef.h"
#include <pthread.h>
#include <time.h>
#include <sys/wait.h>

AUI_MODULE(MUSIC)


#ifdef WEXITSTATUS
#undef WEXITSTATUS
#define WEXITSTATUS(x) (((x) >> 8) & 0xff)
#endif

#define CONDITION_TIMEDWAIT(condition, cmutex, timeout) \
do {\
    struct timespec endtime; \
    endtime.tv_sec= time(NULL) + timeout; \
    endtime.tv_nsec= 0;  \
    pthread_cond_timedwait(condition, cmutex, &endtime); \
} while(0)

#define SIGNAL_CONDITION(signal, cmutex) \
do {\
    pthread_mutex_lock(cmutex); \
    pthread_cond_signal(signal); \
    pthread_mutex_unlock(cmutex); \
} while(0)

// a is the process name, b is an argument, c is the full path of the execute file
#define launch_player_server(a,b,c) \
do { \
    char *argv[3]={NULL, NULL, NULL};\
    char param[2][64]={{0},{0}};\
    argv[0] = param[0];\
    strncpy(argv[0], a, 63);\
    if(b!=NULL){\
        argv[1] = param[1];\
        strncpy(argv[1], b, 63);\
    }\
    AUI_DBG("this is the child process, pid is %d %d!!\n", getpid(), getppid());\
    prctl(PR_SET_PDEATHSIG, SIGKILL); \
    if (-1 == execv(c, argv)) {\
        AUI_DBG("execv error: %d\n", errno);\
    }\
}while(0)

#define errno  3
#ifndef MANUAL_PAUSE
#define MANUAL_PAUSE     (1 << 31)
#endif

#ifdef INVALID_VALUE
#undef  INVALID_VALUE
#define INVALID_VALUE (-1)
#else
#define INVALID_VALUE (-1)
#endif

#ifdef TRUE
#undef  TRUE
#define TRUE (1)
#else
#define TRUE (1)
#endif

#ifdef FALSE
#undef  FALSE
#define FALSE (0)
#else
#define FALSE (0)
#endif

#define SEND_MSG_TIMEOUT (2000*1000) //2s
#define WAIT_MSG_STEP    (50*1000)   //50ms

#define GOT_METADATA 0x1
#define GOT_TOTALTIME 0x2
#define GOT_SEEKABLE 0x4
#define GOT_CURRENTTIME 0x8
#define GOT_STREAMINFO 0x10
#define GOT_CURSTREAMINFO 0x20
#define GOT_STREAMINFO_F 0x40
#define GOT_CURSTREAMINFO_F 0x80
#define GOT_VIDEOINFO 0x100
#define GOT_DOWNLOAD_SPEED 0x200 
#define GOT_METADATA_F 0x400 
#define GOT_MEDIASIZE 0x800
static int get_flag = 0;

typedef struct 
{
    /** audio codec in container */
    char audio_dec[10]; 
    /** audio strem number in container */
    unsigned long audio_stream_num; 
    /** subtitle number in container */
    unsigned long sub_stream_num; 
    /** total frames in container */
    unsigned long total_frame_num; 
    /** frame period */
    unsigned long frame_period; 
    /** total time of the container */
    unsigned long total_time; 
    /** bitrate of the audio in the container */
    unsigned long audio_bitrate;
    /** audio channel number */
    unsigned long audio_channel_num;
    /** is it seekable */
    unsigned int b_seekable;
    /** filesize */
    unsigned long fsize;
}aui_mp_stream_info;


typedef enum _spdif_mode{
    SPDIF_DISABLE,  // turn off spidf output
    SPDIF_NORMAL,   // 7.1 channels source output
    SPDIF_DEGRADE,  // 5.1channels degrade output
    SPDIF_INVALID
}SPDIF_MODE;

enum _playerstate {
	PLAYSTATE_STOPPED = 0,
    PLAYSTATE_PLAYING,
	PLAYSTATE_PAUSED,
	PLAYSTATE_ERROR,
	PLAYSTATE_BUFFERING = 0x40,
	PLAYSTATE_INVALID = 0xff
};

typedef enum _sourcetype {
    LOCAL_MOVIE = 0,
    LOCAL_MUSIC,
    ONLINE,
    DONGLE,
    INVALID_TYPE = 0xff,
}SOURCE_TYPE_T;

typedef struct _playinginfo {
    int cur_state;
    unsigned int cur_time;
    unsigned int download_speed;
    unsigned int buffering_ratio;
    unsigned int download_ratio;
    unsigned int download_pos;
}PLAYING_INFO_T;

typedef struct _meta{
    char *key;
    char *data;
}META;

typedef struct _metadatainfo {
    int metadata_num;       //total metadata, when metadata_num==INVALID_VALUE means we are receiving metadata
    int metadata_index;     //we have already received metadata_index+1 metadata
    META *meta_data[128];
}METADATA_INFO_T;

typedef struct _pthreadpara {
    pthread_t msgrcv_threadID;
    pthread_t monitor_server_threadID;
    pthread_mutex_t mutex_total_time;
    pthread_cond_t cond_total_time;
    pthread_mutex_t mutex_seekable;
    pthread_cond_t cond_seekable; 
    pthread_mutex_t mutex_videoinfo;
    pthread_cond_t cond_videoinfo;
    pthread_mutex_t mutex_streaminfo;
    pthread_cond_t cond_streaminfo;
}PTHREAD_PARA_T;

typedef struct _msgqpara {
    int msg_qid_send;
    int msg_qid_recv;
    int vmsg_qid_recv;
    stMSG_QUE msg_info_snd;
    stMSG_QUE msg_info_get;
}MSGQ_PARA_T;

typedef struct _serverinfo {
    //int
    int server_pid;//forked pid for player server
    int src_type;//to judge which player server be run
    //char
    const char *p_exec_name;//player server name
    const char *p_exec_fpath;//player server path
    const char *p_exec_arg;//player server argv[1]
    //boolean
    int b_server_initialized;//set true after init().
    int b_player_server_opened;//set true after get first msg from server.
	int b_is_player_finished;//set true can break all threads we create.
    int b_is_src_set;//play() api has been called.
    int b_manual_pause;
    int spdif_output_mode; 
}SERVER_INFO_T;

typedef void (*MP_CALLBACK)(int cb_type, const void *data, int len, void * pobj);

typedef struct _AUI_MP_MODULE
{
    aui_attr_music attr_music;
    
    ///*file adress*/
    char *uri;
    /*restore to the time position last played*/
    unsigned int start_pos;

    SERVER_INFO_T s_server_info;/*the info of the player_server_streaming*/
    MSGQ_PARA_T s_msgq_para;/*the msg para struct*/
    PTHREAD_PARA_T s_pthread_para;/*mutex or signal struct*/
    PLAYING_INFO_T s_playing_info;/*the info of the playing*/
    METADATA_INFO_T s_metadata_info;/*the metadata info of the stream such as id3 info*/
    aui_mp_stream_info s_stream_info;/*the info of the stream*/
}AUI_MP_MODULE;

/****************************LOCAL VAR********************************************/
static AUI_MP_MODULE aui_mp;
static PTHREAD_PARA_T *p = &aui_mp.s_pthread_para;
static MSGQ_PARA_T *msgq_para = &aui_mp.s_msgq_para;
static SERVER_INFO_T *server_info = &aui_mp.s_server_info;
static PLAYING_INFO_T *playing_info = &aui_mp.s_playing_info;
static METADATA_INFO_T *metadata_info = &aui_mp.s_metadata_info;
static aui_mp_stream_info *stream_info = NULL;
static fn_music_end_callback  g_msg_callback[] = {0};

static inline AUI_RTN_CODE send_message_with_timeout(int qid, void *msg_snd, 
    int size, int time_val)
{
    int retry_counter, send_ret = 0;

    for(retry_counter=0;retry_counter<(time_val/WAIT_MSG_STEP);retry_counter++) {
        send_ret = msgsnd(qid, msg_snd, size, IPC_NOWAIT);
        if (send_ret == -1) {
            usleep(WAIT_MSG_STEP);
        } else {
            break;
        }
    }
    if (send_ret == -1) {
        AUI_ERR("msg snd timeout: %s\n", strerror(errno));
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

static inline AUI_RTN_CODE delivery_message(long type, long sub_type, int val, 
    const char *s_msg_data1, const char *s_msg_data2)
{
    MEMSET(&msgq_para->msg_info_snd, 0, sizeof(stMSG_QUE));
    msgq_para->msg_info_snd.msgtype = type;
    msgq_para->msg_info_snd.subtype = sub_type;
    msgq_para->msg_info_snd.val = val;
    
    if (s_msg_data1 && strlen(s_msg_data1) < MAX_MSG_DATA_SIZE) {
        strncpy(msgq_para->msg_info_snd.data1, s_msg_data1, sizeof(msgq_para->msg_info_snd.data1)-1);
    }

    if (s_msg_data2 && strlen(s_msg_data2) < MAX_MSG_DATA_SIZE) {
        strncpy(msgq_para->msg_info_snd.data2, s_msg_data2, sizeof(msgq_para->msg_info_snd.data2)-1);
    }

    if (sub_type == eCMD_TYPE_DVIEW) {
        AUI_DBG("do preview, param is : %s\n", msgq_para->msg_info_snd.data1);  
    }
    if (AUI_RTN_SUCCESS != send_message_with_timeout(msgq_para->msg_qid_send, 
        &msgq_para->msg_info_snd, 
        MSG_SIZE, SEND_MSG_TIMEOUT)) {
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

/*
    delivery a virtual message to player server
*/
static inline AUI_RTN_CODE delivery_vmsg_message(long type, long sub_type, int val, 
    const char *s_msg_data, int msg_data_len)
{
    int msg_length = 0;
    stVMSG_QUE *vmsg_snd = NULL;

	msgq_para->msg_info_snd.msgtype = type;
    msgq_para->msg_info_snd.subtype = eCMD_TYPE_VMSG;
    msg_length = msg_data_len + sizeof(stVMSG_QUE) + 1;
    msgq_para->msg_info_snd.val = msg_length;
    if (AUI_RTN_SUCCESS != send_message_with_timeout(msgq_para->msg_qid_send, 
        &msgq_para->msg_info_snd, 
        MSG_SIZE, SEND_MSG_TIMEOUT)) {
        return AUI_RTN_FAIL;
    }

    vmsg_snd = (stVMSG_QUE *)calloc(sizeof(char), msg_length);//length
    if (!vmsg_snd) {
    	return AUI_RTN_FAIL;
    }
    vmsg_snd->msgtype = type;
    vmsg_snd->subtype = sub_type;
    vmsg_snd->val = val;
    vmsg_snd->len = msg_data_len + 1;
    strncpy(vmsg_snd->data, s_msg_data, msg_data_len);
    if (AUI_RTN_SUCCESS != send_message_with_timeout(msgq_para->msg_qid_send, 
        vmsg_snd, (msg_length - sizeof(long)), SEND_MSG_TIMEOUT)) {
        free(vmsg_snd);
        return AUI_RTN_FAIL;
    }
    free(vmsg_snd);

    return AUI_RTN_SUCCESS;
}

static void drain_msgqueue(int qid)
{
    stMSG_QUE *pmsgq_recv = NULL;
    void *vmsg_recv = NULL;
    size_t msg_length = 0;

    pmsgq_recv = (stMSG_QUE *)malloc(sizeof(stMSG_QUE));
    if (!pmsgq_recv) {
    	return;
    }
    for(;;) {
        if (msgrcv(qid, pmsgq_recv, MSG_SIZE, 0, IPC_NOWAIT) == -1) {
            break;
        } else {
            if (eCMD_TYPE_VMSG == pmsgq_recv->subtype) {
    			msg_length = pmsgq_recv->val;
    			vmsg_recv = malloc(msg_length);//length
    			msg_length = (msg_length - sizeof(long));
                msgrcv(qid, vmsg_recv, msg_length, 0, 0);
                free(vmsg_recv);
            }
        }
    }
    
    free(pmsgq_recv);
}

static inline AUI_RTN_CODE start_player_server(int type)
{
    int pid = 0;
    
    if (-1 == system("killall player_server_streaming")) {
        AUI_ERR("failed to get msg queue\n");
	}

    AUI_DBG("start player server type: %d\n", type);

    server_info->p_exec_name = NULL;
    server_info->p_exec_fpath = NULL;
    server_info->p_exec_arg = NULL;
    switch (type) {     
        case LOCAL_MOVIE:
            server_info->p_exec_name = "player_server_streaming";
            server_info->p_exec_fpath = "/sf-gst-player/player_server_streaming";
            server_info->p_exec_arg = "Movie";
            break;        
        case LOCAL_MUSIC:
            server_info->p_exec_name = "player_server_streaming";
            server_info->p_exec_fpath = "/sf-gst-player/player_server_streaming";
            server_info->p_exec_arg = "Music";
            break;
        case ONLINE:
            server_info->p_exec_name = "player_server_streaming";
            server_info->p_exec_fpath = "/sf-gst-player/player_server_streaming";
            server_info->p_exec_arg = "online";
            break;
        case DONGLE:
        	server_info->p_exec_name = "player_server_streaming";
            server_info->p_exec_fpath = "/sf-gst-player/player_server_streaming";
            server_info->p_exec_arg = "dongle";
            break;
        default:
            server_info->p_exec_name = "player_server_streaming";
            server_info->p_exec_fpath = "/sf-gst-player/player_server_streaming";
            server_info->p_exec_arg = "auto";            
            break; 
    }
    AUI_DBG("START CHILD1 : %s %s %s .... \n", server_info->p_exec_name, 
        server_info->p_exec_arg, server_info->p_exec_fpath);
    
    if ((pid = vfork()) == -1) {
       AUI_ERR("%s Error in vfork %d\n", __func__, errno);
       return AUI_RTN_FAIL;
    }
    if (0 == pid) { //child process 
        launch_player_server(server_info->p_exec_name, 
            server_info->p_exec_arg, server_info->p_exec_fpath);
        return AUI_RTN_SUCCESS;
    } else {
        server_info->server_pid = pid;
    }
    
    return AUI_RTN_SUCCESS;
}

static inline void aui_mp_res_destroy(void)
{ 
    AUI_DBG("remove message queue to avoid block\n");
    if(msgctl(msgq_para->msg_qid_send, IPC_RMID, NULL) == -1) {
        AUI_ERR("remove message send queue failed\n");
    }
    if(msgctl(msgq_para->msg_qid_recv, IPC_RMID, NULL) == -1) {
        AUI_ERR("remove message receive queue failed\n");
    }  
    if(msgctl(msgq_para->vmsg_qid_recv, IPC_RMID, NULL) == -1) {
        AUI_ERR("remove message receive queue failed\n");
    } 

    
    pthread_mutex_destroy(&p->mutex_total_time);
    pthread_cond_destroy(&p->cond_total_time); 
    pthread_mutex_destroy(&p->mutex_seekable);
    pthread_cond_destroy(&p->cond_seekable);
    pthread_mutex_destroy(&p->mutex_videoinfo);
    pthread_cond_destroy(&p->cond_videoinfo);
    pthread_mutex_destroy(&p->mutex_streaminfo);
    pthread_cond_destroy(&p->cond_streaminfo);  
}

//static AUI_RTN_CODE get_value_by_key(const char *src, const char *key)
//{
//    AUI_RTN_CODE ret = AUI_RTN_FAIL;
//    char *start = NULL, *end = NULL;
//    char fbuf[2048];
//
//    MEMSET(fbuf, 0, sizeof(fbuf));
//    strcpy(fbuf, src);
//
//    start = strstr(fbuf, key);
//    if(start != NULL) {
//        end = index(start, ',');
//        if(end != NULL)
//            *end = '\0';
//
//        start = index(start, ':');
//        if(start != NULL)
//            ret = atoi(start+1);
//    }
//
//    return ret;
//}


static void *monitor_child_crashed_thread(void *pointer)
{
    int cpid, status;
    int pid = 0;
    (void)pointer;
    #ifdef _RD_DEBUG_
    int exit_s;
    #endif
    do {
        AUI_DBG("monitor process wait pid %d.....\n", server_info->server_pid);
        cpid = -1;
        //cpid = wait(&status);
        cpid = waitpid(server_info->server_pid, &status, 0);
        #ifdef _RD_DEBUG_
        exit_s = WEXITSTATUS(status);
        AUI_DBG("child exit -- pid = %d, status = %d !\n", cpid, exit_s);       
        #endif
        if (server_info->b_is_player_finished) {
            break;
        }
        if (cpid == server_info->server_pid) {
            server_info->b_player_server_opened = FALSE;
            drain_msgqueue(aui_mp.s_msgq_para.msg_qid_recv);
            if ((pid = vfork()) == -1) {
               AUI_ERR("%s Error in vfork %d\n", __func__, errno);
               break;
            }
            if (pid == 0) {
                launch_player_server(server_info->p_exec_name, 
                    server_info->p_exec_arg, server_info->p_exec_fpath);
                return NULL;
            } else {
                int retry_counter = 0;
                server_info->server_pid = pid;
                while(!server_info->b_player_server_opened) { //2s
                    retry_counter++;
                    if(retry_counter > 40)
                        break;
                    else
                        usleep(50*1000);
                }
                AUI_DBG("send error message to UI .... \n");    
                playing_info->cur_state = PLAYSTATE_ERROR;
                #if 0
                if (aui_mp.mplayer_callback) {               
                    aui_mp.mplayer_callback(CB_ERROR, 
                        "player server crashed!!!!", 
                        strlen("player server crashed!!!!"), 
                        aui_mp.callback_pobj);            
                }
                #endif
            }           
        } 
        usleep(200*1000);
    } while(!server_info->b_is_player_finished);
    AUI_DBG("monitor process exit.....\n");
    
    pthread_exit(NULL);
    return NULL;
}

static inline void reset_player_info(void)
{ 

	server_info->p_exec_name = NULL;//player server name
    server_info->p_exec_fpath = NULL;//player server path
    server_info->p_exec_arg = NULL;//player server argv[1]
    
    MEMSET(playing_info, 0, sizeof(PLAYING_INFO_T));
    MEMSET(metadata_info->meta_data, 0, sizeof(metadata_info->meta_data));
    MEMSET(metadata_info, INVALID_VALUE, sizeof(METADATA_INFO_T));
    //aui_mp.mplayer_callback = NULL;
    MEMSET(&aui_mp, 0, sizeof(AUI_MP_MODULE));

}

static void pthread_mutex_and_cond_init()
{
    pthread_mutex_init(&p->mutex_total_time, NULL);
    pthread_cond_init(&p->cond_total_time, NULL);
    pthread_mutex_init(&p->mutex_seekable, NULL);
    pthread_cond_init(&p->cond_seekable, NULL);
    pthread_mutex_init(&p->mutex_videoinfo, NULL);
    pthread_cond_init(&p->cond_videoinfo, NULL);
    pthread_mutex_init(&p->mutex_streaminfo, NULL);
    pthread_cond_init(&p->cond_streaminfo, NULL);
}

static void stream_info_parse(stVMSG_QUE *vmsg, aui_music_ext_item_get type, int isCurrent)
{
	char *pch = NULL;
	const char *delim = ";";
	char *outer_ptr = NULL;
	char *subpch = NULL;
	const char *subdelim = ",";
	unsigned int count = 0;
	int cur_init = 0;
    unsigned int info_count = 0;

	AUI_DBG ("stream_info_parse: %s\n", (char*)vmsg);
	pch = strtok_r(vmsg->data, delim, &outer_ptr);

	AUI_DBG("\npch = %s\n", pch);
	while (pch != NULL) {
		AUI_DBG ("StreamInfo pch of line: %s\n", pch);
		if ((strstr(pch, "StreamNum") != NULL) || (isCurrent && !cur_init)) {
			if (isCurrent) {
				info_count = 1;
				cur_init = 1;
			}
			else
				info_count = atoi(strstr(pch, ":")+1);
			if (info_count > 0) {
				switch(type) {
					case AUI_MUSIC_GET_AUDIO:
                        stream_info->audio_stream_num = info_count;
					break;

					case AUI_MUSIC_GET_SUBTITLE:
                        stream_info->sub_stream_num = info_count;
					break;
#if 0
					case AUI_MUSIC_GET_VIDEO:
                        //aui_mp.attr_music.st_stream_info.video_stream_num = info_count;
					break;
#endif
					default:
					break;
				}
			}
			AUI_DBG ("streamNum: %d in %d type\n", info_count, type);
			if (isCurrent)
				continue;
		} else {
			subpch = strtok(pch, subdelim);
			while (subpch != NULL && count < info_count) {
				//AUI_DBG ("StreamInfo subpch of line: %s\n", subpch);
				switch(type)
				{
					case AUI_MUSIC_GET_AUDIO:
					{
						if (strstr(subpch, "decoder") != NULL)
						{
                            strncpy(stream_info->audio_dec, strstr(subpch, ":")+1, 10);
						}
					}
					break;

					case AUI_MUSIC_GET_SUBTITLE:
					{
					}
					break;

					default:
					break;
				}
				subpch = strtok (NULL, subdelim);
			}
			count++; /* next stream */
		}
		pch = strtok_r(NULL, delim, &outer_ptr);
	}
	free(pch);
	if (subpch)
		free(subpch);
}

static int vmsg_proc(stVMSG_QUE *vmsg)
{
	if (!vmsg) {
		return 0;
	}
	AUI_DBG("----vmsg proc type=%d, len=%d, st=%d, name=%s----\n",
	(int)vmsg->subtype, (int)vmsg->len, vmsg->val, vmsg->data);

	switch(vmsg->subtype)
	{
        case AUI_MUSIC_GET_AUDIO:
        case AUI_MUSIC_GET_SUBTITLE:
        //case AUI_MUSIC_GET_VIDEO:
		case AUI_MUSIC_GET_MUSIC_EXT_INFO:
		{
			stream_info_parse(vmsg, vmsg->val, 0);
			get_flag |= GOT_STREAMINFO;
		}
		break;
		
		default:
		break;
	}
	return 0;
}

static void *msgrcv_thread(void *pointer)
{
    int vmsg_flag = 0;
	size_t msg_length = 0;
	void *vmsg_recv = NULL;
	MSGQ_PARA_T *msgq_para = (MSGQ_PARA_T *)pointer;

	while (!server_info->b_is_player_finished) {
		if (vmsg_flag) {
			if (msgrcv(msgq_para->vmsg_qid_recv, vmsg_recv, msg_length, 0, 0) == -1) {
				//perror("msgrcv");
				AUI_DBG("---- get vmsg fail %s %d ----\n", __func__, __LINE__);
				continue;
			} else {
				vmsg_proc((stVMSG_QUE *)vmsg_recv);
				vmsg_flag = 0;
				free(vmsg_recv);
				vmsg_recv = NULL;
			}
		}

		if (msgrcv(msgq_para->msg_qid_recv, (stMSG_QUE *)&msgq_para->msg_info_get, MSG_SIZE, 
            0, 0) == -1) {
            usleep(50*1000);
            continue;
		}
        AUI_DBG("msg type %ld\n",msgq_para->msg_info_get.subtype);
    	if (eMSG_TYPE_RESPONSE == msgq_para->msg_info_get.msgtype) {
            switch (msgq_para->msg_info_get.subtype)
            {
            	case eRESPONSE_TYPE_OPEN:
                    AUI_DBG("player server be opened ......\n");
                    server_info->b_player_server_opened = TRUE;
                    msgq_para->msg_qid_send = msgget(KEY_SERVER, 0644);
                    break;

                case eRESPONSE_TYPE_VMSG:
                {
                    vmsg_flag = 1;
                    msg_length = msgq_para->msg_info_get.val;
                    vmsg_recv = (stVMSG_QUE *)calloc(msg_length, sizeof(stVMSG_QUE));
                    msg_length = (msg_length - sizeof(long));
                    AUI_DBG ("---- vmsg : len=%d ----\n", msg_length);
                    if (vmsg_recv == NULL) {
                        AUI_DBG("---- malloc vmsg error ----\n");
                    }
                    break;
                }
                case eRESPONSE_TYPE_STREAM_INFO_FINISH:
				case eRESPONSE_TYPE_STREAM_INFO:
                {
                    get_flag |= GOT_STREAMINFO_F;
					SIGNAL_CONDITION(&p->cond_streaminfo, &p->mutex_streaminfo);
                    AUI_DBG("GOT_STREAMINFO_F\n");
                    break;
                }   
                case eRESPONSE_TYPE_TOTALTIME:
                    stream_info->total_time = msgq_para->msg_info_get.val;
                    get_flag |= GOT_TOTALTIME;
                    AUI_DBG("get total time: %ld\n", stream_info->total_time);
                    SIGNAL_CONDITION(&p->cond_total_time, &p->mutex_total_time);
                    break;
                    
                case eRESPONSE_TYPE_ISSEEKABLE:
                    stream_info->b_seekable = msgq_para->msg_info_get.val;
                    get_flag |= GOT_SEEKABLE;
                    AUI_DBG("get seekable flag: %d\n", stream_info->b_seekable); 
                    SIGNAL_CONDITION(&p->cond_seekable, &p->mutex_seekable);
                    break;
                    
                case eRESPONSE_TYPE_BUFFERING:
                    AUI_DBG("buffering received: %d\n", msgq_para->msg_info_get.val);
                    playing_info->buffering_ratio = msgq_para->msg_info_get.val;
                    playing_info->cur_state = PLAYSTATE_BUFFERING;
                    if (playing_info->buffering_ratio >= 100)
                        playing_info->buffering_ratio = INVALID_VALUE;
                    break;
                    
                case eRESPONSE_TYPE_CURRENTTIME:
                    //AUI_DBG("--- current time: %u, cur state: %d\n", msg_info_get.val, s_playing_info.cur_state);
                    if (playing_info->cur_state == PLAYSTATE_PLAYING) {
                        playing_info->cur_time = msgq_para->msg_info_get.val;
                        #if 0
                        if (aui_mp.mplayer_callback)                    
                            aui_mp.mplayer_callback(CB_CURRENT_TIME, 
                            &playing_info->cur_time, 
                            sizeof(unsigned int), aui_mp.callback_pobj);
                        #endif
                    }
                    break;
                    
                case eRESPONSE_TYPE_STATE_CHANGE:
                    if (msgq_para->msg_info_get.val < PLAYSTATE_INVALID && 
                        msgq_para->msg_info_get.val > PLAYSTATE_STOPPED) {
                        #ifdef _RD_DEBUG_
                        int state;
                        
                        playing_info->cur_state = msgq_para->msg_info_get.val;
                        state = playing_info->cur_state;
                            if ((playing_info->cur_state == PLAYSTATE_PAUSED) \
                                && (server_info->b_manual_pause == TRUE)) {
                                AUI_ERR("manual pause. Last state: %d\n", state);
                                state = playing_info->cur_state | MANUAL_PAUSE;
                            }
                            //if (playing_info->cur_state == PLAYSTATE_PAUSED)
                                //pe_callback[AUI_MP_PLAY_BEGIN];
                        #endif        
                    } else {
                       AUI_ERR("wrong state !!!");
                    }
                    break;

                case eRESPONSE_TYPE_FINISHED:
                    //pe_callback[AUI_MP_PLAY_END];
                    playing_info->cur_state = PLAYSTATE_STOPPED;
                    break;
                    
                case eRESPONSE_TYPE_GET_MEDIA_SIZE:
                {
                    if(msgq_para->msg_info_get.val)
                    {
                    	stream_info->fsize = strtoll (msgq_para->msg_info_get.data1, NULL, 10);
                    }
                    else
                    {
                    	stream_info->fsize = INVALID_VALUE;
                    }
                    get_flag |= GOT_MEDIASIZE;
					SIGNAL_CONDITION(&p->cond_streaminfo, &p->mutex_streaminfo);
                    AUI_DBG("GOT_MEDIASIZE\n");
                }
                break;
				case eRESPONSE_TYPE_CLOSE:
                    break;

                default:
                    break;
            }
    	} else if (eMSG_TYPE_WARNING == msgq_para->msg_info_get.msgtype) {
            char tmp[32];
            MEMSET(tmp, 0, sizeof(tmp));
            if((msgq_para->msg_info_get.subtype == eWARN_SUB_UNSUPPORT_AUDIO)  
               && (msgq_para->msg_info_get.val > 0)) {
                /* current audio track changed */
                AUI_DBG("warning msg, audio id changed: %d\n", msgq_para->msg_info_get.val);
                //audio_info->audio_index = msgq_para->msg_info_get.val;
                snprintf(tmp, sizeof(tmp), "audio:%d", msgq_para->msg_info_get.val);
                //pe_callback[AUI_MP_AUDIO_CODEC_NOT_SUPPORT];
                
            } else if((msgq_para->msg_info_get.subtype == eWARN_SUB_UNSUPPORT_SUBTITLE)
                       && (msgq_para->msg_info_get.val > 0)) {
                /* current subtitle track changed */ 
                AUI_DBG("warning msg, subtitle id changed: %d\n", msgq_para->msg_info_get.val);
                //subtitle_info->sub_index = msgq_para->msg_info_get.val;
                snprintf(tmp, sizeof(tmp), "subtitle:%d", msgq_para->msg_info_get.val);
                //pe_callback[AUI_MP_DECODE_ERROR];

                /* only these 5 types of warning message will be send. */    
            }  else if(msgq_para->msg_info_get.subtype == eWARN_SUB_UNSUPPORT_AUDIO ||
               msgq_para->msg_info_get.subtype == eWARN_SUB_UNSUPPORT_VIDEO ||
               msgq_para->msg_info_get.subtype == eWARN_SUB_DECODE_ERR_AUDIO ||
               msgq_para->msg_info_get.subtype == eWARN_SUB_DECODE_ERR_VIDEO ||
               msgq_para->msg_info_get.subtype == eWARN_SUB_UNSUPPORT_SUBTITLE) {      
               AUI_DBG("warning msg, unsupport type: %ld\n", msgq_para->msg_info_get.subtype);
               snprintf(tmp, sizeof(tmp), "%ld", msgq_para->msg_info_get.subtype);
                //pe_callback[AUI_MP_DECODE_ERROR];
            } else {
                AUI_ERR("discard unexpected warning sub type: %ld\n", msgq_para->msg_info_get.subtype);    
            }
        }  else if (eMSG_TYPE_ERR == msgq_para->msg_info_get.msgtype) {
            playing_info->cur_state = PLAYSTATE_ERROR;
            int err_msg_len = strlen(msgq_para->msg_info_get.data1);
            
            if (err_msg_len > MAX_MSG_DATA_SIZE)
                err_msg_len = MAX_MSG_DATA_SIZE;
            //pe_callback[AUI_MP_ERROR_UNKNOWN];
        }  else if (eMSG_TYPE_EOS == msgq_para->msg_info_get.msgtype) {
            AUI_DBG("get msg eos from player server \n");
            //pe_callback[AUI_MP_PLAY_END];
            playing_info->cur_state = PLAYSTATE_STOPPED;
        }
	}
    AUI_ERR("exit msgrcv thread\n");
    pthread_exit(NULL);
    return NULL;
}


AUI_RTN_CODE aui_music_open(aui_attr_music *pst_music_attr,void **pp_handle_music)
{
    //AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    int retry_counter = 0;
    
    if(NULL == pst_music_attr)
    {
        aui_rtn(AUI_RTN_EINVAL,"pst_music_attr is null");
    }

    *pp_handle_music = (void *)&aui_mp;

    if (server_info->b_server_initialized) {
        if(strstr(aui_mp.uri, (const char *)pst_music_attr->uc_file_name) != NULL)
            aui_rtn(AUI_RTN_FAIL,"no more player!!!");
        AUI_DBG("server initialized, return success\n");   
        return AUI_RTN_SUCCESS;
    }

	if(NULL == stream_info) {
		stream_info = &aui_mp.s_stream_info;
		MEMSET(&stream_info, 0, sizeof(stream_info));
	}
    MEMSET(&aui_mp.attr_music, 0, sizeof(aui_attr_music));
    server_info->b_manual_pause = TRUE;
    MEMCPY(&aui_mp.attr_music, 
        pst_music_attr, sizeof(aui_attr_music));
    //server_info->src_type = pst_music_attr->src_type;
    aui_mp.uri = (char *)pst_music_attr->uc_file_name;

    if(-1 == system("ipcrm -Q 100225")) {
		AUI_ERR("failed to run system call\n");
	}
	if(-1 == system("ipcrm -Q 100230")) {
		AUI_ERR("failed to run system call\n");
	}

	if ((msgq_para->msg_qid_recv = msgget(KEY_CLIENT, 0644 | IPC_CREAT)) == -1) {
        AUI_ERR("failed to get msg queue\n");
		aui_rtn(AUI_RTN_FAIL,NULL);
	} else {
        drain_msgqueue(msgq_para->msg_qid_recv);
    }

	if ((msgq_para->vmsg_qid_recv = msgget(KEY_V_CLIENT, 0644 | IPC_CREAT)) == -1) {
		//perror("msgget");
		aui_rtn(AUI_RTN_FAIL,NULL);
	} else {
        drain_msgqueue(msgq_para->vmsg_qid_recv);
    }

	AUI_DBG("recv id: %d,%d\n",msgq_para->msg_qid_recv, msgq_para->vmsg_qid_recv);

    if (server_info->src_type < 0 || server_info->src_type >= INVALID_TYPE) {
        AUI_ERR("source type error\n");
        aui_rtn(AUI_RTN_FAIL,NULL);
    }

    pthread_mutex_and_cond_init();

    //server_info->spdif_output_mode = get_settings_from_config(SETTING_CONFIG, "[SPDIF]", "mode");
    //if(server_info->spdif_output_mode == MP_RET_FAILURE) {
    //    server_info->spdif_output_mode = SPDIF_DISABLE;
    //}

	if (pthread_create(&p->msgrcv_threadID, 
        NULL, msgrcv_thread, (void*)msgq_para) != 0) {
        AUI_ERR("create msg receive thread failed\n");
        aui_mp_res_destroy();
		aui_rtn(AUI_RTN_FAIL,NULL);
	}


    if (AUI_RTN_SUCCESS != start_player_server(server_info->src_type)) {
        AUI_ERR("player server process start failed.\n");
        aui_rtn(AUI_RTN_FAIL,NULL);
    }

       
    while (!server_info->b_player_server_opened) {
        retry_counter++;
        if (retry_counter > 200) { //10s
            AUI_ERR("start player server error in timeout ,return failed\n");
            server_info->b_is_player_finished = TRUE;

            if (p->msgrcv_threadID > 0) {
                AUI_DBG("cancel msgrecv thread\n");
                pthread_cancel(p->msgrcv_threadID);
                pthread_join(p->msgrcv_threadID, NULL);
                p->msgrcv_threadID = 0;
            }            
           
            if (server_info->server_pid > 0)
            {
                kill(server_info->server_pid, SIGKILL); //kill player server!!
            }
            	aui_mp_res_destroy();

            aui_rtn(AUI_RTN_FAIL,NULL);            
        } else {
            usleep(50*1000);
        }
    }

	if (pthread_create(&p->monitor_server_threadID,
        NULL, monitor_child_crashed_thread, (void*)0) != 0) {
        AUI_ERR("create msg receive thread failed\n");
		aui_rtn(AUI_RTN_FAIL,NULL);
	}    
	server_info->b_server_initialized = TRUE;
    
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_music_close(aui_attr_music *pst_music_attr,void **pp_handle_music)
{
	(void)pst_music_attr;
	(void)pp_handle_music;
	if (server_info->b_server_initialized) {
        AUI_DBG("finalize the player.\n");
		server_info->b_server_initialized = FALSE;
		server_info->b_is_player_finished = TRUE;

        if (p->msgrcv_threadID > 0) {
            AUI_DBG("cancel msgrecv thread\n");
            pthread_cancel(p->msgrcv_threadID);
            pthread_join(p->msgrcv_threadID, NULL);
            p->msgrcv_threadID = 0;
        }            

        if (server_info->server_pid > 0) {
            kill(server_info->server_pid, SIGKILL); //kill player server!!
            AUI_DBG("kill player server id: %d\n", server_info->server_pid);
        }
        pthread_join(p->monitor_server_threadID, NULL);
        aui_mp_res_destroy();
      
        server_info->b_player_server_opened = FALSE;
		server_info->b_is_src_set = FALSE;
        reset_player_info();
	}

	if(stream_info) {
		stream_info = NULL;
	}
    AUI_DBG("turn off the music server.\n");

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_music_start(void *pv_handle_mp)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    int uri_length = 0;
    char *uri = ((AUI_MP_MODULE *)pv_handle_mp)->uri;
    unsigned int resume_pos = ((AUI_MP_MODULE *)pv_handle_mp)->start_pos;
    AUI_DBG ("uri %s, resume pos %d\n", uri, resume_pos);
	if (!server_info->b_server_initialized) {
        aui_rtn(AUI_RTN_FAIL,"\n mp not opened! \n");
    }
    
	if (!server_info->b_is_src_set && NULL != uri) {
        if((server_info->spdif_output_mode == SPDIF_NORMAL) ||
            (server_info->spdif_output_mode == SPDIF_DEGRADE)) {
            rtn_code = delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_SET_SPDIF, \
                server_info->spdif_output_mode, NULL, NULL);
            if (rtn_code != AUI_RTN_SUCCESS) {
                AUI_ERR("set spdif mode to %d error.\n", server_info->spdif_output_mode);
                return rtn_code;
            }
        }
        uri_length = strlen(uri);
        if (uri_length < MAX_MSG_DATA_SIZE) {
            rtn_code = delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_SET_CURRENTSOURCE, 
                resume_pos, uri, NULL);
        } else {
            rtn_code = delivery_vmsg_message(eMSG_TYPE_CMD, eCMD_TYPE_SET_CURRENTSOURCE, 
                resume_pos, uri, uri_length);
        }
        if (rtn_code != AUI_RTN_SUCCESS) {
            return rtn_code;
        }
		server_info->b_is_src_set = TRUE;
	}
    rtn_code = delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_PLAY, 0, NULL, NULL);
    server_info->b_manual_pause = FALSE;
    
    return rtn_code;
}


AUI_RTN_CODE aui_music_stop(void *pv_handle_mp)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
     
    if (!server_info->b_server_initialized) {
        //AUI_DBG("server not initialized, return immediately.\n");
        aui_rtn(AUI_RTN_FAIL,"\n mp not opened! \n");
    }
    
    AUI_DBG("stop player server.\n");
    /* if the source type is mms, we do not send
     * stop command to player to avoid the canceled
     * error when changeing source quickly. instead of
     * it, the player server will be killed directly 
     * every time.
     */
     /** fix bug 10180,when playback music ,we must send stop to player server **/

    rtn_code = delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_STOP, 0, NULL, NULL);
    AUI_DBG("send stop command to player server\n");

    #if 0
    int index = 0;
    int max = 0;
    max = sizeof(metadata_info->meta_data);
    while((index < max) && (metadata_info->meta_data[index] != NULL) && \
            (metadata_info->meta_data[index] > 0)) {
        if(metadata_info->meta_data[index]->key != NULL && (metadata_info->meta_data[index]->key > 0))
            free(metadata_info->meta_data[index]->key);
        if(metadata_info->meta_data[index]->data != NULL && (metadata_info->meta_data[index]->data > 0))
            free(metadata_info->meta_data[index]->data);
        free(metadata_info->meta_data[index]);
        index++;
    }
    #endif
    server_info->b_is_src_set = FALSE;
    //reset_player_info();
    
    return rtn_code;
}

AUI_RTN_CODE aui_music_pause(void *pv_handle_mp)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    
    if (!server_info->b_server_initialized) {
        aui_rtn(AUI_RTN_FAIL,"\n mp not opened! \n");
    }    
    rtn_code = delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_PAUSE, 0, NULL, NULL);
    server_info->b_manual_pause = TRUE;
    return rtn_code;
}

AUI_RTN_CODE aui_music_resume(void *pv_handle_mp)
{
    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    
    server_info->b_manual_pause = FALSE;
    return delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_PLAY, 0, NULL, NULL);
}

AUI_RTN_CODE aui_music_init(aui_func_music_init fn_mp_init)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL != fn_mp_init)
    {
    	fn_mp_init();
    }

    return rtn_code;
}


AUI_RTN_CODE aui_music_de_init(aui_func_music_init fn_mp_de_init)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL == fn_mp_de_init)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);  
    }
    fn_mp_de_init();
    return rtn_code;
}

AUI_RTN_CODE aui_music_seek(void *pv_handle_mp, unsigned long ul_time_in_ms)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    
    if (!server_info->b_server_initialized)
    {
        aui_rtn(AUI_RTN_FAIL,"\n mp not opened! \n");
    }
    else
    {
        if((get_flag & GOT_SEEKABLE) != GOT_SEEKABLE)
        {
            rtn_code = delivery_message(eMSG_TYPE_CMD, 
                eCMD_TYPE_ISSEEKABLE, 0, NULL, NULL);
            pthread_mutex_lock(&p->mutex_seekable);
            CONDITION_TIMEDWAIT(&p->cond_seekable, &p->mutex_seekable, 1);
            pthread_mutex_unlock(&p->mutex_seekable);
        }

        if(stream_info->b_seekable)
            rtn_code = delivery_message(eMSG_TYPE_CMD, 
                eCMD_TYPE_SEEK, ul_time_in_ms, NULL, NULL);
        else
            rtn_code = AUI_RTN_FAIL;
    }
    return rtn_code;
}


AUI_RTN_CODE aui_music_total_time_get(void *pv_handle_mp, unsigned int *pui_total_time)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    if(NULL == pui_total_time)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    if ((get_flag & GOT_TOTALTIME) != GOT_TOTALTIME) {
        delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_ISSEEKABLE, 0, NULL, NULL);
        pthread_mutex_lock(&p->mutex_seekable);
        CONDITION_TIMEDWAIT(&p->cond_seekable, &p->mutex_seekable, 1);
        pthread_mutex_unlock(&p->mutex_seekable);

        delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_TOTALTIME, 0, NULL, NULL);
        pthread_mutex_lock(&p->mutex_total_time);
        CONDITION_TIMEDWAIT(&p->cond_total_time, &p->mutex_total_time, 1);
        pthread_mutex_unlock(&p->mutex_total_time);
    }
    
    *pui_total_time = stream_info->total_time/1000;
    
    return rtn_code;
}

AUI_RTN_CODE aui_music_cur_time_get(void *pv_handle_mp, unsigned int *pui_cur_time)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    if(NULL == pui_cur_time)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_CURRENTTIME, 0, NULL, NULL); 
    
    *pui_cur_time = playing_info->cur_time/1000;
    return rtn_code;
}

AUI_RTN_CODE aui_music_get(void *pv_handle_mp, unsigned long ul_item, void *pv_param)
{
    //aui_mp_stream_info *param = (aui_mp_stream_info *)pv_param;
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    if(NULL == pv_handle_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    
    if(NULL == pv_param)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    switch(ul_item)
    {
        case AUI_MUSIC_GET_MEDIA_SIZE:
        {
            if ((get_flag & GOT_MEDIASIZE) != GOT_MEDIASIZE)
            {
                delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_GET_MEDIA_SIZE, 0, NULL, NULL);
                pthread_mutex_lock(&p->mutex_streaminfo);
                CONDITION_TIMEDWAIT(&p->cond_streaminfo, &p->mutex_streaminfo, 1);
                pthread_mutex_unlock(&p->mutex_streaminfo);
            }
            if ((get_flag & GOT_MEDIASIZE) == GOT_MEDIASIZE)
            {
            	AUI_DBG("get stream info media type success.\n");
            }
            break;
        }
        
        case AUI_MUSIC_GET_MUSIC_EXT_INFO:
        {
            //if ((get_flag & (GOT_STREAMINFO | GOT_STREAMINFO_F))
            //    != (GOT_STREAMINFO | GOT_STREAMINFO_F))
            {
                delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_STREAM_INFO, 0, NULL, NULL);
                pthread_mutex_lock(&p->mutex_streaminfo);
                CONDITION_TIMEDWAIT(&p->cond_streaminfo, &p->mutex_streaminfo, 2);
                pthread_mutex_unlock(&p->mutex_streaminfo);
            }

            if ((get_flag & (GOT_STREAMINFO | GOT_STREAMINFO_F))
				== (GOT_STREAMINFO | GOT_STREAMINFO_F))
            {
               AUI_DBG("Get stream info success.\n");
            }
            break;
        }

        default:
            break;
    }

	if(stream_info == NULL)
	{
		AUI_DBG("Error: stream info is NULL.\n");
	    aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	pv_param = stream_info;
	AUI_DBG("audio_dec = %s\n"
		"audio_stream_num = %ld   sub_stream_num = %ld\n"
		"total_frame_num = %ld   frame_period = %ld\n"
		"total_time = %ld\n"
		"audio_bitrate = %ld   audio_channel_num = %ld\n"
		"b_seekable = %d      fsize = %ld\n",
		stream_info->audio_dec, stream_info->audio_stream_num,
		stream_info->sub_stream_num, stream_info->total_frame_num,
		stream_info->frame_period, stream_info->total_time,
		stream_info->audio_bitrate,
		stream_info->audio_channel_num, stream_info->b_seekable,
		stream_info->fsize);
    return rtn_code;
}

AUI_RTN_CODE aui_music_set(void *pv_hdl_mp,unsigned long ul_item, void *pv_param)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if(NULL == pv_hdl_mp)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }
    
    (void)pv_param; // for remove warning
    switch(ul_item)
    {
#if 0
        case AUI_MP_SET_AUDIO_ID:
            if(NULL == pv_param)
            {
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            
            (((aui_handle_music *)(pv_hdl_mp))->attr_music).ui_audio_id = *((unsigned int *)pv_param);
            delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_CHANGE_AUDIO, 
                *((unsigned int *)pv_param), NULL,NULL);
            break;

        case AUI_MP_SET_SUBTITLE_ID:
            if(NULL == pv_param)
            {
                aui_rtn(AUI_RTN_EINVAL,NULL);
            }
            
            (((aui_handle_music *)(pv_hdl_mp))->attr_music).ui_subtitle_id = *((unsigned int *)pv_param);
            delivery_message(eMSG_TYPE_CMD, eCMD_TYPE_CHANGE_SUBTITLE, 
                *((unsigned int *)pv_param), NULL,NULL);
            break;
#endif
        default:
            break;
    }
    return rtn_code;
}

AUI_RTN_CODE aui_music_set_playend_callback(void *pv_handle_music, enum aui_music_message msg, fn_music_end_callback func)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    if(NULL == pv_handle_music)
    {
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

	  g_msg_callback[msg] = func;
    return rtn_code;
}
