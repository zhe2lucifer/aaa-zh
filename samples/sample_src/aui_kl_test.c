/****************************INCLUDE HEAD FILE************************************/
#include "aui_kl_test.h"

/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************EXTERN VAR*******************************************/
extern aui_hdl g_p_hdl_kl;
extern aui_hdl g_p_hdl_dsc_aes;
extern aui_hdl g_p_hdl_dsc_des;
/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/

AUI_RTN_CODE aui_kl_gen_key_by_cfg(void *p_hdl_kl,aui_cfg_kl *p_kl_cfg,unsigned long *pul_key_pos_dst); //aui_kl_key_info *p_key_info)

unsigned long test_kl_gen_key_by_cfg(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_key_pos_out=0xffffffff;
	aui_cfg_kl cfg_kl;
	unsigned char auc_tmp[256]={0};
	aui_attr_dsc attr_dsc;
	unsigned char auc_tmp_in[256]={0};
	unsigned char auc_tmp_out[256]={0};
	int i=0;
	
	//AUI_TEST_CHECK_NULL(g_p_hdl_kl->pst_dev_kl);
	AUI_TEST_CHECK_NULL(sz_out_put);

	MEMSET(&cfg_kl,0,sizeof(aui_cfg_kl));
	MEMSET(&attr_dsc,0,sizeof(aui_attr_dsc));

	cfg_kl.ul_root_key_pos=AUI_KL_OTP_KEY_0_0;
	cfg_kl.uc_level=1;
	cfg_kl.uc_crypt_algo=AUI_KL_ALGO_DES;
	cfg_kl.uc_crypt_mode=AUI_KL_CRYPT_IS_DECRYPT;
	cfg_kl.ul_key_len=AUI_AES_CE_KEY_LEN;
	MEMCPY(cfg_kl.ac_key_val,auc_tmp,256);
	AUI_TEST_CHECK_RTN(aui_kl_gen_key_by_cfg(g_p_hdl_kl,&cfg_kl,&ul_key_pos_out));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"\r\n_out level[%d] key pos is [%d].",cfg_kl.uc_level,ul_key_pos_out);


	attr_dsc.dsc_data_type=AUI_DSC_DATA_PURE;
	attr_dsc.dsc_key_type=AUI_DSC_CONTENT_KEY_KL;
	attr_dsc.uc_algo=AUI_DSC_ALGO_TDES;
	attr_dsc.uc_mode=AUI_DSC_WORK_MODE_IS_ECB;
	attr_dsc.ul_key_len=128;
	attr_dsc.en_parity=0;
	attr_dsc.en_residue=0;
	attr_dsc.ul_key_pos=ul_key_pos_out;
	//MEMCPY(attr_dsc.puc_key)
	AUI_TEST_CHECK_RTN(aui_dsc_attach_key_info2dsc(g_p_hdl_dsc_des,&attr_dsc));
	AUI_PRINTF("\r\n_attach success.");
	AUI_TEST_CHECK_RTN(aui_dsc_encrypt(g_p_hdl_dsc_des,auc_tmp_in,auc_tmp_out,16));
	AUI_PRINTF("\r\n");
	for(i=0;i<16;i++)
	{
		AUI_PRINTF("[%02x]-",auc_tmp_out[i]);
	}
	AUI_TEST_CHECK_RTN(aui_dsc_close(g_p_hdl_dsc_des));
	return AUI_RTN_SUCCESS;	
}
#if 0

{
	char ac_tmp[32]={0};
	enum kl_dup_channel en_channel=0xffffffff;
	
	//AUI_TEST_CHECK_NULL(g_p_hdl_kl->pst_dev_kl);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_VAL(*argc,1);

	AUI_TEST_CHECK_VAL(0,sscanf(argv[0],"%s",ac_tmp));
	
	/**  get the audio decoder type input*/
	if(0==strcmp(ac_tmp,"KL_DUP_NONE"))
	{
		en_channel=KL_DUP_NONE;
	}
	else if(0==strcmp(ac_tmp,"KL_DUP_L"))
	{
		en_channel=KL_DUP_R;
	}
	else if(0==strcmp(ac_tmp,"KL_DUP_R"))
	{
		en_channel=KL_DUP_R;
	}
	else if(0==strcmp(ac_tmp,"KL_DUP_MONO"))
	{
		en_channel=KL_DUP_MONO;
	}
	else
	{
		en_channel=0xffffffff;
	}
	
	AUI_TEST_CHECK_RTN(aui_kl_channel_set(g_p_hdl_kl,en_channel));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio sound channel type [%s] successed.",ac_tmp);
	return AUI_RTN_SUCCESS;	
}
unsigned long test_kl_channel_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_kl_type;

	//AUI_TEST_CHECK_NULL(g_p_hdl_kl->pst_dev_kl);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  get the audio decoder type */
	AUI_TEST_CHECK_RTN(aui_kl_channel_get(g_p_hdl_kl,&ul_kl_type));
	if(KL_DUP_NONE==ul_kl_type)
	{
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"KL_DUP_NONE");
	}
	else if(KL_DUP_L==ul_kl_type)
	{
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"KL_DUP_L");
	}
	else if(KL_DUP_R==ul_kl_type)
	{
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"KL_DUP_R");
	}
	else if(KL_DUP_MONO==ul_kl_type)
	{
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"KL_DUP_MONO");
	}
	else
	{
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"UNKNOW_AUDIO_TYPE");
		return AUI_RTN_FAIL;
	}
	
	return AUI_RTN_SUCCESS;	
}
#endif
