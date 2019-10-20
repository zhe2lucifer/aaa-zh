/**@file
 *	 @brief 	ALi AUI NIM test
 *	 @author	romain.baeriswyl
 *	 @date		2014-02-12
 *	 @version	1.0.0
 *	 @note		ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <aui_nim.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

/* Frequency band */
#define KU_BAND
//#define C_BAND

int dvbs_connect(int freq, int sym, int fec, int polar, int device)
{
	aui_hdl nim_hdl = 0;
	struct aui_nim_attr nim_attr;
	struct aui_nim_connect_param param;

	MEMSET(&param, 0, sizeof(struct aui_nim_connect_param));

	nim_attr.ul_nim_id = device;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;

	if (aui_nim_open(&nim_attr, &nim_hdl) != AUI_RTN_SUCCESS) {
		printf("open error\n");
		return -1;
	}

	printf("nim device %d opened\n", (int)nim_attr.ul_nim_id);

#ifdef KU_BAND
	if (freq > 11700) {
		param.connect_param.sat.ul22K = 1;
		param.ul_freq = freq - 10600;
	} else {
		param.connect_param.sat.ul22K = 0;
		param.ul_freq = freq - 9750;
	}
#endif

#ifdef C_BAND
	param.connect_param.sat.ul22K = 0;
	param.ul_freq = 5150 - freq;
#endif
	param.connect_param.sat.ul_symrate = sym;
	param.connect_param.sat.fec = fec;
	param.connect_param.sat.ul_polar = polar;
	param.connect_param.sat.ul_lnb_type = AUI_NIM_LNB_CTRL_STD;
	param.connect_param.sat.ul_lnb_low = 0;
	param.connect_param.sat.ul_lnb_high = 0;

	if (aui_nim_connect(nim_hdl, &param) != AUI_RTN_SUCCESS)	{
		printf("connect error\n");
		aui_nim_close(&nim_hdl);
		return -1;
	}

	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int cnt = 0;
	while (lock_status != AUI_NIM_STATUS_LOCKED) {
		if (aui_nim_is_lock(nim_hdl, &lock_status) != AUI_RTN_SUCCESS) {
			printf("is_lock error\n");
			aui_nim_disconnect(&nim_hdl);
			aui_nim_close(&nim_hdl);
			return -1;
		}
		usleep(10000);
		cnt++;
	}

	printf("channel is locked on freq %d in %d ms\n", freq, cnt * 10);

	aui_signal_status info;
	if (aui_nim_signal_info_get(nim_hdl, &info) != AUI_RTN_SUCCESS) {
		printf("info_get error\n");
		aui_nim_disconnect(&nim_hdl);
		aui_nim_close(&nim_hdl);
		return -1;
	}

	printf("info: freq %ld, strength %d, quality %d, ber %d, rf_level %d,"
		   "signal_cn %d\n", info.ul_freq, (int)info.ul_signal_strength,
		   (int)info.ul_signal_quality, (int)info.ul_bit_error_rate,
		   (int)info.ul_rf_level, (int)info.ul_signal_cn);

	aui_nim_disconnect(nim_hdl);

	aui_nim_close(nim_hdl);
	return 0;
}

int fn_aui_ascallback(unsigned char status, unsigned char polar,
			  unsigned int freq, unsigned int sym, unsigned char fec)
{
	printf("*** status = %d,polar = %d,freq = %d,sym = %d"
		   ",fec = %d\n", status, polar, freq, sym, fec);
	return 0;
}

void dvbs_blind_scan_mode(int band, int polar, int dev)
{
	aui_hdl nim_hdl = 0;

	aui_nim_attr nim_attr;
	nim_attr.ul_nim_id = dev;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;
	if (aui_nim_open(&nim_attr,&nim_hdl) != AUI_RTN_SUCCESS) {
		printf("open error\n");
		return;
	}

	aui_autoscan_sat as_info;
	MEMSET(&as_info, 0, sizeof(aui_autoscan_sat));
	as_info.ul_start_asfreq_mhz = 950;
	as_info.ul_stop_asfreq_mhz = 2150;
	as_info.ul_polar = polar;
	as_info.ul22K = band;
	as_info.aui_as_cb = fn_aui_ascallback;

	if (aui_nim_auto_scan_start(nim_hdl, &as_info) != AUI_RTN_SUCCESS) {
		printf("scan_start error\n");
		return;
	}
	return;
}

#include <sys_define.h>

/* from platform/inc/bus/i2c/i2c.h */
#define I2C_TYPE_SCB	0x00000000
#define I2C_TYPE_GPIO	 0x00010000
#define I2C_TYPE_SCB_RM    0x00020000

#define I2C_TYPE_SCB0	 (I2C_TYPE_SCB|0)
#define I2C_TYPE_SCB1	 (I2C_TYPE_SCB|1)
#define I2C_TYPE_SCB2	 (I2C_TYPE_SCB|2)

#define I2C_TYPE_GPIO0	  (I2C_TYPE_GPIO|0)
#define I2C_TYPE_GPIO1	  (I2C_TYPE_GPIO|1)
#define I2C_TYPE_GPIO2	  (I2C_TYPE_GPIO|2)

enum {
	GPIO_I2C0_SDA_INDEX,
	GPIO_I2C0_SCL_INDEX,
	SYS_MUTE_INDEX,
	NIM_RESET_IO,
	GPIO_I2C1_SDA_INDEX,
	GPIO_I2C1_SCL_INDEX,

	GPIO_LNB_CUT0_INDEX,
	GPIO_LNB_CUT1_INDEX
};

#define HAL_GPIO_I_DIR		  0
#define HAL_GPIO_O_DIR		  1

/* associated parameters */
static struct aui_gpio_info gpio_array[] =
{
	{1, HAL_GPIO_O_DIR, 134},	   //XPMU_SDA
	{1, HAL_GPIO_O_DIR, 135},	   //XPMU_SCL
	{0, HAL_GPIO_O_DIR, 19},	   //MUTE
	{1, HAL_GPIO_O_DIR, 5}, 	   //NIM RESET

	{1, HAL_GPIO_O_DIR, 8}, 	   // SCART_SDA
	{1, HAL_GPIO_O_DIR, 7}, 	   // SCART_SCL

	{1, HAL_GPIO_O_DIR, 6}, 	   //LNB_CUT1
	{1, HAL_GPIO_O_DIR, 1}		   //LNB_CUT2
};


AUI_RTN_CODE nim_driver_init_cb(struct aui_nim_config *nim_config)
{
	int i;

	/* Parameters guessed from board_config.c */
	/* Only M3503 supported */

	if (!nim_config)
		return AUI_RTN_FAIL;

	for (i=0; i<2; i++) {
		struct aui_nim_config *nim = nim_config + i;

		nim->tuner.freq_low = 900;
		nim->tuner.freq_high = 2200;

		if (i == 0) {
			nim->id = AUI_NIM_ID_M3503_0;
			nim->nim_reset_gpio = NULL;
			nim->lnb_power_gpio = NULL;

			nim->demod.i2c_type_id =  I2C_TYPE_SCB0;
			nim->demod.i2c_base_addr = 0x66;
			nim->demod.QPSK_Config = /* 0xe9 */
				QPSK_CONFIG_FREQ_OFFSET |
				QPSK_CONFIG_I2C_THROUGH |
				QPSK_CONFIG_NEW_AGC1 |
				QPSK_CONFIG_MODE_8BIT;

			nim->tuner.i2c_type_id = I2C_TYPE_SCB0;
			nim->tuner.i2c_base_addr = 0xc0;
			nim->tuner.id = SHARP_VZ7306;
		} else {
			nim->id = AUI_NIM_ID_M3501_1;

			//gpio_array + NIM_RESET_IO
			nim->nim_reset_gpio = NULL;

			// gpio_array + GPIO_LNB_CUT1_INDEX;
			nim->lnb_power_gpio = NULL;

			nim->demod.i2c_type_id =  I2C_TYPE_GPIO1;
			nim->demod.i2c_base_addr = 0x66;
			nim->demod.QPSK_Config =
				QPSK_CONFIG_FREQ_OFFSET |
				QPSK_CONFIG_I2C_THROUGH |
				QPSK_CONFIG_NEW_AGC1 |
				QPSK_CONFIG_MODE_4BIT;

			nim->tuner.i2c_type_id = I2C_TYPE_GPIO1;
			nim->tuner.i2c_base_addr = 0xc0;
			nim->tuner.id = SHARP_VZ7306;
		}
	}
	return 0;
}


int main(int argc, char *argv[])
{
	int ret;
	int opt;
	int option_index;
	int scan_mode = 0;
	int freq = 0;
	int symb = 0;
	int polar = 0;
	int dev = 0;
	int band = 0;

	static struct option long_options[] = {
		{ "help",		 no_argument,		0, 'h' },
		{ "scan",		 no_argument,		0, 'c' },
		{ "freq",		 required_argument, 0, 'f' },
		{ "symbol rate", required_argument, 0, 's' },
		{ "polar",		 required_argument, 0, 'p' },
		{ "device", 	 required_argument, 0, 'd' },
		{ "band",		 required_argument, 0, 'b' },
		{ 0, 0, 0, 0}
	};

	while ((opt = getopt_long (argc, argv, "hcf:s:p:d:b:",
				   long_options, &option_index)) != -1) {

		switch (opt) {
		case 'c':
			scan_mode = 1;
			break;
		case 'f':
			freq = atoi(optarg);
			break;
		case 's':
			symb = atoi(optarg);
			break;
		case 'p':
			polar = atoi(optarg);
			break;
		case 'd':
			dev = atoi(optarg);
			break;
		case 'b':
			band = atoi(optarg);
			break;
		case 'h':
			printf("Usage: %s -h help\n"
				   " connect:\n"
				   "   -f <frequency>\n"
				   "   -s <symbole rate>\n"
				   "   -p <0:horizontal, 1:vertical>\n"
				   "   -d <device 0 or 1> \n"
				   " autoscan:\n"
				   "   -c\n"
				   "   -b <0:low band, 1:high band> for Ku-band\n"
				   "   -p <0:horizontal, 1:vertical>\n"
				   "   -d <device 0 or 1> \n", argv[0]);
			return 0;

		}
	}

	if (aui_nim_init((p_fun_cb)nim_driver_init_cb) != AUI_RTN_SUCCESS) {
		printf("init error\n");
		return -1;
	}

	if (scan_mode) {
		dvbs_blind_scan_mode(band, polar, dev);
	} else if (freq & symb) {
		ret = dvbs_connect(freq, symb, 0, polar, dev);
	}

	if (aui_nim_de_init(NULL))
		printf("aui_nim_de_init error\n");

	return 0;
}
