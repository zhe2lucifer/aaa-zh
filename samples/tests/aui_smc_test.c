#include <aui_smc.h>
#include <stdio.h>

static int pv_param;
static AUI_RTN_CODE fun_cb(void *pv_param)
{
	printf("[%s]\n", __func__);
	return 0;
}

#define MAX_ATR_LN (33)
#define MAX_RESP_LEN (100)
int main(void)
{
	int status;
	void *smc_handle;
	aui_smc_attr smc_attr;
	unsigned char atr[MAX_ATR_LN];
	unsigned short atr_len = MAX_ATR_LN;
	unsigned char command[] = {0xc0, 0x00, 0x00, 0x00, 0xc0};
	unsigned char resp[MAX_RESP_LEN];
	int resp_len = MAX_RESP_LEN;
	unsigned char transfer_alternate_status_word;

	smc_attr.p_fn_smc_cb = fun_cb;

	status = aui_smc_init(/*fun_cb*/NULL);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_open(&smc_attr, &smc_handle);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_reset(smc_handle, atr, &atr_len, 0);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_transfer(smc_handle,
			command, sizeof(command) / sizeof(unsigned char),
			resp, &resp_len, &transfer_alternate_status_word);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_close(smc_handle);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_de_init(NULL);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	return 0;
}
