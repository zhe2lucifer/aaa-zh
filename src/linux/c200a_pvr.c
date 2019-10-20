/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: c200a_pvr.c
*
*    Description: This file contains the callback functions for re-encryption of transport stream
                 and encryption and HMAC of related metadata
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <basic_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "aui_common_priv.h"
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/trng/trng.h>
#include <bus/otp/otp.h>
#include <ali_dmx_common.h>	
#include <aui_dmx.h>
#include <linux/ali_pvr.h>
#include <ca_kl.h>
#include <ca_dsc.h>
#include "c200a_pvr.h"
#include <aui_ca_pvr.h>
#include <aui_pvr.h>
#include <alidefinition/adf_pvr.h>

AUI_MODULE(CA_PVR)

#define C200A_PVR_DEBUG     AUI_DBG
#define C200A_PVR_ERROR     AUI_ERR

#ifdef NEW_DRIVER
#define INVALID_KL_FD        (-1)
#define INVALID_DSC_FD       (-1)
#define INVALID_ALI_PVR_FD   (-1)
#endif

//extern void pvr_set_block_mode(UINT8 enable); //dd this function is not used.Because this process is called by callback function in upper layer.
//extern int pvr_block_aes_decrypt(pAES_DEV paesdev, UINT16 stream_id, UINT8 *input, UINT32 total_length);//this function is not used.Because this process is called by callback function in upper layer.
#ifdef NEW_DRIVER
#define DSC_DEV_PATH                "/dev/dsc0"
#define KL_DEV_PATH_0               "/dev/kl0"
#define KL_DEV_PATH_1               "/dev/kl1" //C200A AES 
#define KL_DEV_PATH_2               "/dev/kl2" //C200A DES 
#define KL_DEV_PATH_3               "/dev/kl3"
#define KL_DEV_PATH_4               "/dev/kl4"
#define ALI_PVR_DEV_PATH            "/dev/ali_pvr0"
#define C200A_PVR_KEY_OTP_ADDR      (0x70)
#define C200A_PVR_KEY_LEN           (16)        // measured in byte
#define C200A_CRYPTO_KEY_LEVEL      (3)
#define C200A_CRYPTO_KEY_LENGTH     (16) //bytes
#define C200A_CRYPTO_IV_LENGTH      (16) //bytes
#define C200A_TS_PACKAGE_SIZE       (188) //bytes

#define C200A_PVR_KEY_DEBUG_ENABLE       (1)
#define C200A_PVR_RSC_TYPE_REC           (0)
#define C200A_PVR_RSC_TYPE_PLAY          (1)
#define C200A_PVR_RSC_REC_NUM            (PVR_MAX_REC_NUM)
#define C200A_PVR_RSC_PLAY_NUM           (PVR_MAX_PLAY_NUM)
#endif

//#define INVALID_CRYPTO_MODE             (0xffffffff)
#define INVALID_CRYPTO_FD               (-1)
#define INVALID_CRYPTO_CHID               (0xffffffff)
//#define INVALID_PID                     (0x1fff)


//#define C200A_PVR_RSC_MAX            (3)
#if 0
static const UINT8 pvr_otp_random_key[C200A_CRYPTO_KEY_LENGTH]=  
{
    0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,    
};
#endif

#ifdef NEW_DRIVER
//TDES PK can't high 64bits == low 64bits
static const UINT8 const_random_key[C200A_CRYPTO_KEY_LENGTH]=  
{
    0x00,0x00,0x00,0x00,0x22,0x22,0x22,0x22,
    0x11,0x11,0x11,0x11,0x33,0x33,0x33,0x33,
};
//TDES PK can't high 64bits == low 64bits
static const UINT8 const_even_key[C200A_CRYPTO_KEY_LENGTH]=
{
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,

};

//TDES PK can't high 64bits == low 64bits
static const UINT8 const_odd_key[C200A_CRYPTO_KEY_LENGTH]=
{
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,
};
#endif
/****this params are no used.Because these params are used by callback function.Callback function is called in upper layer.***/
//static aui_pvr_c200a_callback_fun get_fd_by_ts_id = NULL;
//static void *c200a_user_data= NULL;
//static int ali_pvr_fd= INVALID_ALI_PVR_FD;


//c200a_crypto_info tmp_enc_info;    // encrypt info
//c200a_crypto_info tmp_dec_info;	  // decrypt info
/*
static struct dmx_dsc_fd_param pvr_de_enconfig[C200A_PVR_RSC_MAX] =
{
    {-1,-1},
    {-1,-1},
    {-1,-1},
};
*/
/****this params are no used.Because these params are used by callback function.Callback function is called in upper layer.***/


/* C200A PVR Manager */

/* pvrDeEnconfig is used by other module, MUST defined as static or global */


/* extern function */


/**
*    @brief         generate random numbers as encrypted keys
*    @param[in]     key_param   store key number, key length(bit), key address
*    @return        successful -- 0, failed -- -1
*    @note
*
*/
int aui_c200a_pvr_generate_keys(pvr_crypto_key_param *key_param)
{
    int i = 0;
    int key_bytes = 0;
    int loop_cnt1 = 0; 
    int loop_cnt2 = 0;
    UINT8 *key_ptr = NULL;

    if (NULL == key_param) {
        AUI_ERR("Error key_param\n");
        return -1;
    }

    key_bytes = (key_param->key_len + 7) / 8;   /* bytes for one key */
    key_bytes = key_bytes * key_param->key_num; /* total bytes for all keys */
    loop_cnt1 = key_bytes / 8;                  /* generate 64bits per loop */
    loop_cnt2 = key_bytes % 8;                  /* generate 1Byte per loop */

    AUI_DBG("generate keys for recording, loop_cnt: (%d, %d)\n", 
            loop_cnt1, loop_cnt2);
            
    key_ptr = key_param->keys_ptr;

    for (i = 0; i < loop_cnt1; i ++) {
        trng_generate_64bits(key_ptr);
        key_ptr += 8;
    }

    for (i = 0; i < loop_cnt2; i ++) {
        trng_generate_byte(key_ptr);
        key_ptr++;
    }

    return 0;
}
#if 0
int c200a_pvr_rec_config(pvr_crypto_general_param *rec_param)
{
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, rec_param->dmx_id);
    if(NULL != get_fd_by_ts_id)
    {
        get_fd_by_ts_id(c200a_user_data, &(tmp_dec_info), &(tmp_enc_info));
    }
    AUI_DBG("c200a_pvr dec fd(%d) enc fd(%d)\n ",tmp_dec_info.sub_device_id,tmp_enc_info.sub_device_id);
	pvr_set_block_mode(1);
	pvr_de_enconfig[0].decrypt_fd = tmp_dec_info.sub_device_id;
    pvr_de_enconfig[0].encrypt_fd = tmp_enc_info.sub_device_id;     
	dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig[0]);
	return 0;
}
#endif

#if 0 //this function is no used.Because callback is abolished.Customer can get fd by ts id in the upper layer.
int aui_c200a_pvr_register_callback(aui_pvr_c200a_callback_fun callback_fun, void * user_data)
{
	c200a_user_data = user_data;
    get_fd_by_ts_id = callback_fun;
	
	return 0;
}
#endif
#ifdef NEW_DRIVER    
static int c200a_load_pk(unsigned char pk_buffer[][C200A_CRYPTO_KEY_LENGTH])
{
    int ret = 0;
    int i = 0;
    
    if(pk_buffer != NULL)
    {
        otp_init();
        memset(&pk_buffer[0][0],0x00,C200A_CRYPTO_KEY_LENGTH);
        for (i = 0; i < C200A_PVR_KEY_LEN / 4; i++)
        {
            otp_read((C200A_PVR_KEY_OTP_ADDR+i)*4, &pk_buffer[i*4], 4);
        }
        memset(&pk_buffer[1][0],0x00,C200A_CRYPTO_KEY_LENGTH);
        memcpy(&pk_buffer[1][0],const_random_key,C200A_CRYPTO_KEY_LENGTH);
        
    }
    else
    {
        ret  = -1;
    }
    return ret;
}
#endif

/**
*    @brief         encrypt or decrypt data, default mode (TDES, CBC)
*    @param[in]     cp  store input/output buffer, buffer length, key, IV, mode
*    @return        successful -- 0, failed -- -1
*    @note
*
*/
int aui_c200a_crypto_data(pvr_crypto_data_param *cp)
{
#ifndef NEW_DRIVER    
    if(NULL != cp)
    {
        memcpy(cp->output,cp->input, cp->data_len);
        return 0;
    }
    return -1;
#else
    int crypt_kl_fd   = INVALID_KL_FD;
    int crypt_dsc_fd  = INVALID_DSC_FD;

    int ret = 0;
    int ca_format = 0;
    
    unsigned char protect_key[C200A_CRYPTO_KEY_LEVEL][C200A_CRYPTO_KEY_LENGTH];
	struct kl_config_key cfg_key;	
    struct kl_gen_key gen_key;	
    struct ca_create_kl_key kl_key_info; 
    struct ca_dio_write_read crypto_io_wr;

    /*********************************** KL  **********************************/
    crypt_kl_fd  = open(KL_DEV_PATH_2,O_RDWR); 
    if (crypt_kl_fd < 0)
    {
        AUI_ERR("Invalid C200A crypt kl id: %d\n", crypt_kl_fd);
        ret =   -1;
        goto f_quit;
    }

    memset(&cfg_key, 0, sizeof(struct kl_config_key));//config key
	cfg_key.algo = KL_ALGO_TDES;//C200a SPEC DISABLE kl aes.
    cfg_key.crypt_mode = KL_DECRYPT;
    cfg_key.ck_size = KL_CK_KEY128;
    cfg_key.level = 3;
    cfg_key.ck_parity = KL_CK_PARITY_ODD_EVEN;
    ret = ioctl(crypt_kl_fd, KL_CONFIG_KEY, (void *)&cfg_key);    
    if(ret < 0)
    {
        AUI_ERR("Invalid C200A config kl error!\n");
        goto f_quit;            
    }
    
    //generate key    
	memset(&gen_key, 0, sizeof(struct kl_gen_key));
    c200a_load_pk(protect_key);
    
	memcpy(gen_key.pk[0], protect_key[0], C200A_CRYPTO_KEY_LENGTH);
	//memcpy(gen_key.pk[1], protect_key[1], C200A_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.pk[1],cp->key_ptr,C200A_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.key_odd, const_odd_key , C200A_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.key_even, const_even_key, C200A_CRYPTO_KEY_LENGTH);
	ret = ioctl(crypt_kl_fd, KL_GEN_KEY, (void *)&gen_key);
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl KL_GEN_KEY fail !! ret:%d \n", ret);         
		goto f_quit;
	}
    /*********************************** DSC **********************************/
    //set format
    crypt_dsc_fd = open(DSC_DEV_PATH, O_RDWR);
    if (crypt_dsc_fd < 0)
    {
        AUI_ERR("Invalid C200A crypt dsc fd: %d\n", crypt_dsc_fd);
        ret =   -1;
        goto f_quit;
    }    
    ca_format = CA_FORMAT_RAW;
	ret = ioctl(crypt_dsc_fd, CA_SET_FORMAT, &ca_format);
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl CA_SET_FORMAT fail !! ret:%d\n",ret); 
		goto f_quit;
	}
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl KL_GEN_KEY fail !! ret:%d \n",ret);         
		goto f_quit;
	}
	//set KL key
	memset(&kl_key_info, 0, sizeof(struct ca_create_kl_key)); 
    kl_key_info.kl_fd = crypt_kl_fd;
    kl_key_info.algo  = CA_ALGO_TDES;
    if(cp->encrypt)
    {
        kl_key_info.crypt_mode = CA_ENCRYPT;
    }
    else
    {
        kl_key_info.crypt_mode = CA_DECRYPT;
    }
    kl_key_info.chaining_mode = CA_MODE_CBC;    
    kl_key_info.residue_mode = CA_RESIDUE_CLEAR;
    kl_key_info.parity = CA_PARITY_EVEN;
    memcpy(kl_key_info.iv_even ,cp->iv_ptr,C200A_CRYPTO_IV_LENGTH);
    kl_key_info.valid_mask = \
        CA_VALID_PARITY|CA_VALID_KEY_EVEN|CA_VALID_IV_EVEN|CA_VALID_RESIDUE_MODE;    
	ret = ioctl(crypt_dsc_fd, CA_CREATE_KL_KEY, (void *)&kl_key_info);	
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl CA_CREATE_KL_KEY fail !! ret:%d \n", ret);         
        goto f_quit;  
    }
    memset(&crypto_io_wr, 0, sizeof(struct ca_dio_write_read));    
    crypto_io_wr.crypt_mode = kl_key_info.crypt_mode;
    crypto_io_wr.length = (ca_format == CA_FORMAT_RAW) ? cp->data_len:(cp->data_len/C200A_TS_PACKAGE_SIZE);
    crypto_io_wr.input = cp->input;
    crypto_io_wr.output = cp->output;
	ret = ioctl(crypt_dsc_fd, CA_DIO_WRITE_READ, (void *)&crypto_io_wr);	
    if(ret < 0)	
    {       
        AUI_ERR("Error: ioctl CA_DIO_WRITE_READ fail !! ret:%d \n",ret);         
        goto f_quit;  
    }
f_quit:
    if(crypt_dsc_fd >= 0)
    {
        close(crypt_dsc_fd);
        crypt_dsc_fd = -1;
    }
    if(crypt_kl_fd >= 0)
    {
        close(crypt_kl_fd);
        crypt_kl_fd = -1;
    }    
    return ret;
#endif
}

#if 0 //this part process is handed by upper layer.
int c200a_pvr_block_decrypt(UINT32 handle, UINT8 *buffer, UINT32 length, UINT32 block_idx)
{
    int ret = -1;
	c200a_pvr_resource *play_rsc = NULL;
    PVR_RPC_RAW_DECRYPT decrypt_info;
    UNUSED(block_idx);
    
    AUI_DBG("run\n");
    if (0 == handle)
    {
        AUI_ERR("PVR handler is NULL\n");
        return ret;
    }
    play_rsc = c200a_pvr_resource_find(C200A_PVR_RSC_TYPE_PLAY, handle);
    if(INVALID_ALI_PVR_FD  == ali_pvr_fd)
    {
        ali_pvr_fd   = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
        AUI_DBG(" open  fd: %d\n", ali_pvr_fd);
        if(ali_pvr_fd < 0)
        {
            AUI_ERR("Invalid C200A ali_pvr_fd  fd: %d\n", ali_pvr_fd);
            ret =   -1;
            return ret;
        }   
    }
    if(NULL != play_rsc)
    {    

    //==============update IV if pure mode and IV !=NULL
        
        if((PURE_DATA_MODE == play_rsc->dec_info.source_mode) && (WORK_MODE_IS_ECB != play_rsc->dec_info.work_mode))
        {
            struct ca_update_params update_params;
            
            memset(&update_params,0,sizeof(update_params));
            update_params.key_handle =  play_rsc->dec_info.key_handle;
            update_params.crypt_mode = CA_DECRYPT;
            update_params.chaining_mode = CA_MODE_CBC;
            update_params.residue_mode = play_rsc->dec_info.residue_mode;
            if(0 == play_rsc->dec_info.key_mode) //even
            {
                memcpy(update_params.iv_even,play_rsc->dec_info.input_iv,16);
                update_params.parity = CA_PARITY_EVEN;
                update_params.valid_mask |= UP_PARAM_IV_EVEN;
                AUI_DBG("EVEN IV will be update!\n");
            }
            else
            {
                memcpy(update_params.iv_odd,play_rsc->dec_info.input_iv,16);
                update_params.parity = CA_PARITY_ODD;
                update_params.valid_mask |= UP_PARAM_IV_ODD;
                AUI_DBG("ODD IV will be update!\n");
            }
            ret = ioctl(play_rsc->dec_info.sub_device_id, CA_UPDATE_PARAMS, (void *)&update_params);       

            
        }

        memset(&decrypt_info, 0, sizeof(PVR_RPC_RAW_DECRYPT));
        decrypt_info.dev =(void*)((unsigned int) play_rsc->dec_info.sub_device_id);
        decrypt_info.algo = AES;
        decrypt_info.input = buffer;
        decrypt_info.length= length;
        decrypt_info.stream_id =0; 
        AUI_DBG("PVR_IO_DECRYPT fd = %d \n",play_rsc->dec_info.sub_device_id);
        ret = ioctl(ali_pvr_fd,PVR_IO_DECRYPT,&decrypt_info);
    }
    else
    {
        AUI_ERR("play_rsc is NULL\n");
    }
    AUI_DBG("pvr_block_aes_decrypt ret = %d \n",ret);
    return ret;
}

int c200a_pvr_block_decrypt_exe(UINT32 handle, UINT8 *buffer, UINT32 length, UINT32 block_idx,UINT32 indicator)
{
    int ret = -1;
	c200a_pvr_resource *play_rsc = NULL;
    PVR_RPC_RAW_DECRYPT decrypt_info;
	enum block_status  block_indicator = BLOCK_NORMAl;
    UNUSED(block_idx);
    AUI_DBG("run\n");
    if (0 == handle)
    {
        AUI_ERR("PVR handler is NULL\n");
        return ret;
    }
    play_rsc = c200a_pvr_resource_find(C200A_PVR_RSC_TYPE_PLAY, handle);
    if(INVALID_ALI_PVR_FD  == ali_pvr_fd)
    {
        ali_pvr_fd   = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
        AUI_DBG(" open  fd: %d\n", ali_pvr_fd);
        if(ali_pvr_fd < 0)
        {
            AUI_ERR("Invalid C200A ali_pvr_fd  fd: %d\n", ali_pvr_fd);
            ret =   -1;
            return ret;
        }   
    }
    if(NULL != play_rsc)
    {    
    //==============update IV if pure mode and IV !=NULL
        if((PURE_DATA_MODE == play_rsc->dec_info.source_mode) && (WORK_MODE_IS_ECB != play_rsc->dec_info.work_mode))
        {
            struct ca_update_params update_params;
            memset(&update_params,0,sizeof(update_params));
            update_params.key_handle =  play_rsc->dec_info.key_handle;
            update_params.crypt_mode = CA_DECRYPT;
            update_params.chaining_mode = CA_MODE_CBC;
            update_params.residue_mode = play_rsc->dec_info.residue_mode;
            if(0 == play_rsc->dec_info.key_mode) //even
            {
                memcpy(update_params.iv_even,play_rsc->dec_info.input_iv,16);
                update_params.parity = CA_PARITY_EVEN;
                update_params.valid_mask |= UP_PARAM_IV_EVEN;
                AUI_DBG("EVEN IV will be update!\n");
            }
            else
            {
                memcpy(update_params.iv_odd,play_rsc->dec_info.input_iv,16);
                update_params.parity = CA_PARITY_ODD;
                update_params.valid_mask |= UP_PARAM_IV_ODD;
                AUI_DBG("ODD IV will be update!\n");
            }
			int i;
			AUI_DBG("iv_parity:0x%08x \n",(unsigned int )play_rsc->dec_info.key_mode);
			for(i=0; i<16; i++)
    		{
        		AUI_DBG("0x%02x, ",play_rsc->dec_info.input_iv[i]);
    		}
			AUI_DBG("up  iv key\n");
            ret = ioctl(play_rsc->dec_info.sub_device_id, CA_UPDATE_PARAMS, (void *)&update_params);       
        }
        memset(&decrypt_info, 0, sizeof(PVR_RPC_RAW_DECRYPT));
        decrypt_info.dev =(void*)((unsigned int) play_rsc->dec_info.sub_device_id);
        decrypt_info.algo = AES;
        decrypt_info.input = buffer;
        decrypt_info.length= length;
        decrypt_info.stream_id =0;
		if(indicator == VOB_START)
        {
            block_indicator = BLOCK_START;
        }
        else if(indicator == VOB_END)
        {
            block_indicator = BLOCK_END;
        }
        else
        {
            block_indicator = BLOCK_NORMAl;
        }
		decrypt_info.indicator = block_indicator;
        AUI_DBG("PVR_IO_DECRYPT fd = %d \n",play_rsc->dec_info.sub_device_id);
        ret = ioctl(ali_pvr_fd,PVR_IO_DECRYPT,&decrypt_info);
    }
    else
    {
        AUI_ERR("play_rsc is NULL\n");
    }
    AUI_DBG("pvr_block_aes_decrypt ret = %d \n",ret);
    return ret;
}
int aui_c200a_pvr_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data)
{
	int ret = -1;
    aui_pvr_block_decrypt_param *decrypt_param = NULL;
    UNUSED(user_data);
    
    switch(msg_type)
    {
		case AUI_EVNT_PVR_MSG_BLOCK_MODE_DECRYPT:
            decrypt_param = (aui_pvr_block_decrypt_param *)msg_code;
            ret = c200a_pvr_block_decrypt((UINT32)handle, decrypt_param->input,decrypt_param->length, decrypt_param->block_idx);
			break;
		default :
			break;
	}
    return ret;
}
#endif
