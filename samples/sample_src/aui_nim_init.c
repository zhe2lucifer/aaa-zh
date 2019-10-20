
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
	//QPSK_Config: 0xfd (1111 1101)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0x7d (0111 1101)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0xe9 (1110 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0x69 (0110 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
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

	//QPSK_Config: 0xfd (1111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0x7d (0111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

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

	//QPSK_Config: 0xfd (1111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0x7d (0111 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;
#else
//#error "Selected board is not this version by AUI"
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
	//QPSK_Config: 0x79 (0111 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#define DVB_S_NIM_M3501_AV2012
//#define DVB_C_NIM_TDA10028_TDA18250

#if defined (DVB_S_NIM_M3501_AV2012)
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

	//QPSK_Config: 0x39 (0011 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_1BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0x39 (0011 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_1BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

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
	//QPSK_Config: 0x39 (0011 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_1BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#elif defined (DVB_C_NIM_TDA10028_TDA18250)
	/*                    DVB-C NIM TDA18250 + TDA10028                   */
	/***********************            TU1             *******************/
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_TDA18250;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBB;
	dvbc_tuner->if_agc_max = 0xFF;
	dvbc_tuner->if_agc_min = 0xB6;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 63;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_10025_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0x18;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/*                    DVB-C NIM TDA18250 + TDA10028                   */
	/***********************            TU2             *******************/
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_TDA18250;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBB;
	dvbc_tuner->if_agc_max = 0xFF;
	dvbc_tuner->if_agc_min = 0xB6;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 63;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_10025_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0x18;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

	/*                    DVB-C NIM TDA18250 + TDA10028                   */
	/***********************            TU3             *******************/
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_TDA18250;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->rf_agc_max = 0xBB;
	dvbc_tuner->if_agc_max = 0xFF;
	dvbc_tuner->if_agc_min = 0xB6;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 63;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1; //I2C_TYPE_SCB1 for old 3733
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5000;

	nim->id = AUI_NIM_ID_10025_0;

	nim->nim_reset_gpio.position = 77;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	dvbc_demod->i2c_type_id =  I2C_TYPE_SCB1;
	dvbc_demod->i2c_base_addr = 0x18;
	dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;
#endif

#elif (defined BOARD_CFG_M3755)

    // Untested: Linux AUI NIM settints
    /* first NIM : DVB-C with internal demod and MxL603 tuner */
    nim = nim_config + 0;
    struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
    //struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

    dvbc_tuner->id = AUI_MXL603;
    dvbc_tuner->rf_agc_max = 0xBA;
    dvbc_tuner->rf_agc_min = 0x2A;
    dvbc_tuner->if_agc_max = 0xFE;
    dvbc_tuner->if_agc_min = 0x01;
    dvbc_tuner->agc_ref = 0x80;

    dvbc_tuner->tuner_crystal = 16;
    dvbc_tuner->i2c_base_addr = 0xc0;
    dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
    dvbc_tuner->tuner_special_config = 0x01;
    dvbc_tuner->wtuner_if_freq = 5000;
    dvbc_tuner->tuner_ref_divratio = 64;
    dvbc_tuner->tuner_agc_top = 1;
    dvbc_tuner->tuner_step_freq = 62.5;
    dvbc_tuner->tuner_if_freq_J83A = 5000;
    dvbc_tuner->tuner_if_freq_J83B = 5380;
    dvbc_tuner->tuner_if_freq_J83C = 5380;

    nim->id = AUI_NIM_ID_M3281_0;

    nim->nim_reset_gpio.position = AUI_GPIO_NONE;
    nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
    nim->lnb_power_gpio.position = AUI_GPIO_NONE;
    nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

    //dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
    //dvbc_demod->i2c_base_addr = 0x66;
    //dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

    /* second NIM : DVB-C with internal demod and MxL603 tuner */
    INC_INDEX(index);
    nim = nim_config + index;
    dvbc_tuner = &nim->config.dvbc.tuner;
    //dvbc_demod = &nim->config.dvbc.demod;

    dvbc_tuner->id = AUI_MXL603;
    dvbc_tuner->rf_agc_max = 0xBA;
    dvbc_tuner->rf_agc_min = 0x2A;
    dvbc_tuner->if_agc_max = 0xFE;
    dvbc_tuner->if_agc_min = 0x01;
    dvbc_tuner->agc_ref = 0x80;

    dvbc_tuner->tuner_crystal = 16;
    dvbc_tuner->i2c_base_addr = 0xc0;
    dvbc_tuner->i2c_type_id = I2C_TYPE_SCB2;
    dvbc_tuner->tuner_special_config = 0x01;
    dvbc_tuner->wtuner_if_freq = 5000;
    dvbc_tuner->tuner_ref_divratio = 64;
    dvbc_tuner->tuner_agc_top = 1;
    dvbc_tuner->tuner_step_freq = 62.5;
    dvbc_tuner->tuner_if_freq_J83A = 5000;
    dvbc_tuner->tuner_if_freq_J83B = 5380;
    dvbc_tuner->tuner_if_freq_J83C = 5380;

    nim->id = AUI_NIM_ID_M3281_1;

    nim->nim_reset_gpio.position = AUI_GPIO_NONE;
    nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
    nim->lnb_power_gpio.position = AUI_GPIO_NONE;
    nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

    //dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
    //dvbc_demod->i2c_base_addr = 0x66;
    //dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

#ifdef CONFIG_ALI_EMULATOR        
//========================================================================//
	INC_INDEX(index);
	nim = nim_config + index;
	struct aui_tuner_dvbs_config *dvbs_tuner;
	struct aui_demod_dvbs_config *dvbs_demod;
	dvbs_tuner = &nim->config.dvbs.tuner;
	dvbs_demod = &nim->config.dvbs.demod;

	dvbs_tuner->freq_low = 900;
	dvbs_tuner->freq_high = 2200;
	dvbs_tuner->i2c_type_id = I2C_TYPE_SCB3;
	dvbs_tuner->i2c_base_addr = 0x40;
	dvbs_tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_0;

	nim->nim_reset_gpio.position = 92;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 6;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	dvbs_demod->i2c_type_id =  I2C_TYPE_SCB3;
	dvbs_demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
#endif    
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
	//QPSK_Config: 0x79 (0111 1001)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
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
	//QPSK_Config: 0x6d (0110 1101)
	dvbs_demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_IQ_SWAP
			| QPSK_CONFIG_FREQ_OFFSET;
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

#elif (defined BOARD_CFG_M3527)

	struct aui_tuner_dvbs_config *tuner;
	struct aui_demod_dvbs_config *demod;
#if 0
	    /* first NIM : DVB-S with internal demod and ALi RDA5815M tuner */
	    nim = nim_config + index;
	    tuner = &nim->config.dvbs.tuner;
	    demod = &nim->config.dvbs.demod;

	    tuner->freq_low = 900;
	    tuner->freq_high = 2200;
	    tuner->i2c_type_id = I2C_TYPE_SCB1;
	    tuner->i2c_base_addr = 0x18;
	    tuner->id = AUI_RDA5815M;

	    nim->id = AUI_NIM_ID_C3505_0;

	    // Internal demodulator does not need reset GPIO
	    nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	//  nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	//  nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	    nim->lnb_power_gpio.position = 71;
	    nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	    nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	    //demod->i2c_type_id =  I2C_TYPE_SCB0;
	    //demod->i2c_base_addr = 0xe6;
	    //QPSK_Config: 0xfd (1111 1101)
	    demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
	            | QPSK_CONFIG_NEW_AGC1
	            | QPSK_CONFIG_POLAR_REVERT
	            | QPSK_CONFIG_I2C_THROUGH
	            | QPSK_CONFIG_IQ_SWAP
	            | QPSK_CONFIG_FREQ_OFFSET;
#endif

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

	// Internal demodulator does not need reset GPIO
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
//	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
//	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 71;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	//demod->i2c_type_id =  I2C_TYPE_SCB0;
	//demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xf9 (1111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : with external demod M3501 and ALi M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB3;
	tuner->i2c_base_addr = 0x42;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 95;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 71;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB3;
	demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;
	
#elif (defined BOARD_CFG_M3528)
	
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

    tuner->diseqc_polar_gpio.gpio_val = 1;
    tuner->diseqc_polar_gpio.position = 96;
    tuner->diseqc_polar_gpio.io = 1;

	nim->id = AUI_NIM_ID_C3505_0;

	// Internal demodulator does not need reset GPIO
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
//	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
//	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
#if (defined AUI_BOARD_VERSION_01V01)
    nim->lnb_power_gpio.position = 93;
#else
	nim->lnb_power_gpio.position = 92;
#endif    
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xf9 (1111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : with external demod M3501 and ALi M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB1;
	tuner->i2c_base_addr = 0x42;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 86;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

#if (defined AUI_BOARD_VERSION_01V01)
    nim->lnb_power_gpio.position = 93;
#else
	nim->lnb_power_gpio.position = 92;
#endif    
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB1;
	demod->i2c_base_addr = 0xA6;
	//QPSK_Config: 0x79 (0111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#elif (defined BOARD_CFG_M3728)
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_tuner->id = AUI_R858;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->tuner_crystal = 24;
	dvbc_tuner->i2c_base_addr = 0xD4;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;
	nim->id = AUI_NIM_ID_M3281_0;
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	
	/* second NIM : DVB-T with external demod and MxL603 tuner */
    INC_INDEX(index);
	nim = nim_config + index;

	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_demod->i2c_base_addr = 0xD8;
	dvbt_demod->i2c_type_id = I2C_TYPE_SCB3;

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
	dvbt_tuner->i2c_type_id = I2C_TYPE_SCB3;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_CXD2856_0;

	nim->nim_reset_gpio.position = 86;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	//nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	//nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;


#elif (defined BOARD_CFG_M3529)

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

	// Internal demodulator does not need reset GPIO
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
//	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
//	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 6;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	//demod->i2c_type_id =  I2C_TYPE_SCB0;
	//demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xf9 (1111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : with external demod M3501 and ALi M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB3;
	tuner->i2c_base_addr = 0x40;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_M3501_1;

	nim->nim_reset_gpio.position = 92;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 6;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB3;
	demod->i2c_base_addr = 0x66;
	//QPSK_Config: 0x79 (0111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* third NIM : with external demod M3501 and ALi M3031 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	tuner = &nim->config.dvbs.tuner;
	demod = &nim->config.dvbs.demod;

	tuner->freq_low = 900;
	tuner->freq_high = 2200;
	tuner->i2c_type_id = I2C_TYPE_SCB3;
	tuner->i2c_base_addr = 0x40;
	tuner->id = AUI_M3031;

	nim->id = AUI_NIM_ID_C3501H_0;

	nim->nim_reset_gpio.position = 92;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 6;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	demod->i2c_type_id =  I2C_TYPE_SCB3;
	demod->i2c_base_addr = 0x24;
	//QPSK_Config: 0x79 (0111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_2BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

#elif (defined BOARD_CFG_M3627)
#ifdef SUPPORT_TWO_TUNER
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

	// Internal demodulator does not need reset GPIO
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
//	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
//	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	nim->lnb_power_gpio.position = 71;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_HIGH;

	//demod->i2c_type_id =  I2C_TYPE_SCB0;
	//demod->i2c_base_addr = 0xe6;
	//QPSK_Config: 0xf9 (1111 1001)
	demod->QPSK_Config = QPSK_CONFIG_MODE_8BIT
			| QPSK_CONFIG_NEW_AGC1
			| QPSK_CONFIG_POLAR_REVERT
			| QPSK_CONFIG_I2C_THROUGH
			| QPSK_CONFIG_FREQ_OFFSET;

	/* second NIM : DVB-T2 with external demod and CXD2872 tuner */
	INC_INDEX(index);
#endif	
	nim = nim_config + index;

	struct aui_tuner_dvbt_config *dvbt_tuner = &nim->config.dvbt.tuner;
	struct aui_demod_dvbt_config *dvbt_demod = &nim->config.dvbt.demod;

	dvbt_demod->i2c_base_addr = 0xD8;
	dvbt_demod->i2c_type_id = I2C_TYPE_SCB3;

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
	dvbt_tuner->i2c_type_id = I2C_TYPE_SCB3;
	dvbt_tuner->i2c_base_addr = 0xC0;
	dvbt_tuner->chip = tuner_chip_maxlinear;

	nim->id = AUI_NIM_ID_CXD2837_0;

	nim->nim_reset_gpio.position = 95;
	nim->nim_reset_gpio.io = AUI_GPIO_O_DIR;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = 96;
	nim->lnb_power_gpio.io = AUI_GPIO_O_DIR;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

#elif (defined BOARD_CFG_M3626)

#elif (defined BOARD_CFG_M3727) || (defined BOARD_CFG_M3712)|| (defined BOARD_CFG_M3712L)
	// Untested: Linux AUI NIM settints
	/* first NIM : DVB-C with internal demod and MxL603 tuner */
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	//struct aui_demod_dvbc_config *dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_MXL603;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;

	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xc0;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_M3281_0;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	//dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
	//dvbc_demod->i2c_base_addr = 0x66;
	//dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

//#define DVB_C_NIM_R836
	/* second NIM : DVB-C with internal demod and MxL603 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	//dvbc_demod = &nim->config.dvbc.demod;

#if defined(DVB_C_NIM_R836)
    dvbc_tuner->id = AUI_R836;
#else
    dvbc_tuner->id = AUI_MXL603;
#endif
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;

	dvbc_tuner->tuner_crystal = 16;
#if defined(DVB_C_NIM_R836)
    dvbc_tuner->i2c_base_addr = 0x34;
#else
    dvbc_tuner->i2c_base_addr = 0xc0;
#endif
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB3;    
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_M3281_1;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

	//dvbc_demod->i2c_type_id =  I2C_TYPE_SCB0;
	//dvbc_demod->i2c_base_addr = 0x66;
	//dvbc_demod->qam_mode = AUI_NIM_DVBC_MODE_J83AC | AUI_NIM_DEMO_SAMPLE_CLK_27M;

#elif (defined BOARD_CFG_M3716L)
	nim = nim_config + 0;
	struct aui_tuner_dvbc_config *dvbc_tuner = &nim->config.dvbc.tuner;
	dvbc_tuner->id = AUI_R858;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;
	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xD4;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB1;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;
	nim->id = AUI_NIM_ID_M3281_0;
	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	/* second NIM : DVB-C with internal demod and R858 tuner */
	INC_INDEX(index);
	nim = nim_config + index;
	dvbc_tuner = &nim->config.dvbc.tuner;
	//dvbc_demod = &nim->config.dvbc.demod;

	dvbc_tuner->id = AUI_R858;
	dvbc_tuner->rf_agc_max = 0xBA;
	dvbc_tuner->rf_agc_min = 0x2A;
	dvbc_tuner->if_agc_max = 0xFE;
	dvbc_tuner->if_agc_min = 0x01;
	dvbc_tuner->agc_ref = 0x80;

	dvbc_tuner->tuner_crystal = 16;
	dvbc_tuner->i2c_base_addr = 0xf4;
	dvbc_tuner->i2c_type_id = I2C_TYPE_SCB3;
	dvbc_tuner->tuner_special_config = 0x01;
	dvbc_tuner->wtuner_if_freq = 5000;
	dvbc_tuner->tuner_ref_divratio = 64;
	dvbc_tuner->tuner_agc_top = 1;
	dvbc_tuner->tuner_step_freq = 62.5;
	dvbc_tuner->tuner_if_freq_J83A = 5000;
	dvbc_tuner->tuner_if_freq_J83B = 5380;
	dvbc_tuner->tuner_if_freq_J83C = 5380;

	nim->id = AUI_NIM_ID_M3281_1;

	nim->nim_reset_gpio.position = AUI_GPIO_NONE;
	nim->nim_reset_gpio.gpio_val = AUI_GPIO_VALUE_LOW;
	nim->lnb_power_gpio.position = AUI_GPIO_NONE;
	nim->lnb_power_gpio.gpio_val = AUI_GPIO_VALUE_LOW;

#else
//#error "Selected board is not supported by AUI"
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
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;
#else
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;
#endif
#elif (defined BOARD_CFG_M3715B)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x9b (1001 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

#elif  (defined BOARD_CFG_M3515B)

	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	/*
	  cfg[1].ul_hw_init_val =  // 0x9b does not work
	  AUI_TSI_IN_CONF_SPI_ENABLE
		| AUI_TSI_IN_CONF_SSI_CLOCK_POL
		| AUI_TSI_IN_CONF_SSI_BIT_ORDER
		| AUI_TSI_IN_CONF_SYNC_SIG_POL
		| AUI_TSI_IN_CONF_VALID_SIG_POL;
	*/
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;

	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

#elif (defined BOARD_CFG_M3733)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = 0x15f;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;
#elif (defined BOARD_CFG_M3755)
    //ul_hw_init_val: 0x8b (1000 1011)
    cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
            | AUI_TSI_IN_CONF_SSI_BIT_ORDER
            | AUI_TSI_IN_CONF_SYNC_SIG_POL
            | AUI_TSI_IN_CONF_VALID_SIG_POL;
    cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

    INC_INDEX(index); cfg++;
    //ul_hw_init_val: 0x8b (1000 1011)
    cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
            | AUI_TSI_IN_CONF_SSI_BIT_ORDER
            | AUI_TSI_IN_CONF_SYNC_SIG_POL
            | AUI_TSI_IN_CONF_VALID_SIG_POL;
    cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

#ifdef CONFIG_ALI_EMULATOR    
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	#if 1
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;
	#else
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;
	#endif
#endif    
	
#elif (defined BOARD_CFG_M3823)
	//ul_hw_init_val: 0x83 (1000 0011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;

#elif (defined BOARD_CFG_M3735)
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;
#elif (defined BOARD_CFG_M3527)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_1;
#elif (defined BOARD_CFG_M3528)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5f (0101 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;
#elif (defined BOARD_CFG_M3728)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
	               | AUI_TSI_IN_CONF_SSI_BIT_ORDER
	               | AUI_TSI_IN_CONF_SYNC_SIG_POL
	               | AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x2f (0010 1111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	//| AUI_TSI_IN_CONF_SSI_BIT_ORDER
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;
#elif (defined BOARD_CFG_M3529)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x5b (0101 1011)

	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_2;

	INC_INDEX(index); cfg++;
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI2B_ENABLE
			| AUI_TSI_IN_CONF_SSI_CLOCK_POL
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI2B_3;

#elif (defined BOARD_CFG_M3626)
    cfg->ul_hw_init_val = 0x27;
    cfg->ul_input_src = AUI_TSI_INPUT_SSI_2;
#elif (defined BOARD_CFG_M3627)
#ifdef SUPPORT_TWO_TUNER
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;

	INC_INDEX(index); cfg++;
#endif
	index = index;  //avoid compilation errors
	//ul_hw_init_val: 0x27 (0010 0111)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SSI_ENABLE
			| AUI_TSI_IN_CONF_ERR_SIG_POL
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SSI_1;
#elif (defined BOARD_CFG_M3727)||(defined BOARD_CFG_M3712)||(defined BOARD_CFG_M3712L)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;

	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SSI_BIT_ORDER
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;
#elif (defined BOARD_CFG_M3716L)
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
	               | AUI_TSI_IN_CONF_SSI_BIT_ORDER
	               | AUI_TSI_IN_CONF_SYNC_SIG_POL
	               | AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_0;
	INC_INDEX(index); cfg++;
	//ul_hw_init_val: 0x8b (1000 1011)
	cfg->ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
	               | AUI_TSI_IN_CONF_SSI_BIT_ORDER
	               | AUI_TSI_IN_CONF_SYNC_SIG_POL
	               | AUI_TSI_IN_CONF_VALID_SIG_POL;
	cfg->ul_input_src = AUI_TSI_INPUT_SPI_3;
#endif
	/* TSI configuration for TSG */
	//ul_hw_init_val: 0x83 (1000 0011)
	tsi_cfg[AUI_TSG_DEV].ul_hw_init_val = AUI_TSI_IN_CONF_SPI_ENABLE
			| AUI_TSI_IN_CONF_SYNC_SIG_POL
			| AUI_TSI_IN_CONF_VALID_SIG_POL;
	tsi_cfg[AUI_TSG_DEV].ul_input_src = AUI_TSI_INPUT_TSG;

	return 0;
};
