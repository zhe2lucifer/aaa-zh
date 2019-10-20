
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_dog.h>
#include <bus/dog/dog.h>
/****************************LOCAL MACRO******************************************/
extern void dog_m3327e_pause(UINT32 id, int en);
extern void dog_m3327e_set_mode(UINT32 id, UINT32 mode, UINT32 duration_us, void (*callback)(UINT32));
extern UINT32 dog_m3327e_get_time(UINT32 id);
extern void dog_m3327e_set_time(UINT32 id, UINT32 us);
/****************************LOCAL TYPE*******************************************/

AUI_MODULE(DOG)

/****************************LOCAL VAR********************************************/
//static aui_dog_module_attr s_dog_module_attr;
/**module's mutex, the mutext would be locked by devices, It's the 1 level of two levels lock */
static OSAL_ID s_mod_mutex_id_dog=0;
/****************************LOCAL FUNC DECLEAR***********************************/


/****************************MODULE DRV IMPLEMENT*************************************/

/** watch_dog device handler */
typedef struct aui_st_handle_dog
{
    aui_dev_priv_data dev_priv_data;
    /** attribute of watchdog device  */
    aui_attr_dog attr_dog;
}aui_handle_dog,*aui_p_handle_dog;


/****************************MODULE IMPLEMENT*************************************/

AUI_RTN_CODE aui_dog_version_get(unsigned long *pul_version)
{
    if(NULL==pul_version)
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==pul_version");
    }
    *pul_version=AUI_MODULE_VERSION_NUM_DOG;
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_init(p_fun_cb p_call_back_init,void *pv_param)
{
    if((NULL==p_call_back_init))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_call_back_init");
    }
    s_mod_mutex_id_dog=osal_mutex_create();
    if(0==s_mod_mutex_id_dog)
    {
        aui_rtn(AUI_RTN_EIO, "0==s_mod_mutex_id_dog");
    }
    return p_call_back_init(pv_param);
}

AUI_RTN_CODE aui_dog_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
    if((NULL==p_call_back_init))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_call_back_init");
    }
    if(E_OK!=osal_mutex_delete(s_mod_mutex_id_dog))
    {
        aui_rtn(AUI_RTN_EIO, "error");
    }
    return p_call_back_init(pv_param);
}

AUI_RTN_CODE aui_dog_open(aui_attr_dog *p_attr_dog,aui_hdl *pp_hdl_dog)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;

    (*(aui_handle_dog **)pp_hdl_dog)=(aui_handle_dog *)MALLOC(sizeof(aui_handle_dog));
    if(NULL==(aui_handle_dog *)(*(aui_handle_dog **)pp_hdl_dog))
    {
        aui_rtn(AUI_RTN_ENOMEM,"NULL==pp_hdl_dog");
    }
    MEMSET((aui_handle_dog *)(*(aui_handle_dog**)pp_hdl_dog),0,sizeof(aui_handle_dog));

    if(NULL==p_attr_dog)
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_attr_dog");
    }
    else
    {
        MEMCPY(&((*(aui_handle_dog **)pp_hdl_dog)->attr_dog),p_attr_dog,sizeof(aui_attr_dog));
    }

    (*(aui_handle_dog **)pp_hdl_dog)->dev_priv_data.dev_idx=p_attr_dog->uc_dev_idx;
    ret = aui_dev_reg(AUI_MODULE_DOG, *pp_hdl_dog);

    return ret;
}

AUI_RTN_CODE aui_dog_close(aui_hdl p_hdl_dog)
{
    AUI_RTN_CODE ret = AUI_RTN_FAIL;

    if((NULL==p_hdl_dog))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_dog");
    }
    MEMSET(p_hdl_dog,0,sizeof(aui_handle_dog));
    FREE(p_hdl_dog);
    p_hdl_dog=NULL;

    ret = aui_dev_unreg(AUI_MODULE_DOG, (aui_hdl)p_hdl_dog);

    return ret;
}

AUI_RTN_CODE aui_dog_start(void *p_hdl_dog,aui_attr_dog *p_attr_dog)
{
    (void)p_attr_dog;
    if((NULL==p_hdl_dog))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_dog");
    }

    dog_pause((((aui_handle_dog *)p_hdl_dog)->dev_priv_data.dev_idx),0);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_stop(aui_hdl p_hdl_dog,aui_attr_dog *p_attr_dog)
{
    (void)p_attr_dog;
    if((NULL==p_hdl_dog))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_dog");
    }

    dog_pause((((aui_handle_dog *)p_hdl_dog)->dev_priv_data.dev_idx),1);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_set(aui_hdl p_hdl_dog,unsigned long ul_item,void *pv_param)
{
    (void) pv_param;
    AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;

    if(NULL==(aui_handle_dog *)p_hdl_dog)
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==(aui_handle_dog *)p_hdl_dog");
    }

    switch(ul_item)
    {
        default:
            {
                aui_rtn(AUI_RTN_EIO, "error");
                break;
            }
    }
    if(AUI_RTN_SUCCESS!=rtn_code)
    {
        return rtn_code;
    }
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_dog_get(void *p_hdl_dog,unsigned long ul_item,void *pv_param)
{
    (void) pv_param;
    AUI_RTN_CODE rtn_code=AUI_RTN_FAIL;
    //unsigned long ul_rtn=RET_FAILURE;

    if(NULL==(aui_handle_dog *)p_hdl_dog)
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==(aui_handle_dog *)p_hdl_dog");
    }

    switch(ul_item)
    {
        default:
            {
                aui_rtn(AUI_RTN_EIO, "error");
                break;
            }
    }
    if(AUI_RTN_SUCCESS!=rtn_code)
    {
        return rtn_code;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_time_set(aui_hdl p_hdl_dog,unsigned long ul_time_us)
{
    if((NULL==p_hdl_dog))
    {
        aui_rtn(AUI_RTN_EINVAL,"NULL==p_hdl_dog");
    }

    dog_set_time((((aui_handle_dog *)p_hdl_dog)->dev_priv_data.dev_idx),ul_time_us);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_time_get(aui_hdl p_hdl_dog,unsigned long *pul_time_us)
{
    if((NULL==p_hdl_dog)||(NULL==pul_time_us))
    {
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_dog)||(NULL==pul_time_us)");
    }

    *pul_time_us=dog_get_time(((aui_handle_dog *)p_hdl_dog)->dev_priv_data.dev_idx);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_dog_config(aui_hdl p_hdl_dog,aui_attr_dog *p_attr_dog)
{
    if((NULL==p_hdl_dog)||(NULL==p_attr_dog))
    {
        aui_rtn(AUI_RTN_EINVAL,"(NULL==p_hdl_dog)||(NULL==p_attr_dog)");
    }

    dog_mode_set((((aui_handle_dog *)p_hdl_dog)->dev_priv_data.dev_idx),p_attr_dog->ul_work_mode,p_attr_dog->ul_time_us,p_attr_dog->dog_cb);

    return AUI_RTN_SUCCESS;
}

