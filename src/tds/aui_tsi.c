#include "aui_common_priv.h"

#include <bus/tsi/tsi.h>
#include <aui_tsi.h>

AUI_MODULE(SMC)

typedef enum aui_tsi_input_id   tsi_input;
typedef enum aui_tsi_channel_id tsi_channel;
typedef enum aui_tsi_output_id  tsi_output;
#define ULONG unsigned long
#define UCHAR unsigned char

/* TSI module version number. */
#define AUI_MODULE_VERSION_NUM_TSI	(0X00020200)

struct aui_tsi_handle {
	struct aui_st_dev_priv_data data;
};

AUI_RTN_CODE aui_tsi_version_get(ULONG *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_TSI;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsi_open(aui_hdl *handle)
{
	struct aui_tsi_handle *hdl = MALLOC(sizeof(struct aui_tsi_handle));

	if (hdl == NULL) {
	    aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	hdl->data.dev_idx = 0;
	if (aui_dev_reg(AUI_MODULE_TSI , hdl)) {
		FREE(hdl);
		aui_rtn(AUI_RTN_EBUSY, "aui_dev_reg error");
	}

	*handle = hdl;

	return AUI_RTN_SUCCESS;
}

static int tsi_input_convert(aui_tsi_input_id input, int *id)
{
    int tsiid = 0;
    switch (input) {
        case AUI_TSI_INPUT_SPI_0:// 0, AUI_TSI_INPUT_SPI_0
            tsiid = TSI_SPI_0;
            break;
        case AUI_TSI_INPUT_SPI_1://{1, AUI_TSI_INPUT_SPI_1
            tsiid = TSI_SPI_1;
            break;
        case AUI_TSI_INPUT_TSG:// 2, AUI_TSI_INPUT_TSG
            tsiid = TSI_SPI_TSG;
            break;
        case AUI_TSI_INPUT_SPI_3:// 3, AUI_TSI_INPUT_SPI_3
            tsiid = TSI_SPI_3;
            break;
        case AUI_TSI_INPUT_SSI_0:// 4, AUI_TSI_INPUT_SSI_0
            tsiid = TSI_SSI_0;
            break;
        case AUI_TSI_INPUT_SSI_1:// 5, AUI_TSI_INPUT_SSI_1
            tsiid = TSI_SSI_1;
            break;
        case AUI_TSI_INPUT_PARA:// 6, AUI_TSI_INPUT_PARA
            tsiid = PARA_MODE_SRC;
            break;
        case AUI_TSI_INPUT_RESERVED:// 7, AUI_TSI_INPUT_RESERVED
            tsiid = TSI_SRC_REVD;
            break;
        case AUI_TSI_INPUT_SSI2B_0:// 8, AUI_TSI_INPUT_SSI2B_0
            tsiid = TSI_SSI2B_0;
            break;
        case AUI_TSI_INPUT_SSI2B_1://{9, AUI_TSI_INPUT_SSI2B_1
            tsiid = TSI_SSI2B_1;
            break;
        case AUI_TSI_INPUT_SSI4B_0://{0xA, AUI_TSI_INPUT_SSI4B_0
            tsiid = TSI_SSI4B_0;
            break;
        case AUI_TSI_INPUT_SSI4B_1:// 0xB, AUI_TSI_INPUT_SSI4B_1
            tsiid = TSI_SSI4B_1;
            break;
        case AUI_TSI_INPUT_SSI_2:// 0xC, AUI_TSI_INPUT_SSI_2
            tsiid = TSI_SSI_2;
            break;
        case AUI_TSI_INPUT_SSI_3:// 0xD, AUI_TSI_INPUT_SSI_3
            tsiid = TSI_SSI_3;
            break;
        case AUI_TSI_INPUT_SSI2B_2:// 0xE, AUI_TSI_INPUT_SSI2B_2
            tsiid = TSI_SSI2B_2;
            break;
        case AUI_TSI_INPUT_SSI2B_3:// 0xF, AUI_TSI_INPUT_SSI2B_3
            tsiid = TSI_SSI2B_3;
            break;
        case AUI_TSI_INPUT_SSI4B_2:// 0x10, AUI_TSI_INPUT_SSI4B_2
            return -1;
            break;
        case AUI_TSI_INPUT_SSI4B_3:// 0x11, AUI_TSI_INPUT_SSI4B_3
            return -1;
            break;
        default:
            return -1;
    }
    *id = tsiid;
    return 0;
}

AUI_RTN_CODE aui_tsi_src_init(aui_hdl handle,
                              aui_tsi_input_id ul_data_src_id,
                              aui_p_attr_tsi p_attr_tsi)
{
    int tsiid = 0;

    if ((NULL == p_attr_tsi) || (NULL == handle)) {
        aui_rtn(AUI_RTN_EINVAL, "input error");
    }

    if (tsi_input_convert(ul_data_src_id, &tsiid) != 0) {
        aui_rtn(AUI_RTN_EINVAL, "input error");
    }
    tsi_mode_set(tsiid, p_attr_tsi->ul_init_param);

    return AUI_RTN_SUCCESS;
}

static int tsi_output_id_2_dmxid(tsi_output tsi_output_id)
{
    int dmxid = TSI_DMX_0;

    switch(tsi_output_id)
    {
        case AUI_TSI_OUTPUT_DMX_0:
            dmxid = TSI_DMX_0;
            break;
        case AUI_TSI_OUTPUT_DMX_1:
            dmxid = TSI_DMX_1;
            break;
        case AUI_TSI_OUTPUT_DMX_2:
            dmxid = TSI_DMX_3;
            break;
        case AUI_TSI_OUTPUT_DMX_3:
            dmxid = TSI_DMX_4;
            break;
        default:
            dmxid = TSI_DMX_0;
            break;
    }
    return dmxid;
}

AUI_RTN_CODE aui_tsi_route_cfg(aui_hdl handle,
                               aui_tsi_input_id uc_data_src,
                               aui_tsi_channel_id uc_tsi_in,
                               aui_tsi_output_id uc_dmx_in)
{
    int tsiid = 0;

    if (NULL == handle) {
        aui_rtn(AUI_RTN_EINVAL, "input error");
    }
    tsi_dmx_src_select(tsi_output_id_2_dmxid(uc_dmx_in), uc_tsi_in);

    if (tsi_input_convert(uc_data_src, &tsiid) != 0) {
        aui_rtn(AUI_RTN_EINVAL, "input error");
    }
    tsi_select(uc_tsi_in, tsiid);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_tsi_close(aui_hdl handle)
{
	struct aui_tsi_handle *hdl = (struct aui_tsi_handle *)handle;

    if(NULL==handle)
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

    aui_dev_unreg(AUI_MODULE_TSI, handle);
	FREE(hdl);

	return AUI_RTN_SUCCESS;
}

#if 1
/*
 * DEPRECATED API. It is not public in the header file.
 * Keep it in the code for compatibility, just in case some customer have used it.
*/
AUI_RTN_CODE aui_tsi_ciconnect_mode_set(ULONG ul_mode)
{
	if((MODE_PARALLEL!=ul_mode)&&(MODE_CHAIN!=ul_mode))
	{
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	tsi_parallel_mode_set(ul_mode);
	return AUI_RTN_SUCCESS;
}
#endif

AUI_RTN_CODE aui_tsi_cicard_src_set(ULONG ul_src_id,ULONG ul_card_id)
{
	tsi_para_src_select(ul_src_id,ul_card_id);
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_tsi_ci_card_bypass_set(aui_hdl handle, ULONG ul_card_id, UCHAR uc_bypass)
{
    // TASK #44551, do not support ci chain mode in driver.
    // Just hard code to parallel mode(1) in AUI.
    (void)handle;
    tsi_parallel_mode_set(AUI_TSI_CI_MODE_PARALLEL); 
    
	tsi_ci_select(uc_bypass,ul_card_id);
	return AUI_RTN_SUCCESS;
}

