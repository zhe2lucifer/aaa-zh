
#ifndef _AUI_DMX_PRIV_H

#define _AUI_DMX_PRIV_H

/*****************************Global Function List*****************************/
#include <aui_common.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define BLOCK_VOB_BUFFER_SIZE	0 //(2*188)

#define AUI_DMX_M2S_SRC_NORMAL 	 		0
#define AUI_DMX_M2S_SRC_CRYPT_BLK   	1
#define AUI_DMX_M2S_SRC_CRYPT_DYNAMIC_BLK 2 // for vmx ott dynamic block size

#define AUI_DMX_M2S_BUF_VALIDSIZE_SET   1
#define AUI_DMX_M2S_SRC_SET				2
#define AUI_DMX_M2S_BUF_REQ_SIZE        3
#define AUI_DMX_M2S_BUF_RET_SIZE		4
#define AUI_DMX_REC_MODE_GET			5
#define AUI_DMX_REC_BLOCK_SIZE_GET		6
#define AUI_DMX_M2S_BUF_VALIDSIZE_GET   7
#define AUI_DMX_M2S_SRC_GET				8

int aui_dmx_ioctl_priv(aui_hdl p_hdl_dmx, unsigned long ul_item, void *param);
int aui_dmx_ali_pvr_de_hdl_get(aui_hdl p_hdl_dmx, unsigned int *ali_pvr_de_hdl, unsigned int *block_size, 
	unsigned int *dsc_process_mode, unsigned int *mmap_addr, unsigned int *mmap_len, aui_hdl *ali_sl_hdl) ;

#ifdef __cplusplus
}
#endif

#endif

/* END OF FILE */

