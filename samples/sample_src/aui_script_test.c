#include <aui_smc.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "aui_common.h"
#include "aui_script_test.h"
#include "aui_test_app.h"

#define PATH_MAX  1024
extern FILE *stream;

unsigned long test_script_run(unsigned long *argc,char **argv,char *sz_out_put)
{
    //AUI_PRINTF("run test_script_run\n");
    char path[PATH_MAX];
    struct stat st;

    MEMSET(path,0, sizeof(path));
    if (*argc > 0) {
        strncpy(path, argv[0], PATH_MAX);
        AUI_PRINTF("get path = %s\n", path);

        if(stat(path,&st) == 0) {
            AUI_PRINTF("\r\ncheck the sample code auto test file!");
            stream = fopen(path, "r+");
        } else {
            AUI_PRINTF("\r\nthe file: %s NOT FOUND!\n", path);
        }
    }
    return AUI_RTN_SUCCESS;
}

unsigned long test_script_help(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_PRINTF("\tScript Command Help:\n");
    AUI_PRINTF("\tload [path]\n");
    AUI_PRINTF("\tExample:\n");
    AUI_PRINTF("\t\tload /mnt/usb/DISK_a1/sample_code_auto_test\n");
    return AUI_RTN_HELP;
}

void aui_load_tu_script()
{
    aui_tu_reg_group("script", "Run script file's test cases");
    aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_script_help, "Script help");
    aui_tu_reg_item(2, "load", AUI_CMD_TYPE_API, test_script_run, "Load script file that specify a path");
}
