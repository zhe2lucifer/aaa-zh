/*
 *	(c) Copyright 2013-2999 ALi Corp. ZHA Linux Team (alitech.com)
 *	All rights reserved
 *
 *	@file			aui_test_nim.c
 *  @brief			
 *
 *	@Version:		1.0
 *	@date:			06/08/2015 06:24:52 PM
 *	@revision:		none
 *
 *	@Author:		Vedic Fu <vedic.fu@alitech.com>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aui_dis.h>
#include "aui_test_stream_nim.h"

static void show_nim_usage() {
    AUI_PRINTF("parameters for DVBS and DVBC(QAM256):\n");
    AUI_PRINTF("      [nim_id] [nim_type] [freq] [symb] [vpid] [apid] [ppid] [video format] [audio format] [dis format]\n");
    AUI_PRINTF("DVBS :    1        0       3840   27500   513    660   8190        0              1            0       \n");
    AUI_PRINTF("DVBC :    0        1       55400  6875    851    852   851         1              3            0       \n");
    AUI_PRINTF("parameters for DVBT:\n");
    AUI_PRINTF("      [nim_id] [nim_type] [freq] [bandwidth] [vpid] [apid] [ppid] [video format] [audio format] [dis format] [DVBT type]\n");
    AUI_PRINTF("DVBT :    0        2      554000      8        821    822   821         1              3            0             0     \n");
}

int main(int argc, char **argv)
{
	if(argc < 10)
	{
		AUI_PRINTF("Error parameters !\n");
		show_nim_usage();
		return -1;
	}
	argc--; // remove execute file name
	argv++;
	test_nim(argc, argv);
	
	return 0;
}

