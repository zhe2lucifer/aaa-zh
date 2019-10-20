#ifdef AUI_LINUX
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#endif

#include <aui_dsc.h>
#include <aui_kl.h>
#include "aui_test_app_cmd.h"
#include "aui_help_print.h"

#define DATA_LEN 16

AUI_RTN_CODE aui_kl_read_key(aui_hdl handle, unsigned char *key);

unsigned long dsc_ram2ram(unsigned long *argc,char **argv,char *sz_output)
{
	aui_attr_dsc attr;
	aui_hdl hdl;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* clear key 128 bits */
	unsigned char clear_key[16 * 2] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d,
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d};

	/* data */
	unsigned char enc_16_cbc[DATA_LEN] = {
		0xe9, 0x84, 0x7e, 0xfe, 0x26, 0x5e, 0x2e, 0x5c,
		0xf9, 0xf9, 0x81, 0x41, 0x7c, 0x26, 0x75, 0x1a };
	unsigned char dec_source[DATA_LEN] = {
		0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c, 0x9a, 0xa0,
		0x61, 0x1b, 0xbb, 0x3e, 0x20, 0x25, 0xa4, 0x5a };

	unsigned char data_out[DATA_LEN];
	unsigned char iv[16 * 2]; /* odd and even */

	attr.uc_dev_idx = 0;
	attr.uc_algo = AUI_DSC_ALGO_AES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl)) {
		AUI_PRINTF("dsc open error\n");
		return 1;
	}

	/* clear key in CBC mode */
	attr.puc_key = clear_key;
	attr.ul_key_len = 16 * 8;
	attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;

	attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
	attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used
	/* no PID */
	attr.ul_pid_cnt = 0;
	attr.pus_pids = NULL;

	/* zero IV */
	MEMSET(iv, 0, sizeof(iv));
	attr.puc_iv_ctr = iv;

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		aui_dsc_close(hdl);
		return 1;
	}

	int err = 0;
	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		aui_dsc_close(hdl);
		return 1;
	}

	if (MEMCMP(data_out, dec_source, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	int i;
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	if (aui_dsc_close(hdl)) {
		AUI_PRINTF("dsc open error\n");
		return 1;
	}

	return err;
}
unsigned long dsc_ram2ram_des(unsigned long *argc,char **argv,char *sz_output)
{
	aui_attr_dsc attr;
	aui_hdl hdl = NULL, hdl2 = NULL, hdl3 = NULL;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* data */
	unsigned char enc_16_cbc[DATA_LEN] = {
		0xe9, 0x84, 0x7e, 0xfe, 0x26, 0x5e, 0x2e, 0x5c,
		0xf9, 0xf9, 0x81, 0x41, 0x7c, 0x26, 0x75, 0x1a };

	/* clear keys odd and even */
	unsigned char clear_key192[24*2] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8e,
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8e};

	unsigned char dec_key192[DATA_LEN] = {
		0x26,0x7f,0xf8,0x8b,0xa6,0x70,0x0c,0x19,
		0x5f,0x26,0x2a,0x31,0xe1,0x48,0x3f,0x75 };

#ifdef AUI_TDS
	unsigned char clear_key128[16*2] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d,
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d};
#else
	unsigned char clear_key128[16*2] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0x89, 0xc9, 0x84, 0x6a, 0x51, 0x9e, 0x90, 0x8d };
#endif
	unsigned char dec_key128[DATA_LEN] = {
		0x66,0x13,0x67,0x54,0x81,0x3f,0x9d,0xec,
		0xa5,0xac,0xaf,0x21,0x1e,0xd2,0x5e,0x6f };

#ifdef AUI_TDS
	unsigned char clear_key64[8*2] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e,
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e};
#else
	unsigned char clear_key64[8*2] = {
		0xbc, 0x0c, 0x94, 0x1e, 0xbc, 0xd2, 0xa1, 0x3e };
#endif

	unsigned char dec_key64[DATA_LEN] = {
		0xd4,0x59,0x01,0x00,0x3f,0xeb,0x20,0x3e,
		0x3e,0xe6,0xc4,0xec,0x4c,0x05,0x33,0xb7 };

	unsigned char data_out[DATA_LEN];
	unsigned char iv[16 * 2]; /* odd and even */
	int err = 0;

	attr.uc_dev_idx = 0;
	attr.uc_algo = AUI_DSC_ALGO_TDES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl)) {
		AUI_PRINTF("dsc open error\n");
		return 1;
	}
	attr.uc_dev_idx = 1;
	attr.uc_algo = AUI_DSC_ALGO_DES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl2)) {
		AUI_PRINTF("dsc open error\n");
		goto des_close;
	}
	attr.uc_dev_idx = 2;
	attr.uc_algo = AUI_DSC_ALGO_TDES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl3)) {
		AUI_PRINTF("dsc open error\n");
		goto des_close;
	}

	/* clear TDES key 128bits in ECB mode */
	attr.puc_key = clear_key128;
	attr.ul_key_len = 16 * 8;
	attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used

	attr.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

	/* no PID */
	attr.ul_pid_cnt = 0;
	attr.pus_pids = NULL;

	/* zero IV */
	MEMSET(iv, 0, sizeof(iv));
	attr.puc_iv_ctr = iv;

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto des_close;
	}
	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto des_close;
	}
	if (MEMCMP(data_out, dec_key128, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}
	AUI_PRINTF("Data out ");
	int i;
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* DES 64 bits */
	attr.puc_key = clear_key64;
	attr.ul_key_len = 8 * 8;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used
	if (aui_dsc_attach_key_info2dsc(hdl2, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto des_close;
	}
	if (aui_dsc_decrypt(hdl2, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto des_close;
	}
	if (MEMCMP(data_out, dec_key64, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}
	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* DES 192 bits */
	attr.puc_key = clear_key192;
	attr.ul_key_len = 24 * 8;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used
	if (aui_dsc_attach_key_info2dsc(hdl3, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto des_close;
	}
	if (aui_dsc_decrypt(hdl3, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto des_close;
	}
	if (MEMCMP(data_out, dec_key192, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}
	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

des_close:
	if (aui_dsc_close(hdl)) {
		AUI_PRINTF("dsc open error\n");
		return 1;
	}
	if (aui_dsc_close(hdl2)) {
		AUI_PRINTF("dsc open error\n");
		return 1;
	}
	if (aui_dsc_close(hdl3)) {
		AUI_PRINTF("dsc open error\n");
		return 1;
	}
	return err;
}



unsigned long dsc_ram2ram_kl(unsigned long *argc,char **argv,char *sz_output)
{
	aui_attr_dsc attr;
	aui_hdl hdl;
	aui_hdl kl_hdl;
	int i;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* dsc data */
	unsigned char enc_16_cbc[DATA_LEN] = {
		0xe9, 0x84, 0x7e, 0xfe, 0x26, 0x5e, 0x2e, 0x5c,
		0xf9, 0xf9, 0x81, 0x41, 0x7c, 0x26, 0x75, 0x1a };

	//unsigned char dec_source[DATA_LEN] = {
	//	0xc2, 0x86, 0x69, 0x6d, 0x88, 0x7c, 0x9a, 0xa0,
	//	0x61, 0x1b, 0xbb, 0x3e, 0x20, 0x25, 0xa4, 0x5a };

	unsigned char data_out[DATA_LEN];
	unsigned char iv[16 * 2]; /* odd and even */

	struct aui_attr_kl kl_attr;

	/* protected keys */
	unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef }};

	/* expected result with root key are zero */
	//unsigned char result_keys[][16] = {
	//{ 0x3e,0x07,0x4d,0xee,0xcf,0xe1,0x74,0xa5,
	//  0x0e,0xde,0x9d,0xbf,0xb3,0xf9,0x65,0x14 },
	//{ 0xd0,0x29,0x84,0x10,0x0e,0xe6,0x90,0x00,
	//  0x24,0x27,0x00,0x28,0x51,0x69,0x5e,0x2c },
	//{ 0xd0,0xc4,0x54,0xbd,0xf3,0xd7,0x23,0x2f,
	//  0xc9,0xf9,0x7b,0x70,0x59,0x56,0xd3,0x84 }};

	unsigned char result_dsc[] = {
		0x01,0xc3,0x92,0x3f,0x26,0x84,0xc5,0xf7,
		0x81,0x54,0x16,0xc0,0x11,0xe5,0x34,0x79 };

	kl_attr.uc_dev_idx = 0;
	kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE;
	kl_attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	kl_attr.en_root_key_idx = 0;

	if (aui_kl_open(&kl_attr, &kl_hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_SINGLE;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(kl_hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
		goto r2r_kl_err;
	}
#if 0
	unsigned char result_key[16]; /* 128 bits */
	if (aui_kl_read_key(kl_hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");
	if (MEMCMP(result_key, result_keys[2], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	} else
		AUI_PRINTF("Key ok in pos %d\n", (int)key_dst_pos);
#endif

	attr.uc_dev_idx = 0;
	attr.uc_algo = AUI_DSC_ALGO_AES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl)) {
		AUI_PRINTF("dsc open error\n");
		err = 1;
		goto r2r_kl_err;
	}

	/* key from KL in CBC mode */
	attr.ul_key_pos = key_dst_pos;
	attr.ul_key_len = 16 * 8;
	attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used

	attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
	attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

	/* no PID */
	attr.ul_pid_cnt = 0;
	attr.pus_pids = NULL;

	/* zero IV */
	MEMSET(iv, 0, sizeof(iv));
	attr.puc_iv_ctr = iv;
	AUI_PRINTF("set IV, all zero\n");

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* Update IV */
	MEMSET(iv, 1, sizeof(iv));
	AUI_PRINTF("Update IV, change to all one\n");

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result is changed\n");
	} else
		err = 1;

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* Update IV */
	MEMSET(iv, 0, sizeof(iv));
	AUI_PRINTF("Update IV, back to all zero\n");

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

r2r_kl_err:
	if (aui_dsc_close(hdl)) {
		AUI_PRINTF("dsc open error\n");
		err = 1;
	}

	if (aui_kl_close(kl_hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}

unsigned long dsc_ram2ram_kl_tdes(unsigned long *argc,char **argv,char *sz_output)
{
	aui_attr_dsc attr;
	aui_hdl hdl;
	aui_hdl kl_hdl;
	int i;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* dsc data */
	unsigned char enc_16_cbc[DATA_LEN] = {
		0xe9, 0x84, 0x7e, 0xfe, 0x26, 0x5e, 0x2e, 0x5c,
		0xf9, 0xf9, 0x81, 0x41, 0x7c, 0x26, 0x75, 0x1a };

	unsigned char data_out[DATA_LEN];
	unsigned char iv[16 * 2]; /* odd and even */

	struct aui_attr_kl kl_attr;

	/* protected keys */
	unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef }};
	//{ 0xf5,0x70,0x13,0x7e,0x9f,0x1d,0x05,0x4f,
	//  0x13,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xfe }};

	/* expected result with root key at zero */
	//unsigned char result_keys[][16] = {
	//{ 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
	//  0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
	//{ 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
	//  0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
	//{ 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd,
	//  0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 }};
	//{ 0x15,0x75,0xf8,0x6d,0xf1,0x9c,0x79,0xb4, /* even */
	//  0x3c,0x42,0xd5,0x1a,0x4c,0x50,0xac,0x32 } };

	unsigned char result_dsc[DATA_LEN] = {
		0x0d,0x8a,0x69,0x0a,0xa4,0x7d,0x92,0xb3,
		0xf4,0x1d,0x06,0xcb,0x0c,0x81,0x52,0x07 };


	kl_attr.uc_dev_idx = 0;
	kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE;
	kl_attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	kl_attr.en_root_key_idx = 0;

	if (aui_kl_open(&kl_attr, &kl_hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(kl_hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
		goto r2r_kl_err;
	}

	unsigned char result_key[8]; /* 128 bits */
	if (aui_kl_read_key(kl_hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

#if 0
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");
	if (MEMCMP(result_key, result_keys[2] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	} else
		AUI_PRINTF("Key ok in pos %d\n", (int)key_dst_pos);
#endif

	attr.uc_dev_idx = 0;
	attr.uc_algo = AUI_DSC_ALGO_AES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl)) {
		AUI_PRINTF("dsc open error\n");
		err = 1;
		goto r2r_kl_err;
	}

	/* key from KL in CBC mode */
	attr.ul_key_pos = key_dst_pos;
	attr.ul_key_len = 16 * 8;
	attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used

	attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
	attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

	/* no PID */
	attr.ul_pid_cnt = 0;
	attr.pus_pids = NULL;

	/* zero IV */
	MEMSET(iv, 0, sizeof(iv));
	attr.puc_iv_ctr = iv;
	AUI_PRINTF("set IV, all zero\n");

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* Update IV */
	MEMSET(iv, 1, sizeof(iv));
	AUI_PRINTF("Update IV, change to all one\n");	

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result is changed\n");
	} else
		err = 1;

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* Update IV */
	MEMSET(iv, 0, sizeof(iv));
	AUI_PRINTF("Update IV, back to all zero\n");
	
	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

r2r_kl_err:
	if (aui_dsc_close(hdl)) {
		AUI_PRINTF("dsc open error\n");
		err = 1;
	}

	if (aui_kl_close(kl_hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}
#ifdef AUI_LINUX
static int get_file_size(FILE *file)
{
    int len;

    /* get file size */
    if (fseek(file, 0, SEEK_END))
        return -1;
    len = ftell(file);
    rewind(file);
    return len;
}

unsigned long dsc_ram2ram_aes_ctr(unsigned long *argc,char **argv,char *sz_output)
{
    aui_attr_dsc attr;
    aui_hdl hdl;
    int ret = 0;
    FILE *file_in = 0;
    FILE *file_out = 0;
    FILE *file_out_expected = 0;
    int data_len = 0;
    unsigned char *data_in = NULL;
    unsigned char *data_out = NULL;
    int cryto_mode;

    (void) sz_output;
    if(*argc < 3) {
        AUI_PRINTF("\t\tInvalid parameters:6 crypt_mode,input_file_name,ouput_file_name\n");
        AUI_PRINTF("\t\tcrypt_mode: =0,decrypt,=1:encrypt\n");
        return -1;
    }
    cryto_mode = ATOI(argv[0]);
    file_in = fopen(argv[1], "rb");
    if (!file_in) {
        printf("failed to open %s\n", argv[0]);
        ret = -1;
        goto r2r_err;
    }
    data_len = get_file_size(file_in);
    /* use NESTOR_MALLOC to handle small memory issue on TDS */
    data_in = MALLOC(data_len);
    if(!data_in) {
        printf("malloc error\n");
        ret = -1;
        goto r2r_err;
    }

    if (0 >= fread(data_in, 1, data_len, file_in)){
		ret = -1;
        goto r2r_err;
	}

    /* dsc data */
    unsigned char enc_16_ctr[2*16] = {
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
        0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x00,
        0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
        0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x00
    };

    unsigned char iv[16 * 2] = {
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    }; /* odd and even */

    attr.uc_dev_idx = 0;
    attr.uc_algo = AUI_DSC_ALGO_AES;
    attr.dsc_data_type = AUI_DSC_DATA_PURE;
    if (aui_dsc_open(&attr, &hdl)) {
        AUI_PRINTF("dsc open error\n");
        ret  = -1;
        goto r2r_err;
    }

    /* clear key in CBC mode */
    attr.puc_key = enc_16_ctr;
    attr.ul_key_len = 16 * 8;
    attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
    attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;

    attr.uc_mode = AUI_DSC_WORK_MODE_IS_CTR;
    attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_RESERVED;
    attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used
    /* no PID */
    attr.ul_pid_cnt = 0;
    attr.pus_pids = NULL;

    /* zero IV */
    attr.puc_iv_ctr = iv;

    if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
        AUI_PRINTF("dsc attach key error\n");
        aui_dsc_close(hdl);
        ret  = -1;
        goto r2r_err;
    }

    file_out = fopen(argv[2], "wb");
    if (!file_out) {
        printf("failed to open %s\n", argv[1]);
        ret  = -1;
        goto r2r_err;
    }

    data_out = (unsigned char *)MALLOC (data_len);

    if(!data_out) {
        printf("malloc error\n");
        ret = -1;
        goto r2r_err;
    }
    switch(cryto_mode) {
        case 0:
            if (aui_dsc_encrypt(hdl, data_in, data_in, data_len)) {
                AUI_PRINTF("dsc decrypt error\n");
                aui_dsc_close(hdl);
                ret = -1;
                goto r2r_err;
            }
            break;
        case 1:
            if (aui_dsc_decrypt(hdl, data_in, data_in, data_len)) {
                AUI_PRINTF("dsc decrypt error\n");
                aui_dsc_close(hdl);
                ret = -1;
                goto r2r_err;
            }
            break;
        default:
            AUI_PRINTF("Error encrypt/decrypt mode,it should be equal to 0 or 1.\n");
            ret = -1;
            goto r2r_err;
    }


    if(file_out) {
        rewind(file_out);
        if(data_len != (int)fwrite((char *)data_in,1,data_len,file_out)) {
            printf("fwrite error\n");
            ret = -1;
            goto r2r_err;
        }
    }

r2r_err:
    if (aui_dsc_close(hdl)) {
        AUI_PRINTF("dsc open error\n");
        ret = 1;
    }
    if (file_out)
        fclose(file_out);
    if (file_in) {
        fclose(file_in);

    }
    if (data_in)
        free(data_in);
    if (data_out)
        free(data_out);

    return ret;
}
#endif
unsigned long dsc_ram2ram_kl_tdes64(unsigned long *argc,char **argv,char *sz_output)
{
	aui_attr_dsc attr;
	aui_hdl hdl;
	aui_hdl kl_hdl;
	int i;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* dsc data */
	unsigned char enc_16_cbc[DATA_LEN] = {
		0x7d,0x9b,0xd6,0x86,0xa0,0x3f,0xd5,0x57,
		0x20,0xff,0x65,0xeb,0x60,0x7a,0xda,0x3f };

	unsigned char data_out[DATA_LEN];
	unsigned char iv[16 * 2]; /* odd and even */

	struct aui_attr_kl kl_attr;

	/* protected keys */
	unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f }};

	/* expected result with root key at zero */
	//unsigned char result_keys[][16] = {
	//{ 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
	//  0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
	//{ 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
	//  0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
	//{ 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd }};

	unsigned char result_dsc[DATA_LEN] = {
		0x0d,0x8a,0x69,0x0a,0xa4,0x7d,0x92,0xb3,
		0x89,0x86,0xd0,0x4d,0xac,0xbe,0x87,0x50 };

	kl_attr.uc_dev_idx = 0;
	kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE;
	kl_attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	kl_attr.en_root_key_idx = 0;

	if (aui_kl_open(&kl_attr, &kl_hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_SINGLE;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(kl_hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
		goto r2r_kl_err;
	}

	unsigned char result_key[8]; /* 128 bits */
	if (aui_kl_read_key(kl_hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

#if 0
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");
	if (MEMCMP(result_key, result_keys[2], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	} else
		AUI_PRINTF("Key ok in pos %d\n", (int)key_dst_pos);
#endif

	attr.uc_dev_idx = 0;
	attr.uc_algo = AUI_DSC_ALGO_DES;
	attr.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr, &hdl)) {
		AUI_PRINTF("dsc open error\n");
		err = 1;
		goto r2r_kl_err;
	}

	/* key from KL in CBC mode */
	attr.ul_key_pos = key_dst_pos;
	attr.ul_key_len = 8 * 8;
	attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	attr.en_parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE; // raw mode not used

	attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
	attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

	/* no PID */
	attr.ul_pid_cnt = 0;
	attr.pus_pids = NULL;

	/* zero IV */
	MEMSET(iv, 0, sizeof(iv));
	attr.puc_iv_ctr = iv;
	AUI_PRINTF("set IV, all zero\n");
	
	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* Update IV */
	MEMSET(iv, 1, sizeof(iv));
	AUI_PRINTF("Update IV, change to all one\n");	

	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result is changed\n");
	} else
		err = 1;

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

	/* Update IV */
	MEMSET(iv, 0, sizeof(iv));
	AUI_PRINTF("Update IV, back to all zero\n");
	
	if (aui_dsc_attach_key_info2dsc(hdl, &attr)) {
		AUI_PRINTF("dsc attach key error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (aui_dsc_decrypt(hdl, enc_16_cbc, data_out, DATA_LEN)) {
		AUI_PRINTF("dsc decrypt error\n");
		err = 1;
		goto r2r_kl_err;
	}

	if (MEMCMP(data_out, result_dsc, DATA_LEN)) {
		AUI_PRINTF("dsc result error\n");
		err = 1;
	}

	AUI_PRINTF("Data out ");
	for (i=0; i<DATA_LEN; i++)
		AUI_PRINTF("%02x ", data_out[i]);
	AUI_PRINTF("\n");

r2r_kl_err:
	if (aui_dsc_close(hdl)) {
		AUI_PRINTF("dsc open error\n");
		err = 1;
	}

	if (aui_kl_close(kl_hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}



unsigned long dsc_sha(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;
	unsigned char sha[64];
	unsigned char data[] = {
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a,
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a,
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a };

	unsigned char sha1[] = { 0xA6, 0x5C, 0x5E, 0x84, 0x8F, 0x16, 0x6D, 0xEA,
				 0xF7, 0xCC, 0x66, 0x6D, 0x63, 0x05, 0xEE, 0xAE,
				 0xAD, 0x7F, 0xB5, 0xF2 };
	int err = 0;
	int len;
	int i;

	/* SHA-1 */
	len = 160 / 8;
	if (aui_dsc_sha_digest(AUI_SHA_SRC_FROM_DRAM, data, sizeof(data),
			       AUI_SHA_1, sha))
		err = 1;
	else {
		for (i=0; i<len; i++)
			if (sha[i] != sha1[i])
				break;
		if (i != len)
			err = 1;
	}
	return err;
}

#ifdef AUI_LINUX
unsigned long dsc_sha_get_buffer(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;
	void *in_data = NULL;
	unsigned char sha[64];
	int size = 16*1024*1024;
	int err = 0;
	int len = 0;
	int i;
	AUI_RTN_CODE ret = AUI_RTN_FAIL;

	unsigned char data[] = {
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a,
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a,
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a };

	unsigned char sha1_result[] = { 0xA6, 0x5C, 0x5E, 0x84, 0x8F, 0x16, 0x6D, 0xEA,
				 0xF7, 0xCC, 0x66, 0x6D, 0x63, 0x05, 0xEE, 0xAE,
				 0xAD, 0x7F, 0xB5, 0xF2 };

    len = sizeof (sha1_result);
	aui_dsc_get_buffer(size, &in_data);

	if (in_data)
		printf ("in_data = %p\n", in_data);
	else
	{
		printf ("in_data is NULL\n");
		err = 1;
	}

	memset (in_data, 1, size);
	memset (sha, 0, sizeof(sha));
	memcpy (in_data, data, sizeof (data));

	printf ("out:\n");
	for (i=0; i<len; i++)
		printf ("0x%02x ", sha[i]);
	printf ("\n");

    ret = aui_dsc_sha_digest(AUI_SHA_SRC_FROM_DRAM, (unsigned char *)in_data, sizeof (data),AUI_SHA_1, sha);
	if (ret){
        printf ("aui_dsc_sha_digest() fail, ret=%d\n", ret);
		err = 1;
	}
	else {
		for (i=0; i<len; i++)
			if (sha[i] != sha1_result[i])
				break;
		if (i != len){
            printf ("the result of SHA is wrong!\n");
			err = 1;
		}
	}

	printf ("out:\n");
	for (i=0; i<len; i++)
		printf ("0x%02x ", sha[i]);
	printf ("\n");
	
	aui_dsc_release_buffer(size, in_data);

	return err;
}
#endif

unsigned long test_dsc_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nDSC Test Help");  
	 
	/* DSC_NOTE_HELP */
	#define 	DSC_NOTE_HELP	"The function of the DSC test is incomplete. Plese DONOT do the DSC test if non-professionals"

	aui_print_help_command("\'NOTE\'");
	aui_print_help_instruction_newline(DSC_NOTE_HELP);
	 
	
	return AUI_RTN_HELP;
}

#ifdef AUI_LINUX
unsigned long dsc_HMAC(unsigned long *argc,char **argv,char *sz_output)
{
	(void) argc;
	(void) argv;
	(void) sz_output;
	unsigned char hmac_out_1[32];
	unsigned char hmac_out_2[32];
	unsigned char data[] = {
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a,
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a,
		0xe9,0x84,0x7e,0xfe,0x26,0x5e,0x2e,0x5c,
		0xf9,0xf9,0x81,0x41,0x7c,0x26,0x75,0x1a };

	int err = 0;
	int len = 32;
	int i;

	unsigned char hmac_key[FIRST_KEY_LENGTH]=
    {
        0x22,0xdf,0x03,0x02,0x02,0x03,0xdf,0x22,
        0xaf,0xbf,0xa5,0xa5,0xaf,0xbf,0x5a,0x5a,
    };

	if (aui_dsc_generate_HMAC(data, sizeof(data), hmac_out_1, hmac_key))
	{
		err = 1;		
	}

	if (aui_dsc_generate_HMAC(data, sizeof(data), hmac_out_2, hmac_key))
	{
		err = 1;		
	}
		
	for (i=0; i<len; i++)
	{
		if (hmac_out_1[i] != hmac_out_2[i])
		{				
			break;
		}
	}

	if (i != len)
	{
		err = 1;
	}

	return err;
}
#endif /*AUI_LINUX*/



unsigned long aui_test_kl(unsigned long *argc,char **argv,char *sz_output)
{
    unsigned long ul_rtn=AUI_RTN_FAIL;
    aui_attr_kl attr_kl;
    aui_cfg_kl cfg_kl;
    aui_hdl hdl_kl=NULL;
    unsigned long ul_dst_key_pos=0XFFFFFFFF;
    unsigned char auc_buf_key[AUI_KL_MUTI_LEVEL_KEY_LEN_MAX]={  0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	                                                            0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71,
                                                                0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	                                                            0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	                                                            0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	                                                            0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef};
    
    unsigned char auc_buf_data[AUI_KL_MUTI_LEVEL_KEY_LEN_MAX]={ 0xe9, 0x84, 0x7e, 0xfe, 0x26, 0x5e, 0x2e, 0x5c,
		                                                        0xf9, 0xf9, 0x81, 0x41, 0x7c, 0x26, 0x75, 0x1a,
                                                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
                                                                0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03};
    aui_attr_dsc attr_dsc;
    aui_hdl hdl_dsc=NULL;

    unsigned char auc_data_out[DATA_LEN]={0};
	unsigned char auc_iv[16 * 2]={0}; /* odd and even */
    int i=0;
    
    MEMSET(&attr_kl,0,sizeof(attr_kl));
    MEMSET(&cfg_kl,0,sizeof(cfg_kl));
    MEMSET(&attr_dsc,0,sizeof(attr_dsc));

    attr_kl.uc_dev_idx=ATOI(argv[0]);
    attr_kl.en_level=ATOI(argv[1]);
    attr_kl.en_key_pattern=ATOI(argv[2]);
    attr_kl.en_root_key_idx=ATOI(argv[3]);
    AUI_PRINTF("\r\n attr_kl=[%08x][%08x][%08x][%08x]",attr_kl.uc_dev_idx,attr_kl.en_level,attr_kl.en_key_pattern,attr_kl.en_root_key_idx);
    ul_rtn=aui_kl_open(&attr_kl,&hdl_kl);
    if(AUI_RTN_SUCCESS!=ul_rtn)
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
        goto ERR;
    }

    cfg_kl.run_level_mode=ATOI(argv[4]);
    cfg_kl.en_crypt_mode=ATOI(argv[5]);
    cfg_kl.en_kl_algo=ATOI(argv[6]);
    cfg_kl.en_cw_key_attr=ATOI(argv[7]);
    AUI_PRINTF("\r\n cfg_kl=[%08x][%08x][%08x][%08x]",cfg_kl.run_level_mode,cfg_kl.en_crypt_mode,cfg_kl.en_kl_algo,cfg_kl.en_cw_key_attr);

    MEMCPY(cfg_kl.ac_key_val,auc_buf_key,AUI_KL_MUTI_LEVEL_KEY_LEN_MAX);
    ul_rtn=aui_kl_gen_key_by_cfg(hdl_kl,&cfg_kl,&ul_dst_key_pos);
    if(AUI_RTN_SUCCESS!=ul_rtn)
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
        goto ERR;
    }
    
    
	attr_dsc.uc_dev_idx = 0;
	attr_dsc.uc_algo = AUI_DSC_ALGO_AES;
	attr_dsc.dsc_data_type = AUI_DSC_DATA_PURE;
	if (aui_dsc_open(&attr_dsc, &hdl_dsc)) 
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
        goto ERR;
    }

	/* key from KL in CBC mode */
	attr_dsc.ul_key_pos = ul_dst_key_pos;
	attr_dsc.ul_key_len = 16 * 8;
	attr_dsc.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	attr_dsc.dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
	attr_dsc.en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0; // raw mode not used

	attr_dsc.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
	attr_dsc.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

	/* no PID */
	attr_dsc.ul_pid_cnt = 0;
	attr_dsc.pus_pids = NULL;

	/* zero IV */
	MEMSET(auc_iv, 0, sizeof(auc_iv));
	attr_dsc.puc_iv_ctr = auc_iv;
	AUI_PRINTF("set IV, all zero\n");

	if (aui_dsc_attach_key_info2dsc(hdl_dsc, &attr_dsc)) 
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
        goto ERR;
    }

	if (aui_dsc_decrypt(hdl_dsc, auc_buf_data, auc_data_out, DATA_LEN)) 
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
        goto ERR;
    }

	//if (MEMCMP(data_out, result_dsc, DATA_LEN)) 
    //{
        //AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
        //goto ERR;
    //}

	AUI_PRINTF("\r\n Data out: ");
	for (i=0; i<DATA_LEN; i++)
	{
		AUI_PRINTF("[%02x] ", auc_data_out[i]);
	}
	AUI_PRINTF("\n");



ERR:
	if (aui_dsc_close(hdl_dsc)) 
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
    }

	if (aui_kl_close(hdl_kl)) 
    {
        AUI_PRINTF("\r\n Func failed:[%s][%d]",__FUNCTION__,__LINE__);
    }
    return ul_rtn;
}

void dsc_test_reg(void)
{
	aui_tu_reg_group("dsc", "dsc ram2ram tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, dsc_ram2ram, "decrypt raw");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, dsc_ram2ram_des, "decrypt raw des");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, dsc_ram2ram_kl, "decrypt raw kl_aes");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, dsc_ram2ram_kl_tdes, "decrypt raw kl_tdes");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, dsc_ram2ram_kl_tdes64, "decrypt raw kl_tdes64");
#ifdef AUI_LINUX
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, dsc_ram2ram_aes_ctr, "encrypt/decrypt raw in aes_ctr");
#endif
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, dsc_sha, "sha");
#ifdef AUI_LINUX
	aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, dsc_HMAC, "generate hmac message");
	aui_tu_reg_item(2, "9", AUI_CMD_TYPE_API, dsc_sha_get_buffer, "dsc_sha_get_buffer");
#endif /*AUI_LINUX*/
    aui_tu_reg_item(2, "kl", AUI_CMD_TYPE_API, aui_test_kl, "kl key test");
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_dsc_help, "DSC test help");
}

