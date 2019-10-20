/**@file
*@brief the aui test application command define here
*@author ray.gong
*@date 2013-4-8
*@version 1.0.0
*@note 
*ali corp. all rights reserved. 2002 copyright (C)
* input file detail description here
* input file detail description here
*/

#ifdef AUI_TDS
#include <types.h>
#include <sys_config.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <api/libc/string.h>
#endif

#include <string.h>
#include <stdio.h>
#include <aui_common.h>
#include "aui_test_app.h"

unsigned char aui_test_user_confirm(char *sz_info);
void drv_set_aui_cmd_task_status(unsigned long bl_run);

/************************LOCAL MACRO**********************************/

/************************EXTERN VAR**********************************/

/************************GLOBAL VAR**********************************/
st_aui_cmd g_st_aui_cmd_str;
st_aui_cmd_cur_status	g_st_aui_cmd_cur_status;
st_aui_cmd_node *g_pst_aui_cmd_tree=NULL;


#define UART_CMD_OUTPUT_LEN_MAX_T 512
#define UART_CMD_OUTPUT_COUNT 20
#ifdef AUI_TDS
#define tsleep(x) osal_task_sleep(100*x)
#else
#define tsleep(x) usleep(x*1000)
#endif



unsigned long aui_format_print_help(char *sz_dst_buf)
{
	char ac_tmp[UART_CMD_OUTPUT_LEN_MAX_T+1]={0};
	strcpy(ac_tmp,"\r\n                   run : run the unit test item of this level menu");
	strcat(sz_dst_buf,ac_tmp);
	strcpy(ac_tmp,"\r\n                    up : upper menu");
	strcat(sz_dst_buf,ac_tmp);
	strcpy(ac_tmp,("\r\n                  root : root menu"));
	strcat(sz_dst_buf,ac_tmp);
	strcpy(ac_tmp,"\r\n                  quit : quit application");
	strcat(sz_dst_buf,ac_tmp);
	strcpy(ac_tmp,"\r\n");
	strcat(sz_dst_buf,ac_tmp);
	return AUI_RTN_CODE_SUCCESS;
}


unsigned long aui_format_print_menu(char *sz_dst_buf,char *sz_cmd,char *sz_cmd_instruction,unsigned long ul_cmd_wide)
{
	int i=0;
	
	if((NULL==sz_cmd)||(NULL==sz_cmd_instruction)||(NULL==sz_dst_buf)) {
		return AUI_RTN_CODE_ERR;
	}
	strcat(sz_dst_buf,"\r\n");
	for(i=0;i<(int)(ul_cmd_wide-strlen(sz_cmd));i++) {
		strcat(sz_dst_buf," ");
		//AUI_PRINTF(" ");
	}
	strcat(sz_dst_buf,sz_cmd);
	strcat(sz_dst_buf," : ");
	strcat(sz_dst_buf,sz_cmd_instruction);
	//AUI_PRINTF("%s : ",sz_cmd);
	//AUI_PRINTF("%s",sz_cmd_instruction);
	
	return AUI_RTN_CODE_SUCCESS;
}


/*-------------------------------------------------------------------------
function name: aui_str_cmd_print_main_menu
function description: test aui1.x RAM TO RAM encrypt engine command.
author: ray.gong (2012-11-10)
input parameter: NONE;
				 
output parameter:NONE
return value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
modify history:
-------------------------------------------------------------------------*/

unsigned long aui_str_cmd_print_main_menu()
{
	unsigned long i=0;
	unsigned long output_count = 0;
	char ac_tmp[UART_CMD_OUTPUT_LEN_MAX_T+1]={0};
	char *sz_out_put=NULL;
	char sz_out_put_temp[UART_CMD_OUTPUT_COUNT][UART_CMD_OUTPUT_LEN_MAX_T+1];
	for(i=0;i<UART_CMD_OUTPUT_COUNT;i++){
		MEMSET(sz_out_put_temp[i],0,(UART_CMD_OUTPUT_LEN_MAX_T+1));
	}
	sz_out_put = sz_out_put_temp[output_count];
    
	MEMSET(&g_st_aui_cmd_cur_status,0,sizeof(st_aui_cmd_cur_status));
	g_st_aui_cmd_cur_status.by_menu_level=1;

	strcpy(ac_tmp,"\r\n                     0 : print menu");
	strcat(sz_out_put,ac_tmp);
	for(i=0;
		((i<USER_CMD_COUNT_MAX)
		&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"NULL"))
		&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"")));i++)	{
		if(g_pst_aui_cmd_tree[i].by_menu_level==1) {
			MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX_T+1);
			aui_format_print_menu(ac_tmp,g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction,UART_CMD_ID_LEN_MAX+2);
			//sprintf(ac_tmp,"\r\n-%s- %s",g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction);
			if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX_T)	{
				strcat(sz_out_put,ac_tmp);
			}else{
				output_count++;
				sz_out_put = sz_out_put_temp[output_count];
				strcat(sz_out_put,ac_tmp);
			}
		}
	}
        
	MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX_T+1);
	aui_format_print_help(ac_tmp);
	if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX_T)	{
		strcat(sz_out_put,ac_tmp);
	}else{
		output_count++;
		sz_out_put = sz_out_put_temp[output_count];
		strcat(sz_out_put,ac_tmp);
	}

	for(i=0;i<(output_count+1);i++){
		AUI_PRINTF("%s",sz_out_put_temp[i]);
		tsleep(1);
	}
	return AUI_RTN_CODE_SUCCESS;
}


/*-------------------------------------------------------------------------
function name: aui_str_cmd_print_cur_menu
function description: test aui1.x RAM TO RAM encrypt engine command.
author: ray.gong (2012-11-10)
input parameter: NONE;
				 
output parameter:NONE
return value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
modify history:
-------------------------------------------------------------------------*/

unsigned long aui_str_cmd_print_cur_menu()
{
	unsigned long i=0;
	char ac_tmp[UART_CMD_OUTPUT_LEN_MAX_T+1]={0};
	//char sz_out_put[UART_CMD_OUTPUT_LEN_MAX_T+1]={0};
	unsigned long ui_idx_start=0;
	unsigned long ui_start_pos_flag=0;

	ui_idx_start=g_st_aui_cmd_cur_status.ui_total_idx;
	unsigned long output_count = 0;
	char *sz_out_put=NULL;
	char sz_out_put_temp[UART_CMD_OUTPUT_COUNT][UART_CMD_OUTPUT_LEN_MAX_T+1];
	for(i=0;i<UART_CMD_OUTPUT_COUNT;i++){
		MEMSET(sz_out_put_temp[i],0,(UART_CMD_OUTPUT_LEN_MAX_T+1));
	}
	sz_out_put = sz_out_put_temp[output_count];

	strcpy(ac_tmp,"\r\n                     0 : print menu");
	strcat(sz_out_put,ac_tmp);
	
	for(i=ui_idx_start;
		((i<USER_CMD_COUNT_MAX)
		&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"NULL"))
		&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"")));i++)	{
		if((0==ui_start_pos_flag)&&(g_pst_aui_cmd_tree[i].by_menu_level<g_st_aui_cmd_cur_status.by_menu_level))
		{
			continue;
		}
		else if(0==ui_start_pos_flag)
		{
			ui_start_pos_flag=1;
		}
		else if((1==ui_start_pos_flag)&&(g_pst_aui_cmd_tree[i].by_menu_level<g_st_aui_cmd_cur_status.by_menu_level))
		{
			break;
		}
			
		if(g_pst_aui_cmd_tree[i].by_menu_level==g_st_aui_cmd_cur_status.by_menu_level)
		{
			MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX_T+1);
			aui_format_print_menu(ac_tmp,g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction,UART_CMD_ID_LEN_MAX+2);

			//sprintf(ac_tmp,"\r\n-%s- %s",g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction);
			if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX_T)	{
				strcat(sz_out_put,ac_tmp);
			}else{
				output_count++;
				sz_out_put = sz_out_put_temp[output_count];
				strcat(sz_out_put,ac_tmp);
			}
		}
	}
	MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX_T+1);
	aui_format_print_help(ac_tmp);
	if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX_T)	{
		strcat(sz_out_put,ac_tmp);
	}else{
		output_count++;
		sz_out_put = sz_out_put_temp[output_count];
		strcat(sz_out_put,ac_tmp);
	}

	for(i=0;i<output_count+1;i++){
		AUI_PRINTF("%s",sz_out_put_temp[i]);
		tsleep(1);
	}
	return AUI_RTN_CODE_SUCCESS;
}

/*-------------------------------------------------------------------------
function name: aui_str_cmd_ent_and_print_upper_menu
function description: test aui1.x RAM TO RAM encrypt engine command.
author: ray.gong (2012-11-10)
input parameter: NONE;
				 
output parameter:NONE
return value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
modify history:
-------------------------------------------------------------------------*/

unsigned long aui_str_cmd_ent_and_print_upper_menu()
{
	int i=0;
	char ac_tmp[UART_CMD_OUTPUT_LEN_MAX_T+1]={0};

	int output_count = 0;
	char *sz_out_put=NULL;
	char sz_out_put_temp[UART_CMD_OUTPUT_COUNT][UART_CMD_OUTPUT_LEN_MAX_T+1];
	for(i=0;i<UART_CMD_OUTPUT_COUNT;i++){
		MEMSET(sz_out_put_temp[i],0,(UART_CMD_OUTPUT_LEN_MAX_T+1));
	}
	sz_out_put = sz_out_put_temp[output_count];


	if(g_st_aui_cmd_cur_status.by_menu_level>1)	{
		g_st_aui_cmd_cur_status.by_menu_level--;
	}
	strcpy(ac_tmp,"\r\n                     0 : print menu");
	strcat(sz_out_put,ac_tmp);

	if(g_st_aui_cmd_cur_status.ui_total_idx>0){
		for(i=g_st_aui_cmd_cur_status.ui_total_idx;
			((i>=0)
			&&(g_pst_aui_cmd_tree[i].by_menu_level>=g_st_aui_cmd_cur_status.by_menu_level)
			&&((NULL!=g_pst_aui_cmd_tree[i].ac_cmdID)&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"NULL")))
			&&((NULL!=g_pst_aui_cmd_tree[i].ac_cmdID)&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,""))));i--){
			if(g_pst_aui_cmd_tree[i].by_menu_level==g_st_aui_cmd_cur_status.by_menu_level){
				g_st_aui_cmd_cur_status.ui_total_idx=i;
			}
		}
	}
	
	for(i=g_st_aui_cmd_cur_status.ui_total_idx;
		((i<USER_CMD_COUNT_MAX)
		&&(g_pst_aui_cmd_tree[i].by_menu_level>=g_st_aui_cmd_cur_status.by_menu_level)
		&&((NULL!=g_pst_aui_cmd_tree[i].ac_cmdID)&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"NULL")))
		&&((NULL!=g_pst_aui_cmd_tree[i].ac_cmdID)&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,""))));i++){
		if(g_pst_aui_cmd_tree[i].by_menu_level==g_st_aui_cmd_cur_status.by_menu_level){
			MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX_T+1);
			aui_format_print_menu(ac_tmp,g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction,UART_CMD_ID_LEN_MAX+2);

			//sprintf(ac_tmp,"\r\n-%s- %s",g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction);
			if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX_T)	{
				strcat(sz_out_put,ac_tmp);
			}else{
				output_count++;
				sz_out_put = sz_out_put_temp[output_count];
				strcat(sz_out_put,ac_tmp);
			}
		}
	}
	MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX_T+1);
	aui_format_print_help(ac_tmp);
	if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX_T)	{
		strcat(sz_out_put,ac_tmp);
	}else{
		output_count++;
		sz_out_put = sz_out_put_temp[output_count];
		strcat(sz_out_put,ac_tmp);
	}

	for(i=0;i<output_count+1;i++){
		AUI_PRINTF("%s",sz_out_put_temp[i]);
		tsleep(1);
	}

	return AUI_RTN_CODE_SUCCESS;
}

/*-------------------------------------------------------------------------
function name: aui_str_cmd_quit
function description: test aui1.x RAM TO RAM encrypt engine command.
author: ray.gong (2012-11-10)
input parameter: NONE;
				 
output parameter:NONE
return value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
modify history:
-------------------------------------------------------------------------*/

unsigned long aui_str_cmd_quit()
{
	drv_set_aui_cmd_task_status(0);
	AUI_PRINTF("\r\n_user exit the AUI test application now!");
	return AUI_RTN_CODE_SUCCESS;
}



/*-------------------------------------------------------------------------
function name: aui_str_cmd_print_cur_menu
function description: test aui1.x RAM TO RAM encrypt engine command.
author: ray.gong (2012-11-10)
input parameter: NONE;
				 
output parameter:NONE
return value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
modify history:
-------------------------------------------------------------------------*/

unsigned long aui_str_cmd_run_level()
{
	unsigned long i=0;
	int k=0;
	char ac_tmp[UART_CMD_OUTPUT_LEN_MAX+1]={0};
	char sz_out_put[UART_CMD_OUTPUT_LEN_MAX+1]={0};
	unsigned long ui_idx_start=0;
	unsigned long ui_start_pos_flag=0;
	unsigned long ui_rtn=AUI_RTN_CODE_ERR;

	ui_idx_start=g_st_aui_cmd_cur_status.ui_total_idx;

	if(strlen("\r\n\r\n\r\n********************test result report start********************************")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcat(sz_out_put,"\r\n\r\n\r\n********************test result report start********************************\r\n");
	}
	for(i=ui_idx_start;
		((i<USER_CMD_COUNT_MAX)
		&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"NULL"))
		&&(0!=strcmp(g_pst_aui_cmd_tree[i].ac_cmdID,"")));i++)
	{
		if((0==ui_start_pos_flag)&&(g_pst_aui_cmd_tree[i].by_menu_level<g_st_aui_cmd_cur_status.by_menu_level))
		{
			continue;
		}
		else if(0==ui_start_pos_flag)
		{
			ui_start_pos_flag=1;
		}
		else if((1==ui_start_pos_flag)&&(g_pst_aui_cmd_tree[i].by_menu_level<g_st_aui_cmd_cur_status.by_menu_level))
		{
			break;
		}
			
		if((g_pst_aui_cmd_tree[i].by_menu_level==g_st_aui_cmd_cur_status.by_menu_level)
			&&(AUI_CMD_TYPE_UNIT==g_pst_aui_cmd_tree[i].ul_cmd_type))
		{


			ui_rtn=g_pst_aui_cmd_tree[i].cmd_handle(&(g_st_aui_cmd_str.ui_cmd_param_cnt),g_st_aui_cmd_str.argv,g_st_aui_cmd_str.ac_cmd_out_put);

			AUI_PRINTF("\r\n%s",g_st_aui_cmd_str.ac_cmd_out_put);
			//if(aui_str_cmd_print_menu!=g_pst_aui_cmd_tree[i].cmd_handle)
			//if(AUI_CMD_TYPE_UNIT==g_pst_aui_cmd_tree[i].ul_cmd_type)
			{
				if(0==ui_rtn)
				{
					AUI_PRINTF("\r\n[%s]",g_pst_aui_cmd_tree[i].ac_cmd_instuction);
					
					if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
					{
						strcat(sz_out_put,"\r\n-->");
					}
					if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
					{
						strcat(sz_out_put,g_pst_aui_cmd_tree[i].ac_cmd_instuction);
					}
					for( k=0;k<(int)(70-strlen(g_pst_aui_cmd_tree[i].ac_cmd_instuction));k++)
					{
						AUI_PRINTF(".");
						if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
						{
							strcat(sz_out_put,".");
						}
					}
					AUI_PRINTF("PASS");
					if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
					{
						strcat(sz_out_put,"PASS");
					}
					//AUI_PRINTF("%s\r\n[%s][RTN]:PASS.",g_pst_aui_cmd_tree[i].ac_cmd_instuction,g_st_aui_cmd_str.ac_cmd_out_put);
				}
				else if(AUI_RTN_HELP == ui_rtn)
				{
					;
				}
				else
				{
					AUI_PRINTF("\r\n[%s]",g_pst_aui_cmd_tree[i].ac_cmd_instuction);
					
					if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
					{
						strcat(sz_out_put,"\r\n-->");
					}
					if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
					{
						strcat(sz_out_put,g_pst_aui_cmd_tree[i].ac_cmd_instuction);
					}
					for( k=0;k<(int)(70-strlen(g_pst_aui_cmd_tree[i].ac_cmd_instuction));k++)
					{
						AUI_PRINTF(".");
						if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
						{
							strcat(sz_out_put,".");
						}
					}
					AUI_PRINTF("FAIL");
					if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
					{
						strcat(sz_out_put,"FAIL");
					}
					//AUI_PRINTF("%s\r\n[%s][RTN]:FAIL.",g_pst_aui_cmd_tree[i].ac_cmd_instuction,g_st_aui_cmd_str.ac_cmd_out_put);
				}
			}
		

		}
	}
	if(strlen("\r\n\r\n*********************test result report end**********************************")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcat(sz_out_put,"\r\n\r\n*********************test result report end**********************************");
	}		
	
	AUI_PRINTF("%s",sz_out_put);
	return AUI_RTN_CODE_SUCCESS;
}

/*-------------------------------------------------------------------------
function name: aui_str_cmd_print_menu
function description: test aui1.x RAM TO RAM encrypt engine command.
author: ray.gong (2012-11-10)
input parameter: NONE;
				 
output parameter:NONE
return value:SUCCESS:RET_SUCCESS: return success;RET_FAILURE: retrun failed
modify history:
-------------------------------------------------------------------------*/

unsigned long aui_str_cmd_print_menu(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long i=0;
	char ac_tmp[UART_CMD_OUTPUT_LEN_MAX]={0};
	
	if(NULL==sz_out_put)
	{
		return AUI_RTN_EINVAL;
	}
	
	g_st_aui_cmd_cur_status.by_menu_level++;
	
	strcpy(ac_tmp,"\r\n                     0 : print menu");
	strcat(sz_out_put,ac_tmp);	
	
	for(i=g_st_aui_cmd_cur_status.ui_total_idx+1;
		((i<USER_CMD_COUNT_MAX)
		&&(0!=strcmp(g_st_aui_cmd_str.ac_cmdID,"NULL"))
		&&(0!=strcmp(g_st_aui_cmd_str.ac_cmdID,""))
		&&(g_pst_aui_cmd_tree[i].by_menu_level>=g_st_aui_cmd_cur_status.by_menu_level));i++)
	{
		if(g_pst_aui_cmd_tree[i].by_menu_level==g_st_aui_cmd_cur_status.by_menu_level)
		{
			MEMSET(ac_tmp,0,UART_CMD_OUTPUT_LEN_MAX);
			aui_format_print_menu(ac_tmp,g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction,UART_CMD_ID_LEN_MAX+2);
			//sprintf(ac_tmp,"\r\n-%s- %s",g_pst_aui_cmd_tree[i].ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmd_instuction);
			if(strlen(ac_tmp)+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
			{
				strcat(sz_out_put,ac_tmp);
			}
		}
	}
		
	if(strlen("\r\n                   run : run the unit test item of this level menu")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcpy(ac_tmp,"\r\n                   run : run the unit test item of this level menu");
		strcat(sz_out_put,ac_tmp);
	}	
	
	if(strlen("\r\n                    up : upper menu")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcpy(ac_tmp,"\r\n                    up : upper menu");
		strcat(sz_out_put,ac_tmp);
	}
	
	if(strlen("\r\n                  root : root menu")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcpy(ac_tmp,("\r\n                  root : root menu"));
		strcat(sz_out_put,ac_tmp);
	}	
	
	if(strlen("\r\n                  quit : quit application")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcpy(ac_tmp,"\r\n                  quit : quit application");
		strcat(sz_out_put,ac_tmp);
	}
	if(strlen("\r\n")+strlen(sz_out_put)<UART_CMD_OUTPUT_LEN_MAX)
	{
		strcat(sz_out_put,"\r\n");
	}		
	//strncpy(sz_out_put,"AUI1.x print menu demo success!",UART_CMD_OUTPUT_LEN_MAX);
	return AUI_RTN_CODE_SUCCESS;
}

unsigned long test_cmd(unsigned long *argc, char **argv,char *sz_out_put)
{
    unsigned long ul_user_int_input=0;
    int i=0;
    char acTmp[UART_CMD_STR_LEN_MAX+1]={0};
    
    if((NULL==argc)||(NULL==argv))
    {
        return AUI_RTN_FAIL;
    }
    
    if(*argc>USER_CMD_PARAM_COUNT_MAX)
    {
        AUI_PRINTF("\r\n Too many parameter,max support [%d] parameters",USER_CMD_PARAM_COUNT_MAX);  
        return AUI_RTN_FAIL;
    }
    AUI_PRINTF("\r\n parameter counter is [%d]",*argc);
    if(*argc>0)
    {
        for(i=0;i< (int)*argc;i++)
        {
            AUI_PRINTF("\r\n argv[%d]=[%d][%s]",i,strlen(argv[i]),(argv[i])); 
        }
    }
    
    AUI_PRINTF("\r\n Please input a HEX num:0x");  
    aui_test_get_user_hex_input(&ul_user_int_input);
    AUI_PRINTF("\r\n Uer input HEX num=[%d]:[0x%02x]",ul_user_int_input,ul_user_int_input);  
    AUI_PRINTF("\r\n Please input a DEC num:0x");  
    aui_test_get_user_dec_input(&ul_user_int_input);
    AUI_PRINTF("\r\n Uer input DEC num=[%d]:[0x%02x]",ul_user_int_input,ul_user_int_input);  
     AUI_PRINTF("\r\n Please input a string:");  
    aui_test_get_user_str_input(acTmp);
    AUI_PRINTF("\r\n Uer input string=[%s];len=[%d]",acTmp,strlen(acTmp));  
    if(!aui_test_user_confirm("do you have success execute this test cmd?"))
	{
		return AUI_RTN_FAIL;
	}
	return 0;
}
void aui_load_tu_test(void)
{
	aui_tu_reg_group("test", "test cmd test");
	aui_tu_reg_item(2, "testcmd", AUI_CMD_TYPE_API, test_cmd, "test cmd test");
}

/*
#if 0
    {1,"testcmd",AUI_CMD_TYPE_API,test_cmd,"test command"},
	{1,"decv",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"Decv test cases"},
	{2,"alltests",AUI_CMD_TYPE_API,test_decv,"Decv test cases"},

	{1,"dmx",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"Dmx test cases"},
	{2,"alltests",AUI_CMD_TYPE_API,test_dmx,"Dmx test cases"},

	{1,"dis",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"dis test"},      
	{2,"open",AUI_CMD_TYPE_UNIT,test_dis_open,"dis open"},        
	{2,"dismodeset",AUI_CMD_TYPE_UNIT,test_dis_mode,"mode set"},       
	{2,"enhance",AUI_CMD_TYPE_UNIT,test_enhance_set,"enhance set"},      
	{2,"zoom",AUI_CMD_TYPE_UNIT,test_zoom,"zoom set"},    
	{2,"osdclose",AUI_CMD_TYPE_UNIT,test_close_osd,"close osd"},      
	{2,"videoen",AUI_CMD_TYPE_UNIT,test_video_enable,"video enable"},      
	{2,"aspect",AUI_CMD_TYPE_UNIT,test_aspect_ratio,"aspect"},       
	{2,"matchmode",AUI_CMD_TYPE_UNIT,test_match_mode,"matchmode"},        
	{2,"tvsys",AUI_CMD_TYPE_UNIT,test_tvsys_mode,"tv sys set"},  
#if 1
	{1,"av",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"AV TEST cases"},
		//{2,"hdmi_init",AUI_CMD_TYPE_API,test_hdmi_init,"HDMI INIT cases"},	
		{2,"av_open",AUI_CMD_TYPE_API,test_av_open,"AV open cases"},
		{2,"av_start",AUI_CMD_TYPE_API,test_av_start,"AV start cases"},
		{2,"av_pause",AUI_CMD_TYPE_API,test_av_pause,"AV pause cases"},
		{2,"av_resume",AUI_CMD_TYPE_API,test_av_resume,"AV resume cases"},
		{2,"av_stop",AUI_CMD_TYPE_API,test_av_stop,"AV stop cases"},
		//{2,"hdmi_deinit",AUI_CMD_TYPE_API,test_hdmi_deinit,"HDMI DEINIT case"},
#endif

	{1,"vbi",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"VBI test cases"},
	{2,"test_vbi",AUI_CMD_TYPE_API,test_ttx,"test ttx"},
#if 0	
#if 1
	{1,"deca",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"audio test"},
		{2,"start",AUI_CMD_TYPE_API,test_deca_start,"[DECA_1]AUDIO_DECODER_START"},
		{2,"stop",AUI_CMD_TYPE_API,test_deca_stop,"[DECA_2]AUDIO_DECODER_STOP"},
		{2,"pause",AUI_CMD_TYPE_API,test_deca_pause,"[DECA_3]AUDIO_DECODER_PAUSE"},
		{2,"resume",AUI_CMD_TYPE_API,test_deca_resume,"[DECA_4]AUDIO_DECODER_RESUME"},
		{2,"getdatainfo",AUI_CMD_TYPE_API,test_deca_data_info_get,"[DECA_4]AUDIO_DECODER_RESUME"},
		{2,"get_status",AUI_CMD_TYPE_API,test_deca_status_get,"[DECA_5]AUDIO_DECODER_STATUS_GET"},
		{2,"set_type",AUI_CMD_TYPE_API,test_deca_type_set,"[DECA_6]AUDIO_DECODER_TYPE_SET"},
		{2,"get_type",AUI_CMD_TYPE_API,test_deca_type_get,"[DECA_7]AUDIO_DECODER_TYPE_GET"},
		{2,"set_sync_mode",AUI_CMD_TYPE_API,test_deca_sync_mode_set,"[DECA_8]AUDIO_DECODER_SYNC_MODE_SET"},
		{2,"get_sync_mode",AUI_CMD_TYPE_API,test_deca_sync_mode_get,"[DECA_9]AUDIO_DECODER_SYNC_MODE_GET"},
		{2,"set_sync_level",AUI_CMD_TYPE_API,test_deca_sync_level_set,"[DECA_10]AUDIO_DECODER_SYNC_LEVEL_SET"},
		{2,"get_sync_level",AUI_CMD_TYPE_API,test_deca_sync_level_get,"[DECA_11]AUDIO_DECODER_SYNC_LEVEL_GET"},
		{2,"set_pid",AUI_CMD_TYPE_API,test_deca_aud_pid_set,"[DECA_12]AUDIO_DECODER_AUDIO_PID_SET"},
		{2,"get_pid",AUI_CMD_TYPE_API,test_deca_aud_pid_get,"[DECA_13]AUDIO_DECODER_AUDIO_PID_GET"},
		{2,"get_dev_cnt",AUI_CMD_TYPE_API,test_deca_dev_cnt_get,"[DECA_14]AUDIO_DECODER_DEV_CNT_GET"},
		{2,"inject_open",AUI_CMD_TYPE_API,test_deca_inject_open,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		{2,"inject_start",AUI_CMD_TYPE_API,test_deca_inject_start,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		{2,"inject_stop",AUI_CMD_TYPE_API,test_deca_inject_stop,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		{2,"inject",AUI_CMD_TYPE_API,test_deca_inject_wt,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		{2,"inject_play_dat",AUI_CMD_TYPE_API,test_deca_inject_play_dat,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		{2,"inject_play_file",AUI_CMD_TYPE_API,test_deca_inject_play_file,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		//{2,"inject_file_set",AUI_CMD_TYPE_API,test_deca_inject_file_name_set,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},
		//{2,"inject_type_set",AUI_CMD_TYPE_API,test_deca_inject_file_type_set,"[DECA_14]AUDIO_DECODER_DATA_PULGIN"},


	{1,"snd",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"audio sound test"},
		{2,"start",AUI_CMD_TYPE_API,test_snd_start,"[SND_1]start sound task"},
		{2,"stop",AUI_CMD_TYPE_API,test_snd_stop,"[SND_2]stop sound task"},
		{2,"pause",AUI_CMD_TYPE_API,test_snd_pause,"[SND_3]pause sound task"},
		{2,"resume",AUI_CMD_TYPE_API,test_snd_resume,"[SND_4]resume sound task"},
		{2,"set_vol",AUI_CMD_TYPE_API,test_snd_vol_set,"[SND_5]set current volume"},
		{2,"get_vol",AUI_CMD_TYPE_API,test_snd_vol_get,"[SND_6]get current volume"},
		{2,"set_mute",AUI_CMD_TYPE_API,test_snd_mute_set,"[SND_7]set mute status"},
		{2,"get_mute",AUI_CMD_TYPE_API,test_snd_mute_get,"[SND_8]get mute status"},
		{2,"set_channel",AUI_CMD_TYPE_API,test_snd_channel_set,"[SND_7]get channel status"},
		{2,"get_channel",AUI_CMD_TYPE_API,test_snd_channel_get,"[SND_8]set channel status"},
		{2,"set_sync_level",AUI_CMD_TYPE_API,test_snd_sync_level_set,"[SND_9]set sync level"},
		{2,"get_sync_level",AUI_CMD_TYPE_API,test_snd_sync_level_get,"[SND_10]get sync level"},
		{2,"set_out_type",AUI_CMD_TYPE_API,test_snd_out_interface_type_set,"[SND_7]set output interface enabel/disabel"},
		{2,"get_out_type",AUI_CMD_TYPE_API,test_snd_out_interface_type_get,"[SND_8]get output interface enabel/disabel status"},
		{2,"set_out_mode",AUI_CMD_TYPE_API,test_snd_out_data_set,"[SND_7]set out data type of interface"},
		{2,"get_out_mode",AUI_CMD_TYPE_API,test_snd_out_data_get,"[SND_8]set out data type of interface"},			
#endif

	{1,"tsg",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"TS stream generator test"},
		{2,"playback",AUI_CMD_TYPE_API,test_tsg_play_back_by_tsg,"[TSG_1]playback a disk file by tsg"},
		//{2,"open",AUI_CMD_TYPE_API,test_tsg_open_file,"[TSG_1]TSG_PLAYBACK1"},
		//{2,"close",AUI_CMD_TYPE_API,test_tsg_close_file,"[TSG_1]TSG_PLAYBACK2"},
			
	{1,"kl",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"key ladder test"},
		{2,"tdes_pure_data_en",AUI_CMD_TYPE_API,test_kl_gen_key_by_cfg,"[KL_1]tdes_pure_data_en"},
			
	{1,"dog",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"watch dog test"},
		{2,"start",AUI_CMD_TYPE_API,test_dog_start,"[DOG_1]WATCH_DOG_START_TEST"},
		{2,"stop",AUI_CMD_TYPE_API,test_dog_stop,"[DOG_2]WATCH_DOG_STOP_TEST"},
		{2,"settime",AUI_CMD_TYPE_API,test_dog_time_set,"[DOG_3]WATCH_DOG_TIMESET_TEST"},
		{2,"gettime",AUI_CMD_TYPE_API,test_dog_time_get,"[DOG_4]WATCH_DOG_TIMEGET_TEST"},
		{2,"config",AUI_CMD_TYPE_API,test_dog_config,"[DOG_5]WATCH_DOG_CONFIG_TEST"},
	{1,"rtc",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"real timer test"},
		{2,"settime",AUI_CMD_TYPE_API,test_rtc_time_set,"[RTC_1]RTC_TIMESET_TEST"},
		{2,"gettime",AUI_CMD_TYPE_API,test_rtc_time_get,"[RTC_2]RTC_TIMEGET_TEST"},
		{2,"config",AUI_CMD_TYPE_API,test_rtc_alm_config,"[RTC_3]RTC_ALM_CONFIG_TEST"},
		{2,"onoff",AUI_CMD_TYPE_API,test_rtc_alm_on_off,"[RTC_4]RTC_ALM_ONOFF_TEST"},	
	{1,"decv",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"video test"},
		{2,"1",AUI_CMD_TYPE_UNIT,test_deca_dev_cnt_get,"[VIDEO_DEC_1]VIDEO_DECODER_TEST"},
		{2,"2",AUI_CMD_TYPE_UNIT,test_deca_dev_cnt_get,"[VIDEO_DEC_2]VIDEO_DISP_TEST"},
		{2,"3",AUI_CMD_TYPE_UNIT,test_deca_dev_cnt_get,"[VIDEO_DEC_3]VIDEO_SYNC_MODE_TEST"},
	{1,"dmx",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"dmx test"},
		{2,"dmxstart",AUI_CMD_TYPE_UNIT,test_dmx_start,"[DMX_1]DMX_START_TEST"},
		{2,"dmxstop",AUI_CMD_TYPE_UNIT,test_dmx_stop,"[DMX_1]DMX_START_TEST"},
		{2,"dmxpause",AUI_CMD_TYPE_UNIT,test_dmx_pause,"[DMX_1]DMX_START_TEST"},
		{2,"dmxresume",AUI_CMD_TYPE_UNIT,test_dmx_resume,"[DMX_1]DMX_START_TEST"},
		//{2,"syncgetdata",AUI_CMD_TYPE_UNIT,test_dmx_data_sync_get,"[DMX_1]DMX_START_TEST"},
		{2,"asyncgetdata",AUI_CMD_TYPE_UNIT,test_dmx_data_async_get,"[DMX_1]DMX_START_TEST"},
		{2,"filteropen",AUI_CMD_TYPE_UNIT,test_dmx_filter_open,"[DMX_1]DMX_START_TEST"},
		{2,"filterclose",AUI_CMD_TYPE_UNIT,test_dmx_filter_close,"[DMX_1]DMX_START_TEST"},
		{2,"filterstart",AUI_CMD_TYPE_UNIT,test_dmx_filter_start,"[DMX_1]DMX_START_TEST"},
		{2,"filterstop",AUI_CMD_TYPE_UNIT,test_dmx_filter_stop,"[DMX_1]DMX_START_TEST"},			
		{2,"channelopen",AUI_CMD_TYPE_UNIT,test_dmx_channel_open,"[DMX_1]DMX_START_TEST"},
		{2,"channelclose",AUI_CMD_TYPE_UNIT,test_dmx_channel_close,"[DMX_1]DMX_START_TEST"},
	{1,"os",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"dmx test"},
		{2,"taskcreate",AUI_CMD_TYPE_UNIT,test_os_task_create,"[DMX_1]DMX_START_TEST"},
		{2,"taskdel",AUI_CMD_TYPE_UNIT,test_os_task_delete,"[DMX_1]DMX_START_TEST"},
		{2,"taskselfid",AUI_CMD_TYPE_UNIT,test_os_task_self_id,"[DMX_1]DMX_START_TEST"},	
		{2,"taskjoin",AUI_CMD_TYPE_UNIT,test_os_task_join,"[DMX_1]DMX_START_TEST"},	
		{2,"msgopen",AUI_CMD_TYPE_UNIT,test_os_msgQCreate,"[DMX_1]DMX_START_TEST"},	
		{2,"msgclose",AUI_CMD_TYPE_UNIT,test_os_msgQDelete,"[DMX_1]DMX_START_TEST"},				
		{2,"msgsnd",AUI_CMD_TYPE_UNIT,test_os_msgQSnd,"[DMX_1]DMX_START_TEST"},	
		{2,"msgrcv",AUI_CMD_TYPE_UNIT,test_os_msgQRcv,"[DMX_1]DMX_START_TEST"},	
		{2,"semcreate",AUI_CMD_TYPE_UNIT,test_os_sem_create,"[DMX_1]DMX_START_TEST"},	
		{2,"semdel",AUI_CMD_TYPE_UNIT,test_os_sem_delete,"[DMX_1]DMX_START_TEST"},				
		{2,"semtake",AUI_CMD_TYPE_UNIT,test_os_sem_wait,"[DMX_1]DMX_START_TEST"},	
		{2,"semgive",AUI_CMD_TYPE_UNIT,test_os_sem_release,"[DMX_1]DMX_START_TEST"},	
		{2,"eventcreate",AUI_CMD_TYPE_UNIT,test_os_event_create,"[DMX_1]DMX_START_TEST"},	
		{2,"eventdel",AUI_CMD_TYPE_UNIT,test_os_event_delete,"[DMX_1]DMX_START_TEST"},				
		{2,"eventset",AUI_CMD_TYPE_UNIT,test_os_event_set,"[DMX_1]DMX_START_TEST"},	
		{2,"eventwait",AUI_CMD_TYPE_UNIT,test_os_event_wait,"[DMX_1]DMX_START_TEST"},	
		{2,"mutexcreate",AUI_CMD_TYPE_UNIT,test_os_mutex_create,"[DMX_1]DMX_START_TEST"},	
		{2,"mutexdel",AUI_CMD_TYPE_UNIT,test_os_mutex_delete,"[DMX_1]DMX_START_TEST"},				
		{2,"mutexlock",AUI_CMD_TYPE_UNIT,test_os_mutex_lock,"[DMX_1]DMX_START_TEST"},	
		{2,"mutexunlock",AUI_CMD_TYPE_UNIT,test_os_mutex_unlock,"[DMX_1]DMX_START_TEST"},
		{2,"mutextrylock",AUI_CMD_TYPE_UNIT,test_os_mutex_try_lock,"[DMX_1]DMX_START_TEST"},	
		{2,"timercreate",AUI_CMD_TYPE_UNIT,test_os_timer_create,"[DMX_1]DMX_START_TEST"},	
		{2,"timerdel",AUI_CMD_TYPE_UNIT,test_os_timer_delete,"[DMX_1]DMX_START_TEST"},				
		{2,"timerset",AUI_CMD_TYPE_UNIT,test_os_timer_set,"[DMX_1]DMX_START_TEST"},	
		{2,"timerget",AUI_CMD_TYPE_UNIT,test_os_timer_get,"[DMX_1]DMX_START_TEST"},
		{2,"timerrun",AUI_CMD_TYPE_UNIT,test_os_timer_run,"[DMX_1]DMX_START_TEST"},	
	{1,"tds",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"tds demo test"},	
		{2,"task",AUI_CMD_TYPE_UNIT,test_os_task_create_demo1,"[TDS_DEMO_1]create simple test task"},
		{2,"msgq",AUI_CMD_TYPE_UNIT,test_os_task_create_demo2,"[TDS_DEMO_2]create msgq test task"},
		{2,"mutex",AUI_CMD_TYPE_UNIT,test_os_task_create_demo3,"[TDS_DEMO_3]create mutex test task"},
		{2,"flag",AUI_CMD_TYPE_UNIT,test_os_task_create_demo4,"[TDS_DEMO_4]create flag test task"},
		{2,"deltask",AUI_CMD_TYPE_UNIT,test_os_task_delete_demo,"[TDS_DEMO_4]delete all task"},
		{2,"int",AUI_CMD_TYPE_UNIT,test_os_interrupt,"[TDS_DEMO_4]delete all task"},
		
	{1,"decv",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"video test"},	
        {2,"init",AUI_CMD_TYPE_UNIT,test_decv_init,"init"},
        {2,"pause",AUI_CMD_TYPE_UNIT,test_decv_pause,"pause"},		
        {2,"resume",AUI_CMD_TYPE_UNIT,test_decv_resume,"resume"}, 

        {2,"start",AUI_CMD_TYPE_UNIT,test_decv_start,"start"},
        {2,"stop",AUI_CMD_TYPE_UNIT,test_decv_stop,"stop"},		
        {2,"open",AUI_CMD_TYPE_UNIT,test_decv_open,"open"}, 
        {2,"close",AUI_CMD_TYPE_UNIT,test_decv_close,"close"}, 
        {2,"format",AUI_CMD_TYPE_UNIT,test_decv_deode_format,"decode format set"}, 
        {2,"sync",AUI_CMD_TYPE_UNIT,test_decv_sync_mode,"sync mode set"}, 
        {2,"chg",AUI_CMD_TYPE_UNIT,test_decv_change_mode,"change mode set"}, 

    {1,"img",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"image test"},	
        {2,"init",AUI_CMD_TYPE_UNIT,test_aui_image_init,"init"},
        {2,"rotate",AUI_CMD_TYPE_UNIT,test_aui_image_rotate,"rotate image"}, 

        {2,"start",AUI_CMD_TYPE_UNIT,test_aui_image_start,"start"},
            
        {2,"open",AUI_CMD_TYPE_UNIT,test_aui_image_open,"open"}, 
        {2,"close",AUI_CMD_TYPE_UNIT,test_aui_image_close,"close"}, 
            
        {2,"move",AUI_CMD_TYPE_UNIT,test_aui_image_move,"move image"}, 
        {2,"enlarge",AUI_CMD_TYPE_UNIT,test_aui_image_enlarge,"enlarge image"}, 
        {2,"set",AUI_CMD_TYPE_UNIT,test_aui_image_set,"set file name"}, 
        {2,"mode",AUI_CMD_TYPE_UNIT,test_aui_image_display_mode_set,"display mode set"}, 
	

    {1,"av",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"av test"},        
        {2,"start",AUI_CMD_TYPE_UNIT,test_av_start,"av test start"},        
        {2,"stop",AUI_CMD_TYPE_UNIT,test_av_stop,"av stop"},      
        {2,"pause",AUI_CMD_TYPE_UNIT,test_av_pause,"av pause"},       
        {2,"resume",AUI_CMD_TYPE_UNIT,test_av_resume,"av resume"},     
        {2,"open",AUI_CMD_TYPE_UNIT,test_av_open,"av open"},        
        {2,"close",AUI_CMD_TYPE_UNIT,test_av_close,"av close"},       
        {2,"init",AUI_CMD_TYPE_UNIT,test_av_init,"av init"},      
        {2,"vpids",AUI_CMD_TYPE_UNIT,test_video_pid_set,"av video pids"},      
        {2,"apids",AUI_CMD_TYPE_UNIT,test_audio_pid_set,"av audio pids"},      
        {2,"pcrpids",AUI_CMD_TYPE_UNIT,test_pcr_pid_set,"av pcr pids"},      
        {2,"set",AUI_CMD_TYPE_UNIT,test_av_set,"av set"},       
        {2,"get",AUI_CMD_TYPE_UNIT,test_av_get,"av get"},


    {1,"dis",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"dis test"},      
        {2,"open",AUI_CMD_TYPE_UNIT,test_dis_open,"dis open"},        
        {2,"dismodeset",AUI_CMD_TYPE_UNIT,test_dis_mode,"mode set"},       
        {2,"enhance",AUI_CMD_TYPE_UNIT,test_enhance_set,"enhance set"},      
        {2,"zoom",AUI_CMD_TYPE_UNIT,test_zoom,"zoom set"},    
        {2,"osdclose",AUI_CMD_TYPE_UNIT,test_close_osd,"close osd"},      
        {2,"videoen",AUI_CMD_TYPE_UNIT,test_video_enable,"video enable"},      
        {2,"aspect",AUI_CMD_TYPE_UNIT,test_aspect_ratio,"aspect"},       
        {2,"matchmode",AUI_CMD_TYPE_UNIT,test_match_mode,"matchmode"},        
        {2,"tvsys",AUI_CMD_TYPE_UNIT,test_tvsys_mode,"tv sys set"},  


    {1,"mp",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"media player test"},
        {2,"open",AUI_CMD_TYPE_UNIT,test_mp_open,"mp open"},
        {2,"close",AUI_CMD_TYPE_UNIT,test_mp_close,"mp close"}, 
        {2,"start",AUI_CMD_TYPE_UNIT,test_mp_start,"mp start"},
        {2,"stop",AUI_CMD_TYPE_UNIT,test_mp_stop,"mp stop"},
        {2,"pause",AUI_CMD_TYPE_UNIT,test_mp_pause,"mp pause"}, 
        {2,"resume",AUI_CMD_TYPE_UNIT,test_mp_resume,"mp resume"}, 
        {2,"seek",AUI_CMD_TYPE_UNIT,test_mp_seek,"mp seek"},  
        {2,"speed",AUI_CMD_TYPE_UNIT,test_mp_speed,"mp speed"},   
        {2,"time",AUI_CMD_TYPE_UNIT,test_mp_get_total_time,"mp total time"}, 
        {2,"curtime",AUI_CMD_TYPE_UNIT,test_mp_get_current_time,"mp cur time"},

     {1,"music",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"music test"},
        {2,"open",AUI_CMD_TYPE_UNIT,test_aui_music_open,"music open"},
        {2,"close",AUI_CMD_TYPE_UNIT,test_aui_music_close,"music close"}, 
        {2,"start",AUI_CMD_TYPE_UNIT,test_aui_music_start,"music start"},
        {2,"stop",AUI_CMD_TYPE_UNIT,test_aui_music_stop,"music stop"},
        {2,"pause",AUI_CMD_TYPE_UNIT,test_aui_music_pause,"music pause"}, 
        {2,"resume",AUI_CMD_TYPE_UNIT,test_aui_music_resume,"music resume"}, 
        {2,"seek",AUI_CMD_TYPE_UNIT,aui_test_music_seek,"music seek"},  
        {2,"time",AUI_CMD_TYPE_UNIT,aui_test_total_time_get,"music total time"}, 
        {2,"curtime",AUI_CMD_TYPE_UNIT,aui_test_cur_time_get,"music cur time"},	
#endif
#ifdef GFX_SUPPORT
	{1,"osd",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"osd Test"},
		{2,"1",AUI_CMD_TYPE_API,test_osd_hw_surface_test,"test_osd_hw_surface_test"},
		{2,"2",AUI_CMD_TYPE_API,test_osd_sw_surface_test,"test_osd_sw_surface_test"},
		{2,"3",AUI_CMD_TYPE_API,test_osd_surface_blit,"test_osd_surface_blit"},
		{2,"4",AUI_CMD_TYPE_API,test_osd_data_blit,"test_osd_data_blit"},
		{2,"5",AUI_CMD_TYPE_API,test_osd_draw_line,"test_osd_draw_line"},
		{2,"6",AUI_CMD_TYPE_API,test_osd_draw_out_line,"test_osd_draw_out_line"},
		{2,"7",AUI_CMD_TYPE_API,test_osd_clip_mode,"test_osd_clip_mode"},
		{2,"8",AUI_CMD_TYPE_API,test_osd_color_key,"test_osd_color_key"},	
		{2,"9",AUI_CMD_TYPE_API,test_osd_scale_blit,"test_osd_scale_blit"},
		{2,"10",AUI_CMD_TYPE_API,test_osd_aphablending_blit,"test_osd_aphablending_blit"},
#ifdef AUI_TDS
		{2,"11",AUI_CMD_TYPE_API,test_osd_bool_test,"test_osd_bool_test"}, 
#endif
#endif
		
	{1,"subt",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"Subtitle Test"},
		{2,"1",AUI_CMD_TYPE_API,test_subtitle,"test subtitle"},
#if 0
	{1,"vbi",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"vbi Test"},
		{2,"test_vbi",AUI_CMD_TYPE_API,test_ttx,"test ttx"},		

	{1,"pvr",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"PVR test"},
		{2,"init",AUI_CMD_TYPE_UNIT,test_pvr_init,"pvr init"},
		{2,"disk_in",AUI_CMD_TYPE_UNIT,test_pvr_disk_attach,"disk attach"},
        {2,"ts_open",AUI_CMD_TYPE_UNIT,test_pvr_ts_open,"pvr ts open"},
        {2,"tms_open",AUI_CMD_TYPE_UNIT,test_pvr_tms_open,"pvr tms open"},
        {2,"ts_open_e",AUI_CMD_TYPE_UNIT,test_pvr_e_ts_open,"pvr ts open with software encrypt"},
        {2,"tms_opene",AUI_CMD_TYPE_UNIT,test_pvr_e_tms_open,"pvr tms open with software encrypt"},
        {2,"pre",AUI_CMD_TYPE_UNIT,pvr_pre,"pvr record index--"},
        {2,"next",AUI_CMD_TYPE_UNIT,pvr_next,"pvr record index++"},
        {2,"r_pause",AUI_CMD_TYPE_UNIT,test_pvr_rec_pause,"pvr recorder pause"},
        {2,"r_resume",AUI_CMD_TYPE_UNIT,test_pvr_rec_resume,"pvr recorder resume"},
        {2,"r_close",AUI_CMD_TYPE_UNIT,test_pvr_rec_close,"pvr record close"},
        {2,"open",AUI_CMD_TYPE_UNIT,test_pvr_play_open,"pvr play open"},
        {2,"play",AUI_CMD_TYPE_UNIT,test_pvr_play_play,"pvr play play"},
        {2,"pause",AUI_CMD_TYPE_UNIT,test_pvr_play_pause,"pvr play pause"},
        {2,"FF",AUI_CMD_TYPE_UNIT,test_pvr_play_FF,"pvr play FF"},
        {2,"FB",AUI_CMD_TYPE_UNIT,test_pvr_play_FB,"pvr play FB"},
        {2,"stop",AUI_CMD_TYPE_UNIT,test_pvr_play_stop,"pvr play stop"},
        {2,"close",AUI_CMD_TYPE_UNIT,test_pvr_play_close,"pvr play close"},
        {2,"get",AUI_CMD_TYPE_UNIT,test_pvr_get,"pvr get"},  
        {2,"set",AUI_CMD_TYPE_UNIT,test_pvr_set,"pvr set"}, 
        {2,"p_register",AUI_CMD_TYPE_UNIT,test_pvr_register,"pvr register"},
        {2,"p_unregister",AUI_CMD_TYPE_UNIT,test_pvr_register,"pvr unregister"},
#endif
	{1,"flash",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"Flash test"},
		{2,"nor",AUI_CMD_TYPE_UNIT,test_nor_flash,"Nor flash test"},
		{2,"nand",AUI_CMD_TYPE_UNIT,test_nand_flash,"Nand flash test"},

    
    {1,"as",AUI_CMD_TYPE_MENU,aui_str_cmd_print_menu,"Decv test cases"},
	    {2,"puredata",AUI_CMD_TYPE_API,test_kl_gen_key_by_cfg,"Test pure data encrypt and decrypte"},
        //{2,"ts",AUI_CMD_TYPE_API,test_kl_gen_key_by_cfg,"Test TS stream encrypt and decrypte"},
#endif

*/
st_aui_cmd_node g_st_aui_cmd_node[USER_CMD_COUNT_MAX+1];


static unsigned long s_ul_cmd_reg_cnt=0;

unsigned long aui_tu_reg_group(char *psz_group_name, char *psz_group_instruction)
{
    if((NULL==psz_group_name)||(NULL==psz_group_instruction)||(s_ul_cmd_reg_cnt>=USER_CMD_COUNT_MAX))
    {
        return AUI_RTN_FAIL;
    }
    strncpy(g_st_aui_cmd_node[s_ul_cmd_reg_cnt].ac_cmdID,psz_group_name,UART_CMD_ID_LEN_MAX);
    strncpy(g_st_aui_cmd_node[s_ul_cmd_reg_cnt].ac_cmd_instuction,psz_group_instruction,UART_CMD_INSTRUCT_LEN_MAX);
    g_st_aui_cmd_node[s_ul_cmd_reg_cnt].by_menu_level=1;
    g_st_aui_cmd_node[s_ul_cmd_reg_cnt].cmd_handle=aui_str_cmd_print_menu;
    g_st_aui_cmd_node[s_ul_cmd_reg_cnt].ul_cmd_type=AUI_CMD_TYPE_MENU;
    s_ul_cmd_reg_cnt++;
    return AUI_RTN_SUCCESS;
}

unsigned long aui_tu_reg_item(unsigned long ul_menu_level,
			      char *psz_group_name,
			      unsigned long ul_cmd_type,
			      unsigned long (*cmd_handle)(unsigned long *argc,char **argv,char *sz_out_put),
			      char *psz_group_instruction)
{
    if((NULL==psz_group_name)||(NULL==psz_group_instruction)||(s_ul_cmd_reg_cnt>=USER_CMD_COUNT_MAX))
    {
        return AUI_RTN_FAIL;
    }
    strncpy(g_st_aui_cmd_node[s_ul_cmd_reg_cnt].ac_cmdID,psz_group_name,UART_CMD_ID_LEN_MAX);
    strncpy(g_st_aui_cmd_node[s_ul_cmd_reg_cnt].ac_cmd_instuction,psz_group_instruction,UART_CMD_INSTRUCT_LEN_MAX);
    g_st_aui_cmd_node[s_ul_cmd_reg_cnt].by_menu_level=ul_menu_level;
    g_st_aui_cmd_node[s_ul_cmd_reg_cnt].cmd_handle=cmd_handle;
    g_st_aui_cmd_node[s_ul_cmd_reg_cnt].ul_cmd_type=ul_cmd_type;
    s_ul_cmd_reg_cnt++;
    return AUI_RTN_SUCCESS;
}

void __rst_reg_cnt()
{
    s_ul_cmd_reg_cnt=0;
}


