/****************************INCLUDE HEAD FILE************************************/
#include "aui_uart_test.h"
#include "aui_help_print.h"

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************EXTERN VAR*******************************************/
//extern aui_handle_deca *g_p_hdl_deca;
/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/

#define TEST_UART_FUNC 1

aui_hdl g_p_hdl_uart;
unsigned long test_uart_read(unsigned long *argc,char **argv,char *sz_out_put)
{
#if TEST_UART_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_uart));
	//AUI_TEST_CHECK_NULL(sz_out_put);

#ifdef _RD_DEBUG_
    char buf[256] = {0};
#endif
    int index = 0;
    unsigned long readed_len = 0;
    char ch = 0;
    aui_uart_clear(g_p_hdl_uart);
	AUI_PRINTF("\nyou should input char in 10 second, or the uart_read will timeout\n");
	AUI_PRINTF(" input Enter key to quit the test!!!\n");
    while(1)
    {
        readed_len = 0;
		AUI_TEST_CHECK_RTN(aui_uart_enable(g_p_hdl_uart, AUI_UART_READ_WRITE));
		//timeout=0 means return right now
		if(AUI_RTN_SUCCESS != aui_uart_read(g_p_hdl_uart,&ch,1,&readed_len,0))
		{
			AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart, AUI_UART_READ_WRITE));
			AUI_PRINTF("aui_uart_read fail\n");
		}
		else
		{
			AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart, AUI_UART_READ_WRITE));
		}
        if(0xd == ch) //enter key
        {
            break;
        }
        if(readed_len > 0)
        {
#ifdef _RD_DEBUG_
            buf[index++] = ch;
#endif
            AUI_PRINTF("you had input : %c\n\n",ch);
        }
    }
#ifdef _RD_DEBUG_
    buf[index] = 0;
    AUI_PRINTF("\nreceive input: %s \n",buf);
#endif

    aui_uart_clear(g_p_hdl_uart);

    if(!aui_test_user_confirm("do you have see right sentense? "))
	{
		return AUI_RTN_FAIL;
	}
#endif

	return AUI_RTN_SUCCESS;
}


unsigned long test_uart_write(unsigned long *argc,char **argv,char *sz_out_put)
{
#if TEST_UART_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_uart));
	//AUI_TEST_CHECK_NULL(sz_out_put);
    unsigned long writed_len = 0;
	char buf[64] = {"uart test string for write"};

	AUI_PRINTF("\n");
	AUI_TEST_CHECK_RTN(aui_uart_enable(g_p_hdl_uart, AUI_UART_READ_WRITE));
	if(AUI_RTN_SUCCESS != aui_uart_write(g_p_hdl_uart,buf,STRLEN(buf),&writed_len,0))
	{
		AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart, AUI_UART_READ_WRITE));
		AUI_PRINTF("aui_uart_write fail\n");
	}
	else
	{
		AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart, AUI_UART_READ_WRITE));
	}
	AUI_PRINTF("\n");
    if(!aui_test_user_confirm("do you have see \"uart test string for write\" ? "))
	{
		return AUI_RTN_FAIL;
	}
#endif
	return AUI_RTN_SUCCESS;
}

unsigned long test_uart_disable_write(unsigned long *argc,char **argv,char *sz_out_put)
{
#if TEST_UART_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_uart));
	//AUI_TEST_CHECK_NULL(sz_out_put);
    unsigned long writed_len = 0;
	char buf[64] = {"This string should not show the screen!"};

    AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart,AUI_UART_READ_WRITE));
#ifdef AUI_TDS
	AUI_TEST_CHECK_RTN(!aui_uart_write(g_p_hdl_uart,buf,STRLEN(buf),&writed_len,0));
#else
	AUI_TEST_CHECK_RTN(aui_uart_write(g_p_hdl_uart,buf,STRLEN(buf),&writed_len,0));
#endif
    if(!aui_test_user_confirm("didn't see anything printed on screen? "))
    {
	    return AUI_RTN_FAIL;
    }

#endif
	return AUI_RTN_SUCCESS;
}

unsigned long test_uart_enable_write(unsigned long *argc,char **argv,char *sz_out_put)
{
#if TEST_UART_FUNC
    AUI_TEST_CHECK_NULL((g_p_hdl_uart));
	//AUI_TEST_CHECK_NULL(sz_out_put);
    char buf[64] = {"hello from test item"};

    AUI_TEST_CHECK_RTN(aui_uart_enable(g_p_hdl_uart,AUI_UART_READ_WRITE));
	if(AUI_RTN_SUCCESS != aui_uart_write(g_p_hdl_uart,buf,STRLEN(buf),NULL,0))
	{
			AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart, AUI_UART_READ_WRITE));
			AUI_PRINTF("aui_uart_write fail\n");
	}
	else
	{
		AUI_TEST_CHECK_RTN(aui_uart_disable(g_p_hdl_uart, AUI_UART_READ_WRITE));
	}

    if(!aui_test_user_confirm("do you see \"hello from test item\" ? "))
	{
		return AUI_RTN_FAIL;
	}
#endif
	return AUI_RTN_SUCCESS;
}

unsigned long test_uart_help(unsigned long *argc,char **argv,char *sz_out_put)
{
        aui_print_help_header("\nUART Test Help");

        /* UART_1_HELP */
        #define     UART_1_HELP         "If the <123abc> charactors string are input, the UART will return <123abc> that can be observed in the terminal."
        aui_print_help_command("\'1\'");
        aui_print_help_instruction_newline("Test the UART reading function");
        aui_print_help_instruction_newline("For example:");
        aui_print_help_instruction_newline(UART_1_HELP);
        aui_print_help_instruction_newline("\r\n");

        /* UART_2_HELP */
        #define     UART_2_HELP         "The charactor string <hello from test item> is returned from the UART if the test is successful."
        aui_print_help_command("\'2\'");
        aui_print_help_instruction_newline("Test the UART writing function");
        aui_print_help_instruction_newline(UART_2_HELP);
        aui_print_help_instruction_newline("\r\n");

        /* UART_3_HELP */
        #define     UART_3_HELP         "After disabling the UART, The charactor string <hello from test item> is not returned from the UART if the test is successful."
        aui_print_help_command("\'3\'");
        aui_print_help_instruction_newline("Disable the UART test");
        aui_print_help_instruction_newline(UART_3_HELP);
        aui_print_help_instruction_newline("\r\n");

        /* UART_4_HELP */
        #define     UART_4_HELP         "After enabling the UART, the charactor string <hello from test item> is returned from the UART if the test is successful."
        aui_print_help_command("\'4\'");
        aui_print_help_instruction_newline("enable the UART test");
        aui_print_help_instruction_newline(UART_4_HELP);
        aui_print_help_instruction_newline("\r\n");

        return AUI_RTN_HELP;

}

void test_uart_reg()
{
	g_p_hdl_uart=NULL;
	aui_find_dev_by_idx(AUI_MODULE_UART, 0, &g_p_hdl_uart);
	if(g_p_hdl_uart == NULL){
		aui_uart_open(0, NULL,&g_p_hdl_uart);
	}
	aui_tu_reg_group("uart", "uart test");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_uart_read, "read charator from uart");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_uart_write, "write charator to uart");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_uart_disable_write, "disable uart");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_uart_enable_write, "enable uart");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_uart_help, "uart help");
}

