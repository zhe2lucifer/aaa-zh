#include "aui_otp.h"
#include "alislotp.h"
#include "aui_common_priv.h"

AUI_MODULE(OTP)

AUI_RTN_CODE aui_otp_version_get(unsigned long *pul_version)
{
	if(NULL==pul_version)
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	*pul_version=AUI_MODULE_VERSION_NUM_OTP;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_otp_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if (p_call_back_init != NULL) {
		p_call_back_init(pv_param);
	}
	return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_otp_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	if((NULL==p_call_back_init))
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	return p_call_back_init(pv_param);
}

AUI_RTN_CODE aui_otp_read(unsigned long ul_addr,unsigned char *puc_data,unsigned long ul_data_len)
{
	if(NULL==puc_data)
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	if(0 != (unsigned int)alislotp_op_read(ul_addr,puc_data,ul_data_len))
	{
		aui_rtn(AUI_RTN_FAIL,"Read OTP failed");
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_otp_write(unsigned long ul_addr,unsigned char *puc_data,unsigned long ul_data_len)
{
	if(NULL==puc_data)
	{
		aui_rtn(AUI_RTN_EINVAL, "Inavlid parameter");
	}
	if(0 != (unsigned int)alislotp_op_write(ul_addr, puc_data, ul_data_len))
	{
		aui_rtn(AUI_RTN_FAIL,"Write OTP failed");
	}
	return AUI_RTN_SUCCESS;
}

