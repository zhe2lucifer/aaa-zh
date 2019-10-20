
#include "aui_ca_test.h"
#include <aui_ca_pvr.h>
#include <aui_dsc.h>
#include "aui_help_print.h"
#include <aui_trng.h>

static unsigned short aui_ca_test_fp_get_ts_dsc_handle_callback(unsigned short program_id,aui_hdl *p_dsc_handler)
{
	unsigned int ret =0 ;
	aui_hdl aui_dsc_handler;
	
	if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_DSC, 0, &aui_dsc_handler))
	{
		AUI_PRINTF("aui_find_dev_by_idx AUI_MODULE_DSC failed!\r\n");
		ret = 0;
		return ret;
	}
	*p_dsc_handler = aui_dsc_handler;
	return 0;
}

/*get PURE DATA DSC (virtual) device handle*/
static unsigned short aui_ca_fp_get_pure_data_dsc_handle_callback(unsigned short program_id,aui_hdl *p_dsc_handler)
{
	unsigned short ret =0;
	aui_hdl aui_dsc_handler;
	
	if(AUI_RTN_SUCCESS != aui_find_dev_by_idx(AUI_MODULE_DSC, 0, &aui_dsc_handler))
	{
		AUI_PRINTF("aui_find_dev_by_idx AUI_MODULE_DSC failed!\r\n");
		ret = 1;
		return ret;
	}
	*p_dsc_handler = aui_dsc_handler;
	return ret;
}

unsigned long ca_test_init(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int ret=0;
	aui_hdl aui_dsc_handler;
	aui_attr_dsc aui_dsc_attr;
	MEMSET(&aui_dsc_attr,0,sizeof(aui_attr_dsc));
	aui_dsc_attr.uc_dev_idx = 0;
	aui_dsc_attr.uc_algo = AUI_DSC_ALGO_CSA;
	aui_dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;

	if(AUI_RTN_SUCCESS != aui_ca_pvr_init())
	{
		ret = 1;
		return ret;
	}

	aui_os_task_sleep(100);
	if(AUI_RTN_SUCCESS == aui_find_dev_by_idx(AUI_MODULE_DSC, 0, &aui_dsc_handler))
	{
		AUI_PRINTF("aui_find_dev_by_idx AUI_MODULE_DSC failed!\r\n");
		ret = 0;
		return ret;
	}
	#if 0
	if(AUI_RTN_SUCCESS !=aui_dsc_init(NULL,NULL))
	{
		ret = 1;
		return ret;
	}
	#endif
	if(AUI_RTN_SUCCESS != aui_dsc_open(&aui_dsc_attr, &aui_dsc_handler))
	{
		ret = 1;
	}
	
	aui_os_task_sleep(100);
	return ret;
}

unsigned long ca_test_register(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int ret=0;
	aui_ca_pvr_callback st_aui_ca_pvr_callback;
	st_aui_ca_pvr_callback.fp_ts_callback = aui_ca_test_fp_get_ts_dsc_handle_callback;
	st_aui_ca_pvr_callback.fp_ts_callback = aui_ca_fp_get_pure_data_dsc_handle_callback;
	if(0 != aui_ca_register_callback(&st_aui_ca_pvr_callback))
	{
		ret =1;
	}
	return ret;
}

static unsigned char g_rd_key[16]={0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                                        0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x21};

static unsigned char g_rd_iv[16]={0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89,
                                        0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf1, 0x13};

unsigned char hmac_key_en[17]={0};
unsigned long ca_test_hmac_key_write(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int ret=0;
	int i=0;
	unsigned char hmac_key[17]={0};
	unsigned char text[256]={0};
	unsigned char hmac[32] ={0};
	
	aui_hdl handle = NULL;
	aui_trng_attr attr = {0};
	aui_trng_param trng_data = {0, 0};

	AUI_PRINTF("\r\n text:");
	for(i=0;i<256;i++)
	{
		if(i % 16 == 0) AUI_PRINTF("\r\n %d",i % 16);

		if (aui_trng_open(&attr, &handle)) {
			ret =1;
			return ret;
		}

		trng_data.puc_rand_output_buffer = &text[i];
		trng_data.ul_rand_bytes = 1;
		if (aui_trng_generate(handle, &trng_data)) {
			ret =1;
			return ret;
		}
			
		if (aui_trng_close(handle)) {
			ret =1;
			return ret;
		}
		
		AUI_PRINTF("%02x ",text[i]);
	}
	
	//create hmac key
	if(AUI_RTN_FAIL ==aui_ca_pvr_generate_keys(16 * 8, 1, hmac_key))
	{
		AUI_PRINTF("\r\n please make sure that chip id is not empty!\r\n");
		ret =1;
		return ret;
	}

	AUI_PRINTF("\r\n hmac_key:");
	for(i=0;i<16;i++)
	{
		AUI_PRINTF("0x%02x	",hmac_key[i]);
	}
	AUI_PRINTF("\r\n");
	if(AUI_RTN_SUCCESS != aui_ca_calculate_hmac(text,256,hmac,hmac_key))
	{
		ret =1;
		return ret;
	}
	AUI_PRINTF("\r\n HMAC result:");
	for(i=0;i<32;i++)
	{
		if(i % 16 == 0) AUI_PRINTF("\r\n");
		AUI_PRINTF("%02x ",hmac[i]);
	}
	
	if(AUI_RTN_FAIL ==aui_ca_pvr_crypto(hmac_key_en,hmac_key,16,
										g_rd_key,g_rd_iv,128,0,1))
	{
		ret =1;
		return ret;
	}

	AUI_PRINTF("\r\n hmac_key_en:");
	for(i=0;i<16;i++)
	{
		AUI_PRINTF("0x%02x	",hmac_key_en[i]);
	}
	AUI_PRINTF("\r\n");
	return ret;
}

unsigned long ca_test_hmac_key_read(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int ret=0;
	int i=0;
	unsigned char hmac_key[17]={0};
	
	if(AUI_RTN_FAIL ==aui_ca_pvr_crypto(hmac_key,hmac_key_en,16,
										g_rd_key,g_rd_iv,128,0,0))
	{
		ret =1;
		return ret;
	}

	AUI_PRINTF("\r\n hmac_key:");
	for(i=0;i<16;i++)
	{
		AUI_PRINTF("0x%02x	",hmac_key[i]);
	}
	AUI_PRINTF("\r\n");
	return ret;
}

unsigned long ca_test_deinit(unsigned long *argc,char **argv,char *sz_out_put)
{
	unsigned int ret=0;
	return ret;
}

unsigned long test_ca_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nCA Test Help");  
	

	/* CA_1_HELP */
	#define 	CA_1_HELP_PART1	"It includes below steps for the CA module init: "
	#define 	CA_1_HELP_PART2 	"Init AUI CA PVR module --> Get DSC handler --> Open DSC module\n"

	aui_print_help_command("\'init\'");
	aui_print_help_instruction_newline("Init the CA module\n");
	aui_print_help_instruction_newline(CA_1_HELP_PART1);
	aui_print_help_instruction_newline(CA_1_HELP_PART2);

	
	/* CA_2_HELP */
	#define 	CA_2_HELP 	"Register the CA call back funcition.\n"

	aui_print_help_command("\'register\'");
	aui_print_help_instruction_newline(CA_2_HELP);
	
	
	return AUI_RTN_HELP;
}

void test_ca_reg()
{
	aui_tu_reg_group("ca", "config CA");
	aui_tu_reg_item(2, "init", AUI_CMD_TYPE_API, ca_test_init, "Initialize CA");
	aui_tu_reg_item(2, "register", AUI_CMD_TYPE_API, ca_test_register, "register DSC for PVR");
	aui_tu_reg_item(2, "hmac_key_write", AUI_CMD_TYPE_API, ca_test_hmac_key_write, "register DSC for PVR");
	aui_tu_reg_item(2, "hmac_key_read", AUI_CMD_TYPE_API, ca_test_hmac_key_read, "register DSC for PVR");
	aui_tu_reg_item(2, "deinit", AUI_CMD_TYPE_API, ca_test_deinit, "Deinitialzie CA");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_ca_help, "CA test help");
}

