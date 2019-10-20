#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <aui_cic.h>
#include <aui_tsi.h>


/* sample code framework head files */
#include "aui_test_app_cmd.h"
#include "aui_test_app.h"
#include "aui_test_stream_cic_tsg.h"


#define CI "CIC"
//#define CU_ASSERT
#define ERR_FAILURE 1
#define RET_SUCCESS 0

#define CIC_MAX_SLOT_NUM    2
#define CIC_MSG_MAX_LEN     512

#define CIC_TOTAL_TEST
#ifdef CIC_TOTAL_TEST
/* Structure for Level 1 Version Information */
#define AUI_CIS_MAX_DEVICES            	   4
#define AUI_CIS_VERS_1_MAX_PROD_STRINGS    4
#define AUI_CIS_CIF_MAX_PROD_STRINGS       8
#define AUI_CIS_POWER_HIGHZ_OK        	0x01
#define AUI_CIS_POWER_HIGHZ_REQ    		0x02
#define AUI_CIS_IO_LINES_MASK    		0x1f
#define AUI_CIS_IO_MAX_WIN        		  16
#define AUI_CIS_MEM_MAX_WIN    			   8
#define AUI_CIS_CFTABLE_DEFAULT       0x0001
#define AUI_CIS_CFTABLE_BVDS          0x0002
#define AUI_CIS_CFTABLE_WP            0x0004
#define AUI_CIS_CFTABLE_RDYBSY        0x0008
#define AUI_CIS_CFTABLE_MWAIT         0x0010
#define AUI_CIS_CFTABLE_AUDIO         0x0800
#define AUI_CIS_CFTABLE_READONLY      0x1000
#define AUI_CIS_CFTABLE_PWRDOWN       0x2000

enum aui_pcmcia_cis_tuple_code
{
    AUI_CIS_NULL                 = 0x00,
    AUI_CIS_DEVICE                 = 0x01,
    AUI_CIS_LONGLINK_CB         = 0x02,
    AUI_CIS_INDIRECT             = 0x03,
    AUI_CIS_CONFIG_CB            = 0x04,
    AUI_CIS_CFTABLE_ENTRY_CB     = 0x05,
    AUI_CIS_LONGLINK_MFC         = 0x06,
    AUI_CIS_BAR                          = 0x07,
    AUI_CIS_PWR_MGMNT            = 0x08,
    AUI_CIS_EXTDEVICE                = 0x09,
    /* 0ah - 0fh reseved */
    AUI_CIS_CHECKSUM                 = 0x10,
    AUI_CIS_LONGLINK_A               = 0x11,
    AUI_CIS_LONGLINK_C            = 0x12,
    AUI_CIS_LINKTARGET               = 0x13,
    AUI_CIS_NO_LINK                  = 0x14,
    AUI_CIS_VERS_1                   = 0x15,
    AUI_CIS_ALTSTR                = 0x16,
    AUI_CIS_DEVICE_A                 = 0x17,
    AUI_CIS_JEDEC_C                  = 0x18,
    AUI_CIS_JEDEC_A                  = 0x19,
    AUI_CIS_CONFIG                = 0x1a,
    AUI_CIS_CFTABLE_ENTRY        = 0x1b,
    AUI_CIS_DEVICE_OC                = 0x1c,
    AUI_CIS_DEVICE_OA                = 0x1d,
    AUI_CIS_DEVICEGEO            = 0x1e,
    AUI_CIS_DEVICEGEO_A              = 0x1f,
    AUI_CIS_MANFID                   = 0x20,
    AUI_CIS_FUNCID                   = 0x21,
    AUI_CIS_FUNCE                = 0x22,
    AUI_CIS_SWIL                         = 0x23,
    /* 24 - 3fh reseved */
    AUI_CIS_VERS_2                   = 0x40,
    AUI_CIS_FORMAT                   = 0x41,
    AUI_CIS_GEOMETRY            = 0x42,
    AUI_CIS_BYTEORDER                = 0x43,
    AUI_CIS_DATE                         = 0x44,
    AUI_CIS_BATTERY                  = 0x45,
    AUI_CIS_ORG                    = 0x46,
    AUI_CIS_FORMAT_A                 = 0x47,
    /* 80h - 8fh Vendor unique tuples */
    AUI_CIS_CPCL                         = 0x90,
    /* 90h - feh Reseved */
    AUI_CIS_END                          = 0xff
};

enum stci_ifn_code
{
    IFN_ZOOMVIDEO        = 0x0141,
    IFN_DVBCI            = 0x0241,
    IFN_OPENCABLEPOD    = 0x0341
};

struct aui_cis_device
{
    unsigned char    ndev;
    struct
    {
        unsigned char                type;
        unsigned char                wp;
        unsigned int                speed;
        unsigned int                size;
    } dev[AUI_CIS_MAX_DEVICES];
};

struct aui_cis_device_o
{
    unsigned char                mwait;
    unsigned char                vcc_used;
    struct aui_cis_device        device;
};

struct aui_cis_vers_1
{
    unsigned char    major;
    unsigned char    minor;
    unsigned char    ns;
    unsigned char    ofs[AUI_CIS_VERS_1_MAX_PROD_STRINGS];
    char        str[254];
    unsigned char    compatible;
};

struct aui_cis_manfid
{
    unsigned short    manf;
    unsigned short    card;
};

struct ccstpl_cif
{
    unsigned int    ifn;
    unsigned char    ns;
    unsigned char    ofs[AUI_CIS_CIF_MAX_PROD_STRINGS];
    char        str[128];
};

struct aui_cis_config
{
    unsigned char                last_idx;
    unsigned int                base;
    unsigned int                rmask[4];
    unsigned char                subtuples;
    struct ccstpl_cif        cif_infor;
};

struct aui_cis_power
{
    unsigned char    present;
    unsigned char    flags;
    unsigned int    param[7];
};

struct aui_cis_timing
{
    unsigned int    wait, waitscale;
    unsigned int    ready, rdyscale;
    unsigned int    reserved, rsvscale;
};

struct aui_cis_io
{
    unsigned char    flags;
    unsigned char    nwin;
    struct
    {
        unsigned int    base;
        unsigned int    len;
    } win[AUI_CIS_IO_MAX_WIN];
};

struct aui_cis_irq
{
    unsigned int    irqinfo1;
    unsigned int    irqinfo2;
};

struct aui_cis_mem
{
    unsigned char    flags;
    unsigned char    nwin;
    struct
    {
        unsigned int    len;
        unsigned int    card_addr;
        unsigned int    host_addr;
    } win[AUI_CIS_MEM_MAX_WIN];
};

struct aui_cis_cftable_entry
{
    unsigned char                    index;
    unsigned short                    flags;
    unsigned char                    interface;
    struct aui_cis_power        vcc, vpp1, vpp2;
    struct aui_cis_timing        timing;
    struct aui_cis_io            io;
    struct aui_cis_irq            irq;
    struct aui_cis_mem            mem;
    unsigned char                    subtuples;
};

struct aui_cis_slot
{
    /* Card Information Structure */
    struct aui_cis_device_o        device_infor;
    struct aui_cis_vers_1        version_infor;
    struct aui_cis_manfid        manid_infor;
    struct aui_cis_config        config_infor;
    struct aui_cis_cftable_entry    cfgtable_infor;
};

struct aui_cis_tuple
{
    unsigned char        tuple_code;
    unsigned char        tuple_link;
    unsigned char        tuple_data_max;
    unsigned char        tuple_data_len;
    unsigned char        *tuple_data;
};
#endif

static void show_cic_usage()
{
    AUI_PRINTF("command as fallow:\n");
    AUI_PRINTF("cmd_num [path],[vpid],[apid],[ppid],[video format],[audio format],[dis format]\n");
    AUI_PRINTF("such as :4 /mnt/uda1/tvstream.ts,234,235,234,0,1,0");
    AUI_PRINTF("if you need more information for help,please input 'h' and enter\n");
}

#define CI_IO_DATA_LENGTH 8
#define CI_ATTRMEM_DATA_LENGTH 16
#define CI_BLOCK_DATA_LENGTH 64

static unsigned char  g_ali_cic_block_data[CI_BLOCK_DATA_LENGTH] = {
    0x64, 0x38, 0x39, 0x40, 0x41, 0x42, 0x43, 0x44,
    0x65, 0x37, 0x18, 0x19, 0x20, 0x21, 0x22, 0x45,
    0x63, 0x36, 0x17, 0x06, 0x07, 0x08, 0x23, 0x46,
    0x62, 0x35, 0x16, 0x05, 0x02, 0x09, 0x24, 0x47,
    0x61, 0x34, 0x15, 0x04, 0x03, 0x10, 0x25, 0x48,
    0x60, 0x33, 0x14, 0x13, 0x12, 0x11, 0x26, 0x49,
    0x59, 0x32, 0x31, 0x30, 0x29, 0x28, 0x27, 0x50,
    0x58, 0x57, 0x56, 0x55, 0x54, 0x53, 0x52, 0x51
};
/*
static unsigned char  g_ali_cic_attrmem_data[CI_ATTRMEM_DATA_LENGTH] = {
    0x88, 0x22, 0x66, 0x44, 0x44, 0x66, 0x22,   0x88,
    0x11, 0x77, 0x33, 0x55, 0x55, 0x33, 0x77,   0x11
};
*/
static aui_hdl cic_hdl = NULL;

static aui_cic_hw_status_cb p_user_callback = NULL;

//static unsigned char cam_present_status[CIC_MAX_SLOT_NUM] = {0, 0};

static unsigned long test_cic_rw_io_data(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    int i = 0;
    (void)argc;
    (void)argv;
    (void)sz_out_put;
    printf("write IO data: %d,as follow:\n",CI_BLOCK_DATA_LENGTH);
    for(i = 0; i<CI_BLOCK_DATA_LENGTH; i++) {
        printf("\t%02x", g_ali_cic_block_data[i]);
        if(i%8 == 0)
            printf("\n");
    }
    printf("\n");
    ret =aui_cic_write_io_data(cic_hdl, 0, CI_BLOCK_DATA_LENGTH, \
                               g_ali_cic_block_data);
    if(ret) {
        printf(",[MODULE:%s,func:%s]:Write %d io data error!\n",CI,__func__,\
               CI_BLOCK_DATA_LENGTH);
    }
    printf(",[MODULE:%s,func:%s]:Write %d io data success!\n",CI,__func__,\
           CI_BLOCK_DATA_LENGTH);

    unsigned char temp[CI_BLOCK_DATA_LENGTH];
    memset(temp, 0, CI_BLOCK_DATA_LENGTH);
    ret =aui_cic_read_io_data(cic_hdl, 0, CI_BLOCK_DATA_LENGTH, temp);
    if(ret) {
        printf(",[MODULE:%s,func:%s]:Read %d io data error!\n",CI,__func__,\
               CI_BLOCK_DATA_LENGTH);
    }
    printf(",[MODULE:%s,func:%s]:Write %d io data success!\n",CI,__func__,\
           CI_BLOCK_DATA_LENGTH);

    printf("Read IO data: %d,as follow:\n",CI_BLOCK_DATA_LENGTH);
    for(i = 0; i<CI_BLOCK_DATA_LENGTH; i++) {
        printf("\t%02x", temp[i]);
        if(i%8 == 0)
            printf("\n");
    }
    printf("\n");

    return ret;
}

static unsigned long test_cic_rw_mem(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    int i = 0;
    (void)argc;
    (void)argv;
    (void)sz_out_put;

	unsigned int   cis_addr = 0;  
	unsigned char   buf[255] = {0};
	struct aui_cis_tuple tuple;
	
	#if 0
    printf("write memory data: %d,as follow:\n",CI_BLOCK_DATA_LENGTH);
    for(i = 0; i<CI_BLOCK_DATA_LENGTH; i++) {
        printf("\t%02x", g_ali_cic_block_data[i]);
        if(i%8 == 0)
            printf("\n");
    }
    printf("\n");
    ret = aui_cic_write_mem(cic_hdl, 0, CI_BLOCK_DATA_LENGTH, \
                            0,g_ali_cic_block_data);
    if(ret) {
        printf(",[MODULE:%s,func:%s]:Write %d memory data error!\n",CI,__func__,\
               CI_BLOCK_DATA_LENGTH);
    }
    printf(",[MODULE:%s,func:%s]:Write %d memory data success!\n",CI,__func__,\
           CI_BLOCK_DATA_LENGTH);
    unsigned char temp[CI_BLOCK_DATA_LENGTH];
    memset(temp, 0, CI_BLOCK_DATA_LENGTH);
    ret = aui_cic_read_mem(cic_hdl, 0, CI_BLOCK_DATA_LENGTH, 0, temp);
    if(ret) {
        printf(",[MODULE:%s,func:%s]:Read %d memory data error!\n",CI,__func__,\
               CI_BLOCK_DATA_LENGTH);
    }
    printf(",[MODULE:%s,func:%s]:Write %d memory data success!\n",CI,__func__,\
           CI_BLOCK_DATA_LENGTH);

    printf("Read memory data: %d,as follow:\n",CI_BLOCK_DATA_LENGTH);
    for(i = 0; i<CI_BLOCK_DATA_LENGTH; i++) {
        printf("\t%02x", temp[i]);
        if(i%8 == 0)
            printf("\n");
    }
    printf("\n");
	#endif

	MEMSET(&tuple, 0, sizeof(struct aui_cis_tuple));
	tuple.tuple_data = buf;
	tuple.tuple_data_max = sizeof(buf);

	while (1) {
		ret = aui_cic_read_mem(cic_hdl,0,1,cis_addr,&(tuple.tuple_code));
	    if(ret) {
	        printf("[MODULE:%s,func:%s]:Read attribute memory data error!\n",CI,__func__);
            return -1;
	    }
		if (tuple.tuple_code == AUI_CIS_END)
	    {
	        printf("[MODULE:%s,func:%s]:parse cis finished!\n", CI,__func__);
	        return 0;
	    }
		cis_addr++;
		ret = aui_cic_read_mem(cic_hdl,0,1,cis_addr,&(tuple.tuple_link));
	    if(ret) {
	        printf("[MODULE:%s,func:%s]:Read attribute memory data error!\n",CI,__func__);
	    }
		tuple.tuple_data_len = tuple.tuple_link;
		if (tuple.tuple_data_len > tuple.tuple_data_max)
	    {
	        printf("[MODULE:%s,func:%s]: CAM too long tuple\n",CI,__func__);
	        return -1;
	    }
		cis_addr++;
		ret = aui_cic_read_mem(cic_hdl,0,tuple.tuple_data_len,cis_addr,tuple.tuple_data);
	    if(ret) {
	        printf("[MODULE:%s,func:%s]:Read attribute memory data error!\n",CI,__func__);
            return -1;
	    }
		cis_addr += tuple.tuple_data_len;

		switch (tuple.tuple_code)
		{
			case AUI_CIS_DEVICE_OA:
			case AUI_CIS_DEVICE_OC:
	            printf("[MODULE:%s,func:%s]:Found a AUI_CIS_DEVICE_OA/OC=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
			case AUI_CIS_VERS_1:		
	            printf("[MODULE:%s,func:%s]:Found a AUI_CIS_VERS_1=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
			case AUI_CIS_MANFID:
	            printf("[MODULE:%s,func:%s]:Found a AUI_CIS_MANFID=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
			case AUI_CIS_CONFIG:
	            printf("[MODULE:%s,func:%s]:Found a AUI_CIS_CONFIG=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
			case AUI_CIS_CFTABLE_ENTRY:
	            printf("[MODULE:%s,func:%s]:Found a AUI_CIS_CFTABLE_ENTRY=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
			case AUI_CIS_END:
	            printf("[MODULE:%s,func:%s]:Found a AUI_CIS_END=0x%02x\n", CI,__func__,tuple.tuple_code);
				return 0;
			case AUI_CIS_NO_LINK:
				printf("[MODULE:%s,func:%s]:Found a AUI_CIS_NO_LINK=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
			default:
	            printf("[MODULE:%s,func:%s]:Found a un-process tuple=0x%02x\n", CI,__func__,tuple.tuple_code);
				break;
		}

		printf("Tuple Code: %02x, Tuple Link: %02x\n", tuple.tuple_code, tuple.tuple_link);
		for (i = 0; i < tuple.tuple_data_len; i++)
		{
			printf("%02x ", tuple.tuple_data[i]);
		}
		printf("\n\n");
	}
    return 0;
}



/*block data R/W testing*/
/*CA_S_MSG:AUI_CIC_DATA/AUI_CIC_CSR/AUI_CIC_SIZELS/AUI_CIC_SIZEMS*/
static unsigned long test_cic_rw_io_reg(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned char  data[2];
    aui_cic_reg reg = 0;
    int ret = 0, count = 0;

    memset(data, 0, 2);

    (void)argc;
    (void)argv;
    (void)sz_out_put;
#if 0
    /*command & status register test*/
    reg = AUI_CIC_DATA;
    data[0] = 0xCD;
    printf("Write AUI_CIC_DATA register:%d,value = %x\n", reg, data[0]);
    ret  = aui_cic_write_io_reg(cic_hdl, 0, reg, data[0]);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Write AUI_CIC_DATA Register OK  !\n",
           CI, __func__);

    data[0] = 0x00;
    ret  = aui_cic_read_io_reg(cic_hdl, 0, reg, data);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Read AUI_CIC_DATA Register OK  !\n",
           CI, __func__);
    if (data[0] == 0xCD) {
        printf("[MODULE:%s] %s : Slot AUI_CIC_DATA Register RW Check OK !\n",
               CI, __func__);
    } else {
        printf("[MODULE:%s] %s : Slot AUI_CIC_DATA Register RW Check Failure !\n",
               CI, __func__);
        //CU_ASSERT(CU_FALSE);
    }
#endif
    /*command & status register test*/
    reg = AUI_CIC_CSR;
    // the CI spec indicates the reset cmd is 0x08.
    data[0] = 0x08;//0xC8;//0xAB;
    printf("Write AUI_CIC_CSR register:%d,value = %x\n", reg, data[0]);
    ret  = aui_cic_write_io_reg(cic_hdl, 0, reg, data[0]);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Write AUI_CIC_CSR Register OK  !\n",
           CI, __func__);

    AUI_SLEEP(200);
	   
    /*data[0] = 0x00;
    printf("Write AUI_CIC_CSR register:%d,value = %x\n", reg, data[0]);
    ret  = aui_cic_write_io_reg(cic_hdl, 0, reg, data[0]);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Write AUI_CIC_CSR Register OK  !\n",
           CI, __func__);*/

	count = 0;
	while(1)
    {
        ret = aui_cic_read_io_reg(cic_hdl, 0, reg, data);
    	if (ret) {
        	printf("[MODULE:%s] %s : slot 0's request error !\n",
               	CI, __func__);
    	}
    	printf("[MODULE:%s] %s : Read AUI_CIC_CSR Register OK (0x%02x) !\n",
           CI, __func__, data[0]);
        // the CI spec indicates: when reset success the bit 6 is set to 1.
        if (data[0] & 0x40/*CIC_FR*/)
        {
           printf("[MODULE:%s] %s : Slot AUI_CIC_CSR Register RW Check OK !\n",
               CI, __func__);
            break;
        }
		printf("[MODULE:%s] %s : Slot AUI_CIC_CSR Register RW Check Failure !\n",
               CI, __func__);
        if(count == 100)
        {
            printf("[MODULE:%s] %s CAM Reset wait FREE timeout!\n", CI, __func__);
            return  AUI_RTN_FAIL;
        }
        count ++;
        AUI_SLEEP(100);        
    }
	
#if 0
    /*command & status register test*/
    reg = AUI_CIC_SIZELS;
    data[0] = 0x11;
    printf("Write AUI_CIC_SIZELS register:%d,value = %x\n", reg, data[0]);
    ret  = aui_cic_write_io_reg(cic_hdl, 0, reg, data[0]);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Write AUI_CIC_SIZELS Register OK  !\n",
           CI, __func__);

    data[0] = 0x00;
    ret = aui_cic_read_io_reg(cic_hdl, 0, reg, data);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Read AUI_CIC_SIZELS Register OK	!\n",
           CI, __func__);

    if (data[0] == 0x11) {
        printf("[MODULE:%s] %s : Slot AUI_CIC_SIZELS Register RW Check OK !\n",
               CI, __func__);
    } else {
        printf("[MODULE:%s] %s : Slot AUI_CIC_SIZELS Register RW Check Failure !\n",
               CI, __func__);
    }

    /*command & status register test*/
    reg = AUI_CIC_SIZEMS;
    data[0] = 0xDB;
    printf("Write AUI_CIC_SIZEMS register:%d,value = %x\n", reg, data[0]);
    ret  = aui_cic_write_io_reg(cic_hdl, 0, reg, data[0]);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Write AUI_CIC_SIZEMS Register OK  !\n",
           CI, __func__);

    data[0] = 0x00;
    ret = aui_cic_read_io_reg(cic_hdl, 0, reg, data);
    if (ret) {
        printf("[MODULE:%s] %s : slot 0's request error !\n",
               CI, __func__);
    }
    printf("[MODULE:%s] %s : Read AUI_CIC_SIZEMS Register OK	!\n",
           CI, __func__);

    if (data[0] == 0xDB) {
        printf("[MODULE:%s] %s : Slot AUI_CIC_SIZEMS Register RW Check OK !\n",
               CI, __func__);
    } else {
        printf("[MODULE:%s] %s : Slot AUI_CIC_SIZEMS Register RW Check Failure !\n",
               CI, __func__);
    }
#endif
    return ret;
}

static unsigned long test_cic_pass_stream(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned int pass = 1;
    int ret = 0;

    ret = aui_tsi_ci_card_bypass_set(cic_hdl, 0, pass);

    if (ret) {
        printf("[MODULE:%s,func:%s] slot 0's status unknown !\n",
               CI, __func__);
    }
    printf("Set  CI card pass stream success!\n");

    return ret;
}

static unsigned long test_cic_bypass_stream(unsigned long *argc, char **argv, char *sz_out_put)
{
    unsigned int pass = 0;
    int ret = 0;

    ret = aui_tsi_ci_card_bypass_set(cic_hdl, 0, pass);

    if (ret) {
        printf("[MODULE:%s,func:%s] : slot 0's status unknown !\n",
               CI, __func__);
    }
    printf("Set CI card bypass stream success!\n");

    return ret;
}

static unsigned long test_cic_slot_init(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    int slot = 0;

    (void)argc;
    (void)argv;
    (void)sz_out_put;

    int detect = 0;
    int ready = 0;

    /* detect cic devcie is present or absent */
    ret = aui_cic_detect_cam(cic_hdl, 0, &detect);
    if(ret) {
        AUI_PRINTF("CIC device detect error!\n");
        goto cic_err1;
    }
    if(0 == detect) {
        AUI_PRINTF("M3733 demo don't have CAM!\n");
        goto cic_err1;
    } else {
        AUI_PRINTF("M3733 demo have CAM!\n");
    }

    ret = aui_cic_enable_cam(cic_hdl, 0);
    if(ret) {
        printf("[MODULE:%s] %s : slot%d's enable request error !\n",
               CI, __func__, slot);
        goto cic_err1;
    }

    /* reset cic device */
    ret = aui_cic_reset_cam(cic_hdl,0);
    if(ret) {
        AUI_PRINTF("Reset CAM error!\n");
        goto cic_err1;
    }
    AUI_PRINTF("Reset CAM success!\n");

	AUI_SLEEP(3000); 

    /* detect cic device is ready or not */
    ret = aui_cic_cam_is_ready(cic_hdl, 0, &ready);
    if(ret) {
        AUI_PRINTF("Detect CAM ready error!\n");
        goto cic_err1;
    }

    printf("Ready = %d\n",ready);
    if(0 == ready) {
        AUI_PRINTF("CAM isn't ready!\n");
        goto cic_err1;
    } else {
        AUI_PRINTF("CAM is ready!\n");
    }

    return 0;

cic_err1:
    return 1;
}

static void user_callback(int slot)
{
    printf("[MODULE:%s] %s : calling user callback !\n",
           CI, __func__);
    printf(" slot = %d\n",slot);

    int ret = 0;
    int detect = 0;
    int ready = 0;

    /* detect cic devcie is present or absent */
    ret = aui_cic_detect_cam(cic_hdl, 0, &detect);
    if(ret) {
        AUI_PRINTF("CIC device detect error!\n");
        return;
    }
    if(0 == detect) {
        AUI_PRINTF("detach CAM card!\n");
        return;
    } else {
        AUI_PRINTF("insert CAM card!\n");
    }

    ret = aui_cic_enable_cam(cic_hdl, 0);
    if(ret) {
        printf("[MODULE:%s] %s : slot%d's enable request error !\n",
               CI, __func__, slot);
        return;
    }

    /* reset cic device */
    ret = aui_cic_reset_cam(cic_hdl,0);
    if(ret) {
        AUI_PRINTF("Reset CAM error!\n");
        return;
    }
    AUI_PRINTF("Reset CAM success!\n");

    /* detect cic device is ready or not */
    ret = aui_cic_cam_is_ready(cic_hdl, 0, &ready);
    if(ret) {
        AUI_PRINTF("Detect CAM ready error!\n");
        return;
    }

    printf("Ready = %d\n",ready);
    if(0 == ready) {
        AUI_PRINTF("CAM isn't ready!\n");
        return;
    } else {
        AUI_PRINTF("CAM is ready!\n");
    }
}

static unsigned long test_ci_init(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    (void)argc;
    (void)argv;
    (void)sz_out_put;

    /* init user data */
    ret = aui_cic_init(NULL);
    if(ret) {
        printf("[MODULE:%s] %s : init user data error !\n",
               CI, __func__);
        goto cic_err2;
    }

    /* open ali cic devcie*/
    ret = aui_cic_open(p_user_callback, &cic_hdl);
    if (!cic_hdl) {
        printf("[MODULE:%s] %s : device ali_cic open error !\n",
               CI, __func__);
        goto cic_err1;
    } else {
        printf("[MODULE:%s] %s : cic device handle 0x%08x,cic open success\n",
               CI, __func__, cic_hdl);
    }

    return ret;
cic_err1:
    if(cic_hdl) {
        ret = aui_cic_close(cic_hdl);
        if(ret) {
            printf("[MODULE:%s] %s : close CIC device error!\n",
                   CI, __func__, cic_hdl);
        }
    }
cic_err2:
    ret = aui_cic_de_init(NULL);
    if(ret) {
        AUI_PRINTF("deinit CIC device error!");
    }
    return ret;
}

static unsigned long test_ci_deinit(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    (void)argc;
    (void)argv;
    (void)sz_out_put;

    if(cic_hdl) {
        ret = aui_cic_close(cic_hdl);
        if(ret) {
            printf("[MODULE:%s] %s : close CIC device error!\n",
                   CI, __func__, cic_hdl);
        }
        printf("[MODULE:%s] %s : close CIC device success!\n",CI, __func__);
        cic_hdl = NULL;
    }

    ret = aui_cic_de_init(NULL);
    if(ret) {
        printf("deinit CIC device error!");
    } else {
        printf("deinit user data success!");
    }

    return ret;
}

static unsigned long test_cic_recmsg(unsigned long *argc, char **argv, char *sz_out_put)
{
    int ret = 0;
    (void)argc;
    (void)argv;
    (void)sz_out_put;
    p_user_callback = user_callback;

    return 0;

}

#ifdef CIC_TOTAL_TEST
static const unsigned char mantissa[] = {
    10, 12, 13, 15, 20, 25, 30, 35,
    40, 45, 50, 55, 60, 70, 80, 90
};

static const unsigned int exponent[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
};

/* Convert an Extended Speed Byte to a Time in Nanoseconds */
#define SPEED_CVT(v)        (mantissa[(((v) >> 3) & 15) - 1] * exponent[(v) & 7] / 10)
/* Convert a Power Byte to a Current in 0.1 Microamps */
#define POWER_CVT(v)    (mantissa[((v) >> 3) & 15] * exponent[(v) & 7] / 10)
#define POWER_SCALE(v)    (exponent[(v) & 7])

#undef le16_to_cpu
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define le16_to_cpu(a)        ((((a) >> 8) & 0xff) | (((a) & 0xff) << 8))
#else
#define le16_to_cpu(a)        (a)
#endif

static  struct aui_cis_slot  cis_slot_info;

/* Parse Strings */
static int parse_strings(unsigned char *p, unsigned char *q, int max, char *s, unsigned char *ofs, unsigned char *found)
{
    int i, j, ns;

    if (p == q)
    {
        return -1;
    }

    ns = 0; j = 0;
    for (i = 0; i < max; i++)
    {
        if (*p == 0xff)
        {
            break;
        }
        ofs[i] = j;    /* Starting Point of Every String */
        ns++;
        for (;;)
        {
                s[j++] = (*p == 0xff) ? '\0' : *p;    /* Return Strings */
                if ((*p == '\0') || (*p == 0xff))
            {
                break;
                }
                if (++p == q)
            {
                return -1;
                }
        }
        if ((*p == 0xff) || (++p == q))
        {
            s[j] = '\0';
            break;
        }
        }
    if (found)
    {
        *found = ns;        /* Number of Strings */
        return -1;
    }
    else
    {
        return (ns == max) ? 0 : -1;
    }
}

/* Parse Config Information */
static int parse_config(struct aui_cis_tuple *tuple, struct aui_cis_config *config)
{
    int rasz, rmsz, i, ifnsz;
    unsigned char *p, *q;

    p = (unsigned char *)tuple->tuple_data;
    q = p + tuple->tuple_data_len;
    rasz = *p & 0x03;            /* Number of Bytes of TPCC_RADR - 1 */
    rmsz = (*p & 0x3c) >> 2;    /* Number of Bytes TPCC_RMSK field - 1 */
    if (tuple->tuple_data_len < rasz + rmsz + 4)
    {
    	return -1;
    }
    config->last_idx = *(++p);    /* Index Number of Final Entry in the Card Configuration Table */
    p++;
    config->base = 0;        /* TPCC_RADR: Configuration Registers Base Address */
    for (i = 0; i <= rasz; i++)
    {
    config->base += p[i] << (8 * i);
    }
    p += rasz + 1;
    for (i = 0; i < 4; i++)        /* TPCC_RMSK: Configuration Registers Present Mask */
    {
        config->rmask[i] = 0;
    }
        for (i = 0; i <= rmsz; i++)
    {
        config->rmask[i >> 2] += p[i] << (8 * (i % 4));
    }
    config->subtuples = tuple->tuple_data_len - (rasz + rmsz + 4);    /* Data Length of Sub-Tuples */

    /* Note: Add Code for Sub-Tuple of AUI_CIS_CIF in DVB CAM Applicatoins */
    if (config->subtuples != 0)
    {
        p += (rmsz + 1);
        if (*p == 0xc0)
        {
            p += 2;
            ifnsz = (*p >> 6) & 3;        /* STCI_IFN_SIZE: Number of Bytes in Custom Interface ID Number */
            config->cif_infor.ifn = 0;    /* Interface ID Number */
            for (i = 0; i <= ifnsz; i++)
            {
                config->cif_infor.ifn += p[i] << (8 * i);
            }
            p += (ifnsz + 1);
            /* Interface Description Strings */
            parse_strings(p, q, AUI_CIS_CIF_MAX_PROD_STRINGS, config->cif_infor.str, config->cif_infor.ofs, &(config->cif_infor.ns));
            config->cif_infor.ns++;    /* Interface Description String Number */
        }
    }

    return 0;
}

/* Parse Power Description Structure of Config Table Entry Information */
static unsigned char * parse_power(unsigned char *p, unsigned char *q, struct aui_cis_power *pwr)
{
    int i;
    unsigned int scale;

    if (p == q)
    {
        return NULL;
    }
    pwr->present = *p;
    pwr->flags = 0;
    p++;

    /* Param Definitions */
    for (i = 0; i < 7; i++)
    {
    if (pwr->present & (1 << i))
    {
            if (p == q)
        {
            return NULL;
        }
            pwr->param[i] = POWER_CVT(*p);
            scale = POWER_SCALE(*p);
            while (*p & 0x80)
        {
            if (++p == q)
            {
                return NULL;
            }
            if ((*p & 0x7f) < 100)    /* Binary Value for Next Two Decimal Digits to Right of Current Value */
                {
                    pwr->param[i] += (*p & 0x7f) * scale / 100;
            }
            else if (*p == 0x7d)    /* High Impedance OK During Sleep or Power-Down only */
                {
                    pwr->flags |= AUI_CIS_POWER_HIGHZ_OK;
            }
            else if (*p == 0x7e)    /* Zero Value */
                {
                    pwr->param[i] = 0;
            }
            else if (*p == 0x7f)    /* High Impedance Required */
                {
                    pwr->flags |= AUI_CIS_POWER_HIGHZ_REQ;
            }
            else
                    return NULL;
            }
            p++;
    }
    }
    return p;
}

/* Parse Configuration Timing Information of Config Table Entry Information */
static unsigned char * parse_timing(unsigned char *p, unsigned char *q, struct aui_cis_timing *timing)
{
    unsigned char scale;

    if (p == q)
    {
        return NULL;
    }

    scale = *p;
    if ((scale & 3) != 3)    /* MAX Wait Time and Wait Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->wait = SPEED_CVT(*p);
        timing->waitscale = exponent[scale & 3];
    }
    else
    {
        timing->wait = 0;
    }

    scale >>= 2;
    if ((scale & 7) != 7)    /* MAX Ready Time and Ready Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
            timing->ready = SPEED_CVT(*p);
            timing->rdyscale = exponent[scale & 7];
        }
    else
    {
        timing->ready = 0;
    }

    scale >>= 3;
    if (scale != 7)            /* Reserved Time Definition and Reserved Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->reserved = SPEED_CVT(*p);
        timing->rsvscale = exponent[scale];
    }
    else
    {
        timing->reserved = 0;
    }

    p++;
    return p;
}

/* Parse I/O Space Addresses Required For This Configuration of Config Table Entry Information */
static unsigned char * parse_io(unsigned char *p, unsigned char *q, struct aui_cis_io *io)
{
    int i, j, bsz, lsz;

    if (p == q)
    {
        return NULL;
    }

    io->flags = *p;
    if (!(*p & 0x80))    /* Only I/O Space Definition Byte */
    {
        io->nwin = 1;
        io->win[0].base = 0;
        io->win[0].len = (1 << (io->flags & AUI_CIS_IO_LINES_MASK));
        return p+1;
    }

    if (++p == q)
    {
        return NULL;
    }

    /* Range Descriptor Byte follows I/O Space Definition Byte */
    io->nwin = (*p & 0x0f) + 1;    /* Number of I/O Address Ranges */
    bsz = (*p & 0x30) >> 4;        /* Number of  Bytes which are Used to Encode Start of I/O Address Block */
    if (bsz == 3)
    {
        bsz++;
    }
    lsz = (*p & 0xc0) >> 6;            /* Number of Bytes which are Used to Encode Length of I/O Address Block */
    if (lsz == 3)
    {
        lsz++;
    }
    p++;

    /* I/O Address Range Description */
    for (i = 0; i < io->nwin; i++)
    {
        io->win[i].base = 0;
        io->win[i].len = 1;
        /* Start of I/O Address Block */
        for (j = 0; j < bsz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
            }
            io->win[i].base += *p << (j * 8);
        }
        /* Length of I/O Address Block */
        for (j = 0; j < lsz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
        }
            io->win[i].len += *p << (j * 8);
        }
    }
    return p;
}

/* Parse Interrupt Request Description Structure of Config Table Entry Information */
static unsigned char * parse_irq(unsigned char *p, unsigned char *q, struct aui_cis_irq *irq)
{
    if (p == q)
    {
        return NULL;
    }
    irq->irqinfo1 = *p;                        /* Interrupt Request Byte 0 */
    p++;
    if (irq->irqinfo1 & 0x10)
    {
        if (p + 2 > q)
        {
            return NULL;
        }
        irq->irqinfo2 = (p[1] << 8) + p[0];        /* Interrupt Request Byte 1 and 2 */
        p += 2;
    }
    return p;
}

/* Parse Memory Space Description Structure of Config Table Entry Information */
static unsigned char * parse_mem(unsigned char *p, unsigned char *q, struct aui_cis_mem *mem)
{
    int i, j, asz, lsz, has_ha;
    unsigned int len, ca, ha;

    if (p == q)
    {
        return NULL;
    }

    mem->nwin = (*p & 0x07) + 1;        /* Number of Window Descriptor */
    lsz = (*p & 0x18) >> 3;                /* Length Field Size */
    asz = (*p & 0x60) >> 5;            /* Card Address Field Size */
    has_ha = (*p & 0x80);                /* Host Address is Present or not */
    if (++p == q)
    {
        return NULL;
    }

    for (i = 0; i < mem->nwin; i++)
    {
        // Length
        len = ca = ha = 0;
        for (j = 0; j < lsz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
            }
            len += *p << (j * 8);
        }
        // Card Address
        for (j = 0; j < asz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
            }
            ca += *p << (j * 8);
        }
        // Host Address
        if (has_ha)
        {
            for (j = 0; j < asz; j++, p++)
            {
                if (p == q)
                {
                    return NULL;
                }
                ha += *p << (j * 8);
            }
        }
        mem->win[i].len = len << 8;
        mem->win[i].card_addr = ca << 8;
        mem->win[i].host_addr = ha << 8;
    }
    return p;
}

/* Parse Config Table Entry Information */
static int parse_cftable_entry(struct aui_cis_tuple *tuple, struct aui_cis_cftable_entry *entry)
{
    unsigned char *p, *q, features;

    p = tuple->tuple_data;
    q = p + tuple->tuple_data_len;
    entry->index = *p & 0x3f;        /* Configuration-Entry-Number */
    entry->flags = 0;
    if (*p & 0x40)                    /* Default Value for Entry */
    {
    entry->flags |= AUI_CIS_CFTABLE_DEFAULT;
    }
    if (*p & 0x80)                    /* Interface Configuration Present or not */
    {
        if (++p == q)
        {
            return AUI_RTN_FAIL;
        }
        if (*p & 0x10)                /* BVDs Active */
            {
                entry->flags |= AUI_CIS_CFTABLE_BVDS;
        }
        if (*p & 0x20)                /* Write Protect (WP) Status Active*/
            {
                entry->flags |= AUI_CIS_CFTABLE_WP;
        }
        if (*p & 0x40)                /* READY Status Active */
            {
                entry->flags |= AUI_CIS_CFTABLE_RDYBSY;
        }
        if (*p & 0x80)                /* M Wait Required */
            {
                entry->flags |= AUI_CIS_CFTABLE_MWAIT;
        }
        entry->interface = *p & 0x0f;        /* Interface Type */
    }
    else
    {
        entry->interface = 0;
    }

    /* Process Optional Features */
    if (++p == q)
    {
        return -1;
    }
    features = *p;                    /* Feature Selection Byte */
    p++;

    /* Power Options */
    if ((features & 3) > 0)
    {
        p = parse_power(p, q, &entry->vcc);        /* VCC Power-Description-Structure */
        if (p == NULL)
        {
            return -1;
        }
    }
    else
    {
        entry->vcc.present = 0;
    }
    if ((features & 3) > 1)
    {
        p = parse_power(p, q, &entry->vpp1);        /* Vpp1 Power-Description-Structure */
        if (p == NULL)
        {
            return -1;
        }
    }
    else
    {
        entry->vpp1.present = 0;
    }
    if ((features & 3) > 2)
    {
        p = parse_power(p, q, &entry->vpp2);        /* Vpp2 Power-Description-Structure */
        if (p == NULL)
        {
            return -1;
        }
    }
    else
    {
        entry->vpp2.present = 0;
    }

    /* Timing Options */
    if (features & 0x04)
    {
        p = parse_timing(p, q, &entry->timing);
        if (p == NULL)
        {
            return -1;
        }
    }
    else
    {
        entry->timing.wait = 0;
        entry->timing.ready = 0;
        entry->timing.reserved = 0;
    }

    /* I/O Space Options */
    if (features & 0x08)
    {
        p = parse_io(p, q, &entry->io);
        if (p == NULL)
        {
            return -1;
        }
    }
    else
    {
        entry->io.nwin = 0;
    }

    /* Interrupt Options */
    if (features & 0x10)
    {
        p = parse_irq(p, q, &entry->irq);
        if (p == NULL)
        {
            return -1;
        }
    }
    else
    {
        entry->irq.irqinfo1 = 0;
    }

    /* Memory Space Option */
    switch (features & 0x60)
    {
        case 0x00:
        entry->mem.nwin = 0;
        break;
        case 0x20:
        /* the address must be Even, because (unsigned short *)p
         * otherwise, STB will reboot
         */
        if (((unsigned int)p) & 0x01)
            return -1;

        entry->mem.nwin = 1;
        entry->mem.win[0].len = le16_to_cpu(*(unsigned short *)p) << 8;
        entry->mem.win[0].card_addr = 0;
        entry->mem.win[0].host_addr = 0;
        p += 2;
        if (p > q)
        {
            return -1;
        }
        break;
        case 0x40:
        /* the address must be Even, because (unsigned short *)p
         * otherwise, STB will reboot
         */
        if (((unsigned int)p) & 0x01)
            return -1;
        entry->mem.nwin = 1;
        entry->mem.win[0].len = le16_to_cpu(*(unsigned short *)p) << 8;
        entry->mem.win[0].card_addr = le16_to_cpu(*(unsigned short *)(p + 2)) << 8;
        entry->mem.win[0].host_addr = 0;
        p += 4;
        if (p > q)
        {
            return -1;
        }
        break;
        case 0x60:
        p = parse_mem(p, q, &entry->mem);
        if (p == NULL)
        {
            return -1;
        }
        break;
    }

    /* Misc Option */
    if (features & 0x80)
    {
        if (p == q)
        {
            return -1;
        }
        entry->flags |= (*p << 8);
        while (*p & 0x80)
        {
            if (++p == q)
            {
                return -1;
            }
        }
        p++;
    }

    entry->subtuples = q - p;

    return 0;
}

static unsigned long test_ci_total(unsigned long *argc, char **argv, char *sz_out_put)
{
    int i = 0, count = 0, ret = 0, status = 0, slot = 0;
    struct aui_cis_slot  *ci_slot = &cis_slot_info;
    struct aui_cis_config *config = &(ci_slot->config_infor);
    struct aui_cis_cftable_entry *cfgtable = &(ci_slot->cfgtable_infor);
    struct aui_cis_tuple tuple;
    unsigned char tmp = 0, data[2] = {0}, buf[255] = {0};
    unsigned int cis_addr = 0;        

	(void)argc;
    (void)argv;
    (void)sz_out_put;

    MEMSET(&tuple, 0, sizeof(struct aui_cis_tuple));
    tuple.tuple_data = buf;
    tuple.tuple_data_max = sizeof(buf);

    /* open ali cic devcie */
	printf("\n");
	printf("[CIC-TEST : STEP 1] >>>>>> open cic device :\n");
    if (aui_cic_open(NULL, &cic_hdl))
   	{
		printf("[CIC-TEST : STEP 1] result : failure.\n");
		return -1;
   	}
	printf("[CIC-TEST : STEP 1] result : success.\n");
    /* detect cam  */
	printf("\n");
	printf("[CIC-TEST : STEP 2] >>>>>> detect cam :\n");
	if (aui_cic_detect_cam(cic_hdl, slot, &status))
	{
		printf("[CIC-TEST : STEP 2] result : failure.\n");
		return -1;
	}
    if(0 == status) 
	{
		printf("[CIC-TEST : STEP 2] result : failure.\n");
		return -1;
    } 
	else
	{
		printf("[CIC-TEST : STEP 2] result : success.\n");
	}
    /* enable cam  */
	printf("\n");
	printf("[CIC-TEST : STEP 3] >>>>>> enable cam :\n");
    if (aui_cic_enable_cam(cic_hdl, slot))
    {
		printf("[CIC-TEST : STEP 3] result : failure.\n");
		return -1;
    }
	printf("[CIC-TEST : STEP 3] result : success.\n");
	AUI_SLEEP(300); 
    /* reset cam  */
	printf("\n");
	printf("[CIC-TEST : STEP 4] >>>>>> reset cam :\n");
    if (aui_cic_reset_cam(cic_hdl, slot))
    {
		printf("[CIC-TEST : STEP 4] result : failure.\n");
		return -1;
    }
	printf("[CIC-TEST : STEP 4] result : success.\n");	
	AUI_SLEEP(5000); 
    /* check cam ready */
	printf("\n");
	printf("[CIC-TEST : STEP 5] >>>>>> check cam ready :\n");
    if (aui_cic_cam_is_ready(cic_hdl, slot, &status))
    {
		printf("[CIC-TEST : STEP 5] result : failure.\n");
		return -1;
    }
	if (status == 0)
	{
		printf("[CIC-TEST : STEP 5] result : failure.\n");
		return -1;
	}
	else
	{
		printf("[CIC-TEST : STEP 5] result : success.\n");
	}	
    /* read cis */
	#if 1
	printf("\n");
	printf("[CIC-TEST : STEP 6] >>>>>> read cam cis :\n");
	while (1) {
		if (aui_cic_read_mem(cic_hdl,slot,1,cis_addr,&(tuple.tuple_code)))
	    {
			printf("[CIC-TEST : STEP 6] result : failure.\n");
			return -1;
	    }
		if (tuple.tuple_code == AUI_CIS_END)
	    {
	        printf("[CIC-TEST : STEP 6] read cis finished.\n");
	        break;
	    }
		cis_addr++;
		if (aui_cic_read_mem(cic_hdl,slot,1,cis_addr,&(tuple.tuple_link)))
	    {
			printf("[CIC-TEST : STEP 6] result : failure.\n");
			return -1;
	    }
		tuple.tuple_data_len = tuple.tuple_link;
		if (tuple.tuple_data_len > tuple.tuple_data_max)
	    {
	        printf("[CIC-TEST : STEP 6] too long tuple.\n");
	        return -1;
	    }
		cis_addr++;
		if (aui_cic_read_mem(cic_hdl,slot,tuple.tuple_data_len,cis_addr,tuple.tuple_data))
	    {
			printf("[CIC-TEST : STEP 6] result : failure.\n");
	    }
		cis_addr += tuple.tuple_data_len;

		switch (tuple.tuple_code)
		{
			case AUI_CIS_DEVICE_OA:
			case AUI_CIS_DEVICE_OC:
	            printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_DEVICE_OA/OC=0x%02x\n", tuple.tuple_code);
				//parse_device_oa(&tuple, &ci_slot->device_infor);
				break;
			case AUI_CIS_VERS_1:		
	            printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_VERS_1=0x%02x\n", tuple.tuple_code);
				break;
			case AUI_CIS_MANFID:
	            printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_MANFID=0x%02x\n", tuple.tuple_code);
				break;
			case AUI_CIS_CONFIG:
	            printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_CONFIG=0x%02x\n",tuple.tuple_code);
				if (parse_config(&tuple, &ci_slot->config_infor))
				{
					printf("[CIC-TEST : STEP 6] result : failure.\n");
					return -1;
				}
				break;
			case AUI_CIS_CFTABLE_ENTRY:
	            printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_CFTABLE_ENTRY=0x%02x\n", tuple.tuple_code);
				if (tuple.tuple_data_len > 2 + 11 + 17)
				{
					if (parse_cftable_entry(&tuple, &ci_slot->cfgtable_infor))
					{
						printf("[CIC-TEST : STEP 6] result : failure.\n");
						return -1;
					}
				}
				break;
			case AUI_CIS_END:
	            printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_END=0x%02x\n", tuple.tuple_code);
				return 0;
			case AUI_CIS_NO_LINK:
				printf("[CIC-TEST : STEP 6]:Found a AUI_CIS_NO_LINK=0x%02x\n", tuple.tuple_code);
				break;
			default:
	            printf("[CIC-TEST : STEP 6]:Found a un-process tuple=0x%02x\n", tuple.tuple_code);
				break;
		}
		#if 0
		printf("Tuple Code: %02x, Tuple Link: %02x\n", tuple.tuple_code, tuple.tuple_link);
		for (i = 0; i < tuple.tuple_data_len; i++)
		{
			printf("%02x ", tuple.tuple_data[i]);
		}
		printf("\n\n");
		#endif
	}
	#endif
	#if 1
	/* config cam */
	printf("\n");
	printf("[CIC-TEST : STEP 7] >>>>>> config cam interface :\n");
    switch (config->cif_infor.ifn)
    {
    	case IFN_DVBCI:
	        printf("CIC-TEST : STEP 7] Found DVB CI Card \n");
	        break;
    	case IFN_OPENCABLEPOD:
	        printf("CIC-TEST : STEP 7] result : failure. (OpenCable POD Card)\n");
	        return -1;
    	case IFN_ZOOMVIDEO:
	        printf("CIC-TEST : STEP 7] result : failure. (Zoom Video Card)\n");
	        return -1;
    	default:
	        printf("CIC-TEST : STEP 7] result : failure. (unknow Card)\n");
	        return -1;
    }
	/* Cfg Optoin Register: Enable the CAM Function */
    if (config->rmask[0] & 1)
    {
        if (aui_cic_read_mem(cic_hdl,slot,1,(config->base >> 1),&tmp))
        {
	        printf("CIC-TEST : STEP 7] result : failure.\n");
		}        
        tmp = (cfgtable->index | (tmp & 0x3f));
        if (aui_cic_write_mem(cic_hdl,slot,1,(config->base >> 1),&tmp))
        {
	        printf("CIC-TEST : STEP 7] result : failure.\n");
        }
    }
	#endif
	/* reset cam interface */
	#if 1
	printf("\n");
	printf("[CIC-TEST : STEP 8] >>>>>> reset cam interface :\n");
    // the CI spec indicates the reset cmd is 0x08.
    aui_cic_write_io_reg(cic_hdl, slot, AUI_CIC_CSR, 0x08);
	AUI_SLEEP(300); 
    //aui_cic_write_io_reg(cic_hdl, slot, AUI_CIC_CSR, 0x00);	
	while(1)
    {
        if (aui_cic_read_io_reg(cic_hdl, slot, AUI_CIC_CSR, data))
    	{
			printf("[CIC-TEST : STEP 8] result : failure.\n");
    	}
        if (data[0] & 0x40/*CIC_FR*/)
        {
			printf("[CIC-TEST : STEP 8] result : success.\n");
            break;
        }
        if(count == 100)
        {
			printf("[CIC-TEST : STEP 8] result : failure.\n");
            return  -1;
        }
        count ++;
        AUI_SLEEP(100);        
    }
	#endif

    return 0;
}
#endif

void test_cic_reg()
{
    aui_tu_reg_group("cic", "test cic device");
#ifdef CIC_TOTAL_TEST
    aui_tu_reg_item(2, "total", AUI_CMD_TYPE_API, test_ci_total, "test cic full function");
#endif
    aui_tu_reg_item(2, "open", AUI_CMD_TYPE_API, test_ci_init, "init user data and open ali cic device");
    aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_cic_slot_init, "cic device detect, reset and enable" );
    aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_cic_rw_io_data, "write and read block data");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_cic_rw_mem, "write and read memory data");
    aui_tu_reg_item(2, "4", AUI_CMD_TYPE_API, test_cic_rw_io_reg, "write and read io register");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_API, test_cic_pass_stream, "test cic device pass stream");
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_API, test_cic_bypass_stream , "test cic device bypass stream");
    aui_tu_reg_item(2, "7", AUI_CMD_TYPE_API, test_cic_tsg , "test cic tsg stream");
    aui_tu_reg_item(2, "8", AUI_CMD_TYPE_API, test_cic_recmsg , "test cic user callback,first this command,then open cicdevice");
    aui_tu_reg_item(2, "close", AUI_CMD_TYPE_API, test_ci_deinit, "close cic device and deinit");
}

