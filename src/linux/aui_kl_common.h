#ifndef __AUI_KL_HEADER__
#define __AUI_KL_HEADER__
#include "aui_common_priv.h"
#include <aui_kl.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <alislce.h>
#include <alislotp.h>
#include <alipltfretcode.h>
#include <pthread.h>
#include <ali_ce_common.h>

#define SDK_DRV_VERSION_NEW
#define PKEY_LEN 16
#define ROOT_KEY_0 0
#define ROOT_KEY_1 1
#define ROOT_KEY_2 2
#define ROOT_KEY_3 3
#define OTP_LEN 4
#define OTP_APP_INFORMATION 0x84
#define OTP_FIVE_LEVELS_MODE 0x5F
#define OTP_CONFIGURATION_0 0x03
#define OTP_CONFIGURE_3 0xDC
#define BIG_BUF_LEN (PKEY_LEN*4)

/* number of level */
#define KL_LEVELS (AUI_KL_KEY_LEVEL_NB-1)
#define OTP_KEY_128_FLAG 1
struct kl_alloc {
    int pos;
    int len; /* 0 not allocated, 1 for 128 bits, 2 for 2x128 bits */
};

/** Key work mode selection */
enum aui_kl_level_work_mode {
    /* 2/3 and 5 levels mode derivations allowed */
    AUI_KL_WORK_MODE_ALL,
    /* 5 levels mode derivations only allowed */
    AUI_KL_WORK_MODE_5_LEVELS_ONLY,

    AUI_KL_WORK_LEVEL_MODE_NB
};

/** Key work mode selection */
enum aui_kl_work_mode {
    AUI_KL_MODE_NORMAL = 0,
    AUI_KL_MODE_FIX_ADDRESS,
    AUI_KL_MODE_FIX_ENGINE,
    AUI_KL_MODE_NB
};

typedef struct board_config_s {
    enum aui_kl_level_work_mode work_mode;
    enum aui_kl_work_mode board_mode;
    unsigned int five_levels_only;
    unsigned int cas_vendor_id;
    unsigned int fix_engine_mode;
    unsigned int fix_addr_mode;
} board_config_t;

struct kl_handler {
    struct aui_st_dev_priv_data data;

    alisl_handle dev;
    int root_key_idx;
    int level;
    int key_pattern;
    int ck_size; /* content key size in bytes */
    int algo;
    unsigned int kl_mode;
    aui_kl_type key_ladder_type;
    enum aui_kl_level_work_mode work_level_mode;

    /* position and length of the key for the KL stages */
    struct kl_alloc key_info[KL_LEVELS];

    unsigned char pk_level1[PKEY_LEN]; /* Protecting keys for each level 1*/
    unsigned char pk_level2[PKEY_LEN]; /* Protecting keys for each level 2*/
    unsigned char pk_level3[PKEY_LEN]; /* Protecting keys for each level 3*/
    unsigned char pk_level4[PKEY_LEN]; /* Protecting keys for each level 4*/
    unsigned char big_key_buf[BIG_BUF_LEN]; /* ALL keys*/

    int pk_level1_valid;
    int pk_level2_valid;
    int pk_level3_valid;
    int pk_level4_valid;

    int last_key_pos;
    //enum aui_kl_level_work_mode work_mode;
};

#ifdef SDK_DRV_VERSION_NEW
/*
enum aui_kl_root_key_idx {
	AUI_KL_ROOT_KEY_0_0=0,	//0x4d
	AUI_KL_ROOT_KEY_0_1=1,	//0x51
	AUI_KL_ROOT_KEY_0_2=2,	//0x55 
	AUI_KL_ROOT_KEY_0_3=3,	//0x59
	AUI_KL_ROOT_KEY_0_4=4,	//0x60
	AUI_KL_ROOT_KEY_0_5=5,	//0x64	
	AUI_KL_ROOT_KEY_0_6_R=6,
	AUI_KL_ROOT_KEY_0_6=7,
	AUI_KL_ROOT_KEY_0_7=8,
	AUI_KL_ROOT_KEY_NB
};
*/
#define OTP_ADDR(root_idx) (OTP_ADDESS_1 + root_idx * 4)   
#define KEY_POS(root_idx) \
    ((root_idx <= AUI_KL_ROOT_KEY_0_5) ? root_idx : INVALID_ALI_CE_KEY_POS)
#else
#define OTP_ADDRESS(key_pos) ((key_pos <= AUI_KL_ROOT_KEY_0_3) ? (OTP_ADDESS_1 + key_pos * 4) : -1)
#define KEY_POS(root_idx) ((root_idx <= AUI_KL_ROOT_KEY_0_3) ? root_idx : INVALID_ALI_CE_KEY_POS)
#endif

#define KL_MIDDLE_CW_SIZE	16		/*Length of KL middle encrypted control word must be equal to 16*/
#define KL_END_SINGLE_CW_SIZE_64	8 /*64 bits single control word */
#define KL_END_SINGLE_CW_SIZE_128	16 /*64 bits single control word */
#endif
