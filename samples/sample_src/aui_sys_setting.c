/****************************INCLUDE HEAD FILE************************************/
#include "aui_sys_setting.h"
#include "aui_help_print.h"

/****************************LOCAL MACRO******************************************/
#define ALL_MODULE  0xFFFF
#define LOG_LEVEL_CNT_MAX  4 // only support 4 level
/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/


#define AUI_MOD_DEF(x, level) #x,
#define SL_MOD(x)


static char *g_sys_aui_module_name[] = {
#include "aui_modules.def"
};

static char *g_sys_log_level[] = {
    "ERR",  // 3, 
    "WARN", // 4
    "INFO", // 6
    "DBG"   // 7
};

/****************************LOCAL FUNC DECLEAR***********************************/

unsigned long test_sys_log_level_set(unsigned long *argc,char **argv,char *sz_out_put)
{
    int module = -1;
    int i;
    int prio = -1;
    

    if (STRCMP(argv[0], "all") == 0) {
        module = ALL_MODULE;
    } else {
        for (i=0; i<AUI_MODULE_LAST; i++) {
            if (STRCMP(argv[0], g_sys_aui_module_name[i]) == 0)
                break;
        }

        if (i == AUI_MODULE_LAST) {
            // ERROR, not found module name
            AUI_PRINTF("module name not found: %s\n", argv[0]);
            return AUI_RTN_FAIL;
        }

        module = i;
    }

    for (i=0; i<LOG_LEVEL_CNT_MAX; i++) {
        if (STRCMP(argv[1], g_sys_log_level[i]) == 0)
            break;
    }

    if (i == LOG_LEVEL_CNT_MAX) {
        AUI_PRINTF("Not supported level: %s\n", argv[1]);
        return AUI_RTN_FAIL;
    }

    switch (i) {
        case 0:
            prio = 3;//LOG_ERR;
            break;
        case 1:
            prio = 4;//LOG_WARNING;
            break;
        case 2:
            prio = 6;//LOG_INFO;
            break;
        case 3:
            prio = 7;//LOG_DEBUG;
            break;
        default:
            AUI_PRINTF("Not supported level\n");
            break;
    }

    if (prio < 0) {
        return AUI_RTN_FAIL;
    }
    
    if (module == ALL_MODULE) {
        for (i=0; i<AUI_MODULE_LAST; i++) {
            aui_log_priority_set(i,prio);
        }
    } else  {
        aui_log_priority_set(module,prio);
    }


	return AUI_RTN_SUCCESS;

}

unsigned long test_sys_setting_help(unsigned long *argc,char **argv,char *sz_out_put)
{
    int i;
    
    aui_print_help_header("\nSample code test setting");

    aui_print_help_command("\'log\'");
    aui_print_help_instruction_newline("Use to set log level of AUI module");
    aui_print_help_instruction_newline(" ");
    aui_print_help_instruction_newline("Format:     log module_name,level");
    aui_print_help_instruction_newline("Example:    log DMX,WARN");
    aui_print_help_instruction_newline("\n");
    aui_print_help_instruction_newline("If you want to set the same level to all modues, the module_name is all");
    aui_print_help_instruction_newline(" ");
    aui_print_help_instruction_newline("Format:     log all,level");
    aui_print_help_instruction_newline("Example:    log all,DBG");
    aui_print_help_instruction_newline("\n");

    aui_print_help_instruction_newline("Module list:\n\t\t");

    for (i=0; i<AUI_MODULE_LAST; i++) {
        AUI_PRINTF("%16s", g_sys_aui_module_name[i]);
        if (((i+1) %8)== 0)
            AUI_PRINTF("\n\t\t");
    }
    AUI_PRINTF("\n");

    aui_print_help_instruction_newline("Log level:\n");
    for (i=0; i<LOG_LEVEL_CNT_MAX; i++) {
        AUI_PRINTF("\t\t%s\n", g_sys_log_level[i]);
    }

    aui_print_help_instruction_newline("\r\n");

    return AUI_RTN_HELP;

}


void test_sys_setting_reg(void)
{
	aui_tu_reg_group("sys", "Sample code test setting");
	aui_tu_reg_item(2, "log", AUI_CMD_TYPE_API, test_sys_log_level_set, "Set module's log level");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_sys_setting_help, "System setting's help");
}

