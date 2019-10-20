/****************************INCLUDE HEAD FILE************************************/
#include "aui_gpio_test.h"
#include "aui_help_print.h"
#include <aui_common.h>

aui_hdl g_p_hdl_gpio;

/****************************EXTERN FUNC *****************************************/

/****************************TEST MODULE IMPLEMENT********************************/

unsigned long test_gpio_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nGPIO Test Help");

	/* GPIO_1_HELP */
	#define 	GPIO_1_HELP		"Before doing the other GPIO test items, the step of open the GPIO module should be executed first.\n"
	aui_print_help_command("\'open\'");
	aui_print_help_instruction_newline("Open the GPIO module.\n");	
	aui_print_help_instruction_newline(GPIO_1_HELP);	
	aui_print_help_instruction_newline("\tCommand : open [index],[dir],[value] \n\t\t\t[index]:\t0 ~ xxx, \n\t\t\t[dir]:\t\t0: input(irq)\t1:output \n\t\t\t[value/status]:\tif dir is output, value 0 or 1\n");
	aui_print_help_instruction_newline("\tExample:");
	aui_print_help_instruction_newline("\t open 4,0       -- mean open gpio index is 4, dir is input");		
	aui_print_help_instruction_newline("\t open 4,1,1     -- mean open gpio index is 4, dir is output, value is 1");	

	/* GPIO_2_HELP */
	#define 	GPIO_2_HELP 	"After Register GPIO callback function, Your function will be invoked when IRQ event has happened.\n"
	aui_print_help_command("\'regcb\'");	
	aui_print_help_instruction_newline("\tCommand : regcb [status],[interval] \n\t\t\t[status]:\t 1 - rising, 2 - falling, 3 - both;\n \t\t\t[interval]: < 0 is 0, 200 is recommend \n");
	aui_print_help_instruction_newline("\tExample:");
	aui_print_help_instruction_newline("\t regcb 1,200      -- mean register current gpio index's irq status is rising, ignore timeout is 200ms");		
	
	aui_print_help_instruction_newline("Register GPIO callback function.\n");
	aui_print_help_instruction_newline(GPIO_2_HELP);

	/* GPIO_3_HELP */
	#define 	GPIO_3_HELP	"After unregister GPIO callback function, no function will be invoked.\n"
	aui_print_help_command("\'unregcb\'");
	aui_print_help_instruction_newline("Unregister GPIO callback function.\n");
	aui_print_help_instruction_newline(GPIO_3_HELP);

	/* GPIO_4_HELP */
	#define 	GPIO_4_HELP 	"After open GPIO device and let it dir is OUT, then you can set a new value to it.\n"
	aui_print_help_command("\'set\'");	
	aui_print_help_instruction_newline("\tCommand : set [value] \n\t\t\t[value]:\t 0 - low, 1 - high\n");
	aui_print_help_instruction_newline("\tExample:");
	aui_print_help_instruction_newline("\t set 1       -- mean set current output gpio's value is high");		
		aui_print_help_instruction_newline(GPIO_4_HELP);

	/* GPIO_5_HELP */
	#define 	GPIO_5_HELP		"Close The GPIO device.\n"
	aui_print_help_command("\'close\'");
	aui_print_help_instruction_newline(GPIO_5_HELP);

	/* GPIO_6_HELP */
	#define 	GPIO_6_HELP		"Test GPIO IRQ Event.\n"
	aui_print_help_command("\'Example\'");
	aui_print_help_instruction_newline(GPIO_6_HELP);
	aui_print_help_instruction_newline("1.open 4,0\n");
	aui_print_help_instruction_newline("2.regcb 1\n");
	aui_print_help_instruction_newline("3.unregcb\n");
	aui_print_help_instruction_newline("4.close\n");

	return AUI_RTN_HELP;
}

unsigned long test_gpio_open(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);

	AUI_PRINTF("You input %d parameters: ", *argc);	
	if (*argc > 0)
		AUI_PRINTF("%s", argv[0]);
	if (*argc > 1)
		AUI_PRINTF(" %s", argv[1]);
	if (*argc > 2)
		AUI_PRINTF(" %s", argv[2]);

	AUI_PRINTF("\n");

    aui_gpio_attr attr_gpio;
	
	MEMSET(&attr_gpio,0,sizeof(aui_gpio_attr));

	if (*argc > 0) {
		attr_gpio.uc_dev_idx = atoi(argv[0]);
		
		AUI_PRINTF("You input index:%d\n", attr_gpio.uc_dev_idx);	
	} else {
		AUI_PRINTF("Must input index of GPIO.\n");
	}

	if (*argc > 1) {
		attr_gpio.io = atoi(argv[1]);
		AUI_PRINTF("You input gpio dir is %s\n", attr_gpio.io > 0 ? "out" : "in");	
		
	}
	
	if (attr_gpio.io && (*argc > 2)) {//out
		attr_gpio.value_out = atoi(argv[2]);			
		AUI_PRINTF("You input value:%d\n", attr_gpio.value_out);	
	}	

    AUI_TEST_CHECK_RTN(aui_gpio_open(&attr_gpio,&g_p_hdl_gpio));
    AUI_PRINTF("gpio device:%x\n",g_p_hdl_gpio);
	
    return AUI_RTN_SUCCESS;
}

unsigned long test_gpio_close(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_gpio_attr attr_gpio;
    memset(&attr_gpio, 0, sizeof(aui_gpio_attr));

	AUI_TEST_CHECK_NULL(sz_out_put);
	AUI_TEST_CHECK_NULL(g_p_hdl_gpio);

	/**  stop the gpio event check */
	aui_gpio_close(g_p_hdl_gpio);

	return AUI_RTN_SUCCESS;
}

void aui_gpio_irq_rising_cb(int gpio_index, aui_gpio_interrupt_type interrupt_type, void *pv_user_data)
{
    AUI_PRINTF("%s -> gpio_index: %u, gpio_status: %u, user_data = %s\n", __FUNCTION__, gpio_index, (int)interrupt_type, (char*)pv_user_data);
}

unsigned long test_gpio_reg_cb(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_gpio_interrupt_attr gpio_irq_cb_param;

	AUI_TEST_CHECK_NULL(sz_out_put);

	AUI_TEST_CHECK_NULL(g_p_hdl_gpio);

    AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);

	AUI_PRINTF("You input %d parameters: ", *argc);	
	if (*argc > 0)
		AUI_PRINTF("%s", argv[0]);
	if (*argc > 1)
		AUI_PRINTF(" %s", argv[1]);
	if (*argc > 2)
		AUI_PRINTF(" %s", argv[2]);

	AUI_PRINTF("\n");

	if (*argc > 1) {
		gpio_irq_cb_param.interrupt_type = ATOI(argv[0]);
		gpio_irq_cb_param.debounce_interval= ATOI(argv[1]);
		gpio_irq_cb_param.p_callback = aui_gpio_irq_rising_cb;
		gpio_irq_cb_param.pv_user_data = "test xxxxx edge";

		AUI_TEST_CHECK_RTN(aui_gpio_interrupt_reg(g_p_hdl_gpio, &gpio_irq_cb_param));
	} else {
		AUI_PRINTF("regcb [index],[interval] index, 1 - rising, 2 - falling, 3 - both, interval: -1 is not set, other interge\nExample:regcb 1,200");
		return AUI_RTN_FAIL;
	}
	
	return AUI_RTN_SUCCESS;
}

unsigned long test_gpio_unreg_cb(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_NULL(sz_out_put);

	AUI_TEST_CHECK_NULL(g_p_hdl_gpio);

	AUI_TEST_CHECK_RTN(aui_gpio_interrupt_unreg(g_p_hdl_gpio));
	
	return AUI_RTN_SUCCESS;
}

unsigned long test_gpio_set_value(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_NULL(sz_out_put);

	AUI_TEST_CHECK_NULL(g_p_hdl_gpio);

    AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);

	AUI_PRINTF("You input %d parameters: ", *argc);	
	if (*argc > 0)
		AUI_PRINTF("%s", argv[0]);

	AUI_PRINTF("\n");

	if (*argc > 0) {
		AUI_TEST_CHECK_RTN(aui_gpio_set_value(g_p_hdl_gpio, ATOI(argv[0])));
	} else {
		AUI_PRINTF("You must set the value of current gpio index, 0 - low, 1 - high\nExample:set 1");
		return AUI_RTN_FAIL;
	}
	return AUI_RTN_SUCCESS;
}

unsigned long test_gpio_get_value(unsigned long *argc,char **argv,char *sz_out_put)
{
	AUI_TEST_CHECK_NULL(sz_out_put);

	AUI_TEST_CHECK_NULL(g_p_hdl_gpio);

    AUI_TEST_CHECK_NULL(argc);
	AUI_TEST_CHECK_NULL(argv);

	aui_gpio_value value;

	AUI_TEST_CHECK_RTN(aui_gpio_get_value(g_p_hdl_gpio, &value));

	AUI_PRINTF("The gpio value is %x\n", value);
	
	return AUI_RTN_SUCCESS;
}

extern void aui_load_tu_gpio()
{
	aui_tu_reg_group("gpio", "gpio Test");
	{		
            aui_tu_reg_item(2, "open", AUI_CMD_TYPE_API, test_gpio_open, "open gpio");
            aui_tu_reg_item(2, "close", AUI_CMD_TYPE_UNIT, test_gpio_close, "close gpio");
            aui_tu_reg_item(2, "set", AUI_CMD_TYPE_API, test_gpio_set_value, "set gpio output value");			
            aui_tu_reg_item(2, "get", AUI_CMD_TYPE_UNIT, test_gpio_get_value, "get gpio input value");
            aui_tu_reg_item(2, "regcb", AUI_CMD_TYPE_UNIT, test_gpio_reg_cb, "test reg gpio callback");
            aui_tu_reg_item(2, "unregcb", AUI_CMD_TYPE_UNIT, test_gpio_unreg_cb, "test unreg gpio callback");		
            aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_gpio_help, "gpio test help");
    }
}

