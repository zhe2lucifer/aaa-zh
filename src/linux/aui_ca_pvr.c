#include <basic_types.h>
/* remove compile warning */
#ifdef SUPPORT_MPEG4_TEST
#undef SUPPORT_MPEG4_TEST
#endif
#include <osal/osal.h>

#include <hld/trng/trng.h>
#include "aui_dsc_common.h"
#include <api/libpvr/lib_pvr.h>
#include <api/libpvr/lib_pvr_eng.h>

#include <aui_ca_pvr.h>


AUI_MODULE(CA_PVR)

typedef struct program_info_t
{
    unsigned int    prog_id;
    unsigned char   used;
    aui_hdl         pvr_handler;
    aui_hdl         ts_dsc_handle;
    aui_hdl         pure_data_dsc_handle;
    unsigned int    pid_num;
    unsigned short  pid_list[PVR_MAX_PID_NUM];
}program_info, *p_program_info;

/*********************** static variable function **************************************/
static int pvr_special_mode = 0xff;
static program_info st_program_info[3];
static aui_ca_pvr_callback s_ca_pvr_callback;

/*********************** extern function **************************************/
extern unsigned int aui_dsc_get_subdev_id(aui_hdl handle);
extern int aui_dsc_get_stream_id(aui_hdl handle);
extern RET_CODE calculate_hmac(unsigned char *input, unsigned long length, unsigned char *output, unsigned char *key);
extern int aui_cas9_crypto_data(pvr_crypto_data_param *cp);
extern int aui_cas9_pvr_init();
extern int aui_c200a_pvr_init();
extern int aui_gen_ca_pvr_init();
//extern int aui_c200a_pvr_register_callback(aui_pvr_c200a_callback_fun callback_fun, void * user_data);

/*********************** static function **************************************/

/**
*    @brief         get stream id by handle
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     p_dsc_handle    DSC handle
*    @return        successful -- stream id, failed -- 0xFFFFFFFF
*    @note
*
*/
static unsigned int aui_ca_pvr_get_stream_id_from_handle(aui_hdl p_dsc_handle)
{
    AUI_DBG("%s line:%d p_dsc_handle:%d\r\n", __FUNCTION__, __LINE__, (int)p_dsc_handle);
    if(NULL != p_dsc_handle) {
        return 0;
    }
    return 0xFFFFFFFF;
}

/**
*    @brief         get CSA sub device id by handle
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     p_dsc_handle    DSC handle
*    @return        successful -- device id, failed -- 0xFFFFFFFF
*    @note
*
*/
static unsigned int aui_ca_pvr_get_csa_device_id_from_handle(aui_hdl p_dsc_handle)
{
    AUI_DBG("%s line:%d p_dsc_handle:%d\r\n",__FUNCTION__, __LINE__, (int)p_dsc_handle);
    if(NULL != p_dsc_handle) {
		return aui_get_dsc_crypt_fd(p_dsc_handle);
    }
    return 0xFFFFFFFF;
}

/*********************** public API **************************************/

/**
*    @brief         CA PVR init
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     
*    @return        always return AUI_RTN_SUCCESS
*    @note
*
*/
AUI_RTN_CODE aui_ca_pvr_init()
{
	static int ca_pvr_init = 0;
	aui_ca_pvr_config config;

    AUI_DBG("%s line:%d \r\n",__FUNCTION__,__LINE__);
	if(ca_pvr_init)
		return AUI_RTN_SUCCESS;
	memset(&config, 0x0, sizeof(config));

	ca_pvr_init = 1;
	config.special_mode = RSM_CAS9_MULTI_RE_ENCRYPTION;
	aui_ca_pvr_init_ext(&config);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_ca_pvr_init_ext(aui_ca_pvr_config* config)
{
	static int ex_init = 0;

	if(ex_init)
		return AUI_RTN_SUCCESS;
	
	if((config->special_mode < RSM_NONE) ||(config->special_mode > RSM_GEN_CA_MULTI_RE_ENCRYPTION)) {
		AUI_DBG("%s %s %d special_mode err!!!\n",__FILE__,__FUNCTION__,__LINE__);
		return AUI_RTN_FAIL;
	}
	ex_init = 1;
    s_ca_pvr_callback.fp_pure_data_callback = NULL;
    //s_ca_pvr_callback.fp_ts_callback = NULL;
    s_ca_pvr_callback.user_data = NULL;
    s_ca_pvr_callback.callback_fun = NULL;
	pvr_special_mode = config->special_mode;
    MEMSET(st_program_info, 0, 3 * sizeof(program_info));

	switch(config->special_mode) {
		case RSM_C0200A_MULTI_RE_ENCRYPTION:
			#if 0//00ndef AUI_LINUX
			//because nagra pvr resource is handled by upper layer.so remove this function used for init nagra pvr resource. 
			aui_c200a_pvr_init();
			#endif
			break;
        case RSM_GEN_CA_MULTI_RE_ENCRYPTION:
            aui_gen_ca_pvr_init();
            break;
		default:
			aui_cas9_pvr_init();
			break;
	} 
	
	return AUI_RTN_SUCCESS;
}

/**
*    @brief         register callback function for get DSC device handle that is used by live program
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     st_aui_ca_pvr_callback  callback for get DSC_HANDLE by program id
*    @return        always return AUI_RTN_SUCCESS
*    @note
*
*/
unsigned short aui_ca_register_callback(aui_ca_pvr_callback *st_aui_ca_pvr_callback)
{
    if(st_aui_ca_pvr_callback == NULL) {
        return 1;
    }
    s_ca_pvr_callback.fp_pure_data_callback = st_aui_ca_pvr_callback->fp_pure_data_callback;
    s_ca_pvr_callback.fp_ts_callback		= st_aui_ca_pvr_callback->fp_ts_callback;
    s_ca_pvr_callback.user_data = st_aui_ca_pvr_callback->user_data;
    s_ca_pvr_callback.callback_fun		= st_aui_ca_pvr_callback->callback_fun;
	
	if(pvr_special_mode == RSM_C0200A_MULTI_RE_ENCRYPTION) {
		//aui_c200a_pvr_register_callback(s_ca_pvr_callback.callback_fun,s_ca_pvr_callback.user_data);
	}
    AUI_DBG("%s line:%d s_ca_pvr_callback.fp_pure_data_callback:%d s_ca_pvr_callback.fp_ts_callback:%d\r\n",
                        __FUNCTION__,__LINE__,
                        (int)s_ca_pvr_callback.fp_pure_data_callback,
                        (int)s_ca_pvr_callback.fp_ts_callback);
    
    return AUI_RTN_SUCCESS;
}

/**
*    @brief         get TS crypto stream id that is used by live program
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     program_id  program id
*    @return        successful -- crypto stream id, failed -- 0xFFFFFFFF
*    @note
*
*/
unsigned int aui_ca_pvr_get_ts_stream_id(unsigned int program_id)
{
    aui_hdl aui_dsc_handler;
	AUI_DBG("aui_ca_pvr_get_ts_stream_id(%d)\r\n", program_id); 
    if (NULL != s_ca_pvr_callback.fp_ts_callback){
        if (0 == s_ca_pvr_callback.fp_ts_callback(program_id, &aui_dsc_handler)) {
            AUI_DBG("aui_dsc_handler=%d\r\n", (int)aui_dsc_handler); 
            return aui_ca_pvr_get_stream_id_from_handle(aui_dsc_handler);
        }
    }

    return 0xFFFFFFFF;
}

/**
*    @brief         get CSA sub device id that is used by live program
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     program_id  program id
*    @return        successful -- device id, failed -- 0xFFFFFFFF
*    @note
*
*/
unsigned int aui_ca_pvr_get_ts_csa_device_id(unsigned int program_id)
{
    aui_hdl aui_dsc_handler;
    AUI_DBG("%s (%d)\r\n", __FUNCTION__, program_id); 
    if (NULL != s_ca_pvr_callback.fp_ts_callback) {
        if (0 == s_ca_pvr_callback.fp_ts_callback(program_id,&aui_dsc_handler)) {
            AUI_DBG("aui_dsc_handler=%d\r\n", (int)aui_dsc_handler);
            return aui_ca_pvr_get_csa_device_id_from_handle(aui_dsc_handler);
        }
    }

    return 0xFFFFFFFF;
}

/**
*    @brief         get pure data stream id that is used by live program
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     program_id  program id
*    @return        successful -- device id, failed -- 0xFFFFFFFF
*    @note
*
*/
unsigned int aui_ca_pvr_get_pure_data_stream_id(unsigned int program_id)
{
    aui_hdl aui_dsc_handler;
    AUI_DBG("%s (%d)\r\n", __FUNCTION__, program_id); 
    if (NULL != s_ca_pvr_callback.fp_pure_data_callback) {
        if (0 == s_ca_pvr_callback.fp_pure_data_callback(program_id, &aui_dsc_handler)) {
			AUI_DBG("aui_dsc_handler=%d\r\n", (int)aui_dsc_handler); 
            return aui_ca_pvr_get_stream_id_from_handle(aui_dsc_handler);
        }
    }

    return 0xFFFFFFFF;
}

/**
*    @brief         get DSC pid from st_program_info[0]
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[out]    pid_list    pid list
*    @param[in]     pid_number  pid number of pid list
*    @return        always return pid number
*    @note
*
*/
UINT16 aui_ca_get_dsc_pid(unsigned short *pid_list, unsigned short pid_number)
{
    AUI_DBG("%s line:%d pid_list:%d   pid_number:%d \r\n",__FUNCTION__,__LINE__,
            (int)pid_list, (int)pid_number);
	MEMCPY(pid_list, &st_program_info[0].pid_list, pid_number*sizeof(unsigned short));
    AUI_DBG("%s line:%d &st_program_info[0].pid_list:%d pid_number:%d\r\n",
        __FUNCTION__, __LINE__, (int)(&st_program_info[0].pid_list), pid_number);
    return pid_number;
}

/**
*    @brief         set DSC PID for multi-program by program id
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[in]     pid_list    pid list
*    @param[in]     pid_number  pid number of pid list
*    @param[in]     prog_id     program id
*    @return        return pid number
*    @note
*
*/
unsigned short aui_ca_set_dsc_pid_multi_des(unsigned short *pid_list, unsigned short pid_number,unsigned int prog_id)
{
    int i = 0;

    if (pid_number == 0) {
        for(i=0;i<3;i++){
            if(st_program_info[i].used == 1 && st_program_info[i].prog_id == prog_id)
                MEMSET(&st_program_info[i],0,sizeof(program_info));	
        }
        return 0;
    }

    for (i = 0; i < pid_number; i ++) {
        AUI_DBG("    pid[%d]:0x%x ",i,pid_list[i]);
    }
    AUI_DBG("\r\n");

    for (i = 0; i < 3; i ++) {
        if(st_program_info[i].used == 0 ){
            MEMCPY(st_program_info[i].pid_list,pid_list,pid_number*sizeof(unsigned short));
            st_program_info[i].prog_id = prog_id;
            st_program_info[i].pid_num = pid_number;
            st_program_info[i].used = 1;
            return pid_number;
        }
    }
    return 0;
}

/**
*    @brief         get DSC PID for multi-program by program id
*    @author        Will.Qian
*    @date          2014-11-03
*    @param[out]    pid_list    pid list
*    @param[in]     pid_number  pid number of pid list
*    @param[in]     prog_id     program id
*    @return        return pid number
*    @note
*
*/
unsigned short aui_ca_get_dsc_pid_multi_des(unsigned short *pid_list, unsigned short pid_number,unsigned int prog_id)
{
    int i=0;

    UNUSED(pid_number);

    for (i = 0; i < 3; i ++) {
        if(st_program_info[i].used == 1 && st_program_info[i].prog_id == prog_id){
            MEMCPY(pid_list, st_program_info[i].pid_list, st_program_info[i].pid_num*sizeof(unsigned short));
            return st_program_info[i].pid_num;
        }
    }
    return 0;
}

/**
*    @brief         calculate HMAC value
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     input       address of input data
*    @param[in]     length      length of input data
*    @param[out]    output      address of HMAC value
*    @param[in]     key         key of HMAC
*    @return        successful -- AUI_RTN_SUCCESS, failed -- AUI_RTN_FAIL
*    @note          length of output is 32 byte.
*
*/
AUI_RTN_CODE aui_ca_calculate_hmac(unsigned char *input, unsigned long length, unsigned char *output, unsigned char *key)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

    if (RET_SUCCESS != calculate_hmac(input, length, output, key)) {
        ret = AUI_RTN_FAIL;
    }

    return ret;
}

/**
*    @brief         generate random key
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[in]     key_len     bit number of key
*    @param[in]     key_num     number of keys
*    @param[out]    key_ptr     address of HMAC value
*    @return        successful -- AUI_RTN_SUCCESS, failed -- AUI_RTN_FAIL
*	 @note          length of output is 32 byte.
*
*/
AUI_RTN_CODE aui_ca_pvr_generate_keys(unsigned int key_len,unsigned int key_num,unsigned char *key_ptr)
{
    int i = 0;
    int key_bytes = 0;
    int loop_cnt1 = 0; 
    int loop_cnt2 = 0;

    if (NULL == key_ptr) {
        return AUI_RTN_FAIL;
    }

    key_bytes = (key_len + 7) / 8;          /* bytes for one key */
    key_bytes = key_bytes * key_num;        /* total bytes for all keys */
    loop_cnt1 = key_bytes / 8;              /* generate 64bits per loop */
    loop_cnt2 = key_bytes % 8;              /* generate 1Byte per loop */

    for (i = 0; i < loop_cnt1; i ++) {
        trng_generate_64bits(key_ptr);
        key_ptr += 8;
    }

    for (i = 0; i < loop_cnt2; i ++) {
        trng_generate_byte(key_ptr);
        key_ptr ++;
    }

    return AUI_RTN_SUCCESS;
}

/**
*    @brief         encrypt/decrpt data 
*    @author        Will.Qian
*    @date          2014-11-10
*    @param[out]    out_data       	address of input data
*    @param[in]     in_data        	length of input data
*    @param[in]     data_len        length of data.
*    @param[in]     key_ptr        	address of key.
*    @param[in]     iv_ptr        	address of IV(Init vector).
*    @param[in]     key_len        	bit length of key.Such as 128 bit.
*    @param[in]     crypto_mode    	crypto/decrypt mode. (only support TDES_CBC now, TODO: support other mode)
*    @param[in]     encrypt	    	1:encrypt, 0:decrypt.
*    @return        successful -- AUI_RTN_SUCCESS, failed -- AUI_RTN_FAIL
*	 @note          Length of out_data is must be the same with in_data. data-len is the length of in_data.\n
*                   The length key an IV is the same with IV. key_len is the length of the key.
*
*/
AUI_RTN_CODE aui_ca_pvr_crypto(unsigned char *out_data,unsigned char *in_data,unsigned int data_len,
                            unsigned char *key_ptr,unsigned char *iv_ptr,unsigned int key_len,
                            unsigned char crypto_mode,unsigned char encrypt)
{  
    pvr_crypto_data_param cp;

    UNUSED(crypto_mode);
    
    MEMSET(&cp, 0, sizeof(pvr_crypto_data_param));
    cp.output = out_data;
    cp.input = in_data;
    cp.data_len = data_len;
    cp.key_ptr = key_ptr;
    cp.iv_ptr = iv_ptr;
    cp.key_len =  key_len;
    cp.crypto_mode = PVR_CRYPTO_MODE_TDES_CBC;
    cp.encrypt = encrypt;
    if(0 == aui_cas9_crypto_data(&cp)){
    	return AUI_RTN_SUCCESS;
    }

    return AUI_RTN_FAIL;
}

int aui_ca_get_pvr_special_mode(void)
{
	return pvr_special_mode;
}

