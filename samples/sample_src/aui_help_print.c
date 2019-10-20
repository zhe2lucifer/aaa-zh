/****************************************************************
 *  @file               aui_help_print.c
 *  @brief              The interfaces for help print
 *
 *  @version            1.0
 *  @date               10/23/2014
 *  @revision           none
 *
 *  @author             Harlin.Wu  
 ****************************************************************/
 
#include "aui_help_print.h"

void aui_print_help_header(char *string)
{
        if(NULL == string)
        {
                return;
        }
        
        AUI_PRINTF("\r\n\033[1m %s  ", string);
        AUI_PRINTF("\r\n\r\nCOMMAND\033[0m");
}
 
void aui_print_help_command(char *string)
{
        if(NULL == string)
        {
                return;
        }

        AUI_PRINTF("\r\n\t\033[1m%s:\033[0m", string);
}
 
void aui_print_help_instruction(char *string)
{
        if(NULL == string)
        {
                return;
        }
        
        AUI_PRINTF("%s", string);
}

void aui_print_help_instruction_newline(char *string)
{
        if(NULL == string)
        {
                return;
        }
        
        AUI_PRINTF("\r\n\t      %s", string);
}


