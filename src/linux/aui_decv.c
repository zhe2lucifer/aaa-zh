/**@file
 *    @brief        AUI decv module interface implement
 *    @author       henry.xie
 *    @date         2013-6-27
 *   @version       1.0.0
 *    @note         ali corp. all rights reserved. 2013~2999 copyright (C)
 */
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_decv.h>
#include <alislvdec.h>
#include <alisldis.h>
#include <alislavsync.h>
#include "aui_decv_priv.h"

AUI_MODULE(DECV)

typedef struct decv_device {
	aui_dev_priv_data       dev_priv_data;
	alisl_handle            handle;
	alisl_handle            avsync_hdl;
	alisl_handle            dis_hdl;
	aui_attr_decv           attr;
	void					*req_buf;
	int                     req_size;
	int                     update_size;
	int                     see_dmx_id;
	aui_decv_user_data_type userdata_type;
	char                    userdata_type_any;
	/**either this callback_func 
	   or callback in aui_decv_callback_node which is in aui_attr_decv 
	   takes value of callback funtion */
	common_callback callback_func[AUI_DECV_CB_MAX+1];
} decv_device_t;

typedef struct decv_user_data_info {
    aui_decv_user_data_type userdata_type;
    unsigned char start_code[4];
    unsigned int start_code_len;
    unsigned char send_extension_info;
} decv_user_data_info;   




static aui_fun_first_showed pfun_cur_first_showed = NULL;

/**************************** callback functions *************************************/

static void info_changed_callback(decv_device_t *decv_dev, uint32_t param1, uint32_t param2)
{
	(void)param2;
	struct aui_decv_info_cb new_info;
	struct vdec_info_from_cb *info = (struct vdec_info_from_cb *)param1;

	if (decv_dev->callback_func[AUI_DECV_CB_INFO_CHANGED]) {
		struct change_info new_info;
		int status = 0;
		status = info->flag;
		memset(&new_info, 0, sizeof(new_info));
		new_info.pic_width = info->pic_width;
		new_info.pic_height = info->pic_height;
		new_info.frame_rate = info->frame_rate;
		new_info.active_format = info->active_format;
		decv_dev->callback_func[AUI_DECV_CB_INFO_CHANGED](status, (unsigned int)&new_info);
	} else if (NULL != decv_dev->attr.callback_nodes[AUI_DECV_CB_INFO_CHANGED].callback) {
		memset(&new_info, 0, sizeof(new_info));
		new_info.flag = info->flag;
		new_info.pic_width = info->pic_width;
		new_info.pic_height = info->pic_height;
		new_info.frame_rate = info->frame_rate;
		new_info.active_format = info->active_format;
		new_info.sar_width = info->sar_width;
		new_info.sar_height = info->sar_height;
		decv_dev->attr.callback_nodes[AUI_DECV_CB_INFO_CHANGED].callback(
			decv_dev->attr.callback_nodes[AUI_DECV_CB_INFO_CHANGED].puser_data, (unsigned int)&new_info, 0);
	}
}

static void status_changed_callback(decv_device_t *decv_dev, uint32_t param1, uint32_t param2)
{
	enum aui_decv_state_type status = AUI_DECV_STATE_NODATA;
	(void)param2; // unused

	switch (param1) {
		case VDEC_DECODER_STATUS_NODATA:
			status = AUI_DECV_STATE_NODATA;
			break;
		case VDEC_DECODER_STATUS_DECODING:
			status = AUI_DECV_STATE_DECODING;
			break;
		default:
			return;
			break;
	}
	
	if (decv_dev->callback_func[AUI_DECV_CB_STATE_CHANGE]) {
		decv_dev->callback_func[AUI_DECV_CB_STATE_CHANGE](status, 0);
	} else if(NULL != decv_dev->attr.callback_nodes[AUI_DECV_CB_STATE_CHANGE].callback) {
		decv_dev->attr.callback_nodes[AUI_DECV_CB_STATE_CHANGE].callback(
			decv_dev->attr.callback_nodes[AUI_DECV_CB_STATE_CHANGE].puser_data, status, 0);
	}
}

static void decv_error_callback(decv_device_t *decv_dev, uint32_t param1, uint32_t param2)
{
	aui_decv_error_type error;
	(void) param2; // unused

	switch (param1) {
		case VDEC_ERROR_CODE_NODATA:
			error = AUI_DECV_ERROR_NO_DATA;
			break;
		case VDEC_ERROR_CODE_HARDWARE:
			error = AUI_DECV_ERROR_HARDWARE;
			break;
		case VDEC_ERROR_CODE_SYNC:
			error = AUI_DECV_ERROR_SYNC;
			break;
		case VDEC_ERROR_CODE_FRAMEDROP:
			error = AUI_DECV_ERROR_FRAME_DROP;
			break;
		default:
			return;
			break;
	}
	
	if (decv_dev->callback_func[AUI_DECV_CB_ERROR]) {
		decv_dev->callback_func[AUI_DECV_CB_ERROR](error, 0);
	} else if(NULL != decv_dev->attr.callback_nodes[AUI_DECV_CB_ERROR].callback) {
		decv_dev->attr.callback_nodes[AUI_DECV_CB_ERROR].callback(
			decv_dev->attr.callback_nodes[AUI_DECV_CB_ERROR].puser_data, error, 0);
	}
}

static enum vdec_callback_type aui_decv_cb_type_aui_to_sl(enum aui_en_decv_callback_type type)
{
    enum aui_en_decv_callback_type sl_type = AUI_DECV_CB_MAX;
    
    switch(type)
    {
        case AUI_DECV_CB_FIRST_SHOWED:
            sl_type = SL_VDEC_CB_FIRST_SHOWED; 
            break;
        case AUI_DECV_CB_MODE_SWITCH_OK:
            sl_type = SL_VDEC_CB_MODE_SWITH_OK;
            break;
        case AUI_DECV_CB_BACKWARD_RESTART_GOP:
            sl_type = SL_VDEC_CB_BACKWARD_RESTART_GOP;
            break;
        case AUI_DECV_CB_FIRST_HEAD_PARSED:
            sl_type = SL_VDEC_CB_FIRST_HEAD_PARSED;
            break;
        case AUI_DECV_CB_FIRST_I_FRAME_DECODED:
            sl_type = SL_VDEC_CB_FRIST_I_FRAME_DECODED;
            break;
        case AUI_DECV_CB_USER_DATA_PARSED:
            sl_type = SL_VDEC_CB_USER_DATA_PARSED;
            break;
        case AUI_DECV_CB_INFO_CHANGED:
            sl_type = SL_VDEC_CB_INFO_CHANGED;
            break;
        case AUI_DECV_CB_STATE_CHANGE:
            sl_type = SL_VDEC_CB_STATUS_CHANGED;
            break; 
        case AUI_DECV_CB_ERROR:
            sl_type = SL_VDEC_CB_ERROR;
            break;
        case AUI_DECV_CB_MONITOR_GOP:
            sl_type = SL_VDEC_CB_MONITOR_GOP;
            break;    
        default:
            sl_type = SL_VDEC_CB_FIRST_SHOWED;
            break;
    }
    return sl_type;
}


static enum aui_en_decv_callback_type aui_decv_cb_type_sl_to_aui(enum vdec_callback_type type)
{
    enum aui_en_decv_callback_type aui_type = AUI_DECV_CB_MAX;
    
    switch(type)
    {
        case SL_VDEC_CB_FIRST_SHOWED:
            aui_type = AUI_DECV_CB_FIRST_SHOWED; 
            break;
        case  SL_VDEC_CB_MODE_SWITH_OK:
            aui_type = AUI_DECV_CB_MODE_SWITCH_OK;
            break;
        case SL_VDEC_CB_BACKWARD_RESTART_GOP:
            aui_type = AUI_DECV_CB_BACKWARD_RESTART_GOP;
            break;
        case SL_VDEC_CB_FIRST_HEAD_PARSED:
            aui_type = AUI_DECV_CB_FIRST_HEAD_PARSED;
            break;
        case SL_VDEC_CB_FRIST_I_FRAME_DECODED:
            aui_type = AUI_DECV_CB_FIRST_I_FRAME_DECODED;
            break;
        case SL_VDEC_CB_USER_DATA_PARSED:
            aui_type = AUI_DECV_CB_USER_DATA_PARSED;
            break;
        case SL_VDEC_CB_INFO_CHANGED:
            aui_type = AUI_DECV_CB_INFO_CHANGED;
            break;
        case SL_VDEC_CB_STATUS_CHANGED:
            aui_type = AUI_DECV_CB_STATE_CHANGE;
            break; 
        case SL_VDEC_CB_ERROR:
            aui_type = AUI_DECV_CB_ERROR;
            break;
        case SL_VDEC_CB_MONITOR_GOP:
            aui_type = AUI_DECV_CB_MONITOR_GOP;
            break;    
        default:
            aui_type = AUI_DECV_CB_MAX;
            break;
    }
    return aui_type;
}

/*start_code:  the start code of GA94 or SCTE-20*/
static int aui_decv_user_data_matched(unsigned char *buff, unsigned int len,unsigned char *start_code, unsigned int start_code_len)
{
	unsigned int i = 0;
	unsigned int is_find = 1;
	if (len < start_code_len){
        return 0;
	} 
	//when user data type is AUI_DECV_USER_DATA_ALL,start_code_len=0,return is_find
	for (i = 0; i < start_code_len; i++) {   
       
		if (buff[i] != start_code[i]) {
			is_find = 0;
			break;
		}	
	}
	return is_find;     
}


static int aui_decv_get_first_user_data_type(unsigned char *buff, unsigned int len)
{
    unsigned int i = 0;                                  //decv_user_data_info* user_data_info                                          
    for (i = 0; i < len; i ++){
        if ((buff[i] == 0x47)
            && (buff[i + 1] == 0x41)
            && (buff[i + 2] == 0x39)
            && (buff[i + 3] == 0x34)) {
            return AUI_DECV_USER_DATA_ATSC53;
        } else if((buff[i] == 0x03)
			&& (buff[i + 1] == 0x81)) {
            return AUI_DECV_USER_DATA_SCTE20;
        }
    }
    return AUI_DECV_USER_DATA_ANY;
}

static void aui_decv_send_userdata(decv_device_t *decv_dev, 
                                         unsigned char *buff, 
                                         unsigned int len,
                                         unsigned char *extension_info,
                                         unsigned char is_avc_last) //will be delete after avc extenstion info is ready
{
    int data_len = len;
    decv_user_data_info user_data_info[AUI_DECV_USER_DATA_ALL + 1] = {
       {AUI_DECV_USER_DATA_DEFAULT, {0x47,0x41,0x39,0x34}, 4, 0},
       {AUI_DECV_USER_DATA_ANY, {0x00,0x00,0x00,0x00}, 0, 1},
       {AUI_DECV_USER_DATA_ATSC53, {0x47,0x41,0x39,0x34}, 4, 1},
       {AUI_DECV_USER_DATA_SCTE20, {0x03,0x81,0x00,0x00}, 2, 1},
       {AUI_DECV_USER_DATA_ALL, {0x00,0x00,0x00,0x00}, 0, 1},      //AUI_DECV_USER_DATA_ALL,send all user data
    };                                                             //there is no need to identify start code,start_code_len=0

    //determine any type
    if (decv_dev->userdata_type == AUI_DECV_USER_DATA_ANY) {
        decv_dev->userdata_type = aui_decv_get_first_user_data_type(buff, data_len);    
        if (decv_dev->userdata_type == AUI_DECV_USER_DATA_ANY) {
            //AUI_DECV_USER_DATA_ANY can only be GA94 or SCTE20, otherwise drop the data
            return;
        }
    }

    //to check the user data depends on userdata_type, if matched send, if not drop
    if (aui_decv_user_data_matched(buff, data_len, 
                            user_data_info[decv_dev->userdata_type].start_code, 
                            user_data_info[decv_dev->userdata_type].start_code_len)) {
        if (user_data_info[decv_dev->userdata_type].send_extension_info) {
            //move extension info          
            if (is_avc_last) {//will be delete after avc extenstion info is ready
                //now AVC video does not have extension info, so move forward 4 bytes
                memmove(buff - 4, buff, data_len);               
                data_len += 4;
                buff -= 4;
            } 
            //use start code buffer to store extension info, 4 bytes
            memcpy(&buff[data_len-4], extension_info, 4);
        } else {
            //subtract the length of extension info (4 bytes)
            if (!is_avc_last) {  //AVC video has no extension info, so the last user data no need remove extension info length
                data_len -= 4;
            }    
        }
       
        if (decv_dev->callback_func[AUI_DECV_CB_USER_DATA_PARSED]) {         
            decv_dev->callback_func[AUI_DECV_CB_USER_DATA_PARSED](data_len, (unsigned int)buff);
        } else if (NULL != decv_dev->attr.callback_nodes[AUI_DECV_CB_USER_DATA_PARSED].callback) {
            decv_dev->attr.callback_nodes[AUI_DECV_CB_USER_DATA_PARSED].callback(
                decv_dev->attr.callback_nodes[AUI_DECV_CB_USER_DATA_PARSED].puser_data, data_len,(unsigned int)buff);
        }
                        

    }
}

/*start_code:  the start code of MPEG2 or AVC video*/
static void aui_decv_userdata_filter(decv_device_t *decv_dev, 
                                         unsigned char *buff, 
                                         unsigned int buff_len,
                                         unsigned char *start_code,
                                         unsigned int start_code_len,
                                         unsigned char *extension_info)
{
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int user_data_start_index = 0;//start index of each user data 
    unsigned int last_start_code_index = 0;//the start index of the last one user data. 
    bool next_start_code_flag = FALSE;
    unsigned int user_data_len = 0;

    for (j = 0; j + start_code_len < buff_len; j++){    
        if (!memcmp(&buff[j], start_code, start_code_len) || next_start_code_flag) {
            user_data_start_index = j + start_code_len;
            next_start_code_flag = FALSE;
            /*find the next start code(the end of current user data):  
                        if find: send current user data
                        cannot find: if there is no more start code, break to handle the last user data                      
                    */
            for (i = j + start_code_len; i + start_code_len < buff_len; i++) {               
                if (!memcmp(&buff[i], start_code, start_code_len)) {
                    // 4 is extension info length, use start code buffer to store extension info
                    user_data_len = i - j - (start_code_len - 4); 
                    aui_decv_send_userdata(decv_dev, buff + user_data_start_index, user_data_len, extension_info, 0);
                    last_start_code_index = i;
                    next_start_code_flag = TRUE;
                    j = i - 1;//j++ will happen after this loop
                    break;
                }
            }
            if (next_start_code_flag == FALSE)
                break;
        }
    }

    //the last one user data info
    user_data_start_index = last_start_code_index + start_code_len ;
    user_data_len = buff_len - user_data_start_index;

    //the last one user data, special case for AVC video ,AVC does not have extension info
    //will be changed after avc extenstion info is ready by driver
    if (start_code_len == 5) {
        aui_decv_send_userdata(decv_dev, buff + user_data_start_index, user_data_len, extension_info, 1);  //AVC
    } else {
        aui_decv_send_userdata(decv_dev, buff + user_data_start_index, user_data_len, extension_info, 0);  //mpeg2
    }    
}

static void userdata_parsed_callback(decv_device_t *decv_dev, uint32_t param1, uint32_t param2)
{
  	unsigned char *buff = (unsigned char *)param2;
    unsigned int buff_len = param1;
    unsigned char avc_extension_info[4] = {0x00, 0x00, 0x00, 0x00};//will be delete after avc extenstion info is ready
    unsigned char mpeg2_start_code[4] = {0x00, 0x00, 0x01, 0xb2};
    unsigned char avc_start_code[5] = {0x00, 0x00, 0x00, 0x01, 0x06};
    
    if (!memcmp(buff, mpeg2_start_code, 4)) {
        aui_decv_userdata_filter(decv_dev, buff, buff_len, mpeg2_start_code, 4, &buff[buff_len-4]);
    } else if (!memcmp(buff, avc_start_code, 5)) {
        aui_decv_userdata_filter(decv_dev, buff, buff_len, avc_start_code, 5, avc_extension_info);
    }
}

static void aui_decv_common_callback(void *user_data, enum vdec_callback_type type,uint32_t param1, uint32_t param2)
{
	decv_device_t *decv_dev = NULL;
	enum aui_en_decv_callback_type cb_type = aui_decv_cb_type_sl_to_aui(type);
	if (NULL == user_data) {
		AUI_DBG("decv_dev is NULL\n");
		return ;
	}

	decv_dev = (decv_device_t *)user_data;

	switch (cb_type) {
		case AUI_DECV_CB_FIRST_SHOWED:
		case AUI_DECV_CB_MODE_SWITCH_OK:
		case AUI_DECV_CB_BACKWARD_RESTART_GOP:
		case AUI_DECV_CB_FIRST_HEAD_PARSED: 
		case AUI_DECV_CB_FIRST_I_FRAME_DECODED:
		case AUI_DECV_CB_MONITOR_GOP:
		{
			if (decv_dev->callback_func[cb_type]) {
				decv_dev->callback_func[cb_type](param1, param2);
			} else if(NULL != decv_dev->attr.callback_nodes[cb_type].callback) {
				decv_dev->attr.callback_nodes[cb_type].callback(
					decv_dev->attr.callback_nodes[cb_type].puser_data, param1, param2);
			}
			break;
		}
		case AUI_DECV_CB_USER_DATA_PARSED:
			userdata_parsed_callback(decv_dev, param1, param2);
			break;
		case AUI_DECV_CB_INFO_CHANGED:
			info_changed_callback(decv_dev, param1, param2);
			break;
		case AUI_DECV_CB_STATE_CHANGE:
			status_changed_callback(decv_dev, param1, param2);
			break;
		case AUI_DECV_CB_ERROR:
			decv_error_callback(decv_dev, param1, param2);
			break;
		default:
			break;
	}
}

static enum vdec_playback_speed convert_to_vdecsl_play_speed(enum aui_playback_speed speed)
{
	switch(speed) {
		case AUI_PLAYBACK_SPEED_1_2  :
				return VDEC_PLAYBACK_SPEED_1_2;
		case AUI_PLAYBACK_SPEED_1_4  :
			return VDEC_PLAYBACK_SPEED_1_4;
		case AUI_PLAYBACK_SPEED_1_8  :
			return VDEC_PLAYBACK_SPEED_1_8;
		case AUI_PLAYBACK_SPEED_STEP :
			return VDEC_PLAYBACK_SPEED_STEP;
		case AUI_PLAYBACK_SPEED_1    :
			return VDEC_PLAYBACK_SPEED_1;
		case AUI_PLAYBACK_SPEED_2    :
			return VDEC_PLAYBACK_SPEED_2;
		case AUI_PLAYBACK_SPEED_4    :
			return VDEC_PLAYBACK_SPEED_4;
		case AUI_PLAYBACK_SPEED_8    :
			return VDEC_PLAYBACK_SPEED_8;
		default:
			return -1;
	}
}

static enum vdec_playback_speed convert_from_vdecsl_play_speed(enum vdec_playback_speed speed)
{
	switch(speed) {
		case VDEC_PLAYBACK_SPEED_1_2  :
				return AUI_PLAYBACK_SPEED_1_2;
		case VDEC_PLAYBACK_SPEED_1_4  :
			return AUI_PLAYBACK_SPEED_1_4;
		case VDEC_PLAYBACK_SPEED_1_8  :
			return AUI_PLAYBACK_SPEED_1_8;
		case VDEC_PLAYBACK_SPEED_STEP :
			return AUI_PLAYBACK_SPEED_STEP;
		case VDEC_PLAYBACK_SPEED_1    :
			return AUI_PLAYBACK_SPEED_1;
		case VDEC_PLAYBACK_SPEED_2    :
			return AUI_PLAYBACK_SPEED_2;
		case VDEC_PLAYBACK_SPEED_4    :
			return AUI_PLAYBACK_SPEED_4;
		case VDEC_PLAYBACK_SPEED_8    :
			return AUI_PLAYBACK_SPEED_8;
		default:
			return -1;
	}
}

static enum vdec_playback_direction convert_to_vdecsl_play_dir(enum aui_playback_direction direction)
{
	switch (direction) {
		case AUI_PLAYBACK_FORWARD :
				return VDEC_PLAYBACK_FORWARD;
		case AUI_PLAYBACK_BACKWARD    :
			return VDEC_PLAYBACK_BACKWARD ;
		default:
			return -1;
	}
}

static enum vdec_playback_direction convert_from_vdecsl_play_dir(enum vdec_playback_direction direction)
{
	switch (direction) {
		case VDEC_PLAYBACK_FORWARD :
				return AUI_PLAYBACK_FORWARD;
		case VDEC_PLAYBACK_BACKWARD    :
			return AUI_PLAYBACK_BACKWARD ;
		default:
			return -1;
	}
}

static AUI_RTN_CODE aui_decv_callback_reg(aui_hdl pv_hdl_decv, struct aui_decv_callback_node *cb_node)
{
	enum vdec_callback_type cb_type = 0xff;
	vdec_callback func = aui_decv_common_callback;
	struct decv_device *decv_dev = pv_hdl_decv;
	
	if(cb_node->type >= AUI_DECV_CB_MAX) {
		aui_rtn(AUI_RTN_FAIL, "decv callback type[%d] is not supported\n", cb_node->type);
	}
	cb_type = aui_decv_cb_type_aui_to_sl(cb_node->type);
	memcpy(&decv_dev->attr.callback_nodes[cb_node->type], cb_node, sizeof(struct aui_decv_callback_node));

	if (alislvdec_reg_callback(decv_dev->handle, decv_dev, cb_type, func, NULL)) {
		aui_rtn(AUI_RTN_FAIL, "alislvdec_reg_callback cb_type[%d] fail!\n", cb_type);
	}

	AUI_DBG("decv callback type[%d] has been registered!", cb_node->type);

	return AUI_RTN_SUCCESS;
}

static AUI_RTN_CODE aui_decv_callback_unreg(aui_hdl pv_hdl_decv, enum aui_en_decv_callback_type type)
{
	enum vdec_callback_type cb_type = 0xff;
	struct decv_device *decv_dev = pv_hdl_decv;
	
	if(type >= AUI_DECV_CB_MAX) {
		aui_rtn(AUI_RTN_FAIL, "decv callback type[%d] is not supported\n", type);
	}
	cb_type = aui_decv_cb_type_aui_to_sl(type);
	memset(&decv_dev->attr.callback_nodes[type], 0, sizeof(struct aui_decv_callback_node));

	if (alislvdec_reg_callback(decv_dev->handle, decv_dev, cb_type, NULL, NULL)) {
		aui_rtn(AUI_RTN_FAIL, "alislvdec_reg_callback cb_type[%d] fail!\n", cb_type);
	}

	AUI_DBG("decv callback type[%d] has been unregistered!", type);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_init(aui_func_decv_init fn_decv_init)
{
	if (fn_decv_init != NULL) {
		fn_decv_init();
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_open(aui_attr_decv *pst_attr_decv, void **ppv_handle_decv)
{
    struct decv_device *dev = NULL;
    enum vdec_dis_layer layer = VDEC_DIS_MAIN_LAYER;
    enum vdec_decoder_type type;
    
    if (NULL == pst_attr_decv || NULL == ppv_handle_decv) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    dev = malloc(sizeof(*dev));

    if (NULL == dev) {
        aui_rtn(AUI_RTN_ENOMEM, NULL);
    }

    memset(dev, 0, sizeof(*dev));
    memcpy(&dev->attr, pst_attr_decv, sizeof(*pst_attr_decv));
    dev->see_dmx_id = 0;

    //AUI_DBG("pst_attr_decv->uc_dev_idx: %d\n", pst_attr_decv->uc_dev_idx);
    if (alislvdec_open(&dev->handle, pst_attr_decv->uc_dev_idx)) {
        free(dev);
        aui_rtn(AUI_RTN_FAIL, "open failed!\n");
    }

    if (alislavsync_open(&dev->avsync_hdl)) {
        AUI_DBG("open avsync fail\n");
    }

    if (alisldis_open(DIS_HD_DEV, &dev->dis_hdl)) {
        free(dev);
        aui_rtn(AUI_RTN_FAIL, "alisldis_open open failed!\n");
    }

    /*Annotate default format settings to avoid setting video decoder errors*/
    if (alislvdec_get_decoder_ext(dev->handle, &type)) {
        //set default format when there is no select video after close to avoid set error
        AUI_DBG("set default to mpeg2\n");
        alislvdec_set_decoder(dev->handle, VDEC_DECODER_MPEG, 0);//set default video format
    }
    
    //set dis_layer to sl for storing
    if (dev->attr.dis_layer == AUI_DIS_LAYER_VIDEO){
        layer = VDEC_DIS_MAIN_LAYER;
    } else if (dev->attr.dis_layer == AUI_DIS_LAYER_AUXP) {
        layer= VDEC_DIS_AUX_LAYER;
    }
    
    //set display layer
    alislvdec_set_display_layer(dev->handle, layer);

    /* For compatibility: convert decv callback_nodes index to callback type */
    int idx = 0;
    enum aui_en_decv_callback_type cb_type = AUI_DECV_CB_MAX;

    for (idx = 0; idx < AUI_DECV_CB_MAX; idx++) {
        memset(&dev->attr.callback_nodes[idx], 0, sizeof(aui_decv_callback_node));
    }
    
    // 75695(old) fix bug for PVR no first show callback, reg callback in SL before start
    for (idx = 0; idx < AUI_DECV_CB_MAX; idx++) {
        if (NULL != pst_attr_decv->callback_nodes[idx].callback) {
            cb_type = pst_attr_decv->callback_nodes[idx].type;
            
            if (cb_type < AUI_DECV_CB_MAX) {
                memcpy(&dev->attr.callback_nodes[cb_type], &pst_attr_decv->callback_nodes[idx], sizeof(struct aui_decv_callback_node));
                AUI_DBG("callback_nodes index %d, cb_type %d", idx, cb_type);
                aui_decv_callback_reg(dev, &dev->attr.callback_nodes[cb_type]);
            }
        }
    }

    *ppv_handle_decv = dev;
    dev->dev_priv_data.dev_idx = pst_attr_decv->uc_dev_idx;
    aui_dev_reg(AUI_MODULE_DECV, *ppv_handle_decv);

    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_close(aui_attr_decv *pst_attr_decv,  void **ppv_handle_decv)
{
	struct decv_device *dev;
	enum aui_en_decv_callback_type cb_type = AUI_DECV_CB_MAX;

	///fixme
	if ((NULL == ppv_handle_decv) && (!pst_attr_decv || pst_attr_decv)) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	dev = (struct decv_device *)(*ppv_handle_decv);
	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	
	for (cb_type = AUI_DECV_CB_FIRST_SHOWED; cb_type < AUI_DECV_CB_MAX; cb_type++) {
        /* 
         * Although this condition is rigorous, for AUI.But we can't do that.
         * Because we can't control the callback message from aliplatform level, 
         * if other modules register callback not through AUI (such as PVR).
         * So we must unreg all the callbacks unconditionally.
         */
		//if ((dev->callback_func[cb_type]) || (dev->attr.callback_nodes[cb_type].callback)) 
        {
			aui_decv_callback_unreg(dev, cb_type);
		}
	}

    if (alislvdec_set_variable_resolution(dev->handle, 0)) {
		aui_rtn(AUI_RTN_FAIL, "\nclose fail in disable variable resolution\n");
    }
	
	if (alislvdec_close(dev->handle)) {
		aui_rtn(AUI_RTN_FAIL, "\nclose fail in decv close\n");
	}

	if (alislavsync_close(dev->avsync_hdl)) {
		AUI_DBG("close avsync fail\n");
	}

	if (alisldis_close(dev->dis_hdl)) {
		aui_rtn(AUI_RTN_FAIL, "\nclose fail in decv close\n");
	}

	aui_dev_unreg(AUI_MODULE_DECV, *ppv_handle_decv);

	free(dev);
	*ppv_handle_decv = NULL;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_start(void *pv_handle_decv)
{
	struct decv_device *dev = pv_handle_decv;
	enum vdec_dis_layer layer = VDEC_DIS_MAIN_LAYER;
	enum aui_en_decv_callback_type cb_type = AUI_DECV_CB_MAX;
	aui_decv_format decv_format = AUI_DECV_FORMAT_INVALID;
	
	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	//all the settings should set after select format
	
	//need resigster first showed callback to get info for tvsys and stream progressive
	if (alislvdec_reg_callback(dev->handle, dev, SL_VDEC_CB_FIRST_SHOWED, aui_decv_common_callback, NULL)) {
		aui_rtn(AUI_RTN_FAIL, "alislvdec_reg_callback SL_VDEC_CB_FIRST_SHOWED fail!\n");
	}
    
	for (cb_type = AUI_DECV_CB_FIRST_SHOWED; cb_type < AUI_DECV_CB_MAX; cb_type++) {
		if (NULL != dev->attr.callback_nodes[cb_type].callback) {
			aui_decv_callback_reg(dev, &dev->attr.callback_nodes[cb_type]);
		}
	}	

	if (dev->attr.dis_layer == AUI_DIS_LAYER_VIDEO){
		layer = VDEC_DIS_MAIN_LAYER;
	} else if (dev->attr.dis_layer == AUI_DIS_LAYER_AUXP) {
		layer= VDEC_DIS_AUX_LAYER;
	}
	
	//set display layer
	alislvdec_set_display_layer(dev->handle, layer);

	aui_decv_decode_format_get(dev, &decv_format);
	if (AUI_DECV_FORMAT_MPEG == decv_format) {
		//VDEC_USER_DATA_ALL, get all user data from decv,and then filter user data for callback base on user data type 
		alislvdec_set_user_data_type(dev->handle, VDEC_USER_DATA_ALL);

		if (dev->userdata_type_any) {    
		dev->userdata_type = AUI_DECV_USER_DATA_ANY;
		}
	}

	if (alislvdec_start(dev->handle)) {
		aui_rtn(AUI_RTN_FAIL, "\nstart fail in decv start\n");
	}
	//vbv mode should set after start
    alislvdec_set_vbv_buf_mode(dev->handle, dev->attr.en_vbv_buf_mode);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_stop(void *pv_handle_decv)
{
	struct decv_device *dev = pv_handle_decv;
	struct vdec_info info;
	enum vdec_decoder_type type = VDEC_DECODER_MPEG;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	memset(&info, 0, sizeof(struct vdec_info));
	alislvdec_get_info(dev->handle, &info);
	alislvdec_get_decoder(dev->handle, &type);
	/*
	 * not support backup for AUX layer
	 * still: not close display when stop -> just close vpo to avoid mess screen
	 * black or others: close display
	 */
	if (info.layer == VDEC_DIS_AUX_LAYER) {//DIS_LAYER_AUXP
		//AUI_DBG("\n>>>> backup picture, en_chg_mode: %d\n", dev->attr.en_chg_mode);
		if (alislvdec_stop(dev->handle, 1, 0)) {
				aui_rtn(AUI_RTN_FAIL, "\nstop fail in decv stop\n");
		}
	} else { // DIS_LAYER_MAIN
		if (alislvdec_stop(dev->handle, 0, 0)) {
			aui_rtn(AUI_RTN_FAIL, "\nstop fail in decv stop\n");
		}
		
		if(NULL == dev->dis_hdl) 
		    alisldis_open(DIS_HD_DEV, &dev->dis_hdl);
		alisldis_free_backup_picture(dev->dis_hdl);
		//AUI_DBG("\n>>>> backup picture, en_chg_mode: %d\n", dev->attr.en_chg_mode);
	    if(AUI_DECV_CHG_STILL == dev->attr.en_chg_mode)
		    alisldis_backup_picture(dev->dis_hdl, DIS_CHG_STILL);
		else {
	    	alisldis_backup_picture(dev->dis_hdl, DIS_CHG_BLACK);
		}    
	}
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_pause(void *pv_handle_decv)
{
	struct decv_device *dev = pv_handle_decv;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (alislvdec_pause(dev->handle)) {
		aui_rtn(AUI_RTN_FAIL, "\nstart fail in decv start\n");
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_resume(void *pv_handle_decv)
{
	struct decv_device *dev = pv_handle_decv;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (alislvdec_resume(dev->handle)) {
		aui_rtn(AUI_RTN_FAIL, "\nstart fail in decv start\n");
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_de_init(aui_func_decv_init fn_decv_de_init)
{
	if (NULL == fn_decv_de_init) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	fn_decv_de_init();
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_decode_format_set(void *pv_handle_decv,  enum aui_decv_format en_format)
{
	struct decv_device *dev = pv_handle_decv;
	enum vdec_decoder_type type;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	switch(en_format)
	{
	case AUI_DECV_FORMAT_MPEG: type = VDEC_DECODER_MPEG; break;
	case AUI_DECV_FORMAT_AVC: type = VDEC_DECODER_AVC; break;
	case AUI_DECV_FORMAT_AVS: type = VDEC_DECODER_AVS; break;
	case AUI_DECV_FORMAT_HEVC: type = VDEC_DECODER_HEVC; break;
	default: AUI_ERR("set invalid video format\n"); return -1;break;
	}
	alislvdec_set_decoder(dev->handle, type, 0);
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_decode_format_get(void *pv_handle_decv,  enum aui_decv_format *en_format)
{
	struct decv_device *dev = pv_handle_decv;
	enum vdec_decoder_type type;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	alislvdec_get_decoder(dev->handle, &type);

	switch(type)
	{
	case VDEC_DECODER_MPEG: *en_format = AUI_DECV_FORMAT_MPEG; break;
	case VDEC_DECODER_AVC: *en_format = AUI_DECV_FORMAT_AVC; break;
	case VDEC_DECODER_AVS: *en_format = AUI_DECV_FORMAT_AVS; break;
	case VDEC_DECODER_HEVC: *en_format = AUI_DECV_FORMAT_HEVC; break;
	default: AUI_ERR("get invalid video format\n"); return -1;break;
	}
	
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_playmode_set(void *pv_handle_decv,
		enum aui_playback_speed speed, enum aui_playback_direction direction)
{
	struct decv_device *dev = pv_handle_decv;
	enum vdec_playback_speed sp;
	enum vdec_playback_direction dir;
	alisl_retcode ret;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	sp = convert_to_vdecsl_play_speed(speed);
	dir = convert_to_vdecsl_play_dir(direction);

	if (sp < 0 || dir < 0) {
			aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	ret = alislvdec_set_playmode(dev->handle, dir, speed);

	// TODO: why do we have to this for the callback to work ????
	struct vdec_pvr_config_param pvr_param;
	pvr_param.is_scrambled = 1;
	alislvdec_config_pvr(dev->handle, &pvr_param);

	return ret;
}

AUI_RTN_CODE aui_decv_trickmode_set(void *pv_handle_decv,
		enum aui_playback_speed speed,
		enum aui_playback_direction direction,
		int mode)
{
	struct decv_device *dev = pv_handle_decv;
	enum vdec_playback_speed sp;
	enum vdec_playback_direction dir;
	alisl_retcode ret;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	sp = convert_to_vdecsl_play_speed(speed);
	dir = convert_to_vdecsl_play_dir(direction);

	if (sp < 0 || dir < 0) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	ret = alislvdec_set_trickmode(dev->handle, dir, speed, mode);

	// TODO: why do we have to this for the callback to work ????
	struct vdec_pvr_config_param pvr_param;
	pvr_param.is_scrambled = 1;
	alislvdec_config_pvr(dev->handle, &pvr_param);

	return ret;
}

AUI_RTN_CODE aui_decv_trickinfo_get(void *pv_handle_decv, int *if_hw_err, int *if_frm_dis) {
	struct decv_device *dev = pv_handle_decv;
	alisl_retcode ret;
	
	if ((NULL == dev)||(NULL == if_hw_err)||(NULL == if_frm_dis)) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	ret = alislvdec_get_trick_info(dev->handle, if_hw_err, if_frm_dis);

	return ret;
}
AUI_RTN_CODE aui_decv_sync_mode(void *pv_handle_decv,  enum aui_stc_avsync_mode en_sync_mode)
{
	struct decv_device *dev = pv_handle_decv;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (en_sync_mode == AUI_STC_AVSYNC_PCR) {
		alislavsync_set_av_sync_mode(dev->avsync_hdl, AVSYNC_PCR);
	} else if (en_sync_mode == AUI_STC_AVSYNC_AUDIO) {
		alislavsync_set_av_sync_mode(dev->avsync_hdl, AVSYNC_AUDIO);
	} else {
		alislavsync_set_av_sync_mode(dev->avsync_hdl, AVSYNC_AV_FREERUN);
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_chg_mode_set(void *pv_handle_decv, enum aui_en_decv_chg_mode en_chg_mode)
{
	return aui_decv_set(pv_handle_decv, AUI_DECV_SET_CHGMODE, &en_chg_mode);
}

static int vdec_info_convert(struct vdec_info *src, aui_decv_status_info *dst)
{
    switch (src->state) {
        case VDEC_STATE_STARTED: {
            dst->u_cur_status = AUI_DECV_DECODING;
            break;
        }
        
        case VDEC_STATE_STOPPED: {
            dst->u_cur_status = AUI_DECV_STOPPED;
            break;
        }
        
        case VDEC_STATE_PAUSE: {
            dst->u_cur_status = AUI_DECV_PAUSED;
            break;
        }
        
        default: {
            dst->u_cur_status = AUI_DECV_ATTACHED;
            break;
        }
    }

    dst->u_first_pic_showed = src->first_pic_showed;
    dst->b_first_header_got = src->first_header_parsed;
    dst->frames_decoded = src->frames_decoded;
    dst->last_pts = src->frame_last_pts;
    dst->pic_width = src->pic_width;
    dst->pic_height = src->pic_height;
    /** statues and pointers for the vbv buffer,only for debug*/
    dst->status_flag = 0;
    dst->read_p_offset = 0;
    dst->write_p_offset = 0;
    dst->display_idx = src->frames_displayed;
    /** not used now */
    dst->use_sml_buf = 0;

    switch (src->out_mode) {
        case VDEC_OUT_FULLVIEW: {
            dst->output_mode = AUI_DECV_MP_MODE;
            break;
        }
        
        case VDEC_OUT_PREVIEW: {
            dst->output_mode = AUI_DECV_PREVIEW_MODE;
            break;
        }
        
        //only support mp mode and preview mode right now
        default: {
            dst->output_mode = AUI_DECV_MP_MODE;
            break;
        }
    }

    /** vbv valid size,may only for debug*/
    dst->valid_size = src->buffer_used;
    /** not used now */
    dst->mpeg_format = 0;

    switch (src->aspect) {
        case VDEC_ASPECT_FORBIDDEN: {
            dst->aspect_ratio = AUI_DECV_DAR_FORBIDDEN;
            break;
        }
        
        case VDEC_ASPECT_SAR: {
        dst->aspect_ratio = AUI_DECV_SAR;
        break;
        }
        
        case VDEC_ASPECT_4_3: {
            dst->aspect_ratio = AUI_DECV_DAR_4_3;
            break;
        }
        
        case VDEC_ASPECT_16_9: {
            dst->aspect_ratio = AUI_DECV_DAR_16_9;
            break;
        }
        
        case VDEC_ASPECT_DAR_221_1: {
            dst->aspect_ratio = AUI_DECV_DAR_221_1;
            break;
        }
        
        default: {
            dst->aspect_ratio = AUI_DECV_DAR_FORBIDDEN;
            break;
        }
    }

    dst->frame_rate = src->frame_rate;
    /** TODO */
    dst->bit_rate = 0;
    dst->hw_dec_error = src->hw_dec_error;
    /** always FALSE for live play */
    dst->display_frm = 0;
    /** not used for live play */
    dst->top_cnt = 0;
    /** not used for live play */
    dst->play_direction = convert_from_vdecsl_play_dir(src->playback_dir);
    /** not used for live play */
    dst->play_speed = convert_from_vdecsl_play_speed(src->playback_speed);
    /** not used for live play */
    dst->api_play_direction = convert_from_vdecsl_play_dir(src->api_playback_dir);
    /** not used for live play */
    dst->api_play_speed = convert_from_vdecsl_play_speed(src->api_playback_speed);
    /** whether we support this kind of format or resolution */
    dst->is_support = src->is_support;
    /** vbv size for video decoder */
    dst->vbv_size = src->buffer_size;
    /** dma channel used by video decoder,only for debug */
    dst->cur_dma_ch = 0;
    dst->sar_height = src->sar_height;
    dst->sar_width = src->sar_width;
    dst->active_format = src->active_format;
    
    if (src->layer == VDEC_DIS_AUX_LAYER) {
        dst->display_layer = AUI_DIS_LAYER_AUXP;    
    } else {
        dst->display_layer = AUI_DIS_LAYER_VIDEO;
    }
    
    /** frame count */
    dst->frames_decoded = src->frames_decoded;
    /** decode error count */
    dst->decode_error_cnt = src->decode_error_cnt;
    dst->rect_switch_done = src->rect_switch_done;
    return 0;
}

static void vdec_tvsys_convert(enum dis_tvsys *src, enum aui_dis_tvsys *dst)
{
	switch (*src) {
		case DIS_TVSYS_PAL:                {
			*dst = AUI_DIS_TVSYS_PAL;
			break;
		}
		case DIS_TVSYS_NTSC:               {
			*dst = AUI_DIS_TVSYS_NTSC;
			break;
		}
		case DIS_TVSYS_PAL_M:              {
			*dst = AUI_DIS_TVSYS_PAL_M;
			break;
		}
		case DIS_TVSYS_PAL_N:              {
			*dst = AUI_DIS_TVSYS_PAL_N;
			break;
		}
		case DIS_TVSYS_PAL_60:             {
			*dst = AUI_DIS_TVSYS_PAL_60;
			break;
		}
		case DIS_TVSYS_NTSC_443:           {
			*dst = AUI_DIS_TVSYS_NTSC_443;
			break;
		}
		case DIS_TVSYS_MAC:                {
			*dst = AUI_DIS_TVSYS_MAC;
			break;
		}
		case DIS_TVSYS_LINE_720_25:        {
			*dst = AUI_DIS_TVSYS_LINE_720_50;
			break;
		}
		case DIS_TVSYS_LINE_720_30:        {
			*dst = AUI_DIS_TVSYS_LINE_720_60;
			break;
		}
		case DIS_TVSYS_LINE_1080_25:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_25;
			break;
		}
		case DIS_TVSYS_LINE_1080_30:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_30;
			break;
		}
		case DIS_TVSYS_LINE_1080_50:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_50;
			break;
		}
		case DIS_TVSYS_LINE_1080_60:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_60;
			break;
		}
		case DIS_TVSYS_LINE_1080_24:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_24;
			break;
		}
		case DIS_TVSYS_LINE_1152_ASS:      {
			*dst = AUI_DIS_TVSYS_LINE_1152_ASS;
			break;
		}
		case DIS_TVSYS_LINE_1080_ASS:      {
			*dst = AUI_DIS_TVSYS_LINE_1080_ASS;
			break;
		}
		case DIS_TVSYS_PAL_NC:             {
			*dst = AUI_DIS_TVSYS_PAL_NC;
			break;
		}
		case DIS_TVSYS_LINE_576P_50_VESA:
		case DIS_TVSYS_LINE_720P_60_VESA:
		case DIS_TVSYS_LINE_1080P_60_VESA:
		default:
			break;
	}
}

AUI_RTN_CODE aui_decv_buffer_reset(void *pv_handle_decv, unsigned int reset_buffer)
{
	struct decv_device *dev = pv_handle_decv;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	alislvdec_buffer_reset(dev->handle, reset_buffer);

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_get(aui_hdl pv_hdl_decv, enum aui_en_decv_item_get ul_item,
						  void *pv_param)
{
	struct decv_device *dev = pv_hdl_decv;
	struct vdec_info info;
    memset(&info, 0, sizeof(info));
	enum dis_tvsys tvsys;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	switch (ul_item) {
		case AUI_DECV_GET_INFO: {
			aui_decv_info *pst_info = (aui_decv_info *)pv_param;
			bool progressive = 0;
			if (NULL == pv_param) {
				aui_rtn(AUI_RTN_EINVAL, NULL);
			}
			alislvdec_get_info(dev->handle, &info);
			vdec_info_convert(&info, &pst_info->st_info);		
			alislvdec_get_tvsys(dev->handle, &tvsys);
			vdec_tvsys_convert(&tvsys, &pst_info->en_tv_mode);
			alislvdec_is_stream_progressive(dev->handle, &progressive);	
			pst_info->b_progressive = progressive?1:0;
			
			struct avsync_statistics_info statistics;
			memset(&statistics, 0, sizeof(statistics));
			if (0 != alislavsync_get_statistics(dev->avsync_hdl, &statistics)) {
			    aui_rtn(AUI_RTN_FAIL, "Decv get avsync statistics info fail!\n");
			}
			pst_info->st_info.drop_cnt = (unsigned int)statistics.total_v_drop_cnt;
			
			break;
		}	
		default:
			break;
	}

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_set(aui_hdl pv_hdl_decv, enum aui_en_decv_item_set ul_item,
						  void *pv_param)
{
    struct decv_device *dev = pv_hdl_decv;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    enum vdec_vbv_buf_mode vbv_buf_mode;
    enum aui_decv_frame_type aui_frame_type;
    enum vdec_frame_type frame_type;
    struct avsync_advance_param avsync_params;

    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    switch (ul_item) {
        case AUI_DECV_SET_CHGMODE:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            dev->attr.en_chg_mode = *((unsigned char *)(pv_param));
            break;
        
        case AUI_DECV_SET_VBV_BUF_MODE:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            } 
            
            dev->attr.en_vbv_buf_mode = *((unsigned char *)(pv_param));
            vbv_buf_mode = (enum vdec_vbv_buf_mode) (dev->attr.en_vbv_buf_mode);

            if (alislvdec_set_vbv_buf_mode(dev->handle, vbv_buf_mode)) {
                aui_rtn(AUI_RTN_FAIL, "\nset device fail in decv open\n");
                ret = AUI_RTN_FAIL;
            }
            break;
            
        case AUI_DECV_SET_REG_CALLBACK:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            ret = aui_decv_callback_reg(dev, (struct aui_decv_callback_node *)pv_param);
            break;

        case AUI_DECV_SET_UNREG_CALLBACK:
            ret = aui_decv_callback_unreg(dev, *(enum aui_en_decv_callback_type *)pv_param);
            break;

        case AUI_DECV_SET_COLOR_BAR:
            alislvdec_set_decoder(dev->handle, VDEC_DECODER_MPEG, 0);
            alislvdec_start(dev->handle);
            alislvdec_stop(dev->handle, 0, 0);
            ret = alislvdec_set_colorbar(dev->handle, 0);
            usleep(50*1000);//wait colorbar to show, or backup will fail

            if(NULL == dev->dis_hdl) 
                alisldis_open(DIS_HD_DEV, &dev->dis_hdl);
                
            alisldis_free_backup_picture(dev->dis_hdl);
            alisldis_backup_picture(dev->dis_hdl, DIS_CHG_STILL);   	
            break;
            
        case AUI_DECV_SET_VARIABLE_RESOLUTION:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            ret = alislvdec_set_variable_resolution(dev->handle, *(bool *)pv_param);
            break;
            
        case AUI_DECV_SET_FRAME_TYPE:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            aui_frame_type = *((enum aui_decv_frame_type *)(pv_param));
            frame_type = (enum vdec_frame_type)aui_frame_type;
            ret = alislvdec_set_frame_type(dev->handle, frame_type);
            break;
        case AUI_DECV_SET_FIRST_I_FREERUN:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            if (alislavsync_get_advance_params(dev->avsync_hdl, &avsync_params)) {
                aui_rtn(AUI_RTN_FAIL, "\nset device fail in decv open\n");
                ret = AUI_RTN_FAIL;
            } else {
                if (*(unsigned char *)(pv_param) == 1) {
                    avsync_params.disable_first_video_freerun = 0;
                } else {
                    avsync_params.disable_first_video_freerun = 1;
                }
                
                alislavsync_config_advance_params(dev->avsync_hdl, &avsync_params);
            }
            break;
            
        case AUI_DECV_SET_AVSYNC_SMOOTH:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            ret = alislavsync_set_video_smoothly_play(dev->avsync_hdl,
                AVSYNC_VIDEO_SMOOTH_LEVEL_1,*(unsigned char*)pv_param);
            break;
            
        case AUI_DECV_SET_CONTINUE_ON_DECODER_ERROR:
            if (NULL == pv_param) {
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            ret = alislvdec_set_continue_on_error(dev->handle, *(unsigned char*)pv_param);
            break;
            
        default:
            break;
    }

    return ret;
}

AUI_RTN_CODE aui_decv_inject_buf_get(void *pv_handle_decv,
									 unsigned long request_size,
									 void **inject_buf,
									 int *size_got)
{
	struct decv_device *dev = pv_handle_decv;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	dev->req_buf = malloc(request_size);
	if (NULL == dev->req_buf) {
		aui_rtn(AUI_RTN_FAIL, "\ninject buf fail\n");
	}

	dev->req_size = request_size;
	dev->update_size = 0;
	*inject_buf = dev->req_buf;
	*size_got = dev->req_size;

	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_inject_update(void *pv_handle_decv,  int size_got)
{
	struct decv_device *dev = pv_handle_decv;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	alislvdec_inject(dev->handle, dev->req_buf, size_got);
	dev->update_size += size_got;

	if (dev->update_size >= dev->req_size) {
		free(dev->req_buf);
		dev->req_buf = NULL;
	}

	return AUI_RTN_SUCCESS;
}
static void first_showed_callback(void *user_data, enum vdec_callback_type type,uint32_t param1, uint32_t param2)
{
	(void)user_data;
	(void)type;
	(void)param1;
	(void)param2;
	

	if (NULL != pfun_cur_first_showed) {
		pfun_cur_first_showed();
	}
}

AUI_RTN_CODE aui_decv_first_showed_set(void *pv_handle_decv,  aui_fun_first_showed first_showed)
{
	struct decv_device *dev = pv_handle_decv;
	alisl_handle dis_dev = NULL;

	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (AUI_DECV_CHG_STILL == dev->attr.en_chg_mode) {
		alisldis_open(DIS_HD_DEV, &dis_dev);
		alisldis_free_backup_picture(NULL);
		alisldis_close(dis_dev);
	}

	pfun_cur_first_showed = first_showed;

	alislvdec_reg_callback(dev->handle,
						   dev,
						   SL_VDEC_CB_FIRST_SHOWED,
						   first_showed_callback,
						   NULL);
	
	return AUI_RTN_SUCCESS;
}


/********* add new functions for sync with TDS ***************/
AUI_RTN_CODE aui_decv_copy_data(aui_hdl pv_handle_decv, unsigned int src_addr,unsigned int req_data, unsigned int *got_size)
{
	struct decv_device *dev = pv_handle_decv;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	if (alislvdec_inject(dev->handle, (unsigned int*)src_addr, req_data))
	{
        aui_rtn(AUI_RTN_FAIL, "\nset device fail in decv open\n");
        *got_size = 0;
        ret = AUI_RTN_FAIL;
    } else {
	    *got_size = req_data;
	}
	return ret;
}

AUI_RTN_CODE aui_decv_get_sl_handle(aui_hdl pv_hdl_decv, alisl_handle * pv_hdl_sl_vdec)
{
	struct decv_device *dev = pv_hdl_decv;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
	if (NULL == dev || NULL == pv_hdl_sl_vdec) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	*pv_hdl_sl_vdec = dev->handle;
	return ret;
}

AUI_RTN_CODE aui_decv_get_video_id(aui_hdl pv_hdl_decv, unsigned char *video_id)
{
	struct decv_device *dev = pv_hdl_decv;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
	if (NULL == dev || NULL == video_id) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	//AUI_DBG("aui_decv_get_video_id dev->attr.uc_dev_idx: %d\n", dev->attr.uc_dev_idx);
	*video_id = dev->attr.uc_dev_idx;
	return ret;
}

AUI_RTN_CODE aui_decv_get_current_opened_sl_handle(alisl_handle * pv_hdl_sl_vdec)
{
	unsigned int i = 0;
	if (NULL == pv_hdl_sl_vdec) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	for(i = 0; i < VDEC_NB_VIDEO; i++) {
		if (0 == alislvdec_get_decoder_by_id((enum vdec_video_id)i, pv_hdl_sl_vdec)) {
			return AUI_RTN_SUCCESS;
		}
	}
	//no video opened right now, please open it by yourself
	return AUI_RTN_FAIL;
}

AUI_RTN_CODE aui_decv_set_see_dmx_id(aui_hdl pv_hdl_decv, int see_dmx_id)
{
	struct decv_device *dev = pv_hdl_decv;
	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	dev->see_dmx_id = see_dmx_id;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_get_see_dmx_id(aui_hdl pv_hdl_decv, int *see_dmx_id)
{
	struct decv_device *dev = pv_hdl_decv;
	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	*see_dmx_id = dev->see_dmx_id;
	return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_common_callback_register (void *pv_handle_decv,
	int callback_type, common_callback com_cb)
{
	struct decv_device *dev = pv_handle_decv;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    struct aui_decv_callback_node callback_node;
    
	if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	dev->callback_func[callback_type] = com_cb;
	memset(&callback_node, 0, sizeof(callback_node));
	callback_node.type = callback_type;
	callback_node.callback = (aui_decv_callback)com_cb;//store for decv start to register callback
	if (com_cb) {
		ret = aui_decv_callback_reg(dev, &callback_node);
	} else {
		ret = aui_decv_callback_unreg(dev, callback_type);
	}
	return ret;
}

AUI_RTN_CODE aui_decv_io_flush (aui_hdl pv_handle_decv, unsigned int  enable)
{
    struct decv_device *dev = pv_handle_decv;
    vdec_show_last_frame_mode mode;
    
    if (NULL == dev) {
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

    if (0 == enable) {
        mode = VDEC_SHOW_LAST_FRAM_AFTER_NO_DATA;
    } else {
        mode = VDEC_SHOW_LAST_FRAM_IMMEDIATE;
    }

	if (alislvdec_show_last_frame(dev->handle, mode)) {
		aui_rtn(AUI_RTN_FAIL, "\nshow last frame failed\n");
	}

	return AUI_RTN_SUCCESS;
}    


AUI_RTN_CODE aui_decv_user_data_type_set (aui_hdl pv_handle_decv, aui_decv_user_data_type type)
{
    struct decv_device *dev = pv_handle_decv;  
    aui_decv_format decv_format = AUI_DECV_FORMAT_INVALID;
    //VDEC_USER_DATA_ALL, get all user data from decv,and then filter user data for callback base on user data type 
    vdec_user_data_type sl_type = VDEC_USER_DATA_ALL;  

    if (NULL == dev) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if (type ==  AUI_DECV_USER_DATA_ANY) {
        /*when playing a stream, userdata_type was setted as AUI_DECV_USER_DATA_ANY,
          and had found the first user data type it detect is a,
          and now if set AUI_DECV_USER_DATA_ANY again,the first user data type it will be detected again,
          it maybe is b,different from a */
        if (dev->userdata_type_any != 1) {      
            dev->userdata_type = type;
        }

        dev->userdata_type_any = 1;

    } else {
        dev->userdata_type_any = 0;
        dev->userdata_type = type;
    }

    aui_decv_decode_format_get(dev, &decv_format);
    if (AUI_DECV_FORMAT_MPEG == decv_format) {
        /*this vesion does not support VDEC_USER_DATA_ALL when play AVC video */
        alislvdec_set_user_data_type(dev->handle, sl_type);
    }
    return AUI_RTN_SUCCESS;
}
