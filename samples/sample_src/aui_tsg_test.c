/****************************INCLUDE HEAD FILE************************************/
#include "aui_tsg_test.h"

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************EXTERN VAR*******************************************/
extern aui_handle_tsg *g_p_hdl_tsg;
FILE *g_auipf=NULL;

/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/

unsigned long test_tsg_open_file(unsigned long *argc,char **argv,char *sz_out_put)
{
	//strcpy(g_ac_tmp,"r");
	g_auipf=fopen("/mnt/uda1/2ch_datarate_256_2_dd_h264.trp","r");
	if(NULL==g_auipf)
	{
		return AUI_RTN_EINVAL;
	}
	return AUI_RTN_SUCCESS;
}
unsigned long test_tsg_close_file(unsigned long *argc,char **argv,char *sz_out_put)
{
	if(NULL==g_auipf)
	{
		return AUI_RTN_EINVAL;
	}	
	if(0!=fclose(g_auipf))
	{
		return AUI_RTN_EINVAL;
	}
	return AUI_RTN_SUCCESS;
}
static unsigned char s_uc_pe_flag=0;
unsigned long test_tsg_play_back_by_tsg(unsigned long *argc,char **argv,char *sz_out_put)
{
	char ac_tmp[64]={0};
	st_nocs_cmd st_nocs_cmd_str;
	aui_cfg_tsg cfg_tsg;
	unsigned long ul_vid_pid=0;
	unsigned long ul_aud_pid=0;
	unsigned long ul_pcr_pid=0;
	unsigned long ul_vid_type=0;
	unsigned long ul_aud_type=0;
	
	MEMSET(&cfg_tsg,0,sizeof(aui_cfg_tsg));
	MEMSET(&st_nocs_cmd_str,0,sizeof(st_nocs_cmd));

	AUI_TEST_CHECK_NULL(g_p_hdl_tsg);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);

	AUI_TEST_CHECK_NOT_EQU_VAL(aui_send_data2dmx(NULL,NULL),AUI_RTN_SUCCESS);

	sscanf(argv[0],"%s",ac_tmp);
	if(AUI_RTN_SUCCESS != str2ulong(ac_tmp,strlen(ac_tmp),&ul_vid_pid))
	{
		return AUI_RTN_EINVAL;
	}
	sscanf(argv[1],"%s",ac_tmp);
	if(AUI_RTN_SUCCESS != str2ulong(ac_tmp,strlen(ac_tmp),&ul_aud_pid))
	{
		return AUI_RTN_EINVAL;
	}
	sscanf(argv[2],"%s",ac_tmp);
	if(AUI_RTN_SUCCESS != str2ulong(ac_tmp,strlen(ac_tmp),&ul_pcr_pid))
	{
		return AUI_RTN_EINVAL;
	}
	sscanf(argv[3],"%s",ac_tmp);
	if(AUI_RTN_SUCCESS != str2ulong(ac_tmp,strlen(ac_tmp),&ul_vid_type))
	{
		return AUI_RTN_EINVAL;
	}
	sscanf(argv[4],"%s",ac_tmp);
	if(AUI_RTN_SUCCESS != str2ulong(ac_tmp,strlen(ac_tmp),&ul_aud_type))
	{
		return AUI_RTN_EINVAL;
	}	

	cfg_tsg.uc_crypt_type=AUI_TSG_DATA_CRYPT_NONE;
	cfg_tsg.uc_data_type=AUI_TSG_DATA_TYPE_FILE;
	//cfg_tsg.ul_buf_blk_cnt=1024;
	cfg_tsg.ul_pad_pkg_cnt=16;
	cfg_tsg.ul_pkg_cnt_once_send=256;
	cfg_tsg.ul_tsi_idx=TSI_TS_A;
	cfg_tsg.us_dmx_idx=TSI_DMX_0;
	cfg_tsg.ucCIBypass=TRUE;
	cfg_tsg.uc_pe_cache_enable=FALSE;
	cfg_tsg.ul_pe_cache_size=1024*128;
	cfg_tsg.ul_pe_block_size=1024*47;
	if(5==*argc)
	{
		strcpy(cfg_tsg.ac_file_name,"/mnt/uda1/2ch_datarate_256_2_dd_h264.trp");//"/mnt/uda1/2ch_datarate_256_2_dd_h264.trp");
	}
	else
	{
		strcpy(cfg_tsg.ac_file_name,argv[5]);//"/mnt/uda1/2ch_datarate_256_2_dd_h264.trp");
	}
	aui_tsg_send_data_task_start();
	AUI_TEST_CHECK_RTN(aui_tsg_send_data_task_init(g_p_hdl_tsg,&cfg_tsg));

	if(TRUE==Star_PL_AV_Start(ul_vid_pid,ul_aud_pid,ul_pcr_pid,ul_vid_type,ul_aud_type))
	{
		return NOCS_RTN_CODE_ERR;
	}

	AUI_PRINTF("\r\n_do you want to quit this test item?press 'y' to quit\r\n");

	while(1)
	{
		if(0==aui_tsg_send_data_task_status_get())
		{
				aui_tsg_send_data_task_stop();
				osal_task_sleep(3000);
				(void)Star_PL_AV_Stop();
				return AUI_RTN_SUCCESS;
		}
		AUI_TEST_CHECK_NULL(get_ask_cmd_buf_addr());

		if(0==nocs_get_user_cmd_string_ex(get_ask_cmd_buf_addr(),st_nocs_cmd_str.ac_cmdID,&(st_nocs_cmd_str.ui_cmd_param_cnt),st_nocs_cmd_str.argv))
		{
			if((0==drv_compare_ask_cmd("y"))||(0==drv_compare_ask_cmd("Y")))
			{
				aui_tsg_send_data_task_stop();
				(void)Star_PL_AV_Stop();
				return AUI_RTN_SUCCESS;
			}
			else
			{
				rst_ask_cmd_status();
			}
		}
		
		osal_task_sleep(100);
	}

	return AUI_RTN_SUCCESS;	
}
