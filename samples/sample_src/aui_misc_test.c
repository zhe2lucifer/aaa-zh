/**@file
 *  (c) Copyright 2013-2999  ALi Corp. ZHA Linux SDK Team (alitech.com)
 *  All rights reserved
 *
 *  @file               aui_misc_test.c
 *  @brief              misc module's sample test code
 *
 *  @version            1.0
 *  @date               05/19/2014 02:04:56 PM
 *  @revision           none
 *
 *  @author             Stephen Xiao <stephen.xiao@alitech.com>
 */

#include "aui_misc_test.h"
#include <time.h>
#include <aui_rtc.h>
#include <string.h>
#include<stdio.h>

static int get_time(struct tm *timeinfo) {
	int err = 0;
	aui_clock clock;
	aui_hdl p_rtc_handler = NULL;

	memset(&clock, 0, sizeof(aui_clock));
	memset(timeinfo, 0, sizeof(struct tm));
	aui_find_dev_by_idx(AUI_MODULE_RTC, 0, &p_rtc_handler);
	if(p_rtc_handler == NULL) {
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

#ifdef AUI_LINUX
#include <sys/mount.h>

// In Linux, must unmount the USB drive before fast standby.
// The USB drive need to re-mount after wake up.
static int umount_usd()
{
	char *mounts = "/proc/mounts";
	char *sdx = "/mnt/usb/sd";
	FILE *fp = NULL;
	char buf[4096] = {0};
	char *target = NULL;
	unsigned int i = 0;

	if ((fp = fopen(mounts,"r")) == NULL) {
		AUI_PRINTF("%s,%d, error: open %s fail\n",
				__FUNCTION__, __LINE__, mounts);
		return -1;
	}
	while (fgets(buf, sizeof(buf), fp)) {
		if ((target = strstr(buf, sdx)) == NULL)
			continue;
		for (i = target - buf; i < sizeof(buf); i++) {
			if (buf[i] == ' ')
				buf[i] = 0;
		}
		if (umount2(target, MNT_DETACH)) {
			AUI_PRINTF("%s,%d, error: umount2 %s fail\n",
					__FUNCTION__, __LINE__, target);
		}
	}
	fclose(fp);

	return 0;
}

static int do_scan_disk(char *diskname)
{
	int ret = 0;

	ret |= setenv("ACTION", "add", 1);
	ret |= setenv("MDEV", diskname, 1);
	ret |= system("/etc/plug.sh");

	return ret;
}

// Mount all the USB drives
// aui_test use the "hotplug" to mount the USB drive
static int mount_usd()
{
	do_scan_disk("sda");
	do_scan_disk("sdb");
	do_scan_disk("sdc");
	do_scan_disk("sdd");

	return 0;
}

#endif

unsigned long test_misc_pm_standby(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) argc;
	(void) argv;
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_standby_set_ir(0x60df708f, 0, 0x60df9867, 0); // power and sleep key NEC

#ifdef AUI_LINUX
    umount_usd();
#endif

    ret = aui_enter_pm_standby();

    if (ret != AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("%s,%d,pm standby failed!\n",
                __FUNCTION__, __LINE__);
    }

#ifdef AUI_LINUX
    mount_usd();
#endif

    return ret;
}
unsigned long test_misc_pmu_standby(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned long keyA = 0x60DF609F; // red key NEC
    unsigned long keyB = 0x60DF9867; // sleep key NEC
    unsigned long key = 0;
    struct tm time;

    if (*argc > 0) {
    	sscanf(argv[0], "%lX", &key);
    	if (key > 0) {
    		keyA = key;
    	}
    }
    if (*argc > 1) {
    	sscanf(argv[1], "%lX", &key);
    	if (key > 0) {
    		keyB = key;
    	}
    }
    aui_standby_set_ir(keyA, 0, keyB, 0);
    AUI_PRINTF("%s,%d, \nPress key(NEC) 0x%x 0x%x to wake up!\n",
        		__FUNCTION__, __LINE__, (unsigned int)keyA, (unsigned int)keyB);
    aui_standby_set_show_type(AUI_SHOW_OFF);

    ret = get_time(&time);
    if (ret != 0) {
    	AUI_PRINTF("Get RTC ERROR!\n");
    } else {
    	aui_standby_set_current_timer(&time);
    }
    ret = aui_enter_pmu_standby();
    AUI_PRINTF("%s,%d, \nPress sleep key to wake up platform!\n",
                    __FUNCTION__, __LINE__);
    if (ret != AUI_RTN_SUCCESS)
    {
        AUI_PRINTF("%s,%d,pmu standby failed!\n",
                __FUNCTION__, __LINE__);
    }

    return ret;
}
unsigned long test_misc_sys_reboot(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	int time_ms = 3000;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    char *pchar;

    if (*argc > 0) {
        time_ms = strtol(argv[0], &pchar, 0);
    }
    AUI_PRINTF("%s,%d,system will reboot in %dms!\n",
            __FUNCTION__, __LINE__, time_ms);
    ret = aui_sys_reboot(time_ms);

    if (ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("%s,%d,system reboot failed!\n",
                __FUNCTION__, __LINE__);
    }
    return ret;
}
unsigned long test_misc_wakeup_timer(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    struct tm time;
    struct tm alarm_time;

    ret = aui_standby_init();
    if (ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("%s,%d,aui_standby_init failed!\n", __FUNCTION__, __LINE__);
    }

    ret = get_time(&time);
    if (ret != 0) {
    	AUI_PRINTF("Get RTC ERROR!\n");
    }
    ret = aui_standby_set_current_timer(&time);
    if (ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("%s,%d,aui_standby_set_current_timer failed!\n",
                __FUNCTION__, __LINE__);
    }
	if (*argc < 5) {
		AUI_PRINTF("Input error. Please see MISC help.\n");
		return AUI_RTN_EINVAL;
	} else {
		alarm_time.tm_mon = ATOI(argv[0]);
		alarm_time.tm_mday = ATOI(argv[1]);
		alarm_time.tm_hour = ATOI(argv[2]);
		alarm_time.tm_min = ATOI(argv[3]);
		alarm_time.tm_sec = ATOI(argv[4]);
	}

	ret = aui_standby_set_wakeup_timer(&alarm_time);
    if (ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("%s,%d,aui_standby_set_wakeup_timer failed!\n",
                __FUNCTION__, __LINE__);
        return AUI_RTN_FAIL;
    }
    //aui_standby_set_ir(0x60df708f, 0, 0x60df9867, 0); // power and sleep key NEC
    aui_standby_set_show_type(AUI_SHOW_TIME);
    AUI_PRINTF("%s,%d, \nWakeup timer is OK if platform wake up on time!\n",
    		__FUNCTION__, __LINE__);
    AUI_PRINTF("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
    		time.tm_year,time.tm_mon, time.tm_mday,
			time.tm_hour, time.tm_min, time.tm_sec);
    AUI_PRINTF("Wake up time: %02d-%02d %02d:%02d:%02d\n",
    		alarm_time.tm_mon, alarm_time.tm_mday,
			alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_sec);
    ret = aui_standby_enter();
    if (ret != AUI_RTN_SUCCESS) {
        AUI_PRINTF("%s,%d,standby enter failed!\n", __FUNCTION__, __LINE__);
    }
    return ret;
}

unsigned long test_misc_set_pm_standby(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) argc;
	(void) argv;
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_standby_setting setting;

	memset(&setting, 0, sizeof(aui_standby_setting));
	setting.state = AUI_POWER_PM_STANDBY;

#ifdef AUI_LINUX
    umount_usd();
#endif

	ret = aui_standby_set_state(setting);

#ifdef AUI_LINUX
    mount_usd();
#endif

	return ret;
}

unsigned long test_misc_set_pmu_standby(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_standby_setting setting;
	unsigned long key = 0;
    unsigned long keyType = 0xff;

	memset(&setting, 0, sizeof(aui_standby_setting));

/*
If you use LIRC, you can find the IR code at /etc/lirc/lircd.conf
The IR code in lircd.conf consists of "pre_data" and "coder".
Use the follow remote as an example
	KEY_YELLOW 0x60DFF807
	KEY_BLUE 0x60DFBA45

begin remote
	name  ali_60_key_remote
	bits           16
	flags SPACE_ENC|CONST_LENGTH
	eps            30
	aeps          100

	header       8913  4448
	one           555  1679
	zero          555   566
	ptrail        553
	repeat       8918  2222
	pre_data_bits   16
	pre_data       0x60DF
	gap          107508
	toggle_bit_mask 0x0

		begin codes
			KEY_YELLOW               0xF807
			KEY_BLUE                 0xBA45
		end codes
end remote
*/
	setting.standby_key[0].protocol = AUI_IR_NEC;
	setting.standby_key[0].code = 0x60DF609F; // red key NEC
	setting.standby_key[1].protocol = AUI_IR_NEC;
	setting.standby_key[1].code = 0x60DF9867; // sleep key NEC

	if (*argc > 0) {
		sscanf(argv[0], "%lX", &keyType);
		
		if (keyType != 0xff) {
			if (*argc > 1) {
				sscanf(argv[1], "%lX", &key);
				if (key) {
					setting.standby_key[0].protocol = keyType;
					setting.standby_key[0].code = key;
				}

				key = 0;
				if (*argc > 2) {
					sscanf(argv[2], "%lX", &key);
					if (key) {
						setting.standby_key[1].protocol = keyType;
						setting.standby_key[1].code = key;
					}
				}
			}
		}
	}

	setting.display_mode = AUI_PANNEL_SHOW_NONE;
	setting.state = AUI_POWER_PMU_STANDBY;
	ret = aui_standby_set_state(setting);
	return ret;
}

unsigned long test_misc_set_wakeup_time(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_standby_setting setting;
	struct tm cur_time;
	struct tm alarm_time;

	memset(&setting, 0, sizeof(aui_standby_setting));
	if (*argc < 5) {
		AUI_PRINTF("Input error. Please see MISC help.\n");
		return AUI_RTN_EINVAL;
	} else {
		alarm_time.tm_mon = ATOI(argv[0]);
		alarm_time.tm_mday = ATOI(argv[1]);
		alarm_time.tm_hour = ATOI(argv[2]);
		alarm_time.tm_min = ATOI(argv[3]);
		alarm_time.tm_sec = ATOI(argv[4]);
	}

	setting.state = AUI_POWER_PMU_STANDBY;
	setting.wakeup_time = &alarm_time;
	setting.display_mode = AUI_SHOW_TIME;

	setting.standby_key[0].code = 0x60DF609F; // red key NEC
	setting.standby_key[0].protocol = AUI_IR_NEC;
	setting.standby_key[1].code = 0x60DF9867; // sleep key NEC
	setting.standby_key[1].protocol = AUI_IR_NEC;

	memset(&cur_time, 0, sizeof(cur_time));
	if (get_time(&cur_time)) {
		AUI_PRINTF("Get RTC ERROR!\n");
	}

	AUI_PRINTF("%s,%d, \nWakeup timer is OK if platform wake up on time!\n",
			__FUNCTION__, __LINE__);
	AUI_PRINTF("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
			cur_time.tm_year,cur_time.tm_mon, cur_time.tm_mday,
			cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec);
	AUI_PRINTF("Wake up time: %02d-%02d %02d:%02d:%02d\n",
			alarm_time.tm_mon, alarm_time.tm_mday,
			alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_sec);

	AUI_PRINTF("[SETTING]: state=%d, wakeup_time=%02d-%02d %02d:%02d:%02d, "
					"key1=%#lx[%d], key2=%#lx[%d], display_mode=%d\n",
					setting.state,
					setting.wakeup_time->tm_mon, setting.wakeup_time->tm_mday,
					setting.wakeup_time->tm_hour, setting.wakeup_time->tm_min,
					setting.wakeup_time->tm_sec,
					setting.standby_key[0].code, setting.standby_key[0].protocol,
					setting.standby_key[1].code, setting.standby_key[1].protocol,
					setting.display_mode);
	ret = aui_standby_set_state(setting);
	return ret;
}
unsigned long test_misc_pm_set_wakeup_time(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_standby_setting setting;
    struct tm cur_time;
    struct tm alarm_time;

    memset(&setting, 0, sizeof(aui_standby_setting));
    if (*argc < 5) {
        AUI_PRINTF("Input error. Please see MISC help.\n");
        return AUI_RTN_EINVAL;
    } else {
        alarm_time.tm_mon = ATOI(argv[0]);
        alarm_time.tm_mday = ATOI(argv[1]);
        alarm_time.tm_hour = ATOI(argv[2]);
        alarm_time.tm_min = ATOI(argv[3]);
        alarm_time.tm_sec = ATOI(argv[4]);
    }

    setting.state = AUI_POWER_PM_STANDBY;
    setting.wakeup_time = &alarm_time;
    setting.display_mode = AUI_SHOW_TIME;

    setting.standby_key[0].code = 0x60DF609F; // red key NEC
    setting.standby_key[0].protocol = AUI_IR_NEC;
    setting.standby_key[1].code = 0x60DF9867; // sleep key NEC
    setting.standby_key[1].protocol = AUI_IR_NEC;

    memset(&cur_time, 0, sizeof(cur_time));
    if (get_time(&cur_time)) {
            AUI_PRINTF("Get RTC ERROR!\n");
    }

    AUI_PRINTF("%s,%d, \nWakeup timer is OK if platform wake up on time!\n",
            __FUNCTION__, __LINE__);
    AUI_PRINTF("Current time: %04d-%02d-%02d %02d:%02d:%02d\n",
            cur_time.tm_year, cur_time.tm_mon, cur_time.tm_mday,
            cur_time.tm_hour, cur_time.tm_min, cur_time.tm_sec);
    AUI_PRINTF("Wake up time: %02d-%02d %02d:%02d:%02d\n",
            alarm_time.tm_mon, alarm_time.tm_mday,
            alarm_time.tm_hour, alarm_time.tm_min, alarm_time.tm_sec);

    AUI_PRINTF("[SETTING]: state=%d, wakeup_time=%02d-%02d %02d:%02d:%02d, "
                    "key1=%#lx[%d], key2=%#lx[%d], display_mode=%d\n",
                    setting.state,
                    setting.wakeup_time->tm_mon, setting.wakeup_time->tm_mday,
                    setting.wakeup_time->tm_hour, setting.wakeup_time->tm_min,
                    setting.wakeup_time->tm_sec,
                    setting.standby_key[0].code, setting.standby_key[0].protocol,
                    setting.standby_key[1].code, setting.standby_key[1].protocol,
                    setting.display_mode);

#ifdef AUI_LINUX
    umount_usd();
#endif

    ret = aui_standby_set_state(setting);

#ifdef AUI_LINUX
    mount_usd();
#endif

    return ret;
}

unsigned long test_misc_set_panel_display(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_standby_setting setting;
	int mode = AUI_SHOW_TIME;

	memset(&setting, 0, sizeof(aui_standby_setting));
	if (*argc > 0) {
		mode = atoi(argv[0]);
	}
	setting.state = AUI_POWER_PMU_STANDBY;
	setting.display_mode = mode;
	ret = aui_standby_set_state(setting);
	return ret;
}

unsigned long test_misc_get_power_status(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void) argc;
	(void) argv;
	(void) sz_out_put;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_standby_type type = 0;
	char types[][8] = {"Power", "Timer", "Panel", "IR", "WDT", "Reboot"};
	ret = aui_get_power_status(&type);
	if ((ret == AUI_RTN_SUCCESS) && (type < sizeof(types) / sizeof(types[0]))) {
		AUI_PRINTF("Wake up from %s\n", types[type]);
	} else {
		AUI_PRINTF("Get wake up status fail\n");
	}
	unsigned long chip_id = 0;
	unsigned long rev = 0;
	unsigned long product_id = 0;
	aui_sys_get(AUI_SYS_GET_CHIP_ID, &chip_id);
	aui_sys_get(AUI_SYS_GET_REV_ID, &rev);
	aui_sys_get(AUI_SYS_GET_PRODUCT_ID, &product_id);
	AUI_PRINTF("CHIP_ID=%#lx REV_ID=%#lx PRODUCT_ID=%#lx\n",
			chip_id, rev, product_id);
	return ret;
}

/////////////////niker.li add start//////////////////////////////////////
#ifdef BOARD_CFG_M3755

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"
#define MAX_SIZE 32

unsigned long test_misc_get_temperature(unsigned long *argc, char **argv, char *sz_out_put)
{
	int fd = 0;
	double temp = 0;
	char buf[MAX_SIZE];
	int i = 0;

    // open /sys/class/thermal/thermal_zone0/temp
    fd = open(TEMP_PATH, O_RDONLY);
    if (fd < 0) {
        AUI_PRINTF("failed to open thermal_zone0/temp\n");
        return AUI_RTN_EINVAL;
    }

    // Read the temperature value
    if (read(fd, buf, MAX_SIZE) < 0) {
		AUI_PRINTF("failed to read temp\n");
        return AUI_RTN_EINVAL;
    }

    //Converts to floating point printing
	temp = atoi(buf) / 1000.0;
    AUI_PRINTF("temp: %.2f C\n", temp);

	//close fd
    close(fd);

	return AUI_RTN_SUCCESS;
}
#endif
//////////////////niker.li add end/////////////////////////////////

unsigned long test_misc_help(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void)argc;
	(void)argv;
	(void)sz_out_put;

	char help[] =
"\n\nMISC Test Help \n\
COMMAND \n\
        '1': \n\
                Fast standby. \n\
                Need platform hardware support. \n\
\n\
        '2': \n\
                PMU standby. \n\
                Format:         2 [key1],[key2] \n\
                Example:        2 60df609F,60df9867 \n\
                                Use RED,SLEEP key to wake up platform. \n\
                Support 2 keys to wake up platform. \n\
                Default keys is 60df609F,60df9867 (RED,SLEEP). \n\
\n\
        '3': \n\
                Reboot the system. \n\
                Format:         3 [ms delay] \n\
                Example:        3 5000 \n\
                                Reboot the system in 5000ms. \n\
                Default delay time is 3000ms. \n\
\n\
        '4': \n\
                PMU standby wake up by timer. \n\
                Format:         4 [month],[month day],[hour],[minute],[second] \n\
                Example:        4 0,1,0,0,10 \n\
                                Get the current time by RTC module, and set wake up time properly. \n\
                If current time is 2014-12-31 23:59:00,  \n\
                the example(4 0,1,0,0,10) will make the platform wake up in 70 seconds.\n\
                Cause: struct tm tm_mon is 0~11, while aui_clock month and aui_min_alarm month is 1~12. \n\
                       struct tm tm_year is years since 1900 while aui_clock year is normal year. \n\
\n\
        '5': \n\
                Fast standby, using API aui_standby_set_state. \n\
                Need platform hardware support. \n\
\n\
        '6': \n\
                PMU standby, using API aui_standby_set_state. \n\
                Format:         6 [key type],[key1],[key2] \n\
                Key Type: \n\
                    0: NEC \n\
                    1: LAB\n\
                    2: 50560\n\
                    3: KF\n\
                    4: LOGIC\n\
                    5: SRC\n\
                    6: NSE\n\
                    7: RC5\n\
                    8: RC5_X\n\
                    9: RC6\n\
                Example:        6 0,60df609F,60df9867 \n\
                                Use User's keys to wake up platform. \n\
                Support 2 keys to wake up platform. \n\
                Default Key Type is 0, Default Keys is 60df609F,60df9867 (RED,SLEEP). \n\
\n\
        '7': \n\
                PMU standby wake up by timer, using API aui_standby_set_state. \n\
                Format:         7 [month],[month day],[hour],[minute],[second] \n\
                Example:        7 0,1,0,0,10 \n\
                                Get the current time by RTC module, and set wake up time properly. \n\
                If current time is 2014-12-31 23:59:00,  \n\
                the example(7 0,1,0,0,10) will make the platform wake up in 70 seconds.\n\
                Cause: struct tm tm_mon is 0~11, while aui_clock month and aui_min_alarm month is 1~12. \n\
\n\
        '8': \n\
                PMU standby panel display mode. \n\
                Format:         8 [mode] \n\
                Example:        8 2 \n\
                                0: Display blank \n\
                                1: Display string \"OFF\" \n\
                                2: Display time \n\
                                3: Just keep panel on, do not changed the content. \n\
                Default display mode is 2 (TIME). \n\
\n\
        '9': \n\
                Get the wake up type. \n\
                The platform can wake up by Power button/IR remote/panel button/timer \n\
\n\
        '10': \n\
                Faster standby wake up by timer, using API aui_standby_set_state. \n\
                Format:         10 [month],[month day],[hour],[minute],[second] \n\
                Example:        10 0,1,0,0,10 \n\
                                Get the current time by RTC module, and set wake up time properly. \n\
                If current time is 2014-12-31 23:59:00,  \n\
                the example(10 0,1,0,0,10) will make the platform wake up in 70 seconds.\n\
                Cause: struct tm tm_mon is 0~11, while aui_clock month and aui_min_alarm month is 1~12. \n\
\n\n";

	AUI_PRINTF("%s", help);
	return AUI_RTN_HELP;
}

void misc_test_reg(void)
{
    aui_tu_reg_group("misc", "misc tests");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_misc_pm_standby,  "fast standby");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_misc_pmu_standby, "PMU standby");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_misc_sys_reboot,  "reboot");
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_misc_wakeup_timer, "wake up timer");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_misc_set_pm_standby, "faster standby, using API aui_standby_set_state");
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_misc_set_pmu_standby, "PMU standby, using API aui_standby_set_state");
    aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_misc_set_wakeup_time, "wake up timer, using API aui_standby_set_state");
    aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, test_misc_set_panel_display, "set PMU standby panel display mode");
    aui_tu_reg_item(2, "9", AUI_CMD_TYPE_API, test_misc_get_power_status, "get power up status");
    aui_tu_reg_item(2, "10", AUI_CMD_TYPE_API, test_misc_pm_set_wakeup_time, "faster standby and wake up timer, using API aui_standby_set_state");
#ifdef BOARD_CFG_M3755
	aui_tu_reg_item(2, "11", AUI_CMD_TYPE_API, test_misc_get_temperature, "get ic temperature");
#endif
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_misc_help, "MISC help");
}
