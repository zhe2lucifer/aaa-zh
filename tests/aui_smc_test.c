#include <aui_smc.h>
#include <stdio.h>
#include <unistd.h>

static void fun_cb(int x)
{
	if (x == 1)
		printf("smart card detected\n");
	else
		printf("missing smart card\n");
}

#define MAX_ATR_LN (33)
#define MAX_RESP_LEN (100)
#define ID (0)
int main(void)
{
	int status;
	int scratch;
	void *smc_handle;
	aui_smc_attr smc_attr;
	unsigned char atr[MAX_ATR_LN];
	unsigned short atr_len = MAX_ATR_LN;
	unsigned char expected_atr[] =	{
		0x3b, 0xdb, 0x13, 0xff, 0xc0, 0x80, 0x31, 0x80, 0x75, 0x5a,
		0x43, 0x35, 0x2e, 0x35, 0x20, 0x52, 0x45, 0x56, 0x20, 0x4d,
		0x88};
	unsigned char case_1[] = {0xc0, 0x00, 0x00, 0x00};
	unsigned char case_2s1[] = {0xc0, 0x00, 0x00, 0x00, 0x0c};
	unsigned char case_2s3[] = {0xc0, 0x00, 0x00, 0x00, 0x08};
	unsigned char case_3[] = {0xc0, 0x04, 0xff, 0xff, 0x02, 0x00, 0x01};
	unsigned char case_4s2[] = {0xc0, 0x14, 0x01, 0x00, 0x02, 0x80, 0x81, 0x02};
	unsigned char resp[MAX_RESP_LEN];
	int resp_len = MAX_RESP_LEN;
	aui_smc_param_t smc_param;
	int i;

	smc_attr.p_fn_smc_cb = fun_cb;
	smc_attr.ul_smc_id = ID;

	status = aui_smc_init(NULL);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_open(&smc_attr, &smc_handle);

	if (status != AUI_RTN_SUCCESS)
		return -1;


	status = aui_smc_active(smc_handle);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_reset(smc_handle, atr, &atr_len, 0);

	for (i = 0; i < atr_len; i++) {
		/* printf("%02x ", atr[i]); */
		if (atr[i] != expected_atr[i])
			return -1;
	}
	/* printf("\n"); */

	if (status != AUI_RTN_SUCCESS)
		return -1;
/*
	status = aui_smc_transfer(smc_handle,
			case_1, sizeof(case_1) / sizeof(unsigned char),
			resp, &resp_len);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;
*/
	status = aui_smc_transfer(smc_handle,
			case_2s1, sizeof(case_2s1) / sizeof(unsigned char),
			resp, &resp_len);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_transfer(smc_handle,
			case_2s3, sizeof(case_2s3) / sizeof(unsigned char),
			resp, &resp_len);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_transfer(smc_handle,
			case_3, sizeof(case_3) / sizeof(unsigned char),
			resp, &resp_len);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_transfer(smc_handle,
			case_4s2, sizeof(case_4s2) / sizeof(unsigned char),
			resp, &resp_len);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_send(smc_handle,
			case_3, sizeof(case_3) / sizeof(unsigned char),
			&scratch,
			resp,
			0);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_receive(smc_handle,
			case_2s1, resp, resp_len,
			&resp_len,
			NULL,
			scratch);

	printf("resp_len=%i\n", resp_len);
	for (i = 0; i < resp_len; i++)
		printf("%02x\n", resp[i]);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	/*
	sleep(10);
	*/
	status = aui_smc_isexist(smc_handle);

	if (status == 0)
		return -1;

	/*
	sleep(10);
	*/
	status = aui_smc_detect(smc_handle);
	printf("detect status = %i\n", status);

	if (status != AUISMC_ERROR_READY)
		return -1;

	status = aui_smc_param_get(smc_handle, &smc_param);
	printf("protocol = %i\n", smc_param.m_e_protocol);

	if (status != 0)
		return -1;

	smc_param.m_nETU = 2/*372/4*/;
	smc_param.m_n_baud_rate = -1;
	smc_param.m_n_frequency = -1;
	smc_param.m_e_standard = -1;
	smc_param.m_e_protocol = -1;
	smc_param.m_e_stop_bit = -1;
	smc_param.m_e_check_bit = -1;

	status = aui_smc_param_set(smc_handle, &smc_param);
	printf("set etu return status %i\n", status);
	if (status != 0)
		return -1;

	status = aui_smc_close(smc_handle);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	status = aui_smc_de_init(NULL);

	if (status != AUI_RTN_SUCCESS)
		return -1;

	return 0;
}
