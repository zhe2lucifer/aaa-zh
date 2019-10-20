/**
<!--
Notice:             This file mainly contains Doxygen-style comments to generate
                    the HTML help file (.chm) provided by ALi Corporation to
                    give ALi Customers a quick support for using ALi AUI API
                    Interface.\n
                    Furthermore, the comments have been wrapped at about 80 bytes
                    in order to avoid the horizontal scrolling when splitting
                    the display in two part for coding, testing, debugging
                    convenience
Current ALi author: Davy.Wu
Last update         2017.02.25
-->

@file   aui_os.h

@brief  Operating System (OS) Module.

        <b> Operating System (OS) Module </b>is mainly used to access the
        resource of the <b> Task Dispatch System (TDS) </b> which is the ALi
        proprietary Real Time Operating Systen (RTOS)\n\n

         It mainly features:
         - Task Management
         - Semaphore
         - Mutex
         - Message
         - Event
         - Multi events
         - Timer
         - Memory Management

@note   For further details, please refer to the ALi document
        <b><em>
        ALi_AUI_Porting_Guide_Modules.pdf - Chapter "AUI-OS Module
        (only for TDS OS)"
        </em></b>

@copyright  Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
*/

#ifndef _AUI_OS_H

#define _AUI_OS_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Macro List*****************************/

/**
Macro used for OS debugging
*/
#define aui_os_printf(...)

/**
Macro to indicate the <b> version number of OS Module </b> which is a
hexadecimal number that consist of three (3) reading parts (sorted from left
to right) as exemplified below:\n\n

Version number 0x00010000, where
- 0x0001 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 00 = <b> 2nd level sub-version </b>
*/
#define AUI_MODULE_VERSION_NUM_OS    (0X00010000)

/**
Macro to indicate the <b> maximum length </b> acceptable (in characters) for
the <b> name </b> of a the OS Module
*/
#define AUI_DEV_NAME_LEN    (32)

/**
Macro to indicate the <b> maximum queue count </b> acceptable (in integer) for
the <b> messages </b>.
*/
#define AUI_OS_MAX_MSGQUEUE_COUNT        256

/**
Macro to indicate the <b> maximum timeout </b> acceptable (in milliseconds unit)
for the <b> waiting time </b>.
*/
#define AUI_TMO_FOREVER_TIME    (0xFFFFFFFF)

/*******************************Global Type List*******************************/

/**
Function pointer to specify the type of callback function that will be called
when the pre-set timeout value of the timer is up, which is created by the
function #aui_os_timer_create

@note   The members @b timer_call_back and @b ul_para1 of the struct
        #aui_attr_timer (which is using this function pointer) should be
        configured before calling the function #aui_os_timer_create
*/
typedef void (*aui_p_fun_timer_cb) (

    unsigned int param

    );

/**
Function pointer to specify the type of user task which will be executed when
the task is scheduled by TDS OS.

@note   The members @b p_fun_task_entry, @b para1 and @b para2 of the struct
        #aui_attr_task (which is using this function pointer) should be
        configured before calling the function #aui_os_task_create
*/
typedef unsigned long (*aui_p_fun_task_entry) (

    void *pv_para1,

    void *pv_para2

    );

/**
Function pointer to specify the type of function which will be used to map
customer's task priority into the system task priority list
*/
typedef unsigned long (*aui_p_fun_task_prio_adapt) (

    unsigned long ul_priority

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_os_hsr_reg for the <b> High/Low Priority Service Routing (H/LPSR)
</b>
*/
typedef void (*aui_os_hsr_func_cb) (

    void *pv_param

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify the list
        of attributes related to the task
        </div> @endhtmlonly

        Struct to specify the list of attributes related to the task
*/
typedef struct aui_st_attr_task {

    /**
    Member to specify the <b> Task Name </b>
    */
    char    sz_name[AUI_DEV_NAME_LEN+1];

    /**
    Member to specify the <b> Task Entry Function </b> as per comment of the
    function pointer #aui_p_fun_task_entry
    */
    aui_p_fun_task_entry    p_fun_task_entry;

    /**
    Member to specify the <b> Task Priority Rule </b> as below:
    - The task priority range is from 1 to 31 in TDS OS System
    - The small number has higher priority than larger number.
    - Four (4) kinds of priority level are defined as below:
      - LOW (31)
      - NORMAL (20)
      - HIGH (17)
      - HSR (10)
    - The application should use LOW/NORMAL level in most of the cases.\n
      If the application want to use HIGH or more high priority level for very
      special case, please check with ALi R&D Dept.
    */
    unsigned long        ul_priority;

    /**
    Member to specify the <b> Task Stack Size </b> (in @a byte unit)
    */
    unsigned long        ul_stack_size;

    /**
    Member to specify <b> Task Time Slice </b> (in @a millisecond unit)
    */
    unsigned long          ul_quantum;

    /**
    Member to specify the <b> Task Entry Function Parameter 1 </b>
    */
    unsigned long          para1;

    /**
    Member to specify the <b> Task Entry Function Parameter 2 </b>
    */
    unsigned long          para2;

} aui_attr_task, *aui_p_attr_task;

/**
Enum to specify the <b> Task Status </b>
*/
typedef enum aui_en_task_status {

    /**
    Value to specify that the task is in <b> Idle State </b>
    */
    AUI_TASK_STATUS_IDLE=0,

    /**
    Value to specify that the task is in <b> Starting State </b>
    */
    AUI_TASK_STATUS_START,

    /**
    Value to specify that the task is in <b> Starting Failed State </b>
    */
    AUI_TASK_STATUS_START_FAIL,

    /**
    Value to specify that the task has been quieted and has returned an error
    */
    AUI_TASK_STATUS_RTN_ERR,

    /**
    Value to specify that the task has been quieted and has not returned
    any error
    */
    AUI_TASK_STATUS_RTN_OK,

    /**
    Value to specify that the task is in <b> Running State </b>
    */
    AUI_TASK_STATUS_RUN,

    /**
    Value to specify the end of the list of values of this enum.
    */
    AUI_TASK_STATUS_LAST

} aui_task_status, *aui_p_task_status;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify a list
        of attributes related to the message queue
        </div> @endhtmlonly

        Struct to specify a list of attributes related to the message queue
*/
typedef struct aui_st_attr_msgq {

    /**
    Member to specify the <b> Message Queue Buffer Size </b>
    */
    unsigned long    ul_buf_size;

    /**
    Member to specify the <b> Maximum Length </b> of each message
    */
    unsigned long    ul_msg_size_max;

    /**
    Member to specify the <b> Message Queue Name </b>
    */
    char            sz_name[AUI_DEV_NAME_LEN+1];

} aui_attr_msgq, *aui_p_attr_msgq;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify a list
        of attributes related to the semaphore
        </div> @endhtmlonly

        Struct to specify a list of attributes related to the semaphore
*/
typedef struct aui_st_attr_sem {

    /**
    Member to specify the <b> Initial Value </b> of the semaphore

    @warning    This member is reserved to ALi R&D Dept.
    */
    unsigned long    ul_init_val;

    /**
    Member to specify the <b> Maximum Value </b> of the semaphore
    */
    unsigned long    ul_max_val;

    /**
    Member to specify the <b> Current Count Value </b> of the semaphore
    */
    unsigned long ul_cnt;

    /**
    Member to specify the <b> Semaphore Name </b>
    */
    char    sz_name[AUI_DEV_NAME_LEN+1];

} aui_attr_sem, *aui_p_attr_sem;

/**
Enum to specify the <b> Mutex type </b>
*/
typedef enum aui_en_mutex_type {

    /**
    Value to specify a <b> Mutex Standard Type </b> for TDS OS.
    */
    AUI_MUTEX_TYPE_TDS=0,

    /**
    Value to specify a Mutex type similar to #AUI_MUTEX_TYPE_TDS but with a
    different implementation
    */
    AUI_MUTEX_TYPE_LINUX_NORMAL,

    /**
    Value to specify a Mutex type which supports a <b> nest lock </b>.
    */
    AUI_MUTEX_TYPE_LINUX_NEST,

    /**
    Value to specify the end of the list of values of this enum
    */
    AUI_MUTEX_TYPE_LAST

} aui_mutex_type, *aui_p_mutex_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify a list
        of attributes related to the mutex
        </div> @endhtmlonly

        Struct to specify a list of attributes related to the mutex
*/
typedef struct aui_st_attr_mutex {

    /**
    Member to specify the <b> Mutex type </b> as defined in the enum
    #aui_mutex_type
    */
    aui_mutex_type  mutex_type;

    /**
    Member to specify the <b> Current Count Value </b> of the mutex
    */
    unsigned long ul_cnt;

    /**
    Member to specify the <b> Mutex Name </b>
    */
    char    sz_name[AUI_DEV_NAME_LEN+1];

} aui_attr_mutex, *aui_p_attr_mutex;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify a list
        of attributes related to the event </div> @endhtmlonly

        Struct to specify a list of attributes related to the event
*/
typedef struct aui_st_attr_event {

    /**
    Member to specify the <b> Auto Reset Event </b>
    */
    unsigned long b_auto_reset;

    /**
    Member to specify the <b> Initial State of the event </b>
    */
    unsigned long b_initial_state;

    /**
    Member to specify the <b> Event Name </b>
    */
    char    sz_name[AUI_DEV_NAME_LEN+1];

} aui_attr_event, *aui_p_attr_event;

/**
Enum to specify different <b> Multi Event Working Modes </b>
*/
typedef enum aui_en_muti_event_mode {

    /**
    Value to specify that <b> all tasks </b> can receive the event
    */
    AUI_MUTI_EVENT_MODE_BROADCASE,

    /**
    Value to specify that <b> only the targeted tasks </b> can receive the event
    */
    AUI_MUTI_EVENT_MODE_ATTACH_TASK,

    /**
    Value to specify the end of the list of values of this enum
    */
    AUI_MUTI_EVENT_MODE_LAST

} aui_muti_event_mode;

/**
Enum to specify a list of different <b> working modes </b> of the semaphore
*/
typedef enum aui_en_sem_work_mode {

    /**
    Value to specify that the maximum count value of the semaphore <b> is checked
    </b> with its current count value when it is released
    */
    AUI_SEM_MODE_RELEASE_CHECK_MAX_CNT,

    /**
    Value to specify that the maximum count value of the semaphore <b> is not
    checked </b> with its current count value when it is released
    */
    AUI_SEM_MODE_RELEASE_NO_CHECK_MAX_CNT,

    /**
    Value to specify the end of the list of values of this enum
    */
    AUI_SEM_MODE_LAST

} aui_sem_work_mode;

/**
Enum to specify <b> Multi Event Options </b>
*/
typedef enum aui_en_muti_event_option {

    /**
    Value to specify that <b> all events </b> are @b ON
    */
    AUI_MUTI_EVENT_OPTION_AND=(1<<0),

    /**
    Value to specify that <b> only one event </b> is @b ON
    */
    AUI_MUTI_EVENT_OPTION_OR=(1<<1),

    /**
    Value to specify that the event will be reset automatically.
    */
    AUI_MUTI_EVENT_OPTION_CLR=(1<<2),

    /**
    Value to specify the end of the list of values of this enum
    */
    AUI_MUTI_EVENT_OPTION_LAST

} aui_muti_event_option;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify the
        attributes for the Multi Event
        </div> @endhtmlonly

        Struct to specify the attributes for the Multi Event
 */
typedef struct aui_st_attr_muti_event {

    /**
    Member to specify the <b> Initial Value </b> of the event
    */
    unsigned long ul_init_event_val;

    /**
    Member to specify the <b> Name of the Multi Event </b>
    */
    char    sz_name[AUI_DEV_NAME_LEN+1];

} aui_attr_muti_event, *aui_p_attr_muti_event;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify memory
        attributes
        </div> @endhtmlonly

        Struct to specify memory attributes
*/
typedef struct aui_st_mem_info {

    /**
    @attention  This member is @a reserved to ALi R&D Dept. for testing, user
                can ignore it
    */
    unsigned long ul_id;

    /**
    @attention  This member is @a reserved to ALi R&D Dept. for testing, user
                can ignore it
    */
    unsigned long ul_mem_size;

    /**
    @attention  This member is @a reserved to ALi R&D Dept. for testing, user
                can ignore it
    */
    unsigned long ul_mem_free_size;

} aui_mem_info, *aui_p_mem_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify the memory
        pool attributes
        </div> @endhtmlonly

        Struct to specify the memory pool attributes
*/
typedef struct aui_st_mem_pool {

    /**
    @attention  This member is @a reserved to ALi R&D Dept. for testing
    */
    unsigned long ul_id;

} aui_mem_pool, *aui_p_mem_pool;

/**
Enum to specify different <b> Task Return Modes </b>
*/
typedef enum aui_en_task_rtn_mode {

    /**
    Value to specify that the task will automatically release the resources
    when it returns
    */
    AUI_TASK_RTN_MODE_AUTO_FREE=0,

    /**
    Value to specify that the tasks will release the resources by calling the
    function #aui_os_task_delete when it returns
    */
    AUI_TASK_RTN_MODE_MANUAL_FREE,

    /**
    Value to specify an invalid task return mode
    */
    AUI_TASK_RTN_MODE_LAST

} aui_task_rtn_mode, *aui_p_task_rtn_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify the heap
        information
        </div> @endhtmlonly

        Struct to specify the heap information
*/
typedef struct aui_st_heap_info {

    /**
    Member to specify the <b> Heap Start Address </b>
    */
    unsigned char *base;

    /**
    Member to specify the <b> Heap Length </b>
    */
    unsigned long len;

    /**
    Member to specify the @b handle to access the heap
    */
    unsigned long handle;

    /**
    Member to specify the <b> total number of malloc </b> called from the heap
    */
    unsigned long total_num;

    /**
    Member to specify the <b> total malloc memory size </b> of the heap
    (in @a bytes unit)
    */
    unsigned long total_malloced;

    /**
    Member to specify the <b> free memory size </b> of the heap.
    */
    unsigned long free;

    /**
    @attention  This member is reserved to ALi R&D Dept., user can ignore it
    */
    unsigned long max;

} aui_heap_info, *aui_p_heap_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify the
        memory pool attribute
        </div> @endhtmlonly

        Struct to specify the memory pool attribute
*/
typedef struct aui_st_attr_mempool {

    /**
    Member to specify the <b> Heap Start Address </b>.
    */
    unsigned char *base;

    /**
    Member to specify the <b> Heap Length </b>
    */
    unsigned long len;

    /**
    Member to specify the <b> Pool Name </b>
    */
    char sz_name[AUI_DEV_NAME_LEN+1];

} aui_attr_mempool, *aui_p_attr_mempool;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify the
        timer attribute
        </div> @endhtmlonly

        Struct to specify the timer attribute
*/
typedef struct aui_st_attr_timer {

    /**
    Member to specify <b> Timer Name </b>
    */
    char    sz_name[AUI_DEV_NAME_LEN+1];

    /**
    Member to specify the <b> Timer Callback Function </b> as per comment for
    the function pointer #aui_p_fun_timer_cb
    */
    aui_p_fun_timer_cb timer_call_back;

    /**
    Member to specify the <b> Timer Type </b>, in particular
    - @b 1 = Alarm once
    - @b 2 = Loop alarm
    */
    unsigned long    ul_type;

    /**
    Member to specify the <b> Timeout Value </b> of the  timer
    (in @a millisecond (ms) unit)
    */
    unsigned long    ul_time_out;

    /**
    Member to specify the @b parameter of the <b> timer callback function </b>
    */
    unsigned long    ul_para1;

    /**
    Member to specify the <b> initial value </b> of the timer
    (in @a millisecond (ms) unit))
    */
    unsigned long    ul_init_val;

    /**
    Member to specify the <b> maximum value </b> of the timer
    (in @b millisecond (ms))
    */
    unsigned long    ul_max_val;

} aui_attr_timer, *aui_p_attr_timer;

/**
Enum to specify different setting for the timer
*/
typedef enum aui_en_timer_item_set {

    /**
    Value to specify the <b> Timer Type </b>.
    */
    AUI_TIMER_TYPE_SET=0,

    /**
    Value to specify the <b> Delay Value </b>
    (in @b ticks, where @b 1 tick = @b 1 millisecond (ms))
    */
    AUI_TIMER_DELAY_SET,

    /**
    Value to specify the <b> Current Value </b>
    */
    AUI_TIMER_VAL_SET,

    /**
    Value to specify the total number of items available in this enum
    #aui_timer_item_set
    */
    AUI_TIMER_GET_CMD_LAST

} aui_timer_item_set;

/**
Enum to specify different items to get from the timer
*/
typedef enum aui_en_timer_item_get {

    /**
    Value to specify the time interval from the start system to the present
    (in @a second unit)
    */
    AUI_TIMER_SECOND_GET=0,

    /**
    Value to specify the time interval from the start system to the present
    (in @b ticks, where @1 tick = @1 millisecond (ms) unit)
    */
    AUI_TIMER_TICK_GET,

    /**
    Value to specify the total number of items available in this enum
    #aui_timer_item_set
    */
    AUI_TIMER_SET_CMD_LAST

} aui_timer_item_get;

/**
Enum to specify <b> Recursive/Normal Lock </b>
*/
typedef enum aui_en_mutex_lock_mode {

    /**
    Value to specify that Mutex doesn't support the recursive lock then the
    normal lock
    */
    AUI_MUTEX_NORMAL,

    /**
    Value to specify that Mutex supports the recursive lock
    */
    AUI_MUTEX_RECURSIVE

} aui_mutex_lock_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Operating System (OS) Module </b> to specify OS
        attributes
        </div> @endhtmlonly

        Struct to specify OS attributes
*/
typedef struct aui_st_attr_os {

    /**
    Member to specify the return mode of the user task, as defined in the enum
    #aui_task_rtn_mode
    */
    aui_task_rtn_mode task_rtn_mode;

    /**
    Member to specify the system large heap information, as defined in the enum
    #aui_heap_info.

  @note  If user configures this member the uses the function #aui_os_malloc
         to allocate a heap buffer which size is larger than the value
         specified with the member #malloc_limit_size of this struct, the
         system will allocate memory from this heap.
    */
    aui_heap_info large_heap;

    /**
    Member to specify the system small heap information, as defined in the enum
    #aui_heap_info.

    @note  If user configures this member then uses the function #aui_os_malloc
           to allocate a heap buffer which size is smaller than the value specifed
           with the member #malloc_limit_size of this struct, the system will
           allocate memory from this heap.
    */
    aui_heap_info min_heap;

    /**
    Member as the callback function used for changing the user task priority into
    the system task priority list, as per comment for the function pointer
    #aui_p_fun_task_prio_adapt
    */
    aui_p_fun_task_prio_adapt p_fun_task_prio_adapt;

    /**
    Member to specify the maximum user event number
    */
    unsigned short event_num;

    /**
    Member to specify a limit size to determine large or small heap buffer
    as defined in the member @b large_heap or @b min_heap of this struct
    respectively

    @warning    Generally it should be set to the value zero (0).
    */
    unsigned long malloc_limit_size;

    /**
    Member to specify the multi event working mode, as defined in the enum
    #aui_muti_event_mode
    */
    aui_muti_event_mode muti_event_mode;

    /**
    Member to specify the working mode for the semaphore, as defined in the
    enum #aui_sem_work_mode
    */
    aui_sem_work_mode sem_work_mode;

    /**
    @attention  This member is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    unsigned long reserve;

} aui_attr_os, *aui_p_attr_os;


/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to get the version number of the OS Module as
                defined in the macro #AUI_MODULE_VERSION_NUM_OS

@param[out]     pul_version             = Pointer to the version number of the
                                          OS Module

@return         @b AUI_RTN_SUCCESS      = Getting of the version number of the
                                          OS Module performed successfully
@return         @b AUI_RTN_EINVAL       = The output parameter (i.e. [out]) is
                                          invalid
@return         @b Other_Values         = Getting of the version number of the
                                          OS Module failed for some reason
*/
AUI_RTN_CODE aui_os_version_get (

    unsigned long* const pul_version

    );

/**
@brief          Function used to initialize the OS module.

@param[in]      p_os_mod_attr           = Pointer to a struct #aui_attr_os,
                                          which collects the desired attributes
                                          for the OS Module

@return         @b AUI_RTN_SUCCESS      = OS module initialized successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Initializing of the OS Module failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_init (

    aui_attr_os *p_os_mod_attr

    );

/**
@brief          Function used to De-Initialize the OS module.

@param[in]      p_call_back_init        = Callback function used to de-initialize
                                          the OS module as per comment for the
                                          function pointer #p_fun_cb
@param[in]      pv_param                = Input parameter of the callback
                                          function @b p_call_back_init

@return         @b AUI_RTN_SUCCESS      = OS module de-initialized successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e.
                                          [in]) is invalid
@return         @b Other_Values         = De-initializing of the OS Module
                                          failed for some reasons
*/
AUI_RTN_CODE aui_os_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to create a task for OS Module

@param[in]      p_task_attr             = Pointer to a struct #aui_attr_task,
                                          which collects the attributes related
                                          to the task to be created

@param[out]     pp_hdl_task             = #aui_hdl pointer to the handle of the
                                          task just created

@return         @b AUI_RTN_SUCCESS      = Creating of the task performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the task failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_task_create (

    const aui_attr_task *p_task_attr,

    aui_hdl* const pp_hdl_task

    );

/**
@brief          Function used to delete a task from OS Module.

@param[in]      p_hdl_task              = #aui_hdl pointer to the handle of the
                                          task to be deleted.
@param[in]      ul_force_kill           = Flag used to force the deleting of a
                                          task, in particular
                                          - 1 = force deleting;
                                          - 0 = Don't force deleting
                                                - @b Caution: In this case, the
                                                     return value of this function
                                                     will be FALSE when deleting
                                                     a running task

@return         @b AUI_RTN_SUCCESS      = Deleting of the task performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Deleting of the task failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_task_delete (

    aui_hdl *p_hdl_task,

    unsigned long ul_force_kill

    );


/**
@brief          Function used to get the task attributes information

@warning        This function is in pair with the function #aui_os_task_attr_set
                and is reserved for future use then user can ignore it

@param[in]      hdl_task                = #aui_hdl handle of the task already
                                          created and to be managed for getting
                                          task attribute information
@param[in]      p_task_attr             = Pointer to a struct #aui_attr_task,
                                          which collects the attributes related
                                          to the task to be gotten

@return         @b AUI_RTN_SUCCESS      = Getting of the task attributes
                                          information performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Getting of the task attributes
                                          information failed for some reasons
*/
AUI_RTN_CODE aui_os_task_attr_get (

    aui_hdl hdl_task,

    aui_attr_task* const p_task_attr

    );

/**
@brief          Function used to make sleeping the current task for a short
                time interval

@param[in]      ul_time_ms              = Task sleeping time, which @b must be
                                          &ge; @b 1ms

@return         @b AUI_RTN_SUCCESS      = Setting of the task sleeping time
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the task sleeping time
                                          failed for some reasons
*/
AUI_RTN_CODE aui_os_task_sleep (

    unsigned long ul_time_ms

    );

/**
@brief          Function used to yield the current task for a short time interval
                then the CPU will be released to shedule running another task

@param[in]      hdl_task                = #aui_hdl handle of the current task

@return         @b AUI_RTN_SUCCESS      = Yielding of the current task performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Yielding of the current task failed
                                          for some reasons

@note           By this function the current task will go into sleep mode for
                the task time slice specified in the member @b ul_quantum of
                the struct #aui_st_attr_task
*/
AUI_RTN_CODE aui_os_task_yield (

    aui_hdl hdl_task

    );

/**
@brief          Function used to get the handle of the current task

@param[out]     pp_hdl_task             = #aui_hdl pointer to the handle of the
                                          current task just gotten

@return         @b AUI_RTN_SUCCESS      = Getting of the handle of the current
                                          task performed successfully
@return         @b AUI_RTN_EINVAL       = The parameter (i.e. [out]) is invalid
@return         @b Other_Values         = Getting of the handle of the current
                                          task failed for some reasons
*/
AUI_RTN_CODE aui_os_task_get_self_hdl (

    aui_hdl* const pp_hdl_task

    );


/**
@brief          Function used to get the id of specified task

@param[in]      hdl_task                = #aui_hdl pointer to the handle specified task
                                          When it is NULL, means get current task's id

@param[out]     task_id                 = The output task id of specified task

@return         @b AUI_RTN_SUCCESS      = Getting of the task id of the specified task
                                          performed successfully
@return         @b Other_Values         = Getting of the task id of the specified task
                                          failed for some reasons

@note           There are two ways to get current task's id

                1. Get the current task handle by API #aui_os_task_get_self_hdl, then 
                   use this API to get task id
                2. Use this API directly, but set parameter #hdl_task = NULL.

*/
AUI_RTN_CODE aui_os_task_id_get (

    aui_hdl hdl_task,

    unsigned long *task_id

    );

/**
@brief          Function used to get the total number of user task created by
                the function #aui_os_task_create

@param[out]     pul_task_cnt            = Pointer to the total number of user
                                          task already created

@return         @b AUI_RTN_SUCCESS      = Getting of the total number of user
                                          task performed successfully
@return         @b AUI_RTN_EINVAL       = The output parameter (i.e. [out]) is
                                          invalid
@return         @b Other_Values         = Getting of the total number of user
                                          task failed for some reasons
*/
AUI_RTN_CODE aui_os_task_cnt (

    unsigned long* const pul_task_cnt

    );

/**
@brief          Function used for joining a new tasks.

@param[in]      hdl_task                = #aui_hdl pointer to the handle of the
                                          new task to be joined
@param[in]      ul_time_out             = Timeout interval time
                                          (in @a second unit)

@return         @b AUI_RTN_SUCCESS      = Joining of the new task performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Joining of the new task failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_task_join (

    aui_hdl hdl_task,

    unsigned long ul_time_out

    );

/**
@brief          Function used to create a message queue.

@param[in]      p_msgq_attr             = Pointer to a struct #aui_st_attr_msgq,
                                          which collects the attributes related
                                          to the message queue to be created

@param[out]     pp_hdl_msgq             = #aui_hdl pointer to the handle of the
                                          message queue just created.

@return         @b AUI_RTN_SUCCESS      = Creating of the message queue performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the message queue failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_msgq_create (

    const aui_attr_msgq *p_msgq_attr,

    aui_hdl* const pp_hdl_msgq

    );

/**
@brief          Function used to delete a message queue already created by the
                function #aui_os_msgq_create

@param[in]      pp_hdl_msgq             = #aui_hdl pointer to the handle of the
                                          message queue already created and to
                                          be deleted

@return         @b AUI_RTN_SUCCESS      = Deleting of the message queue performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the message queue failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_msgq_delete (

    aui_hdl *pp_hdl_msgq

    );

/**
@brief          Function used to receive message from a message queue.

@param[in]      p_hdl_msgq              = #aui_hdl handle of the message queue
                                          already created
@param[in]      pv_msg                  = Pointer to the buffer used for receiving
                                          messages from the message queue
@param[in]      ul_msg_size             = Maximum length for a received message
@param[in]      ul_time_out             = Timeout (in @a millisecond (ms) unit)
                                          for receiving messages from the message
                                          queue

@param[out]     pul_actual_size         = Actual length of the received message

@return         @b AUI_RTN_SUCCESS      = Receiving of the message from the
                                          message queue performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Receiving of the message from the
                                          message queue failed for some reasons
*/
AUI_RTN_CODE aui_os_msgq_rcv (

    aui_hdl p_hdl_msgq,

    void *pv_msg,

    unsigned long ul_msg_size,

    unsigned long* const pul_actual_size,

    unsigned long ul_time_out

    );

/**
@brief          Function used to send message to a message queue

@param[in]      p_hdl_msgq              = #aui_hdl handle of the message queue
                                          already created
@param[in]      pv_msg                  = Pointer to the buffer used for sending
                                          messages to a message queue
@param[in]      ul_msg_size             = Maximum length for a sent message
@param[in]      ul_time_out             = Timeout (in @a millisecond (ms) unit)
                                          for sending messages to the message queue

@return         @b AUI_RTN_SUCCESS      = Sending of the message to the message
                                          queue performed successfully.
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Receiving of the message to the messages
                                          queue failed for some reasons
*/
AUI_RTN_CODE aui_os_msgq_snd (

    aui_hdl p_hdl_msgq,

    void *pv_msg,

    unsigned long ul_msg_size,

    unsigned long ul_time_out

    );


/**
@brief          Function used to get the message queue attributes

@param[in]      p_hdl_msgq              = #aui_hdl handle of the message queue
                                          already created and to be managed for
                                          getting the message queue attributes

@param[out]     p_msgq_attr             = Pointer to a struct #aui_st_attr_msgq,
                                          which collects the message queue
                                          attributes

@return         @b AUI_RTN_SUCCESS      = Getting of the message attributes
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the message queue attributes
                                          failed for some reasons
*/
AUI_RTN_CODE aui_os_msgq_attr_get (

    aui_hdl p_hdl_msgq,

    aui_attr_msgq* const p_msgq_attr

    );

/**
@brief          Function used to create a semaphore.

@param[in]      p_sem_attr              = Pointer to a struct #aui_attr_sem,
                                          which collects the attributes related
                                          to the semaphore to be created

@param[out]     pp_hdl_sem              = #aui_hdl pointer to the handle of the
                                          semaphore just created

@return         @b AUI_RTN_SUCCESS      = Creating of the semaphore performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the semaphore failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_sem_create (

    const aui_attr_sem *p_sem_attr,

    aui_hdl* const pp_hdl_sem

    );

/**
@brief          Function used to delete a semaphore.

@param[in]      pp_hdl_sem              = #aui_hdl pointer to the handle of the
                                          semaphore already created and to be
                                          deleted

@return         @b AUI_RTN_SUCCESS      = Deleting of the semaphore performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the semaphore failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_sem_delete (

    aui_hdl *pp_hdl_sem

    );

/**
@brief          Function used to keep the semaphore in waiting state

@param[in]      p_hdl_sem               = #aui_hdl handle of the semaphore
                                          already created
@param[in]      ul_time_out             = Interval time (in @a millisecond (ms)
                                          unit) of the waiting state of the
                                          semaphore

@return         @b AUI_RTN_SUCCESS      = Keeping of the semaphore in the waiting
                                          state performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Keeping of the semaphore in the waiting
                                          state failed for some reasons
*/
AUI_RTN_CODE aui_os_sem_wait (

    aui_hdl p_hdl_sem,

    unsigned long ul_time_out

    );

/**
@brief          Function used to release a semaphore then the related resources

@param[in]      p_hdl_sem               = #aui_hdl handle of the semaphore
                                          already created

@return         @b AUI_RTN_SUCCESS      = Releasing of the semaphore performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Releasing of the semaphore failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_sem_release (

    aui_hdl p_hdl_sem

    );

/**
@brief          Function used to get the semaphore attributes

@param[in]      p_hdl_sem               = #aui_hdl handle of the semaphore
                                          already created and to be managed for
                                          getting semaphore attributes

@param[out]     p_sem_attr              = Pointer to a struct #aui_attr_sem,
                                          which collects the attributes for the
                                          semaphore

@return         @b AUI_RTN_SUCCESS      = Getting of the semaphore attributes
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the semaphore attributes
                                          failed for some reasons
*/
AUI_RTN_CODE aui_os_sem_attr_get (

    aui_hdl p_hdl_sem,

    aui_attr_sem* const p_sem_attr

    );

/**
@brief          Function used to create a mutex.

@param[in]      p_mutex_attr            = Pointer to a struct #aui_st_attr_mutex,
                                          which collects the attributes related
                                          to the mutex to be created

@param[out]     pp_hdl_mutex            = #aui_hdl pointer to the handle of the
                                          mutex just created

@return         @b AUI_RTN_SUCCESS      = Creating of the mutex performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the mutex failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_mutex_create (

    const aui_attr_mutex *p_mutex_attr,

    aui_hdl* const pp_hdl_mutex

    );

/**
@brief          Function used to delete a mutex

@param[in]      pp_hdl_mutex            = #aui_hdl pointer to the handle of the
                                          mutex already created and to be deleted

@return         @b AUI_RTN_SUCCESS      = Deleting of the mutex performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the mutex failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_mutex_delete (

    aui_hdl *pp_hdl_mutex

    );

/**
@brief          Function used to keep the mutex in locking state

@param[in]      p_hdl_mutex             = #aui_hdl handle of the mutex already
                                          created and to be locked.
@param[in]      ul_time_out             = Interval time (in @a millisecond (ms)
                                          unit) of the locking state of the mutex

@return         @b AUI_RTN_SUCCESS      = Keeping of the mutex in the locking
                                          state performed successfully.
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Keeping of the mutex in the locking
                                          state failed for some reasons
*/
AUI_RTN_CODE aui_os_mutex_lock (

    aui_hdl p_hdl_mutex,

    unsigned long ul_time_out

    );

/**
@brief          Function used to unlock a mutex already in locking state.

@param[in]      p_hdl_mutex             = #aui_hdl handle of the mutex already
                                          created, locked and to be unlocked

@return         @b AUI_RTN_SUCCESS      = Unlocking of the mutex performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Unlocking of the mutex failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_mutex_unlock (

    aui_hdl p_hdl_mutex

    );

/**
@brief          Function used to try locking a mutex (depending of the availability
                of resources).\n

@param[in]      p_hdl_mutex             = #aui_hdl handle of the mutex already
                                          created and to be locked (trying)

@return         @b AUI_RTN_SUCCESS      = Locking of the mutex performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Locking of the mutex failed for some
                                          reasons (including the unavailability
                                          of resources for locking the mutex)
*/
AUI_RTN_CODE aui_os_mutex_try_lock (

    aui_hdl p_hdl_mutex

    );

/**
@brief          Function used to create an event.

@param[in]      p_event_attr            = Pointer to a struct #aui_st_attr_event,
                                          which collects the attributes related
                                          to the event to be created

@param[out]     pp_hdl_event            = #aui_hdl pointer to the handle of the
                                          event just created.

@return         @b AUI_RTN_SUCCESS      = Creating of the event created
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the event failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_event_create (

    const aui_attr_event *p_event_attr,

    aui_hdl* const pp_hdl_event

    );

/**
@brief          Function used to delete an event.

@param[in]      pp_hdl_event            = #aui_hdl pointer to the handle of the
                                          event already created and to be deleted

@return         @b AUI_RTN_SUCCESS      = Deleting of the event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter is (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the event failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_event_delete (

    aui_hdl *pp_hdl_event

    );

/**
@brief          Function used to set the event.

@param[in]      p_hdl_event             = #aui_hdl handle of the event already
                                          created and to be set
@param[in]      ul_enable               = This parameter is reserved to ALi R&D
                                          Dept., user can set it to the value
                                          zero (0)

@return         @b AUI_RTN_SUCCESS      = Setting of the event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the event failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_event_set (

    aui_hdl p_hdl_event,

    unsigned long ul_enable

    );

/**
@brief          Function used by the wait task to identify if the event occurred
                in the pre-set waiting time interval.\n
                In particular, if the event occurred and the parameter
                @b is_autorst flag is set to @b 1 then the event will be
                auto-reset by the task

@param[in]      p_hdl_event             = #aui_hdl handle of the event already
                                          set and to be managed to switch into
                                          waiting state
@param[in]      ul_time_out             = Interval time (in @a millisecond (ms)
                                          unit) of the waiting state of the event
@param[in]      is_autorst              = Flag for auto-reset the event, in
                                          particular
                                          - @b 1 = The event will be auto-reset
                                                   by the wait task
                                          - @b 0 = Nothing happens

@return         @b AUI_RTN_SUCCESS      = Event wait task performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Event wait task failed for some reasons
*/
AUI_RTN_CODE aui_os_event_wait (

    aui_hdl p_hdl_event,

    unsigned long ul_time_out,

    int is_autorst

    );

/**
@brief          Function used to clear an event.

@param[in]      p_hdl_event             = #aui_hdl handle of the event already
                                          created and to be cleared.

@return         @b AUI_RTN_SUCCESS      = Clearing of the event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Clearing of the event failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_event_clear (

    aui_hdl p_hdl_event

    );

/**
@brief          Function used to get the event information.

@param[in]      p_hdl_event             = #aui_hdl handle of the event already
                                          created and to be managed for getting
                                          event information.

@return         @b AUI_RTN_SUCCESS      = Getting of the event information
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Getting of the event information failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_event_info_get (

    aui_hdl p_hdl_event

    );

/**
@brief          Function used to create a timer.

@param[in]      p_timer_attr            = Pointer to a struct #aui_st_attr_timer,
                                          which collects the attributes related
                                          to the timer to be created.

@param[out]     pp_hdl_timer            = #aui_hdl pointer to the handle of the
                                          timer just created

@return         @b AUI_RTN_SUCCESS      = Creating of the timer performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the timer failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_timer_create (

    const aui_attr_timer *p_timer_attr,

    aui_hdl* const pp_hdl_timer

    );

/**
@brief          Function used to delete a timer.

@param[in]      pp_hdl_timer            = #aui_hdl pointer to the handle of the
                                          timer already created and to be deleted

@return         @b AUI_RTN_SUCCESS      = Deleting of the timer performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the timer failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_timer_delete (

    aui_hdl *pp_hdl_timer

    );

/**
@brief          Function used to reset a timer.

@param[in]      p_hdl_timer             = #aui_hdl handle of timer already
                                          created and to be reset

@return         @b AUI_RTN_SUCCESS      = Resetting of the timer performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Resetting of the timer failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_timer_rst (

    aui_hdl p_hdl_timer

    );

/**
@brief          Function used to start/stop the timer.

@param[in]      p_hdl_timer             = #aui_hdl handle of timer already
                                          created and to be started/stopped
@param[in]      ul_enable               = Flag to start/stop the timer, in
                                          particular
                                          - @b 1 = Start
                                          - @b 0 = Stop

@return         @b AUI_RTN_SUCCESS      = Starting/Stopping of the timer
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Starting/Stopping of the timer failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_timer_run (

    aui_hdl p_hdl_timer,

    unsigned long ul_enable

    );

/**
brief           Function used to set the timer

@param[in]      p_hdl_timer             = #aui_hdl handle of the timer already
                                          created and to be set
@param[in]      ul_item                 = The item related to the specific
                                          setting of the timer to be performed,
                                          as defined in the enum
                                          #aui_en_timer_item_set
@param[in]      pv_param                = Pointer to the specific setting of the
                                          timer to be performed, as defined in
                                          the enum #aui_en_timer_item_set

@return         @b AUI_RTN_SUCCESS      = Setting of the timer performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the timer failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_timer_set (

    aui_hdl p_hdl_timer,

    unsigned long ul_item,

    void *pv_param

    );

/**
brief           Function used to get the settings of the timer

@param[in]      p_hdl_timer             = #aui_hdl handle of the timer already
                                          created and to be managed for getting
                                          the setting of the timer
@param[in]      ul_item                 = The item related to the specific
                                          setting of the timer to be gotten,
                                          as defined in the enum
                                          #aui_en_timer_item_get

@param[out]     pv_param                = Pointer to the specific setting of the
                                          timer to be gotten, as defined in the
                                          enum #aui_en_timer_item_get
                                          - @b Caution: This parameter could be
                                               set as NULL when the parameter
                                               @b ul_item is set to the value
                                               #AUI_TIMER_SECOND_GET or
                                               #AUI_TIMER_TICK_GET
@return         @b AUI_RTN_SUCCESS      = Getting of the setting of the timer
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = A least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Getting of the setting of the timer
                                          failed for some reasons
*/
AUI_RTN_CODE aui_os_timer_get (

    aui_hdl p_hdl_timer,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to allocate a memory block in the user heap buffer
                (i.e. in the one allocate as specified in the member @b large_heap
                or @b min_heap of the struct #aui_attr_os)

@warning        If no heap buffer is configured by user, memory will be allocated
                from the TDS system heap.

@param[in]      ul_size                 = Size (in @a bytes unit) of the memory
                                          block to be allocated in the heap

@return         This function can return
                - Either a pointer to the allocated memory block in the heap,
                  if performed successfully
                - Or NULL, if the allocation of the memory block in the heap
                  failed for some reasons
*/
void* aui_os_malloc (

    unsigned long ul_size

    );

/**
@brief          Function used to resize a memory block already allocated in the
                user heap buffer by the function #aui_os_malloc.

@warning        If no heap buffer is configures by user, memory will be allocated
                from the TDS system heap


@param[in]      p_addr                  = Pointer to the memory block already
                                          allocated in the heap and to be resized
@param[in]      ul_size                 = New size (in @a bytes unit) of the
                                          memory block in the heap

@return         This function can return
                - Either the pointer to the resized memory block in the heap,
                  if performed successfully.
                - Or NULL, if the resizing of the memory block in the heap
                  failed for some reasons
*/
void* aui_os_re_malloc (

    void *p_addr,

    unsigned long ul_size

    );

/**
@brief          Function used to allocate and initialize to the zero (0) value
                a memory block in the user heap buffer with dimensions 
                @b ul_elem_cnt * @b ul_size

@warning        If no heap buffer is configured by user, memory will be allocated
                from the TDS system heap.

@param[in]      ul_elem_cnt             = Number of elements of the memory block
                                          to be allocated in the heap
@param[in]      ul_size                 = Size (in @a bytes unit) of each element
                                          of the memory block to be allocated in
                                          the heap

@return         This function can return
                - Either a pointer to the allocated memory block in the heap,
                  if performed successfully
                - Or NULL, if the allocation of the memory block in the heap
                  failed for some reasons
*/
void* aui_os_calloc (

    unsigned long ul_elem_cnt,

    unsigned long ul_size

    );

/**
@brief          Function used to deallocate a memory block already allocated in
                the heap by the functions either #aui_os_malloc or
                #aui_os_re_malloc or #aui_os_calloc.

@param[in]      p_addr                  = Pointer to the memory block in the
                                          heap to be deallocated

@return         @b AUI_RTN_SUCCESS      = Memory block in the heap deallocated
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Memory block in the heap not
                                          deallocated for some reasons
*/
AUI_RTN_CODE aui_os_free (

    void *p_addr

    );

/**
@brief          Function used to get the memory attributes of the large heap,
                i.e. of the one allocated as specified in the member @b large_heap
                of the struct #aui_attr_os

@param[out]     p_mem_info              = Pointer to a struct #aui_mem_info,
                                          which collect the memory attributes

@return         @b AUI_RTN_SUCCESS      = Getting of the memory attributes
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Getting of the memory attributes
                                          failed for some reasons
*/
AUI_RTN_CODE aui_os_mem_info_get (

    aui_mem_info* const p_mem_info

    );

/**
@brief          Function used to create a memory pool.

@param[in]      p_pool_attr             = Pointer to a struct
                                          #aui_st_attr_mempool, which collects
                                          the attributes related to memory pool
                                          to be created

@param[out]     pp_hdl_pool             = #aui_hdl pointer to the handle of the
                                          memory pool just created

@return         @b AUI_RTN_SUCCESS      = Creating of the memory pool performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the memory pool failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_mempool_create (

    const aui_attr_mempool *p_pool_attr,

    aui_hdl* const pp_hdl_pool

    );

/**
@brief          Function used to delete a memory pool.

@param[in]      pp_hdl_mempool          = #aui_hdl pointer to the handle of the
                                          memory pool already created and to be
                                          deleted

@return         @b AUI_RTN_SUCCESS      = Deleting of the memory pool performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the memory pool failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_mempool_delete (

    aui_hdl *pp_hdl_mempool

    );

/**
@brief          Function used to allocate a memory block in the memory pool
                already created by the function #aui_os_mempool_create

@param[in]      p_hdl_mempool           = #aui_hdl handle of the memory pool
                                          already created and to be managed to
                                          allocate a memory block into it
@param[in]      ul_size                 = Size (in @a bytes unit) of the memory
                                          block to be allocated in the memory
                                          pool

@return         This function can return
                - Either a pointer to the allocated memory block in the memory
                  pool, if performed successfully
                - Or NULL, if the allocation of the memory block in the memory
                  pool failed for some reasons
*/
void* aui_os_mempool_malloc (

    aui_hdl p_hdl_mempool,

    unsigned long ul_size

    );

/**
@brief          Function used to deallocate a memory block already allocated in
                the memory pool by the function #aui_os_mempool_malloc

@param[in]      p_hdl_mempool           = #aui_hdl handle of the memory pool
                                          already created and to be managed to
                                          deallocate a memory block from it
@param[in]      pv_addr                 = Pointer to the memory block to be
                                          deallocated from the memory pool

@return         @b AUI_RTN_SUCCESS      = Memory block in the memory pool
                                          deallocated successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Memory block in the memory pool not
                                          deallocated for some reasons
*/
AUI_RTN_CODE aui_os_mempool_free (

    aui_hdl p_hdl_mempool,

    void *pv_addr

    );

/**
@brief          Function used to get the current count of messages in queue

@param[in]      p_hdl_msgq              = #aui_hdl handle of the message queue
                                          already created

@param[out]     pul_msgq_cnt            = Pointer to the current count of
                                          messages in queue

@return         @b AUI_RTN_SUCCESS      = Getting of the current count of
                                          messages in queue performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid.
@return         @b Other_Values         = Getting of the current count of
                                          messages in queue failed for some
                                          reasons
*/
AUI_RTN_CODE aui_os_msgq_cnt_get (

    aui_hdl p_hdl_msgq,

    unsigned long* const pul_msgq_cnt

    );

/**
@brief          Function used to create a multi event.

@param[in]      p_muti_event_attr       = Pointer to a struct
                                          #aui_st_attr_muti_event, which
                                          collects the attributes for the multi
                                          event to be created
@param[in]      pul_events              = Pointer to a multi event

@param[out]     p_hdl_muti_event        = #aui_hdl pointer to the handle of the
                                          multi event just created

@return         @b AUI_RTN_SUCCESS      = Creating of the multi event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Creating of the multi event failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_muti_event_create (

    const aui_attr_muti_event *p_muti_event_attr,

    unsigned long* const pul_events,

    aui_hdl *p_hdl_muti_event

    );

/**
@brief          Function used to delete a multi event

@param[in]      pp_hdl_muti_event       = #aui_hdl pointer to the handle of the
                                          multi event already created and to be
                                          deleted
@param[in]      ul_events               = Event bitmap

@return         @b AUI_RTN_SUCCESS      = Deleting of the multi event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Deleting of the multi event failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_muti_event_delete (

    aui_hdl *pp_hdl_muti_event,

    unsigned long ul_events

    );

/**
@brief          Function used to set a multi event.

@param[in]      hdl_muti_event          = #aui_hdl handle of the multi event
                                          already created and to be set
@param[in]      task_id                 = #aui_hdl handle used for receiving
                                          the events
@param[in]      ul_sent_events          = Event bitmap

@return         @b AUI_RTN_SUCCESS      = Setting of the multi event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the multi event failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_muti_event_set (

    aui_hdl hdl_muti_event,

    aui_hdl task_id,

    unsigned long ul_sent_events

    );

/**
@brief          Function used by the wait task to identify if the multi event
                occurred in the pre-set waiting time interval.

@param[in]      hdl_muti_event          = #aui_hdl handle of multi event already
                                          created and to be managed for
                                          performing a setting
@param[in]      en_option               = Option to define the trigger condition
                                          of the multi event
@param[in]      ul_expected_events      = The expected event (i.e. bits) to be
                                          received
@param[in]      pul_received_events     = Pointer to the actual event (i.e.
                                          bits) received
@param[in]      ul_timeout              = Timeout (in @a millisecond (ms))

@return         @b AUI_RTN_SUCCESS      = Multi event wait task performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = Either one or both of the parameter
                                          (i.e. [in], [out]) are invalid
@return         @b Other_Values         = Multi event failed for some reasons
*/
AUI_RTN_CODE aui_os_muti_event_wait (

    aui_hdl hdl_muti_event,

    aui_muti_event_option en_option,

    unsigned long ul_expected_events,

    unsigned long *pul_received_events,

    unsigned long ul_timeout

    );

/**
@brief          Function used to clear a multi event

@param[in]      hdl_muti_event          = #aui_hdl handle of the multi event
                                          already created and to be cleared
@param[in]      ul_clear_event          = The event (i.e. bits) to be cleared
@param[in]      task_id                 = #aui_hdl handle of the events to
                                          be cleared

@return         @b AUI_RTN_SUCCESS      = Clearing of the multi event performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Clearing of the multi event failed for
                                          some reasons
*/
AUI_RTN_CODE aui_os_muti_event_clear (

    aui_hdl hdl_muti_event,

    aui_hdl task_id,

    unsigned long ul_clear_event

    );

/**
@brief          Function used to register a callback function used for the High
                Priority Service Routing (HPSR)

@param[in]      call_back               = The callback function to be registered
                                          for the HPSR, as per comment for the
                                          function pointer #aui_os_hsr_func_cb
@param[in]      p_void                  = The input parameter of the callback
                                          function to be registered

@return         @b AUI_RTN_SUCCESS      = Registering of the callback function
                                          for HPSR performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Registering of the callback function
                                          for HPSR failed for some reasons
*/
AUI_RTN_CODE aui_os_hsr_reg (

    aui_os_hsr_func_cb call_back,

    void *p_void

    );

/**
@brief          Function used to register a callback function used for the Low
                Priority Service Routing (LPSR)

@param[in]      irq_num                 = Interrupt number for registering the
                                          callback function for LPSR
@param[in]      call_back               = The callback function to be registered
                                          for the LPSR, as per comment for the
                                          function pointer #aui_os_hsr_func_cb
@param[in]      p_void                  = The input parameter of the callback
                                          function to be registered

@return         @b AUI_RTN_SUCCESS      = Registering of the callback function
                                          for LPSR performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Registering of the callback function
                                          for LPSR failed for some reasons
*/
AUI_RTN_CODE aui_os_lsr_reg (

    unsigned short irq_num,

    aui_os_hsr_func_cb call_back,

    void *p_void

    );

/**
@brief          Function used to un-register a callback function used for the
                Low Priority Service Routing (LPSR)

@param[in]      irq_nb                  = Interrupt number for un-registering
                                          the callback function for LPSR
@param[in]      call_back               = The callback function for the LPSR to
                                          be un-registered , as per comment for
                                          the function pointer #aui_os_hsr_func_cb

@return         @b AUI_RTN_SUCCESS      = Un-registering of the callback function
                                          for LPSR performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Un-registering of the callback function
                                          for LPSR failed for some reasons
*/
AUI_RTN_CODE aui_os_lsr_unreg (

    unsigned short irq_nb,

    aui_os_hsr_func_cb call_back

    );

/**
@brief          Function used to start the protection of critical data while the
                interrupt will be closed

@return         @b AUI_RTN_SUCCESS      = Starting of the protection of critical
                                          data performed successfully
@return         @b Other_Values         = Starting of the protection of critical
                                          data failed for some reasons
*/
AUI_RTN_CODE aui_os_crit_start (

    void

    );

/**
@brief          Function used to stop the protection of critical data after
                starting by the function #aui_os_crit_start

@return         @b AUI_RTN_SUCCESS      = Stopping of the protection of critical
                                          data performed successfully
@return         @b Other_Values         = Stopping of the protection of critical
                                          data failed for some reasons
*/
AUI_RTN_CODE aui_os_crit_stop (

    void

    );

/**
@brief          Function used to start the protection of the critical data while
                the interrupt is still open but the task schedule is close

@return         @b AUI_RTN_SUCCESS      = Starting of the protection of critical
                                          data performed successfully
@return         @b Other_Values         = Starting of the protection of critical
                                          data failed for some reasons
*/
AUI_RTN_CODE aui_os_task_crit_start (

    void

    );

/**
@brief          Function used to stop the protection of critical data after
                starting by the function #aui_os_task_crit_start

@return         @b AUI_RTN_SUCCESS      = Stopping of the protection of critical
                                          data performed successfully
@return         @b Other_Values         = Stopping of the protection of critical
                                          data failed for some reasons
*/
AUI_RTN_CODE aui_os_task_crit_stop (

    void

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

///@cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define AUI_OS_MUTI_EVENT_MODE_BROADCASE     AUI_MUTI_EVENT_MODE_BROADCASE

#define AUI_OS_MUTI_EVENT_MODE_ATTACH_TASK     AUI_MUTI_EVENT_MODE_ATTACH_TASK

#define AUI_OS_MUTI_EVENT_MODE_LAST     AUI_MUTI_EVENT_MODE_LAST

#define AUI_OS_SEM_MODE_RELEASE_CHECK_MAX_CNT     AUI_SEM_MODE_RELEASE_CHECK_MAX_CNT

#define AUI_OS_SEM_MODE_RELEASE_NO_CHECK_MAX_CNT     AUI_SEM_MODE_RELEASE_NO_CHECK_MAX_CNT

#define AUI_OS_SEM_MODE_LAST     AUI_SEM_MODE_LAST

#define AUI_OS_MUTI_EVENT_OPTION_AND     AUI_MUTI_EVENT_OPTION_AND

#define AUI_OS_MUTI_EVENT_OPTION_OR     AUI_MUTI_EVENT_OPTION_OR

#define AUI_OS_MUTI_EVENT_OPTION_CLR     AUI_MUTI_EVENT_OPTION_CLR

#define AUI_OS_MUTI_EVENT_OPTION_LAST     AUI_MUTI_EVENT_OPTION_LAST

#define aui_stOSModule_attr     aui_st_attr_os

#define aui_OSModule_attr     aui_attr_os

#define aui_pOSModule_attr     aui_p_attr_os

/**
@brief          Function used to set the task attributes

@warning        This function is reserved for future use then user can ignore
                it presently

@param[in]      hdl_task                = #aui_hdl handle of the task already
                                          created and to be managed for setting
                                          the related attributes
@param[in]      p_task_attr             = Pointer to a struct #aui_attr_task,
                                          which collects the attributes related
                                          to the task to be set

@return         @b AUI_RTN_SUCCESS      = Settings of the task attributes
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Settings of the task attributes failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_task_attr_set (

    aui_hdl hdl_task,

    const aui_attr_task *p_task_attr

    );


/**
@brief          Function used to reset a message queue.

@warning        This function is @a reserved to ALi R&D Dept, then user can
                ignore it

@param[in]      p_hdl_msgq              = #aui_hdl handle of the message queue
                                          already created

@return         @b AUI_RTN_SUCCESS      = Resetting of the message queue
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Resetting of the message queue failed
                                          for some reasons
*/
AUI_RTN_CODE aui_os_msgq_rst (

    aui_hdl p_hdl_msgq

    );

#endif

///@endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */

