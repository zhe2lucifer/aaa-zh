#ifndef __AUI_KL__INNER_H_H
#define __AUI_KL__INNER_H_H
#include "aui_common_priv.h"
#include <aui_kl.h>
#include <hld/crypto/crypto.h>
#include <hld/dsc/dsc.h>
#include <hld/cf/cf.h>
#include <hld/pvr/pvr_remote.h>
#include "aui_dsc_inner.h"

/****************************LOCAL MACRO******************************************/
#define AUI_KL_KEY_POS_INVALID  (0xffffffff)

//#define AUI_KL_DEBUG   1
#define AUI_KL_DEBUG_NEW 1

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
    /** internal use,saved data list head */
    aui_dev_priv_data dev_priv_data;
    /** mutex lock,internal use */
    OSAL_ID dev_mutex_id;
    /** HLD layer KL device handle*/
    struct ce_device *pst_dev_kl;
    /** aui KL occupied pos resource array,when it isn't used again,and need to realse.
    it only save the ultimate key pos,when the ultimate key is 128 bit and odd_even key,
    ul_key_pos_out save even key,(ul_key_pos_out + 1) is odd key
    */
    unsigned long ul_key_pos_out;
    /**the ultimate key occupied key pos counts,
    64 bits odd,even or odd_even key occupy one key pos,
    128 bits odd or even occupy one key pos,
    128 bits odd_even occupy two key pos.
    */
    unsigned long ul_key_pos_cnt;
    /*ultimate key pattern:AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE,AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN,
    AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE,AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN,
    */
    enum aui_kl_output_key_pattern key_pattern;
    /*root key pos:KEY_0_0,KEY_0_1.KEY_0_2,KEY_0_3
                    0       1       2       3
    */
    int root_key_idx;
    /*generated key level: AUI_KL_KEY_FIVE_LEVEL,AUI_KL_KEY_ONE_LEVEL,AUI_KL_KEY_TWO_LEVEL,
    */
    enum aui_kl_crypt_key_level level;
    int ck_size; /* content key size in bytes ,ultimate key size:8 or 16*/
    enum aui_kl_algo  algo;/**AUI_KL_ALGO_TDES,AUI_KL_ALGO_AES*/
    enum aui_kl_work_mode kl_mode;
    enum aui_kl_level_work_mode work_level_mode;

    unsigned char pk_level1[PKEY_LEN]; /* Protecting keys for each level 1*/
    unsigned char pk_level2[PKEY_LEN]; /* Protecting keys for each level 2*/
    unsigned char pk_level3[PKEY_LEN]; /* Protecting keys for each level 3*/
    unsigned char pk_level4[PKEY_LEN]; /* Protecting keys for each level 4*/
    unsigned char big_key_buf[BIG_BUF_LEN]; /* ALL keys*/

    int pk_level1_valid;/*pk_level1 whether validation or not*/
    int pk_level2_valid;/*pk_level2 whether validation or not*/
    int pk_level3_valid;/*pk_level3 whether validation or not*/
    int pk_level4_valid;/*pk_level4 whether validation or not*/
};

#ifdef SDK_DRV_VERSION_NEW
/*
enum aui_kl_root_key_idx {
    AUI_KL_ROOT_KEY_0_0=0,  //0x4d
    AUI_KL_ROOT_KEY_0_1=1,  //0x51
    AUI_KL_ROOT_KEY_0_2=2,  //0x55
    AUI_KL_ROOT_KEY_0_3=3,  //0x59
    AUI_KL_ROOT_KEY_0_4=4,  //0x60
    AUI_KL_ROOT_KEY_0_5=5,  //0x64
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

/* copy protected key by key pattern */
#define AUI_KL_CPY_KEY(dst,src,len,key_pattern) do{\
    switch (key_pattern) {\
        case CE_PARITY_EVEN:{\
            MEMCPY(dst+8,src,len);break;}\
        case CE_PARITY_ODD:\
		case CE_PARITY_EVEN_ODD:{\
            MEMCPY(dst,src,len);break;}\
        default:{\
            osal_mutex_unlock(mutex_kl);\
            aui_rtn(AUI_MODULE_KL, AUI_RTN_EINVAL, "Invalid hdl->key_pattern\n");}\
    }}while(0)

#define KL_MIDDLE_CW_SIZE   16      /*Length of KL middle encrypted control word must be equal to 16*/
#define KL_END_SINGLE_CW_SIZE_64    8 /*64 bits single control word */
#define KL_END_SINGLE_CW_SIZE_128   16 /*64 bits single control word */
#endif
