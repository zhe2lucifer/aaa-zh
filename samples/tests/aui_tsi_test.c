/**@file
*	@brief	   ALi AUI TSI test
*	@author    romain.baeriswyl
*	@date	   2014-02-12
*	@version   1.0.0
*	@note	   ali corp. all rights reserved. 2013-2999 copyright (C)
*/

#include <aui_tsi.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ali_soc_common.h>


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

int main()
{
	aui_hdl tsi_handle;
	struct aui_attr_tsi attr;
	int soc_rev_id = soc_relay_func(ALI_SOC_REV_ID);
	printf("soc_rev_id %d (0x%x)\n", (soc_rev_id & 0xffff) - 1, soc_rev_id);

	int input_id = (soc_rev_id == IC_REV_0) ?
		AUI_TSI_INPUT_SPI_0 : AUI_TSI_INPUT_SPI_3;

	if (aui_tsi_open(&tsi_handle)) {
		printf("aui_tsi_open error\n");
		return -1;
	}

	/* test without configuring input */
	if (aui_tsi_route_cfg(tsi_handle, input_id, AUI_TSI_CHANNEL_0,
				  AUI_TSI_OUTPUT_DMX_0))
		printf("aui_tsi_route_cfg error\n");

	/* configure TSI channel 0 : SPI to DMX 0 */
	attr.ul_init_param = AUI_TSI_IN_CONF_ENABLE |
		AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	if (aui_tsi_src_init(tsi_handle, input_id, &attr))
		printf("aui_tsi_src_init error\n");

	if (aui_tsi_route_cfg(tsi_handle, input_id, AUI_TSI_CHANNEL_0,
				  AUI_TSI_OUTPUT_DMX_0)) {
		printf("aui_tsi_route_cfg error\n");
	}
	printf("Configured TSI channel 0\n");

	/* confiure TSI channel 1 : SSI2B to DMX 1 */
	attr.ul_init_param = AUI_TSI_IN_CONF_ENABLE |
		AUI_TSI_IN_CONF_SSI_ORDER |
		AUI_TSI_IN_CONF_SSI_CLK_POL |
		AUI_TSI_IN_CONF_VALID_SIG_POL |
		AUI_TSI_IN_CONF_SYNC_SIG_POL;
	if (aui_tsi_src_init(tsi_handle, AUI_TSI_INPUT_SSI2B_0, &attr))
		printf("aui_tsi_src_init error\n");

	if (aui_tsi_route_cfg(tsi_handle, AUI_TSI_INPUT_SSI2B_0,
				  AUI_TSI_CHANNEL_1, AUI_TSI_OUTPUT_DMX_1)) {
		printf("aui_tsi_route_cfg error\n");
	}
	printf("Configured TSI channel 1\n");

	if (aui_tsi_close(tsi_handle))
		printf("aui_tsi_close error\n");
	tsi_handle = NULL;
	printf("TSI closed\n");

	/* Check without handler */
	if (aui_tsi_route_cfg(tsi_handle, AUI_TSI_INPUT_SSI2B_0,
				  AUI_TSI_CHANNEL_1, AUI_TSI_OUTPUT_DMX_1)) {
		printf("aui_tsi_route_cfg error\n");
	}

	return 0;
}
