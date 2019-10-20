/**@file
 *    (c) Copyright 2013-2999  ALi Corp(alitech.com).
 *    All rights reserved
 *
 *    @file         main.c
 *    @brief          Linux AUI unit test entry
 *
 *    @version        1.0
 *    @date         11/27/2013  18:32:5
 *    @reversion          null
 *
 *    @author        Peter Pan <peter.pan@alitech.com>
 */
#include <sys/stat.h>
#include "aui_test_app.h"
#include "aui_pip_test.h"
#include "aui_multi_nim_test.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

void test_sys_setting_reg(void);
void flash_test_reg();
void nim_tests_reg();
void osd_test_reg();
void dis_test_reg();
//void subt_test_reg();
//void cc_test_reg();
void test_uart_reg();
void aui_load_tu_dmx();
void aui_load_tu_deca();
void aui_load_tu_snd();
void aui_load_tu_decv();
void aui_load_tu_i2c();

void aui_load_tu_av();
void aui_load_tu_hdmi();
void aui_load_tu_gpio();
#ifdef LINUX_MP_SUPPORT
void mp_test_reg();
#endif
void aui_load_tu_script();
void av_injecter_tests_reg();
void smc_test_reg();
void kl_tests_reg();
void dsc_test_reg();
void test_live_reg();
#ifdef LINUX_VSC_SUPPORT
void test_conaxvsc_reg();
#endif
void misc_test_reg();
void test_dog_reg();
void test_rtc_reg();
void test_panel_reg();
void test_cic_reg();
#ifdef VMX_PLUS_SUPPORT
void vmx_test_reg();
#endif

#ifdef LINUX_PVR_SUPPORT
void test_pvr_reg();
#endif
void test_channel_change_reg();
void test_fcc_reg();
void test_full_nim_reg();
void test_vbi_reg();
void test_trng_reg();
#ifdef AUI_PACKAGE_ALIPLATFORM_INPUT
void test_key_reg();
#endif

#ifdef AUI_TA_SUPPORT
void ta_test_reg();
#endif

FILE *stream;//added by steven@20161010 for sample code support script.
char script_path[1024] = "/usr/mnt_app/sample_code_auto_test";

/* test app */
int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    struct stat st;
    
    /* init unit test framework */
    if(!auiscpi_init()) {
        AUI_PRINTF("\r\nauiscpi_init failed!");
        return 1;
    }
    test_sys_setting_reg();
    flash_test_reg();
    nim_tests_reg();
    osd_test_reg();
    dis_test_reg();
    //subt_test_reg();
    //cc_test_reg();
    aui_load_tu_dmx();
    aui_load_tu_deca();
    aui_load_tu_snd();
    aui_load_tu_decv();
    aui_load_tu_i2c();
    aui_load_tu_av();
    aui_load_tu_hdmi();
    aui_load_tu_gpio();
#ifdef LINUX_MP_SUPPORT
    mp_test_reg();
#endif
    aui_load_tu_script();
    av_injecter_tests_reg();
    smc_test_reg();
    kl_tests_reg();
    dsc_test_reg();
    test_live_reg();
#ifdef LINUX_VSC_SUPPORT
    test_conaxvsc_reg();
#endif
#ifdef VMX_PLUS_SUPPORT
    vmx_test_reg();
#endif
    misc_test_reg();
    test_dog_reg();
    test_rtc_reg();
    test_panel_reg();
    test_cic_reg();
    test_uart_reg();  //add by vedic.fu
#ifdef LINUX_PVR_SUPPORT
    test_pvr_reg();
#endif
    test_channel_change_reg();
    test_multi_nim_reg();
    test_vbi_reg();
    test_trng_reg();
#ifdef AUI_PACKAGE_ALIPLATFORM_INPUT
    test_key_reg();
#endif
    test_pip_reg();

#ifdef AUI_TA_SUPPORT
    ta_test_reg();
#endif

    aui_str_cmd_print_main_menu();

        if (argc > 1) {
            AUI_PRINTF("\r\nAUI: you input the scirpt path = %s\n", script_path);
          memcpy(script_path, argv[1], strlen(argv[1])+1);
        }

    if(stat(script_path,&st) == 0) {
            AUI_PRINTF("\r\nAUI: checked the sample code auto test file path = %s.\n", script_path);
            stream = fopen(script_path, "r+");
        } 

    while(1) {
        if(drv_get_aui_cmd_task_status()) {
            drv_process_cmd();
        } else {
            break;
        }
        usleep(100 * 1000);
    }

    return 0;
}
