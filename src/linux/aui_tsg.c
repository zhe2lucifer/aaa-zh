/**@file
*   @brief     ALi AUI TSG function implementation
*   @author    romain.baeriswyl
*   @date      2014-02-12
*   @version   1.0.0
*   @note      ali corp. all rights reserved. 2013-2999 copyright (C)
*/

#include "aui_common_priv.h"
#include <aui_tsg.h>
#include <alisltsg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

AUI_MODULE(TSG)

/* TSG module version number. */
#define AUI_MODULE_VERSION_NUM_TSG	(0X00020200)

#define SEND_TIMEOUT 50 /* ms */

#define TS_PKT_SIZE 188

//#define ENABLE_MISSING_IOCTL

struct handle_tsg {
	struct aui_st_dev_priv_data data;
	aui_attr_tsg attr;
	alisl_handle dev;
	char *conti_cnts; /* continuity counter for each PID */
};


AUI_RTN_CODE aui_tsg_version_get(unsigned long *version)
{
	if (!version)
		aui_rtn(AUI_RTN_EINVAL,NULL);

	*version = AUI_MODULE_VERSION_NUM_TSG;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_init(p_fun_cb call_back_init, void *param)
{
	if (call_back_init)
		return call_back_init(param);

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_de_init(p_fun_cb call_back_init, void *param)
{
	if (call_back_init)
		return call_back_init(param);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsg_open(const aui_attr_tsg *attr, aui_hdl* const handle)
{
	if(!handle || !attr)
		aui_rtn(AUI_RTN_EINVAL, NULL);

	struct handle_tsg *hdl = calloc(sizeof(struct handle_tsg), 1);
	if (!hdl)
		aui_rtn(AUI_RTN_ENOMEM, NULL);

	hdl->data.dev_idx = 0;
	if (aui_dev_reg(AUI_MODULE_TSG, hdl)) {
		free(hdl);
		aui_rtn(AUI_RTN_EINVAL, "aui_dev_reg error");
	}

	if (alisltsg_open(&hdl->dev, TSG_ID_M36_0, NULL)) {
		aui_tsg_close(hdl);
		aui_rtn(AUI_RTN_EINVAL, "tsg open error");
	}
	memcpy(&hdl->attr, attr, sizeof(struct aui_attr_tsg));

	AUI_DBG("configure TSG_ID_M36_0 with clock %d (%d kHz)\n",
		 (int)attr->ul_tsg_clk, 100000 / (int)attr->ul_tsg_clk);
	if (alisltsg_set_clkasync(hdl->dev, attr->ul_tsg_clk)) {
		aui_tsg_close(hdl);
		aui_rtn(AUI_RTN_FAIL, "tsg set_clk_async error");
	}

	*handle = hdl;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_close(aui_hdl handle)
{
	struct handle_tsg *hdl = (struct handle_tsg *)handle;
	int err = AUI_RTN_SUCCESS;

	if (!hdl)
		return AUI_RTN_SUCCESS;

	if (hdl->conti_cnts) {
		free(hdl->conti_cnts);
		hdl->conti_cnts = NULL;
	}

	if (hdl->dev)
		if (alisltsg_close(&hdl->dev)) {
			err = AUI_RTN_FAIL;
			pr_err("sl close error\n");
		}
	if (aui_dev_unreg(AUI_MODULE_TSG, hdl)) {
		err = AUI_RTN_FAIL;
		pr_err("unreg error");
	}
	free(hdl);
	return err;
}


AUI_RTN_CODE aui_tsg_start(aui_hdl handle, const struct aui_attr_tsg *attr)
{
	struct handle_tsg *hdl = (struct handle_tsg *)handle;
	if (!hdl || !hdl->dev || !attr)
		return AUI_RTN_EINVAL;

#ifdef ENABLE_MISSING_IOCTL
	if (alisltsg_insertionstart(hdl->dev, attr->ul_bit_rate))
		aui_rtn(AUI_RTN_FAIL, "tsg insertionstart error");
#endif
	hdl->attr.ul_bit_rate = attr->ul_bit_rate;
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_stop(aui_hdl handle, const struct aui_attr_tsg *attr)
{
	struct handle_tsg *hdl = (struct handle_tsg *)handle;
	if (!hdl || !hdl->dev)
		return AUI_RTN_EINVAL;
#ifdef ENABLE_MISSING_IOCTL
	if (alisltsg_insertionstop(hdl->dev))
		aui_rtn(AUI_RTN_FAIL, "tsg insertionstop error");
#endif
	if (hdl->conti_cnts) {
		free(hdl->conti_cnts);
		hdl->conti_cnts = NULL;
	}

    // unused parameter
    (void)attr;

	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_send(aui_hdl handle, const struct aui_attr_tsg *attr)
{
	struct handle_tsg *hdl = (struct handle_tsg *)handle;
	if (!hdl || !hdl->dev || !attr)
		return AUI_RTN_EINVAL;

	if (alisltsg_copydata(hdl->dev, attr->ul_addr, attr->ul_pkg_cnt * TS_PKT_SIZE))
		aui_rtn(AUI_RTN_FAIL, "tsg copydata error");

	if (attr->uc_sync_mode) {
		/* wait transfer is complete */
		uint32_t pkt_cnt = 1;
		int timeout = SEND_TIMEOUT; /* ms */

		while (pkt_cnt && timeout--) {
			if (alisltsg_check_remain_buf(hdl->dev, &pkt_cnt))
				aui_rtn(AUI_RTN_FAIL,
					"tsg check_remain_buf error");
			usleep(1000);
		}
		if (timeout == 0)
			aui_rtn(AUI_RTN_FAIL, "tsg send timeout");
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_check_buf_status(aui_hdl handle, unsigned long buffer,
				      unsigned long pkt_cnt)
{
	struct handle_tsg *hdl = (struct handle_tsg *)handle;
	if (!hdl || !hdl->dev || !buffer)
		return AUI_RTN_EINVAL;

	int pid;
	unsigned long cnt = 0;
	char *p;
	int exp_conti_cnt;
	int conti_cnt;
	char *buf = (char *)buffer;
	int err = 0;

	if (!hdl->conti_cnts) {
		hdl->conti_cnts = malloc(8192);
		if (!hdl->conti_cnts)
			aui_rtn(AUI_RTN_FAIL,"malloc error");
		memset(hdl->conti_cnts, -1, 8192);
	}

	while (cnt < pkt_cnt) {
	    // need to make sure pid > 0
		pid = (((unsigned char)buf[1] & 0x1F) << 8) | (unsigned char)buf[2];
		conti_cnt = buf[3] & 0xf;

		p = hdl->conti_cnts + pid;
		exp_conti_cnt = *p;

		if (exp_conti_cnt != -1) {
			if ((buf[3] >> 5) & 1) /* payload present */
				exp_conti_cnt = (exp_conti_cnt + 1) & 0xf;

			if (exp_conti_cnt != conti_cnt) {
				err++;
				exp_conti_cnt = conti_cnt;
			}
		}
		buf += TS_PKT_SIZE;
		*p = exp_conti_cnt;
		cnt++;
	}
	if (err) {
		AUI_DBG("%d discontinuity\n", err);
		return AUI_RTN_FAIL;
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsg_check_remain_pkt(aui_hdl handle,unsigned long *pkt_cnt)
{
	struct handle_tsg *hdl = (struct handle_tsg *)handle;
	uint32_t cnt;
	if (!hdl || !hdl->dev || !pkt_cnt)
		return AUI_RTN_EINVAL;

	if (alisltsg_check_remain_buf(hdl->dev, &cnt))
		aui_rtn(AUI_RTN_FAIL, "tsg check_remain_buf error");

	*pkt_cnt = cnt / TS_PKT_SIZE;
	return AUI_RTN_SUCCESS;
}
