#ifndef __AUI_TEST_APP_CMD__
#define __AUI_TEST_APP_CMD__
#include <aui_common.h>

#ifdef AUI_TDS
#include <sys_config.h>
#include "aui_os_test.h"
#else

#endif

#define AUI_TEST_APP_VER	"1.0.0"

#define UART_CMD_OUTPUT_LEN_MAX	(1024*2)
#define USER_CMD_COUNT_MAX		(512)
#define UART_CMD_ID_LEN_MAX		(20)
#define UART_CMD_INSTRUCT_LEN_MAX		(128)
#define USER_CMD_PARAM_STR_LEN_MAX (128)

#define USER_CMD_PARAM_COUNT_MAX	(18)


#define UART_CMD_STR_LEN_MAX	(UART_CMD_ID_LEN_MAX+USER_CMD_PARAM_COUNT_MAX*USER_CMD_PARAM_STR_LEN_MAX+10)
							
typedef struct st_aui_cmd
{
	char ac_cmdID[UART_CMD_ID_LEN_MAX+1];
	unsigned long ui_cmd_param_cnt;
	char *argv[USER_CMD_PARAM_COUNT_MAX];
	char ac_cmd_out_put[UART_CMD_OUTPUT_LEN_MAX+1];

}st_aui_cmd;

typedef enum en_aui_cmd_type
{
	AUI_CMD_TYPE_MENU=0,
	AUI_CMD_TYPE_API,
	AUI_CMD_TYPE_UNIT,
	AUI_CMD_TYPE_SYS
}en_aui_cmd_type;


typedef struct st_aui_cmd_node
{
	unsigned char by_menu_level;												//the level of menu item
	char ac_cmdID[UART_CMD_ID_LEN_MAX+1];							//the command of menu item 
	unsigned long ul_cmd_type;												//the command type 
	unsigned long (*cmd_handle)(unsigned long *argc,char **argv,char *sz_out_put);	//the command parameter,aui system no need
	char ac_cmd_instuction[UART_CMD_INSTRUCT_LEN_MAX+1];				//the description of test item
	
}st_aui_cmd_node;

typedef struct st_aui_cmd_cur_status
{
	unsigned char by_menu_level;
	unsigned long ui_item_idx;
	unsigned long ui_total_idx;
	
}st_aui_cmd_cur_status;

unsigned long test_cmd(unsigned long *argc, char **argv,char *sz_out_put);
unsigned long aui_str_cmd_print_main_menu();
unsigned long aui_str_cmd_print_cur_menu();
unsigned long aui_str_cmd_ent_and_print_upper_menu();
unsigned long aui_str_cmd_quit();
unsigned long aui_str_cmd_run_level();

unsigned long aui_str_cmd_print_menu(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long aui_tu_reg_group(char *psz_group_name, char *psz_group_instruction);


unsigned long aui_tu_reg_item(unsigned long ul_menu_level,
			      char *psz_group_name,
			      unsigned long ul_cmd_type,
			      unsigned long (*cmd_handle)(unsigned long *argc,char **argv,char *sz_out_put),
			      char *psz_group_instruction);
#endif
