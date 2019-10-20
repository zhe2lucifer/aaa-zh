
/****************************INCLUDE FILE***********************************/
#include <stdlib.h>
#include <aui_panel.h>
#include "aui_panel_test.h"
#include "aui_help_print.h"
#ifdef AUI_LINUX
#include <signal.h>
#include <unistd.h>
#endif
/****************************GLOBAL MACRO***********************************/

/****************************GLOBAL TYPE************************************/

/****************************TEST MODULE IMPLEMENT**************************/

#define TEST_PANEL_FUNC 1

aui_hdl g_p_hdl_panel;

const struct aui_pannel_key_map ali_pankey_map[] =
{
	{(0xffff0000),		116/*KEY_POWER*/},
	{(0xffff0001),		28/*KEY_ENTER*/},
	{(0xffff0002),		103/*KEY_UP*/},
	{(0xffff0003),		108/*KEY_DOWN*/},
	{(0xffff0004),		105/*KEY_LEFT*/},
	{(0xffff0005),		106/*KEY_RIGHT*/},
	{(0xffff0006),		139/*KEY_MENU*/},
};

unsigned long test_panel_display(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_panel));

    AUI_TEST_CHECK_RTN(aui_panel_display(g_p_hdl_panel, AUI_PANEL_DATA_ANSI, "12:34", 5));

    if(!aui_test_user_confirm("do you see the panel display \"1234\" ? ")) {
        return AUI_RTN_FAIL;
    }
#endif
    return AUI_RTN_SUCCESS;
}

unsigned long test_panel_clear(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_panel));

    AUI_TEST_CHECK_RTN(aui_panel_clear(g_p_hdl_panel, AUI_PANEL_DATA_ANSI));

    if(!aui_test_user_confirm("do you see the panel has no display? ")) {
        return AUI_RTN_FAIL;
    }
#endif
    return AUI_RTN_SUCCESS;
}


unsigned long test_panel_display_lock_led(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_panel));

    AUI_TEST_CHECK_RTN(aui_panel_display(g_p_hdl_panel, AUI_PANEL_CMD_LED_LOCK, NULL, 0));

    if(!aui_test_user_confirm("do you see the lock leg on ? ")) {
        return AUI_RTN_FAIL;
    }
#endif
    return AUI_RTN_SUCCESS;
}

unsigned long test_panel_set_led(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_panel));
	if(*argc != 2) {
		AUI_PRINTF("Input parameter error,Please see help!\n");
        return AUI_RTN_EINVAL;
    }
    unsigned char led_num = atoi(argv[0]);
    unsigned char led_val = atoi(argv[1]);
    AUI_TEST_CHECK_RTN(aui_panel_set_led_state(g_p_hdl_panel,led_num,led_val));

    if(!aui_test_user_confirm("do you see the lock leg on ? ")) {
        return AUI_RTN_FAIL;
    }
#endif
    return AUI_RTN_SUCCESS;
}

unsigned long test_panel_clear_lock_led(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC


    AUI_TEST_CHECK_NULL((g_p_hdl_panel));

    AUI_TEST_CHECK_RTN(aui_panel_clear(g_p_hdl_panel, AUI_PANEL_CMD_LED_LOCK));

    if(!aui_test_user_confirm("do you see the lock leg on ? ")) {
        return AUI_RTN_FAIL;
    }
#endif
    return AUI_RTN_SUCCESS;
}

#ifdef AUI_LINUX
unsigned long test_panel_set_key_map(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_TEST_CHECK_NULL((g_p_hdl_panel));

    struct aui_pannel_key_map_cfg panel_cfg;
    panel_cfg.map_entry = (struct aui_pannel_key_map *)ali_pankey_map;
    panel_cfg.unit_num = sizeof(ali_pankey_map) / sizeof(struct aui_pannel_key_map);
	AUI_PRINTF("aui_key_set_panel_map\n");
	AUI_TEST_CHECK_RTN(aui_panel_set_key_map(g_p_hdl_panel, &panel_cfg));
    AUI_TEST_CHECK_RTN(aui_panel_set_key_rep_interval(g_p_hdl_panel, 1000, 1000));
    return AUI_RTN_SUCCESS;
}
#endif

unsigned long test_panel_help(unsigned long *argc, char **argv, char *sz_out_put)
{
    aui_print_help_header("\nPANEL Test Help");


    /* PANEL_1_HELP */
#define     PANEL_1_HELP         "The <12:34> string will display on the panel if the test is successful."
    aui_print_help_command("\'1\'");
    aui_print_help_instruction_newline("Test the panel's display");
    aui_print_help_instruction_newline(PANEL_1_HELP);
    aui_print_help_instruction_newline("\r\n");

    /* PANEL_2_HELP */
#define     PANEL_2_HELP         "The panel displays nothing"
    aui_print_help_command("\'2\'");
    aui_print_help_instruction_newline("Close the panel's display");
    aui_print_help_instruction_newline(PANEL_2_HELP);
    aui_print_help_instruction_newline("\r\n");

    /* PANEL_3_HELP */
#define     PANEL_3_HELP         "The power led with oringe will be turned on if the test is successful."
    aui_print_help_command("\'3\'");
    aui_print_help_instruction_newline("Turn on the power led");
    aui_print_help_instruction_newline(PANEL_3_HELP);
    aui_print_help_instruction_newline("\r\n");

    /* PANEL_4_HELP */
#define     PANEL_4_HELP         "The power led is turned off."
    aui_print_help_command("\'4\'");
    aui_print_help_instruction_newline("Turn off the power led");
    aui_print_help_instruction_newline(PANEL_4_HELP);
    aui_print_help_instruction_newline("\r\n");

    /* PANEL_5_HELP */
#define     PANEL_5_HELP       	"command format:  5 [led_number],[output_led_value]\n"\
                                "              led_number: 0 ----> LOCK_LED1\n"\
							    "                          1 ----> LOCK_LED2\n"\
							    "                          2 ----> LOCK_LED3\n"\
							    "                          3 ----> LOCK_LED4\n"\
							    "              output_led_value: 0 ----> LED off\n"\
							    "                                1 ----> LED light\n"\
							    "              for example:5 0,1"

    aui_print_help_command("\'5\'");
    aui_print_help_instruction_newline("Set led light and off");
    aui_print_help_instruction_newline(PANEL_5_HELP);
    aui_print_help_instruction_newline("\r\n");
    
    #ifdef AUI_LINUX
    /* PANEL_8_HELP */
#define     PANEL_8_HELP       	"set pannel key map"

    aui_print_help_command("\'8\'");
    aui_print_help_instruction_newline("set pannel key map");
    aui_print_help_instruction_newline(PANEL_8_HELP);
    aui_print_help_instruction_newline("\r\n");
#endif

    return AUI_RTN_HELP;

}

#ifdef AUI_LINUX
static unsigned int sub_pid;
unsigned long test_panel_multiprocess(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC
	sub_pid = fork();
	if(sub_pid)/*patent process return 0,sub-process continue running*/
		return 0;
	g_p_hdl_panel=NULL;
	if(g_p_hdl_panel == NULL) {
		aui_panel_open(0, NULL, &g_p_hdl_panel);
	}
    AUI_TEST_CHECK_NULL((g_p_hdl_panel));
	unsigned char led_num = 0;
	unsigned char led_val = 0;
	int i;
	while(1)
	{
		for(i = 0; i < 4; i++)
		{
			AUI_TEST_CHECK_RTN(aui_panel_set_led_state(g_p_hdl_panel, i, led_val));			
		}
		led_val = (led_val)?0:1;
		sleep(5);
	}
#endif
    return AUI_RTN_SUCCESS;
}
unsigned long test_panel_kill_process(unsigned long *argc, char **argv, char *sz_out_put)
{
#if TEST_PANEL_FUNC
	if(0 == sub_pid)
		return 0;
	if( kill(sub_pid,SIGKILL)<0)
		AUI_PRINTF("FUNC:%s,LINE:%d,kill() fail\n",__func__,__LINE__);
	sub_pid = 0;
#endif
    return AUI_RTN_SUCCESS;
}
#endif
void test_panel_reg()
{
    g_p_hdl_panel=NULL;
    aui_find_dev_by_idx(AUI_MODULE_PANEL, 0, &g_p_hdl_panel);
    if(g_p_hdl_panel == NULL) {
        aui_panel_open(0, NULL, &g_p_hdl_panel);
    }
    aui_tu_reg_group("panel", "panel test");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_panel_display, "display string");
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_panel_clear, "clear panel");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_panel_display_lock_led, "lock led");
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_panel_clear_lock_led, "clear led");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_panel_set_led, "set led light and off");
#ifdef AUI_LINUX
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_panel_multiprocess, "start another process,loops display led");
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_panel_kill_process, "kill type '6' command create process");
    aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, test_panel_set_key_map, "set pannel key map");
#endif
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_panel_help, "panel help");
}


