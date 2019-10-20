#ifndef __CAS9_PVR_H__
#define __CAS9_PVR_H__

#include <sys_config.h>

#include <api/libpvr/lib_pvr_eng.h>

#define CAS9_PVR_REENCRYPT_PIDS_FROM_CA

int aui_pvr_generate_keys(pvr_crypto_key_param *key_param);
UINT16 aui_pvr_check_reencrypt_pids(UINT16 *pid_list, UINT16 pid_num);
UINT16 aui_pvr_set_reencrypt_pids(struct pvr_pid_info *pid_info,UINT16 *pid_list, UINT16 pid_list_size);

int aui_cas9_pvr_rec_config(pvr_crypto_general_param *rec_param);
//extern int cas9_pvr_playback_config(pvr_crypto_general_param *play_param);
int aui_cas9_pvr_playback_config(pvr_crypto_general_param *play_param,INT8 timeshift_flag);
int aui_cas9_pvr_rec_stop(pvr_crypto_general_param *rec_param);
int aui_cas9_pvr_playback_stop(pvr_crypto_general_param *play_param);

void aui_cas9_set_dsc_for_live_play(UINT16 dmx_id, UINT32 stream_id);

int aui_cas9_pvr_init(void);

int aui_cas9_crypto_data(pvr_crypto_data_param *cp);
int aui_cas9_pvr_vsc_init(void);
//#endif /* CAS9_PVR_SUPPORT */

#endif /* __CAS9_PVR_H__ */

