#include "aui_common_priv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ali_pmu_common.h>
#include <aui_rtc.h>
#include <alislstandby.h>
#include <alislevent.h>
#include <sys/epoll.h>

AUI_MODULE(RTC)

#define MIN_ALARM_NUM_START (0)
#define MIN_ALARM_NUM_END (1)
#define MS_ALARM_NUM_START (2)
#define MS_ALARM_NUM_END (3)
#define ALARM_NUM_END (MS_ALARM_NUM_END > MIN_ALARM_NUM_END \
		? MS_ALARM_NUM_END : MIN_ALARM_NUM_END)
#define CB_EN (1)

typedef struct aui_st_handle_rtc
{
	int 					rtc_fd;
	int					init_flag;
	int					open_flag;
	struct alislrtc_timer	timers[MS_ALARM_NUM_END + 1];
	alisl_handle ev_handle;
	aui_rtc_alarm_attr	info;
}aui_handle_rtc;

typedef struct aui_st_rtc_module
{
	aui_dev_priv_data dev_priv_data;
	aui_handle_rtc rtc_dev;
} aui_rtc_module_s, *aui_p_rtc_module_s;

static aui_rtc_module_s rtc_mod_op;

AUI_RTN_CODE aui_rtc_version_get(unsigned long *pul_version)
{
	if (NULL == pul_version) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	*pul_version = AUI_MODULE_VERSION_NUM_RTC;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_init(void)
{
	AUI_API("");

	memset(&rtc_mod_op, 0x00, sizeof(aui_rtc_module_s));
	rtc_mod_op.rtc_dev.init_flag = true;
	rtc_mod_op.rtc_dev.open_flag = false;
	rtc_mod_op.rtc_dev.rtc_fd = -1;
	rtc_mod_op.rtc_dev.info.min_alarm_num_start = MIN_ALARM_NUM_START;
	rtc_mod_op.rtc_dev.info.min_alarm_num_end = MIN_ALARM_NUM_END;
	rtc_mod_op.rtc_dev.info.ms_alarm_num_start = MS_ALARM_NUM_START;
	rtc_mod_op.rtc_dev.info.ms_alarm_num_end = MS_ALARM_NUM_END;
	rtc_mod_op.rtc_dev.info.cb_en = CB_EN;


	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_open(aui_hdl *pp_hdl_rtc)
{
	AUI_API("");

	if (rtc_mod_op.rtc_dev.init_flag != true) {
		AUI_ERR("aui_rtc_open error init_flag\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if (!pp_hdl_rtc) {
		AUI_ERR("aui_rtc_open error pp_hdl_rtc\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	rtc_mod_op.rtc_dev.rtc_fd = open("/dev/ali_pmu", O_RDWR|O_NONBLOCK);
	if (rtc_mod_op.rtc_dev.rtc_fd < 0){
		AUI_ERR("aui_rtc_open error open\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	if (alislevent_open(&rtc_mod_op.rtc_dev.ev_handle)) {
		AUI_ERR("aui_rtc_open error alislevent_open\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	rtc_mod_op.rtc_dev.open_flag = true;
	*pp_hdl_rtc = (aui_hdl)&rtc_mod_op;

	aui_dev_reg(AUI_MODULE_RTC, &rtc_mod_op);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_rtc_close(aui_hdl p_hdl_rtc)
{
	AUI_API("");

	if ((rtc_mod_op.rtc_dev.init_flag != true)
			|| (rtc_mod_op.rtc_dev.open_flag != true)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	aui_rtc_module_s *rtc_index = (aui_rtc_module_s *)p_hdl_rtc;

	if (rtc_index != &rtc_mod_op) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	alislevent_close(rtc_mod_op.rtc_dev.ev_handle);

	close(rtc_mod_op.rtc_dev.rtc_fd);
	rtc_mod_op.rtc_dev.open_flag = false;
	rtc_mod_op.rtc_dev.rtc_fd = -1;
	aui_dev_unreg(AUI_MODULE_RTC, &rtc_mod_op);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_set_time(aui_hdl rtc_handler,aui_clock *p_clock)
{
	AUI_RTN_CODE ret  = AUI_RTN_SUCCESS;
	int i;

	AUI_API("%d-%d-%d %d:%d:%d", p_clock->year, p_clock->month,
		p_clock->day, p_clock->hour, p_clock->min, p_clock->sec);

	if (!rtc_handler || !p_clock) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if ((rtc_mod_op.rtc_dev.init_flag != true)
			|| (rtc_mod_op.rtc_dev.open_flag != true)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	aui_rtc_module_s *rtc_index = (aui_rtc_module_s *)rtc_handler;
	if (rtc_index != &rtc_mod_op) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	struct rtc_time_pmu set_time;
	memset(&set_time,0x00,sizeof(struct rtc_time_pmu));
	set_time.year = p_clock->year;
	set_time.month = p_clock->month;
	set_time.day = p_clock->day;	// day of week
	set_time.date = p_clock->date;  // day of month
	set_time.hour = p_clock->hour;
	set_time.min = p_clock->min;
	set_time.sec = p_clock->sec;

	ret = alislrtc_ioctl(rtc_mod_op.rtc_dev.rtc_fd, ALI_PMU_RTC_SET_VAL,
			&set_time);
	if (!ret) {
		for (i = 0; i <= ALARM_NUM_END; i++) {
			if (rtc_mod_op.rtc_dev.timers[i].state == 1) {
				aui_rtc_alarm_switch(rtc_handler, i, 1);
			}
		}
	}
	return ret;
}


AUI_RTN_CODE aui_rtc_get_time(aui_hdl rtc_handler,aui_clock *p_clock)
{
	if (!rtc_handler || !p_clock) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if ((rtc_mod_op.rtc_dev.init_flag != true)
			|| (rtc_mod_op.rtc_dev.open_flag != true)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	aui_rtc_module_s *rtc_index = (aui_rtc_module_s *)rtc_handler;
	if (rtc_index != &rtc_mod_op) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	struct rtc_time_pmu read_time;
	memset(&read_time,0x00,sizeof(struct rtc_time_pmu));

	alislrtc_ioctl(rtc_mod_op.rtc_dev.rtc_fd,  ALI_PMU_RTC_RD_VAL, &read_time);
//	AUI_DBG("\r ALI_PMU_RTC_RD_VAL [%04d-%02d-%02d %02d:%02d:%02d].\n",
//			read_time.year, read_time.month, read_time.date,
//			read_time.hour, read_time.min, read_time.sec);

	p_clock->year = read_time.year;
	p_clock->month = read_time.month;
	p_clock->date = read_time.date;  // day of month
	p_clock->day = read_time.day;    // day of week
	p_clock->hour = read_time.hour;
	p_clock->min = read_time.min;
	p_clock->sec = read_time.sec;

	AUI_API("%d-%d-%d %d:%d:%d", p_clock->year, p_clock->month,
		p_clock->day, p_clock->hour, p_clock->min, p_clock->sec);

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_rtc_de_init()
{
	AUI_API("");

	if ((rtc_mod_op.rtc_dev.rtc_fd >= 0)
			&& (rtc_mod_op.rtc_dev.open_flag == true)) {
		close(rtc_mod_op.rtc_dev.rtc_fd);
		rtc_mod_op.rtc_dev.rtc_fd = -1;
		rtc_mod_op.rtc_dev.open_flag = false;
	}
	rtc_mod_op.rtc_dev.init_flag = false;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_get_alarm_info(aui_hdl rtc_handler,
		aui_rtc_alarm_attr *p_alarm)
{
	AUI_API("");

	(void)rtc_handler;
	*p_alarm = rtc_mod_op.rtc_dev.info;
	return AUI_RTN_SUCCESS;
}

void *rtc_callback_func(void *param)
{
	int alarm_index;
	struct alislrtc_timer *timer;
	aui_rtc_alarm_callback cb;

	alarm_index = (int)param;

	AUI_DBG("index %d", alarm_index);

	timer = &rtc_mod_op.rtc_dev.timers[alarm_index];
//	AUI_DBG("RTC alarm[%d] is on. fd: %d\n", alarm_index, timer->slev.fd);
	cb = (aui_rtc_alarm_callback)timer->cb;
	if (timer->cb != NULL) {
		timer->state = 2;
		cb();
	}
	return (void *)0;
}

AUI_RTN_CODE aui_rtc_min_alarm_set(aui_hdl rtc_handler,
		int alarm_num,
		aui_min_alarm *p_min_alarm,
		aui_rtc_alarm_callback cb)
{
	AUI_RTN_CODE ret  = AUI_RTN_SUCCESS;
	struct alislrtc_timer *timer;
	struct alislevent *slev;

	AUI_API("");

	if (!rtc_handler) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if ((rtc_mod_op.rtc_dev.init_flag != true)
			|| (rtc_mod_op.rtc_dev.open_flag != true)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	aui_rtc_module_s *rtc_index = (aui_rtc_module_s *)rtc_handler;
	if (rtc_index != &rtc_mod_op) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if (!(rtc_mod_op.rtc_dev.info.min_alarm_num_start <= alarm_num
			&& rtc_mod_op.rtc_dev.info.min_alarm_num_end >= alarm_num)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	timer = &rtc_mod_op.rtc_dev.timers[alarm_num];

	timer->tm.tm_mon = p_min_alarm->month - 1;
	timer->tm.tm_mday = p_min_alarm->date;
	timer->tm.tm_hour = p_min_alarm->hour;
	timer->tm.tm_min = p_min_alarm->min;
	timer->type = 0;
	timer->cb = (void *)cb;

	slev = &timer->slev;
	slev->cb = (alisl_event_callback)rtc_callback_func;
	slev->events = EPOLLIN | EPOLLET;
	slev->data = (void *)alarm_num;
	timer->config = 1;

	//	AUI_DBG("SET RTC MIN TIMER: %d\n", alarm_num);
	return ret;
}

AUI_RTN_CODE aui_rtc_ms_alarm_set(aui_hdl rtc_handler,
		int alarm_num,
		aui_ms_alarm *p_ms_alarm,
		aui_rtc_alarm_callback cb)
{
	AUI_RTN_CODE ret  = AUI_RTN_SUCCESS;
	struct alislrtc_timer *timer;
	struct alislevent *slev;

	AUI_API("");

	if (!rtc_handler) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if ((rtc_mod_op.rtc_dev.init_flag != true)
			|| (rtc_mod_op.rtc_dev.open_flag != true)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	aui_rtc_module_s *rtc_index = (aui_rtc_module_s *)rtc_handler;
	if (rtc_index != &rtc_mod_op) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if (!(rtc_mod_op.rtc_dev.info.ms_alarm_num_start <= alarm_num
			&& rtc_mod_op.rtc_dev.info.ms_alarm_num_end >= alarm_num)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	timer = &rtc_mod_op.rtc_dev.timers[alarm_num];

	timer->tm.tm_hour = p_ms_alarm->hour;
	timer->tm.tm_min = p_ms_alarm->min;
	timer->tm.tm_sec = p_ms_alarm->sec;
	timer->ms = p_ms_alarm->ms;
	timer->type = 1;
	timer->cb = (void *)cb;

	slev = &timer->slev;
	slev->cb = (alisl_event_callback)rtc_callback_func;
	slev->events = EPOLLIN | EPOLLET;
	slev->data = (void *)alarm_num;
	timer->config = 1;

//	AUI_DBG("SET RTC MIN TIMER: %d\n", alarm_num);
	return ret;
}


AUI_RTN_CODE aui_rtc_alarm_switch(aui_hdl rtc_handler,
		unsigned char alarm_num,
		unsigned char uc_enable)
{
	struct alislrtc_timer *timer;
	AUI_RTN_CODE ret  = AUI_RTN_SUCCESS;

	AUI_API("");

	if (!rtc_handler) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if ((rtc_mod_op.rtc_dev.init_flag != true)
			|| (rtc_mod_op.rtc_dev.open_flag != true)) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	aui_rtc_module_s *rtc_index = (aui_rtc_module_s *)rtc_handler;
	if (rtc_index != &rtc_mod_op) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if (alarm_num < rtc_mod_op.rtc_dev.info.min_alarm_num_start
			&& alarm_num > rtc_mod_op.rtc_dev.info.ms_alarm_num_end) {
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	timer = &rtc_mod_op.rtc_dev.timers[alarm_num];
	//AUI_DBG("timers[%d]:%p fp: %d\n", alarm_num, timer, timer->slev.fd);

	if (!timer->config) {
		AUI_ERR("RTC timer %d is not set yet.\n", alarm_num);
		return -1;
	}

//	AUI_DBG("Turn %s RTC TIMER: %d\n", uc_enable? "ON" : "OFF", alarm_num);
	ret = alislrtc_setalarm(rtc_mod_op.rtc_dev.rtc_fd,
			rtc_mod_op.rtc_dev.ev_handle, timer, uc_enable);

	return ret;
}























/// ----------------- deprecated API


AUI_RTN_CODE aui_rtc_set_alarm_minutes(aui_hdl rtc_handler,
		int alarm_num,
		aui_min_alarm *p_min_alarm,
		aui_rtc_alarm_callback *cb)
{
	return aui_rtc_min_alarm_set(rtc_handler, alarm_num, p_min_alarm, (aui_rtc_alarm_callback)cb);
}

AUI_RTN_CODE aui_rtc_set_alarm_ms(aui_hdl rtc_handler,
		int alarm_num,
		aui_ms_alarm *p_ms_alarm,
		aui_rtc_alarm_callback *cb)
{
	return aui_rtc_ms_alarm_set(rtc_handler, alarm_num, p_ms_alarm, (aui_rtc_alarm_callback)cb);
}


