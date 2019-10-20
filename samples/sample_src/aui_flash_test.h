/**  @file
*    @brief 	aui flash test header
*    @author 	andy.yu
*    @date 		2013-7-29
*    @version 	1.0.0
*    @note 		ali corp. all rights reserved. 2013-2999 copyright (C)
*    			input file detail description here
*    			input file detail description here
*    			input file detail description here
*/

#ifndef _AUI_FLASH_TEST_H
#define _AUI_FLASH_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_flash.h>
#include "aui_test_app.h"

#ifdef __cplusplus
extern "C" {
#endif



unsigned long test_nor_flash(unsigned long *argc,char **argv,char *sz_out_put);
unsigned long test_nand_flash(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef AUI_TDS
unsigned long test_misc_set_mac(unsigned long *argc,char **argv,char *sz_out_put);
#endif

void flash_test_reg(void);

#ifdef __cplusplus
	}
#endif
	
#endif

