#ifndef _AUI_DSC_INNER_H
#define _AUI_DSC_INNER_H
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_dsc.h>
#include <hld/dsc/dsc.h>
#include <hld/pvr/pvr_remote.h>
#include <hld/dmx/dmx.h>
#include "aui_kl_inner.h"

/****************************LOCAL MACRO******************************************/
#ifndef INVALID_DSC_SUB_DEV_ID
#define INVALID_DSC_SUB_DEV_ID			  0xff
#endif

#define AUI_DSC_HDL_KEY_GROUP_CNT_MAX	(16)
#define AUI_DSC_HDL_KEY_PID_CNT_MAX   (64)
/****************************LOCAL TYPE*******************************************/
/** DSC???? */
typedef struct aui_st_dsc_key_hdl
{
	unsigned long ul_key_used_flag;
	aui_dsc_key_type key_from;
	/** number of PIDs */
	unsigned long ul_pid_cnt;	 
	/** The key handle generator from driver */
	unsigned long aul_key_param_handles[AUI_DSC_HDL_KEY_PID_CNT_MAX];
	unsigned short aus_pids_key_hdls[AUI_DSC_HDL_KEY_PID_CNT_MAX];
	
}aui_dsc_key_hdl;

/** DSC???? */
typedef struct aui_st_handle_dsc
{
	/** 设备内部数据结构 */
	aui_dev_priv_data dev_priv_data;
	/** 设备锁，AUI内部使用 */
	OSAL_ID dev_mutex_id;
	/** HLD层的DSC设备句柄 */
	p_dsc_dev p_dev_dsc;
	/** HLD层的DES设备句柄 */
	p_des_dev p_dev_des;
	/** DES子设备ID */
	unsigned long ul_sub_des_dev_id;
	/** HLD层的AES设备句柄 */
	p_aes_dev p_dev_aes;
	/** AES子设备ID */
	unsigned long ul_sub_aes_dev_id;
	/** HLD层的SHA设备句柄 */
	p_sha_dev p_dev_sha;
	/** SHA子设备ID */
	unsigned long ul_sub_sha_dev_id;
	/** HLD层的CSA设备句柄 */
	p_csa_dev p_dev_csa;
	/** CSA子设备ID */
	unsigned long ul_sub_csa_dev_id;
	/** DSC设备属性 */
	aui_attr_dsc attr_dsc;

	///** The stream id */
	unsigned long ul_stream_id;
	aui_dsc_key_hdl key_hdls[AUI_DSC_HDL_KEY_GROUP_CNT_MAX];
	/** The set stream id flag*/
    void *pv_live_dsc_hdl;
	/*following field for pvr block mode record*/
	int pvr_crypto_mode;/*pvr init flag,pvr_crypto_mode == 1 used pvr encrypt/decrypt TS data*/
	enum WORK_SUB_MODULE dsc_sub_device;//CSA , AES, TDES ...
	UINT32 key_handle;                  //key handle
	unsigned long ul_sub_dev_id;
	
	unsigned char dmx_src_mode_flag;//for block mode test in nestor.
	unsigned char iv_update_buffer[16];//buffer to store iv for updating iv
}aui_handle_dsc,*aui_p_handle_dsc;

AUI_RTN_CODE aui_dsc_set_playback(aui_hdl handle);
AUI_RTN_CODE aui_dsc_free_block_mode(aui_hdl handle);
int aui_dsc_get_stream_id(aui_hdl handle);
unsigned int aui_dsc_get_subdev_id(aui_hdl handle);


#endif
