/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: gen_ca_pvr.c
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
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/trng/trng.h>
#include <bus/otp/otp.h>
#include <ali_dmx_common.h>	
#include <linux/ali_pvr.h>
#include <ca_kl.h>
#include <ca_dsc.h>
#include "gen_ca_pvr.h"
#include <aui_ca_pvr.h>
#include "aui_common_priv.h"
#include <api/libpvr/lib_pvr.h>

AUI_MODULE(CA_PVR)

#define DSC_DEV_PATH    				"/dev/dsc0"

#define KL_DEV_PATH_0   				"/dev/kl/kl0"
#define KL_DEV_PATH_1   				"/dev/kl/kl1" //GEN_CA AES 
#define KL_DEV_PATH_2   				"/dev/kl/kl2" //GEN_CA DES 
#define KL_DEV_PATH_3   				"/dev/kl/kl3"
#define KL_DEV_PATH_4   				"/dev/kl/kl4"
#define ALI_PVR_DEV_PATH                "/dev/ali_pvr0"
#define GEN_CA_PVR_KEY_OTP_ADDR           0x70
#define GEN_CA_PVR_KEY_LEN                16        // measured in byte
#define GEN_CA_CRYPTO_KEY_LEVEL           2
#define GEN_CA_CRYPTO_KEY_LENGTH          16 //bytes
#define GEN_CA_CRYPTO_IV_LENGTH           16 //bytes
#define GEN_CA_TS_PACKAGE_SIZE           	188 //bytes

#define GEN_CA_PVR_RSC_TYPE_REC           0
#define GEN_CA_PVR_RSC_TYPE_PLAY          1
#define GEN_CA_PVR_RSC_REC_NUM            PVR_MAX_REC_NUM
#define GEN_CA_PVR_RSC_PLAY_NUM           PVR_MAX_PLAY_NUM

#define INVALID_KL_FD        (-1)
#define INVALID_DSC_FD       (-1)
#define INVALID_ALI_PVR_FD   (-1)

#define TS_SCR_BIT_EVEN      (0x02)           
#define TS_SCR_BIT_ODD       (0x03)


/*******************************************************************************
* Local variables declaration
*******************************************************************************/

typedef struct
{
    UINT32      pvr_hnd;
    int         dsc_fd;
    int         kl_fd;
    int         ali_pvr_fd;    
#if defined(MULTI_DESCRAMBLE)
    UINT8   session_id;         //back up prog session
#endif
} gen_ca_pvr_resource;

typedef struct
{
    ID                    mutex_id;
    gen_ca_pvr_resource     rec[GEN_CA_PVR_RSC_REC_NUM];
    gen_ca_pvr_resource     play[GEN_CA_PVR_RSC_PLAY_NUM];
} gen_ca_pvr_mgr;

/* GEN_CA PVR Manager */
static gen_ca_pvr_mgr g_gen_ca_pm;
#ifdef MULTI_DESCRAMBLE
static UINT16 timeshift_stream_id = 0xff;     //save the stream id for do timeshift
#endif

#define aui_gen_ca_pvr_lock()        		osal_mutex_lock(g_gen_ca_pm.mutex_id, OSAL_WAIT_FOREVER_TIME)
#define aui_gen_ca_pvr_un_lock()        	osal_mutex_unlock(g_gen_ca_pm.mutex_id)

//TDES PK can't high 64bits == low 64bits
static const UINT8 pvr_otp_random_key[GEN_CA_CRYPTO_KEY_LENGTH]=  
{
    0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,    
};

//TDES PK can't high 64bits == low 64bits
static const UINT8 const_random_key[GEN_CA_CRYPTO_KEY_LENGTH]=  
{
    0x00,0x00,0x00,0x00,0x22,0x22,0x22,0x22,
    0x11,0x11,0x11,0x11,0x33,0x33,0x33,0x33,
};

//TDES PK can't high 64bits == low 64bits
static const UINT8 const_even_key[GEN_CA_CRYPTO_KEY_LENGTH]=
{
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,

};

//TDES PK can't high 64bits == low 64bits
static const UINT8 const_odd_key[GEN_CA_CRYPTO_KEY_LENGTH]=
{
    0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,
    0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,
};

static struct dmx_dsc_fd_param pvr_de_enconfig[GEN_CA_PVR_RSC_REC_NUM] =
{
    {-1,-1},
    {-1,-1},
    {-1,-1},
};

static int aui_gen_ca_load_pk(unsigned char pk_buffer[][GEN_CA_CRYPTO_KEY_LENGTH],int key_level)
{
    int ret = 0;
    int i = 0;
    
    if(pk_buffer != NULL)
    {
        otp_init();
        if(key_level >= 1)
        {
            memset(&pk_buffer[0][0],0x00,GEN_CA_CRYPTO_KEY_LENGTH);
            for (i = 0; i < GEN_CA_PVR_KEY_LEN / 4; i++)
            {
                otp_read((GEN_CA_PVR_KEY_OTP_ADDR+i)*4, &pk_buffer[0][i*4], 4);
                
            }
            for(i = 0; i < GEN_CA_CRYPTO_KEY_LENGTH;i++)
            {
                pk_buffer[0][i] = ((~pk_buffer[0][i]) & pvr_otp_random_key[i]) | pvr_otp_random_key[i];  
            }
            
            if(key_level > 1)
            {
                for(i = 0; i < key_level - 2;i++)
                {
                    memset(&pk_buffer[i + 1][0],0x00,GEN_CA_CRYPTO_KEY_LENGTH);
                    memcpy(&pk_buffer[i + 1][0],const_random_key,GEN_CA_CRYPTO_KEY_LENGTH);
                }
            }
        }
        else
        {
            ret = -2;
        }
    }
    else
    {
        ret  = -1;
    }

    return ret;
}

/*******************************************************************************
*    Local functions
*******************************************************************************/
static gen_ca_pvr_resource *aui_gen_ca_pvr_resource_occupy(int rsc_type, UINT32 pvr_hnd)
{
    int i = -1;
    gen_ca_pvr_resource *rsc = NULL;

    aui_gen_ca_pvr_lock();
    if (GEN_CA_PVR_RSC_TYPE_REC == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_REC_NUM; ++i)
        {
            if (0 == g_gen_ca_pm.rec[i].pvr_hnd)
            {
                rsc = &g_gen_ca_pm.rec[i];
                rsc->pvr_hnd = pvr_hnd;
                break;
            }
        }
    }
    // Fixed cpptest bug
    //else if (GEN_CA_PVR_RSC_TYPE_PLAY == rsc_type)
    else
    {
        for (i = 0; i < GEN_CA_PVR_RSC_PLAY_NUM; ++i)
        {
            if (0 == g_gen_ca_pm.play[i].pvr_hnd)
            {
                rsc = &g_gen_ca_pm.play[i];
                rsc->pvr_hnd = pvr_hnd;
                break;
            }
        }
    }
    aui_gen_ca_pvr_un_lock();

    return rsc;
}

static gen_ca_pvr_resource *aui_gen_ca_pvr_resource_find(int rsc_type, UINT32 pvr_hnd)
{
    int i = -1;
    gen_ca_pvr_resource *rsc = NULL;

    aui_gen_ca_pvr_lock();
    if (GEN_CA_PVR_RSC_TYPE_REC == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_REC_NUM; ++i)
        {
            if (g_gen_ca_pm.rec[i].pvr_hnd == pvr_hnd)
            {
                rsc = &g_gen_ca_pm.rec[i];
                break;
            }
        }
    }
    else if (GEN_CA_PVR_RSC_TYPE_PLAY == rsc_type)
    {
        for (i = 0; i < GEN_CA_PVR_RSC_PLAY_NUM; ++i)
        {
            if (g_gen_ca_pm.play[i].pvr_hnd == pvr_hnd)
            {
                rsc = &g_gen_ca_pm.play[i];
                break;
            }
        }
    }
    aui_gen_ca_pvr_un_lock();

    return rsc;
}

static int aui_gen_ca_pvr_resource_release(gen_ca_pvr_resource *rsc)
{
    if(NULL == rsc)
    {
        return -1;
    }

    aui_gen_ca_pvr_lock();
    rsc->pvr_hnd = 0;
    rsc->dsc_fd = INVALID_DSC_FD;
    rsc->kl_fd = INVALID_DSC_FD;
    rsc->ali_pvr_fd = INVALID_ALI_PVR_FD;
    aui_gen_ca_pvr_un_lock();


    return 0;
}

// reset crypto for FTA TS recording
static int aui_gen_ca_pvr_reset_rec_config(UINT16 dmx_id,UINT8 index)
{
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);

    if(index < GEN_CA_PVR_RSC_REC_NUM)
    {
        pvr_de_enconfig[index].decrypt_fd = -1; // for FTA, no need to open decrypt device
        pvr_de_enconfig[index].encrypt_fd = -1;
        dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig[index]);    
    }
	
    return 0;
}

/*
static int aui_gen_ca_pvr_get_prog_ca_dsc_fd(void)
{
    return -1;
}
*/
static int aui_gen_ca_pvr_get_prog_ca_session_id(void)
{
    return 0;
}

// configure crypto for re-encrypt ts, and encrypt key
int aui_gen_ca_pvr_rec_config(pvr_crypto_general_param *rec_param)
{
    int ret = 0;
    int i = 0;
    int ca_format = 0;
    int session_id = 0;
    int crypt_kl_fd   = INVALID_KL_FD;
    int crypt_dsc_fd  = INVALID_DSC_FD;
    int decrypt_dsc_fd = INVALID_DSC_FD;
    int ali_pvr_fd = INVALID_ALI_PVR_FD;
    gen_ca_pvr_resource *rec_rsc = NULL;
    struct dmx_device *dmx = NULL;    
    unsigned char protect_key[GEN_CA_CRYPTO_KEY_LEVEL][GEN_CA_CRYPTO_KEY_LENGTH];
    int key_handle = -1;
	struct kl_config_key        cfg_key;	
    struct kl_gen_key           gen_key;	
    struct ca_create_kl_key     kl_key_info; 
    struct ali_pvr_reencrypt    pvr_enc_param;
    struct ca_pid               pid_info;
    UINT32 prog_id = 0;
    
    prog_id = pvr_r_get_channel_id(rec_param->pvr_hnd);
    decrypt_dsc_fd = aui_ca_pvr_get_ts_csa_device_id(prog_id);;//aui_gen_ca_pvr_get_prog_ca_dsc_fd();
    session_id = aui_gen_ca_pvr_get_prog_ca_session_id();

    if(NULL == rec_param)
    {
        AUI_ERR("No free GEN_CA input rec_param error!!!!!\n");
        ret =  -1;
        goto f_quit;        
    }
    
    rec_rsc = aui_gen_ca_pvr_resource_occupy(GEN_CA_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
    AUI_DBG("[%s][%d] rec_rsc:0x%08x,pvr_hnd:0x%08x\n",__FUNCTION__,__LINE__,rec_rsc,rec_param->pvr_hnd);
    if (NULL == rec_rsc)
    {
        AUI_ERR("No free GEN_CA PVR record resource\n");
        ret =  -1;
        goto f_quit;
    }

    ali_pvr_fd   = open(ALI_PVR_DEV_PATH,O_RDWR | O_CLOEXEC);
    AUI_DBG(" open  fd: %d\n", ali_pvr_fd);
    if(ali_pvr_fd < 0)
    {
        AUI_ERR("Invalid GEN_CA ali_pvr_fd  fd: %d\n", ali_pvr_fd);
        ret =   -1;
        goto f_quit;
    }   

    crypt_dsc_fd = open(DSC_DEV_PATH, O_RDWR);
    if (crypt_dsc_fd < 0)
    {
        AUI_ERR("Invalid GEN_CA crypt dsc fd: %d\n", crypt_dsc_fd);
        ret =   -1;
        goto f_quit;
    }
    
    /*********************************** KL  **********************************/
    crypt_kl_fd  = open(KL_DEV_PATH_1,O_RDWR); 
    if (crypt_kl_fd < 0)
    {
        AUI_ERR("Invalid GEN_CA crypt kl id: %d\n", crypt_kl_fd);
        ret =   -1;
        goto f_quit;
    }

    memset(&cfg_key, 0, sizeof(struct kl_config_key));//config key
	cfg_key.algo = KL_ALGO_TDES;//GEN_CA SPEC DISABLE kl aes.
    cfg_key.crypt_mode = KL_DECRYPT;
    cfg_key.ck_size = KL_CK_KEY128;
    cfg_key.level = GEN_CA_CRYPTO_KEY_LEVEL;
    cfg_key.ck_parity = KL_CK_PARITY_ODD_EVEN;
    ret = ioctl(crypt_kl_fd, KL_CONFIG_KEY, (void *)&cfg_key);
    if(ret < 0)
    {
        AUI_ERR("Invalid GEN_CA config kl error!\n");
        goto f_quit;            
    }
    
    //generate key    
	memset(&gen_key, 0, sizeof(struct kl_gen_key));
    aui_gen_ca_load_pk(protect_key,GEN_CA_CRYPTO_KEY_LEVEL);

    for(i = 0;i < GEN_CA_CRYPTO_KEY_LEVEL - 1;i++)
    {
	    memcpy(gen_key.pk[0], protect_key[0], GEN_CA_CRYPTO_KEY_LENGTH);
    }

    AUI_DUMP("OTP level 1 PK key\n", (char *)protect_key[0], GEN_CA_CRYPTO_KEY_LENGTH);
    AUI_DUMP("OTP level 2 PK key\n", (char *)protect_key[1], GEN_CA_CRYPTO_KEY_LENGTH);

	memcpy(gen_key.key_odd, const_odd_key , GEN_CA_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.key_even, const_even_key, GEN_CA_CRYPTO_KEY_LENGTH);
	ret = ioctl(crypt_kl_fd, KL_GEN_KEY, (void *)&gen_key);
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl KL_GEN_KEY fail !! ret:%d \n", ret);         
		goto f_quit;
	}
    /*********************************** DSC **********************************/
    //set format
    ca_format = CA_FORMAT_TS188;
	ret = ioctl(crypt_dsc_fd, CA_SET_FORMAT, &ca_format);
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl CA_SET_FORMAT fail !! ret:%d\n",ret); 
		goto f_quit;
	}
	//set KL key
	memset(&kl_key_info, 0, sizeof(struct ca_create_kl_key)); 
    kl_key_info.kl_fd = crypt_kl_fd;
    kl_key_info.algo  = CA_ALGO_AES;
    kl_key_info.chaining_mode = CA_MODE_ECB;
    kl_key_info.crypt_mode = CA_ENCRYPT;
    kl_key_info.residue_mode = CA_RESIDUE_CLEAR;    
    kl_key_info.parity = CA_PARITY_EVEN;
    kl_key_info.valid_mask = 0xFF;     
	ret = ioctl(crypt_dsc_fd, CA_CREATE_KL_KEY, (void *)&kl_key_info);	
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl CA_CREATE_KL_KEY fail !! ret:%d \n", ret);         
        goto f_quit;  
    }
    else
    {
        key_handle = ret;
    }
    
#if defined(MULTI_DESCRAMBLE)
    rec_rsc->session_id = session_id;    //back up session id
    if(session_id > (GEN_CA_PVR_RSC_TYPE_REC - 1))
    {
        AUI_ERR("Error: !! session id :%d \n", session_id);         
        goto f_quit;          
    }
#endif

	for(i = 0; i < rec_param->pid_num; i++)
    {
		memset(&pid_info, 0, sizeof(struct ca_pid));
		pid_info.key_handle = key_handle;
		pid_info.pid = rec_param->pid_list[i];
        pid_info.tsc_flag = TS_SCR_BIT_EVEN;//scarmble control bit //
		ret = ioctl(crypt_dsc_fd, CA_ADD_PID, (void *)&pid_info);
        
		if(ret < 0)
		{
			AUI_ERR("Error: ioctl CA_ADD_PIDS fail !! ret:%d \n", ret); 
			goto f_quit;  ;
		}
	}	

    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, rec_param->dmx_id);
    AUI_DBG("dmx:0x%x\n",(unsigned int)dmx);
    pvr_de_enconfig[(UINT8)session_id].decrypt_fd = decrypt_dsc_fd;
    pvr_de_enconfig[(UINT8)session_id].encrypt_fd = crypt_dsc_fd;    
    AUI_DBG("IO_SET_DEC_CONFIG sid:%d,0x%x\n",session_id,(unsigned int)(&pvr_de_enconfig[(UINT8)session_id]));
    dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvr_de_enconfig[(UINT8)session_id]);
    memset(&pvr_enc_param,0x00,sizeof(struct ali_pvr_reencrypt ));
    for(i = 0; i < rec_param->pid_num; i++)
    {
        pvr_enc_param.pid_list[i] = rec_param->pid_list[i]; 
        AUI_DBG("pid :%d\n",pvr_enc_param.pid_list[i]);
    }
    pvr_enc_param.pid_num = rec_param->pid_num;    
    pvr_enc_param.source_mode =  (1<<24);//TS mode
    pvr_enc_param.dsc_fd = crypt_dsc_fd;
    AUI_DBG("PVR_IO_START_REENCRYPT  :%d\n",PVR_IO_START_REENCRYPT);
    ret = ioctl(ali_pvr_fd,PVR_IO_SET_BLOCK_SIZE,(64*GEN_CA_TS_PACKAGE_SIZE));
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl PVR_IO_SET_BLOCK_SIZE fail !! ret:%d \n", ret);         
        goto f_quit;  
    }
    ret = ioctl(ali_pvr_fd,PVR_IO_START_REENCRYPT,&pvr_enc_param);
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl PVR_IO_START_BLOCK fail !! ret:%d \n", ret);         
        goto f_quit;  
    }    
    AUI_DBG("IO_SET_DEC_CONFIG  OK\n");
    rec_param->crypto_stream_hnd = crypt_dsc_fd;    
    rec_rsc->dsc_fd = crypt_dsc_fd;
    rec_rsc->kl_fd = crypt_kl_fd;   
    rec_rsc->ali_pvr_fd = ali_pvr_fd;
    AUI_DBG("rec config  OK:%d\n",ret);
    if(ret >= 0)
    {
        ret = 0;
    }
    return ret;    
f_quit:;
    if(ali_pvr_fd >= 0)
    {
        close(ali_pvr_fd);
        ali_pvr_fd = -1;
    }
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
    if(rec_rsc != NULL)
    {
        aui_gen_ca_pvr_resource_release(rec_rsc);
    }
	
    return ret;
}

// decrypt key and configure crypto for decrypt ts
int aui_gen_ca_pvr_playback_config(pvr_crypto_general_param *play_param,INT8 timeshift_flag)
{
    int i = 0;
    int ret = -1;
    int ca_format = 0;
    int decrypt_dsc_fd = INVALID_DSC_FD;
    int decrypt_kl_fd  = INVALID_DSC_FD;
    struct dmx_device *dmx =NULL;
    gen_ca_pvr_resource *play_rsc = NULL;
    int key_handle = -1;
	struct kl_config_key cfg_key;	
    struct kl_gen_key gen_key;	
    struct ca_create_kl_key kl_key_info; 
    unsigned char protect_key[GEN_CA_CRYPTO_KEY_LEVEL][GEN_CA_CRYPTO_KEY_LENGTH];
    struct ca_pid   pid_info;
    
    UNUSED(timeshift_flag);
    
    play_rsc = aui_gen_ca_pvr_resource_find(GEN_CA_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (NULL != play_rsc)
    {
        AUI_ERR("playback crypto stream resource is not released!\n");
        aui_gen_ca_pvr_playback_stop(play_param);
    }
    
    play_rsc = aui_gen_ca_pvr_resource_occupy(GEN_CA_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (NULL == play_rsc)
    {
        AUI_ERR("No free GEN_CA PVR play resource\n");
        ret =  -1;
        goto f_quit;
    }
    /*********************************** KL  **********************************/
    decrypt_kl_fd  = open(KL_DEV_PATH_1,O_RDWR); 
    if (decrypt_kl_fd < 0)
    {
        AUI_ERR("Invalid GEN_CA crypt kl id: %d\n", decrypt_kl_fd);
        ret =   -1;
        goto f_quit;
    }
    
    memset(&cfg_key, 0, sizeof(struct kl_config_key));//config key
	cfg_key.algo = KL_ALGO_TDES;//GEN_CA SPEC DISABLE kl aes.
    cfg_key.crypt_mode = KL_DECRYPT;
    cfg_key.ck_size = KL_CK_KEY128;
    cfg_key.level = GEN_CA_CRYPTO_KEY_LEVEL;
    cfg_key.ck_parity = KL_CK_PARITY_ODD_EVEN;
    ret = ioctl(decrypt_kl_fd, KL_CONFIG_KEY, (void *)&cfg_key);    
    if(ret < 0)
    {
        AUI_ERR("Invalid GEN_CA config kl error!\n");
        goto f_quit;            
    }   
    //generate key    
	memset(&gen_key, 0, sizeof(struct kl_gen_key));
    aui_gen_ca_load_pk(protect_key,GEN_CA_CRYPTO_KEY_LEVEL);

    for(i = 0;i < GEN_CA_CRYPTO_KEY_LEVEL - 1;i++)
    {
	    memcpy(gen_key.pk[i], protect_key[0], GEN_CA_CRYPTO_KEY_LENGTH);
    }

	memcpy(gen_key.key_odd, const_odd_key , GEN_CA_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.key_even, const_even_key, GEN_CA_CRYPTO_KEY_LENGTH);
	ret = ioctl(decrypt_kl_fd, KL_GEN_KEY, (void *)&gen_key);
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl KL_GEN_KEY fail !! ret:%d \n", ret);         
		goto f_quit;
	}    

    /*********************************** DSC **********************************/
    decrypt_dsc_fd = open(DSC_DEV_PATH, O_RDWR);
    //set format
    ca_format = CA_FORMAT_TS188;
	ret = ioctl(decrypt_dsc_fd, CA_SET_FORMAT, &ca_format);
	if(ret < 0)
	{
		AUI_ERR("Error: ioctl CA_SET_FORMAT fail !! ret:%d\n",ret); 
		goto f_quit;
	}
	//set KL key
	memset(&kl_key_info, 0, sizeof(struct ca_create_kl_key)); 
    kl_key_info.kl_fd = decrypt_kl_fd;
    kl_key_info.algo  = CA_ALGO_AES;
    kl_key_info.chaining_mode = CA_MODE_ECB;
    kl_key_info.crypt_mode = CA_DECRYPT;
    kl_key_info.residue_mode = CA_RESIDUE_CLEAR;    
    kl_key_info.parity = CA_PARITY_AUTO;
    kl_key_info.valid_mask = \
        CA_VALID_PARITY|CA_VALID_RESIDUE_MODE;     
	ret = ioctl(decrypt_dsc_fd, CA_CREATE_KL_KEY, (void *)&kl_key_info);	
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl CA_CREATE_KL_KEY fail !! ret:%d \n", ret);         
        goto f_quit;  
    }
    else
    {
        key_handle = ret;
    } 
    
	for(i = 0; i < play_param->pid_num; i++)
    {
		memset(&pid_info, 0, sizeof(struct ca_pid));
		pid_info.key_handle = key_handle;
		pid_info.pid        = play_param->pid_list[i];
		ret = ioctl(decrypt_dsc_fd, CA_ADD_PID, (void *)&pid_info);        
		if(ret < 0)
		{
			AUI_ERR("Error: ioctl CA_ADD_PIDS fail !! ret:%d \n", ret); 
			goto f_quit;  ;
		}
	}    
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id);
    if(dmx == NULL)
    {
        AUI_ERR("Error: get dmx handle error !! \n");         
        goto f_quit;
    }
    dmx_io_control(dmx, DMX_IO_SET_DEC_FD_HANDLE, (UINT32)&decrypt_dsc_fd);
    dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
    play_rsc->dsc_fd = decrypt_dsc_fd;
    play_rsc->kl_fd  = decrypt_kl_fd;
    play_param->crypto_stream_hnd = decrypt_dsc_fd;  
    AUI_DBG("playback  OK !! ret:%d,dmx_id:%d \n", ret,play_param->dmx_id);  
    if(ret > 0)
    {
        ret = 0;
    }
    AUI_DBG("playback OK end !! ret:%d \n", ret);

    return ret;
f_quit:;
    if(decrypt_dsc_fd >= 0)
    {
        close(decrypt_dsc_fd);
    }
    if(decrypt_kl_fd >= 0)
    {
        close(decrypt_kl_fd);
    }
    if(play_rsc != NULL)
    {
        aui_gen_ca_pvr_resource_release(play_rsc);
    }

    return ret;    
}

// when pvr stop record or timeshift to live, need to set dsc again
void aui_gen_ca_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id)
{
    UNUSED(dmx_id);
    UNUSED(stream_id);
}

// when stop record, need delete encrypt stream.
int aui_gen_ca_pvr_rec_stop(pvr_crypto_general_param *rec_param)
{
    int ret = 0;
    int crypt_kl_fd   = INVALID_KL_FD;
    int crypt_dsc_fd  = INVALID_DSC_FD;
    int ali_pvr_fd = INVALID_ALI_PVR_FD;
    UINT8  session_id = 0;
    gen_ca_pvr_resource *rec_rsc = NULL;
    
    if(NULL == rec_param)
    {
        ret =  -1;
        goto f_quit;
    }
    rec_rsc = aui_gen_ca_pvr_resource_find(GEN_CA_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
#ifdef MULTI_DESCRAMBLE    
    session_id = rec_rsc->session_id;
#endif
    if(rec_rsc != NULL)
    {
        crypt_kl_fd = rec_rsc->kl_fd;
        crypt_dsc_fd = rec_rsc->dsc_fd;
        ali_pvr_fd = rec_rsc->ali_pvr_fd;
        ret = ioctl(ali_pvr_fd,PVR_IO_STOP_REENCRYPT,crypt_dsc_fd);
        if(ret >= 0)
        {
            ret = 0;
        }
        else
        {
            AUI_ERR("PVR_IO_STOP_REENCRYPT:  ret %d\n", ret);
        }
        
        close(crypt_dsc_fd);
        close(crypt_kl_fd);
        close(ali_pvr_fd);
    }
    AUI_DBG("[%s][%d] rec_rsc:0x%08x,pvr_hnd:0x%08x\n",__FUNCTION__,__LINE__,rec_rsc,rec_param->pvr_hnd);
    aui_gen_ca_pvr_resource_release(rec_rsc);
    aui_gen_ca_pvr_reset_rec_config(rec_param->dmx_id,session_id);    
f_quit:;

    return ret;
}

int aui_gen_ca_pvr_playback_stop(pvr_crypto_general_param *play_param)
{
    int ret = -1;
    struct dmx_device *dmx = NULL;
    gen_ca_pvr_resource *play_rsc = NULL;
    
    if(NULL == play_param)
    {
        return ret;
    }

    play_rsc = aui_gen_ca_pvr_resource_find(GEN_CA_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
    if (NULL == play_rsc)
    {
        AUI_ERR("Cannot find play crypto stream resource, pvr handle: 0x%x\n", (unsigned int)play_param->pvr_hnd);
        return -1;
    }
    dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id);
    
    if(play_rsc->dsc_fd >= 0)
    {
        close(play_rsc->dsc_fd);
        play_rsc->dsc_fd = INVALID_DSC_FD;
    }
    if(play_rsc->kl_fd >= 0)
    {
        close(play_rsc->kl_fd);
        play_rsc->kl_fd = INVALID_KL_FD;
    }
    dmx_io_control(dmx, DMX_IO_SET_DEC_FD_HANDLE, (UINT32)(&(play_rsc->dsc_fd)));
    dmx_io_control(dmx, IO_SET_DEC_STATUS, 0);
    aui_gen_ca_pvr_resource_release(play_rsc);

    return 0;
}

int aui_gen_ca_pvr_generate_keys(pvr_crypto_key_param *key_param)
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

UINT16 aui_gen_ca_pvr_check_reencrypt_pids(UINT16 *pid_list, UINT16 pid_num)
{
    UINT16 valid_pid_num = 0;
    UINT16 i, j;

    for (i = 0; i < pid_num; i++) {
        AUI_DBG("0x%X ", pid_list[i]);
    }
    AUI_DBG("\n");

    for (i = 0; i < pid_num; i++) {
        if ((0 == pid_list[i]) || ((pid_list[i] & INVALID_PID) == INVALID_PID)) {
            continue;
        }

        for (j = 0; j < valid_pid_num; j++) {
            if (pid_list[i] == pid_list[j]) {
                pid_list[i] = INVALID_PID;
                break;
            }
        }

        if (j >= valid_pid_num) {
            pid_list[valid_pid_num++] = pid_list[i];
        }
    }

    AUI_DBG("%s() pid_list 2: ", __FUNCTION__);
    for (i = 0; i < valid_pid_num; i++) {
        AUI_DBG("0x%X ", pid_list[i]);
    }
    AUI_DBG("\n");

    return valid_pid_num;
}

UINT16 aui_gen_ca_pvr_set_reencrypt_pids(struct pvr_pid_info *pid_info,
									  UINT16 *pid_list, UINT16 pid_list_size)
{
    UINT16 pid_num = 0;
    UINT16 i;

    if ((pid_info->video_pid != INVALID_PID) && (pid_num < pid_list_size)) {
        AUI_DBG("re-encrypt video pid: 0x%X\n", pid_info->video_pid);
        pid_list[pid_num++] = pid_info->video_pid;
    }

    for (i = 0; (i < pid_info->audio_count) && (pid_num < pid_list_size); i++) {
        AUI_DBG("re-encrypt audio pid %d: 0x%X\n", i, pid_info->audio_pid[i]);
        pid_list[pid_num++] = pid_info->audio_pid[i];
    }

    for (i = 0; (i < pid_info->ttx_pid_count) && (pid_num < pid_list_size); i++) {
        AUI_DBG("re-encrypt ttx pid %d: 0x%X\n", i, pid_info->ttx_pids[i]);
        pid_list[pid_num++] = pid_info->ttx_pids[i];
    }

    for (i = 0; (i < pid_info->ttx_subt_pid_count) && (pid_num < pid_list_size); i++) {
        AUI_DBG("re-encrypt ttx_subt pid %d: 0x%X\n", i, pid_info->ttx_subt_pids[i]);
        pid_list[pid_num++] = pid_info->ttx_subt_pids[i];
    }

    for (i = 0; (i < pid_info->subt_pid_count) && (pid_num < pid_list_size); i++) {
        AUI_DBG("re-encrypt subt pid %d: 0x%X\n", i, pid_info->subt_pids[i]);
        pid_list[pid_num++] = pid_info->subt_pids[i];
    }
    
    return aui_gen_ca_pvr_check_reencrypt_pids(pid_list, pid_num);
}

/**
 * encrypt/decrypt data: default to (TDES, CBC)
 */
int aui_gen_ca_crypto_data(pvr_crypto_data_param *cp)
{
    int ret = 0;
    int ca_format = 0;
    int crypt_kl_fd   = INVALID_KL_FD;
    int crypt_dsc_fd  = INVALID_DSC_FD;
	struct kl_config_key cfg_key;	
    struct kl_gen_key gen_key;	
    struct ca_create_kl_key kl_key_info; 
    struct ca_dio_write_read crypto_io_wr;

    /*********************************** KL  **********************************/
    crypt_kl_fd  = open(KL_DEV_PATH_2,O_RDWR); //gen ca Kl2 is one level key ladder.
    if (crypt_kl_fd < 0)
    {
        AUI_ERR("Invalid GEN_CA crypt kl id: %d\n", crypt_kl_fd);
        ret =   -1;
        goto f_quit;
    }

    memset(&cfg_key, 0, sizeof(struct kl_config_key));//config key
	cfg_key.algo = KL_ALGO_TDES;//GEN_CA SPEC DISABLE kl aes.
    cfg_key.crypt_mode = KL_DECRYPT;
    cfg_key.ck_size = KL_CK_KEY128;
    cfg_key.level = 1;
    cfg_key.ck_parity = KL_CK_PARITY_ODD_EVEN;
    ret = ioctl(crypt_kl_fd, KL_CONFIG_KEY, (void *)&cfg_key);    
    if(ret < 0)
    {
        AUI_ERR("Invalid GEN_CA config kl error!\n");
        goto f_quit;            
    }
    
    //generate key    
	memset(&gen_key, 0, sizeof(struct kl_gen_key));
#if 0
    int i = 0;
    int count_level = 0;
    unsigned char protect_key[GEN_CA_CRYPTO_KEY_LEVEL][GEN_CA_CRYPTO_KEY_LENGTH];

    aui_gen_ca_load_pk(protect_key,GEN_CA_CRYPTO_KEY_LEVEL - 1);
    count_level = 0;
    for(i = 0;i <  GEN_CA_CRYPTO_KEY_LEVEL - 2 - 1;i++)
    {
	    memcpy(gen_key.pk[i], protect_key[i], GEN_CA_CRYPTO_KEY_LENGTH);
        count_level++;
    }
	memcpy(gen_key.pk[count_level],cp->key_ptr,GEN_CA_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.key_odd, const_odd_key , GEN_CA_CRYPTO_KEY_LENGTH);
	memcpy(gen_key.key_even, const_even_key, GEN_CA_CRYPTO_KEY_LENGTH);
#else    
    memcpy(gen_key.key_odd, cp->key_ptr , GEN_CA_CRYPTO_KEY_LENGTH);
    memcpy(gen_key.key_even, cp->key_ptr , GEN_CA_CRYPTO_KEY_LENGTH);
#endif
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
        AUI_ERR("Invalid GEN_CA crypt dsc fd: %d\n", crypt_dsc_fd);
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
//	if(ret < 0)
//	{
//		AUI_ERR("Error: ioctl KL_GEN_KEY fail !! ret:%d \n",ret);
//		goto f_quit;
//	}
	//set KL key
	memset(&kl_key_info, 0, sizeof(struct ca_create_kl_key)); 
    kl_key_info.kl_fd = crypt_kl_fd;
    kl_key_info.algo  = CA_ALGO_TDES;
    if(cp->encrypt)
    {
        AUI_DBG("CA_ENCRYPT here\n");
        kl_key_info.crypt_mode = CA_ENCRYPT;
    }
    else
    {
        AUI_DBG("CA_DECRYPT here\n");
        kl_key_info.crypt_mode = CA_DECRYPT;
    }
    kl_key_info.chaining_mode = CA_MODE_CBC;    
    kl_key_info.residue_mode = CA_RESIDUE_CLEAR;
    kl_key_info.parity = CA_PARITY_EVEN;
    memcpy(kl_key_info.iv_even ,cp->iv_ptr,GEN_CA_CRYPTO_IV_LENGTH);
    kl_key_info.valid_mask = 0xFF;    
	ret = ioctl(crypt_dsc_fd, CA_CREATE_KL_KEY, (void *)&kl_key_info);	
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl CA_CREATE_KL_KEY fail !! ret:%d \n", ret);         
        goto f_quit;  
    }
    
    memset(&crypto_io_wr, 0, sizeof(struct ca_dio_write_read));   
    
    crypto_io_wr.crypt_mode = kl_key_info.crypt_mode;
    crypto_io_wr.length = (ca_format == CA_FORMAT_RAW) ? cp->data_len:(cp->data_len/GEN_CA_TS_PACKAGE_SIZE);
    crypto_io_wr.input = (char *)cp->input;
    crypto_io_wr.output = (char *)cp->output;
    AUI_DBG("mode %d\n",crypto_io_wr.crypt_mode);
    AUI_DBG("lenth %d\n",crypto_io_wr.length);
    AUI_DBG("crypto_io_wr.input:0x%x\n",(unsigned int)(crypto_io_wr.input));
    AUI_DBG("crypto_io_wr.output:0x%x\n",(unsigned int)(crypto_io_wr.output));
	ret = ioctl(crypt_dsc_fd, CA_DIO_WRITE_READ, (void *)&crypto_io_wr);	
    if(ret < 0)	
    {       
        AUI_ERR("Error: ioctl CA_DIO_WRITE_READ fail !! ret:%d \n",ret);         
        goto f_quit;  
    }
    AUI_DBG("ret:%d  data OK\n",ret);

f_quit:;
    AUI_DBG("%s %d fd:%d data OK\n",__FUNCTION__,__LINE__,crypt_dsc_fd);
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
    AUI_DBG("%s %d OK\n",__FUNCTION__,__LINE__);

    return ret;
}

int aui_gen_ca_pvr_init(void)
{
    int i = -1;
	
    memset(&g_gen_ca_pm, 0, sizeof(gen_ca_pvr_mgr));
    if (OSAL_INVALID_ID == (g_gen_ca_pm.mutex_id = osal_mutex_create()))
    {
        AUI_ERR("Create mutex failed!\n");
        ASSERT(0);
        while(1);
        return -1;
    }

    aui_gen_ca_pvr_lock();
    for (i = 0; i < GEN_CA_PVR_RSC_REC_NUM; ++i)
    {
        g_gen_ca_pm.rec[i].pvr_hnd = 0;
        g_gen_ca_pm.rec[i].dsc_fd = INVALID_DSC_FD;
        g_gen_ca_pm.rec[i].kl_fd = INVALID_KL_FD;
    }

    for (i = 0; i < GEN_CA_PVR_RSC_PLAY_NUM; ++i)
    {
        g_gen_ca_pm.play[i].pvr_hnd = 0;
        g_gen_ca_pm.play[i].dsc_fd = INVALID_DSC_FD;
        g_gen_ca_pm.play[i].kl_fd = INVALID_KL_FD;        
    }
    aui_gen_ca_pvr_un_lock();

    return 0;
}


