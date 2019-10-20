#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <termios.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>
#include <semaphore.h>

#include <aui_common.h>
#include <aui_av.h>
#include <aui_deca.h>
#include <aui_decv.h>
#include <aui_snd.h>
#include <aui_dmx.h>
#include <sys/times.h>

#if 0
#define print_red(fmt, args...) printf("\033[0;31m "fmt"\033[0m", ##args)
#else
#define print_red(...) 
#endif
#if 0
#define print_green(fmt, args...) printf("\033[0;32m "fmt"\033[0m", ##args)
#else
#define print_green(...) 
#endif

typedef struct rec_blk_buf{
    struct aui_list_head  node;
    unsigned long ul_buffer_size;
    void *p_buffer;
} rec_blk_buf;

typedef struct pcm_rec_context {
    FILE *out_fp;
    volatile int loop;
    pthread_t thread_id;
    pthread_mutex_t job_queue_mutex;
    sem_t job_queue_count;
    struct aui_list_head  revieved_buffers;        /**< list head of buffers */
    int cap_cnt;           /* capture count */
    int rec_cnt;           /* record count */ 
}pcm_rec_context;
pcm_rec_context pcm_rec_ctx;

pthread_t th;
pthread_attr_t attr;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *file = NULL;

static volatile int is_paused = 0;
static volatile int is_pcm_cap_quited = 0;
static volatile int is_pcm_rec_quited = 0;
static int cap_quit_flag = 0;
static int rec_quit_flag = 0;
static char *sz_file= "/mnt/usb/sda1/pcmcapd";

static aui_hdl snd_hdl = NULL;
int mygetch ( void )
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
            return 0;
        return c;
    }
    return 0; 
}

static int app_snd_open()
{
    if (aui_find_dev_by_idx(AUI_MODULE_SND,0,&snd_hdl))
    {
        aui_attr_snd attr_snd;
        MEMSET(&attr_snd,0,sizeof(aui_attr_snd));
        snd_hdl = NULL;
        if (aui_snd_open(&attr_snd,&snd_hdl)) {
            printf("%s -> snd open fail\n", __func__);
            return -1;
        }
    }
    return AUI_RTN_SUCCESS;
}


static long pcm_block_in_queue(aui_snd_i2s_output_capture_buffer *p_buf, unsigned long cnt)
{
    /* Lock the mutex on the job queue. */
    pthread_mutex_lock (&pcm_rec_ctx.job_queue_mutex);
    int i = 0;
    for (i=0;i<cnt;i++) {
        rec_blk_buf *tmp_node = (rec_blk_buf *)malloc(sizeof(rec_blk_buf));
        if (!tmp_node) {
            printf("%s -> malloc node fail\n", __func__);
            while(1);
        }
        tmp_node->ul_buffer_size = p_buf[i].ul_buffer_length;
        tmp_node->p_buffer = malloc(tmp_node->ul_buffer_size);
        if (!tmp_node->p_buffer) {
            printf("%s -> malloc buf %lu fail\n", __func__, tmp_node->ul_buffer_size);
            while(1);
        }
        memcpy(tmp_node->p_buffer, p_buf[i].pv_buffer_data, tmp_node->ul_buffer_size);
        /* add to recieved buffer queue */
        aui_list_add_tail(&tmp_node->node, &pcm_rec_ctx.revieved_buffers);
        pcm_rec_ctx.cap_cnt++;
        print_red("rec: %d, cap: %d \n", pcm_rec_ctx.rec_cnt, pcm_rec_ctx.cap_cnt );
    }
    pthread_mutex_unlock (&pcm_rec_ctx.job_queue_mutex);
    /* Post to the semaphore to indicate that another job is available. If
    threads are blocked, waiting on the semaphore, one will become
    unblocked so it can process the job. */
    sem_post (&pcm_rec_ctx.job_queue_count);
    return 0;
}

static void* pcm_rec_loop(void* arg)
{  
    FILE* out_fp = pcm_rec_ctx.out_fp;
    pthread_detach(pthread_self());
    
    if (!out_fp) {
        AUI_PRINTF("%s,%d: out_fp is null. \n", __FUNCTION__, __LINE__);
        goto EXIT;
    }
    pcm_rec_ctx.loop = 1;
    while(pcm_rec_ctx.loop && (0==rec_quit_flag)) { 
        rec_blk_buf *buffer, *tmp; 
        buffer = tmp = NULL;
        /* wait for record buffer */
        sem_wait(&pcm_rec_ctx.job_queue_count);
        /* Lock the mutex on the job queue. */
        pthread_mutex_lock (&pcm_rec_ctx.job_queue_mutex);
        aui_list_for_each_entry_safe(buffer, tmp, &pcm_rec_ctx.revieved_buffers, node) {
            pthread_mutex_unlock (&pcm_rec_ctx.job_queue_mutex);
            /* write record data into a file*/
            fwrite(buffer->p_buffer, 1, buffer->ul_buffer_size, out_fp);
            pthread_mutex_lock (&pcm_rec_ctx.job_queue_mutex);
            aui_list_del(&buffer->node);
            free(buffer->p_buffer);
            free(buffer); 
            pcm_rec_ctx.rec_cnt++;
            print_green("rec: %d, cap: %d \n", pcm_rec_ctx.rec_cnt, pcm_rec_ctx.cap_cnt );
            buffer = NULL;
        }
        pthread_mutex_unlock (&pcm_rec_ctx.job_queue_mutex);
    };

EXIT:
    AUI_PRINTF("%d %d frames recorded\n", pcm_rec_ctx.cap_cnt ,pcm_rec_ctx.rec_cnt );
    
    printf("%s -> it is time to quit\n", __FUNCTION__);
    is_pcm_rec_quited = 1;
    printf("%s -> quit successfully\n", __FUNCTION__);
    return NULL;
}


static void* pcm_cap_loop(void* arg)
{
    aui_snd_i2s_output_capture_buffer *p_buf = NULL;
    unsigned long cnt = 0;

    pthread_detach(pthread_self());
    if (aui_snd_i2s_output_capture_start(snd_hdl, NULL)) {
        printf("%s -> capture start fail\n", __func__);
        return NULL;
    }
    while (0 == cap_quit_flag) {
        p_buf = NULL;
        cnt = 0;
        if (aui_snd_i2s_output_capture_buffer_get(snd_hdl, &p_buf, &cnt)) {
            printf("%s -> capture_buffer_get fail\n", __func__);
            break;
        } else {
            pcm_block_in_queue(p_buf, cnt);
            if (aui_snd_i2s_output_capture_buffer_release(snd_hdl, cnt)) {
                printf("%s -> release capture_buffer fail\n", __func__);
                break;
            }
        }
    }
    if (aui_snd_i2s_output_capture_stop(snd_hdl))
        printf("%s -> capture stop fail\n", __func__);
    if (aui_snd_close(snd_hdl)) 
        printf("%s -> close snd fail\n", __func__);
    printf("%s -> it is time to quit\n", __FUNCTION__);
    is_pcm_cap_quited = 1;
    printf("%s -> quit successfully\n", __FUNCTION__);
    return NULL;
}

int handle_user_input()
{
    while(1) {
        char ch = mygetch();
        if (ch == 'q') {
            cap_quit_flag = 1;
            rec_quit_flag = 1;
            while ((0 == is_pcm_cap_quited) ||(0 == is_pcm_rec_quited)) {
                sem_post(&pcm_rec_ctx.job_queue_count);
                printf("%s -> wait cap: %d, rec:%d to quit first\n", __func__, is_pcm_cap_quited, is_pcm_rec_quited);
                usleep(200);
            }
            fsync(fileno(file));
            fclose(file);
            sem_destroy(&pcm_rec_ctx.job_queue_count);
            pthread_mutex_destroy(&pcm_rec_ctx.job_queue_mutex);
            return 0;
        } 
    }
}

int do_pcm_capture()
{
    int ret = 0;

    if (app_snd_open()) {
        return -1;
    }
    
    /* open input file */
    file = fopen(sz_file, "wb");
    if (file == NULL) {
        printf("Could not open file %s!\n", sz_file);
        return -1;
    }
    
    memset(&pcm_rec_ctx, 0, sizeof(pcm_rec_ctx));
    pcm_rec_ctx.out_fp = file;
    AUI_INIT_LIST_HEAD(&pcm_rec_ctx.revieved_buffers);
    sem_init(&pcm_rec_ctx.job_queue_count, 0, 0);
    pthread_mutex_init(&pcm_rec_ctx.job_queue_mutex, NULL); // = PTHREAD_MUTEX_INITIALIZER;

    ret = pthread_create(&th, NULL, pcm_cap_loop, NULL);
    if (ret) {
        printf("pthread_create cap error\n");
        aui_snd_close(snd_hdl);
        fclose(file);
        return -1;
    }
    
    ret = pthread_create(&th, NULL, pcm_rec_loop, NULL);
    if (ret) {
        printf("pthread_create rec error\n");
        aui_snd_close(snd_hdl);
        fclose(file);
        return -1;
    }
    handle_user_input();
    return 0;    
}

void print_help(char *sz_appname)
{
    printf("Uasage %s [f:h]\n", sz_appname);
    printf("\nCommand line options\n\n");
    printf("\t-f --file <filename storing pcm data>\n");
    printf("\t-h --help\n");
    printf("\n app control\n\n");
    printf("\t[q] - Exit application\n");
}

int main(int argc, char **argv)
{
    int c;
    int option_index = 0;
    char *short_options = "f:h";
    struct option long_options[] = {
        {"file",     required_argument, 0,  'f'},
        {"help",    no_argument, 0,  'h'},
        {0,         0,                 0,  0 }
    };

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        switch (c) {
            case 'f':
                sz_file = optarg;
                break;
            case 'h':
                print_help(argv[0]);
                return 0;
            default:
                print_help(argv[0]);
                return -1;
        }
    }

    printf("filepath=%s \n", sz_file);
    int res = do_pcm_capture();

    return res;
}
