/**@file
*     @brief         AUI˽�еĹ���ģ��ͷ�ļ�
*     @author        ray.gong
*     @date              2013-5-21
*      @version           1.0.0
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
#include <api/libc/string.h> // place before aui_common.h to clear redefinition warning
#include <aui_common.h>
#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/printf.h>
#include <api/libfs2/stdio.h>

#include "aui_log.h"



#define ERRO_INFO      1
#define WARRMING_INFO 2

#define MAX_PID_NUM 32
#define AUI_AC3_DES_EXIST                    (1<<13)//0x2000
#define AUI_AAC_DES_EXIST                    (2<<13)//0x4000//LATM_AAC
#define AUI_EAC3_DES_EXIST                    (3<<13)//0x0001//EAC3
#define AUI_ADTS_AAC_DES_EXIST                (4<<13)//0x8000//ADTS_AAC

#define AUI_DECV_AVC_MASK                    (1<<13)//0x2000
#define AUI_DECV_AVS_MASK                    (1<<14)//0x4000
#define AUI_DECV_HEVC_MASK                    (1<<15)//0x8000

//extern unsigned int g_debug_level ;
//unsigned long aui_dbg_printf(aui_module_id MdlId,UINT32 uiLevel,const char *format, ...);

/** AUI version number. */
#define AUI_VERSION_NUM                    (0X00010000)

/** common module version number. */
#define AUI_MODULE_VERSION_NUM_COMMON     (0X00010000)

/** AUI�豸���Ƴ������ֵ */
#define AUI_DEV_NAME_LEN_MAX    (32)

/** take semaphore operate*/
#define AUI_TAKE_SEM(x)    do                                               \
                            {                                                \
                                if(x != OSAL_INVALID_ID)         \
                                    osal_semaphore_capture(x,TMO_FEVR); \
                            }while(0)
/** give semaphore operate*/
#define AUI_GIVE_SEM(x)    do                                               \
                            {                                                \
                                if(x != OSAL_INVALID_ID)         \
                                    osal_semaphore_release(x);     \
                            }while(0)

#define AUI_CREATE_DEV_MUTEX(mod_mutex_id,dev_mutex_id,mod_id,err_no) \
                                        osal_mutex_lock(mod_mutex_id,OSAL_WAIT_FOREVER_TIME);\
                                        dev_mutex_id=osal_mutex_create();\
                                        if(INVALID_ID==dev_mutex_id)\
                                        {\
                                            osal_mutex_unlock(mod_mutex_id);\
                                            aui_rtn(err_no,"\r\n_create mutex err.");\
                                        }\
                                        osal_mutex_lock(dev_mutex_id,OSAL_WAIT_FOREVER_TIME);\
                                        osal_mutex_unlock(mod_mutex_id);

#if 0
#define AUI_DELETE_DEV_MUTEX(mod_mutex_id,dev_mutex_id,dev_hdl,mod_id,err_no) \
                                        osal_mutex_lock(mod_mutex_id,OSAL_WAIT_FOREVER_TIME);\
                                        if(NULL==dev_hdl)
                                        {\
                                            aui_rtn(err_no,"\r\n_dev handle is NULL.");\
                                        }\
                                        if(0!=osal_mutex_delete(dev_mutex_id))\
                                        {\
                                            aui_rtn(err_no,"\r\n_delete mutex err.");\
                                        }\
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
    /** ͬ���豸list */
    struct aui_list_head list;
    /** ͬ�����豸list */
    struct aui_list_head sub_list;
    /** �豸magic number */
    unsigned long ul_dev_magic_num;
    /** �豸����ID */
    unsigned long ul_dev_type_id;
    /** �豸������ID */
    unsigned long ul_dev_sub_type_id;
    /** �豸mutex */
    OSAL_ID dev_mutex_id;
    /** �豸id */
    unsigned long dev_idx;
    /** �豸����״̬ */
    enum aui_en_dev_status en_status;
    /** �豸���� */
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
*     @brief         ע��AUI�豸
*     @author        ray.gong
*     @date              2013-6-8
*     @param[in]        dev_type         ע����豸����
*     @param[in]        dev_hdl           ע����豸���
*      @return          AUI_RTN_SUCCESS �ɹ�
*      @return          AUI_RTN_EINVAL  ��Ч�������
*      @return          others          ʧ��
*     @note              һ��ΪAUI�Ŀ�����Աʹ��
*
*/
AUI_RTN_CODE aui_dev_reg(aui_module_id dev_type,aui_hdl dev_hdl);
/**
*     @brief         ע��AUI�豸
*     @author        ray.gong
*     @date              2013-6-8
*     @param[in]        dev_type         ע�����豸����
*     @param[in]        dev_hdl           ע�����豸���
*      @return          AUI_RTN_SUCCESS �ɹ�
*      @return          AUI_RTN_EINVAL  ��Ч�������
*      @return          others          ʧ��
*     @note              һ��ΪAUI�Ŀ�����Աʹ��
*
*/
AUI_RTN_CODE aui_dev_unreg(aui_module_id dev_type,aui_hdl dev_hdl);
/**
*     @brief         ͨ���豸��Ų���AUI�豸
*     @author        ray.gong
*     @date              2013-6-8
*     @param[in]        dev_type         �豸����
*     @param[in]        ul_dev_idx          �豸���
*     @param[out]    ul_dev_idx           ���ز��ҵ����豸���
*      @return          AUI_RTN_SUCCESS �ɹ�
*      @return          AUI_RTN_EINVAL  ��Ч�������
*      @return          others          ʧ��
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
