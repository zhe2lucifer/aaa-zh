/**@file
*	 @brief 		AUI decv module interface implement
*	 @author		henry.xie
*	 @date			  2013-6-27
*	  @version		   1.0.0
*	 @note			  ali corp. all rights reserved. 2013~2999 copyright (C)
*/
/****************************INCLUDE HEAD FILE************************************/
#include "aui_common_priv.h"
#include <aui_decv.h>
#include <hld/decv/decv.h>
#include <hld/dmx/dmx.h>//to include <bus/dma/dma.h>
#include <hld/dis/vpo.h>
#include <aui_dis.h>

AUI_MODULE(DECV)
/****************************LOCAL MACRO******************************************/

/****************************LOCAL TYPE*******************************************/
#ifdef CHANCHG_VIDEOTYPE_SUPPORT
#ifndef VFB_SUPPORT
/* for VFB protected can't not backup in Main memory */
#define BACKUP_IN_AUI
#else
#define BACKUP_IN_SEE
#endif
#endif

#ifdef BACKUP_IN_AUI
//0x408000 for hevc supported, 0x308000 for hevc not supported
#define BACKUP_MEMORY_MIN_SIZE 0x408000

struct cc_backup_addr
{
    UINT8  *top_y_base[2];
    UINT8  *top_c_base[2];
    UINT8  *bot_y_base[2];  //add for H265
    UINT8  *bot_c_base[2];  //add for H265
    UINT8  *maf_buf_base[2];
    UINT8  need_free;
};

/**
Struct to specify the buffer used for store the last still frame when 
changing program by the function #aui_decv_still_frame_buffer_set.

@note This struct is used @a only in projects based on <b> TDS OS </b>.
*/
typedef struct aui_decv_still_frame_buffer {

    /**
    Member to specify the pointer to the memory to store the frame
        - NULL = malloc memory automatically by DECV to store the frame
        - others = the pointer of the memory to store the frame
    */
    unsigned char *buffer;

    /**
    Member to specify the size of the memory to store the frame
    */
    unsigned int size;

} aui_decv_still_frame_buffer;

enum BACKUP_PIC_RES
{
    CMD_NOT_IMPLEMENT,
    NO_PICTURE_SHOWED,
    MALLOC_MEMORY_FAIL,
    MALLOC_MEMORY_OK,
    BACKUP_PICTURE_FAIL,
    BACKUP_PICTURE_OK,
};

static struct cc_backup_addr cc_backup_info;
static aui_decv_still_frame_buffer g_still_buffer_info;
static OSAL_ID g_mutex_backup = OSAL_INVALID_ID;
#endif

#define SEAMLESS_SWITCH_DISABLE  1
#define SEAMLESS_SWITCH_ENABLE   2

typedef struct priv_attr_decv {
	char variable_resolution; //0:disable; 1: enable; -1: default
	char continue_on_decode_error; //0: disable; 1: enable; -1:default
} priv_attr_decv, *p_priv_attr_decv;

extern enum aui_view_mode aui_dis_mode;
/** the last format set to decv module,
	as the default format of current video which will be decoded.
	It is used by dmx to get the current video format. 
*/	
aui_decv_format g_aui_decv_format = AUI_DECV_FORMAT_MPEG;
/****************************LOCAL VAR********************************************/
static OSAL_ID s_mod_mutex_id_decv=OSAL_INVALID_ID;
static OSAL_ID g_mutex_callback = OSAL_INVALID_ID;
static aui_fun_first_showed pfun_cur_first_showed = NULL;
static enum aui_en_decv_chg_mode cur_chg_mode = AUI_DECV_CHG_BLACK;
static unsigned long enable_drop_freerun_pic = FALSE;
// TDS vdec open /close is complicated, so AUI layer Just open when module init, and close when deinit.
static int s_last_open_dev_idx = -1;  
static int s_current_dev_idx = -1;

typedef struct
{
	aui_dev_priv_data dev_priv_data;
	struct vdec_device *pst_dev_decv;
	aui_attr_decv attr_decv;
	aui_decv_format format;
	/**either this callback_func 
	   or callback in aui_decv_callback_node which is in aui_attr_decv 
	   takes value of callback funtion */
	common_callback callback_func[AUI_DECV_CB_MAX+1];
	OSAL_ID dev_mutex_id;
	/* for show I frame it need to set dma to DMA_INVALID_CHA
	   because when play audio only after the system boot, dmx will set decv dma to 0  
	*/
	unsigned char invalid_dma;
	priv_attr_decv priv_attr;
}aui_handle_decv,*aui_p_handle_decv;
/****************************LOCAL FUNC DECLEAR***********************************/

#ifdef BACKUP_IN_SEE
void cc_backup_free(void)
{   
    struct vpo_device *vpo = NULL;

    vpo = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    if (NULL == vpo)
    {
        return;
    }

    vpo_ioctl(vpo, VPO_IO_FREE_BACKUP_PICTURE, 0);
}

BOOL cc_backup_picture(aui_decv_chg_mode en_chg_mode)
{
    struct vpo_device *vpo = NULL;
    struct vpo_io_get_picture_info info;
    vpo = (struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS);
    
    if (NULL == vpo) {
        return FALSE;
    }
    //free before backup, not really free the memory, just reset some status
    vpo_ioctl(vpo, VPO_IO_FREE_BACKUP_PICTURE, 0);
    MEMSET(&info, 0, sizeof(info));

    if (en_chg_mode == AUI_DECV_CHG_BLACK) {
        info.reserved[0] = TRUE;//TRUE: backup black frame, FLASE: backup still frame
    } 

    if (RET_SUCCESS != vpo_ioctl(vpo, VPO_IO_BACKUP_CURRENT_PICTURE, (UINT32)&info)) {
        return FALSE;
    }
    
    return TRUE;	
}
#endif

#ifdef BACKUP_IN_AUI
//can be only configured before aui_decv_init, or there is memory leak
AUI_RTN_CODE aui_decv_set_backup_memory_info(unsigned char* buffer, unsigned int size)
{
    g_still_buffer_info.buffer = buffer;
    g_still_buffer_info.size = size;

    return AUI_RTN_SUCCESS;
}

static unsigned char *cc_backup_malloc(unsigned int size, unsigned char **buffer, unsigned int *remain_size)
{
    unsigned char *ptr = NULL;
    
    if (NULL == *buffer) {
        ptr = MALLOC(size);
        //AUI_PRINTF("%s: %p(%d)\n",  __func__, ptr, size);
    } else {
        if (*remain_size >= size) {
            ptr = *buffer;
            *buffer = *buffer + size;
            *remain_size -= size;
            //AUI_PRINTF("%s: %p(%d), %p(%d)\n", __func__, ptr, size, *buffer, *remain_size);
        } else {
                //AUI_PRINTF("%s malloc failed:size: %d, remain: %d\n", __func__, size, *remain_size);
        }
    }
    
    /* to avoid green screen 
    when stop after first shown callback and before DE show the first frame,
    there is a green screen before the first picture shown*/
    //if (ptr)
    //	MEMSET(ptr, 0, size);

    return ptr;
}

//free without lock
static void cc_backup_info_free(void)
{
    UINT32 i = 0;

    for(i=0; i<2; i++) {
        //AUI_PRINTF("cc_backup_info.need_free: %d\n", cc_backup_info.need_free);
        if (cc_backup_info.need_free) {
            if(cc_backup_info.top_y_base[i] != NULL)
                FREE(cc_backup_info.top_y_base[i]);
            if(cc_backup_info.top_c_base[i] != NULL)
                FREE(cc_backup_info.top_c_base[i]);
            if(cc_backup_info.bot_y_base[i] != NULL)
                FREE(cc_backup_info.bot_y_base[i]);
            if(cc_backup_info.bot_c_base[i] != NULL)
                FREE(cc_backup_info.bot_c_base[i]);	
            if(cc_backup_info.maf_buf_base[i] != NULL)
                FREE(cc_backup_info.maf_buf_base[i]);
        }
        
        cc_backup_info.top_y_base[i] = NULL;
        cc_backup_info.top_c_base[i] = NULL;
        cc_backup_info.bot_y_base[i] = NULL;
        cc_backup_info.bot_c_base[i] = NULL;
        cc_backup_info.maf_buf_base[i] = NULL;
    }
}

void cc_backup_free(void)
{ 
    osal_mutex_lock(g_mutex_backup, OSAL_WAIT_FOREVER_TIME);
    cc_backup_info_free();   
    osal_mutex_unlock(g_mutex_backup);
}

BOOL cc_backup_picture(aui_decv_chg_mode en_chg_mode)
{
    aui_decv_still_frame_buffer *buffer_info = &g_still_buffer_info;
    struct vpo_io_get_picture_info info_src[2], info_bak;
    enum BACKUP_PIC_RES res[2];
    //struct vdec_status_info cur_status;
    int i;
    unsigned char *backup_buffer = buffer_info->buffer;
    unsigned int buffer_size = buffer_info->size;
    struct vdec_fill_frm_info fill_color_src_info;
    struct ycb_cr_color color;

    osal_mutex_lock(g_mutex_backup, OSAL_WAIT_FOREVER_TIME);
    MEMSET(&fill_color_src_info, 0, sizeof(fill_color_src_info));
    MEMSET(&color, 0, sizeof(color));
    //MEMSET(&cur_status, 0, sizeof(struct vdec_status_info));
    res[0] = CMD_NOT_IMPLEMENT;
    res[1] = CMD_NOT_IMPLEMENT;
    //vdec_io_control(get_selected_decoder(), VDEC_IO_GET_STATUS, (UINT32)&cur_status);
   
    /* i=0: DE_N i=1: DE_O, only need to backup DE_N right now, the DE_O will return error status = 0*/
    for(i=0; i<2; i++) {
        MEMSET(&info_src[i], 0, sizeof(struct vpo_io_get_picture_info));
        info_src[i].de_index = i;
        info_src[i].sw_hw = 0;
        vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_GET_CURRENT_DISPLAY_INFO, (unsigned int)&info_src[i]);

        if(info_src[i].status == 0) {/* control command is not implemented */
            res[i] = CMD_NOT_IMPLEMENT;
        } else if(info_src[i].status == 1) {/* control command is implemented but no picture displayed */
            res[i] = NO_PICTURE_SHOWED;
        } else if(info_src[i].status == 2) { /* control command is implemented and picture info is got */
            if((!info_src[i].y_buf_size) || (!info_src[i].c_buf_size)) { /* information is not correct */
                res[i] = NO_PICTURE_SHOWED;
                continue;
            }
            /* information is correct, and start to malloc memory */
            res[i] = MALLOC_MEMORY_FAIL;
            
            do {              
                if (backup_buffer == NULL) {
                    cc_backup_info.need_free = 1;
                } else {
                    cc_backup_info.need_free = 0;
                }
                
                if((cc_backup_info.top_y_base[i] != NULL) 
                    || (cc_backup_info.top_c_base[i] != NULL) \
                    || (cc_backup_info.maf_buf_base[i] != NULL)) {
                    /* 
                    sometimes there is no picture showed, 
                    just display the last one without freeing and mallocing again,
                    the cc_backup_info will be freed when first showed
                    */
                    if (cc_backup_info.need_free == 0) {
                        /* free just reset the info of cc_backup_info,
                           will not reset the memory of current frame buffer
                        */
                        cc_backup_info_free();
                    } else {
                        /* may have problem when stop twice
                         * this following flow may have problem: 
                         * stop live -> play mp -> stop mp -> stop live again
                         */                        
                        res[i] = MALLOC_MEMORY_OK;
                        break;
                    }
                }
                
                //malloc memory to store the display info
                if(info_src[i].reserved[3] == MAPPING_MODE_H265_PROGRESSIVE) {                       
                    cc_backup_info.top_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x4000, &backup_buffer, &buffer_size);//address alignment 
                } else if(info_src[i].reserved[3] == MAPPING_MODE_H265_INTERLACE) {                        
                    cc_backup_info.top_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x4000, &backup_buffer, &buffer_size);//address alignment 
                    cc_backup_info.bot_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x4000, &backup_buffer, &buffer_size);//address alignment 
                    if (cc_backup_info.bot_y_base[i] == NULL) {
                        break;
                    }	
                } else { 
                    /* 
                    *	y,c buffer need to 4K alignment, or the backup picture look strange and the info get from 
                    *	DE is right
                    */
                    cc_backup_info.top_y_base[i] = cc_backup_malloc(info_src[i].y_buf_size + 0x1000, &backup_buffer, &buffer_size);//address alignment 
                }

                cc_backup_info.top_c_base[i] = cc_backup_malloc(info_src[i].c_buf_size + 0x1000, &backup_buffer, &buffer_size);//address alignment 

                if((cc_backup_info.top_y_base[i] == NULL)
                    || (cc_backup_info.top_c_base[i] == NULL)) {
                    break;
                }
                
                if(info_src[i].maf_buf_size) {
                    /* if maf_buf_base == NULL, we will not backup maf_buf, it just affects the 
                    * picture quality
                    */
                    cc_backup_info.maf_buf_base[i] = cc_backup_malloc(info_src[i].maf_buf_size + 0x1000, &backup_buffer, &buffer_size);
                }

                res[i] = MALLOC_MEMORY_OK;
            } while(0);

            if(res[i] == MALLOC_MEMORY_FAIL)
            break;
        }
    }


    if((res[0] == MALLOC_MEMORY_FAIL) || (res[1] == MALLOC_MEMORY_FAIL) || \
        ((res[0] == CMD_NOT_IMPLEMENT) && (res[1] == CMD_NOT_IMPLEMENT))) {
        cc_backup_info_free();
        /* backup picture fail or
        'get current display picture information' is not implemented */
        osal_mutex_unlock(g_mutex_backup);
        return FALSE;
    }
    
    for(i=0; i<2; i++) {
        if(res[i] == MALLOC_MEMORY_OK) {
            //store the last display picture info the the backup info
            MEMSET(&info_bak, 0, sizeof(struct vpo_io_get_picture_info));
            info_bak.de_index = i;

            if(info_src[i].reserved[3] == MAPPING_MODE_H265_PROGRESSIVE) {
                info_bak.top_y = ((UINT32)cc_backup_info.top_y_base[i]+ 0x3fff) & 0xFFFFC000;
                info_bak.top_c = info_bak.top_y;
                
                if (en_chg_mode == AUI_DECV_CHG_STILL) {
                    MEMCPY((char *)info_bak.top_y, (char *)info_src[i].top_y, info_src[i].y_buf_size);
                    osal_cache_flush((void *)info_bak.top_y, info_src[i].y_buf_size);
                }
            } else if(info_src[i].reserved[3] == MAPPING_MODE_H265_INTERLACE) {
                info_bak.top_y = ((UINT32)cc_backup_info.top_y_base[i]+ 0x3fff) & 0xFFFFC000;
                info_bak.top_c = info_bak.top_y;               
                info_bak.bot_y = ((UINT32)cc_backup_info.bot_y_base[i]+ 0x3fff) & 0xFFFFC000;
                info_bak.bot_c = info_bak.bot_y;
                
                if (en_chg_mode == AUI_DECV_CHG_STILL) {
                    MEMCPY((char *)info_bak.top_y, (char *)info_src[i].top_y, info_src[i].y_buf_size);
                    osal_cache_flush((void *)info_bak.top_y, info_src[i].y_buf_size);
                    MEMCPY((char *)info_bak.bot_y, (char *)info_src[i].bot_y, info_src[i].y_buf_size);
                    osal_cache_flush((void *)info_bak.bot_y, info_src[i].y_buf_size);
                }
            } else {
                info_bak.top_y = ((UINT32)cc_backup_info.top_y_base[i]+0xfff) & 0xFFFFF000;
                info_bak.top_c = ((UINT32)cc_backup_info.top_c_base[i]+0xfff) & 0xFFFFF000;
                //AUI_PRINTF("0x%x, 0x%x, y: %d, c: %d\n", info_bak.top_y, info_bak.top_c, info_src[i].y_buf_size, info_src[i].c_buf_size);

                if (en_chg_mode == AUI_DECV_CHG_STILL) {
                    MEMCPY((char *)info_bak.top_y, (char *)info_src[i].top_y, info_src[i].y_buf_size);
                    MEMCPY((char *)info_bak.top_c, (char *)info_src[i].top_c, info_src[i].c_buf_size);
                    osal_cache_flush((void *)info_bak.top_y, info_src[i].y_buf_size);
                    osal_cache_flush((void *)info_bak.top_c, info_src[i].c_buf_size);
                }
            }
        
            if (en_chg_mode == AUI_DECV_CHG_STILL) {
                if(cc_backup_info.maf_buf_base[i] != NULL) {
                    info_bak.maf_buffer = ((UINT32)cc_backup_info.maf_buf_base[i]+0xfff) & 0xFFFFF000;
                    //AUI_PRINTF("0x%x, %d\n", info_bak.maf_buffer, info_src[i].maf_buf_size);

                    /**
                    for AS project, the address of maf_buf_size is belong to see,
                    main CPU can't read and write
                    */
#ifndef HW_SECURE_ENABLE  
                    if(info_src[i].maf_buf_size) {
                        MEMCPY((char *)info_bak.maf_buffer, (char *)info_src[i].maf_buffer, info_src[i].maf_buf_size);
                        osal_cache_flush((void *)info_bak.maf_buffer, info_src[i].maf_buf_size);
                    }
#endif
                }	            
            }

            if (en_chg_mode == AUI_DECV_CHG_BLACK) {
			   
                //fill color for black mode
                fill_color_src_info.top_y = info_bak.top_y;
                fill_color_src_info.top_c = info_bak.top_c;
                fill_color_src_info.bot_y = info_bak.bot_y;
                fill_color_src_info.bot_c = info_bak.bot_c;
                fill_color_src_info.y_buf_size = info_src[i].y_buf_size;
                fill_color_src_info.c_buf_size = info_src[i].c_buf_size;
                fill_color_src_info.is_compressed = 0;
                fill_color_src_info.sample_bits_y = info_src[i].reserved[4];//sample bit;
                fill_color_src_info.sample_bits_c = info_src[i].reserved[4];//sample bit;
                fill_color_src_info.mapping_type = info_src[i].reserved[3];//mapping mode;
                color.u_y  = 16;
                color.u_cb = 128;
                color.u_cr = 128;
                vdec_fill_single_color(&fill_color_src_info, &color);                
            }	
            //AUI_PRINTF("mapping type: %d\n", info_src[i].reserved[3]);
            //set the backup info the VPO to display
            vpo_ioctl((struct vpo_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), VPO_IO_BACKUP_CURRENT_PICTURE, (unsigned int)&info_bak);
            if(info_bak.status == 0) {/* control command is not implemented */
                res[i] = BACKUP_PICTURE_FAIL;
                break;
            } else {/* control command is implemented */           
                res[i] = BACKUP_PICTURE_OK;
            }
        }
    }

    if((res[0] == BACKUP_PICTURE_FAIL) || (res[1] == BACKUP_PICTURE_FAIL)) {
        cc_backup_info_free();
        /* backup picture fail or
        'backup picture' is not implemented */
        osal_mutex_unlock(g_mutex_backup);
        return FALSE;
    }

    osal_mutex_unlock(g_mutex_backup);
    return TRUE;
}

#endif
/****************************MODULE DRV IMPLEMENT*************************************/

/**
*	 @brief
*	 @author
*	 @date			  2013-6-27
*	 @param[in] 	   param1 param1_description
*	 @param[out]	param2 param2_description
*	 @return
*	 @note
*
*/

AUI_RTN_CODE aui_decv_init(aui_func_decv_init fn_decv_init)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    s_mod_mutex_id_decv = osal_mutex_create();
    
    if(OSAL_INVALID_ID==s_mod_mutex_id_decv) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    g_mutex_callback = osal_mutex_create();
    if(OSAL_INVALID_ID == g_mutex_callback) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    if(NULL != fn_decv_init) {
        fn_decv_init();
    }

    if(0 !=vdec_open((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0))) {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    // Default open is mpg vdec
    s_last_open_dev_idx = 0;

#ifdef BACKUP_IN_AUI
    if ((g_still_buffer_info.buffer == NULL) && (g_still_buffer_info.size == 0)) {
        g_still_buffer_info.buffer = MALLOC(BACKUP_MEMORY_MIN_SIZE);
        g_still_buffer_info.size = BACKUP_MEMORY_MIN_SIZE;
    }
    g_mutex_backup = osal_mutex_create();
#endif   
    return rtn_code;
}


//new add begin ,for aui display 
static int g_first_i_showed = 0;

int aui_decv_first_i_showed()
{
	return g_first_i_showed;
}

//new add end 
static void first_showed_wrapper(UINT32 u_param1, UINT32 u_param2)
{
    aui_handle_decv *hdl_decv = NULL;
    int callback_type = AUI_DECV_CB_FIRST_SHOWED;
        
	g_first_i_showed = 1;

	AUI_INFO("para1:%x para2:%x is reserved!\n",u_param1,u_param2);
	
#ifdef CHANCHG_VIDEOTYPE_SUPPORT
	/*
	 * free the last backup info
	 */
	cc_backup_free();	
#endif

	if(pfun_cur_first_showed != NULL)
	{
		pfun_cur_first_showed();
		return;
	}
	
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    void *user_data = hdl_decv->attr_decv.callback_nodes[callback_type].puser_data;
    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](0,0);
    } 
    else if (hdl_decv->attr_decv.callback_nodes[callback_type].callback)
    {
        hdl_decv->attr_decv.callback_nodes[callback_type].callback(user_data,0,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_unregister_all(void *pv_handle_decv)
{
    int i = 0;
    int has_reg_cb = 0;
    aui_handle_decv *hdl_decv = (aui_handle_decv *) pv_handle_decv;
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    struct vdec_device *p_vdec_dev = ((aui_handle_decv *) (pv_handle_decv))->pst_dev_decv;
    struct vdec_io_reg_callback_para cc_cb_para;

    for (i = 0; i < AUI_DECV_CB_MAX; i++)
    {
        has_reg_cb = 0;

        if (((aui_handle_decv *) (pv_handle_decv))->callback_func[i])
        {
            has_reg_cb = 1;
        } 
        else if (p_attr->callback_nodes[i].callback)
        {
            has_reg_cb = 1;
        }    

        if (has_reg_cb == 0)
        {
            // No registered callback in this type
            continue;
        }

        MEMSET(&cc_cb_para, 0, sizeof(struct vdec_io_reg_callback_para));

        switch(i)
        {
            case AUI_DECV_CB_FIRST_SHOWED:
                cc_cb_para.e_cbtype= VDEC_CB_FIRST_SHOWED;
                break;
            case AUI_DECV_CB_MODE_SWITCH_OK:
                cc_cb_para.e_cbtype= VDEC_CB_MODE_SWITCH_OK;
                break;
            case AUI_DECV_CB_BACKWARD_RESTART_GOP:
                cc_cb_para.e_cbtype= VDEC_CB_BACKWARD_RESTART_GOP;
                break;
            case AUI_DECV_CB_FIRST_HEAD_PARSED:
                cc_cb_para.e_cbtype= VDEC_CB_FIRST_HEAD_PARSED;
                break;
            case AUI_DECV_CB_FIRST_I_FRAME_DECODED:
                cc_cb_para.e_cbtype= VDEC_CB_FIRST_I_DECODED;
                break;
            case AUI_DECV_CB_USER_DATA_PARSED:
                cc_cb_para.e_cbtype= VDEC_CB_MONITOR_USER_DATA_PARSED;
                break;
            case AUI_DECV_CB_INFO_CHANGED:
                cc_cb_para.e_cbtype= VDEC_CB_INFO_CHANGE;
                break;
            case AUI_DECV_CB_ERROR:
                cc_cb_para.e_cbtype= VDEC_CB_ERROR;
                break;
            case AUI_DECV_CB_MONITOR_START:
                cc_cb_para.e_cbtype= VDEC_CB_MONITOR_VDEC_START;
                break;
            case AUI_DECV_CB_MONITOR_STOP:
                cc_cb_para.e_cbtype= VDEC_CB_MONITOR_VDEC_STOP;
                break;
            case AUI_DECV_CB_MONITOR_STATE_CHANGE:
                cc_cb_para.e_cbtype= VDEC_CB_MONITOR_FRAME_VBV;
                break;
            case AUI_DECV_CB_STATE_CHANGE:
                cc_cb_para.e_cbtype= VDEC_CB_STATE_CHANGED;
                break;
            default:
                break;
        }

        // unregister callback
        vdec_io_control(p_vdec_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&cc_cb_para));
    } // for

}

static aui_playback_speed aui_decv_speed_hld_to_aui(unsigned char hdl_speed)
{
    aui_playback_speed aui_speed = AUI_PLAYBACK_SPEED_1;
    
    switch(hdl_speed)
    {
        case VDEC_SPEED_1_2:
            aui_speed = AUI_PLAYBACK_SPEED_1_2; 
            break;
        case VDEC_SPEED_1_4:
            aui_speed = AUI_PLAYBACK_SPEED_1_4;
            break;
        case VDEC_SPEED_1_8:
            aui_speed = AUI_PLAYBACK_SPEED_1_8;
            break;
        case VDEC_SPEED_STEP:
            aui_speed = AUI_PLAYBACK_SPEED_STEP;
            break;
        case VDEC_SPEED_1:
            aui_speed = AUI_PLAYBACK_SPEED_1;
            break;
        case VDEC_SPEED_2:
            aui_speed = AUI_PLAYBACK_SPEED_2;
            break;
        case VDEC_SPEED_4:
            aui_speed = AUI_PLAYBACK_SPEED_4;
            break;
        case VDEC_SPEED_8:
            aui_speed = AUI_PLAYBACK_SPEED_8;
            break;   
        default:
            aui_speed = -1;
            break;
    }
    return aui_speed;
}

static enum vdec_speed aui_decv_speed_aui_to_hld(aui_playback_speed aui_speed)
{
    aui_playback_speed hld_speed = VDEC_SPEED_1;
    
    switch(aui_speed)
    {
        case AUI_PLAYBACK_SPEED_1_2:
            hld_speed = VDEC_SPEED_1_2; 
            break;
        case AUI_PLAYBACK_SPEED_1_4:
            hld_speed = VDEC_SPEED_1_4;
            break;
        case AUI_PLAYBACK_SPEED_1_8:
            hld_speed = VDEC_SPEED_1_8;
            break;
        case AUI_PLAYBACK_SPEED_STEP:
            hld_speed = VDEC_SPEED_STEP;
            break;
        case AUI_PLAYBACK_SPEED_1:
            hld_speed = VDEC_SPEED_1;
            break;
        case AUI_PLAYBACK_SPEED_2:
            aui_speed = VDEC_SPEED_2;
            break;
        case AUI_PLAYBACK_SPEED_4:
            hld_speed = VDEC_SPEED_4;
            break;
        case AUI_PLAYBACK_SPEED_8:
            hld_speed = VDEC_SPEED_8;
            break;   
        default:
            hld_speed = VDEC_SPEED_1;
            AUI_WARN("wrong decv speed %d, set it to %d\n", aui_speed, hld_speed);
            break;
    }
    return hld_speed;
}

AUI_RTN_CODE aui_decv_open(aui_attr_decv *pst_attr_decv,void **ppv_handle_decv)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	OSAL_ID dev_mutex_id = 0;
	struct vdec_device *mpg, *avc;
	aui_attr_decv *p_attr = NULL;
	aui_handle_decv *p_hdl_decv = NULL;
	
	AUI_CREATE_DEV_MUTEX(s_mod_mutex_id_decv, dev_mutex_id, AUI_MODULE_DECV, AUI_RTN_EINVAL);

	if((NULL == pst_attr_decv) || (NULL == ppv_handle_decv))
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	p_hdl_decv = MALLOC(sizeof(aui_handle_decv));

	if(NULL == p_hdl_decv)
	{
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_ENOMEM, NULL);
	}

	p_attr = &(p_hdl_decv->attr_decv);

	MEMSET(p_hdl_decv, 0, sizeof(aui_handle_decv));
	MEMCPY(p_attr, pst_attr_decv, sizeof(aui_attr_decv));

	mpg = (struct vdec_device *)dev_get_by_id (HLD_DEV_TYPE_DECV, 0);
	avc = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
	if(pst_attr_decv->uc_dev_idx == 0)
	{
		p_hdl_decv->pst_dev_decv = mpg;		
	}
	else if(pst_attr_decv->uc_dev_idx == 1)
	{
		p_hdl_decv->pst_dev_decv = avc;
	}
	else
	{		
		FREE(p_hdl_decv);
		p_hdl_decv = NULL;
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\nget device fail in decv open\n");
	}
	// keep the last opened vdec ID
	s_last_open_dev_idx = pst_attr_decv->uc_dev_idx;
	g_aui_decv_format = p_hdl_decv->format;
	if(NULL == p_hdl_decv->pst_dev_decv)
	{	
		FREE(p_hdl_decv);
		p_hdl_decv = NULL;
		osal_mutex_unlock(dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\nget device fail in decv open\n");
	}

	p_hdl_decv->dev_mutex_id=dev_mutex_id;
	// for compatible issue, the old AUI DECV use uc_dev_idx as format
	p_hdl_decv->format = (aui_decv_format)pst_attr_decv->uc_dev_idx;
	p_hdl_decv->dev_priv_data.dev_idx = pst_attr_decv->uc_dev_idx;
	p_hdl_decv->priv_attr.continue_on_decode_error = -1;
	p_hdl_decv->priv_attr.variable_resolution = -1;
	aui_dev_reg(AUI_MODULE_DECV, p_hdl_decv);

	//new add end
	*ppv_handle_decv = p_hdl_decv;
	osal_mutex_unlock(dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_decv_close(aui_attr_decv *pst_attr_decv, void **ppv_handle_decv)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	if(pst_attr_decv != NULL)
	{
		AUI_INFO("pst_attr_decv:%x may not be used!\n");
	}

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if((NULL == *ppv_handle_decv)
		||(NULL==(*(aui_handle_decv **)ppv_handle_decv)->pst_dev_decv)
		||(OSAL_INVALID_ID==(*(aui_handle_decv **)ppv_handle_decv)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock((*(aui_handle_decv **)ppv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	// unregister all callback when close decv
	cb_unregister_all(*ppv_handle_decv);

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock((*(aui_handle_decv **)ppv_handle_decv)->dev_mutex_id);

	if(0!=osal_mutex_delete((*(aui_handle_decv **)ppv_handle_decv)->dev_mutex_id))
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, "\r\n_delete mutex err.");
	}

	aui_dev_unreg(AUI_MODULE_DECV, *ppv_handle_decv);
	FREE(*ppv_handle_decv);
	*ppv_handle_decv = NULL;
	osal_mutex_unlock(s_mod_mutex_id_decv);

	return rtn_code;
}

AUI_RTN_CODE aui_decv_start(void *pv_handle_decv)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	struct vdec_io_reg_callback_para tpara;
	int i;
	aui_handle_decv *dev = (aui_handle_decv *)pv_handle_decv;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(dev->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);


	struct vdec_device *p_vdec_dev = dev->pst_dev_decv;
	aui_attr_decv *pst_attr_decv = &dev->attr_decv;

	osal_mutex_unlock(dev->dev_mutex_id);//unlock for aui_decv_set
	//register the callbacks config by open
	for (i = 0; i < AUI_DECV_CB_MAX; i ++) {
		if (NULL != pst_attr_decv->callback_nodes[i].callback) {
			aui_decv_set(dev, AUI_DECV_SET_REG_CALLBACK, (void *)&pst_attr_decv->callback_nodes[i]);
		}
	}
	osal_mutex_lock(dev->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	//register first showed callback for free backup picture
	memset(&tpara,0,sizeof(struct vdec_io_reg_callback_para));
	tpara.e_cbtype = VDEC_CB_FIRST_SHOWED;
	tpara.p_cb = (vdec_cbfunc)first_showed_wrapper;
	vdec_io_control(p_vdec_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&tpara));

	/*
	App may set some attributes before setting video format, and these attributes
	are stored and would be configured again internally to driver after setting video format.
	*/
	if (1 == dev->priv_attr.variable_resolution)
		vdec_io_control(p_vdec_dev, VDEC_IO_SEAMLESS_SWITCH_ENABLE, 
		(AUI_DECV_FORMAT_AVC == dev->format)?SEAMLESS_SWITCH_ENABLE:TRUE);
	else if(0 == dev->priv_attr.variable_resolution)
		vdec_io_control(p_vdec_dev, VDEC_IO_SEAMLESS_SWITCH_ENABLE, 
		(AUI_DECV_FORMAT_AVC == dev->format)?SEAMLESS_SWITCH_DISABLE:FALSE);

	if (1 == dev->priv_attr.continue_on_decode_error)
		vdec_io_control(p_vdec_dev, VDEC_IO_CONTINUE_ON_ERROR, TRUE);
	else if(0 == dev->priv_attr.variable_resolution)
		vdec_io_control(p_vdec_dev, VDEC_IO_CONTINUE_ON_ERROR, FALSE);
		
	if(RET_SUCCESS != vdec_start(p_vdec_dev))
	{
		osal_mutex_unlock(dev->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\nstart fail in decv start\n");
	}
    s_current_dev_idx = dev->attr_decv.uc_dev_idx;
    g_aui_decv_format = dev->format;
	cur_chg_mode = dev->attr_decv.en_chg_mode;

	if(enable_drop_freerun_pic)
		vdec_io_control(p_vdec_dev,VDEC_IO_DROP_FREERUN_PIC, TRUE);
	
	osal_mutex_unlock(dev->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_decv_stop(void *pv_handle_decv)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    
    if(NULL == pv_handle_decv) {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    aui_handle_decv *p_decv = (aui_handle_decv *)(pv_handle_decv);
    struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

    p_decv->invalid_dma = 0;
    
#ifdef CHANCHG_VIDEOTYPE_SUPPORT//need backup still picture
    if((AUI_DECV_CHG_STILL == p_decv->attr_decv.en_chg_mode) 
        || (AUI_DECV_CHG_BLACK == p_decv->attr_decv.en_chg_mode)) {      
        if(RET_SUCCESS != vdec_stop(p_vdec_dev,0, 0)) {
            osal_mutex_unlock(p_decv->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL, "\nstop fail in decv stop\n");
        }
        
        if(FALSE == cc_backup_picture(p_decv->attr_decv.en_chg_mode)) {
            osal_mutex_unlock(p_decv->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL, "\nstop fail in backup\n");
        }        
    } else {
        if(RET_SUCCESS != vdec_stop(p_vdec_dev,0, 0)) {
            osal_mutex_unlock(p_decv->dev_mutex_id);
            aui_rtn(AUI_RTN_EINVAL, "\nstop fail in decv stop\n");
        }
    }
#else
    //can not support change channel mode, close vpo as default
    if(RET_SUCCESS != vdec_stop(p_vdec_dev,1, 1)) {
        osal_mutex_unlock(p_decv->dev_mutex_id);
        aui_rtn(AUI_RTN_EINVAL, "\nstop fail in decv stop\n");
    }
#endif
	p_decv->priv_attr.continue_on_decode_error = -1;
	p_decv->priv_attr.variable_resolution = -1;
    osal_mutex_unlock(p_decv->dev_mutex_id);
    //temp patch
    osal_task_sleep(40);
    return rtn_code;
}

AUI_RTN_CODE aui_decv_pause(void *pv_handle_decv)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;
	//Don't close VPO to avoid black screen when pausing
	if(RET_SUCCESS != vdec_stop(p_vdec_dev,0,0))
	{
		osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\npause fail in decv pause\n");
	}
	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_decv_resume(void *pv_handle_decv)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL,NULL);
	}

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	if(RET_SUCCESS != vdec_start(p_vdec_dev))
	{
		osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\nresume fail in decv pause\n");
	}
	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_decv_de_init(aui_func_decv_init fn_decv_de_init)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	struct vdec_device *vdec = NULL;

	if(E_OK!=osal_mutex_delete(s_mod_mutex_id_decv))
	{
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	if (s_last_open_dev_idx == 0)
	{
		vdec = (struct vdec_device *)dev_get_by_id (HLD_DEV_TYPE_DECV, 0);
	}
	else if (s_last_open_dev_idx == 1)
	{
		vdec = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
	}


	if((vdec != NULL) && (RET_SUCCESS != vdec_close(vdec)))
	{
		aui_rtn(AUI_RTN_EINVAL, "\nclose fail in decv close\n");
	}

	s_last_open_dev_idx = -1;

	if(NULL != fn_decv_de_init)
	{
		fn_decv_de_init();
	}
	return rtn_code;
}

AUI_RTN_CODE aui_decv_decode_format_set(void *pv_handle_decv, enum aui_decv_format en_format)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	unsigned char b_pre_enable = 0;
	aui_handle_decv *dev = (aui_handle_decv *)(pv_handle_decv);

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(dev->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);
	if(aui_dis_mode == AUI_VIEW_MODE_FULL)
	{
		b_pre_enable = 0;
	}
	else
	{
		b_pre_enable = 1;
	}

	switch(en_format) {
		case AUI_DECV_FORMAT_MPEG:
			video_decoder_select(MPEG2_DECODER, b_pre_enable);
			dev->pst_dev_decv = (struct vdec_device *)dev_get_by_name("DECV_S3601_0");
			break;	
		case AUI_DECV_FORMAT_AVC:
			video_decoder_select(H264_DECODER, b_pre_enable);
			dev->pst_dev_decv = (struct vdec_device *)dev_get_by_name("DECV_AVC_0");
			break;
		case AUI_DECV_FORMAT_AVS:
			video_decoder_select(AVS_DECODER, b_pre_enable);
			dev->pst_dev_decv = (struct vdec_device *)dev_get_by_name("DECV_AVS_0");
			break;
		case AUI_DECV_FORMAT_HEVC:
			video_decoder_select(H265_DECODER, b_pre_enable);
			dev->pst_dev_decv = (struct vdec_device *)dev_get_by_name("DECV_HEVC_0");
			break;
		default:
			rtn_code = AUI_RTN_EINVAL;
			break;
	}

	dev->format = en_format;

	g_aui_decv_format = dev->format;
	
	osal_mutex_unlock(dev->dev_mutex_id);
	
	return rtn_code;
}

AUI_RTN_CODE aui_decv_decode_format_get(void *pv_handle_decv, enum aui_decv_format *en_format)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	aui_handle_decv *handle = (aui_handle_decv *)pv_handle_decv;
	enum video_decoder_type video_type;

	osal_mutex_lock(s_mod_mutex_id_decv, OSAL_WAIT_FOREVER_TIME);
	if((NULL == pv_handle_decv) || (NULL == en_format))
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(handle->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	video_type = get_current_decoder();

	switch(video_type) {

    	case MPEG2_DECODER:
    		*en_format = AUI_DECV_FORMAT_MPEG;
    		break;
    	case H264_DECODER:
    		*en_format = AUI_DECV_FORMAT_AVC;
    		break;
    	case AVS_DECODER:
    		*en_format = AUI_DECV_FORMAT_AVS;
    		break;
		case H265_DECODER:
    		*en_format = AUI_DECV_FORMAT_HEVC;
    		break;
    	default:
    		*en_format = AUI_DECV_FORMAT_INVALID;
    		break;
	}
	
#if 0
	struct vdec_device *cur_dev = get_selected_decoder();
	if(cur_dev == (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0))
	{
		*en_format = AUI_DECV_FORMAT_MPEG;
	}
	else
	{
		*en_format = AUI_DECV_FORMAT_AVC;
	}
#endif

	osal_mutex_unlock(handle->dev_mutex_id);

	return rtn_code;
}

AUI_RTN_CODE aui_decv_playmode_set(void *pv_handle_decv,
		enum aui_playback_speed speed, enum aui_playback_direction direction)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
	enum vdec_speed hld_speed = VDEC_SPEED_1;

    if ((speed < AUI_PLAYBACK_SPEED_1_2) || (speed > AUI_PLAYBACK_SPEED_128))
    {
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

    hld_speed = aui_decv_speed_aui_to_hld(speed);
    rtn_code = vdec_playmode(p_vdec_dev, direction, hld_speed);

    AUI_PRINTF("%s,%d,p_vdec_dev:%x,direction:%x,speed(aui %x):%x,rtn_code:%x\n", 
        __FUNCTION__, __LINE__, p_vdec_dev, direction, speed, hld_speed, rtn_code);

	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_decv_sync_mode(void *pv_handle_decv, enum aui_stc_avsync_mode en_sync_mode)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	if(en_sync_mode == AUI_STC_AVSYNC_PCR)
	{
		vdec_sync_mode(p_vdec_dev, VDEC_SYNC_PTS ,VDEC_SYNC_I|VDEC_SYNC_P|VDEC_SYNC_B);
	}
	else
	{
		vdec_sync_mode(p_vdec_dev, VDEC_SYNC_FREERUN, VDEC_SYNC_I|VDEC_SYNC_P|VDEC_SYNC_B);
	}
	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_decv_chg_mode_set(void *pv_handle_decv, enum aui_en_decv_chg_mode en_chg_mode)
{
	return aui_decv_set(pv_handle_decv,AUI_DECV_SET_CHGMODE,&en_chg_mode);
}

static void vdec_tvsys_convert(enum tvsystem *src, aui_dis_tvsys *dst)
{
	switch (*src) {
		case PAL:                {
			*dst = AUI_DIS_TVSYS_PAL;
			break;
		}
		case NTSC:               {
			*dst = AUI_DIS_TVSYS_NTSC;
			break;
		}
		case PAL_M:              {
			*dst = AUI_DIS_TVSYS_PAL_M;
			break;
		}
		case PAL_N:              {
			*dst = AUI_DIS_TVSYS_PAL_N;
			break;
		}
		case PAL_60:             {
			*dst = AUI_DIS_TVSYS_PAL_60;
			break;
		}
		case NTSC_443:           {
			*dst = AUI_DIS_TVSYS_NTSC_443;
			break;
		}
		case MAC:                {
			*dst = AUI_DIS_TVSYS_MAC;
			break;
		}
		case LINE_720_25:        {
			*dst = AUI_DIS_TVSYS_LINE_720_50;
			break;
		}
		case LINE_720_30:        {
			*dst = AUI_DIS_TVSYS_LINE_720_60;
			break;
		}
		case LINE_1080_25:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_25;
			break;
		}
		case LINE_1080_30:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_30;
			break;
		}
		case LINE_1080_50:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_50;
			break;
		}
		case LINE_1080_60:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_60;
			break;
		}
		case LINE_1080_24:       {
			*dst = AUI_DIS_TVSYS_LINE_1080_24;
			break;
		}
		case LINE_1152_ASS:      {
			*dst = AUI_DIS_TVSYS_LINE_1152_ASS;
			break;
		}
		case LINE_1080_ASS:      {
			*dst = AUI_DIS_TVSYS_LINE_1080_ASS;
			break;
		}
		case PAL_NC:             {
			*dst = AUI_DIS_TVSYS_PAL_NC;
			break;
		}
		case LINE_576P_50_VESA:
		case LINE_720P_60_VESA:
		case LINE_1080P_60_VESA:
		default:
			break;
	}
}

AUI_RTN_CODE aui_decv_get(void *pv_hdl_decv,enum aui_en_decv_item_get ul_item, void *pv_param)
{
    struct vdec_status_info  vdec_stat;
    enum tvsystem tv_sys = TV_SYS_INVALID;

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    
    if(NULL == pv_hdl_decv) {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    struct vdec_device *p_vdec_dev = get_selected_decoder();
    aui_decv_info *pst_info = (aui_decv_info *)pv_param;
    switch(ul_item) {
        case AUI_DECV_GET_INFO:
            if(NULL == pv_param) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }

            MEMSET(&vdec_stat,0,sizeof(struct vdec_status_info));
            vdec_io_control(p_vdec_dev, VDEC_IO_GET_STATUS, (unsigned int)&vdec_stat);

            pst_info->st_info.top_cnt = vdec_stat.top_cnt;
            pst_info->st_info.aspect_ratio = vdec_stat.aspect_ratio;
            pst_info->st_info.b_first_header_got = vdec_stat.b_first_header_got;
            pst_info->st_info.api_play_direction =vdec_stat.api_play_direction;
            pst_info->st_info.api_play_speed = aui_decv_speed_hld_to_aui(vdec_stat.api_play_speed);
            pst_info->st_info.bit_rate = vdec_stat.bit_rate;
            pst_info->st_info.cur_dma_ch  = vdec_stat.cur_dma_ch;
            pst_info->st_info.display_frm = vdec_stat.display_frm;
            pst_info->st_info.display_idx = vdec_stat.display_idx;
            pst_info->st_info.frame_rate = vdec_stat.frame_rate;
            pst_info->st_info.hw_dec_error = vdec_stat.hw_dec_error;
            pst_info->st_info.is_support  = vdec_stat.is_support;
            pst_info->st_info.mpeg_format = vdec_stat.mpeg_format;
            pst_info->st_info.output_mode = vdec_stat.output_mode;
            pst_info->st_info.pic_height = vdec_stat.pic_height;
            pst_info->st_info.pic_width = vdec_stat.pic_width;
            pst_info->st_info.play_direction = vdec_stat.play_direction;
            pst_info->st_info.play_speed = aui_decv_speed_hld_to_aui(vdec_stat.play_speed);
            pst_info->st_info.read_p_offset = vdec_stat.read_p_offset;
            pst_info->st_info.status_flag = vdec_stat.status_flag;
            pst_info->st_info.use_sml_buf = vdec_stat.use_sml_buf;
            pst_info->st_info.u_cur_status = vdec_stat.u_cur_status;
            pst_info->st_info.u_first_pic_showed = vdec_stat.u_first_pic_showed;
            pst_info->st_info.valid_size = vdec_stat.valid_size;
            pst_info->st_info.vbv_size = vdec_stat.vbv_size;
            pst_info->st_info.write_p_offset = vdec_stat.write_p_offset;
            pst_info->st_info.sar_width = vdec_stat.sar_width;
            pst_info->st_info.sar_height = vdec_stat.sar_height;
            pst_info->st_info.active_format = vdec_stat.active_format;
            pst_info->b_progressive = vdec_stat.progressive?1:0;

            vdec_io_control(p_vdec_dev, VDEC_IO_GET_MODE, (unsigned int)&tv_sys);
            vdec_tvsys_convert(&tv_sys, &pst_info->en_tv_mode);
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);

            break;
        default:
            osal_mutex_unlock(s_mod_mutex_id_decv);// bug detective
            AUI_ERR("Invalid item\n");// bug detective
            return AUI_RTN_EINVAL;// bug detective
    }
    return AUI_RTN_SUCCESS;
}


AUI_RTN_CODE aui_decv_set(void *pv_hdl_decv, enum aui_en_decv_item_set ul_item, void *pv_param)
{
    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);

    if(NULL == pv_hdl_decv) {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }
    
    int is_free_run;
    aui_handle_decv *hdl_decv = (aui_handle_decv *)pv_hdl_decv;
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_hdl_decv))->pst_dev_decv;

    osal_mutex_lock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    struct aui_decv_callback_node *p_cb_node = (struct aui_decv_callback_node *)pv_param;

    switch(ul_item) {
        case AUI_DECV_SET_CHGMODE:
            if(pv_param == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            (((aui_handle_decv *)(pv_hdl_decv))->attr_decv).en_chg_mode = *((unsigned char *)(pv_param));
            cur_chg_mode = (((aui_handle_decv *)(pv_hdl_decv))->attr_decv).en_chg_mode;
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            break;
            
        case AUI_DECV_SYNC_DELAY_TIME:
            if(pv_param == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            
            vdec_io_control(get_selected_decoder(), VDEC_IO_SET_SYNC_DELAY, *((unsigned int *)(pv_param)));
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            break;
            
        case AUI_DECV_SET_COLOR_BAR:
            video_decoder_select(MPEG2_DECODER, 0);
            vdec_start((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0));
            vdec_stop((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0),0, 0);
            //colorbar use address 0(configured in driver automatically) for all cases including AS and retailer
            vdec_io_control((struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0),VDEC_IO_COLORBAR, 0);
#ifndef _GEN_CA_ENABLE_ //gen CA show logo by using see memory, can not backup in main
            osal_task_sleep(50);//wait colorbar to show, or backup will fail
            cc_backup_free();
            cc_backup_picture(AUI_DECV_CHG_STILL);
#endif
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            break;
            
        case AUI_DECV_SET_REG_CALLBACK:
            if(p_cb_node == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            
            memcpy(&(p_attr->callback_nodes[p_cb_node->type]),p_cb_node,sizeof(struct aui_decv_callback_node));

            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            aui_decv_common_callback_register(pv_hdl_decv,p_cb_node->type,(common_callback)p_cb_node->callback);
            osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
            /** callback_nodes takes the value of callback function, 
            the callback_func here need to be set to NULL*/
            ((aui_handle_decv *)pv_hdl_decv)->callback_func[p_cb_node->type] = NULL;
            osal_mutex_unlock(g_mutex_callback);
            break;

        case AUI_DECV_SET_FIRST_I_FREERUN:
            if(pv_param == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            
            if(p_vdec_dev == (struct vdec_device *)dev_get_by_id (HLD_DEV_TYPE_DECV, 0)) {
                is_free_run = *((unsigned int *)(pv_param));
                if(is_free_run) {
                    /* for mpeg drop freerun will be reset when call vdec_start, 
                    store the info and set again after stating*/                    
                    enable_drop_freerun_pic = 0;
                } else {
                    enable_drop_freerun_pic = 1;
                }    
                vdec_io_control(p_vdec_dev,VDEC_IO_DROP_FREERUN_PIC,enable_drop_freerun_pic); 
            } else {
                vdec_io_control(p_vdec_dev,VDEC_IO_FIRST_I_FREERUN,*((unsigned int *)(pv_param)));
            }
            
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            break;
            
        case AUI_DECV_SET_CONTINUE_ON_DECODER_ERROR:     
            if(pv_param == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            
            if (*((unsigned int *)(pv_param)) == 0)
                hdl_decv->priv_attr.continue_on_decode_error = 0;
            else
                hdl_decv->priv_attr.continue_on_decode_error = 1;

            //default value: mpeg2: 1, avc: 0
            vdec_io_control(p_vdec_dev, VDEC_IO_CONTINUE_ON_ERROR, *((unsigned char *)(pv_param)));
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            break;
            
        case AUI_DECV_SET_VARIABLE_RESOLUTION: {
            unsigned int seamless_param = 1;
            if(pv_param == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, NULL);
            }
            
            if (*((unsigned int *)(pv_param)) == 0) {
                hdl_decv->priv_attr.variable_resolution = 0;
                seamless_param = 0; 
                
                if (AUI_DECV_FORMAT_AVC == hdl_decv->format)
                    seamless_param = 1;//for AVC, hold or drop frames when the resolution changes
                } else {
                    hdl_decv->priv_attr.variable_resolution = 1;
                    seamless_param = 1;
                    if (AUI_DECV_FORMAT_AVC == hdl_decv->format)
                        seamless_param = 2;//for AVC, play smoothly when the resolution changes
                }
                
                vdec_io_control(p_vdec_dev, VDEC_IO_SEAMLESS_SWITCH_ENABLE, seamless_param);
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            }
            break;
            
        case AUI_DECV_SET_UNREG_CALLBACK: {
            enum aui_en_decv_callback_type type = AUI_DECV_CB_MAX;
            
            if(pv_param == NULL) {
                osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            
            type = *(enum aui_en_decv_callback_type *)pv_param;
            MEMSET(&(p_attr->callback_nodes[type]), 0, sizeof(struct aui_decv_callback_node));	            
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            aui_decv_common_callback_register(pv_hdl_decv, type, NULL);
            osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
            /** callback_nodes takes the value of callback function, 
            the callback_func here need to be set to NULL*/
            ((aui_handle_decv *)pv_hdl_decv)->callback_func[type] = NULL;
            osal_mutex_unlock(g_mutex_callback);   
            break;
        }
        
        case AUI_DECV_SET_FRAME_TYPE: { 
            unsigned char frame_type = 0;
            
            if(pv_param == NULL) {
                osal_mutex_unlock(hdl_decv->dev_mutex_id);
                aui_rtn(AUI_RTN_EINVAL, "invalid parameter\n");
            }
            
            if (*((enum aui_decv_frame_type *)(pv_param)) == AUI_DECV_FRAME_TYPE_ALL) {
                frame_type = 0;
            } else {
                frame_type = 1;
            }
            
            vdec_io_control(p_vdec_dev, VDEC_IO_SET_DEC_FRM_TYPE, frame_type);
            osal_mutex_unlock(hdl_decv->dev_mutex_id);
            break;
        }
        
        default:
            osal_mutex_unlock(((aui_handle_decv *)pv_hdl_decv)->dev_mutex_id);
            break;
    }
    return AUI_RTN_SUCCESS;
}

AUI_RTN_CODE aui_decv_inject_buf_get(void *pv_handle_decv, unsigned long request_size,void **inject_buf, int *size_got)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	if(0 != vdec_vbv_request((void *)p_vdec_dev, request_size, inject_buf, (UINT32 *)size_got,NULL))
	{
		osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
		aui_rtn(AUI_RTN_EINVAL, "\ninject buf fail\n");
	}

	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
	return rtn_code;

}

AUI_RTN_CODE aui_decv_inject_update(void *pv_handle_decv, int size_got)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	vdec_vbv_update(p_vdec_dev, size_got);

	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
	return rtn_code;
}

AUI_RTN_CODE aui_decv_copy_data(aui_hdl pv_handle_decv, unsigned int src_addr,unsigned int req_data, unsigned int *got_size)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if((NULL == pv_handle_decv)||(!got_size)||(!src_addr)||(!req_data))
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	aui_handle_decv *p_decv = (aui_handle_decv *)(pv_handle_decv);
	struct vdec_device *p_vdec_dev = p_decv->pst_dev_decv;

	osal_mutex_lock(p_decv->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

    osal_cache_flush((void*)src_addr, req_data);
    
    if (p_decv->invalid_dma == 0) {
    	/* #57277 special case for play audio after staring
    	 * can only do one time when show logo, because set DMA channel 
    	 * to DMA_INVALID_CHA will reset vdec status
    	 */
    	vdec_io_control(p_vdec_dev, VDEC_SET_DMA_CHANNEL, DMA_INVALID_CHA);
    	p_decv->invalid_dma = 1;
    } 
	vdec_copy_data((UINT32)p_vdec_dev,src_addr,req_data,(UINT32 *)got_size);

	osal_mutex_unlock(p_decv->dev_mutex_id);
	return rtn_code;
}


static void cb_vdec_info_change(UINT32 param1,UINT32 param2)
{
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_INFO_CHANGED;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    struct vdec_info_cb_param *pinfo = (struct vdec_info_cb_param *)param1;
    int status = 0;
    struct change_info info;
    MEMSET(&info,0,sizeof(struct change_info));

    if(pinfo->info_change_flags & VDEC_CHANGE_DIMENSIONS)
    {
        status = status|AUI_CHANGE_DIMENSIONS;
        //send the info change message one by one
        if(p_attr->callback_nodes[callback_type].callback)
        {           
            struct aui_decv_info_cb new_info;
            MEMSET(&new_info,0,sizeof(struct aui_decv_info_cb));
            new_info.flag = 0;
            new_info.pic_height = pinfo->pic_height;
            new_info.pic_width = pinfo->pic_width;
            osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
            p_attr->callback_nodes[callback_type].callback(user_data, (unsigned int)&new_info, 0);
            osal_mutex_unlock(g_mutex_callback);
        }
    }

    if(pinfo->info_change_flags & VDEC_CHANGE_FRAMERATE)
    {
        status = status|AUI_CHANGE_FRAME_RATE;
        if(p_attr->callback_nodes[callback_type].callback)
        {           
            struct aui_decv_info_cb new_info;
            MEMSET(&new_info,0,sizeof(struct aui_decv_info_cb));
            new_info.flag = 1;
            new_info.frame_rate = pinfo->frame_rate;
            osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
            p_attr->callback_nodes[callback_type].callback(user_data, (unsigned int)&new_info, 0);
            osal_mutex_unlock(g_mutex_callback);        }
    }

    if(pinfo->info_change_flags & VDEC_CHANGE_AFD)
    {
        status = status|AUI_CHANGE_AFD;
        if(p_attr->callback_nodes[callback_type].callback)
        {           
            struct aui_decv_info_cb new_info;
            MEMSET(&new_info,0,sizeof(struct aui_decv_info_cb));
            new_info.flag = 2;
            new_info.active_format = pinfo->active_format;
            osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
            p_attr->callback_nodes[callback_type].callback(user_data, (unsigned int)&new_info, 0);
            osal_mutex_unlock(g_mutex_callback);
        }
    }

    if(pinfo->info_change_flags & VDEC_CHANGE_SAR)
    {
        if(p_attr->callback_nodes[callback_type].callback)
        {           
            struct aui_decv_info_cb new_info;
            MEMSET(&new_info,0,sizeof(struct aui_decv_info_cb));
            new_info.flag = 3;
            new_info.sar_width = pinfo->sar_width;
            new_info.sar_height = pinfo->sar_height;
            osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
            p_attr->callback_nodes[callback_type].callback(user_data, (unsigned int)&new_info, 0);
            osal_mutex_unlock(g_mutex_callback);
        }
    }


    info.active_format = pinfo->active_format;
    info.frame_rate = pinfo->frame_rate;
    info.pic_height = pinfo->pic_height;
    info.pic_width = pinfo->pic_width;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);

    if(hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](status,(unsigned int)&info);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_error(UINT32 param1,UINT32 param2)
{
    (void) param2;
    UINT32 flag = param1;
    UINT32 status = 0;

    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_ERROR;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;


    if(flag & VDEC_ERROR_NODATA)
    {
        status = status|AUI_DECV_ERROR_NO_DATA;
    }

    if(flag & VDEC_ERROR_HARDWARE)
    {
        status = status|AUI_DECV_ERROR_HARDWARE;
    }

    if(flag & VDEC_ERROR_SYNC)
    {
        status = status|AUI_DECV_ERROR_SYNC;
    }

    if(flag & VDEC_ERROR_FRAMEDROP)
    {
        status = status|AUI_DECV_ERROR_FRAME_DROP;
    }

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);

    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](status,(unsigned int)NULL);
    } 
    else if (p_attr->callback_nodes[callback_type].callback)
    {
        p_attr->callback_nodes[callback_type].callback(user_data,status,(unsigned int)NULL);
    }
    
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_start(UINT32 param1,UINT32 param2)
{
    (void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_MONITOR_START;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    struct aui_decv_callback_node *p_cb_node = &p_attr->callback_nodes[callback_type];
    void *user_data = p_cb_node->puser_data;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](0,0);
    } 
    else if (p_attr->callback_nodes[callback_type].callback)
    {
        p_attr->callback_nodes[callback_type].callback(user_data,0,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_stop(UINT32 param1,UINT32 param2)
{
    (void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_MONITOR_STOP;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](0,0);
    } 
    else if (p_attr->callback_nodes[callback_type].callback)
    {
        p_attr->callback_nodes[callback_type].callback(user_data,0,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_decode(UINT32 param1,UINT32 param2)
{
    (void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_MONITOR_STATE_CHANGE;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](0,0);
    } 
    else if (p_attr->callback_nodes[callback_type].callback)
    {
        p_attr->callback_nodes[callback_type].callback(user_data,0,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_state(UINT32 param1,UINT32 param2)
{
    (void) param2;
    unsigned int flag = param1;
    unsigned int status = 0;

    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_STATE_CHANGE;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    if(flag&VDEC_STATE_NODATA)
    {
        status|=AUI_DECV_STATE_NODATA;
    }
    if(flag&VDEC_STATE_DECODING)
    {
        status|=AUI_DECV_STATE_DECODING;
    }
    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](status,0);
    } 
    else if (p_attr->callback_nodes[callback_type].callback)
    {
        p_attr->callback_nodes[callback_type].callback(user_data,status,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_user_data(UINT32 param1,UINT32 param2)
{
	(void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_USER_DATA_PARSED;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

	struct user_data_pram *data = (struct user_data_pram *)param1;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type]((unsigned int)data->user_data_size,(unsigned int)data->user_data);
    } 
    else if (p_attr->callback_nodes[callback_type].callback)
    {
        p_attr->callback_nodes[callback_type].callback(user_data,(unsigned int)data->user_data_size,(unsigned int)data->user_data);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_first_head_parsed(UINT32 param1,UINT32 param2)
{
	(void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_FIRST_HEAD_PARSED;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](0,0);
    } 
    else if (hdl_decv->attr_decv.callback_nodes[callback_type].callback)
    {
        hdl_decv->attr_decv.callback_nodes[callback_type].callback(user_data,0,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_first_I_frame_decoded(UINT32 param1,UINT32 param2)
{
	(void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_FIRST_I_FRAME_DECODED;

    if(hdl_decv == NULL)
    {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
    if (hdl_decv->callback_func[callback_type])
    {
        hdl_decv->callback_func[callback_type](0,0);
    } 
    else if (hdl_decv->attr_decv.callback_nodes[callback_type].callback)
    {
        hdl_decv->attr_decv.callback_nodes[callback_type].callback(user_data,0,0);
    }
    osal_mutex_unlock(g_mutex_callback);
}

static void cb_vdec_gop_decode(UINT32 param1,UINT32 param2)
{
    (void) param1;
    (void) param2;
    aui_handle_decv *hdl_decv = NULL;
    aui_find_dev_by_idx(AUI_MODULE_DECV,s_current_dev_idx,(aui_hdl *)&hdl_decv);
    int callback_type = AUI_DECV_CB_MONITOR_GOP;

    if(hdl_decv == NULL) {
        AUI_ERR("Get decv handle error!\n\n");
        return;
    }
    
    aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
    void *user_data = p_attr->callback_nodes[callback_type].puser_data;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);
	
    if (hdl_decv->callback_func[callback_type]) {
        hdl_decv->callback_func[callback_type](0,0);
    } else if (p_attr->callback_nodes[callback_type].callback) {
        p_attr->callback_nodes[callback_type].callback(user_data,0,0);
    }
	
    osal_mutex_unlock(g_mutex_callback);
}


AUI_RTN_CODE aui_decv_common_callback_register(void *pv_handle_decv,int callback_type,common_callback com_cb)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    struct vdec_io_reg_callback_para cc_cb_para;

    if((callback_type < 0) || (callback_type >= AUI_DECV_CB_MAX))
    {
		aui_rtn(AUI_RTN_EINVAL, NULL);
    }

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

    aui_handle_decv *hdl_decv = (aui_handle_decv *)pv_handle_decv;
    //aui_attr_decv *p_attr = &(hdl_decv->attr_decv);
	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

    osal_mutex_lock(g_mutex_callback,OSAL_WAIT_FOREVER_TIME);

	hdl_decv->callback_func[callback_type] = com_cb;

    osal_mutex_unlock(g_mutex_callback);
    osal_mutex_unlock(s_mod_mutex_id_decv);
    MEMSET(&cc_cb_para, 0, sizeof(struct vdec_io_reg_callback_para));
    switch(callback_type)
    {
    	case AUI_DECV_CB_FIRST_SHOWED:
    		cc_cb_para.e_cbtype= VDEC_CB_FIRST_SHOWED;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:first_showed_wrapper);
			break;
    	case AUI_DECV_CB_FIRST_HEAD_PARSED:
			cc_cb_para.e_cbtype= VDEC_CB_FIRST_HEAD_PARSED;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_first_head_parsed);
			break;
		case AUI_DECV_CB_FIRST_I_FRAME_DECODED:
			cc_cb_para.e_cbtype= VDEC_CB_FIRST_I_DECODED;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_first_I_frame_decoded);
			break; 
        case AUI_DECV_CB_INFO_CHANGED:
            cc_cb_para.e_cbtype= VDEC_CB_INFO_CHANGE;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_info_change);
            break;
        case AUI_DECV_CB_ERROR:
            cc_cb_para.e_cbtype= VDEC_CB_ERROR;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_error);
            break;
        case AUI_DECV_CB_MONITOR_START:
            cc_cb_para.e_cbtype= VDEC_CB_MONITOR_VDEC_START;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_start);
            break;
        case AUI_DECV_CB_MONITOR_STOP:
            cc_cb_para.e_cbtype= VDEC_CB_MONITOR_VDEC_STOP;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_stop);
            break;
        case AUI_DECV_CB_MONITOR_STATE_CHANGE:
            cc_cb_para.e_cbtype= VDEC_CB_MONITOR_FRAME_VBV;
            cc_cb_para.monitor_rate = 50;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_decode);
            break;
        case AUI_DECV_CB_STATE_CHANGE:
            cc_cb_para.e_cbtype= VDEC_CB_STATE_CHANGED;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_state);
            break;
		case AUI_DECV_CB_USER_DATA_PARSED:
            cc_cb_para.e_cbtype= VDEC_CB_MONITOR_USER_DATA_PARSED;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_user_data);
			break;
		case AUI_DECV_CB_MONITOR_GOP:
		    cc_cb_para.e_cbtype = VDEC_CB_MONITOR_GOP;
			cc_cb_para.p_cb = (com_cb==NULL? NULL:cb_vdec_gop_decode);
            break;
        default:
            AUI_ERR("Invalid callback type\n");// bug detective
            return AUI_RTN_EINVAL;// bug detective
    }
	vdec_io_control(p_vdec_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&cc_cb_para));
    return rtn_code;
}

AUI_RTN_CODE aui_decv_first_showed_set(void *pv_handle_decv, aui_fun_first_showed first_showed)
{
	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	
	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);

	pfun_cur_first_showed = first_showed;

	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
	return rtn_code;
}


AUI_RTN_CODE aui_decv_reg_frm_fin_callback(void *pv_handle_decv,vdec_cbfunc callback,void* param1,void* param2)
{
    (void) param1;
    (void) param2;
	struct vdec_io_reg_callback_para vpara;

	AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

	osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
	if(NULL == pv_handle_decv)
	{
		osal_mutex_unlock(s_mod_mutex_id_decv);
		aui_rtn(AUI_RTN_EINVAL, NULL);
	}
	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,OSAL_WAIT_FOREVER_TIME);
	osal_mutex_unlock(s_mod_mutex_id_decv);


	vpara.e_cbtype = VDEC_CB_DECODER_FINISH;
	vpara.p_cb = callback;
	//Now this io control not support param registration
	vdec_io_control(p_vdec_dev, VDEC_IO_REG_CALLBACK, (UINT32)(&vpara));

	osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
	return rtn_code;


}



AUI_RTN_CODE aui_decv_set_time_code(void *pv_handle_decv,
                               struct aui_decv_time_code_info aui_time_code_info)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    struct vpo_io_set_time_code time_code_info;
    MEMSET(&time_code_info, 0, sizeof(struct vpo_io_set_time_code));

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    if(NULL == pv_handle_decv)
    {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,
                                                       OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    time_code_info.time_code.hour = aui_time_code_info.time_code.hour;
    time_code_info.time_code.minute = aui_time_code_info.time_code.minute;
    time_code_info.time_code.second = aui_time_code_info.time_code.second;
    time_code_info.time_code.frame = aui_time_code_info.time_code.frame;
    time_code_info.mode = aui_time_code_info.mode;

    // set time code and mode
    rtn_code = vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),
                                        VPO_IO_SET_TIMECODE,  (UINT32)&time_code_info);

    osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_decv_get_time_code(void *pv_handle_decv,
                                             struct aui_decv_time_code *time_code)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    struct time_code_t cur_time_code;
    MEMSET(&cur_time_code, 0, sizeof(struct time_code_t));

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    if(NULL == pv_handle_decv)
    {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,
                                                       OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    // get time code
    rtn_code = vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),
                                          VPO_IO_GET_TIMECODE, (UINT32)&cur_time_code);
    if (0 == rtn_code)
    {
        time_code->hour = cur_time_code.hour;
        time_code->minute = cur_time_code.minute;
        time_code->second = cur_time_code.second;
        time_code->frame = cur_time_code.frame;
    }

    osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_decv_cancel_time_code(void *pv_handle_decv,
                                                       aui_decv_time_code_mode mode)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    time_code_mode time_code_mode = REPORT_TIME_CODE;

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    if(NULL == pv_handle_decv)
    {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,
                                                       OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    // cancel time code
    if (AUI_DECV_REPORT_TIME_CODE == mode)
    {
        time_code_mode = REPORT_TIME_CODE;
    }
    else if (AUI_DECV_FREEZE_FRAME_AT_TIME_CODE == mode)
    {
        time_code_mode = FREEZE_FRAME_AT_TIME_CODE;
    }
    rtn_code = vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),
                                        VPO_IO_CANCEL_TIMECODE, time_code_mode);

    osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_decv_reg_time_code_callback(void *pv_handle_decv, aui_time_code_cbfunc callback)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    struct vp_io_reg_callback_para tpara;
    MEMSET(&tpara, 0, sizeof(struct vp_io_reg_callback_para));

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    if(NULL == pv_handle_decv)
    {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,
                                                       OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

    tpara.e_cbtype = VPO_CB_REPORT_TIMECODE;
    tpara.p_cb = callback;

    // register callback function
    rtn_code = vpo_ioctl((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),
                                  VPO_IO_REG_CALLBACK, (unsigned long)(&tpara));

    osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
    return rtn_code;
}


AUI_RTN_CODE aui_decv_io_sameless_switch(aui_hdl pv_handle_decv,unsigned int  enable)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;

    struct vpo_io_set_time_code time_code_info;
    MEMSET(&time_code_info, 0, sizeof(struct vpo_io_set_time_code));

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    if(NULL == pv_handle_decv)
    {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,
                                                       OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	vdec_io_control(p_vdec_dev, VDEC_IO_SEAMLESS_SWITCH_ENABLE, enable);

    osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
    return rtn_code;
}


AUI_RTN_CODE aui_decv_io_flush(aui_hdl pv_handle_decv,unsigned int  enable)
{
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    
    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    if(NULL == pv_handle_decv)
    {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id,
                                                       OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);

	struct vdec_device *p_vdec_dev = ((aui_handle_decv *)(pv_handle_decv))->pst_dev_decv;

	vdec_io_control(p_vdec_dev, VDEC_IO_FLUSH, enable);

    osal_mutex_unlock(((aui_handle_decv *)pv_handle_decv)->dev_mutex_id);
    return rtn_code;
}

AUI_RTN_CODE aui_decv_user_data_type_set (aui_hdl pv_handle_decv, aui_decv_user_data_type type)
{
    aui_handle_decv *dev = (aui_handle_decv *)pv_handle_decv;
    AUI_RTN_CODE rtn_code = AUI_RTN_SUCCESS;
    unsigned int get_all = 0; 
    unsigned int get_info = 0;

    osal_mutex_lock(s_mod_mutex_id_decv,OSAL_WAIT_FOREVER_TIME);
    
    if(NULL == pv_handle_decv) {
        osal_mutex_unlock(s_mod_mutex_id_decv);
        aui_rtn(AUI_RTN_EINVAL, NULL);
    }

    osal_mutex_lock(dev->dev_mutex_id, OSAL_WAIT_FOREVER_TIME);
    osal_mutex_unlock(s_mod_mutex_id_decv);
        
    if (type ==  AUI_DECV_USER_DATA_ALL) {
        get_all = 1;
        get_info = 1;
    }

    vdec_io_control(dev->pst_dev_decv, VDEC_IO_GET_ALL_USER_DATA, get_all);
    vdec_io_control(dev->pst_dev_decv, VDEC_IO_GET_USER_DATA_INFO, get_info);
    osal_mutex_unlock(dev->dev_mutex_id);
    return rtn_code;
}
