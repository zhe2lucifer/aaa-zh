/**  @file       aui_fs.c
*    @brief 	A middle layer about ali udi smc card.
*    @author 	seiya.cao
*    @date 		2013-8-22
*    @version 	1.0.0
*    @note 		ali corp. all rights reserved. 2013-2999 copyright (C)
*    			for many reasons,ali native fs system  is not POSIX compatiable.this file offer a middle layer implention oriented POSIX semantic.
*    			not completed.
*/
#include "aui_common_priv.h"

#include <sys_config.h>
#include <hld/smc/smc.h>
#include <osal/osal_timer.h>
#include <aui_smc.h>

AUI_MODULE(SMC)

//#define _AUI_SMC_DEBUG
#ifdef _AUI_SMC_DEBUG
	#define AUI_SMC_BAID(x...)        __asm__  __volatile__(".word 0x7000003f;nop")
#else
	#define AUI_SMC_BAID(x...)        do {}while(0)
#endif

#define HEADER_LEN	(5)
#define SW1SW2_LEN	(2)

#define PPS_MAX_LEN (6)
#define PPS_MIN_LEN (3)
#define PPSS ((char)0x0FF)

#define RECV_MAX_LENGTH (256)

typedef struct aui_smartcard_info
{
	struct aui_st_dev_priv_data data; /* struct for dev reg/unreg */
	int index;
	unsigned int state;
	unsigned int slot_state;/*0 --close ,1 --open*/
	aui_smc_param_t  ps_smcparams_temp;
	struct smc_device *dev;
	OSAL_ID sema;
	ID smc_timer_id;
	aui_smc_p_fun_cb callback;
	unsigned char iso_cmd[AUI_SMC_ISOCMD_LEN];

	int b_cold_rst;
}aui_smc_info_t;

typedef struct
{
	int msg_code;
	void *callback;
	int param1;
	int param2;
}aui_msg;

static unsigned char smc_iso_cmd[AUI_SMC_ISOCMD_LEN]= { 0x80, 0x46, 0x00, 0x00, 0x04, 0x07, 0x00, 0x00, 0x08 };
static unsigned char pps_buf[PPS_MAX_LEN] = {0};

static struct aui_smartcard_info *aui_smc_info = NULL;
AUI_RTN_CODE aui_smc_init(p_fun_cb psmc_cb_init)
{
	int ret = AUI_RTN_SUCCESS;
	if(( aui_smc_info = MALLOC(sizeof(aui_smc_info_t)))  ==NULL)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_ENOMEM, "Malloc error");
	}

	aui_smc_info->smc_timer_id = OSAL_INVALID_ID;
	aui_smc_info->index = -1;
	aui_smc_info->state = AUISMC_ERROR_OUT;
	aui_smc_info->slot_state = 0;
	aui_smc_info->dev = NULL;
	aui_smc_info->sema = OSAL_INVALID_ID;
	aui_smc_info->callback = NULL;
	MEMCPY(aui_smc_info->iso_cmd, smc_iso_cmd, AUI_SMC_ISOCMD_LEN);

	if(psmc_cb_init)
		return psmc_cb_init(NULL);
	return ret;
}


void auismc_tmo_handler(unsigned long param)
{
	if(param)
	{
		aui_msg msg = *(aui_msg *) param;

		if(msg.callback)
			((aui_smc_p_fun_cb)msg.callback)(msg.param1,msg.param2);
	}
}

static void  stop_auismc_timer(void)
{
	if(OSAL_INVALID_ID != aui_smc_info->smc_timer_id)
		osal_timer_delete(aui_smc_info->smc_timer_id);
	aui_smc_info->smc_timer_id = OSAL_INVALID_ID;
}

static void start_auifsmc_timer(unsigned int param)
{
	OSAL_T_CTIM     t_dalm;
	t_dalm.callback = auismc_tmo_handler;
	t_dalm.param = (UINT)param;
	t_dalm.type = TIMER_ALARM;
	t_dalm.time = 300;
	aui_smc_info->smc_timer_id = osal_timer_create(&t_dalm);
}

static void smartcard_event_callback(unsigned long param)
{
	static aui_msg msg;
	switch(param)
	{
		case 1:/*IN*/
		case 0:/*OUT*/
			msg.msg_code = 0;
			msg.callback = (void *)aui_smc_info->callback;
			msg.param1 = aui_smc_info->index;
			msg.param2 = param;
			stop_auismc_timer();
			start_auifsmc_timer((unsigned int)&msg);
			break;
		default:
			break;
	}
}

AUI_RTN_CODE aui_smc_open(aui_smc_attr *psmc_attr,aui_hdl *pp_smc_handle)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code;

	if(!psmc_attr||!pp_smc_handle)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(((psmc_attr->ul_smc_id>=AUI_MAX_SMARTCARD_NUM)||\
		((int)psmc_attr->ul_smc_id<0))||\
		(NULL == psmc_attr->p_fn_smc_cb))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	aui_smc_info->dev = (struct smc_device *)dev_get_by_id(HLD_DEV_TYPE_SMC, 0);

	if(NULL == aui_smc_info->dev )
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	aui_smc_info->callback = psmc_attr->p_fn_smc_cb;
	aui_smc_info->index = psmc_attr->ul_smc_id;
	aui_smc_info->sema=osal_semaphore_create(1);
	aui_smc_info->state = AUISMC_ERROR_OUT;

	error_code = smc_open(aui_smc_info->dev, smartcard_event_callback);

	if(error_code != 0)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	aui_smc_info->slot_state = 1;
	osal_task_sleep(200);

	*pp_smc_handle = (aui_hdl)aui_smc_info;

   	/* set default value, so the smc work without setting parameters */
	/* -1 means this field is auto-detected in the driver and can be ignored*/
	aui_smc_info->ps_smcparams_temp.m_nETU = 372;
	aui_smc_info->ps_smcparams_temp.m_n_baud_rate = -1;
	aui_smc_info->ps_smcparams_temp.m_n_frequency = -1;
	aui_smc_info->ps_smcparams_temp.m_e_standard = AUI_SMC_STANDARD_ISO;
	aui_smc_info->ps_smcparams_temp.m_e_protocol = AUI_SMC_PROTOCOL_UNKNOWN;
	aui_smc_info->ps_smcparams_temp.m_e_stop_bit = -1;
	aui_smc_info->ps_smcparams_temp.m_e_check_bit = -1;

	aui_smc_info->data.dev_idx = psmc_attr->ul_smc_id;
	(void)aui_dev_reg(AUI_MODULE_SMC, aui_smc_info);

	return ret;
}

AUI_RTN_CODE aui_smc_param_get(aui_hdl smc_handle,aui_smc_param_t * p_smc_param)
{
	int ret = AUI_RTN_SUCCESS;
	unsigned int protocol;
	unsigned int clk;

	if(((void*) aui_smc_info != (void*)smc_handle)||(!p_smc_param))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	
	ret = smc_io_control(aui_smc_info->dev, SMC_DRIVER_GET_PROTOCOL,
		(unsigned int)&protocol);
	if (ret)
		return AUI_RTN_FAIL;

	if (protocol == 0){
		aui_smc_info->ps_smcparams_temp.m_e_protocol = AUI_SMC_PROTOCOL_T0;
	} else if (protocol == 1){
		// AUI_SMC_PROTOCOL_T1_ISO is T=1 raw data mode.
		if (aui_smc_info->ps_smcparams_temp.m_e_protocol != EM_AUISMC_PROTOCOL_T1
				|| aui_smc_info->ps_smcparams_temp.m_e_protocol != AUI_SMC_PROTOCOL_T1_ISO){
			aui_smc_info->ps_smcparams_temp.m_e_protocol = EM_AUISMC_PROTOCOL_T1;
		}
	} else if (protocol == 14){
		aui_smc_info->ps_smcparams_temp.m_e_protocol = AUI_SMC_PROTOCOL_T14;
	}

    // Get clock frequency in Hz: SMC_CMD_GET_WCLK
    if (smc_io_control(aui_smc_info->dev, SMC_DRIVER_GET_WCLK, (unsigned int)&clk)) {
        AUI_DBG("SMC_DRIVER_GET_WCLK error\n");
        return AUI_RTN_FAIL;
    } else {
        aui_smc_info->ps_smcparams_temp.m_n_frequency = clk;
    }

	memcpy(p_smc_param, &aui_smc_info->ps_smcparams_temp,sizeof(aui_smc_param_t));

	return ret;
}

AUI_RTN_CODE aui_smc_param_set(aui_hdl smc_handle,const aui_smc_param_t * p_smc_param)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code = 0;
	unsigned int protocol;

	if(((void*) aui_smc_info != (void*)smc_handle)||(!p_smc_param))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if( (p_smc_param->m_e_protocol > AUI_SMC_PROTOCOL_MAX)|| \
		(p_smc_param->m_e_standard > AUI_SMC_STANDARD_MAX)|| \
		(p_smc_param->m_nETU < 0))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	//memset(&aui_smc_info->ps_smcparams_temp,0,sizeof( aui_smc_param_t ));
	//memcpy(&aui_smc_info->ps_smcparams_temp,p_smc_param,sizeof( aui_smc_param_t ));
	switch (p_smc_param->m_e_protocol) {
	case AUI_SMC_PROTOCOL_T0:
		protocol = 0;
		break;
	case EM_AUISMC_PROTOCOL_T1:
	case AUI_SMC_PROTOCOL_T1_ISO:
		protocol = 1;
		break;
	case AUI_SMC_PROTOCOL_T14:
		protocol = 14;
		break;
	default:
		protocol = (unsigned int)-1;
		break;
	}

	// 0 ~ 15: Transmission protocol defined in ISO/IEC 7816-3
	if (protocol < 16) {
	    error_code = smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_PROTOCOL, protocol);
	    if (!error_code) {
	        aui_smc_info->ps_smcparams_temp.m_e_protocol = p_smc_param->m_e_protocol;
	    } else {
	        AUI_DBG("Set protocol %d fail\n", p_smc_param->m_e_protocol);
	        ret = AUI_RTN_FAIL;
	    }
	}

	// Irdeto T=14 smart card can be reset only in a special ETU(620)
	// But driver not allow to set ETU before reset or card inserted.
	// This is a collision. Linux had fixed it.
	// There is no demand to support Irdeto T=14 smart card in TDS/alidownloader now.
	if (p_smc_param->m_nETU > 0) {
	    error_code = smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_ETU, p_smc_param->m_nETU);
	    if (!error_code) {
	        aui_smc_info->ps_smcparams_temp.m_nETU = p_smc_param->m_nETU;
	    } else {
	        AUI_DBG("Set ETU %d fail\n", p_smc_param->m_nETU);
            ret |= AUI_RTN_FAIL;
        }
	}

	return ret;
}

// IRDETO T=14 smart card transmission
// Smartcard Low-Level Interface Document No. 753354
// 4.4 Message Structure Description
// step 1: SendRaw all data
// step 2: ReadRaw 8 bytes respond header(include payload length)
// step 3: ReadRaw payload and checksum
static int smc_irdeto_t14_exchange(
        aui_hdl smc_handle,
        unsigned char *sendData_p,
        int sendDataLength,
        unsigned char *recvData_p,
        short *recvDataLength_p)
{
    int ret = 0;
    unsigned int LC; // Payload length.
    unsigned int RESPONSE_HEAD_LEN = 8;
    //unsigned int read_head_len = 8;
    short recvLength = 0;

    if (aui_smc_raw_write(smc_handle, sendData_p,
            (short)sendDataLength, &recvLength) != AUI_RTN_SUCCESS) {
        AUI_ERR("T14 write fail\n");
        return 1;
    }

    memset(recvData_p, 0, *recvDataLength_p);
    *recvDataLength_p = 0;
    recvLength = 0;
    if (aui_smc_raw_read(smc_handle, recvData_p,
            RESPONSE_HEAD_LEN, &recvLength)) {
        AUI_ERR("T14 read respond header fail\n");
        return 1;
    }

    LC = *((unsigned char *)recvData_p + RESPONSE_HEAD_LEN - 1);

    recvLength = 0;
    if (aui_smc_raw_read(smc_handle, recvData_p + RESPONSE_HEAD_LEN,
            LC + 1, &recvLength)) {
        AUI_ERR("T14 read payload and checksum fail %d bytes\n", LC + 1);
        return 1;
    }

    *recvDataLength_p = recvLength + RESPONSE_HEAD_LEN;
    return ret;
}

AUI_RTN_CODE aui_smc_transfer(aui_hdl smc_handle,unsigned char  *puc_write_data,int n_number_to_write,\
									unsigned char  *pc_response_data,int *pn_number_read)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code;
	short len = 0;
	short actual_len_temp = 0;
	long actual_len_long_temp = 0;
	//unsigned int IN = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_IN);
	unsigned int OUT = AUISMC_ERROR_OUT;
	unsigned int READY = AUI_SMC_ERROR_READY;
	unsigned int protocol = 0;
	unsigned char recv_buf[RECV_MAX_LENGTH];

	if(((void*) aui_smc_info != (void*)smc_handle)|| !puc_write_data|| \
		!pc_response_data||!pn_number_read||(n_number_to_write < 0))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(aui_smc_info->state == OUT)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	if(aui_smc_info->state != READY)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	len = *pn_number_read;
	if (aui_smc_info->ps_smcparams_temp.m_e_protocol == AUI_SMC_PROTOCOL_UNKNOWN){
		smc_io_control(aui_smc_info->dev, SMC_DRIVER_GET_PROTOCOL,
			(unsigned int)&protocol);

		if (protocol == 0){
			aui_smc_info->ps_smcparams_temp.m_e_protocol = AUI_SMC_PROTOCOL_T0;
		} else if (protocol == 1){
			aui_smc_info->ps_smcparams_temp.m_e_protocol = EM_AUISMC_PROTOCOL_T1;
		} else if (protocol == 14){
			aui_smc_info->ps_smcparams_temp.m_e_protocol = AUI_SMC_PROTOCOL_T14;
		}
	}
	if(( aui_smc_info->dev == NULL)
	        || ((aui_smc_info->ps_smcparams_temp.m_e_protocol != AUI_SMC_PROTOCOL_T0)
	                && (aui_smc_info->ps_smcparams_temp.m_e_protocol != EM_AUISMC_PROTOCOL_T1)
	                && (aui_smc_info->ps_smcparams_temp.m_e_protocol != AUI_SMC_PROTOCOL_T1_ISO)
	                && (aui_smc_info->ps_smcparams_temp.m_e_protocol != AUI_SMC_PROTOCOL_T14)))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	
	if(aui_smc_info->ps_smcparams_temp.m_e_protocol == AUI_SMC_PROTOCOL_T0)
	{
		AUI_DBG("aui_smc_transfer AUI_SMC_PROTOCOL_T0\n");
		error_code = smc_iso_transfer(aui_smc_info->dev, puc_write_data, \
									(short)n_number_to_write, pc_response_data, len, &actual_len_temp);

		*pn_number_read = actual_len_temp;

		if(error_code != RET_SUCCESS)
		{
			AUI_SMC_BAID();
			aui_rtn(AUI_RTN_EIO, "error");
		}
	}
	else if (aui_smc_info->ps_smcparams_temp.m_e_protocol == EM_AUISMC_PROTOCOL_T1)
	{
		AUI_DBG("aui_smc_transfer EM_AUISMC_PROTOCOL_T1\n");
		error_code = smc_t1_transfer(aui_smc_info->dev,0x00,puc_write_data,n_number_to_write,pc_response_data,len);

		if(error_code < 0)
		{
			AUI_SMC_BAID();
			aui_rtn(AUI_RTN_EIO, "error");
		}

		*pn_number_read = error_code;
	}
	else if (aui_smc_info->ps_smcparams_temp.m_e_protocol == AUI_SMC_PROTOCOL_T1_ISO)
	{
		AUI_DBG("aui_smc_transfer AUI_SMC_PROTOCOL_T1_ISO\n");
		error_code = smc_iso_transfer_t1(aui_smc_info->dev,puc_write_data,n_number_to_write,pc_response_data,len,&actual_len_long_temp);

		if(error_code < 0)
		{
			AUI_SMC_BAID();
			aui_rtn(AUI_RTN_EIO, "error");
		}
		*pn_number_read = actual_len_long_temp;
	} else if (aui_smc_info->ps_smcparams_temp.m_e_protocol == AUI_SMC_PROTOCOL_T14) {
		AUI_DBG("aui_smc_transfer T14\n");
		error_code = smc_irdeto_t14_exchange(smc_handle, puc_write_data,
				n_number_to_write, recv_buf, &actual_len_temp);
		if (error_code) {
			AUI_SMC_BAID();
			aui_rtn(AUI_RTN_EIO, "error");
		}
		if (actual_len_temp < (unsigned int)*pn_number_read) {
			*pn_number_read = actual_len_temp;
		}
		memcpy(pc_response_data, recv_buf, *pn_number_read);
	}
	return ret;
}


AUI_RTN_CODE aui_smc_send(aui_hdl smc_handle,unsigned char  *puc_hdr_body_buf,int n_number_to_write,\
								int *pn_number_write,unsigned char  *puc_status_word,int  n_timeout)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code;
	//unsigned int IN = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_IN);
	unsigned int OUT = AUISMC_ERROR_OUT;
	unsigned int READY = AUI_SMC_ERROR_READY;

	if(((void*) aui_smc_info != (void*)smc_handle)|| !puc_hdr_body_buf|| \
		!puc_status_word||!pn_number_write||(n_number_to_write < 0)||(n_timeout < 0))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(aui_smc_info->state != READY)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(( aui_smc_info->dev == NULL)||(aui_smc_info->state == OUT))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	osal_semaphore_capture(aui_smc_info->sema, OSAL_WAIT_FOREVER_TIME);

	error_code = smc_iso_transfer(aui_smc_info->dev, puc_hdr_body_buf, \
		(short)n_number_to_write, puc_status_word, 2, (short*)pn_number_write);

	if(error_code == 0)
	{
		osal_semaphore_release( aui_smc_info->sema );
		return AUI_RTN_SUCCESS;
	}
	else
	{
		osal_semaphore_release( aui_smc_info->sema );
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return ret;
}

AUI_RTN_CODE aui_smc_receive(aui_hdl smc_handle, unsigned char *puc_readcmd_hdr,unsigned char  *puc_read_buf,\
									int n_number_to_read,int *pn_number_read,unsigned char  *puc_status_word,int n_timeout)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code;

	if(((void*) aui_smc_info != (void*)smc_handle)||!puc_readcmd_hdr||!puc_status_word||\
		!puc_read_buf||!pn_number_read||(n_number_to_read < 0)||(n_timeout < 0))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	memcpy((void*)aui_smc_info->iso_cmd,puc_readcmd_hdr, HEADER_LEN);

	osal_semaphore_capture(aui_smc_info->sema, OSAL_WAIT_FOREVER_TIME);

	error_code = smc_iso_transfer(aui_smc_info->dev, aui_smc_info->iso_cmd, HEADER_LEN, puc_read_buf, \
									n_number_to_read, (short*)pn_number_read);

	if(error_code == 0)
	{
		if(*pn_number_read >= SW1SW2_LEN)
		{
			memcpy(puc_status_word,&puc_read_buf[*pn_number_read-SW1SW2_LEN],SW1SW2_LEN);
		}
		osal_semaphore_release( aui_smc_info->sema );
		return AUI_RTN_SUCCESS;
	}
	else
	{
		osal_semaphore_release( aui_smc_info->sema );
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return ret;
}

AUI_RTN_CODE aui_smc_detect(aui_hdl smc_handle)
{
	int ret = AUI_RTN_SUCCESS;
	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(( aui_smc_info->dev == NULL))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	ret = aui_smc_info->state;

	return ret;
}

AUI_RTN_CODE aui_smc_active(aui_hdl smc_handle)
{
	int ret = AUI_RTN_SUCCESS;
//	int ii ;
//	int i;
//	unsigned short alength=0;
//	unsigned char puc_atr[33];
//	unsigned int IN = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_IN);
//	unsigned int OUT = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_OUT);
//	unsigned int READY = AUI_SMC_ERROR_READY;

	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(( aui_smc_info->dev == NULL))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	ret = smc_card_exist(aui_smc_info->dev);
	if( 0 == ret )
	{
		aui_smc_info->state = AUISMC_ERROR_IN;
		// Why reset the card in active, it's wrong.
//		for(i=0; i<2; i++)
//		{
//			ret = smc_reset(aui_smc_info->dev,puc_atr,&alength);
//			if(ret == 0)
//			{
//				//according to CAS porting doc, the wwt time should be change.
//				smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_WWT, 2000);
//				ret = 0;
//				aui_smc_info->state =READY;
//				AUI_DBG("\n atr data: ");
//				for( ii = 0 ; ii < alength ; ii ++)
//				{
//					if(ii%16 == 0)
//						AUI_DBG("\n ");
//					AUI_DBG("%02x ",puc_atr[ii]);
//				}
//				AUI_DBG("\n\n smart card :: ready \n\n");
//				break;
//			}
//		}
//		if(ret != RET_SUCCESS)
//		{
//			AUI_SMC_BAID();
//			aui_rtn(AUI_RTN_EIO, "error");
//		}
	}
	else if (1 == ret)
	{
		aui_smc_info->state = AUISMC_ERROR_OUT;
	}
	else
	{
		aui_smc_info->state = AUISMC_ERROR_OUT;
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_smc_reset(aui_hdl smc_handle, unsigned char * puc_atr, unsigned short * pn_atr_length, int b_cold_rst)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code;
//	int ii ;
	int i;
	unsigned short alength=0;
//	unsigned int IN = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_IN);
//	unsigned int OUT = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_OUT);
	unsigned int READY = AUI_SMC_ERROR_READY;
	aui_smc_param_t smc_params;

	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(!puc_atr||!pn_atr_length)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(aui_smc_info->dev == NULL)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	error_code = smc_card_exist(aui_smc_info->dev);
	if( SUCCESS == error_code )
	{
		if(b_cold_rst == TRUE )
		{
			smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_RESET_MODE, 0);
		}
		else if(b_cold_rst == FALSE)
		{
			smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_RESET_MODE, 1);
		}
		for(i = 0; i < 4; i ++)
		{
			ret = smc_reset(aui_smc_info->dev,puc_atr,&alength);
			memset(&smc_params, 0, sizeof(smc_params));
			// Reset the SMC parameter in aui_smc_info,
			// in TDS, function aui_smc_param_get will do that.
			ret |= aui_smc_param_get(smc_handle, &smc_params);
			if(ret == RET_SUCCESS)
			{
				ret = AUI_RTN_SUCCESS;
				// Conax test failed because of setting WT in reset.
				// Should not set WT here,
				// It not the same concept against the following delay.
				// ISO/IEC 7816-3 The answer on I/O shall begin between
				// 400 and 40000 clock cycles after the rising edge of
				// the signal on RST.

				// CAS porting doc means the Nagra porting doc?
				// according to CAS porting doc, the wwt time should be change.
				//smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_WWT, 4000);
				aui_smc_info->state = READY;
				*pn_atr_length = alength;
				break;
			}

		}
		if(ret != RET_SUCCESS)
		{
			AUI_SMC_BAID();
			aui_rtn(AUI_RTN_EIO, "error");
		}
	}
	else
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	return ret;
}

AUI_RTN_CODE aui_smc_deactive(aui_hdl smc_handle)
{
	int ret = AUI_RTN_SUCCESS;
//	unsigned int IN = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_IN);
//	unsigned int OUT = AUI_RTN_VAL(AUI_MODULE_SMC,AUISMC_ERROR_OUT);
//	unsigned int READY = AUI_SMC_ERROR_READY;

	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(( aui_smc_info->dev == NULL))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	// Deactivate can be perform no matter what the card state is.
//	if(aui_smc_info->state != READY)
//	{
//		AUI_SMC_BAID();
//		aui_rtn(AUI_RTN_EIO, "error");
//	}

	if( smc_deactive(aui_smc_info->dev)==RET_SUCCESS)
	{
		ret = AUI_RTN_SUCCESS;
	}
	else
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	osal_task_sleep(1000);
	return ret;
}
AUI_RTN_CODE aui_smc_close(aui_hdl smc_handle)
{
	int  ret = AUI_RTN_SUCCESS;
	int error_code;

	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if((NULL == aui_smc_info->dev )||aui_smc_info->slot_state != 1)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if (aui_smc_info->sema != OSAL_INVALID_ID)
	{
		osal_semaphore_delete(aui_smc_info->sema);
		aui_smc_info->sema = OSAL_INVALID_ID;
	}

	error_code = smc_close(aui_smc_info->dev);
	if( 0 == error_code)
	{
		aui_smc_info->slot_state = 0;
		aui_smc_info->dev = NULL;
		ret = AUI_RTN_SUCCESS ;
	}
	else
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	(void)aui_dev_unreg(AUI_MODULE_SMC, aui_smc_info);
	return ret;
}

AUI_RTN_CODE aui_smc_setpps(aui_hdl smc_handle,unsigned char  *puc_write_data,int number_to_writelen, \
								unsigned char  *puc_response_data,int *pn_response_datalen)
{
	short act_len=0;
	if(((void*) aui_smc_info != (void*)smc_handle)||!puc_write_data||!puc_response_data|| \
		!pn_response_datalen||(number_to_writelen < 0))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}
	if((puc_write_data[0] & 0x10) != 0)  //  PPS1 valid
	{
		AUI_DBG("PPS1 valid \n");
	}
	else if((puc_write_data[0] & 0x20) != 0) //   PPS2 valid
	{
		AUI_DBG("PPS2 valid \n");
	}
	else if((puc_write_data[0] & 0x40) != 0)  //  PPS3 valid
	{
		AUI_DBG("PPS3 valid \n");
	}
	else
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(!aui_smc_info->dev)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(smc_raw_write(aui_smc_info->dev, puc_write_data, number_to_writelen, &act_len) != RET_SUCCESS)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	if(smc_raw_read(aui_smc_info->dev,puc_response_data, *pn_response_datalen ,&act_len) != RET_SUCCESS)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	*pn_response_datalen = act_len;

	return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_smc_de_init(p_fun_cb pnim_call_back_init)
{
	if(!aui_smc_info)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	else
	{
		MEMSET((void*)aui_smc_info,0x00,sizeof(aui_smc_info_t));
		FREE(aui_smc_info);
		aui_smc_info = NULL;
		if(pnim_call_back_init)
			return pnim_call_back_init(NULL);

		return AUI_RTN_SUCCESS;
	}
}

AUI_RTN_CODE aui_smc_isexist(aui_hdl smc_handle)
{
	int ret = AUI_RTN_SUCCESS;
//	unsigned short alength=0;

	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(aui_smc_info->dev == NULL)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	ret = smc_card_exist(aui_smc_info->dev);
	return ret;
}

// Setup the interface device after a successful PPS exchange
int ifd_pps_set(aui_hdl smc_handle, char *pps)
{
    unsigned char FI, DI;
    int Fi, Di, etu;
    aui_smc_param_t smc_param;

    // ISO/IEC 7813-3:2006
    static int conversion_factor[16] = {
            372, 372, 558, 744, 1116, 1488, 1860, -1,
            -1, 512, 768, 1024, 1536, 2048, -1, -1
    };
    static int adjustment_factor[16] = {
            -1, 1, 2, 4, 8, 16, 32, 64, 12, 20, -1, -1, -1, -1, -1, -1
    };
    // IRDETO T=14
    static int t14_conversion_factor[16] = {
            -1, 416, 620, -1, -1, -1, -1, -1,
            -1, 512, -1, 1152, -1, -1, -1, -1
    };
    static int t14_adjustment_factor[16] = {
            -1, 1, 2, 4, 8, 16, 32, 64,
            -1, -1, -1, -1, -1, -1, -1, -1
    };

    if (pps == NULL) {
        return 1;
    }
    memset(&smc_param, 0, sizeof(smc_param));
    FI = 1; // default
    DI = 1;
    if (pps[1] & 0x10) {
        FI = (pps[2] >> 4) & 0x0F;
        DI = pps[2] & 0x0F;
    }
    switch (pps[1] & 0xF) {
        case 14:
            Fi = t14_conversion_factor[FI];
            Di = t14_adjustment_factor[DI];
            break;
        default:
            Fi = conversion_factor[FI];
            Di = adjustment_factor[DI];
            break;
    }

    // Remove CPPTEST warning. Di will never be 0.
    if (Di == 0) {
        AUI_DBG("[PPS] Di == 0 !!!\n");
        return 1;
    }
    etu = Fi / Di;
    smc_param.m_nETU = etu;
    smc_param.m_e_protocol = pps[1] & 0x0F;
    //AUI_DBG("[PPS]ETU: %d T: %d\n", smc_param.m_nETU, smc_param.m_e_protocol);
    if (aui_smc_param_set(smc_handle, &smc_param)
            != AUI_RTN_SUCCESS) {
        AUI_DBG("[PPS] aui_smc_param_set failed\n");
        return 1;
    }
    return 0;
}

AUI_RTN_CODE aui_smc_raw_read(aui_hdl smc_handle,unsigned char *buffer, signed short size, signed short *actlen)
{
	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(aui_smc_info->dev == NULL)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(!buffer || !actlen)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(smc_raw_read(aui_smc_info->dev,buffer,size,actlen) != SUCCESS)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	} else if ((*(char *)buffer == PPSS)
	        && (*actlen >= PPS_MIN_LEN)
	        && (*actlen <= PPS_MAX_LEN)) {
	    // Setup interface device after a successful PPS exchange
	    // ISO/IEC 7816-3:2006 9.3 Successful PPS exchange
	    // The success PPS exchange the first 2 bytes is identical
	    if (!memcmp(pps_buf, buffer, 2)) {
	        if (ifd_pps_set(smc_handle, (char *)buffer)) {
	            AUI_DBG("Raw transmission PPS ifd_pps_set fail\n");
	        }
	    }
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_raw_write(aui_hdl smc_handle, unsigned char *buffer, signed short size, signed short *actlen)
{
	if(((void*) aui_smc_info != (void*)smc_handle))
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(aui_smc_info->dev == NULL)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(!buffer || !actlen)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if(smc_raw_write(aui_smc_info->dev,buffer,size,actlen) != SUCCESS)
	{
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	} else {
	    if ((*(char *)buffer == PPSS)
	            && (*actlen >= PPS_MIN_LEN)
	            && (*actlen <= PPS_MAX_LEN)) {
	        // Monitoring PPS exchange
	        memcpy(pps_buf, buffer, *actlen);
	    }
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_smc_set(aui_hdl smc_handle, aui_smc_cmd_t cmd,
		void *pv_param)
{
	int ret = AUI_RTN_SUCCESS;
	int error_code;

	if (((void *) aui_smc_info != (void *)smc_handle)) {
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EINVAL, "input error");
	}

	if (( aui_smc_info->dev == NULL)) {
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}

	error_code = smc_card_exist(aui_smc_info->dev);
	if (0 == error_code) {
		switch (cmd) {
		case AUI_SMC_CMD_SET_WWT:
			smc_io_control(aui_smc_info->dev, SMC_DRIVER_SET_WWT,
					(unsigned long)pv_param);
			break;
		default:
		    aui_rtn(AUI_RTN_EIO, "error");
		}
	} else {
		AUI_SMC_BAID();
		aui_rtn(AUI_RTN_EIO, "error");
	}
	return ret;
}
