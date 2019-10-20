/**@file
*     @brief        AUI common utility
*     @author        ray.gong
*     @date          2013-5-21
*     @version         1.0.0
*     @note
*ali corp. all rights reserved. 2013 - 2020 copyright (C)
*/

/**
 * @mainpage
 *
 * @brief
 *      AUI interface instruction documents
 *
 *
 *    @section hist HISTORY
 *     revision 0.0.1 2013-05-20\n
 *     first draft version
 *
 *
 *
 *    @section cr COPYRIGHT
 *       copyright (c) 2013 - 2018 ALi. all rights reserved.\n
 *
 *
 */
#ifndef _AUI_COMMON_PRIV_H
#define _AUI_COMMON_PRIV_H

/****************************INCLUDE HEAD FILE************************************/
#include <aui_common.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "aui_log.h"

/** AUI version number. */
#define AUI_VERSION_NUM    (0X00010000)

/** common module version number. */
#define AUI_MODULE_VERSION_NUM_COMMON     (0X00010000)

/** AUI设备名称长度最大值 */
#define AUI_DEV_NAME_LEN_MAX    (32)

/** take semaphore operate*/
#define AUI_TAKE_SEM(x)    do                        \
    {                                \
        if(x != OSAL_INVALID_ID)                \
            osal_semaphore_capture(x,TMO_FEVR);        \
    }while(0)
/** give semaphore operate*/
#define AUI_GIVE_SEM(x)    do                        \
    {                                \
        if(x != OSAL_INVALID_ID)                \
            osal_semaphore_release(x);            \
    }while(0)

#define AUI_CREATE_DEV_MUTEX(mod_mutex_id,dev_mutex_id,modID,err_no) \
    osal_mutex_lock(mod_mutex_id,OSAL_WAIT_FOREVER_TIME);         \
    dev_mutex_id=osal_mutex_create();                 \
    if(INVALID_ID==dev_mutex_id) {                      \
        osal_mutex_unlock(mod_mutex_id);            \
        aui_rtn(modID,err_no,"\r\n_create mutex err.");    \
    }                                 \
    osal_mutex_lock(dev_mutex_id,OSAL_WAIT_FOREVER_TIME);         \
    osal_mutex_unlock(mod_mutex_id);

#if 0
#define AUI_DELETE_DEV_MUTEX(mod_mutex_id,dev_mutex_id,dev_hdl,modID,err_no) \
    osal_mutex_lock(mod_mutex_id,OSAL_WAIT_FOREVER_TIME);         \
    if(NULL==dev_hdl) { \
        aui_rtn(modID,err_no,"\r\n_dev handle is NULL.");     \
    }                                 \
    if(0!=osal_mutex_delete(dev_mutex_id)) {            \
        aui_rtn(modID,err_no,"\r\n_delete mutex err.");    \
    }                                 \
    osal_mutex_unlock(mod_mutex_id);
#endif

#define MBF_SIZE_LOG            (2408)
#define AUI_HANDLE_MAGIC_NUM    (0x5a726179)
/****************************GLOBAL TYPE************************************/


/** common module attribute */
typedef struct aui_st_common_module_attr
{
    /** this member is just for test*/
    unsigned long ul_id;
}aui_common_module_attr,*aui_p_common_module_attr;

typedef struct aui_st_module4prt
{
    aui_module_id mdl_id;
    unsigned long ul_prt_level;
}aui_module4prt;

enum aui_en_dev_status
{
    AUI_DEV_STATUS_INIT=0,
    AUI_DEV_STATUS_OPEN,
    AUI_DEV_STATUS_CLOSED,

    AUI_DEV_STATUS_LAST
};
typedef struct aui_st_dev_priv_data
{
    /** 同类设备list */
    struct aui_list_head list;
    /** 同类子设备list */
    struct aui_list_head sub_list;
    /** 设备magic number */
    unsigned long ul_dev_magic_num;
    /** 设备类型ID */
    unsigned long ul_dev_type_id;
    /** 设备子类型ID */
    unsigned long ul_dev_sub_type_id;
    /** 设备mutex */
    //OSAL_ID dev_mutex_id;
    /** share lib dev handle */
    unsigned long ul_sl_hdl;
    /** 设备id */
    unsigned long dev_idx;
    /** 设备运行状态 */
    enum aui_en_dev_status en_status;
    /** 设备名称 */
    char dev_name[AUI_DEV_NAME_LEN_MAX+1];

}aui_dev_priv_data;

typedef struct aui_st_dev_all
{
    struct aui_list_head lst_dev[AUI_MODULE_LAST];


}aui_dev_all;
/****************************GLOBAL FUNC DECLEAR************************************/

#ifdef __cplusplus
extern "C" {
#endif

extern AUI_RTN_CODE del_dev_from_list(struct aui_list_head *tar_dev,struct aui_list_head *head_dev);
extern AUI_RTN_CODE add_dev_2_list(struct aui_list_head *tar_dev,struct aui_list_head *head_dev);


/**
*     @brief        注册AUI设备
*     @author        ray.gong
*     @date              2013-6-8
*     @param[in]        dev_type        注册的设备类型
*     @param[in]        dev_hdl          注册的设备句柄
*      @return         AUI_RTN_SUCCESS 成功
*      @return         AUI_RTN_EINVAL  无效输入参数
*      @return         others        失败
*     @note              一般为AUI的开发人员使用
*
*/
AUI_RTN_CODE aui_dev_reg(aui_module_id dev_type,aui_hdl dev_hdl);
/**
*     @brief         注销AUI设备
*     @author        ray.gong
*     @date              2013-6-8
*     @param[in]        dev_type        注销的设备类型
*     @param[in]        dev_hdl           注销的设备句柄
*      @return          AUI_RTN_SUCCESS 成功
*      @return          AUI_RTN_EINVAL  无效输入参数
*      @return          others          失败
*     @note              一般为AUI的开发人员使用
*
*/
AUI_RTN_CODE aui_dev_unreg(aui_module_id dev_type,aui_hdl dev_hdl);
/**
*     @brief         通过设备序号查找AUI设备
*     @author        ray.gong
*     @date              2013-6-8
*     @param[in]        dev_type         设备类型
*     @param[in]        ul_dev_idx          设备序号
*     @param[out]    ul_dev_idx           返回查找到的设备句柄
*      @return          AUI_RTN_SUCCESS 成功
*      @return          AUI_RTN_EINVAL  无效输入参数
*      @return          others          失败
*     @note
*
*/
//AUI_RTN_CODE aui_find_dev_by_idx(aui_module_id dev_type,unsigned long ul_dev_idx,aui_hdl *p_aui_hdl);

AUI_RTN_CODE aui_handle_set_magic_num(aui_module_id dev_type_id,aui_hdl handle);

AUI_RTN_CODE aui_handle_check(aui_hdl handle);

typedef enum aui_nim_state {
    /** Put tuner into standby */
    AUI_NIM_TUNER_STANDBY
} aui_nim_state;
AUI_RTN_CODE aui_nim_set_state(aui_hdl handle, aui_nim_state state);

#ifdef __cplusplus
}
#endif

#endif
