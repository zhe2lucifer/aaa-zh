/**@file
 *	 @brief 	ALi AUI NIM TSI DMX test
 *	 @author	romain.baeriswyl
 *	 @date		2014-02-12
 *	 @version	1.0.0
 *	 @note		ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <aui_nim.h>
#include <aui_tsi.h>
#include <aui_tsg.h>

#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ali_soc_common.h>
#include <errno.h>

/* shared lib for now */
#include <alipltfretcode.h>
#include <alisldmx.h>
#include <alislvdec.h>
#include <alislsnd.h>
#include <alisldis.h>

/* Frequency band */
#define KU_BAND
//#define C_BAND

/* TSG parameters */
#define PKT_NUM 256
#define PKT_SIZE 188

static int dvbs_connect(int freq, int sym, int fec, int polar, int device,
			aui_hdl *aui_nim_hdl)
{
	aui_hdl nim_hdl = 0;
	struct aui_nim_attr nim_attr;
	struct aui_nim_connect_param param;

	MEMSET(&param, 0, sizeof(struct aui_nim_connect_param));

	nim_attr.ul_nim_id = device;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;

	if (aui_nim_open(&nim_attr, &nim_hdl) != AUI_RTN_SUCCESS) {
		printf("open error\n");
		return -EFAULT;
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

	if (aui_nim_connect(nim_hdl, &param) != AUI_RTN_SUCCESS) {
		printf("connect error\n");
		aui_nim_close(&nim_hdl);
		return -EFAULT;
	}

	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int cnt = 0;
	while (lock_status != AUI_NIM_STATUS_LOCKED) {
		if (aui_nim_is_lock(nim_hdl, &lock_status) != AUI_RTN_SUCCESS) {
			printf("is_lock error\n");
			aui_nim_disconnect(&nim_hdl);
			aui_nim_close(&nim_hdl);
			return -EFAULT;
		}
		usleep(10000);
		cnt++;
	}

	printf("channel locked on freq %d in %d ms\n", freq, cnt * 10);

	aui_signal_status info;
	if (aui_nim_signal_info_get(nim_hdl, &info) != AUI_RTN_SUCCESS) {
		printf("info_get error\n");
		aui_nim_disconnect(&nim_hdl);
		aui_nim_close(&nim_hdl);
		return -EFAULT;
	}
	printf("info: freq %d, strength %d, quality %d, ber %d, rf_level %d, signal_cn %d\n",
		   (int)info.ul_freq, (int)info.ul_signal_strength, (int)info.ul_signal_quality,
		   (int)info.ul_bit_error_rate, (int)info.ul_rf_level, (int)info.ul_signal_cn);

	*aui_nim_hdl = nim_hdl;
	return 0;
}

#include <sys_define.h>

/* from platform/inc/bus/i2c/i2c.h */
#define I2C_TYPE_SCB	0x00000000
#define I2C_TYPE_GPIO	0x00010000
#define I2C_TYPE_SCB_RM 0x00020000

#define I2C_TYPE_SCB0	(I2C_TYPE_SCB|0)
#define I2C_TYPE_SCB1	(I2C_TYPE_SCB|1)
#define I2C_TYPE_SCB2	(I2C_TYPE_SCB|2)

#define I2C_TYPE_GPIO0	(I2C_TYPE_GPIO|0)
#define I2C_TYPE_GPIO1	(I2C_TYPE_GPIO|1)
#define I2C_TYPE_GPIO2	(I2C_TYPE_GPIO|2)

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

#define HAL_GPIO_I_DIR 0
#define HAL_GPIO_O_DIR 1

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


static AUI_RTN_CODE aui_nim_driver_init_cb(struct aui_nim_config *nim_config)
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
			nim->demod.i2c_base_addr = 0xe6;
			nim->demod.QPSK_Config = /* 0xe9 */
				QPSK_CONFIG_FREQ_OFFSET |
				QPSK_CONFIG_I2C_THROUGH |
				QPSK_CONFIG_NEW_AGC1 |
				QPSK_CONFIG_MODE_8BIT;

			nim->tuner.i2c_type_id = I2C_TYPE_SCB0;
			nim->tuner.i2c_base_addr = 0xc0;
			nim->tuner.id = SHARP_VZ7306;
		} else {
			nim->id = AUI_NIM_ID_M3501_0;
			nim->nim_reset_gpio = NULL;
			nim->lnb_power_gpio = NULL;

			nim->demod.i2c_type_id =  I2C_TYPE_GPIO1;
			nim->demod.i2c_base_addr = 0x66;
			nim->demod.QPSK_Config = /* 0x69 */
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


static int soc_relay_func(int cmd)
{
	int ret;
	int fd = open("/dev/ali_soc", O_RDONLY);
	if (fd < 0)
		return -1;
	ret = ioctl(fd, cmd, 0);
	close(fd);
	return ret;
}

static AUI_RTN_CODE aui_tsg_initialize(aui_hdl *tsg_hdl, struct aui_attr_tsg *attr)
{
	/* does nothing, should it be removed ? */
	if (aui_tsg_init(NULL, NULL)) {
		printf("aui_tsg_init error\n");
		return 1;
	}

	/* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz */
	attr->ul_tsg_clk = 24; /* TSG clock = 4.16 MHz */
	if (aui_tsg_open(attr, tsg_hdl)) {
		printf("aui_tsg_init error\n");
		aui_tsg_de_init(NULL, NULL);
		return -1;
	}

	attr->ul_bit_rate = 100; /* default bitrates */
	if (aui_tsg_start(*tsg_hdl, attr)) {
		printf("aui_tsg_start error\n");
		aui_tsg_close(*tsg_hdl);
		aui_tsg_de_init(NULL, NULL);
		return 1;
	}
	return 0;
}


static AUI_RTN_CODE aui_tsi_init(int channel, aui_hdl *tsi_hdl)
{
	struct aui_attr_tsi attr;
	aui_hdl hdl;
	int input_id;

	int soc_rev_id = soc_relay_func(ALI_SOC_REV_ID);
	printf("soc_rev_id %d (0x%x)\n", (soc_rev_id & 0xffff) - 1, soc_rev_id);

	if (aui_tsi_open(&hdl)) {
		printf("aui_tsi_open error\n");
		return -1;
	}

	switch (channel) {
	case 0: /* configure TSI channel 0 : SPI to DMX 0 */
		input_id = (soc_rev_id == IC_REV_0) ?
			AUI_TSI_INPUT_SPI_0 : AUI_TSI_INPUT_SPI_3;
		attr.ul_init_param = AUI_TSI_IN_CONF_ENABLE |
			AUI_TSI_IN_CONF_VALID_SIG_POL |
			AUI_TSI_IN_CONF_SYNC_SIG_POL;
		break;
	case 1: /* configure TSI channel 0 : SSI2B to DMX 0 */
		input_id = AUI_TSI_INPUT_SSI2B_0;
		attr.ul_init_param = AUI_TSI_IN_CONF_ENABLE |
			AUI_TSI_IN_CONF_SSI_ORDER |
			AUI_TSI_IN_CONF_SSI_CLK_POL |
			AUI_TSI_IN_CONF_VALID_SIG_POL |
			AUI_TSI_IN_CONF_SYNC_SIG_POL;
		break;
	case 2:
		input_id = AUI_TSI_INPUT_TSG;
		attr.ul_init_param = AUI_TSI_IN_CONF_ENABLE |
			AUI_TSI_IN_CONF_VALID_SIG_POL |
			AUI_TSI_IN_CONF_SYNC_SIG_POL;
		break;
	default:
		break;
	}
	if (aui_tsi_src_init(hdl, input_id, &attr)) {
		printf("aui_tsi_src_init error\n");
		aui_tsi_close(hdl);
		return -1;
		}
	if (aui_tsi_route_cfg(hdl, input_id, AUI_TSI_CHANNEL_0,
				  AUI_TSI_OUTPUT_DMX_0)) {
		printf("aui_tsi_route_cfg error\n");
		aui_tsi_close(hdl);
		return -1;
	}
	*tsi_hdl = hdl;
	return 0;
}

int create_av_stream(int dmx_id,
			 unsigned int front_end,
			 unsigned int nim_chipid,
			 unsigned short video_pid,
			 unsigned short audio_pid,
			 unsigned short audio_desc_pid,
			 unsigned short pcr_pid,
			 alisl_handle *dmx_handle)
{
	struct dmx_channel_attr attr;
	alisl_handle handle;
	uint32_t channelid;

	if (alisldmx_open(&handle, dmx_id, 0)) {
		printf("dmx open fail\n");
		return -1;
	}

	if (alisldmx_start(handle)) {
		printf("dmx start fail\n");
		return -1;
	}

	MEMSET(&attr, 0, sizeof(attr));
	alisldmx_set_front_end(handle, front_end);
	alisldmx_set_nim_chipid(handle, nim_chipid);

	if (video_pid != 0x1FFF) {
		attr.stream = DMX_STREAM_VIDEO;
		alisldmx_allocate_channel(handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(handle, channelid, &attr);
		alisldmx_set_channel_pid(handle, channelid, video_pid);
		alisldmx_control_channel(handle, channelid, DMX_CTRL_ENABLE);
		printf("dmx video pid %d\n", video_pid);
	}

	if (audio_pid != 0x1FFF) {
		attr.stream = DMX_STREAM_AUDIO;
		alisldmx_allocate_channel(handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(handle, channelid, &attr);
		alisldmx_set_channel_pid(handle, channelid, audio_pid);
		alisldmx_control_channel(handle, channelid, DMX_CTRL_ENABLE);
		printf("dmx audio pid %d\n", audio_pid);
	}

	if (audio_desc_pid != 0x1FFF) {
		attr.stream = DMX_STREAM_AUDIO_DESCRIPTION;
		alisldmx_allocate_channel(handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(handle, channelid, &attr);
		alisldmx_set_channel_pid(handle, channelid, audio_desc_pid);
		alisldmx_control_channel(handle, channelid, DMX_CTRL_ENABLE);
		printf("dmx audio desc pid %d\n", audio_desc_pid);
	}

	if (video_pid != 0x1FFF &&
		audio_pid != 0x1FFF &&
		(pcr_pid & 0x1FFF) != (video_pid & 0x1FFF) &&
		(pcr_pid & 0x1FFF) != (audio_pid & 0x1FFF)) {
		attr.stream = DMX_STREAM_PCR;
		alisldmx_allocate_channel(handle, DMX_CHANNEL_STREAM, &channelid);
		alisldmx_set_channel_attr(handle, channelid, &attr);
		alisldmx_set_channel_pid(handle, channelid, pcr_pid);
		alisldmx_control_channel(handle, channelid, DMX_CTRL_ENABLE);

		printf("dmx pcr id %d\n", pcr_pid);
	}

	alisldmx_set_avsync_mode(handle, DMX_AVSYNC_LIVE);
	alisldmx_avstart(handle);

	*dmx_handle = handle;
	return 0;
}


int main(int argc, char *argv[])
{
	int opt;
	int option_index;
	int freq = 0;
	int symb = 0;
	int polar = 0;
	int dev = 0;
	char tsg_file[50];
	int tsg_enable = 0;

	aui_hdl nim_hdl;
	aui_hdl tsi_hdl;
	aui_hdl tsg_hdl;
	struct aui_attr_tsg tsg_attr;

	alisl_handle dmx_hdl = 0;
	alisl_handle snd_hdl = 0;
	alisl_handle dis_hdl = 0;
	alisl_handle vdec_hdl = 0;

	static struct option long_options[] = {
		{ "help",		 no_argument,		0, 'h' },
		{ "freq",		 required_argument, 0, 'f' },
		{ "symbol rate", required_argument, 0, 's' },
		{ "polar",		 required_argument, 0, 'p' },
		{ "device", 	 required_argument, 0, 'd' },
		{ "tsg file",	 required_argument, 0, 'g' },
		{ 0, 0, 0, 0}
	};

	while ((opt = getopt_long (argc, argv, "hcf:s:p:d:g:", long_options, &option_index)) != -1) {

		switch (opt) {
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
		case 'g':
			tsg_enable = 1;
			strcpy(tsg_file, optarg);
			break;
		case 'h':
			printf("Usage: %s -h help\n"
				   "playback with NIM: -f <frequency> -s <symbole rate> "
				   "-p <0:horizontal, 1:vertical> -d <device 0 or 1>\n"
				   "playback with TSG: -g <filename>\n", argv[0]);
			return 0;

		}
	}

	if (tsg_enable) {
		printf("tsg file %s\n", tsg_file);
		if (aui_tsg_initialize(&tsg_hdl, &tsg_attr) != AUI_RTN_SUCCESS) {
			printf("tsg init error\n");
			return -1;
		}
	} else {
		if (aui_nim_init((p_fun_cb)aui_nim_driver_init_cb) != AUI_RTN_SUCCESS) {
			printf("nim init error\n");
			return -1;
		}
		if (dvbs_connect(freq, symb, AUI_NIM_FEC_AUTO, polar, dev, &nim_hdl)) {
			printf("nim connect error\n");
			return -1;
		}
	}

	if (aui_tsi_init(tsg_enable ? 2 : dev, &tsi_hdl)) {
		printf("tsi init error\n");
		return -1;
	}

	printf("tsi channel 0 initialized with DMX 0\n");

	/* Configure DMX */
	if (create_av_stream(0 /* dmx id */,0 /* frontend id */,
				 0 /* nim id */, 0x1fff /* video pid */,
				 403 /* audio */, 0x1fff /* audio descr */,
				 0 /* pcr */, &dmx_hdl)) {
		printf("Configure dmx error\n");
		return -1;
	}

	if (alislsnd_open(&snd_hdl)) {
		printf("snd open fail\n");
		return -1;
	}
	if (alislsnd_start(snd_hdl)) {
		printf("snd start fail\n");
		return -1;
	}

	if (alisldis_open(DIS_HD_DEV, &dis_hdl)) {
		printf("Display creat fail\n");
		return -1;
	}

	if (alislvdec_open(&vdec_hdl)) {
		printf("vdec open fail\n");
		return -1;
	}
	if (alislvdec_start(vdec_hdl)) {
		printf("vdec start fail\n");
		return -1;
	}

	alislsnd_set_mute(snd_hdl,0,0);
	alislsnd_set_volume(snd_hdl,50,0);

	if (tsg_enable) {
		unsigned int buffer[PKT_NUM * PKT_SIZE / sizeof(int)];
		int len, ret;

		FILE *file;
		file = fopen(tsg_file, "r");
		if (!file) {
			printf("tsg file open errno %d\n", errno);
			goto err_tsg_send;
		}

		len = PKT_NUM * PKT_SIZE / sizeof(int);
		ret = fread(buffer, 1, len, file);
		if (ret < 0) {
			printf("tsg file open errno %d\n", errno);
			fclose(file);
			goto err_tsg_send;
		}
		if (ret != len)
			printf("file read %d instead of %d\n",
				   ret, len);
		/* DRAM buff must be 16 bytes align */
		tsg_attr.ul_addr = (unsigned char *)buffer;
		tsg_attr.ul_pkg_cnt = PKT_NUM;
		tsg_attr.uc_sync_mode = AUI_TSG_XFER_SYNC;
		ret = aui_tsg_send(tsg_hdl, &tsg_attr);
		if (ret) {
			printf("\naui_tsg_send error 0x%x\n", ret);
			fclose(file);
			goto err_tsg_send;
		}
	} else {
		printf("press enter to stop test\n");
		getchar();
	}

	/* closing */
err_tsg_send:
	alislvdec_stop(vdec_hdl, 1, 1);
	alislvdec_close(vdec_hdl);

	alisldis_close(dis_hdl);

	alislsnd_stop(snd_hdl);
	alislsnd_close(snd_hdl);

	alisldmx_stop(dmx_hdl);
	alisldmx_close(dmx_hdl);

	if (aui_tsi_close(tsi_hdl))
		printf("aui_tsi_close error\n");

	if (tsg_enable) {
		aui_tsg_stop(tsg_hdl, &tsg_attr);
		aui_tsg_close(tsg_hdl);
		aui_tsg_de_init(NULL, NULL);
	} else {
		if (aui_nim_close(nim_hdl))
			printf("aui_nim_close error\n");
		if (aui_nim_de_init(NULL))
			printf("aui_nim_de_init error\n");
	}
	return 0;
}
