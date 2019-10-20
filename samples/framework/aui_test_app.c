/**@file
*@brief string command process
*@author ray.gong
*@date 2013-04-08
*@version 1.0.0
*@note
*this file mainly to process the input from terminal,such as serial
*or net,parse the input to get command id and parameter(if need).
*and according to the command register to execute the callback to run
*the test item.usually we do not need to modify this file.
*/

#ifdef AUI_TDS
#include <types.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>
#include <bus/sci/sci.h>
#include <api/libc/string.h>

extern char drvGetChar4Nocs();

extern void drvSetChar4Nocs(unsigned char by_char_input);
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern FILE *stream;

#endif
#include "aui_test_app.h"

/************************LOCAL MACRO**********************************/

static char s_argv[USER_CMD_PARAM_COUNT_MAX][USER_CMD_PARAM_STR_LEN_MAX];
/************************EXTERN VAR**********************************/
extern st_aui_cmd g_st_aui_cmd_str;
extern st_aui_cmd_node g_st_aui_cmd_node[];
extern st_aui_cmd_cur_status	g_st_aui_cmd_cur_status;
extern st_aui_cmd_node *g_pst_aui_cmd_tree;
extern void __rst_reg_cnt();

#ifdef AUI_LINUX 
#ifndef AUI_PACKAGE_ALIPLATFORM_INPUT
#include <sys/socket.h>
#include <sys/un.h>
#define DEV_LIRCD	"lircd"
#define VARRUNDIR "/var/run"
#define PACKAGE "lirc"
#define LIRCD	        VARRUNDIR "/" PACKAGE "/" DEV_LIRCD
int wait_ir()
{
	int fd, i;
	char buf[128];
	struct sockaddr_un addr;
	int c;

	addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, LIRCD);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
            AUI_PRINTF("\nsock failed!\n");
            return 0;
	};
	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            AUI_PRINTF("\nconnect failed!\n");
            return 0;
	};

	for (;;) {
		i = read(fd, buf, 128);
		if (i == -1) {
                   continue ;
		};
		if (!i)
                    continue;
                break;;
	};
        return 1;
}
#else
int wait_ir();
#endif
#endif
/************************GLOBAL VAR*******************************/

/**@brief static var to manage the system command and user input*/
static st_cmd_dev g_st_cmd_dev;

/**
*@brief get the buffer address of ask command
*@author ray.gong
*@date 2013-4-8
*@return the buffer address of ask command
*@note
*the address can not be null,it init in auiscpi_init at first.
*/
char *get_ask_cmd_buf_addr()
{
	return g_st_cmd_dev.pby_ask_cmd_buf;
}


/**
*@brief judge whether user input correct.
*@author ray.gong
*@date 2013-4-8
*@param[in] sz_rcv_cmd the sz_rcv_cmd is the  one of the valide user input
*@return AUI_RTN_CODE_SUCCESS the function successful return.
*@return AUI_RTN_CODE_ERR the function failed return.
*@note
*during executing the command,compare the user input with parameter sz_rcv_cmd,
*the sz_rcv_cmd is the  one of the valide user input.
*/
unsigned long drv_compare_ask_cmd(char *sz_rcv_cmd)
{
	if(0==strcmp((const char *)g_st_cmd_dev.pby_ask_cmd_buf,sz_rcv_cmd))
	{
		return AUI_RTN_CODE_SUCCESS;
	}

	return AUI_RTN_CODE_ERR;
}


/**
*@brief reset the ask command status and buf.
*@author ray.gong
*@date 2013-4-8
*@note
*reset the ask command status and buf.it usually be call after receive some invalid input to ask command bufffer.
*/

void rst_ask_cmd_status()
{
	g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx=0;
	MEMSET(g_st_cmd_dev.pby_ask_cmd_buf,0,UART_CMD_STR_LEN_MAX+1);
	return;
}


/**
*@brief reset the main command status and buf.
*@author ray.gong
*@date 2013-4-8
*@note
*reset the main command status and buf.it usually be call after receive some invalid input to main command bufffer.
*/
void rst_main_cmd_status()
{
	g_st_cmd_dev.ui_main_cmd_rcv_cur_idx=0;
	MEMSET(g_st_cmd_dev.pby_main_cmd_buf,0,UART_CMD_STR_LEN_MAX+1);
	return;
}


/**
*@brief parase the string user input and get command id
*@author ray.gong
*@date 2013-4-8
*@param[in] sz_input the main command that user input.
*@param[out] sz_cmd the main command id that user input.
*@param[out] argc the main command parameter counter that user input.
*@param[out] argv the main command parameter that user input.
*@return AUI_RTN_CODE_SUCCESS the function successful return.
*@return AUI_RTN_CODE_ERR the function failed return.
*@note
*parse the main command that user input and get command id,parameter counter and parameter.
*but in aui test appliation.the parameter is not used.
*/

#define 	BACKSPACE_HEX		0x08
#define 	ESC_HEX				0x1b
#define	K_F_FLAG			0x4f
#define	K_SPECAIL_FLAG		0x5b

unsigned long aui_get_user_cmd_string(char *sz_input,char *sz_cmd,unsigned char *argc,char **argv)
{
	//unsigned long timeout = 1000;
	unsigned char by_rcv_char_tmp=0;
	unsigned long i=0;
	unsigned long ui_cmdIDEnd_pos=0;
	unsigned long ui_param_cnt=0;
	unsigned long ui_param_pos_str=0;
	unsigned long ui_param_pos_end=0;
	unsigned long bl_rcv_total_cmd=0;
	//unsigned long ul_param_len=0;
	unsigned long ul_find_sp=0;
	unsigned long ul_input_total_len=0;
	char acTmp[USER_CMD_PARAM_STR_LEN_MAX+1]={0};
	//char *psz_para=argv;

	if((NULL==sz_cmd)||(NULL==argc)||(NULL==argv))
	{
		return -1;
	}

#ifndef AUI_TDS	
	char* pchar = NULL;//added by steven@20161130
    memset(g_st_cmd_dev.pby_main_cmd_buf, 0, UART_CMD_STR_LEN_MAX + 1);
    //scanf("%[0-9a-zA-Z, ]",g_st_cmd_dev.pby_main_cmd_buf);
    ///scanf("%s %s",g_st_cmd_dev.pby_main_cmd_buf,ac_para);
    //gets(g_st_cmd_dev.pby_main_cmd_buf);
    if (NULL != stream) {//added by steven@20161010 for sample code support script.
        if(NULL == fgets(g_st_cmd_dev.pby_main_cmd_buf, UART_CMD_STR_LEN_MAX - 1, stream)) {
                if (stream)
			fclose(stream);//handle end of file
		stream = NULL;
        } else {
			char special_char = '#';
        	pchar = strchr(g_st_cmd_dev.pby_main_cmd_buf, special_char);
			if (pchar) {
				*pchar = '\0';
				if (g_st_cmd_dev.pby_main_cmd_buf[0] == '\0') {//handle commented
					char cmd[] = "skip";
					strncpy(g_st_cmd_dev.pby_main_cmd_buf, cmd, sizeof(cmd));
				}
			}
        }
    } else {
       if(NULL == fgets(g_st_cmd_dev.pby_main_cmd_buf, UART_CMD_STR_LEN_MAX - 1, stdin)) {
                if (stream)
		        fclose(stream);//handle end of file
		stream = NULL;
	   }
    }

    if(strlen(g_st_cmd_dev.pby_main_cmd_buf)<=0)
    {
        return -1;
    }

//	if(!((1 == strlen(g_st_cmd_dev.pby_main_cmd_buf)) && ('\n' == g_st_cmd_dev.pby_main_cmd_buf[0])))
//	{
//		strcat(g_st_cmd_dev.pby_main_cmd_buf,"\n");
//	}
	strcpy(sz_input, (const char *)(g_st_cmd_dev.pby_main_cmd_buf));
	#endif

	for(i=0;i<8;i++)
	{
		if(g_st_cmd_dev.ui_main_cmd_rcv_cur_idx!=0)
		{
			//AUI_PRINTF("\r\n[%02x][%d]\r\n",sz_input[i],g_st_cmd_dev.ui_main_cmd_rcv_cur_idx);
		}
	}

	ul_find_sp=0;

	MEMSET(&(g_st_aui_cmd_str.ac_cmdID),0,UART_CMD_ID_LEN_MAX);
	g_st_aui_cmd_str.ui_cmd_param_cnt=0;
	MEMSET(&(g_st_aui_cmd_str.ac_cmd_out_put),0,UART_CMD_OUTPUT_LEN_MAX+1);
	MEMSET(s_argv,0,USER_CMD_PARAM_COUNT_MAX*USER_CMD_PARAM_STR_LEN_MAX);

	ul_input_total_len = strlen(sz_input);
	//int icount = 0;
	if(ul_input_total_len > 0)
	{
		#if 0
		AUI_PRINTF("sz_input[len: %d]: ", ul_input_total_len);
		for(icount = 0; icount < ul_input_total_len; icount++)
		{
			AUI_PRINTF("0x%02x ", sz_input[icount]);
		}
		AUI_PRINTF("\n");
		#endif
		for(i=0;i<ul_input_total_len;i++)
		{
			if(i<UART_CMD_STR_LEN_MAX)
			{
				by_rcv_char_tmp=sz_input[i];
				unsigned long ncpy = 0;
				switch(by_rcv_char_tmp)
				{
					case ' ':
						ui_cmdIDEnd_pos=i;
						ui_param_pos_str=i+1;
						strncpy(sz_cmd,sz_input,ui_cmdIDEnd_pos);
						sz_input[i]=by_rcv_char_tmp;
						ul_find_sp=i;
						break;
					case ',':
						ui_param_pos_end=i;
						ncpy = ui_param_pos_end < ui_param_pos_str
								? 0 : ui_param_pos_end - ui_param_pos_str;
						strncpy(argv[ui_param_cnt], sz_input + ui_param_pos_str,
								ncpy);
						ui_param_pos_str=ui_param_pos_end+1;
						ui_param_cnt++;
						sz_input[i]=by_rcv_char_tmp;
						break;
					case '\r':
					case '\n':
						if(0!=ui_param_pos_end)
						{
							ui_param_pos_end=i;
							ncpy = ui_param_pos_end < ui_param_pos_str
									? 0 : ui_param_pos_end - ui_param_pos_str;
							strncpy(argv[ui_param_cnt],
									sz_input + ui_param_pos_str, ncpy);
							ui_param_pos_str=ui_param_pos_end+1;
							ui_param_cnt++;
							sz_input[i]=by_rcv_char_tmp;
						}
						else if(ul_find_sp)
						{
							if(i>ul_find_sp+1)
							{
								ncpy = (i - ul_find_sp - 1) <= 0
										? 0 : i - ul_find_sp - 1;
                                if (ncpy >= USER_CMD_PARAM_STR_LEN_MAX)
                                {
                                    // parameter can' not large than USER_CMD_PARAM_STR_LEN_MAX
                                    return -1;
                                }
                                if (ui_param_cnt >= USER_CMD_PARAM_COUNT_MAX)
                                {
                                    // parameter count error
                                    return -1;
                                }
								strncpy(acTmp, sz_input + ul_find_sp + 1, ncpy);
								strcpy(argv[ui_param_cnt],acTmp);
								ui_param_cnt++;
							}
						}

						if(!((ul_input_total_len == 1) && (sz_input[i] == 0x0D)))
						{
							sz_input[i]='\0';
						}

						bl_rcv_total_cmd=1;
						break;
					case BACKSPACE_HEX:
						sz_input[i]='\0';

						if(i > 0)
						{
							sz_input[i-1]='\0';

							g_st_cmd_dev.ui_main_cmd_rcv_cur_idx--;
						}

						g_st_cmd_dev.ui_main_cmd_rcv_cur_idx--;
						AUI_PRINTF("\raui>%s \b", sz_input);
						break;

					case ESC_HEX:
						if(sz_input[i +1] != 0 && sz_input[i +1] != 0)
						{
							if((sz_input[i+1] == K_F_FLAG) || (sz_input[i+1] == K_SPECAIL_FLAG))
							{
								sz_input[i]='\0';
								sz_input[i+1]='\0';
								sz_input[i+2]='\0';
								//MEMCPY(&sz_input[i], 0, ul_input_total_len-(i+3));
								if((ul_input_total_len - (i+3)) > 0)
								{
									MEMCPY(&sz_input[i], &sz_input[i+3], ul_input_total_len-(i+3));
									sz_input[ul_input_total_len-1]='\0';
									sz_input[ul_input_total_len-2]='\0';
									sz_input[ul_input_total_len-3]='\0';
								}
								g_st_cmd_dev.ui_main_cmd_rcv_cur_idx -=3;
								AUI_PRINTF("\raui>%s   \b\b\b", sz_input);
							}
						}
						break;

					default:
						if(by_rcv_char_tmp > 0x1F)
						{
							sz_input[i]=by_rcv_char_tmp;
						}
						break;
				}

			}
		}

		if(0==ui_cmdIDEnd_pos)
		{
			strncpy(sz_cmd,sz_input,UART_CMD_ID_LEN_MAX);
		}

		*argc=ui_param_cnt;

		#ifdef AUI_LINUX
		bl_rcv_total_cmd=1;
		#endif

		if((strlen(sz_cmd)>0)&&(bl_rcv_total_cmd))
		{
			return 0;
		}
		else
		{
			return -1;
		}

	}
	return -1;
}



/**
*@brief parse the string user input and get user response during the executing of main command.
*@author ray.gong
*@date 2013-4-8
*@param[in] sz_input the main command that user input.
*@param[out] sz_cmd the main command id that user input.
*@param[out] argc the main command parameter counter that user input.
*@param[out] argv the main command parameter that user input.
*@return AUI_RTN_CODE_SUCCESS the function successful return.
*@return AUI_RTN_CODE_ERR the function failed return.
*@note
*parse the user response that user input and get command id,parameter counter and parameter.
*but in aui test appliation.the parameter is not used.
*/
unsigned long aui_get_user_cmd_string_ex(char *sz_input,char *sz_cmd,unsigned long *argc,char *argv[])
{
    //unsigned long timeout = 1000;
	//unsigned char by_rcv_char_tmp=0;
	unsigned long i=0;
	//unsigned long ui_cmdIDEnd_pos=0;
	//unsigned long ui_param_cnt=0;
	//unsigned long ui_param_pos_str=0;
	//unsigned long ui_param_pos_end=0;
	//unsigned long bl_rcv_total_cmd=0;

	if(1==g_st_cmd_dev.ui_cmd_status)
	{
		#ifdef AUI_TDS
		if(0!=drvGetChar4Nocs())
		{
			AUI_PRINTF("%c",drvGetChar4Nocs());
			drvSetChar4Nocs(0);
		}
		#endif
	}
	if((NULL==sz_cmd))
	{
		return -1;
	}

	#ifndef AUI_TDS
	struct timeval tv;
	fd_set read_fd;
	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);
	//will not block when we call fgets
	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
		return -1;
	memset(g_st_cmd_dev.pby_ask_cmd_buf, 0, UART_CMD_STR_LEN_MAX+1);
	//scanf("%[0-9a-zA-Z, ]",g_st_cmd_dev.pby_main_cmd_buf);
	///scanf("%s %s",g_st_cmd_dev.pby_main_cmd_buf,ac_para);
	//gets(g_st_cmd_dev.pby_ask_cmd_buf);
	if (FD_ISSET(0,&read_fd))
		if(NULL == fgets(g_st_cmd_dev.pby_ask_cmd_buf, UART_CMD_STR_LEN_MAX - 1, stdin)) {
                        if (stream)
			        fclose(stream);//handle end of file
		        stream = NULL;
		}
	//scanf("%s",g_st_cmd_dev.pby_main_cmd_buf);
	if(strlen(g_st_cmd_dev.pby_ask_cmd_buf)<=0)
	{
		return -1;
	}
//    strcat(g_st_cmd_dev.pby_ask_cmd_buf,"\n");
	strcpy(sz_input,(g_st_cmd_dev.pby_ask_cmd_buf));

	#endif


    for(i=0;i<UART_CMD_STR_LEN_MAX;i++)
    {
        if(('\r'==g_st_cmd_dev.pby_ask_cmd_buf[i])||('\n'==g_st_cmd_dev.pby_ask_cmd_buf[i]))
        {
            g_st_cmd_dev.pby_ask_cmd_buf[i]=0;
            return 0;
        }
    }

#if 0
	for(i=0;i<8;i++)
	{
		if(g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx!=0)
		{
			//AUI_PRINTF("\r\n[%02x][%d]\r\n",sz_input[i],g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx);
		}
	}
	if(strlen(sz_input)>0)
	{
		for(i=0;i<strlen(sz_input);i++)
		{
			if(i<UART_CMD_STR_LEN_MAX)
			{
				by_rcv_char_tmp=sz_input[i];

				switch(by_rcv_char_tmp)
				{
					case ' ':

						ui_cmdIDEnd_pos=i;
						ui_param_pos_str=i+1;
						strncpy(sz_cmd,sz_input,ui_cmdIDEnd_pos);
						sz_input[i]=by_rcv_char_tmp;
						break;
					case ',':

						ui_param_pos_end=i;
					    strncpy(argv[ui_param_cnt],sz_input+ui_param_pos_str,((ui_param_pos_end-ui_param_pos_str)<0)?0:(ui_param_pos_end-ui_param_pos_str));
						ui_param_pos_str=ui_param_pos_end+1;
						ui_param_cnt++;
						sz_input[i]=by_rcv_char_tmp;
						break;
					case '\r':
					case '\n':

						//if(0!=ui_param_pos_end)
						{

							ui_param_pos_end=i;
							strncpy(argv[ui_param_cnt],sz_input+ui_param_pos_str,((ui_param_pos_end-ui_param_pos_str)<0)?0:(ui_param_pos_end-ui_param_pos_str));
							ui_param_pos_str=ui_param_pos_end+1;
							ui_param_cnt++;
							sz_input[i]=by_rcv_char_tmp;
						}
						sz_input[i]='\0';
						bl_rcv_total_cmd=1;
						break;
					default:
						sz_input[i]=by_rcv_char_tmp;
						break;
				}

			}
	    }

		if(0==ui_cmdIDEnd_pos)
		{
			strncpy(sz_cmd,sz_input,UART_CMD_ID_LEN_MAX);
		}

		//*argc=ui_param_cnt;

		//MEMSET(sz_input,0,UART_CMD_STR_LEN_MAX+1);
		if((strlen(sz_cmd)>0)&&(bl_rcv_total_cmd))
		{
			return 0;
		}
		else
		{
			return -1;
		}

	}
	return -1;
#endif
    return -1;
}

/**
*@brief get a char from terminal
*@author ray.gong
*@param[in] by_rcv_cmd_char the char get from terminal
*@return void
*@note
*get a char from terminal,it could be serial,net ,memory or file.
*/
void drv_get_cmd_char(unsigned char by_rcv_cmd_char)
{
	if(0!=by_rcv_cmd_char)
	{
		if(1==g_st_cmd_dev.ui_cmd_status)
		{
			if(g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx!=strlen(g_st_cmd_dev.pby_ask_cmd_buf))
			{
				rst_ask_cmd_status();
			}
			if((NULL!=g_st_cmd_dev.pby_ask_cmd_buf)&&(g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx<UART_CMD_STR_LEN_MAX))
			{
				if(0x7f==by_rcv_cmd_char)
				{
					if(g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx<=0)
					{
						g_st_cmd_dev.pby_ask_cmd_buf[0]=0;
						g_st_cmd_dev.pby_ask_cmd_buf[1]=0;
					}
					else
					{
						g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx--;
						g_st_cmd_dev.pby_ask_cmd_buf[g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx]=0;
						g_st_cmd_dev.pby_ask_cmd_buf[g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx+1]=0;
					}
				}
				else if((g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx>0)
					&&(('\r'==g_st_cmd_dev.pby_ask_cmd_buf[g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx])
						||('\n'==g_st_cmd_dev.pby_ask_cmd_buf[g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx])))
				{
					;
				}
				else
				{
					g_st_cmd_dev.pby_ask_cmd_buf[g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx++]=by_rcv_cmd_char;
				}
			}


		}
		else
		{
			if(g_st_cmd_dev.ui_main_cmd_rcv_cur_idx!=strlen(g_st_cmd_dev.pby_main_cmd_buf))
			{
				rst_main_cmd_status();
			}
			if((NULL!=g_st_cmd_dev.pby_main_cmd_buf)&&(g_st_cmd_dev.ui_main_cmd_rcv_cur_idx<UART_CMD_STR_LEN_MAX))
			{
				if(0x7f==by_rcv_cmd_char)
				{
					if(g_st_cmd_dev.ui_main_cmd_rcv_cur_idx<=0)
					{
						g_st_cmd_dev.pby_main_cmd_buf[0]=0;
						g_st_cmd_dev.pby_main_cmd_buf[1]=0;
					}
					else
					{
						g_st_cmd_dev.ui_main_cmd_rcv_cur_idx--;
						g_st_cmd_dev.pby_main_cmd_buf[g_st_cmd_dev.ui_main_cmd_rcv_cur_idx]=0;
						g_st_cmd_dev.pby_main_cmd_buf[g_st_cmd_dev.ui_main_cmd_rcv_cur_idx+1]=0;
					}
				}
				else if((g_st_cmd_dev.ui_main_cmd_rcv_cur_idx>0)
					&&(('\r'==g_st_cmd_dev.pby_main_cmd_buf[g_st_cmd_dev.ui_main_cmd_rcv_cur_idx])
						||('\n'==g_st_cmd_dev.pby_main_cmd_buf[g_st_cmd_dev.ui_main_cmd_rcv_cur_idx])))
				{
					;
				}
				else
				{
					g_st_cmd_dev.pby_main_cmd_buf[g_st_cmd_dev.ui_main_cmd_rcv_cur_idx++]=by_rcv_cmd_char;
				}
			}


		}
	}
	//#endif

}

/**
*@brief execute the function register in command list.
*@author ray.gong
*@date 2013-4-8
*@note
*this function not need return value.it print the result to terminal directly for user.
*/

void drv_process_cmd()
{
    unsigned long ui_rtn=-1;
    unsigned long i=0;
    int k=0;
    unsigned long b_find_cmd=0;
    unsigned long i_cnt=USER_CMD_COUNT_MAX;

    if(1!=g_st_cmd_dev.ui_cmd_status) {
#ifdef AUI_TDS
        if(0!=drvGetChar4Nocs()) {
            AUI_PRINTF("%c",drvGetChar4Nocs());
            drvSetChar4Nocs(0);
        }
#endif
    }
    ui_rtn = aui_get_user_cmd_string(g_st_cmd_dev.pby_main_cmd_buf,
            g_st_aui_cmd_str.ac_cmdID,
            (unsigned char *)&(g_st_aui_cmd_str.ui_cmd_param_cnt),
            g_st_aui_cmd_str.argv);
    if(0==ui_rtn) {
        if(0==strcmp("0",g_st_aui_cmd_str.ac_cmdID)) {
            //AUI_PRINTF("\r\n_your choice is :%s.\r\n",g_st_cmd_dev.pby_main_cmd_buf);
            ui_rtn=aui_str_cmd_print_cur_menu();

            if(0==ui_rtn) {
                //AUI_PRINTF("\r\n[RTN]:SUCCESSFUL.");
            } else {
                //AUI_PRINTF("\r\n[RTN]:FAILED.");
            }
            b_find_cmd=1;
        }
#ifdef AUI_LINUX
        if (0 == strcmp("wait", g_st_aui_cmd_str.ac_cmdID)) {
            AUI_PRINTF("process %s\n", g_st_aui_cmd_str.ac_cmdID);
            AUI_SLEEP(atoi(g_st_aui_cmd_str.argv[0]));
            b_find_cmd=1;
        }
        else if (0 == strcmp("wait_ir", g_st_aui_cmd_str.ac_cmdID)) {
            AUI_PRINTF("process %s\n", g_st_aui_cmd_str.ac_cmdID);
            b_find_cmd=1;
            AUI_PRINTF("Please press any key!\n");
            wait_ir();
        }
        else if (0 == strcmp("skip", g_st_aui_cmd_str.ac_cmdID)) {
            AUI_PRINTF("process %s\n", g_st_aui_cmd_str.ac_cmdID);
            b_find_cmd = 1;
        }
#endif
        else if ((0 == strcmp("run",g_st_aui_cmd_str.ac_cmdID))
                || (0 == strcmp("RUN",g_st_aui_cmd_str.ac_cmdID)))
        {
            //AUI_PRINTF("\r\n_your choice is :%s.\r\n",g_st_cmd_dev.pby_main_cmd_buf);
            ui_rtn=aui_str_cmd_run_level();

            if(0==ui_rtn) {
                //AUI_PRINTF("\r\n[RTN]:SUCCESSFUL.");
            } else {
                //AUI_PRINTF("\r\n[RTN]:FAILED.");
            }
            b_find_cmd=1;
        } else if((0==strcmp("up",g_st_aui_cmd_str.ac_cmdID))||(0==strcmp("UP",g_st_aui_cmd_str.ac_cmdID))) {
            //AUI_PRINTF("\r\n_your choice is :%s.\r\n",g_st_cmd_dev.pby_main_cmd_buf);
            ui_rtn=aui_str_cmd_ent_and_print_upper_menu();

            if(0==ui_rtn) {
                //AUI_PRINTF("\r\n[RTN]:SUCCESSFUL.");
            } else {
                //AUI_PRINTF("\r\n[RTN]:FAILED.");
            } b_find_cmd=1;
        } else if((0==strcmp("root",g_st_aui_cmd_str.ac_cmdID))||(0==strcmp("ROOT",g_st_aui_cmd_str.ac_cmdID))) {
            //AUI_PRINTF("\r\n_your choice is :%s.\r\n",g_st_cmd_dev.pby_main_cmd_buf);
            ui_rtn=aui_str_cmd_print_main_menu();

            if(0==ui_rtn) {
                //AUI_PRINTF("\r\n[RTN]:SUCCESSFUL.");
            } else {
                //AUI_PRINTF("\r\n[RTN]:FAILED.");
            }
            b_find_cmd=1;
        }
        else if((0==strcmp("quit",g_st_aui_cmd_str.ac_cmdID))||(0==strcmp("QUIT",g_st_aui_cmd_str.ac_cmdID)))
        {
            //AUI_PRINTF("\r\n_your choice is :%s.\r\n",g_st_cmd_dev.pby_main_cmd_buf);
            ui_rtn=aui_str_cmd_quit();


            if(0==ui_rtn)
            {
                //AUI_PRINTF("\r\n[RTN]:SUCCESSFUL.");
            }
            else
            {
                //AUI_PRINTF("\r\n[RTN]:FAILED.");
            }
            b_find_cmd=1;
        }
        else if((1 == strlen(g_st_aui_cmd_str.ac_cmdID)) && (0xd == g_st_aui_cmd_str.ac_cmdID[0]))
        {
            b_find_cmd=1;
        }
        else
        {
            for(i=g_st_aui_cmd_cur_status.ui_total_idx;
                    ((i<i_cnt)&&(0!=strcmp(g_st_aui_cmd_str.ac_cmdID,"NULL"))&&(0!=strcmp(g_st_aui_cmd_str.ac_cmdID,"")));
                    i++)
            {
                if((g_st_aui_cmd_cur_status.by_menu_level>g_pst_aui_cmd_tree[i].by_menu_level)
                        &&(i>g_st_aui_cmd_cur_status.ui_total_idx))
                {
                    b_find_cmd=0;
                    break;
                }
                g_st_cmd_dev.ui_cmd_status=1;
                if((0==strcmp(g_st_aui_cmd_str.ac_cmdID,g_pst_aui_cmd_tree[i].ac_cmdID))
                        &&(g_st_aui_cmd_cur_status.by_menu_level==g_pst_aui_cmd_tree[i].by_menu_level)
                        &&(i>=g_st_aui_cmd_cur_status.ui_total_idx)) {
                    if((unsigned long)(g_pst_aui_cmd_tree[i].cmd_handle)==(unsigned long)aui_str_cmd_print_menu) {
                        g_st_aui_cmd_cur_status.ui_total_idx=i;
                    }
                    ui_rtn=g_pst_aui_cmd_tree[i].cmd_handle(&(g_st_aui_cmd_str.ui_cmd_param_cnt),g_st_aui_cmd_str.argv,g_st_aui_cmd_str.ac_cmd_out_put);

                    //AUI_PRINTF("\r\n_your choice is :%s.\r\n",g_st_cmd_dev.pby_main_cmd_buf);
                    AUI_PRINTF("\r\n %s",g_st_aui_cmd_str.ac_cmd_out_put);
                    //AUI_PRINTF("\r\n[%d][%d][%08x][%08x]\r\n",i,g_pst_aui_cmd_tree[i].ul_cmd_type,aui_str_cmd_print_menu,g_pst_aui_cmd_tree[i].cmd_handle);
                    //if(aui_str_cmd_print_menu!=g_pst_aui_cmd_tree[i].cmd_handle)
                    if(AUI_CMD_TYPE_MENU!=g_pst_aui_cmd_tree[i].ul_cmd_type)
                    {
                        if(0==ui_rtn)
                        {
                            AUI_PRINTF("\r\n[%s]",g_pst_aui_cmd_tree[i].ac_cmd_instuction);
                            for( k=0;k<(int)(70-strlen(g_pst_aui_cmd_tree[i].ac_cmd_instuction));k++)
                            {
                                AUI_PRINTF(".");
                            }
                            AUI_PRINTF("PASS");
                            //AUI_PRINTF("%s\r\n[%s][RTN]:PASS.",g_pst_aui_cmd_tree[i].ac_cmd_instuction,g_st_aui_cmd_str.ac_cmd_out_put);
                        }
                        else if(AUI_RTN_HELP == ui_rtn)
                        {
                            ;
                        }
                        else
                        {
                            AUI_PRINTF("\r\n[%s]",g_pst_aui_cmd_tree[i].ac_cmd_instuction);
                            for( k=0;k<(int)(70-strlen(g_pst_aui_cmd_tree[i].ac_cmd_instuction));k++)
                            {
                                AUI_PRINTF(".");
                            }
                            AUI_PRINTF("FAIL");
                            //AUI_PRINTF("%s\r\n[%s][RTN]:FAIL.",g_pst_aui_cmd_tree[i].ac_cmd_instuction,g_st_aui_cmd_str.ac_cmd_out_put);
                        }
                    }
                    b_find_cmd=1;
                    g_st_cmd_dev.ui_cmd_status=0;
                    break;
                }

			}
		}

		if(!b_find_cmd)
		{
			g_st_cmd_dev.ui_cmd_status=0;
			AUI_PRINTF("\r\n_command:%s",g_st_cmd_dev.pby_main_cmd_buf);
			AUI_PRINTF("\r\n_undefine command!");
		}

		g_st_cmd_dev.ui_main_cmd_rcv_cur_idx=0;
		g_st_cmd_dev.ui_ask_cmd_rcv_cur_idx=0;
		MEMSET(g_st_cmd_dev.pby_main_cmd_buf,0,UART_CMD_STR_LEN_MAX+1);
		MEMSET(g_st_cmd_dev.pby_ask_cmd_buf,0,UART_CMD_STR_LEN_MAX+1);

		MEMSET(&(g_st_aui_cmd_str.ac_cmdID),0,UART_CMD_ID_LEN_MAX);
		g_st_aui_cmd_str.ui_cmd_param_cnt=0;
		MEMSET(&(g_st_aui_cmd_str.ac_cmd_out_put),0,UART_CMD_OUTPUT_LEN_MAX+1);

		g_st_aui_cmd_str.ui_cmd_param_cnt=0;
		MEMSET(g_st_aui_cmd_str.ac_cmdID,0,UART_CMD_ID_LEN_MAX+1);
		MEMSET(g_st_aui_cmd_str.ac_cmd_out_put,0,UART_CMD_OUTPUT_LEN_MAX+1);

		AUI_PRINTF("\r\naui>");
	}
	else
	{
		b_find_cmd=0;
	}

}

/**
*@brief get the task status of aui command process.
*@author ray.gong
*@date 2013-4-8
*@return 1 the task is running.
*@return 0 the task is stoped.
*@note
*this task status only be change by user command 'q'.
*/

unsigned long drv_get_aui_cmd_task_status()
{
	return g_st_cmd_dev.bl_aui_scpi_task_flag;
}

/**
*@brief set the task status of aui command process.
*@author ray.gong
*@date 2013-4-8
*@param[in] bl_run the task run status.TURE:running;0:stop
*@note
*this task status only be change by user command 'q'.
*/
void drv_set_aui_cmd_task_status(unsigned long bl_run)
{
	g_st_cmd_dev.bl_aui_scpi_task_flag=bl_run;
}

/**
*@brief printf some useful information after system startup.
*@author ray.gong
*@date 2013-4-8
*@note
*the information include title and version.these information could be update according to the requirement.
*/

void drv_print_logo()
{
	int max_len = 1024;
	char aui_package_info[max_len];
	aui_log_package_info_get(aui_package_info, max_len);
	AUI_PRINTF("\r\n%s", aui_package_info);

	AUI_PRINTF("\r\n********************************************************************************");
	AUI_PRINTF("\r\n*                                                                              *");
	AUI_PRINTF("\r\n*                       AUI test application main MENU                        *");
	AUI_PRINTF("\r\n*                                                               app ver:%s  *",AUI_TEST_APP_VER);
	AUI_PRINTF("\r\n********************************************************************************");
}
#ifdef AUI_TDS
void drv_comCB(unsigned char by_char)
{
	drvSetChar4Nocs(by_char);
	drv_get_cmd_char(by_char);
}
#endif

/**
*@brief initialize the var,malloc the memory for program and execute some operate only once at first.
*@author ray.gong
*@date 2013-4-8
*@return AUI_RTN_CODE_SUCCESS the function successful return.
*@return AUI_RTN_CODE_ERR the function failed return.
*@note
*such as the printf logo at first.
*/

unsigned char auiscpi_init()
{
	unsigned long	i;

	#ifdef AUI_TDS
	g_ComIntrruptCB=drv_comCB;
	#endif
	g_pst_aui_cmd_tree=g_st_aui_cmd_node;
	MEMSET(&g_st_cmd_dev,0,sizeof(st_cmd_dev));
	g_st_cmd_dev.bl_aui_scpi_task_flag=1;

	g_st_cmd_dev.pby_ask_cmd_buf=MALLOC(UART_CMD_STR_LEN_MAX+1);
	if(NULL == g_st_cmd_dev.pby_ask_cmd_buf)
	{
		AUI_PRINTF("\nmalloc pby_ask_cmd_buf failed\n");
		return 0;
	}
	MEMSET(g_st_cmd_dev.pby_ask_cmd_buf,0,UART_CMD_STR_LEN_MAX+1);

	g_st_cmd_dev.pby_main_cmd_buf=MALLOC(UART_CMD_STR_LEN_MAX+1);
	if(NULL == g_st_cmd_dev.pby_main_cmd_buf)
	{
		AUI_PRINTF("\nmalloc pby_main_cmd_buf failed\n");
		return 0;
	}
	MEMSET(g_st_cmd_dev.pby_main_cmd_buf,0,UART_CMD_STR_LEN_MAX+1);
	MEMSET(&g_st_aui_cmd_str,0,sizeof(st_aui_cmd));
	MEMSET(&g_st_aui_cmd_cur_status,0,sizeof(st_aui_cmd_cur_status));
	g_st_aui_cmd_cur_status.by_menu_level=1;
	#if 1

	MEMSET(s_argv,0,USER_CMD_PARAM_STR_LEN_MAX*USER_CMD_PARAM_COUNT_MAX);

	for(i=0;i<USER_CMD_PARAM_COUNT_MAX;i++)
	{
        g_st_aui_cmd_str.argv[i]=s_argv[i];//MALLOC(USER_CMD_PARAM_STR_LEN_MAX);
	}

	#endif
    for(i=0;i<USER_CMD_COUNT_MAX+1;i++)
	{
        aui_tu_reg_item(1,"NULL",0,NULL,"NULL");
	}
    __rst_reg_cnt();
	drv_print_logo();
#if 0
	if(AUI_RTN_CODE_SUCCESS!=aui_str_cmd_print_main_menu())
	{
		return 0;
	}
#endif
	return 1;
}

static char char2hex(unsigned char ch)
{
	char ret =  - 1;
	if ((ch <= 0x39) && (ch >= 0x30))
	// '0'~'9'
		ret = ch &0xf;
	else if ((ch <= 102) && (ch >= 97))
	//'a'~'f'
		ret = ch - 97+10;
	else if ((ch <= 70) && (ch >= 65))
	//'A'~'F'
		ret = ch - 65+10;

	return ret;
}
static char char2byte(unsigned char ch)
{
	char ret =  - 1;
	if ((ch <= 0x39) && (ch >= 0x30))
	// '0'~'9'
		ret = ch &0xf;

	return ret;
}
static unsigned long drv_str2UINT32(unsigned long ui_mode,unsigned char *str, unsigned char len)
{
	unsigned long ret = 0;
	unsigned char i;
	int temp;

	if (str == NULL)
		return 0;

	if(0==ui_mode)
	{
		for (i = 0; i < len; i++)
		{
			temp = char2byte(*str++);
			if (temp ==  - 1)
				return -1;

			ret=ret*10+temp;
		}
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			temp = char2hex(*str++);
			if (temp ==  - 1)
				return 0;

			ret = (ret << 4) | temp;
		}
	}
	return ret;
}


/**
*    @brief 		let user to confirm the test result
*    @author		ray.gong
*    @date			2013-6-25
*    @return 		AUI_RTN_CODE_SUCCESS the function successful return.
*    @return 		AUI_RTN_CODE_ERR the function failed return.
*    @note
*
*/
unsigned char aui_test_user_confirm(char *sz_info)
{
	unsigned long ul_rtn=AUI_RTN_CODE_ERR;
	st_aui_cmd st_aui_cmd_str;
	if(NULL==sz_info)
	{
		return AUI_RTN_CODE_ERR;
	}
	MEMSET(&st_aui_cmd_str,0,sizeof(st_aui_cmd));
	AUI_PRINTF("\r\n%s",sz_info);
	while(1)
	{
		if(NULL==get_ask_cmd_buf_addr())
		{
			return AUI_RTN_CODE_ERR;
		}
	    ul_rtn = aui_get_user_cmd_string_ex(get_ask_cmd_buf_addr(),st_aui_cmd_str.ac_cmdID,&(st_aui_cmd_str.ui_cmd_param_cnt),st_aui_cmd_str.argv);
		if(0==ul_rtn)
		{
			if((0==drv_compare_ask_cmd("y"))||(0==drv_compare_ask_cmd("Y")))
			{
				return 1;
			}
			if((0==drv_compare_ask_cmd("n"))||(0==drv_compare_ask_cmd("N")))
			{
				return 0;
			}
			else
			{
				rst_ask_cmd_status();
			}
		}

		AUI_SLEEP(100);
	}
	return 0;
}


/**
*    @brief 		let user to confirm the test result
*    @author		ray.gong
*    @date			2013-6-25
*    @return 		AUI_RTN_CODE_SUCCESS the function successful return.
*    @return 		AUI_RTN_CODE_ERR the function failed return.
*    @note
*
*/
AUI_RTN_CODE aui_test_get_user_hex_input(unsigned long *pul_user_in)
{
	unsigned long ul_rtn=AUI_RTN_FAIL;
	st_aui_cmd st_aui_cmd_str;

	MEMSET(&st_aui_cmd_str,0,sizeof(st_aui_cmd));

	while(1)
	{
		if(NULL==get_ask_cmd_buf_addr())
		{
			return AUI_RTN_FAIL;
		}
	    ul_rtn = aui_get_user_cmd_string_ex(get_ask_cmd_buf_addr(),st_aui_cmd_str.ac_cmdID,&(st_aui_cmd_str.ui_cmd_param_cnt),st_aui_cmd_str.argv);
		if(0==ul_rtn)
		{
			*pul_user_in=drv_str2UINT32(1,get_ask_cmd_buf_addr(),strlen(get_ask_cmd_buf_addr()));
			rst_ask_cmd_status();
			break;

		}

		AUI_SLEEP(100);
	}
	return AUI_RTN_SUCCESS;
}


/**
*    @brief 		let user to confirm the test result
*    @author		ray.gong
*    @date			2013-6-25
*    @return 		AUI_RTN_CODE_SUCCESS the function successful return.
*    @return 		AUI_RTN_CODE_ERR the function failed return.
*    @note
*
*/
AUI_RTN_CODE aui_test_get_user_dec_input(unsigned long *pul_user_in)
{
	unsigned long ul_rtn=AUI_RTN_FAIL;
	st_aui_cmd st_aui_cmd_str;

	MEMSET(&st_aui_cmd_str,0,sizeof(st_aui_cmd));

	while(1)
	{
		if(NULL==get_ask_cmd_buf_addr())
		{
			return AUI_RTN_FAIL;
		}
	    ul_rtn = aui_get_user_cmd_string_ex(get_ask_cmd_buf_addr(),st_aui_cmd_str.ac_cmdID,&(st_aui_cmd_str.ui_cmd_param_cnt),st_aui_cmd_str.argv);
		if(0==ul_rtn)
		{
			*pul_user_in=drv_str2UINT32(0,get_ask_cmd_buf_addr(),strlen(get_ask_cmd_buf_addr()));
			rst_ask_cmd_status();
			break;

		}

		AUI_SLEEP(100);
	}
	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_test_get_user_str_input(char *pul_user_in)
{
	unsigned long ul_rtn=AUI_RTN_FAIL;
	st_aui_cmd st_aui_cmd_str;

	MEMSET(&st_aui_cmd_str,0,sizeof(st_aui_cmd));

	while(1)
	{
		if(NULL==get_ask_cmd_buf_addr())
		{
			return AUI_RTN_FAIL;
		}
	    ul_rtn = aui_get_user_cmd_string_ex(get_ask_cmd_buf_addr(),st_aui_cmd_str.ac_cmdID,&(st_aui_cmd_str.ui_cmd_param_cnt),st_aui_cmd_str.argv);
		if(0==ul_rtn)
		{
			STRCPY(pul_user_in,get_ask_cmd_buf_addr());
			rst_ask_cmd_status();
			break;

		}

		AUI_SLEEP(100);
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_test_get_user_key_input(char *pul_user_in)
{
	unsigned long ul_rtn=AUI_RTN_FAIL;
	st_aui_cmd st_aui_cmd_str;

	MEMSET(&st_aui_cmd_str,0,sizeof(st_aui_cmd));

	if(NULL==get_ask_cmd_buf_addr())
	{
		return AUI_RTN_FAIL;
	}
    ul_rtn = aui_get_user_cmd_string_ex(get_ask_cmd_buf_addr(),st_aui_cmd_str.ac_cmdID,&(st_aui_cmd_str.ui_cmd_param_cnt),st_aui_cmd_str.argv);
	if(0==ul_rtn)
	{
		STRCPY(pul_user_in,get_ask_cmd_buf_addr());
		rst_ask_cmd_status();
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE str2ulong(unsigned char *psz_in, unsigned long ul_str_len, unsigned long *pul_out)
{
	unsigned long ul_rtn = 0;
	unsigned char i;
	int i_tmp;

	if ((psz_in == NULL)||(NULL==pul_out))
	{
		return 0;
	}

	for (i = 0; i < ul_str_len; i++)
	{
		i_tmp = char2hex(*psz_in++);
		if (i_tmp ==  - 1)
		{
			return AUI_RTN_FAIL;
		}

		*pul_out = ((*pul_out) << 4) | i_tmp;
	}

	return ul_rtn;

}

