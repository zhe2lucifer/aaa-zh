/**@file
*     @brief     ALi UDI common function implement
*     @author    ray.gong
*     @date          2013-5-21
*      @version     1.0.0
*     @note           ali corp. all rights reserved. 2013-2999 copyright (C)
*                 input file detail description here
*                 input file detail description here
*                 input file detail description here
*/

/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <api/libsi/si_tdt.h>
#include "aui_rtc.h"
#include <stdarg.h>
#include <stddef.h>
#include "aui_dis.h"
/****************************LOCAL MACRO************************************/

#define INPUT_TASK_PRIORITY        OSAL_PRI_NORMAL
#define INPUT_TASK_STACKSIZE    0x1000
#define INPUT_TASK_QUANTUM          10

/****************************LOCAL TYPE************************************/
/** log handle */
typedef struct aui_st_log_handle
{
    unsigned char *puc_log_buf_addr;
    unsigned long ul_log_buf_len;
    unsigned long ul_log_buf_cur_pos;
    OSAL_ID sem_log;
}aui_log_handle,*aui_p_log_handle;

/** log handle */
typedef struct aui_st_log_cfg_rd
{
    unsigned char *puc_log_buf_addr;
    unsigned long ul_log_buf_len;
    OSAL_ID sem_log;
}aui_log_cfg_rd,*aui_p_log_cfg_rd;
/****************************LOCAL VAR************************************/

AUI_MODULE(COMMON)

//static unsigned char *s_uc_log_buf_addr_start=NULL;
//static unsigned long s_ul_log_buf_len=0x80000;
//static aui_log_handle s_log_hld;

aui_dev_all g_all_dev_hdl;
//char g_ac_log_string[MBF_SIZE_LOG+1]={0};
/****************************LOCAL FUNC DECLEAR************************************/


/****************************LOCAL FUNC IMPLEMENT************************************/
#if 0
static unsigned char char2hex(unsigned char ch)
{
    unsigned char ret =  - 1;
    if ((ch <= 0x39) && (ch >= 0x30))
    // '0'~'9'
        ret = ch &0xf;
    else if ((ch <= 102) && (ch >= 97))
    //'a'~'f'
        ret = ch - 97+10;
    else if ((ch <= 70) && (ch >= 65))
    //'A'~'F'
        ret = ch - 65+10;

    return ret;
}
#endif

static void list_mutex_lock(UINT32 lock)
{
    static OSAL_ID list_mutex= OSAL_INVALID_ID;
    if(OSAL_INVALID_ID == list_mutex)
    {
        list_mutex= osal_mutex_create();
        ASSERT(OSAL_INVALID_ID != list_mutex);
    }
    if(lock)
    {
        osal_mutex_lock(list_mutex, OSAL_WAIT_FOREVER_TIME);
    }
    else
    {
        osal_mutex_unlock(list_mutex);
    }
}


/****************************MODULE IMPLEMENT************************************/




/**
*     @brief         get ALi UDI version
*     @author        ray.gong
*     @date              2013-6-7
*     @param[out]    pul_version point to the ALi UDI version number
*      @return          AUI_RNT_SUCCESS initialize a ring buffer successful
*      @return          AUI_RTN_EINVAL  initialize a ring buffer failed,because input parameters invalid
*      @return          others  initialize a ring buffer failed
*     @note
*
*/
AUI_RTN_CODE aui_version_get(unsigned long *pul_version)
{
    if(NULL==pul_version)
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }
    *pul_version=AUI_VERSION_NUM;
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         get aui package information
*     @author        steven.zhang
*     @date              2017-5-9
*     @param[out]   the buffer for pass the aui package version 
*     @param[out]   the buffer for pass the aui been compiled datetime 
*      @return         AUI_RTN_SUCCESS get the information of aui package successful
*      @return         AUI_RTN_EINVAL  getting the infomation of aui package invalid
*     @note
*
*/
AUI_RTN_CODE aui_log_package_info_get(
	char* package_info,
	int max_len
)
{
	char* msg = "aui not support this!\n";
	if( max_len < strlen(msg)) {
		aui_rtn(AUI_RTN_EINVAL, "max_len is less than string length!\n");
	}

	snprintf(package_info, max_len,"%s", msg);
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         get common module version
*     @author        ray.gong
*     @date              2013-6-7
*     @param[out]    pul_version point to the common module version number
*      @return          AUI_RNT_SUCCESS initialize a ring buffer successful
*      @return          AUI_RTN_EINVAL  initialize a ring buffer failed,because input parameters invalid
*      @return          others  initialize a ring buffer failed
*     @note
*
*/
AUI_RTN_CODE aui_common_version_get(unsigned long *pul_version)
{
    if(NULL==pul_version)
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }
    *pul_version=AUI_MODULE_VERSION_NUM_COMMON;
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         initialize the common module
*     @author        ray.gong
*     @date              2013-5-20
*     @param[in]        p_common_module_attr point to the attribute of common module
*      @return          AUI_RTN_SUCCESS initialize the task module successful
*      @return          AUI_RTN_EINVAL  initialize the task module failed,because input parameters invalid
*      @return          others  initialize the task module failed
*     @note
*
*/
AUI_RTN_CODE aui_common_init(aui_p_common_module_attr p_common_module_attr)
{
    if(NULL==p_common_module_attr)
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    return AUI_RTN_SUCCESS;
}

/**
*     @brief         de_initialize the common module
*     @author        ray.gong
*     @date              2013-5-20
*      @return          AUI_RTN_SUCCESS deinitialize the task module successful
*      @return          AUI_RTN_EINVAL  deinitialize the task module failed,because input parameters invalid
*      @return          others  initialize the task module failed
*     @note
*
*/
AUI_RTN_CODE aui_common_de_init()
{

    return AUI_RTN_SUCCESS;
}


/**
*     @brief         initialize a ring buffer
*     @author        ray.gong
*     @date              2013-5-21
*     @param[in]        ul_buf_len ring buffer length
*     @param[out]    p_ring_buf new ring buffer handle
*      @return          AUI_RNT_SUCCESS initialize a ring buffer successful
*      @return          AUI_RTN_EINVAL  initialize a ring buffer failed,because input parameters invalid
*      @return          others  initialize a ring buffer failed
*     @note
*
*/

AUI_RTN_CODE aui_common_init_ring_buf(unsigned long ul_buf_len,aui_ring_buf* p_ring_buf)
{

    p_ring_buf->pby_ring_buf=MALLOC(ul_buf_len);
    if(NULL==p_ring_buf->pby_ring_buf)
    {
        aui_rtn(AUI_RTN_ENOMEM,"Malloc fail");
    }
    MEMSET(p_ring_buf->pby_ring_buf,0,ul_buf_len);
    p_ring_buf->ul_ring_buf_len=ul_buf_len;

    if(( p_ring_buf->sem_ring==0)||( p_ring_buf->sem_ring==OSAL_INVALID_ID))
    {
        p_ring_buf->sem_ring = osal_semaphore_create(1);

        if (OSAL_INVALID_ID == p_ring_buf->sem_ring)
        {
            ASSERT(0);
        }
    }

    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_common_rst_ring_buf(aui_ring_buf* p_ring_buf)
{

    if(NULL==p_ring_buf->pby_ring_buf)
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }
    AUI_TAKE_SEM(p_ring_buf->sem_ring);
    p_ring_buf->ul_ring_buf_rd = p_ring_buf->ul_ring_buf_wt = 0;
    AUI_GIVE_SEM(p_ring_buf->sem_ring);
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         deinitialize the ring buffer
*     @author        ray.gong
*     @date              2013-5-21
*     @param[in]        p_ring_buf new ring buffer handle
*      @return          AUI_RNT_SUCCESS deinitialize the ring buffer successful
*      @return          AUI_RTN_EINVAL  deinitialize the ring buffer failed,because input parameters invalid
*      @return          others  deinitialize the ring buffer failed
*     @note
*
*/
AUI_RTN_CODE aui_common_un_init_ring_buf(aui_ring_buf* p_ring_buf)
{
    if(NULL==p_ring_buf)
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    if(p_ring_buf->pby_ring_buf)
    {
        FREE(p_ring_buf->pby_ring_buf);
        p_ring_buf->pby_ring_buf=NULL;
        osal_semaphore_delete(p_ring_buf->sem_ring);
        MEMSET(p_ring_buf,0,sizeof(aui_ring_buf));
    }

    return AUI_RTN_SUCCESS;

}

/**
*    @brief         get the ring buffer start address
*    @author        ray.gong
*    @date            2013-5-21
*    @param[in]        p_ring_buf ring buffer handle
*    @param[out]    p_buf_addr point to the ring buffer start address
*     @return         AUI_RNT_SUCCESS get the ring buffer start address successful
*     @return         AUI_RTN_EINVAL  get the ring buffer start address failed,because input parameters invalid
*     @return         others  get the ring buffer start address failed
*    @note
*
*/
AUI_RTN_CODE aui_common_get_ring_buf_base_addr(aui_ring_buf* p_ring_buf,unsigned char** pp_buf_addr)
{
    if(NULL==p_ring_buf)
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    *pp_buf_addr=(unsigned char*)p_ring_buf->pby_ring_buf;
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         get the ring buffer data length
*     @author        ray.gong
*     @date              2013-5-21
*     @param[in]        p_ring_buf ring buffer handle
*     @param[out]    pul_data_len point to the ring buffer data length
*      @return          AUI_RNT_SUCCESS get the ring buffer data length successful
*      @return          AUI_RTN_EINVAL  get the ring buffer data length failed,because input parameters invalid
*      @return          others  get the ring buffer data length failed
*     @note
*
*/
AUI_RTN_CODE aui_common_ring_buf_data_len(aui_ring_buf* p_ring_buf,unsigned long *pul_data_len)
{
    if((NULL==p_ring_buf)||(NULL==pul_data_len))
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }
    AUI_TAKE_SEM(p_ring_buf->sem_ring);
    if(p_ring_buf->ul_ring_buf_wt>=p_ring_buf->ul_ring_buf_rd)
    {
        *pul_data_len=p_ring_buf->ul_ring_buf_wt-p_ring_buf->ul_ring_buf_rd;
    }
    else
    {
        *pul_data_len=p_ring_buf->ul_ring_buf_len+p_ring_buf->ul_ring_buf_wt-p_ring_buf->ul_ring_buf_rd;
    }
    AUI_GIVE_SEM(p_ring_buf->sem_ring);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_common_ring_buf_lock(aui_ring_buf* p_ring_buf)
{
    if(NULL==p_ring_buf)
    {
        return AUI_RTN_SUCCESS;
    }
    AUI_TAKE_SEM(p_ring_buf->sem_ring);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_common_ring_buf_unlock(aui_ring_buf* p_ring_buf)
{
    if(NULL==p_ring_buf)
    {
        return AUI_RTN_SUCCESS;
    }

    AUI_GIVE_SEM(p_ring_buf->sem_ring);
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         get the ring buffer remain space length
*     @author        ray.gong
*     @date              2013-5-21
*     @param[in]        p_ring_buf ring buffer handle
*     @param[out]    pul_buf_remain point to the ring buffer remain space length
*      @return          AUI_RNT_SUCCESS get the ring buffer remain space length successful
*      @return          AUI_RTN_EINVAL  get the ring buffer remain space length failed,because input parameters invalid
*      @return          others  get the ring buffer remain space length failed
*     @note
*
*/
AUI_RTN_CODE aui_common_ring_buf_remain(aui_ring_buf* p_ring_buf,unsigned long *pul_buf_remain)
{
    unsigned long ul_data_len=0;

    if((NULL==p_ring_buf)||(NULL==pul_buf_remain))
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    if(SUCCESS!=aui_common_ring_buf_data_len(p_ring_buf,&ul_data_len))
    {
        aui_rtn(AUI_RTN_FAIL, "get ring buffer data len fail");
    }
    AUI_TAKE_SEM(p_ring_buf->sem_ring);
    *pul_buf_remain=p_ring_buf->ul_ring_buf_len-ul_data_len-1;
    AUI_GIVE_SEM(p_ring_buf->sem_ring);
    return AUI_RTN_SUCCESS;

}


/**
*     @brief         read data from ring buffer
*     @author        ray.gong
*     @date              2013-5-21
*     @param[in]        p_ring_buf ring buffer handle
*     @param[in]        ul_buf_len read data length
*     @param[out]    p_data_out point to the data read from ring buffer
*      @return          AUI_RNT_SUCCESS read data from ring buffer successful
*      @return          AUI_RTN_EINVAL  read data from ring buffer failed,because input parameters invalid
*      @return          OS_RING_RD_NO_DATA  the remain data in ring buffer is less than want to read length
*      @return          others  read data from ring buffer failed
*     @note
*
*/
AUI_RTN_CODE aui_common_ring_buf_rd(aui_ring_buf* p_ring_buf,unsigned long ul_buf_len,unsigned long *pul_real_data_len,unsigned char *p_data_out)
{
    unsigned long ul_buf_len_tmp=0;
    unsigned long ul_data_len=0;

    if((NULL==p_ring_buf->pby_ring_buf)||(NULL==p_data_out)||(NULL==pul_real_data_len))
    {

        //*pul_real_data_len = 0;
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    if(SUCCESS!=aui_common_ring_buf_data_len(p_ring_buf,&ul_data_len))
    {
        *pul_real_data_len = 0;
        aui_rtn(AUI_RTN_FAIL,"get ring buffer data len fail");
    }

    if((ul_buf_len>ul_data_len))
    {
        ul_buf_len_tmp=ul_data_len;
    }
    else
    {
        ul_buf_len_tmp=ul_buf_len;
    }
    AUI_TAKE_SEM(p_ring_buf->sem_ring);
    if(ul_buf_len_tmp<= p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd)
    {
        osal_cache_invalidate(p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_rd,ul_buf_len_tmp);
        MEMCPY(p_data_out,p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_rd,ul_buf_len_tmp);
        p_ring_buf->ul_ring_buf_rd+=ul_buf_len_tmp;
        if(p_ring_buf->ul_ring_buf_rd==p_ring_buf->ul_ring_buf_len)
        {
            p_ring_buf->ul_ring_buf_rd=0;
        }
    }
    else
    {
        osal_cache_invalidate(p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_rd,p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd);
        MEMCPY(p_data_out,p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_rd,p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd);
        osal_cache_invalidate(p_ring_buf->pby_ring_buf,ul_buf_len_tmp-(p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd));
        MEMCPY(p_data_out +p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd ,
        p_ring_buf->pby_ring_buf,ul_buf_len_tmp-(p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd));
        p_ring_buf->ul_ring_buf_rd=(ul_buf_len_tmp-(p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_rd));
    }
    *pul_real_data_len = ul_buf_len_tmp;
    AUI_GIVE_SEM(p_ring_buf->sem_ring);
    return AUI_RTN_SUCCESS;
}

/**
*     @brief         write data to ring buffer
*     @author        ray.gong
*     @date              2013-5-21
*     @param[in]        p_ring_buf ring buffer handle
*     @param[in]        ul_buf_len read data length
*     @param[out]    p_data_in point to the data write to ring buffer
*      @return          AUI_RNT_SUCCESS write data to ring buffer successful
*      @return          AUI_RTN_EINVAL  write data to ring buffer failed,because input parameters invalid
*      @return          OS_RING_WT_NO_BUF  the remain space in ring buffer is less than want to write length
*      @return          others  write data to ring buffer failed
*     @note
*
*/
AUI_RTN_CODE aui_common_ring_buf_wt(aui_ring_buf* p_ring_buf,unsigned long ul_buf_len,unsigned char *p_data_in)
{
    unsigned long ul_buf_remain=0;

    if((NULL==p_ring_buf->pby_ring_buf)||(NULL==p_data_in))
    {
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    if(SUCCESS!=aui_common_ring_buf_remain(p_ring_buf,&ul_buf_remain))
    {
        aui_rtn(AUI_RTN_FAIL,"get ring buffer remain data fail");
    }

    if(ul_buf_len>ul_buf_remain)
    {
        aui_rtn(AUI_RTN_RING_WT_NO_BUF,"(ul_buf_len>ul_buf_remain)");
    }
    AUI_TAKE_SEM(p_ring_buf->sem_ring);
    if(ul_buf_len<= p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt)
    {
        MEMCPY(p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_wt,p_data_in,ul_buf_len);
        osal_cache_flush(p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_wt,ul_buf_len);
        p_ring_buf->ul_ring_buf_wt+=ul_buf_len;
        if(p_ring_buf->ul_ring_buf_wt==p_ring_buf->ul_ring_buf_len)
        {
            p_ring_buf->ul_ring_buf_wt=0;
        }
    }
    else
    {
        MEMCPY(p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_wt,p_data_in,p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt);
        osal_cache_flush(p_ring_buf->pby_ring_buf+p_ring_buf->ul_ring_buf_wt,p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt);
        MEMCPY(p_ring_buf->pby_ring_buf,p_data_in +p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt,
        ul_buf_len-(p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt));
        osal_cache_flush(p_ring_buf->pby_ring_buf,ul_buf_len-(p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt));
        p_ring_buf->ul_ring_buf_wt=(ul_buf_len-(p_ring_buf->ul_ring_buf_len-p_ring_buf->ul_ring_buf_wt));
    }
    AUI_GIVE_SEM(p_ring_buf->sem_ring);
    return AUI_RTN_SUCCESS;
}


static AUI_RTN_CODE check_dev_exist(struct aui_list_head *tar_dev,struct aui_list_head *head_dev)
{
    struct aui_list_head *tmp_next=NULL;
    struct aui_list_head *tmp_prev=NULL;
    struct aui_list_head *cur=NULL;

    cur=head_dev;
    tmp_next=cur->next;
    tmp_prev=cur->prev;

    list_mutex_lock(1);
    do
    {
        if(tar_dev==cur)
        {
            list_mutex_lock(0);
            return AUI_RTN_SUCCESS;
        }
        if(NULL!=cur)
        {
            tmp_prev=cur->next;
        }
        cur=cur->next;
        if(NULL!=cur)
        {
            tmp_next=cur->next;
        }
    }while(cur!=NULL);

    list_mutex_lock(0);
    (void)tmp_next;
    (void)tmp_prev;


    return AUI_RTN_FAIL;
}

AUI_RTN_CODE add_dev_2_list(struct aui_list_head *tar_dev,struct aui_list_head *head_dev)
{
    struct aui_list_head *tmp_next=NULL;
    struct aui_list_head *tmp_prev=NULL;
    struct aui_list_head *cur=NULL;
    //AUI_PRINTF("\r\n tar[%08x] head[%08x]",tar_dev,head_dev);
    if((NULL==tar_dev)||(NULL==head_dev))
    {
        return AUI_RTN_FAIL;
    }

    if(AUI_RTN_SUCCESS==check_dev_exist(tar_dev,head_dev))
    {
        return AUI_RTN_FAIL;
    }

    cur=head_dev;
    tmp_next=cur->next;
    tmp_prev=cur->prev;

    list_mutex_lock(1);
    do
    {
        if(NULL==tmp_next)
        {
            tar_dev->prev=tmp_prev;
            cur->next=tar_dev;
            tar_dev->next=NULL;
            list_mutex_lock(0);
            return AUI_RTN_SUCCESS;
        }
        if(NULL!=cur)
        {
            tmp_prev=cur->next;
        }
        cur=cur->next;
        if(NULL!=cur)
        {
            tmp_next=cur->next;
        }
    }while(cur!=NULL);
    list_mutex_lock(0);
    return AUI_RTN_FAIL;
}

AUI_RTN_CODE del_dev_from_list(struct aui_list_head *tar_dev,struct aui_list_head *head_dev)
{
    struct aui_list_head *tmp_next=NULL;
    struct aui_list_head *tmp_prev=NULL;
    struct aui_list_head *cur=NULL;

    if((NULL==tar_dev)||(NULL==head_dev))
    {
        return AUI_RTN_FAIL;
    }
    cur=head_dev;
    tmp_next=cur->next;
    tmp_prev=cur->prev;

    list_mutex_lock(1);
    do
    {
        if(tar_dev==cur)
        {
            if((NULL==cur->next)&&(NULL==cur->prev))
            {
                head_dev->next=NULL;
            }
            else if((NULL==cur->next)&&(NULL!=cur->prev))
            {
                cur->prev->next=NULL;
            }
            else if((NULL!=cur->next)&&(NULL==cur->prev))
            {
                cur->next->prev=NULL;
                head_dev->next=cur->next;
            }
            else
            {
                cur->prev->next=cur->next;
                cur->next->prev=cur->prev;
                cur->next=NULL;
                cur->prev=NULL;
            }
            list_mutex_lock(0);
            return AUI_RTN_SUCCESS;
        }
        if(NULL!=cur)
        {
            tmp_prev=cur->next;
        }
        cur=cur->next;
        if(NULL!=cur)
        {
            tmp_next=cur->next;
        }
    }while(cur!=NULL);

    list_mutex_lock(0);
    (void) tmp_next;
    (void) tmp_prev;

    return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_find_dev_by_idx(aui_module_id dev_type,unsigned long ul_dev_idx,aui_hdl *p_aui_hdl)
{
    AUI_RTN_CODE aui_rtn_code=AUI_RTN_FAIL;
    struct aui_list_head *tmp_next=NULL;
    struct aui_list_head *tmp_prev=NULL;
    struct aui_list_head *cur=NULL;
    struct aui_list_head head_dev;
#if 1

    if(dev_type>=AUI_MODULE_LAST)
    {
        return AUI_RTN_FAIL;
    }
    //AUI_PRINTF("\r\n head dev[%d]=[%08x]",dev_type,head_dev);
    /*
    tds version use AUI_MODULE_DIS_HD,AUI_MODULE_DIS_SD enum before.
    To maintain compatibility with previous versions,we didn't remove them in aui_error_stb.h
    if remove these in future version,next if-else statement  should be removed.
    */
    /*AUI_MODULE_DIS translate begin*/
    /*
    if(AUI_MODULE_DIS_HD==dev_type)
    {
        dev_type = AUI_MODULE_DIS;
        ul_dev_idx = AUI_DIS_HD;
    }
    else if(AUI_MODULE_DIS_SD==dev_type)
    {
        dev_type = AUI_MODULE_DIS;
        ul_dev_idx = AUI_DIS_SD;
    }
    */
    head_dev=(g_all_dev_hdl.lst_dev[dev_type]);

    /*AUI_MODULE_DIS translate end*/
#else
    switch(dev_type)
    {
        case AUI_MODULE_DECA:
        {
            head_dev=&(g_all_dev_hdl.lst_dev_deca);
            break;
        }
        case AUI_MODULE_SND:
        {
            head_dev=&(g_all_dev_hdl.lst_dev_snd);
            break;
        }
        case AUI_MODULE_DMX:
        {
            head_dev=&(g_all_dev_hdl.lst_dev_dmx);
            break;
        }
    }

#endif
    if(NULL==head_dev.next)
    {
        return AUI_RTN_FAIL;
    }
    cur=head_dev.next;
    tmp_next=cur->next;
    tmp_prev=cur->prev;
    list_mutex_lock(1);
    do
    {
        //AUI_PRINTF("\r\n cur dev[%d]=[%08x]",((aui_dev_priv_data *)cur)->dev_idx,cur,cur);
        if(ul_dev_idx==((aui_dev_priv_data *)cur)->dev_idx)
        {
            *p_aui_hdl=cur;
            list_mutex_lock(0);
            return AUI_RTN_SUCCESS;
        }

        tmp_prev=cur->next;
        cur=cur->next;
        if(NULL!=cur)
        {
            tmp_next=cur->next;
        }
    }while(cur!=NULL);

    list_mutex_lock(0);
    (void)tmp_next;
    (void)tmp_prev;


    return aui_rtn_code;
}



AUI_RTN_CODE aui_dev_reg(aui_module_id dev_type,aui_hdl dev_hdl)
{
    struct aui_list_head *p_list_tar=NULL;
    AUI_RTN_CODE aui_rtn_code=AUI_RTN_FAIL;
    //AUI_PRINTF("\r\nline[%d]dev_type,dev_hdl=[%08x][%08x]",__LINE__,dev_type,dev_hdl);
    if(NULL==dev_hdl)
    {
        return AUI_RTN_FAIL;
    }
    p_list_tar=dev_hdl;

    //AUI_PRINTF("\r\nline[%d]dev_type=[%08x]",__LINE__,dev_type);
#if 1
    if(dev_type>=AUI_MODULE_LAST)
    {
        return AUI_RTN_FAIL;
    }
    aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev[dev_type]));
    if(AUI_RTN_SUCCESS!=aui_rtn_code)
    {
        goto FUNC_END;
    }
#else
    switch(dev_type)
    {
        case AUI_MODULE_DECA:
        {
            aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev_deca));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_SND:
        {
            aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev_snd));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_DMX:
        {

            aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev_dmx));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                //AUI_PRINTF("\r\nline[%d]aui_rtn_code=[%08x]",__LINE__,aui_rtn_code);
                goto FUNC_END;
            }
            //AUI_PRINTF("\r\nline[%d](g_all_dev_hdl.lst_dev_dmx)=[%08x]",__LINE__,(g_all_dev_hdl.lst_dev_dmx));
            break;
        }
        case AUI_MODULE_DMX_CHANNEL:
        {
            aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev_dmx_channel));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_DMX_FILTER:
        {
            aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev_dmx_filter));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_OS_TASK:
        {
            aui_rtn_code=add_dev_2_list(p_list_tar,&(g_all_dev_hdl.lst_dev_os_task));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
    }
#endif
FUNC_END:
    //AUI_PRINTF("\r\n aui_rtn_code=[%08x]",aui_rtn_code);
    return aui_rtn_code;
}

AUI_RTN_CODE aui_dev_unreg(aui_module_id dev_type,aui_hdl dev_hdl)
{
    struct aui_list_head *p_list_tar=NULL;
    AUI_RTN_CODE aui_rtn_code=AUI_RTN_FAIL;

    if(NULL==dev_hdl)
    {
        return AUI_RTN_FAIL;
    }
    p_list_tar=dev_hdl;

#if 1
    if(dev_type>=AUI_MODULE_LAST)
    {
        return AUI_RTN_FAIL;
    }
    aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev[dev_type]));
    if(AUI_RTN_SUCCESS!=aui_rtn_code)
    {
        goto FUNC_END;
    }
#else
    switch(dev_type)
    {
        case AUI_MODULE_DECA:
        {
            aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev_deca));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_SND:
        {
            aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev_snd));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_DMX:
        {
            aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev_dmx));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_DMX_CHANNEL:
        {
            aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev_dmx_channel));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_DMX_FILTER:
        {
            aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev_dmx_filter));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
        case AUI_MODULE_OS_TASK:
        {
            aui_rtn_code=del_dev_from_list(p_list_tar,&(g_all_dev_hdl.lst_dev_os_task));
            if(AUI_RTN_SUCCESS!=aui_rtn_code)
            {
                goto FUNC_END;
            }
            break;
        }
    }
#endif
FUNC_END:
    return aui_rtn_code;
}
AUI_RTN_CODE aui_handle_set_magic_num(aui_module_id dev_type_id,aui_hdl handle)
{
    if(0x80000000!=((unsigned long)handle&0xf0000000))
    {
        aui_rtn(AUI_RTN_EINVAL,"\r\n Invalid handle value.");
    }
    ((aui_dev_priv_data *)handle)->ul_dev_type_id=dev_type_id;
    ((aui_dev_priv_data *)handle)->ul_dev_magic_num=AUI_HANDLE_MAGIC_NUM+((aui_dev_priv_data *)handle)->ul_dev_type_id;

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_handle_check(aui_hdl handle)
{
    if(0x80000000!=((unsigned long)handle&0xf0000000))
    {
        aui_rtn(AUI_RTN_EINVAL,"\r\n Invalid handle value.");
    }
    if(AUI_HANDLE_MAGIC_NUM+((aui_dev_priv_data *)handle)->ul_dev_type_id!=((aui_dev_priv_data *)handle)->ul_dev_magic_num)
    {
        aui_rtn(AUI_RTN_EINVAL,"\r\n Dev magic number err!");
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE _aui_find_dev_list(aui_module_id dev_type,aui_hdl *p_list)
{
    if(NULL==p_list)
    {
        return AUI_RTN_FAIL;
    }

    if(dev_type>=AUI_MODULE_LAST)
    {
        return AUI_RTN_FAIL;
    }
    *p_list=g_all_dev_hdl.lst_dev[dev_type].next;

    return AUI_RTN_SUCCESS;
}

#ifndef AUI_SUBT_SUPPORT
//the flowing function is stub for ali lib
void ts_route_set_nim_tsiid(UINT8 nim_id, INT8 tsiid)
{
    (void)nim_id;
    (void)tsiid;
}

void get_local_time(date_time *dt)
{
    aui_hdl hdl_rtc=NULL;

    aui_clock time_now;

    MEMSET(&time_now,0,sizeof(time_now));

    aui_rtc_open(&hdl_rtc);

    if(NULL==hdl_rtc)
    {
        return;
    }

    if(AUI_RTN_SUCCESS!=aui_rtc_get_time(hdl_rtc,&time_now))
    {
        return;
    }
    dt->year=time_now.year;
    dt->month=time_now.month;
    dt->day=time_now.date;
    dt->weekday=time_now.day;
    dt->hour=time_now.hour;
    dt->min=time_now.min;
    dt->sec=time_now.sec;

}
#endif
