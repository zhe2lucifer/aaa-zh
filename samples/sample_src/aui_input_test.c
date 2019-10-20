
/****************************INCLUDE FILE************************************/
#include "aui_input_test.h"
#include <aui_input.h>
#include "aui_help_print.h"

#ifndef AUI_TDS
#include <linux/input.h>
#include <ali_ir_common.h>
#endif


/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
/****************************TEST MODULE IMPLEMENT********************************/

#ifdef AUI_LINUX
const struct aui_key_map ali_key_map[] =
{
        {0x60df2ad5,            KEY_MENU},

        {(0x60df12ed),          KEY_RIGHT},
        {(0x60dfb847),          KEY_DOWN},
        {(0x60df3ac5),          KEY_ENTER},
        {(0x60df38c7),          KEY_LEFT},
        {(0x60df22dd),          KEY_UP},

        {(0x60df926d),          KEY_0},
        {(0x60dfc837),          KEY_1},
        {(0x60df08f7),          KEY_2},
        {(0x60df8877),          KEY_3},
        {(0x60dff00f),          KEY_4},
        {(0x60df30cf),          KEY_5},
        {(0x60dfb04f),          KEY_6},
        {(0x60dfd02f),          KEY_7},
        {(0x60df10ef),          KEY_8},
        {(0x60df906f),          KEY_9},

        {(0x60df50af),          KEY_MEDIA},
        {(0x60dfd22d),          KEY_PAGEDOWN},
        {(0x60dfe01f),          KEY_PAGEUP},
        {(0x60df827d),          KEY_TEXT},
        {(0x60df708f),          KEY_POWER},
        {(0x60df20df),          KEY_PREVIOUS},
        {(0x60df0af5),          KEY_NEXT},

        {(0x60df629d),          KEY_AUDIO},
        {(0x60df807f),          KEY_SUBTITLE},

        {(0x60df9867),          KEY_SLEEP},
        {(0x60dfe21d),          KEY_FIND},
        {(0x60dfa05f),          KEY_MUTE},
        {(0x60df7a85),          KEY_PAUSE},
#if 1
        {(0x60dfa25d),          KEY_FORMAT},
#else
        {(0x60dfa25d),                  KEY_HELP},
#endif
        {(0x60df6897),          KEY_INFO},
        {(0x60df42bd),          KEY_ESC},
        {(0x60df52ad),          KEY_STB},
        {(0x60df02fd),          KEY_RADIO},
        {(0x60dfc23d),          KEY_FAVORITES},
#if 1
        {(0x60dfa857),          KEY_RECORD},
        {(0x60df18e7),          KEY_PLAY},
        {(0x60df58a7),          KEY_FRAMEBACK},
        {(0x60dfd827),          KEY_FRAMEFORWARD},
        {(0x60df4ab5),          KEY_REVSLOW},
        {(0x60dfaa55),          KEY_SLOW},
        {(0x60dfe817),          KEY_STOP},
#endif
        {(0x60df40bf),          KEY_ZOOM},
        {(0x60df00ff),          KEY_EPG},
        {(0x60dfc03f),          KEY_LAST},
        {(0x60df609f),          KEY_RED},
        {(0x60df7887),          KEY_GREEN},
        {(0x60dff807),          KEY_YELLOW},
        {(0x60dfba45),          KEY_BLUE},

        {(0x60dfca35),          KEY_DVR_INFO},
        {(0x60dfb24d),          KEY_FILE},
        {(0x60df8a75),          KEY_DVRLIST},
        {(0x60df1ae5),          KEY_USBREMOVE},
        {(0x60df6a95),          KEY_PIP},
        {(0x60df9a65),          KEY_PIP_LIST},
        {(0x60df5aa5),          KEY_SWAP},
        {(0x60dfda25),          KEY_MOVE},
        {(0x60dfea15),          KEY_AB},
        {(0x60df28d7),          KEY_FAVUP},
        {(0x60df48b7),          KEY_FAVDOWN},

};

#endif

aui_hdl g_p_hdl_key;

#define TEST_KEY_FUNC 1

void receive_key(aui_key_info *p_key_info, void *pv_user_data)
{
	char *key_states[5] = {"UNUSE", "PRESS", "RELEASE", "UNUSE", "REPEAT"};
	AUI_PRINTF("Receive key form %s %s, 0x%08x, n_count: %d, pv_user_data: %p\n",
			p_key_info->e_type == AUI_KEY_TYPE_FRONTPANEL ? "Panel" : "IR",
			key_states[p_key_info->e_status],
			p_key_info->n_key_code,
			p_key_info->n_count,
			pv_user_data);
}

extern unsigned long test_key_callback_test(unsigned long *argc,char **argv,char *sz_out_put)
{
#ifdef TEST_KEY_FUNC
    g_p_hdl_key=NULL;
//    aui_find_dev_by_idx(AUI_MODULE_PANEL, 1, &g_p_hdl_key);
    if(g_p_hdl_key == NULL){
    	aui_key_open(0, NULL, &g_p_hdl_key);
    }
    AUI_PRINTF("*g_p_hdl_key: %p\n", g_p_hdl_key);
    AUI_TEST_CHECK_NULL((g_p_hdl_key));

#ifdef AUI_LINUX
    struct aui_key_map_cfg ir_cfg;
    ir_cfg.map_entry = (struct aui_key_map *)ali_key_map;
    ir_cfg.unit_num = sizeof(ali_key_map) / sizeof(struct aui_key_map);

    if (*argc > 0 && !strcmp(argv[0], "map")) {
    	AUI_PRINTF("aui_key_set_ir_map\n");
    	aui_key_set_ir_map(g_p_hdl_key, &ir_cfg);
    }  

    aui_key_set_ir_rep_interval(g_p_hdl_key, 1000, 1000);
#endif

    AUI_PRINTF("\n_register callback\n");
    AUI_TEST_CHECK_RTN(aui_key_callback_register(g_p_hdl_key,receive_key));
    AUI_SLEEP(10*1000);

    AUI_PRINTF("unregister callback\n");
    AUI_TEST_CHECK_RTN(aui_key_callback_register(g_p_hdl_key,NULL));

    AUI_TEST_CHECK_RTN(aui_key_close(g_p_hdl_key));
    g_p_hdl_key = NULL;

    if(!aui_test_user_confirm("do you see right key? "))
    {
    	return AUI_RTN_FAIL;
    }

#endif
    return AUI_RTN_SUCCESS;
}

#ifdef AUI_LINUX
extern int wait_ir()
{
#ifdef TEST_KEY_FUNC

	g_p_hdl_key=NULL;
	if(g_p_hdl_key == NULL){
		aui_key_open(0, NULL, &g_p_hdl_key);
	}
	AUI_TEST_CHECK_NULL((g_p_hdl_key));

	int i;
	aui_key_info key_info;
	do {
		if (aui_key_key_get(g_p_hdl_key, &key_info) == AUI_RTN_SUCCESS) {
			receive_key(&key_info, NULL);
			break;
		}
		AUI_SLEEP(100);
	} while(1);

	AUI_TEST_CHECK_RTN(aui_key_close(g_p_hdl_key));
	g_p_hdl_key = NULL;
	
#endif
		return AUI_RTN_SUCCESS;

}
#endif

extern unsigned long test_get_key_test(unsigned long *argc,char **argv,char *sz_out_put)
{
#ifdef TEST_KEY_FUNC
    g_p_hdl_key=NULL;
//    aui_find_dev_by_idx(AUI_MODULE_PANEL, 1, &g_p_hdl_key);
    if(g_p_hdl_key == NULL){
        aui_key_open(0, NULL, &g_p_hdl_key);
    }
    AUI_PRINTF("*g_p_hdl_key: %p\n", g_p_hdl_key);
    AUI_TEST_CHECK_NULL((g_p_hdl_key));

#ifdef AUI_LINUX
    struct aui_key_map_cfg ir_cfg;
    ir_cfg.map_entry = (struct aui_key_map *)ali_key_map;
    ir_cfg.unit_num = sizeof(ali_key_map) / sizeof(struct aui_key_map);

    if (*argc > 0 && !strcmp(argv[0], "map")) {
        AUI_PRINTF("aui_key_set_ir_map\n");
        aui_key_set_ir_map(g_p_hdl_key, &ir_cfg);
    }

    aui_key_set_ir_rep_interval(g_p_hdl_key, 1000, 1000);
#endif

    int i;
    aui_key_info key_info;
    for (i = 0; i < 100; i++) {
        if (aui_key_key_get(g_p_hdl_key, &key_info) == AUI_RTN_SUCCESS) {
            receive_key(&key_info, NULL);
        }
        AUI_SLEEP(100);
    }

    AUI_TEST_CHECK_RTN(aui_key_close(g_p_hdl_key));
    g_p_hdl_key = NULL;

    if(!aui_test_user_confirm("do you see right key? "))
    {
    	return AUI_RTN_FAIL;
    }

#endif
    return AUI_RTN_SUCCESS;
}

unsigned long test_key_help(unsigned long *argc,char **argv,char *sz_out_put)
{
        aui_print_help_header("\nKey Test Help");

        /* KEY_1_HELP */
        #define     KEY_1_HELP_PART1         "Press or release the STB remote-controller's key. The key value and key action are returned."
        #define     KEY_1_HELP_PART2         "The key action: '1' -- pressed     '2' -- released."
        #define     KEY_1_HELP_PART3         "The key test lasts about 10s. During the 10s, the various keys can be done the press and release test ."
        #define     KEY_1_HELP_PART4         "After 10s, the key test will be finished."
        #define     KEY_1_HELP_PART5         "With parameter \"map\", set the ir key map."
        #define     KEY_1_HELP_PART6         "Note: \n\t\tIf the test command is \"1 map\", you need to set panel key map first, \n\t\totherwise can't get panel key. (How to set panel key map? Run \"up\" - > \"panel\" -> \"8\") "

        aui_print_help_command("\'1\'");
        aui_print_help_instruction_newline("The Key of STB remote-controller test");
        aui_print_help_instruction_newline(KEY_1_HELP_PART1);
        aui_print_help_instruction_newline(KEY_1_HELP_PART2);
        aui_print_help_instruction_newline(KEY_1_HELP_PART3);
        aui_print_help_instruction_newline(KEY_1_HELP_PART4);
#ifndef AUI_TDS
        aui_print_help_instruction_newline(KEY_1_HELP_PART5);
        aui_print_help_instruction_newline(KEY_1_HELP_PART6);
#endif

        return AUI_RTN_HELP;
}


void test_key_reg()
{
	aui_tu_reg_group("key", "key test");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_key_callback_test, "key callback test");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_get_key_test, "Get test");

	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_key_help, "key help");
}

