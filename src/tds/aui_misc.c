/**@file
 *    @brief           AUI misc module interface implement
 *    @author          Stephen.Xiao
 *    @date            2014-3-21
 *    @version         1.0.0
 *    @note            ali corp. all rights reserved. 2013~2020 copyright (C)\n
 *    This module will support some misc functions.
 *
 */
/****************************INCLUDE HEAD FILE********************************/
#include "aui_common_priv.h"
//#include <hld/pmu/ali_pmu_bin.h>
#include "aui_common.h"
#include <aui_misc.h>
#include <aui_rtc.h>
#include <aui_os.h>
#include <aui_nim.h>

#include <hld/pmu/pmu.h>
#include <bus/rtc/rtc.h>
//#include "Ali_pmu_bin.h"

AUI_MODULE(MISC)

/****************************LOCAL MACRO**************************************/

// These values identify some different wakeup reasons about WDT.
// Set-top box started as a result of a reboot from the watch dog, 0x52454254 'REBT'
// In lld layer pmu_api.h:#define ALI_WDT_REBOOT (0x52454254) // 'REBT'
#define ALISL_PWR_WDT (0x52454254)
// Set-top box started as a result of a reboot requested, 0x424F4F54 'BOOT'
#define ALISL_PWR_REBOOT (0x424F4F54)
// Set-top box started as a result of wake up form standby mode, 0x57414B45 'WAKE'
#define ALISL_PWR_STANDBY (0x57414B45)

/****************************LOCAL TYPE***************************************/

/****************************LOCAL VAR****************************************/

static PANNEL_SHOW_TYPE g_standby_type = SHOW_TIME;
static PMU_BIN g_pmu_bin = {0, 0};

/****************************LOCAL FUNC DECLEAR*******************************/

/****************************MODULE MISC IMPLEMENT****************************/

#ifdef _RD_DEBUG_
#include <aui_dog.h>

aui_hdl misc_dog_handle = NULL;
static int totoaltime = 0;

static int dog_start (int reset_time_s)
{
	aui_attr_dog attr_dog;

	totoaltime = reset_time_s;
	AUI_DBG ("dog start timer %ds\n", reset_time_s);
	memset(&attr_dog, 0, sizeof(aui_attr_dog));
	attr_dog.uc_dev_idx = 0;
	attr_dog.ul_time_us = reset_time_s * 1000 * 1000;

	if (aui_find_dev_by_idx(AUI_MODULE_DOG, 0, &misc_dog_handle)
			!= AUI_RTN_SUCCESS) {
		if (aui_dog_open(&attr_dog, &misc_dog_handle) != AUI_RTN_SUCCESS) {
			AUI_DBG("cannot open watch dog.\n");
			return -1;
		}
	}
	if (aui_dog_config(misc_dog_handle,&attr_dog) != AUI_RTN_SUCCESS) {
		AUI_DBG("cannot config watch dog.\n");
		return -1;
	}
	if (aui_dog_start(misc_dog_handle,&attr_dog) != AUI_RTN_SUCCESS) {
		AUI_DBG("cannot start watch dog.\n");
		return -1;
	}
	AUI_DBG("start watch dog %ld us.\n", attr_dog.ul_time_us);
	return 0;
}
#endif

static int get_time(struct tm *timeinfo) {
	int err = 0;
	aui_clock clock;

	memset(&clock, 0, sizeof(aui_clock));
	memset(timeinfo, 0, sizeof(struct tm));

	aui_hdl p_rtc_handler = NULL;

	aui_find_dev_by_idx(AUI_MODULE_RTC, 0, &p_rtc_handler);
	if(p_rtc_handler == NULL){
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
	return err;
}

/* Put the working tuners into standby
 * Tuners loop-through output still work in standby mode
 */
int tuner_standby()
{
	int i = 0;
	int ret = 0;
	aui_hdl handle;

	for(i = 0; i < AUI_NIM_HANDLER_MAX; i++) {
		if (aui_nim_handle_get_byid(i, &handle) != AUI_RTN_SUCCESS)
			continue;
		if (aui_nim_set_state(handle, AUI_NIM_TUNER_STANDBY)) {
			AUI_DBG("Tuner %d standby FAIL\n", i);
			ret += 1;
		}
	}
	return ret;
}

void aui_pmu_notify(unsigned long param)
{
	(void)param;
}

AUI_RTN_CODE aui_reg_pmu_bin(unsigned char *pmu_data, unsigned long data_len)
{
	if(NULL == pmu_data) {
		return AUI_RTN_FAIL;
	}

	g_pmu_bin.buf = pmu_data;
	g_pmu_bin.len = data_len;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_enter_pm_standby(void)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	AUI_DBG("No support now.\n");
	return rtn_code;
}

AUI_RTN_CODE aui_enter_pmu_standby(void)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	rtn_code = aui_standby_enter();
	return rtn_code;
}

AUI_RTN_CODE aui_sys_reboot(unsigned long time_ms)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	(void)time_ms;
	struct pmu_device *pmu_dev = NULL;

	pmu_dev = (struct pmu_device *)dev_get_by_id(HLD_DEV_TYPE_PMU, 0);
	if (pmu_dev != NULL) {
		pmu_io_control(pmu_dev, ALI_PMU_SAVE_WDT_REBOOT_FLAG, ALISL_PWR_REBOOT);
	} else {
		AUI_DBG("AUI reboot flag setup fail\n");
	}

#ifndef _RD_DEBUG_
	AUI_DBG("sys_watchdog_reboot()\n");
	sys_watchdog_reboot();
#else
	AUI_DBG("System reboot by dog\n");
	dog_start(0);
	AUI_SLEEP(10);
#endif
	return rtn_code;
}

AUI_RTN_CODE aui_standby_init()
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

	struct pmu_device *pmu_dev =
			(struct pmu_device *)dev_get_by_id(HLD_DEV_TYPE_PMU, 0);
	if (pmu_dev == NULL){
		ret = pmu_m36_attach(aui_pmu_notify, 0);
	}
	return ret;
}

AUI_RTN_CODE aui_standby_set_show_type(AUI_PANNEL_SHOW_TYPE standby_type)
{
	switch(standby_type){
	case AUI_PANNEL_SHOW_NONE:
		g_standby_type = SHOW_BANK;
		break;
	case AUI_SHOW_OFF:
		g_standby_type = SHOW_OFF;
		break;
	case AUI_SHOW_TIME:
		g_standby_type = SHOW_TIME;
		break;
	case AUI_SHOW_ON:
		// TODO
		g_standby_type = standby_type;
		break;
	default:
		g_standby_type = standby_type;
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_standby_set_ir(unsigned int firstkey,
		unsigned int firstkey_type,
		unsigned int secondkey,
		unsigned int secondkey_type)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	ret = pmu_m36_ir_init();
	if (ret != AUI_RTN_SUCCESS) {
		aui_rtn(AUI_RTN_EIO, "pmu_m36_ir_init error");
	}
	//g_standby_type = SHOW_OFF;
	pmu_key_transfrom_extend(firstkey, firstkey_type,
			secondkey, secondkey_type, NULL);
	return ret;
}

AUI_RTN_CODE aui_standby_set_current_timer(struct tm *p_time)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct rtc_time pmu_time = {0, 0, 0, 0, 0, 0, 0};

	pmu_time.year = p_time->tm_year + 1900;
	pmu_time.month = p_time->tm_mon + 1;
	pmu_time.date = p_time->tm_mday;
	pmu_time.day = p_time->tm_wday;
	pmu_time.hour = p_time->tm_hour;
	pmu_time.min = p_time->tm_min;
	pmu_time.sec = p_time->tm_sec;

	AUI_DBG("aui_standby_set_current_timer [%d-%d-%d %d:%d:%d]\n",
			pmu_time.year, pmu_time.month, pmu_time.date,
			pmu_time.hour, pmu_time.min, pmu_time.sec);
	if (pmu_time.month > 12 || pmu_time.date > 31
			|| pmu_time.hour > 23 || pmu_time.min > 59
			|| pmu_time.sec > 59) {
		ret = 1;
	} else {
		ret = pmu_mcu_enter_stby_timer_set_value(
				(struct rtc_time *)(&pmu_time));
	}
	return ret;
}

AUI_RTN_CODE aui_standby_get_current_timer(struct tm *p_time)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct rtc_time * pmu_time = NULL;

	pmu_time = (struct rtc_time *)rtc_time_read_init_value();

	p_time->tm_year = pmu_time->year - 1900;
	p_time->tm_mon = pmu_time->month - 1;
	p_time->tm_mday = pmu_time->date;
	p_time->tm_wday = pmu_time->day;
	p_time->tm_hour = pmu_time->hour;
	p_time->tm_min = pmu_time->min;
	p_time->tm_sec = pmu_time->sec;

//	AUI_DBG("aui_standby_get_current_timer [%d-%d-%d %d:%d:%d]\n",
//			pmu_time->year, pmu_time->month, pmu_time->date,
//			pmu_time->hour, pmu_time->min, pmu_time->sec);
//	AUI_DBG("aui_standby_get_current_timer tm [%d-%d-%d %d:%d:%d]\n",
//			p_time->tm_year, p_time->tm_mon, p_time->tm_mday,
//			p_time->tm_hour, p_time->tm_min, p_time->tm_sec);
	if (pmu_time->month > 12 || pmu_time->date > 31
			|| pmu_time->hour > 23 || pmu_time->min > 59
			|| pmu_time->sec > 59) {
		ret = 1;
	}
	return ret;
}

AUI_RTN_CODE aui_standby_set_wakeup_timer(struct tm *p_time)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct min_alarm wakeup_time = {1,1,0,0,0,0,0,0,0,0,0,0,0,0};

	wakeup_time.month = p_time->tm_mon + 1;
	wakeup_time.date = p_time->tm_mday;
	wakeup_time.hour = p_time->tm_hour;
	wakeup_time.min = p_time->tm_min;
	wakeup_time.sec = p_time->tm_sec;
	//g_standby_type = SHOW_TIME;

	AUI_DBG("aui_standby_set_wakeup_timer [%d-%d %d:%d:%d]\n",
			wakeup_time.month, wakeup_time.date,
			wakeup_time.hour, wakeup_time.min, wakeup_time.sec);
	if (wakeup_time.month > 12 || wakeup_time.date > 31
			|| wakeup_time.hour > 23 || wakeup_time.min > 59
			|| wakeup_time.sec > 59) {
		ret = 1;
	} else {
		ret = pmu_mcu_wakeup_timer_set_min_alarm(&wakeup_time,0);
	}
	return ret;
}

AUI_RTN_CODE aui_standby_enter()
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	PMU_BIN pmu_bin;
	struct pmu_device *pmu_dev =
			(struct pmu_device *)dev_get_by_id(HLD_DEV_TYPE_PMU, 0);
	if(pmu_dev == NULL)
	{
		return AUI_RTN_FAIL;
	}

	tuner_standby();
	pmu_bin.buf = g_pmu_bin.buf;
	pmu_bin.len = g_pmu_bin.len;
	pmu_io_control(pmu_dev, PMU_PANNEL_POWER_STATUS, PANNLE_POWER_ON);
	pmu_io_control(pmu_dev, PMU_SHOW_TYPE_CMD, g_standby_type);
	pmu_io_control(pmu_dev, ALI_PMU_SAVE_WDT_REBOOT_FLAG, ALISL_PWR_STANDBY);
	pmu_io_control(pmu_dev, PMU_LOAD_BIN_CMD, (UINT32)(&pmu_bin));
	pmu_m36_en();
	return ret;
}

AUI_RTN_CODE aui_get_power_status(aui_standby_type *p_standby_type)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	PMU_EXIT_FLAG standby_type;
	int param;
	PMU_RESUME_FLAG pmu_status;
	unsigned int pwr_flag = 0;

	struct pmu_device *pmu_dev =
			(struct pmu_device *)dev_get_by_id(HLD_DEV_TYPE_PMU, 0);
	if (pmu_dev == NULL) {
		return AUI_RTN_FAIL;
	}

	if(AUI_RTN_SUCCESS != pmu_io_control(pmu_dev,PMU_RESUME_STATUS,(UINT32)&pmu_status)){
		AUI_DBG("pmu_resume_status fail!\n");
		return AUI_RTN_FAIL;
	}

	if(pmu_status == COLD_BOOT){
		*p_standby_type = AUI_STANDBY_COLD_BOOT;
		AUI_DBG("AUI_STANDBY_COLD_BOOT p_standby_type:%d\n",*p_standby_type);
		return AUI_RTN_SUCCESS;;
	}

	pwr_flag = pmu_io_control(pmu_dev, ALI_PMU_GET_WDT_REBOOT_FLAG, (UINT32)&param);
	standby_type = pmu_io_control(pmu_dev, PMU_POWER_ON_STATUS, (UINT32)&param);

	switch (pwr_flag) {
	        case ALISL_PWR_WDT:
	            // NOTE: WDT reboot
	            *p_standby_type = AUI_STANDBY_WATCHDOG;
	            break;
	        case ALISL_PWR_REBOOT:
	            // NOTE: reboot
	            *p_standby_type = AUI_STANDBY_REBOOT;
	            break;
	        case ALISL_PWR_STANDBY:
	            // NOTE: wake up from standby
	            switch (standby_type) {
	            case IR_EXIT:
	                *p_standby_type = AUI_STANDBY_IR;
	                AUI_DBG("AUI_STANDBY_IR p_standby_type:%d\n",*p_standby_type);
	                break;
	            case KEY_EXIT:
	                *p_standby_type = AUI_STANDBY_PANEL;
	                AUI_DBG("AUI_STANDBY_PANEL p_standby_type:%d\n",*p_standby_type);
	                break;
	            case RTC_EXIT:
	                *p_standby_type = AUI_STANDBY_TIMER;
	                AUI_DBG("AUI_STANDBY_TIMER p_standby_type:%d\n",*p_standby_type);
	                break;
	            default:
	                AUI_DBG("PMU unkown resume type\n");
	                return AUI_RTN_FAIL;
	            }
	            break;
	        default:
	            *p_standby_type = AUI_COLD_BOOT;
	            break;
	    }
	return ret;
}

AUI_RTN_CODE aui_standby_set_state(aui_standby_setting setting)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	struct tm time;

	AUI_DBG("[aui_standby_set_state] setting is:\n");
	AUI_DBG("state=%d, display_mode=%d\n", setting.state,
			setting.display_mode);
	AUI_DBG("key1=%#lx[%d], key2=%#lx[%d]\n",
			setting.standby_key[0].code, setting.standby_key[0].protocol,
			setting.standby_key[1].code, setting.standby_key[1].protocol);
//	if (setting.wakeup_time) {
//		AUI_DBG("wakeup_time: [%d-%d %d:%d]\n",
//				setting.wakeup_time->tm_mon, setting.wakeup_time->tm_mday,
//				setting.wakeup_time->tm_hour, setting.wakeup_time->tm_min);
//	}

	if (setting.standby_key[0].code) {
		ret = aui_standby_set_ir(setting.standby_key[0].code,
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
		ret |= aui_standby_set_wakeup_timer(setting.wakeup_time);
		if (ret != AUI_RTN_SUCCESS) {
			AUI_DBG("Set wake up time fail!\n");
			return ret;
		}
	}

	aui_standby_set_show_type(setting.display_mode);
	AUI_DBG("Set panel display mode: %d.\n", setting.display_mode);

	switch (setting.state) {
	case AUI_POWER_ON:
		break;
	case AUI_POWER_PMU_STANDBY:
		ret = aui_standby_enter();
		break;
	case AUI_POWER_PM_STANDBY:
		ret = aui_enter_pm_standby();
		break;
	default:
		AUI_DBG("Unknown AUI POWER state: %d.\n", setting.state);
		break;
	}

	return ret;
}

//#include <asm/chip.h>
AUI_RTN_CODE aui_sys_get(unsigned long ul_item, void *pv_param)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned long id = 0;

	switch (ul_item) {
	case AUI_SYS_GET_CHIP_ID:
		id = sys_ic_get_chip_id();
		break;
	case AUI_SYS_GET_REV_ID:
		id = sys_ic_get_rev_id();
		break;
	case AUI_SYS_GET_PRODUCT_ID:
		id = sys_ic_get_product_id();
		break;
	default:
		AUI_DBG("Unknown SYS item: %ld.\n", ul_item);
		id = (unsigned long)-1;
		ret = AUI_RTN_FAIL;
	}

	*(unsigned long *)pv_param = id;
	return ret;
}

