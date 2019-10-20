#include <basic_types.h>
#include <aui_ca_pvr.h>
#include "aui_dsc_inner.h"
#include <api/libpvr/lib_pvr.h>
#include "cas9_pvr.h"

#define __EK_AUI_CA_PVR__
#ifdef __EK_AUI_CA_PVR__
#define AUI_CA_PVR_PRINTF   libc_printf
#else
#define AUI_CA_PVR_PRINTF(...) do{} while(0)      
#endif

typedef struct program_info_t
{
	unsigned int	prog_id;
    unsigned char   used;
	aui_hdl			pvr_handler;
	aui_hdl 		ts_dsc_handle;
	aui_hdl 		pure_data_dsc_handle;
    unsigned int    pid_num;
	unsigned short	pid_list[PVR_MAX_PID_NUM];
}program_info, *p_program_info;

/*********************** static variable function **************************************/
static program_info st_program_info[3];

static aui_ca_pvr_callback s_ca_pvr_callback;

/*********************** static function **************************************/
/**
*   @brief      get stream id by handle
*   @author     raynard.wang
*   @date       2014-7-7
*   @param[in]  p_dsc_handle        DSC device handle
*   @return     stream id  or 0xFFFFFFFF
*/
unsigned int aui_ca_pvr_get_stream_id_from_handle(aui_hdl p_dsc_handle)
{
    AUI_CA_PVR_PRINTF("%s line:%d p_dsc_handle:0x%08x\r\n",__FUNCTION__,__LINE__,p_dsc_handle);
	if(p_dsc_handle !=NULL)
	{
		aui_handle_dsc *p_dsc = (aui_handle_dsc *)p_dsc_handle;
		return p_dsc->ul_stream_id;
	}
	return 0xFFFFFFFF;
}

unsigned int aui_ca_pvr_get_csa_device_id_from_handle(aui_hdl p_dsc_handle)
{
    AUI_CA_PVR_PRINTF("%s line:%d p_dsc_handle:0x%08x\r\n",__FUNCTION__,__LINE__,p_dsc_handle);
	if(p_dsc_handle !=NULL)
	{
		aui_handle_dsc *p_dsc = (aui_handle_dsc *)p_dsc_handle;
		return p_dsc->ul_sub_csa_dev_id;
	}
	return 0xFFFFFFFF;
}

/*********************** public API **************************************/

/**
*   @brief      init AUI CA PVR module
*   @author     raynard.wang
*   @date       2014-7-7
*   @return     AUI_RTN_SUCCESS or error code
*/
AUI_RTN_CODE aui_ca_pvr_init()
{
    AUI_CA_PVR_PRINTF("%s line:%d \r\n",__FUNCTION__,__LINE__);
	s_ca_pvr_callback.fp_pure_data_callback = NULL;
	s_ca_pvr_callback.fp_ts_callback = NULL;

	MEMSET(st_program_info,0,3*sizeof(program_info));
#if 0
    int i=0,j=0;
    for(j=0;j<3;j++){
        AUI_CA_PVR_PRINTF("st_program_info[%d].pid_list [address:0x%08x]\r\n",j,st_program_info[j].pid_list);
        for(i=0;i<PVR_MAX_PID_NUM;i++)
        {
            AUI_CA_PVR_PRINTF("st_program_info[%d].pid_list[%d]:[address:0x%08x] \r\n",j,i,&st_program_info[j].pid_list[i]);
        }
    }
#endif
    return AUI_RTN_SUCCESS;
}

/**
*   @brief      get TS DSC (virtual) device handle that the program is used.
*   @author     raynard.wang
*   @date       2014-7-7
*   @param[in]  st_aui_ca_pvr_callback          callback for get DSC_HANDLE by program id
*   @return     AUI_RTN_SUCCESS or error code
*/
unsigned short aui_ca_register_callback(aui_ca_pvr_callback *st_aui_ca_pvr_callback)
{
	if(st_aui_ca_pvr_callback == NULL)
	{
		return 1;
	}
	s_ca_pvr_callback.fp_pure_data_callback = st_aui_ca_pvr_callback->fp_pure_data_callback;
	s_ca_pvr_callback.fp_ts_callback		= st_aui_ca_pvr_callback->fp_ts_callback;
    AUI_CA_PVR_PRINTF("%s line:%d s_ca_pvr_callback.fp_pure_data_callback:0x%08x s_ca_pvr_callback.fp_ts_callback:0x%08x\r\n",
                            __FUNCTION__,__LINE__,
                            s_ca_pvr_callback.fp_pure_data_callback,
                            s_ca_pvr_callback.fp_ts_callback);
    
    return 0;
}

/**
*   @brief      get TS stream id
*   @author     raynard.wang
*   @date       2014-7-7
*   @param[in]  p_dsc_handle        DSC device handle
*   @return     stream id (TS_MODE) or 0xFFFFFFFF
*/
unsigned int aui_ca_pvr_get_ts_stream_id(unsigned int program_id)
{
	aui_hdl aui_dsc_handler;
	AUI_PRINTF("[EK CONAX]aui_ca_pvr_get_ts_stream_id(%d)\r\n.", program_id); 
	if(s_ca_pvr_callback.fp_ts_callback != NULL){
		if(0 == s_ca_pvr_callback.fp_ts_callback(program_id,&aui_dsc_handler)){
			AUI_PRINTF("[EK CONAX]aui_dsc_handler=0x%x\r\n.", aui_dsc_handler); 
			return aui_ca_pvr_get_stream_id_from_handle(aui_dsc_handler);
		}
	}

	return 0xFFFFFFFF;
}

unsigned int aui_ca_pvr_get_ts_csa_device_id(unsigned int program_id)
{
	aui_hdl aui_dsc_handler;
	AUI_PRINTF("[EK CONAX]%s (%d)\r\n.", __FUNCTION__,program_id); 
	if(s_ca_pvr_callback.fp_ts_callback != NULL){
		if(0 == s_ca_pvr_callback.fp_ts_callback(program_id,&aui_dsc_handler)){
			AUI_PRINTF("[EK CONAX]aui_dsc_handler=0x%x\r\n.", aui_dsc_handler); 
			return aui_ca_pvr_get_csa_device_id_from_handle(aui_dsc_handler);
		}
	}

	return 0xFFFFFFFF;
}

/**
*   @brief      get PURE_DATA stream id
*   @author     raynard.wang
*   @date       2014-7-7
*   @param[in]  p_dsc_handle          address of DSC device handle
*   @return     stream id (PURE_DATA_MODE) or 0xFFFFFFFF
*/
unsigned int aui_ca_pvr_get_pure_data_stream_id(unsigned int program_id)
{
	aui_hdl aui_dsc_handler;

	AUI_PRINTF("[EK CONAX]aui_ca_pvr_get_pure_data_stream_id(%d)\r\n.", program_id); 
	if(s_ca_pvr_callback.fp_pure_data_callback != NULL){
		if(0 == s_ca_pvr_callback.fp_pure_data_callback(program_id,&aui_dsc_handler)){
			AUI_PRINTF("[EK CONAX]aui_dsc_handler=0x%x\r\n.", aui_dsc_handler); 
			return aui_ca_pvr_get_stream_id_from_handle(aui_dsc_handler);
		}
	}

	return 0xFFFFFFFF;
}
 

UINT16 aui_ca_get_dsc_pid(unsigned short *pid_list, unsigned short pid_number)
{
    AUI_CA_PVR_PRINTF("%s line:%d pid_list:0x%08x   pid_number:%d \r\n",__FUNCTION__,__LINE__,
        pid_list,pid_number);
	memcpy(pid_list,&st_program_info[0].pid_list,pid_number*sizeof(unsigned short));
    AUI_CA_PVR_PRINTF("%s line:%d &st_program_info[0].pid_list:%08x pid_number:%d\r\n  ",
        __FUNCTION__,__LINE__,
        &st_program_info[0].pid_list,pid_number);
    return pid_number;
}

unsigned short aui_ca_set_dsc_pid_multi_des(unsigned short *pid_list, unsigned short pid_number,unsigned int prog_id)
{
	int i=0;

    AUI_CA_PVR_PRINTF("%s line:%d pid_list:0x%08x   pid_number:%d prog_id:%d\r\n",__FUNCTION__,__LINE__,
        pid_list,pid_number,prog_id);
	if(pid_number == 0)
	{
		for(i=0;i<3;i++){
			if((st_program_info[i].used == 1) && (st_program_info[i].prog_id == prog_id))
				memset(&st_program_info[i],0,sizeof(program_info));	
		}
		
		return 0;
	}

    for(i = 0;i< pid_number;i++)
    {
        AUI_CA_PVR_PRINTF("    pid[%d]:0x%x ",i,pid_list[i]);
    }
    
    AUI_CA_PVR_PRINTF("\r\n");
	for(i=0;i<3;i++){
		if(st_program_info[i].used == 0 ){
          AUI_CA_PVR_PRINTF("%s line:%d pid_list:0x%08x   pid_number:%d prog_id:%d st_program_info[%d].pid_list:0x%08x\r\n",__FUNCTION__,__LINE__,
        pid_list,pid_number,prog_id,i,st_program_info[i].pid_list);
			memcpy(st_program_info[i].pid_list,pid_list,pid_number*sizeof(unsigned short));
			st_program_info[i].prog_id = prog_id;
            st_program_info[i].pid_num = pid_number;
            st_program_info[i].used = 1;
			return pid_number;
		}
	}
	return 0;
}

unsigned short aui_ca_get_dsc_pid_multi_des(unsigned short *pid_list, unsigned short pid_number,unsigned int prog_id)
{
	int i=0;
    AUI_CA_PVR_PRINTF("%s line:%d pid_list:0x%08x   pid_number:%d prog_id:%d\r\n",__FUNCTION__,__LINE__,
        pid_list,pid_number,prog_id);
	for(i=0;i<3;i++){
		if((st_program_info[i].used == 1) && (st_program_info[i].prog_id == prog_id)){
			memcpy(pid_list,st_program_info[i].pid_list,st_program_info[i].pid_num*sizeof(unsigned short));
			return st_program_info[i].pid_num;
		}
	}
	return 0;
}

extern RET_CODE calculate_hmac(unsigned char *input, unsigned long length,
                      unsigned char *output, unsigned char *key);
AUI_RTN_CODE aui_ca_calculate_hmac(unsigned char *input, unsigned long length, unsigned char *output, unsigned char *key)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	if(RET_SUCCESS != calculate_hmac(input,length,output,key))
	{
		ret = AUI_RTN_FAIL;
	}

	return ret;
	
}

extern RET_CODE trng_generate_64bits( UINT8 *data);
extern RET_CODE trng_generate_byte( UINT8 *data );
AUI_RTN_CODE aui_ca_pvr_generate_keys(unsigned int key_len,unsigned int key_num,unsigned char *key_ptr)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	int i, key_bytes, loop_cnt1, loop_cnt2;
	if(key_ptr == NULL)
	{
		return AUI_RTN_FAIL;
	}
	key_bytes = (key_len + 7) / 8; 		// bytes for one key
	key_bytes = key_bytes * key_num;	// total bytes for all keys
	loop_cnt1 = key_bytes / 8;			// generate 64bits per loop
	loop_cnt2 = key_bytes % 8;			// generate 1Byte per loop
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
	return ret;
}


AUI_RTN_CODE aui_ca_pvr_crypto(unsigned char *out_data,unsigned char *in_data,unsigned int data_len,
							unsigned char *key_ptr,unsigned char *iv_ptr,unsigned int key_len,
							unsigned char crypto_mode,unsigned char encrypt)
{
    (void) crypto_mode;
	pvr_crypto_data_param cp;
	MEMSET(&cp, 0, sizeof(pvr_crypto_data_param));
    cp.output = out_data;
    cp.input = in_data;
    cp.data_len = data_len;
    cp.key_ptr = key_ptr;
    cp.iv_ptr = iv_ptr;
    cp.key_len =  key_len;          //bits
    cp.crypto_mode = PVR_CRYPTO_MODE_TDES_CBC;
    cp.encrypt = encrypt;
	if(0 == aui_cas9_crypto_data(&cp)){
		return AUI_RTN_SUCCESS;
	}

	return AUI_RTN_FAIL;
}


