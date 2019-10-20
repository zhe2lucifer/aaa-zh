#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <linux/input.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/select.h>
#include <aui_dmx.h>
#include <aui_dis.h>
#include <aui_av.h>
#include <aui_dsc.h>


//#define CA_CHG_KEY

#ifndef CA_CHG_KEY
#define FILE_NAME "/mnt/usb/sda1/xxx359.ts"
#define DSC_NUM 1
int blk_cnt[]={0, 359};
#else
#define FILE_NAME "/mnt/usb/sda1/aes_ecb_pure.ts"
#define DSC_NUM 3
int blk_cnt[]={0, 133,223,337};
#endif
#define BLOCK_SIZE (48128) //47*1024
static aui_hdl dsc_hdl[3];
static aui_attr_dsc dsc_attr[3];

static int test_dsc_open(int id)
{
    memset(&dsc_attr[id], 0, sizeof(aui_attr_dsc));

    dsc_attr[id].uc_dev_idx = id;
    dsc_attr[id].uc_algo = AUI_DSC_ALGO_AES;
    dsc_attr[id].dsc_data_type = AUI_DSC_DATA_PURE;
	if(id == 1)
	{
		dsc_attr[id].uc_algo = AUI_DSC_ALGO_TDES;
	}

    if (aui_find_dev_by_idx(AUI_MODULE_DSC, dsc_attr[id].uc_dev_idx, &dsc_hdl[id])) {
        if (aui_dsc_open(&dsc_attr[id], &dsc_hdl[id])) {
            AUI_PRINTF("dsc open error\n");
            return 1;
        }
    }
    return 0;
}
unsigned char iv[16]={0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
unsigned char key0[16]={0x1a, 0x3c, 0x16, 0xf8, 0xd7, 0x66, 0x43, 0xc3, 0x5c, 0x9e, 0x64, 0x23, 0x67, 0x07, 0x60, 0x06};
unsigned char key1[16]={0x1a, 0x3c, 0x16, 0xf8, 0xd7, 0x66, 0x43, 0xc3, 0x5c, 0x9e, 0x64, 0x23, 0x67, 0x07, 0x63, 0x33};
unsigned char key2[16]={0xd0, 0x8f, 0x87, 0x09, 0xc1, 0x7c, 0x63, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static int test_dsc_attach_key(int id)
{
    aui_attr_dsc attr;
    aui_hdl hdl;
    unsigned long type;

    if (aui_find_dev_by_idx(AUI_MODULE_DSC, id, &hdl)) {
        printf("aui_find_dev_by_idx fault LINE %d\n",__LINE__);
        return 1;
    }

    MEMSET(&attr, 0, sizeof(aui_attr_dsc));

	if(0 == id)
	{
	    attr.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	    attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	    attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

		attr.en_en_de_crypt = AUI_DSC_ENCRYPT;
	    attr.ul_key_len =128;
	    attr.puc_key = key0;
	    attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;

	    if (aui_dsc_get(hdl, AUI_DSC_GET_DATA_TYPE, &type)) {
	        AUI_PRINTF("[aui dsc] get data type error\n");
	        return 1;
	    }

	    attr.ul_pid_cnt = 0;
	    attr.pus_pids = NULL;
	    attr.puc_iv_ctr = iv;
	    attr.dsc_data_type = type;
		attr.uc_algo = AUI_DSC_ALGO_AES;
	}
	else if(1==id)
	{
		attr.uc_mode = AUI_DSC_WORK_MODE_IS_ECB;
	    attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	    attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

		attr.en_en_de_crypt = AUI_DSC_ENCRYPT;
	    attr.ul_key_len =128;
	    attr.puc_key = key1;//"1a3c16f8d76643c35c9e642367076333";
	    attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;

	    if (aui_dsc_get(hdl, AUI_DSC_GET_DATA_TYPE, &type)) {
	        AUI_PRINTF("[aui dsc] get data type error\n");
	        return 1;
	    }

	    attr.ul_pid_cnt = 0;
	    attr.pus_pids = NULL;
	    attr.puc_iv_ctr =iv;
	    attr.dsc_data_type = type;
		attr.uc_algo = AUI_DSC_ALGO_TDES;
	}
	else if(2==id)
	{
		attr.uc_mode = AUI_DSC_WORK_MODE_IS_CBC;
	    attr.ul_key_pattern = AUI_DSC_KEY_PATTERN_SINGLE;
	    attr.en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;

		attr.en_en_de_crypt = AUI_DSC_ENCRYPT;
	    attr.ul_key_len =128;
	    attr.puc_key = key2;//"d08f8709c17c632e0000000000000000";
	    attr.dsc_key_type = AUI_DSC_HOST_KEY_SRAM;

	    if (aui_dsc_get(hdl, AUI_DSC_GET_DATA_TYPE, &type)) {
	        AUI_PRINTF("[aui dsc] get data type error\n");
	        return 1;
	    }

	    attr.ul_pid_cnt = 0;
	    attr.pus_pids = NULL;
	    attr.puc_iv_ctr = iv;
	    attr.dsc_data_type = type;
		attr.uc_algo = AUI_DSC_ALGO_AES;
	}
	else
	{
		printf("%s -> %d invalid key %d\n",  __FUNCTION__, __LINE__, id);
		return 1;
	}

    /* << debug info */
    AUI_PRINTF("[aui dsc] attributes:\n"
               "attr.ul_key_len %ld,attr.dsc_key_type %d\n"
               "attr.uc_mode %d ,attr.ul_key_pattern %ld, attr.en_residue %d\n"
               "attr.en_en_de_crypt %d, attr.uc_algo %d\n"
               "attr.dsc_data_type %d, attr.ul_pid_cnt %ld, ",
               attr.ul_key_len, attr.dsc_key_type,
               attr.uc_mode, attr.ul_key_pattern, attr.en_residue,
               attr.en_en_de_crypt, attr.uc_algo,
               attr.dsc_data_type, attr.ul_pid_cnt);
    short i;
    AUI_PRINTF("\n");
    if (attr.puc_key) {
        AUI_PRINTF("attr.puc_key:");
        for(i=0; i<16; i++) {
            AUI_PRINTF("%02x",attr.puc_key[i]&0xFF);
        }
        AUI_PRINTF("\n");
    }
    if (attr.puc_iv_ctr) {
        AUI_PRINTF("attr.puc_iv_ctr:");
        for(i=0; i<16; i++) {
            AUI_PRINTF("%02x",attr.puc_iv_ctr[i]&0xFF);
        }
        AUI_PRINTF("\n");
    }

	aui_dsc_process_status dsc_process_status;
	memset(&dsc_process_status, 0, sizeof(dsc_process_status));
    if (aui_dsc_update_pvr_encrypt_key_info(hdl, &attr, NULL, &dsc_process_status)) {
        AUI_PRINTF("[aui dsc] attach pvr key error\n");
        return 1;
    }
    return 0;
}

static int test_dsc_init(int id)
{
	if(test_dsc_open(id))
	{
		printf("%s -> dsc %d open fail\n", __FUNCTION__, id);
		return 1;
	}
	aui_dsc_process_attr process_attr;
	process_attr.process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
	process_attr.ul_block_size = BLOCK_SIZE;
	aui_dsc_process_attr_set(dsc_hdl[id], &process_attr);
	if(test_dsc_attach_key(id))
	{
		printf("%s -> dsc %d attach key fail\n", __FUNCTION__, id);
		return 1;
	}
	return 0;
}

static int test_dsc_deinit(int id)
{
	if(aui_dsc_close(dsc_hdl[id]))
	{
		printf("%s -> close dsc %d fail\n", __FUNCTION__, id);
		return 1;
	}
	return 0;
}

int volatile quit_flag = 0;
static char ca_kbhit(void)
{
	struct timeval tv;
	fd_set read_fd;

	tv.tv_sec=0;
	tv.tv_usec=0;
	FD_ZERO(&read_fd);
	FD_SET(0,&read_fd);

	if(select(1, &read_fd, NULL, NULL, &tv) == -1)
		return 0;

	if (FD_ISSET(0,&read_fd)){
		char c = getchar();
		if (c == 0xa)
			return 0;
		return c;
	}
	return 0;
}

int do_run_ca()
{
	if(test_dsc_init(0))
	{
		printf("%s @ %d error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	aui_hdl dmx_hdl = NULL;
	aui_attr_dmx attr_dmx;
	memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
	attr_dmx.uc_dev_idx = AUI_DMX_ID_DEMUX0;
	if(aui_find_dev_by_idx(AUI_MODULE_DMX, AUI_DMX_ID_DEMUX0, &dmx_hdl)) {
        AUI_PRINTF("find aui_dmx handle fail\n");
		if(aui_dmx_open(&attr_dmx, &dmx_hdl)) {
			AUI_PRINTF("dmx open fail\n");
			goto EXIT;
		}
		aui_dmx_start(dmx_hdl, NULL);
	}

	aui_dmx_data_path data_path;
	memset(&data_path, 0, sizeof(data_path));
	data_path.data_path_type = AUI_DMX_DATA_PATH_EN_REC;
	data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;
	data_path.p_hdl_en_dev = dsc_hdl[0];
	if(aui_dmx_data_path_set(dmx_hdl, &data_path)) {
		AUI_PRINTF("data path set failed\n");
		goto EXIT1;
	}
	aui_dmx_dsc_id dmx_dsc_id;
	aui_dmx_dsc_id_get(dmx_hdl, &dmx_dsc_id);

	unsigned int i = 0;
	for(i=0;i<sizeof(dmx_dsc_id.identifier);i++) {
		printf("%02x", dmx_dsc_id.identifier[i]);
	}
	printf("\n");
	
#ifdef CA_CHG_KEY
	if(test_dsc_init(1))
	{
		printf("%s @ %d error\n", __FUNCTION__, __LINE__);
		goto EXIT1;
	}
	if(test_dsc_init(2))
	{
		printf("%s @ %d error\n", __FUNCTION__, __LINE__);
		goto EXIT2;
	}
#endif
	char c = 0;
	while(!quit_flag) {
		sleep(1);
		c = ca_kbhit();
		if(c == 'q')
			quit_flag = 1;
	}
#ifdef CA_CHG_KEY
	test_dsc_deinit(2);
	EXIT2:
	test_dsc_deinit(1);
#endif
	EXIT1:
	aui_dmx_close(dmx_hdl);
	EXIT:
	test_dsc_deinit(0);
	printf("%s@%d CA process quit\n", __FUNCTION__, __LINE__);
	return 0;
}



int main(int argc, char **argv)
{
	int res = do_run_ca();
	return res;
}


