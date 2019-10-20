/**@file
*    @brief 		UDI rtc module interface implement
*    @author		ray.gong
*    @date			2013-5-21
*	 @version 		1.0.0
*    @note			ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <bus/rtc/rtc.h>
#include <aui_rtc.h>

AUI_MODULE(RTC)

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/
typedef struct aui_st_hld_clock
{
	long (*pf_rtc_time_set)(struct rtc_time* base_time);
	struct rtc_time* (*pf_rtc_time_get)(void);
	unsigned long (*pf_rtc_time_ms_get)(void);
	long (*pf_rtc_min_alm_set)(struct min_alarm* alarm,
		unsigned char num, void *rtc_callback);
	long (*pf_rtc_ms_alm_set)(struct ms_alarm* alarm,unsigned char num, 
		void *rtc_callback);
	long (*pf_rtc_alm_enable)(unsigned char enable,unsigned char num);
}aui_hld_clock,*p_aui_hld_clock;

typedef struct aui_st_rtc_module
{
	aui_dev_priv_data dev_priv_data;
	aui_hld_clock *hld_clock;
	int init_flag;
	int open_flag;
} aui_rtc_module_s, *aui_p_rtc_module_s;

/***************    LOCAL VAR             **************/
static aui_rtc_alarm_attr  aui_alarm_info;
static aui_hld_clock hld_clock;
static aui_rtc_module_s rtc_mod_op;

/***************    LOCAL FUNC DECLEAR    **************/
extern INT32 rtc_s3811_set_value(struct rtc_time* base_time);
extern struct rtc_time* rtc_s3811_read_value(void);
extern UINT32 rtc_s3811_read_ms_value(void);
extern INT32 rtc_s3811_set_min_alarm(struct min_alarm* alarm,UINT8 num, 
	void *rtc_callback);
extern INT32 rtc_s3811_set_ms_alarm(struct ms_alarm* alarm,UINT8 num, 
	void *rtc_callback);
extern INT32 rtc_s3811_en_alarm(UINT8 enable,UINT8 num);

/***************   MODULE DRV IMPLEMENT  *****************/
static void aui_dev_attach_rtc()
{
	MEMSET(&hld_clock,0,sizeof(aui_hld_clock));
	MEMSET(&aui_alarm_info,0,sizeof(aui_rtc_alarm_attr));
	if(((ALI_S3811==sys_ic_get_chip_id()) && 
		(sys_ic_get_rev_id() >= IC_REV_1)))
	{
		hld_clock.pf_rtc_time_set=rtc_s3811_set_value;
		hld_clock.pf_rtc_time_get=rtc_s3811_read_value;
		hld_clock.pf_rtc_time_ms_get=rtc_s3811_read_ms_value;
		hld_clock.pf_rtc_min_alm_set=rtc_s3811_set_min_alarm;
		hld_clock.pf_rtc_ms_alm_set=rtc_s3811_set_ms_alarm;
		hld_clock.pf_rtc_alm_enable=rtc_s3811_en_alarm;
		aui_alarm_info.min_alarm_num_start = 0;
		aui_alarm_info.min_alarm_num_end = 7;
		aui_alarm_info.ms_alarm_num_start = 8;
		aui_alarm_info.ms_alarm_num_end = 9;
		aui_alarm_info.cb_en =0;
	}
	else
	{
		hld_clock.pf_rtc_time_set=rtc_s3602_set_value;
		hld_clock.pf_rtc_time_get=rtc_s3602_read_value;
		hld_clock.pf_rtc_time_ms_get=rtc_s3602_read_ms_value;
		hld_clock.pf_rtc_min_alm_set=rtc_s3602_set_min_alarm;
		hld_clock.pf_rtc_ms_alm_set=rtc_s3602_set_ms_alarm;
		hld_clock.pf_rtc_alm_enable=rtc_s3602_en_alarm;
		aui_alarm_info.min_alarm_num_start = 0;
		aui_alarm_info.min_alarm_num_end = 1;
		aui_alarm_info.ms_alarm_num_start = 0;
		aui_alarm_info.ms_alarm_num_end = 1;
		aui_alarm_info.cb_en =1;
	}
}



/****************************MODULE IMPLEMENT*************************************/


AUI_RTN_CODE aui_rtc_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_RTC;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_rtc_init()
{
	aui_dev_attach_rtc();
	rtc_mod_op.init_flag = 1;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_rtc_de_init()
{
	rtc_mod_op.init_flag = 0;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_open(aui_hdl *p_rtc_handler)
{
	if (rtc_mod_op.open_flag == 0) {
		rtc_mod_op.hld_clock = &hld_clock;
		*p_rtc_handler = (aui_hdl)&rtc_mod_op;
		rtc_mod_op.open_flag = 1;
		aui_dev_reg(AUI_MODULE_RTC, (aui_hdl)&rtc_mod_op);
	} else {
		*p_rtc_handler = (aui_hdl)&rtc_mod_op;
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_rtc_close(aui_hdl rtc_handler)
{
    if(NULL == rtc_handler)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	if (rtc_mod_op.open_flag != 0) {
		rtc_mod_op.open_flag = 0;
		aui_dev_unreg(AUI_MODULE_RTC, (aui_hdl)&rtc_mod_op);
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_set_time(aui_hdl rtc_handler,aui_clock *p_clock)
{
	aui_rtc_module_s *p_rtc_module = (aui_rtc_module_s *)rtc_handler;
	aui_hld_clock *p_hdl_clock = p_rtc_module->hld_clock;
	if(NULL == p_hdl_clock || NULL == p_clock || 
		NULL ==p_hdl_clock->pf_rtc_time_set)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(RET_SUCCESS!=(p_hdl_clock->pf_rtc_time_set)((struct rtc_time*)p_clock))
	{
	    aui_rtn(AUI_RTN_EIO, "error");
	}
	return AUI_RTN_SUCCESS;
	
}

AUI_RTN_CODE aui_rtc_get_time(aui_hdl rtc_handler,aui_clock *p_clock)
{
	aui_clock *p_temp=NULL;
	aui_rtc_module_s *p_rtc_module = (aui_rtc_module_s *)rtc_handler;
	aui_hld_clock *p_hdl_clock = p_rtc_module->hld_clock;
	if(NULL == p_hdl_clock || NULL == p_clock || 
		NULL ==p_hdl_clock->pf_rtc_time_set)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	p_temp = (aui_clock *)(p_hdl_clock->pf_rtc_time_get());
	if(NULL==p_temp)
	{
	    aui_rtn(AUI_RTN_EIO, "error");
	}
	MEMCPY(p_clock,p_temp,sizeof(aui_clock));
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_get_alarm_info(aui_hdl rtc_handler,
	aui_rtc_alarm_attr *p_alarm)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	if( NULL== rtc_handler || NULL == p_alarm)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	MEMCPY(p_alarm,&aui_alarm_info,sizeof(aui_rtc_alarm_attr));
	return ret;
}


AUI_RTN_CODE aui_rtc_min_alarm_set(aui_hdl rtc_handler,
									int alarm_num,
									aui_min_alarm *p_min_alarm,
									aui_rtc_alarm_callback cb)
{
	aui_rtc_module_s *p_rtc_module = (aui_rtc_module_s *)rtc_handler;
	aui_hld_clock *p_hdl_clock = p_rtc_module->hld_clock;
	struct min_alarm st_min_alarm;
	MEMSET(&st_min_alarm,0,sizeof(struct min_alarm));
	if(NULL == p_hdl_clock || NULL == p_min_alarm || 
		NULL ==p_hdl_clock->pf_rtc_min_alm_set)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	st_min_alarm.month= p_min_alarm->month % 12;
	st_min_alarm.date= p_min_alarm->date % 32;
	st_min_alarm.hour = p_min_alarm->hour % 24;
	st_min_alarm.min = p_min_alarm->min % 60;
	
	if(RET_SUCCESS!=(p_hdl_clock->pf_rtc_min_alm_set)(&st_min_alarm,alarm_num,cb))
	{
	    aui_rtn(AUI_RTN_EIO, "error");
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_ms_alarm_set(aui_hdl rtc_handler,
							int alarm_num,
							aui_ms_alarm *p_ms_alarm,
							aui_rtc_alarm_callback cb)
{
	aui_rtc_module_s *p_rtc_module = (aui_rtc_module_s *)rtc_handler;
	aui_hld_clock *p_hdl_clock = p_rtc_module->hld_clock;
	struct ms_alarm st_ms_alarm;
	
	if(NULL == p_hdl_clock || NULL == p_ms_alarm || 
		NULL ==p_hdl_clock->pf_rtc_min_alm_set)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	MEMSET(&st_ms_alarm,0,sizeof(struct ms_alarm));
	st_ms_alarm.sec = p_ms_alarm->sec % 60;
	st_ms_alarm.ms = p_ms_alarm->ms %1000;
	st_ms_alarm.hour = p_ms_alarm->hour %24;
	st_ms_alarm.min = p_ms_alarm->min% 60;
	if(RET_SUCCESS!=(p_hdl_clock->pf_rtc_ms_alm_set)(&st_ms_alarm,alarm_num,cb))
	{
	    aui_rtn(AUI_RTN_EIO, "error");
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_rtc_alarm_switch(aui_hdl rtc_handler,
				unsigned char alarm_num,
				unsigned char uc_enable)
{
	aui_rtc_module_s *p_rtc_module = (aui_rtc_module_s *)rtc_handler;
	aui_hld_clock *p_hdl_clock = p_rtc_module->hld_clock;
	if(NULL == p_hdl_clock || NULL ==p_hdl_clock->pf_rtc_alm_enable)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(RET_SUCCESS!=(p_hdl_clock->pf_rtc_alm_enable)(uc_enable,alarm_num))
	{
	    aui_rtn(AUI_RTN_EIO, "error");
	}	
	return AUI_RTN_SUCCESS;
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



