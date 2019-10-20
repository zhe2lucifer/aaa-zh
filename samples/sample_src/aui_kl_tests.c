/**@file
 *   @brief     ALi AUI KL test
 *   @author    romain.baeriswyl
 *   @date      2014-02-25
 *   @version   1.0.0
 *   @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <aui_kl.h>
#include <aui_dsc.h>
#include "aui_test_app_cmd.h"
#include "aui_help_print.h"
#ifdef AUI_LINUX
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#endif

/* Debug function to read last processed key */
AUI_RTN_CODE aui_kl_read_key(aui_hdl handle, unsigned char *key);

/* test open and close */
unsigned long test_kl_openclose(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl, p;
	int dev_idx = 0;
	aui_hdl hdls[10];
	int i;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* test 1 */
	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE;
	attr.en_level = 0;
	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	if (aui_find_dev_by_idx(AUI_MODULE_KL, dev_idx, &p)) {
		AUI_PRINTF("aui_find_dev_by_idx fault\n");
		err = 1;
	}
	if (p != hdl) {
		AUI_PRINTF("aui_find_dev_by_idx wrong result\n\n");
		err = 1;
	}
	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_close error\n");
		err = 1;
	}
	AUI_PRINTF("open/close first level %s\n\n", err ? "FAILED" : "PASSED");
	int errs = err;

	/* test 2 */
	err = 0;
	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_close error\n");
		err = 1;
	}
	AUI_PRINTF("open/close three levels with single 64bits key %s\n\n",
	       err ? "FAILED" : "PASSED");
	errs += err;

	/* test 3 */
	err = 0;
	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_close error\n");
		err = 1;
	}
	AUI_PRINTF("open/close 1x three levels with odd/even 128bits key %s\n\n",
	       err ? "FAILED" : "PASSED");
	errs += err;

	/* test 4 */
	err = 0;
	for (i=0; i<3; i++) {
		attr.uc_dev_idx = dev_idx + i;
		attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
		attr.en_level = AUI_KL_KEY_THREE_LEVEL;
		if (aui_kl_open(&attr, &hdls[i])) {
			AUI_PRINTF("aui_kl_open error\n");
			err = 1;
		}
	}
	for (i=0; i<3; i++) {
		if (aui_find_dev_by_idx(AUI_MODULE_KL, dev_idx + i, &p)) {
			AUI_PRINTF("aui_find_dev_by_idx fault\n");
			err = 1;
		}
		if (p != hdls[i]) {
			AUI_PRINTF("aui_find_dev_by_idx wrong result\n");
			err = 1;
		}
	}
	for (i=0; i<3; i++)
		if (aui_kl_close(hdls[i])) {
			AUI_PRINTF("aui_kl_close error\n");
			err = 1;
		}

	AUI_PRINTF("open/close 3x three levels with odd/even 64bits key %s\n\n",
	       err ? "FAILED" : "PASSED");
	errs += err;

	/* test 5 */
	err = 0;
	for (i=0; i<4; i++) {
		attr.uc_dev_idx = dev_idx + i;
		attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
		attr.en_level = AUI_KL_KEY_THREE_LEVEL;
		if (aui_kl_open(&attr, &hdls[i])) {
			AUI_PRINTF("aui_kl_open error\n");
			err = 1;
		}
	}
	for (i=0; i<4; i++) {
		if (aui_find_dev_by_idx(AUI_MODULE_KL, dev_idx + i, &p)) {
			AUI_PRINTF("aui_find_dev_by_idx fault\n");
			err = 1;
		}
		if (p != hdls[i]) {
			AUI_PRINTF("aui_find_dev_by_idx wrong result\n");
			err = 1;
		}
	}
	for (i=0; i<4; i++)
		if (aui_kl_close(hdls[i])) {
			AUI_PRINTF("aui_kl_close error\n");
			err = 1;
		}

	AUI_PRINTF("open/close 4x three levels with odd/even 128bits key %s\n",
	       err ? "FAILED" : "PASSED");
	errs += err;

	return errs;
}


/* test one level key decryption */
unsigned long test_kl_onelevel(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* expected result with root key are zero */
	const unsigned char aes_key_1_level_zero_keys[16] = {
		0x14, 0x0f, 0x0f, 0x10, 0x11, 0xb5, 0x22, 0x3d,
		0x79, 0x58, 0x77, 0x17, 0xff, 0xd9, 0xec, 0x3a };

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE;
	attr.en_level = AUI_KL_KEY_ONE_LEVEL;
	attr.en_root_key_idx = 2;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_SINGLE;

	/* protected key at 0 */
	MEMSET(cfg.ac_key_val, 0, 16);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	AUI_PRINTF("Result in key pos %d\n", (int)key_dst_pos);

	unsigned char result_key[16]; /* 128 bits */
	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	int i;
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, aes_key_1_level_zero_keys, 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}

	return err;
}

/* test three level key decryption with protected key at zero */
unsigned long test_kl_threelevel_keyzero(unsigned long *argc, char **argv,
						 char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* expected result with root key at zero */
	const unsigned char aes_key_2_3_level_zero_keys[][16] = {
	{ 0x28,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xe8,0x46,0xd4,0xb0,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x86,0x7e,0x9f,0x1d,0x05,0x4f } };

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_SINGLE;

	/* protected keys at 0 */
	MEMSET(cfg.ac_key_val, 0, sizeof(cfg.ac_key_val));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	unsigned char result_key[16]; /* 128 bits */
	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	int i;
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, aes_key_2_3_level_zero_keys[1], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}
	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}

/* test three level key decryption with none zero protected key */
unsigned long test_kl_threelevel(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* protected keys */
	const unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef }};

	/* expected result with root key at zero */
	const unsigned char result_keys[][16] = {
	{ 0x3e,0x07,0x4d,0xee,0xcf,0xe1,0x74,0xa5,
	  0x0e,0xde,0x9d,0xbf,0xb3,0xf9,0x65,0x14 },
	{ 0xd0,0x29,0x84,0x10,0x0e,0xe6,0x90,0x00,
	  0x24,0x27,0x00,0x28,0x51,0x69,0x5e,0x2c },
	{ 0xd0,0xc4,0x54,0xbd,0xf3,0xd7,0x23,0x2f,
	  0xc9,0xf9,0x7b,0x70,0x59,0x56,0xd3,0x84 }};



	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
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

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	unsigned char result_key[16]; /* 128 bits */
	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	int i;
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}
	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}

#ifdef AUI_LINUX
/* test three level key AES decryption with none zero protected key and odd/even */
unsigned long test_kl_threelevel_parity(unsigned long *argc,char **argv,char *sz_output)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* protected keys with odd CW */
	const unsigned char protected_keys_odd[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef }};

	/* protected keys with even CW */
	const unsigned char protected_keys_even[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x13,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x13,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xfe }};

	/* expected result with root key at zero */
	const unsigned char result_keys[][16] = {
	{ 0x3e,0x07,0x4d,0xee,0xcf,0xe1,0x74,0xa5,
	  0x0e,0xde,0x9d,0xbf,0xb3,0xf9,0x65,0x14 },
	{ 0xd0,0x29,0x84,0x10,0x0e,0xe6,0x90,0x00,
	  0x24,0x27,0x00,0x28,0x51,0x69,0x5e,0x2c },
	{ 0xd0,0xc4,0x54,0xbd,0xf3,0xd7,0x23,0x2f, /* odd */
	  0xc9,0xf9,0x7b,0x70,0x59,0x56,0xd3,0x84 },
	{ 0x76,0x86,0x61,0xda,0xa2,0x0f,0xfd,0x65, /* even */
	  0xa4,0xb3,0xa1,0x8d,0x82,0xa4,0xc6,0x08 }};

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;
	int i;
	unsigned char result_key[16]; /* 128 bits */

	/* Get EVEN key by running all levels */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys_even, sizeof(protected_keys_even));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("EVEN key result: ");
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/* Get ODD key by running only last level */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;

	MEMCPY(cfg.ac_key_val, protected_keys_odd, sizeof(protected_keys_odd));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("ODD key result: ");
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}

/* test three level key TDES decryption with none zero protected key and odd/even */
unsigned long test_kl_threelevel_tdes(unsigned long *argc,char **argv,char *sz_output)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	int i;
	unsigned char result_key[16]; /* 128 bits */
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* protected keys */
	const unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f } };

	const unsigned char protected_cw_odd[16] = {
		0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
		0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef };

	const unsigned char protected_cw_even[16] = {
		0xf5,0x70,0x13,0x7e,0x9f,0x1d,0x05,0x4f,
		0x13,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xfe };

	/* expected result with root key at zero */
	const unsigned char result_keys[][16] = {
	{ 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
	  0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
	{ 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
	  0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
	{ 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd, /* odd CW */
	  0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 },
	{ 0x15,0x75,0xf8,0x6d,0xf1,0x9c,0x79,0xb4, /* even CW */
	  0x3c,0x42,0xd5,0x1a,0x4c,0x50,0xac,0x32 } };

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;

	/*
	 * generate EVEN key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys), protected_cw_even, 16);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/*
	 * generate ODD key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys), protected_cw_odd, 16);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/*
	 * generate last level ODD & EVEN keys
	 */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys), protected_cw_odd, 16);
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys) + 16, protected_cw_even, 16);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}

/* test three level key TDES decryption with none zero protected key and odd/even */
unsigned long test_kl_threelevel_tdes64(unsigned long *argc,char **argv,char *sz_output)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	unsigned char result_key[16];
	int i;
	(void) argc;
	(void) argv;
	(void) sz_output;

	/* protected keys */
	const unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f }};

	const unsigned char protected_cw_odd[8] = {
		0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f };

	const unsigned char protected_cw_even[8] = {
		0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef };

	/* expected result with root key are zero */
	const unsigned char result_keys[][16] = {
	{ 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
	  0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
	{ 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
	  0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
	{ 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd }, /* odd */
	{ 0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 }}; /* even */

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;

	/*
	 * generate EVEN key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys), protected_cw_even, 8);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}


	/*
	 * generate ODD key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys), protected_cw_odd, 8);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/*
	 * generate last level ODD & EVEN keys
	 */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys), protected_cw_odd, 8);
	MEMCPY(cfg.ac_key_val + sizeof(protected_keys) + 8, protected_cw_even, 8);

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}
#else
/* test three level key AES decryption with none zero protected key and odd/even */
unsigned long test_kl_threelevel_parity(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* protected keys */
	const unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef },
	{ 0xf5,0x70,0x13,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x13,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xfe }};

	/* expected result with root key at zero */
	const unsigned char result_keys[][16] = {
	{ 0x3e,0x07,0x4d,0xee,0xcf,0xe1,0x74,0xa5,
	  0x0e,0xde,0x9d,0xbf,0xb3,0xf9,0x65,0x14 },
	{ 0xd0,0x29,0x84,0x10,0x0e,0xe6,0x90,0x00,
	  0x24,0x27,0x00,0x28,0x51,0x69,0x5e,0x2c },
	{ 0xd0,0xc4,0x54,0xbd,0xf3,0xd7,0x23,0x2f,
	  0xc9,0xf9,0x7b,0x70,0x59,0x56,0xd3,0x84 },
	{ 0x76,0x86,0x61,0xda,0xa2,0x0f,0xfd,0x65,
	  0xa4,0xb3,0xa1,0x8d,0x82,0xa4,0xc6,0x08 }};

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;
	int i;
	unsigned char result_key[16]; /* 128 bits */

	/* Get EVEN key by running all levels */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("EVEN key result: ");
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/* Get ODD key by running only last level */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD;
	cfg.en_kl_algo = AUI_KL_ALGO_AES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("ODD key result: ");
	for (i=0; i<16; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3], 16)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}

/* test three level key TDES decryption with none zero protected key and odd/even */
unsigned long test_kl_threelevel_tdes(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	int i;
	unsigned char result_key[16]; /* 128 bits */
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* protected keys */
	const unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef },
	{ 0xf5,0x70,0x13,0x7e,0x9f,0x1d,0x05,0x4f,
	  0x13,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xfe }};

	/* expected result with root key at zero */
	const unsigned char result_keys[][16] = {
	{ 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
	  0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
	{ 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
	  0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
	{ 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd,
	  0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 },
	{ 0x15,0x75,0xf8,0x6d,0xf1,0x9c,0x79,0xb4,
	  0x3c,0x42,0xd5,0x1a,0x4c,0x50,0xac,0x32 } };

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;

	/*
	 * generate EVEN key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/*
	 * generate ODD key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/*
	 * generate last level ODD & EVEN keys
	 */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2] + 8, 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}


/* test three level key TDES decryption with none zero protected key and odd/even */
unsigned long test_kl_threelevel_tdes64(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct aui_attr_kl attr;
	aui_hdl hdl;
	int dev_idx = 0;
	int err = 0;
	unsigned char result_key[16];
	int i;
	(void) argc;
	(void) argv;
	(void) sz_out_put;

	/* protected keys */
	const unsigned char protected_keys[][16] = {
	{ 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
	  0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
	  0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
	{ 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef }};

	/* expected result with root key are zero */
	const unsigned char result_keys[][16] = {
	{ 0xb8,0x90,0x58,0xdb,0x13,0x1e,0x06,0xd7,
	  0xe3,0x01,0x22,0xa5,0x51,0x9c,0xbb,0x0e },
	{ 0xb0,0xe0,0xc4,0xae,0xbd,0x2f,0x77,0xc7,
	  0x20,0x1d,0xf7,0x3c,0x53,0xe5,0x1f,0x26 },
	{ 0x8d,0x5e,0x5d,0x6a,0x31,0x11,0xf2,0xbd },
	{ 0x07,0xf9,0x04,0xe0,0x57,0xf9,0x80,0xe4 }};

	attr.uc_dev_idx = dev_idx;
	attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
	attr.en_level = AUI_KL_KEY_THREE_LEVEL;
	attr.en_root_key_idx = 0;

	if (aui_kl_open(&attr, &hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		return 1;
	}

	unsigned long key_dst_pos;
	struct aui_cfg_kl cfg;

	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
	cfg.en_kl_algo = AUI_KL_ALGO_TDES;
	cfg.en_crypt_mode = AUI_KL_DECRYPT;

	/*
	 * generate EVEN key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}


	/*
	 * generate EVEN key
	 */
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;

	MEMCPY(cfg.ac_key_val, protected_keys, sizeof(protected_keys));

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}

	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[3], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	/*
	 * generate last level ODD & EVEN keys
	 */
	cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD;
	cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD_EVEN;

	if (aui_kl_gen_key_by_cfg(hdl, &cfg, &key_dst_pos)) {
		AUI_PRINTF("aui_kl_gen_key_by_cfg error\n");
		err = 1;
	}

	if (aui_kl_read_key(hdl, result_key)) {
		AUI_PRINTF("aui_kl_read_key error\n");
		err = 1;
	}
	AUI_PRINTF("key result: ");
	for (i=0; i<8; i++)
		AUI_PRINTF("%02x ", result_key[i]);
	AUI_PRINTF("\n");

	if (MEMCMP(result_key, result_keys[2], 8)) {
		AUI_PRINTF("Key result wrong\n");
		err = 1;
	}

	if (aui_kl_close(hdl)) {
		AUI_PRINTF("aui_kl_open error\n");
		err = 1;
	}
	return err;
}
#endif

#ifdef AUI_LINUX
unsigned long test_kl_hdcp(unsigned long *argc,char **argv,char *sz_output)
{
	int ret, err = 0;
	FILE *file;
	char buffer[288];

	*sz_output = 0;
	if (*argc != 1) {
		printf("hdcp <file>\n");
		return -1;
	}

	file = fopen(argv[0], "r");
	if (!file) {
		printf("file open error\n");
		return -1;
	}
	ret = fread(buffer, 1, 288, file);
	if (ret != 288) {
		printf("file read error\n");
		err = 1;
		goto err_hdcp;
	}

	if (aui_kl_load_hdcp_key((unsigned char *)buffer, 288)) {
		printf("hdcp decrypt error\n");
		err = 1;
	}
err_hdcp:
	fclose(file);
	return err;
}
#endif

unsigned long test_kl_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nKeyLadder Test Help");  
	 
	/* KL_1_HELP */
	#define 	KL_1_HELP_PART1	"Test 4 patterns of KL module's opening and closing. The patterns of KL is below:"
	#define 	KL_1_HELP_PART2	"1. Output 64BIT single key in last level(3rd) calculated"
	#define 	KL_1_HELP_PART3 	"2. Output 64BIT odd and even key in last level(3rd) calculated"
	#define 	KL_1_HELP_PART4 	"3. Output 128BIT single key in last level(3rd) calculated"
	#define 	KL_1_HELP_PART5 	"4. Output 128BIT odd and even key in last level(3rd) calculated\n"
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Test the KL module's opening and closing\n");
	aui_print_help_instruction_newline(KL_1_HELP_PART1);
	aui_print_help_instruction_newline(KL_1_HELP_PART2);
	aui_print_help_instruction_newline(KL_1_HELP_PART3);
	aui_print_help_instruction_newline(KL_1_HELP_PART4);
	aui_print_help_instruction_newline(KL_1_HELP_PART5);
	 
	/* KL_2_HELP */
	#define 	KL_2_HELP	"Test one level key AES decryption\n"
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline(KL_2_HELP);
	
	/* KL_3_HELP */
	#define 	KL_3_HELP	"Test three level key AES decryption with protected key at zero\n"
	aui_print_help_command("\'3\'");
	aui_print_help_instruction_newline(KL_3_HELP);
	
	/* KL_4_HELP */
	#define 	KL_4_HELP	"Test three level key AES decryption with none zero protected key\n"
	aui_print_help_command("\'4\'");
	aui_print_help_instruction_newline(KL_4_HELP);

	/* KL_5_HELP */
	#define 	KL_5_HELP	"Test three level key AES decryption with none zero protected key and odd/even\n"
	aui_print_help_command("\'5\'");
	aui_print_help_instruction_newline(KL_5_HELP);

	/* KL_6_HELP */
	#define 	KL_6_HELP	"Test three level key TDES decryption with none zero protected key and odd/even\n"
	aui_print_help_command("\'6\'");
	aui_print_help_instruction_newline(KL_6_HELP);
	
	/* KL_7_HELP */
	#define 	KL_7_HELP	"Test three level 64BIT key TDES decryption with none zero protected key and odd/even\n"
	aui_print_help_command("\'7\'");
	aui_print_help_instruction_newline(KL_7_HELP);
	
	return AUI_RTN_HELP;
}



void kl_tests_reg(void)
{
	aui_tu_reg_group("kl", "kl tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_kl_openclose, "open/close");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_kl_onelevel, "1 level AES");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_kl_threelevel_keyzero,
			"3 level AES key=0");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_kl_threelevel, "3 level AES");
	aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_kl_threelevel_parity,
			"3 level AES parity");
	aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_kl_threelevel_tdes,
			"3 level TDES parity");
	aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_kl_threelevel_tdes64,
			"3 level TDES64 parity ");
#ifdef AUI_LINUX
	aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, test_kl_hdcp, "hdcp");
#endif
	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_kl_help, "KL test help");
}

