#ifndef _AUI_I2C_TEST_H
#define _AUI_I2C_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_i2c.h>
#include <aui_test_app.h>
/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/

#ifdef __cplusplus
extern "C" {
#endif

void aui_load_tu_i2c();

unsigned long test_i2c_write_data(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_i2c_write_no_subaddr(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_i2c_read_data(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_i2c_read_no_subaddr(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_i2c_write_read(unsigned long *argc,char **argv,char *sz_out_put);

unsigned long test_i2c_write_read_no_subaddr(unsigned long *argc,char **argv,char *sz_out_put);



#ifdef __cplusplus
}
#endif

#endif

