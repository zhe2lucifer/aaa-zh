#ifndef __AUI_NIM_INIT_H__
#define __AUI_NIM_INIT_H__

#include <aui_nim.h>



AUI_RTN_CODE nim_init_cb(void *pv_param);

#define AUI_TSG_DEV	AUI_NIM_HANDLER_MAX
#define MAX_TSI_DEVICE	(AUI_TSG_DEV + 1)

struct aui_tsi_config {
	unsigned long ul_hw_init_val;
	unsigned long ul_input_src;
};

int aui_tsi_config(struct aui_tsi_config *cfg);

#endif
