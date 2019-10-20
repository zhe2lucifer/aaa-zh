/**@file
 *   @brief     ALi AUI NIM test
 *   @author    romain.baeriswyl
 *   @date      2014-02-12
 *   @version   1.0.0
 *   @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */

#include <aui_nim.h>
#include <aui_tsi.h>
#include <string.h>
#include <stdio.h>
#ifndef TEST_APP_CMD
#include <getopt.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#ifndef AUI_TDS
#include <pthread.h>
#include <sys/time.h>
#endif

#include "aui_nim_tests.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_app_cmd.h"

// C Band LNB L.O. frequency
#define LO_5150 (5150)
#define LO_5750 (5750)
// Ku Band LNB L.O. frequency
#define LO_9750 (9750)
#define LO_10600 (10600)

// C Band Vertical signal beginning I.F.
#define FREQ_1550 (1550)

//cctv2
#define FREQ 3840
#define SYMB 27500


#define DVBC_FREQ 1000
#define DVBC_SYMB 6750
#define DVBC_MOD AUI_NIM_QAM64

#define CONNECT_TIMEOUT_MS 1000

static int as_polar;
static int as_c_band;
static int as_high_band;
static int tp_cnt;
static int tp_hor_cnt;
static int tp_ver_cnt;
static int dvbc_connect(int device, int freq, int symb, int mod)
{
	aui_hdl hdl = 0;
	struct aui_nim_attr nim_attr;
	struct aui_nim_connect_param param;
	int ret = 0;
	int connect_time = 0;
	int lock_time = 0;
#ifndef AUI_TDS
    struct timeval begin_timeval = {0, 0};
    struct timeval end_timeval = {0, 0};
    struct timeval all_timeval = {0, 0};
#endif
#ifndef AUI_LINUX
    unsigned long begin_time = 0;
    unsigned long end_time = 0;
#endif
    
    memset(&nim_attr, 0, sizeof(struct aui_nim_attr));
	memset(&param, 0, sizeof(struct aui_nim_connect_param));

	nim_attr.ul_nim_id = device;
	nim_attr.en_dmod_type = AUI_NIM_QAM;
#ifndef AUI_TDS
	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}
#else
	if (aui_nim_handle_get_byid(device, &hdl))
	{
		AUI_PRINTF("aui_nim_handle_get_byid error\n");
		return -1;
	}
#endif

	param.ul_freq = freq;
	param.en_demod_type = AUI_NIM_QAM;
	param.connect_param.cab.ul_symrate = symb;
	param.connect_param.cab.en_qam_mode = mod;
	AUI_PRINTF("nim connect dev %d freq %d, symb %d, mod %d\n",
	       (int)nim_attr.ul_nim_id, freq, symb, mod);

#ifndef AUI_TDS
    gettimeofday(&begin_timeval, NULL);
#endif
#ifndef AUI_LINUX
    begin_time = osal_get_tick();
#endif

	if (aui_nim_connect(hdl, &param)) {
		AUI_PRINTF("connect error\n");
#ifndef AUI_TDS
		aui_nim_close(&hdl);
#endif
		return -1;
	}

#ifndef AUI_TDS
    gettimeofday(&end_timeval, NULL);
    timersub(&end_timeval, &begin_timeval, &all_timeval);
    connect_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
    end_time = osal_get_tick();
    connect_time = end_time - begin_time;
#endif

	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int timeout = CONNECT_TIMEOUT_MS / 10;
	while (lock_status != AUI_NIM_STATUS_LOCKED && timeout) {
		if ((ret = aui_nim_is_lock(hdl, &lock_status))) {
			AUI_PRINTF("is_lock error\n");
			goto dvbc_err_connect;
		}
		AUI_SLEEP(10); // wait 10MS
		timeout--;
	}

	if (lock_status != AUI_NIM_STATUS_LOCKED) {
		AUI_PRINTF("channel is not locked\n");
		ret = -1;
		goto dvbc_err_connect;
	}

#ifndef AUI_TDS
    gettimeofday(&end_timeval, NULL);
    timersub(&end_timeval, &begin_timeval, &all_timeval);
    lock_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
    end_time = osal_get_tick();
    lock_time = end_time - begin_time;
#endif

    AUI_PRINTF("aui_nim_connect takes %d MS\n", connect_time);
    AUI_PRINTF("Locking takes %d MS\n", lock_time);

    (void)lock_time;//just for build.
    (void)connect_time;//just for build.

    // Driver need at lease 50MS to prepare the signal info after locked
    AUI_SLEEP(100); // wait 100MS

	aui_signal_status info;
	int i = 0;
	for (i = 0; i < 10; i++) {
		if ((ret = aui_nim_signal_info_get(hdl, &info))) {
			AUI_PRINTF("info_get error\n");
			goto dvbc_err_connect;
		}

		AUI_PRINTF("info: freq %ld, strength %d, quality %d, ber %d, rf_level %d,"
				"signal_cn %d\n", info.ul_freq, (int)info.ul_signal_strength,
				(int)info.ul_signal_quality, (int)info.ul_bit_error_rate,
				(int)info.ul_rf_level, (int)info.ul_signal_cn);
		AUI_SLEEP(500); // 500MS
	}

dvbc_err_connect:
	aui_nim_disconnect(hdl);
#ifndef AUI_TDS
	aui_nim_close(hdl);
#endif
	return ret;
}

static int dvbs_connect(int freq, int symb, int fec, int polar, int src, int device, int stream_id)
{
	aui_hdl hdl = 0;
	struct aui_nim_attr nim_attr;
	struct aui_nim_connect_param param;
	int ret = 0;
	int connect_time = 0;
    int lock_time = 0;
#ifndef AUI_TDS
    struct timeval begin_timeval = {0, 0};
    struct timeval end_timeval = {0, 0};
    struct timeval all_timeval = {0, 0};
#endif
#ifndef AUI_LINUX
    unsigned long begin_time = 0;
    unsigned long end_time = 0;
#endif

	if ((freq < 3400 || freq >4200) && (freq < 10700 || freq > 12750)) {
		AUI_PRINTF("Input frequency error.\n");
		AUI_PRINTF("C-BAND: 3400MHz ~ 4200MHz Ku-Band: 10700MHz ~ 12750MHz\n");
		return -1;
	}
	memset(&param, 0, sizeof(struct aui_nim_connect_param));

	nim_attr.ul_nim_id = device;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;
#ifndef AUI_TDS
	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}
#else
	if (aui_nim_handle_get_byid(device, &hdl))
	{
		AUI_PRINTF("aui_nim_handle_get_byid error\n");
		return -1;
	}
#endif
	// All C-band LNB have a local oscillator (L.O.) frequency of 5.150 GHz
	// but Ku-band LNB may come in many different frequencies
	// typically 9.750 to 12.75 GHz.

	// C-band dual polarity LNB (One-Cable-Solution)
	// Different polarizations use different  L.O Frequency, So users can receive
	// different polarizations signal at same time.
	// Polarization  Frequency band  L.O Frequency  I.F.
	// Horizontal    3.7-4.2 GHz     5.15 GHz       950-1,450 MHz
	// Vertical      3.7-4.2 GHz     5.75 GHz       1,550-2,050 MHz

	// Ku-band LNB
	// Europe Universal LNB ("Astra" LNB)
	// Voltage  Tone    Polarization  Frequency        band  L.O Frequency  I.F.
	// 13 V     0 kHz   Vertical      10.70-11.70 GHz  low   9.75 GHz       950-1,950 MHz
	// 18 V     0 kHz   Horizontal    10.70-11.70 GHz  low   9.75 GHz       950-1,950 MHz
	// 13 V     22 kHz  Vertical      11.70-12.75 GHz  high  10.60 GHz      1,100-2,150 MHz
	// 18 V     22 kHz  Horizontal    11.70-12.75 GHz  high  10.60 GHz      1,100-2,150 MHz
	if (freq < LO_5150) {
		as_c_band = 1;
		if (polar) {
			param.ul_freq = LO_5750 - freq;
		} else {
			param.ul_freq = LO_5150 - freq;
		}
		as_high_band = AUI_NIM_LOW_BAND;
	} else {
		as_c_band = 0;
		if (freq > 11700) {
			as_high_band = AUI_NIM_HIGH_BAND;
			param.ul_freq = freq - LO_10600;
		} else {
			as_high_band = AUI_NIM_LOW_BAND;
			param.ul_freq = freq - LO_9750;
		}
	}

	param.connect_param.sat.ul_freq_band = as_high_band;
	param.connect_param.sat.ul_symrate = symb;
	param.connect_param.sat.fec = fec;
	param.connect_param.sat.ul_polar = polar;
	param.connect_param.sat.ul_src = src;
	param.connect_param.sat.ul_stream_id = stream_id;
	param.en_demod_type = AUI_NIM_QPSK;

	/* check aui_find_dev_by_idx() */
	aui_hdl p;
	if (aui_find_dev_by_idx(AUI_MODULE_NIM, device, &p))
		AUI_PRINTF("aui_find_dev_by_idx fault\n");

	if (p != hdl)
		AUI_PRINTF("aui_find_dev_by_idx return wrong pointer\n");

	AUI_PRINTF("nim connect dev %ld src %d, polar %d, IF %d (freq %d)(band %d),"
			" symb %d\n",
			nim_attr.ul_nim_id, src, polar, param.ul_freq, freq, as_high_band,
			symb);

#ifndef AUI_TDS
	gettimeofday(&begin_timeval, NULL);
#endif
#ifndef AUI_LINUX
	begin_time = osal_get_tick();
#endif

	if (aui_nim_connect(hdl, &param)) {
		AUI_PRINTF("connect error\n");
#ifndef AUI_TDS
		aui_nim_close(hdl);
#endif
		return -1;
	}

#ifndef AUI_TDS
    gettimeofday(&end_timeval, NULL);
    timersub(&end_timeval, &begin_timeval, &all_timeval);
    connect_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
    end_time = osal_get_tick();
    connect_time = end_time - begin_time;
#endif

	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int timeout = CONNECT_TIMEOUT_MS / 10;
	while (lock_status != AUI_NIM_STATUS_LOCKED && timeout) {
		if ((ret = aui_nim_is_lock(hdl, &lock_status))) {
			AUI_PRINTF("is_lock error\n");
			goto err_connect;
		}
		AUI_SLEEP(10); // wait 10MS
		timeout--;
	}

	if (lock_status != AUI_NIM_STATUS_LOCKED) {
		AUI_PRINTF("channel is not locked\n");
		ret = -1;
		goto err_connect;
	}

#ifndef AUI_TDS
	gettimeofday(&end_timeval, NULL);
	timersub(&end_timeval, &begin_timeval, &all_timeval);
	lock_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
	end_time = osal_get_tick();
	lock_time = end_time - begin_time;
#endif

    AUI_PRINTF("aui_nim_connect takes %d MS\n", connect_time);
    AUI_PRINTF("Locking takes %d MS\n", lock_time);
    (void)lock_time;//just for build.
    (void)connect_time;//just for build.

	// Driver need at lease 50MS to prepare the signal info after locked
	AUI_SLEEP(100); // wait 100MS

	int freqency = 0;
	aui_signal_status info;
	int i = 0;
	if (as_c_band == 1) {
		if (polar)
			freqency = LO_5750 - info.ul_freq;
		else
			freqency = LO_5150 - info.ul_freq;
	} else {
		if (as_high_band)
			freqency = info.ul_freq + LO_10600;
		else
			freqency = info.ul_freq + LO_9750;
	}

	for (i = 0; i < 10; i++) {
		if ((ret = aui_nim_signal_info_get(hdl, &info))) {
			AUI_PRINTF("info_get error\n");
			goto err_connect;
		}

		AUI_PRINTF("info: IF %ld (freq %d ), strength %d, quality %d, ber %d, "
				"rf_level %d, signal_cn %d\n",
				info.ul_freq, freqency, (int)info.ul_signal_strength,
				(int)info.ul_signal_quality, (int)info.ul_bit_error_rate,
				(int)info.ul_rf_level, (int)info.ul_signal_cn);
		AUI_SLEEP(500); // 500MS
	}
	(void)freqency;//just for build.

err_connect:
	aui_nim_disconnect(hdl);
#ifndef AUI_TDS
	aui_nim_close(hdl);
#endif
	return ret;
}

static int dvbt_connect(int device, unsigned int std, int freq, int band)
{
	aui_hdl hdl = 0;
	struct aui_nim_attr nim_attr;
	struct aui_nim_connect_param param;
	int ret = 0;
    int connect_time = 0;
    int lock_time = 0;
#ifndef AUI_TDS
    struct timeval begin_timeval = {0, 0};
    struct timeval end_timeval = {0, 0};
    struct timeval all_timeval = {0, 0};
#endif
#ifndef AUI_LINUX
    unsigned long begin_time = 0;
    unsigned long end_time = 0;
#endif

	memset(&param, 0, sizeof(struct aui_nim_connect_param));
	memset(&nim_attr, 0, sizeof(struct aui_nim_attr));

	nim_attr.ul_nim_id = device;
	nim_attr.en_dmod_type = AUI_NIM_OFDM;
	nim_attr.en_ter_std = std;
#ifndef AUI_TDS
	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}
#else
	if (aui_nim_handle_get_byid(device, &hdl))
	{
		AUI_PRINTF("aui_nim_handle_get_byid error\n");
		return -1;
	}
#endif

	param.ul_freq = freq;
	param.en_demod_type = AUI_NIM_OFDM;
	switch(band)
	{
	case 6: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; break; //added by vedic.fu
	case 7: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_7_MHZ; break;
	case 8: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_8_MHZ; break;
	default : AUI_PRINTF("wrong bandwidth %d\n", band); return -1;
	}
	param.connect_param.ter.std = std;
	param.connect_param.ter.fec = 0;
	param.connect_param.ter.spectrum = 0;
	param.connect_param.ter.u.dvbt2.plp_index = -1; // -1 ->auto
	AUI_PRINTF("nim connect dev %d freq %d, band %d\n",
	       (int)nim_attr.ul_nim_id, freq, band);

#ifndef AUI_TDS
	gettimeofday(&begin_timeval, NULL);
#endif
#ifndef AUI_LINUX
	begin_time = osal_get_tick();
#endif

	if (aui_nim_connect(hdl, &param)) {
		AUI_PRINTF("connect error\n");
#ifndef AUI_TDS
		aui_nim_close(hdl);
#endif
		return -1;
	}

#ifndef AUI_TDS
	gettimeofday(&end_timeval, NULL);
	timersub(&end_timeval, &begin_timeval, &all_timeval);
	connect_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
	end_time = osal_get_tick();
	connect_time = end_time - begin_time;
#endif

	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int timeout = CONNECT_TIMEOUT_MS / 10;
	while (lock_status != AUI_NIM_STATUS_LOCKED && timeout) {
		if ((ret = aui_nim_is_lock(hdl, &lock_status))) {
			AUI_PRINTF("is_lock error\n");
			goto dvbt_err_connect;
		}
		AUI_SLEEP(10); // wait 10MS
	}

	if (lock_status != AUI_NIM_STATUS_LOCKED) {
		AUI_PRINTF("channel is not locked\n");
		ret = -1;
		goto dvbt_err_connect;
	}

#ifndef AUI_TDS
	gettimeofday(&end_timeval, NULL);
	timersub(&end_timeval, &begin_timeval, &all_timeval);
	lock_time = all_timeval.tv_sec * 1000 + all_timeval.tv_usec / 1000;
#endif
#ifndef AUI_LINUX
	end_time = osal_get_tick();
	lock_time = end_time - begin_time;
#endif

	AUI_PRINTF("aui_nim_connect takes %d MS\n", connect_time);
	AUI_PRINTF("Locking takes %d MS\n", lock_time);
        (void)lock_time;//just for build.
        (void)connect_time;//just for build.
    // Driver need at lease 50MS to prepare the signal info after locked
    AUI_SLEEP(100); // wait 100MS

	aui_signal_status info;
	int i = 0;
	for (i = 0; i < 10; i++) {
		if ((ret = aui_nim_signal_info_get(hdl, &info))) {
			AUI_PRINTF("info_get error\n");
			goto dvbt_err_connect;
		}
		AUI_PRINTF("info: freq %ld, strength %d, quality %d, ber %d, rf_level %d,"
				"signal_cn %d\n", info.ul_freq, (int)info.ul_signal_strength,
				(int)info.ul_signal_quality, (int)info.ul_bit_error_rate,
				(int)info.ul_rf_level, (int)info.ul_signal_cn);
		AUI_SLEEP(500); // 500MS
	}

dvbt_err_connect:
	aui_nim_disconnect(hdl);
#ifndef AUI_TDS
	aui_nim_close(hdl);
#endif
	return ret;
}

static int fn_aui_ascallback(unsigned char status,
		unsigned char polar, unsigned int freq, unsigned int sym,
		unsigned char fec, void *pv_user_data, unsigned int stream_id)
{
    int frequency = freq;
    (void)frequency;
	if (!as_c_band) {
		polar = as_polar;
		// Assume using Europe Universal LNB
		if (as_high_band)
			freq += LO_10600;
		else
			freq += LO_9750;
	} else {
		// Assume using ONE CABLE SOLUTION c band LNB
		if (freq < FREQ_1550) {
			polar = 0;
			freq = LO_5150 - freq;
		} else{
			polar = 1;
			freq = LO_5750 - freq;
		}
	}
	
	if (status) {
		tp_cnt++;
		if (polar) {
		    tp_ver_cnt++;
		} else {
		    tp_hor_cnt++;
		}
	}
	AUI_PRINTF("[NIM %d] lock = %d, polar = %s, IF = %4d(%4d), sym = %5d, fec = %d, stream_id:%d\n",
		pv_user_data, status, polar ? "V": "H", frequency, freq, sym, fec, stream_id);

	return 0;
}

/*
 * When using external demodulator, TSI must be configured
 */
static aui_hdl setup_tsi(int dev)
{
	aui_hdl hdl;
	aui_attr_tsi attr;
	struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];

    if (aui_find_dev_by_idx(AUI_MODULE_TSI, 0, &hdl)){
    	if (aui_tsi_open(&hdl)) {
    		AUI_PRINTF("aui_tsi_open error\n");
    		return 0;
    	}
	}

	aui_tsi_config(tsi_cfg);
	MEMSET(&attr, 0, sizeof(aui_attr_tsi));
	attr.ul_init_param = tsi_cfg[dev].ul_hw_init_val;
	if (aui_tsi_src_init(hdl, tsi_cfg[dev].ul_input_src, &attr)) {
		AUI_PRINTF("aui_tsi_src_init error\n");
		return 0;
	}
	if (aui_tsi_route_cfg(hdl, tsi_cfg[dev].ul_input_src, 1, 1)) {
		AUI_PRINTF("aui_tsi_route_cfg error\n");
		return 0;
	}
	return hdl;
}

struct dvbs_as_stop_data {
	unsigned long dev;
	unsigned int as_stop_delay;
};
#ifdef AUI_TDS
#define sleep(x) osal_task_sleep(1000*x)
static void nim_as_stop(DWORD p, DWORD unused)
#else
static void nim_as_stop(void *p)
#endif
{
	aui_hdl hdl;
	struct dvbs_as_stop_data *data = (struct dvbs_as_stop_data *)p;

	AUI_PRINTF("NIM stopping autoscan in %d seconds\n", data->as_stop_delay);
	sleep(data->as_stop_delay);

	// In Linux, No need to stop auto scan if the NIM has been closed.
	if (aui_find_dev_by_idx(AUI_MODULE_NIM, data->dev, &hdl)) {
		AUI_PRINTF("error: nim not opened\n");
		return;
	}

	if (aui_nim_auto_scan_stop(hdl))
		AUI_PRINTF("error stopping autoscan\n");
	else
		AUI_PRINTF("NIM %d stop autoscan OK\n", data->dev);

#ifdef AUI_TDS
	osal_task_exit(0);
#endif

}

static int dvbs_blind_scan_mode(int band, int polar, int src, int dev, unsigned int timeout)
{
	aui_hdl hdl = 0;
	aui_hdl tsi_hdl;

	aui_nim_attr attr;
	attr.ul_nim_id = dev;
	attr.en_dmod_type = AUI_NIM_QPSK;
	struct dvbs_as_stop_data as_stop_data;
	unsigned int as_stop_delay = timeout;
	int ret = 0;

#ifndef AUI_TDS
	if (aui_nim_open(&attr,&hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}
#else
	if (aui_nim_handle_get_byid(dev, &hdl))
	{
		AUI_PRINTF("aui_nim_handle_get_byid error\n");
		return -1;
	}
#endif

	aui_autoscan_sat as_info;
	as_info.pv_user_data = (void *)dev;
	as_info.ul_start_freq = 950;
	as_info.ul_stop_freq = 2200;
	as_info.ul_polar = polar;
	as_info.ul_freq_band = band;
	as_info.ul_src = src;
	as_info.aui_as_cb = fn_aui_ascallback;
	as_polar = polar;
    as_high_band = band;

	AUI_PRINTF("nim as dev %d src %d, polar %s, band %d\n",
	       (int)attr.ul_nim_id, src, polar ? "V": "H", band);


	if ((tsi_hdl = setup_tsi(dev)) == NULL) {
		AUI_PRINTF("setup_tsi error\n");
#ifndef AUI_TDS
		aui_nim_close(hdl);
#endif
		return -1;
	}

	// Stop auto scan in some seconds
	// The stop command execute in another thread
#ifdef AUI_TDS
	OSAL_ID stop_thread = OSAL_INVALID_ID;
#else
	pthread_t stop_thread;
	pthread_attr_t thread_attr;
	int pthread_create_ret = 0;
#endif
	long int as_start_time=0,as_stop_time=0;

	if (as_stop_delay) {
		memset(&as_stop_data, 0, sizeof(as_stop_data));
		as_stop_data.dev = dev;
		as_stop_data.as_stop_delay = as_stop_delay;
#ifdef AUI_TDS
		T_CTSK  task_param;

		task_param.task = nim_as_stop;
		task_param.name[0] = 'S';
		task_param.name[1] = 'A';
		task_param.name[2] = 'S';
		task_param.quantum = 10;
		task_param.itskpri = OSAL_PRI_NORMAL;
		task_param.stksz = 0xd000 * 8;
		task_param.para1 = (DWORD)&as_stop_data;
		task_param.para2 = 0;

		stop_thread = osal_task_create(&task_param);
		AUI_PRINTF("Run task %s in task %d\n", "SAS", stop_thread);

		// Do not use the Nestor function run_tds_task in sample code.
		//stop_thread = run_tds_task( nim_as_stop, (DWORD)&as_stop_data, 0, "stop as");
#else
		pthread_attr_init(&thread_attr);
		pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

		pthread_create_ret = pthread_create(&stop_thread, &thread_attr,
				(void *)&nim_as_stop, (void *)&as_stop_data);
		if (pthread_create_ret) {
			AUI_PRINTF("Fail to set auto scan timeout\n");
		}
#endif
	}

#ifdef AUI_TDS
	as_start_time = osal_get_time();
#else
	as_start_time = time(NULL);
#endif
	tp_cnt = 0;
	tp_ver_cnt = 0;
	tp_hor_cnt = 0;
	ret = aui_nim_auto_scan_start(hdl, &as_info);
	if (ret) {
		AUI_PRINTF("scan_start error\n");
	}

#ifdef AUI_TDS
	if (stop_thread != OSAL_INVALID_ID) {
		osal_task_delete(stop_thread);
		AUI_PRINTF("Delete stop_thread %d\n", stop_thread);
	}
#else
	if (!pthread_create_ret)
		pthread_cancel(stop_thread);
#endif

#ifdef AUI_TDS
		as_stop_time = osal_get_time();
#else
		as_stop_time = time(NULL);
#endif
	AUI_PRINTF("\nAutoscan lock %d TP (HOR TP %d, VER TP %d), using time = %ld\n",
			tp_cnt, tp_hor_cnt, tp_ver_cnt, (as_stop_time - as_start_time));
        (void)as_stop_time;//just for build.
        (void)as_start_time;//just for build.

#ifndef AUI_TDS
	aui_nim_close(hdl);
#endif
	aui_tsi_close(tsi_hdl);
	return ret;
}


static int aui_nim_dvbs_test(int scan_mode, int src, int band,
			     int freq, int symb, int polar, int dev, unsigned int timeout, int stream_id)
{
	int ret = 0;
#ifndef AUI_TDS
	if (aui_nim_init(nim_init_cb)) {
		aui_nim_de_init( NULL);
		if (aui_nim_init(nim_init_cb)) {
			AUI_PRINTF("aui nim init error\n");
			return -1;
		}
	}
#endif

	if (scan_mode) {
		if ((ret = dvbs_blind_scan_mode(band, polar, src, dev, timeout)))
			goto nim_test_err;
	} else if (freq & symb) {
		if ((ret = dvbs_connect(freq, symb, 0, polar, src, dev, stream_id)))
			goto nim_test_err;
	}

nim_test_err:
#ifndef AUI_TDS
	if (aui_nim_de_init(NULL))
		AUI_PRINTF("aui_nim_de_init error\n");
#endif
	return ret;
}


static int aui_nim_dvbc_test(int dev, int freq, int symb, int mod)
{
	int ret = 0;
#ifndef AUI_TDS
	if (aui_nim_init(nim_init_cb)) {
		aui_nim_de_init( NULL);
		return -1;
	}
#endif

	ret = dvbc_connect(dev, freq, symb, mod);
#ifndef AUI_TDS
	if (aui_nim_de_init(NULL))
		AUI_PRINTF("aui_nim_de_init error\n");
#endif
	return ret;
}

static int aui_nim_dvbt_test(int dev, unsigned int std, int freq, int band) {
	int ret = 0;
#ifndef AUI_TDS
	if (aui_nim_init(nim_init_cb)) {
		aui_nim_de_init( NULL);
		return -1;
	}
#endif

	ret = dvbt_connect(dev, std, freq, band);
#ifndef AUI_TDS
	if (aui_nim_de_init(NULL)) {
		AUI_PRINTF("aui_nim_de_init error\n");
	}
#endif
	return ret;
}

#ifdef TEST_APP_CMD

#ifndef AUI_TDS
unsigned long nim_test_open(unsigned long *argc,char **argv,char *sz_output)
{
	aui_hdl hdl = 0;
	struct aui_nim_attr nim_attr;
	int dev = 0;
	aui_nim_std std=0;
	(void) sz_output;

	if (*argc == 0){
	AUI_PRINTF("Use:\n");
	AUI_PRINTF("cmd_num nim_dev,[DVBT_std]\n");
	AUI_PRINTF("DVBT_std: 0[ISDBT] 1[DVBT] 2[DVBT2] 3[COMBO]\n");
	AUI_PRINTF("e.g.:\n");
	AUI_PRINTF("1 0\n");
	return 0;
	}
	if (*argc > 0)
		dev = atoi(argv[0]);
	if (*argc > 1)
		std = atoi(argv[1]);

	if (aui_nim_init(nim_init_cb)) {
		aui_nim_de_init(NULL);
		return -1;
	}
	nim_attr.ul_nim_id = dev;
	nim_attr.en_dmod_type = AUI_NIM_OFDM;
	nim_attr.en_ter_std = std;
	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}

	aui_nim_close(hdl);

	if (aui_nim_de_init(NULL))
		AUI_PRINTF("aui_nim_de_init error\n");
	return 0;
}
#endif

unsigned long nim_test_dvbs_connect(unsigned long *argc,char **argv,char *sz_output)
{
	int freq = FREQ;
	int symb = SYMB;
	int polar = 0;
	int dev = 1;
	int src = 0;
	(void) sz_output;
	int stream_id = 0;
	(void)stream_id;

	if (*argc < 5) {
		AUI_PRINTF("Use:\n");
		AUI_PRINTF("cmd_num dev,src,polar,freq,symb,stream id\n");
		AUI_PRINTF("e.g.:\n");
		AUI_PRINTF("2 0,0,0,3840,27500,0\n");
		return 0;
	}
	dev = atoi(argv[0]);
	src = atoi(argv[1]);
	polar = atoi(argv[2]);
	freq = atoi(argv[3]);
	symb = atoi(argv[4]);
	if (*argc > 5)
		stream_id = atoi(argv[5]);

	return aui_nim_dvbs_test(0 /* connect mode */, src, 0, freq, symb, polar, dev, 0, stream_id);
}

unsigned long nim_test_dvbt_connect(unsigned long *argc, char **argv, char *sz_output) {
    int freq = 0;
	int band = 0;
    int dev = 0;
	unsigned int std=0;
	(void) sz_output;

	if (*argc == 0){
	AUI_PRINTF("Use:\n");
	AUI_PRINTF("cmd_num nim_dev,DVBT_std,freq,band\n");
	AUI_PRINTF("DVBT_std: 0[ISDBT] 1[DVBT] 2[DVBT2] 3[COMBO]\n");
	AUI_PRINTF("e.g.:\n");
	AUI_PRINTF("5 0,0,554000,8\n");
	return 0;
	}
	if (*argc > 0)
		dev = atoi(argv[0]);
	if (*argc > 1)
		std = atoi(argv[1]);
	if (*argc > 2)
		freq = atoi(argv[2]);
	if (*argc > 3)
		band = atoi(argv[3]);

	return aui_nim_dvbt_test(dev, std, freq, band);

}

unsigned long nim_test_dvbc_connect(unsigned long *argc,char **argv,char *sz_output)
{
	int freq = DVBC_FREQ;
	int symb = DVBC_SYMB;
	int dev = 0;
	int mod = DVBC_MOD;
	(void) sz_output;

	if (*argc == 0){
		AUI_PRINTF("Use:\n");
		AUI_PRINTF("cmd_num nim_dev,freq,symb,mod\n");
		AUI_PRINTF("mod: 2[QMA32] 3[QAM64] 4[QAM128] 5[QAM256]\n");
		AUI_PRINTF("e.g.:\n");
		AUI_PRINTF("4 0,55400,6875,5\n");
		return 0;
	}
	if (*argc > 0)
		dev = atoi(argv[0]);
	if (*argc > 1)
		freq = atoi(argv[1]);
	if (*argc > 2)
		symb = atoi(argv[2]);
	if (*argc > 3)
		mod = atoi(argv[3]);

	return aui_nim_dvbc_test(dev, freq, symb, mod);
}

unsigned long nim_test_dvbs_autoscan(unsigned long *argc,char **argv,char *sz_output)
{
	int polar = 0;
	int dev = 0;
	int band = 0;
	int src = 0;
	unsigned int timeout = 0;
	(void) sz_output;

	if (*argc != 6) {
		AUI_PRINTF("Use:\n");
		AUI_PRINTF("cmd_num dev,src,polar, Hi/Lo band,timeout, C/Ku band\n");
		AUI_PRINTF("e.g.:\n");
		AUI_PRINTF("3 0,0,0,1,100,C\n");
		AUI_PRINTF("3 0,0,0,2,100,K\n");
		return 0;
	}
	dev = atoi(argv[0]);
	src = atoi(argv[1]);
	polar = atoi(argv[2]);
	band = atoi(argv[3]);
	timeout = atoi(argv[4]);
	if (*argv[5] == 'C' || *argv[5] == 'c')
		as_c_band = 1;
	else
		as_c_band = 0;
	return aui_nim_dvbs_test(1 /* scan mode */, src, band, 0, 0, polar, dev, timeout, 0);
}

static unsigned char diseqc_reset[] = { 0xE0, 0x00, 0x00 }; // reset diseqc

static unsigned char commands[][4] =    {
                                { 0xE0, 0x10, 0x38, 0xf0 }, // port 0
                                { 0xE0, 0x10, 0x38, 0xf1 }, // port 1
                                { 0xE0, 0x10, 0x38, 0xf2 }, // port 2
                                { 0xE0, 0x10, 0x38, 0xf3 },
                                { 0xE0, 0x10, 0x38, 0xf4 },
                                { 0xE0, 0x10, 0x38, 0xf5 },
                                { 0xE0, 0x10, 0x38, 0xf6 },
                                { 0xE0, 0x10, 0x38, 0xf7 },
                                { 0xE0, 0x10, 0x38, 0xf8 },
                                { 0xE0, 0x10, 0x38, 0xf9 },
                                { 0xE0, 0x10, 0x38, 0xfa },
                                { 0xE0, 0x10, 0x38, 0xfb },
                                { 0xE0, 0x10, 0x38, 0xfc },
                                { 0xE0, 0x10, 0x38, 0xfd },
                                { 0xE0, 0x10, 0x38, 0xfe },
                                { 0xE0, 0x10, 0x38, 0xff }, // port 15
                                { 0xe0, 0x31, 0x69, 0x00 }, // turn west always
                                { 0xe0, 0x31, 0x68, 0x00 }, // turn east always
                                };

unsigned long nim_test_diseqc(unsigned long *argc,char **argv,char *sz_output)
{   
    int dev;
    int cmd_num;
    int ret = -1;
    aui_hdl hdl = 0;
    struct aui_nim_attr nim_attr;

    
    if (*argc != 2) {
		AUI_PRINTF("Use:\n");
		AUI_PRINTF("cmd_num dev,cmd\n");
		AUI_PRINTF("e.g.:\n");
		AUI_PRINTF("8 0,1\n");
        AUI_PRINTF("[cmd]:1~15:choose port 1 ~ 15\n");
        AUI_PRINTF("      16:turn monitor west\n");
        AUI_PRINTF("      17:turn monitor east\n");
        AUI_PRINTF("      30:reset diseqc\n");
        AUI_PRINTF("      31:set 22K off\n");
        AUI_PRINTF("      32:set 22K on\n");
		return 0;
	}

    dev = atoi(argv[0]);
	cmd_num = atoi(argv[1]);
    
    if (aui_nim_init(nim_init_cb)) {
		aui_nim_de_init(NULL);
		return -1;
	}
	nim_attr.ul_nim_id = dev;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;
	
	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}

    AUI_PRINTF("cmd_num = %d\n", cmd_num);
    switch(cmd_num)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            AUI_PRINTF("choose port...\n");
	     break;
        case 16:
        case 17:
            AUI_PRINTF("starting turn monitor west/east, please wait...\n");
            ret = aui_diseqc_oper(hdl, AUI_DISEQC_MODE_BYTES, commands[cmd_num], 4, NULL, NULL);
            break;
        case 30:
            AUI_PRINTF("reset diseqc...\n");
            ret = aui_diseqc_oper(hdl, AUI_DISEQC_MODE_BYTES, diseqc_reset, 3, NULL, NULL);
            break;
        case 31:
            AUI_PRINTF("set 22K off\n");
            ret = aui_diseqc_oper(hdl, AUI_DISEQC_MODE_22KOFF, NULL, 0, NULL, NULL);
            break;
        case 32:
            AUI_PRINTF("set 22K on\n");
            ret = aui_diseqc_oper(hdl, AUI_DISEQC_MODE_22KON, NULL, 0, NULL, NULL);
            break;

        case 33:
            ret = aui_diseqc_oper(hdl, AUI_DISEQC_MODE_BURST0, NULL, 0, NULL, NULL);
            break;

        case 34:
            ret = aui_diseqc_oper(hdl, AUI_DISEQC_MODE_BURST1, NULL, 0, NULL, NULL);
            break;

        default:
            AUI_PRINTF("don't have this cmd_num\n");
            ret = 1;
    }

    if (cmd_num == 16 || cmd_num == 17) {
        AUI_SLEEP(1000*30);
    } else {
        AUI_SLEEP(1000*3);
    }

    aui_nim_close(hdl);

	if (aui_nim_de_init(NULL))
		AUI_PRINTF("aui_nim_de_init error\n");
    
    return ret;
}

unsigned long nim_test_dvbs_special(unsigned long *argc,char **argv,char *sz_output)
{
    int polar = 0;
	int dev = 0;
	int band = 0;
	int src = 0;
    int status_22k = -1;
    aui_hdl hdl = 0;
	struct aui_nim_attr nim_attr;
	(void) sz_output;

	if (*argc != 3) {
		AUI_PRINTF("Use:\n");
		AUI_PRINTF("cmd_num dev,polar,Hi/Lo band\n");
		AUI_PRINTF("e.g.:\n");
		AUI_PRINTF("7 0,0,1\n");
		return 0;
	}

    dev = atoi(argv[0]);
	polar = atoi(argv[1]);
	band = atoi(argv[2]);
    
    if (aui_nim_init(nim_init_cb)) {
		aui_nim_de_init(NULL);
		return -1;
	}
	nim_attr.ul_nim_id = dev;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;
	
	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}

    if ((band == AUI_NIM_LOW_BAND) || (band == AUI_NIM_HIGH_BAND)) {
		/* Set polarity */
		if (!(polar & ~1)) {
			if (aui_nim_set_polar(hdl, polar)) {
                AUI_PRINTF("aui_nim_set_polar error\n");
				return -1;
			}
		}

        status_22k = (band == AUI_NIM_HIGH_BAND)
						? AUI_NIM_22K_ON : AUI_NIM_22K_OFF;
		if (aui_nim_set22k_onoff(hdl, status_22k)) {
			AUI_PRINTF("aui_nim_set22k_onoff error\n");
			return -1;
		}
		AUI_PRINTF("DVBS dev:%d set polar %d and band %d success\n", dev, polar, band);
    }

    AUI_SLEEP(1000*2);
	aui_nim_close(hdl);

	if (aui_nim_de_init(NULL))
		AUI_PRINTF("aui_nim_de_init error\n");
	return 0;
}

static int config_tsi_route(unsigned long *argc,char **argv, 
	struct ali_app_nim_init_para *init_para_nim,
	struct ali_app_tsi_init_para *init_para_tsi) {
	unsigned long  nim_dev = 0;
    unsigned long  nim_demod_type = 0;
    unsigned long  freq = 0;
    unsigned long  symb = 0;
    unsigned long  band = 0;
    aui_ter_std nim_std=0; /*choose ISDBT default*/
    unsigned long  polar = AUI_NIM_POLAR_HORIZONTAL;
    long plp_index = 0;
    long plp_id = 0;
    aui_tsi_channel_id tsi_input_chn_id = AUI_TSI_CHANNEL_0;
    aui_tsi_output_id tsi_output_dmx_id = AUI_TSI_OUTPUT_DMX_0;
	struct aui_tsi_config tsi_cfg[MAX_TSI_DEVICE];
	
    /* extract para from argv*/
    if (*argc > 0) {
        nim_dev = atoi(argv[0]);/*nim device index,generally have tow device:0 and 1*/
    }
    if (*argc > 1) {
        nim_demod_type = atoi(argv[1]);/*signal demodulation type:DVB-S,DVB-C,DVB-T */
    }
    if (*argc > 2) {
        freq = atoi(argv[2]);/* channel frequency*/
    }
    if (*argc > 3) {
        symb = atoi(argv[3]);/* channel symbal*/
        band = symb;/*channel bandwidth*/
    }
    if (*argc > 4) {
        // DVB-S polar
        if (argv[4][0] == 'V' || argv[4][0] == 'v'
                || argv[4][0] == 'R' || argv[4][0] == 'r') {
            polar = AUI_NIM_POLAR_VERTICAL;
        } else {
            polar = AUI_NIM_POLAR_HORIZONTAL;
        }
        nim_std = (aui_ter_std)atoi(argv[4]);/*DVBT type*/
    }
    if (*argc > 5) {
    	tsi_input_chn_id = atoi(argv[5]);
    	AUI_PRINTF("tsi input index: %d\n", tsi_input_chn_id);
    }
    if (*argc > 6) {
    	tsi_output_dmx_id = atoi(argv[6]);
    	AUI_PRINTF("tsi output index: %d\n", tsi_output_dmx_id);
    }
    if (*argc > 7) {
        plp_index = atoi(argv[7]);
        AUI_PRINTF("PLP index: %d\n", plp_index);
    }
    if (*argc > 8) {
        plp_id = atoi(argv[8]);
        AUI_PRINTF("PLP id: %d\n", plp_id);
    }
    init_para_nim->ul_device = nim_dev;
    init_para_nim->ul_freq = freq;
    init_para_nim->ul_symb_rate = symb;
    init_para_nim->ul_nim_type = nim_demod_type;
    init_para_nim->ul_freq_band = band;
    init_para_nim->ul_nim_std = nim_std;
    init_para_nim->ul_polar = polar;
    init_para_nim->plp_id  = plp_id;
    init_para_nim->plp_index  = plp_index;
	memset(tsi_cfg, 0, sizeof(struct aui_tsi_config)*MAX_TSI_DEVICE);
    aui_tsi_config(tsi_cfg);
    init_para_tsi->ul_hw_init_val = tsi_cfg[nim_dev].ul_hw_init_val;
    init_para_tsi->ul_input_src = tsi_cfg[nim_dev].ul_input_src;
    init_para_tsi->ul_tsi_id = 0;
    init_para_tsi->ul_tis_port_idx = tsi_input_chn_id;
    init_para_tsi->ul_dmx_idx = tsi_output_dmx_id;
	return 0;
}

static void sample_config_tsi_route_usage(const char * cmd)
{
    AUI_PRINTF("Arguments for DVBS:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[symb],[polar],[tsi_in_id],[tsi_out_id]\n");
    AUI_PRINTF("such as CCTV2,input: %s 1,0,3840,27500,H,1,0\n", cmd);
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for DVBC:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[symb],[polar],[tsi_in_id],[tsi_out_id]\n");
    AUI_PRINTF("\t input: %s 1,1,55400,6875,H,1,0\n", cmd);
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for DVBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[DVBT(T2/ISDBT) type],[tsi_in_id],[tsi_out_id]\n");
    AUI_PRINTF("Example for DVBT:\n");
    AUI_PRINTF("\t input: %s 0,2,850000,8,1,1,0\n", cmd);
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for DVB-T2 PLP:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[DVBT(T2/ISDBT) type],[tsi_in_id],[tsi_out_id],[PLP index]\n");
    AUI_PRINTF("PLP index is 0 ~ 255. When index is AUI_NIM_PLP_AUTO");
    AUI_PRINTF("When PLP index is AUI_NIM_PLP_AUTO, it will select the 0 PLP and you can get the PLP number from aui_signal_status\n");
    AUI_PRINTF("Example for play certain PLP index:\n");
    AUI_PRINTF("\t input: %s 0,2,722000,8,2,1,0,3\n", cmd);
    AUI_PRINTF("\n");
    AUI_PRINTF("Arguments for ISDBT:\n");
    AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth],[DVBT(T2/ISDBT) type],[tsi_in_id],[tsi_out_id]\n");
    AUI_PRINTF("Example for ISDBT:\n");
    AUI_PRINTF("\t input: %s 0,2,587143,6,0,1,0\n", cmd);
    AUI_PRINTF("Valid [tsi_in_id] value:\n");
    AUI_PRINTF("\t 1 -> AUI_TSI_CHANNEL_0\n");
    AUI_PRINTF("\t 2 -> AUI_TSI_CHANNEL_1\n");
    AUI_PRINTF("\t 3 -> AUI_TSI_CHANNEL_2\n");
    AUI_PRINTF("\t 4 -> AUI_TSI_CHANNEL_3\n");
    AUI_PRINTF("Valid [tsi_out_id] value:\n");
    AUI_PRINTF("\t 0 -> AUI_TSI_OUTPUT_DMX_0\n");
    AUI_PRINTF("\t 1 -> AUI_TSI_OUTPUT_DMX_1\n");
}

unsigned long sample_config_tsi_route(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct ali_app_tsi_init_para init_para_tsi;
	struct ali_app_nim_init_para init_para_nim;
	memset(&init_para_nim, 0, sizeof(init_para_nim));
	memset(&init_para_tsi, 0, sizeof(init_para_tsi));
	(void)sz_out_put;
	if (0 == *argc) {
		sample_config_tsi_route_usage("6");
		return -1;
	}

	config_tsi_route(argc, argv, &init_para_nim, &init_para_tsi);
	aui_hdl nim_hdl=NULL, tsi_hdl=NULL;

	aui_nim_init(nim_init_cb);
	if (nim_connect(&init_para_nim,&nim_hdl)) {
		AUI_PRINTF("\nnim connect error\n");
		goto fail0;
	}
	AUI_PRINTF("nim connect success\n");

	if (ali_app_tsi_init(&init_para_tsi, &tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto fail0;
	}
	AUI_PRINTF("tsi init success\n");
    return 0;
fail0:
	if (nim_hdl && ali_app_nim_deinit(nim_hdl))
		AUI_PRINTF("%s ->nim deinit fail\n", __func__);
	//Don`t close tsi.
	return -1;
}

void nim_tests_reg(void)
{
	aui_tu_reg_group("nim", "nim tests");
#ifndef AUI_TDS
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, nim_test_open, "nim open");
#endif
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, nim_test_dvbs_connect, "dvbs connect");
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, nim_test_dvbs_autoscan, "dvbs autoscan");
	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, nim_test_dvbc_connect, "dvbc connect");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, nim_test_dvbt_connect, "dvbt connect");
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, sample_config_tsi_route, "configure TSI route");
    aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, nim_test_dvbs_special, "dvbs special test");
    aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, nim_test_diseqc, "dvbs diseqc test");
}

#else

int main(int argc, char *argv[])
{
	int opt;
	int option_index;
	int scan_mode = 0;
	int freq = 0;
	int symb = 0;
	int polar = 0;
	int dev = 0;
	int band = 0;
	int src = 0;

	static struct option long_options[] = {
		{ "help",        no_argument,       0, 'h' },
		{ "scan",        no_argument,       0, 'c' },
		{ "freq",        required_argument, 0, 'f' },
		{ "symbol rate", required_argument, 0, 's' },
		{ "polar",       required_argument, 0, 'p' },
		{ "device",      required_argument, 0, 'd' },
		{ "band",        required_argument, 0, 'b' },
		{ "src",         required_argument, 0, 'r' },
		{ 0, 0, 0, 0}
	};

	while ((opt = getopt_long (argc, argv, "hcf:s:p:d:b:r:",
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
		case 'r':
			src = atoi(optarg);
			break;
		case 'h':
			AUI_PRINTF("Usage: %s -h help\n"
			       " connect:\n"
			       "   -f <frequency>\n"
			       "   -s <symbole rate>\n"
			       "   -p <0:horizontal, 1:vertical>\n"
			       "   -r <0:satellite A, 1:satellite B>\n"
			       "   -d <device 0 or 1> \n"
			       " autoscan:\n"
			       "   -c\n"
			       "   -b <0:low band, 1:high band> for Ku-band\n"
			       "   -p <0:horizontal, 1:vertical>\n"
			       "   -r <0:satellite A, 1:satellite B>\n"
			       "   -d <device 0 or 1> \n", argv[0]);
			return 0;

		}
	}

	return aui_nim_test(BOARD_CFG, scan_mode, src, band, freq, symb, polar, dev);
}
#endif
