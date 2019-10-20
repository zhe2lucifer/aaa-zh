#ifndef _AUI_HDMI_TEST_H
#define _AUI_HDMI_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_hdmi.h>
#include "aui_test_app.h"
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

void aui_load_tu_hdmi();

unsigned long test_hdmi_start(unsigned long *argc,char **argv,char *sz_out_put);


unsigned long test_hdmi_on(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_off(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_hdmi_audio_on(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_audio_off(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_para_get(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_hdmi_cec_on(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_cec_off(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_cec_allocate_logic_address(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_cec_receive(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_reg_callback_plug_in_sample(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_reg_callback_plug_out(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_reg_callback_hdcp_error(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_del_callback_plug_in(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_del_callback_plug_out(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_del_callback_hdcp_fail(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_hdmi_hdcp_on(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_hdmi_hdcp_off(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_hdmi_av_blank_mute(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_hdmi_set_color_space(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_hdmi_set_deep_color(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_hdmi_help(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef __cplusplus
}
#endif

#endif
