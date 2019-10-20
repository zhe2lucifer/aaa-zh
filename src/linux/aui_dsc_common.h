/**
 * @file
 * @brief      Function used in AUI DMX to retrieve info from AUI DSC handler
 *             It is required to link a AUI DSC instance to the AUI DMX instance.
 * @author     Romain Baeriswyl
 * @date       2014-04-03
 * @version    1.0.0
 * @note       Copyright(C) ALi Corporation. All rights reserved.
 */

#ifndef __AUI_DSC_COMMON_H__
#define __AUI_DSC_COMMON_H__

#include "aui_common_priv.h"
#include <aui_dsc.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <ali_dsc_common.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <alisldsc.h>
#include <alislotp.h>
#include "aui_dsc_common.h"
#include "aui_otp.h"
//#include "adf_ce.h"
#include "aui_kl_common.h"
#include <errno.h>
#include <ali_dmx_common.h>

#ifdef INT64_MAX
#undef INT64_MAX
#endif
#include <alidefinition/adf_pvr.h>

struct algo_info {
    int dsc_fd;
    unsigned short *pid;
    int pid_len;
	int pvr_fd;
	int pvr_crypto_mode;/*pvr init flag,pvr_crypto_mode == 1 used pvr encrypt/decrypt TS data*/
	unsigned long pvr_block_size;
	int dsc_data_type;
	aui_dsc_process_attr process_attr;
};
enum dsc_crypt_mode_internal {
    AUI_DSC_DECRYPT_IN,
    AUI_DSC_ENCRYPT_IN,
    AUI_DSC_CRYPT_NB
};
enum parity_in{
	AUI_DSC_ODD_MODE_IN,
	AUI_DSC_EVEN_MODE_IN,
	PARITY_NB_IN
};

#define aui_list_get_single_entry(pos, head, member) \
    (((head)->next == (head)->prev) ? aui_list_entry((head)->next, typeof(*pos), member) : NULL);


#define STREAM_ID_INVALID (-1)
#define SUBDEV_ID_INVALID ((unsigned int)-1)

#define KL1_POS(pos) (16 <= (pos) && (pos)<= 31)
#define KL2_POS(pos) (32 <= (pos) && (pos)<= 47)
#define KL3_POS(pos) (48 <= (pos) && (pos)<= 56)
#define KL4_POS(pos) (57 <= (pos) && (pos)<= 65)

/* PID to be used in ram to ram operation in order to have
 * alisldsc_update_key() working.
 */
#define R2R_PID     SL_R2R_PID
#define PACKET_SIZE (0x100000)

/** For generate HMAC message */
#define HASH_BLOCK_LENGTH   64
#define OTP_DW_LEN  0x4
#define AUI_DSC_KL_TYPE_ETSI (1<<4)
#define AUI_DSC_PVR_BLOCK_SIZE_DEFAULT 47*1024


#define CPY_IV(parity,key_param,buf,len,valid) do{\
        switch (parity) {\
                        case AUI_DSC_KEY_PATTERN_SINGLE: \
                        case AUI_DSC_KEY_PATTERN_EVEN: {\
                            memcpy((key_param)->iv_even, (buf), (len));\
                            valid |= CA_VALID_IV_EVEN;\
                            break;}\
                        case AUI_DSC_KEY_PATTERN_ODD:{\
                            memcpy((key_param)->iv_odd, (buf), (len));\
                            valid |= CA_VALID_IV_ODD;break;}\
                        case AUI_DSC_KEY_PATTERN_ODD_EVEN:{\
                            memcpy((key_param)->iv_odd, (buf), (len));\
                            memcpy((key_param)->iv_even, (buf) + (len), (len));\
                            valid |= CA_VALID_IV_EVEN | CA_VALID_IV_ODD;break;}}} while(0);

typedef struct hmac_param {
    unsigned char key[FIRST_KEY_LENGTH];
    unsigned char k0[HASH_BLOCK_LENGTH];
    unsigned char ipad[HASH_BLOCK_LENGTH];
    unsigned char opad[HASH_BLOCK_LENGTH];
    unsigned char hout[HASH_BLOCK_LENGTH];
} HMAC_PARAM, *PHMAC_PARAM;

struct mkey {
    struct aui_list_head list;
    struct key_from key_from;
    unsigned int key_type;
    unsigned short *pid_list;
    unsigned short pid_len;
    unsigned char ctr[32];
    int initialized;
    int not_refresh_iv;
	int key_handle;
};

/* internal handle structure */
struct dsc_handler{
    struct aui_st_dev_priv_data data;

    alisl_handle dev;
    unsigned int algo;
    struct aui_list_head key_list;/*key param list head*/
    unsigned int key_type;
    unsigned int enable_change_key;
    unsigned int mode;
    unsigned int data_type;
    int encrypt;
    unsigned int kl_mode;
	/* for pvr block mode record
	 * if pvr_fd <= 0,pvr fd is invalid
	 */
	int pvr_fd;
	int dsc_fd;
	int pvr_crypto_mode;/*pvr init flag,pvr_crypto_mode == 1 used pvr encrypt/decrypt TS data*/
	unsigned int ul_sub_dev_id;
	unsigned int pid_flag;/*pid_flag = 1 represent for tha the pid have been added*/
	int see_dmx_fd; 
	aui_dsc_process_attr dsc_process_attr;
	alisl_handle sl_pvr_hdl; //add for aui_dsc_pvr_playback_env_init/deinit
	unsigned int ali_pvr_de_hdl;

    int key_attached;/*0, the DSC has not been configurated; 1, the DSC has been configurated*/
    aui_dsc_iv_mode dsc_iv_mode;
};

/* data type conversion between sharelib and AUI */
#define SL2AUI_DATATYPE(x) (((x) == PURE_DATA_MODE) ? AUI_DSC_DATA_PURE : AUI_DSC_DATA_TS)
#define AUI2SL_DATATYPE(x) (((x) == AUI_DSC_DATA_PURE) ? PURE_DATA_MODE : TS_MODE)

/* Key mode: 0 for 64, 1 for 128, 2 for 192, 3 for 256 bits */
#define KEY_LEN2MODE(x) ((x)/64 - 1)
typedef struct ali_pvr_de_hdl_magic {
	unsigned int ali_pvr_de_hdl;
	unsigned char reserve[4];
} ali_pvr_de_hdl_magic;
AUI_RTN_CODE aui_dsc_get_fd(aui_hdl handle, struct algo_info *pid);
AUI_RTN_CODE aui_dsc_set_pid(aui_hdl handle,struct algo_info algo_info,
	enum dsc_crypt_mode_internal crypt_mode,enum parity_in parity);

AUI_RTN_CODE aui_dsc_free_block_mode(aui_hdl handle);
AUI_RTN_CODE aui_get_dsc_crypt_fd(aui_hdl handle);
AUI_RTN_CODE aui_dsc_pvr_start_record(aui_hdl handle,struct algo_info *algo_info);
AUI_RTN_CODE aui_dsc_pvr_playback_env_init(aui_hdl handle, unsigned int block_size, unsigned int de_dsc_num, unsigned int *ali_pvr_de_hdl);
AUI_RTN_CODE aui_dsc_pvr_playback_key_set(aui_hdl handle, unsigned int decrypt_index, unsigned int block_size,  unsigned int block_cnt, unsigned int ali_pvr_de_hdl);
AUI_RTN_CODE aui_dsc_pvr_playback_env_deinit(struct dsc_handler *hdl);
int aui_dsc_is_vmx_module(aui_hdl handle);
AUI_RTN_CODE aui_vmx_fd_get(aui_hdl handle, int *vmx_fd);
AUI_RTN_CODE aui_vmx_service_index_get(aui_hdl handle, int *service_index);

int aui_dsc_pvr_mmap(aui_hdl *handle, unsigned int *mmap_addr, unsigned int *mmap_len);
int aui_dsc_pvr_munmap(aui_hdl handle, unsigned int *mmap_addr, unsigned int *mmap_len);
AUI_RTN_CODE aui_dsc_pvr_decrypt_block_data(aui_hdl sl_pvr_hdl, unsigned int ali_pvr_de_hdl, const unsigned char *buf, 
				int size, int block_idx, unsigned char *iv, unsigned int iv_len, enum pvr_ott_data_type type)  ;

AUI_RTN_CODE aui_dsc_pvr_decrypt_block_data_subsample(aui_hdl sl_pvr_hdl, unsigned int ali_pvr_de_hdl, const unsigned char *buf, 
				int size, int block_idx, unsigned char *iv, unsigned int iv_len, enum pvr_ott_data_type type);
#endif
