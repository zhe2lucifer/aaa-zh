/**
 *    @brief     TRNG test module
 *    @author    Oscar Shi
 *    @date      2015-05-08
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2014 copyright (C)
 */
#ifdef AUI_TDS
#include <api/libc/alloc.h>
#endif
#include "aui_trng_test.h"
#include <aui_trng.h>

unsigned long test_trng(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long ret = 0;
	aui_hdl handle = NULL;
	aui_trng_attr attr = {0};
	aui_trng_param trng_data = {0, 0};
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	int i = 0;
	int j = 0;

	rtn_code = aui_trng_open(&attr, &handle);
	if (rtn_code) {
		return 1;
	}

	trng_data.puc_rand_output_buffer = malloc(200);
	if (!trng_data.puc_rand_output_buffer) {
		return 1;
	}

	for (i = 1; i < 200; i++) {
		trng_data.ul_rand_bytes = i;
		ret = aui_trng_generate(handle, &trng_data);
		rtn_code |= ret;
		if (ret) {
			AUI_PRINTF("Get TRNG fail len=%3d.\n", trng_data.ul_rand_bytes);
		}
		AUI_PRINTF("len=%3d: ", trng_data.ul_rand_bytes);
		for (j = 0; j < i; j++) {
			AUI_PRINTF("%02x", trng_data.puc_rand_output_buffer[j]);
		}
		AUI_PRINTF("\n");
	}
	rtn_code |= aui_trng_close(handle);
	free(trng_data.puc_rand_output_buffer);

	return (unsigned long)rtn_code;
}

#ifndef AUI_TDS
#include <sys/time.h>
#endif
#ifndef AUI_LINUX
#include <osal/osal_timer.h>
#endif

unsigned long test_trng_generate_speed(unsigned long *argc, char **argv, char *sz_out_put)
{
	unsigned long ret = 0;
	aui_hdl handle = NULL;
	aui_trng_attr attr = {0};
	aui_trng_param trng_data = {0, 0};
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	int i = 0;
	int j = 0;
	int times = 100;
#ifndef AUI_TDS
	struct timeval begin = {0, 0};
	struct timeval end = {0, 0};
	struct timeval time_all = {0, 0};
#endif
#ifndef AUI_LINUX
	unsigned long begin_time = 0;
	unsigned long end_time = 0;
#endif
	unsigned long len_list[10][2] = {
			{1, 0},			// 8bits
			{1 + 1, 0},		// 8+8bits
			{8, 0},			// 64bits
			{8 + 1, 0},		// 64+8bits
			{8 * 2, 0},		// 64*2bits
			{8 * 2 + 1, 0},	// 64*2+8bits
			{8 * 3, 0},		// 64*3bits
			{0, 0}
	};

	rtn_code = aui_trng_open(&attr, &handle);
	if (rtn_code) {
		return 1;
	}

	trng_data.puc_rand_output_buffer = malloc(200);
	if (!trng_data.puc_rand_output_buffer) {
		return 1;
	}

	for (j = 0; j < 10; j++) {
		trng_data.ul_rand_bytes = len_list[j][0];
		if (!trng_data.ul_rand_bytes) {
			break;
		}

#ifndef AUI_TDS
		gettimeofday(&begin, NULL);
#endif
#ifndef AUI_LINUX
		begin_time = osal_get_tick();
#endif

		for (i = 1; i < times; i++) {
			ret = aui_trng_generate(handle, &trng_data);
		}

#ifndef AUI_TDS
		gettimeofday(&end, NULL);
		timersub(&end, &begin, &time_all);
		len_list[j][1] = time_all.tv_usec;
		AUI_PRINTF("%d bits %d times takes %dus (%dus one time)\n",
				len_list[j][0] * 8, times, len_list[j][1],
				len_list[j][1] / times);
#endif
#ifndef AUI_LINUX
		end_time = osal_get_tick();
		len_list[j][1] += end_time - begin_time;
		AUI_PRINTF("%d bits %d times takes %dms (%dms one time)\n",
						len_list[j][0] * 8, times, len_list[j][1],
						len_list[j][1] / times);
#endif
		rtn_code |= ret;
		if (ret) {
			AUI_PRINTF("Get TRNG fail len=%3d.\n", trng_data.ul_rand_bytes);
		}
	}

	rtn_code |= aui_trng_close(handle);
	free(trng_data.puc_rand_output_buffer);

	return (unsigned long)rtn_code;
}

unsigned long test_trng_help(unsigned long *argc, char **argv, char *sz_out_put)
{
	(void)argc;
	(void)argv;
	(void)sz_out_put;

	char help[] =
			"\n\nMISC Test Help \n\
COMMAND \n\
        '1': \n\
                Test TRNG. \n\
        '2': \n\
                TRNG generate speed test \n\
\n\n";

	AUI_PRINTF("%s", help);
	return AUI_RTN_HELP;
}

void test_trng_reg(void)
{
	aui_tu_reg_group("trng", "TRNG tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_trng,  "TRNG test");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_trng_generate_speed,  "TRNG generate speed test");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_trng_help, "TRNG help");
}
