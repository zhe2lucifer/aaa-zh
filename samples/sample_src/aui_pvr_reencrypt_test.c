#include "aui_pvr_reencrypt_test.h"

#define DWORD unsigned long

static int loop;
static int g_pvr_reencrypt_mode;
static int g_serial = 0;     //change odd or even
static sample_mode rec_play_mode;


#ifdef AUI_LINUX
pthread_t key_change_record_thread;
pthread_t key_change_play_thread;
#else 
OSAL_T_CTSK  task_param;
OSAL_ID task_key_change_record_id;
OSAL_ID task_key_change_playback_id;
#endif

void pvr_reencrypt_key_set(struct pvr_crypto_key* p_key)
{
    int serial = g_serial;
	g_serial++;
    
	memset(p_key, 0, sizeof(*p_key));
    
    static unsigned char default_key[16 * 2] = {
    	0x3c,0x0c,0x14,0x1e,0x3c,0x52,0x01,0x3e,
    	0x09,0x49,0x04,0x6a,0x51,0x1e,0x10,0x0d,
    	0xbc,0x0c,0x94,0x1e,0xbc,0xd2,0xa1,0x3e,
    	0x89,0xc9,0x84,0x6a,0x51,0x9e,0x90,0x8d
    }; 
    static unsigned char default_iv[16 * 2] = {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x11,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    };
    static unsigned char clear_key[16 * 2] = {0};
  
    // >>>============ the code below need to be set correctly based on the CAS requirement    
    static unsigned char protected_keys[][16] = {
       { 0x18,0xb6,0x7d,0xe3,0x5d,0x1a,0x3b,0x31,
         0xd8,0x46,0xd4,0xb3,0xe3,0x32,0x06,0x71 },
       { 0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef,
         0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f },
      // { 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
      //   0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef },      
      // { 0xf5,0x70,0x16,0x7e,0x9f,0x1d,0x05,0x4f,
      //   0x10,0xf6,0x97,0xf9,0xd6,0xf4,0xfd,0xef }      
    };
    static unsigned char iv[16 * 2] = {
    	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    }; 

	p_key->kl_level = AUI_KL_KEY_THREE_LEVEL; 
    p_key->kl_algo = AUI_KL_ALGO_AES;
    p_key->kl_root_key_idx = AUI_KL_ROOT_KEY_0_0; 
    p_key->dsc_algo = AUI_DSC_ALGO_AES;	//PVR_KEY_ALGO_AES, PVR_KEY_ALGO_DES, PVR_KEY_ALGO_TDES
	p_key->dsc_mode = AUI_DSC_WORK_MODE_IS_ECB;	//PVR_KEY_MODE_ECB, PVR_KEY_MODE_CBC, PVR_KEY_MODE_OFB, PVR_KEY_MODE_CFB, PVR_KEY_MODE_CTR
    p_key->key_length = 16;
    // >>>============ the code above need to be set correctly based on the CAS requirement

	int i;
	for (i = 0; i < 16 * 2; i++) {
		clear_key[i] = default_key[i] ^ serial;
	}
	if (rec_play_mode == REC_MODE) {
    	if (0 == (serial & 0x01)) {
            AUI_PRINTF("%s -> rec cfg even key\n", __func__);
    		p_key->parity = AUI_DSC_PARITY_MODE_EVEN_PARITY_MODE;	       //PVR_KEY_PARITY_EVEN, PVR_KEY_PARITY_ODD, PVR_KEY_PARITY_AUTO
            p_key->p_key = &clear_key[16];                 
    	} else{
    	    AUI_PRINTF("%s -> rec cfg odd key\n", __func__);
    		p_key->parity = AUI_DSC_PARITY_MODE_ODD_PARITY_MODE;	      //PVR_KEY_PARITY_EVEN, PVR_KEY_PARITY_ODD, PVR_KEY_PARITY_AUTO
            p_key->p_key = clear_key;
    	}
    } else if (rec_play_mode == PLAYBACK_MODE) {
        if (0 == (serial & 0x01)) {
            AUI_PRINTF("%s -> playback cfg even key\n", __func__);
            AUI_PRINTF("%s -> serial =  %d\n", __func__,serial);
            p_key->parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 2;
       		p_key->p_key = &clear_key[16];               
    	} else {
    	    AUI_PRINTF("%s -> playback cfg odd key\n", __func__);
            AUI_PRINTF("%s -> serial =  %d\n", __func__,serial);
            p_key->parity = AUI_DSC_PARITY_MODE_AUTO_PARITY_MODE0 >> 1;
    		p_key->p_key = clear_key;
    	}
    }
	p_key->p_iv = iv;
    p_key->p_default_iv = default_iv;
    p_key->p_protected_keys = protected_keys[0];

    p_key->dsc_block_size = 256 * GEN_CA_TS_PACKAGE_SIZE;
}

void pvr_reencrypt_mode_init(int mode) 
{
	g_pvr_reencrypt_mode = mode;
}

void pvr_sample_mode_set(int mode)
{
    rec_play_mode = mode;
}

int pvr_reencrypt_mode_get()
{
	return g_pvr_reencrypt_mode;
}

int pvr_reencrypt_module_init()
{
	if (AUI_PVR_VMX_MULTI_RE_ENCRYPTION == g_pvr_reencrypt_mode) {
        g_serial = 0;
        struct pvr_crypto_key key;
        pvr_reencrypt_key_set(&key);
        
        struct pvr_set_crypto_key_hd key_hd;
        memset(&key_hd, 0, sizeof(key_hd));
        if (AUI_RTN_SUCCESS != pvr_crypto_key_set(&key, &key_hd, NULL)) {
            AUI_PRINTF("pvr_crypto_key_set failed\n");
            return 1;
        }  
	} 
    return 0;	
}

void pvr_reencrypt_module_deinit()
{
	if (AUI_PVR_VMX_MULTI_RE_ENCRYPTION == g_pvr_reencrypt_mode) {   
		pvr_crypto_key_free();
    	g_serial = 0;
	}
        
	pvr_reencrypt_mode_init(AUI_PVR_NONE);
}

void pvr_reencrypt_callback(aui_hdl handle, unsigned int msg_type, unsigned int msg_code, void* user_data)
{
	switch (msg_type) {
		case AUI_EVNT_PVR_MSG_REC_START_OP_STARTDMX:
			pvr_encrypt_start((aui_pvr_crypto_general_param*)msg_code);
			break;
			
		case AUI_EVNT_PVR_MSG_REC_STOP_OP_STOPDMX:
			pvr_encrypt_stop((aui_pvr_crypto_general_param*)msg_code);
			break;

		case AUI_EVNT_PVR_MSG_PLAY_START_OP_STARTDMX:
			pvr_decrypt_start((aui_pvr_crypto_general_param*)msg_code);
            
			break;

		case AUI_EVNT_PVR_MSG_PLAY_STOP_OP_STOPDMX:
			pvr_decrypt_stop((aui_pvr_crypto_general_param*)msg_code);
			break;
		
		case AUI_EVNT_PVR_MSG_PLAY_SET_REENCRYPT_PIDS:
		case AUI_EVNT_PVR_MSG_REC_SET_REENCRYPT_PIDS:		
            pvr_crypto_pids_set((aui_pvr_crypto_pids_param*)msg_code);
			break;
         
        case AUI_EVNT_PVR_MSG_PLAY_STOP:
            g_is_pvr_reencrypt_status_inited = false;
            pvr_reencrypt_module_init();

		default:
			;
	}
}

void pvr_reencrypt_key_change()
{
	if (AUI_PVR_VMX_MULTI_RE_ENCRYPTION == g_pvr_reencrypt_mode){
    	struct pvr_crypto_key key;
    	pvr_reencrypt_key_set(&key);
        
    	if (AUI_RTN_SUCCESS != pvr_crypto_key_set(&key, NULL, NULL)){
    		AUI_PRINTF("pvr_crypto_key_set failed\n");
    	}	
	}

}

#ifdef AUI_LINUX
static void pvr_crypto_key_change_record_handler(void *handler)
#else
static void pvr_crypto_key_change_record_handler(DWORD handler, DWORD unused)
#endif
{
    
#ifdef AUI_LINUX
	aui_hdl aui_pvr_handler = handler;
#else
    aui_hdl aui_pvr_handler = (void*)handler;
#endif

    unsigned int duarion;
    AUI_SLEEP(2000);

    while (loop) {
        do{
            aui_pvr_get(aui_pvr_handler,AUI_PVR_REC_TIME_S,&duarion,0,0);
            AUI_PRINTF("************************PVR recorder duarion[%d]****************************\n",duarion);
            AUI_SLEEP(1000);
        } while (duarion %10 != 0);
        pvr_reencrypt_key_change();
    }


}

#ifdef AUI_LINUX
static void pvr_crypto_key_change_play_handler(void *handler)
#else
static void pvr_crypto_key_change_play_handler(DWORD handler, DWORD unused)
#endif
{

#ifdef AUI_LINUX
	aui_hdl aui_pvr_handler = handler;
#else
    aui_hdl aui_pvr_handler = (void*)handler;
#endif

    unsigned int duarion;
    unsigned int duarion_old = 0;
    AUI_SLEEP(2000);

    while (loop) {
        aui_pvr_get(aui_pvr_handler,AUI_PVR_PLAY_TIME_S,&duarion,0,0);
        if(duarion != duarion_old){
             AUI_PRINTF("************************PVR playback duarion[%d]****************************\n",duarion);
        }           
            
        AUI_SLEEP(600);   

        while (duarion %10 == 5) {
            if(duarion == duarion_old) {
                break;
            }           
            pvr_reencrypt_key_change();
            AUI_SLEEP(600);  
            aui_pvr_get(aui_pvr_handler,AUI_PVR_PLAY_TIME_S,&duarion,0,0);
            duarion_old = duarion;
        } ;
        duarion_old = duarion;
    }
}


void pvr_crypto_key_change_record_run(aui_hdl handler)
{
    loop = true;
#ifdef AUI_LINUX
    if (pthread_create(&key_change_record_thread, NULL, (void *)&pvr_crypto_key_change_record_handler, handler) != 0) {
        AUI_PRINTF("fail to creat pthread key_change_record_thread\n"); 
    }
#else
    task_param.task = pvr_crypto_key_change_record_handler;
    task_param.name[0] = 'k';
    task_param.name[1] = 'c';
    task_param.name[2] = 'r';
    task_param.quantum = 10;
    task_param.itskpri = OSAL_PRI_NORMAL;
    task_param.stksz = 0x1000;      //4k
    task_param.para1 = (DWORD)handler;
    task_param.para2 = 0;

    task_key_change_record_id = osal_task_create(&task_param);
    AUI_PRINTF("Run task %s in task %d\n", "kcr", task_key_change_record_id);
#endif


}

void pvr_crypto_key_change_record_stop()
{
    loop = false;
    
#ifdef AUI_LINUX
    pthread_cancel(key_change_record_thread);
    pthread_join(key_change_record_thread,NULL);
#else
    if (task_key_change_record_id != OSAL_INVALID_ID) {
        osal_task_delete(task_key_change_record_id);
        AUI_PRINTF("Delete key_change_record_task id:%d\n", task_key_change_record_id);
    }
#endif

}

void pvr_crypto_key_change_playback_run(aui_hdl handler)
{
    loop = true;
#ifdef AUI_LINUX    
    if (pthread_create(&key_change_play_thread, NULL, (void *)&pvr_crypto_key_change_play_handler, handler) != 0) {
        AUI_PRINTF("fail to creat pthread change_key_play\n"); 
    }       
#else 
    task_param.task = pvr_crypto_key_change_play_handler;  
    task_param.name[0] = 'k';
    task_param.name[1] = 'c';
    task_param.name[2] = 'p';
    task_param.quantum = 10;
    task_param.itskpri = OSAL_PRI_NORMAL;
    task_param.stksz = 0x1000;
    task_param.para1 = (DWORD)handler;
    task_param.para2 = 0;

    task_key_change_playback_id = osal_task_create(&task_param);
    AUI_PRINTF("Run task %s in task %d\n", "kcp", task_key_change_playback_id);
#endif

}

void pvr_crypto_key_change_playbck_stop()
{
    loop = false;
#ifdef AUI_LINUX
    pthread_cancel(key_change_play_thread);
    pthread_join(key_change_play_thread,NULL);
#else
    if (task_key_change_playback_id != OSAL_INVALID_ID) {
        osal_task_delete(task_key_change_playback_id);
        AUI_PRINTF("Delete task_key_change_playback id:%d\n", task_key_change_playback_id);
    }
#endif

}



