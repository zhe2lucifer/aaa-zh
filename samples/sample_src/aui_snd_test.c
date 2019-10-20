/****************************INCLUDE HEAD FILE************************************/
#include "aui_snd_test.h"
#include <aui_common.h>

#include <aui_dmx.h>
#include <aui_decv.h>
#include <aui_deca.h>

#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_help_print.h"

#include <string.h>
#include <stdio.h>


/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/

/****************************LOCAL VAR********************************************/

/****************************LOCAL FUNC DECLEAR***********************************/

/****************************EXTERN VAR*******************************************/
//extern aui_hdl g_p_hdl_snd;
aui_hdl g_p_hdl_snd;

/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/

unsigned long test_snd_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nSND Test Help");

    aui_print_help_instruction("\n\n -------------------------------NOTICE-------------------------------\n");
    aui_print_help_instruction("It is suggested to use play command of stream menu to play a channel first\n");
    aui_print_help_instruction("-------------------------------NOTICE-------------------------------\n\n");
    
	/* SND_1_HELP */
	#define 	SND_1_HELP		"Before doing the other SND test items, the step of init the SND module should be executed first.\n"
	aui_print_help_command("\'init\'");
	aui_print_help_instruction_newline("Init the SND module.\n");
	aui_print_help_instruction_newline(SND_1_HELP);


	/* SND_2_HELP */
	#define 	SND_2_HELP		"Stop the SND function running, not only mute.\n"
	aui_print_help_command("\'pause\'");
	aui_print_help_instruction_newline("Pause the SND function.\n");
	aui_print_help_instruction_newline(SND_2_HELP);


	/* SND_3_HELP */
	#define 	SND_3_HELP		"Resume the SND function from the pause state of SND.\n"
	aui_print_help_command("\'resume\'");
	aui_print_help_instruction_newline("Resume the SND function.\n");
	aui_print_help_instruction_newline(SND_3_HELP);


	/* SND_4_HELP */
	#define 	SND_4_HELP_PART1		"Format:	   synclvset [sync level], Only for TDS."
	#define 	SND_4_HELP_PART2		"			   [sync level]: 0 ~ 8.\n"
	#define 	SND_4_HELP_PART3		"Example:	   If the sync level is set to \'3\', the input is"
	#define 	SND_4_HELP_PART4		"			   synclvset 3\n"

	aui_print_help_command("\'synclvset\'");
	aui_print_help_instruction_newline("SND sync level set.\n");
	aui_print_help_instruction_newline(SND_4_HELP_PART1);
	aui_print_help_instruction_newline(SND_4_HELP_PART2);
	aui_print_help_instruction_newline(SND_4_HELP_PART3);
	aui_print_help_instruction_newline(SND_4_HELP_PART4);

	/* SND_5_HELP */
	#define 	SND_5_HELP		"Get the SND sync level, Only for TDS.\n"
	aui_print_help_command("\'synclvget\'");
	aui_print_help_instruction_newline(SND_5_HELP);

	/* SND_6_HELP */
	#define 	SND_6_HELP_PART1		"Format:	   volset [volume value]."
	#define 	SND_6_HELP_PART2		"			   [volume value]: 0 ~ 100.\n"
	#define 	SND_6_HELP_PART3		"Example:	   If the volumel is set to \'62\', the input is"
	#define 	SND_6_HELP_PART4		"			   volset 62\n"

	aui_print_help_command("\'volset\'");
	aui_print_help_instruction_newline("Volume set.\n");
	aui_print_help_instruction_newline(SND_6_HELP_PART1);
	aui_print_help_instruction_newline(SND_6_HELP_PART2);
	aui_print_help_instruction_newline(SND_6_HELP_PART3);
	aui_print_help_instruction_newline(SND_6_HELP_PART4);

	/* SND_7_HELP */
	#define 	SND_7_HELP		"Get the SND volume value.\n"
	aui_print_help_command("\'volget\'");
	aui_print_help_instruction_newline(SND_7_HELP);

	/* SND_8_HELP */
	#define 	SND_8_HELP_PART1		"Format:	   muteset [enable]."
	#define 	SND_8_HELP_PART2		"			   [enable]: 0:unmute 1:mute.\n"
	#define 	SND_8_HELP_PART3		"Example:	   If the mute mode is set, the input is"
	#define 	SND_8_HELP_PART4		"			   muteset 1\n"

	aui_print_help_command("\'muteset\'");
	aui_print_help_instruction_newline("Mute set.\n");
	aui_print_help_instruction_newline(SND_8_HELP_PART1);
	aui_print_help_instruction_newline(SND_8_HELP_PART2);
	aui_print_help_instruction_newline(SND_8_HELP_PART3);
	aui_print_help_instruction_newline(SND_8_HELP_PART4);

	/* SND_9_HELP */
	#define 	SND_9_HELP		"Mute state: 0:unmute 1:mute.\n"
	aui_print_help_command("\'muteget\'");
	aui_print_help_instruction_newline("Get the state of mute.\n");
	aui_print_help_instruction_newline(SND_9_HELP);

	/* SND_10_HELP */
	#define 	SND_10_HELP_PART1		"Format:	   outtypeset [output type],[state]."
	#define 	SND_10_HELP_PART2		"			   [output type]: 0:I2SO, 1:SPDIF, 2:HDMI.\n"
	#define 	SND_10_HELP_PART3		"			   [state]: 1:enable, 0:disable.\n"
	#define 	SND_10_HELP_PART4		"Example:	   If the sound output type is set to SPDIF and it is enabled, the input is"
	#define 	SND_10_HELP_PART5		"			   outtypeset 1,1\n"

	aui_print_help_command("\'outtypeset\'");
	aui_print_help_instruction_newline("Set the sound output type.\n");
	aui_print_help_instruction_newline(SND_10_HELP_PART1);
	aui_print_help_instruction_newline(SND_10_HELP_PART2);
	aui_print_help_instruction_newline(SND_10_HELP_PART3);
	aui_print_help_instruction_newline(SND_10_HELP_PART4);
	aui_print_help_instruction_newline(SND_10_HELP_PART5);

	/* SND_11_HELP */
	#define 	SND_11_HELP_PART1		"Format:	   outtypeget [output type]."
	#define 	SND_11_HELP_PART2		"			   [output type]: 0:I2SO, 1:SPDIF, 2:HDMI.\n"
	#define 	SND_11_HELP_PART3		"Example:	   If the I2SO sound output state is got, the input is"
	#define 	SND_11_HELP_PART4		"			   outtypeget 0\n"
	#define 	SND_11_HELP_PART5		"State: 0:diable 1:enable \n"
	aui_print_help_command("\'outtypeget\'");
	aui_print_help_instruction_newline("Get the state of some type of the sound output.\n");
	aui_print_help_instruction_newline(SND_11_HELP_PART1);
	aui_print_help_instruction_newline(SND_11_HELP_PART2);
	aui_print_help_instruction_newline(SND_11_HELP_PART3);
	aui_print_help_instruction_newline(SND_11_HELP_PART4);
	aui_print_help_instruction_newline(SND_11_HELP_PART5);

	/* SND_12_HELP */
	#define 	SND_12_HELP_PART1		"Format:	   outdataset [output mode],[out type]."
	#define 	SND_12_HELP_PART2		"			   [output mode]: 0:PCM, 1:RAW, 2:force dd."
	#define 	SND_12_HELP_PART3		"			   [out type]: 0:I2SO, 1:SPDIF, 2:HDMI.\n"
	#define 	SND_12_HELP_PART4		"Example:	   If the sound data output type is I2SO - RAM, the input is"
	#define 	SND_12_HELP_PART5		"			   outdataset 1,0\n"

	aui_print_help_command("\'outdataset\'");
	aui_print_help_instruction_newline("Set the sound data's output type.\n");
	aui_print_help_instruction_newline(SND_12_HELP_PART1);
	aui_print_help_instruction_newline(SND_12_HELP_PART2);
	aui_print_help_instruction_newline(SND_12_HELP_PART3);
	aui_print_help_instruction_newline(SND_12_HELP_PART4);
	aui_print_help_instruction_newline(SND_12_HELP_PART5);

	/* SND_13_HELP */
	#define 	SND_13_HELP_PART1		"Format:	   outdataget [output type]."
	#define 	SND_13_HELP_PART2		"			   [output type]: 0:I2SO, 1:SPDIF, 2:HDMI.\n"
	#define 	SND_13_HELP_PART3		"Example:	   If the SPDIF sound output mode is got, the input is"
	#define 	SND_13_HELP_PART4		"			   outdataget 1\n"
	#define 	SND_13_HELP_PART5		"Output mode: 0:PCM, 1:RAW, 2:force dd \n"

	aui_print_help_command("\'outtypeget\'");
	aui_print_help_instruction_newline("Get the data mode of the sound output.\n");
	aui_print_help_instruction_newline(SND_13_HELP_PART1);
	aui_print_help_instruction_newline(SND_13_HELP_PART2);
	aui_print_help_instruction_newline(SND_13_HELP_PART3);
	aui_print_help_instruction_newline(SND_13_HELP_PART4);
	aui_print_help_instruction_newline(SND_13_HELP_PART5);

	/* SND_14_HELP */
	#define 	SND_14_HELP_PART1		"Format:	   spdifdelayset [spdif delay time]."
	#define 	SND_14_HELP_PART2		"			   [delay value]: 0 ~ 250.\n"
	#define 	SND_14_HELP_PART3		"Example:	   If the volumel is set to \'60ms\', the input is"
	#define 	SND_14_HELP_PART4		"			   spdifdelayset 60\n"

	aui_print_help_command("\'spdifdelayset\'");
	aui_print_help_instruction_newline("Spdif delay time set.\n");
	aui_print_help_instruction_newline(SND_14_HELP_PART1);
	aui_print_help_instruction_newline(SND_14_HELP_PART2);
	aui_print_help_instruction_newline(SND_14_HELP_PART3);
	aui_print_help_instruction_newline(SND_14_HELP_PART4);

    #ifdef AUI_LINUX
	/* SND_15_HELP */
	#define 	SND_15_HELP		"Get the SND SPDIF delay time value.\n"
	aui_print_help_command("\'spdifdelayget\'");
	aui_print_help_instruction_newline(SND_15_HELP);
	#endif
    
	/* SND_16_HELP */
	#define 	SND_16_HELP		"Register SND callback function\n"
	aui_print_help_command("\'regcb\'");
	aui_print_help_instruction_newline(SND_16_HELP);

	#define 	SND_17_HELP		        "Set channel type of sound\n"
	#define 	SND_17_HELP_PART1		"Format: channelset [chanel_type]"
	#define 	SND_17_HELP_PART2		"[channel type]: stereo, left, right, mono\n"
	#define 	SND_17_HELP_PART3		"Example: Set sound channel to mono"
	#define 	SND_17_HELP_PART4		"channelset mono\n"
	aui_print_help_command("\'channelset\'");
	aui_print_help_instruction_newline(SND_17_HELP);
	aui_print_help_instruction_newline(SND_17_HELP_PART1);
	aui_print_help_instruction_newline(SND_17_HELP_PART2);
	aui_print_help_instruction_newline(SND_17_HELP_PART3);
	aui_print_help_instruction_newline(SND_17_HELP_PART4);

	#define 	SND_18_HELP		"Get channel type of sound\n"
	aui_print_help_command("\'channelget\'");
	aui_print_help_instruction_newline(SND_18_HELP);

	return AUI_RTN_HELP;
}


extern void aui_load_tu_snd()
{
	aui_tu_reg_group("snd", "snd Test");
	{
    	aui_tu_reg_item(2, "init", AUI_CMD_TYPE_UNIT, test_snd_init, "init snd");
   		//aui_tu_reg_item(2, "start", AUI_CMD_TYPE_UNIT, test_snd_start, "start snd");
    	//aui_tu_reg_item(2, "stop", AUI_CMD_TYPE_UNIT, test_snd_stop, "stop snd");
   		aui_tu_reg_item(2, "pause", AUI_CMD_TYPE_UNIT, test_snd_pause, "pause snd");
    	aui_tu_reg_item(2, "resume", AUI_CMD_TYPE_UNIT, test_snd_resume, "resume snd");
   		aui_tu_reg_item(2, "synclvset", AUI_CMD_TYPE_UNIT, test_snd_sync_level_set, "set sync level");
    	aui_tu_reg_item(2, "synclvget", AUI_CMD_TYPE_UNIT, test_snd_sync_level_get, "get sync level");
   		aui_tu_reg_item(2, "volset", AUI_CMD_TYPE_UNIT, test_snd_vol_set, "test_snd_vol_set");
    	aui_tu_reg_item(2, "volget", AUI_CMD_TYPE_UNIT, test_snd_vol_get, "test_snd_vol_get");


		aui_tu_reg_item(2, "muteset", AUI_CMD_TYPE_UNIT, test_snd_mute_set, "test_snd_mute_set,0:unmute 1:mute");
    	aui_tu_reg_item(2, "muteget", AUI_CMD_TYPE_UNIT, test_snd_mute_get, "test_snd_mute_get");
   		aui_tu_reg_item(2, "outtypeset", AUI_CMD_TYPE_UNIT, test_snd_out_interface_type_set, "test_snd_out_interface_type_set");
    	aui_tu_reg_item(2, "outtypeget", AUI_CMD_TYPE_UNIT, test_snd_out_interface_type_get, "test_snd_out_interface_type_get");
   		aui_tu_reg_item(2, "outdataset", AUI_CMD_TYPE_UNIT, test_snd_out_data_set, "test_snd_out_data_set 0:pcm 1:raw 2:forcedd");
    	aui_tu_reg_item(2, "outdataget", AUI_CMD_TYPE_UNIT, test_snd_out_data_get, "test_snd_out_data_get");
		aui_tu_reg_item(2, "regcb", AUI_CMD_TYPE_UNIT, test_snd_reg_cb, "test reg snd callback");
			
   		aui_tu_reg_item(2, "spdifdelayset", AUI_CMD_TYPE_UNIT, test_snd_spdif_delay_set, "test_snd_spdif_delay_set");
        #ifdef AUI_LINUX
    	aui_tu_reg_item(2, "spdifdelayget", AUI_CMD_TYPE_UNIT, test_snd_spdif_delay_get, "test_snd_spdif_delay_get");
        #endif
        aui_tu_reg_item(2, "channelset", AUI_CMD_TYPE_UNIT, test_snd_channel_set, "set sound channel");
        aui_tu_reg_item(2, "channelget", AUI_CMD_TYPE_UNIT, test_snd_channel_get, "get sound channel");
		aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_snd_help, "snd test help");
    }
}

unsigned long test_snd_init(unsigned long *argc,char **argv,char *sz_out_put)
{
    if (aui_find_dev_by_idx(AUI_MODULE_SND,0,&g_p_hdl_snd))
    {
        aui_attr_snd attr_snd;
    	MEMSET(&attr_snd,0,sizeof(aui_attr_snd));
        AUI_TEST_CHECK_RTN(aui_snd_open(&attr_snd,&g_p_hdl_snd));
    }
    AUI_PRINTF("snd device:%x\n",g_p_hdl_snd);
    return AUI_RTN_SUCCESS;
}

unsigned long test_snd_start(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_snd attr_snd;
    memset(&attr_snd, 0, sizeof(aui_attr_snd));

	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  start up the audio decoder */
	AUI_TEST_CHECK_RTN(aui_snd_start(g_p_hdl_snd,&attr_snd));

	if(!aui_test_user_confirm("do you have success start play the sound?"))
	{
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_stop(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_snd attr_snd;
    memset(&attr_snd, 0, sizeof(aui_attr_snd));

	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  stop the audio decoder */
	AUI_TEST_CHECK_RTN(aui_snd_stop(g_p_hdl_snd,&attr_snd));

	if(!aui_test_user_confirm("do you have hear a break voice when it's playing?"))
	{
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}


unsigned long test_snd_pause(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_snd attr_snd;
    memset(&attr_snd, 0, sizeof(aui_attr_snd));

	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  pause the audio decoder */
	AUI_TEST_CHECK_RTN(aui_snd_pause(g_p_hdl_snd,&attr_snd));

	if(!aui_test_user_confirm("do you have success pause play the sound?"))
	{
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_resume(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_attr_snd attr_snd;
    memset(&attr_snd, 0, sizeof(aui_attr_snd));

	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  resume play the audio decoder */
	AUI_TEST_CHECK_RTN(aui_snd_resume(g_p_hdl_snd,&attr_snd));

	if(!aui_test_user_confirm("do you have success resume play the sound?"))
	{
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_sync_level_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	char ac_tmp[32]={0};
	unsigned long ul_sync_level=0xffffffff;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	if(*argc < 1){
	    AUI_PRINTF("usage: [test item] [sync level]\n");
	    AUI_PRINTF("       sync level: 0-8\n");
	    return AUI_RTN_FAIL;
	}

	AUI_TEST_CHECK_VAL(1,sscanf(argv[0],"%s",ac_tmp));
	if(!((ac_tmp[0]>='0')&&(ac_tmp[0]<'9')))
	{
		return AUI_RTN_EINVAL;
	}
	ul_sync_level=ac_tmp[0]-'0';

	AUI_TEST_CHECK_RTN(aui_snd_sync_level_set(g_p_hdl_snd,ul_sync_level));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio decoder sync level [%ld] successed.",ul_sync_level);
	return AUI_RTN_SUCCESS;
}


unsigned long test_snd_sync_level_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_snd_sync_level;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  get the decoder run status */
	AUI_TEST_CHECK_RTN(aui_snd_sync_level_get(g_p_hdl_snd,&ul_snd_sync_level));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"get audio decoder sync level is [%ld].",ul_snd_sync_level);
	return AUI_RTN_SUCCESS;
}


static unsigned char chartohex(unsigned char ch)
{
	unsigned char ret =  - 1;
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

AUI_RTN_CODE aui_common_strtoULong(unsigned char *psz_in, unsigned long ul_str_len,unsigned long *pul_out)
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
		i_tmp = chartohex(*psz_in++);
		if (i_tmp ==  - 1)
		{
			return AUI_RTN_FAIL;
		}

		*pul_out = ((*pul_out) << 4) | i_tmp;
	}

	return ul_rtn;
}

unsigned long test_snd_vol_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long ul_aud_vol=0;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	if(*argc < 1){
	    AUI_PRINTF("usage: [test item] [vol]\n");
	    AUI_PRINTF("       vol: volume(e.g. 30)\n");
	    return AUI_RTN_FAIL;
	}

    //if(AUI_RTN_SUCCESS!=aui_common_strtoULong(argv[0],strlen(ac_tmp),&ul_aud_vol))
    //{
        //AUI_PRINTF("invalid parameter!\n");
        //return AUI_RTN_FAIL;
	//}
    AUI_PRINTF("vol:%s\n",argv[0]);
    ul_aud_vol = atoi(argv[0]);

	AUI_TEST_CHECK_RTN(aui_snd_vol_set(g_p_hdl_snd,ul_aud_vol));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio volume [%d] successed.",(unsigned char)ul_aud_vol);
	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_vol_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned char uc_aud_vol;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  get the decoder run status */
	AUI_TEST_CHECK_RTN(aui_snd_vol_get(g_p_hdl_snd,&uc_aud_vol));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"get audio volume is [%d].",uc_aud_vol);
	return AUI_RTN_SUCCESS;
}


unsigned long test_snd_spdif_delay_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned long spdif_delay_time=0;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	if(*argc < 1){
	    AUI_PRINTF("usage: [test item] [delay]\n");
	    AUI_PRINTF("       delay: time(e.g. 30ms)\n");
	    return AUI_RTN_FAIL;
	}

    AUI_PRINTF("spdif delay:%s\n",argv[0]);
    spdif_delay_time = atoi(argv[0]);
	AUI_TEST_CHECK_RTN(aui_snd_set(g_p_hdl_snd,AUI_SND_SPDIF_DELAY_TIME, &spdif_delay_time));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set snd spdif delay [%ld]ms successed.",spdif_delay_time);
	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_spdif_delay_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned char spdif_delay_time=0;

	AUI_TEST_CHECK_NULL(sz_out_put);
	
	AUI_TEST_CHECK_RTN(aui_snd_get(g_p_hdl_snd,AUI_SND_SPDIF_DELAY_TIME_GET, &spdif_delay_time));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"get snd spdif delay is [%d]ms.",spdif_delay_time);
	return AUI_RTN_SUCCESS;
}


unsigned long test_snd_mute_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned char uc_mute_enable=0xff;
	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);

	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
    if(*argc < 1){
	    AUI_PRINTF("usage: [test item] [mute]\n");
	    AUI_PRINTF("       mute: 0:unmute 1:mute\n");
	    return AUI_RTN_FAIL;
	}
    uc_mute_enable = atoi(argv[0]);

	AUI_TEST_CHECK_RTN(aui_snd_mute_set(g_p_hdl_snd,uc_mute_enable));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio sound [%s] successed.",(uc_mute_enable>0)?"mute":"unmute");
	return AUI_RTN_SUCCESS;
}


unsigned long test_snd_mute_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned char uc_mute_enable = 0;
    //AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  get the decoder run status */
	AUI_TEST_CHECK_RTN(aui_snd_mute_get(g_p_hdl_snd,&uc_mute_enable));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"get audio sound is [%s].",(uc_mute_enable>0)?"mute":"unmute");
	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_channel_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	char ac_tmp[32]={0};
	aui_snd_channel_mode en_channel=AUI_SND_CHANNEL_MODE_LAST;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);
	//AUI_TEST_CHECK_VAL(*argc,1);
	if (*argc != 1) {
		AUI_PRINTF("usage: [test item] [channel type]\n");
		AUI_PRINTF("[channel type]: stereo, left, right, mono \n");
		return AUI_RTN_FAIL;
	}
	AUI_TEST_CHECK_NOT_EQU_VAL(0,sscanf(argv[0],"%s",ac_tmp));

	/**  get the audio decoder type input*/
	if(0==strcmp(ac_tmp,"stereo"))
		en_channel=AUI_SND_CHANNEL_MODE_SND_DUP_NONE;
	else if(0==strcmp(ac_tmp,"left"))
		en_channel=AUI_SND_CHANNEL_MODE_SND_DUP_L;
	else if(0==strcmp(ac_tmp,"right"))
		en_channel=AUI_SND_CHANNEL_MODE_SND_DUP_R;
	else if(0==strcmp(ac_tmp,"mono"))
		en_channel=AUI_SND_CHANNEL_MODE_SND_DUP_MONO;
	else
	{
		AUI_PRINTF("usage: [test item] [channel type]\n");
		AUI_PRINTF("[channel type]: stereo, left, right, mono \n");
		return AUI_RTN_FAIL;
	}

	AUI_TEST_CHECK_RTN(aui_snd_channel_set(g_p_hdl_snd,en_channel));
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio sound channel type [%s] successed.",ac_tmp);
	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_channel_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	enum aui_snd_channel_mode ul_snd_type;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);

	/**  get the audio decoder type */
	AUI_TEST_CHECK_RTN(aui_snd_channel_get(g_p_hdl_snd,&ul_snd_type));
	if(AUI_SND_CHANNEL_MODE_SND_DUP_NONE==ul_snd_type)
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"stereo");
	else if(AUI_SND_CHANNEL_MODE_SND_DUP_L==ul_snd_type)
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"left");
	else if(AUI_SND_CHANNEL_MODE_SND_DUP_R==ul_snd_type)
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"right");
	else if(AUI_SND_CHANNEL_MODE_SND_DUP_MONO==ul_snd_type)
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"mono");
	else {
		snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"unknown channel");
		return AUI_RTN_FAIL;
	}

	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_out_interface_type_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_snd_out_type_status snd_out_type_status;

    #if 1
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);

	if(*argc<2)
	{
			AUI_PRINTF("usage: [test item] [out type],[status]\n");
			AUI_PRINTF("[output type]: 0:I2SO, 1:SPDIF, 2:HDMI.\n");
			AUI_PRINTF("[status]: 1:enable, 0:disable.\n");
			return	AUI_RTN_FAIL;
	}

	if(!strcmp("0",argv[0]))
    {
		snd_out_type_status.snd_out_type=AUI_SND_OUT_I2SO;
    }
    else if(!strcmp("1",argv[0]))
    {
		snd_out_type_status.snd_out_type=AUI_SND_OUT_SPDIF;
    }
    else if(!strcmp("2",argv[0]))
    {
		snd_out_type_status.snd_out_type=AUI_SND_OUT_HDMI;
    }
    else
    {
        AUI_PRINTF("invalid parameter1:%s!\n",argv[0]);
        return AUI_RTN_FAIL;
    }

	/**  get the audio decoder type input*/
	if(!strcmp("1",argv[1]))
	{
		snd_out_type_status.uc_enabel=1;
	}
	else if(!strcmp("0",argv[1]))
	{
		snd_out_type_status.uc_enabel=0;
	}
	else
	{
		AUI_PRINTF("invalid parameter2:%s!\n",argv[0]);
		return AUI_RTN_FAIL;
	}
	
	AUI_TEST_CHECK_RTN(aui_snd_out_interface_type_set(g_p_hdl_snd,snd_out_type_status));
    #else
    snd_out_type_status.snd_out_type=AUI_SND_OUT_I2SO;
	snd_out_type_status.uc_enabel=0;

	AUI_TEST_CHECK_RTN(aui_snd_out_interface_type_set(g_p_hdl_snd,snd_out_type_status));

    snd_out_type_status.snd_out_type=AUI_SND_OUT_SPDIF;
	snd_out_type_status.uc_enabel=0;

	AUI_TEST_CHECK_RTN(aui_snd_out_interface_type_set(g_p_hdl_snd,snd_out_type_status));

    snd_out_type_status.snd_out_type=AUI_SND_OUT_HDMI;
	snd_out_type_status.uc_enabel=0;

	AUI_TEST_CHECK_RTN(aui_snd_out_interface_type_set(g_p_hdl_snd,snd_out_type_status));

#endif

	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio sound output type [%d] %d successed.",snd_out_type_status.snd_out_type, snd_out_type_status.uc_enabel);
	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_out_interface_type_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_snd_out_type_status snd_out_type_status;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);

	if(*argc < 1){
	    AUI_PRINTF("usage: [test item] [out type]\n");
	    AUI_PRINTF("       out type: 0:I2SO, 1:SPDIF, 2:HDMI\n");
	    return AUI_RTN_FAIL;
	}

    if(!strcmp("0",argv[0]))
    {
		snd_out_type_status.snd_out_type=AUI_SND_OUT_I2SO;
    }
    else if(!strcmp("1",argv[0]))
    {
		snd_out_type_status.snd_out_type=AUI_SND_OUT_SPDIF;
    }
    else if(!strcmp("2",argv[0]))
    {
		snd_out_type_status.snd_out_type=AUI_SND_OUT_HDMI;
    }
    else
    {
        AUI_PRINTF("invalid parameter1:%s!\n",argv[0]);
        return AUI_RTN_FAIL;
    }

	/**  get the audio decoder type */
	AUI_TEST_CHECK_RTN(aui_snd_out_interface_type_get(g_p_hdl_snd,&snd_out_type_status));
	AUI_PRINTF("\r\n[%d]-[%d].",snd_out_type_status.snd_out_type,snd_out_type_status.uc_enabel);

	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_out_data_set(unsigned long *argc,char **argv,char *sz_out_put)
{
	char ac_tmp[32]={0};
    char ac_tmp2[32] = {0};
	aui_snd_out_mode snd_out_mode;


	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);
	AUI_TEST_CHECK_NULL(sz_out_put);

    if(*argc < 2){
        AUI_PRINTF("usage: [test item] [output mode],[out type]\n");
        AUI_PRINTF("       output mode: 0:pcm, 1:raw, 2:force dd\n");
        AUI_PRINTF("       out type: 0:cvbs, 1:spdif, 2:hdmi\n");
    }

    if(!strcmp("0",argv[0]))
    {
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_DECODED;
        strcpy(ac_tmp,"pcm mode ");
    }
    else if(!strcmp("1",argv[0]))
    {
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_ENCODED;
        strcpy(ac_tmp,"raw mode ");
    }
    else if(!strcmp("2",argv[0]))
    {
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_FORCE_DD;
        strcpy(ac_tmp,"force dd mode ");
    }
    else
    {
        AUI_PRINTF("invalid parameter1:%s!\n",argv[0]);
        return AUI_RTN_FAIL;
    }

    if(!strcmp("0",argv[1]))
    {
    	snd_out_mode.snd_out_type=AUI_SND_OUT_I2SO;
        strcpy(ac_tmp2," to cvbs ");
    }
    else if(!strcmp("1",argv[1]))
    {
    	snd_out_mode.snd_out_type=AUI_SND_OUT_SPDIF;
        strcpy(ac_tmp2," to spdif ");
    }
    else if(!strcmp("2",argv[1]))
    {
    	snd_out_mode.snd_out_type=AUI_SND_OUT_HDMI;
        strcpy(ac_tmp2," to hdmi ");
    }
    else
    {
        AUI_PRINTF("invalid parameter2:%s!\n",argv[1]);
        return AUI_RTN_FAIL;
    }

    #if 0
	AUI_TEST_CHECK_NOT_EQU_VAL(0,sscanf(argv[0],"%s",ac_tmp));

	/**  get the audio decoder type input*/
	if(0==strcmp(ac_tmp,"I2SO"))
	{
		snd_out_mode.snd_out_type=AUI_SND_OUT_I2SO;
	}
	else if(0==strcmp(ac_tmp,"SPDIF"))
	{
		snd_out_mode.snd_out_type=AUI_SND_OUT_SPDIF;
	}
	else if(0==strcmp(ac_tmp,"HDMI"))
	{
		snd_out_mode.snd_out_type=AUI_SND_OUT_HDMI;
	}
	else
	{
		snd_out_mode.snd_out_type=0xffffffff;
	}

	AUI_TEST_CHECK_NOT_EQU_VAL(0,sscanf(argv[1],"%s",ac_tmp));

	/**  get the audio decoder type input*/
	if(0==strcmp(ac_tmp,"PCM"))
	{
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_DECODED;
	}
	else if(0==strcmp(ac_tmp,"BS"))
	{
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_ENCODED;
	}
	else if(0==strcmp(ac_tmp,"DD"))
	{
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_FORCE_DD;
	}
	else
	{
		snd_out_mode.snd_data_type=0xffffffff;
	}
    #endif
	AUI_TEST_CHECK_RTN(aui_snd_out_data_type_set(g_p_hdl_snd,snd_out_mode));

	if((snd_out_mode.snd_data_type!=AUI_SND_OUT_MODE_DECODED) &&(snd_out_mode.snd_out_type!=AUI_SND_OUT_I2SO)){
		aui_hdl hdl_deca = NULL;
		if (aui_find_dev_by_idx(AUI_MODULE_DECA,0,&hdl_deca))
	    {
	        aui_attr_deca attr_deca;
	    	MEMSET(&attr_deca,0,sizeof(aui_attr_deca));
	        AUI_TEST_CHECK_RTN(aui_deca_open(&attr_deca,&hdl_deca));
	    }
		enum aui_deca_stream_type type = AUI_DECA_STREAM_TYPE_AC3;
	    AUI_TEST_CHECK_RTN(aui_deca_set(hdl_deca, AUI_DECA_ADD_BS_SET, &type));
	    type = AUI_DECA_STREAM_TYPE_EC3;
	    AUI_TEST_CHECK_RTN(aui_deca_set(hdl_deca, AUI_DECA_ADD_BS_SET, &type));
		AUI_PRINTF("set ac3&ec3 bypass successfully\n");
	}
	snprintf(sz_out_put,UART_CMD_OUTPUT_LEN_MAX,"set audio sound output mode [%s %s] successed.",ac_tmp,ac_tmp2);
	return AUI_RTN_SUCCESS;
}

unsigned long test_snd_out_data_get(unsigned long *argc,char **argv,char *sz_out_put)
{
	char ac_tmp[32]={0};
	aui_snd_out_mode snd_out_mode;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);
	if(*argc < 1){
	    AUI_PRINTF("usage: [test item] [out type]\n");
	    AUI_PRINTF("       out type: 0:I2SO, 1:SPDIF, 2:HDMI\n");
	    return AUI_RTN_FAIL;
	}
    if(!strcmp("0",argv[0]))
    {
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_DECODED;
        strcpy(ac_tmp,"I2SO");
    }
    else if(!strcmp("1",argv[0]))
    {
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_ENCODED;
        strcpy(ac_tmp,"SPDIF");
    }
    else if(!strcmp("2",argv[0]))
    {
		snd_out_mode.snd_data_type=AUI_SND_OUT_MODE_FORCE_DD;
        strcpy(ac_tmp,"HDMI");
    }
    else
    {
        AUI_PRINTF("invalid parameter1:%s!\n",argv[0]);
        return AUI_RTN_FAIL;
    }

	if(0==strcmp(ac_tmp,"I2SO"))
	{
		snd_out_mode.snd_out_type=AUI_SND_OUT_I2SO;
	}
	else if(0==strcmp(ac_tmp,"SPDIF"))
	{
		snd_out_mode.snd_out_type=AUI_SND_OUT_SPDIF;
	}
	else if(0==strcmp(ac_tmp,"HDMI"))
	{
		snd_out_mode.snd_out_type=AUI_SND_OUT_HDMI;
	}
	else
	{
		snd_out_mode.snd_out_type=0xffffffff;
	}

	/**  get the audio decoder type */
	AUI_TEST_CHECK_RTN(aui_snd_out_data_type_get(g_p_hdl_snd,&snd_out_mode));
	AUI_PRINTF("\r\nout type[%d]-data type[%d](0:pcm, 1:raw, 2:force dd).",snd_out_mode.snd_out_type,snd_out_mode.snd_data_type);

	return AUI_RTN_SUCCESS;
}
void snd_cb(void *pv_param)
{
    struct aui_snd_io_reg_callback_para *p=pv_param;
    AUI_PRINTF("\r\n func:[%s][%08x][%08x]",__FUNCTION__,pv_param,p->e_cbtype);
}
unsigned long test_snd_reg_cb(unsigned long *argc,char **argv,char *sz_out_put)
{
    struct aui_snd_io_reg_callback_para cb_para;

	//AUI_TEST_CHECK_NULL(g_p_hdl_snd->pst_dev_snd);
	AUI_TEST_CHECK_NULL(sz_out_put);

    MEMSET(&cb_para,0,sizeof(cb_para));
    cb_para.e_cbtype=AUI_SND_CB_MONITOR_ERRORS_OCCURED;
    cb_para.p_cb=snd_cb;
    cb_para.pv_param=(void *)1;
    AUI_PRINTF("\r\n [%08x]",&cb_para);
	AUI_TEST_CHECK_RTN(aui_snd_set(g_p_hdl_snd,AUI_SND_REG_CB,&cb_para));

    cb_para.e_cbtype=AUI_SND_CB_MONITOR_OUTPUT_DATA_END;
    cb_para.p_cb=snd_cb;
    cb_para.pv_param=(void *)2;
    AUI_PRINTF("\r\n [%08x]",&cb_para);
	AUI_TEST_CHECK_RTN(aui_snd_set(g_p_hdl_snd,AUI_SND_REG_CB,&cb_para));


	return AUI_RTN_SUCCESS;
}
