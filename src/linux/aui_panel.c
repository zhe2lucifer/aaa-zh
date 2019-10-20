#include <pthread.h>
#include <aui_panel.h>
#include <linux/input.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <ali_front_panel_common.h>
#include "aui_common_priv.h"

AUI_MODULE(PANEL)

#ifndef ULONG
#define ULONG unsigned long
#endif
#ifndef uchar
#define uchar unsigned char
#endif
#ifndef DATA_TYPE
#define DATA_TYPE enum aui_panel_data_type
#endif

#define PANEL_DEV_FILE ("/dev/pan_ch455")
#define SEG_NUM 5//default including ':'

// the following commands is copied from hdl pan.h
/* ESC command: 27 (ESC code), PAN_ESC_CMD_xx (CMD type), param1, param2 */
#define PAN_ESC_CMD_LBD         'L'     /* LBD operate command */
#define PAN_ESC_CMD_LBD_FUNCA   0       /* Extend function LBD A */
#define PAN_ESC_CMD_LBD_FUNCB   1       /* Extend function LBD B */
#define PAN_ESC_CMD_LBD_FUNCC   2       /* Extend function LBD C */
#define PAN_ESC_CMD_LBD_FUNCD   3       /* Extend function LBD D */
#define PAN_ESC_CMD_LBD_LEVEL   5       /* Level status LBD, no used */

#define PAN_ESC_CMD_LED         'E'     /* LED operate command */
#define PAN_ESC_CMD_LED_LEVEL   0       /* Level status LED */
#define PAN_ESC_CMD_LBD_ON      1       /* Set LBD to turn on status */
#define PAN_ESC_CMD_LBD_OFF     0       /* Set LBD to turn off status */

typedef struct {
	int           device_fd;
	int           device_id;
	aui_cfg_panel config;
} panel_device;

/* In the input module, after user set ir key map, 
need to set panel key map with this panel key map automatically. */
struct aui_pannel_key_map_cfg g_panel_key_map_cfg;


AUI_RTN_CODE
aui_panel_version_get(ULONG *p_ver) {
	if(NULL == p_ver) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	*p_ver = AUI_MODULE_VERSION_NUM_PANEL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_init(p_fun_cb cb, void *p_param) {
	if (cb != NULL) {
		cb(p_param);
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_de_init(p_fun_cb cb, void *p_param) {
	if (cb != NULL) {
		cb(p_param);
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_open(ULONG panel_id, aui_cfg_panel *p_cfg, aui_hdl *p_handle) {
	panel_device *p_dev = NULL;
	int dev_fd = -1;

	if (panel_id >= MAX_PANEL_ID || NULL == p_handle) {
        AUI_ERR("invalid params.\n");
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	dev_fd = open(PANEL_DEV_FILE, O_RDWR | O_NONBLOCK);
	if (dev_fd < 0) {
        AUI_ERR("open %s fail.\n", PANEL_DEV_FILE);
		aui_rtn(AUI_RTN_FAIL, NULL);
	}

	p_dev = (panel_device *) malloc(sizeof(panel_device));
	if (NULL == p_dev) {
        AUI_ERR("malloc fail.\n");
		close(dev_fd);
		aui_rtn(AUI_RTN_ENOMEM,NULL);
	}

	p_dev->device_id = panel_id;
	p_dev->device_fd = dev_fd;
	if (p_cfg != NULL) {
		p_dev->config = *p_cfg;
	} else {
		p_dev->config.ul_led_cnt = SEG_NUM; // 5 LED by default including ':'
	}

	*p_handle = (aui_hdl) p_dev;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_close(aui_hdl handle) {
    if (NULL == handle ) {
        AUI_ERR("invalid params.\n");
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	panel_device *p_dev = (panel_device *) handle;
	close(p_dev->device_fd);
	free(p_dev);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_display(aui_hdl handle, DATA_TYPE type, uchar *p_buf, ULONG len) {
	if (NULL == handle || (NULL == p_buf && AUI_PANEL_CMD_LED_LOCK != type)) {
        AUI_ERR("invalid params.\n");
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	ULONG wr_len = len;
	char data[4] = { 0 }; // control data length is 4
	panel_device *p_dev = (panel_device *) handle;

	if (AUI_PANEL_DATA_ANSI == type) {
		if (wr_len > p_dev->config.ul_led_cnt) {
			wr_len = p_dev->config.ul_led_cnt;
		}
		if(-1 == write(p_dev->device_fd, p_buf, wr_len)){
			AUI_ERR("failed to run system call\n");
		}
	}

	if (AUI_PANEL_CMD_LED_LOCK == type) {
		data[0] = 27;
		data[1] = PAN_ESC_CMD_LBD;
		data[2] = PAN_ESC_CMD_LBD_FUNCB;
		data[3] = PAN_ESC_CMD_LBD_ON;
		if(-1 == write(p_dev->device_fd, data, sizeof(data))){
			AUI_ERR("failed to run system call\n");
		}
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_clear(aui_hdl handle, DATA_TYPE type) {
	if (NULL == handle) {
        AUI_ERR("invalid params.\n");
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	char data[4] = { 0 };
	panel_device *p_dev = (panel_device *) handle;
	ULONG wr_len = p_dev->config.ul_led_cnt;

	if (wr_len <= 0 || wr_len > sizeof(data)) {
		wr_len = sizeof(data);
	}

	if (AUI_PANEL_DATA_ANSI == type) {
		if(-1 == write(p_dev->device_fd, data, wr_len)){
			AUI_ERR("failed to run system call\n");
		}
	}

	if (AUI_PANEL_CMD_LED_LOCK == type) {
		data[0] = 27;
		data[1] = PAN_ESC_CMD_LBD;
		data[2] = PAN_ESC_CMD_LBD_FUNCB;
		data[3] = PAN_ESC_CMD_LBD_OFF;
		if(-1 == write(p_dev->device_fd, data, sizeof(data))) {
			AUI_ERR("failed to run system call\n");
		}
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_get(aui_hdl handle, aui_panel_item_get item, void* p_param) {
	(void)handle;
	(void)item;
	(void)p_param;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_set(aui_hdl handle, aui_panel_item_set item, void* p_param) {
	(void)handle;
	(void)item;
	(void)p_param;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE
aui_panel_set_led_state(aui_hdl handle, unsigned long ul_led_number,
                        unsigned char uc_led_active)
{
    if (NULL == handle ||
        ((0 != ul_led_number) && (1 != ul_led_number) && (2 != ul_led_number)
         && (3 != ul_led_number)) || ((1 != uc_led_active) && (0 != uc_led_active))) {
        AUI_ERR("invalid params.\n");
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    ULONG wr_len = 4;
    char data[4] = { 0 }; // control data length is 4
    panel_device *p_dev = (panel_device *) handle;

    data[0] = 27;
    data[1] = PAN_ESC_CMD_LBD;
    switch(ul_led_number) {
        case 0:
            data[2] = PAN_ESC_CMD_LBD_FUNCA;
			break;
        case 1:
            data[2] = PAN_ESC_CMD_LBD_FUNCB;
            break;
        case 2:
            data[2] = PAN_ESC_CMD_LBD_FUNCC;
			break;
        case 3:
            data[2] = PAN_ESC_CMD_LBD_FUNCD;
            break;
    }
    if(1 == uc_led_active)
        data[3] = PAN_ESC_CMD_LBD_ON;
    else
        data[3] = PAN_ESC_CMD_LBD_OFF;

    if(-1 == write(p_dev->device_fd, data, wr_len)){
		AUI_ERR("failed to run system call\n");
	}

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_panel_set_key_map(aui_hdl pv_hdl_panel, struct aui_pannel_key_map_cfg *cfg)
{
    AUI_RTN_CODE err = AUI_RTN_SUCCESS;
    panel_device *p_dev = (panel_device *) pv_hdl_panel;
    static struct ali_fp_key_map_cfg panel_key_map_cfg = {NULL, 0, 0, 0, 0, 0};		/* panel key map */

    if (NULL == p_dev || p_dev->device_fd == -1) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
    memset(&g_panel_key_map_cfg, 0, sizeof(struct aui_pannel_key_map_cfg));
    memcpy(&g_panel_key_map_cfg, cfg, sizeof(struct aui_pannel_key_map_cfg));
    AUI_DBG("%p, %p, unit_num: %ld\n", &g_panel_key_map_cfg, cfg, g_panel_key_map_cfg.unit_num);
    panel_key_map_cfg.phy_code = 0;
    panel_key_map_cfg.map_entry = NULL;
	panel_key_map_cfg.map_len = cfg->unit_num * sizeof(struct aui_pannel_key_map);
	panel_key_map_cfg.unit_len = sizeof(struct aui_pannel_key_map);
	panel_key_map_cfg.unit_num = cfg->unit_num;
	panel_key_map_cfg.map_entry = (unsigned char *)cfg->map_entry;
    err = (AUI_RTN_CODE)ioctl(p_dev->device_fd, ALI_FP_CONFIG_KEY_MAP, (unsigned long)(&panel_key_map_cfg));
    return err;
}

long aui_panel_get_input_event_id(char *dev, int *event_id)
{
	char *position = NULL;
	char *end = NULL;
	char buffer[1024];
	long fd = -1;
	long ret = -1;
	long read_size = 0;


	if (NULL == dev)
	{
		AUI_ERR("device_name is NULL\n");
		return -1;
	}

	AUI_DBG("dev = %s\n", dev);

	memset(buffer, 0x00, sizeof(buffer));
	fd = open("/proc/bus/input/devices", O_RDONLY|O_NONBLOCK);
	if (fd < 0)
	{
		AUI_ERR("open /proc/bus/input/devices Fail!\n");
		return -1;
	}

	read_size = read(fd, buffer, sizeof(buffer));
	if (-1 == read_size)
	{
		AUI_ERR("read fail!\n");
		ret = -1;
		goto END;
	}

	position = strstr(buffer, dev);
	if (NULL == position)
	{
		AUI_ERR("Fail!, dev = %s\n", dev);
		ret = -1;
		goto END;
	}

	position = strstr(position, "Handlers");
	if (NULL == position)
	{
		AUI_ERR("find Handlers Fail!\n");
		ret = -1;
		goto END;
	}

	position = strstr(position, "event");
	if (NULL == position)
	{
		AUI_ERR("find event Fail!\n");
		ret = -1;
		goto END;
	}

	if (strstr(position, " "))
	{
		end = strstr(position, " ");
	}
	else
	{
		end = strstr(position, "\n");
	}
	if (NULL != end)
	{
	    #if 0
		if ((end - position) <= 16)
		{
			memcpy(input_name, position, end - position);
		}
		else
		{
			memcpy(input_name, position, 16);
		}
        #endif
        *event_id = atoi(position + 5);
		AUI_INFO("input_event id = %d\n", *event_id);
	}
	else
	{
		AUI_ERR("Fail!\n");
		ret = -1;
		goto END;
	}

	ret = 0;


	END:
	{
		close(fd);
		return ret;
	}
}


AUI_RTN_CODE aui_panel_set_key_rep_interval(aui_hdl pv_hdl_panel,
		unsigned long delay,
		unsigned long interval)
{
    (void)pv_hdl_panel;
    AUI_RTN_CODE err = AUI_RTN_SUCCESS;
	/*panel_device *p_dev = (panel_device *) pv_hdl_panel;
	if (NULL == p_dev || p_dev->device_fd == -1) {
		AUI_ERR("aui_panel_set_panel_rep_interval error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}*/
    int input_event_id = 0;
    int input_dev_path_size = 36;
    char *input_dev_path = NULL;
    const char input_path[] = "/dev/input/event%d";
    input_dev_path = (char *)malloc(input_dev_path_size);
	if (NULL == input_dev_path) {
		AUI_ERR("malloc failed\n");
        return AUI_RTN_FAIL;
	}

    memset(input_dev_path, 0x00, input_dev_path_size);
    if (0 == aui_panel_get_input_event_id("pan_ch455", &input_event_id)) {
        snprintf(input_dev_path, input_dev_path_size, input_path, input_event_id);
        AUI_INFO("input dev path: %s\n", input_dev_path);
        long fd = open(input_dev_path, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
            AUI_ERR("open %s fail!\n", input_dev_path);
            free(input_dev_path);
            input_dev_path = NULL;
            return err;
        }
        unsigned long rep[2] = {0, 0};
        rep[0] = delay;
    	rep[1] = interval;
    	AUI_DBG("aui_panel_set_panel_rep_interval, delay: %ld, interval: %ld\n", rep[0], rep[1]);
    	err = ioctl(fd, EVIOCSREP, rep);    
		close(fd);		
    //	err = ioctl(p_dev->device_fd, ALI_FP_SET_REPEAT_INTERVAL, rep);
    } else {
        AUI_ERR("aui_panel_set_panel_rep_interval fail! get input name fail!\n");
    }
    free(input_dev_path);
    input_dev_path = NULL;
	return err;
}

