/**  @file aui_input.c
 *	 @brief 	AUI input module
 *	 @author	oscar.shi
 *	 @date		2015-6-23
 *	 @version	1.0.0
 *	 @note		ali corp. all rights reserved. 2013-2020 copyright (C)
 */

/****************************INCLUDE HEAD FILE******************************/
#include "aui_common_priv.h"
#include <aui_input.h>
#include <aui_common.h>

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <pthread.h>
#include <aui_panel.h>

/* share library headers */
#include <alipltfretcode.h>
#include <alislinput.h>
#include <alislevent.h>

/****************************LOCAL MACRO************************************/
AUI_MODULE(INPUT)
/****************************LOCAL TYPE*************************************/


/** AUI input module dev */
typedef struct aui_key_dev
{
	struct aui_st_dev_priv_data data;

	aui_cfg_key cfg_key;
	struct alislinput_device *alislinput;
	pthread_t *thread;
	unsigned char phy_code;
	alisl_handle ev_handle;
} aui_key_dev, *aui_p_key_dev;

/****************************LOCAL VAR**************************************/
static aui_key_dev input_dev;
static unsigned long g_key_count;
struct alislinput_key_info g_key_info;
extern struct aui_pannel_key_map_cfg  g_panel_key_map_cfg;

/****************************LOCAL FUNC DECLEAR*****************************/

/****************************LOCAL FUNC IMPLEMENT***************************/
AUI_RTN_CODE aui_key_version_get(unsigned long *pul_version)
{
	(void) pul_version;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_key_init(p_fun_cb p_call_back_init,void *pv_param)
{
	(void)pv_param;
	(void)p_call_back_init;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_key_de_init(p_fun_cb p_call_back_de_init,void *pv_param)
{
	(void)p_call_back_de_init;
	(void)pv_param;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_key_open(unsigned long ul_devID,
		aui_cfg_key *p_key_param,
		aui_hdl *pp_hdl_key)
{
	AUI_RTN_CODE result = AUI_RTN_SUCCESS;

	(void) ul_devID;	
	if (NULL == pp_hdl_key) {
		AUI_ERR("Input error pp_hdl_key is NULL.\n");
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	result = alislinput_attach();
	if (result) {
		AUI_ERR("alislinput_attach fail.\n");
		return result;
	}
	result = (AUI_RTN_CODE)alislinput_open(&input_dev.alislinput);
	if (result) {
		AUI_ERR("alislinput_open fail.\n");
		alislinput_detach(input_dev.alislinput);
		return result;
	}
	if (p_key_param != NULL) {
		input_dev.cfg_key = *p_key_param;
	} else {
		memset(&input_dev.cfg_key, 0, sizeof(input_dev.cfg_key));
	}
    *pp_hdl_key = NULL;
	*pp_hdl_key = &input_dev;

#if 1
	// Must set a NULL key map with phy_code:2
	// then you can get physical key value
	input_dev.phy_code = 2;
	result |= alislinput_config_key_map(input_dev.alislinput, 2, NULL, 0, 0);
	result |= alislinput_config_panel_map(input_dev.alislinput, 2, NULL, 0, 0);
	if (result) {
		AUI_ERR("config key map error!\n");
		alislinput_close(input_dev.alislinput);
		alislinput_detach(input_dev.alislinput);
		*pp_hdl_key = NULL;
	}
#endif
	return result;
}

AUI_RTN_CODE aui_key_close(aui_hdl pv_hdl_key)
{
	AUI_RTN_CODE result = AUI_RTN_SUCCESS;
	aui_key_dev *p_hdl_key= (aui_key_dev *)pv_hdl_key;
	if (NULL == p_hdl_key || p_hdl_key != &input_dev
			|| input_dev.alislinput == NULL) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	result = (AUI_RTN_CODE)alislinput_close(input_dev.alislinput);
	alislinput_detach(input_dev.alislinput);
	if (!result) {
		input_dev.alislinput = NULL;
		//aui_dev_unreg(AUI_MODULE_PANEL, pv_hdl_key);
	}
	return result;
}

#ifdef USE_THREAD_TO_GET_KEY
static void *get_key_thread(void *p_param)
{
	(void)p_param;
	aui_key_info key;
	while (input_dev.cfg_key.fn_callback) {

		if (input_dev.phy_code == 2) {
			struct alislinput_key_info *key_info;
			key_info = alislinput_get_key_info(input_dev.alislinput, 1);

			if (key_info != NULL) {
				if (key_info->type == PAN_KEY_TYPE_PANEL) {
					key.e_type = AUI_KEY_TYPE_FRONTPANEL;
				} else if (key_info->type == PAN_KEY_TYPE_REMOTE) {
					key.e_type = AUI_KEY_TYPE_REMOTECTRL;
				}

				if (key_info->state == PAN_KEY_PRESSED) {
					key.e_status = AUI_KEY_STATUS_PRESS;
				} else if (key_info->state == PAN_KEY_RELEASE) {
					key.e_status = AUI_KEY_STATUS_RELEASE;
				} else if (key_info->state == PAN_KEY_REPEAT) {
					key.e_status = AUI_KEY_STATUS_REPEAT;
				}

				key.n_key_code = key_info->code_low;
				key.p_ext_input_data = (void*)key_info->code_high;

				if (key_info->state == PAN_KEY_RELEASE){
					g_key_count = 0;
				} else {
					if (g_key_info.code_low == key_info->code_low
							&& g_key_info.code_high  == key_info->code_high
							&& g_key_info.state != PAN_KEY_RELEASE) {
						g_key_count++;
					} else {
						g_key_count = 0;
					}
				}
				key.n_count = g_key_count;
				if (key.n_count)
					key.e_status = AUI_KEY_STATUS_REPEAT;
				g_key_info = *key_info;
				input_dev.cfg_key.fn_callback(&key,
						input_dev.cfg_key.pv_user_data);
			}
		} else {
			struct alislinput_key *sl_key;
			sl_key = alislinput_get_key(input_dev.alislinput, 1);
			if (sl_key != NULL) {
				if (sl_key->type == PAN_KEY_TYPE_PANEL) {
					key.e_type = AUI_KEY_TYPE_FRONTPANEL;
				} else if (sl_key->type == PAN_KEY_TYPE_REMOTE) {
					key.e_type = AUI_KEY_TYPE_REMOTECTRL;
				}

				if (sl_key->state == PAN_KEY_PRESSED) {
					key.e_status = AUI_KEY_STATUS_PRESS;
				} else if (sl_key->state == PAN_KEY_RELEASE) {
					key.e_status = AUI_KEY_STATUS_RELEASE;
				}

				if (sl_key->count > 1) {
					key.e_status = AUI_KEY_STATUS_REPEAT;
					key.n_count = sl_key->count - 1;
				} else {
					key.n_count = 0;
				}
				key.n_key_code = sl_key->code;
				key.p_ext_input_data = NULL;
				input_dev.cfg_key.fn_callback(&key,
						input_dev.cfg_key.pv_user_data);
			}
		}
		usleep(10 * 1000);
	}
	pthread_exit(NULL);
}

AUI_RTN_CODE aui_key_callback_register(aui_hdl pv_hdl_key,
		aui_key_callback fn_callback)
{
	AUI_RTN_CODE result = AUI_RTN_SUCCESS;
	aui_key_dev *p_hdl_key= (aui_key_dev *)pv_hdl_key;
	if (NULL == p_hdl_key || p_hdl_key != &input_dev
			|| input_dev.alislinput == NULL) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	p_hdl_key->cfg_key.fn_callback = fn_callback;
	if (p_hdl_key->cfg_key.fn_callback != NULL) {
		if (input_dev.thread == NULL) {
			input_dev.thread = (pthread_t *)malloc(sizeof(pthread_t));
			if (pthread_create(input_dev.thread, NULL, get_key_thread, NULL)
					!= 0) {
				result = 1;
			}
		}
	} else {
		if (input_dev.thread != NULL) {
			pthread_join(*input_dev.thread, NULL);
			free(input_dev.thread);
			input_dev.thread = NULL;
			input_dev.cfg_key.fn_callback = NULL;
			AUI_DBG("unreg aui_key_callback_register ok\n");
		}
	}

	return result;
}
#endif

static void *get_key_callback(void *p_param)
{
	(void)p_param;
	aui_key_info key;
	if (input_dev.cfg_key.fn_callback) {

		if (input_dev.phy_code == 2) { // get physical code
			struct alislinput_key_info *key_info;
			key_info = alislinput_get_key_info(input_dev.alislinput, 1);

			if (key_info != NULL) {
				if (key_info->type == PAN_KEY_TYPE_PANEL) {
					key.e_type = AUI_KEY_TYPE_FRONTPANEL;
				} else if (key_info->type == PAN_KEY_TYPE_REMOTE) {
					key.e_type = AUI_KEY_TYPE_REMOTECTRL;
				}

				if (key_info->state == PAN_KEY_PRESSED) {
					key.e_status = AUI_KEY_STATUS_PRESS;
				} else if (key_info->state == PAN_KEY_RELEASE) {
					key.e_status = AUI_KEY_STATUS_RELEASE;
				} else if (key_info->state == PAN_KEY_REPEAT) {
					key.e_status = AUI_KEY_STATUS_REPEAT;
				}

				key.n_key_code = key_info->code_low;
				key.p_ext_input_data = (void*)key_info->code_high;

				if (key_info->state == PAN_KEY_RELEASE){
					g_key_count = 0;
				} else {
					if (g_key_info.code_low == key_info->code_low
							&& g_key_info.code_high  == key_info->code_high
							&& g_key_info.state != PAN_KEY_RELEASE) {
						g_key_count++;
					} else {
						g_key_count = 0;
					}
				}
				key.n_count = g_key_count;
				if (key.n_count)
					key.e_status = AUI_KEY_STATUS_REPEAT;
				g_key_info = *key_info;
				input_dev.cfg_key.fn_callback(&key,
						input_dev.cfg_key.pv_user_data);
			}
		} else { // get logic code
			struct alislinput_key *sl_key;
			sl_key = alislinput_get_key(input_dev.alislinput, 1);
			if (sl_key != NULL) {
				if (sl_key->type == PAN_KEY_TYPE_PANEL) {
					key.e_type = AUI_KEY_TYPE_FRONTPANEL;
				} else if (sl_key->type == PAN_KEY_TYPE_REMOTE) {
					key.e_type = AUI_KEY_TYPE_REMOTECTRL;
				}

				if (sl_key->state == PAN_KEY_PRESSED) {
					key.e_status = AUI_KEY_STATUS_PRESS;
				} else if (sl_key->state == PAN_KEY_RELEASE) {
					key.e_status = AUI_KEY_STATUS_RELEASE;
				}

				if (sl_key->count > 1) {
					key.e_status = AUI_KEY_STATUS_REPEAT;
					key.n_count = sl_key->count - 1;
				} else {
					key.n_count = 0;
				}
				key.n_key_code = sl_key->code;
				key.p_ext_input_data = NULL;
				input_dev.cfg_key.fn_callback(&key,
						input_dev.cfg_key.pv_user_data);
			}
		}
		//usleep(10 * 1000);
	}
	return NULL;
}

AUI_RTN_CODE aui_key_callback_register(aui_hdl pv_hdl_key,
		aui_key_callback fn_callback)
{
	AUI_RTN_CODE result = AUI_RTN_SUCCESS;
	aui_key_dev *p_hdl_key= (aui_key_dev *)pv_hdl_key;
	if (NULL == p_hdl_key || p_hdl_key != &input_dev
			|| input_dev.alislinput == NULL) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if (p_hdl_key->ev_handle == NULL) {
		alislevent_open(&p_hdl_key->ev_handle);
	}
	if (p_hdl_key->ev_handle == NULL) {
		AUI_ERR("alislevent_open fail\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}


	p_hdl_key->cfg_key.fn_callback = fn_callback;

	if (fn_callback != NULL) {
		result = alislinput_callback_register(p_hdl_key->alislinput,
				get_key_callback);
	} else {
		result = alislinput_callback_register(p_hdl_key->alislinput, NULL);
	}

	return result;
}

AUI_RTN_CODE aui_key_key_get(aui_hdl pv_hdl_key, aui_key_info* pkey_info)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_key_dev *p_hdl_key= (aui_key_dev *)pv_hdl_key;
	if (NULL == p_hdl_key || p_hdl_key != &input_dev
			|| input_dev.alislinput == NULL) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	if (input_dev.phy_code == 2) {
		aui_key_info key;
		struct alislinput_key_info *key_info;
		key_info = alislinput_get_key_info(input_dev.alislinput, 1);

		if (key_info != NULL) {
			if (key_info->type == PAN_KEY_TYPE_PANEL) {
				key.e_type = AUI_KEY_TYPE_FRONTPANEL;
			} else if (key_info->type == PAN_KEY_TYPE_REMOTE) {
				key.e_type = AUI_KEY_TYPE_REMOTECTRL;
			}

			if (key_info->state == PAN_KEY_PRESSED) {
				key.e_status = AUI_KEY_STATUS_PRESS;
			} else if (key_info->state == PAN_KEY_RELEASE) {
				key.e_status = AUI_KEY_STATUS_RELEASE;
			} else if (key_info->state == PAN_KEY_REPEAT) {
				key.e_status = AUI_KEY_STATUS_REPEAT;
			}

			key.n_key_code = key_info->code_low;
			key.p_ext_input_data = (void*)key_info->code_high;

			if (key_info->state == PAN_KEY_RELEASE){
				g_key_count = 0;
			} else {
				if (g_key_info.code_low == key_info->code_low
						&& g_key_info.code_high  == key_info->code_high
						&& g_key_info.state != PAN_KEY_RELEASE) {
					g_key_count++;
				} else {
					g_key_count = 0;
				}
			}
			key.n_count = g_key_count;
			if (key.n_count)
				key.e_status = AUI_KEY_STATUS_REPEAT;
			g_key_info = *key_info;
			*pkey_info = key;
		} else {
		    ret = AUI_RTN_FAIL; // Return fail if no key input
		}
	} else {
		struct alislinput_key *sl_key;
		sl_key = alislinput_get_key(input_dev.alislinput, 1);
		if (sl_key != NULL) {
			if (sl_key->type == PAN_KEY_TYPE_PANEL) {
				pkey_info->e_type = AUI_KEY_TYPE_FRONTPANEL;
			} else if (sl_key->type == PAN_KEY_TYPE_REMOTE) {
				pkey_info->e_type = AUI_KEY_TYPE_REMOTECTRL;
			}

			if (sl_key->state == PAN_KEY_PRESSED) {
				pkey_info->e_status = AUI_KEY_STATUS_PRESS;
			} else if (sl_key->state == PAN_KEY_RELEASE) {
				pkey_info->e_status = AUI_KEY_STATUS_RELEASE;
			}

			if (sl_key->count > 1) {
				pkey_info->e_status = AUI_KEY_STATUS_REPEAT;
				pkey_info->n_count = sl_key->count - 1;
			} else {
				pkey_info->n_count = 0;
			}
			pkey_info->n_key_code = sl_key->code;
			pkey_info->p_ext_input_data = NULL;
		}else {
            ret = AUI_RTN_FAIL; // Return fail if no key input
        }
	}

	return ret;
}

AUI_RTN_CODE aui_pan_ir_set_endian(struct aui_pan_ir_endian aui_ir_endian)
{
	(void)aui_ir_endian;
	AUI_WARN("TODO aui_pan_ir_set_endian\n");
	return AUI_RTN_SUCCESS;
}

/**
 * @brief		Set IR remote key map.
 * @param[in]	pv_hdl_key AUI handle.
 * @param[in]	cfg        AUI key map configuration.
 * @return		return AUI_RTN_SUCCESS, if success. or others.
 * @note		none
 */
AUI_RTN_CODE aui_key_set_ir_map(aui_hdl pv_hdl_key,
		struct aui_key_map_cfg *cfg)
{
	AUI_RTN_CODE err = AUI_RTN_SUCCESS;
	aui_key_dev *p_hdl_key= (aui_key_dev *)pv_hdl_key;
	if (NULL == p_hdl_key || p_hdl_key != &input_dev
			|| input_dev.alislinput == NULL) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	input_dev.phy_code = 0;
	err = (AUI_RTN_CODE)alislinput_config_key_map(input_dev.alislinput,
			input_dev.phy_code,
			(unsigned char *)cfg->map_entry,
			cfg->unit_num * sizeof(struct aui_key_map),
			sizeof(struct aui_key_map));

    /* Why do this?
     * Because the panel key map was clear in "aui_key_open",
     * if we do not set panel key map here, 
     * the driver would not report panel key anymore.
     * The "g_panel_key_map_cfg" would be assigned by API "aui_panel_set_key_map" in Panel module.
    */
    if (g_panel_key_map_cfg.map_entry) {
        AUI_DBG("auto set panel map! %p, unit_num: %ld\n", 
                    &g_panel_key_map_cfg, g_panel_key_map_cfg.unit_num);
        err |= (AUI_RTN_CODE)alislinput_config_panel_map(input_dev.alislinput,
			input_dev.phy_code,
			(unsigned char *)g_panel_key_map_cfg.map_entry,
			g_panel_key_map_cfg.unit_num * sizeof(struct aui_pannel_key_map),
			sizeof(struct aui_pannel_key_map));
    }
	return err;
}


/**
 * @brief		Set IR remote key repeat interval.
 * @param[in]	pv_hdl_key	AUI handle.
 * @param[in]	delay		The delay before repeating a keystroke.
 * @param[in]	interval	The period to repeat a keystroke.
 * @return		return AUI_RTN_SUCCESS, if success. or others.
 * @note		All duration values are expressed in ms.
 */
AUI_RTN_CODE aui_key_set_ir_rep_interval(aui_hdl pv_hdl_key,
		unsigned long delay,
		unsigned long interval)
{
	AUI_RTN_CODE err = AUI_RTN_SUCCESS;
	aui_key_dev *p_hdl_key= (aui_key_dev *)pv_hdl_key;
	if (NULL == p_hdl_key || p_hdl_key != &input_dev
			|| input_dev.alislinput == NULL) {
		AUI_ERR("Input error\n");
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}
	err = (AUI_RTN_CODE)alislinput_config_ir_rep_interval(input_dev.alislinput,
			delay, interval);
	return err;
}

