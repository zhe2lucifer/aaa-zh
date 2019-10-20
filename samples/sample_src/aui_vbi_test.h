/**  @file
*    @brief 	aui vbi test header
*    @author 	andy.yu
*    @date 		2013-7-29
*    @version 	1.0.0
*    @note 		ali corp. all rights reserved. 2013-2999 copyright (C)
*    			input file detail description here
*    			input file detail description here
*    			input file detail description here
*/

#ifndef _AUI_VBI_TEST_H
#define _AUI_VBI_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_vbi.h>
#include "aui_test_app.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
  * this struct is used to save device IO parameter
  */
struct aui_vbi_io_param
{
     unsigned char *io_buff_in;
     unsigned long buff_in_len;
     unsigned char *io_buff_out;
     unsigned long buff_out_len;
};

void test_vbi_reg();

unsigned long test_ttx(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef __cplusplus
	}
#endif
	

#endif

