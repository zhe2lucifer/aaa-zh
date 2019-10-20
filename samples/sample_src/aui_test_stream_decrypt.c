#ifdef AUI_LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <api/libfs2/stdio.h>
#endif

#include <aui_deca.h>
#include <aui_snd.h>
#include <aui_decv.h>
#include <aui_dmx.h>
#include <aui_tsi.h>
#include <aui_nim.h>
#include <aui_av.h>
#include <aui_tsg.h>
#include <aui_kl.h>
#include <aui_dsc.h>

#include "aui_test_stream_decrypt.h"
#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_test_stream.h"
#include "aui_nim_init.h"

static unsigned long test_decrypt(unsigned long *argc,char **argv,char *sz_output)
{
	AUI_PRINTF("\t\t\tthis item haven'n been used!\n");
	return 0;
}

void test_live_decrypt_reg()
{
    aui_tu_reg_group("stream_decrypt", "stream_decrypt");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_decrypt, "stream decrypt");
}
