   
/****************************INCLUDE HEAD FILE************************************/
#include <errno.h>
#include <aui_os.h>
//#include <api/libc/list.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>
#include "aui_common_priv.h"

/****************************LOCAL MACRO******************************************/
#define EVENT_SET_AUTORESET(eventid) ((eventid) | 0x8000)
#define EVENT_CHECK_AUTORESET(eventid) ((eventid) & 0x8000)
#define EVENT_REMOVE_FLAGS(eventid) ((eventid) & 0x3fff)

/****************************LOCAL TYPE*******************************************/
// MUTEX functions
// porting from pthread win32

typedef struct aui_st_hdl_task
{
    /** Device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The global task mutex */
    unsigned long dev_mutex_id;
    /** The task ID return by OS layer */
    unsigned long ul_id;
    /** The TDS task priority value */
    unsigned long ul_tds_priority;
    /** The Task attribute setting */
    aui_attr_task task_attr;
    /** The mutex for user task, It is different for each task */
    unsigned long user_task_mutex_id;
    /** The current status of task */
    aui_task_status task_status;
    /** The user task's return value, it is valid when the task in status: AUI_TASK_STATUS_RETURN */
    AUI_RTN_CODE user_entry_rtn_val;
    OSAL_ID x_event_flag_id;
    unsigned long x_events;
    /** Reserved. */
    void *p_prev;
    /** Point to next task */
    void *p_next;

}aui_hdl_task,*aui_p_hdl_task;

typedef struct aui_st_hdl_mempool
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The memory pool's global mutex */
    unsigned long dev_mutex_id;
    /** The heap information in handle */
    aui_heap_info heap;
    /** The heap attribute */
    aui_attr_mempool mempool_attr;
}aui_hdl_mempool,*aui_pst_hdl_mempool;


/** The semaphare handle structure */
typedef struct aui_st_hdl_sem
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The semaphare global mutex */
    unsigned long dev_mutex_id;
    /** The semaphare ID return from OS layer */
    unsigned long ul_id;
    /** The current semaphare count */
    unsigned long ul_cnt;
    /** The semaphare attribute */
    aui_attr_sem sem_attr;
}aui_hdl_sem,*aui_pst_hdl_sem;


/** The mutex handle structure */
typedef struct aui_st_hdl_mutex
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The mutex's global mutex */
    unsigned long dev_mutex_id;
    /** The mutex ID return from OS layer */
    unsigned long ul_id;
    /** The current mutex count */
    unsigned long ul_cnt;
    /** The mutex attribute */
    aui_attr_mutex mutex_attr;
}aui_hdl_mutex,*aui_pst_hdl_mutex;

/** The event handle structure */
typedef struct aui_st_hdl_event
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The mutex's global mutex */
    unsigned long dev_mutex_id;
    /** The event ID return from OS layer */
    unsigned long ul_id;
    /** The event attribute */
    aui_attr_event event_attr;
}aui_hdl_event,*aui_pst_hdl_event;

/** The timer handle structure */
typedef struct aui_st_hdl_timer
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The timer's global mutex */
    unsigned long dev_mutex_id;
    /** The timer ID from OS layer */
    unsigned long ul_id;
    /** The timer attribute */
    aui_attr_timer timer_attr;
}aui_hdl_timer,*aui_pst_hdl_timer;

/** The message queue handle structure */
typedef struct aui_st_hdl_msgq
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The msgq's global mutex */
    unsigned long dev_mutex_id;
    /** The msgq from OS layer */
    unsigned long ul_id;
    /** The message queue attribute */
    aui_attr_msgq msgq_attr;
}aui_hdl_msgq,*aui_pst_hdl_msgq;

/** The muti-event handle structure */
typedef struct aui_st_hdl_muti_event
{
    /** The device private data structure */
    aui_dev_priv_data dev_priv_data;
    /** The muti-event's global mutex */
    unsigned long dev_mutex_id;
    /** muti-event ID from OS layer*/
    unsigned long ul_id;
    /** muti-event source task's ID */
    aui_hdl hdl_task_src;
    /** muti-event destination task's ID, if it is NULL, then it is the broadcast mode*/
    aui_hdl hdl_task_dst;
    /** The muti-event attribute */
    aui_attr_muti_event muti_event_attr;
}aui_hdl_muti_event,*aui_pst_hdl_muti_event;

struct cs_mutex
{
    struct aui_list_head list;
    unsigned long      handle;
    long    lock_idx;        /* provides exclusive access to mutex state
                          via the interlocked* mechanism.
                           0: unlocked/free.
                           1: locked - no other waiters.
                          -1: locked - with possible other waiters.
                       */
    int recursive_count;       /* number of unlocks a thread needs to perform
                              before the lock is released (recursive
                                  mutexes only). */
    int kind;            /* mutex type. */
    aui_hdl        event;
    unsigned long     owner_thread;
};

/** OS attribute */
typedef struct aui_st_os_init_attr
{

    /** return mode of user task */
    aui_task_rtn_mode task_rtn_mode;
    /** system large heap information */
    aui_heap_info large_heap;
    /** system small heap information */
    aui_heap_info min_heap;
    /** maximum user heap size */
    unsigned long malloc_limit_size;
    /** callback function for changing user task priority to system task priority */
    aui_p_fun_task_prio_adapt p_fun_task_prio_adapt;
    /** maximum user event number */
    unsigned short event_num;
    /** muti_event work mode */
    aui_muti_event_mode muti_event_mode;
    /** reserved */
    unsigned long reserve;
    /** module initialial flag, 0: not initialization*/
    unsigned long ulInitFlag;
    /** user event array count    */
    unsigned long event_array_num;
    /** total user event array number    */
    unsigned long event_total_num;
    /** user event array */
    unsigned long *event_array;
    /** sem work mode */
    aui_sem_work_mode sem_work_mode;
}aui_os_init_attr,*aui_p_os_init_attr;

typedef struct aui_st_os
{
    aui_os_init_attr aui_os_init_para;

    OSAL_ID root_task_mutex_id;
    /** The first task's handle address*/
    void *p_task_head;
    /** The first msgq's handle address */
    void *p_msgq_head;
    /** The first user mutex's handle address */
    void *p_mutex_head;
    /** The first user event handle address */
    void *p_event_head;

    ID *event_flg_id;

    struct aui_list_head mutex_list;
    
    struct aui_list_head task_list;

    unsigned long mutex_index;
    /** internal's mutex*/
    ID mutex_interlock;

    int msg_size[AUI_OS_MAX_MSGQUEUE_COUNT];

}aui_os,*aui_p_os;
/****************************LOCAL VAR********************************************/

AUI_MODULE(OS)

//static aui_OSMdl_attr s_OSMdl_attr;
static OSAL_ID s_mod_mutex_id_os=OSAL_INVALID_ID;

static aui_os g_os;
aui_p_fun_task_prio_adapt g_p_fun_task_prio_adapt=NULL;
extern unsigned long g_n_test_data;
extern unsigned long g_b_thread_end;
extern aui_dev_all g_all_dev_hdl;
/****************************LOCAL FUNC DECLEAR***********************************/
#define AUI_OS_LOG(x...)        //AUI_DEBUG(AUI_DEBUG_OS, ##x)

/****************************LOCAL FUNC IMPLEMENT***********************************/
static inline INT32 interlocked_exchange(INT32 *target, INT32 value)
{
    os_disable_interrupt();
    INT32 ret = *target;
    *target = value;
    os_enable_interrupt();
    return ret;
}

static inline INT32 interlocked_compare_exchange(INT32 *target, INT32 exchange, INT32 comperand)
{
    os_disable_interrupt();
    INT32 ret = *target;
    if(ret == comperand)
        *target = exchange;
    os_enable_interrupt();
    return ret;
}
static struct cs_mutex *aui_find_mutex_by_handle(UINT32 handle)
{
    struct cs_mutex *ret = NULL;
    struct aui_list_head *p;

    osal_mutex_lock(g_os.mutex_interlock, OSAL_WAIT_FOREVER_TIME);
    list_for_each(p, &g_os.mutex_list)
    {
        struct cs_mutex *mx = list_entry(p, struct cs_mutex, list);
        if(mx->handle == handle)
        {
            ret = mx;
            break;
        }
    }
    osal_mutex_unlock(g_os.mutex_interlock);

    return ret;
}
void *aui_malloc(aui_heap_info *heap, unsigned int u_size)
{
    void *ret = NULL;

    if((heap == NULL) || (u_size == 0))
    {
        ret = NULL;
        AUI_ERR("Invalid parameter\n");
        return ret;
    }

    ret = (void *)__malloc(heap->handle, u_size);

    if (ret != NULL) {
        AUI_DBG("malloc size = %d [0x%x], addr = 0x%x\n", u_size, u_size, ret);
        heap->total_num++;
        heap->total_malloced += u_size;
        __get_free_ramsize(heap->handle, (int*)&heap->free, (int*)&heap->max);        
    }

    return ret;
}


AUI_RTN_CODE aui_free(aui_heap_info *heap, void * pv_addr)
{
    UINT32 temp = (unsigned long)pv_addr;
    UINT32 align = (temp&0x7);
    
    if(heap == NULL)
    {
        aui_rtn(AUI_RTN_FAIL,"(heap == NULL)");
    }
    
    if(align)
    {
        AUI_ERR("_fatal ERROR: free addr not 8-aligned, addr=0x%08x!!!!!!!!!!!!!!!!!!!!!!!!!!", pv_addr);
        return AUI_RTN_FAIL;
    }
    
    if(pv_addr)
    {
        __free(heap->handle, pv_addr);
        __get_free_ramsize(heap->handle, (int*)&heap->free, (int*)&heap->max);
    }

    return AUI_RTN_SUCCESS;
}
/****************************MODULE DRV IMPLEMENT*************************************/



/****************************MODULE IMPLEMENT*************************************/

extern void aui_task_root(unsigned long ul_para1, unsigned long ul_para2);
extern FP os_get_current_thread_entry(void);

#define MAX_OS_LIST_TRT_CNT 5
#define MAX_OS_TASK_HANDLE 512
AUI_RTN_CODE aui_find_task_hdlby_id(aui_hdl p_head,OSAL_ID task_id,aui_hdl *pp_target)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    struct aui_list_head *p_list_tmp=NULL;
    struct aui_list_head *p_list_next=NULL;
    int list_try_cnt = 0;
    int fail_cnt = 0;
    p_head=(aui_hdl)(g_os.task_list.next);
    if((NULL==p_head)||(NULL==pp_target))
    {
        rtn=AUI_RTN_FAIL;
        AUI_ERR("Invalid parameters");
        return rtn;
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_head))
    {
        aui_rtn(AUI_RTN_EINVAL,"aui_handle_check(p_head)");
    }
    aui_hdl_task *p_head_tmp=(aui_hdl_task *)p_head;
    //aui_hdl_task *p_target_tmp=*(aui_hdl_task **)pp_target;
    aui_hdl_task *p_cur=p_head_tmp;


    if((unsigned long)os_get_current_thread_entry()!=(unsigned long)aui_task_root)
    {
        rtn=AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM;
        AUI_ERR("AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM");
        return rtn;
    }

    for(;;)
    {

        if(list_try_cnt>=MAX_OS_TASK_HANDLE)
        {
            list_try_cnt = 0;
            fail_cnt++;
            p_cur = (aui_hdl)(g_os.task_list.next);
        }

        if(fail_cnt>=MAX_OS_LIST_TRT_CNT)
        {
            AUI_ERR("fail_cnt>=MAX_OS_LIST_TRT_CNT");
            return AUI_RTN_FAIL;
        }

        if(AUI_RTN_SUCCESS!=aui_handle_check(p_cur))
        {
            list_try_cnt = 0;
            p_cur = (aui_hdl)(g_os.task_list.next);
            fail_cnt++;
            AUI_ERR("aui_handle_check fail");
        }

        if((task_id==p_cur->ul_id)&&(AUI_TASK_STATUS_RUN==p_cur->task_status))
        {
            *pp_target=p_cur;
            break;
        }

        p_list_tmp=(struct aui_list_head *)p_cur;
        p_list_next=p_list_tmp->next;

        p_cur=(aui_hdl_task *)(p_list_next);
        list_try_cnt++;

    }
    return AUI_RTN_SUCCESS;
}

static unsigned long s_ul_root_task_sleep=1;
void aui_task_root(unsigned long ul_para1, unsigned long ul_para2)
{
    aui_hdl_task *pst_hdl_task=NULL;
    AUI_RTN_CODE aui_rtn=AUI_RTN_FAIL;
    osal_mutex_lock(g_os.root_task_mutex_id,OSAL_WAIT_FOREVER_TIME);

    (void)ul_para2;

    AUI_DBG("\r\n start root task in time[%08x]",osal_get_tick());
    pst_hdl_task = (aui_hdl_task*)ul_para1;
    for(;;)
    {
        if(AUI_RTN_SUCCESS==aui_handle_check(pst_hdl_task))
        {
            if(AUI_RTN_SUCCESS==aui_handle_check(pst_hdl_task))
            {
                if(AUI_RTN_SUCCESS==aui_handle_check(pst_hdl_task))
                {
                    break;
                }
            }
        }
        else
        {
            osal_task_sleep(s_ul_root_task_sleep);
        }
    }

    pst_hdl_task->task_status=AUI_TASK_STATUS_START;
    osal_mutex_lock(pst_hdl_task->user_task_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(g_os.root_task_mutex_id);

    if(NULL==(pst_hdl_task->task_attr).p_fun_task_entry)
    {
        pst_hdl_task->task_status=AUI_TASK_STATUS_START_FAIL;
        goto ERR_END;
    }
    pst_hdl_task->task_status=AUI_TASK_STATUS_RUN;

    aui_rtn=(pst_hdl_task->task_attr).p_fun_task_entry((void*)((pst_hdl_task->task_attr).para1), (void*)((pst_hdl_task->task_attr).para2));
    if(AUI_RTN_SUCCESS!=aui_rtn)
    {

        pst_hdl_task->user_entry_rtn_val=aui_rtn;
        pst_hdl_task->task_status=AUI_TASK_STATUS_RTN_ERR;
        goto ERR_END;
    }

    pst_hdl_task->user_entry_rtn_val=aui_rtn;
    pst_hdl_task->task_status=AUI_TASK_STATUS_RTN_OK;

    if(AUI_TASK_RTN_MODE_AUTO_FREE==g_os.aui_os_init_para.task_rtn_mode)
    {


        aui_list_del(&pst_hdl_task->dev_priv_data.list);
        /*
        if(AUI_RTN_SUCCESS!=aui_dev_unreg(AUI_MODULE_OS_TASK,pst_hdl_task))
        {
            goto ERR_END;
        }
*/

        if(INVALID_ID==pst_hdl_task->user_task_mutex_id)
        {
            AUI_ERR("\r\n[%s][%d]",__FUNCTION__,__LINE__);
            return;
        }
        if(0!=osal_mutex_delete(pst_hdl_task->user_task_mutex_id))
        {
            AUI_ERR("\r\n[%s][%d]",__FUNCTION__,__LINE__);
        }
        (pst_hdl_task->user_task_mutex_id)=INVALID_ID;

        if(INVALID_ID!=(pst_hdl_task->dev_mutex_id))
        {
            if(0!=osal_mutex_delete(pst_hdl_task->dev_mutex_id))
            {
                AUI_ERR("\r\n[%s][%d]",__FUNCTION__,__LINE__);
            }
            (pst_hdl_task->dev_mutex_id)=INVALID_ID;
        }
        else
        {
            AUI_ERR("\r\n[%s][%d]",__FUNCTION__,__LINE__);
            return;
        }

        MEMSET(pst_hdl_task,0,sizeof(aui_hdl_task));
        FREE(pst_hdl_task);
        pst_hdl_task=NULL;
    }
    else if(AUI_TASK_RTN_MODE_MANUAL_FREE==g_os.aui_os_init_para.task_rtn_mode)
    {
        osal_mutex_unlock(pst_hdl_task->user_task_mutex_id);
    }
    else
    {
        osal_mutex_unlock(pst_hdl_task->user_task_mutex_id);
    }
    return;
ERR_END:

    if(AUI_TASK_RTN_MODE_AUTO_FREE==g_os.aui_os_init_para.task_rtn_mode)
    {
        aui_list_del(&pst_hdl_task->dev_priv_data.list);
/*
        if(AUI_RTN_SUCCESS!=aui_dev_unreg(AUI_MODULE_OS_TASK,pst_hdl_task))
        {
            goto ERR_END;
        }
*/
        if(INVALID_ID==pst_hdl_task->user_task_mutex_id)
        {
            return;
        }
        osal_mutex_unlock(pst_hdl_task->user_task_mutex_id);

        if(0!=osal_mutex_delete(pst_hdl_task->user_task_mutex_id))
        {
            AUI_ERR("osal_mutex_delete fail");
        }
        (pst_hdl_task->user_task_mutex_id)=INVALID_ID;

        if(INVALID_ID!=(pst_hdl_task->dev_mutex_id))
        {
            if(0!=osal_mutex_delete(pst_hdl_task->dev_mutex_id))
            {
                AUI_ERR("osal_mutex_delete fail");
            }
            (pst_hdl_task->dev_mutex_id)=INVALID_ID;
        }
        else
        {
            AUI_ERR("pst_hdl_task->dev_mutex_id is INVALID_ID");
            return;
        }
        MEMSET(pst_hdl_task,0,sizeof(aui_hdl_task));
        FREE(pst_hdl_task);
        pst_hdl_task=NULL;
    }
    else if(AUI_TASK_RTN_MODE_MANUAL_FREE==g_os.aui_os_init_para.task_rtn_mode)
    {
        osal_mutex_unlock(pst_hdl_task->user_task_mutex_id);
    }
    else
    {
        osal_mutex_unlock(pst_hdl_task->user_task_mutex_id);
    }
    return;

}

static void aui_timer_cb(UINT para)
{
    aui_hdl_timer *hdl_timer = (aui_hdl_timer *)para;

    if(hdl_timer->timer_attr.timer_call_back)
    {
        hdl_timer->timer_attr.timer_call_back(hdl_timer->timer_attr.ul_para1);
    }
    if(hdl_timer->timer_attr.ul_type == 1) //alarm_handler will release timer handle automatically
    {
        hdl_timer->ul_id = 0;
    }
}

/**
*     @brief         Set the callback that let use change the task priority to TDS's priority
*
*/
AUI_RTN_CODE aui_os_priority_switch(aui_p_fun_task_prio_adapt p_fun_task_prio_adapt)
{
    g_p_fun_task_prio_adapt=p_fun_task_prio_adapt;
    
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_version_get(unsigned long* const pul_version)
{
    if(NULL==pul_version)
    {
        aui_rtn(AUI_RTN_EINVAL,"pul_version invalid");
    }
    *pul_version=AUI_MODULE_VERSION_NUM_OS;
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_init(aui_attr_os *p_os_mod_attr)
{
    //unsigned long event_num=32;
    unsigned long i=0;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    if((NULL==p_os_mod_attr))
    {
        aui_rtn(AUI_RTN_EINVAL, "p_os_mod_attr invalid");
    }
    s_mod_mutex_id_os=osal_mutex_create();
    if(INVALID_ID==s_mod_mutex_id_os)
    {
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    MEMSET(&g_os,0,sizeof(aui_os));

    g_os.aui_os_init_para.task_rtn_mode=p_os_mod_attr->task_rtn_mode;
    MEMCPY(&(g_os.aui_os_init_para.large_heap),&(p_os_mod_attr->large_heap),sizeof(aui_heap_info));
    MEMCPY(&(g_os.aui_os_init_para.min_heap),&(p_os_mod_attr->min_heap),sizeof(aui_heap_info));
    g_os.aui_os_init_para.malloc_limit_size=p_os_mod_attr->malloc_limit_size;
    g_os.aui_os_init_para.p_fun_task_prio_adapt=p_os_mod_attr->p_fun_task_prio_adapt;
    g_os.aui_os_init_para.event_num=p_os_mod_attr->event_num;
    g_os.aui_os_init_para.muti_event_mode=p_os_mod_attr->muti_event_mode;
    g_os.aui_os_init_para.sem_work_mode=p_os_mod_attr->sem_work_mode;
    g_os.aui_os_init_para.reserve=p_os_mod_attr->reserve;

    if((g_os.aui_os_init_para.large_heap.base != 0) && (g_os.aui_os_init_para.large_heap.len > 0))
    {
        __alloc_initialize(&g_os.aui_os_init_para.large_heap.handle, 
                            g_os.aui_os_init_para.large_heap.base,
                            g_os.aui_os_init_para.large_heap.base+g_os.aui_os_init_para.large_heap.len);
    }
    if((g_os.aui_os_init_para.min_heap.base != 0) && (g_os.aui_os_init_para.min_heap.len > 0))
    {
        __alloc_initialize(&g_os.aui_os_init_para.min_heap.handle, 
                            g_os.aui_os_init_para.min_heap.base, 
                            g_os.aui_os_init_para.min_heap.base+g_os.aui_os_init_para.min_heap.len);
    }


    g_os.root_task_mutex_id=osal_mutex_create();
    if(0==g_os.root_task_mutex_id)
    {
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    //g_os.task_rtn_mode=p_os_mod_attr->task_rtn_mode;

    AUI_INIT_LIST_HEAD(&(g_os.mutex_list));
    AUI_INIT_LIST_HEAD(&(g_os.task_list));

    g_os.mutex_index = 1;
    g_os.mutex_interlock = OSAL_INVALID_ID;

    g_os.aui_os_init_para.event_total_num = p_os_mod_attr->event_num ;
    g_os.aui_os_init_para.event_array_num = (g_os.aui_os_init_para.event_total_num + 31) >> 5;
    //if(g_pOSInfo->event_array_num > flg_total_num)
    //      ASSERT(0);
    g_os.aui_os_init_para.event_array  = (unsigned long *)malloc(g_os.aui_os_init_para.event_array_num * sizeof(unsigned long));
    if(g_os.aui_os_init_para.event_array  == NULL)
    {
        ASSERT(0);
        aui_rtn(AUI_RTN_ENOMEM,"g_os.aui_os_init_para.event_array malloc fail");
    }
    g_os.event_flg_id = (ID *)malloc(g_os.aui_os_init_para.event_array_num * sizeof(ID));
    if( g_os.event_flg_id == NULL)
    {
        ASSERT(0);
        aui_rtn(AUI_RTN_ENOMEM,"g_os.event_flg_id malloc fail");
    }
    for(i = 0; i < g_os.aui_os_init_para.event_array_num; i++)
    {
        g_os.aui_os_init_para.event_array [i] = 0;
        g_os.event_flg_id[i] = os_create_flag(0);
        
        if(INVALID_ID == g_os.event_flg_id[i])
        {
            ASSERT(0);
            aui_rtn(AUI_RTN_FAIL,"os_create_flag failed");
        }
    }


    //return p_call_back_init(pv_param);
    return AUI_RTN_SUCCESS;
}




AUI_RTN_CODE aui_os_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    if((NULL==p_call_back_init))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_call_back_init is null");
    }
    if(E_OK!=osal_mutex_delete(s_mod_mutex_id_os))
    {
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    return p_call_back_init(pv_param);
}



AUI_RTN_CODE aui_os_task_create(const aui_attr_task *p_task_attr,aui_hdl* const p_hdl_task)
{
    OSAL_T_CTSK st_tsk;
    OSAL_ID task_id=OSAL_INVALID_ID;
    OSAL_ID dev_mutex_id=INVALID_ID;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_hdl_task *hdl_task=NULL;


    AUI_ENTER_FUNC(AUI_MODULE_OS);
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);
    if((NULL==p_hdl_task)||(NULL==p_task_attr))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"\r\n[506]input para err.");
    }
    if(NULL==p_task_attr->p_fun_task_entry)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"\r\n[511]input para err.");
    }
    hdl_task=(aui_hdl_task *)MALLOC(sizeof(aui_hdl_task));
    if(NULL==hdl_task)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"\r\n[518]system no mem.");
    }
    MEMSET(hdl_task,0,sizeof(aui_hdl_task));
    hdl_task->dev_mutex_id=dev_mutex_id;
    st_tsk.stksz        = p_task_attr->ul_stack_size;
    st_tsk.quantum      = p_task_attr->ul_quantum;
    if(NULL==g_p_fun_task_prio_adapt)
    {
        st_tsk.itskpri      = p_task_attr->ul_priority;
    }
    else
    {
        st_tsk.itskpri      =(g_p_fun_task_prio_adapt)(p_task_attr->ul_priority);
    }
    st_tsk.name[0]      = p_task_attr->sz_name[0];
    st_tsk.name[1]      = p_task_attr->sz_name[1];
    st_tsk.name[2]      = p_task_attr->sz_name[2];
    st_tsk.task         = (FP)aui_task_root;//(p_task_attr->p_fun_task_entry);
    (hdl_task->user_task_mutex_id)=osal_mutex_create();
    if(INVALID_ID==(hdl_task->user_task_mutex_id))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,"\r\n[511]create mutex err.");
    }
    MEMCPY(&(hdl_task->task_attr),p_task_attr,sizeof(aui_attr_task));
    st_tsk.para1        = (unsigned long)hdl_task;
    st_tsk.para2     =  0;//(unsigned long)(p_task_attr->para2);

    hdl_task->ul_tds_priority=st_tsk.itskpri;
    hdl_task->user_task_mutex_id=(hdl_task->user_task_mutex_id);
    MEMCPY(&(hdl_task->task_attr),p_task_attr,sizeof(aui_attr_task));

    hdl_task->dev_mutex_id=dev_mutex_id;

    task_id     =      osal_task_create(&st_tsk);
    if(task_id==OSAL_INVALID_ID)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,"\r\n[583]create task err.");
    }
    hdl_task->ul_id=task_id;

    aui_handle_set_magic_num(AUI_MODULE_OS,hdl_task);
//    aui_dev_reg(AUI_MODULE_OS_TASK,hdl_task);
    aui_list_add(&hdl_task->dev_priv_data.list, &g_os.task_list);
    hdl_task->dev_priv_data.en_status=AUI_DEV_STATUS_OPEN;
    *p_hdl_task=hdl_task;
    AUI_DBG("\r\n CREATE TASK[%08x]=[%08x]:[%08x]\r\n",p_hdl_task,*p_hdl_task,hdl_task);
    osal_mutex_unlock(dev_mutex_id);

    rtn =  AUI_RTN_SUCCESS;
    return rtn;
}


AUI_RTN_CODE aui_os_task_delete(aui_hdl *p_hdl_task,unsigned long ul_force_kill)
{
    aui_hdl_task **ppst_hdl_task_tmp=NULL;
    aui_hdl_task *pst_hdl_task=NULL;

    if((INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"INVALID_ID==s_mod_mutex_id_os");
    }


    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_task))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_task");
    }
    ppst_hdl_task_tmp = (aui_hdl_task **)p_hdl_task;
    if(AUI_RTN_SUCCESS!=aui_handle_check(*ppst_hdl_task_tmp))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"p_hdl_task not exist");
    }
    pst_hdl_task=*ppst_hdl_task_tmp;
    AUI_DBG("\r\n BEFORE DELETE TASK[%08x]->[%08x]\r\n",ppst_hdl_task_tmp,pst_hdl_task);
    
    osal_mutex_lock(pst_hdl_task->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);
    if(AUI_DEV_STATUS_OPEN!=pst_hdl_task->dev_priv_data.en_status)
    {
        aui_rtn(AUI_RTN_EINVAL, "AUI_DEV_STATUS_OPEN!=pst_hdl_task->dev_priv_data.en_status");
    }
    if(ul_force_kill)
    {
        if(AUI_TASK_STATUS_RUN!=pst_hdl_task->task_status)
        {
            ;
        }
        else
        {
            if(INVALID_ID!=(pst_hdl_task->user_task_mutex_id))
            {
                if(0!=osal_mutex_delete(pst_hdl_task->user_task_mutex_id))
                {
                    AUI_ERR("osal_mutex_delete fail");
                }
                (pst_hdl_task->user_task_mutex_id)=INVALID_ID;
            }

            if(INVALID_ID!=(pst_hdl_task->dev_mutex_id))
            {
                if(0!=osal_mutex_delete(pst_hdl_task->dev_mutex_id))
                {
                    AUI_ERR("osal_mutex_delete fail");
                }
                (pst_hdl_task->dev_mutex_id)=INVALID_ID;
            }


            if(RET_SUCCESS!=osal_task_delete(pst_hdl_task->ul_id))
            {
                osal_mutex_unlock(pst_hdl_task->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,"\r\n osal del task failed!");
            }
        }
    }
    else
    {
        if(/*(AUI_TASK_STATUS_IDLE==((aui_hdl_task *)p_hdl_task)->task_status)
            ||*/(AUI_TASK_STATUS_RUN==pst_hdl_task->task_status))
        {
            osal_task_sleep(2);
        }
        if(/*(AUI_TASK_STATUS_IDLE==((aui_hdl_task *)p_hdl_task)->task_status)
            ||*/(AUI_TASK_STATUS_RUN==pst_hdl_task->task_status))
        {
            osal_mutex_unlock(pst_hdl_task->dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL,"\r\n task is running!");
        }
    }
    ////osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    ////osal_mutex_unlock(pst_hdl_task->dev_mutex_id);

    ////if(NULL==pst_hdl_task)
    ////{
    ////    osal_mutex_unlock(s_mod_mutex_id_os);
    ////    aui_rtn(AUI_RTN_FAIL,"\r\n_handle is NULL.");
    ////}
    pst_hdl_task->dev_priv_data.en_status=AUI_DEV_STATUS_CLOSED;
    if(!((ul_force_kill)&&(AUI_TASK_STATUS_RUN==pst_hdl_task->task_status)))
    {
        if(INVALID_ID!=(pst_hdl_task->user_task_mutex_id))
        {
            if(0!=osal_mutex_delete(pst_hdl_task->user_task_mutex_id))
            {
                AUI_ERR("osal_mutex_delete fail");
            }
            (pst_hdl_task->user_task_mutex_id)=INVALID_ID;
        }
        if(INVALID_ID!=(pst_hdl_task->dev_mutex_id))
        {
            if(0!=osal_mutex_delete(pst_hdl_task->dev_mutex_id))
            {
                AUI_ERR("osal_mutex_delete fail");
            }
            (pst_hdl_task->dev_mutex_id)=INVALID_ID;
        }
    }
    
    aui_list_del(&pst_hdl_task->dev_priv_data.list);
/*
    if(AUI_RTN_SUCCESS!=aui_dev_unreg(AUI_MODULE_OS_TASK,pst_hdl_task))
    {
        ////osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_MODULE_OS,AUI_ERR_GENERIC,"\r\n_delete handle list err.");
    }
    */
    MEMSET(pst_hdl_task,0,sizeof(aui_hdl_task));
    FREE(pst_hdl_task);
    //p_hdl_task=NULL;
    pst_hdl_task=NULL;
    *p_hdl_task=NULL;

    AUI_DBG("\r\n AFTER DELETE TASK[%08x]->[%08x]\r\n",ppst_hdl_task_tmp,pst_hdl_task);
    ////osal_mutex_unlock(s_mod_mutex_id_os);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_task_attr_set(aui_hdl p_hdl_task,const aui_attr_task *p_task_attr)
{

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_task)||(NULL==p_task_attr))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_task)||(NULL==p_task_attr)");
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_hdl_task))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_hdl_task is not exist");
    }
    osal_mutex_lock(((aui_hdl_task *)p_hdl_task)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);



    osal_mutex_unlock(((aui_hdl_task *)p_hdl_task)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_task_attr_get(aui_hdl p_hdl_task,aui_attr_task* const p_task_attr)
{

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_task)||(NULL==p_task_attr))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_task)||(NULL==p_task_attr))");
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_hdl_task))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"p_hdl_task is not exist");
    }
    osal_mutex_lock(((aui_hdl_task *)p_hdl_task)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);
    MEMCPY(p_task_attr,&(((aui_hdl_task *)p_hdl_task)->task_attr),sizeof(aui_attr_task));
    strncpy(p_task_attr->sz_name,(((aui_hdl_task *)p_hdl_task)->task_attr).sz_name,AUI_DEV_NAME_LEN);
    osal_mutex_unlock(((aui_hdl_task *)p_hdl_task)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_task_sleep(unsigned long ul_time_ms)
{

    //AUI_ENTER_FUNC(AUI_MODULE_OS);
    osal_task_sleep(ul_time_ms);
    //AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_task_yield(aui_hdl p_hdl_task)
{
    if((NULL==p_hdl_task))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_task");
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_hdl_task))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_hdl_task is not exist");
    }

    osal_task_sleep((((aui_hdl_task *)p_hdl_task)->task_attr).ul_quantum);

    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_task_get_self_hdl(aui_hdl* const pp_hdl_task)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);


    if(NULL==pp_hdl_task)
    {

        aui_rtn(AUI_RTN_FAIL,"\r\ninput task handle null");
    }

    rtn=aui_find_task_hdlby_id((g_os.p_task_head),osal_task_get_current_id(),pp_hdl_task);
    if(AUI_RTN_SUCCESS!=rtn)
    {
        if(rtn == (AUI_RTN_CODE)AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM)
        {
            *pp_hdl_task = (aui_hdl)((unsigned long)osal_task_get_current_id());
        }

        AUI_ERR("AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM");
        return rtn;
    }

    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_task_id_get(aui_hdl hdl_task, unsigned long *task_id)
{
    aui_hdl_task *p_hdl = NULL;
        
    AUI_INFO("Enter");

    if (task_id == NULL) {
        aui_rtn(AUI_RTN_EINVAL, "Invalid parameter.");
    }

    if((hdl_task != NULL) && (AUI_RTN_SUCCESS!=aui_handle_check(hdl_task)))
    {
        aui_rtn(AUI_RTN_EINVAL,"hdl_task is not exist");
    }

    if (hdl_task) {
        // Get task id from specified task
        p_hdl = hdl_task;

        *task_id = (unsigned long)p_hdl->ul_id;

        AUI_DBG("Got task id from specified task 0x%X, id = %d\n", p_hdl, *task_id);
        
    } else {
        *task_id = (unsigned long)osal_task_get_current_id();
        AUI_DBG("Got task id of current task id = %d\n", p_hdl, *task_id);
    }
    
    AUI_INFO("Leave");

    return AUI_RTN_SUCCESS;
}


/**
*     @brief         Get all user task handles list, and task count
*    @warning       Only for ALi internal use
*/
AUI_RTN_CODE aui_os_task_get_all_hdl(aui_hdl p_head,aui_hdl_task **pp_hdl_task_list,unsigned long *pul_usr_task_cnt,unsigned long ul_buffer_size)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    unsigned long ul_idx=0;
    struct aui_list_head *p_list_tmp=NULL;
    struct aui_list_head *p_list_next=NULL;
//    p_head=(aui_hdl)(g_all_dev_hdl.lst_dev[AUI_MODULE_OS_TASK].next);
    
    p_head=(aui_hdl)(g_os.task_list.next);
    if((NULL==p_head)||(NULL==pp_hdl_task_list)||(NULL==pul_usr_task_cnt)||(0==ul_buffer_size))
    {
        if((NULL!=pul_usr_task_cnt))
        {
             *pul_usr_task_cnt=0;
        }
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_head))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_head is not exist");
    }
    aui_hdl_task *p_head_tmp=(aui_hdl_task *)p_head;
    //aui_hdl_task *p_target_tmp=*(aui_hdl_task **)pp_target;
    aui_hdl_task *p_cur=p_head_tmp;

    *pul_usr_task_cnt=0;
    for(;;)
    {
        if(AUI_RTN_SUCCESS!=aui_handle_check(p_cur))
        {
            AUI_ERR("\r\n not find task id in time[%08x]",osal_get_tick());
            break;
        }

        *pul_usr_task_cnt=*pul_usr_task_cnt+1;
        //AUI_DBG("\r\n*pul_usr_task_cnt=[%d]",*pul_usr_task_cnt);
        pp_hdl_task_list[ul_idx++]=p_cur;
        //AUI_DBG("\r\npp_hdl_task_list[%d]=[%08x]",ul_idx-1,pp_hdl_task_list[ul_idx-1]);
        //AUI_DBG("\r\n ul_idx=[%d][%d];",ul_idx,ul_buffer_size);
        if(ul_idx>=ul_buffer_size)
        {
            //AUI_DBG("\r\n break ul_idx=[%d][%d];",ul_idx,ul_buffer_size);
            break;
        }

        p_list_tmp=(struct aui_list_head *)p_cur;
        p_list_next=p_list_tmp->next;
        if(AUI_RTN_SUCCESS!=aui_handle_check(p_list_next))
        {
            AUI_ERR("check handle error\n");
            break;
        }
        p_cur=(aui_hdl_task *)(p_list_next);



    }
    *pul_usr_task_cnt=ul_idx;
    //AUI_DBG("\r\n last ul_idx=[%d];",ul_idx);
    return AUI_RTN_SUCCESS;
}
/**
*   @brief         Check the task handle which user specified exist or not.
*   @warning    Only for ALi internal use.
*/
AUI_RTN_CODE aui_os_user_task_hdl_check(aui_hdl p_target)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    struct aui_list_head *p_list_tmp=NULL;
    struct aui_list_head *p_list_next=NULL;

    aui_hdl p_head=(aui_hdl)(g_os.task_list.next);
//    aui_hdl p_head=(aui_hdl)(g_all_dev_hdl.lst_dev[AUI_MODULE_OS_TASK].next);

    if (NULL==p_head)
    {
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    if(AUI_RTN_SUCCESS!=aui_handle_check(p_head))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_head is not exist");
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_target))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_target is not exist");
    }

    aui_hdl_task *p_head_tmp=(aui_hdl_task *)p_head;
    aui_hdl_task *p_cur=p_head_tmp;

    for(;;)
    {
        if(NULL==p_cur)
        {
            rtn=AUI_RTN_FAIL;
            break;
        }
        if(p_target==p_cur)
        {
            return AUI_RTN_SUCCESS;
        }

        if(AUI_RTN_SUCCESS!=aui_handle_check(p_cur))
        {
            AUI_ERR("aui_handle_check error");
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        p_list_tmp=(struct aui_list_head *)p_cur;
        p_list_next=p_list_tmp->next;
        if(AUI_RTN_SUCCESS!=aui_handle_check(p_list_next))
        {
            AUI_ERR("aui_handle_check error");
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        p_cur=(aui_hdl_task *)(p_list_next);
        if(AUI_RTN_SUCCESS!=aui_handle_check(p_cur))
        {
            AUI_ERR("aui_handle_check error");
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
    }
    rtn=AUI_RTN_FAIL;
    return rtn;
}



AUI_RTN_CODE aui_os_task_cnt(unsigned long* const pul_task_cnt)
{
    aui_hdl_task ***ppp_cur=NULL;
    aui_hdl_task **pp_cur=NULL;
    unsigned long ul_usr_task_cnt=0;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pul_task_cnt))
    {
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pul_task_cnt)");
    }

    ppp_cur=(aui_hdl_task ***)&(g_os.p_task_head);
    /*if(NULL==ppp_cur) //bug detective
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }*/
    pp_cur=*ppp_cur;
    //AUI_DBG("\r\n1ppp_cur[%08x]-[%08x]",ppp_cur,pp_cur);
    while(1)
    {
        if(NULL==pp_cur)
        {
            break;
        }
        if(NULL==ppp_cur)
        {
            //osal_mutex_unlock(s_mod_mutex_id_os);
            rtn=AUI_RTN_FAIL;
        return rtn;
        }
        ppp_cur=(aui_hdl_task ***)(&((*(*ppp_cur))->p_next));
    /*    if(NULL==ppp_cur)
        {
            //osal_mutex_unlock(s_mod_mutex_id_os);
            rtn=AUI_RTN_FAIL;
        return rtn;
        }*/
        pp_cur=*ppp_cur;
        ul_usr_task_cnt++;

    }

    *pul_task_cnt=ul_usr_task_cnt;

    //osal_mutex_unlock(s_mod_mutex_id_os);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_task_join(aui_hdl p_hdl_task,unsigned long ul_time_out)
{
    unsigned long ul_start_time=0;
    unsigned long ul_end_time=0;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if(NULL==p_hdl_task)
    {
        aui_rtn(AUI_RTN_FAIL,"\r\ninput task handle null");
    }
    if(AUI_RTN_SUCCESS!=aui_handle_check(p_hdl_task))
    {
        aui_rtn(AUI_RTN_EINVAL,"p_hdl_task is not exist");
    }
    ul_start_time=osal_get_time();
    while(1)
    {
        ul_end_time=osal_get_time();
        if((ul_time_out>0)&&(ul_end_time-ul_start_time>=ul_time_out))
        {
            if((AUI_TASK_STATUS_RTN_OK==((aui_hdl_task *)p_hdl_task)->task_status)
            ||(AUI_TASK_STATUS_RTN_ERR==((aui_hdl_task *)p_hdl_task)->task_status))
            {
                return AUI_RTN_SUCCESS;
            }
            else
            {
                return AUI_RTN_FAIL;
            }
        }
        if((AUI_TASK_STATUS_RTN_OK==((aui_hdl_task *)p_hdl_task)->task_status)
            ||(AUI_TASK_STATUS_RTN_ERR==((aui_hdl_task *)p_hdl_task)->task_status))
        {
            break;
        }
        osal_task_sleep(10);
    }
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_msgq_create(const aui_attr_msgq *p_msgq_attr,aui_hdl* const pp_hdl_msgq)
{
    OSAL_T_CMBF st_msgq;
    OSAL_ID msg_qid=OSAL_INVALID_ID;
    //aui_hdl_msgq hdl_msgq;
    OSAL_ID dev_mutex_id=INVALID_ID;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_hdl_msgq **pp_out_msgq=(aui_hdl_msgq**)pp_hdl_msgq;
    aui_hdl_msgq *pst_hdl_msgq=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

    if((NULL==pp_hdl_msgq)||(NULL==p_msgq_attr))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==pp_hdl_msgq)||(NULL==p_msgq_attr))");
    }
    if((0==p_msgq_attr->ul_buf_size)||(0==p_msgq_attr->ul_msg_size_max))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"((0==p_msgq_attr->ul_buf_size)||(0==p_msgq_attr->ul_msg_size_max))");
    }
    pst_hdl_msgq=(aui_hdl_msgq *)MALLOC(sizeof(aui_hdl_msgq));
    if(NULL==pst_hdl_msgq)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pst_hdl_msgq)");
    }

    pst_hdl_msgq->dev_mutex_id=dev_mutex_id;
    st_msgq.bufsz     = p_msgq_attr->ul_buf_size
                    +(p_msgq_attr->ul_buf_size/p_msgq_attr->ul_msg_size_max
                    +(p_msgq_attr->ul_buf_size%p_msgq_attr->ul_msg_size_max==0?0:1))*8;
    st_msgq.maxmsz      = p_msgq_attr->ul_msg_size_max;
    st_msgq.name[0]    = p_msgq_attr->sz_name[0];
    st_msgq.name[1]    = p_msgq_attr->sz_name[1];
    st_msgq.name[2]    = p_msgq_attr->sz_name[2];
    MEMCPY(&(pst_hdl_msgq->msgq_attr),p_msgq_attr,sizeof(aui_attr_msgq));

    msg_qid     =      osal_msgqueue_create(&st_msgq);
    if(msg_qid==OSAL_INVALID_ID)
    {
        FREE(pst_hdl_msgq);
        osal_mutex_unlock(dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    pst_hdl_msgq->ul_id=msg_qid;
    MEMCPY(&(pst_hdl_msgq->msgq_attr),p_msgq_attr,sizeof(aui_attr_msgq));
    pst_hdl_msgq->dev_mutex_id=dev_mutex_id;
    pst_hdl_msgq->dev_priv_data.en_status=AUI_DEV_STATUS_OPEN;
    *pp_out_msgq = pst_hdl_msgq;

    osal_mutex_unlock(dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_msgq_delete(aui_hdl *pp_hdl_msgq)
{
    aui_hdl_msgq **pp_hdl_msgq_tmp=NULL;
    aui_hdl_msgq *p_hdl_msgq=NULL;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((OSAL_INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"((OSAL_INVALID_ID==s_mod_mutex_id_os))");
    }

    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pp_hdl_msgq))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pp_hdl_msgq)");
    }
    pp_hdl_msgq_tmp=(aui_hdl_msgq **)pp_hdl_msgq;
    p_hdl_msgq=*pp_hdl_msgq_tmp;
    if((NULL==p_hdl_msgq)||(INVALID_ID==p_hdl_msgq->dev_mutex_id))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_msgq)||(INVALID_ID==p_hdl_msgq->dev_mutex_id))");
    }
    osal_mutex_lock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);
    if(AUI_DEV_STATUS_OPEN!=p_hdl_msgq->dev_priv_data.en_status)
    {
        aui_rtn(AUI_RTN_EINVAL,"\r\n sem handle have been del.");
    }
    if(RET_SUCCESS!=osal_msgqueue_delete(p_hdl_msgq->ul_id))
    {
        osal_mutex_unlock(p_hdl_msgq->dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }


    p_hdl_msgq->dev_priv_data.en_status=AUI_DEV_STATUS_CLOSED;
    if(0!=osal_mutex_delete(p_hdl_msgq->dev_mutex_id))
    {
        AUI_ERR("osal_mutex_delete fail");
    }


#if 1
    /*if(NULL==p_hdl_msgq)
    {
        aui_rtn(AUI_RTN_FAIL,"\r\n_handle is NULL.");
    }*/

    MEMSET(p_hdl_msgq,0,sizeof(aui_hdl_msgq));
    FREE(p_hdl_msgq);
    //p_hdl_msgq=NULL;
    *((aui_hdl_msgq **)pp_hdl_msgq)=NULL;
#endif

    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}




AUI_RTN_CODE aui_os_msgq_rcv(aui_hdl p_hdl_msgq,void *pst_msg, unsigned long ul_msg_size,
                            unsigned long* const pul_actual_size, unsigned long ul_time_out)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    ER err=OSAL_E_FAIL;

    (void)ul_msg_size;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_msgq)||(NULL==pst_msg)||(NULL==pul_actual_size))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_msgq)||(NULL==pst_msg)||(NULL==pul_actual_size))");
    }

    //osal_mutex_lock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);
    err=osal_msgqueue_receive(pst_msg,(long*)pul_actual_size,(((aui_hdl_msgq *)p_hdl_msgq)->ul_id),ul_time_out);
    if(RET_SUCCESS!=err)
    {
        //osal_mutex_unlock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id);
        if(E_TIMEOUT==err)
        {
            rtn=AUI_RTN_OS_MSGQ_TIMEOUT;
        }
        else
        {
            rtn=AUI_RTN_FAIL;
        }
        return rtn;
    }

    //osal_mutex_unlock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_msgq_snd(aui_hdl p_hdl_msgq,void *pst_msg, unsigned long ul_msg_size,unsigned long ul_time_out)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    ER err=OSAL_E_FAIL;
    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_msgq)||(NULL==pst_msg))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_msgq)||(NULL==pst_msg))");
    }

    //osal_mutex_lock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);
    err=osal_msgqueue_send((((aui_hdl_msgq *)p_hdl_msgq)->ul_id),pst_msg,ul_msg_size,ul_time_out);
    if(RET_SUCCESS!=err)
    {
        //osal_mutex_unlock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id);
        if(E_TIMEOUT==err)
        {
            rtn=AUI_RTN_OS_MSGQ_TIMEOUT;
        }
        else
        {
            rtn=AUI_RTN_FAIL;
        }
        return rtn;
    }

    //osal_mutex_unlock(((aui_hdl_msgq *)p_hdl_msgq)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_msgq_rst(aui_hdl p_hdl_msgq)
{
    (void)p_hdl_msgq;
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_msgq_attr_get(aui_hdl p_hdl_msgq,aui_attr_msgq* const p_msgq_attr)
{

    if((NULL==p_hdl_msgq)||(NULL==p_msgq_attr))
    {
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_msgq)||(NULL==p_msgq_attr))");
    }
    MEMCPY(p_msgq_attr,&(((aui_hdl_msgq *)p_hdl_msgq)->msgq_attr),sizeof(aui_attr_msgq));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_msgq_attr_set(aui_hdl p_hdl_msgq,const aui_attr_msgq* p_msgq_attr)
{

    if((NULL==p_hdl_msgq)||(NULL==p_msgq_attr))
    {
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_msgq)||(NULL==p_msgq_attr))");
    }
    MEMCPY((void*)p_msgq_attr,&(((aui_hdl_msgq *)p_hdl_msgq)->msgq_attr),sizeof(aui_attr_msgq));
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_msgq_cnt_get(aui_hdl p_hdl_msgq,unsigned long* const p_msgq_cnt)
{

    if((NULL==p_hdl_msgq)||(NULL==p_msgq_cnt))
    {
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_msgq)||(NULL==p_msgq_cnt))");
    }
    *p_msgq_cnt = ((aui_hdl_msgq *)p_hdl_msgq)->ul_id;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_sem_create(const aui_attr_sem *p_sem_attr,aui_hdl* const pp_hdl_sem)
{
    OSAL_ID sem_id=OSAL_INVALID_ID;
    //aui_hdl_sem hdl_sem;
    OSAL_ID dev_mutex_id=INVALID_ID;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_hdl_sem * pst_hdl_sem=NULL;

    //aui_hdl_sem ***ppp_cur=NULL;
    //aui_hdl_sem **pp_cur=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

    if((NULL==pp_hdl_sem)||(NULL==p_sem_attr))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==pp_hdl_sem)||(NULL==p_sem_attr))");
    }

    pst_hdl_sem=(aui_hdl_sem *)MALLOC(sizeof(aui_hdl_sem));
    if(NULL==pst_hdl_sem)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pst_hdl_sem)");
    }

    pst_hdl_sem->dev_mutex_id=dev_mutex_id;

    MEMCPY(&(pst_hdl_sem->sem_attr),p_sem_attr,sizeof(aui_attr_sem));

    sem_id       =     osal_semaphore_create(p_sem_attr->ul_max_val);
    if(sem_id==OSAL_INVALID_ID)
    {
        FREE(pst_hdl_sem);
        osal_mutex_unlock(dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    pst_hdl_sem->ul_id=sem_id;
    MEMCPY(&(pst_hdl_sem->sem_attr),p_sem_attr,sizeof(aui_attr_sem));
    pst_hdl_sem->dev_mutex_id=dev_mutex_id;
    pst_hdl_sem->ul_cnt=p_sem_attr->ul_max_val;
    pst_hdl_sem->dev_priv_data.en_status=AUI_DEV_STATUS_OPEN;
    *pp_hdl_sem=pst_hdl_sem;
    pst_hdl_sem = NULL;
    osal_mutex_unlock(dev_mutex_id);

    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_sem_delete(aui_hdl *pp_hdl_sem)
{
    aui_hdl_sem **pp_hdl_sem_tmp=NULL;
    aui_hdl_sem *p_hdl_sem=NULL;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((OSAL_INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"(OSAL_INVALID_ID==s_mod_mutex_id_os)");
    }

    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pp_hdl_sem))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pp_hdl_sem)");
    }
    pp_hdl_sem_tmp=(aui_hdl_sem **)pp_hdl_sem;
    p_hdl_sem=*pp_hdl_sem_tmp;
    if((NULL==p_hdl_sem))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_sem)");
    }
    osal_mutex_lock(p_hdl_sem->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);
    if(AUI_DEV_STATUS_OPEN!=p_hdl_sem->dev_priv_data.en_status)
    {
        aui_rtn(AUI_RTN_EINVAL,"\r\n sem handle have been del.");
    }
    if(RET_SUCCESS!=osal_semaphore_delete(p_hdl_sem->ul_id))
    {
        osal_mutex_unlock(p_hdl_sem->dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    p_hdl_sem->dev_priv_data.en_status=AUI_DEV_STATUS_CLOSED;
    if(0!=osal_mutex_delete(p_hdl_sem->dev_mutex_id))
    {
        AUI_ERR("osal_mutex_delete fail");
    }

#if 1
    /*if(NULL==p_hdl_sem)
    {
        aui_rtn(AUI_RTN_FAIL,"\r\n_handle is NULL.");
    }*/

    MEMSET(p_hdl_sem,0,sizeof(aui_hdl_sem));
    FREE(p_hdl_sem);
    //p_hdl_sem=NULL;
    *((aui_hdl_sem **)pp_hdl_sem)=NULL;
#endif

    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}




AUI_RTN_CODE aui_os_sem_wait(aui_hdl p_hdl_sem,unsigned long ul_time_out)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    ER err=OSAL_E_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_sem))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_sem)");
    }
    if(AUI_SEM_MODE_RELEASE_CHECK_MAX_CNT==g_os.aui_os_init_para.sem_work_mode)
    {
        //osal_mutex_lock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
        //osal_mutex_unlock(s_mod_mutex_id_os);
        err=osal_semaphore_capture((((aui_hdl_sem *)p_hdl_sem)->ul_id),ul_time_out);
        if(RET_SUCCESS!=err)
        {
            //osal_mutex_unlock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id);
            if(E_TIMEOUT==err)
            {
                rtn=AUI_RTN_OS_SEM_TIMEOUT;
            }
            else
            {
                rtn=AUI_RTN_FAIL;
            }

            return rtn;
        }

        //osal_mutex_unlock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id);
        osal_task_dispatch_off();
        if((((aui_hdl_sem *)p_hdl_sem)->ul_cnt)>0)
        {
            (((aui_hdl_sem *)p_hdl_sem)->ul_cnt)--;
            (((aui_hdl_sem *)p_hdl_sem)->sem_attr).ul_cnt=(((aui_hdl_sem *)p_hdl_sem)->ul_cnt);
        }
        else
        {
            osal_task_dispatch_on();
            aui_rtn(AUI_RTN_FAIL,"((p_hdl_sem)->ul_cnt)<=0");
        }
        osal_task_dispatch_on();
    }
    else if(AUI_SEM_MODE_RELEASE_NO_CHECK_MAX_CNT==g_os.aui_os_init_para.sem_work_mode)
    {
        err=osal_semaphore_capture((((aui_hdl_sem *)p_hdl_sem)->ul_id),ul_time_out);
        if(RET_SUCCESS!=err)
        {
            //osal_mutex_unlock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id);
            if(E_TIMEOUT==err)
            {
                rtn=AUI_RTN_OS_SEM_TIMEOUT;
            }
            else
            {
                rtn=AUI_RTN_FAIL;
            }

            return rtn;
        }
    }

    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_sem_release(aui_hdl p_hdl_sem)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_sem))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_sem)");
    }

    //osal_mutex_lock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);
    if(AUI_SEM_MODE_RELEASE_CHECK_MAX_CNT==g_os.aui_os_init_para.sem_work_mode)
    {
        osal_task_dispatch_off();

        (((aui_hdl_sem *)p_hdl_sem)->ul_cnt)++;
        (((aui_hdl_sem *)p_hdl_sem)->sem_attr).ul_cnt=(((aui_hdl_sem *)p_hdl_sem)->ul_cnt);
        osal_task_dispatch_on();

        if(RET_SUCCESS!=osal_semaphore_release(((aui_hdl_sem *)p_hdl_sem)->ul_id))
        {
            osal_task_dispatch_off();

            if((((aui_hdl_sem *)p_hdl_sem)->ul_cnt)>0)
            {
                (((aui_hdl_sem *)p_hdl_sem)->ul_cnt)--;
            }
            (((aui_hdl_sem *)p_hdl_sem)->sem_attr).ul_cnt=(((aui_hdl_sem *)p_hdl_sem)->ul_cnt);
            osal_task_dispatch_on();

            //osal_mutex_unlock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return rtn;
        }
    }
    else if(AUI_SEM_MODE_RELEASE_NO_CHECK_MAX_CNT==g_os.aui_os_init_para.sem_work_mode)
    {
        if(RET_SUCCESS!=osal_semaphore_release2(((aui_hdl_sem *)p_hdl_sem)->ul_id))
        {
            //osal_mutex_unlock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return rtn;
        }
    }

    //osal_mutex_unlock(((aui_hdl_sem *)p_hdl_sem)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_sem_attr_get(aui_hdl p_hdl_sem,aui_attr_sem* const p_sem_attr)
{

    if((NULL==p_hdl_sem)||(NULL==p_sem_attr))
    {
        aui_rtn(AUI_RTN_EINVAL,"((NULL==p_hdl_sem)||(NULL==p_sem_attr))");
    }
    p_sem_attr->ul_cnt=(((aui_hdl_sem *)p_hdl_sem)->ul_cnt);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_mutex_create(const aui_attr_mutex *p_mutex_attr,aui_hdl* const pp_hdl_mutex)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    //OSAL_ID mutex_id=OSAL_INVALID_ID;
    aui_hdl_mutex hdl_mutex;
    OSAL_ID dev_mutex_id=INVALID_ID;
    ID mutex_id=INVALID_ID;
    aui_attr_event event_attr;
    //aui_hdl_mutex ***ppp_cur=NULL;
    //aui_hdl_mutex **pp_cur=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

    if((NULL==pp_hdl_mutex)||(NULL==p_mutex_attr))
    {
        //osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==pp_hdl_mutex)||(NULL==p_mutex_attr))");
    }

    MEMSET(&hdl_mutex,0,sizeof(aui_hdl_mutex));
    MEMSET(&event_attr,0,sizeof(aui_attr_event));

    *((aui_hdl_mutex **)pp_hdl_mutex)=(aui_hdl_mutex *)MALLOC(sizeof(aui_hdl_mutex));
    if(NULL==(*((aui_hdl_mutex **)pp_hdl_mutex)))
    {
        //osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM,"(NULL==(*((aui_hdl_mutex **)pp_hdl_mutex)))");
    }

    (*(aui_hdl_mutex **)pp_hdl_mutex)->dev_mutex_id=dev_mutex_id;

    MEMCPY(&((*((aui_hdl_mutex **)pp_hdl_mutex))->mutex_attr),p_mutex_attr,sizeof(aui_attr_mutex));
    if(AUI_MUTEX_TYPE_TDS==p_mutex_attr->mutex_type)
    {
        mutex_id     =       osal_mutex_create();
        if(mutex_id==OSAL_INVALID_ID)
        {
            //osal_mutex_unlock(dev_mutex_id);
            rtn=AUI_RTN_FAIL;
        return rtn;
        }
        hdl_mutex.ul_id=mutex_id;
    }
    else if((AUI_MUTEX_TYPE_LINUX_NORMAL==p_mutex_attr->mutex_type)||(AUI_MUTEX_TYPE_LINUX_NEST==p_mutex_attr->mutex_type))
    {
        struct cs_mutex *mx = NULL;

        //if the mutex for the interlock is invilid, create it;
        if(g_os.mutex_interlock == OSAL_INVALID_ID)
        {
            g_os.mutex_interlock = osal_mutex_create();
            if(g_os.mutex_interlock == OSAL_INVALID_ID)
            {
                //osal_mutex_unlock(dev_mutex_id);
                rtn=AUI_RTN_FAIL;
                return rtn;
            }
        }

        mx = MALLOC(sizeof(struct cs_mutex));
        if(!mx)
        {
            rtn=AUI_RTN_FAIL;
            return rtn;

        }
        MEMSET(mx, 0, sizeof(struct cs_mutex));

        mx->lock_idx = 0;
        mx->recursive_count = 0;
        mx->owner_thread = 0;
        if(AUI_MUTEX_TYPE_LINUX_NORMAL==p_mutex_attr->mutex_type)
        {
            mx->kind = AUI_MUTEX_NORMAL;
        }
        else if(AUI_MUTEX_TYPE_LINUX_NEST==p_mutex_attr->mutex_type)
        {
            mx->kind = AUI_MUTEX_RECURSIVE;
        }
        event_attr.b_auto_reset=TRUE;
        event_attr.b_initial_state=FALSE;
        #if 1
        if(AUI_RTN_SUCCESS!=aui_os_event_create(&event_attr, &(mx->event)))
        {
            //osal_mutex_unlock(dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        #else
        mx->event = osal_event_create(TRUE, FALSE);
        #endif
        osal_mutex_lock(g_os.mutex_interlock, OSAL_WAIT_FOREVER_TIME);
        mx->handle = g_os.mutex_index++;
        aui_list_add(&mx->list, &g_os.mutex_list);
        osal_mutex_unlock(g_os.mutex_interlock);

        hdl_mutex.ul_id=mx->handle;
    }

    MEMCPY(&(hdl_mutex.mutex_attr),p_mutex_attr,sizeof(aui_attr_mutex));
    MEMCPY((*((aui_hdl_mutex **)pp_hdl_mutex)),&hdl_mutex,sizeof(aui_hdl_mutex));
    (*(aui_hdl_mutex **)pp_hdl_mutex)->dev_mutex_id=dev_mutex_id;
    //osal_mutex_unlock(dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_mutex_delete(aui_hdl *pp_hdl_mutex)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_hdl_mutex **pp_hdl_mutex_tmp=NULL;
    aui_hdl_mutex *p_hdl_mutex=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((OSAL_INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"(OSAL_INVALID_ID==s_mod_mutex_id_os)");
    }

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pp_hdl_mutex))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pp_hdl_mutex)");
    }
    pp_hdl_mutex_tmp=(aui_hdl_mutex **)pp_hdl_mutex;
    p_hdl_mutex=*pp_hdl_mutex_tmp;
    if((NULL==p_hdl_mutex))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_mutex)");
    }
    //osal_mutex_lock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    if(AUI_MUTEX_TYPE_TDS==(p_hdl_mutex->mutex_attr).mutex_type)
    {
        if(RET_SUCCESS!=osal_mutex_delete(p_hdl_mutex->ul_id))
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }

    }
    else if((AUI_MUTEX_TYPE_LINUX_NORMAL==(p_hdl_mutex->mutex_attr).mutex_type)
           ||(AUI_MUTEX_TYPE_LINUX_NEST==(p_hdl_mutex->mutex_attr).mutex_type))
    {
        int result = 0;
        struct cs_mutex *mx = aui_find_mutex_by_handle(p_hdl_mutex->ul_id);
        if(mx == NULL)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }

        aui_hdl self = 0;
        AUI_RTN_CODE aui_rtn_code1 =aui_os_task_get_self_hdl(&self);
        if((aui_rtn_code1 != AUI_RTN_SUCCESS)&&(aui_rtn_code1 != AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM))
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }

    destory_retry:
        result = aui_os_mutex_try_lock((aui_hdl_mutex *)p_hdl_mutex);

        /*
        * if trylock succeeded and the mutex is not recursively locked it
        * can be destroyed.
        */
        if (result == 0)
        {
            if ((mx->kind != AUI_MUTEX_RECURSIVE) || (1 == mx->recursive_count))
            {
                /*
                * FIXME!!!
                * the mutex isn't held by another thread but we could still
                * be too late invalidating the mutex below since another thread
                * may already have entered mutex_lock and the check for a valid
                * *mutex != NULL.
                *
                * note that this would be an unusual situation because it is not
                * common that mutexes are destroyed while they are still in
                * use by other threads.
                */

                result = aui_os_mutex_unlock((aui_hdl_mutex *)p_hdl_mutex);

                if (result == 0)
                {

                    if(AUI_RTN_SUCCESS!=aui_os_event_delete(&(mx->event)))
                    {
                        result = EINVAL;
                    }
                    else
                    {
                        osal_mutex_lock(g_os.mutex_interlock, OSAL_WAIT_FOREVER_TIME);
                        aui_list_del(&mx->list);
                        osal_mutex_unlock(g_os.mutex_interlock);
                        FREE(mx);
                    }
                }
                else
                {
                    /*
                    * restore the mutex before we return the error.
                    */
                    //*mutex = mx;
                    AUI_DBG("the mutex [handle = %d] destory failed!\n", ((aui_hdl_mutex *)p_hdl_mutex));
                }
            }
            else            /* mx->recursive_count > 1 */
            {
                /*
                * the mutex must be recursive and already locked by us (this thread).
                */
                mx->recursive_count--;      /* undo effect of pthread_mutex_trylock() above */
                aui_os_mutex_unlock (((aui_hdl_mutex *)p_hdl_mutex));
                goto destory_retry;
                //result = EBUSY;
            }
        }
        if(0!=result)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            if(E_TIMEOUT==result)
            {
                rtn=AUI_RTN_OS_MUTEX_TIMEOUT;
            }
            else if(EINVAL==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EINVAL;
            }
            else if(EDEADLK==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EDEADLK;
            }
            else if(EPERM==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EPERM;
            }
            else if(EBUSY==result)
            {
                rtn=AUI_RTN_OS_MUTEX_BUSY;
            }
            else
            {
                rtn=AUI_RTN_FAIL;
            }

            return rtn;

        }
    }

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
    if(NULL==((aui_hdl_mutex *)p_hdl_mutex))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_FAIL,"\r\n_handle is NULL.");
    }


#if 1
    MEMSET(p_hdl_mutex,0,sizeof(aui_hdl_mutex));
    FREE(p_hdl_mutex);
    //p_hdl_mutex=NULL;
    *((aui_hdl_mutex **)pp_hdl_mutex)=NULL;
#endif
    //osal_mutex_unlock(s_mod_mutex_id_os);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_mutex_lock(aui_hdl p_hdl_mutex,unsigned long ul_time_out)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    ER err=OSAL_E_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_mutex))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_mutex)");
    }

    //osal_mutex_lock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    if(AUI_MUTEX_TYPE_TDS==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type)
    {
        err=osal_mutex_lock(((aui_hdl_mutex *)p_hdl_mutex)->ul_id,ul_time_out);
        if(RET_SUCCESS!=err)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            if(E_TIMEOUT==err)
            {
                rtn=AUI_RTN_OS_MUTEX_TIMEOUT;
            }
            else
            {
                rtn=AUI_RTN_FAIL;
            }
            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return rtn;
        }
        else
        {
            osal_task_dispatch_off();
            ((aui_hdl_mutex *)p_hdl_mutex)->ul_cnt++;
            osal_task_dispatch_on();
        }

    }
    else if((AUI_MUTEX_TYPE_LINUX_NORMAL==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type)
            ||(AUI_MUTEX_TYPE_LINUX_NEST==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type))
    {
        int result = 0;
        struct cs_mutex *mx = aui_find_mutex_by_handle(((aui_hdl_mutex *)p_hdl_mutex)->ul_id);
        if(mx == NULL)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL,"(mx == NULL)");
        }

        aui_hdl self = NULL;
        AUI_RTN_CODE aui_rtn_code1 = (AUI_RTN_CODE)aui_os_task_get_self_hdl(&self);

        if((aui_rtn_code1 != AUI_RTN_SUCCESS)&&(aui_rtn_code1 != AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM))
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL,"aui_os_task_get_self_hdl failed");
        }

        if (mx->kind == AUI_MUTEX_NORMAL)
        {
            if (interlocked_exchange(&mx->lock_idx, 1) != 0)
            {
                while(interlocked_exchange(&mx->lock_idx, (INT32) -1) != 0)
                {
                    #if 1
                    rtn=aui_os_event_wait(&(mx->event),ul_time_out,0);
                    if(AUI_RTN_SUCCESS!=rtn)
                    {
                        result = rtn;
                        break;
                    }
                    #else
                    ER err = osal_event_wait(mx->event, ul_time_out);
                    if(err != E_OK)
                    {
                        result = err;
                        break;
                    }
                    #endif
                }
            }
        }
        else
        {
            if (interlocked_compare_exchange(&mx->lock_idx,  1,  0) == 0)
            {
                mx->recursive_count = 1;
                mx->owner_thread = (unsigned long)self;
            }
            else
            {
                if(mx->owner_thread == (unsigned long)self)
                {
                    if (mx->kind == AUI_MUTEX_RECURSIVE)
                    {
                        mx->recursive_count++;
                    }
                    else
                    {
                        result = EDEADLK;
                    }
                }
                else
                {
                    while(interlocked_exchange(&mx->lock_idx, (INT32) -1) != 0)
                    {
                        #if 1
                        rtn=aui_os_event_wait((mx->event),ul_time_out,0);
                        if(AUI_RTN_SUCCESS!=rtn)
                        {
                            result = rtn;
                            break;
                        }
                        #else
                        ER err = osal_event_wait(mx->event, ul_time_out);
                        if(err != E_OK)
                        {
                            result = err;
                            break;
                        }
                        #endif
                    }

                    if (0 == result)
                    {
                        mx->recursive_count = 1;
                        mx->owner_thread = (unsigned long)self;
                    }
                }
            }
        }

        if(0 != result)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            if(E_TIMEOUT==result)
            {
                rtn=AUI_RTN_OS_MUTEX_TIMEOUT;
            }
            else if(EINVAL==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EINVAL;
            }
            else if(EDEADLK==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EDEADLK;
            }
            else if(EPERM==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EPERM;
            }
            else if(EBUSY==result)
            {
                rtn=AUI_RTN_OS_MUTEX_BUSY;
            }
            else
            {
                if(rtn == (AUI_RTN_CODE)AUI_RTN_OS_EVENT_TIMEOUT)
                {
                    rtn=AUI_RTN_OS_MUTEX_TIMEOUT;
                }
            }

            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return rtn;
        }

    }

    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_mutex_unlock(aui_hdl p_hdl_mutex)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_mutex))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_mutex)");
    }

    //osal_mutex_lock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

     if(AUI_MUTEX_TYPE_TDS==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type)
    {
        if(0 == (((aui_hdl_mutex *)p_hdl_mutex)->ul_cnt))
        {
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        if(RET_SUCCESS!=osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->ul_id))
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        else
        {
            osal_task_dispatch_off();
            ((aui_hdl_mutex *)p_hdl_mutex)->ul_cnt--;
            osal_task_dispatch_on();
        }

    }
    else if((AUI_MUTEX_TYPE_LINUX_NORMAL==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type)
            ||(AUI_MUTEX_TYPE_LINUX_NEST==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type))
    {
        int result = 0;
        struct cs_mutex *mx = aui_find_mutex_by_handle(((aui_hdl_mutex *)p_hdl_mutex)->ul_id);
        if(mx == NULL)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL,"aui_find_mutex_by_handle failed");
        }

        aui_hdl self = 0;
        AUI_RTN_CODE aui_rtn_code1 =aui_os_task_get_self_hdl(&self);
        if((aui_rtn_code1 != AUI_RTN_SUCCESS)&&(aui_rtn_code1 != (AUI_RTN_CODE)(AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM)))
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL,"aui_os_task_get_self_hdl failed");
        }

        if (mx->kind == AUI_MUTEX_NORMAL)
        {
             INT32 idx;

            idx = interlocked_exchange(&mx->lock_idx, 0);
            if (idx != 0)
            {
                if (idx < 0)
                {
                    /*
                    * someone may be waiting on that mutex.
                    */
                    #if 1
                    rtn=aui_os_event_set((mx->event),0);
                    if(AUI_RTN_SUCCESS!=rtn)
                    {
                        result = EINVAL;
                    }
                    #else
                    if (osal_event_set(mx->event) != E_OK)
                    {
                        result = EINVAL;
                    }
                    #endif
                }
            }
            else
            {
                /*
                * was not locked (so can't be owned by us).
                */
                result = EPERM;
            }
        }
        else
        {
            if (mx->owner_thread == (unsigned long)self)
            {
                if ((mx->kind != AUI_MUTEX_RECURSIVE) || (0 == --mx->recursive_count))
                {
                    mx->owner_thread = 0;

                    if (interlocked_exchange(&mx->lock_idx, 0) < 0)
                    {
                        /* someone may be waiting on that mutex */
                        #if 1
                        rtn=aui_os_event_set((mx->event),0);
                        if(AUI_RTN_SUCCESS!=rtn)
                        {
                            result = EINVAL;
                        }
                        #else
                        if (osal_event_set(mx->event) != E_OK)
                        {
                            result = EINVAL;
                        }
                        #endif
                    }
                }
            }
            else
            {
                result = EPERM;
            }
        }
        if(0 != result)
        {
            //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
            if(E_TIMEOUT==result)
            {
                rtn=AUI_RTN_OS_MUTEX_TIMEOUT;
            }
            else if(EINVAL==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EINVAL;
            }
            else if(EDEADLK==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EDEADLK;
            }
            else if(EPERM==result)
            {
                rtn=AUI_RTN_OS_MUTEX_EPERM;
            }
            else if(EBUSY==result)
            {
                rtn=AUI_RTN_OS_MUTEX_BUSY;
            }
            else
            {
                if(rtn == (AUI_RTN_CODE)AUI_RTN_OS_EVENT_TIMEOUT)
                {
                    rtn = AUI_RTN_OS_MUTEX_TIMEOUT;
                }
            }

            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return rtn;
        }
    }
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;

}




AUI_RTN_CODE aui_os_mutex_try_lock(aui_hdl p_hdl_mutex)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_mutex))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_mutex");
    }

    //osal_mutex_lock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

     if(AUI_MUTEX_TYPE_TDS==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type)
    {

        aui_rtn(AUI_RTN_FEATURE_NOT_SUPPORTED, "mutex_type is AUI_MUTEX_TYPE_TDS");


    }
    else if((AUI_MUTEX_TYPE_LINUX_NORMAL==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type)
             ||(AUI_MUTEX_TYPE_LINUX_NEST==(((aui_hdl_mutex *)p_hdl_mutex)->mutex_attr).mutex_type))
    {
        int result = 0;
        struct cs_mutex *mx = aui_find_mutex_by_handle(((aui_hdl_mutex *)p_hdl_mutex)->ul_id);
        if(mx == NULL)
        {
            AUI_DBG("the mutex [handle = %d] doesn't exist or have been destoried!\n", (aui_hdl_mutex *)p_hdl_mutex);
            return AUI_RTN_EINVAL;
        }

        aui_hdl self = 0;
        AUI_RTN_CODE aui_rtn_code1 =aui_os_task_get_self_hdl(&self);
        if((aui_rtn_code1 != AUI_RTN_SUCCESS)&&(aui_rtn_code1 != (AUI_RTN_CODE)(AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM)))
        {
            aui_rtn(AUI_RTN_EINVAL,"aui_os_task_get_self_hdl failed");
        }

        if (0 == interlocked_compare_exchange(&mx->lock_idx, 1,  0))
        {
            if (mx->kind != AUI_MUTEX_NORMAL)
            {
                mx->recursive_count = 1;
                mx->owner_thread = (unsigned long)self;
            }
        }
        else
        {
            if((mx->kind == AUI_MUTEX_RECURSIVE) && (mx->owner_thread == (unsigned long)self))
            {
                mx->recursive_count++;
            }
            else
            {
                result = EBUSY;
            }
        }



        if(EBUSY==result)
        {
            rtn=AUI_RTN_OS_MUTEX_BUSY;
            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return rtn;
        }
    }

    //osal_mutex_unlock(((aui_hdl_mutex *)p_hdl_mutex)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;


}

AUI_RTN_CODE aui_os_mutex_attr_get(aui_hdl p_hdl_mutex,aui_attr_mutex* const p_mutex_attr)
{

    if((NULL==p_hdl_mutex)||(NULL==p_mutex_attr))
    {
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_mutex)");
    }

    p_mutex_attr->ul_cnt=(((aui_hdl_mutex *)p_hdl_mutex)->ul_cnt);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_event_create(const aui_attr_event *p_event_attr,aui_hdl* const pp_hdl_event)
{
    aui_hdl_event hdl_event;
    OSAL_ID dev_mutex_id=INVALID_ID;
    unsigned long i=0,j=0;
    ID event_id=INVALID_ID;
    BOOL get_event_flag = FALSE;

    //aui_hdl_event ***ppp_cur=NULL;
    //aui_hdl_event **pp_cur=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //AUI_DBG("\r\n [%08x][%d][%d] create event in ",osal_get_tick(),osal_task_get_current_id(),g_n_test_data);
    //AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

    if((NULL==pp_hdl_event)||(NULL==p_event_attr))
    {
        //osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==pp_hdl_event)||(NULL==p_event_attr))");
    }

    MEMSET(&hdl_event,0,sizeof(aui_hdl_event));

    *((aui_hdl_event **)pp_hdl_event)=(aui_hdl_event *)MALLOC(sizeof(aui_hdl_event));
    if(NULL==(*((aui_hdl_event **)pp_hdl_event)))
    {
        //osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM,"malloc failed");
    }

    (*(aui_hdl_event **)pp_hdl_event)->dev_mutex_id=dev_mutex_id;

    MEMCPY(&((*((aui_hdl_event **)pp_hdl_event))->event_attr),p_event_attr,sizeof(aui_attr_event));

    os_disable_dispatch();

    for(i = 0; i < g_os.aui_os_init_para.event_array_num; i++)
    {
        for(j = 0; j < sizeof(unsigned long)*8; j++)
        {
            if(!((1 << j)&g_os.aui_os_init_para.event_array [i]))
            {
                event_id = (i << 5) + j;
                g_os.aui_os_init_para.event_array [i] |= (1 << j);

                if(p_event_attr->b_auto_reset)
                {
                    event_id = EVENT_SET_AUTORESET(event_id);
                }
                if(p_event_attr->b_initial_state)
                {
                    if(E_OK != os_set_flag(g_os.event_flg_id[i], (1 << j)))
                    {
                        ASSERT(0);
                    }
                }
                else
                {
                    if(E_OK != os_clear_flag(g_os.event_flg_id[i], (1 << j)))
                    {
                        ASSERT(0);
                    }
                }

                //successfully get the event
                hdl_event.ul_id=event_id;
                get_event_flag = TRUE;
                break;
            }
        }

        // get one free event successfully, just break and return
        if (TRUE == get_event_flag)
        {
            break;
        }
    }

    // can not find free event, before return fail, need free some resource
    if(FALSE == get_event_flag)
    {
        if(NULL!=*((aui_hdl_event **)pp_hdl_event))
        {
            FREE(*((aui_hdl_event **)pp_hdl_event));
            *((aui_hdl_event **)pp_hdl_event)=NULL;
        }
        os_enable_dispatch();
        AUI_LEAVE_FUNC(AUI_MODULE_OS);
        return AUI_RTN_FAIL;
    }

    hdl_event.ul_id=event_id;//&0x0000ffff;
    MEMCPY(&(hdl_event.event_attr),p_event_attr,sizeof(aui_attr_event));
    MEMCPY((*((aui_hdl_event **)pp_hdl_event)),&hdl_event,sizeof(aui_hdl_event));
    (*(aui_hdl_event **)pp_hdl_event)->dev_mutex_id=dev_mutex_id;
    os_enable_dispatch();
    //osal_mutex_unlock(dev_mutex_id);
    //AUI_DBG("\r\n [%08x][%d][%d] create event out ",osal_get_tick(),osal_task_get_current_id(),g_nTestData);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_event_delete(aui_hdl *pp_hdl_event)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_hdl_event **pp_hdl_event_tmp=NULL;
    aui_hdl_event *p_hdl_event=NULL;
    unsigned long i=0, j=0;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((OSAL_INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"(OSAL_INVALID_ID==s_mod_mutex_id_os)");
    }

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pp_hdl_event))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pp_hdl_event)");
    }
    pp_hdl_event_tmp=(aui_hdl_event **)pp_hdl_event;
    p_hdl_event=*pp_hdl_event_tmp;
    if((NULL==p_hdl_event))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_event)");
    }
    //osal_mutex_lock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    (((aui_hdl_event *)p_hdl_event)->ul_id) = EVENT_REMOVE_FLAGS((((aui_hdl_event *)p_hdl_event)->ul_id));
    if((((aui_hdl_event *)p_hdl_event)->ul_id) > (g_os.aui_os_init_para.event_total_num - 1))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    i = (((aui_hdl_event *)p_hdl_event)->ul_id) >> 5;
    j = (((aui_hdl_event *)p_hdl_event)->ul_id) & 0x1f;

    os_disable_dispatch();

    if(!(g_os.aui_os_init_para.event_array [i] & (1 << j)))
    {
        os_enable_dispatch();
        return E_FAILURE;
    }
    g_os.aui_os_init_para.event_array [i] &= ~(1 << j);

    os_enable_dispatch();

    if(E_OK != os_set_flag(g_os.event_flg_id[i], (1 << j))) //notify orther suspended task
    {
        ASSERT(0);
    }


    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);

#if 1
    MEMSET(p_hdl_event,0,sizeof(aui_hdl_event));
    FREE(p_hdl_event);
    //p_hdl_event=NULL;
    *((aui_hdl_event **)pp_hdl_event)=NULL;
#endif
    //osal_mutex_unlock(s_mod_mutex_id_os);
    //AUI_DBG("\r\n [%08x][%d][%d] del event out ",osal_get_tick(),osal_task_get_current_id(),g_n_test_data);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_event_set(aui_hdl p_hdl_event,unsigned long b_enable)
{
    int i=0,j=0;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    unsigned long ul_id_tmp=INVALID_ID;

    (void)b_enable;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((NULL==p_hdl_event)||(0x80000000 != (((unsigned long)p_hdl_event)&(0xf0000000))))
    {
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_event)");
    }
    //AUI_DBG("\r\n [%08x][%d][%d] set event in ",osal_get_tick(),osal_task_get_current_id(),g_n_test_data);
    #if 0
    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_event))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,NULL);
    }

    osal_mutex_lock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);
    #endif
    ul_id_tmp=((((aui_hdl_event *)p_hdl_event)->ul_id));

    ul_id_tmp = EVENT_REMOVE_FLAGS(ul_id_tmp);

    if(ul_id_tmp > (g_os.aui_os_init_para.event_total_num - 1))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    i = ul_id_tmp >> 5;
    j = ul_id_tmp & 0x1f;

    if(!(g_os.aui_os_init_para.event_array [i] & (1 << j)))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    if(E_OK != os_set_flag(g_os.event_flg_id[i], (1 << j)))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}




AUI_RTN_CODE aui_os_event_wait(aui_hdl p_hdl_event,unsigned long ul_time_out,int is_auto_reset)
{
    int i=0,j=0;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    unsigned long ul_id_tmp=INVALID_ID;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((NULL==p_hdl_event)||(0x80000000 != (((unsigned long)p_hdl_event)&(0xf0000000))))
    {
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_event)");
    }

    ul_id_tmp=((((aui_hdl_event *)p_hdl_event)->ul_id));
    UINT flt_ptn = 0;
    UINT wfmode = TWF_ORW;
    if(EVENT_CHECK_AUTORESET(ul_id_tmp)||is_auto_reset)
    {
        wfmode |= TWF_CLR;
    }

    ul_id_tmp = EVENT_REMOVE_FLAGS(ul_id_tmp);
    if(ul_id_tmp > (g_os.aui_os_init_para.event_total_num - 1))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    i = ul_id_tmp >> 5;
    j = ul_id_tmp & 0x1f;

    if(!(g_os.aui_os_init_para.event_array [i] & (1 << j)))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    if(E_OK == os_wait_flag_time_out(&flt_ptn, g_os.event_flg_id[i], (1 << j), wfmode, ul_time_out))
    {
        if(!(g_os.aui_os_init_para.event_array [i] & (1 << j))) //event already deleted
        {
            if(E_OK != os_set_flag(g_os.event_flg_id[i], (1 << j))) //notify orther suspended task
            {
                ASSERT(0);
            }
            //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        else
        {
            //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
            AUI_LEAVE_FUNC(AUI_MODULE_OS);
            return AUI_RTN_SUCCESS;
        }
    }

    if(TMO_FEVR != ul_time_out)
    {
        rtn = AUI_RTN_OS_EVENT_TIMEOUT;
        return rtn;
    }

    //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
    rtn=AUI_RTN_FAIL;
    return rtn;

}



AUI_RTN_CODE aui_os_event_clear(aui_hdl p_hdl_event)
{
    int i=0,j=0;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_event)||(0x80000000!=(((unsigned long)p_hdl_event)&(0xf0000000))))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_event)");
    }

    //osal_mutex_lock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    (((aui_hdl_event *)p_hdl_event)->ul_id) = EVENT_REMOVE_FLAGS((((aui_hdl_event *)p_hdl_event)->ul_id));

    if((((aui_hdl_event *)p_hdl_event)->ul_id) > (g_os.aui_os_init_para.event_total_num - 1))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    i = (((aui_hdl_event *)p_hdl_event)->ul_id) >> 5;
    j = (((aui_hdl_event *)p_hdl_event)->ul_id) & 0x1f;

    if(!(g_os.aui_os_init_para.event_array [i] & (1 << j)))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    if(E_OK != os_clear_flag(g_os.event_flg_id[i], (1 << j)))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }


    //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;

}



AUI_RTN_CODE aui_os_event_info_get(aui_hdl p_hdl_event)
{
    int i=0,j=0;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_event)||(0x80000000!= (((unsigned long)p_hdl_event)&0xf0000000)))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_event)");
    }

    //osal_mutex_lock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    UINT flt_ptn = 0;

    UINT wfmode = TWF_ORW;

    (((aui_hdl_event *)p_hdl_event)->ul_id) = EVENT_REMOVE_FLAGS((((aui_hdl_event *)p_hdl_event)->ul_id));
    if((((aui_hdl_event *)p_hdl_event)->ul_id) > (g_os.aui_os_init_para.event_total_num - 1))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    i = (((aui_hdl_event *)p_hdl_event)->ul_id) >> 5;
    j = (((aui_hdl_event *)p_hdl_event)->ul_id) & 0x1f;

    if(!(g_os.aui_os_init_para.event_array [i] & (1 << j)))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    if(E_OK == os_wait_flag_time_out(&flt_ptn, g_os.event_flg_id[i], (1 << j), wfmode, 0))
    {
        //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
        OS_ASSERT(0);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    //osal_mutex_unlock(((aui_hdl_event *)p_hdl_event)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;

}


AUI_RTN_CODE aui_os_timer_create(const aui_attr_timer *p_timer_attr,aui_hdl* const pp_hdl_timer)
{
    T_TIMER st_timer;
    OSAL_ID timer_id=OSAL_INVALID_ID;
    aui_hdl_timer hdl_timer;
    OSAL_ID dev_mutex_id=INVALID_ID;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    //aui_hdl_timer ***ppp_cur=NULL;
    //aui_hdl_timer **pp_cur=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

    if((NULL==pp_hdl_timer)||(NULL==p_timer_attr))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"((NULL==pp_hdl_timer)||(NULL==p_timer_attr))");
    }

    MEMSET(&hdl_timer,0,sizeof(aui_hdl_timer));

    *((aui_hdl_timer **)pp_hdl_timer)=(aui_hdl_timer *)MALLOC(sizeof(aui_hdl_timer));
    if(NULL==(*((aui_hdl_timer **)pp_hdl_timer)))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"malloc failed");
    }

    (*(aui_hdl_timer **)pp_hdl_timer)->dev_mutex_id=dev_mutex_id;
    st_timer.callback     = aui_timer_cb;
    st_timer.type     = p_timer_attr->ul_type;
    st_timer.time     = p_timer_attr->ul_time_out;
    st_timer.param      = (UINT)(*((aui_hdl_timer **)pp_hdl_timer));
    st_timer.name[0]    = p_timer_attr->sz_name[0];
    st_timer.name[1]    = p_timer_attr->sz_name[1];
    st_timer.name[2]    = p_timer_attr->sz_name[2];
    MEMCPY(&((*((aui_hdl_timer **)pp_hdl_timer))->timer_attr),p_timer_attr,sizeof(aui_attr_timer));

    timer_id     =       osal_timer_create(&st_timer);
    if(timer_id==OSAL_INVALID_ID)
    {
        osal_mutex_unlock(dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    hdl_timer.ul_id=timer_id;
    MEMCPY(&(hdl_timer.timer_attr),p_timer_attr,sizeof(aui_attr_timer));
    MEMCPY((*((aui_hdl_timer **)pp_hdl_timer)),&hdl_timer,sizeof(aui_hdl_timer));
    (*(aui_hdl_timer **)pp_hdl_timer)->dev_mutex_id=dev_mutex_id;
    osal_mutex_unlock(dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_timer_delete(aui_hdl *pp_hdl_timer)
{
    aui_hdl_timer **pp_hdl_timer_tmp=NULL;
    aui_hdl_timer *p_hdl_timer=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((OSAL_INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"(OSAL_INVALID_ID==s_mod_mutex_id_os)");
    }

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pp_hdl_timer))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"(NULL==pp_hdl_timer)");
    }
    pp_hdl_timer_tmp=(aui_hdl_timer **)pp_hdl_timer;
    p_hdl_timer=*pp_hdl_timer_tmp;
    if((NULL==p_hdl_timer)||(INVALID_ID==((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }
    //osal_mutex_lock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    if((0 != ((aui_hdl_timer *)p_hdl_timer)->ul_id) &&
        (RET_SUCCESS!=osal_timer_delete(((aui_hdl_timer *)p_hdl_timer)->ul_id)))
    {
        //osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,"\r\n_delete timer err.");
    }

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);

    if(0!=osal_mutex_delete(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id))
    {
        ////AUI_DBG("\r\n[%s][%d]",__FUNCTION__,__LINE__);
        //aui_rtn(AUI_RTN_FAIL,"\r\n_delete mutex err.");
    }
#if 1

    MEMSET(p_hdl_timer,0,sizeof(aui_hdl_timer));
    FREE(p_hdl_timer);
    //p_hdl_timer=NULL;
    *((aui_hdl_timer **)pp_hdl_timer)=NULL;
#endif

    //osal_mutex_unlock(s_mod_mutex_id_os);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_timer_set(aui_hdl p_hdl_timer,unsigned long ul_item,void *pv_param)
{
    AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_timer))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    osal_mutex_lock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);

    switch(ul_item)
    {
        case AUI_TIMER_TYPE_SET:
        {

            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_TIMER_DELAY_SET:
        {
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
            }
            osal_delay((unsigned long)(*(unsigned long *)pv_param));
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_TIMER_VAL_SET:
        {
            if(NULL==pv_param)
            {
                osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
            }
            if((0 != ((aui_hdl_timer *)p_hdl_timer)->ul_id) &&
                (RET_SUCCESS!=osal_timer_set((((aui_hdl_timer *)p_hdl_timer)->ul_id),*((unsigned long *)pv_param))))
            {
                osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,"osal_timer_set failed");
            }
            ((aui_hdl_timer *)p_hdl_timer)->timer_attr.ul_time_out = *((unsigned long *)pv_param);
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }

        default:
        {
            osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
            aui_rtn(AUI_RTN_FEATURE_NOT_SUPPORTED,"Not support");
            break;
        }
    }
    if(AUI_RTN_SUCCESS!=rtn_code)
    {
        osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
        return rtn_code;
    }

    osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_timer_get(aui_hdl p_hdl_timer,unsigned long ul_item,void *pv_param)
{
    AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

    (void)p_hdl_timer;

    switch(ul_item)
    {
        case AUI_TIMER_SECOND_GET:
        {
            if(NULL==pv_param)
            {
                aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
            }
            *(unsigned long *)pv_param=osal_get_time();
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }
        case AUI_TIMER_TICK_GET:
        {
            if(NULL==pv_param)
            {
                aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
            }
            *(unsigned long *)pv_param=osal_get_tick();
            rtn_code=AUI_RTN_SUCCESS;
            break;
        }

        default:
        {
            aui_rtn(AUI_RTN_FEATURE_NOT_SUPPORTED,"Not support");
            break;
        }
    }
    if(AUI_RTN_SUCCESS!=rtn_code)
    {
        return rtn_code;
    }


    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_timer_rst(aui_hdl p_hdl_timer)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    T_TIMER st_timer;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_timer))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"invalid parameter");
    }

    osal_mutex_lock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);

    if(0 == ((aui_hdl_timer *)p_hdl_timer)->ul_id)
    {
        OSAL_ID timer_id=OSAL_INVALID_ID;
        aui_attr_timer *p_timer_attr = &((aui_hdl_timer *)p_hdl_timer)->timer_attr;

        st_timer.callback     = aui_timer_cb;
        st_timer.type     = p_timer_attr->ul_type;
        st_timer.time     = p_timer_attr->ul_time_out;
        st_timer.param      = (UINT)p_hdl_timer;
        st_timer.name[0]    = p_timer_attr->sz_name[0];
        st_timer.name[1]    = p_timer_attr->sz_name[1];
        st_timer.name[2]    = p_timer_attr->sz_name[2];

        timer_id     =       osal_timer_create(&st_timer);
        if(timer_id==OSAL_INVALID_ID)
        {
            osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }
        ((aui_hdl_timer *)p_hdl_timer)->ul_id=timer_id;
    }
    else
    {
        if(RET_SUCCESS!=osal_timer_set((((aui_hdl_timer *)p_hdl_timer)->ul_id),
            ((aui_hdl_timer *)p_hdl_timer)->timer_attr.ul_time_out))
            {
                osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
                aui_rtn(AUI_RTN_FAIL,"osal_timer_set fail");
            }
    }
    osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_timer_run(aui_hdl p_hdl_timer,unsigned long b_enable)
{
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==p_hdl_timer))
    {
        osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    osal_mutex_lock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_os);

    if((0 == ((aui_hdl_timer *)p_hdl_timer)->ul_id) ||
        (RET_SUCCESS!=osal_timer_activate((((aui_hdl_timer *)p_hdl_timer)->ul_id),b_enable)))
    {
        osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }

    osal_mutex_unlock(((aui_hdl_timer *)p_hdl_timer)->dev_mutex_id);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}




void* aui_os_malloc(unsigned long ul_size)
{
    void *ret = NULL;
    aui_heap_info *_heap = NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    // larger than malloc_limit_size, let's try to use large heap
    if (ul_size >= g_os.aui_os_init_para.malloc_limit_size) {
        // maybe we should use large heap
        if (g_os.aui_os_init_para.large_heap.handle) {
            _heap = &g_os.aui_os_init_para.large_heap;
        }
    } else {
        // maybe we should use min heap
        if (g_os.aui_os_init_para.min_heap.handle) {
            _heap = &g_os.aui_os_init_para.min_heap;
        }
    }
    
    if (_heap) {
        ret = aui_malloc(_heap, ul_size);
    } else {
        // If user not configured large_heap & min_heap, just malloc from TDS system heap
        ret = (void*)malloc(ul_size);
    }
    
    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return ret;
}


void* aui_os_re_malloc(void* pv_addr,unsigned long ul_size)
{
    void *ret = NULL;
    UINT32 handle = 0;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    if (pv_addr == NULL) {
        // Case1: Just malloc new buffer
        ret = aui_os_malloc(ul_size);
    } else {
        if ((g_os.aui_os_init_para.large_heap.handle) 
            && (pv_addr >= (void*)g_os.aui_os_init_para.large_heap.base)
            && (pv_addr < (void*)(g_os.aui_os_init_para.large_heap.base + g_os.aui_os_init_para.large_heap.len))) {
            
            // Case2: pv_addr in large_heap, Use large heap
            handle = g_os.aui_os_init_para.large_heap.handle;
        } else if ((g_os.aui_os_init_para.min_heap.handle)
                    && (pv_addr >= (void*)g_os.aui_os_init_para.min_heap.base)
                    && (pv_addr < (void*)(g_os.aui_os_init_para.min_heap.base + g_os.aui_os_init_para.min_heap.len))) {
                    
            // Case3: pv_addr in min_heap, Use min heap
            handle = g_os.aui_os_init_para.min_heap.handle;
        }

        if (handle) {
            // realloc in user heap
            ret = (void *)__realloc(handle, pv_addr, ul_size);
        } else {
            // Case4: No user heap, just realloc from TDS system heap
            ret = (void *) realloc(pv_addr, ul_size);
        }
    }


    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return ret;
}



void* aui_os_calloc(unsigned long ul_elem_cnt,unsigned long ul_size)
{
    void *ret = NULL;
    aui_heap_info *_heap = NULL;
    unsigned long total_size = 0;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    total_size = ul_elem_cnt * ul_size;

    // larger than malloc_limit_size, let's try to use large heap
    if (total_size >= g_os.aui_os_init_para.malloc_limit_size) {
        // maybe we should use large heap
        if (g_os.aui_os_init_para.large_heap.handle) {
            _heap = &g_os.aui_os_init_para.large_heap;
        }
    } else {
        // maybe we should use min heap
        if (g_os.aui_os_init_para.min_heap.handle) {
            _heap = &g_os.aui_os_init_para.min_heap;
        }
    }
    
    if (_heap) {
        ret = aui_malloc(_heap, total_size);
        // calloc need to memset the buffer
        if(ret != NULL)
        {
            MEMSET(ret, 0, total_size);
        }
    } else {
        // If user not configured large_heap & min_heap, just calloc from TDS system heap
        ret = calloc(ul_elem_cnt, ul_size);
    }

    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return ret;
}




AUI_RTN_CODE aui_os_free(void* pv_addr)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    aui_heap_info *_heap = NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    if ((g_os.aui_os_init_para.large_heap.handle) 
        && (pv_addr >= (void*)g_os.aui_os_init_para.large_heap.base)
        && (pv_addr < (void*)(g_os.aui_os_init_para.large_heap.base + g_os.aui_os_init_para.large_heap.len))) {
        
        // Case1: pv_addr in large_heap, Use large heap
        _heap = &g_os.aui_os_init_para.large_heap;
    } else if ((g_os.aui_os_init_para.min_heap.handle)
                && (pv_addr >= (void*)g_os.aui_os_init_para.min_heap.base)
                && (pv_addr < (void*)(g_os.aui_os_init_para.min_heap.base + g_os.aui_os_init_para.min_heap.len))) {
                
        // Case2: pv_addr in min_heap, Use min heap
        _heap = &g_os.aui_os_init_para.min_heap;
    }
    
    if (_heap) {
        // free in user heap
        ret = aui_free(_heap, pv_addr);
    } else {
        // Case3: No user heap, just free in TDS system heap
        free(pv_addr);
        ret = AUI_RTN_SUCCESS;
    }

    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return ret;
}

AUI_RTN_CODE aui_os_mempool_create(const aui_attr_mempool *p_pool_attr, aui_hdl* const pp_hdl_pool)
{
    OSAL_ID dev_mutex_id=INVALID_ID;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;
    aui_hdl_mempool * pst_hdl_mempool=NULL;
    aui_heap_info * heap;
    //aui_hdl_sem ***ppp_cur=NULL;
    //aui_hdl_sem **pp_cur=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

    if((NULL==pp_hdl_pool)||(NULL==p_pool_attr))
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    pst_hdl_mempool=(aui_hdl_mempool *)MALLOC(sizeof(aui_hdl_mempool));
    if(NULL==pst_hdl_mempool)
    {
        osal_mutex_unlock(dev_mutex_id);
        aui_rtn(AUI_RTN_ENOMEM,"Malloc failed");
    }

    pst_hdl_mempool->dev_mutex_id=dev_mutex_id;

    MEMCPY(&(pst_hdl_mempool->mempool_attr), p_pool_attr, sizeof(aui_attr_mempool));
    heap = &(pst_hdl_mempool->heap);
    MEMSET(heap, 0, sizeof(aui_heap_info));
    heap->base = p_pool_attr->base;
    heap->len = p_pool_attr->len;
    if(!__alloc_initialize(&heap->handle, heap->base, heap->base+heap->len))
    {
        osal_mutex_unlock(dev_mutex_id);
        FREE(pst_hdl_mempool);
        rtn=AUI_RTN_FAIL;
        return rtn;
    }
    *pp_hdl_pool=pst_hdl_mempool;
    osal_mutex_unlock(dev_mutex_id);

    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_mempool_delete(aui_hdl *pp_hdl_mempool)
{
    aui_hdl_mempool **pp_hdl_mempool_tmp=NULL;
    aui_hdl_mempool *p_hdl_mempool=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((OSAL_INVALID_ID==s_mod_mutex_id_os))
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    //osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
    if((NULL==pp_hdl_mempool))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }
    pp_hdl_mempool_tmp=(aui_hdl_mempool **)pp_hdl_mempool;
    p_hdl_mempool=*pp_hdl_mempool_tmp;

    if((NULL==p_hdl_mempool)||(INVALID_ID==p_hdl_mempool->dev_mutex_id))
    {
        //osal_mutex_unlock(s_mod_mutex_id_os);
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }
    //osal_mutex_lock(p_hdl_mempool->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    //osal_mutex_unlock(s_mod_mutex_id_os);

    if(RET_SUCCESS!=__alloc_cleanup(p_hdl_mempool->heap.handle))
    {
        //osal_mutex_unlock(p_hdl_mempool->dev_mutex_id);
        aui_rtn(AUI_RTN_FAIL,"\r\n_cleanup pool err.");
    }

    if(0!=osal_mutex_delete(p_hdl_mempool->dev_mutex_id))
    {
        ////AUI_DBG("\r\n[%s][%d]",__FUNCTION__,__LINE__);
        //aui_rtn(AUI_RTN_FAIL,"\r\n_delete mutex err.");
    }
#if 1

    MEMSET(p_hdl_mempool,0,sizeof(aui_hdl_mempool));
    FREE(p_hdl_mempool);
    //p_hdl_mempool=NULL;
    *((aui_hdl_mempool **)pp_hdl_mempool)=NULL;
#endif

    //osal_mutex_unlock(s_mod_mutex_id_os);
    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return AUI_RTN_SUCCESS;
}

void* aui_os_mempool_malloc(aui_hdl p_hdl_mempool, unsigned long ul_size)
{
    void *ret = NULL;
    aui_heap_info *_heap = NULL;
    aui_hdl_mempool *p_hdl_mempool_tmp=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if((NULL==p_hdl_mempool) || (ul_size == 0))
    {
        return ret;
    }
    p_hdl_mempool_tmp = (aui_hdl_mempool *)p_hdl_mempool;
    _heap = &p_hdl_mempool_tmp->heap;

    ret = aui_malloc(_heap, ul_size);

    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return ret;

}

AUI_RTN_CODE aui_os_mempool_free(aui_hdl p_hdl_mempool, void* pv_addr)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;
    aui_heap_info *_heap = NULL;
    aui_hdl_mempool *p_hdl_mempool_tmp=NULL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    if(NULL==p_hdl_mempool)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    p_hdl_mempool_tmp = (aui_hdl_mempool *)p_hdl_mempool;

    _heap = &p_hdl_mempool_tmp->heap;

    if((pv_addr < (void*)_heap->base) || (pv_addr >= (void*)(_heap->base+_heap->len)))
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    ret = aui_free(_heap, pv_addr);

    AUI_LEAVE_FUNC(AUI_MODULE_OS);

    return ret;
}

AUI_RTN_CODE aui_os_mem_info_get(aui_mem_info* const p_mem_info)
{
    int free_mem = 0, max_mem = 0;
    UINT32 handle = 0;

    AUI_ENTER_FUNC(AUI_MODULE_OS);

    handle = g_os.aui_os_init_para.large_heap.handle;

    if(NULL==p_mem_info)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }

    if (handle) {
        __get_free_ramsize(handle, &free_mem, &max_mem);

        p_mem_info->ul_mem_free_size  = free_mem;
    } else {
        // The value is not very accurate
        p_mem_info->ul_mem_free_size = (unsigned long)heap_get_free_size();
    }
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_dbg_lv_set(aui_module_id mdl_id,unsigned long ul_prt_level)
{
    if(SUCCESS!=drv_set_module_print_level(mdl_id,ul_prt_level))
    {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_muti_event_create(const aui_attr_muti_event *p_muti_event_attr,unsigned long* const px_event,aui_hdl *p_hdl_muti_event)
{
    aui_hdl_task *x_task_id = NULL;
    unsigned long i = 0;
    ID event_id=OSAL_INVALID_ID;
    aui_hdl_muti_event *pst_hdl_muti_event=NULL;
    OSAL_ID dev_mutex_id=OSAL_INVALID_ID;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if(AUI_MUTI_EVENT_MODE_ATTACH_TASK==g_os.aui_os_init_para.muti_event_mode)
    {
        if ((aui_os_task_get_self_hdl((aui_hdl*)&x_task_id)) == (AUI_RTN_CODE)AUI_RTN_FAIL)
        {
            return AUI_RTN_FAIL;
        }

        if(!x_task_id)
        {
            return AUI_RTN_FAIL;
        }
        if ((OSAL_INVALID_ID == x_task_id->x_event_flag_id) || (0 == x_task_id->x_event_flag_id))
        {
            x_task_id->x_event_flag_id = osal_flag_create(0x0);
            if ((OSAL_INVALID_ID == x_task_id->x_event_flag_id) || (0 == x_task_id->x_event_flag_id))
            {
                return AUI_RTN_FAIL;
            }
        }

        *px_event = 0;

        for (i=0; i<sizeof(unsigned long)*8; i++)
        {
            if ( 0 == (x_task_id->x_events & (1 << i)))
            {
                *px_event = (1 << i);
                x_task_id->x_events |= (1 << i);
                break;
            }
        }

        if (sizeof(unsigned long)*8 == i)
        {
            return AUI_RTN_FAIL;
        }
    }
    else if(AUI_MUTI_EVENT_MODE_BROADCASE==g_os.aui_os_init_para.muti_event_mode)
    {
        AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_os,dev_mutex_id,AUI_MODULE_OS,AUI_RTN_FAIL);

        if((NULL==p_hdl_muti_event)||(NULL==p_muti_event_attr))
        {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }

        pst_hdl_muti_event=(aui_hdl_muti_event *)MALLOC(sizeof(aui_hdl_muti_event));
        if(NULL==pst_hdl_muti_event)
        {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL,"Malloc failed.");
        }
        MEMSET(pst_hdl_muti_event,0,sizeof(aui_hdl_muti_event));
        event_id = osal_flag_create(p_muti_event_attr->ul_init_event_val);
        if ((OSAL_INVALID_ID == event_id) || (0 == event_id))
        {
            osal_mutex_unlock(dev_mutex_id);
            aui_rtn(AUI_RTN_FAIL,"Create flag failed.");
        }
        pst_hdl_muti_event->dev_mutex_id=dev_mutex_id;
        pst_hdl_muti_event->ul_id=event_id;
        pst_hdl_muti_event->dev_priv_data.en_status=AUI_DEV_STATUS_OPEN;
        *p_hdl_muti_event=pst_hdl_muti_event;

        osal_mutex_unlock(dev_mutex_id);
    }

    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_muti_event_delete(aui_hdl *pp_hdl_muti_event,unsigned long x_events)
{
    aui_hdl_task *x_task_id = NULL;
    aui_hdl_muti_event **pp_hdl_muti_event_tmp=NULL;
    aui_hdl_muti_event *p_hdl_muti_event=NULL;
    AUI_RTN_CODE rtn=AUI_RTN_FAIL;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if(AUI_MUTI_EVENT_MODE_ATTACH_TASK==g_os.aui_os_init_para.muti_event_mode)
    {
        if ((aui_os_task_get_self_hdl((aui_hdl*)&x_task_id) == (AUI_RTN_CODE)AUI_RTN_FAIL) || (!x_task_id))
        {
            return AUI_RTN_FAIL;
        }

        if ((OSAL_INVALID_ID == x_task_id->x_event_flag_id) || (0 == x_task_id->x_event_flag_id))
        {
            return AUI_RTN_FAIL;
        }

        x_task_id->x_events &= (~x_events);

        if (0 == x_task_id->x_events)
        {
            osal_flag_delete(x_task_id->x_event_flag_id);
            x_task_id->x_event_flag_id = OSAL_INVALID_ID;
        }
    }
    else if(AUI_MUTI_EVENT_MODE_BROADCASE==g_os.aui_os_init_para.muti_event_mode)
    {
        if((OSAL_INVALID_ID==s_mod_mutex_id_os))
        {
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }

        osal_mutex_lock(s_mod_mutex_id_os,OSAL_WAIT_FOREVER_TIME);
        if((NULL==pp_hdl_muti_event))
        {
            osal_mutex_unlock(s_mod_mutex_id_os);
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }
        pp_hdl_muti_event_tmp=(aui_hdl_muti_event **)pp_hdl_muti_event;
        p_hdl_muti_event=*pp_hdl_muti_event_tmp;
        if((NULL==p_hdl_muti_event)||(INVALID_ID==p_hdl_muti_event->dev_mutex_id))
        {
            osal_mutex_unlock(s_mod_mutex_id_os);
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }
        osal_mutex_lock(((aui_hdl_muti_event *)p_hdl_muti_event)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
        osal_mutex_unlock(s_mod_mutex_id_os);
        if(AUI_DEV_STATUS_OPEN!=p_hdl_muti_event->dev_priv_data.en_status)
        {
            aui_rtn(AUI_RTN_EINVAL,"\r\n sem handle have been del.");
        }
        if(RET_SUCCESS!=osal_flag_delete(p_hdl_muti_event->ul_id))
        {
            osal_mutex_unlock(p_hdl_muti_event->dev_mutex_id);
            rtn=AUI_RTN_FAIL;
            return rtn;
        }


        p_hdl_muti_event->dev_priv_data.en_status=AUI_DEV_STATUS_CLOSED;
        if(0!=osal_mutex_delete(p_hdl_muti_event->dev_mutex_id))
        {
            AUI_DBG("\r\n[%s][%d]",__FUNCTION__,__LINE__);
            //aui_rtn(AUI_RTN_FAIL,"\r\n_delete mutex err.");
        }

        MEMSET(p_hdl_muti_event,0,sizeof(aui_hdl_muti_event));
        FREE(p_hdl_muti_event);
        //p_hdl_muti_event=NULL;
        *((aui_hdl_muti_event **)pp_hdl_muti_event)=NULL;

        AUI_LEAVE_FUNC(AUI_MODULE_OS);
        return AUI_RTN_SUCCESS;
    }


    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_os_muti_event_wait(aui_hdl hdl_muti_event,
                                                aui_muti_event_option en_option,
                                                unsigned long x_expected_events,
                                                unsigned long *px_received_events,
                                                unsigned long x_timeout)
{
    aui_hdl_task * x_task_id = NULL;
    unsigned long x_received_flags = 0;
    unsigned long x_expected_flags = (unsigned long)x_expected_events;
    unsigned long ul_flag=0;
    aui_hdl_muti_event *p_hdl_muti_event=NULL;
    ER err_rtn_code=E_FAILURE;

    AUI_ENTER_FUNC(AUI_MODULE_OS);
    if(NULL==px_received_events)
    {
        aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
    }
    if(AUI_MUTI_EVENT_MODE_ATTACH_TASK==g_os.aui_os_init_para.muti_event_mode)
    {
        if (aui_os_task_get_self_hdl((aui_hdl*)&x_task_id) == (AUI_RTN_CODE)AUI_RTN_FAIL)
        {
            return AUI_RTN_FAIL;
        }

        if ((!x_task_id) || (OSAL_INVALID_ID == x_task_id->x_event_flag_id) || (0 == x_task_id->x_event_flag_id))
        {
            return AUI_RTN_FAIL;
        }

        if ((x_task_id->x_events & x_expected_events) != x_expected_events)
        {
            return AUI_RTN_FAIL;
        }

        if(en_option&AUI_MUTI_EVENT_OPTION_AND)
        {
            ul_flag=ul_flag|OSAL_TWF_ANDW;
        }
        if(en_option&AUI_MUTI_EVENT_OPTION_OR)
        {
            ul_flag=ul_flag|OSAL_TWF_ORW;
        }
        if(en_option&AUI_MUTI_EVENT_OPTION_CLR)
        {
            ul_flag=ul_flag|OSAL_TWF_CLR;
        }

        if (E_TIMEOUT == osal_flag_wait(&x_received_flags,
            x_task_id->x_event_flag_id,
            x_expected_flags,
            ul_flag,//OSAL_TWF_ORW | OSAL_TWF_CLR,
            x_timeout))
        {
            *px_received_events = 0;
        }
        else
        {
            *px_received_events = (x_received_flags & x_expected_flags);
        }

    }
    else if(AUI_MUTI_EVENT_MODE_BROADCASE==g_os.aui_os_init_para.muti_event_mode)
    {
        if((NULL==hdl_muti_event)||(0x80000000!= (((unsigned long)hdl_muti_event)&0xf0000000)))
        {
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }
        p_hdl_muti_event=hdl_muti_event;

        if(en_option&AUI_MUTI_EVENT_OPTION_AND)
        {
            ul_flag=ul_flag|OSAL_TWF_ANDW;
        }
        if(en_option&AUI_MUTI_EVENT_OPTION_OR)
        {
            ul_flag=ul_flag|OSAL_TWF_ORW;
        }
        if(en_option&AUI_MUTI_EVENT_OPTION_CLR)
        {
            ul_flag=ul_flag|OSAL_TWF_CLR;
        }

        err_rtn_code     =       osal_flag_wait(px_received_events,p_hdl_muti_event->ul_id,x_expected_events,ul_flag,x_timeout);
        if(err_rtn_code!=E_OK)
        {
            if(err_rtn_code==E_TIMEOUT)
            {
                aui_rtn(AUI_RTN_OS_EVENT_TIMEOUT,"Osal falg wait timeout.");
            }
            else
            {
                aui_rtn(AUI_RTN_FAIL,"Osal falg wait failed.");
            }
        }

    }
    AUI_LEAVE_FUNC(AUI_MODULE_OS);
    return AUI_RTN_SUCCESS;
}



AUI_RTN_CODE aui_os_muti_event_set(aui_hdl hdl_muti_event,aui_hdl x_task_id,unsigned long x_sent_events)
{
    aui_hdl_task *x_task_self = x_task_id;
    unsigned long x_set_flags = x_sent_events;
    aui_hdl_muti_event *p_hdl_muti_event=NULL;

    if(AUI_MUTI_EVENT_MODE_ATTACH_TASK==g_os.aui_os_init_para.muti_event_mode)
    {
        if(AUI_RTN_SUCCESS!=aui_handle_check(x_task_self))
        {
            aui_rtn(AUI_RTN_EINVAL,"x_task_self not exist");
        }
        
        osal_flag_set(x_task_self->x_event_flag_id, x_set_flags);
    }
    else if(AUI_MUTI_EVENT_MODE_BROADCASE==g_os.aui_os_init_para.muti_event_mode)
    {
        if((NULL==hdl_muti_event)||(0x80000000!= (((unsigned long)hdl_muti_event)&0xf0000000)))
        {
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }
        p_hdl_muti_event=hdl_muti_event;

        if(osal_flag_set(p_hdl_muti_event->ul_id,x_sent_events)!=E_OK)
        {
            aui_rtn(AUI_RTN_EINVAL,"Osal flag set failed.");
        }

    }
    return AUI_RTN_SUCCESS;
}
AUI_RTN_CODE aui_os_muti_event_clear(aui_hdl hdl_muti_event,aui_hdl x_task_id,unsigned long ul_clear_event)
{
    aui_hdl_task *x_task_self = x_task_id;
    unsigned long x_set_flags = ul_clear_event;
    aui_hdl_muti_event *p_hdl_muti_event=NULL;

    if(AUI_MUTI_EVENT_MODE_ATTACH_TASK==g_os.aui_os_init_para.muti_event_mode)
    {
        if(NULL==x_task_self)
        {
            if (aui_os_task_get_self_hdl((aui_hdl*)&x_task_self) == (AUI_RTN_CODE)AUI_RTN_FAIL)
            {
                aui_rtn(AUI_RTN_FAIL, "aui_os_task_get_self_hdl failed");
            }
        }
        if(AUI_RTN_SUCCESS!=aui_handle_check(x_task_self))
        {
            aui_rtn(AUI_RTN_EINVAL,"x_task_self is not exist");
        }

        osal_flag_clear(x_task_self->x_event_flag_id, x_set_flags);
    }
    else if(AUI_MUTI_EVENT_MODE_BROADCASE==g_os.aui_os_init_para.muti_event_mode)
    {
        if((NULL==hdl_muti_event)||(0x80000000!= (((unsigned long)hdl_muti_event)&0xf0000000)))
        {
            aui_rtn(AUI_RTN_EINVAL,"Invalid parameter");
        }
        p_hdl_muti_event=hdl_muti_event;

        if(osal_flag_clear(p_hdl_muti_event->ul_id,ul_clear_event)!=E_OK)
        {
            aui_rtn(AUI_RTN_EINVAL,"Osal flag set failed.");
        }

    }
    return AUI_RTN_SUCCESS;
}




AUI_RTN_CODE aui_os_hsr_reg(aui_os_hsr_func_cb call_back, void *p_void)
{
    ER err=E_FAILURE;
    err=osal_interrupt_register_hsr((T_HSR_PROC_FUNC_PTR)call_back, (DWORD )p_void);
    if(E_OK!=err)
    {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_lsr_reg(unsigned short irq_nb, aui_os_hsr_func_cb call_back, void *p_void)
{
    ER err=E_FAILURE;
    err=osal_interrupt_register_lsr((UINT16)irq_nb, (ISR_PROC)call_back, (UINT32)p_void);
    if(E_OK!=err)
    {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_lsr_unreg(unsigned short irq_nb, aui_os_hsr_func_cb call_back)
{
    ER err=E_FAILURE;
    err=osal_interrupt_unregister_lsr((UINT16)irq_nb, (ISR_PROC)call_back);
    if(E_OK!=err)
    {
        return AUI_RTN_FAIL;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_crit_start(void)
{
    osal_interrupt_disable();
    return  AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_crit_stop(void)
{
    osal_interrupt_enable();
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_task_crit_start(void)
{
    osal_task_dispatch_off();
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_os_task_crit_stop(void)
{
    osal_task_dispatch_on();
    return AUI_RTN_SUCCESS;
}


