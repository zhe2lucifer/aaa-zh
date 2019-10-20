
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
#include <semaphore.h>
#include <sys/epoll.h>
#include <sys/wait.h>

#include <getopt.h>
#include <termios.h>

#include <aui_dmx.h>
#include <aui_tsg.h>
#include <aui_tsi.h>
#include <aui_dis.h>
#include <aui_av.h>
#include <aui_dsc.h>

#define FILE_NAME "/mnt/usb/sda1/sample_rec.ts"

int rb_video_pid = 513;
int rb_audio_pid = 660;
int rb_time = 8;
char *rb_file = FILE_NAME;

#define BLOCK_SIZE (48128) //47*1024

struct list_head {
	struct list_head *next, *prev;
};

typedef struct {
	struct list_head  node;
	unsigned long ul_buffer_size;
	void * p_buffer;
} record_buffer;

typedef struct {
	struct rb_thread_ctx* context;
	/*
	  channel config, channel handle, etc.
	  different channel may have different recieving buffer size
	*/
	int record_buffer_size;
	/* recieving data buffer for this channel */
	record_buffer* record_buffer;
} channel_buffer;

typedef struct rb_thread_ctx {
	FILE *out_fp;
	volatile int loop;
	/* arrays for saving handles on dmx record*/
	aui_hdl hdl_channel;
	aui_hdl hdl_filter;
	channel_buffer revieving_buffers;
    volatile int loop_end;
	pthread_t thread_id;
	pthread_mutex_t job_queue_mutex;
	sem_t job_queue_count;
	struct list_head  revieved_buffers;        /**< list head of buffers */
	int record_data_size;           /* bytes received */
	int write_data_size;           /* bytes  written */
	int start_ok;
    /* start test data to debug memory leak issue */
    int malloc_cnt;
    int request_cnt;
    int update_cnt;
    int free_cnt;
    struct list_head malloc_buffers;
    /* end test data to debug memory leak issue */
} rb_thread_ctx;

rb_thread_ctx ctx;
rb_thread_ctx *context = &ctx;
unsigned long ul_pid_cnt = 2;
unsigned short pids[2] = {513,660};



#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}
static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}
static inline void __list_add(struct list_head *new,
							  struct list_head *prev,
							  struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

static long rb_request_cb(void *p_user_hdl, unsigned long ul_req_size, void ** pp_req_buf,
				  unsigned long *req_buf_size, struct aui_avsync_ctrl *pst_ctrl_blk)
{
    aui_attr_dmx_filter *filter_attr = NULL;
    channel_buffer *ch_buf = NULL;
    unsigned long ul_size = 0;
    rb_thread_ctx *context = NULL;
    record_buffer *buffer = NULL;

	aui_dmx_filter_get(p_user_hdl, AUI_DMX_FILTER_ATTR_GET, &filter_attr);
	ch_buf = (channel_buffer *)(filter_attr->usr_data);
	context = ch_buf->context;
	ul_size = ch_buf->record_buffer_size + sizeof(record_buffer);
	/* Lock the mutex on the job queue. */

	
	pthread_mutex_lock (&context->job_queue_mutex);
	/* Todo: allocate buffer from memory pool */
	//buffer = (record_buffer*) malloc(ul_size);
	
	buffer = (record_buffer*)malloc(ul_size);
    if(NULL == buffer)
    {
        AUI_PRINTF("%s:%d -> memory is not enough\n", __FUNCTION__, __LINE__);
	    pthread_mutex_unlock (&context->job_queue_mutex);
        return -1;
    }
    context->malloc_cnt++;
	buffer->ul_buffer_size = ch_buf->record_buffer_size;
	/* the rest of the memory is data buffer*/
	buffer->p_buffer = (void*)&buffer[1];

	ch_buf->record_buffer = buffer;

	*pp_req_buf = buffer->p_buffer;
	*req_buf_size = buffer->ul_buffer_size;
    context->request_cnt++;
    list_add_tail((struct list_head*)ch_buf->record_buffer, &context->malloc_buffers);
	pthread_mutex_unlock (&context->job_queue_mutex);

	printf("%s -> req_size: %lu, buf:%p, got_size: %lu\n", __func__, ul_req_size, *pp_req_buf, *req_buf_size);
	/* don't use the request size from the dmx, we can optimize the buffer
	management by ourself*/
	(void)ul_req_size;
	(void)pst_ctrl_blk;
	return 0;
}

static long rb_update_cb(void *p_user_hdl, unsigned long ul_size)
{
    aui_attr_dmx_filter *filter_attr = NULL;
	channel_buffer* ch_buf = NULL;
	rb_thread_ctx*  context = NULL;

    aui_dmx_filter_get(p_user_hdl, AUI_DMX_FILTER_ATTR_GET, &filter_attr);
	ch_buf = (channel_buffer *)(filter_attr->usr_data);
    context = ch_buf->context;
	printf("%s -> upd_size: %lu\n", __func__, ul_size);
    if(0 == context->start_ok)
    {
        AUI_PRINTF("%s: -> filter is not ok now\n", __FUNCTION__);
        return -1;
    }
	/* Lock the mutex on the job queue. */

	pthread_mutex_lock (&context->job_queue_mutex);

	/* update the real buffer size */
	ch_buf->record_buffer->ul_buffer_size = ul_size;
    list_del((struct list_head*)ch_buf->record_buffer);
	/* add to recieved buffer queue */
	list_add_tail((struct list_head*)ch_buf->record_buffer, &context->revieved_buffers);

	// AUI_PRINTF("%p buffer of chn %p commited\n", ch_buf->record_buffer, ch_buf);

	/*clean reveiving channel buffer address in the channel context*/
	ch_buf->record_buffer = NULL;
	context->record_data_size += ul_size;
    context->update_cnt++;
	pthread_mutex_unlock (&context->job_queue_mutex);
	/* Post to the semaphore to indicate that another job is available. If
	threads are blocked, waiting on the semaphore, one will become
	unblocked so it can process the job. */
	sem_post (&context->job_queue_count);
	return 0;
}

static void* rb_thread_loop(void* arg)
{
	record_buffer *buffer, *tmp;
	rb_thread_ctx  *context = (rb_thread_ctx*)arg;
    (void)buffer;
    
    FILE* out_fp = context->out_fp;

    if (!out_fp) {
        AUI_PRINTF("%s,%d: out_fp is null. \n", __FUNCTION__, __LINE__);
        goto EXIT;
    }

    buffer = tmp = NULL;

    context->loop = 1;
	while(context->loop) {
        /* wait for record buffer */
		sem_wait(&context->job_queue_count);
		/* Lock the mutex on the job queue. */
		pthread_mutex_lock (&context->job_queue_mutex);

		list_for_each_entry_safe(buffer, tmp, &context->revieved_buffers, node) {
			/* write record data into a file*/
			fwrite(buffer->p_buffer, 1, buffer->ul_buffer_size, out_fp);

			context->write_data_size += buffer->ul_buffer_size;
			list_del((struct list_head*)buffer);
			free(buffer); //free(buffer);
			context->free_cnt++;
            buffer = NULL;
			//AUI_PRINTF("%p buffer written\n", buffer);
		}
        #ifdef AUI_LINUX
		pthread_mutex_unlock (&context->job_queue_mutex);
        #else
        osal_semaphore_release(context->job_queue_mutex);
        #endif
	};

EXIT:
	AUI_PRINTF("%d %d K bytes recorded\n", context->record_data_size / 1024,
		   context->write_data_size / 1024);         /* bytes received */
    //add this to avoid memory leak risk test
    int leak  = 0;
    buffer = tmp = NULL;
    list_for_each_entry_safe(buffer, tmp, &context->revieved_buffers, node) {
        AUI_PRINTF("%s -> memory leak in revieved buffers\n", __FUNCTION__);
        leak = 1;
		list_del((struct list_head*)buffer);
		free(buffer); //free(buffer);
		context->free_cnt++;
        buffer = NULL;
	}
    buffer = tmp = NULL;
    list_for_each_entry_safe(buffer, tmp, &context->malloc_buffers, node) {
        AUI_PRINTF("%s -> memory leak in malloc_buffers\n", __FUNCTION__);
        leak = 1;
		list_del((struct list_head*)buffer);
		free(buffer); //free(buffer);
		context->free_cnt++;
        buffer = NULL;
	}
    AUI_PRINTF("%s -> memory leak: %d, malloc: %d, free: %d, request:%d, update: %d \n",
        __FUNCTION__, leak, context->malloc_cnt, context->free_cnt, context->request_cnt, context->update_cnt);
    context->loop_end = 1;
	return NULL;
}

static int rb_thread_start(rb_thread_ctx* context)
{
	/* Initialize the semaphore which counts jobs in the queue. Its
	initial value should be zero. */

	sem_init(&context->job_queue_count, 0, 0);
	pthread_mutex_init(&context->job_queue_mutex, NULL); // = PTHREAD_MUTEX_INITIALIZER;
	  
	if(pthread_create(&context->thread_id, NULL, rb_thread_loop, (void*)context))
	{
        AUI_PRINTF("%s->Create thread fail\n", __FUNCTION__);
        sem_destroy(&context->job_queue_count);
        pthread_mutex_destroy(&context->job_queue_mutex);
        return -1;
	}

    context->start_ok = 1;
	return 0;
}
	
static int start_rec_block(aui_dmx_dsc_id * p_dmx_dsc_id)
{
	aui_hdl hdl;
	aui_attr_dmx attr_dmx;
	aui_dmx_stream_pid pid_list;

	memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
	memset(&pid_list, 0, sizeof(aui_dmx_stream_pid));

	attr_dmx.uc_dev_idx = 0;
	if(aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl)) {
		if (aui_dmx_open(&attr_dmx, &hdl)) {
			AUI_PRINTF("dmx open failed\n");
			return -1;
		}
        AUI_PRINTF("aui_dmx_open success\n");
	}

	if (aui_dmx_start(hdl, &attr_dmx)) {
		AUI_PRINTF("aui_dmx_start failed\n");
		return -1;
	}
	aui_dmx_data_path path;
    MEMSET(&path, 0, sizeof(aui_dmx_data_path));
	path.data_path_type = AUI_DMX_DATA_PATH_EN_REC; //AUI_DMX_DATA_PATH_EN_REC
	path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_ID;//AUI_DMX_DATA_PATH_DSC_TYPE_ID;
	path.p_dsc_id = p_dmx_dsc_id;

	if (aui_dmx_data_path_set(hdl, &path)) {
		AUI_PRINTF("\r\n aui_dmx_data_path_set failed\n");
		return -1;
	}
	AUI_PRINTF("step 1: dmx data path set %d\n", path.data_path_type);

	unsigned int i;
	//  the first element is the size of the array
	
    memset(&ctx, 0, sizeof(rb_thread_ctx));
    INIT_LIST_HEAD(&(context->revieved_buffers));
	
    context->revieving_buffers.record_buffer = NULL;
	context->revieving_buffers.record_buffer_size = 47*1024;
	context->revieving_buffers.context = context;
    INIT_LIST_HEAD(&(context->malloc_buffers));
	
	context->record_data_size = 0;
	context->write_data_size = 0;
    
    
	context->out_fp = fopen(rb_file, "wb");
	if (!context->out_fp) {
		AUI_PRINTF("opening file failed - %s\n", strerror(errno));
		return -1;
	}
	/* start the channels */
	aui_attr_dmx_channel attr_channel;
	aui_attr_dmx_filter attr_filter;

	memset(&attr_channel, 0, sizeof(attr_channel));
	memset(&attr_filter, 0, sizeof(attr_filter));

	//Capture the channel are bind with PID(value is 0)
	if (ul_pid_cnt > 0) {
		attr_channel.us_pid = AUI_INVALID_PID;
		attr_channel.ul_pid_cnt = ul_pid_cnt;
		for (i = 0; i < ul_pid_cnt; i ++) {
			attr_channel.us_pid_list[i] = pids[i];
		}
	}

	attr_channel.dmx_data_type = AUI_DMX_DATA_REC;

	//Open the dmx channel device
	if(aui_dmx_channel_open(hdl, &attr_channel, &context->hdl_channel)) {
        AUI_PRINTF("raw channel allocated fail\n");
        goto EXIT0;
	}
    AUI_PRINTF("step 2: %s -> channel open ok\n", __FUNCTION__);

	// everything is done, go
	//Start the dmx channel device , it should be the last step
	if(aui_dmx_channel_start(context->hdl_channel, &attr_channel)) {
        AUI_PRINTF("raw channel start fail\n");
        goto EXIT1;
	}
    AUI_PRINTF("step 3: %s -> channel start ok\n", __FUNCTION__);
	//Open the dmx filter device
	attr_filter.usr_data = &context->revieving_buffers;
    //#ifdef AUI_LINUX
	attr_filter.p_fun_data_req_wtCB = rb_request_cb;
	attr_filter.p_fun_data_up_wtCB = rb_update_cb;
    //#endif
	if(aui_dmx_filter_open(context->hdl_channel, &attr_filter, &context->hdl_filter)) {
        AUI_PRINTF("raw filter allocated fail\n");
        goto EXIT2;
	}
    AUI_PRINTF("step 4: %s -> filter open ok\n", __FUNCTION__);
	//Start the dmx filter device,
	if(aui_dmx_filter_start(context->hdl_filter, &attr_filter)) {
        AUI_PRINTF("raw filter open fail\n");
        goto EXIT3;
	}
    AUI_PRINTF("step 5: %s -> filter start ok\n", __FUNCTION__);
    #if 1
    if(rb_thread_start(context) < 0) {
        AUI_PRINTF("rb_thread_start failed\n");
        goto EXIT4;
	}
    #endif
	return 0;
    EXIT4:
         AUI_PRINTF("%s -> exit4 to stop filter\n", __FUNCTION__);
         aui_dmx_filter_stop(context->hdl_filter, NULL);         
    EXIT3:
         AUI_PRINTF("%s -> exit3 to close filter\n", __FUNCTION__);
         aui_dmx_filter_close(&context->hdl_filter);
    EXIT2:
         AUI_PRINTF("%s -> exit2 to stop channel\n", __FUNCTION__);
         aui_dmx_channel_stop(context->hdl_channel, NULL);
    EXIT1:
         AUI_PRINTF("%s -> exit1 to close channel\n", __FUNCTION__);
         aui_dmx_channel_close(&context->hdl_channel);
    EXIT0:
         if(context->out_fp)
         {
            fclose(context->out_fp);
            context->out_fp = NULL;
         }
    return -1;
}

static void rb_thread_stop(rb_thread_ctx* context)
{
    if(!context->out_fp)
    {
        AUI_PRINTF("warning: record_thread_start maybe fail\n");
        return;
    }
	// set the exit flag
	context->loop = 0;
	// unblock the thread

	sem_post(&context->job_queue_count);
    AUI_PRINTF("sem_post success\n");
	pthread_join(context->thread_id, NULL);
    sem_destroy(&context->job_queue_count);
    pthread_mutex_destroy(&context->job_queue_mutex);
    AUI_PRINTF("pthread_join success\n");

    context->thread_id = 0;

    if (context->out_fp) {
		fflush(context->out_fp);
		fsync(fileno(context->out_fp));
		AUI_PRINTF("fsync on context->out_fp\n");
		fclose(context->out_fp);
		context->out_fp = NULL;
		AUI_PRINTF("%s,%d close\n", __func__, __LINE__);
    }
}

static int end_rec_block()
{
	aui_hdl hdl;
	context->start_ok = 0;
	if (aui_find_dev_by_idx(AUI_MODULE_DMX, 0, &hdl) == 0) {
		if (context->hdl_channel != NULL) {
			//Close the dmx filter device
			aui_dmx_filter_close(&context->hdl_filter);
            AUI_PRINTF("aui_dmx_filter_close success\n");
			//Close the dmx channel device
			aui_dmx_channel_stop(context->hdl_channel, NULL);
            AUI_PRINTF("aui_dmx_channel_stop success\n");
			aui_dmx_channel_close(&context->hdl_channel);
            AUI_PRINTF("aui_dmx_channel_close success\n");
			context->hdl_channel = NULL;
			context->hdl_filter  = NULL;
		}
	}

	rb_thread_stop(context);
	return 0;
}

void do_rec(aui_dmx_dsc_id *p_dmx_dsc_id) {
	if(!p_dmx_dsc_id) {
		printf("%s@%d null pointer\n", __func__, __LINE__);
	}
	start_rec_block(p_dmx_dsc_id);
	sleep(rb_time);
	end_rec_block();
	printf("%s@%d process exit\n", __func__, __LINE__);
}
void print_help(char *sz_appname)
{
	printf("Uasage %s [m:h]\n", sz_appname);
	printf("\nCommand line options\n\n");
	printf("\t-m --magic_number <magic_number>\n");
	printf("\t[q] - Exit application\n");


}

static int char2num(char c){
	if((c >='0') && (c<='9')) {
		return (c-'0');
	} else if((c >='a') && (c<='f')) {
		return 10 + (c-'a');
	} else if((c >='A') && (c<='F')) {
		return 10 + (c-'A');
	} 
	return 0;
}
static void str2dmx_dsc_id(char *id, aui_dmx_dsc_id *p_dmx_dsc_id) {
	unsigned int i = 0;
	printf("%s -> strlen: %d\n", __func__, strlen(id));
	for(i=0;i<strlen(id);) {
		p_dmx_dsc_id->identifier[i/2] = char2num(id[i])*16 + char2num(id[i+1]); 
		printf("%02x", p_dmx_dsc_id->identifier[i/2]);
		i +=2;
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	int c;
	int option_index = 0;

	char *short_options = "m:d:p:s:b:V:v:A:a:f:t:x:h";

	struct option long_options[] = {
	    {"magic_num",     required_argument, 0,  'm'},
		{"dmx id",     required_argument, 0,  'd'},
		{"data_path_type",     required_argument, 0,  'p'},
		{"dsc_process_mode",     required_argument, 0,  's'},
		{"block size",     required_argument, 0,  'b'},
		{"video type",     required_argument, 0,  'V'},
		{"video pid",     required_argument, 0,  'v'},
		{"audio type",     required_argument, 0,  'A'},
		{"audio pid",     required_argument, 0,  'a'},
		{"file path",     required_argument, 0,  'f'},
		{"rec time length",     required_argument, 0,  't'},
		{"magic_string",     required_argument, 0,  'x'},
	    {"help",    no_argument, 0,  'h'},
	    {0,         0,                 0,  0 }
	};

	aui_dmx_dsc_id dmx_dsc_id;
	memset(&dmx_dsc_id, 0, sizeof(dmx_dsc_id));

	while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {

		switch (c) {
			case 'm':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'd':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'p':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 's':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'b':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				break;
			case 'v':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				rb_video_pid= strtoul(optarg, 0, 10);
				break;
			case 'a':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				rb_audio_pid= strtoul(optarg, 0, 10);
				break;
			case 'f':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				rb_file= optarg;
				break;
			case 't':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				rb_time = strtoul(optarg, 0, 10);
				break;
			case 'x':
				printf("%s -> %s\n", __FUNCTION__, optarg);
				str2dmx_dsc_id(optarg, &dmx_dsc_id);
				break;
			case 'h':
				print_help(argv[0]);
				return 0;
			default:
				print_help(argv[0]);
				return -1;
		}
	}
	do_rec(&dmx_dsc_id);
	return 0;
}


