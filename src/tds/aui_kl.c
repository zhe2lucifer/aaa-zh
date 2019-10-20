/**@file
 *    @brief     ALi AUI KL (key ladder) function implementation
 *    @author    Adolph.liu
 *    @date      2016.3.1
 *    @version   0.0.2
 *    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */

/****************************INCLUDE HEAD FILE************************************/
#include "aui_kl_inner.h"

AUI_MODULE(KL);

/** 模块的互斥信号量，此信号量用于锁内部的各个设备的锁，是二级锁的第一级 */
static OSAL_ID mutex_kl=0;

#define pthread_mutex_lock(mutex_kl_mod)    osal_mutex_lock(mutex_kl_mod,OSAL_WAIT_FOREVER_TIME)
#define pthread_mutex_unlock(mutex_kl_mod) osal_mutex_unlock(mutex_kl_mod)

typedef struct aui_kl_index_map{
    enum CE_OTP_KEY_SEL kl_index;
    unsigned int otp_addr;
}aui_kl_index_map;

/*
TDS KL key map:
            KL1     KL2     KL3     KL4
OTP addr    0x4d    0x51    0x55    0x59
                            0x60    0x64
root_key_addr     rootkey_index    kl_device    kl_index
0x4d                0               KL1             0
0x51                1               KL2           1
0x55                2               KL3           2
0x59                3               KL4             3    
0x60                4              kL3           2
0x64                5               KL4           3
                            
*/
static aui_kl_index_map m_kl_index_list[] = 
{
    {OTP_KEY_0_0, OTP_ADDESS_1},
    {OTP_KEY_0_1, OTP_ADDESS_2},
    {OTP_KEY_0_2, OTP_ADDESS_3},
    {OTP_KEY_0_3, OTP_ADDESS_4}, //HDCP key
    {OTP_KEY_0_2, OTP_ADDESS_5},
    {OTP_KEY_0_3, OTP_ADDESS_6},
};

#define AUI_KL_OTP_ADDR(root_key)  (m_kl_index_list[root_key & 0x0f].otp_addr)
#define AUI_KL_INDEX(root_key)     \
        ((root_key & ETSI_KL_PREFIX) ? \
         (m_kl_index_list[root_key & 0x0f].kl_index)|ETSI_KL_PREFIX : \
         (m_kl_index_list[root_key & 0x0f].kl_index) \
        )

AUI_RTN_CODE aui_kl_init(p_fun_cb p_call_back_init,void *pv_param)
{
    mutex_kl=osal_mutex_create();
    if(0==mutex_kl) {
        aui_rtn(AUI_RTN_FAIL,"osal_mutex_create fail");
    }
    if(NULL!=p_call_back_init) {
        return p_call_back_init(pv_param);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    if(E_OK!=osal_mutex_delete(mutex_kl)) {
        aui_rtn(AUI_RTN_FAIL,"osal_mutex_delete fail");
    }
    if(NULL!=p_call_back_init) {
        return p_call_back_init(pv_param);
    }
    return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_kl_res_take(aui_hdl p_hdl_kl,CE_FOUND_FREE_POS_PARAM *p_res_kl)
{
    if((p_res_kl->ce_key_level>FIVE_LEVEL)||(p_res_kl->number>4)) {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }
    p_res_kl->pos=INVALID_ALI_CE_KEY_POS;
    if (RET_SUCCESS!=ce_ioctl(((struct kl_handler *)p_hdl_kl)->pst_dev_kl, IO_CRYPT_FOUND_FREE_POS, (unsigned long)(p_res_kl))) {
        aui_rtn(AUI_RTN_NO_RESOURCE,"Get free key pos fail");
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_open(aui_attr_kl *attr, aui_hdl *handle)
{
    struct kl_handler *hdl;

    if ((!attr) || (!handle) || (attr->en_level >= AUI_KL_KEY_LEVEL_NB)
        || (attr->en_key_pattern >= AUI_KL_OUTPUT_KEY_PATTERN_NB))
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");

    int root_key = KEY_POS(attr->en_root_key_idx);
    if (root_key == INVALID_ALI_CE_KEY_POS)
        aui_rtn(AUI_RTN_EINVAL, "invalid root key position");
    if(attr->en_root_key_idx == AUI_KL_ROOT_KEY_0_3){
        aui_rtn(AUI_RTN_EINVAL, "rootkey = AUI_KL_ROOT_KEY_0_3 is only used to generate hdcp key!\n");
    }

    hdl = (struct kl_handler *) MALLOC(sizeof(struct kl_handler));
    if (!hdl)
        aui_rtn( AUI_RTN_ENOMEM, "Malloc failed");
    MEMSET(hdl,0,sizeof(struct kl_handler));
    hdl->dev_priv_data.dev_idx = attr->uc_dev_idx;
    hdl->ul_key_pos_out = AUI_KL_KEY_POS_INVALID;
    hdl->pst_dev_kl=(struct ce_device *)dev_get_by_id(HLD_DEV_TYPE_CE,0);
    if(NULL==hdl->pst_dev_kl) {
        MEMSET(hdl,0,sizeof(struct kl_handler));
        FREE(hdl);
        hdl=NULL;
        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_FAIL,"Get device failed");
    }

    /*register hdl to generally managed list*/
    if (aui_dev_reg(AUI_MODULE_KL, hdl)) {
        FREE(hdl);
        aui_rtn(AUI_RTN_EINVAL, "dev idx already used");
    }

    hdl->level = attr->en_level;/*generated key level,one,tow,three,for five*/
    hdl->key_pattern = attr->en_key_pattern;
    hdl->root_key_idx = (int) attr->en_root_key_idx;
    if (AUI_KL_TYPE_ETSI == attr->en_key_ladder_type)
    {
        hdl->root_key_idx = (ETSI_KL_PREFIX | hdl->root_key_idx);
    }
    else if(AUI_KL_TYPE_CONAXVSC == attr->en_key_ladder_type)
    {
        CE_FOUND_FREE_POS_PARAM kl_key_res;
        
        if(AUI_KL_KEY_POS_INVALID == hdl->ul_key_pos_out) {
            MEMSET(&kl_key_res,0,sizeof(CE_FOUND_FREE_POS_PARAM));
            if(AUI_KL_KEY_ONE_LEVEL == hdl->level)
                kl_key_res.ce_key_level = ONE_LEVEL;
            else if(AUI_KL_KEY_TWO_LEVEL == hdl->level)
                kl_key_res.ce_key_level = TWO_LEVEL;
            else if((AUI_KL_KEY_THREE_LEVEL == hdl->level) ||(AUI_KL_KEY_FIVE_LEVEL == hdl->level))
                kl_key_res.ce_key_level = THREE_LEVEL;
            kl_key_res.pos = INVALID_ALI_CE_KEY_POS;
            kl_key_res.number = 1;

            kl_key_res.root = AUI_KL_INDEX(hdl->root_key_idx);
            if(AUI_RTN_SUCCESS!=aui_kl_res_take(hdl,&kl_key_res)) 
                aui_rtn(AUI_RTN_FAIL,"Get resource fail");
            hdl->ul_key_pos_out = kl_key_res.pos;
            hdl->ul_key_pos_cnt = 1;
        }
    }

    /* clear key size in bytes */
    if ((hdl->key_pattern == AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE)
        || (hdl->key_pattern == AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN))
        hdl->ck_size = 8;/*for csa1/csa2/des*/
    else
        hdl->ck_size = 16;/*for aes/tdes/csa3*/

    if(AUI_KL_KEY_FIVE_LEVEL == hdl->level) {
        hdl->work_level_mode = AUI_KL_WORK_MODE_5_LEVELS_ONLY;
    }
    /* check level config */
    if ((hdl->work_level_mode == AUI_KL_WORK_MODE_5_LEVELS_ONLY)
        && (hdl->level != AUI_KL_KEY_FIVE_LEVEL)) {
        aui_rtn( AUI_RTN_FAIL, "[aui kl] 5-Levels KL not allowed "
                "with the current configuration");
    }

    if ((hdl->work_level_mode != AUI_KL_WORK_MODE_5_LEVELS_ONLY)
        && (hdl->level == AUI_KL_KEY_FIVE_LEVEL)) {
        aui_rtn( AUI_RTN_FAIL, "[aui kl] 5-Levels KL not allowed "
                "with the current configuration");
    }

    if (((hdl->level == AUI_KL_KEY_THREE_LEVEL)
         || (hdl->level == AUI_KL_KEY_TWO_LEVEL))
        && (hdl->work_level_mode != AUI_KL_WORK_MODE_ALL))
        aui_rtn( AUI_RTN_FAIL, "[aui kl] 3-Levels KL not allowed "
                "with the current configuration");
    *handle = (aui_hdl) hdl;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_close(aui_hdl handle)
{
    struct kl_handler* hdl = (struct kl_handler *) handle;
    int err = AUI_RTN_SUCCESS;
    unsigned int i;

    osal_mutex_lock(mutex_kl,OSAL_WAIT_FOREVER_TIME);
    if ((!hdl) || (NULL==hdl->pst_dev_kl)) {
        goto err;/*default return success*/
    }

    if (hdl->level == AUI_KL_KEY_SKIP_LEVEL) {
        AUI_ERR("[aui kl] clear key, howto ?");
        err = AUI_RTN_FAIL;
        goto err;
    }

    for(i=0; i<hdl->ul_key_pos_cnt; i++) {
        /*128bit odd_even key pos is released tow times*/
        if(ce_ioctl(hdl->pst_dev_kl, IO_CRYPT_POS_SET_IDLE, hdl->ul_key_pos_out+i)) {
            AUI_ERR("[aui kl] key pos release fail!,hdl->ul_key_pos_out[%d]:%ul\n",i,hdl->ul_key_pos_out+i);
            err = AUI_RTN_FAIL;
            goto err;
        }
    }
    hdl->ul_key_pos_out = AUI_KL_KEY_POS_INVALID;/*reset key pos value*/
    hdl->ul_key_pos_cnt = 0;

    aui_dev_unreg(AUI_MODULE_KL,hdl);
    MEMSET(hdl,0,sizeof(struct kl_handler));
    FREE(hdl);
    osal_mutex_unlock(mutex_kl);
    return AUI_RTN_SUCCESS;
err:
    osal_mutex_unlock(mutex_kl);
    return err;
}

#define DEBUG_STAB()     do{\
    AUI_DBG("p_key_param->algo: %d,p_key_param->crypto_mode:%d,p_key_param->otp_addr:0x%08x,\n"\
    "p_key_param->parity:%d,p_key_param->pos:%d,p_key_param->protecting_key_num:%d,p_key_param->kl_index:%u,oper_size:%d\n",p_key_param->algo,\
        p_key_param->crypto_mode,p_key_param->otp_addr,p_key_param->parity,p_key_param->pos,\
        p_key_param->protecting_key_num,p_key_param->kl_index,single_key_size);\
    AUI_DUMP("p_key_param->protecting_key",(char*)p_key_param->protecting_key,64);\
    AUI_DUMP("p_key_param->content_key",(char*)p_key_param->content_key,16);}while(0)
static AUI_RTN_CODE generate_all_key(aui_hdl handle, CE_NLEVEL_PARAM *p_key_param, aui_cfg_kl *cfg,
                                   unsigned char *key_buf, int single_key_size)
{
    int temp;
    struct kl_handler* hdl = (struct kl_handler *) handle;
    
    switch(hdl->key_pattern) // Prepare the content_key and destination key_pos for generating key.
    {
        case AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE:    //64bit 
        case AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:
            switch (cfg->en_cw_key_attr){        
                case AUI_KL_CW_KEY_SINGLE:
                case AUI_KL_CW_KEY_EVEN:
                    /*the last level protected key map:
                     - Ciphered Content Key Odd (64 bit)
                     - Ciphered Content Key Even (64 bit)
                      so when generating ODD key, directly copy key_buf to content_key; when EVEN key ,must content_ket+8
                    */
                    MEMCPY(p_key_param->content_key+8,key_buf,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out ;
                    break;
                case AUI_KL_CW_KEY_ODD:
                    MEMCPY(p_key_param->content_key,key_buf,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out ;
                    break;
                case AUI_KL_CW_KEY_ODD_EVEN:
                    MEMCPY(p_key_param->content_key,key_buf,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out ;
                    break;
                default :
                    break;
            }
            break;
        case AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE : //128bit 
            switch(cfg->en_cw_key_attr){            
                case AUI_KL_CW_KEY_SINGLE:
                case AUI_KL_CW_KEY_EVEN:
                    MEMCPY(p_key_param->content_key,key_buf,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out;
                    break;
                case AUI_KL_CW_KEY_ODD:
                    MEMCPY(p_key_param->content_key,key_buf,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out + 1;    
                    break;
                default:
                    break;
            }
            break;
        case AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN :    //128bit_ODD_EVEN
            switch(cfg->en_cw_key_attr){
                case  AUI_KL_CW_KEY_ODD:
                case  AUI_KL_CW_KEY_ODD_EVEN:
                    //For the ( AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN , AUI_KL_CW_KEY_ODD_EVEN) case, here just prepare to generate ODD key first.
                    MEMCPY(p_key_param->content_key,key_buf,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out + 1;            
                    break;
                case AUI_KL_CW_KEY_EVEN:
                    temp = (cfg->en_cw_key_attr == AUI_KL_CW_KEY_ODD_EVEN) ? single_key_size : 0;
                    MEMCPY(p_key_param->content_key,key_buf + temp,single_key_size);
                    p_key_param->pos = hdl->ul_key_pos_out;/*generate 128bit even key*/
                    break;    
                default:
                    break;            
            }
            break;
        default:
            break;
    }
    
    DEBUG_STAB();
    // 128 bits only generate one key here, ODD key, or EVEN key. In that case which need generate two keys, here just generate ODD key first.
    if(RET_SUCCESS!=ce_ioctl(((struct kl_handler *)hdl)->pst_dev_kl, IO_CRYPT_GEN_NLEVEL_KEY, (unsigned long)p_key_param)) 
        aui_rtn(AUI_RTN_FAIL,"Gen key failed");
        
    if     ((AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN == hdl->key_pattern) && (cfg->en_cw_key_attr == AUI_KL_CW_KEY_ODD_EVEN))    //this condition call ce_ioctl again.
    {// In that case need to generate ODD & EVEN Key, here prepare content_key and key_pos, to generate EVEN KEY.
        temp = (cfg->en_cw_key_attr == AUI_KL_CW_KEY_ODD_EVEN) ? single_key_size : 0;
        MEMCPY(p_key_param->content_key,key_buf + temp,single_key_size);
        p_key_param->pos = hdl->ul_key_pos_out;/*generate 128bit even key*/
        DEBUG_STAB();
        /*ce_gen_nlevel_key*/
        if(RET_SUCCESS!=ce_ioctl(((struct kl_handler *)hdl)->pst_dev_kl, IO_CRYPT_GEN_NLEVEL_KEY, (unsigned long)p_key_param)) 
            aui_rtn(AUI_RTN_FAIL,"Gen key failed");    
    }
    return 0;
    
}
/* FIX ADDRESS MODE
 * Allocate keys for each stages
 * Perform derivation
 * De-allocate keys at each stage too
 * (For security no keys must stay in SRAM) */
AUI_RTN_CODE aui_kl_gen_key_by_cfg(aui_hdl handle, aui_cfg_kl *cfg,
                                   unsigned long *key_dst_pos)
{
    struct kl_handler* hdl = (struct kl_handler *) handle;
    CE_NLEVEL_PARAM gen_kl_key;
    int key_pos_counts = 0;
    CE_FOUND_FREE_POS_PARAM kl_key_res;
    /* operation size DES 64bits (8 bytes), AES 128bits (16 bytes) */
    /* =1,last protected control word use 8 byte,   =0,last protected control word use 16 byte*/
    int oper_size = -1;/*KL_END_SINGLE_CW_SIZE_128: KL_END_SINGLE_CW_SIZE_64*/

    MEMSET(&gen_kl_key,0,sizeof(CE_NLEVEL_PARAM ));

    if ((!hdl) || (!cfg) || (!key_dst_pos))
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");

    if (!hdl->pst_dev_kl)
        aui_rtn(AUI_RTN_FAIL, "ce in HLD layer not opened");

    if ((cfg->en_cw_key_attr >= AUI_KL_CW_KEY_ATTR_NB)
        || (cfg->en_crypt_mode >= AUI_KL_NB))
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");

    /*
      * algo    pattern          oper_size     ck_size   parity
      * TDES    64_SINGLE        8             8         no
      *         64_ODD_EVEN      8             8         yes
      *         128_SINGLE       8             16        no
      *         128_ODD_EVEN     8             16        yes
      * AES     128_SINGLE       16            16        no
      *         128_ODD_EVEN     16            16        yes
      */

    /* check key attribut with key pattern */
    /*When using #CE_SELECT_DES to generate key for DSC AES, TDES or
                    CSA3 (algo's key length is 128-bit), the 'parity' must be set to
                    #CE_PARITY_EVEN_ODD.
      When using #CE_SELECT_AES, this parameter can be ignored.              
      */
    switch (hdl->key_pattern) {
        case AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE:
            if ((cfg->en_cw_key_attr == AUI_KL_CW_KEY_SINGLE) 
                || (AUI_KL_CW_KEY_EVEN == cfg->en_cw_key_attr))
                gen_kl_key.parity = CE_PARITY_EVEN;
            else if((AUI_KL_CW_KEY_ODD == cfg->en_cw_key_attr))
                gen_kl_key.parity = CE_PARITY_ODD;
            else
                aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
            key_pos_counts = 1;
            oper_size = 8;
            break;
        case AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE:            
            if ((cfg->en_cw_key_attr != AUI_KL_CW_KEY_SINGLE)
                && (AUI_KL_CW_KEY_EVEN != cfg->en_cw_key_attr)
                && (AUI_KL_CW_KEY_ODD != cfg->en_cw_key_attr))
                aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
            gen_kl_key.parity = CE_PARITY_EVEN_ODD;
        
      if (AUI_KL_CW_KEY_SINGLE == cfg->en_cw_key_attr) {
        // Pure data only need one key pos
          key_pos_counts = 1;
      } else if ((AUI_KL_CW_KEY_ODD == cfg->en_cw_key_attr)
                  || (AUI_KL_CW_KEY_EVEN == cfg->en_cw_key_attr)){
         // When use EVEN/ODD key, that means we need to play TS data,
         // DSC need use two key pos, and the first key pos will be recognized as EVEN key pos.
         // In fact, Driver suggest use two key pos whether is PURE data or TS data when key size
         // is 128 bit to simplified the flow. 
         key_pos_counts = 2;
      }      
            oper_size = 16;
            break;
        case AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:
            if (AUI_KL_CW_KEY_EVEN == cfg->en_cw_key_attr){
                gen_kl_key.parity = CE_PARITY_EVEN;
                oper_size = 8;
            }else if (AUI_KL_CW_KEY_ODD == cfg->en_cw_key_attr){
                gen_kl_key.parity = CE_PARITY_ODD;
                oper_size = 8;
            }else if (AUI_KL_CW_KEY_ODD_EVEN == cfg->en_cw_key_attr){
                gen_kl_key.parity = CE_PARITY_EVEN_ODD;
                oper_size = 16;
            }else{
                aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
            }
            key_pos_counts = 1;
            break;
        case AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN:
            if ((cfg->en_cw_key_attr != AUI_KL_CW_KEY_ODD)
                && (cfg->en_cw_key_attr != AUI_KL_CW_KEY_EVEN)
                && (cfg->en_cw_key_attr != AUI_KL_CW_KEY_ODD_EVEN))
                aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
            oper_size = 16;
            gen_kl_key.parity = CE_PARITY_EVEN_ODD;    
            key_pos_counts  = 2;
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, "Invalid hdl->key_pattern\n");
			break;
    }

    if ( (AUI_KL_KEY_SKIP_LEVEL== hdl->level) ||
         (AUI_KL_KEY_SKIP_FOURLEVEL == hdl->level)) {
        AUI_DBG("[aui kl] clear key, howto ?");
        return AUI_RTN_SUCCESS;
    }

    /* check algo with key pattern */
    switch (cfg->en_kl_algo) {
        case AUI_KL_ALGO_AES:
        case AUI_KL_ALGO_TDES:
            hdl->algo = cfg->en_kl_algo;
            gen_kl_key.algo= (AUI_KL_ALGO_AES == cfg->en_kl_algo)?CE_SELECT_AES: CE_SELECT_DES;
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, "algo invalid");
			break;
    }

    gen_kl_key.crypto_mode = (AUI_KL_ENCRYPT == cfg->en_crypt_mode)?CE_IS_ENCRYPT: CE_IS_DECRYPT;
    /*Number of the protecting keys, 0, 1, 2 or 4. (Shall equal to the KL level subtract 1)*/
    osal_mutex_lock(mutex_kl,OSAL_WAIT_FOREVER_TIME);


    if(AUI_KL_KEY_POS_INVALID == hdl->ul_key_pos_out) {
        MEMSET(&kl_key_res,0,sizeof(CE_FOUND_FREE_POS_PARAM));
        if(AUI_KL_KEY_ONE_LEVEL == hdl->level)
            kl_key_res.ce_key_level = ONE_LEVEL;
        else if(AUI_KL_KEY_TWO_LEVEL == hdl->level)
            kl_key_res.ce_key_level = TWO_LEVEL;
        else if((AUI_KL_KEY_THREE_LEVEL == hdl->level) ||
                (AUI_KL_KEY_FIVE_LEVEL == hdl->level))
            kl_key_res.ce_key_level = THREE_LEVEL;
        kl_key_res.pos = INVALID_ALI_CE_KEY_POS;
        kl_key_res.number = key_pos_counts;/* middle kl level only get one pos*/

        kl_key_res.root = AUI_KL_INDEX(hdl->root_key_idx);
        if(AUI_RTN_SUCCESS!=aui_kl_res_take(hdl,&kl_key_res)) { /*get free key pos*/
            osal_mutex_unlock(mutex_kl);
            aui_rtn(AUI_RTN_FAIL,"get free key pos failed");
        }
        hdl->ul_key_pos_out = kl_key_res.pos;/*save key pos*/
        hdl->ul_key_pos_cnt = key_pos_counts;/*save key pos amount*/
    }
    gen_kl_key.kl_index = AUI_KL_INDEX(hdl->root_key_idx);
    gen_kl_key.otp_addr = AUI_KL_OTP_ADDR(hdl->root_key_idx);
    //gen_kl_key.otp_addr = OTP_ADDR((unsigned int)(hdl->root_key_idx & 0x0f));
    gen_kl_key.protecting_key_num = hdl->level -1;
    /* fill buffers and check ALGO */
    switch (hdl->level) {
        case AUI_KL_KEY_FIVE_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL:
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIVE:/*five level key must be generated once*/
                    MEMCPY(gen_kl_key.protecting_key,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                    MEMCPY(gen_kl_key.protecting_key + KL_MIDDLE_CW_SIZE,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,KL_MIDDLE_CW_SIZE);
                    MEMCPY(gen_kl_key.protecting_key + KL_MIDDLE_CW_SIZE*2,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2,KL_MIDDLE_CW_SIZE);
                    MEMCPY(gen_kl_key.protecting_key + KL_MIDDLE_CW_SIZE*3,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*3,KL_MIDDLE_CW_SIZE);
                    if(RET_SUCCESS != generate_all_key(hdl,&gen_kl_key,cfg,cfg->ac_key_val + KL_MIDDLE_CW_SIZE*4,oper_size)){
                        osal_mutex_unlock(mutex_kl);
                        aui_rtn(AUI_RTN_FAIL,"Gen key fail");
                    }
                    break;
                default:
                    osal_mutex_unlock(mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
					break;
            }
            break;
        }
        case AUI_KL_KEY_THREE_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST:
                    MEMCPY(hdl->big_key_buf,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                    hdl->pk_level1_valid = 1;
                    break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND:
                    if(hdl->pk_level1_valid) {
                        MEMCPY(hdl->big_key_buf + KL_MIDDLE_CW_SIZE,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                        hdl->pk_level2_valid = 1;
                    } else {
                        osal_mutex_unlock(mutex_kl);
                        aui_rtn(AUI_RTN_EINVAL, "should be set first level key!\n");
                    }
                    break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD:
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL: {/*three level key must be generated once*/
                    if(AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD == cfg->run_level_mode) {
                        if((!hdl->pk_level1_valid) || (!hdl->pk_level2_valid)) {
                            osal_mutex_unlock(mutex_kl);
                            aui_rtn(AUI_RTN_EINVAL, "should be set first and second level key!\n");
                        }
                        MEMCPY(gen_kl_key.protecting_key,hdl->big_key_buf,KL_MIDDLE_CW_SIZE);
                        MEMCPY(gen_kl_key.protecting_key + KL_MIDDLE_CW_SIZE,hdl->big_key_buf + KL_MIDDLE_CW_SIZE,KL_MIDDLE_CW_SIZE);
                        /*generate 64 even key ,64 odd even key,128 even/single key*/
                        if(RET_SUCCESS != generate_all_key(hdl,&gen_kl_key,cfg,cfg->ac_key_val,oper_size)){
                            osal_mutex_unlock(mutex_kl);
                            aui_rtn(AUI_RTN_FAIL,"Gen key fail");
                           }
                    } else { /*AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL*/
                        MEMCPY(gen_kl_key.protecting_key,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                        MEMCPY(gen_kl_key.protecting_key + KL_MIDDLE_CW_SIZE,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,KL_MIDDLE_CW_SIZE);
                        /*generate 64 even key ,64 odd even key,128 even/single key*/
                        if(RET_SUCCESS != generate_all_key(hdl,&gen_kl_key,cfg,cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2,oper_size)){
                            osal_mutex_unlock(mutex_kl);
                            aui_rtn(AUI_RTN_FAIL,"Gen key failed");
                           }
                    }
                    break;
                }
                default:
                    osal_mutex_unlock(mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
					break;
            }
            break;
        }
        case AUI_KL_KEY_TWO_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST:
                    MEMCPY(hdl->big_key_buf,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                    hdl->pk_level1_valid = 1;
                    break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND:
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL: { /*tow level key must be generated once*/
                    if(AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND == cfg->run_level_mode) {
                        if((!hdl->pk_level1_valid) || (!hdl->pk_level2_valid)) {
                            AUI_ERR("should be set first and second level key!\n");
                            osal_mutex_unlock(mutex_kl);
                            return AUI_RTN_EINVAL;
                        }
                        MEMCPY(gen_kl_key.protecting_key,hdl->big_key_buf,KL_MIDDLE_CW_SIZE);
                        /*generate 64 even key ,64 odd even key,128 even/single key*/
                        if(RET_SUCCESS != generate_all_key(hdl,&gen_kl_key,cfg,cfg->ac_key_val,oper_size)){
                            osal_mutex_unlock(mutex_kl);
                            aui_rtn(AUI_RTN_FAIL,"Gen key failed");
                           }
                    } else { /*AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL*/
                        MEMCPY(gen_kl_key.protecting_key,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                        /*generate 64 even key ,64 odd even key,128 even/single key*/
                        if(RET_SUCCESS != generate_all_key(hdl,&gen_kl_key,cfg,cfg->ac_key_val + KL_MIDDLE_CW_SIZE,oper_size)){
                            osal_mutex_unlock(mutex_kl);
                            aui_rtn(AUI_RTN_FAIL,"Gen key failed");
                           }
                    }
                    break;
                }
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD:
                    osal_mutex_unlock(mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL,
                            "run level mode invalid with AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST...");
					break;
                default:
                    osal_mutex_unlock(mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
					break;
            }
            break;
        }
        case AUI_KL_KEY_ONE_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST:
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL: {
                    /*generate 64 even key ,64 odd even key,128 even/single key*/
                    if(RET_SUCCESS != generate_all_key(hdl,&gen_kl_key,cfg,cfg->ac_key_val,oper_size)){
                        osal_mutex_unlock(mutex_kl);
                        aui_rtn(AUI_RTN_FAIL,"Gen key failed");
                       }
                    break;
                }
                default:
                    osal_mutex_unlock(mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
					break;
            }
            break;
        }
        default:
            osal_mutex_unlock(mutex_kl);
            aui_rtn(AUI_RTN_EINVAL, "level mode invalid ...");
            break;
    }

    *key_dst_pos = hdl->ul_key_pos_out;
    osal_mutex_unlock(mutex_kl);
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_load_hdcp_key(unsigned char *puc_key,unsigned long ul_byte_len)
{
//#ifndef AUI_CARDLESS_SUPPORT
#if 0
    unsigned int i=0;
    unsigned char temp_data[16]= {0};
    unsigned char *hdcp_internal_keys=NULL;
#endif
    if((NULL==puc_key)||(288!=ul_byte_len)) {
        return AUI_RTN_FAIL;
    }
//#ifdef AUI_CARDLESS_SUPPORT
#if 1
    p_ce_device ce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);

    if(RET_SUCCESS != ce_generate_hdcp_key(ce_dev, puc_key,ul_byte_len)) {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
#else
    CE_DATA_INFO Ce_data_info ;
    p_otp_param opt_info;
    p_ce_device ce_dev = (p_ce_device)dev_get_by_type(NULL, HLD_DEV_TYPE_CE);

    MEMSET(&Ce_data_info,0,sizeof(CE_DATA_INFO));

    hdcp_internal_keys = (unsigned char *)MALLOC(ul_byte_len);
    if(NULL==hdcp_internal_keys) {
        return AUI_RTN_FAIL;
    }
    MEMSET(hdcp_internal_keys ,0 , ul_byte_len);
    MEMCPY(hdcp_internal_keys,puc_key,ul_byte_len);

    opt_info = MALLOC(sizeof(OTP_PARAM));
    if(NULL==opt_info) {
        return AUI_RTN_FAIL;
    }
    MEMSET(opt_info ,0 , sizeof(OTP_PARAM) );

    opt_info->otp_addr = OTP_ADDESS_4; // load m2m2
    opt_info->otp_key_pos = KEY_0_3;

    if( RET_SUCCESS != ce_key_load(ce_dev , opt_info)) {
        FREE( opt_info );
        return !RET_SUCCESS;
    }

    for(i=0; i<ul_byte_len/16; i++) {
        MEMCPY(temp_data,(hdcp_internal_keys+i*16),16);

        MEMCPY(&(Ce_data_info.otp_info),opt_info,sizeof(OTP_PARAM));
        Ce_data_info.data_info.data_len                 = 16;
        Ce_data_info.des_aes_info.aes_or_des        = CE_SELECT_AES ;
        Ce_data_info.des_aes_info.crypt_mode        = CE_IS_DECRYPT;
        Ce_data_info.des_aes_info.des_low_or_high   = 0;
        Ce_data_info.key_info.first_key_pos             = KEY_0_3;
        Ce_data_info.key_info.hdcp_mode             = TARGET_IS_HDCP_KEY_SRAM;
        Ce_data_info.key_info.second_key_pos        = i;
        MEMCPY(Ce_data_info.data_info.crypt_data,temp_data,16);

        if(RET_SUCCESS != ce_generate_single_level_key(ce_dev, &Ce_data_info)) {
            FREE( opt_info );
            FREE(hdcp_internal_keys);
            return !RET_SUCCESS;
        }

    }

    FREE( opt_info );
    FREE(hdcp_internal_keys);
#endif
    return AUI_RTN_SUCCESS;
}


/* debug function */
AUI_RTN_CODE aui_kl_read_key(aui_hdl handle, unsigned char *key)
{
    if((NULL==handle)||(NULL==key)) {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_get(aui_hdl p_hdl_kl,unsigned long ul_item,void *pv_param)
{
    AUI_RTN_CODE rtn_code=AUI_RTN_SUCCESS;
    struct kl_handler *hdl = (struct kl_handler*)p_hdl_kl;
    //unsigned long ul_rtn=RET_FAILURE;
    osal_mutex_lock(mutex_kl,OSAL_WAIT_FOREVER_TIME);
    if((NULL == p_hdl_kl) || (NULL == pv_param)) {
        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    switch(ul_item) {
        case AUI_KL_GET_KEY_POS: {
            *(unsigned long *)pv_param = hdl->ul_key_pos_out;
            break;
        }
        case AUI_KL_GET_KEY_SIZE:
            if ((hdl->key_pattern == AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE)
                || (hdl->key_pattern == AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN)) {
                *(unsigned long *)pv_param= 64;
            } else
                *(unsigned long *)pv_param= 128;
            break;
        default: {
            rtn_code = AUI_RTN_FAIL;
            osal_mutex_unlock(mutex_kl);
            aui_rtn(AUI_RTN_FAIL,"Not supported command");
			break;
        }
    }
    osal_mutex_unlock(mutex_kl);
    return rtn_code;
}

AUI_RTN_CODE aui_kl_cf_target_set(aui_hdl handle, aui_kl_cf_target_attr *p_cf_target_attr)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
    // CF only is supported after C3505
#if !defined(_M3503_)&& !defined(_M3821_)

    int cf_ret = 0;
    int target_pos = 0;
    struct kl_handler* hdl = (struct kl_handler *) handle;
    CE_FOUND_FREE_POS_PARAM kl_key_res;

    if ((NULL == hdl) || (NULL == p_cf_target_attr))
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");

    if (!hdl->pst_dev_kl)
        aui_rtn(AUI_RTN_FAIL, "ce in HLD layer not opened");

    osal_mutex_lock(mutex_kl, OSAL_WAIT_FOREVER_TIME);
       

    // Allocate key pos for the target KL
    if(AUI_KL_KEY_POS_INVALID == hdl->ul_key_pos_out) {

        MEMSET(&kl_key_res,0,sizeof(CE_FOUND_FREE_POS_PARAM));
        // For CF's target KL, just set one level.
        kl_key_res.ce_key_level = ONE_LEVEL;
        kl_key_res.pos          = INVALID_ALI_CE_KEY_POS;
        kl_key_res.number       = 2; // Always allocate two key positions for target KL, because user maybe use for ODD and EVEN

        kl_key_res.root            = AUI_KL_INDEX(hdl->root_key_idx);
        
        // allocate free key pos
        ret = aui_kl_res_take(hdl,&kl_key_res);
        
        if (AUI_RTN_SUCCESS != ret) {
            osal_mutex_unlock(mutex_kl);
            aui_rtn(AUI_RTN_FAIL,"Get free key pos failed");
        }
        
        hdl->ul_key_pos_out = kl_key_res.pos;/*save key pos*/
        hdl->ul_key_pos_cnt = kl_key_res.number;/*save key pos count*/
    }

    // For CF target, only can be set to ODD or EVEN, can not set to ODD_EVEN
    if (AUI_KL_CW_KEY_ODD == p_cf_target_attr->parity) 
        target_pos = hdl->ul_key_pos_out + 1; // generate key to ODD pos
    else if (AUI_KL_CW_KEY_EVEN == p_cf_target_attr->parity)
        target_pos = hdl->ul_key_pos_out;  // generate key to EVEN pos
    else {
        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_FAIL, "Invalid parameter");
    }
        
    cf_ret = cf_set_target(target_pos);

    if (cf_ret != 0) {
        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_FAIL,"Set CF target failed");
    }

    osal_mutex_unlock(mutex_kl);

#endif
    
    return ret;
}

AUI_RTN_CODE aui_kl_gen_key_by_cfg_ext(aui_hdl handle, 
                                       aui_cfg_kl *cfg,
                                       aui_kl_key_source_attr *p_data_source,
                                       aui_kl_key_source_attr *p_key_source)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
    // CF is supported after C3505
#if !defined(_M3503_)&& !defined(_M3821_)

    struct kl_handler *hdl_final = (struct kl_handler *)handle;
    struct kl_handler *hdl_data = NULL;
    struct kl_handler *hdl_key = NULL;
    CE_CW_DERIVATION gen_key_cfg;
    CE_FOUND_FREE_POS_PARAM kl_key_res;
    int key_pos_counts = 0;

    // only can generate ONE key per one call, ODD, or EVEN

    if ((!hdl_final) || (!cfg) || (!p_data_source) || (!p_key_source))
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
    
    if (!hdl_final->pst_dev_kl)
        aui_rtn(AUI_RTN_FAIL, "ce in HLD layer not opened");

    osal_mutex_lock(mutex_kl, OSAL_WAIT_FOREVER_TIME);
    
    memset(&gen_key_cfg,0,sizeof(CE_CW_DERIVATION ));
    memset(&kl_key_res,0,sizeof(CE_FOUND_FREE_POS_PARAM));
     
    if ((cfg->en_cw_key_attr >= AUI_KL_CW_KEY_ATTR_NB)
        || (cfg->en_crypt_mode >= AUI_KL_NB)
        || (cfg->en_kl_algo != AUI_KL_ALGO_AES)) {  // CF case, KL only support AES now
     
        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
    }

    if ((p_data_source->key_source == AUI_KL_KEY_SOURCE_RAM)
        && (p_key_source->key_source == AUI_KL_KEY_SOURCE_RAM)) {

        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_FAIL, "data source and key source cannot all be from RAM!\n");
    }
    

    // First step, check & allocate key pos for the final KL
    if (AUI_KL_KEY_POS_INVALID == hdl_final->ul_key_pos_out) {
        
        // One key position size is 128bits, here check how many key positions need.
        switch (hdl_final->key_pattern) {
            case AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE:
            case AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE:
                // only output one key
                key_pos_counts = 1;
                break;
            case AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:
                key_pos_counts = 1;
                break;
            case AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN:
                key_pos_counts = 2;
                break;
            default:
                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid hdl->key_pattern\n");
				break;
        }
        
        if (AUI_KL_KEY_ONE_LEVEL == hdl_final->level)
            kl_key_res.ce_key_level = ONE_LEVEL;
        else if (AUI_KL_KEY_TWO_LEVEL == hdl_final->level)
            kl_key_res.ce_key_level = TWO_LEVEL;
        else if ((AUI_KL_KEY_THREE_LEVEL == hdl_final->level)
                  ||(AUI_KL_KEY_FIVE_LEVEL == hdl_final->level))
            kl_key_res.ce_key_level = THREE_LEVEL;
         
        kl_key_res.pos = INVALID_ALI_CE_KEY_POS;
        kl_key_res.number = key_pos_counts;
        kl_key_res.root = AUI_KL_INDEX(hdl_final->root_key_idx);

        if (AUI_RTN_SUCCESS!=aui_kl_res_take(hdl_final,&kl_key_res)) { /*get free key pos*/
            osal_mutex_unlock(mutex_kl);
            aui_rtn(AUI_RTN_FAIL,"aui_kl_res_take failed");
        }

        hdl_final->ul_key_pos_out = kl_key_res.pos;/*save key pos*/
        hdl_final->ul_key_pos_cnt = key_pos_counts;/*save key pos amount*/
    }

    // Second step, generate key
    hdl_final->algo  = cfg->en_kl_algo;
    gen_key_cfg.algo = CE_SELECT_AES; // CF case, KL only support AES now
    
    if (AUI_KL_ENCRYPT == cfg->en_crypt_mode)
        gen_key_cfg.crypto_mode = CE_IS_ENCRYPT;
    else 
        gen_key_cfg.crypto_mode = CE_IS_DECRYPT;
    
    /*data source configuration */
    switch(p_data_source->key_source)
    {
        case AUI_KL_KEY_SOURCE_RAM:
            gen_key_cfg.data_src = CE_DATA_IN_FROM_CPU; // in DRAM
            memcpy(gen_key_cfg.data.buf,p_data_source->key_param.buf,16);
            break;
        case AUI_KL_KEY_SOURCE_KEY_LADDER:
            hdl_data = (struct kl_handler *)p_data_source->key_param.key_ladder_handle;
            
            if (!hdl_data) {
                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
            }
            
            gen_key_cfg.data_src = CE_DATA_IN_FROM_SRAM; // in KL SRAM
            
            if(hdl_data->ul_key_pos_out == AUI_KL_KEY_POS_INVALID) {

                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
            }

            if (AUI_KL_CW_KEY_ODD == p_data_source->key_param.parity)            
                gen_key_cfg.data.pos = hdl_data->ul_key_pos_out+1;  // odd key pos
            else if (AUI_KL_CW_KEY_EVEN == p_data_source->key_param.parity)
                gen_key_cfg.data.pos = hdl_data->ul_key_pos_out; // even key pos
            else {
                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
            }
            
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, "Not support command");
			break;
    }

    /*key source configuration */
    switch(p_key_source->key_source)
    {
        case AUI_KL_KEY_SOURCE_RAM:
            gen_key_cfg.key_src = CE_KEY_FROM_CPU; // in DRAM
            memcpy(gen_key_cfg.key.buf,p_key_source->key_param.buf,16);
            break;
        case AUI_KL_KEY_SOURCE_KEY_LADDER:
            hdl_key = (struct kl_handler *)p_key_source->key_param.key_ladder_handle;
            
            if (!hdl_key) {
                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
            }
            
            gen_key_cfg.key_src = CE_KEY_FROM_SRAM;
            
            if (hdl_key->ul_key_pos_out == AUI_KL_KEY_POS_INVALID) {
                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
            }
            
            if (AUI_KL_CW_KEY_ODD == p_key_source->key_param.parity)
                gen_key_cfg.key.pos = hdl_key->ul_key_pos_out+1;  // odd key pos
            else if (AUI_KL_CW_KEY_EVEN == p_key_source->key_param.parity)
                gen_key_cfg.key.pos = hdl_key->ul_key_pos_out;  // even key pos
            else {
                osal_mutex_unlock(mutex_kl);
                aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
            }
            
            break;
        default:
            osal_mutex_unlock(mutex_kl);
            aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
			break;
    }

    if (cfg->en_cw_key_attr == AUI_KL_CW_KEY_ODD) {
        gen_key_cfg.target_pos = hdl_final->ul_key_pos_out+1; // Generate ODD key 
    } else if (cfg->en_cw_key_attr == AUI_KL_CW_KEY_EVEN) {
        gen_key_cfg.target_pos = hdl_final->ul_key_pos_out;  // Generate EVEN key
    }else {
        osal_mutex_unlock(mutex_kl);
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter");
    }
    
    // generate final key 
    if (RET_SUCCESS != ce_ioctl(hdl_final->pst_dev_kl, IO_CRYPT_CW_DERIVE_CW, (unsigned long)(&gen_key_cfg))) {
        ret = AUI_RTN_FAIL;
    }

    osal_mutex_unlock(mutex_kl);

#endif

    return ret;
}


