/**  @file	  aui_flash.c
*	 @brief 	aui flash mode
*	 @author	 andy.yu
*	 @date		   2013-6-24
*	 @version	  1.0.0
*	 @note		   ali corp. all rights reserved. 2013-2999 copyright (C)
*/
#include "aui_common_priv.h"
#include <aui_common.h>

#include <aui_flash.h>
#include <hld/sto/sto.h>
#include <mediatypes.h>

#include <bus/nand/ali_nand.h>
static OSAL_ID flash_mutex = INVALID_ID;

AUI_MODULE(FLASH)
    
//#define ERRO_INFO	  1
//#define WARRMING_INFO 2

#define FLASH_FUNCTION_ENTER AUI_INFO("enter\n");
#define FLASH_FUNCTION_LEAVE AUI_INFO("leave\n");

#define FLASH_LOCK	 osal_mutex_lock(flash_mutex,OSAL_WAIT_FOREVER_TIME)
#define FLASH_UNLOCK osal_mutex_unlock(flash_mutex)

#define LBA_512B 9

typedef struct aui_st_flash_attr
{
	/**flash information*/
	aui_flash_info st_flash_info;
	/**flash device */
	//struct sto_device *flash_dev;
	aui_hdl *flash_dev;
	/**flash open count*/
	unsigned long open_cnt;
    /**flash type*/
    aui_flash_type flash_type;  
    /**flash id*/
	unsigned long flash_id;
    /**bad map of nand flash*/
    unsigned char *bad_map; 
    /**total bad blocks, for Nand flash*/
    unsigned long bad_cnt; 
    /**the valid size of nand flash, logic_size = flash_size - bad_block_size*/
    loff_t logic_size;  
    /**current offset on nand flash*/
    loff_t offset;  
    /**current partition base address*/
    loff_t part_base;
    /**current partition size*/
    loff_t part_size;
}aui_flash_attr;
#define MAX_MTD_NUM  32
static aui_flash_attr st_flash_attr_list[MAX_MTD_NUM];
static aui_flash_partition_table st_flash_part_table = {0};
static int g_part_info_init = 0;
aui_flash_attr st_flash_attr;

//check the flash handle
static BOOL check_flash_handle(aui_flash_attr *flash_handle)
{
	AUI_DBG(" input handl: 0x%x,\n", flash_handle);
    int i = 0;

    for (i = 0; i < MAX_MTD_NUM; i++) {
        if (flash_handle == &st_flash_attr_list[i]) {
            //AUI_PRINTF("[%s(%d)] i:%d\n", __FUNCTION__, __LINE__, i);
            return true;
        }
    }
    return FALSE;
}

//get st_flash_attr
static aui_flash_attr* get_flash_attr(int flash_id)
{
	return &st_flash_attr_list[flash_id];
}

//get flash open count
static unsigned long get_flash_open_cnt(int flash_id)
{
	return st_flash_attr_list[flash_id].open_cnt;

}


//incress flash open count ,use when open
static void incress_open_cnt(int flash_id)
{
	st_flash_attr_list[flash_id].open_cnt++;
}


//reduce flash open count ,use when close
static void reduce_open_cnt(int flash_id)
{
	if(st_flash_attr_list[flash_id].open_cnt > 0)
	{
		st_flash_attr_list[flash_id].open_cnt--;
	}
}

//set the flash device to st_flash_attr
static void set_flash_devic(int flash_id, aui_hdl * flash_devic)
{
	st_flash_attr_list[flash_id].flash_dev = flash_devic;

}
//reset the flash attr
static void rest_flash_attr(int flash_id)
{
	MEMSET(&st_flash_attr_list[flash_id],0,sizeof(aui_flash_attr));
}

static void set_flash_info(int flash_id,aui_flash_info *flash_info)
{
	MEMCPY(&st_flash_attr_list[flash_id].st_flash_info,flash_info,sizeof(aui_flash_info));
}

static void set_flash_type(int flash_id, aui_flash_type flash_type)
{
    st_flash_attr_list[flash_id].flash_id = flash_id;
    st_flash_attr_list[flash_id].flash_type = flash_type;
}
static void set_flash_part_base(int flash_id, loff_t part_base)
{
    st_flash_attr_list[flash_id].part_base= part_base;
}

static int bitarray_get_index(unsigned char *bitarray, size_t idx)
{
	if (bitarray == NULL) {
		return -1;
	}
	return bitarray[idx / 8] & (1 << (idx % 8));
}

static void bitarray_set_index(unsigned char *bitarray, size_t idx)
{
	if (bitarray == NULL) {
		return;
	}
	bitarray[idx / 8] |= (1 << (idx % 8));
}

static void create_bbt_map(aui_flash_attr *flash_hld)
{
    AUI_DBG("start to create bbt map.......\n");
	// bad block scan	
	unsigned long block_cnt = 0;
	unsigned long bad_block_cnt = 0;
    unsigned long flash_id = flash_hld->flash_id;
    UINT32 offset;     
    
    struct ali_nand_device *dev = (struct ali_nand_device *)(flash_hld->flash_dev);
    aui_flash_info info = flash_hld->st_flash_info;
    block_cnt = info.block_cnt;
    int bbt_len = (block_cnt / 8) + (block_cnt % 8 ? 1 : 0);
	flash_hld->bad_map = MALLOC(bbt_len);
    if (flash_hld->bad_map == NULL) {      
        AUI_PRINTF("[%s](%d) bad_map malloc fail\n", __FUNCTION__, __LINE__);      
        return;    
    }
    MEMSET(flash_hld->bad_map, 0, bbt_len);
    AUI_PRINTF("check from : 0x%x, block cnt: %ld\n", 
                (unsigned int)flash_hld->part_base,info.block_cnt);
	for (offset = flash_hld->part_base, block_cnt = 0;
			block_cnt < info.block_cnt;
			offset += info.block_size, block_cnt++) {
		if (0 != nf_block_is_bad(dev, ALI_NAND_PART_ALL, offset >> 9)) {
			AUI_WARN("block %ld is bad block\n", block_cnt);
            //nf_block_mark_bad(dev, ALI_NAND_PART_ALL, offset >> 9);
			bitarray_set_index(st_flash_attr_list[flash_id].bad_map, block_cnt);
			bad_block_cnt++;
		}
	}
    st_flash_attr_list[flash_id].logic_size = info.flash_size;
	if (bad_block_cnt) {
		st_flash_attr_list[flash_id].logic_size -= bad_block_cnt * info.block_size;

        //update the real flash size to flash_info
        info.flash_size = st_flash_attr_list[flash_id].logic_size;
        info.block_cnt = info.flash_size / info.block_size;
        set_flash_info(flash_id, &info);
	} 
    st_flash_attr_list[flash_id].bad_cnt = bad_block_cnt;
    AUI_INFO("has %ld bad blocks\n", bad_block_cnt);
}

static int get_phy_addr(aui_flash_attr *hld, UINT32 offset_logic, UINT32 *offset_phy)
{
    unsigned long block_all = 0;
	unsigned long block_logic = 0;
	unsigned long block_idx = 0;
	unsigned long cnt = 0;
	UINT32 addr = 0;

	aui_flash_attr *dev;
    dev = hld;
    aui_flash_info info = hld->st_flash_info;
	if ((offset_phy == NULL) 
        || (offset_logic > info.flash_size)) {
		AUI_ERR("Input error\n");
		return -1;
	}
	if ((!dev->bad_cnt) || (dev->flash_type != AUI_FLASH_TYPE_NAND)) {
		*offset_phy = offset_logic + dev->part_base;
		return 0;
	}

	block_all = info.block_cnt;
	block_logic = offset_logic / info.block_size;
	for (cnt = 0, block_idx = 0; cnt < block_all; cnt++) {
		if (!bitarray_get_index(dev->bad_map, cnt)) { // is bad block, index++
            //AUI_DBG("[%s]cnt: %d, block_idx: %d, block_logic: %d\n", __FUNCTION__, cnt, block_idx, block_logic);
            if (block_idx == block_logic) 
    			break;
    		
			block_idx++;
		} else {
            AUI_WARN("!!!!!!!! bad block: %d\n", cnt);
        }
	}
	if (block_idx == block_all) {
		AUI_ERR("Logic addr is out of rang\n");
		return -1;
	}
    AUI_INFO("cnt: %d, block_logic: %d, block_idx: %d\n", cnt,block_logic, block_idx);
	addr = cnt * info.block_size;
	addr += offset_logic % info.block_size;
	*offset_phy = addr + dev->part_base;
	return 0;
}

aui_flash_errno aui_nand_erase(aui_flash_attr *flash_attr, unsigned long address, unsigned long erase_size )
{
    aui_flash_errno ret = AUI_FLASH_SUCCESS;
    
    struct ali_nand_device *dev = (struct ali_nand_device *)(flash_attr->flash_dev);
    aui_flash_info info = flash_attr->st_flash_info;
    UINT32 offset = 0;
    UINT32 lba_offs = 0;
    UINT32 lba_size = info.block_size >> LBA_512B;
    unsigned long cnt = 0;

    if (address % info.block_size) {
        AUI_ERR("bad address, address not block aligned!\n");
        return AUI_FLASH_PARAM_INVALID;
    }
    
    get_phy_addr(flash_attr, address, &offset);
    AUI_DBG("<%s> address: 0x%x,  phy_addr: 0x%x, size: 0x%x\n", __FUNCTION__, address, offset, erase_size);
    if (erase_size % info.block_size) { // the erase unit is block
        erase_size += info.block_size;
    }

    UINT32 max_offset = info.flash_size + flash_attr->part_base;
    for (cnt = 0; cnt < erase_size; ) { 
        if ((cnt >= info.flash_size) 
            || (offset >= max_offset)) {
            /* To avoid death loop */
            AUI_ERR("erase out of flash_size! cnt: 0x%x, offset: 0x%x, max_offset: 0x%x\n", 
                    cnt, offset, max_offset);
            return AUI_FLASH_DRIVER_ERROR;
        }
        lba_offs = offset >> LBA_512B;
        if (0 != nf_block_is_bad(dev, ALI_NAND_PART_ALL, lba_offs)) {
            AUI_WARN(" !!!!!!! bad block at: 0x%x\n", offset);
            nf_block_mark_bad(dev, ALI_NAND_PART_ALL, lba_offs);
            offset += info.block_size;
            continue;
        }
        ret = nf_physical_erase(dev, ALI_NAND_PART_ALL, lba_offs, lba_size);
        if (ret){
            AUI_ERR(" nf_physical_erase fail!\n");
            nf_block_mark_bad(dev, ALI_NAND_PART_ALL, lba_offs);
            ret = AUI_FLASH_DRIVER_ERROR;
            return ret;
        }
        offset += info.block_size;
        cnt += info.block_size;
    }
    return ret;
    
}

aui_flash_errno aui_nand_read(aui_flash_attr *flash_attr, unsigned long address,unsigned long read_size,unsigned char *buf)
{
    aui_flash_errno ret = AUI_FLASH_SUCCESS;
    struct ali_nand_device *dev = (struct ali_nand_device *)flash_attr->flash_dev;
    aui_flash_info info = flash_attr->st_flash_info;
    unsigned long read_unit = info.block_size;
    UINT32 offset = 0;
    UINT32 lba_offs= 0;
    UINT32 lba_size = read_unit >> LBA_512B;
    unsigned long cnt = 0;
    if (address % info.block_size) {
        AUI_ERR("bad address, address not block aligned!\n");
        return AUI_FLASH_PARAM_INVALID;
    }
    
    get_phy_addr(flash_attr, address, &offset);
    AUI_DBG("address: 0x%x,  phy_addr: 0x%x, size: 0x%x\n", address, offset, read_size);
    UINT32 max_offset = info.flash_size + flash_attr->part_base;
    for (cnt = 0; cnt < read_size;) {
        if ((cnt >= info.flash_size) 
            || (offset >= max_offset)) {
            /* To avoid death loop */
            AUI_ERR("read out of flash_size! cnt: 0x%x, offset: 0x%x, max_offset: 0x%x\n", 
                    cnt, offset, max_offset);
            return AUI_FLASH_DRIVER_ERROR;
        }
        
        lba_offs = offset >> LBA_512B;
        if (0 != nf_block_is_bad(dev, ALI_NAND_PART_ALL, lba_offs)) {
            AUI_WARN("!!!!!!! bad block at: 0x%x\n", offset);
            //nf_block_mark_bad(dev, ALI_NAND_PART_ALL, lba_offs);
            offset += info.block_size;
            continue;
        }
        if (read_size - cnt < info.block_size) {// not a whole block size
            read_unit = read_size - cnt;
            lba_size = read_unit >> LBA_512B;
            AUI_DBG("adjust lba_size to : 0x%x\n", lba_size);
        }
        ret = nf_physical_read(dev, ALI_NAND_PART_ALL, lba_offs, lba_size, buf);
        if (ret){
            AUI_ERR("nf_physical_read fail!\n");
            ret = AUI_FLASH_DRIVER_ERROR;
            return ret;
        }
            
        offset += read_unit;
        cnt += read_unit;
        buf += read_unit;        
    }

    return cnt;        
}

aui_flash_errno aui_nand_write(aui_flash_attr *flash_attr, unsigned long address, unsigned long write_size, unsigned char *buf)
{
    aui_flash_errno ret = AUI_FLASH_SUCCESS;
    struct ali_nand_device *dev = (struct ali_nand_device *)flash_attr->flash_dev;
    aui_flash_info info = flash_attr->st_flash_info;
    unsigned long write_unit = info.block_size;
    UINT32 offset = 0;
    UINT32 lba_offs = 0;
    UINT32 lba_size = write_unit >> LBA_512B;
    unsigned long cnt = 0;

    if (address % info.block_size) {
        AUI_ERR("bad address, address not block aligned!\n");
        return AUI_FLASH_PARAM_INVALID;
    }
    
    get_phy_addr(flash_attr, address, &offset);
    AUI_DBG(" address: 0x%x,  phy_addr: 0x%x, size: 0x%x\n", address, offset, write_size);
    UINT32 max_offset = info.flash_size + flash_attr->part_base;
    for (cnt = 0; cnt < write_size;) {
        if ((cnt >= info.flash_size)
            || (offset >= max_offset)) {
            AUI_ERR("<%s> write out of flash_size! cnt: 0x%x, offset: 0x%x, max_offset: 0x%x\n", 
                        __FUNCTION__, cnt, offset, max_offset);
            return AUI_FLASH_DRIVER_ERROR;
        }
        
        lba_offs = offset >> LBA_512B;
        if (0 != nf_block_is_bad(dev, ALI_NAND_PART_ALL, lba_offs)) {
            AUI_ERR(" !!!!!!! bad block at: 0x%x\n", offset);
            //nf_block_mark_bad(dev, ALI_NAND_PART_ALL, lba_offs);
            offset += info.block_size;
            continue;
        }
        if (write_size - cnt < info.block_size) {// not a whole block size
            write_unit = (write_size - cnt);
            lba_size = write_unit >> LBA_512B;
        }
        ret = nf_physical_write((struct ali_nand_device *)(flash_attr->flash_dev), ALI_NAND_PART_ALL, lba_offs, lba_size, buf);
        if (ret){
            AUI_ERR(" nf_physical_read fail!\n");
            ret = AUI_FLASH_DRIVER_ERROR;
            return ret;
        }
            
        offset += write_unit;
        cnt += write_unit;
        buf += write_unit;        
    }

    return cnt;
}

loff_t aui_nand_lseek(aui_flash_attr *flash_attr, loff_t offset, aui_flash_seek_type seek_type)
{
    loff_t new_addr;
    int ret = 0;
    
    AUI_ERR(" seek type: %d, offset: %ld, flash_attr->offset: %ld\n", seek_type, offset, flash_attr->offset);
    aui_flash_info info = flash_attr->st_flash_info;
    switch (seek_type)
	{
		case SEEK_SET:

			/* Great than totol size, seek to end */
			if (offset >= info.flash_size)
			{
				flash_attr->offset = info.flash_size - 1;
			}
			/* Common seek */
			else if (offset >= 0)
			{
				flash_attr->offset = offset;
			}

			break;

		case SEEK_CUR:
			new_addr = flash_attr->offset + offset;

			/* Less than base address, seek to begin */
			if (new_addr < 0)
			{
				flash_attr->offset = 0;
			}
			/* Great than totol size, seek to end */
			else if (new_addr >= info.flash_size)
			{
				flash_attr->offset = info.flash_size - 1;
			}
			/* Common seek */
			else
			{
				flash_attr->offset = new_addr;
			}

			break;

		case SEEK_END:
			new_addr = info.flash_size + offset - 1;

			/* Less than base address, seek to begin */
			/*if (new_addr < 0)
			{
				flash_attr->offset = 0;
			}
			// Common seek 
			else */if (offset <= 0)
			{
				flash_attr->offset = new_addr;
			}

			break;

		default:
			AUI_ERR("please check your whenence parameter!\n");
            ret = -1;
            return ret;
	}

    AUI_ERR(" seek success, now offset is: 0x%x\n", flash_attr->offset);
    return flash_attr->offset;
}

/**
*	 @brief 		flash初始化函数
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   p_call_back_init:初始化回调函数
*	 @param[in]    pv_param:p_call_back_init 参数
*	 @return		 错误码
*	 @note		  flash模块在使用之前一定要先调用此函数，传入一个回调函数，aui_flash_init里会调用此回调函数，\n
回调函数一般是实现attach的内容。
*
*/
AUI_RTN_CODE aui_flash_init(p_fun_cb p_call_back_init,void *pv_param)
{
	AUI_RTN_CODE ret = SUCCESS;

	FLASH_FUNCTION_ENTER;

	if(p_call_back_init)
	{
		p_call_back_init(pv_param);
	}
	MEMSET(&st_flash_attr,0,sizeof(aui_flash_attr));
	flash_mutex = osal_mutex_create();
	if(INVALID_ID == flash_mutex)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		goto ERROR;
	}
    g_part_info_init = 0;
    MEMSET(&st_flash_part_table, 0, sizeof(aui_flash_partition_table));
	return ret;

ERROR:
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		flash去初始化函数
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   p_call_back_init:初始化回调函数
*	 @param[in]    pv_param:p_call_back_init 参数
*	 @return		 错误码
*	 @note		  与aui_flash_init配对使用，在退出flash模块时调用，传入一个回调函数，aui_flash_de_init里会调用此回调函数。
*
*/
AUI_RTN_CODE aui_flash_de_init(p_fun_cb p_call_back_init,void *pv_param)
{
	FLASH_FUNCTION_ENTER;

	if(p_call_back_init)
	{
		p_call_back_init(pv_param);
	}
	if(INVALID_ID != flash_mutex)
	{
		osal_mutex_delete(flash_mutex);
		flash_mutex = INVALID_ID;
	}

	FLASH_FUNCTION_LEAVE;

    return AUI_RTN_SUCCESS;

}

/**
*	 @brief 		打开flash设备
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   open_param:open参数
*	 @param[out]	flash_handle:返回的flash句柄
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_open(aui_flash_open_param *open_param,aui_hdl *flash_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	unsigned long open_cnt;
	struct ali_nand_device *aui_nand_dev = NULL;
	void * flash_dev = NULL;
	aui_flash_info flash_info;
	typedef struct driver_flash_info_
	{
		UINT32 flash_size;
		UINT32 total_sectors;
		UINT32 sector_size;
	}st_driver_flash_info;
	st_driver_flash_info driver_flash_info;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
    AUI_DBG(" flash_id: %d, flash_type: %d, part_base: 0x%x, part_size: 0x%x\n", 
                    open_param->flash_id, open_param->flash_type,
                    st_flash_part_table.partitions[open_param->flash_id].ul_offset, 
                    st_flash_part_table.partitions[open_param->flash_id].ul_size);
    if((NULL==open_param) 
        || ((open_param->flash_type == FLASH_TYPE_NOR) && (open_param->flash_id != 0))
        || ((open_param->flash_type == FLASH_TYPE_NAND) && (open_param->flash_id < 1)))
    {
		ret = AUI_FLASH_DRIVER_ERROR;        
		AUI_ERR("ERROR,aui_flash_open,527, please check the open_param! \n"/*,__FUNCTION__,__LINE__*/);
		goto ERROR;
	}

    if (open_param->flash_type == FLASH_TYPE_NOR) { //Nor
        flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
    } else { //NAND
        // have do ali_nand_attach() in aui_root.c;        
        aui_nand_dev = (struct ali_nand_device *)dev_get_by_name("ALI_NAND");        
        flash_dev = (void *)aui_nand_dev;        
    }
	
	if(NULL == flash_dev)
	{
		ret = AUI_FLASH_DRIVER_ERROR;
		AUI_ERR("ERROR,aui_flash_open,542,dev_get_by_type failed \n"/*,__FUNCTION__,__LINE__*/);
		goto ERROR;
	}
	open_cnt = get_flash_open_cnt(open_param->flash_id);
	if(open_cnt == 0)
	{
        if (open_param->flash_type == FLASH_TYPE_NOR) { // Nor
            ret = sto_open((struct sto_device *)flash_dev);
        } else { //Nand
            // have do ali_nand_open in aui_root.c
            //ret = ali_nand_open((struct ali_nand_device *)flash_dev);
        }            
		if(RET_SUCCESS != ret)
		{
			ret = AUI_FLASH_DRIVER_ERROR;
			AUI_ERR("sto_open failed\n");
			goto ERROR;
		}
	}

    if (open_param->flash_type == FLASH_TYPE_NOR) { // Nor
        ret = sto_io_control(flash_dev, STO_DRIVER_GET_FLASH_INFO, (unsigned long)&driver_flash_info);	        
        #if 0 // wait driver commit
        //set flag to inform driver erase by sector or block
        if (open_param->flash_erase_type == AUI_FLASH_ERASE_TYPE_SECTOR)
            ret = sto_io_control((struct sto_device *)(flash_dev), STO_DRIVER_SET_FLAG, STO_FLAG_ERASE_BY_SECTOR);
        #endif
        /* The driver would handle not page align size, so do not care the writesize of nor */
        flash_info.write_size = 1; 
    } else { //Nand
        struct ali_nand_info nand_info;
        nf_get_nand_info((struct ali_nand_device *)aui_nand_dev, &nand_info);
        driver_flash_info.sector_size = nand_info.blocksize;
        driver_flash_info.total_sectors = nand_info.block_per_chip * nand_info.numchips;
        driver_flash_info.flash_size = nand_info.blocksize * nand_info.block_per_chip * nand_info.numchips;
        flash_info.write_size = nand_info.pagesize;
    }
    if(RET_SUCCESS != ret) {
        ret = AUI_FLASH_DRIVER_ERROR;
        AUI_ERR("get flash info failed\n");
        goto ERROR;
    }
    incress_open_cnt(open_param->flash_id);
    AUI_INFO("%s, flash_size=0x%x,total_sectors=%d,sector_size=0x%x\n",
                __func__, driver_flash_info.flash_size,driver_flash_info.total_sectors,driver_flash_info.sector_size);
    if ((open_param->flash_type == FLASH_TYPE_NAND) 
        && (st_flash_part_table.partitions[open_param->flash_id].ul_size > 0)) {
        flash_info.flash_size = st_flash_part_table.partitions[open_param->flash_id].ul_size;
        set_flash_part_base(open_param->flash_id, st_flash_part_table.partitions[open_param->flash_id].ul_offset);
    } else {
        flash_info.flash_size = driver_flash_info.flash_size;
        set_flash_part_base(open_param->flash_id, 0);
    }
    flash_info.block_size = driver_flash_info.sector_size;
    flash_info.block_cnt = flash_info.flash_size / flash_info.block_size;	
    flash_info.star_address = 0;
    set_flash_info(open_param->flash_id, &flash_info);
    set_flash_devic(open_param->flash_id, flash_dev); 
    set_flash_type(open_param->flash_id, open_param->flash_type);

    
    *flash_handle = get_flash_attr(open_param->flash_id);
    if ((open_cnt == 0) && (open_param->flash_type == FLASH_TYPE_NAND)) {
        AUI_DBG("%s, to call create_bbt_map\n", __func__);
        create_bbt_map(*flash_handle);
    }
    FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;

ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		关闭FLASH设备
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:由aui_flash_open返回的句柄
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_close(aui_hdl flash_handle)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;

	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
	reduce_open_cnt(flash_attr->flash_id);
	if(get_flash_open_cnt(flash_attr->flash_id) == 0)
	{
	    if (flash_attr->flash_type == AUI_FLASH_TYPE_NOR)
		    ret = sto_close((struct sto_device *)(flash_attr->flash_dev));
        else {
            // do not clall ali_nand_close/detach here, or other APP can't use flash device
            //ret = ali_nand_close((struct ali_nand_device *)(flash_attr->flash_dev));
            //ret |= ali_nand_detach((struct ali_nand_device *)(flash_attr->flash_dev));
            if (flash_attr->bad_map) {
                free(flash_attr->bad_map);
                flash_attr->bad_map = NULL;
            }
        }
		if(RET_SUCCESS != ret)
		{
			ret = AUI_FLASH_DRIVER_ERROR;
			AUI_ERR("OSDDrv_close failed\n");
			goto ERROR;
		}
		rest_flash_attr(flash_attr->flash_type);
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;

ERROR:

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		获取FLASH信息
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash句柄
*	 @param[out]	p_flash_info:返回的FLASH信息
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_info_get(aui_hdl flash_handle,aui_flash_info *p_flash_info)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if((check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) 
        || (NULL == p_flash_info))
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
	MEMCPY(p_flash_info, &flash_attr->st_flash_info, sizeof(aui_flash_info));

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		擦除指定BLOCK
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   p_address:要擦除的FLASH地址，地址必须是BLOCK对齐
*	 @param[in] 	   erase_size:要擦除的大小 ,大小必须是BLOCK对齐
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_erase(aui_hdl flash_handle,unsigned long address,unsigned long erase_size)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;
	unsigned long erase_param[32];

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR(" input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
	erase_param[0] =address;
	erase_param[1] = erase_size>>10;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG("tds erase nand flash at: 0x%x, size: %d\n", address, erase_size);
        ret = aui_nand_erase(flash_attr, address, erase_size);
    } else {
        ret = sto_io_control((struct sto_device *)(flash_attr->flash_dev), STO_DRIVER_SECTOR_ERASE_EXT, (unsigned long)erase_param); 	
    }
	if(SUCCESS != ret)
	{
		ret = AUI_FLASH_DRIVER_ERROR;
		AUI_ERR("STO_DRIVER_SECTOR_ERASE_EXT failed\n");
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

/**
*	 @brief 		从FLASH里读数据
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   star_addr:需要读的起始地址
*	 @param[in] 	   read_size:需要读的大小
*	 @param[out]	p_return_size:实际读取的大小
*	 @param[out]	buf:存放读取的数据
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_read(aui_hdl flash_handle,unsigned long address,unsigned long read_size,unsigned long *p_return_size,unsigned char *buf)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG(" tds read nand flash\n");
        aui_flash_errno ret_len = aui_nand_read(flash_attr, address, read_size, buf);  
        if ((AUI_FLASH_SUCCESS == ret_len) || (ret_len == read_size))
            *p_return_size = ret_len;
        else {
            *p_return_size = -1;
            AUI_ERR(" aui_nand_write fail, return: %d\n", ret_len);
            goto ERROR;
        }
         
    } else {
	    *p_return_size = sto_get_data((struct sto_device *)(flash_attr->flash_dev),buf,address,read_size);
    }
    
    if (*p_return_size != read_size) {
        AUI_ERR("real read size is: %ld, expect is: %ld\n", *p_return_size, read_size);
        goto ERROR;
    }
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}


/**
*	 @brief 		往FLASH里写数据到
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   star_addr:需要写的起始地址
*	 @param[in] 	   write_size:需要写的大小
*	 @param[in] 	   buf:存放要写入的数据
*	 @param[out]	p_return_size:实际写入的大小
*	 @return		 错误码
*	 @note		  在写FLASH之前，必须保证要写的区域之前没有被写过，如果该区域已经被写过，在写之前必须先擦除
*
*/
AUI_RTN_CODE aui_flash_write(aui_hdl flash_handle,unsigned long star_addr,unsigned long write_size,unsigned long *p_return_size,unsigned char *buf)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG("aui write nand flash\n");
        aui_flash_errno ret_len = aui_nand_write(flash_attr, star_addr, write_size, buf);
        if ((AUI_FLASH_SUCCESS == ret_len) || (ret_len == write_size))
            *p_return_size = ret_len;
        else {
            *p_return_size = -1;
            AUI_ERR(" aui_nand_write fail, return: %d\n", ret_len);
            goto ERROR;
        }
    } else {
#if 0
        *p_return_size = sto_put_data((struct sto_device *)(flash_attr->flash_dev),star_addr,buf,write_size);
#else
        // Internal_Issue #53277, the sto_put_data would erase the flash before do write opration.
        // Here, we only need to write without erase.        
        unsigned int cur_addr = sto_lseek((struct sto_device *)(flash_attr->flash_dev),0,AUI_FLASH_LSEEK_CUR);
        unsigned int seek_to_addr = sto_lseek((struct sto_device *)(flash_attr->flash_dev),star_addr,AUI_FLASH_LSEEK_SET);
        AUI_DBG(" sto seek, get cur_addr: 0x%x\n", cur_addr);
        if (seek_to_addr != star_addr) {
            *p_return_size = -1;
            AUI_ERR("seek to start_addr fail, return: %d\n", cur_addr);
            goto ERROR;
        }
        AUI_DBG(" sto write to: 0x%x\n", seek_to_addr);
        *p_return_size = sto_write((struct sto_device *)(flash_attr->flash_dev),buf,write_size);
        sto_lseek((struct sto_device *)(flash_attr->flash_dev),cur_addr,AUI_FLASH_LSEEK_SET);
#endif
    }

    if (*p_return_size != write_size) {
        AUI_ERR("real write size is: %ld, expect is: %ld\n", *p_return_size, write_size);
        goto ERROR;
    }

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		往FLASH里写数据到,如果被写入的区域被写过，本函数会把整个block先保存起来，再擦除整个BLOCK，\n
把本次需要写入的数据加进之前保存的数据里，然后回写数据。
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   star_addr:需要写的起始地址
*	 @param[in] 	   write_size:需要写的大小
*	 @param[in] 	   buf:存放要写入的数据
*	 @param[out]	p_return_size:实际写入的大小
*	 @return
*	 @note		  1.本函数与aui_flash_write的唯一区别在于aui_flash_auto_erase_write会先检测待写区域是否被写过，如果写过就会自动
擦除再写 。\n 2.由于本函数会自动擦除，因此上层在使用的时候保证被写入的block里的数据是允许擦除的。\n
3.本函数效率比aui_flash_write要低一些，如果上层不需要自动擦除功能，请直接使用aui_flash_write。
*
*/
AUI_RTN_CODE aui_flash_auto_erase_write(aui_hdl flash_handle,unsigned long star_addr,unsigned long write_size,unsigned long *p_return_size,unsigned char *buf)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;
	unsigned char *block_backup_addr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}

	flash_attr = (aui_flash_attr *)flash_handle;
	block_backup_addr = MALLOC(flash_attr->st_flash_info.block_size);
	if(NULL == block_backup_addr)
	{
		ret = AUI_FLASH_NO_MEMORY;
		AUI_ERR("malloc failed\n");
		goto ERROR;
	}
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG(" %s \n", __FUNCTION__);
        ret |= aui_nand_erase(flash_attr, star_addr, write_size);
        aui_flash_errno ret_len = aui_nand_write(flash_attr, star_addr, write_size, buf);
        if ((AUI_FLASH_SUCCESS == ret_len) || (ret_len == write_size)) {
            *p_return_size= ret_len;
            flash_attr->offset += ret_len;
        } else {
            *p_return_size = -1;
            ret |= AUI_RTN_FAIL;
            AUI_ERR(" aui_nand_write fail, return: %d\n", ret_len);
            goto ERROR;
        }
    } else {
    	ret = sto_io_control((struct sto_device *)(flash_attr->flash_dev), STO_DRIVER_SECTOR_BUFFER, (unsigned long)block_backup_addr);
    	ret |= sto_io_control((struct sto_device *)(flash_attr->flash_dev), STO_DRIVER_SET_FLAG, STO_FLAG_AUTO_ERASE|STO_FLAG_SAVE_REST);
    	if(SUCCESS != ret)
    	{
    		ret = AUI_FLASH_DRIVER_ERROR;
    		AUI_ERR("sto_io_control failed\n");
    		goto ERROR;
    	}
    	*p_return_size = sto_put_data((struct sto_device *)(flash_attr->flash_dev),star_addr,buf,write_size);

    	ret = sto_io_control((struct sto_device *)(flash_attr->flash_dev), STO_DRIVER_SECTOR_BUFFER, 0);
    	ret |= sto_io_control((struct sto_device *)(flash_attr->flash_dev), STO_DRIVER_SET_FLAG, 0);
    	if(SUCCESS != ret)
    	{
    		ret = AUI_FLASH_DRIVER_ERROR;
    		AUI_ERR("sto_io_control failed\n");
    		goto ERROR;
    	}
    }

    if (*p_return_size != write_size) {
        AUI_ERR("real write size is: %ld, expect is: %ld\n", *p_return_size, write_size);
        goto ERROR;
    }
	//if(block_backup_addr) //bug detective
	{
		FREE(block_backup_addr);
	}
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:
	if(block_backup_addr)
	{
		FREE(block_backup_addr);
	}
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}



/**
*	 @brief 		重新定位FLASH读写操作的地址
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   seek_offset:SEEK的偏移量
*	 @param[in] 	   seek_type:SEEK的类型
*	 @param[out]	cur_addr:SEEK后FLASH的操作地址
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_seek(aui_hdl flash_handle,long seek_offset,aui_flash_seek_type seek_type,unsigned int *cur_addr)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG(" seek_offset: 0x%x\n", seek_offset);
        *cur_addr = aui_nand_lseek(flash_attr, seek_offset, seek_type);
    } else {
	    *cur_addr = sto_lseek((struct sto_device *)(flash_attr->flash_dev),seek_offset,seek_type);
    }

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		往FLASH当前地址里写入数据，写入后，FLASH的读写操作地址会加上本次的实际写入长度
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   write_len:需要写入的长度
*	 @param[in] 	   buf:存放需要写入的数据
*	 @param[out]	actual_len:实际写入的长度
*	 @return		 错误码
*	 @note		  本函数与aui_flash_write的区别:
1.使用aui_flash_write 需要传入将要写入的地址，而aui_flash_seek_write是在FLASH当前地址写入数据\n
2.aui_flash_seek_write写入数据后会更新FLASH的当前地址，而aui_flash_write不会。\n
在于在写FLASH之前，必须保证要写的区域之前没有被写过，如果该区域已经被写过，在写之前必须先擦除
*
*/
AUI_RTN_CODE aui_flash_seek_write(aui_hdl flash_handle,unsigned long write_len,unsigned char *buf,unsigned long *actual_len)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if((check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) 
        || (NULL == buf)) {
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG("offset: 0x%x, size: 0x%x\n", flash_attr->offset, write_len);
        aui_flash_errno ret_len = aui_nand_write(flash_attr, flash_attr->offset, write_len, buf);
        if ((AUI_FLASH_SUCCESS == ret_len) || (ret_len == write_len)) {
            *actual_len = ret_len;
            flash_attr->offset += ret_len;
        } else {
            *actual_len = -1;
            goto ERROR;
        }
    } else {
	    *actual_len = sto_write((struct sto_device *)(flash_attr->flash_dev),buf,write_len);
    }

    if (*actual_len != write_len) {
        AUI_ERR("real write size is: %ld, expect is: %ld\n", *actual_len, write_len);
        goto ERROR;
    }

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		从FLASH里读数据，读完以后FLASH的读写操作地址会加上本次的实际读出长度
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle:flash 句柄
*	 @param[in] 	   read_len:需要读取的长度
*	 @param[out]	buf:存放读取的数据
*	 @param[out]	actual_len:实际从FLASH里读出的数据长度
*	 @return		 错误码
*	 @note		  本函数与aui_flash_read的区别 :
1.使用aui_flash_read需要传入将要读取数据的地址，而aui_flash_seek_read是从FLASH当前地址开始读取数据\n
2.aui_flash_seek_read 读取数据后会更新FLASH的当前地址，而aui_flash_read不会\n
*
*/
AUI_RTN_CODE aui_flash_seek_read(aui_hdl flash_handle,unsigned long read_len,unsigned char *buf,unsigned long *actual_len)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;

	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if((check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) 
        || (NULL == buf)) {
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_DBG(" offset: 0x%x, size: 0x%x\n", flash_attr->offset, read_len);
        aui_flash_errno ret_len = aui_nand_read(flash_attr, flash_attr->offset, read_len, buf);
        if ((AUI_FLASH_SUCCESS == ret_len) || (ret_len == read_len)) {
            *actual_len = ret_len;
            flash_attr->offset += ret_len;
        } else {
            *actual_len = -1;
            goto ERROR;
        }
    } else {
	    *actual_len = sto_read((struct sto_device *)(flash_attr->flash_dev),buf,read_len);
    }

    if (*actual_len != read_len) {
        AUI_ERR("real read size is: %ld, expect is: %ld\n", *actual_len, read_len);
        goto ERROR;
    }

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

/**
*	 @brief 		设置一些FLASH的命令，一般不需要用到
*	 @author		andy.yu
*	 @date			  2013-6-24
*	 @param[in] 	   flash_handle
*	 @param[in] 	   cmd:需要设置的命令
*	 @param[in] 	   param:命令参数
*	 @param[out]	NULL
*	 @return		 错误码
*	 @note
*
*/
AUI_RTN_CODE aui_flash_set_cmd(aui_hdl flash_handle,unsigned long cmd,unsigned long param)
{
	AUI_RTN_CODE ret = SUCCESS;
	aui_flash_attr * flash_attr = NULL;    
	FLASH_FUNCTION_ENTER;
	FLASH_LOCK;
	if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE)
	{
		ret = AUI_FLASH_PARAM_INVALID;
		AUI_ERR("input flash_handle invalid\n");
		goto ERROR;
	}
	flash_attr = (aui_flash_attr *)flash_handle;
    if (flash_attr->flash_type == AUI_FLASH_TYPE_NAND) {
        AUI_ERR(" only for nor flash\n");
        ret = -1;
	    return ret;
    } 
	ret = sto_io_control((struct sto_device *)(flash_attr->flash_dev),cmd,param);
	if(SUCCESS != ret)
	{
		ret = AUI_FLASH_DRIVER_ERROR;
		AUI_ERR("input sto_io_control failed\n");
		goto ERROR;
	}

	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	return SUCCESS;
ERROR:
	FLASH_UNLOCK;
	FLASH_FUNCTION_LEAVE;
	aui_rtn(ret,NULL);
}

AUI_RTN_CODE aui_flash_is_lock (aui_hdl flash_handle, unsigned long address, unsigned long size, unsigned long *lock)
{
    AUI_RTN_CODE ret = AUI_FLASH_SUCCESS;
    FLASH_FUNCTION_ENTER;
    FLASH_LOCK;

    if(check_flash_handle((aui_flash_attr *)flash_handle) == FALSE) {
        ret = AUI_FLASH_PARAM_INVALID;
        AUI_ERR("input flash_handle invalid\n");
        goto ERROR;
    }

    aui_flash_attr *flash_attr = NULL;

    flash_attr = (aui_flash_attr *)flash_handle;
    ret = sto_is_lock((struct sto_device *)(flash_attr->flash_dev), address, size, (INT32 *)lock);
    if (0 != ret) {
        ret = AUI_FLASH_DRIVER_ERROR;
        AUI_ERR("flash driver return error\n");
        AUI_ERR(" sto_is_lock return fail at: 0x%x, size: 0x%x\n", 
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
        ret = AUI_FLASH_PARAM_INVALID;
        AUI_ERR("input flash_handle invalid\n");
        goto ERROR;
    }

    aui_flash_attr *flash_attr = NULL;

    flash_attr = (aui_flash_attr *)flash_handle;
    unsigned long ret_size = 0;
    ret_size = sto_lock((struct sto_device *)(flash_attr->flash_dev), address, size);
    if (ret_size != size) {
        ret = AUI_FLASH_DRIVER_ERROR;
        AUI_ERR("flash driver return error\n");
        AUI_ERR(" sto_lock return fail at: 0x%x, size: 0x%x\n", 
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
        ret = AUI_FLASH_PARAM_INVALID;
        AUI_ERR("input flash_handle invalid\n");
        goto ERROR;
    }

    aui_flash_attr *flash_attr = NULL;

    flash_attr = (aui_flash_attr *)flash_handle;
    ret = sto_unlock((struct sto_device *)(flash_attr->flash_dev), address, size);
    if (0 != ret) {
        ret = AUI_FLASH_DRIVER_ERROR;
        AUI_ERR("flash driver return error\n");
        AUI_ERR("sto_unlock return fail at: 0x%x, size: 0x%x\n", 
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

/**
@brief              Function used to set the partition table for the aui_flash module.
                    each partition can be opened by aui_flash_open function by specifying 
                    flash_id and flash_type of aui_flash_open_param.

                    flash_id is the element index of the partitions array in struct #aui_flash_partition_table

@param[in]          p_partition_table        =   Pointer to a struct
                                            #aui_flash_partition_table, which
                                            collects the Flash partition table to be
                                            used for opening a Flash Memory
                                            Device

@return         @b AUI_RTN_SUCCESS      = Setting the partition table performed
                                          successfully

@return         @b Other_Values         = Setting the partition table failed for
                                          some reasons
*/

AUI_RTN_CODE aui_flash_init_partition_table (

    aui_flash_partition_table *p_partition_table

    )
{
    if ((p_partition_table->ul_partition_num <= 0)
        || (p_partition_table->partitions == NULL)) {
        AUI_ERR("invalid param.\n");
        return AUI_RTN_FAIL;
    }
    if (g_part_info_init) {
        AUI_WARN("[warning] init partition info again.!!!\n");
    }
    st_flash_part_table.ul_partition_num = p_partition_table->ul_partition_num + 1; // part 0 is nor flash

    unsigned int size = sizeof(aui_flash_partition) * st_flash_part_table.ul_partition_num;
    memset(st_flash_part_table.partitions, 0, size);
    memcpy(st_flash_part_table.partitions + 1, p_partition_table->partitions, size);

    //for debug
    AUI_DBG("total partion: %d\n", (unsigned int)st_flash_part_table.ul_partition_num);
    int i = 0;
    for (i = 0; i < (int)st_flash_part_table.ul_partition_num; i++) {
        AUI_DBG("i: %d, type: %d, offset: 0x%x, size: 0x%x\n", 
                i, (unsigned int)st_flash_part_table.partitions[i].type,
                (unsigned int)st_flash_part_table.partitions[i].ul_offset,
                (unsigned int)st_flash_part_table.partitions[i].ul_size);
    }
    g_part_info_init = 1;
    return AUI_RTN_SUCCESS;
}


