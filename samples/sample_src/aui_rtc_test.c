/****************************INCLUDE HEAD FILE************************************/
#include "aui_rtc_test.h"
#include "aui_help_print.h"
#include <string.h>
#include<stdio.h>

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/
aui_hdl g_p_hdl_rtc;
unsigned char alarm_num =0;


void aui_rtc_call_back_test()
{
	AUI_PRINTF("aui_rtc_call_back_test\n");
	aui_clock st_rtc_time;
	memset(&st_rtc_time, 0, sizeof(aui_clock));
	aui_rtc_get_time(g_p_hdl_rtc, &st_rtc_time);
	AUI_PRINTF("\n\n RTC alarm[%d] time is up. [%04d.%02d.%02d %02d:%02d:%02d].\n",
			alarm_num, st_rtc_time.year, st_rtc_time.month, st_rtc_time.date,
			st_rtc_time.hour, st_rtc_time.min, st_rtc_time.sec);
	aui_rtc_alarm_switch(g_p_hdl_rtc, alarm_num, 0);
}

/****************************EXTERN VAR*******************************************/


/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/


unsigned long test_rtc_time_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_clock st_rtc_time;

	MEMSET(&st_rtc_time,0,sizeof(aui_clock));

	AUI_TEST_CHECK_NULL(g_p_hdl_rtc);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,6);

	st_rtc_time.year = ATOI(argv[0]);
	st_rtc_time.month= ATOI(argv[1]);
	st_rtc_time.date = ATOI(argv[2]);
	st_rtc_time.day  = ATOI(argv[2]);//comer:ALI_S3503 do not set day member in driver!
	st_rtc_time.hour = ATOI(argv[3]);
	st_rtc_time.min  = ATOI(argv[4]);
	st_rtc_time.sec  = ATOI(argv[5]);

	/**  stop the audio decoder */
	AUI_TEST_CHECK_RTN(aui_rtc_set_time(g_p_hdl_rtc,&st_rtc_time));

	return AUI_RTN_SUCCESS;

}

unsigned long test_rtc_time_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_clock st_rtc_time;

	MEMSET(&st_rtc_time,0,sizeof(aui_clock));
	AUI_TEST_CHECK_NULL(g_p_hdl_rtc);
	AUI_TEST_CHECK_NULL(sz_out_put);

	AUI_TEST_CHECK_RTN(aui_rtc_get_time(g_p_hdl_rtc,&st_rtc_time));
	AUI_PRINTF("get RTC time is [year:%04d, month:%02d, date:%02d, hour:%02d, min:%02d, sec:%02d]\r\n",
				st_rtc_time.year,st_rtc_time.month,st_rtc_time.date,st_rtc_time.hour,st_rtc_time.min,st_rtc_time.sec);
	return AUI_RTN_SUCCESS;

}

unsigned long test_rtc_alm_config(unsigned long *argc,char **argv,char *sz_out_put)
{
	char ac_tmp[32]={0};
	aui_min_alarm min_alarm;
	aui_ms_alarm ms_alarm;
	aui_rtc_alarm_attr  alarm_info;

	AUI_TEST_CHECK_NULL(g_p_hdl_rtc);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,6);

	MEMSET(&min_alarm,0,sizeof(aui_min_alarm));
	MEMSET(&ms_alarm,0,sizeof(aui_ms_alarm));
	MEMSET(&alarm_info,0,sizeof(alarm_info));
	if(AUI_RTN_SUCCESS != aui_rtc_get_alarm_info(g_p_hdl_rtc,&alarm_info))
	{
		AUI_PRINTF("aui_rtc_get_alarm_info failed!\r\n");
		return 1;
	}

	AUI_TEST_CHECK_NOT_EQU_VAL(0,sscanf(argv[0],"%s",ac_tmp));
	if(0==strcmp(ac_tmp,"min"))
	{
		memset(&min_alarm, 0, sizeof(aui_min_alarm));
		min_alarm.month= ATOI(argv[2]);
		min_alarm.date = ATOI(argv[3]);
		min_alarm.hour = ATOI(argv[4]);
		min_alarm.min  = ATOI(argv[5]);
		alarm_num = ATOI(argv[1]);

		if(alarm_num >=alarm_info.min_alarm_num_start  &&  alarm_num <=alarm_info.min_alarm_num_end)
		{
			AUI_TEST_CHECK_RTN(aui_rtc_set_alarm_minutes(g_p_hdl_rtc,alarm_num,&min_alarm, (aui_rtc_alarm_callback *)aui_rtc_call_back_test));
			aui_rtc_alarm_switch(g_p_hdl_rtc,alarm_num,1);
			AUI_PRINTF("Set alarm[%d] %02d-%02d %02d:%02d\n",alarm_num,
					min_alarm.month, min_alarm.date, ms_alarm.hour, ms_alarm.min);
		}else{
			AUI_PRINTF("alarm_num[%d] is invalid!\r\n",alarm_num);
			AUI_PRINTF("alarm_num of minute level is in [%d-%d] range!\r\n",alarm_info.min_alarm_num_start,alarm_info.min_alarm_num_end);
			return 1;
		}
	}
	else if(0==strcmp(ac_tmp,"sec"))
	{
		memset(&ms_alarm, 0, sizeof(aui_min_alarm));
		ms_alarm.hour= ATOI(argv[2]);
		ms_alarm.min = ATOI(argv[3]);
		ms_alarm.sec= ATOI(argv[4]);
		ms_alarm.ms = ATOI(argv[5]);
		alarm_num = ATOI(argv[1]);
		if(alarm_num >=alarm_info.ms_alarm_num_start  &&  alarm_num <=alarm_info.ms_alarm_num_end)
		{
			AUI_TEST_CHECK_RTN(aui_rtc_set_alarm_ms(g_p_hdl_rtc,alarm_num,&ms_alarm, (aui_rtc_alarm_callback *)aui_rtc_call_back_test));
			aui_rtc_alarm_switch(g_p_hdl_rtc,alarm_num,1);
			AUI_PRINTF("Set alarm[%d] %02d:%02d:%02d %03dms\n",alarm_num,
					ms_alarm.hour, ms_alarm.min, ms_alarm.sec, ms_alarm.ms);
		}else{
			AUI_PRINTF("alarm_num[%d] is invalid!\r\n",alarm_num);
			AUI_PRINTF("alarm_num of ms level is in [%d-%d] range!\r\n",alarm_info.ms_alarm_num_start,alarm_info.ms_alarm_num_end);
			return 1;
		}
	}

	return AUI_RTN_SUCCESS;

}

unsigned long test_rtc_alm_on_off(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned char ul_enable=0;

	AUI_TEST_CHECK_NULL(g_p_hdl_rtc);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,2);

	alarm_num = ATOI(argv[0]);
	ul_enable = ATOI(argv[1]);
	AUI_TEST_CHECK_RTN(aui_rtc_alarm_switch(g_p_hdl_rtc,alarm_num,ul_enable));

	return AUI_RTN_SUCCESS;

}

#ifndef AUI_TDS
unsigned long rtc_count_auto_test(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_hdl rtc_hdl;
#if 0
	if(aui_rtc_init() != AUI_RTN_SUCCESS)
	{
		while(1)
			printf("%s,line %d,init rtc module fail!\n",__func__,__LINE__);
	}

	if(aui_rtc_open(&rtc_hdl) != AUI_RTN_SUCCESS)
	{
		while(1)
			printf("%s,line %d,init rtc module fail!\n",__func__,__LINE__);
	}
#endif
	rtc_hdl = g_p_hdl_rtc;
	AUI_PRINTF("%s,line %d,rtc_hdl = %d!\n",__func__,__LINE__,rtc_hdl);

	aui_clock set_value,read_value;
	memset(&set_value,0x00,sizeof(aui_clock));
	memset(&read_value,0x00,sizeof(aui_clock));
	set_value.year = 2014;
	set_value.month = 3;
	set_value.day = set_value.date = 5;
	set_value.hour = 16;
	set_value.min = 26;
	set_value.sec = 30;

	if(aui_rtc_set_time(rtc_hdl,&set_value) != AUI_RTN_SUCCESS)
	{
		while(1)
			AUI_PRINTF("%s line %d,set rtc time fail!\n",__func__,__LINE__);
	}


	int sec = 100;
	while (sec --)
	{
		if(aui_rtc_get_time(rtc_hdl,&read_value) != AUI_RTN_SUCCESS)
		{
			while(1)
				AUI_PRINTF("%s line %d,read rtc time fail!\n",__func__,__LINE__);
		}

		AUI_PRINTF("%s line %d,stc time: year = %d,month = %d,date = %d,day = %d,hour = %d,min = %d,sec = %d\n",__func__,__LINE__,read_value.year,read_value.month, \
			read_value.date,read_value.day,read_value.hour,read_value.min,read_value.sec);
		sleep(1);
	}
#if 0 // Not to close the handle according to context
	aui_rtc_close(rtc_hdl);
	aui_rtc_de_init();
#endif
	return 0;
}
#endif

unsigned long test_rtc_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nRTC Test Help");

	/* RTC_1_HELP */
	#define 	RTC_1_HELP_PART1		"Format:       1 [year],[mouth],[day],[hour],[minute],[second]"
	#define 	RTC_1_HELP_PART2		"Example:      If the RTC is set as 2014-12-31 23:59:0, the input is"
	#define 	RTC_1_HELP_PART3		"                        1 2014,12,31,23,59,0\n"
	#define 	RTC_1_HELP_PART4		"After being set, the RTC is running as the same time."
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Set the RTC time.\n");
	aui_print_help_instruction_newline(RTC_1_HELP_PART1);
	aui_print_help_instruction_newline(RTC_1_HELP_PART2);
	aui_print_help_instruction_newline(RTC_1_HELP_PART3);
	aui_print_help_instruction_newline(RTC_1_HELP_PART4);
	aui_print_help_instruction_newline("\r\n");

	/* RTC_2_HELP */
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("Get the current RTC time.");
	aui_print_help_instruction_newline("\r\n");


	/* RTC_3_HELP */
	#define 	RTC_3_HELP_PART1		"Format:       3 [<min>],[alarm index],[mouth],[day],[hour],[minute]"
	#define 	RTC_3_HELP_PART2		"              3 [<sec>],[alarm index],[hour],[minute],[second],[msec]"
	#define 	RTC_3_HELP_PART3		"              [alarm index]: the index of the alarm, corrent only 0 or 1 is chosen to input.\n"
	#define 	RTC_3_HELP_PART4		"Example:      If the alarm responses at 00:00 January 1st and the alarm index is 0, the input is"
	#define 	RTC_3_HELP_PART5		"                        3 min,0,1,1,0,0\n"
	#define 	RTC_3_HELP_PART6		"              If the alarm responses at 00:00:02 154ms and the alarm index is 1, the input is"
	#define 	RTC_3_HELP_PART7		"                        3 sec,0,0,0,2,145\n"
	#define 	RTC_3_HELP_PART8		"After being configured, the alarm callback function which output some string is called when the RTC is the time that the alarm set."
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline("Configurate the alarm.\n");
	aui_print_help_instruction_newline(RTC_3_HELP_PART1);
	aui_print_help_instruction_newline(RTC_3_HELP_PART2);
	aui_print_help_instruction_newline(RTC_3_HELP_PART3);
	aui_print_help_instruction_newline(RTC_3_HELP_PART4);
	aui_print_help_instruction_newline(RTC_3_HELP_PART5);
	aui_print_help_instruction_newline(RTC_3_HELP_PART6);
	aui_print_help_instruction_newline(RTC_3_HELP_PART7);
	aui_print_help_instruction_newline(RTC_3_HELP_PART8);
	aui_print_help_instruction_newline("\r\n");

	/* RTC_4_HELP */
	#define 	RTC_4_HELP_PART1		"Format:       4 [alarm index],[enable alarm]"
	#define 	RTC_4_HELP_PART2		"              [alarm index]: the index of the alarm, corrent only 0 or 1 is chosen to input."
	#define 	RTC_4_HELP_PART3		"              [enable alarm]: 1:on  0:off\n"
	#define 	RTC_4_HELP_PART4		"Example:      If the alarm0 is set on, the input is"
	#define 	RTC_4_HELP_PART5		"                        4 0,1"
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline("Turn on/off the alarm.\n");
	aui_print_help_instruction_newline(RTC_4_HELP_PART1);
	aui_print_help_instruction_newline(RTC_4_HELP_PART2);
	aui_print_help_instruction_newline(RTC_4_HELP_PART3);
	aui_print_help_instruction_newline(RTC_4_HELP_PART4);
	aui_print_help_instruction_newline(RTC_4_HELP_PART5);
	aui_print_help_instruction_newline("\r\n");

#ifndef AUI_TDS
	/* RTC_5_HELP */
	#define 	RTC_5_HELP		   "The RTC time is set to 2014.03.05 16:26:30. Then the RTC time is read every 1s for 100s"
	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline("Do the series test of the RTC function");
	aui_print_help_instruction_newline(RTC_5_HELP);
	aui_print_help_instruction_newline("\r\n");
#endif
	return AUI_RTN_HELP;

}

void test_rtc_reg()
{
	g_p_hdl_rtc=NULL;
	aui_find_dev_by_idx(AUI_MODULE_RTC, 0, &g_p_hdl_rtc);
	if(g_p_hdl_rtc == NULL){
		aui_rtc_init();
		aui_rtc_open(&g_p_hdl_rtc);
		//AUI_PRINTF("aui_rtc_open in function test_rtc_reg\n");
	}
	aui_tu_reg_group("rtc", "realtime clock test");

	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_rtc_time_set, "RTC time set test");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_rtc_time_get, "RTC time get test");

	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_rtc_alm_config, "RTC time config test");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_rtc_alm_on_off, "RTC time on/off test");
#ifndef AUI_TDS
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, rtc_count_auto_test, "rtc_count_auto_test");
#endif
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_rtc_help, "RTC help");
}

