#include <sys_config.h>

#include <api/libci/ci_plus.h>
#include <openssl/fips_rand.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dsc/dsc.h>
#include <hld/dmx/dmx.h>
#include <openssl/aes.h>
#include <basic_types.h>
#include <osal/osal.h>
#include <bus/otp/otp.h>
#include <retcode.h>
#include <hal/hal_gpio.h>
#include <hld/crypto/crypto.h>
#include <api/libc/alloc.h>
#include <hld/dmx/dmx_dev.h>
#include "cas9_pvr.h"
#include "aui_ca_pvr.h"
#include <api/libpvr/lib_pvr.h>
#include <hld/pvr/pvr_remote.h>
#include "aui_common_priv.h"

AUI_MODULE(CA_PVR)

#define NEW_AS_DRIVER 1
#define CAS9_PVR_KEY_DEBUG_ENABLE		0
#define INVALID_DSC_SUB_DEV_ID			0xff
#define INVALID_DSC_STREAM_ID			0xff
#define INVALID_CE_KEY_POS				0xff

#ifdef CAS9_VSC
#define CAS9_M2M2_KEY_POS                KEY_0_2
#define CAS9_M2M2_KEY_OTP_ADDR            OTP_ADDESS_3
#define CAS9_PVR_KEY_POS                KEY_1_2            //first level key pos of PVR
#else
#define CAS9_M2M2_KEY_POS                KEY_0_1
#define CAS9_M2M2_KEY_OTP_ADDR            OTP_ADDESS_2
#define CAS9_PVR_KEY_POS                KEY_1_1
#endif

#define CAS9_PVR_KEY_OTP_ADDR			(ALI_C3701>sys_ic_get_chip_id() ? 0x70: 0x88)
#define CAS9_PVR_KEY_LEN				16		// measured in byte

#define CAS9_CRYPTO_KEY_LEVEL			TWO_LEVEL

#define CAS9_CRYPTO_KEY_LENGTH          16 //bytes
#define CAS9_CRYPTO_IV_LENGTH           16 //bytes
#define CAS9_TS_PACKAGE_SIZE           	188 //bytes
#define INVALID_CRYPTO_STREAM_ID		INVALID_DSC_STREAM_ID
#define INVALID_CRYPTO_STREAM_HANDLE	0xffffffff

#define CAS9_PVR_RSC_TYPE_REC			0
#define CAS9_PVR_RSC_TYPE_PLAY			1

#define CAS9_PVR_RSC_REC_NUM			PVR_MAX_REC_NUM
#define CAS9_PVR_RSC_PLAY_NUM			PVR_MAX_PLAY_NUM
#define KEY_LEVEL_2  1//3503A
typedef struct
{
	UINT32	pvr_hnd;
	UINT32	crypto_stream_hnd;	// crypto stream handle used by PVR to record or playback
	UINT32	crypto_stream_id;	// crypto stream id used by PVR to record or playback
	UINT32	crypto_key_pos;		// crypto stream key pos used by PVR to record or playback
	UINT32    crypto_dev_id; //crypto device id used by PVR to record or playback
    UINT8   session_id;         //back up prog session
} cas9_pvr_resource;

typedef struct
{
	ID					mutex_id;
	cas9_pvr_resource 	rec[CAS9_PVR_RSC_REC_NUM];
	cas9_pvr_resource 	play[CAS9_PVR_RSC_PLAY_NUM];
} cas9_pvr_mgr;

static UINT16 timeshift_stream_id = 0xff;     //save the stream id for do timeshift
/* CAS9 PVR Manager */
static cas9_pvr_mgr g_cpm;

// crypto related devices
static p_ce_device pvrCeDev = NULL;
static p_dsc_dev pvrDscDev  = NULL;
//static p_csa_dev pvrCsaDev  = NULL;
static UINT32 gPvrLevel2Pos = INVALID_CE_KEY_POS;
static UINT32 g_pvr_vsc_flag = 0;
//For new DSC to do less OTP read.
#ifndef _CAS9_VSC_API_ENABLE_
static BOOL  g_bEncryptPKLoad = FALSE;
#endif
UINT8 g_encrypted_pk[CAS9_PVR_KEY_LEN] = {0};
//
static const UINT8 const_ramdom_key[CAS9_PVR_KEY_LEN]={
		0x55,0x55,0x55,0x55,0xa5,0xa5,0xa5,0xa5,
		0xaa,0xaa,0xaa,0xaa,0x5a,0x5a,0x5a,0x5a,
};

// pvrDeEnconfig is used by other module, MUST defined as static or global
static DEEN_CONFIG pvrDeEnconfig[2];

#define CAS9_PVR_Lock()		osal_mutex_lock(g_cpm.mutex_id, OSAL_WAIT_FOREVER_TIME)
#define CAS9_PVR_UnLock()	osal_mutex_unlock(g_cpm.mutex_id)

#ifdef _CAS9_VSC_API_ENABLE_
extern RET_CODE ce_generate_vsc_rec_key(const UINT32 *key_pos, const UINT8 *en_key);
//Need to config the info to change key in see when VSC
/**
 * @brief This function is used to config the change key info to pvr remote in see.
 *
 * @param [IN] sub_device_id, AES device id
 * @param [IN] stream_id, stream id for encrypt/decrypt
 * @param [IN] key_pos, the output key pos
 * @param [IN] rec_play_type, key info for record:0 for play:1
 *
 * @retval RET_SUCCESS Operation success
 * @retval RET_FAILURE Operation failure
 */
extern RET_CODE ce_config_change_key_info(UINT32 sub_device_id ,UINT32 stream_id,UINT32 key_pos,UINT32 rec_play_type);
#endif

extern RET_CODE trng_generate_64bits( UINT8 *data);
extern RET_CODE trng_generate_byte( UINT8 *data );
int aui_pvr_generate_keys(pvr_crypto_key_param *key_param)
{
	int i, key_bytes, loop_cnt1, loop_cnt2;
	AUI_INFO("key_param->key_len:%d key_param->key_num:%d \r\n",key_param->key_len,key_param->key_num);
	key_bytes = (key_param->key_len + 7) / 8; 		// bytes for one key
	AUI_INFO("key_bytes:%d \r\n",key_bytes);
	key_bytes = key_bytes * key_param->key_num;		// total bytes for all keys
	AUI_INFO("key_bytes:%d \r\n",key_bytes);
	loop_cnt1 = key_bytes / 8;						// generate 64bits per loop
	loop_cnt2 = key_bytes % 8;						// generate 1Byte per loop
	AUI_INFO("generate keys for recording, loop_cnt: (%d, %d)\n", loop_cnt1, loop_cnt2);
	UINT8 *key_ptr = key_param->keys_ptr;
	for (i = 0; i < loop_cnt1; ++i)
	{
		trng_generate_64bits(key_ptr);
		key_ptr += 8;
	}

	for (i = 0; i < loop_cnt2; ++i)
	{
		trng_generate_byte(key_ptr);
		key_ptr++;
	}
	return 0;
}

UINT16 aui_pvr_check_reencrypt_pids(UINT16 *pid_list, UINT16 pid_num)
{
	UINT16 valid_pid_num = 0;
	UINT16 i, j;

	for (i = 0; i < pid_num; i++)
	{
		AUI_DBG("0x%X ", pid_list[i]);
	}

	for (i = 0; i < pid_num; i++)
	{
		if ((pid_list[i] == 0) || ((pid_list[i] & INVALID_PID) == INVALID_PID))
			continue;

		for (j = 0; j < valid_pid_num; j++)
		{
			if (pid_list[i] == pid_list[j])
			{
				pid_list[i] = INVALID_PID;
				break;
			}
		}

		if (j >= valid_pid_num)
			pid_list[valid_pid_num++] = pid_list[i];
	}

	AUI_DBG("%s() pid_list 2: ", __FUNCTION__);
	for (i = 0; i < valid_pid_num; i++)
	{
		AUI_DBG("0x%X ", pid_list[i]);
	}
	AUI_DBG("\n");

	return valid_pid_num;
}


/* set re-encrypt pids to @pid_list
 *	@pid_info		[in] : pids info
 *	@pid_list		[out]: pids to be re-encrypted
 *  @pid_list_size	[in] : max size of @pid_list
 *	@return : the valid pid number in @pid_list
 */
UINT16 aui_pvr_set_reencrypt_pids(struct pvr_pid_info *pid_info,
									  UINT16 *pid_list, UINT16 pid_list_size)
{
	UINT16 pid_num = 0;
	UINT8 i;
	if ((pid_info->video_pid != INVALID_PID) && (pid_num < pid_list_size))
	{
		pid_list[pid_num++] = pid_info->video_pid;
	}

	for (i = 0; (i < pid_info->audio_count) && (pid_num < pid_list_size); i++)
	{
		AUI_DBG("re-encrypt audio pid %d: 0x%X\n", i, pid_info->audio_pid[i]);
		pid_list[pid_num++] = pid_info->audio_pid[i];
	}

	for (i = 0; (i < pid_info->ttx_pid_count) && (pid_num < pid_list_size); i++)
	{
		AUI_DBG("re-encrypt ttx pid %d: 0x%X\n", i, pid_info->ttx_pids[i]);
		pid_list[pid_num++] = pid_info->ttx_pids[i];
	}

	for (i = 0; (i < pid_info->ttx_subt_pid_count) && (pid_num < pid_list_size); i++)
	{
		AUI_DBG("re-encrypt ttx_subt pid %d: 0x%X\n", i, pid_info->ttx_subt_pids[i]);
		pid_list[pid_num++] = pid_info->ttx_subt_pids[i];
	}

	for (i = 0; (i < pid_info->subt_pid_count) && (pid_num < pid_list_size); i++)
	{
		AUI_DBG("re-encrypt subt pid %d: 0x%X\n", i, pid_info->subt_pids[i]);
		pid_list[pid_num++] = pid_info->subt_pids[i];
	}

	return aui_pvr_check_reencrypt_pids(pid_list, pid_num);
}

unsigned char aui_pvr_get_record_mode(PVR_HANDLE handle)
{
	struct list_info rec_item_info;
	MEMSET(&rec_item_info,0,sizeof(struct list_info ));
	unsigned short index = pvr_get_index(handle);
	pvr_get_rl_info(index ,&rec_item_info);

	return rec_item_info.rec_special_mode;

}

static cas9_pvr_resource *cas9_pvr_resource_occupy(int rsc_type, UINT32 pvr_hnd)
{
	int i;
	cas9_pvr_resource *rsc = NULL;

	CAS9_PVR_Lock();
	switch(rsc_type) {
	case CAS9_PVR_RSC_TYPE_REC:
		for (i = 0; i < CAS9_PVR_RSC_REC_NUM; ++i)
		{
			if (g_cpm.rec[i].pvr_hnd == 0)
			{
				rsc = &g_cpm.rec[i];
				rsc->pvr_hnd = pvr_hnd;
				rsc->session_id = i;
				break;
			}
		}
		break;
	case CAS9_PVR_RSC_TYPE_PLAY:
		for (i = 0; i < CAS9_PVR_RSC_PLAY_NUM; ++i)
		{
			if (g_cpm.play[i].pvr_hnd == 0)
			{
				rsc = &g_cpm.play[i];
				rsc->pvr_hnd = pvr_hnd;
				break;
			}
		}
		break;
	}
	CAS9_PVR_UnLock();
	return rsc;
}

static cas9_pvr_resource *cas9_pvr_resource_find(int rsc_type, UINT32 pvr_hnd)
{
	int i;
	cas9_pvr_resource *rsc = NULL;

	CAS9_PVR_Lock();
	if (rsc_type == CAS9_PVR_RSC_TYPE_REC)
	{
		for (i = 0; i < CAS9_PVR_RSC_REC_NUM; ++i)
		{
			if (g_cpm.rec[i].pvr_hnd == pvr_hnd)
			{
				rsc = &g_cpm.rec[i];
				break;
			}
		}
	}
	else if (rsc_type == CAS9_PVR_RSC_TYPE_PLAY)
	{
		for (i = 0; i < CAS9_PVR_RSC_PLAY_NUM; ++i)
		{
			if (g_cpm.play[i].pvr_hnd == pvr_hnd)
			{
				rsc = &g_cpm.play[i];
				break;
			}
		}
	}
	CAS9_PVR_UnLock();
	return rsc;
}


static int cas9_pvr_resource_release(cas9_pvr_resource *rsc)
{
	CAS9_PVR_Lock();
	rsc->pvr_hnd = 0;
	rsc->crypto_stream_hnd 	= INVALID_CRYPTO_STREAM_HANDLE;
	rsc->crypto_stream_id 	= INVALID_CRYPTO_STREAM_ID;
	rsc->crypto_key_pos 	= INVALID_CE_KEY_POS;
	rsc->crypto_dev_id = INVALID_DSC_SUB_DEV_ID;
	rsc->session_id = 0;
	CAS9_PVR_UnLock();
	return 0;
}

/**
Lex:Set up the cw key in one time for new drive.
**/
UINT32  cas9_pvr_setup_cw_key_pos(UINT8 *key_ptr)
{
    p_ce_device p_ce_dev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    UINT32 key_pos = INVALID_CE_KEY_POS;
    UINT32 key_level = sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS);
    RET_CODE ret = RET_FAILURE;
    CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
    if(NULL == key_ptr)
    {
        AUI_ERR("key_ptr is null\n");
        return INVALID_CE_KEY_POS;
    }
    if(ALI_C3505 > sys_ic_get_chip_id())
    {
		MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
		if( THREE_LEVEL > key_level)
		{
			key_pos_param.ce_key_level = CAS9_CRYPTO_KEY_LEVEL;
		}
		else
		{
			key_pos_param.ce_key_level = THREE_LEVEL;
		}
		// bing add for cnx cardless
#ifdef _CAS9_VSC_API_ENABLE_
		key_pos_param.ce_key_level = THREE_LEVEL;
#endif
		if((THREE_LEVEL == key_pos_param.ce_key_level) && (INVALID_CE_KEY_POS == gPvrLevel2Pos))
		{
			AUI_ERR("find key pos 2 fail!!\n");
			return INVALID_CE_KEY_POS;	
		}
		key_pos_param.pos = INVALID_CE_KEY_POS;
		key_pos_param.root = CAS9_M2M2_KEY_POS;
		ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
		if (RET_SUCCESS != ret)
		{
			AUI_ERR("find free key pos fail!\n");
			return INVALID_CE_KEY_POS;
		}
		// generate encrypting key
		key_pos = (UINT32)key_pos_param.pos;
#ifdef _CAS9_VSC_API_ENABLE_
		ret=ce_generate_vsc_rec_key(&key_pos_param.pos, key_ptr);
#else
		ret = ce_generate_cw_key(key_ptr, AES, THREE_LEVEL==key_level ? gPvrLevel2Pos : CAS9_PVR_KEY_POS,key_pos);
#endif
		if (RET_SUCCESS != ret)
		{
			AUI_ERR("generate key fail!\n");
			ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
			return INVALID_CE_KEY_POS;
		}	
    }
    else  //Start of new flow
    {
		//1. Find  key slot. 
		MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
		key_pos_param.pos = ALI_INVALID_CRYPTO_KEY_POS;
		key_pos_param.root = CAS9_M2M2_KEY_POS; 
		key_pos_param.number= 1;
		key_pos_param.ce_key_level = key_level; //Must same to the key level

		ret = ce_ioctl(p_ce_dev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
		if (RET_SUCCESS != ret)
		{
             	     AUI_ERR("find free key pos fail!\n");
		     return RET_FAILURE;
		}
		key_pos = key_pos_param.pos;
    #ifndef _CAS9_VSC_API_ENABLE_
    		if(FALSE == g_bEncryptPKLoad)
    		{
    			otp_init(NULL);
			    MEMSET(g_encrypted_pk, 0, CAS9_PVR_KEY_LEN);
			    UINT32 i = 0;
			    for (i = 0; i < CAS9_PVR_KEY_LEN / 4; i++)
			    {
				    otp_read((CAS9_PVR_KEY_OTP_ADDR+i)*4, &g_encrypted_pk[i*4], 4);
			    }
#if (CAS9_PVR_KEY_DEBUG_ENABLE)
			    AUI_DBG("PVR key: ");
			    for (i = 0; i < CAS9_PVR_KEY_LEN; i++)
				    AUI_DBG("%02x ",g_encrypted_pk[i]);
			    AUI_DBG("\n");
#endif
			    g_bEncryptPKLoad = TRUE;
		}
		CE_NLEVEL_PARAM nlevel_param;
		//2. Generate the even key
		MEMSET(&nlevel_param, 0, sizeof(CE_NLEVEL_PARAM));
		nlevel_param.kl_index = ((key_pos >> 8)&0x1F);//ALI_KL_0;
		nlevel_param.otp_addr = CAS9_M2M2_KEY_OTP_ADDR;
		nlevel_param.algo = CE_SELECT_AES;
		nlevel_param.crypto_mode = CE_IS_DECRYPT;
		nlevel_param.pos = key_pos;
		nlevel_param.protecting_key_num = (key_level-1);
		//copy first stage pk to protecting_key[0-15]
		MEMCPY(nlevel_param.protecting_key, g_encrypted_pk, 16);
		//copy second stage pk to &protecting_key[16-32]
		MEMCPY(&(nlevel_param.protecting_key[16]), const_ramdom_key, 16);
		//copy even ck to content_key[0-15]
		MEMCPY(nlevel_param.content_key, key_ptr, 16);

		ret = ce_ioctl(p_ce_dev0, IO_CRYPT_GEN_NLEVEL_KEY, (UINT32)&nlevel_param);
		if (RET_SUCCESS != ret)
		{
			AUI_ERR("generate key fail!\n");
			ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
			return INVALID_CE_KEY_POS;
		}
#else
		//if vsc api enable. we need to use it. the nlevel key will be gen in see.
		ret=ce_generate_vsc_rec_key(&key_pos_param.pos, key_ptr);
		if(ret != RET_SUCCESS)
		{
			AUI_ERR("ce_generate_vsc_rec_key faild!\n");
		}
#endif

		
    }
    ret = key_pos;
    return ret;
}

// @return crypto stream handle
static UINT32 cas9_aes_key_map_ex(UINT32 key_pos, UINT16 *pid_list, UINT16 pid_num, UINT32 stream_id,INT8 index)
{
	AES_INIT_PARAM aes_param;
	KEY_PARAM key_param;
	RET_CODE ret;
	p_aes_dev pvrAesDev  = NULL;

	pvrAesDev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, index);
	MEMSET(&aes_param, 0, sizeof(AES_INIT_PARAM));
	aes_param.dma_mode = TS_MODE ;
	aes_param.key_from = KEY_FROM_CRYPTO;
	aes_param.key_mode = AES_128BITS_MODE ;
	aes_param.parity_mode = AUTO_PARITY_MODE0 ;
	aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	aes_param.scramble_control = 0;
	aes_param.stream_id = stream_id;
	aes_param.work_mode = WORK_MODE_IS_ECB;
	aes_param.cbc_cts_enable = 0;
	ret = aes_ioctl(pvrAesDev, IO_INIT_CMD, (UINT32)&aes_param);
	if (ret != RET_SUCCESS)
	{
	   AUI_ERR("AES IO_INIT_CMD fail\n");
	   return INVALID_CRYPTO_STREAM_HANDLE;
	}

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.ctr_counter = NULL ;
	key_param.init_vector = NULL ;
	key_param.key_length = 128;
	key_param.pid_len = pid_num;
	key_param.pid_list = pid_list;
	key_param.p_aes_iv_info = NULL ;
	key_param.stream_id = stream_id;
	key_param.force_mode = 1;
	key_param.pos = key_pos & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																Bit[7:0]  --->key pos*/
    key_param.kl_sel = (key_pos>>8)&0x0F;
	ret = aes_ioctl(pvrAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
	if (ret != RET_SUCCESS)
	{
		AUI_ERR("AES IO_CREAT_CRYPT_STREAM_CMD fail\n");
		return INVALID_CRYPTO_STREAM_HANDLE;
	}

	return key_param.handle;
}

// @return crypto stream handle and key pos
static UINT32 cas9_load_kreci_to_ce(pvr_crypto_general_param *param, UINT32 stream_id,INT8 index,UINT8 timeshift_flag)
{
    (void) timeshift_flag;
	UINT32 crypto_stream_hnd;
	UINT32 key_level = sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS);
    RET_CODE ret;
//Lex use new dirve method to get the cw key.
    if(sys_ic_get_chip_id()>=ALI_C3505)
   {
   	  param->crypto_key_pos  =  cas9_pvr_setup_cw_key_pos(param->keys_ptr);
	  if(INVALID_CE_KEY_POS == param->crypto_key_pos)
	  {
	  	AUI_ERR("%s cas9_pvr_setup_cw_key_pos faild \n",__FUNCTION__);
	  	return INVALID_CRYPTO_STREAM_HANDLE;
	  }
	  crypto_stream_hnd = cas9_aes_key_map_ex(param->crypto_key_pos, param->pid_list,
		param->pid_num, stream_id,index);
	  
    	if (INVALID_CRYPTO_STREAM_HANDLE == crypto_stream_hnd)
    	{
        	ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, param->crypto_key_pos);
        	param->crypto_key_pos = INVALID_CE_KEY_POS;
        	AUI_ERR("%s generate crypto_stream_hnd fail\n",__FUNCTION__);
    	}
	
	  param->crypto_key_first_pos= CAS9_M2M2_KEY_POS;//(THREE_LEVEL==key_level? g_pvr_level2pos: CAS9_PVR_KEY_POS);  
	  return crypto_stream_hnd ;
    }

	AUI_DBG("%s() key: ", __FUNCTION__);
	UINT32 i;
	for (i = 0; i < param->key_len / 8; ++i)
	{
		AUI_DBG("%02x ", param->keys_ptr[i]);
	}
	AUI_DBG("\n");

	param->crypto_key_pos = INVALID_CE_KEY_POS;

	CE_FOUND_FREE_POS_PARAM key_pos_param;
	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));

	if( key_level < THREE_LEVEL ){
		key_pos_param.ce_key_level = CAS9_CRYPTO_KEY_LEVEL;
	}else{
		key_pos_param.ce_key_level = THREE_LEVEL;
	}

	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root= CAS9_M2M2_KEY_POS;
	ret = ce_ioctl(pvrCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		AUI_DBG("find free key pos fail!\n");
		return INVALID_CRYPTO_STREAM_HANDLE;
	}

	AUI_DBG("Get ce pos(%d)\n",key_pos_param.pos); 	//vicky0715_TEST
	param->crypto_key_pos = key_pos_param.pos;

   	AUI_ERR("%s() keys_ptr(%x),key_level(%d),gLevel2Pos(%d),crypto_key_pos(%d)\n",__FUNCTION__,param->keys_ptr,key_level,gPvrLevel2Pos,param->crypto_key_pos);
#ifdef _CAS9_VSC_API_ENABLE_
		ret =  ce_generate_vsc_rec_key(&param->crypto_key_pos, param->keys_ptr);
#else
		ret = ce_generate_cw_key(param->keys_ptr, AES, key_level==THREE_LEVEL? gPvrLevel2Pos: CAS9_PVR_KEY_POS,
						(UINT8)param->crypto_key_pos);
#endif
	if (ret != RET_SUCCESS)
	{
		AUI_ERR("ce_generate_cw_key() fail\n");
		ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, param->crypto_key_pos);
		param->crypto_key_pos = INVALID_CE_KEY_POS;
		return INVALID_CRYPTO_STREAM_HANDLE;
	}

	crypto_stream_hnd = cas9_aes_key_map_ex(param->crypto_key_pos, param->pid_list,
											param->pid_num, stream_id,index);
	if (crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
	{
		ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, param->crypto_key_pos);
		param->crypto_key_pos = INVALID_CE_KEY_POS;
        	AUI_ERR("%s generate crypto_stream_hnd fail\n",__FUNCTION__);
	}

	param->crypto_key_first_pos=(key_level==THREE_LEVEL? gPvrLevel2Pos: CAS9_PVR_KEY_POS);
	return crypto_stream_hnd;
}

// @AES key map for puredata
static UINT32 cas9_aes_key_map_puredata(p_aes_dev pAesDev,UINT8 *iv, UINT32 key_pos,UINT32 stream_id)
{
    AES_INIT_PARAM aes_param;
    KEY_PARAM key_param;
    RET_CODE ret;
	DES_IV_INFO init_vector[1] = {
								{{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}
								};
    if(iv!=NULL)
    {
        MEMCPY(init_vector,iv,16);
    }
	MEMSET(&aes_param, 0, sizeof(AES_INIT_PARAM));
    aes_param.dma_mode = PURE_DATA_MODE;
    aes_param.key_from = KEY_FROM_CRYPTO;
    aes_param.key_mode = AES_128BITS_MODE ;
    aes_param.parity_mode = AUTO_PARITY_MODE0 ;
    aes_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
    aes_param.scramble_control = 0;
	aes_param.stream_id = stream_id;
    aes_param.work_mode = WORK_MODE_IS_CBC;
    aes_param.cbc_cts_enable = 0;
    ret = aes_ioctl(pAesDev, IO_INIT_CMD, (UINT32)&aes_param);
    if (ret != RET_SUCCESS)
	{
       AUI_ERR("AES IO_INIT_CMD fail\n");
	   return INVALID_CRYPTO_STREAM_HANDLE;
    }

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
    key_param.ctr_counter = NULL ;
    key_param.init_vector = NULL ;
    key_param.key_length = 128;
    key_param.pid_len = 1;
    key_param.pid_list = NULL;
    key_param.p_aes_iv_info =(AES_IV_INFO *)init_vector;
	key_param.stream_id = stream_id;
    key_param.force_mode = 1;
	key_param.pos = key_pos & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																Bit[7:0]  --->key pos*/
	key_param.kl_sel = (key_pos>>8) & 0xFF;
    ret = aes_ioctl(pAesDev ,IO_CREAT_CRYPT_STREAM_CMD , (UINT32)&key_param);
    if (ret != RET_SUCCESS)
    {
		AUI_ERR("AES IO_CREAT_CRYPT_STREAM_CMD fail\n");
		return INVALID_CRYPTO_STREAM_HANDLE;
    }
	return key_param.handle;
}



/* @return RET_SUCCESS or RET_FAILURE
// The function will calculate the OMAC for the input data.
// Key from crypto engine,Use the two level key ladder, the root key in OTP
// Default AES CBC mode, key length 128bit,
// input: input data
// r_data: a random data as the second level key input
// output: 128 bit output OMAC
// data_len: input data length
*/
RET_CODE aui_cas9_cal_omac(UINT8 *input, UINT8 *r_data, UINT8 *output, UINT32 data_len)
{
	UINT32 crypto_stream_hnd;
    RET_CODE ret;
    UINT32 stream_id,id_number;
    UINT32 crypto_key_pos=INVALID_CE_KEY_POS;
    UINT8 *temp_buff = NULL, *temp_buff2 = NULL;
    UINT8 *temp = NULL, *temp2=NULL;
    UINT32 residue = 0;
	DES_IV_INFO init_vector[1] = {
								{{0,0,0,0,0,0,0,0}, {0,0,0,0,0,0,0,0}}
								};
    p_aes_dev pAesDev  = NULL;
	UINT32 key_level = sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS);

	CE_FOUND_FREE_POS_PARAM key_pos_param;
	MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
	if( key_level < THREE_LEVEL ){
		key_pos_param.ce_key_level = CAS9_CRYPTO_KEY_LEVEL;
	}else{
		key_pos_param.ce_key_level = THREE_LEVEL;
	}
	key_pos_param.pos = INVALID_CE_KEY_POS;
	key_pos_param.root= CAS9_M2M2_KEY_POS;
	ret = ce_ioctl(pvrCeDev, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param);
	if (ret != RET_SUCCESS)
	{
		AUI_ERR("find free key pos fail!\n");
		return RET_FAILURE;
	}

	crypto_key_pos = key_pos_param.pos & 0xFF;
#ifdef _CAS9_VSC_API_ENABLE_
		ret =  ce_generate_vsc_rec_key(&crypto_key_pos, r_data);
#else
		ret = ce_generate_cw_key(r_data, AES, key_level==THREE_LEVEL? gPvrLevel2Pos : CAS9_PVR_KEY_POS,
						(UINT8)crypto_key_pos);
#endif
	if (ret != RET_SUCCESS)
	{
		AUI_ERR("ce_generate_cw_key() fail\n");
		ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, crypto_key_pos);
		return RET_FAILURE;
	}

    stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);
	if(stream_id == INVALID_CRYPTO_STREAM_ID)
	{
		AUI_ERR("get pure_data_id fail\n");
		return RET_FAILURE;
	}

	id_number = dsc_get_free_sub_device_id(AES);
	if(id_number == INVALID_DSC_SUB_DEV_ID)
	{
		AUI_ERR("get des dev id fail\n");
        dsc_set_stream_id_idle(stream_id);
		return RET_FAILURE;
	}

    pAesDev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, id_number);

	crypto_stream_hnd = cas9_aes_key_map_puredata(pAesDev,(UINT8 *)init_vector,crypto_key_pos,stream_id);
	if (crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
	{
		ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, crypto_key_pos);
	    dsc_set_stream_id_idle(stream_id);
	    dsc_set_sub_device_id_idle(AES, id_number);
        AUI_ERR("%s generate crypto_stream_hnd fail\n",__FUNCTION__);
        return RET_FAILURE;
	}

    //if the data length is not 16 byte align
    //the residue need fill with zero to 16 byte align, then calculate the OMAC
    residue = data_len%16;
    if(residue)
    {
        data_len +=(16-residue);
    }
    AUI_DBG("data length is:0x%x\n",data_len);

    temp_buff = (UINT8*)((UINT32)MALLOC(data_len+0xf));
    temp = (UINT8 *)(0xFFFFFFF8 & (UINT32)temp_buff);
    temp_buff2 = (UINT8*)((UINT32)MALLOC(data_len+0xf));
    temp2 = (UINT8 *)(0xFFFFFFF8 & (UINT32)temp_buff2);
    MEMSET(temp,0,data_len);
    MEMSET(temp2,0,data_len);
    //copy the input data into input buff
    MEMCPY(temp,input,data_len);
    ret = aes_encrypt(pAesDev, stream_id, temp, temp2, data_len);
    MEMCPY(output, (void *)(&temp2[data_len-16]),16);
	// release resource
	des_ioctl((p_des_dev)pAesDev, IO_DELETE_CRYPT_STREAM_CMD, crypto_stream_hnd);
	ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, (UINT32)crypto_key_pos);
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(AES, id_number);
    FREE(temp_buff);
    FREE(temp_buff2);
	return (ret == RET_SUCCESS) ? 0 : -1;
}
// configure crypto for re-encrypt ts, and encrypt key
int aui_cas9_pvr_rec_config(pvr_crypto_general_param *rec_param)
{
    UINT32 prog_id = 0;
    UINT8 decrypt_stream_id = 0;
    UINT8 session_id = 0;
	unsigned int csa_device_id = 0;
    p_csa_dev pvrCsaDev  = NULL;
    p_aes_dev pvrAesDev  = NULL;
	INT32  ret;
	unsigned long aes_id=INVALID_DSC_SUB_DEV_ID;
    struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, rec_param->dmx_id);
#if !(NEW_AS_DRIVER)
    DSC_PVR_KEY_PARAM pvr_key_param;
#else
    PVR_KEY_PARAM   rec_key_param = {0};
#endif
    prog_id = pvr_r_get_channel_id(rec_param->pvr_hnd);
	if(aui_pvr_get_record_mode(rec_param->pvr_hnd) == RSM_C0200A_MULTI_RE_ENCRYPTION){
    	decrypt_stream_id = 0;
	}else{
	//Get the decrypt stream id from the fp_ts_callback in aui_ca_pvr_callback.
	 //Wraning:Need to make it same as the stream id as the live play.
	    decrypt_stream_id = aui_ca_pvr_get_ts_stream_id(prog_id);
		AUI_INFO("%s() decrypt_stream_id get from aui_ca_pvr_get_ts_stream_id is (%d),please compare with the CA lib!!\n",__FUNCTION__,decrypt_stream_id);
	}

	//add session_id managment
    cas9_pvr_resource *rec_rsc = NULL;

	if (decrypt_stream_id == INVALID_CRYPTO_STREAM_ID)
	{
		AUI_INFO("Invalid CAS9 decrypt stream id: %d\n", decrypt_stream_id);
		//return -1;
	}
	else
	{
		AUI_INFO("%s() decrypt_stream_id is (%d)\n",__FUNCTION__,decrypt_stream_id);
	}
	rec_rsc = cas9_pvr_resource_occupy(CAS9_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
	if (rec_rsc == NULL)
	{
		AUI_ERR("No free CAS9 PVR record resource\n");
		return -1;
	}
	session_id = rec_rsc->session_id;    //back up session id
	rec_rsc->crypto_stream_id = dsc_get_free_stream_id(TS_MODE);
	//Get a free AES device.
	aes_id = dsc_get_free_sub_device_id(AES);
	if (INVALID_DSC_SUB_DEV_ID == aes_id)
	{
		AUI_ERR("aes_id get fail!!!\n");
		return -1;
	}
	rec_rsc->crypto_dev_id= aes_id;
	pvrAesDev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, aes_id);
	if(pvrAesDev==NULL)
	{
		AUI_ERR("pvrAesDev get fail!!!\n");
		return -1;
	}
	//Get the decrypt device id from the fp_ts_callback in aui_ca_pvr_callback.
	csa_device_id = aui_ca_pvr_get_ts_csa_device_id(prog_id);
	if((INVALID_DSC_SUB_DEV_ID == csa_device_id) || (0xFFFFFFFF == csa_device_id )) {
		AUI_ERR("Invalid CAS9 decrypt CSA device id: %d\n", csa_device_id);
		//return -1;
	} 
	else {
		pvrCsaDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, csa_device_id);
	}

	AUI_DBG("%s() get encrypt stream id (%d) and session ID (%d) aes_dev(0x%08x), csa_dev(0x%08x)\n",__FUNCTION__,rec_rsc->crypto_stream_id,session_id,pvrAesDev,pvrCsaDev);
	if (rec_rsc->crypto_stream_id == INVALID_CRYPTO_STREAM_ID)
	{
		AUI_ERR("No free crypto stream id\n");
		cas9_pvr_resource_release(rec_rsc);
		return -1;
	}
	//rec_rsc->crypto_stream_hnd = cas9_load_kreci_to_ce(rec_param, rec_rsc->crypto_stream_id,session_id,0);
	rec_rsc->crypto_stream_hnd = cas9_load_kreci_to_ce(rec_param, rec_rsc->crypto_stream_id,aes_id,0);
	if (rec_rsc->crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
	{
		AUI_ERR("%s() failed! encrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
			__FUNCTION__, rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id,
			rec_param->crypto_key_pos);
		dsc_set_stream_id_idle(rec_rsc->crypto_stream_id);
		cas9_pvr_resource_release(rec_rsc);
		return -1;
	}
	else
	{
		//AUI_INFO("%s() cas9_load_kreci_to_ce Success!!\n ",__FUNCTION__);
	}
	rec_rsc->crypto_key_pos = rec_param->crypto_key_pos;
    	timeshift_stream_id = rec_rsc->crypto_stream_id;
	AUI_DBG("%s()    pvr handle: 0x%X, pid num: %d, dmx_id: %d, decrypt stream id: %d\n"
		"\tencrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
		__FUNCTION__, rec_param->pvr_hnd, rec_param->pid_num, rec_param->dmx_id,
		decrypt_stream_id, rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id,
		rec_rsc->crypto_key_pos);

//Change key by pvr remote.
//We need to config the info to the pvr remote.
#if (NEW_AS_DRIVER) 
	  //Step1:Set the block size for encrypt.
	UINT32 block_size = (64*CAS9_TS_PACKAGE_SIZE);
    ret = pvr_rpc_ioctl(PVR_RPC_IO_SET_BLOCK_SIZE,(void*)block_size);
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl PVR_RPC_IO_SET_BLOCK_SIZE fail !! ret:%d \n", ret);         
        return -1;
    }
#ifndef _CAS9_VSC_API_ENABLE_ 
    struct PVR_BLOCK_ENC_PARAM input_enc ={0};
    //Config the enc parameter.	
     /*conax flag*/
    input_enc.reencrypt_type = 1; 
    /******/
    /*dsc mode*/
    input_enc.work_mode = WORK_MODE_IS_ECB;
    input_enc.source_mode = 1<<24;//TS mode
    input_enc.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE;
    input_enc.key_mode = EVEN_PARITY_MODE;
    input_enc.dsc_sub_device = AES;
    input_enc.sub_device_id   = aes_id;
    input_enc.stream_id = rec_rsc->crypto_stream_id;
    /*kl mode*/
    input_enc.root_key_pos = CAS9_M2M2_KEY_POS;
    input_enc.target_key_pos = rec_rsc->crypto_key_pos;    
    input_enc.kl_level = sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS);
    input_enc.kl_mode = 0;//0:AES ,1:TDES
    memcpy(&(input_enc.input_key[CAS9_CRYPTO_KEY_LENGTH*0]), g_encrypted_pk, CAS9_CRYPTO_KEY_LENGTH);
    memcpy(&(input_enc.input_key[CAS9_CRYPTO_KEY_LENGTH*1]), const_ramdom_key, CAS9_CRYPTO_KEY_LENGTH);
    if(rec_param->keys_ptr != NULL)
    {
        MEMCPY(&(input_enc.input_key[CAS9_CRYPTO_KEY_LENGTH*2]),rec_param->keys_ptr,CAS9_CRYPTO_KEY_LENGTH);//for first gen key.
    }
    else
    {
        AUI_ERR("rec key is NULL\n");
    } 
    UINT32 i;
    for(i = 0; (i < rec_param->pid_num) && (i < 32); i++)
    {
		input_enc.pid_list[i] = rec_param->pid_list[i] & 0x1FFF;
    }
    input_enc.pid_num = i;
   
    //Step2:Set the encrypt parameter to pvr remote.	
    ret = pvr_rpc_ioctl(PVR_RPC_IO_START_BLOCK,&input_enc);
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl PVR_IO_START_BLOCK fail !! ret:%d \n", ret);         
        return -1;
    }
#else
	ret = ce_config_change_key_info(aes_id,rec_rsc->crypto_stream_id,rec_rsc->crypto_key_pos,0);
	if(ret != RET_SUCCESS)
	{
		AUI_ERR("Error: ce_config_change_key_info fail !! ret:%d \n", ret); 
		return -1;
	}
#endif
	//Step3:Send the random key to pvr remote.
    rec_key_param.stream_id    = rec_rsc->crypto_stream_id;
    rec_key_param.pvr_key_length= ((rec_param->key_len + 7)/8);//bit transform to byte
    rec_key_param.valid_key_num= rec_param->key_num;
    rec_key_param.input_key = rec_param->keys_ptr;
    rec_key_param.qn_per_key = rec_param->qn_per_key;
    rec_key_param.quantum_size = QUANTUM_SIZE;
    AUI_DBG(" pvr_key_length %d, valid_key_num: %d \n", rec_key_param.pvr_key_length,  \
        rec_key_param.valid_key_num );
    ret = pvr_rpc_ioctl(PVR_RPC_IO_CAPTURE_PVR_KEY,&rec_key_param);
    if(ret < 0)	
    {
        AUI_ERR("Error: ioctl PVR_IO_CAPTURE_PVR_KEY fail !! ret:%d \n", ret);         
        return -1;  
    }
  	
#else // set dsc to change re-encrypted key automatically
    MEMSET(&pvr_key_param, 0, sizeof(DSC_PVR_KEY_PARAM));
    pvr_key_param.input_addr = (UINT32)rec_param->keys_ptr;
    pvr_key_param.pvr_user_key_pos = rec_param->crypto_key_pos;
    pvr_key_param.total_quantum_number = rec_param->qn_per_key;
    pvr_key_param.valid_key_num = rec_param->key_num;
    pvr_key_param.pvr_key_length = rec_param->key_len;
    pvr_key_param.stream_id = rec_rsc->crypto_stream_id;
    dsc_ioctl(pvr_dsc_dev, IO_DSC_SET_PVR_KEY_PARAM, (UINT32)&pvr_key_param);
#endif

	if((decrypt_stream_id != INVALID_CRYPTO_STREAM_ID) && (csa_device_id != INVALID_DSC_SUB_DEV_ID)) {
		pvrDeEnconfig[session_id].do_decrypt = 1;
	}
	else {
		pvrDeEnconfig[session_id].do_decrypt = 0;
	}
	pvrDeEnconfig[session_id].dec_dev = pvrCsaDev;
	pvrDeEnconfig[session_id].decrypt_mode = CSA;
	pvrDeEnconfig[session_id].dec_dmx_id = decrypt_stream_id;

	pvrDeEnconfig[session_id].do_encrypt = 1;
	pvrDeEnconfig[session_id].enc_dev = pvrAesDev;
	pvrDeEnconfig[session_id].encrypt_mode = AES;
	pvrDeEnconfig[session_id].enc_dmx_id = rec_rsc->crypto_stream_id;

	dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvrDeEnconfig[session_id]);

	/* set other return values */
	rec_param->crypto_stream_hnd = rec_rsc->crypto_stream_hnd;
	rec_param->crypto_stream_id = rec_rsc->crypto_stream_id;
	AUI_DBG("%s-Done-SID(%d),Stream- CSA(%d),AES(%d)\n",__FUNCTION__,session_id,pvrDeEnconfig[session_id].dec_dmx_id,pvrDeEnconfig[session_id].enc_dmx_id);
	return 0;
}

// decrypt key and configure crypto for decrypt ts
int aui_cas9_pvr_playback_config(pvr_crypto_general_param *play_param,INT8 timeshift_flag)
{
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, play_param->dmx_id);
	cas9_pvr_resource *play_rsc = NULL;
	struct dec_parse_param p_param;
	p_aes_dev decrypt_dev = NULL;
#if (NEW_AS_DRIVER)
    RET_CODE  ret  = 0;
#ifndef _CAS9_VSC_API_ENABLE_
    struct	PVR_BLOCK_ENC_PARAM  pvr_playback_key;
#endif
#endif

	play_rsc = cas9_pvr_resource_find(CAS9_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
	if (play_rsc != NULL)
	{
		AUI_ERR("playback crypto stream resource is not released!\n");
		aui_cas9_pvr_playback_stop(play_param);
	}

	play_rsc = cas9_pvr_resource_occupy(CAS9_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
	if (play_rsc == NULL)
	{
		AUI_ERR("No free CAS9 PVR play resource\n");
		return -1;
	}
	
	play_rsc->crypto_dev_id =  dsc_get_free_sub_device_id(AES);
	 if (play_rsc->crypto_dev_id != INVALID_DSC_SUB_DEV_ID)
	 {
	        decrypt_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, play_rsc->crypto_dev_id);
		if(decrypt_dev  == NULL)
		{
			AUI_ERR("get AES device\n");
			return -1;
		}
	    }
            else
            {
                AUI_ERR("no free AES device\n");
                return -1;
            }
//Wraning:This logic is for some IC which only have 4 stream--in timeshift it can reuse the same stream id.
//But be care that the timeshift_stream_id is same as the latest stream id for record.
    if(timeshift_flag == 0)
    {
        play_rsc->crypto_stream_id = dsc_get_free_stream_id(TS_MODE);
    	if (play_rsc->crypto_stream_id == INVALID_CRYPTO_STREAM_ID)
    	{
    		AUI_ERR("No free crypto stream id\n");
    		cas9_pvr_resource_release(play_rsc);
    		return -1;
    	}
    }
    else    //when do timeshift,re-encryption and playback use the same stream id
    {
        AUI_INFO("\t***%s timeshift reuse record stream id,timeshift_stream_id=%d\n",__FUNCTION__,timeshift_stream_id);
        play_rsc->crypto_stream_id = timeshift_stream_id;
    }

    play_rsc->crypto_stream_hnd = cas9_load_kreci_to_ce(play_param, play_rsc->crypto_stream_id,play_rsc->crypto_dev_id ,timeshift_flag);
	if (play_rsc->crypto_stream_hnd == INVALID_CRYPTO_STREAM_HANDLE)
	{
		AUI_ERR("%s() failed! decrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
			__FUNCTION__, play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id,
			play_rsc->crypto_key_pos);
		dsc_set_stream_id_idle(play_rsc->crypto_stream_id);
		cas9_pvr_resource_release(play_rsc);
		return -1;
	}
	play_rsc->crypto_key_pos = play_param->crypto_key_pos;

	AUI_DBG("%s()\n\tpvr handle: 0x%X, pid num: %d, dmx_id: %d\n"
		"\tdecrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
		__FUNCTION__, play_param->pvr_hnd, play_param->pid_num, play_param->dmx_id,
		play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id,
		play_rsc->crypto_key_pos);

	// configure crypto for playback
	MEMSET(&p_param, 0, sizeof(struct dec_parse_param));
	p_param.dec_dev = decrypt_dev;
	p_param.type = AES;
	dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&p_param);
	dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
	dsc_ioctl(pvrDscDev, IO_PARSE_DMX_ID_SET_CMD, play_rsc->crypto_stream_id);

#if (NEW_AS_DRIVER)
	#ifndef _CAS9_VSC_API_ENABLE_
	MEMSET(&pvr_playback_key,0x00,sizeof(struct PVR_BLOCK_ENC_PARAM));
	/*TS mode flag*/
	pvr_playback_key.reencrypt_type = 1;
	/*dsc config*/
	pvr_playback_key.work_mode = WORK_MODE_IS_ECB;
	pvr_playback_key.source_mode = 1<<24;//TS mode
	pvr_playback_key.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE;
	pvr_playback_key.key_mode = EVEN_PARITY_MODE;
	pvr_playback_key.dsc_sub_device = AES;
	pvr_playback_key.sub_device_id = play_rsc->crypto_dev_id;
	pvr_playback_key.stream_id = play_rsc->crypto_stream_id;
	/*kl config*/
	pvr_playback_key.kl_level = sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS);
	pvr_playback_key.root_key_pos = CAS9_M2M2_KEY_POS;
	pvr_playback_key.target_key_pos = play_rsc->crypto_key_pos;     
	pvr_playback_key.kl_mode = 0;//0:AES 1:TDES
	MEMCPY(&(pvr_playback_key.input_key[CAS9_CRYPTO_KEY_LENGTH*0]), g_encrypted_pk, CAS9_CRYPTO_KEY_LENGTH);
	MEMCPY(&(pvr_playback_key.input_key[CAS9_CRYPTO_KEY_LENGTH*1]), const_ramdom_key, CAS9_CRYPTO_KEY_LENGTH);
	
	if(play_param->keys_ptr != NULL)
	{
		MEMCPY(&(pvr_playback_key.input_key[CAS9_CRYPTO_KEY_LENGTH*2]),play_param->keys_ptr,CAS9_CRYPTO_KEY_LENGTH);//for first gen key.
	}
	else
	{
		AUI_ERR("rec key is NULL\n");
	}    
	ret = pvr_rpc_ioctl(PVR_RPC_IO_PVR_PLAYBCK_SET_KEY,&pvr_playback_key);  
	if(ret < 0)
	{
		AUI_ERR("Error: PVR_IO_SET_PLAYBACK_CA_PRAM fail!! ret:%d\n",ret); 
	}
#else
	ret = ce_config_change_key_info(play_rsc->crypto_dev_id ,play_rsc->crypto_stream_id,play_rsc->crypto_key_pos,1);
	if(ret != RET_SUCCESS)
	{
		AUI_ERR("Error: ce_config_change_key_info fail !! ret:%d \n", ret); 
		return -1;
	}
#endif

#endif
	/* set other return values */
	play_param->crypto_stream_hnd = play_rsc->crypto_stream_hnd;
	play_param->crypto_stream_id = play_rsc->crypto_stream_id;
	return 0;
}

//Warning:For scamble stream when pvr stop record or timeshift to live, need to set dsc again
//If not set  dsc the live play can't be work. This function is just a demo of setting it.
//Please make sure that the pCsaDev and stream id is same as the CA lib using!
//Pls refer to aui_dmx_data_path_set.
void aui_cas9_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id)
{
	//We can get the decrypt stream id from the fp_ts_callback in aui_ca_pvr_callback
	p_csa_dev pCsaDev = (p_csa_dev)dev_get_by_id(HLD_DEV_TYPE_CSA, 0);
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
	struct dec_parse_param param;

	AUI_DBG("%s() dmx_id: %d, CSA stream_id: %d\n", __FUNCTION__, dmx_id, stream_id);

	MEMSET(&param, 0, sizeof(struct dec_parse_param));
	param.dec_dev = pCsaDev;
	param.type = CSA;
    dmx_io_control(dmx, IO_SET_DEC_HANDLE, (UINT32)&param);
    dmx_io_control(dmx, IO_SET_DEC_STATUS, 1);
    dsc_ioctl(pvrDscDev, IO_PARSE_DMX_ID_SET_CMD, stream_id);
}


// reset crypto for FTA TS recording
static int cas9_pvr_reset_rec_config(UINT16 dmx_id,INT8 index)
{
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
	UINT8 u_index = (UINT8) index;
	pvrDeEnconfig[u_index].do_decrypt = 0; // for FTA, no need to open decrypt device
	pvrDeEnconfig[u_index].dec_dev = NULL;
	pvrDeEnconfig[u_index].do_encrypt = 0;
	pvrDeEnconfig[u_index].enc_dev = NULL;

	dmx_io_control(dmx, IO_SET_DEC_CONFIG, (UINT32)&pvrDeEnconfig[u_index]);
	return 0;
}

// when stop record, need delete encrypt stream.
int aui_cas9_pvr_rec_stop(pvr_crypto_general_param *rec_param)
{
    INT8 index = 0;
	RET_CODE ret = RET_SUCCESS;
	cas9_pvr_resource *rec_rsc = NULL;
    p_aes_dev pvrAesDev  = NULL;

	rec_rsc = cas9_pvr_resource_find(CAS9_PVR_RSC_TYPE_REC, rec_param->pvr_hnd);
	if (rec_rsc == NULL)
	{
		AUI_ERR("Cannot find record crypto stream resource, pvr handle: 0x%X\n", rec_param->pvr_hnd);
		return -1;
	}

    index = rec_rsc->session_id;
#if (NEW_AS_DRIVER)
	//Release the PVR change key by the record stream id.
	ret = pvr_rpc_ioctl(PVR_RPC_IO_RELEASE_PVR_KEY,(void*)rec_rsc->crypto_stream_id);
	if(RET_SUCCESS != ret)
	{
		AUI_ERR("Calling PVR_RPC_IO_RELEASE_PVR_KEY faild ret = %d\n",ret);
	}
	ret = pvr_rpc_ioctl(PVR_RPC_IO_FREE_BLOCK_EX,(void*)rec_rsc->crypto_stream_id);
	if(RET_SUCCESS != ret )
	{
		AUI_ERR("Calling PVR_RPC_IO_FREE_BLOCK_EX faild ret = %d\n",ret);
	}
#endif

    AUI_DBG("%s,session index=%d\n",__FUNCTION__,index);
    pvrAesDev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, rec_rsc->crypto_dev_id);
    if(pvrAesDev == NULL)
    {
    	AUI_ERR("get decrypt dev faild!");
		ret = RET_FAILURE;
    }

	AUI_DBG("%s() -------->pvr handle: 0x%X, dmx_id: %d\n"
		"\tencrypt stream handle: 0x%X, stream id: %d, key pos: %d\n",
		__FUNCTION__, rec_param->pvr_hnd, rec_param->dmx_id,
		rec_rsc->crypto_stream_hnd, rec_rsc->crypto_stream_id, rec_rsc->crypto_key_pos);

	if (rec_rsc->crypto_stream_hnd != INVALID_CRYPTO_STREAM_HANDLE)
		ret = aes_ioctl(pvrAesDev ,IO_DELETE_CRYPT_STREAM_CMD, rec_rsc->crypto_stream_hnd);
        if (rec_rsc->crypto_dev_id != INVALID_DSC_SUB_DEV_ID)
        {
            dsc_set_sub_device_id_idle(AES, rec_rsc->crypto_dev_id);
        }

	if (rec_rsc->crypto_key_pos != INVALID_CE_KEY_POS)
		ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, rec_rsc->crypto_key_pos);

	if (rec_rsc->crypto_stream_id != INVALID_CRYPTO_STREAM_ID)
	{
        dsc_ioctl(pvrDscDev ,IO_DSC_SET_PVR_KEY_IDLE, rec_rsc->crypto_stream_id);
        dsc_set_stream_id_idle(rec_rsc->crypto_stream_id);
		AUI_DBG("%s-IDLE stream_id(%d)\n",__FUNCTION__,rec_rsc->crypto_stream_id);
	}

	cas9_pvr_resource_release(rec_rsc);
	cas9_pvr_reset_rec_config(rec_param->dmx_id,index);

#if 0
	if ((g_cpm.play[0].pvr_hnd != NULL) &&
		(g_cpm.play[0].crypto_stream_hnd != INVALID_CRYPTO_STREAM_HANDLE) &&
		(g_cpm.play[0].crypto_stream_id != INVALID_CRYPTO_STREAM_ID))
	{
		cas9_set_dsc_for_playback(2, g_cpm.play[0].crypto_stream_id);
	}
#endif

	return (ret == RET_SUCCESS) ? 0 : -1;
}

int aui_cas9_pvr_playback_stop(pvr_crypto_general_param *play_param)
{
	RET_CODE ret = RET_SUCCESS;
	cas9_pvr_resource *play_rsc = NULL;
	p_aes_dev decrypt_dev = NULL;

	play_rsc = cas9_pvr_resource_find(CAS9_PVR_RSC_TYPE_PLAY, play_param->pvr_hnd);
	if (play_rsc == NULL)
	{
		AUI_ERR("Cannot find play crypto stream resource, pvr handle: 0x%X\n", play_param->pvr_hnd);
		return -1;
	}

	AUI_DBG("%s() ---------- pvr handle: 0x%X, dmx_id: %d\n"
		"\tdecrypt stream handle: 0x%X, stream id: %d, key pos: %d,device id:%d\n",
		__FUNCTION__, play_param->pvr_hnd, play_param->dmx_id,
		play_rsc->crypto_stream_hnd, play_rsc->crypto_stream_id, play_rsc->crypto_key_pos,play_rsc->crypto_dev_id);
	if(play_rsc->crypto_dev_id != INVALID_DSC_SUB_DEV_ID)
	{
		decrypt_dev = (p_aes_dev)dev_get_by_id(HLD_DEV_TYPE_AES, play_rsc->crypto_dev_id);
		if(decrypt_dev == NULL)
		{
			AUI_ERR("get decrypt dev faild!");
			return -1;
		}
	}

	if (play_rsc->crypto_stream_hnd != INVALID_CRYPTO_STREAM_HANDLE)
	{
#if (NEW_AS_DRIVER)
		struct PVR_BLOCK_ENC_PARAM	 pvr_playback_key;
		MEMSET(&pvr_playback_key,0x00,sizeof(struct PVR_BLOCK_ENC_PARAM));
		pvr_playback_key.kl_mode = 0xff;
		pvr_playback_key.root_key_pos = 0xff;
		ret = pvr_rpc_ioctl(PVR_RPC_IO_PVR_PLAYBCK_SET_KEY,&pvr_playback_key); //clear playback ca param  
#endif 
		ret = aes_ioctl(decrypt_dev ,IO_DELETE_CRYPT_STREAM_CMD, play_rsc->crypto_stream_hnd);
		dsc_set_sub_device_id_idle(AES, play_rsc->crypto_dev_id);
		play_rsc->crypto_dev_id = INVALID_DSC_SUB_DEV_ID;
		decrypt_dev = NULL;
	}
	if (play_rsc->crypto_key_pos != INVALID_CE_KEY_POS)
		ce_ioctl(pvrCeDev, IO_CRYPT_POS_SET_IDLE, play_rsc->crypto_key_pos);

	if (play_rsc->crypto_stream_id != INVALID_CRYPTO_STREAM_ID)
	{
		dsc_set_stream_id_idle(play_rsc->crypto_stream_id);
	}
	cas9_pvr_resource_release(play_rsc);
	return (ret == RET_SUCCESS) ? 0 : -1;
}

static int cas9_load_m2m2_key(void)
{
	p_ce_device pCeDev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	OTP_PARAM opt_info;

	MEMSET(&opt_info, 0, sizeof(OTP_PARAM));
	opt_info.otp_addr = CAS9_M2M2_KEY_OTP_ADDR;
	opt_info.otp_key_pos = CAS9_M2M2_KEY_POS;

	if (RET_SUCCESS != ce_key_load(pCeDev0, &opt_info))
	{
		AUI_DBG("load m2m2 key failed!");
		return -1;
	}
	return 0;
}

static int cas9_load_pk_to_ce(void)
{
	p_ce_device pCeDev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	CE_DATA_INFO Ce_data_info;
	UINT8 encrypted_pk[CAS9_PVR_KEY_LEN];
	int i;
    CE_FOUND_FREE_POS_PARAM key_pos_param={0,0,0,0};
    //Do nothing to new DSC.
     if(ALI_C3505 <= sys_ic_get_chip_id())
    {
    	return 0;
    }

	otp_init(NULL);
	MEMSET(encrypted_pk, 0, CAS9_PVR_KEY_LEN);
	for (i = 0; i < CAS9_PVR_KEY_LEN / 4; i++)
		otp_read((CAS9_PVR_KEY_OTP_ADDR+i)*4, &encrypted_pk[i*4], 4);

#if (CAS9_PVR_KEY_DEBUG_ENABLE)
	AUI_DBG("PVR key: ");
	for (i = 0; i < CAS9_PVR_KEY_LEN; i++)
		AUI_DBG("%02x ",encrypted_pk[i]);
	AUI_DBG("\n");
#endif

	MEMSET(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
	MEMCPY(Ce_data_info.data_info.crypt_data, encrypted_pk, CAS9_PVR_KEY_LEN);
	Ce_data_info.data_info.data_len 			= CAS9_PVR_KEY_LEN;	/* aes is 16 bytes des/tdes is 8 bytes*/
	Ce_data_info.des_aes_info.aes_or_des 		= CE_SELECT_AES ; 	/* select AES or DES module*/
	Ce_data_info.des_aes_info.crypt_mode 		= CE_IS_DECRYPT;
	Ce_data_info.des_aes_info.des_low_or_high	= 0;				/* for AES it should be LOW_ADDR */
	Ce_data_info.key_info.first_key_pos 		= CAS9_M2M2_KEY_POS ;
	Ce_data_info.key_info.hdcp_mode 			= NOT_FOR_HDCP;
	Ce_data_info.key_info.second_key_pos 		= CAS9_PVR_KEY_POS;
    /* get otp data from this address( 0x4D,0x51,0x55,or0x59) */
    Ce_data_info.otp_info.otp_addr                 = CAS9_M2M2_KEY_OTP_ADDR;
    /*the opt key will load to the position, it  make sure "otp_key_pos" = first_key_pos*/
    Ce_data_info.otp_info.otp_key_pos             = CAS9_M2M2_KEY_POS;
	if (ce_key_generate(pCeDev0, &Ce_data_info) != RET_SUCCESS)
	{
		AUI_ERR("generate PVR Key fail!\n");
		return -1;
	}

#ifdef OTP_NOT_FUSED
	CE_DEBUG_KEY_INFO param;
	param.len = 4 ;
	param.sel = CE_KEY_READ ;
	ce_ioctl(pCeDev0, IO_CRYPT_DEBUG_GET_KEY, &param);
#endif

    if(THREE_LEVEL == sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS))
    {
        if(INVALID_CE_KEY_POS == gPvrLevel2Pos)
        {
             //find level two pos
            MEMSET(&key_pos_param, 0, sizeof(CE_FOUND_FREE_POS_PARAM));
            key_pos_param.ce_key_level = TWO_LEVEL;
            key_pos_param.pos = INVALID_CE_KEY_POS;
            key_pos_param.root = CAS9_M2M2_KEY_POS;
            if (RET_SUCCESS != ce_ioctl(pCeDev0, IO_CRYPT_FOUND_FREE_POS, (UINT32)&key_pos_param))
            {
                AUI_ERR("Error: find free key pos fail!\n");
                return -1;
            }

            if((INVALID_CE_KEY_POS == key_pos_param.pos ) || (KEY_2_7 < key_pos_param.pos))
            {
                AUI_ERR("Error: find level 2 free key pos fail!\n");
                return -1;
            }
            gPvrLevel2Pos = key_pos_param.pos;
        }

        MEMSET(&Ce_data_info, 0, sizeof(CE_DATA_INFO));
        MEMCPY(Ce_data_info.data_info.crypt_data, const_ramdom_key, CAS9_PVR_KEY_LEN);
        Ce_data_info.data_info.data_len             = CAS9_PVR_KEY_LEN;    /* aes is 16 bytes des/tdes is 8 bytes*/
        Ce_data_info.des_aes_info.aes_or_des         = CE_SELECT_AES ;     /* select AES or DES module*/
        Ce_data_info.des_aes_info.crypt_mode         = CE_IS_DECRYPT;
        Ce_data_info.des_aes_info.des_low_or_high    = 0;                /* for AES it should be LOW_ADDR */
        Ce_data_info.key_info.first_key_pos         = CAS9_PVR_KEY_POS ;
        Ce_data_info.key_info.hdcp_mode             = NOT_FOR_HDCP;
        Ce_data_info.key_info.second_key_pos         = gPvrLevel2Pos;
        if (RET_SUCCESS != ce_generate_single_level_key(pCeDev0, &Ce_data_info))
        {
            AUI_ERR("generate PVR Key 2 fail!\n");
            return -1;
        }
        AUI_DBG("%s key_level(%d), gLevel2Pos(%d)\n", \
            __FUNCTION__,sys_ic_get_kl_key_mode(CAS9_M2M2_KEY_POS),gPvrLevel2Pos);
    }
    return 0;
}

extern void bl_generate_store_key_for_pvr(void);
int aui_cas9_pvr_init(void)
{
	int i;

	AUI_DBG("%s() load m2m2 key and PVR key to CE\n", __FUNCTION__);

    if (0 == g_pvr_vsc_flag)
    {
        //add for pvr generate the R1
        if(ALI_S3281 == sys_ic_get_chip_id())
        {
            bl_generate_store_key_for_pvr();
        }
    }

	cas9_load_m2m2_key();
	cas9_load_pk_to_ce();

	pvrCeDev = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
	pvrDscDev = (p_dsc_dev)dev_get_by_id(HLD_DEV_TYPE_DSC, 0);

	ASSERT(pvrCeDev && pvrDscDev);

	MEMSET(&g_cpm, 0, sizeof(cas9_pvr_mgr));
	if ((g_cpm.mutex_id = osal_mutex_create()) == OSAL_INVALID_ID)
	{
		AUI_ERR("Create mutex failed!\n");
		ASSERT(0);
		return -1;
	}

	CAS9_PVR_Lock();

	for (i = 0; i < CAS9_PVR_RSC_REC_NUM; ++i)
	{
		g_cpm.rec[i].pvr_hnd = 0;
		g_cpm.rec[i].crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
		g_cpm.rec[i].crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
		g_cpm.rec[i].crypto_key_pos = INVALID_CE_KEY_POS;
		g_cpm.rec[i].session_id = i;
	}

	for (i = 0; i < CAS9_PVR_RSC_PLAY_NUM; ++i)
	{
		g_cpm.play[i].pvr_hnd = 0;
		g_cpm.play[i].crypto_stream_hnd = INVALID_CRYPTO_STREAM_HANDLE;
		g_cpm.play[i].crypto_stream_id = INVALID_CRYPTO_STREAM_ID;
		g_cpm.play[i].crypto_key_pos = INVALID_CE_KEY_POS;
		g_cpm.rec[i].session_id = i;
	}

	CAS9_PVR_UnLock();

	return 0;
}

int aui_cas9_pvr_vsc_init(void)
{

	if(g_pvr_vsc_flag !=0)
	{
		return 0;
	}
	g_pvr_vsc_flag = 1;

	return 0;
}


/**
 * encrypt/decrypt data: default to (TDES, CBC)
 */
int aui_cas9_crypto_data(pvr_crypto_data_param *cp)
{
	p_ce_device pCeDev0 = NULL;
    p_des_dev pDesDev = NULL;
	UINT16 pid_list[1] = {0x1fff};
	DES_INIT_PARAM des_param ;
	KEY_PARAM key_param;
	static UINT32 key_pos = INVALID_CE_KEY_POS;
	UINT32 des_dev_id = INVALID_DSC_SUB_DEV_ID;
	UINT32 stream_id = INVALID_CRYPTO_STREAM_ID;
	RET_CODE ret;

    if(NULL == cp)
    {
        return -1;
    }
#if 0 // not encrypt/decrypt data, only for test
	MEMCPY(cp->output, cp->input, cp->data_len);
	return RET_SUCCESS;
#endif
	des_dev_id = dsc_get_free_sub_device_id(DES);
	if (des_dev_id == INVALID_DSC_SUB_DEV_ID)
	{
		AUI_ERR("dsc_get_free_sub_device_id() failed\n");
		return -1;
	}
	
	stream_id = dsc_get_free_stream_id(PURE_DATA_MODE);
	if (stream_id == INVALID_CRYPTO_STREAM_ID)
	{
		AUI_ERR("dsc_get_free_stream_id() failed\n");
		dsc_set_sub_device_id_idle(DES, des_dev_id);
		return -1;
	}

    pCeDev0 = (p_ce_device)dev_get_by_id(HLD_DEV_TYPE_CE, 0);
    pDesDev = (p_des_dev)dev_get_by_id(HLD_DEV_TYPE_DES, des_dev_id);
    if (!(pCeDev0 && pDesDev))
    {
        AUI_ERR("No valid device for crypto: (0x%X, 0x%X)\n", pCeDev0, pDesDev);
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }
    //Lex
    key_pos=  cas9_pvr_setup_cw_key_pos(cp->key_ptr);
    if(INVALID_CE_KEY_POS == key_pos)
    {
        AUI_ERR("generate key fail!\n");
        dsc_set_stream_id_idle(stream_id);
        dsc_set_sub_device_id_idle(DES, des_dev_id);
        return -1;
    }

	MEMSET(&des_param, 0, sizeof(DES_INIT_PARAM));
	des_param.dma_mode = PURE_DATA_MODE;
	des_param.key_from = KEY_FROM_CRYPTO;
	des_param.key_mode = TDES_ABA_MODE;
	des_param.parity_mode = EVEN_PARITY_MODE; //AUTO_PARITY_MODE0;
	des_param.residue_mode = RESIDUE_BLOCK_IS_NO_HANDLE ;
	des_param.scramble_control = 0;
	des_param.stream_id = stream_id;
	des_param.work_mode = WORK_MODE_IS_CBC;
    des_param.sub_module = TDES;
	des_param.cbc_cts_enable = 0;
	ret = des_ioctl(pDesDev, IO_INIT_CMD, (UINT32)&des_param);
	if (ret != RET_SUCCESS)
    {
        AUI_ERR("des_ioctl() IO_INIT_CMD failed\n");
		ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
		dsc_set_stream_id_idle(stream_id);
		dsc_set_sub_device_id_idle(DES, des_dev_id);
		return -1;
    }

	MEMSET(&key_param, 0, sizeof(KEY_PARAM));
	key_param.handle = INVALID_CRYPTO_STREAM_HANDLE;
	key_param.ctr_counter = NULL;
	key_param.init_vector = cp->iv_ptr;
	key_param.key_length = cp->key_len; // 128 bits ,or  192bits or 256 bits
	key_param.pid_len = 1; //not used
	key_param.pid_list = pid_list; //not used
	key_param.p_des_iv_info = (DES_IV_INFO *)(cp->iv_ptr);
	key_param.p_des_key_info = NULL;
	key_param.stream_id = stream_id; //0-3 for dmx id , 4-7 for pure data mode
	key_param.force_mode = 1;
	//key_param.pos = key_pos; //Descrambler can find the key which store in Crypto Engine by the key_pos
	key_param.pos = key_pos & 0xFF;  /* ul_key_pos_tmp   Bit[11:8] --->kl_sel for KL device
																Bit[7:0]  --->key pos*/
    key_param.kl_sel = (key_pos>>8)&0x0F;
    ret = des_ioctl(pDesDev, IO_CREAT_CRYPT_STREAM_CMD, (UINT32)&key_param);
    if (ret != RET_SUCCESS)
    {
        AUI_ERR("des_ioctl() IO_CREAT_CRYPT_STREAM_CMD failed\n");
		ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
		dsc_set_stream_id_idle(stream_id);
		dsc_set_sub_device_id_idle(DES, des_dev_id);
    	return -1;
    }

	// encrypt or decrypt data
	if (cp->encrypt)
		ret = des_encrypt(pDesDev, stream_id, cp->input, cp->output, cp->data_len);
	else
		ret = des_decrypt(pDesDev, stream_id, cp->input, cp->output, cp->data_len);

	if (ret != RET_SUCCESS)
	{
		AUI_ERR("%scrypt pure data failed! ret = %d\n", cp->encrypt ? "En" : "De", ret);
	}

	// release resource
	des_ioctl(pDesDev, IO_DELETE_CRYPT_STREAM_CMD, key_param.handle);
    ret = ce_ioctl(pCeDev0, IO_CRYPT_POS_SET_IDLE, (UINT32)key_pos);
    if(ret != RET_SUCCESS)
    {
		AUI_ERR(" ce_ioctl(p_ce_dev0, IO_CRYPT_POS_SET_IDLE, %d) faild!!! \n",key_pos);
    }
	dsc_set_stream_id_idle(stream_id);
	dsc_set_sub_device_id_idle(DES, des_dev_id);
	return (ret == RET_SUCCESS) ? 0 : -1;
}

