
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_otp.h>
#include <bus/otp/otp.h>
/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/
/** 设备句柄 */
typedef struct aui_st_handle_otp
{
	/** dev handle */
	unsigned long ul_id;
}aui_handle_otp,*aui_p_handle_otp;
/****************************LOCAL VAR********************************************/

AUI_MODULE(OTP)

/** 模块的互斥信号量，此信号量用于锁内部的各个设备的锁，是二级锁的第一级 */
static OSAL_ID s_mod_mutex_id_otp=0;
/****************************LOCAL FUNC DECLEAR***********************************/



/****************************MODULE IMPLEMENT*************************************/


AUI_RTN_CODE aui_otp_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_OTP;
	return SUCCESS;
}

AUI_RTN_CODE aui_otp_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if((NULL==p_call_back_init))
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	s_mod_mutex_id_otp=osal_mutex_create();
	if(0==s_mod_mutex_id_otp)
	{
		aui_rtn(AUI_RTN_FAIL,"Create mutex failed.");
	}
	return p_call_back_init(pv_param);
}


AUI_RTN_CODE aui_otp_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if((NULL==p_call_back_init))
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	if(E_OK!=osal_mutex_delete(s_mod_mutex_id_otp))
	{
		aui_rtn(AUI_RTN_FAIL,"Delete mutex failed.");
	}
	return p_call_back_init(pv_param);
}

AUI_RTN_CODE aui_otp_read(unsigned long ul_addr,unsigned char *puc_data,unsigned long ul_data_len)
{
    if(NULL==puc_data)
    {
        aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
    }
    if((INT32)ul_data_len!=otp_read(ul_addr,puc_data,ul_data_len))
    {
        aui_rtn(AUI_RTN_FAIL, "Read OTP failed");
    }
    return SUCCESS;
}

AUI_RTN_CODE aui_otp_write(unsigned long ul_addr,unsigned char *puc_data,unsigned long ul_data_len)
{
	if(NULL==puc_data)
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	if((INT32)ul_data_len!=otp_write(puc_data,ul_addr,ul_data_len))
	{
		aui_rtn(AUI_RTN_FAIL, "Write OTP failed");
	}
	return SUCCESS;
}


