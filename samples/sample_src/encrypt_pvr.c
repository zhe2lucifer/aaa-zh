#ifdef linux
#define LINUX_MUXTE
#else
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX_MUXTE
#include <pthread.h>
#else
#endif

#include "encrypt_pvr.h"

#define INVALID_FD       (-1)
#define INVALID_DEV_IDX       (-1)
#define INVALID_PID     0x1fff  //refer to hld


#ifdef linux
#define PVR_ENC_ERR(...) do{printf("ERROR: %s, %d, %s\r\n", __FILE__, __LINE__, __func__); printf(__VA_ARGS__);}while(0)
#define PVR_ENC_WARN(...) do{printf("WARN: %s, %d, %s\r\n", __FILE__, __LINE__, __func__); printf(__VA_ARGS__);}while(0)
#else
#define PVR_ENC_ERR(...) do{AUI_PRINTF("ERROR: %s, %d, %s\r\n", __FILE__, __LINE__, __func__); AUI_PRINTF(__VA_ARGS__);}while(0)
#define PVR_ENC_WARN(...) do{AUI_PRINTF("WARN: %s, %d, %s\r\n", __FILE__, __LINE__, __func__); AUI_PRINTF(__VA_ARGS__);}while(0)
#endif


struct pvr_reencrypt_status
{
    
#ifdef LINUX_MUXTE
    pthread_mutex_t mutex;
#else
    //ID mutex;
#endif
    
    int is_working;   //Tag #is_working, when set to 1 means RECORD or PLAYBACK START. Meanwhile the relevant functions which open devices or initialize p_status will no longer be called.
    
    int is_encrypt;   //Tag #is_encrypt, set in function #start_pvr_en/decrypt. Used to mark it's the encryption process or decryption now. The tag is used when config DSC.

    int is_key_valid; //Tag #is_key_valid, set in function #pvr_crypto_key_init. Used to mark the p_status whether has saved the user set init-value or not. The tag is used when RECORD/PLAYBACK start.
    struct pvr_crypto_key key;   //Struct save user's set about key.

    aui_hdl h_dsc;
    int dsc_dev_idx;
    int enable_close_dsc;   //Tag #enable_close_dsc = true means DSC device has been opened.

    aui_hdl h_kl;
    int kl_dev_idx;
    int enable_close_kl;    //Tag #enable_close_kl = true means KL device has been opened.

    aui_hdl h_dmx;
    int dmx_dev_idx;

    int pid_count;
    unsigned short int* p_pids;
};

#define PVR_REENCRYPT_MAX_COUNT 1
static struct pvr_reencrypt_status g_pvr_reencrypt_status[PVR_REENCRYPT_MAX_COUNT];
int g_is_pvr_reencrypt_status_inited = false;


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to initialize a thread lock.
*     
*    @call time        This function only be called in the initialization stage before RECORD/PLAYBACE OPEN.
*                          This function only be used in Linux OS.
*                                                    
*    @param[in]      *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                    which is to specify the work attibute of pvr en/decryption
*    
*    @return           true   =    Initialize the thread lock successfully or not define #LINUX_MUXTE
*    @return           false  =    Initialize failure 
*/

static int lock_init(struct pvr_reencrypt_status* p_status)
{
#ifdef LINUX_MUXTE
    int ret = pthread_mutex_init(&p_status->mutex, NULL);
    if (0 != ret) {
        PVR_ENC_ERR("pthread_mutex_init fail %d.\n", ret);
        return false;
    }
    return true;
#else
    return true;
#endif
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to destory a thread lock. 
*                          Should be used in cooperate with func #lock_init.
*     
*    @call time       This function only be called in the initialization stage before RECORD/PLAYBACE OPEN,
*                          called when initialize a thread lock failure in func #pvr_reencrypt_status_array_init. 
*                         This function only be used in Linux OS.
*                                                    
*    @param[in]     *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                   which is to specify the work attibute of pvr en/decryption
*/

static void lock_deinit(struct pvr_reencrypt_status* p_status)
{
#ifdef LINUX_MUXTE
        int ret = pthread_mutex_destroy(&p_status->mutex);
        if (0 != ret) {
            PVR_ENC_ERR("pthread_mutex_destroy fail %d.\n", ret);
        }
#endif
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to lock the p_status which save the user' set.
*                          Should be used in cooperate with func #unlock.
*                   
*    @call time       This function is called whenever init key\config key\change key, whether record or playback mode.
*                          This function only be used in Linux OS.
*                                                    
*    @param[in]      *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                    which is to specify the work attibute of pvr en/decryption
*    
*    @return            true   =    Set a lock successfully or not define #LINUX_MUXTE
*    @return            false  =    lock failure 
*/

static inline int lock(struct pvr_reencrypt_status* p_status)
{
#ifdef LINUX_MUXTE
    int ret = pthread_mutex_lock(&p_status->mutex);
    if (0 != ret) {
        PVR_ENC_ERR("pthread_mutex_lock fail %d.\n", ret);
        return false;
    }
    return true;
#else
    return true;
#endif
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to unlock a thread lock. 
*                          Should be used in cooperate with func #lock.
*     
*    @attention       This function only be used in Linux OS.
*                                                    
*    @param[in]      *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                    which is to specify the work attibute of pvr en/decryption
*/

static inline void unlock(struct pvr_reencrypt_status* p_status)
{
#ifdef LINUX_MUXTE
    int ret = pthread_mutex_unlock(&p_status->mutex);
    if (0 != ret) {
        PVR_ENC_ERR("pthread_mutex_unlock fail %d.\n", ret);
    }
#endif
}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear the status of sign the en/decrypt.
*    
*    @call time       This function should not be called when user change keys.
*                          It will be called only in 2 place:
*                          the begining init to save user set & clear all status when the en/decryption end.
*                               
*    @param[out]   *p_status    =     Pointer to a struct #pvr_reencrypt_status
*                                                  which is to specify the work attibute of pvr en/decryption
*/

static void pvr_reencrypt_status_init(struct pvr_reencrypt_status* p_status)
{
    memset(p_status, 0, sizeof(*p_status));
    
    p_status->dsc_dev_idx = INVALID_DEV_IDX;
    p_status->kl_dev_idx = INVALID_DEV_IDX;
    p_status->dmx_dev_idx = INVALID_DEV_IDX;    
    
    p_status->key.kl_level = AUI_KL_KEY_LEVEL_NB;
    p_status->key.kl_root_key_idx = AUI_KL_ROOT_KEY_NB;
    p_status->key.parity = AUI_DSC_PARITY_MODE_NB;
    p_status->key.kl_algo = AUI_KL_ALGO_NB;
    p_status->key.dsc_algo = AUI_DSC_ALGO_NB;
    p_status->key.dsc_mode = AUI_DSC_WORK_MODE_NB;
   
    p_status->enable_close_dsc = false;
    p_status->enable_close_kl = false;
    
    p_status->is_working = false;
    p_status->is_encrypt = false;
    p_status->is_key_valid = false;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to init multiple p_status which save the key value & the work attibute of pvr en/decryption.
*     
*    @attention       This sample code will manage multiple recorders,
*                           recorder information are saved in a global array #g_pvr_reencrypt_status.
*                                                    
*    @call time        It will be called only in 1 place:  the begining init to save user set before RECORD/PLAYBACK START.
*/

static int pvr_reencrypt_status_array_init()
{
    int i = 0;

    for (i = 0; i < PVR_REENCRYPT_MAX_COUNT; i++) {
        pvr_reencrypt_status_init(&g_pvr_reencrypt_status[i]);     //This sample code will manage multiple recorders,recorder information are saved in a global array #g_pvr_reencrypt_status.
    }

    for (i = 0; i < PVR_REENCRYPT_MAX_COUNT; i++) {
        if (true != lock_init(&g_pvr_reencrypt_status[i])) {
            goto pvr_reencrypt_status_array_init_fail;
        }
    }

    return true;

pvr_reencrypt_status_array_init_fail:
    for (i = 0; i < PVR_REENCRYPT_MAX_COUNT; i++) {
        lock_deinit(&g_pvr_reencrypt_status[i]);
    }
    
    return false;
}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear the key value & iv of the en/decrypt.
*    
*    @call time       This function will be called in the following situations:
*                          1.Initializes a new p_status.
*                          2.Fail when save the key which user set, or error at the config key to KL/DSC or config DMX device.
*                          3.When RECORD or PLAYBACK is closed.
*                               
*    @param[in/out]   *p_key    =     Pointer to a struct #pvr_crypto_key
*                                                  which is to specify the key value & re-encrypt attibute
*/

static void pvr_crypto_key_clear(struct pvr_crypto_key* p_key)
{
    if (NULL != p_key->p_key) {
        free(p_key->p_key);
        p_key->p_key = NULL;
    }   
    
    if (NULL != p_key->p_iv) {
        free(p_key->p_iv);
        p_key->p_iv = NULL;
    }   
    if (NULL != p_key->p_protected_keys) {
        free(p_key->p_protected_keys);
        p_key->p_protected_keys= NULL;
    }  
    if (NULL != p_key->p_default_iv) {
        free(p_key->p_default_iv);
        p_key->p_default_iv= NULL;
    }  

    memset(p_key, 0, sizeof(*p_key));
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to save key value & re-encrypt attibute which user sets.                  
*                          Save these info into p_status for the KL/DSC config. These info will be used when config KL generate key and config DSC.
*
*    @call time       This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          Each time when a key is changed, only an odd key or even key is passed in.                       
*                                                    
*    @param[in]         *p_key          =     Pointer to a struct #pvr_crypto_key
*                                                        which is the key value & re-encrypt attibute which the user has set
*    
*    @param[in/out]   *p_dst_key    =     Pointer to a struct #pvr_crypto_key
*                                                        which is an empty structure to store the key & re-encrypt attibute which the user sets
*
*    @return            true   =    Save the key value & re-encrypt attibute which user sets successfully
*    @return            false  =    Save failure or the key values which user set are invalid
*/

static int pvr_crypto_key_save(struct pvr_crypto_key* p_dst_key, const struct pvr_crypto_key* p_key)
{
    int mem_size = 0;
    
    memcpy(p_dst_key, p_key, sizeof(*p_dst_key));

    p_dst_key->p_iv = NULL;
    p_dst_key->p_key = NULL;
    p_dst_key->p_protected_keys = NULL;
    p_dst_key->p_default_iv = NULL;
    
    if (p_dst_key->key_length <= 0) {          
        PVR_ENC_ERR("p_dst_key->key_length %d.\n", p_dst_key->key_length);
        return false;
    }
    mem_size = p_dst_key->key_length;         //The unit of key_length: bytes

    if (NULL != p_key->p_iv) {                //Save the key iv which user set.
        p_dst_key->p_iv = malloc(mem_size);
        if (NULL == p_dst_key->p_iv) {
            PVR_ENC_ERR("p_dst_key->p_iv NULL.\n");
            goto pvr_crypto_key_save_fail;
        }
        
        memcpy(p_dst_key->p_iv, p_key->p_iv, mem_size);
    }

    if (NULL != p_key->p_key) {               //Save the key whenever in the initialization phase before the RECORD START or change keys.
        p_dst_key->p_key = malloc(mem_size);
        if (NULL == p_dst_key->p_key) {
            PVR_ENC_ERR("p_dst_key->p_key NULL.\n");
            goto pvr_crypto_key_save_fail;
        }
        
        memcpy(p_dst_key->p_key, p_key->p_key, mem_size);
    }

    if (p_dst_key->kl_level == AUI_KL_KEY_SKIP_LEVEL ||      //Save protect key when key from KL.
        p_dst_key->kl_level == AUI_KL_KEY_LEVEL_NB) {
        p_dst_key->p_protected_keys = NULL;
    } else {
        if (NULL != p_key->p_protected_keys) {
            p_dst_key->p_protected_keys = malloc((p_key->kl_level - 1) * mem_size);
            if (NULL == p_dst_key->p_protected_keys) {
                PVR_ENC_ERR("p_dst_key->p_protected_key NULL.\n");
                goto pvr_crypto_key_save_fail;
            }
                
            memcpy(p_dst_key->p_protected_keys, p_key->p_protected_keys, (p_key->kl_level - 1) * mem_size);
        }
    }

    if (NULL != p_key->p_default_iv) {                //Save default iv which user set, but it is not used in normal case.
        p_dst_key->p_default_iv = malloc(mem_size);
        if (NULL == p_dst_key->p_default_iv) {
            PVR_ENC_ERR("p_dst_key->p_default_iv NULL.\n");
            goto pvr_crypto_key_save_fail;
        }
        
        memcpy(p_dst_key->p_default_iv, p_key->p_default_iv, mem_size);
    }
    return true;

pvr_crypto_key_save_fail:
    pvr_crypto_key_clear(p_dst_key);

    return false;
}

/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear the data & init p_status in the struct #pvr_reencrypt_status.
*    
*    @call time       This function will be called in the following situations:
*                          1.Initializes a new p_status.
*                          2.Fail when save the key which user set, or error at the config key to KL/DSC or config DMX device.
*                          3.When RECORD or PLAYBACK is closed.
*                               
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                       which is to specify the work attibute of pvr en/decryption
*/

static void pvr_reencrypt_status_clear(struct pvr_reencrypt_status* p_status)
{
    if (NULL == p_status) {
        PVR_ENC_ERR("p_status NULL.\n");
        return;
    }

    if (NULL != p_status->p_pids) {
        free(p_status->p_pids);
        p_status->p_pids = NULL;
    }

    pvr_crypto_key_clear(&p_status->key);

    pvr_reencrypt_status_init(p_status);
    
}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Get the p_status from multiple recorders.
*
*    @param[in]      is_new    =      the p_status number which user want to get.
*
*    @return           &g_pvr_reencrypt_status[0]  =  Pointer to the struct #pvr_reencrypt_status
*                                                                      which is to specify the work attibute of pvr en/decryption
*/

static struct pvr_reencrypt_status* pvr_reencrypt_status_get(int is_new)
{
    UNUSED(is_new);
    
    return &g_pvr_reencrypt_status[0];
}

static int dsc_dev_index_get(){
    return 0;
}

static int kl_dev_index_get(){
    return 0;
}

static unsigned long int dev_index_get(aui_hdl h_dev){
    UNUSED(h_dev);
    return -1;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to verify whether key from KL device or not.              
*
*    @param[in]     *p_status  =      Pointer to a struct #pvr_reencrypt_status
*                                                 which is to specify the work attibute of pvr en/decryption
*
*    @return            true        =    key which user set from KL device
*    @return            false       =    key which user set from DRAM
*/

static int is_enable_kl(const struct pvr_reencrypt_status* p_status)
{
    return ((p_status->key.kl_level != AUI_KL_KEY_SKIP_LEVEL)
        &&(p_status->key.kl_level != AUI_KL_KEY_LEVEL_NB)) ? true : false;
}

/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to open the KL device.                  
*                   
*    @call time        This function should only be called once in the init-stage, when the key from KL meanwhile before RECORD/PLAYBACK START.
*                          This function will be called when func #is_enable_kl return true.
*
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                       which is to specify the work attibute of pvr en/decryption
*
*    @return               h_kl   =     Handle of KL device which be opened,
*                                              which will save in p_status if open successfully.
*/

static aui_hdl kl_dev_open(struct pvr_reencrypt_status* p_status)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    struct aui_attr_kl kl_attr;
    memset(&kl_attr, 0, sizeof(kl_attr));
    
    struct pvr_crypto_key* p_key = &p_status->key;
    aui_hdl h_kl = NULL;
    kl_attr.uc_dev_idx = kl_dev_index_get();          //KL device idx is virtual idx. Usually, we use the device num:0 to generate keys.
    kl_attr.en_level = p_key->kl_level;                       
    kl_attr.en_root_key_idx = p_key->kl_root_key_idx;

    if (true == is_enable_kl(p_status)){
        if (p_key->key_length == 8) {                //8 bytes key  choose->  64 bits output key pattern
            kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_64_ODD_EVEN;
        } else if (p_key->key_length == 16) {        //16 bytes key  choose->  128 bits output key pattern
            kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_128_ODD_EVEN;
        } else {
            PVR_ENC_ERR("length %d\n", p_key->key_length);
            kl_attr.en_key_pattern = AUI_KL_OUTPUT_KEY_PATTERN_NB;
        }       
    }
    
    aui_ret = aui_kl_open(&kl_attr, &h_kl);
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("aui_kl_open error %ld\n", aui_ret);
        return NULL;
    }
    p_status->h_kl = h_kl;
    p_status->kl_dev_idx = kl_attr.uc_dev_idx;
    p_status->enable_close_kl = true;

    return h_kl;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to open the DSC device.                  
*                   
*    @call time        This function should only be called once in the init-stage, before RECORD/PLAYBACK START.
*
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                       which is to specify the work attibute of pvr en/decryption
*
*    @return               h_dsc   =     Handle of DSC device which be opened,
*                                                which will save in p_status if open successfully.
*/

static aui_hdl dsc_dev_open(struct pvr_reencrypt_status* p_status)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    struct pvr_crypto_key* p_key = &p_status->key;
    aui_hdl h_dsc = NULL;

    aui_attr_dsc dsc_attr;
    memset(&dsc_attr, 0, sizeof(dsc_attr));
    
    dsc_attr.uc_dev_idx = dsc_dev_index_get();      //Dsc device idx is virtual idx. Usually, we use the device num:0 to en/decryption.
    dsc_attr.uc_algo = p_key->dsc_algo;
    if (p_key->parity != AUI_DSC_PARITY_MODE_NB) {  //For AUI_DSC_DATA_PURE, parity is ignored. So we can set attr:dsc_data_type by parity.
        dsc_attr.dsc_data_type = AUI_DSC_DATA_TS;
    } else {
        dsc_attr.dsc_data_type = AUI_DSC_DATA_PURE;  //For AUI_DSC_DATA_PURE, parity is ignored.
    }
  
    aui_ret = aui_dsc_open(&dsc_attr, &h_dsc);
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("aui_dsc_open failed %ld\n", aui_ret);
        return NULL;
    }
    p_status->h_dsc = h_dsc;
    p_status->dsc_dev_idx = dsc_attr.uc_dev_idx;
    p_status->enable_close_dsc = true;

    return h_dsc;

}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear all the data of the struct #pvr_reencrypt_status,
*                          make sure DSC & KL device has been opened, then close them.    
*    
*    @call time       This function will be called in the following situations:
*                          1.Fail when save the key which user set, or error at the config key to KL/DSC or config DMX device.
*                          2.When RECORD or PLAYBACK is over.
*                               
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status 
*                                                       which is to specify the work attibute of pvr en/decryption
*                                                      (p_status has been clear when param[out])
*/

static void dsc_context_deinit(struct pvr_reencrypt_status* p_status)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;

    if (true == p_status->enable_close_dsc) {
        if (NULL != p_status->h_dsc) {
            aui_ret = aui_dsc_close(p_status->h_dsc);
            if (AUI_RTN_SUCCESS != aui_ret) {
                PVR_ENC_ERR("aui_dsc_close failed %ld\n", aui_ret);
            }
        }
    }
    p_status->h_dsc = NULL;
    p_status->dsc_dev_idx = INVALID_DEV_IDX;
    p_status->enable_close_dsc = false;


    if (true == p_status->enable_close_kl) {
        if (NULL != p_status->h_kl) {
            aui_ret = aui_kl_close(p_status->h_kl);
            if (AUI_RTN_SUCCESS != aui_ret) {
                PVR_ENC_ERR("aui_kl_close failed %ld\n", aui_ret);
            }
        }   
    }
    p_status->h_kl = NULL;
    p_status->kl_dev_idx = INVALID_DEV_IDX;
    p_status->enable_close_kl = false;

    pvr_reencrypt_status_clear(p_status);
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to open devices about en/decryption(DSC & KL(if user set)).                  
*                          Handler/Index/Enable-tag of KL(if user set) & DSC device in p_status will be update if open successfully. 
*
*    @attention       1.This sample code should establish in DMX open. We only need get DMX message then match key when RECORD/PLAYBACK start.
*                           So this FUNC is not mention about DMX open, also we don't need enable tag: enable_close_dmx. 
*                           Because we don't need open or close DMX in this sample code. 
*                          
*                           2.This function only be called by function : #pvr_crypto_key_init,
*                           it means, it only be called once in the init-stage, before RECORD/PLAYBACK OPEN.        
*
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                       which is to specify the work attibute of pvr en/decryption
*
*    @param[in/out]  *p_key_hd    =      Pointer to a struct #pvr_set_crypto_key_hd
*                                                       which is to specify the device handler of DMX/KL/DSC 
*
*    @return         AUI_RTN_SUCCESS      =  En/Decryption devices be opened successfully
*    @return         AUI_RTN_FAIL            =  En/Decryption devices be opened failed for some reasons   
*/

static AUI_RTN_CODE dsc_context_init(struct pvr_reencrypt_status* p_status, struct pvr_set_crypto_key_hd* p_key_hd)
{
    if (NULL == p_key_hd) {
        return AUI_RTN_SUCCESS;
    }

    if (true == is_enable_kl(p_status)) {  
        if (NULL == p_key_hd->h_kl) {   //Usually, h_dkl is NULL when initializing, and we would open KL in this judgment code.
            p_key_hd->h_kl = kl_dev_open(p_status);
            if (NULL == p_key_hd->h_kl) {
                PVR_ENC_ERR("p_key->h_kl NULL.\n");
                return AUI_RTN_FAIL;
            }
        } else {        //Code could not enter this part in normal case, user could ignore it.
            p_status->h_kl = p_key_hd->h_kl;
            p_status->kl_dev_idx = dev_index_get(p_status->h_kl);  //KL device idx is virtual idx. Usually, we use the device num:0 to generate keys.
            p_status->enable_close_kl= false;   
        }
        
    } else {
        p_key_hd->h_kl = NULL;
    }

    if (NULL == p_key_hd->h_dsc) {    //Usually, h_dsc is NULL when initializing, and we would open DSC in this judgment code.
        p_key_hd->h_dsc = dsc_dev_open(p_status);
        if (NULL == p_key_hd->h_dsc) {
            PVR_ENC_ERR("p_key_hd->h_dsc NULL.\n");
            return AUI_RTN_FAIL;
        }  
    } else {          //Code could not enter this part in normal case, user could ignore it.
        p_status->h_dsc = p_key_hd->h_dsc;
        p_status->dsc_dev_idx = dev_index_get(p_status->h_dsc);   //Dsc device idx is virtual idx. Usually, we use the device num:0 to en/decryption.
        p_status->enable_close_dsc= false;
    }

    if (NULL != p_key_hd->h_dmx) {    //Code could not enter this part in normal case, user could ignore it.
        p_status->h_dmx = p_key_hd->h_dmx;
        p_status->dmx_dev_idx = dev_index_get(p_status->h_dmx);
    }
    return AUI_RTN_SUCCESS;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to configure KL device then generate keys.
*     
*    @call time       1.This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          Each time when a key is changed, the parity of the generation key changes.
*    
*                          2.calling procedure: kl_cfg -> aui_kl_gen_key_by_cfg -> generate_all_key
*
*    @param[in]        *p_status              =   Pointer to a struct #pvr_reencrypt_status
*                                                             which is to specify the work attibute of pvr en/decryption
*
*    @param[in/out]  *p_key_dst_pos     =   Pointer to the output key position that store the content key
*
*    @return         true      =   Generate keys successfully
*    @return         false     =   execute #aui_kl_gen_key_by_cfg failed for some reasons   
*/

static int kl_cfg(struct pvr_reencrypt_status* p_status, unsigned long* p_key_dst_pos)
{
    struct pvr_crypto_key* p_key = &p_status->key;
    aui_hdl h_kl = NULL;
    int key_size = 0;
    int prok_size = 0;
    
    prok_size = (p_key->kl_level -1) * p_key->key_length;    //protect key size.
    struct aui_cfg_kl cfg;
    memset(&cfg, 0, sizeof(cfg));
    
    h_kl = p_status->h_kl;
    if (NULL == h_kl) {
        PVR_ENC_ERR("h_kl NULL\n");
        return false;
    }
   
    cfg.run_level_mode = AUI_KL_RUN_LEVEL_MODE_LEVEL_ALL;
    cfg.en_crypt_mode = AUI_KL_DECRYPT;            //Decrypt protect key, generate a content key for DSC. Then DSC can use the content key to encrypt or decrypt data.
    cfg.en_kl_algo = p_key->kl_algo;
    key_size = p_key->key_length;
    
    if (p_key->parity == AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE ||            //Means the key is even key when RECORD MODE.
        p_key->parity == (AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 2)) {    //Means the key is even key but in PLAYBACK MODE.
        AUI_PRINTF("%s -> cfg even key\n", __func__);
        AUI_PRINTF("%s -> p_key->parity =  %d\n", __func__,p_key->parity);
        cfg.en_cw_key_attr = AUI_KL_CW_KEY_EVEN;
    } else if (p_key->parity == AUI_DSC_PARITY_MODE_ODD_PARITY_MODE ||      //Means the key is odd key when RECORD MODE.
        p_key->parity == (AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 1)) {    //Means the key is odd key but in PLAYBACK MODE.
        AUI_PRINTF("%s -> cfg odd key\n", __func__);
        AUI_PRINTF("%s -> p_key->parity =  %d\n", __func__,p_key->parity);
        cfg.en_cw_key_attr = AUI_KL_CW_KEY_ODD;
    } else {
        PVR_ENC_WARN("parity %d\r\n", p_key->parity);
        cfg.en_cw_key_attr = AUI_KL_CW_KEY_ATTR_NB;
    }
    
    MEMCPY(cfg.ac_key_val, p_key->p_protected_keys, prok_size);  //Pass by all the protect key.
    MEMCPY(&cfg.ac_key_val[prok_size], p_key->p_key, key_size);

    if (aui_kl_gen_key_by_cfg(h_kl, &cfg, p_key_dst_pos)) {        //Generate protect keys and content key.
        PVR_ENC_ERR("aui_kl_gen_key_by_cfg error\n");
        return false;
    }

    return true;

}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to set DSC common attribute according to user set.
*     
*    @call time        This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*
*    @param[in]        *p_status              =   Pointer to a struct #pvr_reencrypt_status
*                                                             which is to specify the work attibute of pvr en/decryption
*
*    @param[in/out]  *p_dsc_attr           =   Pointer to a struct #aui_st_attr_dsc
*                                                             which is to specify the attributes of DSC device
*
*    @return         true      =   Set DSC common attribute successfully
*    @return         false     =   DSC parity mode which user set are invalid  
*/

static int dsc_common_attr_set(struct pvr_reencrypt_status* p_status, aui_attr_dsc* p_dsc_attr)
{
    struct pvr_crypto_key* p_key = &p_status->key;
    p_dsc_attr->uc_dev_idx = p_status->dsc_dev_idx;
    p_dsc_attr->uc_algo = p_key->dsc_algo;
    p_dsc_attr->uc_mode = p_key->dsc_mode;
    p_dsc_attr->ul_key_len = p_key->key_length * 8;    //Unit of key_length change from byte to bit.

    if (p_key->parity == AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE ||       //When RECORD MODE, p_key->parity represents the arrtibute:dsc parity mode.
        p_key->parity == AUI_DSC_PARITY_MODE_ODD_PARITY_MODE) {        
        p_dsc_attr->en_parity = p_key->parity;
    } else {
        p_dsc_attr->en_parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0; //The attibute is stationary when PLAYBACK MODE(TS decryption).
    }
    
    p_dsc_attr->en_residue = AUI_DSC_RESIDUE_BLOCK_IS_NO_HANDLE;
    p_dsc_attr->en_en_de_crypt = 0 != p_status->is_encrypt ? AUI_DSC_ENCRYPT : AUI_DSC_DECRYPT;
    p_dsc_attr->pus_pids = p_status->p_pids;
    p_dsc_attr->ul_pid_cnt = p_status->pid_count;

    switch (p_dsc_attr->en_parity)                       
    {
        case AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE:         
            p_dsc_attr->dsc_data_type = AUI_DSC_DATA_TS;
            if (NULL != p_key->p_iv) {
                p_dsc_attr->puc_iv_ctr = p_key->p_iv;
            } else {
                p_dsc_attr->puc_iv_ctr = p_key->p_default_iv + p_key->key_length;
            }
            break;

        case AUI_DSC_PARITY_MODE_ODD_PARITY_MODE:
        case AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0:  
            p_dsc_attr->dsc_data_type = AUI_DSC_DATA_TS;
            if (NULL != p_key->p_iv) {
                p_dsc_attr->puc_iv_ctr = p_key->p_iv;
            } else {
                p_dsc_attr->puc_iv_ctr = p_key->p_default_iv;
            }
            break;

        default:
            PVR_ENC_ERR("p_key->parity %d\r\n", p_key->parity);
            return false;
    }

    return true;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to set DSC attribute when the key is stored in S-RAM.
*     
*    @call time        This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          Called by function #dsc_key_attach.
*
*    @param[in]        *p_status              =   Pointer to a struct #pvr_reencrypt_status
*                                                             which is to specify the work attibute of pvr en/decryption
*
*    @param[in/out]  *p_dsc_attr           =   Pointer to a struct #aui_st_attr_dsc
*                                                             which is to specify the attributes of DSC device
*
*    @return         true      =   Set DSC common attribute successfully
*    @return         false     =   DSC parity mode which user set are invalid  
*/

static int dsc_clear_attr_set(struct pvr_reencrypt_status* p_status, aui_attr_dsc* p_dsc_attr)
{
    struct pvr_crypto_key* p_key = &p_status->key;

    if (true != dsc_common_attr_set(p_status, p_dsc_attr)) {
        PVR_ENC_ERR("dsc_common_attr_set fail\r\n");
        return false;
    }
 
    if (p_key->parity == AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE ||          //Means the key is even key when RECORD MODE.
        p_key->parity == (AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 2)) {  //Means the key is even key but in PLAYBACK MODE. 
        p_dsc_attr->ul_key_pattern = AUI_DSC_KEY_PATTERN_EVEN;
    } else {
        p_dsc_attr->ul_key_pattern = AUI_DSC_KEY_PATTERN_ODD;             //Otherwise the key which store in S-RAM is odd key. 
    } 

    p_dsc_attr->dsc_key_type = AUI_DSC_HOST_KEY_SRAM;
    p_dsc_attr->puc_key = p_key->p_key;

    return true;

}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to set DSC attribute when the key is stored in KL.
*     
*    @call time        This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          Called by function #dsc_key_attach.
*
*    @param[in]        *p_status              =   Pointer to a struct #pvr_reencrypt_status
*                                                             which is to specify the work attibute of pvr en/decryption
*
*    @param[in/out]  *p_dsc_attr           =   Pointer to a struct #aui_st_attr_dsc
*                                                             which is to specify the attributes of DSC device
*
*    @param[in]        *p_key_dst_pos     =   Pointer to the output key position that store the content key
*
*    @return         true      =   Set DSC common attribute successfully
*    @return         false     =   DSC parity mode which user set are invalid  
*/

static int dsc_kl_attr_set(struct pvr_reencrypt_status* p_status, aui_attr_dsc* p_dsc_attr, unsigned long key_dst_pos)
{

    if (true != dsc_common_attr_set(p_status, p_dsc_attr)) {
        PVR_ENC_ERR("dsc_common_attr_set fail\r\n");
        return false;
    }
    
    p_dsc_attr->dsc_key_type = AUI_DSC_CONTENT_KEY_KL;
    p_dsc_attr->ul_key_pos = key_dst_pos;
    return true;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to set DSC attribute & config key info to DSC device opened.
*     
*    @call time        1.This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          a).first config DSC before REC/PLAYBACK START:     is called by function -> #dsc_cfg
*                          b).config DSC when change keys:                           is called by function -> #pvr_crypto_key_change
*
*                          2.When the key source is different, the branch is generated when the DSC device is configured:
*                          a).Key from KL:        call function -> #dsc_kl_attr_set
*                          b).Key from S-RAM:  call function -> #dsc_clear_attr_set
*
*    @param[in]        *p_status              =   Pointer to a struct #pvr_reencrypt_status
*                                                             which is to specify the work attibute of pvr en/decryption
*
*    @param[in]        *p_key_dst_pos     =   Pointer to the output key position that store the content key
*                                                             (only be used when the key from KL device.)
*
*    @return         AUI_RTN_SUCCESS      =  Config the key attribute to DSC and attach key successfully
*    @return         AUI_RTN_FAIL            =  Set DSC attribute or config key info to DSC device failed
*/

static AUI_RTN_CODE dsc_key_attach(struct pvr_reencrypt_status* p_status, unsigned long key_dst_pos)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    int is_complete = false;
    aui_hdl h_dsc = p_status->h_dsc;

    if (NULL == h_dsc) {
        PVR_ENC_ERR("h_dsc NULL\n");
        return AUI_RTN_FAIL;
    }
    
    aui_attr_dsc dsc_attr;
    memset(&dsc_attr, 0, sizeof(dsc_attr));
            
    if (true == is_enable_kl(p_status)) {        //Set DSC attibute when key from KL.
        is_complete = dsc_kl_attr_set(p_status, &dsc_attr, key_dst_pos);
    } else {                                     //Set DSC attibute when key from S-RAM.
        is_complete = dsc_clear_attr_set(p_status, &dsc_attr);
    }
    if (true != is_complete) {
        PVR_ENC_ERR("is_complete %d\n", is_complete);
        return AUI_RTN_FAIL;
    }

    aui_ret = aui_dsc_attach_key_info2dsc(h_dsc, &dsc_attr);     //Attach key to DSC device which set above.
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("aui_gen_ca_pvr_rec_config_C1800A aui_dsc_attach_key_info2dsc failed %ld\n", aui_ret);
        return aui_ret;
    }   

    return AUI_RTN_SUCCESS;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is a general flow function that configures a user-set key to the DSC device.
*     
*    @call time        1.This function can only be called once, only when the prepare stage before RECORD/PLAYBACK START.
*
*                          2.Function flow:
*                             Verify whether key from KL or not, if so, config KL device then generate keys; 
*                             Set DSC device attribute, then attach key which from KL or S-RAM to DSC.  
*
*    @param[in]        *p_status   =     Pointer to a struct #pvr_reencrypt_status
*                                                    which is to specify the work attibute of pvr en/decryption
*
*    @return               h_dsc       =     Handle of DSC device which be set completely,
*                                                    this device handle will be used by DMX directly and will not be updated in p_status.
*/

static aui_hdl dsc_cfg(struct pvr_reencrypt_status* p_status)
{
    aui_hdl h_dsc = NULL;
    aui_hdl h_kl = NULL;

    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    unsigned long key_dst_pos = 0;

    aui_attr_dsc dsc_attr;
    memset(&dsc_attr, 0, sizeof(dsc_attr));

    struct aui_attr_kl kl_attr;
    memset(&kl_attr, 0, sizeof(kl_attr));

    if (true == is_enable_kl(p_status)) {   //Check if key from KL. Get the h_kl if key from KL.
        if (NULL == p_status->h_kl) {       //Usually the KL device has been opend before, open in function #dsc_context_init. We don't open KL here.
            h_kl = kl_dev_open(p_status);
            if (NULL ==  h_kl) {           
                PVR_ENC_ERR("h_kl NULL\n");
                return NULL;
            }
        } else {                            //Get the KL device handler which opened, prepare to cfg it next.
            h_kl = p_status->h_kl;
        }

        if (true != kl_cfg(p_status, &key_dst_pos)) {  //Config KL here before RECORD/PLAYBACK START.

            PVR_ENC_ERR("kl_cfg failed\n");
            return NULL;
        } 
    }

    if (NULL == p_status->h_dsc) {           //Usually the DSC device has been opend before, open in function #dsc_context_init. We don't open DSC here.
        h_dsc = dsc_dev_open(p_status);
        if (NULL == h_dsc) {
            PVR_ENC_ERR("dsc_dev_open failed\n");
            return NULL;
        }
    } else {
        h_dsc = p_status->h_dsc;
    }

    aui_dsc_process_attr dsc_process_attr;   //Struct #aui_dsc_process_attr is only for projects based on Linux OS.
    memset(&dsc_process_attr, 0, sizeof(dsc_process_attr));
    dsc_process_attr.process_mode = 0 != p_status->is_encrypt ? AUI_DSC_PROCESS_MODE_TS_ENCRYPT : AUI_DSC_PROCESS_MODE_TS_DECRYPT;
    dsc_process_attr.ul_block_size = p_status->key.dsc_block_size;
    aui_ret = aui_dsc_process_attr_set(h_dsc, &dsc_process_attr);
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("aui_dsc_process_attr_set failed %ld\n", aui_ret);
        return NULL;
    }
    aui_ret = dsc_key_attach(p_status, key_dst_pos);      //Here to set DSC attribute then attach key.
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("dsc_cfg dsc_key_attach failed %ld\n", aui_ret);
        return NULL;
    }
    return h_dsc;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to save PIDs which user want to en/decrypt.                  
*                          
*    @call time       This function is called in the prepare stage before RECORD/PLAYBACK START.
*                          These pids will be used in the configuration DSC before RECORD/PLAYBACK START. 
*
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                       which is to specify the work attibute of pvr en/decryption
*                                                       (p_status->p_pids & p_status->pid_count will be update when param[out])
*
*    @param[in]         *p_pids       =      Pointer to the address of the pids which need to be en/decrypted
*
*    @param[in]          count         =      The number of PIDs which need to be en/decrypted
*
*    @return            true   =    Save the PIDs which need to be en/decrypted successfully
*    @return            false  =    Save failure or the parameters which user set are invalid
*/

static int pvr_crypto_pids_save(struct pvr_reencrypt_status* p_status, unsigned short int* p_pids, int count)
{
    int size = 0;
    if (count <= 0) {
        PVR_ENC_ERR("count %d\n", count);
        return false;
    }

    if (NULL == p_pids) {
        PVR_ENC_ERR("p_pids NULL\n");
        return false;
    }

    if (NULL != p_status->p_pids) {
        PVR_ENC_ERR("p_status->p_pids %p\n", p_status->p_pids);
        return false;
    }

    size = sizeof(*(p_status->p_pids)) * count;
    p_status->p_pids = malloc(size);
    if (NULL == p_status->p_pids) {
        PVR_ENC_ERR("p_status->p_pids %p\n", p_status->p_pids);
        return false;
    }

    memcpy(p_status->p_pids, p_pids, size);  //These pids will be used in the configuration DSC before RECORD/PLAYBACK START. 
    p_status->pid_count = count;
 
    return true;

}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to update the struct #p_status which just be initialized, and save the
*                         key-value & re-encrypt attribute which set by the user in. Finally call the function #dsc_context_init 
*                         to open the en/decryption device.
*     
*    @call time       This function can only be called once when the first time (init) before #start-pvr-en/decrypt , 
*                          it means, in the init-stage before RECORD/PLAYBACK START.
*    
*    @param[in/out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                       which is to specify the work attibute of pvr en/decryption
*                                                       (attention: tag p_status->is_key_valid is update when param[out])
*
*    @param[in]         *p_key        =     Pointer to a struct #pvr_crypto_key
*                                                      which is to specify the key value & re-encrypt attibute which user set
*
*    @param[in/out]  *p_key_hd    =      Pointer to a struct #pvr_set_crypto_key_hd
*                                                       which is to specify the device handler of DMX/KL/DSC
*                                                       (all the handlers are NULL when param[in]) 
*
*    @param[in]         *p_pids       =      Pointer to the address of the pids which need to be en/decrypted
*    
*    @return         AUI_RTN_SUCCESS      = Update the struct #p_status and open the en/decryption device successfully
*    @return         AUI_RTN_FAIL            =  Open the en/decryption device failure or the parameters which user set are invalid  
*/

static AUI_RTN_CODE pvr_crypto_key_init(struct pvr_reencrypt_status* p_status, const struct pvr_crypto_key* p_key,
    struct pvr_set_crypto_key_hd* p_key_hd, struct pvr_crypto_pids* p_pids)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
    pvr_reencrypt_status_clear(p_status);          //Clear then make a temp p_status to save user's set.

    if (true != pvr_crypto_key_save(&p_status->key, p_key)) {   //Save user key value & re-encrypt attribute.
        pvr_reencrypt_status_clear(p_status);

        PVR_ENC_ERR("pvr_crypto_key_save fail.\n");
        goto pvr_crypto_key_init_fail;
    }
    p_status->is_key_valid = true;            

    if (NULL != p_pids) {       //Usually, p_pid is NULL when this function be called, we save pids when function #start_pvr_en/decrypt is called, not here.           
        if (true != pvr_crypto_pids_save(p_status, p_pids->p_pids, p_pids->count)) {  
            PVR_ENC_ERR("pvr_crypto_pids_save fail.\n");
            goto pvr_crypto_key_init_fail;
        }
    }
    
    ret = dsc_context_init(p_status, p_key_hd);  //Open devices about en/decryption(DSC & KL(if user set key from KL)).
    if (AUI_RTN_SUCCESS != ret) {
        PVR_ENC_ERR("dsc_context_init fail %ld.\n", ret);
        goto pvr_crypto_key_init_fail;
    }

    return AUI_RTN_SUCCESS;
    
pvr_crypto_key_init_fail:
    dsc_context_deinit(p_status);

    return AUI_RTN_FAIL;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to verify whether key-parity enable to be change or not.              
*
*    @param[in]         *p_dst_key    =     Pointer to a struct #pvr_crypto_key
*                                                        which is the key value & re-encrypt attibute which use now
*    
*    @param[in/out]   *p_key          =     Pointer to a struct #pvr_crypto_key
*                                                        which is the key value & re-encrypt attibute which next to be changed
*
*    @return            true        =    Could change odd->even key, or even->odd key 
*    @return            false       =    Can't change key when KL-level / en/decryption algo or mode is not equal
*/

static int pvr_crypto_key_change_verify(const struct pvr_crypto_key* p_dst_key, const struct pvr_crypto_key* p_key)
{
    if (p_key->kl_level != p_dst_key->kl_level){
        PVR_ENC_ERR("p_key->kl_level %d != %d.\n", p_key->kl_level, p_dst_key->kl_level);
        return false;
    }

    if ((p_key->parity == AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE 
        && p_dst_key->parity != AUI_DSC_PARITY_MODE_ODD_PARITY_MODE)
        ||(p_key->parity == AUI_DSC_PARITY_MODE_ODD_PARITY_MODE
        && p_dst_key->parity != AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE)){    //Check whether parity is different between the using key and the key which will be changed.

        PVR_ENC_ERR("p_key->parity %d != %d.\n", p_key->parity, p_dst_key->parity);
        return false;
    }


    if (p_key->dsc_algo != p_dst_key->dsc_algo){
        PVR_ENC_ERR("p_key->dsc_algo %d != %d.\n", p_key->dsc_algo, p_dst_key->dsc_algo);
        return false;
    }

    if (p_key->dsc_mode != p_dst_key->dsc_mode){
        PVR_ENC_ERR("p_key->dsc_mode %d != %d.\n", p_key->dsc_mode, p_dst_key->dsc_mode);
        return false;
    }

    return true;
}




/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is a general flow function that configures a new user-set key to the DSC device.
*     
*    @call time       1.This function will be called after RECORD/PLAYBACK START when user change key.
*
*                          2.Function flow:
*                             a).Update key value & re-encrypt attibute to new key's in p_status;
*                             b).Verify whether new key from KL or not, if so, config KL device then generate keys; 
*                             c).Set DSC device attribute, then attach new key which from KL or S-RAM to DSC.  
*
*    @param[in/out]        *p_status   =     Pointer to a struct #pvr_reencrypt_status
*                                                          which is to specify the work attibute of pvr en/decryption
*
*    @param[in]              *p_key       =     Pointer to a struct #pvr_crypto_key
*                                                          which is the new key value & re-encrypt attibute which the user update
*
*    @return         AUI_RTN_SUCCESS      = Config DSC device with new key successfully
*    @return         AUI_RTN_FAIL            =  The key which user set is invalid or config DSC device failed
*/

static AUI_RTN_CODE pvr_crypto_key_change(struct pvr_reencrypt_status* p_status, const struct pvr_crypto_key* p_key)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    unsigned long key_dst_pos = 0;
    
    if (true != pvr_crypto_key_change_verify(&p_status->key, p_key)) {    //Verify whether the key enable to be change or not.
        PVR_ENC_ERR("enable_change_pvr_key false\n");
        return AUI_RTN_FAIL;
    }

    pvr_crypto_key_clear(&p_status->key);                        //Clear the old key value & iv.
    if (true != pvr_crypto_key_save(&p_status->key, p_key)) {    //Save the new key in p_status.
        PVR_ENC_ERR("pvr_crypto_key_save false\n");
        return AUI_RTN_FAIL;
    }
   
    if (true == is_enable_kl(p_status)) {               //If key from KL, then config KL to generate new content key.
        if (true != kl_cfg(p_status, &key_dst_pos)) {
            PVR_ENC_ERR("kl_cfg fail.\n");
            return AUI_RTN_FAIL;
        }
    }
    ret = dsc_key_attach(p_status, key_dst_pos);        //Config DSC to attach key.
    if (AUI_RTN_SUCCESS != ret) {
         PVR_ENC_ERR("change pvr_crypto_key dsc_key_attach failed %ld\n", ret);
         return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
}



/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             The function is mainly save the key value & re-encrypt attibute which user set in p_status in the initialization before REC/PLAY START. 
*                          Meanwhile, the function is the config-enter-place of the crypto-flow when change-key.
*                           When called by #.._key_change ,the function mainly execute function #pvr_crypto_key_change to config dsc attach new key.
*
*    @call time       This function is called both in the initialization and in each time when change key, whether in RECORD or PLAYBACK mode.
*                          It is called just after the user-set stage.
*    
*    @param[in]        *p_key         =      Pointer to a struct #pvr_crypto_key
*                                                       which is the key value & re-encrypt attibute which the user set
*
*    @param[in/out]  *p_key_hd    =      Pointer to a struct #pvr_set_crypto_key_hd
*                                                       which is to specify the device handler of DMX/KL/DSC
*                                                       (param[out] because KL&DSC open when init)
*
*    @param[in]         *p_pids       =      Pointer to the address of the pids which need to be en/decrypted
*    
*    @return         AUI_RTN_SUCCESS      = Create p_status which saved user' set or change key successfully
*    @return         AUI_RTN_FAIL            = The input parameters (i.e. [in]) is invalid or function which called return failed
*/

AUI_RTN_CODE pvr_crypto_key_set(const struct pvr_crypto_key* p_key, 
    struct pvr_set_crypto_key_hd* p_key_hd, struct pvr_crypto_pids* p_pids)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    struct pvr_reencrypt_status* p_status = NULL;
    if (true != g_is_pvr_reencrypt_status_inited) { //Only enter this judgment code when init(before called function #start_pvr_en/decrypt).
        if (true != pvr_reencrypt_status_array_init()) {
            return false;
        }
        g_is_pvr_reencrypt_status_inited = true;
    }

    p_status = pvr_reencrypt_status_get(0);  //The value is empty when the function is called for the first time; The value is current status when the function is called after RECORD or PLAYBACK START.
    if (NULL == p_status) {
    
        PVR_ENC_ERR("pvr_reencrypt_status_get fail.\n");
        return AUI_RTN_FAIL;
    }
    if (true != lock(p_status)) {  //Lock the p_status to ensure that the process is not interrupted when you open devices or config devices when change key.
        PVR_ENC_ERR("lock fail.\n");
        return AUI_RTN_FAIL;
    }
    
    if (true == p_status->is_working) {  //CHANGE KEY, only execute after RECORD/PLAYBACK START.
        ret = pvr_crypto_key_change(p_status, p_key);
    } else {  //INIT KEY, only enter this part once before RECORD/PLAYBACK START.
        ret = pvr_crypto_key_init(p_status, p_key, p_key_hd, p_pids);
    }

    unlock(p_status);
    return ret;

}



/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear all the data of the struct #pvr_reencrypt_status,
*                          then close DSC & KL device which has been opened.    
*                          (Finish this function mainly by called FUNC #dsc_context_deinit. Similar with Func stop_pvr_en/decrypt.)
*
*    @call time       This function will be called when RECORD/PLAYBACK CLOSE.
*                          Usually, KL/DSC devices have been closed before here. So this part is just a matter of prevention.
*/

void pvr_crypto_key_free()
{
    struct pvr_reencrypt_status* p_status = NULL;
    if (true != g_is_pvr_reencrypt_status_inited) {
        return;
    }
        
    p_status = pvr_reencrypt_status_get(0);
    if (NULL == p_status) {
        return;
    }
    if (true != lock(p_status)) {
        PVR_ENC_ERR("lock fail.\n");
        return;
    }

    dsc_context_deinit(p_status);

    unlock(p_status);
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to execute the first time configuration of the DSC module
*                          using the p_status set by the user, and then tells DMX the DSC information 
*                          which will be used next.
*     
*    @call time       This function can only be called once, only when the prepare stage before RECORD START.
*                          It will be called automatically by driver when the driver needs to config the connection of DMX with the DSC device.
*    
*    @param[in]     *p_param    =      Pointer to a struct #aui_pvr_crypto_general_param 
*                                                   which save DMX id info and PID info
*                                                    
*    @param[out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                   which is to specify the work attibute of pvr en/decryption
*    
*    @return         AUI_RTN_SUCCESS      = Config DSC device and connect to DMX successfully
*    @return         AUI_RTN_FAIL            = The input parameters (i.e. [in]) is invalid or Config DSC device/Set path to DMX failed
*/

AUI_RTN_CODE pvr_encrypt_start(aui_pvr_crypto_general_param *p_param)
{
    AUI_RTN_CODE aui_ret = AUI_RTN_SUCCESS;
    struct pvr_reencrypt_status* p_status = NULL;

    if (true != g_is_pvr_reencrypt_status_inited) {  //The parameter set to true only when the first time program enters #pvr_crypto_key_set to init p_status. Here is to make sure that p_status save the user's set completely. 
        PVR_ENC_ERR("g_is_pvr_reencrypt_status_inited %d.\n", g_is_pvr_reencrypt_status_inited);
        return AUI_RTN_FAIL;
    }

    p_status = pvr_reencrypt_status_get(0);          //Get the p_status which has been init & has save user's set.
    if (NULL == p_status) {
        PVR_ENC_ERR("pvr_reencrypt_status_get fail.\n");
        return AUI_RTN_FAIL;
    }

    if (true != lock(p_status)) {                  //Protect the encrypted key value & it's attribute not be change when dsc_cfg and set dsc id to dmx.
        PVR_ENC_ERR("lock fail.\n");
        return AUI_RTN_FAIL;
    }
    
    if (true != p_status->is_key_valid) {          //If not save user's set in p_status return err.
        PVR_ENC_ERR("p_status->is_key_valid %d\n", p_status->is_key_valid);
        goto pvr_encrypt_start_fail;
    }
    
    if (true == p_status->is_working) {            //This function #pvr_encrypt_start can only be called once, which is to configure the opened DSC information to DMX after initializing the user's Settings. If you repeat to call this func, return error.
        PVR_ENC_ERR("p_status->is_working %d\n", p_status->is_working);
        goto pvr_encrypt_start_fail;
    }
    p_status->is_encrypt = true;                   //Set tag #is_encrypting true, and it will be used in the function #cfd_dsc and #dsc_common_attr_set to tell dsc it is record or not now.

    if (0 == p_status->pid_count) {                //No pid should be passed in when initializing. It is the correct step to enter the judgment and save the pids which need to be encrypted in p_status here.
        if (true != pvr_crypto_pids_save(p_status, p_param->pid_list, p_param->pid_num)) {
            PVR_ENC_ERR("pvr_crypto_pids_save failed\n");
            goto pvr_encrypt_start_fail1;
        }
    }
    
    aui_hdl h_dsc = dsc_cfg(p_status);           //Configure the DSC with the p_status which set by the user. Here is the first time to pass key to DSC.
    if (NULL == h_dsc) {
        PVR_ENC_ERR("dsc_cfg failed\n");
        goto pvr_encrypt_start_fail1;
    }

    /*
     After config the KL, DSC device, we should tell DMX that the DSC' ID number which has been configured completely and should be used next.
     But before that, we need to make sure that DMX has been opened and get its device handle.
     */
 
    if (NULL == p_status->h_dmx) {               //p_status->h_dmx should not be passed in when initializing. We set p_status->h_dmx in this judgment code.
        if (AUI_RTN_SUCCESS != aui_find_dev_by_idx(AUI_MODULE_DMX, p_param->dmx_id, &p_status->h_dmx)) {  //Usually, dmx has been opened before record open. Call this function #aui_find_dev_by_idx to find out if DMX has been opened. And if opened, h_dmx is returned and assigned to p_status.
            aui_attr_dmx attr_dmx;
            memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
            attr_dmx.uc_dev_idx = p_param->dmx_id;
            if(aui_dmx_open(&attr_dmx, &p_status->h_dmx)) {    //Usually, dmx has been opened before here. So this part should not be used in normal scene.
                PVR_ENC_ERR("aui_find_dev_by_idx AUI_MODULE_DMX failed(dmx_id %d)\n", p_param->dmx_id);
                goto pvr_encrypt_start_fail1;
            }
            //aui_dmx_start(p_status->h_dmx, NULL);
        }
    }

    /*
     After getting the DMX device handle, we started connecting DMX and DSC. That means tell dmx which dsc device will be used for encryption recording.
     But in the case of re-encryption, pvr and dmx are in different processes. This case is not involved in this case.

     First, we need to send to DMX the DSC handle which already get, through the function #aui_dmx_data_path_set.
     */
    aui_dmx_data_path dmx_data_path;            
    memset(&dmx_data_path, 0, sizeof(dmx_data_path));
    dmx_data_path.data_path_type = AUI_DMX_DATA_PATH_EN_REC;            //In this case, TS stream encryption.
    dmx_data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;         //Pass the DSC handle to DMX, which is in the same process.
    dmx_data_path.p_hdl_de_dev = NULL;
    dmx_data_path.p_hdl_en_dev = h_dsc;
    //dmx_data_path.p_dsc_id;
    aui_ret = aui_dmx_data_path_set(p_status->h_dmx, &dmx_data_path);   //Tell DMX which DSC devide to use next.  
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("aui_dmx_data_path_set failed %ld\n", aui_ret);
        goto pvr_encrypt_start_fail1;
    }

    /*
      Because of the different processes, we cannot pass the handle directly.
      But after we did the steps above, DMX had already established a connection with DSC.
      
      Next, we need to use the function #aui_dmx_dsc_id_get to get the DSC' id which other process could recognize,
      so the other process (eg: pvr re-encryption process) could call the DSC device which opened in this process.
      */
    
#ifdef linux
    aui_dmx_dsc_id dmx_dsc_id;
    aui_dmx_dsc_id_get(p_status->h_dmx, &dmx_dsc_id);                  //If it is in multi-process situation, when other processes want to call the DSC device of this process, you need to use this function to get the DSC' id which other processes can recognize.
#else
#endif

    /*
      Finally, we will config DSC' id which other process could recognize to DMX, through function #aui_dmx_data_path_set. In case of another process call.
      */
#ifdef linux
    MEMSET(&dmx_data_path, 0, sizeof(dmx_data_path));
    dmx_data_path.data_path_type = AUI_DMX_DATA_PATH_EN_REC;
    dmx_data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_ID;
    dmx_data_path.p_dsc_id = &dmx_dsc_id;
    if (aui_dmx_data_path_set(p_status->h_dmx, &dmx_data_path)) {      //After get dmx_dsc_id which other processes can recognize, call #aui_dmx_data_path_set to set DMX again. So PVR process find DSC device.
        PVR_ENC_ERR("\r\n aui_dmx_data_path_set failed\n");
    }
#else
#endif

    p_status->is_working = true;   //Set tag #is_working true, means that this function #pvr_encrypt_start will no longer be repeat called, and enable func #pvr_crypto_key_change.

    unlock(p_status);              
    return AUI_RTN_SUCCESS;


pvr_encrypt_start_fail1:
    dsc_context_deinit(p_status);

pvr_encrypt_start_fail:

    unlock(p_status);
    return AUI_RTN_FAIL;

}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear all the data of the struct #pvr_reencrypt_status,
*                          then close  DSC & KL device which has been opened.    
*                          (finish this function mainly by called FUNC #dsc_context_deinit.)
*
*    @call time       This function will be called automatically by driver when RECORD STOP.
*                               
*    @param[in]     *p_param    =      Pointer to a struct #aui_pvr_crypto_general_param 
*                                                   which save DMX id info and PID info
*
*    @return         AUI_RTN_SUCCESS      = Close all the devices successfully
*    @return         AUI_RTN_FAIL            = Close devices failed
*/

AUI_RTN_CODE pvr_encrypt_stop(aui_pvr_crypto_general_param *p_param)
{
    UNUSED(p_param);
    struct pvr_reencrypt_status* p_status = NULL;

    if (true != g_is_pvr_reencrypt_status_inited) {
        PVR_ENC_ERR("g_is_pvr_reencrypt_status_inited %d.\n", g_is_pvr_reencrypt_status_inited);
        return AUI_RTN_FAIL;
    }
    
    p_status = pvr_reencrypt_status_get(0);
    if (NULL == p_status) {
        PVR_ENC_ERR("pvr_reencrypt_status_get fail.\n");
        return AUI_RTN_FAIL;
    }
    if (true != lock(p_status)) {   
        PVR_ENC_ERR("lock fail.\n");
        return AUI_RTN_FAIL;
    }
    
    dsc_context_deinit(p_status);    //Clear all the data in p_status, then close  DSC & KL device which has been opened.  
    
    unlock(p_status);
    return AUI_RTN_SUCCESS;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to execute the first time configuration of the DSC module
*                          using the p_status set by the user, and then tells DMX the DSC information 
*                          which will be used next.
*     
*    @call time       This function can only be called once, only when the prepare stage before PLAYBACK START.
*                          It will be called automatically by driver when the driver needs to config the connection of DMX with the DSC device.
*    
*    @param[in]     *p_param    =      Pointer to a struct #aui_pvr_crypto_general_param 
*                                                   which save DMX id info and PID info
*                                                    
*    @param[out]   *p_status    =      Pointer to a struct #pvr_reencrypt_status
*                                                   which is to specify the work attibute of pvr en/decryption
*    
*    @return         AUI_RTN_SUCCESS      = Config DSC device and connect to DMX successfully
*    @return         AUI_RTN_FAIL            = The input parameters (i.e. [in]) is invalid or Config DSC device/Set path to DMX failed
*/

AUI_RTN_CODE pvr_decrypt_start(aui_pvr_crypto_general_param *p_param)
{
    struct pvr_reencrypt_status* p_status = NULL;
    if (true != g_is_pvr_reencrypt_status_inited) {               //The parameter set to true only when the first time program enters #pvr_crypto_key_set to init p_status. Here is to make sure that p_status save the user's set completely.
        PVR_ENC_ERR("g_is_pvr_reencrypt_status_inited %d.\n", g_is_pvr_reencrypt_status_inited);
        return AUI_RTN_FAIL;
    }
    
    p_status = pvr_reencrypt_status_get(0);                      //Get the p_status which has been init & has save user's set.
    if (NULL == p_status) {
        PVR_ENC_ERR("pvr_reencrypt_status_get fail.\n");
        return AUI_RTN_FAIL;
    }
    if (true != lock(p_status)) {                              //Protect the decrypted key value & it's attribute not be change when dsc_cfg and set dsc id to dmx.
        PVR_ENC_ERR("lock fail.\n");
        return AUI_RTN_FAIL;
    }

    if (true != p_status->is_key_valid) {                      //If not save user's set in p_status return err.
        PVR_ENC_ERR("p_status->is_key_valid %d\n", p_status->is_key_valid);
        goto pvr_decrypt_start_fail;
    }
    
    if (true == p_status->is_working) {                        //This function #pvr_decrypt_start can only be called once, which is to configure the opened DSC information to DMX after initializing the user's Settings. If you repeat to call this func, return error.
        PVR_ENC_ERR("p_status->is_working %d\n", p_status->is_working);
        goto pvr_decrypt_start_fail;
    }
    p_status->is_encrypt = false;                              //Set tag #is_encrypting true, and it will be used in the function #cfd_dsc and #dsc_common_attr_set to tell dsc it is record or not now.

    if (0 == p_status->pid_count) {                            //No pid should be passed in when initializing. It is the correct step to enter the judgment and save the pids which need to be decrypted in p_status here.
        if (true != pvr_crypto_pids_save(p_status, p_param->pid_list, p_param->pid_num)) {
            PVR_ENC_ERR("pvr_crypto_pids_save failed\n");
            goto pvr_decrypt_start_fail1;
        }
    }

    aui_hdl h_dsc = dsc_cfg(p_status);                        //Configure the DSC with the p_status which set by the user. Here is the first time to pass key to DSC.
    if (NULL == h_dsc) {
        PVR_ENC_ERR("dsc_cfg failed\n");
        goto pvr_decrypt_start_fail1;
    }


    /*
       After config the KL, DSC device, we should tell DMX that the DSC' ID number which has been configured completely and should be used next.
       But before that, we need to make sure that DMX has been opened and get its device handle.
       */

    if (NULL == p_status->h_dmx) {                             //p_status->h_dmx should not be passed in when initializing. We set p_status->h_dmx in this judgment code.
        if (AUI_RTN_SUCCESS != aui_find_dev_by_idx(AUI_MODULE_DMX, p_param->dmx_id, &p_status->h_dmx)) {   //Usually, dmx has been opened before. Call this function #aui_find_dev_by_idx to find out if DMX has been opened. And if opened, h_dmx is returned and assigned to p_status.
                aui_attr_dmx attr_dmx;
                memset(&attr_dmx, 0, sizeof(aui_attr_dmx));
                attr_dmx.uc_dev_idx = p_param->dmx_id;  //DMX_ID_SW_DEMUX0;
                if(aui_dmx_open(&attr_dmx, &p_status->h_dmx)) { //If the user enters the sample code interface directly playback the record program, then open DMX here.
                    PVR_ENC_ERR("aui_find_dev_by_idx AUI_MODULE_DMX failed(dmx_id %d)\n", p_param->dmx_id);
                    goto pvr_decrypt_start_fail1;
                }
                //aui_dmx_start(p_status->h_dmx, NULL);
        }
    }

    /*
     After getting the DMX device handle, we started connecting DMX and DSC. That means tell dmx which dsc device will be used for decryption playback.
     But in the case of re-encryption, pvr and dmx are in different processes. This case is not involved in this case.

     First, we need to send to DMX the DSC handle which already get, through the function #aui_dmx_data_path_set.
     */
 
    aui_dmx_data_path dmx_data_path;
    memset(&dmx_data_path, 0, sizeof(dmx_data_path));
    dmx_data_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;      //In this case, TS stream decryption.
    dmx_data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_HANDLE;    //Pass the DSC handle to DMX, which is in the same process.
    dmx_data_path.p_hdl_de_dev = h_dsc;
    dmx_data_path.p_hdl_en_dev = NULL;

    AUI_RTN_CODE aui_ret = aui_dmx_data_path_set(p_status->h_dmx, &dmx_data_path);   //Tell DMX which DSC devide to use next. 
    if (AUI_RTN_SUCCESS != aui_ret) {
        PVR_ENC_ERR("aui_dmx_data_path_set failed %ld\n", aui_ret);
        goto pvr_decrypt_start_fail1;
    }

    /*
      Because of the different processes, we cannot pass the handle directly.
      But after we did the steps above, DMX had already established a connection with DSC.
      
      Next, we need to use the function #aui_dmx_dsc_id_get to get the DSC' id which other process could recognize,
      so the other process (eg: pvr re-encryption process) could call the DSC device which opened in this process.
      */

#ifdef linux    
    aui_dmx_dsc_id dmx_dsc_id;
    aui_dmx_dsc_id_get(p_status->h_dmx, &dmx_dsc_id);             //If it is in multi-process situation, when other processes want to call the DSC device of this process, you need to use this function to get the DSC' id which other processes can recognize.
#else
#endif

    /*
         Finally, we will config DSC' id which other process could recognize to DMX, through function #aui_dmx_data_path_set. In case of another process call.
      */
#ifdef linux
    aui_dmx_data_path data_path;
    memset(&data_path, 0, sizeof(data_path));
    data_path.data_path_type = AUI_DMX_DATA_PATH_DE_PLAY;
    data_path.dsc_type = AUI_DMX_DATA_PATH_DSC_TYPE_ID;
    data_path.p_dsc_id = &dmx_dsc_id;
    if(aui_dmx_data_path_set(p_status->h_dmx, &data_path)) {      //After get dmx_dsc_id which other processes can recognize, call #aui_dmx_data_path_set to set DMX again. So PVR process find DSC device.
        PVR_ENC_ERR("aui_dmx_data_path_set failed\n");
        goto pvr_decrypt_start_fail1;
    }
#else
#endif

    p_status->is_working = true;           //Set tag #is_working true, means that this function #pvr_decrypt_start will no longer be repeat called, and enable func #pvr_crypto_key_change.

    unlock(p_status);
    return AUI_RTN_SUCCESS;   

pvr_decrypt_start_fail1:
    dsc_context_deinit(p_status);
        
pvr_decrypt_start_fail: 
    unlock(p_status);
    return AUI_RTN_FAIL;
}


/****************************MODULE IMPLEMENT*************************************/

/**
*    @brief             Function used to clear all the data of the struct #pvr_reencrypt_status,
*                          then close  DSC & KL device which has been opened.    
*                          (finish this function mainly by called FUNC #dsc_context_deinit.)
*
*    @call time       This function will be called automatically by driver when PLAYBACK STOP.
*                               
*    @param[in]     *p_param    =      Pointer to a struct #aui_pvr_crypto_general_param 
*                                                   which save DMX id info and PID info
*
*    @return         AUI_RTN_SUCCESS      = Close all the devices successfully
*    @return         AUI_RTN_FAIL            = Close devices failed
*/

AUI_RTN_CODE pvr_decrypt_stop(aui_pvr_crypto_general_param *p_param)
{
    UNUSED(p_param);
    
    struct pvr_reencrypt_status* p_status = NULL;

    if (true != g_is_pvr_reencrypt_status_inited) {
        PVR_ENC_ERR("g_is_pvr_reencrypt_status_inited %d.\n", g_is_pvr_reencrypt_status_inited);
        return AUI_RTN_FAIL;
    }
    
    p_status = pvr_reencrypt_status_get(0);
    if (NULL == p_status) {
        PVR_ENC_ERR("pvr_reencrypt_status_get fail.\n");
        return AUI_RTN_FAIL;
    }
    if (true != lock(p_status)) {
        PVR_ENC_ERR("lock fail.\n");
        return AUI_RTN_FAIL;
    }

    dsc_context_deinit(p_status);

    unlock(p_status);
    return AUI_RTN_SUCCESS;
}

/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to delete the repeated PIDs from PID LIST which need to be re-encrypted by the PVR Module.
*    
*    @param[in/out]   *pid_list       =      Pointer to a list which store the useful PIDs that need to be re-encrypted
*
*    @param[in]          pid_num      =      Number of PIDs which the PID LIST have
*    
*    @return          valid_pid_num   =      Number of valid PIDs after delete the repeated PIDs from the list
*/

static unsigned short int pvr_crypto_pids_check(unsigned short int *pid_list, unsigned short int pid_num)
{
    unsigned short int valid_pid_num = 0;
    unsigned short int i, j;

    for (i = 0; i < pid_num; i++) {
        if ((0 == pid_list[i]) || ((pid_list[i] & INVALID_PID) == INVALID_PID)) {
            continue;
        }

        for (j = 0; j < valid_pid_num; j++) {
            if (pid_list[i] == pid_list[j]) {
                pid_list[i] = INVALID_PID;
                break;
            }
        }

        if (j >= valid_pid_num) {
            pid_list[valid_pid_num++] = pid_list[i];
        }
    }

    return valid_pid_num;
}


/****************************MODULE IMPLEMENT*************************************/
        
/**
*    @brief             This function is used to remove useless PIDs which don't need to be re-encrypted.
*     
*    @call time       This function can only be called once, only when the prepare stage before RECORD/PLAYBACK START.
*                          It will be called automatically by driver when the driver needs to know which PIDs should be re-encrypted from lots of PIDs.
*    
*    @param[in/out]     *p_pids_param    =      Pointer to a struct #aui_pvr_crypto_pids_param 
*                                                                which specify the PIDs need to be re-encrypted by the PVR Module                                                  
*    
*    @return         AUI_RTN_SUCCESS      =      Set the PIDs need to be re-encrypted to PVR Module successfully
*    @return         AUI_RTN_FAIL            =      The input parameters (i.e. [in]) is invalid
*/

AUI_RTN_CODE pvr_crypto_pids_set(aui_pvr_crypto_pids_param *p_pids_param)
{
    struct aui_pvr_pid_info *pid_info = NULL;
    unsigned short int *pid_list;
    unsigned short int pid_list_size;
    unsigned short int pid_num = 0;
    unsigned short int i;

    if (NULL == p_pids_param) {
        PVR_ENC_ERR("p_pids_param NULL.\n");
        return AUI_RTN_FAIL;
    }
    
    pid_info = p_pids_param->pid_info;
    if (NULL == pid_info) {
        PVR_ENC_ERR("pid_info NULL.\n");
        return AUI_RTN_FAIL;
    }

    pid_list = p_pids_param->pid_list;
    if (NULL == pid_list) {
        PVR_ENC_ERR("pid_list NULL.\n");
        return AUI_RTN_FAIL;
    }
    
    pid_list_size = p_pids_param->pid_num;

    /*
      Save the useful program PIDs which need to en/decrypted, meanwhile remove other useless pids.
     */  
     
    if ((pid_info->video_pid != INVALID_PID) && (pid_num < pid_list_size)) {    
        pid_list[pid_num++] = pid_info->video_pid;
    }

    for (i = 0; (i < pid_info->audio_count) && (pid_num < pid_list_size); i++) {
        pid_list[pid_num++] = pid_info->audio_pid[i];
    }

    for (i = 0; (i < pid_info->ttx_pid_count) && (pid_num < pid_list_size); i++) {
        pid_list[pid_num++] = pid_info->ttx_pids[i];
    }

    for (i = 0; (i < pid_info->ttx_subt_pid_count) && (pid_num < pid_list_size); i++) {
        pid_list[pid_num++] = pid_info->ttx_subt_pids[i];
    }

    for (i = 0; (i < pid_info->subt_pid_count) && (pid_num < pid_list_size); i++) {
        pid_list[pid_num++] = pid_info->subt_pids[i];
    }
    
    p_pids_param->pid_num = pvr_crypto_pids_check(pid_list, pid_num);  //Delete the repeated pids, make the pid list clear and easy to read.
    if (0 >= p_pids_param->pid_num) {
        PVR_ENC_ERR("pid_num %d.\n", p_pids_param->pid_num);
        return AUI_RTN_FAIL;
    }

    return AUI_RTN_SUCCESS;
    
}

