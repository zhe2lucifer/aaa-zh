/****************************INCLUDE HEAD FILE************************************/
#include <aui_decv.h>
#include "aui_decv_test.h"
#include "aui_flash_test.h"

#include <aui_common.h>
#include <stdio.h>
//#include "unity_fixture.h"
#include "logo_data.h"
//#include <stdlib.h>

unsigned long test_decv_colorbar(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_hdl decv_hdl=0;
	aui_attr_decv attr_decv;
	MEMSET(&attr_decv,0,sizeof(aui_attr_decv));


	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
		if (aui_decv_open(&attr_decv,&decv_hdl)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return -1;
		}
	}
	aui_decv_set(decv_hdl, AUI_DECV_SET_COLOR_BAR, NULL);
	AUI_PRINTF("the color bar is showing...\n");
    return AUI_RTN_SUCCESS;    

}

unsigned long test_decv_showlogo(unsigned long *argc,char **argv,char *sz_out_put)
{
    int logo_len = Logo_glass_m2v_len;
    unsigned char *p_data = Logo_glass_m2v;
    unsigned int size_receive = 0;
    aui_hdl decv_hdl;
	aui_attr_decv attr_decv;
	MEMSET(&attr_decv,0,sizeof(aui_attr_decv));
	aui_decv_frame_type frame_type = AUI_DECV_FRAME_TYPE_I;


	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
		if (aui_decv_open(&attr_decv,&decv_hdl)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return AUI_RTN_FAIL;
		}
	}
    aui_decv_decode_format_set(decv_hdl, AUI_DECV_FORMAT_MPEG);
	
    /*
     * Need to set avsync mode to freerun, otherwise the logo may be not show.
     * If the default avsync mode is PCR, then the logo data may be dropped when do avsync.
     */
    if (aui_decv_sync_mode(decv_hdl, AUI_STC_AVSYNC_FREERUN)) {
        AUI_PRINTF("aui_decv_sync_mode return fail!\n");
    }
   
    /** start vdec first **/
    aui_decv_start(decv_hdl);
    /** set frame type after starting**/
    aui_decv_set(decv_hdl, AUI_DECV_SET_FRAME_TYPE, &frame_type);
    /** copy data to decv module **/
    while(logo_len != 0)
    {
        if(AUI_RTN_SUCCESS != aui_decv_copy_data(decv_hdl, (unsigned int)p_data, logo_len, &size_receive))
        {
            AUI_PRINTF("show logo fail!!\n");
            return AUI_RTN_FAIL;
        }
        AUI_PRINTF("logo_len:%d size_receive:%d \n",logo_len,size_receive);
        logo_len -= size_receive;
        p_data += size_receive;
    }

    /** send last frame again to make video engine decode and display the last frame **/
    /** it is only a logo here,so we send this logo again **/
    logo_len = Logo_glass_m2v_len;
    p_data = Logo_glass_m2v;

    while(logo_len != 0)
    {
        if(AUI_RTN_SUCCESS != aui_decv_copy_data(decv_hdl, (unsigned int)p_data, logo_len, &size_receive))
        {
            AUI_PRINTF("show logo fail!!\n");
            return AUI_RTN_FAIL;
        }
        AUI_PRINTF("logo_len:%d size_receive:%d \n",logo_len,size_receive);
        logo_len -= size_receive;
        p_data += size_receive;
    }
    AUI_PRINTF("the logo is showing...\n");
    AUI_SLEEP(2000);
    aui_decv_chg_mode_set(decv_hdl, AUI_DECV_CHG_STILL);
    AUI_PRINTF("stop showing\n");
    aui_decv_stop(decv_hdl);   
    return AUI_RTN_SUCCESS;
}

static unsigned long test_decv_set_variable_resolution(unsigned long *argc,char **argv,char *sz_out_put)
{
    unsigned char enable_variable_resolution = 0;
    if (*argc == 1) {
        enable_variable_resolution = ATOI(argv[0]);
        if ((0 != enable_variable_resolution) && (1 != enable_variable_resolution)) {
            AUI_PRINTF("Please set a correct param!\n");
            AUI_PRINTF("CMD [param]\n");
            AUI_PRINTF("[param]: 0 - turn off variable resolution, 1 - turn on variable resolution\n");
            AUI_PRINTF("3 0\n");
            AUI_PRINTF("3 1\n");
            return -1;
        }
    } else {
        AUI_PRINTF("Please set a correct param!\n");
        AUI_PRINTF("CMD [param]\n");
        AUI_PRINTF("[param]: 0 - turn off variable resolution, 1 - turn on variable resolution\n");
        AUI_PRINTF("3 0\n");
        AUI_PRINTF("3 1\n");
        return -1;
    }

    aui_hdl decv_hdl = NULL;
	aui_attr_decv attr_decv;
	memset(&attr_decv, 0, sizeof(aui_attr_decv));

	if(aui_find_dev_by_idx(AUI_MODULE_DECV, 0, &decv_hdl)) {
		if (aui_decv_open(&attr_decv,&decv_hdl)) {
			AUI_PRINTF("\n aui_decv_open fail\n");
			return -1;
		}
	}
    
    /*
     * If the resolution of the stream is variable, need to turn on this setting.
     * Otherwise, the previous static allocation of memory may not be suitable for one of the resolutions,
     * so the video will mosaic or stuck.
     * Notice: it will be closed in aui_decv_close.
     */
    if (aui_decv_set(decv_hdl, AUI_DECV_SET_VARIABLE_RESOLUTION, &enable_variable_resolution)) {
        AUI_PRINTF("\n aui_decv_set AUI_DECV_SET_VARIABLE_RESOLUTION fail\n");
        return -1;
    }
    AUI_PRINTF("set variable resolution %c\n", enable_variable_resolution);

    return 0;
}

void aui_load_tu_decv()
{
    aui_tu_reg_group("decv", "decv test cases");
    {
		aui_tu_reg_item(2, "1", AUI_CMD_TYPE_UNIT, test_decv_colorbar, "test decv color bar");
		aui_tu_reg_item(2, "2", AUI_CMD_TYPE_UNIT, test_decv_showlogo, "test decv show logo");
#ifdef AUI_LINUX
		/* variable resolution setting is only support Linux */
		aui_tu_reg_item(2, "3", AUI_CMD_TYPE_UNIT, test_decv_set_variable_resolution, "test decv set variable resolution");
#endif   
    }
}


