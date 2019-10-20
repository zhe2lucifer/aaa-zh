/****************************INCLUDE HEAD FILE************************************/
#include "aui_dog_test.h"
#include "aui_help_print.h"

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

aui_hdl g_p_hdl_dog;
/****************************LOCAL FUNC DECLEAR***********************************/
void aui_dog_call_back_test(unsigned long ul_time)
{
    aui_dog_time_set(g_p_hdl_dog, 0);
#ifdef _RD_DEBUG_
	ul_time=10;
	AUI_PRINTF("\r\n_this is the watch dog callback function for test[%d].",ul_time);
#else
	(void)ul_time;
#endif
}
/****************************EXTERN VAR*******************************************/

/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/


unsigned long test_dog_start(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_dog attr_dog;

	//if(NULL==g_p_hdl_dog->pul_dev_dog)
	//{
	//	aui_rtn(AUI_MODULE_DECA,DECA_ERR,NULL);
	//}
	//AUI_TEST_CHECK_NULL(g_p_hdl_dog->pul_dev_dog);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  start up the audio decoder */
	AUI_TEST_CHECK_RTN(aui_dog_start(g_p_hdl_dog,&attr_dog));

	return AUI_RTN_SUCCESS;
}

unsigned long test_dog_stop(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_dog attr_dog;

	//AUI_TEST_CHECK_NULL(g_p_hdl_dog->pul_dev_dog);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  stop the audio decoder */
	AUI_TEST_CHECK_RTN(aui_dog_stop(g_p_hdl_dog,&attr_dog));

	return AUI_RTN_SUCCESS;

}

unsigned long test_dog_time_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_time=0;

	//AUI_TEST_CHECK_NULL(g_p_hdl_dog->pul_dev_dog);
	AUI_TEST_CHECK_NULL(sz_out_put);

    if (*argc > 0)
	    ul_time = ATOI(argv[0]);

	AUI_PRINTF("\r\n_watch dog set time is %ld us.\n",ul_time);
	AUI_TEST_CHECK_RTN(aui_dog_time_set(g_p_hdl_dog,ul_time));

	return AUI_RTN_SUCCESS;

}

unsigned long test_dog_time_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_time=0;

	//AUI_TEST_CHECK_NULL(g_p_hdl_dog->pul_dev_dog);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  stop the audio decoder */
	AUI_TEST_CHECK_RTN(aui_dog_time_get(g_p_hdl_dog,&ul_time));
	AUI_PRINTF("\r\n_watch dog get time is %ld us.",ul_time);
	return AUI_RTN_SUCCESS;

}

unsigned long test_dog_config(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_dog attr_dog;
	unsigned long ul_work_mode=0;
	unsigned long ul_time_us=0;

	MEMSET(&attr_dog,0,sizeof(attr_dog));

	//AUI_TEST_CHECK_NULL(g_p_hdl_dog->pul_dev_dog);
	AUI_TEST_CHECK_NULL(sz_out_put);

	if (*argc > 1){
	    ul_work_mode = ATOI(argv[0]);
	    ul_time_us = ATOI(argv[1]);
	}

	attr_dog.ul_work_mode=ul_work_mode;
	attr_dog.ul_time_us=ul_time_us;
	attr_dog.dog_cb=aui_dog_call_back_test;

	AUI_TEST_CHECK_RTN(aui_dog_config(g_p_hdl_dog,&attr_dog));

	return AUI_RTN_SUCCESS;

}

unsigned long test_dog_help(unsigned long *argc,char **argv,char *sz_out_put)
{
        aui_print_help_header("\nWatch dog Test Help");

        /* DOG_1_HELP */
        #define     DOG_1_HELP         "The watch dog increases the count at the frequency defined in CLKDIV field (that can be found in the chip datasheet)."
        aui_print_help_command("\'1\'");
        aui_print_help_instruction_newline("Start Watch dog counting");
        aui_print_help_instruction_newline(DOG_1_HELP);
        aui_print_help_instruction_newline("\r\n");

        /* DOG_2_HELP */
        #define     DOG_2_HELP         "The watch dog stops counting and stays at the value when the watch dog stops."
        aui_print_help_command("\'2\'");
        aui_print_help_instruction_newline("Stop the Watch dog counting");
        aui_print_help_instruction_newline(DOG_2_HELP);
        aui_print_help_instruction_newline("\r\n");

        /* DOG_3_HELP */
        #define     DOG_3_HELP_PART1         "Format:       3 [timeout value(us)]\n"
        #define     DOG_3_HELP_PART2         "Example:      If 10000us is set as the watch dog's timeout value, the input is"
        #define     DOG_3_HELP_PART3         "                        3 10000"
        aui_print_help_command("\'3\'");
        aui_print_help_instruction_newline("Set the watch dog's timeout value\n");
        aui_print_help_instruction_newline(DOG_3_HELP_PART1);
        aui_print_help_instruction_newline(DOG_3_HELP_PART2);
        aui_print_help_instruction_newline(DOG_3_HELP_PART3);
        aui_print_help_instruction_newline("\r\n");

        /* DOG_4_HELP */
        aui_print_help_command("\'4\'");
        aui_print_help_instruction_newline("Get the current time to reach timeout of watch dog");
        aui_print_help_instruction_newline("\r\n");

        /* DOG_5_HELP */
        #define     DOG_5_HELP_PART1         "Format:       5 [mode],[timeout value(us)]"
        #define     DOG_5_HELP_PART2         "                [mode]: 0 - as watch dog. When the watch dog timer is timeout, the system is reset. The timeout callback function is not called."
        #define     DOG_5_HELP_PART3         "                        1 - only as timer. When the watch dog timer is timeout, the system is not reset. The timeout callback function is called.\n"
        #define     DOG_5_HELP_PART4         "Example:      If 10000us is set as the watch dog's timeout value and the watch dog mode is set, the input is"
        #define     DOG_5_HELP_PART5         "                        5 0,10000"
        aui_print_help_command("\'5\'");
        aui_print_help_instruction_newline("Re-config and re-start the watch dog\n");
        aui_print_help_instruction_newline(DOG_5_HELP_PART1);
        aui_print_help_instruction_newline(DOG_5_HELP_PART2);
        aui_print_help_instruction_newline(DOG_5_HELP_PART3);
        aui_print_help_instruction_newline(DOG_5_HELP_PART4);
        aui_print_help_instruction_newline(DOG_5_HELP_PART5);

        return AUI_RTN_HELP;

}


void test_dog_reg()
{
	g_p_hdl_dog=NULL;
	aui_attr_dog attr_dog;
	memset(&attr_dog, 0, sizeof(aui_attr_dog));

	aui_find_dev_by_idx(AUI_MODULE_DOG, 0, &g_p_hdl_dog);
	if(g_p_hdl_dog == NULL){
		aui_dog_open(&attr_dog, &g_p_hdl_dog);
	}
	aui_tu_reg_group("dog", "watch dog test");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_dog_start, "watch dog start test");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_dog_stop, "watch dog stop test");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_dog_time_set, "watch dog time set test");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_dog_time_get, "watch dog time get test");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_dog_config, "watch dog config test");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_dog_help, "watch dog help");
}

