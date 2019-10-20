#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "linux/ali_sec.h"
#include <semaphore.h>

#include "aui_dsc_common.h"
#include <aui_dsc.h>


#include "alipltfretcode.h"

#include <alislvmx.h>

#include <aui_vmx.h>

#include "aui_dmx_priv.h"
//for select
#include <sys/select.h>
#include "aui_common_priv.h"

#define VMX_INVALID_ID 0xFFFF


typedef struct vmx_handler{
    struct dsc_handler dsc_hdl;
    aui_vmx_dsc_attr vmx_attr;
    aui_vmx_msg_callback_attr msg_callback_attr;
	pthread_t msg_thread_id;
    sem_t thread_sem;    
    char msg_thread_exit;
}vmx_handler;

#define EMC_VALUE_LENGTH_MAX 255
typedef struct{
	unsigned char bServiceIdx;//[in]
	unsigned char ecm_value[EMC_VALUE_LENGTH_MAX];//[in] ------>unsigned char ecm_value[255]
	unsigned char length;//[in]
}BC_CWC_DATA;

AUI_MODULE(VMX)

#define SELECT_MODE
static void* message_thread_loop(void *arg)
{
	BC_CWC_DATA bc_cwc_data;
	//for kumsgq
	int kumsgq_flag ;
	int kumsgq_fd = -1;
    vmx_handler *vmx_hdl = (vmx_handler *)arg;

	aui_vmx_callback_attr callback_attr;

#ifdef SELECT_MODE
	//for select 
	fd_set fds; 
	struct timeval timeout={10,0};
#endif 
    if (NULL == arg){
        AUI_ERR("thread parameter is error!\n");
        return NULL;
    }

    AUI_DBG("before wait...\n");
    if(sem_wait(&vmx_hdl->thread_sem) == 0)
    {}
    AUI_DBG("after wait...\n");
    if (vmx_hdl->msg_thread_exit){
        sem_post(&vmx_hdl->thread_sem);
        AUI_ERR("thread exit now!\n");
        return NULL;
    }

	// ioctl get kumsgq fd and read.
	kumsgq_flag = O_CLOEXEC;
	kumsgq_fd = alisl_vmx_ioctl(vmx_hdl->dsc_hdl.dev,VMX_IO_GET_KUMSGQ, &kumsgq_flag);
	AUI_DBG("kumsgq_flag  = %d\n",kumsgq_flag);
	if (-1 != kumsgq_fd){
		AUI_DBG("ioctl IO_TAC_GET_KUMSGQ ok, kumsgq_fd = %d\n",kumsgq_fd);
	}
	else {
		AUI_ERR("ioctl IO_TAC_GET_KUMSGQ failed...\n");
        //sem_post(&vmx_hdl->thread_sem);
        //return NULL;
	}


	//select/poll
    while (!vmx_hdl->msg_thread_exit){

#ifdef SELECT_MODE
		AUI_DBG("read mode: SELECT_MODE...\n");
		FD_ZERO(&fds);	
		FD_SET(kumsgq_fd,&fds);
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		switch(select(kumsgq_fd+1,&fds,NULL,NULL,&timeout))
		{
		case -1: 
			//AUI_ERR("@select failed...\n");
			break;
		case 0: 
			//AUI_ERR("select timeout...\n");
			break;
		default:
			//printf("select ok...\n");
			if(FD_ISSET(kumsgq_fd,&fds))
			{
            	int ret = 0;
                void *pv_user_data = vmx_hdl->msg_callback_attr.pv_user_data;
                aui_vmx_msg_callback msg_callback = vmx_hdl->msg_callback_attr.p_fn_vmx_msg_cb;
            	
                memset(&callback_attr, 0, sizeof(aui_vmx_callback_attr));
                
				AUI_DBG("start read kumsqg_fd\n");
				ret = read(kumsgq_fd,&bc_cwc_data,sizeof(BC_CWC_DATA));
				AUI_DBG("read ok,ret = %d,bc_cwc_data:bServiceIdx=0x%x,length=%d.\n",\
				    ret,(int) bc_cwc_data.bServiceIdx,(int)bc_cwc_data.length);
				// add your code here to handle bc_cwc_data
                
				if (msg_callback && bc_cwc_data.length > 0 && ret > 0){
                    callback_attr.callback_type = AUI_VMX_CALLBACK_TYPE_CF_CWC;
        			callback_attr.buffer = bc_cwc_data.ecm_value;
                    callback_attr.buffer_size = bc_cwc_data.length;
                    msg_callback(pv_user_data, (void*)(&callback_attr));
				}

				alisl_vmx_ioctl(vmx_hdl->dsc_hdl.dev, VMX_IO_RPC_RET,0);
			}
			break;
		}
#endif
    }

	close(kumsgq_fd); 	
    
    AUI_DBG("%s() exit!\n", __FUNCTION__);
    
    sem_post(&vmx_hdl->thread_sem);
    
	pthread_exit(NULL);

    return NULL;
}



AUI_RTN_CODE aui_vmx_open(aui_vmx_dsc_attr *vmx_attr, aui_hdl *handle)
{
    vmx_handler *vmx_hdl = NULL;
    vmx_service_index sl_service_idx;

    AUI_DBG("vmx_attr->service_index:0x%x\n", (unsigned int)(vmx_attr->service_index));

    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");        
        return AUI_RTN_EINVAL;
    }
    if (NULL == vmx_attr){
        AUI_ERR("vmx_attr is NULL!\n");        
        return AUI_RTN_EINVAL;
    }
    
    vmx_hdl = (vmx_handler *)calloc(sizeof(vmx_handler), 1);    
    if (NULL == vmx_hdl){
        AUI_ERR("malloc vmx handle error!\n");
        return AUI_RTN_FAIL;
    }
    memset(vmx_hdl, 0, sizeof(vmx_handler));
    switch (vmx_attr->service_index){
        case AUI_VMX_SERVICE_DVB0:
            sl_service_idx = VMX_SERVICE_DVB0;
            break;
        case AUI_VMX_SERVICE_DVB1:
            sl_service_idx = VMX_SERVICE_DVB1;
            break;
        case AUI_VMX_SERVICE_DVB2:
            sl_service_idx = VMX_SERVICE_DVB2;
            break;
        case AUI_VMX_SERVICE_IPTV0:
            sl_service_idx = VMX_SERVICE_IPTV0;
            break;
        case AUI_VMX_SERVICE_IPTV1:
            sl_service_idx = VMX_SERVICE_IPTV1;
            break;
        case AUI_VMX_SERVICE_IPTV2:
            sl_service_idx = VMX_SERVICE_IPTV2;
            break;
        case AUI_VMX_SERVICE_DVR0:
            sl_service_idx = VMX_SERVICE_DVR0;
            break;
        case AUI_VMX_SERVICE_DVR1:
            sl_service_idx = VMX_SERVICE_DVR1;
            break;
        case AUI_VMX_SERVICE_DVR2:
            sl_service_idx = VMX_SERVICE_DVR2;
            break;
        case AUI_VMX_SERVICE_OTT0:
            sl_service_idx = VMX_SERVICE_OTT0;
            break;
        case AUI_VMX_SERVICE_OTT1:
            sl_service_idx = VMX_SERVICE_OTT1;
            break;
        default:
            AUI_ERR("service index error!\n");
            return AUI_RTN_FAIL;
    } 

    vmx_hdl->dsc_hdl.data.dev_idx = vmx_attr->uc_dev_idx;
    //#define SEVICE_IS_BLOCK_MODE(x) (x & 0x80)
    if (alisl_vmx_open(sl_service_idx, &(vmx_hdl->dsc_hdl.dev))){
        AUI_ERR("alisl_vmx_open error!\n");
        free(vmx_hdl);
        return AUI_RTN_FAIL;
    }
    memcpy(&(vmx_hdl->vmx_attr), vmx_attr, sizeof(aui_vmx_dsc_attr));
    vmx_hdl->msg_thread_exit = 0;
    vmx_hdl->msg_thread_id = VMX_INVALID_ID;
	if (alisl_vmx_fd_get(vmx_hdl->dsc_hdl.dev,&(vmx_hdl->dsc_hdl.dsc_fd))) {
        AUI_ERR("alisl_vmx_fd_get error!\n");
        free(vmx_hdl);
        return AUI_RTN_FAIL;
    }

    if (vmx_hdl->dsc_hdl.pvr_fd <= 0){
    	if(alisldsc_pvr_open(&(vmx_hdl->dsc_hdl.sl_pvr_hdl))) {
            AUI_ERR("alisldsc_pvr_open error!\n");
            free(vmx_hdl);
            return AUI_RTN_FAIL;
    	}
        alisldsc_pvr_fd_get(vmx_hdl->dsc_hdl.sl_pvr_hdl, &(vmx_hdl->dsc_hdl.pvr_fd));
    }
    // only DVR and OTT service need to set BLOCK SIZE
    if (sl_service_idx >= VMX_SERVICE_DVR0){
    	if(alisldsc_pvr_ioctl_ex(vmx_hdl->dsc_hdl.sl_pvr_hdl, SL_PVR_IO_SET_BLOCK_SIZE, 
            (unsigned long)(vmx_attr->block_size))){
    		AUI_ERR("set block size error!\n");
    		if(alisldsc_pvr_close(vmx_hdl->dsc_hdl.sl_pvr_hdl)) {
        		AUI_ERR("alisldsc_pvr_close error!\n");
    		}
            free(vmx_hdl);
            return AUI_RTN_FAIL;
    	}
    }
    vmx_hdl->dsc_hdl.dsc_process_attr.ul_block_size = vmx_attr->block_size;
    //if the vmx_hdl->dsc_hdl.dsc_process_attr.process_mode need to set for 
    // aui_dmx?
    //vmx_hdl->dsc_hdl.dsc_process_attr.process_mode = 
    
    AUI_INIT_LIST_HEAD(&(vmx_hdl->dsc_hdl.key_list));

    sem_init(&vmx_hdl->thread_sem, 0, 1);

    if (aui_dev_reg(AUI_MODULE_VMX, vmx_hdl)) {
        free(vmx_hdl);
        AUI_ERR("dev idx already used");
        return AUI_RTN_FAIL;
    }
    
    *handle = vmx_hdl;
    AUI_DBG("open vmx device successfully!vmx_hdl=0x%x\n", (unsigned int)vmx_hdl);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_close(aui_hdl handle)
{
    vmx_handler *vmx_hdl = (vmx_handler *)handle;
    int err = AUI_RTN_SUCCESS;
    
    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");
        return AUI_RTN_EINVAL;
    }
    /*
    if (aui_dev_unreg(AUI_MODULE_DSC, hdl)) {
        err = AUI_RTN_FAIL;
        pr_debug("[aui dsc] aui_dev_unreg error");
    }
    */
    
    vmx_hdl->msg_thread_exit = 1;
    //if (vmx_hdl->msg_thread_id != VMX_INVALID_ID)
    //    pthread_cancel(vmx_hdl->msg_thread_id);
    
    AUI_DBG("before wait...\n");
    if(sem_wait(&vmx_hdl->thread_sem) == 0)
    {}
    AUI_DBG("after wait...\n");    

    if (alisl_vmx_close(vmx_hdl->dsc_hdl.dev)){
        AUI_ERR("alisl_vmx_close error!\n");
        err = AUI_RTN_FAIL;
    }
	if(vmx_hdl->dsc_hdl.see_dmx_fd > 0){
		if (ioctl(vmx_hdl->dsc_hdl.see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_SRC_SET, DMX_MAIN2SEE_SRC_NORMAL)){
			err |= AUI_RTN_FAIL;
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_SRC_SET error\n");
		}

		if (ioctl(vmx_hdl->dsc_hdl.see_dmx_fd, ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET, 0)) {
			err |= AUI_RTN_FAIL;
			AUI_ERR("ALI_DMX_SEE_MAIN2SEE_BUF_VALIDSIZE_SET error\n");
		}
		close(vmx_hdl->dsc_hdl.see_dmx_fd);
        vmx_hdl->dsc_hdl.see_dmx_fd = -1;
	}
	if ((vmx_hdl->dsc_hdl.sl_pvr_hdl) && (vmx_hdl->dsc_hdl.ali_pvr_de_hdl)) {
		aui_dsc_pvr_playback_env_deinit(&(vmx_hdl->dsc_hdl));
	}
    if (vmx_hdl->dsc_hdl.pvr_fd > 0){
        alisldsc_pvr_close(vmx_hdl->dsc_hdl.sl_pvr_hdl);
        vmx_hdl->dsc_hdl.pvr_fd = -1;
    }

    if (aui_dev_unreg(AUI_MODULE_VMX, vmx_hdl)) {
        err = AUI_RTN_FAIL;
        AUI_ERR("aui_dev_unreg error");
    }
    
    sem_post(&vmx_hdl->thread_sem);
    
    sem_destroy(&vmx_hdl->thread_sem);
    vmx_hdl->msg_thread_id = VMX_INVALID_ID;
    
    free(vmx_hdl);

    
    AUI_DBG("Close vmx device successfully!\n");
    return err;
}


AUI_RTN_CODE aui_vmx_dsc_algo_set(aui_hdl handle, aui_vmx_dsc_algo *vmx_algo)
{
    vmx_handler *vmx_hdl = (vmx_handler *)handle;
    CA_ALGO_PARAM vmx_ca_algo;
    
    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");
        return AUI_RTN_EINVAL;
    }
    if (NULL == vmx_algo){
        AUI_ERR("vmx_algo is NULL!\n");
        return AUI_RTN_EINVAL;
    }

    AUI_DBG("service_index:%d, ca_algo:%d, ca_mode:%d, iv_value[5]=0x%x\n",
        vmx_hdl->vmx_attr.service_index, vmx_algo->ca_algo, vmx_algo->ca_mode, vmx_algo->iv_value[5]);

    memset(&vmx_ca_algo, 0, sizeof(vmx_ca_algo));
    vmx_ca_algo.bServiceIdx = vmx_hdl->vmx_attr.service_index;
    vmx_ca_algo.ca_device = vmx_algo->ca_algo;
    vmx_ca_algo.ca_mode = vmx_algo->ca_mode;
    memcpy(vmx_ca_algo.iv_value, vmx_algo->iv_value, sizeof(vmx_ca_algo.iv_value));
    if (AUI_VMX_ALGO_AES_128 == vmx_algo->ca_algo){
        vmx_hdl->dsc_hdl.algo = AUI_DSC_ALGO_AES;
    }else if (AUI_VMX_ALGO_CSA == vmx_algo->ca_algo){
        vmx_hdl->dsc_hdl.algo = AUI_DSC_ALGO_CSA;
    }
    
    if (alisl_vmx_ioctl(vmx_hdl->dsc_hdl.dev, VMX_IO_SET_CA_ALOG, &vmx_ca_algo)){
        AUI_ERR("aui_vmx_dsc_algo_set() error!\n");
        return AUI_RTN_EINVAL;
    }
        
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_pid_set(aui_hdl handle, aui_vmx_pid_info *pid_info)
{
    PID_INFO dsc_pid_info;
    vmx_handler *vmx_hdl = (vmx_handler *)handle;
    
    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");
        return AUI_RTN_EINVAL;
    }
    if (NULL == pid_info){
        AUI_ERR("pid_info is NULL!\n");
        return AUI_RTN_EINVAL;
    }

    AUI_DBG("service_index:%d, pid_number:%d, pid_table[0]:0x%x, pid_table[%d]=0x%x\n",
        vmx_hdl->vmx_attr.service_index, pid_info->pid_number, pid_info->pid_table[0], 
        pid_info->pid_number-1, pid_info->pid_table[pid_info->pid_number-1]);

    memset(&dsc_pid_info, 0, sizeof(dsc_pid_info));
    dsc_pid_info.bServiceIdx = vmx_hdl->vmx_attr.service_index;
    dsc_pid_info.pid_number = pid_info->pid_number;
    memcpy(dsc_pid_info.pid_table, pid_info->pid_table, sizeof(dsc_pid_info.pid_table));
    if (alisl_vmx_ioctl(vmx_hdl->dsc_hdl.dev, VMX_IO_SET_PID, &dsc_pid_info)){
        AUI_ERR("aui_vmx_pid_set() error!\n");
        return AUI_RTN_EINVAL;
    }
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_dsc_status_get(aui_hdl handle, aui_vmx_dsc_status *dsc_status)
{
    BC_DSC_STATUS vmx_status;
    vmx_handler *vmx_hdl = (vmx_handler *)handle;
    
    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");
        return AUI_RTN_EINVAL;
    }
    if (NULL == dsc_status){
        AUI_ERR("dsc_status is NULL!\n");
        return AUI_RTN_EINVAL;
    }

    AUI_DBG("service_index:%d\n", vmx_hdl->vmx_attr.service_index);

    memset(&vmx_status, 0, sizeof(vmx_status));
    vmx_status.bServiceIdx = vmx_hdl->vmx_attr.service_index;
    if (alisl_vmx_ioctl(vmx_hdl->dsc_hdl.dev, VMX_IO_GET_DSC_STATUS, &vmx_status)){
        AUI_ERR("aui_vmx_dsc_status_get() error!\n");
        return AUI_RTN_EINVAL;
    }
    dsc_status->rec_block_count = vmx_status.rec_block_count;
    AUI_DBG("record block count:%d\n", vmx_status.rec_block_count);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_register_message_callback(aui_hdl handle, aui_vmx_msg_callback_attr *callback_attr)
{
    vmx_handler *vmx_hdl = (vmx_handler*)handle;
    
    if (NULL == handle || NULL == callback_attr){
        AUI_ERR("vmx handle is NULL!\n");        
        return AUI_RTN_EINVAL;
    }
    if (NULL == callback_attr){
        AUI_ERR("callback_attr is NULL!\n");        
        return AUI_RTN_EINVAL;
    }

    memcpy(&(vmx_hdl->msg_callback_attr), callback_attr, sizeof(aui_vmx_msg_callback_attr));
    
    if (VMX_INVALID_ID == vmx_hdl->msg_thread_id)
    	pthread_create(&vmx_hdl->msg_thread_id, NULL, \
    			message_thread_loop, (void*)vmx_hdl);
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_fd_get(aui_hdl handle, int *vmx_fd)
{
    vmx_handler *vmx_hdl = (vmx_handler*)handle;
    
    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");        
        return AUI_RTN_EINVAL;
    }
    if (vmx_hdl->dsc_hdl.dsc_fd <= 0){
    	if (alisl_vmx_fd_get(vmx_hdl->dsc_hdl.dev,&(vmx_hdl->dsc_hdl.dsc_fd))) {
            AUI_ERR("alisl_vmx_fd_get error!\n");
            return AUI_RTN_FAIL;
        }
    }
    *vmx_fd = vmx_hdl->dsc_hdl.dsc_fd;
    
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_service_index_get(aui_hdl handle, int *service_index)
{
    vmx_handler *vmx_hdl = (vmx_handler*)handle;
    
    if (NULL == handle){
        AUI_ERR("vmx handle is NULL!\n");        
        return AUI_RTN_EINVAL;
    }
    *service_index = (int)(vmx_hdl->vmx_attr.service_index);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_vmx_resource_id_get (aui_hdl handle, aui_vmx_resource_id *p_resource_id)
{
    return aui_dsc_resource_id_get(handle, (aui_dsc_resource_id*)p_resource_id);
}

