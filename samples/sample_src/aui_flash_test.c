#include "aui_flash_test.h"
#include "aui_help_print.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// set FIXED test size to avoid there have no enough memory to test whole nor flash, 
#define NOR_FLASH_TEST_SIZE  4 * 1024 * 1024 // 4M

struct mtd_stash {
	unsigned char *buf;
	unsigned long size;
	int index;
};
static struct mtd_stash g_stash;


static int flash_stash_partition(int partition_index)
{
	int err = 0;
	aui_hdl flash_handle;
	unsigned char *buf = NULL;
	unsigned long len;
	aui_flash_open_param open_param;
	long partition_size = 0;
	aui_flash_info flash_info;

	if (g_stash.buf && partition_index != g_stash.index) {
    	AUI_PRINTF("Stash buf not empty partition %d\n", g_stash.index);
    	return 1;
    }

	memset(&open_param, 0, sizeof(aui_flash_open_param));
	err = aui_flash_init(NULL, NULL);
	if (err != AUI_RTN_SUCCESS) {
    	return 1;
    }
	open_param.flash_id = partition_index;
	open_param.flash_type = AUI_FLASH_TYPE_NOR;
	err = aui_flash_open(&open_param, &flash_handle);
	if (err != AUI_RTN_SUCCESS) {
    	AUI_PRINTF("aui_flash_open fail %ld\n", open_param.flash_id);
    	return 1;
    }
	memset(&flash_info, 0, sizeof(flash_info));
	err = aui_flash_info_get(flash_handle, &flash_info);
	if (err != AUI_RTN_SUCCESS) {
    	AUI_PRINTF("aui_flash_info_get fail\n");
    	return 1;
    }
    //if (flash_info.flash_size > NOR_FLASH_TEST_SIZE)
    //    partition_size = NOR_FLASH_TEST_SIZE;//flash_info.flash_size;
	//else
        partition_size = flash_info.flash_size;

    // malloc
	buf = g_stash.buf;
	if (!buf) {
    #ifdef USE_NESTOR_MALLOC
        if ((buf = NESTOR_MALLOC(partition_size)) == NULL) {
    #else
    	if ((buf = malloc(partition_size)) == NULL) {
    #endif
        	AUI_PRINTF("%s,%d,MALLOC failed!\n", __FUNCTION__, __LINE__);
#ifndef AUI_LINUX
        	AUI_PRINTF("%s,%d,Use VOB cache __MM_PVR_VOB_BUFFER_ADDR: %lx, len: %d\n",
                	__FUNCTION__, __LINE__, __MM_PVR_VOB_BUFFER_ADDR, __MM_PVR_VOB_BUFFER_LEN);
        	if (partition_size > __MM_PVR_VOB_BUFFER_LEN) { 
                AUI_PRINTF("%s,%d,Use VOB cache __MM_PVR_VOB_BUFFER_ADDR fail! no enough memory. only: %d\n",
                	__FUNCTION__, __LINE__, __MM_PVR_VOB_BUFFER_LEN);
                return 1;
            }
        	buf = (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR;
#endif
#ifndef AUI_TDS
        	return 1;
#endif
        }
    }

	len = 0;
	err |= aui_flash_read(flash_handle, 0, partition_size, &len, buf);
	if (err != AUI_RTN_SUCCESS || len != (unsigned long)partition_size) {
    	AUI_PRINTF("%s,%d, AUI_RTN= %d\n", __FUNCTION__, __LINE__, err);
    	err = 1;
    	AUI_PRINTF("%s,%d, read flash len= %#lx pos=%#lx, err=%d\n",
            	__FUNCTION__, __LINE__, len, 0L, err);
    	g_stash.buf = 0;
    	g_stash.size = 0;
    	g_stash.index = 0;
#ifndef AUI_LINUX
    	if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
#endif
        #ifdef USE_NESTOR_MALLOC
            NESTOR_FREE(buf);
        #else
        	free(buf);
        #endif
#ifndef AUI_LINUX
        }
#endif
    } else {
    	g_stash.buf = buf;
    	g_stash.size = partition_size;
    	g_stash.index = partition_index;
    	AUI_PRINTF("stash ok index=%ld, size: 0x%x\n", open_param.flash_id, (unsigned int)g_stash.size);
    	AUI_PRINTF("%x %x\n", g_stash.buf[0], g_stash.buf[1]);
    }
	err |= aui_flash_close(flash_handle);
	err |= aui_flash_de_init(NULL, NULL);
	return err;
}

static int flash_stash_pop_partition(int partition_index, unsigned long position, unsigned long size)
{
	int err = 0;
	aui_hdl flash_handle;
	unsigned char *buf = NULL;
	unsigned long len;
	aui_flash_open_param open_param;
	unsigned long lock = 0;
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;

	if (!g_stash.buf || partition_index != g_stash.index) {
    	AUI_PRINTF("Stash pop %d input error buf=%p index=%d\n",
            	partition_index, g_stash.buf, g_stash.index);
    	return 1;
    } else {
    	AUI_PRINTF("Stash pop %d buf=%p index=%d, size: 0x%x\n",
            	partition_index, g_stash.buf, g_stash.index, (unsigned int)g_stash.size);
    }

	memset(&open_param, 0, sizeof(aui_flash_open_param));
	err = aui_flash_init(NULL, NULL);
	if (err != AUI_RTN_SUCCESS) {
    	AUI_PRINTF("aui_flash_init fail\n");
    	return 1;
    }
	open_param.flash_id = partition_index;
	open_param.flash_type = AUI_FLASH_TYPE_NOR;
	err = aui_flash_open(&open_param, &flash_handle);
	if (err != AUI_RTN_SUCCESS) {
    	AUI_PRINTF("aui_flash_open fail index=%ld\n", open_param.flash_id);
    	return 1;
    }

    if(size != 0)
    {
        ret = aui_flash_is_lock(flash_handle, position, size, &lock);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
    }
	AUI_PRINTF("lock status: %d\n\n", lock);
	if(lock == 1)
	{
		AUI_PRINTF("flash is locked, need to unlock before restore data.\n\n");
		ret = aui_flash_unlock(flash_handle, position, size);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
	}
	
    // malloc
	buf = g_stash.buf;
	len = 0;
	AUI_PRINTF("%x %x\n",buf[0], buf[1]);
	err |= aui_flash_erase(flash_handle, 0, g_stash.size);
	err |= aui_flash_write(flash_handle, 0, g_stash.size, &len, buf);
	if (err != AUI_RTN_SUCCESS || len != g_stash.size) {
    	AUI_PRINTF("%s,%d, AUI_RTN= %d\n", __FUNCTION__, __LINE__, err);
    	err = 1;
    	AUI_PRINTF("%s,%d, write flash len= %#lx pos=%#lx, err=%d\n",
            	__FUNCTION__, __LINE__, len, 0L, err);
    } else {
    	AUI_PRINTF("stash pop ok index=%ld\n", open_param.flash_id);
    }

	if(lock == 1)
	{
		AUI_PRINTF("flash need to be locked!\n");
		ret = aui_flash_lock(flash_handle, position, size);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
	}
	
	err |= aui_flash_close(flash_handle);
	err |= aui_flash_de_init(NULL, NULL);
	if (!err) {
#ifndef AUI_LINUX
    	if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
#endif
        #ifdef USE_NESTOR_MALLOC
            NESTOR_FREE(buf);
        #else
        	free(buf);
        #endif
#ifndef AUI_LINUX
        }
#endif
    	g_stash.buf = 0;
    	g_stash.size = 0;
    	g_stash.index = 0;
    }
	return err;
}

static int flash_stash_clean()
{
	int ret = 1;
	if (g_stash.buf) {
#ifndef AUI_LINUX
    	if (g_stash.buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
#endif
        #ifdef USE_NESTOR_MALLOC
            NESTOR_FREE(g_stash.buf);
        #else
        	free(g_stash.buf);
        #endif
#ifndef AUI_LINUX
        }
#endif
    	g_stash.buf = 0;
    	ret = 0;
    }
	g_stash.size = 0;
	g_stash.index = 0;
	return ret;
}


unsigned long test_nor_flash(unsigned long *argc, char **argv, char *sz_out_put)
{
	aui_hdl flash_handle;
	unsigned char write_data = 0xA5;
	unsigned char write_data2 = 0xB6;
	unsigned char *buf = NULL;
	unsigned char *compare_buf = NULL;
	unsigned long i,j;
	unsigned long position = 0;
	unsigned long return_len;
	unsigned long erase_type = 0; //by block
	int partition_index = 0;
	aui_flash_open_param open_param;
	char *tailprt = NULL;
	char *usage = \
			"Usage: [1. index,offset,erase_type]\n \
For example, linux: [1 11,0x0], tds: [1 0,0x0]\n \
index is sflash index \n \
This test read/write 4MB flash! \n \
erase_type: 0 - by block, 1 - by sector.\n";

	AUI_PRINTF("argc=%ld\n", *argc);
	AUI_PRINTF("Enter %s\n", __FUNCTION__);

	if (*argc < 2) {
		AUI_PRINTF("%s\n", usage);
		return AUI_RTN_EINVAL;
	}

	partition_index = atoi(argv[0]);
	errno = 0;
	position = strtol(argv[1], &tailprt, 0);
	if (errno) {
		AUI_PRINTF("%s\n", usage);
		return AUI_RTN_EINVAL;
	}

	if (*argc > 2) {
		erase_type = strtol(argv[2], &tailprt, 0);
		AUI_PRINTF("erase type: %ld\n", erase_type);
	}

    flash_stash_partition(partition_index);

#define FLASH_SIZE 1 * 1024 * 1024

	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    int compare_err = 0;
	buf = MALLOC(FLASH_SIZE);
	compare_buf = MALLOC(FLASH_SIZE);

	if ((buf == NULL) || (compare_buf == NULL)) {
		AUI_PRINTF("%s,%d,MALLOC failed!\n",
				__FUNCTION__, __LINE__);
		return AUI_RTN_DRIVER_ERROR;
	}

	memset(&open_param, 0, sizeof(aui_flash_open_param));
	open_param.flash_id = partition_index;
	open_param.flash_type = AUI_FLASH_TYPE_NOR;
	open_param.flash_erase_type = erase_type;
	MEMSET(compare_buf, write_data, FLASH_SIZE);
	ret = aui_flash_init(NULL, NULL);
	CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
	ret = aui_flash_open(&open_param, &flash_handle);
	CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

	aui_flash_info flash_info;
	ret = aui_flash_info_get(flash_handle, &flash_info);
	CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
	AUI_PRINTF("\n\nblock_cnt: %d, block_size: %#x, flash_size: %#x, write_size: %#x\n\n",
			flash_info.block_cnt, flash_info.block_size, flash_info.flash_size, flash_info.write_size);

	for (i = 0; i < 2; i++) {

        if(position+FLASH_SIZE > flash_info.flash_size)
        {
            AUI_PRINTF("test size %#x + %#x > flash partition size %#x, return failed. Please test sflash partition.\n\n", 
				position, (unsigned long)FLASH_SIZE, flash_info.flash_size);
			ret = AUI_RTN_EINVAL;
			break;
        }
		
		// 1. erase
		ret = aui_flash_erase(flash_handle, position, FLASH_SIZE);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
		MEMSET(buf, write_data, FLASH_SIZE);
		MEMSET(compare_buf, write_data, FLASH_SIZE);

        // 2. write
		ret = aui_flash_write(flash_handle,
				position,
				FLASH_SIZE,
				&return_len, buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != FLASH_SIZE)
		{
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
		}

		MEMSET(buf, 0, FLASH_SIZE);
		// 3. read
		ret = aui_flash_read(flash_handle,
				position,
				FLASH_SIZE,
				&return_len,
				buf);

		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != FLASH_SIZE) {
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
		}

		// 4. compare result
		if (MEMCMP(buf, compare_buf, FLASH_SIZE) != 0) {
			AUI_PRINTF("%s,%d, compare result: not match!\n",
					__FUNCTION__, __LINE__);
            for (j = 0; j < FLASH_SIZE; j++) {
                if (buf[j] != write_data) {
                    AUI_PRINTF("not match!!! buf[%d] = 0x%x write wrong!!!! \n", j, buf[j]);
                    break;
                } 
            }
            compare_err |= 1;
		}

		// 5. test write with erase
		MEMSET(buf, write_data2, FLASH_SIZE);
		MEMSET(compare_buf, write_data2, FLASH_SIZE);
		ret = aui_flash_auto_erase_write(flash_handle,
				position,
				FLASH_SIZE,
				&return_len,
				buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != FLASH_SIZE) {
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
		}

		MEMSET(buf, 0, FLASH_SIZE);
        // 6. read
		ret = aui_flash_read(flash_handle,
				position,
				FLASH_SIZE,
				&return_len,
				buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != FLASH_SIZE) {
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
		}

        // 7. compare
		if (MEMCMP(buf, compare_buf, FLASH_SIZE) != 0 ) {
			AUI_PRINTF("%s,%d, compare result: not match!\n",
					__FUNCTION__, __LINE__);
            for (j = 0; j < FLASH_SIZE; j++) {
                if (buf[j] != write_data) {
                    AUI_PRINTF("not match!!! buf[%d] = 0x%x write wrong!!!! \n", j, buf[j]);
                    break;
                } 
            }
            compare_err |= 1;
		}

		AUI_PRINTF("%s,%d,test 0x%x done.\n",
				__FUNCTION__, __LINE__, position);
		position += FLASH_SIZE;
	}

	//	position = 0;
	position = strtol(argv[1], &tailprt, 0);

	for (i = 0; i < 2; i++) {

		if(position+FLASH_SIZE > flash_info.flash_size)
        {
            AUI_PRINTF("test size %#x + %#x > flash partition size %#x, return failed. Please test sflash partition.\n\n", 
				position, (unsigned long)FLASH_SIZE, flash_info.flash_size);
			ret = AUI_RTN_EINVAL;
			break;
        }
		
		ret = aui_flash_seek(flash_handle,
				position,
				AUI_FLASH_LSEEK_SET,
				(unsigned int *)&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        // 1. erase
		ret = aui_flash_erase(flash_handle, position, FLASH_SIZE);

        // 2. check earse result
        MEMSET(buf, 0, FLASH_SIZE);
        AUI_PRINTF("-- check erase success? \n");
        ret = aui_flash_read(flash_handle,
				position,
				FLASH_SIZE,
				&return_len,
				buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        for (j = 0; j < FLASH_SIZE; j++) {
            if (buf[j] != 0xff) {
                AUI_PRINTF("not 0xff!!! buf[%d] = 0x%x erase wrong!!!! \n", j, buf[j]);
                compare_err |= 1;
                break;
            }                
        }
        AUI_PRINTF("=== check ok ===\n");

        // 3. seek & write
		MEMSET(buf, write_data, FLASH_SIZE);
		MEMSET(compare_buf, write_data, FLASH_SIZE);
        ret = aui_flash_seek(flash_handle,
				position,
				AUI_FLASH_LSEEK_SET,
				(unsigned int *)&return_len);
		ret = aui_flash_seek_write(flash_handle,
				FLASH_SIZE,
				buf,
				&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
		if (return_len != FLASH_SIZE) {
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
			return AUI_RTN_DRIVER_ERROR;
		}

        // 4. seek & read
		ret = aui_flash_seek(flash_handle,
				position,
				AUI_FLASH_LSEEK_SET,
				(unsigned int *)&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
		MEMSET(buf, 0, FLASH_SIZE);
		ret = aui_flash_seek_read(flash_handle,
				FLASH_SIZE,
				buf,
				&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != FLASH_SIZE) {
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
			return AUI_RTN_DRIVER_ERROR;
		}

        // 5. compare
		if (MEMCMP(buf, compare_buf, FLASH_SIZE) != 0 ) {
			AUI_PRINTF("%s,%d, compare result: not match!\n",
					__FUNCTION__, __LINE__);
            
            for (j = 0; j < FLASH_SIZE; j++) {
                if (buf[j] != write_data) {
                    AUI_PRINTF("not match!!! buf[%d] = 0x%x write wrong!!!! \n", j, buf[j]);
                    break;
                } 
            }
			//return AUI_RTN_DRIVER_ERROR;
			compare_err |= 1;
		}

		AUI_PRINTF("%s,%d,test 0x%x done.\n",
				__FUNCTION__, __LINE__, position);
		position += FLASH_SIZE;
	}

	aui_flash_close(flash_handle);
	aui_flash_de_init(NULL, NULL);

	if (buf) {
		FREE(buf);
	}
	if (compare_buf) {
		FREE(compare_buf);
	}

	flash_stash_pop_partition(partition_index, 0, 0);

	return ret | compare_err;
}

unsigned long test_nand_flash(unsigned long *argc, char **argv, char *sz_out_put)
{
	aui_hdl flash_handle;
	int write_data = 0xA5;
	int write_data2 = 0xB6;
	unsigned char *buf = NULL;
	unsigned char *compare_buf = NULL;
	unsigned long i;
	unsigned long position = 0x5540000;
	int partition_index = 0;
	unsigned long return_len;
	char *tailprt = NULL;
	unsigned long erasesize;
	char *usage = \
			"Usage: [2. index,offset]\n \
For example, [2 1,0x200000]\n \
Cause: This test will modify data of flash!\n \
For TDS, the command should be: 2 1,0.";

	AUI_PRINTF("Enter %s\n", __FUNCTION__);
	AUI_PRINTF("argc=%ld\n", *argc);
	if (*argc < 2) {
		AUI_PRINTF("%s\n", usage);
		return AUI_RTN_EINVAL;
	}

	partition_index = atoi(argv[0]);
	errno = 0;
	position = strtol(argv[1], &tailprt, 0);
	if (errno) {
		AUI_PRINTF("%s\n", usage);
		return AUI_RTN_EINVAL;
	}

	flash_stash_partition(partition_index);

	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	aui_flash_open_param open_param;

	ret = aui_flash_init(NULL, NULL);
	CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

	memset(&open_param, 0, sizeof(aui_flash_open_param));
	open_param.flash_id = partition_index;
	open_param.flash_type = AUI_FLASH_TYPE_NAND;

	ret = aui_flash_open(&open_param, &flash_handle);
	CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

	aui_flash_info flash_info;
	ret = aui_flash_info_get(flash_handle, &flash_info);
	CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
	AUI_PRINTF("\n\nblock_cnt: %d, block_size: %#x, flash_size: %#x, write_size: %#x\n\n",
			flash_info.block_cnt, flash_info.block_size, flash_info.flash_size, flash_info.write_size);
	erasesize = 2 * flash_info.block_size;

	buf = MALLOC(erasesize);
	compare_buf = MALLOC(erasesize);

	if ((buf == NULL) || (compare_buf == NULL)) {
		AUI_PRINTF("%s,%d,MALLOC failed!\n", __FUNCTION__, __LINE__);
		aui_flash_close(flash_handle);
		aui_flash_de_init(NULL, NULL);
		return AUI_RTN_DRIVER_ERROR;
	}

	//position = 0x8000000;
	for (i = 0; i < 2; i++) {
		
		if(position+erasesize > flash_info.flash_size)
        {
            AUI_PRINTF("test size %#x + %#x > flash partition size %#x, return failed.\n\n", 
				position, (unsigned long)erasesize, flash_info.flash_size);
			ret = AUI_RTN_EINVAL;
			break;
        }

		/*if(position+erasesize > NOR_FLASH_TEST_SIZE)
        {
            AUI_PRINTF("test size %#x + %#x > flash stash size %#x, return failed. Flash data will be erased and cannot restore\n\n", 
				position, (unsigned long)erasesize, NOR_FLASH_TEST_SIZE);
			ret = AUI_RTN_EINVAL;
			break;
        }*/
		
		write_data += 1;
		write_data2 += 1;
		//write
		ret = aui_flash_erase(flash_handle, position, erasesize);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
		MEMSET(buf, write_data, erasesize);
		MEMSET(compare_buf, write_data, erasesize);
		ret = aui_flash_write(flash_handle,
				position,
				erasesize,
				&return_len, buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != erasesize) {
			AUI_PRINTF("%s,%d,return len = %d\n", __FUNCTION__, __LINE__);
		}

		memset(buf, 0, erasesize);
#ifdef AUI_TDS
        /*
        * for #58058, the nand driver use hard ware dma to get buffer data, but memset is by CPU 
        * and may not be update the ram directly.
        * It may cause critical section. So call osal_cache_flush to update memset to ram at once.
        */
        osal_cache_flush(buf, erasesize);
#endif
		//read
		ret = aui_flash_read(flash_handle,
				position,
				erasesize,
				&return_len,
				buf);

		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != erasesize) {
			AUI_PRINTF("%s,%d,return len = %d\n", __FUNCTION__, __LINE__);
		}

		//compare result
		if (MEMCMP(buf, compare_buf, erasesize) != 0) {
			AUI_PRINTF("%s,%d,error,data error!\n", __FUNCTION__, __LINE__);
		}

		//test write with erase
		memset(buf, write_data2, erasesize);
		memset(compare_buf, write_data2, erasesize);
    #ifdef AUI_TDS
        osal_cache_flush(buf, erasesize);
        osal_cache_flush(compare_buf, erasesize);
    #endif
		ret = aui_flash_auto_erase_write(flash_handle,
				position,
				erasesize,
				&return_len,
				buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != erasesize) {
			AUI_PRINTF("%s,%d,return len = %d\n", __FUNCTION__, __LINE__);
		}

		memset(buf, 0, erasesize);
#ifdef AUI_TDS
        osal_cache_flush(buf, erasesize);
#endif
		ret = aui_flash_read(flash_handle,
				position,
				erasesize,
				&return_len,
				buf);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != erasesize) {
			AUI_PRINTF("%s,%d,return len = %d\n", __FUNCTION__, __LINE__);
		}

		if (MEMCMP(buf, compare_buf, erasesize) != 0 ) {
			AUI_PRINTF("%s,%d,error,data error!\n", __FUNCTION__, __LINE__);
		}

		AUI_PRINTF("%s,%d,test success\n", __FUNCTION__, __LINE__);
		position += erasesize;
	}

	//position = 0x8100000;
	position = strtol(argv[1], &tailprt, 0);

	for (i = 0; i < 2; i++)
	{
        if(position+erasesize > flash_info.flash_size)
        {
            AUI_PRINTF("test size %#x + %#x > flash partition size %#x, return failed.\n\n", 
				position, (unsigned long)erasesize, flash_info.flash_size);
			ret = AUI_RTN_EINVAL;
			break;
        }

		/*if(position+erasesize > NOR_FLASH_TEST_SIZE)
        {
            AUI_PRINTF("test size %#x + %#x > flash stash size %#x, return failed. Flash data will be erased and cannot restore\n\n", 
				position, (unsigned long)erasesize, NOR_FLASH_TEST_SIZE);
			ret = AUI_RTN_EINVAL;
			break;
        }*/
	
		write_data += 1;
		write_data2 += 1;

		ret = aui_flash_erase(flash_handle, position, erasesize);
		MEMSET(buf, write_data, erasesize);
		MEMSET(compare_buf, write_data, erasesize);
#ifdef AUI_TDS
        osal_cache_flush(buf, erasesize);
        osal_cache_flush(compare_buf, erasesize);
#endif
		ret = aui_flash_seek(flash_handle,
				position,
				AUI_FLASH_LSEEK_SET,
				(unsigned int *)&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
		ret = aui_flash_seek_write(flash_handle,
				erasesize,
				buf,
				&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != erasesize)
		{
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
			return AUI_RTN_DRIVER_ERROR;
		}

		ret = aui_flash_seek(flash_handle,
				position,
				AUI_FLASH_LSEEK_SET,
				(unsigned int *)&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
		MEMSET(buf, 0, erasesize);
#ifdef AUI_TDS
        osal_cache_flush(buf, erasesize);
#endif
		ret = aui_flash_seek_read(flash_handle,
				erasesize,
				buf,
				&return_len);
		CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

		if (return_len != erasesize)
		{
			AUI_PRINTF("%s,%d,return len = %d\n",
					__FUNCTION__, __LINE__);
			return AUI_RTN_DRIVER_ERROR;
		}

		if (MEMCMP(buf, compare_buf, erasesize) != 0 )
		{
			AUI_PRINTF("%s,%d,error,data error!\n",
					__FUNCTION__, __LINE__);
			return AUI_RTN_DRIVER_ERROR;
		}

		AUI_PRINTF("%s,%d,test 0x%x success\n",
				__FUNCTION__, __LINE__, position);
		position += erasesize;
	}

	aui_flash_close(flash_handle);
	aui_flash_de_init(NULL, NULL);

	if (buf) {
		FREE(buf);
	}
	if (compare_buf) {
		FREE(compare_buf);
	}

	flash_stash_pop_partition(partition_index, 0, 0);
	return ret;
}

unsigned long test_flash_help(unsigned long *argc,char **argv,char *sz_out_put)
{
	aui_print_help_header("\nFlash Test Help");

	/* FLASH_1_HELP */
#define     FLASH_1_HELP_PART1		"1st, the NOR flash is erased."
#define 	FLASH_1_HELP_PART2		"2nd, the NOR flash data whose size 1MB is written."
#define 	FLASH_1_HELP_PART3		"3th, the NOR flash data is read that was written last step."
#define 	FLASH_1_HELP_PART4		"4th, compare the written data and read data. If the same, the test is successful."
#define 	FLASH_1_HELP_PART5		"5th, repeat the 1st ~ 4th step test at the four different flash memor position."
#define 	FLASH_1_HELP_PART6		"6th, seek the flash memory and then do the test following the 1st ~ 5th step."
	aui_print_help_command("\'1\'");
	aui_print_help_instruction_newline("Start NOR flash reading and writing test.\n");
	aui_print_help_instruction_newline(FLASH_1_HELP_PART1);
	aui_print_help_instruction_newline(FLASH_1_HELP_PART2);
	aui_print_help_instruction_newline(FLASH_1_HELP_PART3);
	aui_print_help_instruction_newline(FLASH_1_HELP_PART4);
	aui_print_help_instruction_newline(FLASH_1_HELP_PART5);
	aui_print_help_instruction_newline(FLASH_1_HELP_PART6);
	aui_print_help_instruction_newline("\r\n");

	/* FLASH_2_HELP */
#define 	FLASH_2_HELP_PART1		"1st, the NAND flash is erased."
#define 	FLASH_2_HELP_PART2		"2nd, the NAND flash data whose size 1MB is written."
#define 	FLASH_2_HELP_PART3		"3th, the NAND flash data is read that was written last step."
#define 	FLASH_2_HELP_PART4		"4th, compare the written data and read data. If the same, the test is successful."
#define 	FLASH_2_HELP_PART5		"5th, repeat the 1st ~ 4th step test at the four different flash memor position."
#define 	FLASH_2_HELP_PART6		"6th, seek the flash memory and then do the test following the 1st ~ 5th step."
	aui_print_help_command("\'2\'");
	aui_print_help_instruction_newline("Start NAND flash reading and writing test.\n");
	aui_print_help_instruction_newline(FLASH_2_HELP_PART1);
	aui_print_help_instruction_newline(FLASH_2_HELP_PART2);
	aui_print_help_instruction_newline(FLASH_2_HELP_PART3);
	aui_print_help_instruction_newline(FLASH_2_HELP_PART4);
	aui_print_help_instruction_newline(FLASH_2_HELP_PART5);
	aui_print_help_instruction_newline(FLASH_2_HELP_PART6);
	aui_print_help_instruction_newline("\r\n");

#define	FLASH_TIP_HELP		"It takes a long time for the flash test. Please wait with patience until the test finishes."
	//aui_print_help_command("TIPS");
	aui_print_help_instruction("\r\n\033[1mTIPS\033[0m");
	aui_print_help_instruction_newline(FLASH_TIP_HELP);
	return AUI_RTN_HELP;
}

#ifdef AUI_TDS
#define FLASH_SIZE_WR 128 * 1024
int write_norflash_tail(unsigned char *input, unsigned long data_len)
{
	int err = 0;
	aui_hdl flash_handle = 0;
	aui_flash_open_param open_param;
	unsigned long cur_addr = 0x412345;
	unsigned long len = 0;
	unsigned char *buf = NULL;
	unsigned long i = 0;
	aui_flash_info flash_info;
	long flash_size_wr = FLASH_SIZE_WR;

	if (data_len > flash_size_wr) {
		AUI_PRINTF("Data too long, the max len is %#lx!\n", flash_size_wr);
	}

	memset(&open_param, 0, sizeof(aui_flash_open_param));
	err = aui_flash_init(NULL, NULL);
	if (err != AUI_RTN_SUCCESS) {
		return 1;
	}
	open_param.flash_id = 0;
	open_param.flash_type = AUI_FLASH_TYPE_NOR;
	err = aui_flash_open(&open_param, &flash_handle);
	if (err != AUI_RTN_SUCCESS) {
		return 1;
	}
	memset(&flash_info, 0, sizeof(flash_info));
	err = aui_flash_info_get((aui_hdl *)flash_handle, &flash_info);
	//flash_size_wr = flash_info.block_size;
	err = aui_flash_seek(flash_handle, 1 - flash_size_wr, AUI_FLASH_LSEEK_END,
			(unsigned int *)&cur_addr);
	if (err != 0) {
		AUI_PRINTF("FLASH seek fail! %#lX\n", cur_addr);
		return 1;
	}
	buf = malloc(flash_size_wr);
	if (buf == NULL) {
		AUI_PRINTF("malloc fail!\n");
		//return 1;
		AUI_PRINTF("Use more 0x%x of __MM_PVR_VOB_BUFFER_ADDR as tds_upgrade_buf.\n",
				flash_size_wr);
		AUI_PRINTF("__MM_PVR_VOB_BUFFER_LEN = 0x%x\n", __MM_PVR_VOB_BUFFER_LEN);
		buf = (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR;
	}
	err = aui_flash_read(flash_handle, cur_addr, flash_size_wr, &len, buf);
	if (err != 0 || len != (unsigned long)flash_size_wr) {
		AUI_PRINTF("FLASH read fail! %#lX\n", cur_addr);
		if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
			free(buf);
		}
		return 1;
	}

	AUI_PRINTF("Flash tail :\n");
	for (i = 0; i < data_len; i++) {
		AUI_PRINTF("%02X ", buf[flash_size_wr - data_len + i]);
	}
	AUI_PRINTF("\n");

	memcpy(buf + flash_size_wr - data_len, input, data_len);

	//write
	err = aui_flash_seek(flash_handle, 1 - flash_size_wr, AUI_FLASH_LSEEK_END,
			(unsigned int *)&cur_addr);
	if (err != 0) {
		AUI_PRINTF("AUI_FLASH_LSEEK_END fail! %#lX\n", cur_addr);
		if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
			free(buf);
		}
		return 1;
	}
	len = 0;
	err = aui_flash_erase(flash_handle, cur_addr, flash_size_wr);
	err |= aui_flash_write(flash_handle, cur_addr, flash_size_wr, &len, buf);
	if (err != 0 || len != (unsigned long)flash_size_wr) {
		AUI_PRINTF("FLASH tail write fail! %#lX\n", len);
		free(buf);
		return 1;
	}

	// read
	memset(buf, 0, flash_size_wr);
	err = aui_flash_seek(flash_handle, 1 - flash_size_wr, AUI_FLASH_LSEEK_END,
			(unsigned int *)&cur_addr);
	if (err != 0) {
		AUI_PRINTF("AUI_FLASH_LSEEK_END fail! %#lX\n", cur_addr);
		if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
			free(buf);
		}
		return 1;
	}
	len = 0;
	err = aui_flash_read(flash_handle, cur_addr, flash_size_wr, &len, buf);
	if (err != 0 || len != (unsigned long)flash_size_wr) {
		AUI_PRINTF("FLASH read fail! %#lX\n", len);
		if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
			free(buf);
		}
		return 1;
	}
	AUI_PRINTF("Flash tail was filled with:\n");
	for (i = 0; i < data_len; i++) {
		AUI_PRINTF("%02X ", buf[flash_size_wr - data_len + i]);
	}
	AUI_PRINTF("\n");
	if (memcmp(buf + flash_size_wr - data_len, input, data_len)) {
		AUI_PRINTF("TEST FAIL!\n");
	} else {
		AUI_PRINTF("TEST PASS!\n");
	}
	if (buf != (unsigned char *)__MM_PVR_VOB_BUFFER_ADDR) {
		free(buf);
	}
	return 0;
}

unsigned long test_write_norflash_tail(unsigned long *argc, char **argv, char *sz_out_put)
{
	AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
	unsigned char *str = NULL;
	unsigned char buf[3] = {0, 0, 0};
	char *pchar = NULL;
	int i = 0;
	int len = 0;
	int cnt = 0;

	if (*argc < 1) {
		AUI_PRINTF("No input data! Please input HEX string.\n");
		return AUI_RTN_EINVAL;
	}
	len = strlen(argv[0]);
	if (len % 2) {
		len += 1;
	}
	str = malloc(len / 2);
	if (str != NULL) {
		memset(str, 0, len / 2);
	} else {
		AUI_PRINTF("Malloc buffer fail!\n");
		return AUI_RTN_EINVAL;
	}

	cnt = 0;
	for (i = 0; i < len; i++) {
		memcpy(buf, argv[0] + (i * 2), 2);
		str[i] = strtol(buf, &pchar, 16);
		if (!str[i] && pchar == (char *)buf) {
			break;
		}
		cnt++;
	}

	if (cnt > 0) {
		AUI_PRINTF("\ninput:\n");
		for (i = 0; i < cnt; i++) {
			AUI_PRINTF("%02X ", str[i]);
		}
		AUI_PRINTF("\n");
		ret = write_norflash_tail(str, i);
	} else {
		AUI_PRINTF("Input data ERROR! Please input HEX string.\n");
		ret =  AUI_RTN_EINVAL;
	}
	free(str);
	return ret;
}

#endif

unsigned long test_norflash_write_protect(unsigned long *argc, char **argv, char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    aui_hdl flash_handle;
    unsigned char write_data = 0xA5;
    unsigned char write_data2 = 0xB6;
    unsigned char *buf = NULL;
    unsigned char *compare_buf = NULL;
    unsigned long i;
    unsigned long position = 0, size = 0;
    unsigned long return_len;
    aui_flash_open_param open_param;
    unsigned long lock = 0, compare_rlt = 0;	
    char *tailprt = NULL;
    char *usage = \
                "Usage: [3 offset,size,id]\n \
                For example, lock at 0 - 0x40000: [3 0,0x40000,25]\n ";

    AUI_PRINTF("argc=%ld\n", *argc);
    AUI_PRINTF("Enter %s\n", __FUNCTION__);

    if (*argc < 2) {
        AUI_PRINTF("%s\n", usage);
        return AUI_RTN_EINVAL;
    }
    errno = 0;
    position = strtol(argv[0], &tailprt, 0);
    if (errno) {
        AUI_PRINTF("%s\n", usage);
        return AUI_RTN_EINVAL;
    }
    size = strtol(argv[1], &tailprt, 0);
    if (errno) {
        AUI_PRINTF("%s\n", usage);
        return AUI_RTN_EINVAL;
    }
    AUI_PRINTF("lock at: 0x%x, size: 0x%x\n", position, size);

    memset(&open_param, 0, sizeof(aui_flash_open_param));
    if (*argc > 2) 
		open_param.flash_id = strtol(argv[2], &tailprt, 0);
	else
        open_param.flash_id = 0;
    open_param.flash_type = AUI_FLASH_TYPE_NOR;

	flash_stash_partition(open_param.flash_id);

    ret = aui_flash_init(NULL, NULL);
    CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
    ret = aui_flash_open(&open_param, &flash_handle);
    CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

    aui_flash_info flash_info;
    ret = aui_flash_info_get(flash_handle, &flash_info);
    CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
    AUI_PRINTF("\n\nblock_cnt: %d, block_size: %#x, flash_size: %#x, write_size: %#x\n\n",
    		flash_info.block_cnt, flash_info.block_size, flash_info.flash_size, flash_info.write_size);

    //FLASH_SIZE = 3 * (flash_info.block_size);
    buf = MALLOC(size);
    compare_buf = MALLOC(size);

    if ((buf == NULL) || (compare_buf == NULL)) {
        AUI_PRINTF("<%s>,%d,MALLOC failed!\n",
        		__FUNCTION__, __LINE__);
        return AUI_RTN_DRIVER_ERROR;
    }
    
    unsigned long idx = 0;
    for (i = 0; i < 2; i++) {
        // 1. is lock?
        ret = aui_flash_is_lock(flash_handle, position, size, &lock);
        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        AUI_PRINTF("<%s> 1. is lock? %ld, expect lock? %ld\n", __func__, lock, 1-lock);

       
        // 1.1 unlock, lock it
        if (0 == lock) {
            ret = aui_flash_lock(flash_handle, position, size);
            CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
            AUI_PRINTF("<%s> 1.1 lock done.\n", __func__);
        } else if (1 == lock) {
            ret = aui_flash_unlock(flash_handle, position, size);
            CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
            AUI_PRINTF("<%s> 1.1 unlock done.\n", __func__);
        }

        // 1.2 check lock status
        ret = aui_flash_is_lock(flash_handle, position, size, &lock);
        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        AUI_PRINTF("<%s> 1.2 check again, is lock? %ld\n", __func__, lock);
        

        // 2. erase
        AUI_PRINTF("<%s> 2.1 erase at: 0x%x\n", __func__, position);
        ret = aui_flash_erase(flash_handle, position, size);
        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

        // 2.1 check erase result
        AUI_PRINTF("<%s> 2.2 read at: 0x%x\n", __func__, position);
        MEMSET(buf, 0, size);
        ret = aui_flash_read(flash_handle,
                                position,
                                size,
                                &return_len,
                                buf);
        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        if (return_len != size) {
            AUI_PRINTF("<%s>,%d,return len = %d\n", __FUNCTION__, __LINE__);
        }

        AUI_PRINTF("<%s> 2.3 check erase result at: 0x%x\n", __func__, position);
        for (idx = 0; idx < size; idx++) {
            if (0xFF != buf[idx]) {
                if (lock == 0)
                    AUI_PRINTF("<%s> erase fail, not 0xFF! at %d\n", __func__, idx);
                else
                    AUI_PRINTF("<%s> is lock, expected erase fail, not 0xFF! at %d. \n", __func__, idx);
                break;
            } 
        }        

        // 3. write
        AUI_PRINTF("<%s> 3.1 write at: 0x%x\n", __func__, position);
        MEMSET(buf, write_data, size);
        MEMSET(compare_buf, write_data, size);
        ret = aui_flash_write(flash_handle,
                                position,
                                size,
                                &return_len, buf);
        //CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        AUI_PRINTF("ret: %d, lock: %d\n", ret, lock);
        if (ret == AUI_RTN_SUCCESS && lock == 0) {
            AUI_PRINTF("<%s> write success.\n", __FUNCTION__);
        } else if (/*ret != AUI_RTN_SUCCESS && */lock == 1) {
            AUI_PRINTF("<%s> is lock, ignore write result - fit expected.\n", __FUNCTION__);
        } else {
            AUI_PRINTF("<%s> write fail.\n", __FUNCTION__);
            ret = AUI_RTN_FAIL;
            goto OUT;
        }

        if (return_len != size) {
            AUI_PRINTF("<%s>,%d,return len = %d\n", __FUNCTION__, __LINE__);
        }		

        // 4. read & check
        // 4.1 read
        AUI_PRINTF("<%s> 4.1 read at: 0x%x\n", __FUNCTION__, position);
        MEMSET(buf, 0, size);
        ret = aui_flash_read(flash_handle,
                                position,
                                size,
                                &return_len,
                                buf);

        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);

        if (return_len != size) {
            AUI_PRINTF("%s,%d,return len = %d\n", __FUNCTION__, __LINE__);
        }

        // 4.2 compare result
        AUI_PRINTF("<%s> 4.2 compare at: 0x%x\n", __FUNCTION__, position);
        if (MEMCMP(buf, compare_buf, size) != 0) {
            if (lock == 0) {
                compare_rlt |= 1;
                AUI_PRINTF("<%s> %d,error,data error!\n", __FUNCTION__, __LINE__);
            }else
                AUI_PRINTF("<%s> is lock, expected data error - fit expected \n", __func__, idx);
        } 
        // 5 check lock status
        /*ret = aui_flash_is_lock(flash_handle, position, size, &lock);
        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        AUI_PRINTF("<%s> 5 check again, is lock? %ld\n", __func__, lock);*/
        AUI_PRINTF("<%s> ,%d,test 0x%x success\n\n", __FUNCTION__, __LINE__, position);
        write_data = write_data2;
    }

    // unlock the nor flash
    /*if (1 == lock) {
        ret = aui_flash_unlock(flash_handle, position, FLASH_SIZE);
        CHECK_AUI_RET(ret == AUI_RTN_SUCCESS);
        AUI_PRINTF("[%s] unlock done.\n", __func__);
    }*/

OUT:
    aui_flash_close(flash_handle);
    aui_flash_de_init(NULL, NULL);

    FREE(buf);
    buf = NULL;
    FREE(compare_buf);
    compare_buf = NULL;

	flash_stash_pop_partition(open_param.flash_id, position, size);

    return ret|compare_rlt;
}

void flash_test_reg(void)
{
	aui_tu_reg_group("flash", "flash tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_API, test_nor_flash, "test nor flash");
//#ifndef AUI_TDS
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_API, test_nand_flash, "test nand flash");
    aui_tu_reg_item(2, "3", AUI_CMD_TYPE_API, test_norflash_write_protect, "test write protect for nor flash.");

//#endif
#ifdef AUI_TDS
	aui_tu_reg_item(2, "9", AUI_CMD_TYPE_API, test_write_norflash_tail, "Write Hex to the norflash tail.");
#endif

	aui_tu_reg_item(2, "h", AUI_CMD_TYPE_API, test_flash_help, "flash help");
}

