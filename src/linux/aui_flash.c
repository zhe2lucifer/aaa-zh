/**@file
*	 @brief 	ALi UDI common function implement
*	 @author	demon.yan
*	 @date		2014-2-10
*	 @version	1.0.0
*	 @note		ali corp. all rights reserved. 2013-2999 copyright (C)
*				input file detail description here
*				input file detail description here
*				input file detail description here
*/

/******************************INCLUDE HEAD FILE*******************************/
#include "aui_common_priv.h"
#include "alipltfretcode.h"
#include <stdarg.h>
#include <stddef.h>
#include <pthread.h>

#include <aui_flash.h>
#include <alislstorage.h>

AUI_MODULE(FLASH)

/*********************************LOCAL MACRO**********************************/
static pthread_mutex_t flash_mutex;

#define FLASH_FUNCTION_ENTER AUI_INFO("enter\n");
#define FLASH_FUNCTION_LEAVE AUI_INFO("leave\n");

#define FLASH_LOCK pthread_mutex_lock(&flash_mutex)
#define FLASH_UNLOCK pthread_mutex_unlock(&flash_mutex)

typedef struct aui_st_flash_attr {
	/** equipment internal data structures */
	aui_dev_priv_data		dev_priv_data;
	/** flash device */
	alisl_handle dev;
    int open_cnt;
} aui_flash_attr;

#define MAX_MTD_NUM  32//MAX_MTD_NUM of Nor+Nand
aui_flash_attr *g_flash_handle[MAX_MTD_NUM] = {NULL};
static unsigned int g_flash_init = 0;

static bool check_flash_handle(aui_flash_attr *flash_handle)
{
	int i = 0;
	
	for (i = 0; i < MAX_MTD_NUM; i++) {
		if (flash_handle == g_flash_handle[i]) {
			AUI_DBG("valid flash handle at i:%d\n", i);
			return true;
		}
	}
	return false;
}

/**
*	 @brief 	   flash��ʼ������
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    p_call_back_init:��ʼ���ص�����
*	 @param[in]    pv_param:p_call_back_init ����
*	 @return	   ������
*	 @note		   flashģ����ʹ��֮ǰһ��Ҫ�ȵ��ô˺���������һ���ص�������
*				   aui_flash_init�����ô˻ص��������ص�����һ����ʵ��attach�����ݡ�
*
*/
AUI_RTN_CODE aui_flash_init(p_fun_cb p_call_back_init, void *pv_param)
{
	FLASH_FUNCTION_ENTER;

	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    if (g_flash_init == 1) {
        AUI_WARN("already init! If you want to init again, call de_init first.\n");
        return ret;
    }
	if (p_call_back_init) {
		p_call_back_init(pv_param);
	}

	int i = 0;
	for (i = 0; i < MAX_MTD_NUM; i++) {
		g_flash_handle[i] = MALLOC(sizeof(aui_flash_attr));
		if (g_flash_handle[i] == NULL) {
            AUI_ERR("malloc fail.\n");
			ret = AUI_RTN_ENOMEM;
			goto ERROR;
		}
		//AUI_DBG("[%s(%d)] g_flash_handle[%d]=0x%x\n", __FUNCTION__, __LINE__, i, (unsigned int)g_flash_handle[i]);
		MEMSET(g_flash_handle[i], 0, sizeof(aui_flash_attr));
	}

	pthread_mutex_init(&flash_mutex, NULL);

    g_flash_init = 1;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   flashȥ��ʼ������
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    p_call_back_init:��ʼ���ص�����
*	 @param[in]    pv_param:p_call_back_init ����
*	 @return	   ������
*	 @note		   ��aui_flash_init���ʹ�ã����˳�flashģ��ʱ���ã�
*				   ����һ���ص�������aui_flash_de_init�����ô˻ص�������
*
*/
AUI_RTN_CODE aui_flash_de_init(p_fun_cb p_call_back_init, void *pv_param)
{
	FLASH_FUNCTION_ENTER;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    if (g_flash_init == 0) {
        AUI_WARN("already de-init!\n");
        return ret;
    }
	if (p_call_back_init) {
		p_call_back_init(pv_param);
	}

	int i = 0;
	for (i = 0; i < MAX_MTD_NUM; i++) {
		FREE(g_flash_handle[i]);
		g_flash_handle[i] = NULL;
		AUI_DBG("free g_flash_handle[%d]=0x%x\n",  i, (unsigned int)g_flash_handle[i]);
	}

	pthread_mutex_destroy(&flash_mutex);
    g_flash_init = 0;
	FLASH_FUNCTION_LEAVE;
	return ret;
}

/**
*	 @brief 	   ��flash�豸
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    open_param:open����
*	 @param[out]   handle:���ص�flash���
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_open(aui_flash_open_param *open_param, aui_hdl *flash_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	//aui_flash_type type;
	alisl_handle dev;
	int flash_id = 0, erase_type = 0;;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (open_param == NULL) {
		//type = STO_TYPE_NAND;
		flash_id = 0;
	} else {
		//type = open_param->flash_type;
		flash_id = open_param->flash_id;
        erase_type = open_param->flash_erase_type;
	}

	if (alislsto_mtd_open(&dev, flash_id, O_RDWR, erase_type) != STO_ERR_NONE) {
        AUI_ERR("alislsto_mtd_open fail.\n");
        ret = AUI_RTN_FAIL;
        goto ERROR;
    }

    int i = 0;
    for (i = 0; i < MAX_MTD_NUM; i++) {
        if (g_flash_handle[i]->dev == dev) {			
            break;
        }
        if (g_flash_handle[i]->dev == NULL) {
            g_flash_handle[i]->dev = dev;
            g_flash_handle[i]->dev_priv_data.dev_idx = open_param->flash_id;
            //*flash_handle = g_flash_handle[i];
            break;
        } 
    }
    *flash_handle = g_flash_handle[i];
	if ((*flash_handle) == NULL) {
		ret = AUI_RTN_EINVAL;
		AUI_ERR("fail\n");
		goto ERROR;
	}
	AUI_DBG("flash_handle=0x%x, open_cnt: %d\n", 
     (unsigned int)(*flash_handle), g_flash_handle[i]->open_cnt);
    g_flash_handle[i]->open_cnt++;
    if (1 == g_flash_handle[i]->open_cnt) {
        // just registor one time
        AUI_DBG("registor.\n");
        aui_dev_reg(AUI_MODULE_FLASH, *flash_handle);
    }

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   �ر�FLASH�豸
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:��aui_flash_open���صľ��
*	 @param[out]   NULL
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_close(aui_hdl flash_handle)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;
	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("invalid handle\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	if (alislsto_mtd_close(handle->dev) != STO_ERR_NONE) {
        AUI_ERR("alislsto_mtd_close fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

    AUI_DBG("flash_handle=0x%x, open_cnt: %d\n", 
     (unsigned int)(handle), handle->open_cnt);
    handle->open_cnt--;
    if (0 == handle->open_cnt) {
        AUI_DBG("un-registor.\n");
	    aui_dev_unreg(AUI_MODULE_FLASH, handle);    
    	int i = 0;
    	for (i = 0; i < MAX_MTD_NUM; i++) {
    		if (g_flash_handle[i] == flash_handle) {
    			MEMSET(g_flash_handle[i], 0, sizeof(aui_flash_attr));
    			break;
    		}
    	}
    }
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ��ȡFLASH��Ϣ
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash���
*	 @param[out]   p_flash_info:���ص�FLASH��Ϣ
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_info_get(aui_hdl flash_handle, aui_flash_info *p_flash_info)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	alislsto_param_t stoparam;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if ((check_flash_handle(handle) == false)
		|| (p_flash_info == NULL)) {
		AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	if (alislsto_get_param(handle->dev, &stoparam) != STO_ERR_NONE) {
        AUI_ERR("alislsto_get_param fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	p_flash_info->block_cnt = stoparam.info->size / stoparam.info->erasesize;
	p_flash_info->block_size = stoparam.info->erasesize;
	p_flash_info->flash_size = stoparam.info->size;
	p_flash_info->star_address = 0x0;
    p_flash_info->write_size = stoparam.info->writesize;

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ����ָ��BLOCK
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    start_addr:Ҫ������FLASH��ַ����ַ������BLOCK����
*	 @param[in]    erase_size:Ҫ�����Ĵ�С,��С������BLOCK����
*	 @param[out]   NULL
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_erase(aui_hdl flash_handle,
		unsigned long start_addr,
		unsigned long erase_size)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;
	loff_t addr = 0;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	if (alislsto_get_phy_addr(handle->dev, start_addr, &addr)) {
        AUI_ERR("alislsto_get_phy_addr fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}
	if (alislsto_erase(handle->dev, addr, erase_size) != STO_ERR_NONE) {
        AUI_ERR("alislsto_erase fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ��FLASH�������
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    start_addr:��Ҫ������ʼ��ַ
*	 @param[in]    read_size:��Ҫ���Ĵ�С
*	 @param[out]   actual_size:ʵ�ʶ�ȡ�Ĵ�С
*	 @param[out]   buf:��Ŷ�ȡ������
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_read(aui_hdl flash_handle,
		unsigned long start_addr,
		unsigned long read_size,
		unsigned long *actual_size,
		unsigned char *buf)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;
	//loff_t offset = 0;
	alisl_retcode err = STO_ERR_NONE;
	alislsto_rw_param_t para;
	size_t size = 0;
	loff_t addr = 0;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("invalid handle.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	if (alislsto_get_phy_addr(handle->dev, start_addr, &addr)) {
        AUI_ERR("alislsto_get_phy_addr fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	para.handle = handle->dev;
	para.size = read_size;
	para.offset = addr;
	para.whenence = SEEK_SET;
	para.idx = -1;
	para.flag = true;
	err = alislsto_lock_read(para, buf, &size);
	*actual_size = size;
	if (err != STO_ERR_NONE || read_size != *actual_size) {
        AUI_ERR("alislsto_lock_read fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ��FLASH��д���ݵ�
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    start_addr:��Ҫд����ʼ��ַ
*	 @param[in]    write_size:��Ҫд�Ĵ�С
*	 @param[in]    buf:���Ҫд�������
*	 @param[out]   actual_size:ʵ��д��Ĵ�С
*	 @return	   ������
*	 @note		   ��дFLASH֮ǰ�����뱣֤Ҫд������֮ǰû�б�д����
*				   ����������Ѿ���д������д֮ǰ�����Ȳ�����
*
*/
AUI_RTN_CODE aui_flash_write(aui_hdl flash_handle,
		unsigned long start_addr,
		unsigned long write_size,
		unsigned long *actual_size,
		unsigned char *buf)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;
	//loff_t offset = 0;
	alisl_retcode err = STO_ERR_NONE;
	alislsto_rw_param_t para;
	size_t size = 0;
	loff_t addr = 0;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
		ret = AUI_RTN_EINVAL;
		AUI_ERR("invalid handle!!\n");
		goto ERROR;
	}

	if (alislsto_get_phy_addr(handle->dev, start_addr, &addr)) {
		ret = AUI_RTN_FAIL;
		AUI_ERR("alislsto_get_phy_addr fail!!\n");
		goto ERROR;
	}

	para.handle = handle->dev;
	para.size = write_size;
	para.offset = addr;
	para.whenence = SEEK_SET;
	para.idx = -1;
	para.flag = true;
	err = alislsto_lock_write(para, buf, &size);
	*actual_size = size;
	if (err != STO_ERR_NONE || write_size != *actual_size) {
		ret = AUI_RTN_FAIL;
		AUI_ERR("alislsto_lock_write fail!! return: %d\n,write_size: 0x%x, actual_size: 0x%x\n",
			   err, (unsigned int)write_size, (unsigned int)(*actual_size));
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ��FLASH��д���ݵ�,�����д�������д����������
*				   �������block�ȱ����������ٲ�������BLOCK�ѱ�����Ҫ
*				   д������ݼӽ�֮ǰ����������Ȼ���д���ݡ�
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    start_addr:��Ҫд����ʼ��ַ
*	 @param[in]    write_size:��Ҫд�Ĵ�С
*	 @param[in]    buf:���Ҫд�������
*	 @param[out]   actual_size:ʵ��д��Ĵ�С
*	 @return
*	 @note		   1.��������aui_flash_write����������aui_flash_auto_erase_write
*				   ���ȼ���д�����Ƿ�д�������д���ͻ��Բ�����д��
*				   2.���ڱ��������Զ�����������ϲ���ʹ�õ�ʱ��֤��д���
*				   block�����������������ġ�
*				   3.������Ч�ʱ�aui_flash_writeҪ��һЩ������ϲ㲻��Ҫ�Զ�
*				   �������ܣ���ֱ��ʹ��aui_flash_write��
*
*/
AUI_RTN_CODE aui_flash_auto_erase_write(aui_hdl flash_handle,
		unsigned long start_addr,
		unsigned long write_size,
		unsigned long *actual_size,
		unsigned char *buf)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;
    /*for bug #56342, MUST use loff_t instead of off_t.*/
	loff_t offset = 0; 
	//alisl_retcode err = STO_ERR_NONE;
	alislsto_rw_param_t para;
	//size_t size = 0;
	loff_t addr = 0;

	memset(&para, 0, sizeof(alislsto_rw_param_t));
	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("inavlid handle.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	if (alislsto_get_phy_addr(handle->dev, start_addr, &addr)) {
        AUI_ERR("alislsto_get_phy_addr fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	if (alislsto_get_offset(handle->dev, &offset) != STO_ERR_NONE) {
        AUI_ERR("alislsto_get_offset fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}
    
	alislsto_lseek(handle->dev, addr, SEEK_SET);

    /* 
      for #CSTM_Issue_Internal #53746, the API "aui_flash_auto_erase_write" should:
          When we write data to a space which have been writed before, 
          this function can save the block data to a buffer,
          then write data to this buffer, and last it will write 
          the whole buffer to falsh block.
    */
	*actual_size = alislsto_write(handle->dev, buf, write_size);
    
	if (alislsto_set_offset(handle->dev, offset, true) != STO_ERR_NONE) {
        AUI_ERR("alislsto_set_offset fail.\n");
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}
    
	if (*actual_size != write_size) {
        AUI_ERR("*actual_size(0x%x) != write_size(0x%x).\n", *actual_size, write_size);
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ���¶�λFLASH��д�����ĵ�ַ
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    seek_offset:SEEK��ƫ����
*	 @param[in]    seek_type:SEEK������
*	 @param[out]   cur_addr:SEEK��FLASH�Ĳ�����ַ
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_seek(aui_hdl flash_handle,
		long seek_offset,
		aui_flash_seek_type seek_type,
		unsigned int *cur_addr)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;
	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	//*cur_addr = alislsto_lseek(handle->dev, seek_offset, seek_type);
	*cur_addr = alislsto_lseek_logic(handle->dev, seek_offset, seek_type);
	if (*cur_addr == (unsigned int)-1) {
        AUI_ERR("alislsto_lseek_logic.\n");
		ret = AUI_RTN_EINVAL;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ��FLASH��ǰ��ַ��д�����ݣ�д���FLASH�Ķ�д������ַ
*				   ����ϱ��ε�ʵ��д�볤��
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    write_len:��Ҫд��ĳ���
*	 @param[in]    buf:�����Ҫд�������
*	 @param[out]   actual_len:ʵ��д��ĳ���
*	 @return	   ������
*	 @note		   ��������aui_flash_write������:
*				   1.ʹ��aui_flash_write ��Ҫ���뽫Ҫд��ĵ�ַ��
*				   ��aui_flash_seek_write����FLASH��ǰ��ַд�����ݡ�
*				   2.aui_flash_seek_writeд�����ݺ�����FLASH�ĵ�ǰ��ַ��
*				   ��aui_flash_write���ᡣ��дFLASH֮ǰ�����뱣֤Ҫд������
*				   ֮ǰû�б�д��������������Ѿ���д������д֮ǰ�����Ȳ�����
*
*/
AUI_RTN_CODE aui_flash_seek_write(aui_hdl flash_handle,
		unsigned long write_len,
		unsigned char *buf,
		unsigned long *actual_len)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("invalid params.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	*actual_len = alislsto_write(handle->dev, buf, write_len);

	if (*actual_len != write_len) {
        AUI_ERR("*actual_len(0x%x) != write_len(0x%x).\n", *actual_len, write_len);
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ��FLASH������ݣ������Ժ�FLASH�Ķ�д������ַ����ϱ��ε�ʵ�ʶ�������
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle:flash ���
*	 @param[in]    read_len:��Ҫ��ȡ�ĳ���
*	 @param[out]   buf:��Ŷ�ȡ������
*	 @param[out]   actual_len:ʵ�ʴ�FLASH����������ݳ���
*	 @return	   ������
*	 @note		   ��������aui_flash_read������:
*				   1.ʹ��aui_flash_read��Ҫ���뽫Ҫ��ȡ���ݵĵ�ַ��
*				   ��aui_flash_seek_read�Ǵ�FLASH��ǰ��ַ��ʼ��ȡ���ݡ�
*				   2.aui_flash_seek_read ��ȡ���ݺ�����FLASH�ĵ�ǰ��ַ��
*				   ��aui_flash_read���ᡣ
*
*/
AUI_RTN_CODE aui_flash_seek_read(aui_hdl flash_handle,
		unsigned long read_len,
		unsigned char *buf,
		unsigned long *actual_len)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
        AUI_ERR("invalid handle.\n");
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	*actual_len = alislsto_read(handle->dev, buf, read_len);

	if (*actual_len != read_len) {
        AUI_ERR("*actual_len(0x%x) != read_len(0x%x).\n", *actual_len, read_len);
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

/**
*	 @brief 	   ����һЩFLASH�����һ�㲻��Ҫ�õ�
*	 @author	   demon.yan
*	 @date		   2014-2-10
*	 @param[in]    handle
*	 @param[in]    cmd:��Ҫ���õ�����
*	 @param[in]    param:�������
*	 @param[out]   NULL
*	 @return	   ������
*	 @note
*
*/
AUI_RTN_CODE aui_flash_set_cmd(aui_hdl flash_handle,
		unsigned long cmd,
		unsigned long param)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_attr *handle = (aui_flash_attr *)flash_handle;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if (check_flash_handle(handle) == false) {
		ret = AUI_RTN_EINVAL;
		goto ERROR;
	}

	if (alislsto_ioctl(handle->dev, cmd, &param) != STO_ERR_NONE) {
		ret = AUI_RTN_FAIL;
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return ret;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn((int)ret, NULL);
}

AUI_RTN_CODE aui_flash_is_lock (aui_hdl flash_handle, unsigned long address, unsigned long size, unsigned long *lock)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    FLASH_FUNCTION_ENTER;
    FLASH_LOCK;

    if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) {
        ret = AUI_RTN_EINVAL;
        AUI_ERR("input flash_handle invalid\n");
        goto ERROR;
    }

    aui_flash_attr *flash_attr = NULL;

    flash_attr = (aui_flash_attr *)flash_handle;    
    int sl_ret = alislsto_islock_nor(flash_attr->dev, address, size, lock);
    AUI_DBG("alislsto_islock_nor at: 0x%x, size: 0x%x, return: %d\n", 
            (unsigned int)address, (unsigned int)size,(unsigned int)sl_ret);
    if (sl_ret < 0) {
        ret = AUI_RTN_FAIL;
        AUI_ERR("sto_is_lock return fail at: 0x%x, size: 0x%x\n", 
                    (unsigned int)address, (unsigned int)size);
        goto ERROR;
    }

    FLASH_UNLOCK;
    FLASH_FUNCTION_LEAVE;
    return ret;

ERROR:
    FLASH_UNLOCK;
    FLASH_FUNCTION_LEAVE;
    aui_rtn(ret,NULL);
}

AUI_RTN_CODE aui_flash_lock (aui_hdl flash_handle, unsigned long address, unsigned long size)
{
    AUI_RTN_CODE ret = AUI_FLASH_SUCCESS;
    FLASH_FUNCTION_ENTER;
    FLASH_LOCK;

    if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) {
        ret = AUI_RTN_FAIL;
        AUI_ERR("input flash_handle invalid\n");
        goto ERROR;
    }

    aui_flash_attr *flash_attr = NULL;

    flash_attr = (aui_flash_attr *)flash_handle;
    ret = alislsto_lock_nor(flash_attr->dev, 1, address, size);
    AUI_DBG("alislsto_lock_nor at: 0x%x, size: 0x%x, return: %d\n", 
            (unsigned int)address, (unsigned int)size, (unsigned int)ret);
    if (0 != ret) {
        ret = AUI_FLASH_DRIVER_ERROR;
        AUI_ERR("lock return fail at: 0x%x, size: 0x%x\n", 
                    (unsigned int)address, (unsigned int)size);
        goto ERROR;
    } 

    FLASH_UNLOCK;
    FLASH_FUNCTION_LEAVE;
    return ret;

ERROR:
    FLASH_UNLOCK;
    FLASH_FUNCTION_LEAVE;
    aui_rtn(ret,NULL);
}

AUI_RTN_CODE aui_flash_unlock (aui_hdl flash_handle, unsigned long address, unsigned long size)
{
    AUI_RTN_CODE ret = AUI_FLASH_SUCCESS;
    FLASH_FUNCTION_ENTER;
    FLASH_LOCK;

    if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) {
        ret = AUI_RTN_FAIL;
        AUI_ERR("input flash_handle invalid\n");
        goto ERROR;
    }

    aui_flash_attr *flash_attr = NULL;
    flash_attr = (aui_flash_attr *)flash_handle;
    ret = alislsto_lock_nor(flash_attr->dev, 0, address, size);
    AUI_DBG("alislsto_lock_nor(unlock) at: 0x%x, size: 0x%x, return: %d\n", 
            (unsigned int)address, (unsigned int)size,(unsigned int)ret);
    if (0 != ret) {
        ret = AUI_FLASH_DRIVER_ERROR;
        AUI_ERR("unlock return fail at: 0x%x, size: 0x%x\n",
                   (unsigned int)address, (unsigned int)size);
        goto ERROR;
    } 

    FLASH_UNLOCK;
    FLASH_FUNCTION_LEAVE;
    return ret;

ERROR:
    FLASH_UNLOCK;
    FLASH_FUNCTION_LEAVE;
    aui_rtn(ret,NULL);
}
