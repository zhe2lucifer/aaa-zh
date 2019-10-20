/**@file
*	 @brief 			AUI misc module interface implement
*	 @author			Stephen.Xiao
*	 @date				2014-3-21
*	 @version			1.0.0
*	 @note				ali corp. all rights reserved. 2013~2020 copyright (C)\n
*	 This module will support some misc functions.
*
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_misc.h>
#include <aui_rtc.h>
#include <alislstandby.h>
#include <alislwatchdog.h>
#include <aui_nim.h>

AUI_MODULE(MISC)

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************MODULE MISC IMPLEMENT********************************/

static int g_panel_show_type = 1;

static int get_time(struct tm *timeinfo) {
	int err = 0;
	aui_clock clock;
	aui_hdl p_rtc_handler = NULL;

	memset(&clock, 0, sizeof(aui_clock));
	memset(timeinfo, 0, sizeof(struct tm));
	aui_find_dev_by_idx(AUI_MODULE_RTC, 0, &p_rtc_handler);
	if (p_rtc_handler == NULL){
		aui_rtc_init();
		aui_rtc_open(&p_rtc_handler);
	}
	err = aui_rtc_get_time(p_rtc_handler, &clock);

	timeinfo->tm_year = clock.year - 1900;
	timeinfo->tm_mon = clock.month - 1;
	timeinfo->tm_mday = clock.date;
	timeinfo->tm_hour = clock.hour;
	timeinfo->tm_min = clock.min;
	timeinfo->tm_sec = clock.sec;

	AUI_DBG("get_time: %d-%02d-%02d %02d:%02d:%02d\n",
			timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
			timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

	return err;
}

/* Put the working tuners into standby
 * Tuners loop-through output still work in standby mode
 */
int tuner_standby()
{
	int i = 0;
	int ret = 0;
	aui_hdl handle = NULL;

	for(i = 0; i < AUI_NIM_HANDLER_MAX; i++) {
		handle = NULL;
		if (aui_nim_handle_get_byid(i, &handle) != AUI_RTN_SUCCESS) {
			AUI_DBG("[tuner_standby] NIM[%d] not open\n", i);
			continue;
		}
		AUI_DBG("[tuner_standby] NIM[%d] %d standby\n", i, handle);
		if (aui_nim_set_state(handle, AUI_NIM_TUNER_STANDBY)) {
		    AUI_DBG("Tuner %d standby FAIL\n", i);
			ret += 1;
		}
	}
	return ret;
}

AUI_RTN_CODE aui_enter_pm_standby(void)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	rtn_code = alislstandby_enter_pm_standby();
	return rtn_code;
}

AUI_RTN_CODE aui_enter_pmu_standby(void)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	tuner_standby();
	//rtn_code = alislstandby_enter_pmu_standby();
	rtn_code = alislstandby_standby_enter(); // Just enter standby mode
	return rtn_code;
}

AUI_RTN_CODE aui_sys_reboot(unsigned long time_ms)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    rtn_code = alislwatchdog_reboot(time_ms * 1000);
    return rtn_code;
}

AUI_RTN_CODE aui_standby_set_ir(unsigned int firstkey,
								unsigned int firstkey_type,
								unsigned int secondkey,
								unsigned int secondkey_type)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	rtn_code = alislstandby_set_ir(firstkey, firstkey_type, secondkey,
			secondkey_type);
    return rtn_code;
}

AUI_RTN_CODE aui_reg_pmu_bin(unsigned char *pmu_data, unsigned long data_len)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	(void)pmu_data;
	(void)data_len;
	return rtn_code;
}

AUI_RTN_CODE aui_standby_init()
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	rtn_code = alislstandby_standby_init();
    return rtn_code;
}

AUI_RTN_CODE aui_standby_set_current_timer(struct tm *p_time)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	AUI_DBG("aui_standby_set_current_timer: %d-%02d-%02d %02d:%02d\n",
			p_time->tm_year, p_time->tm_mon, p_time->tm_mday,
			p_time->tm_hour, p_time->tm_min);
	rtn_code = (AUI_RTN_CODE)alislstandby_set_current_timer(p_time);
	return rtn_code;
}

AUI_RTN_CODE aui_standby_get_current_timer(struct tm *p_time)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	rtn_code = (AUI_RTN_CODE)alislstandby_get_current_timer(p_time);
	AUI_DBG("alislstandby_get_current_timer: %d-%02d-%02d %02d:%02d\n",
			p_time->tm_year, p_time->tm_mon, p_time->tm_mday,
			p_time->tm_hour, p_time->tm_min);
	return rtn_code;
}

AUI_RTN_CODE aui_standby_set_wakeup_timer(struct tm *p_time)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	rtn_code = alislstandby_set_wakeup_timer(p_time);
    return rtn_code;
}

AUI_RTN_CODE aui_standby_enter()
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	tuner_standby();
	rtn_code = alislstandby_standby_enter();
    return rtn_code;
}

AUI_RTN_CODE aui_get_power_status(aui_standby_type *p_standby_type)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned char standby_type = 0;
	ret = alislstandby_get_powerup_status(&standby_type);
	if (ret != 0) {
		*p_standby_type = 0;
		AUI_ERR("aui_get_power_status fail!\n");
		return ret;
	}

	switch(standby_type) {
	case ALISLS_PMU_IR_EXIT:
		*p_standby_type = AUI_STANDBY_IR;
		AUI_DBG("AUI_STANDBY_IR\n");
		break;
	case ALISLS_PMU_PANEL_EXIT:
		*p_standby_type = AUI_STANDBY_PANEL;
		AUI_DBG("AUI_STANDBY_PANEL\n");
		break;
	case ALISLS_PMU_TIMER_EXIT:
		*p_standby_type = AUI_STANDBY_TIMER;
		AUI_DBG("AUI_STANDBY_TIMER\n");
		break;
	case ALISLS_PMU_POWER_EXIT:
		*p_standby_type = 0;
		AUI_DBG("Cold boot\n");
		break;
	case ALISLS_PMU_WDT_EXIT:
		*p_standby_type = AUI_STANDBY_WATCHDOG;
		AUI_DBG("Watch dog reboot\n");
		break;
	case ALISLS_PMU_REBOOT_EXIT:
		*p_standby_type = AUI_STANDBY_REBOOT;
		AUI_DBG("Boot by reboot command\n");
		break;
	default:
	    AUI_DBG("Unknown PMU wake up type: %d!\n", *p_standby_type);
		*p_standby_type = standby_type;
		ret = 1;
	}
	return ret;
}

AUI_RTN_CODE aui_standby_set_show_type(AUI_PANNEL_SHOW_TYPE standby_type)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

	switch (standby_type){
	case AUI_PANNEL_SHOW_NONE:
		g_panel_show_type = ALISLS_PMU_SHOW_BLANK;
		break;
	case AUI_SHOW_OFF:
		g_panel_show_type = ALISLS_PMU_SHOW_OFF;
		break;
	case AUI_SHOW_TIME:
		g_panel_show_type = ALISLS_PMU_SHOW_TIME;
		break;
	case AUI_SHOW_ON:
		g_panel_show_type = ALISLS_PMU_SHOW_NO_CHANGE;
		break;
	default:
		g_panel_show_type = ALISLS_PMU_SHOW_DEFAULT;
		break;
	}

	ret = alislstandby_set_panel_display_mode(g_panel_show_type);
	if (ret != AUI_RTN_SUCCESS) {
	    AUI_DBG("Set panel display mode: %d fail!\n", standby_type);
	} else {
	    AUI_DBG("Set panel display g_panel_show_type: %d OK!\n", g_panel_show_type);
	}
	return ret;
}

/**
 *  Cause: struct tm tm_mon is 0~11,
 *  while aui_clock month and aui_min_alarm month is 1~12.
 *  struct tm tm_year is years since 1900 while aui_clock year is normal year.
 */
AUI_RTN_CODE aui_standby_set_state(aui_standby_setting setting)
{
	AUI_RTN_CODE ret = AUI_RTN_FAIL;
	struct tm time;

	AUI_DBG("[aui_standby_set_state] setting is:\n");
	AUI_DBG("state=%d, display_mode=%d\n", setting.state, setting.display_mode);
	AUI_DBG("key1=%#lx[%d], key2=%#lx[%d]\n",
			setting.standby_key[0].code, setting.standby_key[0].protocol,
			setting.standby_key[1].code, setting.standby_key[1].protocol);
	if (setting.wakeup_time) {
	    AUI_DBG("Wake up time: %02d-%02d %02d:%02d\n",
				setting.wakeup_time->tm_mon, setting.wakeup_time->tm_mday,
				setting.wakeup_time->tm_hour, setting.wakeup_time->tm_min);
	}

	if (setting.standby_key[0].code) {
		ret = alislstandby_set_ir(setting.standby_key[0].code,
				setting.standby_key[0].protocol,
				setting.standby_key[1].code,
				setting.standby_key[1].protocol);
		if (ret != AUI_RTN_SUCCESS) {
		    AUI_DBG("Set standby key fail!\n");
			return ret;
		}
	}

	ret = get_time(&time);
	if (ret == 0) {
		ret = aui_standby_set_current_timer(&time);
		if (ret != AUI_RTN_SUCCESS) {
		    AUI_DBG("SET standby current TIME fail!\n");
		}
	} else {
	    AUI_DBG("Get RTC TIME fail!\n");
	}
	if (setting.wakeup_time) {
		ret |= alislstandby_set_wakeup_timer(setting.wakeup_time);
		if (ret != AUI_RTN_SUCCESS) {
		    AUI_DBG("Set wake up time fail!\n");
			return ret;
		}
	}

	ret = aui_standby_set_show_type(setting.display_mode);
	if (ret != AUI_RTN_SUCCESS) {
	    AUI_DBG("Set panel display mode: %d fail!\n", setting.display_mode);
	}

	switch (setting.state) {
	case AUI_POWER_ON:
		break;
	case AUI_POWER_PMU_STANDBY:
		ret = aui_standby_enter();
		//alislstandby_enter_pmu_standby();
		break;
	case AUI_POWER_PM_STANDBY:
		ret = alislstandby_enter_pm_standby();
		break;
	default:
	    AUI_DBG("Unknown AUI POWER state: %d.\n", setting.state);
		break;
	}
	return ret;
}

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>


static int aui_mem_fd = -1;
#define	MEM_PHY_BASE 0x18000000
#define	MEM_SIZE 4

static unsigned long aui_soc_relay_func(unsigned long cmd, unsigned long *id)
{
    void *mem_var_base = NULL;
	unsigned long content = 0;

    if(-1 == aui_mem_fd) {
        aui_mem_fd = open("/dev/mem", O_RDONLY | O_SYNC);
        if(aui_mem_fd < 0) {
            AUI_DBG("[Error]: open /dev/mem error\n");
            return AUI_RTN_FAIL;
        }
    }

    mem_var_base = mmap(NULL, MEM_SIZE, PROT_READ, MAP_SHARED,aui_mem_fd, MEM_PHY_BASE);

    if((NULL == mem_var_base)|| (MAP_FAILED == mem_var_base) ) {
        AUI_DBG("[Error]: mem_var_base mmap error\n");
        return AUI_RTN_FAIL;
    }

	content = *((unsigned long *)mem_var_base);
	AUI_DBG("content 0x%lx\n",content);

	switch (cmd) {
	case AUI_SYS_GET_CHIP_ID:
		*id = ((content & 0xffff0000) >>16);
		break;
	case AUI_SYS_GET_REV_ID:
		*id = (content & 0xff);
		break;
	case AUI_SYS_GET_PRODUCT_ID:
		AUI_DBG("Read PRODUCT_ID no support\n");
		break;
	default:
		AUI_DBG("Unknown SYS item: %ld.\n", cmd);
		return AUI_RTN_FAIL;
	}

	close(aui_mem_fd);
	aui_mem_fd = -1;

	munmap(mem_var_base, MEM_SIZE);

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_sys_get(unsigned long ul_item, void *pv_param)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long id = 0;

	if(aui_soc_relay_func(ul_item, &id)) {
		AUI_DBG("[Error]: aui_soc_relay_func error\n");
		ret = AUI_RTN_FAIL;
	}

	if (id == (unsigned long)-1) {
		ret = AUI_RTN_FAIL;
	}
	*(unsigned long *)pv_param = id;

	return ret;
}

