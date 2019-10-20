#include <aui_common.h>
#include <aui_errno_sys.h>

#include <stdio.h>
#include <aui_i2c.h>
#include "aui_i2c_test.h"
//#include "unity_fixture.h"

unsigned long test_i2c_write_data(unsigned long *argc,char **argv,char *sz_out_put)
{
    //the default input for test
    aui_attr_i2c p_attr;
    aui_hdl p_i2c_handle = 0;
    memset(&p_attr, 0, sizeof(aui_attr_i2c));
    p_attr.uc_dev_idx = 1;                                     //the i2c bus device idx that chip mounted on
    unsigned long chip_addr = 0x20;                        //the address of slave device tuner_id = <M3031>, it can also be other device of the development board
    unsigned long sub_addr = 0x5C;                         //the register address in slave device
    unsigned char *buff = "abc def ghi";                  //the data to write to the device
    unsigned long buf_len = 11 ;                              //the length of the wrote data(byte)

    if (5 == *argc){
        p_attr.uc_dev_idx = atoi(argv[0]);
        chip_addr = atoi(argv[1]);
        sub_addr = atoi(argv[2]);
        buf_len= atoi(argv[3]);
        buff = argv[4];

    }

    /* check i2c if already opened */
    if (aui_find_dev_by_idx(AUI_MODULE_I2C, p_attr.uc_dev_idx, (aui_hdl *)&p_i2c_handle)) {
        if (aui_i2c_open( &p_attr, &p_i2c_handle )){
            AUI_PRINTF("open i2c device failed!\n");
            return AUI_RTN_FAIL;
        }
    }

    if (aui_i2c_write(p_i2c_handle, chip_addr, sub_addr, buff, buf_len)){
        AUI_PRINTF("fail to write data!\n");
        return AUI_RTN_FAIL;
    }

    if (aui_i2c_close(p_i2c_handle)){
        AUI_PRINTF("fail to close i2c device!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}

unsigned long test_i2c_write_no_subaddr(unsigned long *argc,char **argv,char *sz_out_put)
{
     //the default input for test
    aui_attr_i2c p_attr;
    aui_hdl p_i2c_handle = 0;
    memset(&p_attr, 0, sizeof(aui_attr_i2c));
    p_attr.uc_dev_idx = 4;                             //the i2c bus device idx that chip mounted on
    unsigned long chip_addr0 = 0x24;              //write 0x01 to this address to config the write and read function of panel
    unsigned long chip_addr = 0x35;               //the address of slave device panel dig1, it can also be other device of the development board
    unsigned char buff0 = 0x01;
    unsigned char buff = 0x7d;                         //the data to write to the device
    unsigned long buf_len0 = 1;
    unsigned long buf_len = 1 ;                       //the length of the wrote data(byte)
    unsigned char *buff_tem=NULL;

    if (4 == *argc){
        p_attr.uc_dev_idx = atoi(argv[0]);
        chip_addr = atoi(argv[1]);
        buf_len = atoi(argv[2]);
        buff_tem = argv[3];
        buff = *buff_tem;
    }

    /* check i2c if already opened */
    if (aui_find_dev_by_idx(AUI_MODULE_I2C, p_attr.uc_dev_idx, (aui_hdl *)&p_i2c_handle)) {
        if (aui_i2c_open( &p_attr, &p_i2c_handle )){
            AUI_PRINTF("open i2c device failed!\n");
            return AUI_RTN_FAIL;
        }
    }

    if (aui_i2c_write_no_subaddr(p_i2c_handle, chip_addr0, &buff0, buf_len0)){
        AUI_PRINTF("fail to write data!\n");
        return AUI_RTN_FAIL;
    }
    
    if (aui_i2c_write_no_subaddr(p_i2c_handle, chip_addr, &buff, buf_len)){
        AUI_PRINTF("fail to write data!\n");
        return AUI_RTN_FAIL;
    }

    if (aui_i2c_close(p_i2c_handle)){
        AUI_PRINTF("fail to close i2c device!\n");
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}


unsigned long test_i2c_read_data(unsigned long *argc,char **argv,char *sz_out_put)
{
    unsigned int i;
    aui_attr_i2c p_attr;
    aui_hdl p_i2c_handle = 0;
    memset(&p_attr, 0, sizeof(aui_attr_i2c));

    //the default input for test
    p_attr.uc_dev_idx = 1;                                    //the i2c bus device idx that chip mounted on
    unsigned long chip_addr = 0x20;                       //the address of slave device tuner_id = <M3031>, it can also be other device of the development board
    unsigned long sub_addr = 0x5c;                        //the register address in slave device
    unsigned char *buff = NULL;                            //the buffer to store the read data
    unsigned long buf_len = 11 ;                             //the length of the read data(byte)

    if (4 == *argc){
        p_attr.uc_dev_idx = atoi(argv[0]);
        chip_addr = atoi(argv[1]);
        sub_addr = atoi(argv[2]);
        buf_len = atoi(argv[3]) ;
    }

    buff = (unsigned char *)malloc(buf_len * sizeof(unsigned char));
    memset(buff, 0 , buf_len);

    /* check i2c if already opened */
    if (aui_find_dev_by_idx(AUI_MODULE_I2C, p_attr.uc_dev_idx, (aui_hdl *)&p_i2c_handle)) {
        if (aui_i2c_open( &p_attr, &p_i2c_handle )){
            free(buff);
            AUI_PRINTF("open i2c device failed!\n");
            return AUI_RTN_FAIL;
        }
    }

    if (aui_i2c_read(p_i2c_handle, chip_addr, sub_addr, buff, buf_len)){
        free(buff);
        AUI_PRINTF("fail to read data!\n");
        return AUI_RTN_FAIL;
    }

    for(i = 0 ; i < buf_len; i++){
        AUI_PRINTF("buff[%d]=%c\n",i,buff[i]);
    }

    if (aui_i2c_close(p_i2c_handle)){
        free(buff);
        AUI_PRINTF("fail to close i2c device!\n");
        return AUI_RTN_FAIL;
    }

    free(buff);
    return AUI_RTN_SUCCESS;
}

unsigned long test_i2c_read_no_subaddr(unsigned long *argc,char **argv,char *sz_out_put)
{
    unsigned int i;
    aui_attr_i2c p_attr;
    aui_hdl p_i2c_handle = 0;
    memset(&p_attr, 0, sizeof(aui_attr_i2c));

    //the default input for test
    p_attr.uc_dev_idx = 4;                                //the i2c bus device idx that chip mounted on
    unsigned long chip_addr = 0x35;                   //the address of slave device panel dig1, it can also be other device of the development board
    unsigned char *buff = NULL;                       //the buffer to store the read data
    unsigned long buf_len = 1 ;                           //the length of the read data(byte)

    if (3 == *argc){
        p_attr.uc_dev_idx = atoi(argv[0]);
        chip_addr = atoi(argv[1]);
        buf_len = atoi(argv[2]) ;
    }

    buff = (unsigned char *)malloc(buf_len * sizeof(unsigned char));
    memset(buff, 0 , buf_len);

    /* check i2c if already opened */
    if (aui_find_dev_by_idx(AUI_MODULE_I2C, p_attr.uc_dev_idx, (aui_hdl *)&p_i2c_handle)) {
        if (aui_i2c_open( &p_attr, &p_i2c_handle )){
            free(buff);
            AUI_PRINTF("open i2c device failed!\n");
            return AUI_RTN_FAIL;
        }
    }

    if (aui_i2c_read_no_subaddr(p_i2c_handle, chip_addr, buff, buf_len)){
        free(buff);
        AUI_PRINTF("fail to read data!\n");
        return AUI_RTN_FAIL;
    }

    for(i = 0 ; i < buf_len; i++) {
        AUI_PRINTF("buff[%d]=%c\n",i,buff[i]);
    }

    if (aui_i2c_close(p_i2c_handle)) {
        free(buff);
        AUI_PRINTF("fail to close i2c device!\n");
        return AUI_RTN_FAIL;
    }
    
    free(buff);
    return AUI_RTN_SUCCESS;
}

unsigned long test_i2c_write_read(unsigned long *argc,char **argv,char *sz_out_put)
{
    //the default input for test
    unsigned int i = 0;
    aui_attr_i2c p_attr;
    aui_hdl p_i2c_handle = 0;
    memset(&p_attr, 0, sizeof(aui_attr_i2c));
    p_attr.uc_dev_idx = 1;                                     //the i2c bus device idx that chip mounted on
    unsigned long chip_addr = 0x20;                        //the address of slave device tuner_id = <M3031>, it can also be other device of the development board
    unsigned long sub_addr = 0x5C;                         //the register address in slave device
    unsigned char *write_buff = "abc def ghi";         //the data to write to the device
    unsigned long write_buf_len = 11 ;                    //the length of the write data(byte)
    unsigned char *read_buff = NULL;                    //the buffer to store the read data
    unsigned long read_buf_len = 11 ;                     //the length of the read data(byte)

    if (6 == *argc) {
        p_attr.uc_dev_idx = atoi(argv[0]);
        chip_addr = atoi(argv[1]);
        sub_addr = atoi(argv[2]);
        write_buff = argv[3];
        write_buf_len= atoi(argv[4]);
        read_buf_len = atoi(argv[5]);
    }

    read_buff = (unsigned char *)malloc(read_buf_len * sizeof(unsigned char));
    memset(read_buff, 0 , read_buf_len);

    /* check i2c if already opened */
    if (aui_find_dev_by_idx(AUI_MODULE_I2C, p_attr.uc_dev_idx, (aui_hdl *)&p_i2c_handle)) {
        if (aui_i2c_open( &p_attr, &p_i2c_handle )){
            free(read_buff);
            AUI_PRINTF("open i2c device failed!\n");
            return AUI_RTN_FAIL;
        }
    }

    if (aui_i2c_write_read(p_i2c_handle, chip_addr, sub_addr, write_buff, write_buf_len, read_buff, read_buf_len)) {
     free(read_buff);
        AUI_PRINTF("fail to write data!\n");
        return AUI_RTN_FAIL;
    }

    for(i = 0 ; i < read_buf_len; i++) {
        AUI_PRINTF("buff[%d]=%c\n",i,read_buff[i]);
    }

    if (aui_i2c_close(p_i2c_handle)) {
     free(read_buff);
        AUI_PRINTF("fail to close i2c device!\n");
        return AUI_RTN_FAIL;
    }

    free(read_buff);
    return AUI_RTN_SUCCESS;
}

unsigned long test_i2c_write_read_no_subaddr(unsigned long *argc,char **argv,char *sz_out_put)
{
    //the default input for test
    unsigned int i =0;
    aui_attr_i2c p_attr;
    aui_hdl p_i2c_handle = 0;
    memset(&p_attr, 0, sizeof(aui_attr_i2c));
    p_attr.uc_dev_idx = 4;                                 //the i2c bus device idx that chip mounted on
    unsigned long chip_addr0 = 0x24;              //write 0x01 to this address to config the write and read function of panel
    unsigned long chip_addr = 0x36;                    //the address of slave device panel dig1, it can also be other device of the development board
    unsigned char write_buff0 = 0x01;
    unsigned char write_buff = 0x6f;                  //the data to be wrote to the device
    unsigned long write_buf_len0 = 1;
    unsigned long write_buf_len = 1 ;                  //the length of the wrote data(byte)
    unsigned char *read_buff = NULL;                //the buffer to store the read data
    unsigned long read_buf_len = 1 ;                   //the length of the read data(byte)
    unsigned char *buff_tem=NULL;

    if (5 == *argc) {
        p_attr.uc_dev_idx = atoi(argv[0]);
        chip_addr = atoi(argv[1]);
        buff_tem = argv[2];
        write_buff = *buff_tem;
        write_buf_len= atoi(argv[3]);
        read_buf_len = atoi(argv[4]);
    }

    read_buff = (unsigned char *)malloc(read_buf_len * sizeof(unsigned char));
    memset(read_buff, 0 , read_buf_len);

    /* check i2c if already opened */
    if (aui_find_dev_by_idx(AUI_MODULE_I2C, p_attr.uc_dev_idx, (aui_hdl *)&p_i2c_handle)) {
        if (aui_i2c_open( &p_attr, &p_i2c_handle )){
            free(read_buff);
            AUI_PRINTF("open i2c device failed!\n");
            return AUI_RTN_FAIL;
        }
    }

    if (aui_i2c_write_no_subaddr(p_i2c_handle, chip_addr0, &write_buff0, write_buf_len0)){
        free(read_buff);
        AUI_PRINTF("fail to write data!\n");
        return AUI_RTN_FAIL;
    }
    
    if (aui_i2c_write_read_no_subaddr(p_i2c_handle, chip_addr, &write_buff, write_buf_len, read_buff, read_buf_len)) {
        free(read_buff);
        AUI_PRINTF("fail to write data!\n");
        return AUI_RTN_FAIL;
    }

    for(i = 0 ; i < read_buf_len; i++) {
        AUI_PRINTF("buff[%d]=%c\n",i,read_buff[i]);
    }

    if (aui_i2c_close(p_i2c_handle)) {
        free(read_buff);
        AUI_PRINTF("fail to close i2c device!\n");
        return AUI_RTN_FAIL;
    }

    free(read_buff);
    return AUI_RTN_SUCCESS;
}



void aui_load_tu_i2c()
{
    aui_tu_reg_group("i2c", "i2c test cases");
    {
        aui_tu_reg_item(2, "1", AUI_CMD_TYPE_UNIT, test_i2c_write_data, "test i2c write data");
        aui_tu_reg_item(2, "2", AUI_CMD_TYPE_UNIT, test_i2c_read_data, "test i2c read data");
        aui_tu_reg_item(2, "3", AUI_CMD_TYPE_UNIT, test_i2c_write_no_subaddr, "test i2c write no subaddr");
        aui_tu_reg_item(2, "4", AUI_CMD_TYPE_UNIT, test_i2c_read_no_subaddr, "test i2c read no subaddr");
        aui_tu_reg_item(2, "5", AUI_CMD_TYPE_UNIT, test_i2c_write_read, "test i2c write read");
        aui_tu_reg_item(2, "6", AUI_CMD_TYPE_UNIT, test_i2c_write_read_no_subaddr, "test i2c write read no subaddr");
    }
}


