/****************************************************************
 *  @file               aui_help_print.h
 *  @brief              head file of aui_help_print.c
 *
 *  @version            1.0
 *  @date               10/23/2014
 *  @revision           none
 *
 *  @author             Harlin.Wu  
 ****************************************************************/

#ifndef _AUI_HELP_PRINT_H
#define _AUI_HELP_PRINT_H

#include "aui_test_app.h"
#ifdef __cplusplus
extern "C" {
#endif

extern void aui_print_help_header(char *string);
extern void aui_print_help_command(char *string);
extern void aui_print_help_instruction(char *string);
extern void aui_print_help_instruction_newline(char *string);

#ifdef __cplusplus
}
#endif

#endif

