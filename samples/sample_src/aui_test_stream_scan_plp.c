#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "aui_ini_config.h"
#else
#include <api/libfs2/stdio.h>
#endif

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_tsg.h>
#include <aui_nim.h>
#include <aui_dis.h>
#include <aui_decv.h>
#include <aui_deca.h>
#include <aui_av.h>

#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_nim_init.h"
#include "aui_test_stream.h"
#include "aui_test_stream_nim.h"
#include "aui_test_stream_scan_plp.h"

#define NIM_TIMEOUT_MS 1000

static struct ali_aui_hdls *s_p_hdls=NULL;

static void show_usage()
{
	AUI_PRINTF("arguments for DVB-T2 PLP scan:\n");
	AUI_PRINTF("cmd_num [nim_id],[nim_type],[freq],[bandwidth]\n");
	AUI_PRINTF("scan_plp 1,2,722000,8\n");
}

int nim_scan_plp(struct ali_app_nim_init_para *para,aui_hdl *aui_nim_hdl)
{
	aui_hdl hdl = 0;
	struct aui_nim_connect_param param;
	int timeout = NIM_TIMEOUT_MS / 10;
	int lock_status = AUI_NIM_STATUS_UNKNOWN;
	int plp_num = 0;
	int i = 0;
	int ret = 0;

#ifdef AUI_LINUX
	struct aui_nim_attr nim_attr;
    MEMSET(&nim_attr, 0, sizeof(struct aui_nim_attr));

    nim_attr.ul_nim_id = para->ul_device;
    nim_attr.en_dmod_type = AUI_NIM_OFDM;
    nim_attr.en_ter_std = AUI_STD_DVBT2;
    if (para->ul_nim_std == AUI_STD_DVBT2_COMBO) {
        nim_attr.en_ter_std = AUI_STD_DVBT2_COMBO;
    }
    AUI_PRINTF("\nul_nim_std = %d\n",para->ul_nim_std);

	if (aui_nim_open(&nim_attr, &hdl)) {
		AUI_PRINTF("open error\n");
		return -1;
	}
#else
	if (aui_nim_handle_get_byid(para->ul_device, &hdl)) {
		AUI_PRINTF("aui_nim_handle_get_byid error\n");
		return -1;
	}
#endif

	MEMSET(&param, 0, sizeof(struct aui_nim_connect_param));
	param.ul_freq = para->ul_freq;
	param.en_demod_type = AUI_NIM_OFDM;

	switch(para->ul_freq_band) {
	case 6: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_6_MHZ; break; //added by vedic.fu
	case 7: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_7_MHZ; break;
	case 8: param.connect_param.ter.bandwidth = AUI_NIM_BANDWIDTH_8_MHZ; break;
	default : AUI_PRINTF("wrong bandwidth %d\n", para->ul_freq_band);
#ifdef AUI_LINUX
	aui_nim_close(hdl);
#endif
	return -1;
	}

	param.connect_param.ter.std = AUI_STD_DVBT2;
	if (para->ul_nim_std == AUI_STD_DVBT2_COMBO) {
		param.connect_param.ter.std = AUI_STD_DVBT2_COMBO;
	}
	param.connect_param.ter.fec = AUI_NIM_FEC_AUTO;
	param.connect_param.ter.spectrum = AUI_NIM_SPEC_AUTO;

	i = 0;
	do {
		if (i == 0) {
			param.connect_param.ter.u.dvbt2.plp_index = AUI_NIM_PLP_AUTO;
		} else {
			param.connect_param.ter.u.dvbt2.plp_index = AUI_NIM_PLP_SCAN_NEXT;
		}
		lock_status = AUI_NIM_STATUS_UNKNOWN;
		timeout = NIM_TIMEOUT_MS / 10;
		if (aui_nim_connect(hdl, &param)) {
			AUI_PRINTF("%d connect error\n", __LINE__);
#ifdef AUI_LINUX
			aui_nim_close(hdl);
#endif
			return -1;
		}
		while (lock_status != AUI_NIM_STATUS_LOCKED && timeout) {
			if (aui_nim_is_lock(hdl, &lock_status)) {
				AUI_PRINTF("%d is_lock error\n", __LINE__);
				aui_nim_disconnect(hdl);
				aui_nim_close(hdl);
				return -1;
			}
			AUI_SLEEP(10);
			timeout--;
		}
		AUI_PRINTF("[channel is locked] on freq %d in %d ms\n",
				para->ul_freq, NIM_TIMEOUT_MS - timeout*10);
		aui_signal_status info;
		if (aui_nim_signal_info_get(hdl, &info)) {
			AUI_PRINTF("%d info_get error\n", __LINE__);
			aui_nim_disconnect(hdl);
#ifdef AUI_LINUX
			aui_nim_close(hdl);
#endif
			return -1;
		}
		if (i == 0) {
			plp_num = info.u.dvbt2.plp_num;
		}
		AUI_PRINTF("[PLP] index: %d id: %d num %d\n",
				info.u.dvbt2.plp_index,info.u.dvbt2.plp_id, info.u.dvbt2.plp_num);
		i++;
	} while (i < plp_num);

	AUI_PRINTF("%d DVB-T2 PLP scan OK\n", __LINE__);
	aui_nim_disconnect(hdl);
#ifdef AUI_LINUX
	ret = aui_nim_close(hdl);
	AUI_PRINTF("%d close ret: %d\n", __LINE__, ret);
#endif
	(void)ret; //just for build.
	return 0;
}

unsigned long test_aui_nim_scan_plp(unsigned long *argc,char **argv,char *sz_out_put)
{
	struct ali_app_modules_init_para init_para;
	/*assert arguement*/
	if (0 == *argc) {
		show_usage();
		return 0;
	}
	s_p_hdls = malloc(sizeof(struct ali_aui_hdls));
	if(NULL == s_p_hdls) return -1;
	MEMSET(s_p_hdls, 0, sizeof(struct ali_aui_hdls));
	/*init all used device in this mode*/
	ali_aui_init_para_for_test_nim(argc,argv,&init_para);

#ifdef AUI_LINUX
	/*nim_init_cb is callback function point,to init nim device about special board
	 * for example M3733,M3515B*/
	if (aui_nim_init(nim_init_cb)) {
		AUI_PRINTF("\nnim init error\n");
		goto err_live;
	}
	AUI_PRINTF("AUI NIM opened\n");/* Start streaming */
#endif

	if (ali_app_tsi_init(&init_para.init_para_tsi, &s_p_hdls->tsi_hdl)) {
		AUI_PRINTF("\r\n ali_app_tsi_init failed!");
		goto err_live;
	}
	AUI_PRINTF("AUI TSI opened\n");

	if (nim_scan_plp(&init_para.init_para_nim,&s_p_hdls->nim_hdl)) {
		AUI_PRINTF("\nnim scan plp error\n");
		goto err_live;
	}
	AUI_PRINTF("nim scan plp success\n");

	ali_app_deinit(s_p_hdls);
	return AUI_RTN_SUCCESS;

err_live:
	ali_app_deinit(s_p_hdls);
	return AUI_RTN_SUCCESS;
}

