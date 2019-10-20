#include "aui_test_app.h"
#include "aui_sys_setting.h"
#include "aui_flash_test.h"
#include "aui_nim_tests.h"
#include "aui_osd_test.h"
#include "aui_dis_test.h"
#include "aui_av_test.h"
#include "aui_decv_test.h"
#include "aui_deca_test.h"
#include "aui_snd_test.h"
#include "aui_pvr_test.h"
#include "aui_test_stream.h"
#include "aui_dmx_test.h"
#include "aui_hdmi_test.h"
#include "aui_image_test.h"
#include "aui_music_test.h"
#include "aui_uart_test.h"
#include "aui_panel_test.h"
#include "aui_input_test.h"
#include "aui_rtc_test.h"
#include "aui_vbi_test.h"
#include "aui_dog_test.h"
#include "aui_test_stream_decrypt.h"
#include "aui_ca_test.h"
#include "aui_channel_change_test.h"
#include "aui_fs_test.h"
#include "aui_trng_test.h"
#include "aui_smc_test.h"
#include "aui_misc_test.h"
#include "aui_channel_change_test.h"

#ifdef _CAS9_VSC_ENABLE_
#include "aui_test_conaxvsc.h"
#endif

extern AUI_RTN_CODE aui_os_task_sleep(unsigned long ul_time_ms);
#ifdef HDMI_SUPPORT
extern void aui_load_tu_hdmi();
#endif
extern void mp_test_reg();
extern void aui_load_tu_test(void);

/* test app */
int tds_aui_test_entry()
{
    /* init unit test framework */
    if(!auiscpi_init()) {
        AUI_PRINTF("\r\nauiscpi_init failed!");
        return 1;
    }

    test_sys_setting_reg();
    flash_test_reg();
    // _AUI_TEST_MINI_SUPPORT_: TDS CI firmware only include flash relative sample code
    /// for MAC modification.
#ifndef _AUI_TEST_MINI_SUPPORT_
    aui_load_tu_test();
    aui_load_tu_os();


    nim_tests_reg();
#ifdef AUI_TDS
    osd_test_reg();
#else
#ifdef GFX_SUPPORT
    osd_test_reg();
#endif
#endif

    dis_test_reg();
    #ifdef HDMI_SUPPORT
    aui_load_tu_hdmi();
    #endif
#ifndef _BUILD_OTA_E_
    // OTA no need these test module
    aui_load_tu_av();
    aui_load_tu_decv();
    aui_load_tu_deca();
    aui_load_tu_snd();
    mp_test_reg();
    test_pvr_reg();
    test_live_reg();
#endif
#ifdef _CAS9_VSC_ENABLE_
    test_conaxvsc_reg();
#endif
    aui_load_tu_dmx();
    aui_load_tu_hdmi();


    music_test_reg();
    image_test_reg();

     /*
     follow jerry long`s advice:
     for 3823 conax prj
     remove kl/dsc AUI sample code test from aui_test menu
     PS: kl/dsc AUI sample code only be tested on linux platform by GVA
     if somebody fix the dsc&kl bug on aui_test, then can reopen them
     */
     //kl_tests_reg();
     //dsc_test_reg();
    test_uart_reg();
    test_panel_reg();
    test_key_reg();
    test_dog_reg();
    test_rtc_reg();

    test_live_decrypt_reg();
#if    (TTX_ON == 1)    
    test_vbi_reg(); 
#endif
#if 0
    // modify by fawn.fu@20151124, for #43956, remove cc/ttx/subtitle from aui
    test_cc_reg();
#ifdef AUI_SUBT_SUPPORT
    test_ttx_reg();
    subt_test_reg();
#endif
#endif
#ifndef _BUILD_OTA_E_
    // ota no need these modules
    test_ca_reg();
    test_channel_change_reg();
    auifs_tb_register();
    test_trng_reg();
    smc_test_reg();
#endif
    misc_test_reg();
#endif //_AUI_TEST_MINI_SUPPORT_

    aui_str_cmd_print_main_menu();
    //auifs_tb_register();

    while(1) {
        if(drv_get_aui_cmd_task_status()) {
            drv_process_cmd();
        } else {
            break;
        }
        aui_os_task_sleep(100);
    }
    return 0;
}
