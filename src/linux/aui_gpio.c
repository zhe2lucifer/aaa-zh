/**@file
 *    @brief     ALi AUI GPIO function implementation
 *    @author    Vedic.Fu
 *    @date      2015-05-06
 *
 *    @modified Steven.Zhang
 *    @date      2016-07-22 
 *    @ for add gpio irq support with driver m36_gpio.c
 *    
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/version.h>

#include <aui_common.h>
#include "aui_common_priv.h"
#include <aui_gpio.h>

#include <alislevent.h>
#include <sys/epoll.h>

AUI_MODULE(GPIO)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
#define CALL_SL_GPIO            1
#include <alislgpio.h>
#endif

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

typedef struct aui_gpio_handle {
    struct aui_st_dev_priv_data data;	/* struct for dev reg/unreg */
    aui_gpio_interrupt_attr irq_gpio_attr;	//added by steven
    struct aui_gpio_attr gpio_attr;
    aui_gpio_interrupt_type last_irq_type;
    int open_cnt;
    unsigned long last_tick;
    struct alislevent sl_gpio_cb_event;
    alisl_handle gpio_event_handle;
} aui_gpio_handle;

static pthread_mutex_t m_gpio_mutex;

unsigned int get_tick_ms()
{
    struct timeval t_start; 
    gettimeofday(&t_start, NULL); 
    return ((long)t_start.tv_sec) * 1000 + (long)t_start.tv_usec / 1000; 
}

int check_interval_timeout(unsigned long last_tick, unsigned long debounce_interval)
{
    return get_tick_ms() - last_tick > debounce_interval ? 1 : 0;
}

static int gpio_export(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) {
        AUI_ERR("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof (buf), "%d", gpio);
    if(-1 == write(fd, buf, len)){
		AUI_ERR("failed to run system call\n");
	}
    close(fd);

    return 0;
}

static int gpio_unexport(unsigned int gpio)
{
    int fd, len;
    char buf[MAX_BUF];

    fd = open(SYSFS_GPIO_DIR "/unexport", O_WRONLY);
    if (fd < 0) {
        AUI_ERR("gpio/export");
        return fd;
    }

    len = snprintf(buf, sizeof (buf), "%d", gpio);
    if(-1 == write(fd, buf, len)){
		AUI_ERR("failed to run system call\n");
	}
    close(fd);
    return 0;
}

static int gpio_set_dir(unsigned int gpio, unsigned int out_flag)
{
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/direction", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        AUI_ERR("gpio/direction");
        return fd;
    }

    if (out_flag) {
        if(-1 == write(fd, "out", 4)) {
			AUI_ERR("failed to run system call\n");
		}
	} else {
        if(-1 == write(fd, "in", 3)) {
			AUI_ERR("failed to run system call\n");
		}
	}

    close(fd);
    return 0;
}

static int gpio_set_value(unsigned int gpio, unsigned int value)
{
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        AUI_ERR("gpio/set-value");
        return fd;
    }

    if (value) {
       if( -1 == write(fd, "1", 2)){
			AUI_ERR("failed to run system call\n");
	   }
	} else {
        if(-1 == write(fd, "0", 2)) {
			AUI_ERR("failed to run system call\n");
		}
	}

    close(fd);
    return 0;
}

static int gpio_get_value(unsigned int gpio, unsigned int *value)
{
    int fd;
    char buf[MAX_BUF];
    char ch;

    snprintf(buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY);
    if (fd < 0) {
        AUI_ERR("gpio/get-value");
        return fd;
    }

    if(0  >= read(fd, &ch, 1)) {
		AUI_ERR("failed to run system call\n");
	}

    if (ch != '0') {
        *value = 1;
    } else {
        *value = 0;
    }

    close(fd);
    return 0;
}

static int gpio_set_edge(unsigned int gpio, char *edge)
{
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio);

    fd = open(buf, O_WRONLY);
    if (fd < 0) {
        AUI_ERR("gpio/set-edge");
        return fd;
    }

    if( -1 == write(fd, edge, strlen(edge) + 1)) {
		AUI_ERR("failed to run system call\n");
	}
    close(fd);
    return 0;
}

static int gpio_fd_open(unsigned int gpio)
{
    int fd;
    char buf[MAX_BUF];

    snprintf(buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio);

    fd = open(buf, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        AUI_ERR("gpio/fd_open");
    }
    return fd;
}

static int gpio_fd_close(int fd)
{
    return close(fd);
}

/* Check if gpio exists in /sys/class/gpio/ */
static int aui_gpio_exist(int gpio, int *exist_flag)
{
    DIR *d;
    struct dirent *dir;
    char str[10];

    *exist_flag = 0;
    snprintf(str, sizeof (str), "gpio%d", gpio);
    d = opendir("/sys/class/gpio");

    if (!d) {
        AUI_ERR(">>> opendir fail\n");
        return AUI_RTN_FAIL;
    }

    while ((dir = readdir(d))) {
        if (strcmp(dir->d_name, str) == 0) {
            AUI_ERR(">> gpio = %d exist............\n", gpio);
            *exist_flag = 1;
            break;
        }
    }

    closedir(d);
    return AUI_RTN_SUCCESS;
}

static void *gpio_poll_cb(void *param)
{	
    struct aui_gpio_handle *gpio_handle = (struct aui_gpio_handle *)param;
    aui_gpio_interrupt_type int_t_edge;
    char buf[MAX_BUF];
    int gpio_index;
    int fd;

    if (gpio_handle->sl_gpio_cb_event.events & EPOLLPRI) {
        lseek(gpio_handle->sl_gpio_cb_event.fd, 0, SEEK_SET);
        memset(buf, 0, MAX_BUF);
        if (read(gpio_handle->sl_gpio_cb_event.fd, buf, MAX_BUF) > 0) {
            if (NULL == gpio_handle->irq_gpio_attr.p_callback) {
                AUI_ERR("hdl is NULL or handle->irq_gpio_attr.p_callback is NULL!\n");
                return NULL;
            }

            gpio_index = gpio_handle->gpio_attr.uc_dev_idx;

            //we don't care the status read from event's fd, just read it from /sys/class/gpioXX/edge
            //to get the edge's value we have set.
            memset(buf, 0x0, MAX_BUF);
            snprintf(buf, sizeof (buf), "/sys/class/gpio/gpio%d/edge", gpio_index);

            fd = open(buf, O_RDONLY);
            if (fd < 0) {
                AUI_ERR("gpio/get-edge");
                return NULL;
            }

            memset(buf, 0, MAX_BUF);
            if(0 >= read(fd, buf, 1)) {
				AUI_ERR("failed to run system call\n");
			}
            close(fd);

            if (buf[0] == 'f') {//falling
                int_t_edge= AUI_GPIO_INTERRUPT_FALLING_EDGE;
                AUI_INFO("falling\n");
            } else if (buf[0] == 'r') {//rasing
                int_t_edge = AUI_GPIO_INTERRUPT_RISING_EDGE;
                AUI_INFO("rising\n");
            } else {
                int_t_edge = AUI_GPIO_INTERRUPT_EDGE;
                AUI_INFO("both\n");
            }

			//if user press the button so quickly, and we should ignore it.
            if (check_interval_timeout(gpio_handle->last_tick, (unsigned long)gpio_handle->irq_gpio_attr.debounce_interval)){
                gpio_handle->irq_gpio_attr.p_callback(gpio_index, int_t_edge, gpio_handle->irq_gpio_attr.pv_user_data);
                gpio_handle->last_tick = get_tick_ms();
            } else {
                AUI_INFO("It in the interval,don't call user function!\n");
            }
        }
    }

    return NULL;
}

/**
 *  Function Name:      	aui_gpio_interrupt_reg
 *  @brief             	cal this function to register user's callback function that be called when event happended.
 *
 * @param[in]     		p_hdl:			gpio irq device pointer
 * @param[in]     		p_interrupt_attr:	gpio irq attribute's pointer
 *
 *  @return             	alisl_retcode
 *
 *  @author             	Steven Zhang <steven.zhang@alitech.com>
 *  @date               	2016.07.14, Created
 *
 *  @note
 */

AUI_RTN_CODE aui_gpio_interrupt_reg(aui_hdl p_hdl, aui_gpio_interrupt_attr *p_interrupt_attr)
{
    struct aui_gpio_handle *gpio_handle = p_hdl;
	char buf[MAX_BUF];
    aui_gpio_interrupt_type irq_type;

    if ((NULL == p_hdl)
            || (NULL == p_interrupt_attr)
            || (NULL == p_interrupt_attr->p_callback)) {
        aui_rtn(AUI_RTN_EINVAL, "NULL attribute!\n");
    }

    //irq_type
    irq_type = p_interrupt_attr->interrupt_type;
    //interval_timeout
    if(p_interrupt_attr->debounce_interval < 0)
        gpio_handle->irq_gpio_attr.debounce_interval = 0;
    else if (p_interrupt_attr->debounce_interval == 0)
        gpio_handle->irq_gpio_attr.debounce_interval = 200;
    else
        gpio_handle->irq_gpio_attr.debounce_interval = p_interrupt_attr->debounce_interval;

    if (AUI_GPIO_INTERRUPT_DISABLED > irq_type || AUI_GPIO_INTERRUPT_MAX <= irq_type) {
        AUI_ERR("invalid irq type\n");
        return AUI_RTN_EINVAL;
    }
    if (AUI_GPIO_INTERRUPT_DISABLED != gpio_handle->irq_gpio_attr.interrupt_type) {
        if (alislevent_del(gpio_handle->gpio_event_handle, &gpio_handle->sl_gpio_cb_event)) {
            AUI_ERR("alislevent_del fail\n");
            return AUI_RTN_EINVAL;
        }
        gpio_fd_close(gpio_handle->sl_gpio_cb_event.fd);
        gpio_handle->sl_gpio_cb_event.fd = -1;
    }
    switch (irq_type) {
        case AUI_GPIO_INTERRUPT_DISABLED:
            gpio_set_edge(gpio_handle->gpio_attr.uc_dev_idx, "none");
            gpio_handle->irq_gpio_attr.interrupt_type = irq_type;
            return AUI_RTN_SUCCESS;

        case AUI_GPIO_INTERRUPT_RISING_EDGE:	//Interrupt on a 0->1 transition.
            gpio_set_edge(gpio_handle->gpio_attr.uc_dev_idx, "rising");
            break;
        case AUI_GPIO_INTERRUPT_FALLING_EDGE:	//Interrupt on a 1->0 transition.
            gpio_set_edge(gpio_handle->gpio_attr.uc_dev_idx, "falling");
            break;
        case AUI_GPIO_INTERRUPT_EDGE:	//Interrupt on both a 0->1 and a 1->0 transition.
            gpio_set_edge(gpio_handle->gpio_attr.uc_dev_idx, "both");
            break;
        default:
            break;
    }

    gpio_handle->irq_gpio_attr.p_callback = p_interrupt_attr->p_callback;
    gpio_handle->irq_gpio_attr.pv_user_data = p_interrupt_attr->pv_user_data;
    gpio_handle->irq_gpio_attr.interrupt_type = irq_type;

    gpio_handle->sl_gpio_cb_event.cb = gpio_poll_cb;
    gpio_handle->sl_gpio_cb_event.data = (void *)gpio_handle;
    gpio_handle->sl_gpio_cb_event.events = EPOLLPRI;//there are some data can be read.
    gpio_handle->sl_gpio_cb_event.fd = gpio_fd_open(gpio_handle->gpio_attr.uc_dev_idx);

	//must read data from enevnt fd before event add, otherwise, the event return a evnet immediately.
	if (0 >= read(gpio_handle->sl_gpio_cb_event.fd, buf, MAX_BUF)) {
		AUI_ERR("failed to run system call\n");
		//do nothing
	}
    if (alislevent_add(gpio_handle->gpio_event_handle, &gpio_handle->sl_gpio_cb_event)) {
        AUI_ERR("alislevent_add fail\n");
        return AUI_RTN_FAIL;
    }
	//init last tick
	gpio_handle->last_tick = get_tick_ms();

    return AUI_RTN_SUCCESS;
}

/**
 *  Function Name:      	aui_gpio_interrupt_unreg
 *  @brief             	unregister user's callback function
 *
 * @param[in]     		p_hdl:		gpio irq device pointer
 *
 *  @return             	alisl_retcode
 *
 *  @author             	Steven Zhang <steven.zhang@alitech.com>
 *  @date               	2016.07.14, Created
 *
 *  @note
 */

AUI_RTN_CODE aui_gpio_interrupt_unreg(aui_hdl p_hdl)
{
    if (NULL == p_hdl) {
        aui_rtn(AUI_RTN_EINVAL, "NULL attribute!\n");
    }

    struct aui_gpio_handle *gpio_handle = p_hdl;

    if (AUI_GPIO_INTERRUPT_DISABLED != gpio_handle->irq_gpio_attr.interrupt_type) {
        if (alislevent_del(gpio_handle->gpio_event_handle, &gpio_handle->sl_gpio_cb_event)) {
            AUI_ERR("alislevent_del fail\n");
            return AUI_RTN_EINVAL;
        }
        gpio_fd_close(gpio_handle->sl_gpio_cb_event.fd);
        gpio_handle->sl_gpio_cb_event.fd = -1;
        gpio_set_edge(gpio_handle->gpio_attr.uc_dev_idx, "none");
        gpio_handle->irq_gpio_attr.interrupt_type = AUI_GPIO_INTERRUPT_DISABLED;
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gpio_set_value(aui_hdl handle, aui_gpio_value value)
{
    struct aui_gpio_handle *gpio_dev = (struct aui_gpio_handle *)handle;

    if (NULL == handle)
        return AUI_RTN_FAIL;

    pthread_mutex_lock(&m_gpio_mutex);
    if (gpio_set_value(gpio_dev->gpio_attr.uc_dev_idx, value)) {
        AUI_ERR("aui_gpio_set fail!\n");
        pthread_mutex_unlock(&m_gpio_mutex);
        return AUI_RTN_FAIL;
    }
    pthread_mutex_unlock(&m_gpio_mutex);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gpio_get_value(aui_hdl handle, aui_gpio_value * value)
{
    struct aui_gpio_handle *gpio_dev = (struct aui_gpio_handle *)handle;

    if (NULL == handle)
        return AUI_RTN_FAIL;

    pthread_mutex_lock(&m_gpio_mutex);
    if (gpio_get_value(gpio_dev->gpio_attr.uc_dev_idx, (unsigned int *)value)) {
        AUI_ERR("aui_gpio_get fail\n");
        pthread_mutex_unlock(&m_gpio_mutex);
        return AUI_RTN_FAIL;
    }
    pthread_mutex_unlock(&m_gpio_mutex);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gpio_open(aui_gpio_attr * attr, aui_hdl * handle)
{
    aui_hdl hdl;
    char buf[MAX_BUF];
    int ret = AUI_RTN_SUCCESS;
    int exist_flag = 0;
    struct aui_gpio_handle *g = NULL;
    //void *m_gpio_sl_dev = NULL;

    if (NULL == handle)
        return AUI_RTN_FAIL;

    if (attr->uc_dev_idx == AUI_GPIO_NONE) {
        AUI_ERR("invalid !\n");
        return AUI_RTN_FAIL;
    }

    AUI_INFO("valid GPIO_index = %d\n", attr->uc_dev_idx);

    pthread_mutex_lock(&m_gpio_mutex);

    /* check gpio if already opened */
    if (AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_GPIO, attr->uc_dev_idx, &hdl)) {
        *handle = hdl;
        g = (struct aui_gpio_handle *)hdl;
        g->open_cnt++;
        ret = AUI_RTN_SUCCESS;
        goto EXIT;
    }

    /* check gpio if already exist, aui_find_dev_by_idx() can not find but it exist it means
       something error like app crash before close gpio, in this case, just close and open again */

    if (AUI_RTN_SUCCESS != aui_gpio_exist(attr->uc_dev_idx, &exist_flag)) {
        AUI_ERR(">>>> aui_gpio_exist() fail\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    if (exist_flag) {
        if (gpio_unexport(attr->uc_dev_idx)) {
            AUI_ERR("access /sys/class/gpio/unexport fail! \n");
            ret = AUI_RTN_FAIL;
            goto EXIT;
        }
    }

    if (gpio_export(attr->uc_dev_idx)) {
        AUI_ERR("access /sys/class/gpio/export fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    if (gpio_set_dir(attr->uc_dev_idx, attr->io)) {
        AUI_ERR(">>>> can not open /sys/class/gpio/gpio%d/direction\n", attr->uc_dev_idx);
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    //if set gpio's direction is out, then set value first.
    snprintf(buf, sizeof (buf), "/sys/class/gpio/gpio%d/value", attr->uc_dev_idx);
    if (attr->io) {
        if (gpio_set_value(attr->uc_dev_idx, attr->value_out)) {
            AUI_ERR(">>>> can not write /sys/class/gpio/gpio%d/value\n", attr->uc_dev_idx);
            ret = AUI_RTN_FAIL;
            goto EXIT;
        }
    }

    g = malloc(sizeof (*g));
    if (!g) {
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    memset(g, 0x0, sizeof (*g));
    g->gpio_attr.uc_dev_idx = attr->uc_dev_idx;
    g->data.dev_idx = attr->uc_dev_idx;
    g->gpio_attr.io = attr->io;
    g->gpio_attr.value_out = attr->value_out;

    if (alislevent_open(&g->gpio_event_handle)) {
        AUI_ERR("alislevent_open fail\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    // set gpio interrupt initial state
    g->irq_gpio_attr.interrupt_type = AUI_GPIO_INTERRUPT_DISABLED;
    g->irq_gpio_attr.debounce_interval = 200;
    g->last_irq_type = AUI_GPIO_INTERRUPT_DISABLED;
    g->sl_gpio_cb_event.fd = -1;

    if (aui_dev_reg(AUI_MODULE_GPIO, g)) {
        AUI_ERR("aui_dev_reg AUI_MODULE_GPIO fail\n");
        free(g);
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    *handle = (aui_hdl) g;
    g->open_cnt++;

EXIT:
    pthread_mutex_unlock(&m_gpio_mutex);
    return ret;
}

AUI_RTN_CODE aui_gpio_close(aui_hdl handle)
{
    struct aui_gpio_handle *gpio_dev = (struct aui_gpio_handle *)handle;
    int ret = AUI_RTN_SUCCESS;

    if (NULL == gpio_dev)
        return AUI_RTN_FAIL;

    pthread_mutex_lock(&m_gpio_mutex);

    if (--gpio_dev->open_cnt != 0) {
        ret = AUI_RTN_SUCCESS;
        goto EXIT;
    }

    if (gpio_set_edge(gpio_dev->gpio_attr.uc_dev_idx, "none")) {
        AUI_ERR("set interrupt edge fail\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    if (alislevent_close(gpio_dev->gpio_event_handle)) {
        AUI_ERR("write /sys/class/gpio/unexport fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    if (-1 != gpio_dev->sl_gpio_cb_event.fd) {
        gpio_fd_close(gpio_dev->sl_gpio_cb_event.fd);
        gpio_dev->sl_gpio_cb_event.fd = -1;
    }

    if (gpio_unexport(gpio_dev->gpio_attr.uc_dev_idx)) {
        AUI_ERR("write /sys/class/gpio/unexport fail!\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    if (aui_dev_unreg(AUI_MODULE_GPIO, gpio_dev)) {
        AUI_ERR("aui_dev_unreg AUI_MODULE_GPIO fail\n");
        ret = AUI_RTN_FAIL;
        goto EXIT;
    }

    gpio_dev->data.dev_idx = -1;

    free(gpio_dev);
    gpio_dev = NULL;
EXIT:
    pthread_mutex_unlock(&m_gpio_mutex);
    return ret;
}

AUI_RTN_CODE aui_gpio_init(p_fun_cb p_call_back_init, void *pv_param)
{
    if (0 != pthread_mutex_init(&m_gpio_mutex, NULL)) {
        AUI_ERR("pthread_mutex_init fail!");
        return AUI_RTN_FAIL;
    }

    if (p_call_back_init && pv_param)
        return p_call_back_init(pv_param);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_gpio_deinit()
{
    if (0 != pthread_mutex_destroy(&m_gpio_mutex))
        return AUI_RTN_FAIL;

    return AUI_RTN_SUCCESS;
}
