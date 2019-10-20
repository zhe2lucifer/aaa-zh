#include "aui_dsc_common.h"
#include "aui_dmx_priv.h"
#include <alidefinition/adf_pvr.h>
#include <linux/ali_pvr.h>


AUI_MODULE(DSC)



#define AUI_DSC_KL_TYPE_ETSI (1<<4)
/* mutex lock,serial access to dsc handle */

#define MAX_SHA_COMMON_LENGTH  (1024*1024)
#define ALI_PVR_DEV_PATH                "/dev/ali_pvr0"

static pthread_mutex_t dsc_mutex = PTHREAD_MUTEX_INITIALIZER;
/*pvr record minimun unit,the unit byte*/
static unsigned long dsc_pvr_block_size = AUI_DSC_PVR_BLOCK_SIZE_DEFAULT;

AUI_RTN_CODE aui_dsc_open(const aui_attr_dsc *attr, aui_hdl* const handle)
{
    struct dsc_handler *hdl;
    unsigned int otp_03 = 0;
    unsigned int otp_dc = 0;
    int ret = AUI_RTN_FAIL;

    if (!attr || !handle)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    AUI_DBG("dev: %d\n", attr->uc_dev_idx);
    if (attr->dsc_data_type >= AUI_DSC_DATA_TYPE_NB ||
        attr->uc_algo >= AUI_DSC_ALGO_NB)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    hdl = (struct dsc_handler *)calloc(sizeof(struct dsc_handler), 1);
    if(NULL == hdl)
        aui_rtn(AUI_RTN_FAIL,"calloc fail");

    /* Read OTP to get KL mode */
    ret = alislotp_op_read(0x3 * 4, (unsigned char *)&otp_03, 4);

    if (ret < 0) {
        free(hdl);
        aui_rtn(AUI_RTN_FAIL,"otp read error");
    }

    ret = alislotp_op_read(0xdc * 4, (unsigned char *)&otp_dc, sizeof(otp_dc));

    if (ret < 0) {
        free(hdl);
        aui_rtn(AUI_RTN_FAIL,"otp read error");
    }

    if(((otp_dc & 0x80) >> 7))
        hdl->kl_mode = AUI_KL_MODE_FIX_ENGINE;
    else
        hdl->kl_mode = AUI_KL_MODE_NORMAL;

    hdl->data.dev_idx = attr->uc_dev_idx;
    hdl->algo = attr->uc_algo;
    hdl->mode = attr->uc_mode;
    hdl->encrypt = attr->en_en_de_crypt; /* encryption if true */
    hdl->key_type = AUI_DSC_KEY_TYPE_NB;
    hdl->enable_change_key = 0;
    hdl->key_attached = 0;
    hdl->dsc_iv_mode = AUI_DSC_IV_MODE_RESET;

    if (alisldsc_dsc_open(&hdl->dev)) {
        aui_dsc_close(hdl->dev);
        free(hdl);
        aui_rtn(AUI_RTN_FAIL, "sl open error");
    }
    hdl->data_type =  attr->dsc_data_type;

    AUI_DBG("open mode %s\n",
             (hdl->data_type == AUI_DSC_DATA_TS) ? "TS" : "RAW");

    AUI_INIT_LIST_HEAD(&hdl->key_list);

    if (aui_dev_reg(AUI_MODULE_DSC, hdl)) {
        free(hdl);
        aui_rtn(AUI_RTN_EINVAL,
                "dev idx already used");
    }

	if (alisl_dsc_get_fd(hdl->dev,&hdl->dsc_fd)) {
        aui_rtn(AUI_RTN_FAIL, "get dsc fd error!\n");
    }

    //AUI_DBG("alisl dev %p, algo dev %p\n", hdl->dev, hdl->algo_dev);
    *handle = (aui_hdl *)hdl;

#if 1
    /*
    Do following to Set some paramters for DMX recorde if necessary.
    Because sometimes aui_dmx_data_path_set() may be called first, it
    need some paramters(stream id, sub device, etc). This only influence normal
    dmx record
    */
    ///////////////////////////////////////////////////////////////////
    int algo = CA_ALGO_AES;
    unsigned int data_type;
    switch(hdl->algo){
         case AUI_DSC_ALGO_DES:
            algo = CA_ALGO_DES;
            break;
        case AUI_DSC_ALGO_TDES:
            algo = CA_ALGO_TDES;
            break;
        case AUI_DSC_ALGO_AES:
            algo = CA_ALGO_AES;
            break;
        case AUI_DSC_ALGO_CSA:
            algo = CA_ALGO_CSA1;
            break;
        default:
            algo = CA_ALGO_DES;
            break; 
    }  
    if (hdl->data_type == AUI_DSC_DATA_PURE)
        data_type = CA_FORMAT_RAW;
    else
        data_type = CA_FORMAT_TS188;

    if (alisldsc_set_data_type(hdl->dev, data_type))
        aui_rtn(AUI_RTN_FAIL, "alisldsc_dsc_ioctl: CA_SET_FORMAT");

    if (alisldsc_dsc_ioctl(hdl->dev, CA_INIT_SESSION, (unsigned int)&algo))
        aui_rtn(AUI_RTN_FAIL, "alisldsc_dsc_ioctl: CA_INIT_SESSION");
    ///////////////////////////////////////////////////////////////////
#endif

    return AUI_RTN_SUCCESS;
}


static int release_mkey(aui_hdl handle, struct mkey *mkey)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    int err = AUI_RTN_SUCCESS;
    if (!hdl || !mkey)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    if (mkey->pid_list)
        free(mkey->pid_list);

    struct key_from *key = &mkey->key_from;
    AUI_DBG("release key %p,%p,%p\n",key->kl_key,key->otp_key,key->clear_key);
    if(key->kl_key)
        free(key->kl_key);
    if(key->otp_key)
        free(key->otp_key);
    if(key->clear_key)
        free(key->clear_key);
    free(mkey);
    return err;
}

AUI_RTN_CODE aui_dsc_close(aui_hdl handle)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    int err = AUI_RTN_SUCCESS;

    if (!hdl)
        return AUI_RTN_SUCCESS;
    AUI_DBG("hdl->dev:%p\n", hdl->dev);

    if (hdl->pvr_fd > 0){
        if (hdl->pvr_crypto_mode){
            if(alisldsc_pvr_free_resource(&hdl->pvr_fd)){
                err |= AUI_RTN_FAIL;
                AUI_ERR("alisldsc_pvr_free_resource error!\n");
            }
        }else{
            if(alisldsc_pvr_free_block_mode(&hdl->pvr_fd,hdl->dsc_fd)){
                err |= AUI_RTN_FAIL;
                AUI_ERR("alisldsc_pvr_free_block_mode error!\n");
            }
        }
                            
        close(hdl->pvr_fd);
        hdl->pvr_fd = -1;
    }

	
	if(hdl->see_dmx_fd > 0){
		if (ioctl(hdl->see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_SRC_SET, DMX_MAIN2SEE_SRC_NORMAL)){
			err |= AUI_RTN_FAIL;
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_SRC_SET error\n");
		}

		if (ioctl(hdl->see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET, 0)) {
			err |= AUI_RTN_FAIL;
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET error\n");
		}
		close(hdl->see_dmx_fd);
	}
	
    if(hdl->dev) {

        /* delete all attached keys */
        struct mkey *mkey, *tmp;
        aui_list_for_each_entry_safe(mkey, tmp, &hdl->key_list, list) {
            AUI_DBG("released mkey %p\n", mkey);
			AUI_DBG("mkey address: %p,mkey->pid_len: %d\n",mkey,mkey->pid_len);

            if (mkey->key_handle > 0)
            {
                alisldsc_delete_key_handle(hdl->dev, mkey->key_handle);
            }
			aui_list_del(&mkey->list);
            release_mkey(hdl, mkey);
            mkey = NULL;
        }
        AUI_DBG("hdl->dev:%p\n", hdl->dev);
        if (alisldsc_dsc_close(hdl->dev)) {
            AUI_ERR("sl algo close error\n");
            err = AUI_RTN_FAIL;
        }
    }
	if ((hdl->sl_pvr_hdl) && (hdl->ali_pvr_de_hdl)) {
		aui_dsc_pvr_playback_env_deinit(hdl);
	}
    if (aui_dev_unreg(AUI_MODULE_DSC, hdl)) {
        err = AUI_RTN_FAIL;
        AUI_ERR("aui_dev_unreg error");
    }
	
    free(hdl);
	AUI_DBG("\n");
    return err;
}

static struct mkey *find_mkey_with_pid(struct aui_list_head  *head, unsigned short pid)
{
    struct mkey *mkey;
    int i;
    aui_list_for_each_entry(mkey, head, list)
    for (i=0; i<mkey->pid_len; i++)
        if (mkey->pid_list[i] == pid)
            return mkey;
    return NULL;
}

static int workmod_to_chainmod(enum aui_dsc_work_mode mode)
{
    int ret = -1;
    switch(mode) {
        case AUI_DSC_WORK_MODE_IS_CBC:
            ret = CA_MODE_CBC;
            break;
        case AUI_DSC_WORK_MODE_IS_ECB:
            ret = CA_MODE_ECB;
            break;
        case AUI_DSC_WORK_MODE_IS_OFB:
            ret = CA_MODE_OFB;
            break;
        case AUI_DSC_WORK_MODE_IS_CFB:
            ret = CA_MODE_CFB;
            break;
        case AUI_DSC_WORK_MODE_IS_CTR:
            ret = CA_MODE_CTR;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

static int caparity_to_parity(aui_dsc_parity_mode parity)
{
    int ret = -1;
    switch(parity) {
        case AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE:
            ret = CA_PARITY_EVEN;
            break;
        case AUI_DSC_PARITY_MODE_ODD_PARITY_MODE:
            ret = CA_PARITY_ODD;
            break;
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0:
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE1:
            ret = CA_PARITY_AUTO;
            break;
        default:
            ret = -1;
    }

    return ret;
}

static int cares_to_res(aui_dsc_residue_block res)
{
    int ret = -1;
    switch(res) {
        case AUI_DSC_RESIDUE_BLOCK_IS_RESERVED:
        case AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE:
            ret = CA_RESIDUE_CLEAR;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC:
            ret = CA_RESIDUE_AS_ATSC;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_HW_CTS:
            ret = CA_RESIDUE_HW_CTS;
            break;
        default:
            ret = -1;
            break;
            //case RESIDUE_BLOCK_IS_RESERVED:
            //    return CA_RESIDUE_CTR_HDL;/*RESIDUE_BLOCK_IS_RESERVED*/
    }
    return ret;
}

static AUI_RTN_CODE aui_create_kl_key(struct dsc_handler* hdl, const aui_attr_dsc *attr,
                                      struct ca_create_kl_key *key_param)
{
    int valid = CA_VALID_PARITY;
    if (AUI_DSC_DECRYPT == attr->en_en_de_crypt)
        key_param->crypt_mode = CA_DECRYPT;
    else
        key_param->crypt_mode = CA_ENCRYPT;

    //In TS encryption, we should set the tsc flag in TS header, so DSC HW can
    //get odd/even key from key buffer to decrypt according to the tsc flag.
    key_param->parity = caparity_to_parity(attr->en_parity);/*force even mode for r2r*/

    //the tsc flag should be comfirmed to ODD or Even, 
    // tsc flag has no CA_PARITY_AUTO mode in TS encryption
    if (AUI_DSC_ENCRYPT == attr->en_en_de_crypt)
        if (CA_PARITY_AUTO == key_param->parity)
            key_param->parity = CA_PARITY_EVEN;
    
    key_param->residue_mode = cares_to_res(attr->en_residue);

    switch(hdl->algo) {
        case AUI_DSC_ALGO_DES:
        case AUI_DSC_ALGO_TDES:
            //key_param->algo = (AUI_DSC_ALGO_TDES == hdl->algo)?CA_ALGO_TDES: CA_ALGO_DES;
            key_param->algo = CA_ALGO_TDES;
			if(AUI_DSC_WORK_MODE_IS_ECB != attr->uc_mode)
            	CPY_IV(attr->ul_key_pattern,key_param,attr->puc_iv_ctr,8,valid);
			key_param->chaining_mode = workmod_to_chainmod(attr->uc_mode);
			valid |=  CA_VALID_CHAINING_MODE | CA_VALID_RESIDUE_MODE;
            break;
        case AUI_DSC_ALGO_AES:
            key_param->algo = CA_ALGO_AES;
			if(AUI_DSC_WORK_MODE_IS_ECB != attr->uc_mode)
            	CPY_IV(attr->ul_key_pattern,key_param,attr->puc_iv_ctr,16,valid);
			key_param->chaining_mode = workmod_to_chainmod(attr->uc_mode);
			valid |=  CA_VALID_CHAINING_MODE | CA_VALID_RESIDUE_MODE;
            break;
        case AUI_DSC_ALGO_CSA:
            if(AUI_DSC_CSA1 == attr->csa_version)
                key_param->algo = CA_ALGO_CSA1;
            else if(AUI_DSC_CSA2 == attr->csa_version)
                key_param->algo = CA_ALGO_CSA2;
            else
                key_param->algo= CA_ALGO_CSA3;
            break;
        default:/*bug detective*/
            break;
    }
    key_param->kl_fd = attr->ul_key_pos;
    key_param->valid_mask= valid;
    return 0;
}
static AUI_RTN_CODE aui_create_clear_key(struct dsc_handler* hdl, const aui_attr_dsc *attr,
        struct ca_create_clear_key *key_param)
{
    int valid = CA_VALID_PARITY;
    if (AUI_DSC_DECRYPT == attr->en_en_de_crypt)
        key_param->crypt_mode = CA_DECRYPT;
    else
        key_param->crypt_mode = CA_ENCRYPT;
    
    //In TS encryption, we should set the tsc flag in TS header, so DSC HW can
    //get odd/even key from key buffer to decrypt according to the tsc flag.
    key_param->parity = caparity_to_parity(attr->en_parity);/*force even mode for r2r*/

    //the tsc flag should be comfirmed to ODD or Even, 
    // tsc flag has no CA_PARITY_AUTO mode in TS encryption
    if (AUI_DSC_ENCRYPT == attr->en_en_de_crypt)
        if (CA_PARITY_AUTO == key_param->parity)
            key_param->parity = CA_PARITY_EVEN;
    
    key_param->key_size = attr->ul_key_len/8;
    key_param->residue_mode = cares_to_res(attr->en_residue);
    switch (attr->ul_key_pattern) {
        case AUI_DSC_KEY_PATTERN_SINGLE:
        case AUI_DSC_KEY_PATTERN_EVEN:
            memcpy(key_param->key_even, attr->puc_key, key_param->key_size);
            valid |= CA_VALID_KEY_EVEN;
            break;
        case AUI_DSC_KEY_PATTERN_ODD:
            memcpy(key_param->key_odd, attr->puc_key, key_param->key_size);
            valid |= CA_VALID_KEY_ODD;
            break;
        case AUI_DSC_KEY_PATTERN_ODD_EVEN:
            memcpy(key_param->key_odd, attr->puc_key, key_param->key_size);//attr->puc_key format: ood buffer, then even buffer
            memcpy(key_param->key_even, attr->puc_key + key_param->key_size, key_param->key_size);
            valid |= CA_VALID_KEY_EVEN | CA_VALID_KEY_ODD;
            break;
    }
    switch(hdl->algo) {
        case AUI_DSC_ALGO_DES:
        case AUI_DSC_ALGO_TDES:
            //key_param->algo = (AUI_DSC_ALGO_TDES == hdl->algo)?CA_ALGO_TDES: CA_ALGO_DES;
            key_param->algo = (key_param->key_size > 8)?CA_ALGO_TDES: CA_ALGO_DES;
			if(AUI_DSC_WORK_MODE_IS_ECB != attr->uc_mode)
            	CPY_IV(attr->ul_key_pattern,key_param,attr->puc_iv_ctr,8,valid);
			key_param->chaining_mode = workmod_to_chainmod(attr->uc_mode);
			valid |=  CA_VALID_CHAINING_MODE | CA_VALID_RESIDUE_MODE;
            break;
        case AUI_DSC_ALGO_AES:
            key_param->algo = CA_ALGO_AES;
			if(AUI_DSC_WORK_MODE_IS_ECB != attr->uc_mode)
            	CPY_IV(attr->ul_key_pattern,key_param,attr->puc_iv_ctr,16,valid);
			key_param->chaining_mode = workmod_to_chainmod(attr->uc_mode);
			valid |= CA_VALID_CHAINING_MODE | CA_VALID_RESIDUE_MODE;
            break;
        case AUI_DSC_ALGO_CSA:
            if(AUI_DSC_CSA1 == attr->csa_version)
                key_param->algo = CA_ALGO_CSA1;
            else if(AUI_DSC_CSA2 == attr->csa_version)
                key_param->algo = CA_ALGO_CSA2;
            else
                key_param->algo= CA_ALGO_CSA3;
            break;
        default:/*bug detective*/
            break;
    }
    key_param->valid_mask= valid;
    return 0;
}

static AUI_RTN_CODE aui_create_otp_key(struct dsc_handler* hdl, const aui_attr_dsc *attr,
                                       struct ca_create_otp_key *key_param)
{
    int valid = CA_VALID_PARITY;
    if (AUI_DSC_DECRYPT == attr->en_en_de_crypt)
        key_param->crypt_mode = CA_DECRYPT;
    else
        key_param->crypt_mode = CA_ENCRYPT;
    //key_param->crypt_mode = CA_DECRYPT;/*default decrypt mode*/
    
    key_param->residue_mode = cares_to_res(attr->en_residue);

    switch(hdl->algo) {
        case AUI_DSC_ALGO_DES:
        case AUI_DSC_ALGO_TDES:
            //key_param->algo = (AUI_DSC_ALGO_TDES == hdl->algo)?CA_ALGO_TDES: CA_ALGO_DES;
            key_param->algo = CA_ALGO_TDES;
			if(AUI_DSC_WORK_MODE_IS_ECB != attr->uc_mode){
                memcpy(key_param->iv_even, attr->puc_iv_ctr, 8);
                valid |= CA_VALID_IV_EVEN;
			}
			key_param->chaining_mode = workmod_to_chainmod(attr->uc_mode);
			valid |=  CA_VALID_CHAINING_MODE | CA_VALID_RESIDUE_MODE;
            break;
        case AUI_DSC_ALGO_AES:
            key_param->algo = CA_ALGO_AES;
			if(AUI_DSC_WORK_MODE_IS_ECB != attr->uc_mode){
                memcpy(key_param->iv_even, attr->puc_iv_ctr, 16);
                valid |= CA_VALID_IV_EVEN;
			}                
			key_param->chaining_mode = workmod_to_chainmod(attr->uc_mode);
			valid |=  CA_VALID_CHAINING_MODE | CA_VALID_RESIDUE_MODE;
            break;
        case AUI_DSC_ALGO_CSA:
            if(AUI_DSC_CSA1 == attr->csa_version)
                key_param->algo = CA_ALGO_CSA1;
            else if(AUI_DSC_CSA2 == attr->csa_version)
                key_param->algo = CA_ALGO_CSA2;
            else
                key_param->algo= CA_ALGO_CSA3;
            break;
        default:/*bug detective*/
            break;
    }
    if (AUI_DSC_PARITY_MODE_OTP_KEY_FROM_68 == attr->en_parity)
        key_param->otp_key_select = CA_OTP_KEY_6;
    if (AUI_DSC_PARITY_MODE_OTP_KEY_FROM_6C == attr->en_parity)
        key_param->otp_key_select = CA_OTP_KEY_7;
    else{
        aui_rtn(AUI_RTN_EINVAL,
                "OTP only support KEY_68 and KEY_6C");
    }
	AUI_DBG("otp_key_select=%d\n",key_param->otp_key_select);   
        
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_attach_key_info2dsc(aui_hdl handle, const aui_attr_dsc *attr)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    struct mkey *mkey = NULL;
    struct mkey *mkey_new = NULL;
    void *temp = NULL;
    unsigned int data_type = 0;
    int ret = AUI_RTN_SUCCESS;
    int add_pid_flag = 0;
    int ca_crypto_mode;
    char add_mkey_to_list = 0;

    /* Check PIDs */
    if (hdl->data_type == AUI_DSC_DATA_PURE) {
        if ((attr->ul_key_pattern != AUI_DSC_KEY_PATTERN_SINGLE)
			&& (attr->ul_key_pattern != AUI_DSC_KEY_PATTERN_EVEN))
            aui_rtn(AUI_RTN_EINVAL,
                    "parity not supported in RAW mode");
        /* even parity forced in raw mode */
        data_type = CA_FORMAT_RAW;
    } else {
        if (!attr->ul_pid_cnt)
            aui_rtn(AUI_RTN_EINVAL,
                    "pid required in stream mode");
        data_type = CA_FORMAT_TS188;
    }

    if (AUI_DSC_DECRYPT == attr->en_en_de_crypt)
        ca_crypto_mode = CA_DECRYPT;
    else
        ca_crypto_mode = CA_ENCRYPT;

    if (hdl->data_type == AUI_DSC_DATA_TS && attr->dsc_key_type == AUI_DSC_CONTENT_KEY_OTP){
        aui_rtn(AUI_RTN_EINVAL,
                "Otp key can not use in TS data tpye!!!!");
    }

    /* Search if key handler already active */
    mkey = find_mkey_with_pid(&hdl->key_list,
                              attr->ul_pid_cnt ? attr->pus_pids[0] : R2R_PID);
	AUI_DBG("mkey:%p\n",mkey);
	if(mkey){
		AUI_DBG("mkey->key_from.clear_key:%p,mkey->key_from.otp_key:%p,mkey->key_from.kl_key:%p\n",
		mkey->key_from.clear_key,mkey->key_from.otp_key,mkey->key_from.kl_key);
        add_pid_flag = 1;
	}
    else {
        add_pid_flag = 0;
    }
    pthread_mutex_lock(&dsc_mutex);
    hdl->key_attached = 1;
    /* check that new algo parameters match the on-going parameters */
    switch (attr->dsc_key_type) {
        case AUI_DSC_CONTENT_KEY_KL: {
            struct ca_create_kl_key *key_param = NULL;
			struct ca_create_kl_key kl_key;
			int key_handle_temp = -1;
			memset(&kl_key,0,sizeof(kl_key));
            if(mkey && mkey->key_from.kl_key) {/*update kl key*/
				AUI_DBG("\n");
                /*check parameters*/
		      #if 1
				aui_create_kl_key(hdl,attr, &kl_key);
				if(kl_key.kl_fd != mkey->key_from.kl_key->kl_fd){
					key_handle_temp = mkey->key_handle;
					if(0 != alisldsc_attach_kl_key(hdl->dev,data_type,&kl_key,
                                       mkey->pid_list,mkey->pid_len,&mkey->key_handle)) {
		                ret = AUI_RTN_EINVAL;
		                goto err;
		            }
					
					memcpy(mkey->key_from.kl_key,&kl_key,sizeof(kl_key));

					if(alisldsc_add_pid(hdl->dev,attr->pus_pids,attr->ul_pid_cnt,
						AUI_DSC_DECRYPT_IN,AUI_DSC_EVEN_MODE_IN, mkey->key_handle)) {
						goto err;
				    }

					if(alisldsc_delete_key_handle(hdl->dev,key_handle_temp)){
						AUI_DBG("\n");
						ret = AUI_RTN_EINVAL;
	            		goto err;
	            	}

					
					pthread_mutex_unlock(&dsc_mutex);
					return 0;
				}
				memcpy(mkey->key_from.kl_key,&kl_key,sizeof(kl_key));
              #else
				aui_create_kl_key(hdl,attr,mkey->key_from.kl_key);
			  #endif
				
                goto only_updata_para;/* only update parameter */
            }else if(mkey && mkey->key_from.clear_key){/*kl to r2r key*/
            	
				AUI_DBG("\n");
				aui_list_del(&mkey->list);/*delete mkey from key list*/
				
				pthread_mutex_unlock(&dsc_mutex);
				if(aui_dsc_attach_key_info2dsc(hdl,attr)){
					aui_rtn(AUI_RTN_EINVAL,"create r2r key again error!");
				}
				AUI_DBG("\n");
                /* 
                // add CA PID operation will be do latter
				if(alisldsc_add_pid(hdl->dev,attr->pus_pids,attr->ul_pid_cnt,
					AUI_DSC_DECRYPT_IN,AUI_DSC_EVEN_MODE_IN)) {
					return -1;
			    }
			    */

				AUI_DBG("\n");
                pthread_mutex_lock(&dsc_mutex);
            	if(alisldsc_delete_key_handle(hdl->dev,mkey->key_handle)){
					AUI_DBG("\n");
					ret = AUI_RTN_EINVAL;
            		goto err;
            	}
			    
				/* delete all attached keys */
	            release_mkey(hdl, mkey);
	            mkey = NULL;
				unsigned int i;
				for(i = 0;i < attr->ul_pid_cnt;i++){
					AUI_DBG("attr->pus_pids[%d]: %d\n",i,attr->pus_pids[i]);
				}
					
				mkey = find_mkey_with_pid(&hdl->key_list,
                              attr->ul_pid_cnt ? attr->pus_pids[0] : R2R_PID);
				AUI_DBG("mkey:%p\n",mkey);
				if(mkey){
					AUI_DBG("mkey->key_from.clear_key:%p,mkey->key_from.otp_key:%p,mkey->key_from.kl_key:%p,mkey->key_handle:%d\n",
					mkey->key_from.clear_key,mkey->key_from.otp_key,mkey->key_from.kl_key,mkey->key_handle);
				}
				pthread_mutex_unlock(&dsc_mutex);
				return 0;
            }
			AUI_DBG("\n");
			key_param = (struct ca_create_kl_key*)calloc(sizeof(struct ca_create_kl_key), 1);
            if(NULL == key_param){
                ret = AUI_RTN_ENOMEM;
                goto err;
            }
            temp = key_param;/* save allocate memory pointer,if create error,the memory is released by it */
            aui_create_kl_key(hdl,attr,key_param);
            break;
        }
        case AUI_DSC_CONTENT_KEY_OTP: {
            struct ca_create_otp_key *key_param = NULL;
            if(mkey) {
                /*check parameters*/
                if(mkey->key_from.otp_key) {
                    AUI_DBG("\n");
                    if (aui_create_otp_key(hdl,attr,mkey->key_from.otp_key))
                    {
        				ret = AUI_RTN_EINVAL;
                		goto err;
                    }

                #if 1  //for dsc driver issues, if update parameter directory in OTP key mode, 
                       // the output data of encrypt/decrypt may be wrong, so far we delete the key handle,
                       // then re-create a new key handle.
                    int key_handle_temp = -1;
                    key_handle_temp = mkey->key_handle;
    				AUI_DBG("\n");
        			if(alisldsc_delete_key_handle(hdl->dev,key_handle_temp)){
        				ret = AUI_RTN_EINVAL;
                		goto err;
                	}
                    if(0 != alisldsc_attach_otp_key(hdl->dev,data_type,mkey->key_from.otp_key,
                        &mkey->key_handle)){
                        ret = AUI_RTN_EINVAL;
                        goto err;
                    }
                    pthread_mutex_unlock(&dsc_mutex);
                    return AUI_RTN_SUCCESS;
                 #else
     				AUI_DBG("\n");
                    goto only_updata_para;
                 #endif
                } else{/*attr->dsc_key_type have differetn with created key*/
                    ret = AUI_RTN_EINVAL;
                    goto err;
                }
             }else{
                 AUI_DBG("\n");
                 key_param = (struct ca_create_otp_key*)calloc(sizeof(struct ca_create_otp_key), 1);
                 if(NULL == key_param){
                    ret = AUI_RTN_ENOMEM;
                    goto err;
                }
            }
            temp = key_param;
            aui_create_otp_key(hdl,attr,key_param);
            break;
        }
        case AUI_DSC_HOST_KEY_SRAM: {
            struct ca_create_clear_key *key_param = NULL;
			AUI_DBG("\n");
            if(mkey && mkey->key_from.clear_key) {/*update r2r key*/
				AUI_DBG("\n");
				AUI_DBG("mkey address: %p,mkey->pid_len: %d\n",mkey,mkey->pid_len);
                /*check parameters*/
                aui_create_clear_key(hdl,attr,mkey->key_from.clear_key);
                goto only_updata_para;
            }else if(mkey && mkey->key_from.kl_key){/*kl key to r2r key*/
            	AUI_DBG("\n");
                
				aui_list_del(&mkey->list);/*delete mkey from key list*/
				
				pthread_mutex_unlock(&dsc_mutex);
				if(aui_dsc_attach_key_info2dsc(hdl,attr)){
					aui_rtn(AUI_RTN_EINVAL,"create r2r key again error!");
				}
                /*
                // add CA PID operation will be do latter
				if(alisldsc_add_pid(hdl->dev,attr->pus_pids,attr->ul_pid_cnt,
					AUI_DSC_DECRYPT_IN,AUI_DSC_EVEN_MODE_IN)) {
					return -1;
			    }
			    */

				if(alisldsc_delete_key_handle(hdl->dev,mkey->key_handle)){
					AUI_DBG("\n");
					 ret = AUI_RTN_EINVAL;
            		goto err;
            	}

				/* delete all attached keys */
	            release_mkey(hdl, mkey);
	            mkey = NULL;
				unsigned int i;
				for(i = 0;i < attr->ul_pid_cnt;i++){
					AUI_DBG("attr->pus_pids[%d]: %d\n",i,attr->pus_pids[0]);
				}
					
				mkey = find_mkey_with_pid(&hdl->key_list,
                              attr->ul_pid_cnt ? attr->pus_pids[0] : R2R_PID);
				AUI_DBG("mkey:%p\n",mkey);
				if(mkey){
					AUI_DBG("mkey->key_from.clear_key:%p,mkey->key_from.otp_key:%p,mkey->key_from.kl_key:%p,mkey->key_handle:%d\n",
					mkey->key_from.clear_key,mkey->key_from.otp_key,mkey->key_from.kl_key,mkey->key_handle);
				}
				AUI_DBG("\n");
				pthread_mutex_unlock(&dsc_mutex);
				return 0;
            }
			key_param = (struct ca_create_clear_key*)calloc(sizeof(struct ca_create_clear_key), 1);
            if(NULL == key_param){
                ret = AUI_RTN_ENOMEM;
                goto err;
            }
			AUI_DBG("\n");
            temp = key_param;
            aui_create_clear_key(hdl,attr,key_param);
            break;
        }
        default:
            ret = AUI_RTN_EINVAL;
            goto err;
    }

    /* create new key */
    mkey_new = (struct mkey*)calloc(sizeof(struct mkey), 1);
    if (!mkey_new) {
        AUI_ERR("mallocated mkey_new is NULL!\n");
        ret = AUI_RTN_ENOMEM;
        goto err;
    }
    mkey_new->pid_len = attr->ul_pid_cnt ? attr->ul_pid_cnt : 1;
    mkey_new->pid_list = malloc(mkey_new->pid_len * sizeof(unsigned short));
    if (!mkey_new->pid_list) {
		AUI_ERR("mkey_new->pid_list is NULL!\n");
        ret = AUI_RTN_ENOMEM;
        goto err;
    }
    if (attr->ul_pid_cnt)
        memcpy(mkey_new->pid_list, attr->pus_pids, mkey_new->pid_len * sizeof(unsigned short));
    else {
        mkey_new->pid_list[0] = R2R_PID;
    }
	AUI_DBG("mkey_new address: %p,mkey_new->pid_len: %d\n",mkey_new,mkey_new->pid_len);
    switch (attr->dsc_key_type) {
        case AUI_DSC_CONTENT_KEY_KL:
            mkey_new->key_from.kl_key = (struct ca_create_kl_key *)temp;
            if(0 != alisldsc_attach_kl_key(hdl->dev,data_type,mkey_new->key_from.kl_key,
                                           mkey_new->pid_list,mkey_new->pid_len,&mkey_new->key_handle)) {
				AUI_ERR("alisldsc_attach_kl_key() failed\n");
                ret = AUI_RTN_EINVAL;
                goto err;
            }
            break;
        case AUI_DSC_CONTENT_KEY_OTP:
            mkey_new->key_from.otp_key = (struct ca_create_otp_key *)temp;
            if(0 != alisldsc_attach_otp_key(hdl->dev,data_type,mkey_new->key_from.otp_key,
                &mkey_new->key_handle)){
                AUI_ERR("alisldsc_attach_otp_key() failed!\n");
                ret = AUI_RTN_EINVAL;
                goto err;
            }
            break;
        case AUI_DSC_HOST_KEY_SRAM:
            mkey_new->key_from.clear_key = (struct ca_create_clear_key *)temp;
            if(0 != alisldsc_attach_clear_key(hdl->dev,data_type,mkey_new->key_from.clear_key,
                                              mkey_new->pid_list,mkey_new->pid_len,&mkey_new->key_handle)) {
                AUI_ERR("alisldsc_attach_clear_key() failed!\n");
                ret = AUI_RTN_EINVAL;
                goto err;
            }
            break;
        default:
            ret = AUI_RTN_EINVAL;
            goto err;
    }

    aui_list_add(&mkey_new->list, &hdl->key_list);
    add_mkey_to_list = 1;
    hdl->key_type = attr->dsc_key_type;
	AUI_DBG("mkey_new->key_handle: %d\n",mkey_new->key_handle);
	AUI_DBG("mkey_new->key_from.clear_key:%p,mkey_new->key_from.otp_key:%p,mkey_new->key_from.kl_key:%p,mkey_new->key_handle:%d\n",
    	mkey_new->key_from.clear_key,mkey_new->key_from.otp_key,mkey_new->key_from.kl_key,mkey_new->key_handle);

    /*
    NOTE: the following settings only be set one time in a DSC lifetime
    */
    if ((AUI_DSC_DATA_TS == hdl->data_type) &&
        (add_pid_flag != 1)){

        enum dsc_crypt_mode_internal crypt_mode = AUI_DSC_DECRYPT_IN;
        enum parity_in parity_in = AUI_DSC_EVEN_MODE_IN;
    	struct algo_info algo_info;
    	memset(&algo_info,0,sizeof(algo_info));
        int pvr_process_mode = AUI_DSC_PROCESS_MODE_UNDEFINED;

        if(aui_dsc_get_fd(hdl,&algo_info)){
            AUI_ERR("dsc_get_fd() failed!\n");
            ret = AUI_RTN_EINVAL;
            goto err1;
		}
        
        pvr_process_mode = algo_info.process_attr.process_mode;

        if (AUI_DSC_PROCESS_MODE_UNDEFINED == pvr_process_mode){
            if (algo_info.pvr_crypto_mode)
                pvr_process_mode = AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT;
            else
                pvr_process_mode = AUI_DSC_PROCESS_MODE_TS_ENCRYPT;
        }

        if (
            AUI_DSC_PROCESS_MODE_TS_ENCRYPT == pvr_process_mode ||
            AUI_DSC_PROCESS_MODE_TS_DECRYPT == pvr_process_mode ||
            AUI_DSC_PROCESS_MODE_BLOCK_DECRYPT == pvr_process_mode
            ){
            
            if (AUI_DSC_DECRYPT == attr->en_en_de_crypt)
                crypt_mode = AUI_DSC_DECRYPT_IN;
            else
            {
                crypt_mode = AUI_DSC_ENCRYPT_IN;
            }

    		if(aui_dsc_set_pid(hdl,algo_info,crypt_mode,parity_in)){
				AUI_ERR("aui_dsc_set_pid failed!\n");
	            ret = AUI_RTN_FAIL;
	            goto err1;
    		}
            //hdl->pid_flag = 1;

            if (AUI_DSC_ENCRYPT == attr->en_en_de_crypt){
                if(aui_dsc_pvr_start_record(hdl,&algo_info)){
    				AUI_ERR("start pvr record fail!\n");
                    ret = AUI_RTN_FAIL;
                    goto err1;
                }
            }
            
        }

    }


    pthread_mutex_unlock(&dsc_mutex);
    return ret;

only_updata_para:
    /*update clear key*/
    if(hdl->key_type == AUI_DSC_HOST_KEY_SRAM) {
        if (alisldsc_update_key(hdl->dev, &mkey->key_from, mkey->key_handle)){
			AUI_ERR("alisldsc_update_key() failed!\n");
            ret = AUI_RTN_FAIL;
            goto err;
        }
        AUI_DBG("key updated\n");
    }
    /*update iv,chaining mode and so on*/
    if(alisldsc_update_param(hdl->dev, &mkey->key_from,ca_crypto_mode, mkey->key_handle)) {
		AUI_ERR("alisldsc_update_param() failed!\n");
        ret = AUI_RTN_FAIL;
        goto err;
    }
    pthread_mutex_unlock(&dsc_mutex);
    return ret;

err:
    if(NULL != temp)
        free(temp);
	//when error happens, we should release the new mkey resource. the old mkey do not need released
    if(NULL != mkey_new){
        if (NULL != mkey_new->pid_list)
            free(mkey_new->pid_list);

		//When error happen, if the new mkey has been added to list, we should remove it,
		// else if the mkey will free in aui_dsc_close again.
        if (add_mkey_to_list)
            aui_list_del(&mkey_new->list);
        
        free(mkey_new);
    }
err1:    
    pthread_mutex_unlock(&dsc_mutex);

    AUI_ERR("dsc create key error!");
    return ret;
}


AUI_RTN_CODE aui_dsc_deattach_key_by_pid(aui_hdl handle,unsigned short pid)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    struct mkey *mkey;

    if (!hdl)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    mkey = find_mkey_with_pid(&hdl->key_list, pid);
    if (!mkey) {
        AUI_ERR("no key handler found with pid %d\n", pid);
        return AUI_RTN_FAIL;
    }
    
    if (alisldsc_delete_pid(hdl->dev, mkey->key_handle, mkey->pid_list,mkey->pid_len)){
        AUI_ERR("delete pids fail!\n");
        return AUI_RTN_FAIL;
    }

    if (mkey->key_handle > 0)
    {
        alisldsc_delete_key_handle(hdl->dev, mkey->key_handle);
    }
    
    AUI_DBG("released mkey %p\n", mkey);
	aui_list_del(&mkey->list);
    return release_mkey(hdl, mkey);
}



alisl_retcode alisldsc_algo_get_priv(alisl_handle handle, void **priv);

AUI_RTN_CODE aui_dsc_encrypt(aui_hdl handle, unsigned char *data_in,
                             unsigned char* data_out, unsigned long len)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    unsigned long nb_packets = 0;
    unsigned long packet_size = (len>PURE_DATA_MAX_SIZE) ? PACKET_SIZE : len;
    unsigned long packet = 0;
    unsigned long algo_blen = 0;
    unsigned char *temp_out = NULL;
    int len_scramble = 0;

    if (!hdl || !data_in || !data_out || !len ||
        hdl->data_type != AUI_DSC_DATA_PURE) {
        aui_rtn(AUI_RTN_EINVAL, "param is null\n");
    }

    struct mkey *mkey;
    mkey = aui_list_get_single_entry(mkey, &hdl->key_list, list);
    if (!mkey)
        aui_rtn(AUI_RTN_EINVAL, "only one key supported");

    if (mkey->pid_len != 1 || mkey->pid_list[0] != R2R_PID) {
        AUI_DBG("Warning: PID should be %d in ram2ram operation\n", R2R_PID);
    }
    /*PUREDATA  at least one block for CTS residue handling */
    algo_blen = (AUI_DSC_ALGO_AES == hdl->algo)?16:8;
    if((len % packet_size) && (len % packet_size < algo_blen))
        packet_size -= algo_blen;
    /*
    because driver reuse output poiting to buffer,when output is equal to input,
    descramble/scramble will fail.So we must reallocate temporary memory.
    */
    if(data_in == data_out) {
        temp_out = (unsigned char *)malloc((len>PURE_DATA_MAX_SIZE) ? PACKET_SIZE : len);
        if(NULL == temp_out) {
            aui_rtn(AUI_RTN_EINVAL, "only one key supported");
        }
    }

    if (AUI_DSC_IV_MODE_RESET == hdl->dsc_iv_mode){
        /* key handle already generated */
        if(hdl->key_type == AUI_DSC_HOST_KEY_SRAM) {
            if (alisldsc_update_key(hdl->dev, &mkey->key_from, mkey->key_handle)){
                if(NULL != temp_out){
                    free(temp_out);
                }
                aui_rtn(AUI_RTN_FAIL, "sl update cw error");
            }
            AUI_DBG("key updated\n");
        }
        if(hdl->key_type != AUI_DSC_CONTENT_KEY_OTP){
        	if(alisldsc_update_param(hdl->dev, &mkey->key_from,CA_ENCRYPT, mkey->key_handle)) {
                if(NULL != temp_out){
                    free(temp_out);
                }
            	aui_rtn(AUI_RTN_FAIL, "sl update cw error");
        	}
        }
    }
    
    for(packet = 0; packet < len; packet += packet_size) {

        /* descramble/scramble data length in this time*/
        len_scramble = ((nb_packets+1)*packet_size <= len)?packet_size:(len%packet_size);
        if(NULL != temp_out) {
            if (alisldsc_encrypt(hdl->dev, data_in, temp_out, len_scramble)){
                free(temp_out);
                aui_rtn(AUI_RTN_FAIL, "sl dsc encrypt error");
            }
            memcpy(data_out,temp_out,len_scramble);
        } else {
            if (alisldsc_encrypt(hdl->dev, data_in, data_out, len_scramble))
                aui_rtn(AUI_RTN_FAIL, "sl dsc encrypt error");
        }

        /* Next 1M packet */
        data_in+=packet_size;
        data_out+=packet_size;

        /* count number of packets */
        nb_packets++;
    }
    /*free temporary malloc memory*/
    if(NULL != temp_out) {
        free(temp_out);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_decrypt(aui_hdl handle, unsigned char *data_in,
                             unsigned char* data_out, unsigned long len)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    unsigned long nb_packets = 0;
    unsigned long packet_size = (len>PURE_DATA_MAX_SIZE) ? PACKET_SIZE : len;
    unsigned long packet = 0;
    unsigned long algo_blen = 0;
    unsigned char *temp_out = NULL;
    int len_scramble = 0;

    if (!hdl || !data_in || !data_out || !len ||
        hdl->data_type != AUI_DSC_DATA_PURE)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    struct mkey *mkey;
    mkey = aui_list_get_single_entry(mkey, &hdl->key_list, list);
    if (!mkey)
        aui_rtn(AUI_RTN_EINVAL, "only one key supported");

    if (mkey->pid_len != 1 || mkey->pid_list[0] != R2R_PID) {
        AUI_DBG("Warning: PID should be %d in ram2ram operation\n", R2R_PID);
    }
    /*PUREDATA  at least one block for CTS residue handling */
    algo_blen = (AUI_DSC_ALGO_AES == hdl->algo)?16:8;
    if((len % packet_size) && (len % packet_size < algo_blen))
        packet_size -= algo_blen;
    /*
    because driver reuse output poiting to buffer,when output is equal to input,
    descramble/scramble will fail.So we must reallocate temporary memory.
    */
    if(data_in == data_out) {
        temp_out = (unsigned char *)malloc((len>PURE_DATA_MAX_SIZE) ? PACKET_SIZE : len);
        if(NULL == temp_out) {
            aui_rtn(AUI_RTN_EINVAL, "only one key supported");
        }
    }

    if (AUI_DSC_IV_MODE_RESET == hdl->dsc_iv_mode){
        if(hdl->key_type == AUI_DSC_HOST_KEY_SRAM) {
            /* key handle already generated */
            if (alisldsc_update_key(hdl->dev, &mkey->key_from, mkey->key_handle)){
                if(NULL != temp_out){
                    free(temp_out);
                }
                aui_rtn(AUI_RTN_FAIL, "sl update cw error");
            }
            AUI_DBG("key updated\n");
        }
    	if(hdl->key_type != AUI_DSC_CONTENT_KEY_OTP){
            if(alisldsc_update_param(hdl->dev, &mkey->key_from,CA_DECRYPT, mkey->key_handle)) {
                if(NULL != temp_out){
                    free(temp_out);
                }
                aui_rtn(AUI_RTN_FAIL, "sl update cw error");
            }
    	}
    }

    for(packet = 0; packet < len; packet+=packet_size) {

        /* descramble/scramble data length in this time*/
        len_scramble = ((nb_packets+1)*packet_size <= len)?packet_size:(len%packet_size);
        if(NULL != temp_out) {
            if (alisldsc_decrypt(hdl->dev, data_in, temp_out, len_scramble)){
                free(temp_out);
                aui_rtn(AUI_RTN_FAIL, "sl dsc decrypt error");
            }
            memcpy(data_out,temp_out,len_scramble);
        } else {
            if (alisldsc_decrypt(hdl->dev, data_in, data_out, len_scramble))
                aui_rtn(AUI_RTN_FAIL, "sl dsc decrypt error");
        }

        /* Next 1M packet */
        data_in+=packet_size;
        data_out+=packet_size;

        /* count number of packets */
        nb_packets++;
    }

    /*free temporary malloc memory*/
    if(NULL != temp_out) {
        free(temp_out);
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_sha_digest(unsigned long source, unsigned char *data, unsigned long len,
                                unsigned long mode, unsigned char *out)
{
    int digest_len = -1;
    if(!data || !out)
        aui_rtn(AUI_RTN_EINVAL, NULL);
    switch (mode) {
        case AUI_SHA_1:
            digest_len = SL_SHA1_DIGEST_SIZE;
            break;
        case AUI_SHA_224:
            digest_len = SL_SHA224_DIGEST_SIZE;
            break;
        case AUI_SHA_256:
            digest_len = SL_SHA256_DIGEST_SIZE;
            break;
        case AUI_SHA_384:
            digest_len = SL_SHA384_DIGEST_SIZE;
            break;
        case AUI_SHA_512:
            digest_len = SL_SHA512_DIGEST_SIZE;
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (source > SHA_DATA_SOURCE_FROM_FLASH) {
        aui_rtn(AUI_RTN_EINVAL, "source > SHA_DATA_SOURCE_FROM_FLASH");
    }

    if (alisldsc_sha_digest(data, out, len, digest_len)) {
        aui_rtn(AUI_RTN_FAIL, "sl dsc sha error");
    }

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_get_buffer(unsigned long size,  void **pp_buf)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    int sha_ret = -1;

    AUI_DBG("size: %d\n", (int)size);
    char *buf = NULL;
    if(NULL == pp_buf)
        aui_rtn(AUI_RTN_FAIL, "Invalid parameters!");
    
    if (size > MAX_SHA_COMMON_LENGTH)
    {
        sha_ret = alisldsc_sha_get_buffer(NULL, size, (void*)(&buf));
        AUI_DBG("buf:0x%x\n", (unsigned int)buf);
        if (sha_ret)
        {
            AUI_ERR("sha get buffer return %d\n", sha_ret);
            buf = malloc(size);
        }
    }
    else
    {
        buf = malloc(size);
    }
    if(NULL == buf)
        aui_rtn(AUI_RTN_FAIL, "malloc error!");
    *pp_buf = (void*)buf;

    return ret;
}

AUI_RTN_CODE aui_dsc_release_buffer(unsigned long size, void *p_buf)
{
    (void)size;/*warning*/
    AUI_DBG("buf:0x%x, size:%d\n", (unsigned int)p_buf, (int)size);
    
    if (size > MAX_SHA_COMMON_LENGTH)
    {
        if(0 == alisldsc_sha_release_buffer(NULL, p_buf, size))
        {
            return AUI_RTN_SUCCESS;
        }
    }
    
    free(p_buf);
    return AUI_RTN_SUCCESS;
}


int aui_dsc_mem(aui_hdl handle, int cmd, int *mem)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    struct ali_dsc_krec_mem krec_mem;

    krec_mem.size = MEM_ALLOC_UNIT;
    krec_mem.va_mem = mem;
    int ret = alisl_dsc_operate_mem(hdl->dev, cmd, &krec_mem);
    if (ret)
        aui_rtn(AUI_RTN_FAIL, "alisl_dsc_operate_mem error");
    mem = krec_mem.va_mem;
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dsc_get(aui_hdl handle,unsigned long item, void *param)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;

    if (!hdl || !param)
        aui_rtn(AUI_RTN_EINVAL, NULL);

    switch (item) {
        case AUI_DSC_GET_DATA_TYPE:
            *((unsigned long *)param) = hdl->data_type;
            break;
        default:
            aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    return 0;
}

AUI_RTN_CODE aui_dsc_version_get(unsigned long *pul_version)
{
    if(!pul_version)
        aui_rtn(AUI_RTN_EINVAL,NULL);

    *pul_version = AUI_MODULE_VERSION_NUM_DSC;
    return AUI_RTN_SUCCESS;
}

/* Reserved function */
AUI_RTN_CODE aui_dsc_init(p_fun_cb call_back,void *param)
{
    if (call_back != NULL) {
        call_back(param);
    }
    return AUI_RTN_SUCCESS;
}


/* Reserved function */
AUI_RTN_CODE aui_dsc_de_init(p_fun_cb call_back,void *param)
{
    if (call_back != NULL) {
        call_back(param);
    }
    return AUI_RTN_SUCCESS;
}

/* Internal function used by generate HMAC msg*/
static void xor(unsigned char *dst, unsigned char *src, unsigned long len)
{
    unsigned int i;

    if ((NULL == dst)||(NULL == src)||(len == 0)) {
        return ;
    }

    for (i = 0; i < len; i++) {
        dst[i] = dst[i]^src[i];
    }
}

/* Internal function used by generate HMAC msg*/
static int get_pf_chip_id(unsigned long *pchipid)
{
    static unsigned long rl_chip_id = 0;
    unsigned long chip_id_t1 = 0;
    unsigned long chip_id_t2= 0;

    if (0 == rl_chip_id) {
        if (AUI_RTN_SUCCESS != aui_otp_read(0, (unsigned char *)(&chip_id_t1), OTP_DW_LEN)) {
            return FALSE;
        }

        if (AUI_RTN_SUCCESS != aui_otp_read(0, (unsigned char *)(&chip_id_t2), OTP_DW_LEN)) {
            return FALSE;
        }

        if (chip_id_t1 == chip_id_t2) {
            rl_chip_id = chip_id_t1;
        }else{
            return FALSE;
        }

    }

    if (NULL != pchipid) {
        *pchipid = rl_chip_id;
    }

    return TRUE;
}

AUI_RTN_CODE aui_dsc_generate_HMAC(unsigned char *input, unsigned long length,
                                   unsigned char *output, unsigned char *key)
{
    int ret = AUI_RTN_FAIL;
    const unsigned long ipad = 0x36;
    const unsigned long opad = 0x5c;
    unsigned char *temp_buff = NULL;
    unsigned char *temp = NULL;
    unsigned long chip_id = 0;
    unsigned long sha_buf_len = 0;
    HMAC_PARAM k_buffer;

    if (( NULL == key )||( length == 0 )) {
        aui_rtn(AUI_RTN_FAIL, "Invalid params!");
    }

    /* 1, prepare k0, use the dsc decrypt the R2 to get the k0 */
    memset(&k_buffer, 0, sizeof(HMAC_PARAM));
    memcpy(k_buffer.k0, key, FIRST_KEY_LENGTH);

    if (FALSE == get_pf_chip_id(&chip_id)) {
        AUI_DBG("read chip id fail \n");
        return AUI_RTN_FAIL;
    }

    AUI_DBG("chip id 0x%lx, length:%d\n", chip_id, length);
    memcpy(&k_buffer.k0[FIRST_KEY_LENGTH], &chip_id, OTP_DW_LEN);

    AUI_DBG("K0:\n");
    AUI_DUMP("rsa: ", (char *)k_buffer.k0, HASH_BLOCK_LENGTH);
    /* 2, k0 xor ipad */
    memset(k_buffer.ipad, ipad, HASH_BLOCK_LENGTH);
    xor(k_buffer.k0, k_buffer.ipad, HASH_BLOCK_LENGTH);

    AUI_DBG("K0:\n");
    AUI_DUMP("rsa: ", (char *)k_buffer.k0, HASH_BLOCK_LENGTH);

    sha_buf_len = length + 0xf + HASH_BLOCK_LENGTH;
    //If the length of data less than 64bytes(HASH_BLOCK_LENGTH), it 
    //must enlarge the sha_buf_len. Else if the temp buffer is too small
    //so that the sha digisting would overflow
    if (sha_buf_len < (HASH_BLOCK_LENGTH*2 + 0xf))
        sha_buf_len = (HASH_BLOCK_LENGTH*2 + 0xf);

    /* 3, (k0 xor ipad) || text */
    if (sha_buf_len > PURE_DATA_MAX_SIZE) {
        aui_dsc_get_buffer(sha_buf_len, (void **)&temp_buff);
    } else {
        temp_buff = (unsigned char*)((unsigned long)MALLOC(sha_buf_len));
    }
    if (NULL == temp_buff) {
        AUI_DBG("out of memory\n");
        return AUI_RTN_ENOMEM;
    }
    memset(temp_buff, 0, sha_buf_len);
    //The buffer is aligned 8 bytes
    temp = (unsigned char *)(0xFFFFFFF8 & (unsigned long)(temp_buff + 0x7));
    memcpy(temp,k_buffer.k0, HASH_BLOCK_LENGTH);
    memcpy(&temp[HASH_BLOCK_LENGTH], input,length);

    /* 4, Hash((k0 xor ipad) || text) */
    ret = aui_dsc_sha_digest(SHA_DATA_SOURCE_FROM_DRAM, temp, (length + HASH_BLOCK_LENGTH), AUI_SHA_256, k_buffer.hout);
    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("RSA ERROR: sha digest error!\n");
        if (sha_buf_len > PURE_DATA_MAX_SIZE) {
            aui_dsc_release_buffer(sha_buf_len, (void *)temp_buff);
        } else {
            free(temp_buff);
        }
        return AUI_RTN_FAIL;
    }

    /* 5, k0 xor opad */
    xor(k_buffer.k0, k_buffer.ipad, HASH_BLOCK_LENGTH);
    memset(k_buffer.opad,opad, HASH_BLOCK_LENGTH);
    xor(k_buffer.k0, k_buffer.opad, HASH_BLOCK_LENGTH);

    /* 6, (k0 xor opad) || Hash((k0 xor ipad) || text) */
    memcpy(temp, k_buffer.k0, HASH_BLOCK_LENGTH);
    memcpy(&temp[HASH_BLOCK_LENGTH], k_buffer.hout, HASH_BLOCK_LENGTH);

    /* 7, Hash((k0 xor opad) || Hash((k0 xor ipad) || text)) */
    ret = aui_dsc_sha_digest(SHA_DATA_SOURCE_FROM_DRAM, temp, (2*HASH_BLOCK_LENGTH), AUI_SHA_256, k_buffer.hout);
    if (ret != AUI_RTN_SUCCESS) {
        AUI_ERR("RSA ERROR: sha digest error!\n");
        if (sha_buf_len > PURE_DATA_MAX_SIZE) {
            aui_dsc_release_buffer(sha_buf_len, (void *)temp_buff);
        } else {
            free(temp_buff);
        }
        return AUI_RTN_FAIL;
    }
    memcpy(output,k_buffer.hout, HMAC_OUT_LENGTH);
    if (sha_buf_len > PURE_DATA_MAX_SIZE) {
        aui_dsc_release_buffer(sha_buf_len, (void *)temp_buff);
    } else {
        free(temp_buff);
    }
    AUI_DUMP("rsa: ", (char *)output, HMAC_OUT_LENGTH);
    AUI_DBG("HMAC done, ret: %d!\n", ret);
    return AUI_RTN_SUCCESS;
}

/* locate in adf_dsc.h
typedef struct DSC_BL_UK_PARAM
{
    unsigned char *input_key;
    unsigned char *r_key;
    unsigned char *output_key;
    unsigned int crypt_type;
}DSC_BL_UK_PARAM,*pDSC_BL_UK_PARAM;
*/
/*
    this function used to encrypt the bootloader universal key
    param[in] : input_key -> clear bl uk 128bit
    param[in] : r_key -> random key 128bit
    param[out] : output_key -> encrypted bl uk 128bit
    param[in] : encrypt_type
    0: encrypt bl uk use key 6
    1: encrypt bl uk use key 6 with r1
    2: encrypt bl uk use key 7
*/
AUI_RTN_CODE aui_dsc_encrypt_bl_uk(aui_hdl handle, unsigned char *input_key, unsigned char *r_key,
                                   unsigned char *output_key, unsigned int encrypt_type)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    DSC_BL_UK_PARAM param;

    if ((!hdl) || (!input_key) || (!r_key) || (!output_key) || (encrypt_type > 3)) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    memset(&param, 0, sizeof(param));
    param.input_key = input_key;
    param.r_key = r_key;
    param.output_key = output_key;
    param.crypt_type = encrypt_type;
    if (alisldsc_dsc_ioctl(hdl->dev, IO_DSC_ENCRYTP_BL_UK, (unsigned int)&param)) {
        aui_rtn(AUI_RTN_FAIL, "alisldsc_dsc_ioctl");
    }
    return 0;
}

AUI_RTN_CODE aui_dsc_set_sram_change_key_param(aui_hdl handle, unsigned char* keys_start_ptr,
        unsigned int keys_num, unsigned int key_length_bit, unsigned int quantum_num_per_key)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    DSC_PVR_KEY_PARAM key_param;
    void *enc_dev = NULL;

    if (!hdl || !keys_start_ptr || 0 == keys_num || 0 == quantum_num_per_key) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (AUI_DSC_DATA_TS != hdl->data_type) {
        aui_rtn(AUI_RTN_FAIL, "data type error, not TS mode");
    }

    if (AUI_DSC_ALGO_AES != hdl->algo) {
        aui_rtn(AUI_RTN_FAIL, "algorithm type error, not AES mode");
    }

    if (AUI_DSC_HOST_KEY_SRAM != hdl->key_type) {
        aui_rtn(AUI_RTN_FAIL, "key type error, not SRAM mode");
    }

    if (0 != alisldsc_algo_get_priv(hdl->dev, &enc_dev)) {
        aui_rtn(AUI_RTN_FAIL, "get encode sub device handle failed\n");
    }

    MEMSET(&key_param, 0, sizeof(DSC_PVR_KEY_PARAM));
    key_param.input_addr = (unsigned int)keys_start_ptr;
    key_param.total_quantum_number = quantum_num_per_key;
    key_param.valid_key_num = keys_num;
    key_param.pvr_key_length = key_length_bit;
    key_param.pvr_user_key_pos = 0xff; // MUST set 0xff even if SRAM mode

    //key_param.enc_dev = enc_dev;
    //key_param.encrypt_mode = hdl->algo;

    if (0 != alisldsc_dsc_set_pvr_key_param(hdl->dev, &key_param)) {
        aui_rtn(AUI_RTN_FAIL, "set change key param failed\n");
    }
    hdl->enable_change_key = 1;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_get_fd(aui_hdl handle, struct algo_info *algo_info)
{
    struct dsc_handler* hdl = (struct dsc_handler *)handle;
    if(NULL == hdl || NULL  == algo_info)
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameters!\n");

    if (alisl_dsc_get_fd(hdl->dev,&algo_info->dsc_fd)) {
        aui_rtn(AUI_RTN_FAIL, "get dsc fd error!\n");
    }
	
	if(hdl->pvr_fd <= 0){
		hdl->pvr_fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
		if(hdl->pvr_fd  < 0) {
	        AUI_ERR("Invalid ali_pvr_fd  fd: %d\n", hdl->pvr_fd);
	       	return AUI_RTN_ENOENT;
	    } 
	}
    struct mkey *mkey;
    /*get first key parameters,at the present,hdl->key_list only support to saving one 'struct mkey' pointer*/
    aui_list_for_each_entry(mkey, &hdl->key_list, list){
        AUI_DBG("mkey address: %p,mkey->pid_len: %d\n",mkey,mkey->pid_len);
        algo_info->pid_len = mkey->pid_len;
        algo_info->pid = mkey->pid_list;
        break;
    }


	algo_info->pvr_crypto_mode = hdl->pvr_crypto_mode;/*return flag that represent for the TS record or block mode rerord*/
    if (aui_dsc_is_vmx_module(handle)){
    	algo_info->pvr_fd = algo_info->dsc_fd;
    }else{
    	algo_info->pvr_fd = hdl->pvr_fd;
    }
    
    if (aui_dsc_is_vmx_module(hdl))
    	algo_info->pvr_block_size = hdl->dsc_process_attr.ul_block_size;
    else
    {
        algo_info->pvr_block_size = dsc_pvr_block_size;
    }
	algo_info->dsc_data_type = hdl->data_type;
	memcpy(&(algo_info->process_attr), &(hdl->dsc_process_attr), sizeof(aui_dsc_process_attr));
	AUI_DBG("pvr_fd:%d, pvr_crypto_mode:%d, pvr_block_size: %d\n", algo_info->pvr_fd, 
	    algo_info->pvr_crypto_mode, (int)(algo_info->pvr_block_size));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dsc_set_pid(aui_hdl handle,struct algo_info algo_info,
	enum dsc_crypt_mode_internal crypt_mode,enum parity_in parity)
{
	struct dsc_handler* hdl = (struct dsc_handler *)handle;
	int crypt_mode_temp = (crypt_mode == AUI_DSC_DECRYPT_IN) ? CA_DECRYPT: CA_ENCRYPT;
	int parity_temp = (parity == AUI_DSC_ODD_MODE_IN) ? CA_PARITY_ODD: CA_PARITY_EVEN;
    int key_handle = -1;
     struct mkey *mkey = NULL;

    
	if(NULL == hdl)
		aui_rtn(AUI_RTN_EINVAL, "Invalid parameters!\n");

    mkey = find_mkey_with_pid(&hdl->key_list,
                              algo_info.pid_len ? algo_info.pid[0] : R2R_PID);
	if(mkey){
        AUI_DBG("mkey->key_handle:%d\n",mkey->key_handle);
        key_handle = mkey->key_handle;
	}
    
	if(alisldsc_add_pid(hdl->dev,algo_info.pid,algo_info.pid_len,
		crypt_mode_temp,parity_temp, key_handle)) {
		return -1;
    }
    return 0;
}

AUI_RTN_CODE aui_get_dsc_crypt_fd(aui_hdl handle)
{
	struct dsc_handler* hdl = (struct dsc_handler *)handle;
	if(NULL == hdl)
		aui_rtn(AUI_RTN_EINVAL, "Invalid parameters!\n");

    if(hdl->pvr_crypto_mode)
    {
        return hdl->pvr_fd;
    }
    else
    {
        return hdl->dsc_fd;
    }
	
}

static int caparity_to_parity_for_pvr(aui_dsc_parity_mode parity)
{
    int ret = -1;
    switch(parity) {
        case AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE:
            ret = EVEN_PARITY_MODE;
            break;
        case AUI_DSC_PARITY_MODE_ODD_PARITY_MODE:
            ret = ODD_PARITY_MODE;
            break;
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0:
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE1:
            ret = AUTO_PARITY_MODE0;
            break;
        default:
            ret = -1;
    }

    return ret;
}

static int cares_to_res_for_pvr(aui_dsc_residue_block res)
{
    int ret = -1;
    switch(res) {
        case AUI_DSC_RESIDUE_BLOCK_IS_RESERVED:
        case AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE:
            ret = RESIDUE_BLOCK_IS_NO_HANDLE;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_AS_ATSC:
            ret = RESIDUE_BLOCK_IS_AS_ATSC;
            break;
        case AUI_DSC_RESIDUE_BLOCK_IS_HW_CTS:
            ret = RESIDUE_BLOCK_IS_HW_CTS;
            break;
        default:
            ret = -1;
            break;
            //case RESIDUE_BLOCK_IS_RESERVED:
            //    return CA_RESIDUE_CTR_HDL;/*RESIDUE_BLOCK_IS_RESERVED*/
    }
    return ret;
}

static int workmod_to_chainmod_for_pvr(enum aui_dsc_work_mode mode)
{
    int ret = -1;
    switch(mode) {
        case AUI_DSC_WORK_MODE_IS_CBC:
            ret = WORK_MODE_IS_CBC;
            break;
        case AUI_DSC_WORK_MODE_IS_ECB:
            ret = WORK_MODE_IS_ECB;
            break;
        case AUI_DSC_WORK_MODE_IS_OFB:
            ret = WORK_MODE_IS_OFB;
            break;
        case AUI_DSC_WORK_MODE_IS_CFB:
            ret = WORK_MODE_IS_CFB;
            break;
        case AUI_DSC_WORK_MODE_IS_CTR:
            ret = WORK_MODE_IS_CTR;
            break;
        default:
            ret = -1;
            break;
    }
    return ret;
}

AUI_RTN_CODE aui_dsc_update_pvr_encrypt_key_info(
	aui_hdl handle,
	aui_attr_dsc *attr,
	struct aui_dsc_encrypt_kl_param *p_kl_attr,
	aui_dsc_process_status* p_encrypt_status)
{
	struct dsc_handler *pst_hdl_dsc_en=(struct dsc_handler *)handle;
	int ret = 0;
	struct PVR_BLOCK_ENC_PARAM input_enc;
	unsigned int key_length = 16;/*the unit byte,default aes*/
	unsigned int iv_length = 16;/*the unit byte,default aes*/
   	
	if ((NULL == handle ) || (NULL == attr))
			aui_rtn(AUI_RTN_EINVAL, NULL);
	(void)p_kl_attr;
	MEMSET(&input_enc,0,sizeof(struct PVR_BLOCK_ENC_PARAM ));

    /* Check PIDs */
    if (attr->dsc_data_type == AUI_DSC_DATA_PURE) {
		input_enc.pid_num = 1;
		input_enc.pid_list[0] = R2R_PID;
    } else {
        if (!attr->ul_pid_cnt)
            aui_rtn(AUI_RTN_EINVAL,
                    "pid required in stream mode");
		if(attr->ul_pid_cnt > 32)/*max pid support 32*/
			input_enc.pid_num = 32;
		else
			input_enc.pid_num = attr->ul_pid_cnt;
		MEMCPY(&input_enc.pid_list,attr->pus_pids,(sizeof(unsigned short))*input_enc.pid_num);
	}
	if(AUI_DSC_ALGO_AES == attr->uc_algo){
		input_enc.dsc_sub_device = AES;
		key_length = 16;
		iv_length = 16;
	}else if((AUI_DSC_ALGO_DES == attr->uc_algo)
		|| (AUI_DSC_ALGO_TDES == attr->uc_algo)){/*only use TDES*/
		input_enc.dsc_sub_device = TDES;
		key_length = 16;
		iv_length = 8;
	}else{
		AUI_ERR("algorithm config error!\n");
		aui_rtn(AUI_RTN_EINVAL,
                    "parity not supported in RAW mode");
	}
	input_enc.work_mode = workmod_to_chainmod_for_pvr(attr->uc_mode);
	input_enc.source_mode = (attr->dsc_data_type == AUI_DSC_DATA_PURE)? PURE_DATA_MODE: TS_MODE;
	input_enc.residue_mode = cares_to_res_for_pvr(attr->en_residue);
	input_enc.key_mode = caparity_to_parity_for_pvr(attr->en_parity);
	switch(attr->dsc_key_type){
		case AUI_DSC_CONTENT_KEY_KL:{
			
			if (AUI_KL_TYPE_CONAXVSC == p_kl_attr->key_ladder_type) {
				input_enc.root_key_pos = AUI_KL_ROOT_KEY_0_2;
				input_enc.kl_mode = 0; /* AUI_KL_ALGO_AES */
				input_enc.kl_level = 1;
			}
			else if(AUI_KL_TYPE_ALI == p_kl_attr->key_ladder_type){
				input_enc.root_key_pos = KEY_POS(p_kl_attr->rootkey_index);//root_key_pos | 0x10;//p_kl_attr->rootkey_index;
				input_enc.kl_mode = (p_kl_attr->kl_algo == AUI_KL_ALGO_AES)? 0: 1;
				input_enc.kl_level = p_kl_attr->kl_level;
			}else if(AUI_KL_TYPE_ETSI == p_kl_attr->key_ladder_type){
				input_enc.root_key_pos = KEY_POS(p_kl_attr->rootkey_index) | AUI_DSC_KL_TYPE_ETSI;//root_key_pos | 0x10;//p_kl_attr->rootkey_index;
				input_enc.kl_mode = (p_kl_attr->kl_algo == AUI_KL_ALGO_AES)? 0: 1;
				input_enc.kl_level = p_kl_attr->kl_level;
			}else
				aui_rtn(AUI_RTN_EINVAL, NULL);
			input_enc.target_key_pos = INVALID_ALI_CE_KEY_POS;

			if(p_kl_attr->control_word)
				MEMCPY(input_enc.input_key, p_kl_attr->control_word, p_kl_attr->kl_level*16);
			break;
		}
		case AUI_DSC_HOST_KEY_SRAM:{
			input_enc.root_key_pos= 0xFF; //r2r key,don't use kl to generate key
			if((AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE == attr->en_parity) 
				|| (AUI_DSC_PARITY_MODE_ODD_PARITY_MODE == attr->en_parity)){
				if(attr->puc_key == NULL){
					AUI_DBG("R2R key shouldn't be empty\n");
					aui_rtn(AUI_RTN_EINVAL,NULL);
				}
				MEMCPY(&input_enc.input_key[32],attr->puc_key,key_length);/*only support 128 bit even key*/
			}
			else{
				AUI_ERR("parity config error!\n");
				aui_rtn(AUI_RTN_EINVAL,NULL);
			}
			break;
		}
		default:
			AUI_ERR("key type config error!\n");
			aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if(attr->uc_mode != AUI_DSC_WORK_MODE_IS_ECB){
		if(NULL == attr->puc_iv_ctr){
			AUI_DBG("IV can't be empty\n");
			aui_rtn(AUI_RTN_EINVAL,NULL);
		}
		MEMCPY(&input_enc.input_iv,attr->puc_iv_ctr,iv_length);
	}
	//input_enc.sub_device_id = pst_hdl_dsc_en->ul_sub_dev_id;//(p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);
	/*first init,the following two variable is transfered to the see pvr from main cpu*/
	//input_enc.stream_id= pst_hdl_dsc_en->ul_stream_id;
	

	AUI_DUMP("input_enc.iv:", (char *)input_enc.input_iv, 16);
	AUI_DUMP("input_key.key:", (char *)input_enc.input_key, 48);
	AUI_DBG("IO_DSC_START_BLOCK_PVR: dsc_sub_device=0x%x,work_mode=0x%x,source_mode=0x%x,root_key_pos=0x%x\n",
		(unsigned int)input_enc.dsc_sub_device,(unsigned int)input_enc.work_mode,
		(unsigned int)input_enc.source_mode,(unsigned int)input_enc.root_key_pos);
	AUI_DBG("key_mode=0x%x,sub_device_id=0x%x,stream_id=0x%x,target_key_pos=0x%x\n",
		(unsigned int)input_enc.key_mode,(unsigned int)input_enc.sub_device_id,
		(unsigned int)input_enc.stream_id,(unsigned int)input_enc.target_key_pos);
	int i = 0;
	AUI_DBG("pid count: %d, pid_list:",input_enc.pid_num);
	for(i = 0; i < input_enc.pid_num; i++)
		AUI_DBG(" [%d]: %d,",i,input_enc.pid_list[i]);

	if(!pst_hdl_dsc_en->pvr_crypto_mode){/*start block mode, init pvr resource in see cpu*/

      #if 0
        // delete former PVR block resource that set in aui_dsc_process_attr_set().
        // because here we should really re-set the PVR block config parameter.
        if (pst_hdl_dsc_en->pvr_fd > 0){
        	ret = alisldsc_pvr_free_resource(&pst_hdl_dsc_en->pvr_fd);
        	if(ret != 0){
        		aui_rtn(AUI_RTN_FAIL,NULL);
        	}
            pst_hdl_dsc_en->pvr_fd = -1;
        }
      #endif

		pst_hdl_dsc_en->pvr_crypto_mode = 1;/*this field identify the block mode have been initialized*/
		input_enc.stream_id = 0xFF;
		ret = alisldsc_pvr_start_block_mode(&pst_hdl_dsc_en->pvr_fd,(unsigned long *)&input_enc,dsc_pvr_block_size);
		if(ret != 0){
			AUI_DBG("start block mode fail!\n");
			aui_rtn(AUI_RTN_FAIL,NULL);
		}
		p_encrypt_status->ul_block_count = input_enc.block_count;
		AUI_DBG("p_encrypt_status->ul_block_count = 0x%08x\n",(unsigned int)p_encrypt_status->ul_block_count);
        AUI_DBG("stream id:%d, sub device id: %d\n", input_enc.stream_id, input_enc.sub_device_id);
		return 0;
	}
	//input_enc.key_handle = pst_hdl_dsc_en->key_handle;
	AUI_DBG("input_enc.key_handle: 0x%08x\n",(unsigned int)input_enc.key_handle);
	ret = alisldsc_pvr_ioctl(pst_hdl_dsc_en->pvr_fd,SL_PVR_IO_UPDATE_ENC_PARAMTOR,(unsigned long)&input_enc);
	if(ret != 0){
		aui_rtn(AUI_RTN_FAIL,NULL);
	}

    
	/*if(attr->dsc_key_type == AUI_DSC_CONTENT_KEY_KL){
		pst_hdl_dsc_en->attr_dsc.ul_key_pos = input_enc.target_key_pos;
	}*/
	/*after init,the following two variable is transfered to main cpu from the see pvr */
	p_encrypt_status->ul_block_count = input_enc.block_count;
	AUI_DBG("input_enc.key_handle: 0x%08x\n",(unsigned int)input_enc.key_handle);
	//aui_dsc_set_playback(pst_hdl_dsc_en);
	AUI_DBG("PVR_RPC_IO_UPDATE_ENC_PARAMTOR ret=%d ,input_enc.stream_id=%d,"
		"pst_hdl_dsc_en->ul_sub_dev_id: %u\n" ,ret, input_enc.stream_id,pst_hdl_dsc_en->ul_sub_dev_id);
	AUI_DBG("p_encrypt_status->ul_block_count = 0x%08x\n",(unsigned int)p_encrypt_status->ul_block_count);
	AUI_DBG("input_enc.target_key_pos: %d\n",input_enc.target_key_pos);
	AUI_DBG("<<<<<<<<<<<<<<<<<<update block mode parameters success>>>>>>>>>>>>>>>>>>>>>>>\n");

	return ret;
}

AUI_RTN_CODE aui_dsc_set_playback(aui_hdl handle)
{
	(void)handle;
	return 0;
}

AUI_RTN_CODE aui_dsc_free_block_mode(aui_hdl handle)
{
	struct dsc_handler *pst_hdl_dsc_en=(struct dsc_handler *)handle;
	if (!handle)
		aui_rtn(AUI_RTN_EINVAL, NULL);
    if (pst_hdl_dsc_en->pvr_fd > 0)
    	if(alisldsc_pvr_free_block_mode(&pst_hdl_dsc_en->pvr_fd,pst_hdl_dsc_en->dsc_fd))
    		aui_rtn(AUI_RTN_EINVAL, NULL);
	AUI_DBG("<<<<<<<<<<<<<<<<<<free block mode>>>>>>>>>>>>>>>>>>>>>>>\n");
	return 0;
}

int aui_dsc_get_stream_id(aui_hdl handle)
{
	(void)handle;
	return 0;
}

/*
Caution in vmx project handle will be NULL.
*/
AUI_RTN_CODE aui_dsc_process_attr_set (
    aui_hdl handle,
    aui_dsc_process_attr *p_process_attr)
{
    int ret = AUI_RTN_SUCCESS;
    char pvr_block_size_is_set = 0;

	struct dsc_handler *dsc_hdl= (struct dsc_handler *)handle;
	if(!p_process_attr) {
		aui_rtn(AUI_RTN_EINVAL, "null input arg\n");
	}
	dsc_pvr_block_size = p_process_attr->ul_block_size;
	if(dsc_hdl) {
		memcpy(&(dsc_hdl->dsc_process_attr), p_process_attr, sizeof(aui_dsc_process_attr));
		AUI_DBG("mode %d, block_size: %lu\n", p_process_attr->process_mode, p_process_attr->ul_block_size);
	}
    if (aui_dsc_is_vmx_module(handle)){
        AUI_DBG("process_mode: %d, block size:%d\n", (int)(dsc_hdl->dsc_process_attr.process_mode),
            (int)(dsc_hdl->dsc_process_attr.ul_block_size));        
        return ret;
    }

    /*
    Do following is to get stream id, sub device for DMX channel start. For TS data tpye, because we do not know
    pids of TS, so we set Pure data type of PVR block record defaultly. In TS data type, if start pvr record first, 
    then dsc update key, because the beginning data is Pure data type, STOP DMX recode will return fail, but it
    dose not influence the video display.
    So api calling flow in TS data type should be better: aui_dsc_process_attr_set-> dsc update key -> dmx start record
    */
    if ((dsc_hdl && (!dsc_hdl->pvr_crypto_mode)) && 
        (AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT == p_process_attr->process_mode))
	{
        struct PVR_BLOCK_ENC_PARAM input_enc;
    	MEMSET(&input_enc,0,sizeof(struct PVR_BLOCK_ENC_PARAM ));

		input_enc.pid_num = 1;
		input_enc.pid_list[0] = R2R_PID;
        input_enc.stream_id = 0xFF;
    	if(AUI_DSC_ALGO_AES == dsc_hdl->algo){
    		input_enc.dsc_sub_device = AES;
    	}else if((AUI_DSC_ALGO_DES == dsc_hdl->algo)
    		|| (AUI_DSC_ALGO_TDES == dsc_hdl->algo)){/*only use TDES*/
    		input_enc.dsc_sub_device = TDES;
    	}else{
    		AUI_ERR("algorithm config error!\n");
    		aui_rtn(AUI_RTN_EINVAL,
                        "parity not supported in RAW mode");
    	}
	    input_enc.work_mode = workmod_to_chainmod_for_pvr(dsc_hdl->mode);
	    input_enc.source_mode = (dsc_hdl->data_type == AUI_DSC_DATA_PURE)? PURE_DATA_MODE: TS_MODE;
        input_enc.root_key_pos= 0xFF;
        
        pvr_block_size_is_set = 1;
        ret = alisldsc_pvr_start_block_mode(&dsc_hdl->pvr_fd,(unsigned long *)&input_enc,dsc_pvr_block_size);
        AUI_DBG("%s(): alisldsc_pvr_start_block_mode return:%d\n", __func__, ret);
        if (0 != ret){
            AUI_ERR("%s(): alisldsc_pvr_start_block_mode return fail!!\n", __func__);
        }
        else{
            dsc_hdl->pvr_crypto_mode = 1;
        }
        
    }
    
    //Except AUI_DSC_PROCESS_MODE_BLOCK_ENCRYPT mode, other modes also should set the
    //PVR block size, and PVR block size can be changed again. Otherwise, the PVR block size of encryption may 
    // be different with decryption, and the block size in DMX may be different with PVR block size.
    if (!pvr_block_size_is_set){
    	if(dsc_hdl->pvr_fd <= 0){
    		dsc_hdl->pvr_fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
    		if(dsc_hdl->pvr_fd  < 0) {
    	        AUI_ERR("Invalid ali_pvr_fd  fd: %d\n", dsc_hdl->pvr_fd);
    	       	return AUI_RTN_FAIL;
    	    } 
    	}
		ret = alisldsc_pvr_ioctl(dsc_hdl->pvr_fd, SL_PVR_IO_SET_BLOCK_SIZE,(unsigned long)dsc_pvr_block_size);
		if(ret != 0){
            AUI_ERR("SL_PVR_IO_SET_BLOCK_SIZE fail!\n");
            return AUI_RTN_FAIL;
		}
    }

    if (dsc_hdl)
    {
        dsc_hdl->pid_flag = 0;
    }
    
	return ret;
}

AUI_RTN_CODE aui_encrypted_stream_inject_to_dmx(
	aui_hdl handle,	unsigned char *ts_buf, int buf_len)
{
	//int i;	
	struct dsc_handler *pst_hdl_dsc_de= (struct dsc_handler *)handle;
	PVR_RPC_RAW_DECRYPT de_input;
	int ret = 0;
	if((handle == NULL)
		|| (ts_buf == NULL)
		|| (buf_len % dsc_pvr_block_size != 0)){
		AUI_ERR("\n");
		aui_rtn(AUI_RTN_FAIL, NULL);
	}
		
	if(pst_hdl_dsc_de->pvr_fd <= 0){
		pst_hdl_dsc_de->pvr_fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
		if(pst_hdl_dsc_de->pvr_fd  < 0) {
	        AUI_ERR("Invalid C200A ali_pvr_fd  fd: %d\n", pst_hdl_dsc_de->pvr_fd);
	       	return 1;
	    } 
	}

	if (pst_hdl_dsc_de->see_dmx_fd <= 0) {
		pst_hdl_dsc_de->see_dmx_fd = open("/dev/ali_m36_dmx_see_0", O_RDWR);
		if (pst_hdl_dsc_de->see_dmx_fd < 0) {
			AUI_ERR("open see dmx fd error!\n");
			return 1;
		}
        
        //sometimes pst_hdl_dsc_de->pvr_fd will be opened in other place, 
        //but we should set block size when playback block mode PVR,
        //move to here, only set once in block playback
		ret = alisldsc_pvr_ioctl(pst_hdl_dsc_de->pvr_fd, SL_PVR_IO_SET_BLOCK_SIZE,(unsigned long)dsc_pvr_block_size);
		if(ret != 0){
			aui_rtn(AUI_RTN_FAIL,NULL);
		}
        
		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd , ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET, (dsc_pvr_block_size+BLOCK_VOB_BUFFER_SIZE)*4);
		if (ret < 0){
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET error\n");
			return 1;
		}

		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd , ALI_DMX_SEE_MAIN2SEE_SRC_SET, DMX_MAIN2SEE_SRC_CRYPT_BLK);
		if (ret < 0){
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_SRC_SET error\n");
			return 1;
		}
	}
 #if 0
    // add CA PID operation will be do in aui_dsc_attach_key_info2dsc()
	struct algo_info algo_info;
	memset(&algo_info,0,sizeof(algo_info));
	if(pst_hdl_dsc_de->pid_flag != 1){
		/* get dsc decrypt fd */
		if(pst_hdl_dsc_de->data_type != AUI_DSC_DATA_PURE){
			if(aui_dsc_get_fd(pst_hdl_dsc_de,&algo_info)){
				aui_rtn(AUI_RTN_FAIL, "dsc_get_fd failed");
			}
			if(aui_dsc_set_pid(pst_hdl_dsc_de,algo_info,AUI_DSC_DECRYPT_IN,AUI_DSC_EVEN_MODE_IN)){
				aui_rtn(AUI_RTN_FAIL, "dsc_set_pid failed");
			}
		}
		pst_hdl_dsc_de->pid_flag = 1;
	}
  #endif
    //VMX AS do not need to identify data_type in AUI
    if (!aui_dsc_is_vmx_module(handle)){
    	/*refresh iv*/
    	if(pst_hdl_dsc_de->data_type == AUI_DSC_DATA_PURE){/*update iv*/
    		struct mkey *mkey;
    	    mkey = aui_list_get_single_entry(mkey, &pst_hdl_dsc_de->key_list, list);
    	    if (!mkey)
    	        aui_rtn(AUI_RTN_EINVAL, "only one key supported");

    	    if (mkey->pid_len != 1 || mkey->pid_list[0] != R2R_PID) {
    	        AUI_DBG("Warning: PID should be %d in ram2ram operation\n", R2R_PID);
    	    }
    		if(alisldsc_update_param(pst_hdl_dsc_de->dev, &mkey->key_from,CA_DECRYPT, mkey->key_handle)) {
                aui_rtn(AUI_RTN_FAIL, "sl update cw error");
            }
    	}
    }

	MEMSET(&de_input,0,sizeof(de_input));
	if (!pst_hdl_dsc_de){
		aui_rtn(AUI_RTN_EINVAL, NULL);
		AUI_ERR("\n");
	}
    //VMX AS do not need to identify algorithm in AUI
    if (!aui_dsc_is_vmx_module(handle)){
    	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->algo){
    		de_input.algo = AES;
    	}else if(AUI_DSC_ALGO_TDES	== pst_hdl_dsc_de->algo){
    		de_input.algo = TDES;
    	}else{
			aui_rtn(AUI_RTN_FAIL,"algorithm config error!");
    		AUI_ERR("\n");
    	}
    }
	de_input.stream_id= 0;
	de_input.dev = (void*)((unsigned int)pst_hdl_dsc_de->dsc_fd);
	de_input.input = ts_buf;
	de_input.length = buf_len;
	
   
	#if 0
	int got_len = 0;
	#endif
	for (;;){
		#if 0
		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_BUF_REQ_SIZE, &got_len);

		if ((ret < 0) || (0 == got_len)){
			//usleep();
			//AUI_DBG("\n");
			continue;
		}

		if(got_len <= (buf_len+BLOCK_VOB_BUFFER_SIZE)){
			//AUI_DBG("\n");
			//sleep(5);
			continue;		
		}
		#endif
		ret = alisldsc_pvr_ioctl(pst_hdl_dsc_de->pvr_fd,SL_PVR_IO_DECRYPT,(unsigned long)&de_input);
		if(ret < 0) {
			AUI_ERR("ioctl error!pvr_fd:%d\n", pst_hdl_dsc_de->pvr_fd);
			AUI_ERR("send data to pvr fail!data counts: %d bytes\n",buf_len);	
			AUI_ERR("error string: %s\n",strerror(errno));
			return 1;
		} 
		#if 0
		AUI_ERR("send data to pvr success!data counts: %d bytes\n",buf_len);	

		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_BUF_RET_SIZE, (buf_len+BLOCK_VOB_BUFFER_SIZE));
		if (ret < 0){
			//sleep(5);
			continue;
		}
		#endif
		break;
	}

#if 0
#if 0
	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->attr_dsc.uc_algo)
	{
		pvr_block_aes_decrypt(de_input.dev, de_input.stream_id,ts_buf, buf_len);
	}
	else if(AUI_DSC_ALGO_TDES == pst_hdl_dsc_de->attr_dsc.uc_algo)
	{
		pvr_block_des_decrypt(de_input.dev, de_input.stream_id,ts_buf, buf_len);
	}
	return 0;
#else
	for (i = 0;i < buf_len;){
#if 0
#ifdef DUAL_ENABLE		
		buf = dmx_main2see_buf_req(dsc_pvr_block_size, &got_len);
#endif
#endif
		de_input.input = ts_buf + i;
		de_input.length = ((buf_len - i) > dsc_pvr_block_size) ? dsc_pvr_block_size : (buf_len - i);
		(void)de_input;

		if(pvr_rpc_ioctl(PVR_RPC_IO_RAW_DECRYPT,(UINT32)&de_input)){
			osal_task_sleep(10);
			continue;
		}
		
		i += de_input.length;
		if( i%(20*dsc_pvr_block_size) == 0){
			AUI_DBG("send data to pvr success!data counts: %d bytes\n",i);
			AUI_DBG("de_input.algo: %d,de_input.dev: %p,de_input.input: %p,de_input.length: 0x%08x,de_input.stream_id: %d\n",
				de_input.algo,de_input.dev,de_input.input,de_input.length,de_input.stream_id);
			
		}
		
		osal_task_sleep(5);
	}

	AUI_DBG("send data to pvr success!data counts: %d bytes\n",i);
	return 0;
#endif
#endif
	return 0;
}
/*
* this function is only used for nestor test.Fobidden to use it other destination.
* It could be deleted in future.
*/
AUI_RTN_CODE aui_encrypted_stream_inject_to_dmx_temp(
	aui_hdl handle,	unsigned char *ts_buf, int buf_len)
{
	//int i;	
	struct dsc_handler *pst_hdl_dsc_de= (struct dsc_handler *)handle;
	PVR_RPC_RAW_DECRYPT de_input;
	int ret = 0;
	if((handle == NULL)
		|| (ts_buf == NULL)
		|| (buf_len % dsc_pvr_block_size != 0)){
		AUI_ERR("\n");
		aui_rtn(AUI_RTN_FAIL, NULL);
	}
		
	if(pst_hdl_dsc_de->pvr_fd <= 0){
		pst_hdl_dsc_de->pvr_fd = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
		if(pst_hdl_dsc_de->pvr_fd  < 0) {
	        AUI_ERR("Invalid C200A ali_pvr_fd  fd: %d\n", pst_hdl_dsc_de->pvr_fd);
	       	return 1;
	    }  
	}
	if (pst_hdl_dsc_de->see_dmx_fd <= 0) {
		pst_hdl_dsc_de->see_dmx_fd = open("/dev/ali_m36_dmx_see_0", O_RDWR);
		if (pst_hdl_dsc_de->see_dmx_fd < 0) {
			AUI_ERR("open see dmx fd error!\n");
			return 1;
		}
		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd , ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET, (dsc_pvr_block_size+BLOCK_VOB_BUFFER_SIZE)*4);
		if (ret < 0){
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET error\n");
			return 1;
		}

		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd , ALI_DMX_SEE_MAIN2SEE_SRC_SET, DMX_MAIN2SEE_SRC_CRYPT_BLK);
		if (ret < 0){
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_SRC_SET error\n");
			return 1;
		}
	}
  #if 0
    // add CA PID operation will be do in aui_dsc_attach_key_info2dsc()
	struct algo_info algo_info;
	memset(&algo_info,0,sizeof(algo_info));
	if(pst_hdl_dsc_de->pid_flag != 1){
		/* get dsc decrypt fd */
		if(pst_hdl_dsc_de->data_type != AUI_DSC_DATA_PURE){
			if(aui_dsc_get_fd(pst_hdl_dsc_de,&algo_info)){
				aui_rtn(AUI_RTN_FAIL, "dsc_get_fd failed");
			}
			if(aui_dsc_set_pid(pst_hdl_dsc_de,algo_info,AUI_DSC_DECRYPT_IN,AUI_DSC_EVEN_MODE_IN)){
				aui_rtn(AUI_RTN_FAIL, "dsc_set_pid failed");
			}
		}
		pst_hdl_dsc_de->pid_flag = 1;
	}
  #endif
    if (!aui_dsc_is_vmx_module(handle)){
    	/*refresh iv*/
    	if(pst_hdl_dsc_de->data_type == AUI_DSC_DATA_PURE){/*update iv*/
    		struct mkey *mkey;
    	    mkey = aui_list_get_single_entry(mkey, &pst_hdl_dsc_de->key_list, list);
    	    if (!mkey)
	        	aui_rtn(AUI_RTN_EINVAL, "only one key supported");

    	    if (mkey->pid_len != 1 || mkey->pid_list[0] != R2R_PID) {
    	        AUI_DBG("Warning: PID should be %d in ram2ram operation\n", R2R_PID);
    	    }
    		if(alisldsc_update_param(pst_hdl_dsc_de->dev, &mkey->key_from,CA_DECRYPT, mkey->key_handle)) {
            	aui_rtn(AUI_RTN_FAIL, "sl update cw error");
            }
    	}
    }
	MEMSET(&de_input,0,sizeof(de_input));
	if (!pst_hdl_dsc_de){
		aui_rtn(AUI_RTN_EINVAL, NULL);
		AUI_ERR("\n");
	}
    //VMX AS do not need to identify algorithm in AUI
    if (!aui_dsc_is_vmx_module(handle)){
    	if(AUI_DSC_ALGO_AES == pst_hdl_dsc_de->algo){
    		de_input.algo = AES;
    	}else if(AUI_DSC_ALGO_TDES	== pst_hdl_dsc_de->algo){
    		de_input.algo = TDES;
    	}else{
			aui_rtn(AUI_RTN_FAIL,"algorithm config error!");
			AUI_ERR("\n");
    	}
    }
	de_input.stream_id= 0;
	de_input.dev = (void*)((unsigned int)pst_hdl_dsc_de->dsc_fd);
	de_input.input = ts_buf;
	de_input.length = buf_len;

	int got_len = 0;
	for (;;){
		/*following code must be run,or playback fail.*/
		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_BUF_REQ_SIZE, &got_len);
		
		if ((ret < 0) || (0 == got_len)){
			continue;
		}

		if(got_len <= (buf_len+BLOCK_VOB_BUFFER_SIZE)){
			continue;		
		}

		ret = alisldsc_pvr_ioctl(pst_hdl_dsc_de->pvr_fd,SL_PVR_IO_DECRYPT,(unsigned long)&de_input);
		if(ret < 0) {
			AUI_ERR("ioctl error!pvr_fd:%d\n", pst_hdl_dsc_de->pvr_fd);
			AUI_ERR("send data to pvr fail!data counts: %d bytes\n",buf_len);	
			AUI_ERR("error string: %s\n",strerror(errno));
			return 1;
		} 
		AUI_DBG("send data to pvr success!data counts: %d bytes\n",buf_len);	
		#if 1	
		ret = ioctl(pst_hdl_dsc_de->see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_BUF_RET_SIZE, (buf_len+BLOCK_VOB_BUFFER_SIZE));
		if (ret < 0){
			continue;
		}
		#endif
		break;
	}
	return 0;
}

AUI_RTN_CODE aui_dsc_pvr_start_record(aui_hdl handle,struct algo_info *algo_info)
{
	//int i;	
	struct dsc_handler *pst_hdl_dsc_de= (struct dsc_handler *)handle;
	if((handle == NULL)
		|| (algo_info == NULL)){
		AUI_ERR("\n");
		aui_rtn(AUI_RTN_FAIL, NULL);
	}
    /* caution : do not change block count 64 * 188 for dmx demand */
	if(alisldsc_config_pvr(&pst_hdl_dsc_de->pvr_fd, algo_info->dsc_fd,
					algo_info->pid_len,algo_info->pid, (64*188)) < 0)
		aui_rtn(AUI_RTN_FAIL, "start pvr record error!\n");
	return 0;
}
 

AUI_RTN_CODE aui_dsc_pvr_playback_env_init(aui_hdl handle, unsigned int block_size, unsigned int de_dsc_num, unsigned int *ali_pvr_de_hdl) 
{
	struct dsc_handler *dsc_hdl= (struct dsc_handler *)handle;
	
	if (!ali_pvr_de_hdl){
		aui_rtn(AUI_RTN_FAIL, "invalid null pointer\n");
	}

	alisl_handle sl_pvr_hdl = NULL;
	if(alisldsc_pvr_open(&sl_pvr_hdl)) {
		AUI_ERR("fail\n");
		return -1;
	}

	if(alisldsc_pvr_ioctl_ex(sl_pvr_hdl, SL_PVR_IO_SET_BLOCK_SIZE, (unsigned long)block_size)){
		AUI_ERR("SL_PVR_IO_CAPTURE_DECRYPT_RES fail\n");

		if(alisldsc_pvr_close(sl_pvr_hdl)) {
			AUI_ERR("ali pvr close fail\n");
		}
		return -1;
	} else {
		AUI_DBG("SL_PVR_IO_SET_BLOCK_SIZE block_size: %u\n", block_size);
	}

	//capture ali_pvr decrypt resource
	struct ali_pvr_capt_decrypt_res_param capt_param;
	MEMSET(&capt_param, 0, sizeof(capt_param));
	capt_param.block_data_size = block_size;
	capt_param.decrypt_dsc_num = de_dsc_num;
	if(alisldsc_pvr_ioctl_ex(sl_pvr_hdl, SL_PVR_IO_CAPTURE_DECRYPT_RES, (unsigned long)&capt_param)){
		AUI_ERR("SL_PVR_IO_CAPTURE_DECRYPT_RES fail\n");
		if(alisldsc_pvr_close(sl_pvr_hdl)) {
			AUI_ERR("ali pvr close fail\n");
		}
		return -1;
	} else {
		AUI_DBG("SL_PVR_IO_CAPTURE_DECRYPT_RES block_size: %u, dsc_num: %u, handle: 0x%x\n",
			capt_param.block_data_size, capt_param.decrypt_dsc_num, capt_param.decrypt_hdl);
	}
	*ali_pvr_de_hdl = capt_param.decrypt_hdl;
	dsc_hdl->sl_pvr_hdl = sl_pvr_hdl;
	dsc_hdl->ali_pvr_de_hdl = capt_param.decrypt_hdl;
	return 0;
}

AUI_RTN_CODE aui_dsc_pvr_playback_env_deinit(struct dsc_handler *hdl) {
	int ret = 0;
	
	if ((!hdl) ||(!hdl->sl_pvr_hdl) ||(!hdl->ali_pvr_de_hdl)){
		AUI_ERR("invalid arg\n");
		return -1;
	}
	
	if(alisldsc_pvr_ioctl_ex(hdl->sl_pvr_hdl, SL_PVR_IO_RELEASE_DECRYPT_RES, (unsigned long)hdl->ali_pvr_de_hdl)) {
		AUI_ERR("SL_PVR_IO_RELEASE_DECRYPT_RES fail\n");
		ret = -1;
	}
	else {
		AUI_DBG("SL_PVR_IO_RELEASE_DECRYPT_RES success\n");
	}
	if(alisldsc_pvr_close(hdl->sl_pvr_hdl)) {
		AUI_ERR("fail\n");
		return -1;
	}
	hdl->sl_pvr_hdl = NULL;
	hdl->ali_pvr_de_hdl = 0;
	return ret;
}

AUI_RTN_CODE aui_dsc_pvr_playback_key_set(aui_hdl handle, unsigned int decrypt_index, unsigned int block_size, 
	unsigned int block_cnt, unsigned int ali_pvr_de_hdl)
{
	struct dsc_handler *dsc_hdl= (struct dsc_handler *)handle;

	if((!handle) ||(!ali_pvr_de_hdl) ||(decrypt_index>1) || (!dsc_hdl->sl_pvr_hdl) ||(!dsc_hdl->ali_pvr_de_hdl)){
		aui_rtn(AUI_RTN_FAIL, "invalid null pointer\n");
	}	
	//set ali pvr decrypt resource
	struct ali_pvr_set_decrypt_res_param set_res_param;
	MEMSET(&set_res_param, 0, sizeof(set_res_param));
	set_res_param.block_data_size = block_size;
	set_res_param.decrypt_hdl = ali_pvr_de_hdl;
	set_res_param.decrypt_index = decrypt_index;
	set_res_param.decrypt_switch_block = block_cnt;

	unsigned char iv_data[16] = {0};
    if (aui_dsc_is_vmx_module(handle)){
        set_res_param.iv_parity=KEY_IV_MODE_EVEN;
        set_res_param.dsc_mode = BLOCK_DATA_MODE_PURE_DATA;
        //aui_vmx_fd_get(dsc_hdl, (int *)&set_res_param.dsc_fd);
        alisl_dsc_get_fd(dsc_hdl->dev, (int *)&set_res_param.dsc_fd);
		set_res_param.iv_lenth = 16;
        //may here will call ioctl to set IV/algo in PVR SEE
    }else{

    	set_res_param.dsc_mode = (dsc_hdl->data_type==AUI_DSC_DATA_TS)?BLOCK_DATA_MODE_TS:BLOCK_DATA_MODE_PURE_DATA;
    	alisl_dsc_get_fd(dsc_hdl->dev, (int *)&set_res_param.dsc_fd);

    	struct mkey *m_key=NULL;
        m_key = aui_list_get_single_entry(m_key, &dsc_hdl->key_list, list);
    	if(!m_key) {
			AUI_ERR("m_key is null\n");
    		return -1;
    	}
    	AUI_DBG("m_key address: %p,m_key->pid_len: %d\n",m_key, m_key->pid_len);
    	aui_dsc_key_type key_type = dsc_hdl->key_type;
    	set_res_param.key_handle = m_key->key_handle;
    	set_res_param.iv_lenth = 8;
    	if(AUI_DSC_ALGO_AES == dsc_hdl->algo) {
    		set_res_param.iv_lenth = 16;
    	}
    	switch(key_type) {
    		case AUI_DSC_HOST_KEY_SRAM:
    			if(CA_MODE_ECB == (m_key->key_from.clear_key->chaining_mode | CA_MODE_ECB)) {
    				set_res_param.iv_lenth = 0;
    				break;
    			}
    			if(CA_VALID_IV_EVEN == (m_key->key_from.clear_key->valid_mask & CA_VALID_IV_EVEN)) {
    				set_res_param.iv_parity=KEY_IV_MODE_EVEN;
    				memcpy(iv_data, m_key->key_from.clear_key->iv_even, set_res_param.iv_lenth);
    			} else if(CA_VALID_IV_ODD== (m_key->key_from.clear_key->valid_mask & CA_VALID_IV_ODD)) {
    				set_res_param.iv_parity=KEY_IV_MODE_ODD;
    				memcpy(iv_data, m_key->key_from.clear_key->iv_odd, set_res_param.iv_lenth);
    			}
    			break;
    		case AUI_DSC_CONTENT_KEY_KL:
    			if(CA_MODE_ECB == (m_key->key_from.kl_key->chaining_mode | CA_MODE_ECB)) {
    				set_res_param.iv_lenth = 0;
    				break;
    			}
    			if(CA_VALID_IV_EVEN == (m_key->key_from.kl_key->valid_mask & CA_VALID_IV_EVEN)) {
    				set_res_param.iv_parity=KEY_IV_MODE_EVEN;
    				memcpy(iv_data, m_key->key_from.kl_key->iv_even, set_res_param.iv_lenth);
    			} else if(CA_VALID_IV_ODD== (m_key->key_from.kl_key->valid_mask & CA_VALID_IV_ODD)) {
    				set_res_param.iv_parity=CA_VALID_IV_ODD;
    				memcpy(iv_data, m_key->key_from.kl_key->iv_odd, set_res_param.iv_lenth);
    			}
    			break;
    		default:
    			break;
    	}
        
    }
        
	set_res_param.iv_data=iv_data;
	
	AUI_DBG("de_hdl: 0x%x, key_handle: %u, block_size: %u, de_idx: %u, block_cnt: %u, dsc_mode: %d, dsc_fd: %u\n",
		set_res_param.decrypt_hdl, set_res_param.key_handle, set_res_param.block_data_size,
		set_res_param.decrypt_index,
		set_res_param.decrypt_switch_block, set_res_param.dsc_mode, set_res_param.dsc_fd);
	AUI_DBG("parity: %d, iv_len: %d, iv: ", set_res_param.iv_parity, set_res_param.iv_lenth);
	unsigned int i = 0;
	for(i= 0; i<set_res_param.iv_lenth;i++) {
		AUI_DBG("0x%02x ", set_res_param.iv_data[i]);
	}
	AUI_DBG("\n");
	if(alisldsc_pvr_ioctl_ex(dsc_hdl->sl_pvr_hdl, SL_PVR_IO_SET_DECRYPT_RES, (unsigned long)&set_res_param)){
		AUI_ERR("set_res_param fail\n");
		return -1;
	} else {
		AUI_DBG("set_res_param success\n");
	}
	return 0;
}

AUI_RTN_CODE aui_dsc_pvr_decrypt_block_data_subsample(alisl_handle sl_pvr_hdl, unsigned int ali_pvr_de_hdl, const unsigned char *buf, 
				int size, int block_idx, unsigned char *iv, unsigned int iv_len, enum pvr_ott_data_type type) 
{	
	if ((!ali_pvr_de_hdl) ||(!sl_pvr_hdl)){
		printf("invalid input arg\n");
		return -1;
	}
	struct ali_pvr_data_decrypt_param de_param;
	int ret = 0;
	memset(&de_param, 0, sizeof(de_param));
	de_param.decrypt_hdl=ali_pvr_de_hdl;
	de_param.block_index= block_idx;
	de_param.input = (unsigned char *)buf;
	de_param.length = size;
	de_param.iv_data = (unsigned long *)iv;
	de_param.iv_length = iv_len;
	de_param.des_flag = type;
	ret = alisldsc_pvr_ioctl_ex(sl_pvr_hdl, SL_PVR_IO_DECRYPT_EVO_SUB, (unsigned long)&de_param);
	if(ret) {
		AUI_ERR("SL_PVR_IO_DECRYPT_EVO hdl:0x%x idx:%d,iv:%p, iv_len: %lu, fail, ret: %d\n", de_param.decrypt_hdl, 
			block_idx, de_param.iv_data, de_param.iv_length, ret);
	} else {
		AUI_DBG("%s -> SL_PVR_IO_DECRYPT_EVO hdl:0x%x %d success\n", __FUNCTION__, de_param.decrypt_hdl, block_idx);
	}
	return ret;

}

AUI_RTN_CODE aui_dsc_pvr_decrypt_block_data(alisl_handle sl_pvr_hdl, unsigned int ali_pvr_de_hdl, const unsigned char *buf, 
				int size, int block_idx, unsigned char *iv, unsigned int iv_len, enum pvr_ott_data_type type) 
{	
	if ((!ali_pvr_de_hdl) ||(!sl_pvr_hdl)){
		AUI_ERR("invalid input arg\n");
		return -1;
	}
	struct ali_pvr_data_decrypt_param de_param;
	int ret = 0;
	memset(&de_param, 0, sizeof(de_param));
	de_param.decrypt_hdl=ali_pvr_de_hdl;
	de_param.block_index= block_idx;
	de_param.input = (unsigned char *)buf;
	de_param.length = size;
	de_param.iv_data = (unsigned long *)iv;
	de_param.iv_length = iv_len;
	de_param.des_flag = type;
	ret = alisldsc_pvr_ioctl_ex(sl_pvr_hdl, SL_PVR_IO_DECRYPT_EVO, (unsigned long)&de_param);
	if(ret) {
		AUI_ERR("SL_PVR_IO_DECRYPT_EVO hdl:0x%x idx:%d,iv:%p, iv_len: %lu, fail, ret: %d\n", de_param.decrypt_hdl, 
			block_idx, de_param.iv_data, de_param.iv_length, ret);
	} else {
		//AUI_DBG("%s -> SL_PVR_IO_DECRYPT_EVO hdl:0x%x %d success\n", __FUNCTION__, de_param.decrypt_hdl, block_idx);
	}
	return ret;
}

AUI_RTN_CODE aui_dsc_resource_id_get (aui_hdl handle, aui_dsc_resource_id *p_resource_id) 
{
	struct dsc_handler* hdl = (struct dsc_handler *)handle;
    if((NULL == hdl) || (NULL  == p_resource_id))
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameters!\n");
	ali_pvr_de_hdl_magic tmp;
	memset(&tmp, 0, sizeof(tmp));
	//for vmx ott project, block_size is 0
	unsigned int block_size = 0;
	unsigned int dsc_num = 1;
	unsigned int dsc_index = 0;
	unsigned int block_cnt = 0xFFFFFFFE;
	if(aui_dsc_pvr_playback_env_init(handle, block_size, dsc_num, &tmp.ali_pvr_de_hdl)) {
		AUI_ERR("fail\n");
		return -1;
	}
	if(aui_dsc_pvr_playback_key_set(handle, dsc_index, block_size, block_cnt, tmp.ali_pvr_de_hdl)) {
		AUI_ERR("fail\n");
		aui_dsc_pvr_playback_env_deinit(hdl);
		return -1;
	}
	memcpy(p_resource_id, &tmp, sizeof(tmp));
	return AUI_RTN_SUCCESS;
}

int aui_dsc_pvr_mmap(aui_hdl *handle, unsigned int *mmap_addr, unsigned int *mmap_len) 
{
	if((!handle)||(!mmap_addr)||(!mmap_len)) {
		AUI_ERR("fail\n");
		return -1;
	}
	alisl_handle hdl = NULL;
	if(alisldsc_pvr_open(&hdl)) {
		AUI_ERR("fail\n");
		return -1;
	}
	if(alisldsc_pvr_mmap(hdl, mmap_addr, mmap_len, NEED_MMAP)) {
		AUI_ERR("fail\n");
		return -1;
	}
	*handle = hdl;
	return 0;
}

int aui_dsc_pvr_munmap(aui_hdl handle, unsigned int *mmap_addr, unsigned int *mmap_len) 
{
	if((!handle)||(!mmap_addr)||(!mmap_len)) {
		AUI_ERR("fail\n");
		return -1;
	}
	alisl_handle hdl = handle;
	
	if(alisldsc_pvr_mmap(hdl, mmap_addr, mmap_len, NEED_MUNMAP)) {
		AUI_ERR("fail\n");
		return -1;
	}
	if(alisldsc_pvr_close(hdl)) {
		AUI_ERR("fail\n");
		return -1;
	}
	return 0;
}

//check if the handle is the handle of VMX device
int aui_dsc_is_vmx_module(aui_hdl handle)
{
    int is_vmx_module = 0;
	aui_hdl aui_handle = NULL;
    struct dsc_handler* check_hdl = (struct dsc_handler *)handle;
    
	if (aui_find_dev_by_idx(AUI_MODULE_VMX, check_hdl->data.dev_idx, &aui_handle)) {
		//AUI_ERR("can not find the VMX device of specal index!\n");
		return 0;
	}
    if ((unsigned long)aui_handle == (unsigned long)check_hdl){
        is_vmx_module = 1;
    }else{
        is_vmx_module = 0;
    }

    return is_vmx_module;
}

AUI_RTN_CODE aui_dsc_iv_attr_set(aui_hdl handle, aui_attr_dsc_iv *iv_attr)
{
    struct dsc_handler* dsc_hdl = (struct dsc_handler*)handle;

    if ((NULL == dsc_hdl) || (NULL == iv_attr)){
        aui_rtn(AUI_RTN_EINVAL, "dsc_hdl:0x%x, iv_attr:0x%x\n", 
            (unsigned int)dsc_hdl, (unsigned int)iv_attr);
    }

    if (!dsc_hdl->key_attached){
        aui_rtn(AUI_RTN_EINVAL, "DSC not be attached!!\n");
    }

    dsc_hdl->dsc_iv_mode = iv_attr->dsc_iv_mode;
    if (AUI_DSC_IV_MODE_UPDATE == iv_attr->dsc_iv_mode){
        int ret = -1;
        int iv_length = 0;
        unsigned short pid = 0;
        struct mkey *mkey = NULL;
        struct ca_update_params update_params;

        if (dsc_hdl->data_type == AUI_DSC_DATA_PURE){
            if (NULL == iv_attr->iv_even){
                //pure data only can update IV even.
                aui_rtn(AUI_RTN_EINVAL, "pure data only can update IV EVEN, please set IV EVEN!\n");
            }
            pid = R2R_PID;
        }else{
            if (NULL == iv_attr->pids){
                AUI_ERR("Please set PIDs for TS mode!\n");
                return AUI_RTN_FAIL;
            }
            pid = iv_attr->pids[0];
        }
        memset(&update_params, 0, sizeof(update_params));

        if ((AUI_DSC_ALGO_DES == dsc_hdl->algo) || (AUI_DSC_ALGO_TDES == dsc_hdl->algo))
            iv_length = 8;
        else if (AUI_DSC_ALGO_AES == dsc_hdl->algo)
            iv_length = 16;
        else
            AUI_ERR("CSA do not support IV!\n");

        if (iv_length){
            update_params.valid_mask = 0;
			//search the dsc resource through pid
			mkey = find_mkey_with_pid(&dsc_hdl->key_list, pid);
            if (!mkey)
                aui_rtn(AUI_RTN_EINVAL, "only one key supported");
            
            if (iv_attr->iv_odd){
                update_params.valid_mask |= CA_VALID_IV_ODD;
                memcpy(update_params.iv_odd, iv_attr->iv_odd, iv_length);
                AUI_DUMP("iv_odd: ", (char*)(update_params.iv_odd), (int)iv_length);
                
            }
            if (iv_attr->iv_even){
                update_params.valid_mask |= CA_VALID_IV_EVEN;
                memcpy(update_params.iv_even, iv_attr->iv_even, iv_length);
                AUI_DUMP("iv_even: ", (char*)(update_params.iv_even), (int)iv_length);
            }

            if (update_params.valid_mask){
                update_params.key_handle = mkey->key_handle;
                ret = alisldsc_dsc_ioctl(dsc_hdl->dev, CA_UPDATE_PARAMS, (unsigned int)(&update_params));
                if (ret)
                    aui_rtn(AUI_RTN_FAIL, "alisldsc_dsc_ioctl err: %d\n", ret);
            }
        }
    }

    return AUI_RTN_SUCCESS;
}
