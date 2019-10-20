/**
 *    @brief     TRNG test module
 *    @author    Oscar Shi
 *    @date      2015-05-08
 *    @version   1.0.0
 *    @note      ali corp. all rights reserved. 2014 copyright (C)
 */

#ifndef _AUI_TRNG_TEST_H
#define _AUI_TRNG_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_trng.h>
#include "aui_test_app.h"

#ifdef __cplusplus
extern "C" {
#endif

void test_trng_reg(void);

unsigned long test_teng(unsigned long *argc, char **argv, char *sz_out_put);
unsigned long test_trng_help(unsigned long *argc, char **argv, char *sz_out_put);

#ifdef __cplusplus
	}
#endif

#endif // _AUI_TRNG_TEST_H


