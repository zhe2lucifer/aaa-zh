/**@file
 *    @brief     ALi AUI KL (key ladder) function implementation
 *    @author    romain.baeriswyl
 *    @date      2014-03-24
 *    @version   0.0.1
 *    @note      ali corp. all rights reserved. 2013-2999 copyright (C)
 */
#include "aui_kl_common.h"
#include "aui_common_priv.h"

AUI_MODULE(KL)

static pthread_mutex_t mutex_kl = PTHREAD_MUTEX_INITIALIZER;

board_config_t board_conf;

AUI_RTN_CODE aui_kl_read_key(aui_hdl handle, unsigned char *key);

AUI_RTN_CODE aui_kl_read_otp(board_config_t *board_config)
{

    int ret = AUI_RTN_SUCCESS;

    /* Read "fixed engine mode" OTP */
    unsigned int otp_03 = 0;
    unsigned int otp_dc = 0;
    unsigned int otp_5f = 0;
    unsigned int otp_84 = 0;

    if (NULL == board_config) {
        aui_rtn(AUI_RTN_FAIL, "level ERROR");
    }

    ret = alislotp_op_read(OTP_CONFIGURATION_0 * 4, (unsigned char *) &otp_03,
                           OTP_LEN);
    if (ret < 0)
        aui_rtn(AUI_RTN_FAIL, "otp read error");

    ret = alislotp_op_read(OTP_CONFIGURE_3 * 4, (unsigned char *) &otp_dc,
                           OTP_LEN);
    if (ret < 0)
        aui_rtn(AUI_RTN_FAIL, "otp read error");

    /* 5- Levels derivation mode allowed */
    ret = alislotp_op_read(OTP_FIVE_LEVELS_MODE * 4, (unsigned char *) &otp_5f,
                           OTP_LEN);
    if (ret < 0)
        aui_rtn(AUI_RTN_FAIL, "otp read error");

    /* CAS part number*/
    ret = alislotp_op_read(OTP_APP_INFORMATION * 4, (unsigned char *) &otp_84,
                           OTP_LEN);
    if (ret < 0)
        aui_rtn(AUI_RTN_FAIL, "otp read error");

    board_config->fix_addr_mode = ((otp_03 & 0x0400) >> 14);
    board_config->fix_engine_mode = ((otp_dc & 0x80) >> 7);

    /* to be removed, just for test */
    //board_config->five_levels_only = 0x01;
    //board_config->cas_vendor_id = 0x17;

    if ((board_conf.five_levels_only == 0x01)
        && (board_conf.cas_vendor_id == 0x17)) {
        board_config->work_mode = AUI_KL_WORK_MODE_5_LEVELS_ONLY;
    }

    /*
     *
     * In normal mode 0x03 OTP
     * Test TRI_LADDER_MODE_1&2[15] = 0
     * Test TRI_LADDER_MODE_1&2[15] = 0
     * 0xDC OTP
     * TRI_LADDER_MODE_3[15:14] = 0
     * TRI_LADDER_MODE_4[17:16] = 0
     *
     * In Fix Address 0x03 OTP
     * Test TRI_LADDER_MODE_1&2[15] = 1
     * Test TRI_LADDER_MODE_1&2[15] = 1
     * 0xDC OTP
     * FIX_ADDR_MODE[14] = 1
     * TRI_LADDER_MODE_3[15:14] = 11 3-level mode
     * TRI_LADDER_MODE_4[17:16] = 11
     * or
     * TRI_LADDER_MODE_3[15:14] = 10 2-level mode
     * TRI_LADDER_MODE_4[17:16] = 10
     *
     * In Fix Engine 0xDC
     * KEY_LADDER_FIX_ENGINE_EN [7] = 1
     * FIX_ADDR_MODE[14] = 0|1
     *
     */

    if (board_config->fix_engine_mode
        && (board_config->fix_addr_mode == 0
            || board_config->fix_addr_mode == 1)) {

        board_config->board_mode = AUI_KL_MODE_FIX_ENGINE;

    } else if (board_config->fix_addr_mode
               && board_config->fix_engine_mode == 0) {

        board_config->board_mode = AUI_KL_MODE_FIX_ADDRESS;

    } else {

        board_config->board_mode = AUI_KL_MODE_NORMAL;
    }

    return ret;
}

AUI_RTN_CODE aui_kl_open(aui_attr_kl *attr, aui_hdl *handle)
{
    struct kl_handler *hdl;
    struct kl_config_key  cfg_kl_key;

    if (!attr || !handle || attr->en_level >= AUI_KL_KEY_LEVEL_NB
        || attr->en_key_pattern >= AUI_KL_OUTPUT_KEY_PATTERN_NB)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    int root_key = KEY_POS(attr->en_root_key_idx);
    if (root_key == INVALID_ALI_CE_KEY_POS)
        aui_rtn(AUI_RTN_EINVAL, "invalid root key position");
	if(attr->en_root_key_idx == AUI_KL_ROOT_KEY_0_3){
		aui_rtn(AUI_RTN_EINVAL, "rootkey = AUI_KL_ROOT_KEY_0_3 is only used to generate hdcp key!\n");
	}
    hdl = (struct kl_handler *) calloc(sizeof(struct kl_handler), 1);
    if (!hdl)
        aui_rtn(AUI_RTN_ENOMEM, NULL);

    hdl->data.dev_idx = attr->uc_dev_idx;

	if(AUI_KL_TYPE_ALI == attr->en_key_ladder_type){
	    if (alislce_open(&hdl->dev,root_key,SL_CE_ROOT_KEY_DEFAULT)) {
	        aui_kl_close(hdl);
	        aui_rtn(AUI_RTN_FAIL, "alislce_open error");
	    }
	}else if(AUI_KL_TYPE_ETSI == attr->en_key_ladder_type){
		if (alislce_open(&hdl->dev,root_key,SL_CE_ROOT_KEY_ETSI)) {
	        aui_kl_close(hdl);
	        aui_rtn(AUI_RTN_FAIL, "alislce_open error");
	    }
	} else if(AUI_KL_TYPE_CONAXVSC == attr->en_key_ladder_type){
		if (alislce_open(&hdl->dev,root_key,SL_CE_ROOT_KEY_CONAXVSC)) {
	        aui_kl_close(hdl);
	        aui_rtn(AUI_RTN_FAIL, "alislce_open error");
	    }
	}else{
    	aui_kl_close(hdl);
        aui_rtn(AUI_RTN_ENOMEM, NULL);
	}

    hdl->key_ladder_type = attr->en_key_ladder_type;
    hdl->level = attr->en_level;
    hdl->key_pattern = attr->en_key_pattern;
    hdl->root_key_idx = (int) attr->en_root_key_idx;

    /* clear key size in bytes */
    if (hdl->key_pattern == AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE
        || hdl->key_pattern == AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN)
    {
        hdl->ck_size = 8;
	 if(AUI_KL_TYPE_CONAXVSC == attr->en_key_ladder_type)
       {
           cfg_kl_key.ck_size = KL_CK_KEY64;
           if(alislce_create_key_hdl(hdl->dev,&cfg_kl_key))
		   aui_rtn(AUI_RTN_EIO, "config KL for KL_CK_KEY64 fail");
       }
    }
    else
    {
        hdl->ck_size = 16;
	  if(AUI_KL_TYPE_CONAXVSC == attr->en_key_ladder_type)
        {
           cfg_kl_key.ck_size = KL_CK_KEY128;
           if(alislce_create_key_hdl(hdl->dev,&cfg_kl_key))
		   aui_rtn(AUI_RTN_EIO, "config KL for KL_CK_KEY128 fail");
        }
    }

    if(AUI_KL_KEY_FIVE_LEVEL == hdl->level) {
        hdl->work_level_mode = AUI_KL_WORK_MODE_5_LEVELS_ONLY;
    }
    /* check level with board config */
    //if ((board_conf.work_mode == AUI_KL_WORK_MODE_5_LEVELS_ONLY)
    if ((hdl->work_level_mode == AUI_KL_WORK_MODE_5_LEVELS_ONLY)
        && (hdl->level != AUI_KL_KEY_FIVE_LEVEL)) {
        free(hdl);
        aui_rtn(AUI_RTN_FAIL, "5-Levels KL not allowed "
                "with the current configuration");
    }

    //if ((board_conf.work_mode != AUI_KL_WORK_MODE_5_LEVELS_ONLY)
    if ((hdl->work_level_mode != AUI_KL_WORK_MODE_5_LEVELS_ONLY)
        && (hdl->level == AUI_KL_KEY_FIVE_LEVEL)) {
        free(hdl);
        aui_rtn(AUI_RTN_FAIL, "5-Levels KL not allowed "
                "with the current configuration");
    }

    if (((hdl->level == AUI_KL_KEY_THREE_LEVEL)
         || (hdl->level == AUI_KL_KEY_TWO_LEVEL))
        && (hdl->work_level_mode != AUI_KL_WORK_MODE_ALL)){
        free(hdl);
        aui_rtn(AUI_RTN_FAIL, "3-Levels KL not allowed "
                "with the current configuration");
    }
	if (aui_dev_reg(AUI_MODULE_KL, hdl)) {
        free(hdl);
        aui_rtn(AUI_RTN_EINVAL, "dev idx already used");
    }
    *handle = (aui_hdl) hdl;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_close(aui_hdl handle)
{
    struct kl_handler* hdl = (struct kl_handler *) handle;
    int err = AUI_RTN_SUCCESS;
    int i;

    if (!hdl)
        return AUI_RTN_SUCCESS;

    if (hdl->level == AUI_KL_KEY_SKIP_LEVEL) {
        AUI_DBG("clear key, howto ?");
        return AUI_RTN_SUCCESS;
    }

    if (hdl->dev) {
        /* Free all key positions */
        if (alislce_close(hdl->dev)) {
            err = AUI_RTN_FAIL;
            AUI_ERR("alislce_close errror");
        }
    }

    if (aui_dev_unreg(AUI_MODULE_KL, hdl)) {
        err = AUI_RTN_FAIL;
        AUI_ERR("aui_dev_unreg error");
    }

    for (i = 0; i < hdl->level; i++) {
        hdl->key_info[i].len = 0;
        hdl->key_info[i].pos = 0;
    }

    /* Reset all buffers status*/
    hdl->pk_level1_valid = 0;
    hdl->pk_level2_valid = 0;
    hdl->pk_level3_valid = 0;
    hdl->pk_level4_valid = 0;

    free(hdl);
    return err;
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
    struct kl_config_key  cfg_kl_key;
    struct kl_gen_key gen_kl_key;

    memset(&cfg_kl_key,0,sizeof(struct kl_config_key ));
    memset(&gen_kl_key,0,sizeof(struct kl_gen_key ));

    if (!hdl || !cfg || !key_dst_pos)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    if (!hdl->dev)
        aui_rtn(AUI_RTN_FAIL, "sl not opened");

    if (cfg->en_cw_key_attr >= AUI_KL_CW_KEY_ATTR_NB
        || cfg->en_crypt_mode >= AUI_KL_NB)
        aui_rtn(AUI_RTN_EINVAL, NULL);

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
    switch (hdl->key_pattern) {
        case AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE:
		case AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE:
			if ((AUI_KL_CW_KEY_SINGLE ==  cfg->en_cw_key_attr) 
				|| (AUI_KL_CW_KEY_EVEN == cfg->en_cw_key_attr)){
	        	cfg_kl_key.ck_parity = KL_CK_PARITY_EVEN;
				gen_kl_key.run_parity = KL_CK_PARITY_EVEN;/*only update even key*/
			}else if((AUI_KL_CW_KEY_ODD == cfg->en_cw_key_attr)){
				cfg_kl_key.ck_parity = KL_CK_PARITY_ODD;
				gen_kl_key.run_parity = KL_CK_PARITY_ODD;/*only update odd key*/
			}else
				aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
			cfg_kl_key.ck_size = (AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE == hdl->key_pattern)?KL_CK_KEY64: KL_CK_KEY128;
			break;
        case AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:
        case AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN:
            if ((AUI_KL_CW_KEY_SINGLE == cfg->en_cw_key_attr)
				|| (AUI_KL_CW_KEY_EVEN == cfg->en_cw_key_attr)){
				gen_kl_key.run_parity = KL_CK_PARITY_EVEN;/*only update even key*/
			}else if (AUI_KL_CW_KEY_ODD == cfg->en_cw_key_attr){
				gen_kl_key.run_parity = KL_CK_PARITY_ODD;/*only update odd key*/
			}else if (AUI_KL_CW_KEY_ODD_EVEN == cfg->en_cw_key_attr){
				gen_kl_key.run_parity = KL_CK_PARITY_ODD_EVEN;/*update even and odd key*/
			}else{
				aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
			}
			cfg_kl_key.ck_parity = KL_CK_PARITY_ODD_EVEN;/*odd and even key mode*/
			cfg_kl_key.ck_size = (AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN == hdl->key_pattern)?KL_CK_KEY64: KL_CK_KEY128;/*single key length*/
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, "Invalid hdl->key_pattern\n");
    }

    if (hdl->level == AUI_KL_KEY_SKIP_LEVEL) {
        AUI_DBG("clear key, howto ?");
        return AUI_RTN_SUCCESS;
    }

    /* check algo with key pattern */
    switch (cfg->en_kl_algo) {
        case AUI_KL_ALGO_AES:
        case AUI_KL_ALGO_TDES:
            hdl->algo = cfg->en_kl_algo;
            cfg_kl_key.algo= (AUI_KL_ALGO_AES == cfg->en_kl_algo)?KL_ALGO_AES: KL_ALGO_TDES;
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, "algo invalid");
    }

    cfg_kl_key.crypt_mode = (AUI_KL_ENCRYPT == cfg->en_crypt_mode)?KL_ENCRYPT: KL_DECRYPT;
    cfg_kl_key.level = hdl->level;
    pthread_mutex_lock(&mutex_kl);
    /*create key handle*/
    if(alislce_create_key_hdl(hdl->dev,&cfg_kl_key)) {
        AUI_ERR("create kl key handle error!\n");
        pthread_mutex_unlock(&mutex_kl);
        aui_rtn(AUI_RTN_EIO, NULL);
    }
    /* operation size DES 64bits (8 bytes), AES 128bits (16 bytes) */
	/* =1,last protected control word use 8 byte,   =0,last protected control word use 16 byte*/
    int oper_size = (cfg_kl_key.ck_size == KL_CK_KEY128) ? KL_END_SINGLE_CW_SIZE_128: KL_END_SINGLE_CW_SIZE_64;

    /* fill buffers and check ALGO */
    switch (hdl->level) {
        case AUI_KL_KEY_FIVE_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL:
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIVE:/*five level key must be generated once*/
                    memcpy(gen_kl_key.pk[0],(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                    memcpy(gen_kl_key.pk[1],(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,KL_MIDDLE_CW_SIZE);
                    memcpy(gen_kl_key.pk[2],(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2,KL_MIDDLE_CW_SIZE);
                    memcpy(gen_kl_key.pk[3],(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*3,KL_MIDDLE_CW_SIZE);
					if(KL_CK_PARITY_EVEN == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*4,oper_size);
					else if(KL_CK_PARITY_ODD == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*4,oper_size);
					else{
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*4, oper_size);
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*4+oper_size, oper_size);
					}

                    if(alislce_generate_all_level_key(hdl->dev,&gen_kl_key)) {
                        AUI_ERR("create kl key handle error!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EIO, NULL);
                    }
                    break;
                default:
					pthread_mutex_unlock(&mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
            }
            break;
        }
        case AUI_KL_KEY_THREE_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST:
					memcpy(hdl->big_key_buf,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
					hdl->pk_level1_valid = 1;
					break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND:
					if(hdl->pk_level1_valid){
						memcpy(hdl->big_key_buf + KL_MIDDLE_CW_SIZE,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
						hdl->pk_level2_valid = 1;
					}else{
						AUI_ERR("should be set first level key!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EINVAL, NULL);
					}
					break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD:
					if(!hdl->pk_level1_valid || !hdl->pk_level2_valid){
						AUI_ERR("should be set first and second level key!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EINVAL, NULL);
					}
					memcpy(gen_kl_key.pk[0],(char *) hdl->big_key_buf,KL_MIDDLE_CW_SIZE);
                    memcpy(gen_kl_key.pk[1],(char *) hdl->big_key_buf + KL_MIDDLE_CW_SIZE,KL_MIDDLE_CW_SIZE);
					if(KL_CK_PARITY_EVEN == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val,oper_size);
					else if(KL_CK_PARITY_ODD == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val,oper_size);
					else{
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val,oper_size);
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + oper_size,oper_size);
						
					}
	                if(alislce_generate_all_level_key(hdl->dev,&gen_kl_key)) {
                        AUI_ERR("create kl key handle error!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EIO, NULL);
                    }
                    break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL: /*three level key must be generated once*/
                    memcpy(gen_kl_key.pk[0],(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                    memcpy(gen_kl_key.pk[1],(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,KL_MIDDLE_CW_SIZE);
					if(KL_CK_PARITY_EVEN == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2,oper_size);
					else if(KL_CK_PARITY_ODD == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2,oper_size);
					else{
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2,oper_size);
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE*2+oper_size,oper_size);
						
					}
                    if(alislce_generate_all_level_key(hdl->dev,&gen_kl_key)) {
                        AUI_ERR("create kl key handle error!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EIO, NULL);
                    }
                    break;
                default:
                    pthread_mutex_unlock(&mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
            }
            break;
        }
        case AUI_KL_KEY_TWO_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST:
					memcpy(hdl->big_key_buf,(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
					hdl->pk_level1_valid = 1;
					break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_SECOND:
					if(!hdl->pk_level1_valid ){
						AUI_ERR("should be set first level key!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EINVAL, NULL);
					}
					memcpy(gen_kl_key.pk[0],(char *) hdl->big_key_buf,KL_MIDDLE_CW_SIZE);
					if(KL_CK_PARITY_EVEN == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val,oper_size);
					else if(KL_CK_PARITY_ODD == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val,oper_size);
					else{
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val,oper_size);
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + oper_size,oper_size);
						
					}
	                if(alislce_generate_all_level_key(hdl->dev,&gen_kl_key)) {
                        AUI_ERR("create kl key handle error!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EIO, NULL);
                    }
					break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL:/*tow level key must be generated once*/
                    memcpy(gen_kl_key.pk[0],(char *) cfg->ac_key_val,KL_MIDDLE_CW_SIZE);
                    //memcpy(gen_kl_key.pk[1],(char *) cfg->ac_key_val + oper_size,oper_size);
					if(KL_CK_PARITY_EVEN == gen_kl_key.run_parity)
                    	memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,oper_size);
					else if(KL_CK_PARITY_ODD == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,oper_size);
					else{
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE,oper_size);
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + KL_MIDDLE_CW_SIZE+oper_size,oper_size);
					}
                    if(alislce_generate_all_level_key(hdl->dev,&gen_kl_key)) {
                        AUI_DBG("create kl key handle error!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EIO, NULL);
                    }
                    break;
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_THIRD:
                    pthread_mutex_unlock(&mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL,
                            "run level mode invalid with AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST...");
                default:
                    pthread_mutex_unlock(&mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
            }
            break;
        }
        case AUI_KL_KEY_ONE_LEVEL: {
            switch (cfg->run_level_mode) {
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_FIRST:
                case AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL:
					if(KL_CK_PARITY_EVEN == gen_kl_key.run_parity)
                    	memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val ,oper_size);
					else if(KL_CK_PARITY_ODD == gen_kl_key.run_parity)
						memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val,oper_size);
					else{
                    	memcpy(gen_kl_key.key_odd,(char *) cfg->ac_key_val,oper_size);
						memcpy(gen_kl_key.key_even,(char *) cfg->ac_key_val + oper_size,oper_size);
					}
                    if(alislce_generate_all_level_key(hdl->dev,&gen_kl_key)) {
                        AUI_DBG("create kl key handle error!\n");
                        pthread_mutex_unlock(&mutex_kl);
                        aui_rtn(AUI_RTN_EIO, NULL);
                    }
                    break;
                default:
                    pthread_mutex_unlock(&mutex_kl);
                    aui_rtn(AUI_RTN_EINVAL, "run level mode invalid ...");
            }
            break;
        }
        default:
            pthread_mutex_unlock(&mutex_kl);
            aui_rtn(AUI_RTN_EINVAL, "level mode invalid ...");
            break;
    }

    if(alislce_get_key_fd(hdl->dev,key_dst_pos)) {
        pthread_mutex_unlock(&mutex_kl);
        aui_rtn(AUI_RTN_EIO, "get key fd error!");
    }

#if 0
    unsigned char buf[16];
    int j = 0;
    memset(buf,0,sizeof(int)*4);
    aui_kl_read_key(hdl->dev,buf);
    AUI_DBG("\nkey:");
    for(j = 0; j < 16; j++)
        AUI_DBG("%02x",buf[j]);
    AUI_DBG("\n");
#endif

    pthread_mutex_unlock(&mutex_kl);
    return AUI_RTN_SUCCESS;
#if 0
GENERATION_FAILED:

    pthread_mutex_unlock(&mutex_kl);
    return AUI_RTN_EINVAL;
#endif
}

/*
 * KL Debug function enabled
 * if OTP 0x3[bit 2] and OTP 0xDC[bit 24..22] are zero.
 */
AUI_RTN_CODE aui_kl_read_key(aui_hdl handle, unsigned char *key)
{

    if (alislce_get_decrypt_key(handle, key))
        aui_rtn(AUI_RTN_FAIL, "alislce_get_decrypt_key error");

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_version_get(unsigned long *pul_version)
{
    if (!pul_version)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    *pul_version = AUI_MODULE_VERSION_NUM_KL;
    return AUI_RTN_SUCCESS;
}

/* Reserved function */
AUI_RTN_CODE aui_kl_init(p_fun_cb p_call_back, void *pv_param)
{

    if (p_call_back != NULL) {
        p_call_back(pv_param);
    }

    return AUI_RTN_SUCCESS;
}

/* Reserved function */
AUI_RTN_CODE aui_kl_de_init(p_fun_cb p_call_back, void *pv_param)
{
    if (!p_call_back)
        aui_rtn(AUI_RTN_EINVAL, NULL);
    return p_call_back(pv_param);
}

AUI_RTN_CODE aui_kl_set(aui_hdl handle, unsigned long cmd, void *param)
{
    struct kl_handler* hdl = (struct kl_handler *) handle;

    if (!hdl || !param || (long) cmd >= AUI_KL_SET_CMD_NB)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_kl_get(aui_hdl handle, unsigned long cmd, void *param)
{
    struct kl_handler* hdl = (struct kl_handler *) handle;

    if (!hdl || !param)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    switch (cmd) {
        case AUI_KL_GET_KEY_POS:
            if(alislce_get_key_fd(hdl->dev, (unsigned long *)param))
                aui_rtn(AUI_RTN_EIO, NULL);
            break;
        case AUI_KL_GET_KEY_SIZE:
            /* WARNING: For AUI DSC, key length unit is bit */
            *(unsigned long *) param = hdl->ck_size * 8;
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, NULL);
            break;
    }
    return AUI_RTN_SUCCESS;
}

#define HDCP_KEY_LEN 288 /* bytes */

AUI_RTN_CODE aui_kl_load_hdcp_key(unsigned char *key, unsigned long len)
{
    int err = AUI_RTN_SUCCESS;
    alisl_handle dev;
    struct kl_config_key  cfg_kl_key;
    struct kl_gen_hdcp_key gen_hdcp_key;

    if (!key || len != HDCP_KEY_LEN) {
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameters");
    }

    if (alislce_open(&dev,AUI_KL_ROOT_KEY_0_3,SL_CE_ROOT_KEY_DEFAULT))/*at address 0x59 */
        aui_rtn(AUI_RTN_FAIL, "alislce_open error");

    memset(&cfg_kl_key, 0, sizeof(struct kl_config_key));
    cfg_kl_key.algo = KL_ALGO_AES;
    cfg_kl_key.crypt_mode = KL_DECRYPT;
    cfg_kl_key.level = 1;

    /*create key handle*/
    if(alislce_create_key_hdl(dev,&cfg_kl_key)) {
        err = AUI_RTN_FAIL;
        AUI_DBG("create kl key handle error!\n");
        goto hdcp_exit;
    }

    memset(&gen_hdcp_key, 0, sizeof(struct kl_gen_hdcp_key));
    memcpy(gen_hdcp_key.hdcp_pk,key,len);
	/*encrypted hdcp key and load it to HDMI*/
    if(alislce_gen_hdcp_key(dev,&gen_hdcp_key)) {
        err = AUI_RTN_FAIL;
        aui_rtn(AUI_RTN_FAIL, "gen hdcp error");
        goto hdcp_exit;
    }

hdcp_exit:
    if (alislce_close(dev)) {
        err = AUI_RTN_FAIL;
        AUI_ERR("alislce_close errror");
    }
    return err;
}


AUI_RTN_CODE aui_kl_cf_target_set(aui_hdl handle, aui_kl_cf_target_attr *p_cf_target_attr)
{
	struct kl_handler* hdl = (struct kl_handler *) handle;
    struct kl_config_key  cfg_kl_key;
	enum sl_ce_key_parity sl_parity;

    memset(&cfg_kl_key,0,sizeof(struct kl_config_key ));

    if (!hdl)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    if (!hdl->dev)
        aui_rtn(AUI_RTN_FAIL, "sl not opened");
	if (AUI_KL_CW_KEY_ODD == p_cf_target_attr->parity){
		cfg_kl_key.ck_parity = KL_CK_PARITY_ODD;
		sl_parity = SL_CE_KEY_PARITY_ODD;
	}else if (AUI_KL_CW_KEY_EVEN == p_cf_target_attr->parity){
		cfg_kl_key.ck_parity = KL_CK_PARITY_EVEN;
		sl_parity = SL_CE_KEY_PARITY_EVEN;
	}else
		aui_rtn(AUI_RTN_EINVAL, NULL);
	
	/*for CF module,only support AES,cfg_kl_key.ck_size must be equal to 128bits*/
	cfg_kl_key.ck_size = KL_CK_KEY128;
 	hdl->algo = AUI_KL_ALGO_AES;
	cfg_kl_key.algo= KL_ALGO_AES;

    cfg_kl_key.crypt_mode = KL_DECRYPT;/*no used*/
    cfg_kl_key.level = 1;/*misleading*/
    pthread_mutex_lock(&mutex_kl);

	/*create key handle*/
    if(alislce_create_key_hdl(hdl->dev,&cfg_kl_key)) {
        AUI_ERR("create kl key handle error!\n");
        pthread_mutex_unlock(&mutex_kl);
        aui_rtn(AUI_RTN_EIO, NULL);
    }
	
	if(alislcf_set_target_pos(hdl->dev,sl_parity)){
		AUI_ERR("set cf target pos error!");
        pthread_mutex_unlock(&mutex_kl);
        aui_rtn(AUI_RTN_EIO, NULL);
	}
	
	pthread_mutex_unlock(&mutex_kl);
	return 0;
}

AUI_RTN_CODE aui_kl_gen_key_by_cfg_ext(aui_hdl handle, 
	aui_cfg_kl *cfg,
	aui_kl_key_source_attr *p_data_source,
	aui_kl_key_source_attr *p_key_source)
{
	struct kl_handler *hdl_to = (struct kl_handler *)handle;
	struct kl_handler *hdl_kl = NULL;
	struct kl_handler *hdl_cf = NULL;
    struct kl_cw_derivation key_config;
	struct kl_config_key  cfg_kl_key;

    if (!hdl_to || !cfg || !p_data_source || !p_key_source)
        aui_rtn(AUI_RTN_EINVAL, NULL);
	if (!hdl_to->dev)
		 aui_rtn(AUI_RTN_FAIL, "sl not opened");
	memset(&cfg_kl_key,0,sizeof(struct kl_config_key ));
	memset(&key_config,0,sizeof(key_config));
	 
	if(hdl_to->key_ladder_type == AUI_KL_TYPE_CONAXVSC)
	{
		hdl_kl = (struct kl_handler *)p_data_source->key_param.key_ladder_handle;
		if (!hdl_kl)
			aui_rtn(AUI_RTN_EINVAL, NULL);
		key_config.data.fd = alislce_get_fd(hdl_kl->dev);
		if(key_config.data.fd < 0)
			aui_rtn(AUI_RTN_EINVAL, NULL);

		hdl_cf = (struct kl_handler *)p_key_source->key_param.key_ladder_handle;
		if (!hdl_cf)
			aui_rtn(AUI_RTN_EINVAL, NULL);
		key_config.key.fd = alislce_get_fd(hdl_cf->dev);
		if(key_config.key.fd < 0)
			aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	else
	{
	if (cfg->en_cw_key_attr >= AUI_KL_CW_KEY_ATTR_NB
	 || cfg->en_crypt_mode >= AUI_KL_NB
	 || cfg->en_kl_algo != AUI_KL_ALGO_AES)
	 aui_rtn(AUI_RTN_EINVAL, NULL);

	if((p_data_source->key_source == AUI_KL_KEY_SOURCE_RAM)
		&& (p_key_source->key_source == AUI_KL_KEY_SOURCE_RAM))
		aui_rtn(AUI_RTN_FAIL, "data source and key source cannot all be from ram!\n");

	/* check key attribut with key pattern */
	switch (hdl_to->key_pattern) {
		case AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE:
		case AUI_KL_OUTPUT_KEY_PATTERN_128_SINGLE:
			if ((cfg->en_cw_key_attr == AUI_KL_CW_KEY_SINGLE) 
				|| (AUI_KL_CW_KEY_EVEN == cfg->en_cw_key_attr)){
				cfg_kl_key.ck_parity = KL_CK_PARITY_EVEN;
			}else if((AUI_KL_CW_KEY_ODD == cfg->en_cw_key_attr)){
				cfg_kl_key.ck_parity = KL_CK_PARITY_ODD;
			}else
				aui_rtn(AUI_RTN_EINVAL, "invalid key attribut");
			cfg_kl_key.ck_size = (AUI_KL_OUTPUT_KEY_PATTERN_64_SINGLE == hdl_to->key_pattern)?KL_CK_KEY64: KL_CK_KEY128;
			break;
		case AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN:
		case AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN:
			cfg_kl_key.ck_parity = KL_CK_PARITY_ODD_EVEN;/*odd and even key mode*/
			cfg_kl_key.ck_size = (AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN == hdl_to->key_pattern)?KL_CK_KEY64: KL_CK_KEY128;/*single key length*/
			break;
		default:
			 aui_rtn(AUI_RTN_EINVAL, "Invalid hdl->key_pattern\n");
	 }

	 hdl_to->algo = cfg->en_kl_algo;
	 cfg_kl_key.algo= KL_ALGO_AES;
	 cfg_kl_key.crypt_mode = (AUI_KL_ENCRYPT == cfg->en_crypt_mode)?KL_ENCRYPT: KL_DECRYPT;
	 cfg_kl_key.level = hdl_to->level;

	 /*create key handle*/
	 if(alislce_create_key_hdl(hdl_to->dev,&cfg_kl_key)) {
		 AUI_ERR("create kl key handle error!\n");
		 aui_rtn(AUI_RTN_EIO, NULL);
	 }

	/*data source*/
	switch(p_data_source->key_source)
	{
		case AUI_KL_KEY_SOURCE_RAM:
			key_config.data_src = KL_DATA_SW;
			memcpy(key_config.data.buf,p_data_source->key_param.buf,16);
			break;
		case AUI_KL_KEY_SOURCE_KEY_LADDER:
			hdl_kl = (struct kl_handler *)p_data_source->key_param.key_ladder_handle;
			if (!hdl_kl)
				aui_rtn(AUI_RTN_EINVAL, NULL);
			key_config.data_src = KL_DATA_HW;
			key_config.data.fd = alislce_get_fd(hdl_kl->dev);
			if(key_config.data.fd < 0)
				aui_rtn(AUI_RTN_EINVAL, NULL);
			
			if (AUI_KL_CW_KEY_ODD == p_data_source->key_param.parity)
				key_config.data.parity= KL_CK_PARITY_ODD;
			else if (AUI_KL_CW_KEY_EVEN == p_data_source->key_param.parity)
				key_config.data.parity = KL_CK_PARITY_EVEN;
			else
				aui_rtn(AUI_RTN_EINVAL, NULL);
			
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	/*key source*/
	switch(p_key_source->key_source)
	{
		case AUI_KL_KEY_SOURCE_RAM:
			key_config.key_src = KL_KEY_SW;
			memcpy(key_config.key.buf,p_key_source->key_param.buf,16);
			break;
		case AUI_KL_KEY_SOURCE_KEY_LADDER:
			hdl_cf = (struct kl_handler *)p_key_source->key_param.key_ladder_handle;
			if (!hdl_cf)
				aui_rtn(AUI_RTN_EINVAL, NULL);
			key_config.key_src = KL_KEY_HW;
			key_config.key.fd = alislce_get_fd(hdl_cf->dev);
			if(key_config.key.fd < 0)
				aui_rtn(AUI_RTN_EINVAL, NULL);

			if (AUI_KL_CW_KEY_ODD == p_key_source->key_param.parity){
				key_config.key.parity= KL_CK_PARITY_ODD;
				//key_config.target_parity = KL_CK_PARITY_ODD;
			}else if (AUI_KL_CW_KEY_EVEN == p_key_source->key_param.parity){
				key_config.key.parity = KL_CK_PARITY_EVEN;
				//key_config.target_parity = KL_CK_PARITY_EVEN;
			}else
				aui_rtn(AUI_RTN_EINVAL, NULL);
			break;
		default:
			aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (cfg->en_cw_key_attr == AUI_KL_CW_KEY_ODD){
		key_config.target_parity = KL_CK_PARITY_ODD;
	}else if (cfg->en_cw_key_attr == AUI_KL_CW_KEY_EVEN){
		key_config.target_parity = KL_CK_PARITY_EVEN;
	}else
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	if(alislcf_get_target_dev_param(hdl_to->dev,&key_config))
				aui_rtn(AUI_RTN_EINVAL, NULL);

	if (alislce_gen_one_level_plus(hdl_to->dev,&key_config))
		aui_rtn(AUI_RTN_EIO, NULL);

	return 0;
}
