#include <aui_nim.h>
#include <string.h>
#include "aui_test_app.h"

// C Band LNB L.O. frequency
#define LO_5150 (5150)
#define LO_5750 (5750)
// Ku Band LNB L.O. frequency
#define LO_9750 (9750)
#define LO_10600 (10600)

//DVBS demo code 
//parameter freq is medium frequency,which value is between 950-2150Mhz,sym is in kbd unit.
void aui_nim_test_tp_scan_dvbs(unsigned int freq,unsigned int sym,unsigned char fec)		
{
	aui_hdl nim_hdl = 0;

 	aui_nim_connect_param connect_param;
    memset(&connect_param,0,sizeof(aui_nim_connect_param));

#if 0					//suppose to be called when system startup initialization.
	if(aui_nim_init(0) != AUI_RTN_SUCCESS)
	{
		return;
	}
#endif
	
    aui_nim_attr nim_attr;
	memset(&nim_attr,0x00,sizeof(aui_nim_attr));
    nim_attr.ul_nim_id = 0;
   	nim_attr.en_dmod_type = AUI_NIM_QPSK;
	if(aui_nim_open(&nim_attr,&nim_hdl) != AUI_RTN_SUCCESS)
	{
		return;
	}

	if(nim_hdl)
	{
		connect_param.ul_freq = freq;
		connect_param.connect_param.sat.ul_symrate = sym;
		connect_param.connect_param.sat.fec= fec;
    }

	if(aui_nim_connect(nim_hdl,&connect_param) != AUI_RTN_SUCCESS)
	{
		return;
	}


	//ok,now we polling out the lock status;
	aui_lock_status lock_status = AUI_NIM_STATUS_UNKNOWN;
	while (1)
	{
		if(aui_nim_is_lock(nim_hdl, (int *)&lock_status) != AUI_RTN_SUCCESS)
		{
			return;
		}

		if(lock_status == 1)
			break;
	}


	//ok ,lets get the signal information
	aui_signal_status sig_info;
	memset(&sig_info,0x00,sizeof(aui_signal_status));
	if(aui_nim_signal_info_get(nim_hdl,&sig_info) != AUI_RTN_SUCCESS)
	{
		return;
	}
	
	return;
}

//DVBC demo code, in dvbc platform.
void aui_nim_test_tp_scan_dvbc(unsigned int freq,unsigned int sym,unsigned int qam_mode)
{
	aui_hdl nim_hdl = 0;

 	aui_nim_connect_param connect_param;
    memset(&connect_param,0,sizeof(aui_nim_connect_param));
#if 0			//suppose to be called when system startup initialization.
	if(aui_nim_init(0) != AUI_RTN_SUCCESS)
	{
		return;
	}
#endif
    aui_nim_attr nim_attr;
	memset(&nim_attr,0x00,sizeof(aui_nim_attr));
    nim_attr.ul_nim_id = 0;
   	nim_attr.en_dmod_type = AUI_NIM_QAM;
	if(aui_nim_open(&nim_attr,&nim_hdl) != AUI_RTN_SUCCESS)
	{
		return;
	}

	if(nim_hdl)
	{
		connect_param.ul_freq = freq;
		connect_param.connect_param.cab.ul_symrate = sym;
		connect_param.connect_param.cab.en_qam_mode= qam_mode;
    }

	if(aui_nim_connect(nim_hdl,&connect_param) != AUI_RTN_SUCCESS)
	{
		return;
	}


	//ok,now we polling out the lock status;
	aui_lock_status lock_status = AUI_NIM_STATUS_UNKNOWN;
	while (1)
	{
		if(aui_nim_is_lock(nim_hdl, (int *)&lock_status) != AUI_RTN_SUCCESS)
		{
			return;
		}

		if(lock_status == AUI_NIM_STATUS_LOCKED)
			break;
	}


	//ok ,lets get the signal information
	aui_signal_status sig_info;
	memset(&sig_info,0x00,sizeof(aui_signal_status));
	if(aui_nim_signal_info_get(nim_hdl,&sig_info) != AUI_RTN_SUCCESS)
	{
		return;
	}
	
	return;
}


//DVBS blind scan

int fn_aui_ascallback(unsigned char status, unsigned char polar, unsigned int freq, unsigned int sym, unsigned char fec)
{
	AUI_PRINTF("%s line %d,status = %d,polar = %d,freq = %d,sym = %d,fec = %d\n",__func__,__LINE__,status,polar,freq,sym,fec);
	return 0;
}

void dvbs_c_band_blind_scan_mode()
{
	aui_hdl nim_hdl = 0;

 	aui_nim_connect_param connect_param;
    	memset(&connect_param,0,sizeof(aui_nim_connect_param));
#if 0			//suppose to be called when system startup initialization.
	if(aui_nim_init(0) != AUI_RTN_SUCCESS)
	{
		return;
	}
#endif
    aui_nim_attr nim_attr;
	memset(&nim_attr,0x00,sizeof(aui_nim_attr));
    nim_attr.ul_nim_id = 0;
   	nim_attr.en_dmod_type = AUI_NIM_QPSK;
	if(aui_nim_open(&nim_attr,&nim_hdl) != AUI_RTN_SUCCESS)
	{
		return;
	}

	aui_autoscan_sat as_info;
	memset(&as_info,0x00,sizeof(aui_autoscan_sat));
	as_info.ul_start_freq = 950;
	as_info.ul_stop_freq = 2150;
	as_info.ul_polar = 0;
	as_info.aui_as_cb = (aui_autoscan_callback)fn_aui_ascallback;
	if(nim_hdl)
	{
		if(aui_nim_auto_scan_start(nim_hdl,&as_info) != AUI_RTN_SUCCESS)
		{
			return;
		}
	}

	return;
}


//static void test_nim_ku_autoscan()
//{
//
// 	aui_nim_connect_param connect_param;
//    	memset(&connect_param,0,sizeof(aui_nim_connect_param));
//
//	if(aui_nim_init(0) != AUI_RTN_SUCCESS)
//	{
//		return;
//	}
//	aui_hdl aui_nim_hdl = 0;
//    	aui_nim_attr nim_attr;
//	memset(&nim_attr,0x00,sizeof(aui_nim_attr));
//    	nim_attr.ul_nim_id = 0;
//   	 nim_attr.en_dmod_type = AUI_NIM_QPSK;
//	if(aui_nim_open(&nim_attr,&aui_nim_hdl) != AUI_RTN_SUCCESS)
//	{
//		asm(".word 0x7000003f;nop");
//		return;
//	}
//
//	aui_autoscan_sat as_info;
//	memset(&as_info,0x00,sizeof(aui_autoscan_sat));
//
//	int polar = 0;
//	int k22 = 0;
//
//	int lnb_freq[] = {9750,10600};
//	int ku_range[][2] = {{10700,11700},{11700,12750}};
//	int from =0;
//	int to = 0;
//	int lnb_test = 0;
//	for (lnb_test = 0; lnb_test < 3; lnb_test ++)
//	{
//		for(k22 = 0; k22 < 2; k22++)
//		{
//			from = ku_range[k22][0] - lnb_freq[k22];
//			to	  = ku_range[k22][1] - lnb_freq[k22];
//
//			for(polar = 0; polar < 2; polar ++)
//			{
//				as_info.ul_polar = polar;
//				as_info.ul_freq_band = k22;
//				printf("%s line %d,from = %d,to = %d,polar = %d,k22 = %d\n",__func__,__LINE__,from,to,polar,k22);
//				as_info.ul_start_freq = from;
//				as_info.ul_stop_freq = to + 50;
//				as_info.aui_as_cb = (aui_autoscan_callback)fn_aui_ascallback;
//				if(aui_nim_auto_scan_start(aui_nim_hdl,&as_info) != AUI_RTN_SUCCESS)
//				{
//					asm(".word 0x7000003f;nop");
//					return;
//				}
//			}
//		}
//	}
//
//	if(aui_nim_de_init(0) != AUI_RTN_SUCCESS)
//	{
//		return;
//	}
//	return;
//}

int dvbs_connect(int freq, int sym, int fec, int polar, int device)
{
	aui_hdl nim_hdl = 0;
	struct aui_nim_attr nim_attr;
	struct aui_nim_connect_param param;

	memset(&param, 0, sizeof(struct aui_nim_connect_param));

	nim_attr.ul_nim_id = device;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;

	if (aui_nim_open(&nim_attr, &nim_hdl) != AUI_RTN_SUCCESS) {
		AUI_PRINTF("open error\n");
		return -1;
	}

	AUI_PRINTF("nim device %d opened\n", (int)nim_attr.ul_nim_id);

#ifdef KU_BAND
	if (freq > 11700) {
		param.connect_param.sat.ul22K = 1;
		param.ul_freq = freq - LO_10600;
	} else {
		param.connect_param.sat.ul22K = 0;
		param.ul_freq = freq - LO_9750;
	}
#endif

#ifdef C_BAND
	param.connect_param.sat.ul22K = 0;
	param.ul_freq = LO_5150 - freq;
#endif
	param.connect_param.sat.ul_symrate = sym;
	param.connect_param.sat.fec = fec;
	param.connect_param.sat.ul_polar = polar;
	if (aui_nim_connect(nim_hdl, &param) != AUI_RTN_SUCCESS)	{
		AUI_PRINTF("connect error\n");
		aui_nim_close(&nim_hdl);
		return -1;
	}

	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int cnt = 0;
	while (lock_status != AUI_NIM_STATUS_LOCKED) {
		if (aui_nim_is_lock(nim_hdl, &lock_status) != AUI_RTN_SUCCESS) {
			AUI_PRINTF("is_lock error\n");
			aui_nim_disconnect(&nim_hdl);
			aui_nim_close(&nim_hdl);
			return -1;
		}
		int delay = 0;
		while(delay ++ < 10000);
		cnt++;
	}

	AUI_PRINTF("channel is locked on freq %d in %d ms\n", freq, cnt * 10);

	aui_signal_status info;
	if (aui_nim_signal_info_get(nim_hdl, &info) != AUI_RTN_SUCCESS) {
		AUI_PRINTF("info_get error\n");
		aui_nim_disconnect(&nim_hdl);
		aui_nim_close(&nim_hdl);
		return -1;
	}

	AUI_PRINTF("info: freq %ld, strength %d, quality %d, ber %d, rf_level %d,"
	       "signal_cn %d\n", info.ul_freq, (int)info.ul_signal_strength,
	       (int)info.ul_signal_quality, (int)info.ul_bit_error_rate,
	       (int)info.ul_rf_level, (int)info.ul_signal_cn);

	aui_nim_disconnect(nim_hdl);

	aui_nim_close(nim_hdl);
	return 0;
}


void dvbs_blind_scan_mode(int band, int polar, int dev)
{
	aui_hdl nim_hdl = 0;

	aui_nim_attr nim_attr;
	nim_attr.ul_nim_id = dev;
	nim_attr.en_dmod_type = AUI_NIM_QPSK;
	if (aui_nim_open(&nim_attr,&nim_hdl) != AUI_RTN_SUCCESS) {
		AUI_PRINTF("open error\n");
		return;
	}

	aui_autoscan_sat as_info;
	memset(&as_info, 0, sizeof(aui_autoscan_sat));
	as_info.ul_start_freq = 950;
	as_info.ul_stop_freq = 2150;
	as_info.ul_polar = polar;
	as_info.aui_as_cb = (aui_autoscan_callback)fn_aui_ascallback;

	if (aui_nim_auto_scan_start(nim_hdl, &as_info) != AUI_RTN_SUCCESS) {
		AUI_PRINTF("scan_start error\n");
		return;
	}
	return;
}

int main(int argc, char *argv[])
{	
	//guess you have initlaize the app before.
	if (aui_nim_init(0) != AUI_RTN_SUCCESS) {
		AUI_PRINTF("init error\n");
		return -1;
	}


	dvbs_blind_scan_mode(0,0,0);// use nim0,with 22koff,polor H.
	dvbs_blind_scan_mode(0,1,0); //use nim0,with 22koff,polar V.
	dvbs_blind_scan_mode(1,0,0); //use nim0,with 22k on polar H.
	dvbs_blind_scan_mode(1,1,0); //use nim0,with 22k on,polar V.
	

	dvbs_connect(3400, 27500, 0, 0, 0);  //connect TP 3400,27500,with nim0.


	if (aui_nim_de_init(0))
		AUI_PRINTF("aui_nim_de_init error\n");

	return 0;
}
