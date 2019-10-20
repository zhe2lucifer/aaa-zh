/**@file
*    @brief 	        sample code test can run with script file, don't run the command step by step.
*    @author		Steven.Zhang
*    @date	        2016-10-11
*    @version 		1.0.0
*    @note	        ali corp. all rights reserved. 2013~2999 copyright (C)
*/

#ifndef _AUI_SCRIPT_TEST_H_
#define _AUI_SCRIPT_TEST_H_

#ifdef __cplusplus
extern "C" {
#endif
extern unsigned long test_script_run(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long test_script_help(unsigned long *argc,char **argv,char *sz_out_put);
extern unsigned long sample_dmx_get_pat_table(unsigned long *argc,char **argv,char *sz_out_put);

#ifdef __cplusplus
}
#endif

#endif
