
#include <aui_nim.h>
#include <aui_gpio.h>
#include <aui_tsi.h>
#include "aui_nim_init.h"

#define tuner_chip_maxlinear	6

#define INC_INDEX(index) { index++; if (index == AUI_NIM_HANDLER_MAX) return AUI_RTN_FAIL; }

AUI_RTN_CODE nim_init_cb(void *pv_param)
{
	struct aui_nim_config *nim, *nim_config = (struct aui_nim_config *)pv_param;
	int index = 0;
	if (!nim_config)
		return AUI_RTN_FAIL;

#if (defined BOARD_CFG_M3515) || (defined BOARD_CFG_M3715B) // 3715B need modify

	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;

#if (defined AUI_BOARD_VERSION_01V04)
	/* first NIM : DVB-S with internal demod and Sharp VZ7306 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB0;
	tuner->i2c_base_addr = 0xc4;
	tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB0;
	demod->i2c_base_addr = 0xe6;
	demod->QPSK_Config = 0xfd;

	/* second NIM : with external demod M3501 and Sharp VZ7306 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_GPIO0;
	tuner->i2c_base_addr = 0xc4;
	tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_GPIO0;
	demod->i2c_base_addr = 0x66;
	demod->QPSK_Config = 0x7d;

#else
	/* first NIM : DVB-S with internal demod and Sharp VZ7306 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB0;
	tuner->i2c_base_addr = 0xc0;
	tuner->id = AUI_SHARP_VZ7306;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB0;
	demod->i2c_base_addr = 0xe6;
	demod->QPSK_Config = /* 0xe9 */
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_8BIT;

	/* second NIM : with external demod M3501 and Sharp VZ7306 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_GPIO0;
	tuner->i2c_base_addr = 0xc0;
	tuner->id = AUI_SHARP_VZ7306;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_GPIO0;
	demod->i2c_base_addr = 0x66;
	demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_2BIT;
#endif

#elif (defined BOARD_CFG_M3515B)

	struct aui_tuner_dvbs_config *dvbs_tuner;
	struct aui_demod_dvbs_config *dvbs_demod;

#if (defined AUI_BOARD_VERSION_01V01)

	/* first NIM : internal demod and AV2012 tuner */
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB0;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB0;
	dvbs_demod->i2c_base_addr = 0xe6;

	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_IQ_SWAP     |
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_8BIT;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO0;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5; /* DEMO_RST signal */
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO0;
	dvbs_demod->i2c_base_addr = 0x66;
	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_IQ_SWAP     |
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_2BIT;

#elif (defined AUI_BOARD_VERSION_01V02)
	/* first NIM : internal demod and AV2012 tuner */
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB0;
	dvbs_tuner->i2c_base_addr = 0xc4;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3503_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB0;
	dvbs_demod->i2c_base_addr = 0xe6;

	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_IQ_SWAP 	|
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_8BIT;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO0;
	dvbs_tuner->i2c_base_addr = 0xc4;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5; /* DEMO_RST signal */
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1; /* CUT_LNB signal */
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO0;
	dvbs_demod->i2c_base_addr = 0x66;
	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_IQ_SWAP 	|
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_2BIT;
#else
#error "Selected board is not this version by AUI"
#endif

#elif (defined BOARD_CFG_M3733)

	/* first NIM : DVB-C with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL603;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc2;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB0; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_M3281_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
	dvbc_demod->i2c_base_addr = 0x66;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner = &nim->config.dvbs.tuner;
	struct aui_demod_dvbs_config *dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 76;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x66;
	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_2BIT;

	/* Full NIM 1 : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc0;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 76;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x06;
	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_1BIT;

	/* Full NIM 2 : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc0;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_2;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x46;
	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_1BIT;

	/* Full NIM 3 : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbs_tuner->i2c_base_addr = 0xc0;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_3;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbs_demod->i2c_base_addr = 0x26;
	dvbs_demod->QPSK_Config =
		QPSK_CONFIG_FREQ_OFFSET |
		QPSK_CONFIG_I2C_THROUGH |
		QPSK_CONFIG_POLAR_REVERT |
		QPSK_CONFIG_NEW_AGC1 |
		QPSK_CONFIG_MODE_1BIT;

#elif (defined BOARD_CFG_M3823)

#if (defined AUI_BOARD_VERSION_04V01)
	/* first NIM : ISDBT with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	//struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_tuner->freq_low = 40; // KHz
	dvbt_tuner->freq_high = 900; // KHz
	dvbt_tuner->id = AUI_MXL603;
	dvbt_tuner->agc_ref = 0x63;
	//dvbt_tuner->rf_agc_min = 0x2A; // ?
	//dvbt_tuner->rf_agc_max = 0xBA; // ?
	dvbt_tuner->if_agc_max = 0xC3;
	dvbt_tuner->if_agc_min = 0x00;
	dvbt_tuner->tuner_crystal = 16;
	//dvbt_tuner->tuner_special_config = 0x01; // ?
	dvbt_tuner->wtuner_if_freq = 5000;
	//dvbt_tuner->tuner_ref_divratio = 64; // ?
	dvbt_tuner->tuner_agc_top = 252;
	//dvbt_tuner->tuner_step_freq = 62.5; // ?
	dvbt_tuner->i2c_type_id = I2C_TYPE_SCB2;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_S3821_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	/* Second NIM : DVB-S with external demod M3501 and M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner = &nim->config.dvbs.tuner;
	struct aui_demod_dvbs_config *dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO1;
	//dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->i2c_base_addr = 0x40;
	dvbs_tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 6;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 104;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO1;
	dvbs_demod->i2c_base_addr = 0x66;
	dvbs_demod->QPSK_Config = 0x79;
#else
	/* first NIM : ISDBT with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	//struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_tuner->freq_low = 40; // KHz
	dvbt_tuner->freq_high = 900; // KHz
	dvbt_tuner->id = AUI_MXL603;
	dvbt_tuner->agc_ref = 0x63;
	//dvbt_tuner->rf_agc_min = 0x2A; // ?
	//dvbt_tuner->rf_agc_max = 0xBA; // ?
	dvbt_tuner->if_agc_max = 0xC3;
	dvbt_tuner->if_agc_min = 0x00;
	dvbt_tuner->tuner_crystal = 16;
	//dvbt_tuner->tuner_special_config = 0x01; // ?
	dvbt_tuner->wtuner_if_freq = 5000;
	//dvbt_tuner->tuner_ref_divratio = 64; // ?
	dvbt_tuner->tuner_agc_top = 252;
	//dvbt_tuner->tuner_step_freq = 62.5; // ?
	dvbt_tuner->i2c_type_id = I2C_TYPE_SCB2;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_S3821_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	/* Second NIM : DVB-S with external demod M3501 and AV2012 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner = &nim->config.dvbs.tuner;
	struct aui_demod_dvbs_config *dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_GPIO1;
	dvbs_tuner->i2c_base_addr = 0xc2;
	dvbs_tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 6;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 104;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_GPIO1;
	dvbs_demod->i2c_base_addr = 0x66;
	dvbs_demod->QPSK_Config = 0x6d;
#endif

#elif (defined BOARD_CFG_M3735)
	/* first NIM : DVB-C with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0; //0xc2;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;//I2C_TYPE_SCB0;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1; //I2C_TYPE_SCB0;
	dvbc_demod->i2c_base_addr = 0xa0;//0x66;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* second NIM : DVB-C with internal demod and MxL214 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_1;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0xa0;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* thirdth NIM : DVB-C with internal demod and MxL214 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_2;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0xa0;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/* fourth NIM : DVB-C with internal demod and MxL214 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL214C ;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xa0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_MXL214C_3;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0xa0;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

#elif (defined BOARD_CFG_M3755)
	/*Add the 3755 nim configuration here*/
#elif (defined BOARD_CFG_M3527)

	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;

	/* first NIM : DVB-S with internal demod and ALi M3031 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB1;
	tuner->i2c_base_addr = 0x40;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_C3505_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	//demod->i2c_type_id =  I2C_TYPE_SCB0;
	//demod->i2c_base_addr = 0xe6;
	demod->QPSK_Config = 0xf9;

	/* second NIM : with external demod M3501 and ALi M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_GPIO0;
	tuner->i2c_base_addr = 0xc4;
	tuner->id = AUI_AV_2012;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 5;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 1;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_GPIO0;
	demod->i2c_base_addr = 0x66;
	demod->QPSK_Config = 0x7d;

#elif (defined BOARD_CFG_M3627)
	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;

	/* first NIM : DVB-S with internal demod and ALi M3031 tuner */
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB1;
	tuner->i2c_base_addr = 0x40;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_C3505_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;

	//demod->i2c_type_id =  I2C_TYPE_SCB0;
	//demod->i2c_base_addr = 0xe6;
	demod->QPSK_Config = 0xf9;

	/* second NIM : DVB-T2 with external demod and CXD2872 tuner */
	INC_INDEX(index);
	nim = nim_config + index;

	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_demod->i2c_base_addr = 0xD8;
	dvbt_demod->i2c_type_id = 3;

	dvbt_tuner->freq_low = 40; // KHz
	dvbt_tuner->freq_high = 900; // KHz
	dvbt_tuner->id = AUI_CXD2872;
	dvbt_tuner->agc_ref = 0x63;
	//dvbt_tuner->rf_agc_min = 0x2A; // ?
	//dvbt_tuner->rf_agc_max = 0xBA; // ?
	dvbt_tuner->if_agc_max = 0xC3;
	dvbt_tuner->if_agc_min = 0x00;
	dvbt_tuner->tuner_crystal = 16;
	//dvbt_tuner->tuner_special_config = 0x01; // ?
	dvbt_tuner->wtuner_if_freq = 5000;
	//dvbt_tuner->tuner_ref_divratio = 64; // ?
	dvbt_tuner->tuner_agc_top = 252;
	//dvbt_tuner->tuner_step_freq = 62.5; // ?
	dvbt_tuner->i2c_type_id = 3;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_CXD2837_0;

	nim->nim_reset_gpio.position = 95;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

#else
#error "Selected board is not supported by AUI"
#endif
	return 0;
}

int aui_tsi_config(struct aui_tsi_config *tsi_cfg)
{
	int index = 0;
	struct aui_tsi_config *cfg = tsi_cfg;

	/* TSI configuration for NIM 0 & 1 */
#if (defined BOARD_CFG_M3515)
#if (defined AUI_BOARD_VERSION_01V04)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x5b;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;
#else
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x5b;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;
#endif
#elif (defined BOARD_CFG_M3715B)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_SSI_ORDER |
		AUI_TSI_IN_CONF_SSI_CLK_POL | AUI_TSI_IN_CONF_VALID_SIG_POL | AUI_TSI_IN_CONF_SYNC_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

#elif  (defined BOARD_CFG_M3515B)

	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	/*
	  cfg[1].ul_hw_init_val =  // 0x9b does not work
	  AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_SSI_ORDER |
	  AUI_TSI_IN_CONF_SSI_CLK_POL | AUI_TSI_IN_CONF_VALID_SIG_POL |
	  AUI_TSI_IN_CONF_SYNC_SIG_POL;   */
	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x5b;

	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

#elif (defined BOARD_CFG_M3733)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x15f;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x2f;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x2f;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x2f;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;

#elif (defined BOARD_CFG_M3823)
	cfg->ul_hw_init_val = 0x83;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x5b;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;

#elif (defined BOARD_CFG_M3735)
	cfg->ul_hw_init_val = 0x27;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_3;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x27;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x27;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_0;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x27;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;
#elif (defined BOARD_CFG_M3527)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL|AUI_TSI_IN_CONF_SSI_CLK_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x9b;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;

#elif (defined BOARD_CFG_M3627)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE | AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL|AUI_TSI_IN_CONF_SSI_CLK_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x27;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;
#endif
	/* TSI configuration for TSG */
	tsi_cfg[AUI_TSG_DEV].ul_hw_init_val = AUI_TSI_IN_CONF_ENABLE |
		AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	tsi_cfg[AUI_TSG_DEV].ul_input_src = AUI_TSI_INPUT_TSG;

	return 0;
};
