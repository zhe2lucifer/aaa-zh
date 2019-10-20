/**@file
*	@brief	   ALi AUI TSG test
*	@author    romain.baeriswyl
*	@date	   2014-02-12
*	@version   1.0.0
*	@note	   ali corp. all rights reserved. 2013-2999 copyright (C)
*/

#include <aui_tsg.h>
#include <string.h>
#include <stdio.h>

#define PKT_NUM 256
#define PKT_SIZE 188

int main()
{
	struct aui_tsg_module_attr moduleAttr;
	struct aui_attr_tsg attr;
	aui_hdl tsg_handle;
	unsigned int buffer[PKT_NUM * PKT_SIZE / sizeof(int)];
	int ret;

	MEMSET(&moduleAttr, 0, sizeof(struct aui_tsg_module_attr));

	/* does nothing, should it be removed ? */
	if (aui_tsg_init(NULL, NULL)) {
		printf("aui_tsg_init error\n");
		return 1;
	}

	/* TSG clock = MEM_CLK / (ul_tsg_clk * 2) with MEM_CLK = 200MHz */
	attr.ul_tsg_clk = 24; /* TSG clock = 4.16 MHz */
	if (aui_tsg_open(&attr, &tsg_handle)) {
		printf("aui_tsg_init error\n");
		aui_tsg_de_init(NULL, NULL);
		return 1;
	}

	attr.ul_bit_rate = 100; /* default bitrates */
	printf("tsg starting\n");
	ret = aui_tsg_start(tsg_handle, &attr);
	if (ret) {
		printf("aui_tsg_start error %d\n", ret);
		aui_tsg_close(tsg_handle);
		aui_tsg_de_init(NULL, NULL);
		return 1;
	}

	MEMSET(buffer, 0x5a, PKT_NUM * PKT_SIZE);

	attr.ul_addr = (unsigned char *)buffer; /* DRAM buff must be 16 bytes align */
	attr.ul_pkg_cnt = PKT_NUM;
	attr.uc_sync_mode = AUI_TSG_XFER_SYNC;

	printf("tsg sending\n");
	ret = aui_tsg_send(tsg_handle, &attr);
	if (ret) {
		printf("\naui_tsg_send error 0x%x\n", ret);
		aui_tsg_close(tsg_handle);
		aui_tsg_de_init(NULL, NULL);
		return 1;
	}

	printf("\nsent the buffer\n");

	aui_tsg_stop(tsg_handle, &attr);
	aui_tsg_close(tsg_handle);
	aui_tsg_de_init(NULL, NULL);

	return 0;
}