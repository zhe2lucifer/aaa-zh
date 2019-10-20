#include <aui_fs.h>
#include <osal/osal_timer.h>
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fs_main.h>
#include <api/libvfs/vfs.h>
#include <api/libvfs/device.h>
#include "aui_fs_test.h"
#include "aui_test_app_cmd.h"

unsigned long aui_fs_test(unsigned long *argc,char **argv,char *sz_out_put);
void auifs_tb_register(void)
{
	aui_tu_reg_group("fs-bed", "operation on filesystem test-bed");
	aui_tu_reg_item(2, "auto", AUI_CMD_TYPE_API, aui_fs_test, "automatic filesys-op test bed");
}

unsigned long aui_fs_test(unsigned long *argc,char **argv,char *sz_out_put)
{
	int fs_ret = 0;
    aui_f_hdl fs_open_ret = NULL;
	int ecode = SUCCESS;
	DIR *dir = NULL;
	aui_fsdev_info  psdeviceInfo;
	char buf[512] ={0};
	aui_fs_dirent_t *dirent = NULL;

	unsigned int fs_dev = 0;
	unsigned int actual_num=0;
	while((aui_fs_get_alldevid(&fs_dev,1,&actual_num) == AUI_RTN_SUCCESS)&& (fs_dev == 0))
	{
		AUI_PRINTF("Warning: filesystem not mounted,waiting!\n");
		AUI_SLEEP(1000);
		return -1;
	}
	AUI_PRINTF("%s line %d,fs_dev = %d\n",__func__,__LINE__,fs_dev);

	memset(buf, 0, 512);

/****************************************************************************************************/
	AUI_PRINTF("%s line %d, test dir operation.\n",__func__,__LINE__);
	fs_ret = aui_fs_mkdir("/mnt/uda1/test_fs");
	if(fs_ret < 0)
	{
		if(-EEXIST != fs_ret)
		{
		AUI_PRINTF("%s line %d,mkdir failed!fs_ret = %d.\n",__func__,__LINE__,fs_ret);
			return -1;
		}
		else
		{
			AUI_PRINTF("%s line %d, TIP -> dir already exists\n",__func__,__LINE__);
		}
	}

	fs_open_ret = aui_fs_open("/mnt/uda1/test_fs/test1.txt", "w+");
	if(!fs_open_ret)
	{
		AUI_PRINTF("%s line %d,create file failed!fs_open_ret = 0x%08x\n",__func__,__LINE__,fs_open_ret);
		return -1;
	}
	
	dir = aui_fs_open_dir("/mnt/uda1/test_fs");
	if(!dir)
	{
		AUI_PRINTF("%s line %d,open dir failed,dir = %p\n",__func__,__LINE__,dir);
		return -1;
	}

	fs_ret = aui_fs_close_dir(dir);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d,close dir failed,fs_ret = %d\n",__func__,__LINE__,fs_ret);
		return -1;
	}
	
	dir = aui_fs_open_dir("/mnt/uda1/test_fs");

	if(!dir)
	{
		AUI_PRINTF("%s line %d,open dir failed,dir = %p\n",__func__,__LINE__,dir);
		return -1;
	}

	while(1)
	{
		if(NULL != (dirent = aui_fs_read_dir(dir)))
		{
			AUI_PRINTF("    dir content %s\n",dirent->m_c_dir_name);
		}
		else
		{
			break;
		}
	}

	fs_ret = aui_fs_close_dir(dir);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d,close dir failed,fs_ret = %d.\n",__func__,__LINE__,fs_ret);
		return -1;
	}
	

/****************************************************************************************************/
	AUI_PRINTF("%s line %d, get device info test.\n",__func__,__LINE__);
	unsigned int pdwids[32] = {0,};
	int pnactidcnt;
	int i = 0;
	ecode = aui_fs_get_alldevid(pdwids, 32, &pnactidcnt);

	AUI_PRINTF("%s line %d,pnactidcnt = %d\n",__func__,__LINE__,pnactidcnt);

	for(i = 0; i < pnactidcnt; i ++)
	{
		memset(&psdeviceInfo,0x00,sizeof(aui_fsdev_info));
		ecode = aui_fs_get_device_info(pdwids[i], &psdeviceInfo);
		if(ecode != 0)
		{
			AUI_PRINTF("%s line %d, get device info failed!ecode = %d\n",
				__func__,__LINE__,ecode);
			return -1;
		}
		
		AUI_PRINTF("    devid:0x%08x\n"
					"    psdeviceInfo.m_dev_id = 0x%08x\n"
					"    devtype:%s\n"
					"    devname:%s\n"
					"    mntpoint:%s\n"
					"    product_name:%s\n"
					"    manufacturer_name:%s\n"
					"    serial_number_name:%s\n",
					pdwids[0],psdeviceInfo.m_dev_id,(psdeviceInfo.m_dev_type == AUI_FS_DEV_TYPE_UNKNOWN) ? ("unknown") : \
					((psdeviceInfo.m_dev_type == AUI_FS_DEV_TYPE_STORAGE)?  ("storage"):("volume")),psdeviceInfo.m_dev_name,
					psdeviceInfo.m_mount_point,psdeviceInfo.product_name,psdeviceInfo.manufacturer_name,psdeviceInfo.serial_number_name);
		
	}


/****************************************************************************************************/
	AUI_PRINTF("%s line %d,create file and write file test.\n",__func__,__LINE__);
	aui_f_hdl fp = NULL;
	long offset = 0;
	int iter = 0;
		
	fp = aui_fs_open("/mnt/uda1/test_fs/test1.txt", "w+");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(w+)file failed!\n",
			__func__,__LINE__);
		return -1;
	}

	offset = aui_fs_tell(fp);
	if(offset < 0)
	{
		AUI_PRINTF("%s line %d,ftell  failed!offset = %d\n",
			__func__,__LINE__,offset);
		return -1;
	}

	fs_ret = aui_fs_read(fp, buf, 512);
	if(fs_ret < 0)
	{
		AUI_PRINTF("%s line %d,read file failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	AUI_PRINTF("%s line %d print out the reading bytes,count:%d\n",__func__,__LINE__,fs_ret);
	for(iter = 0; iter < fs_ret; iter ++)
	{
		if((iter%16) == 0)
			AUI_PRINTF("    \n0x%04x:",iter);
		AUI_PRINTF("  0x%02x",buf[iter]);
		
	}
	AUI_PRINTF("\n");

	fs_ret = aui_fs_seek(fp, 0, SEEK_SET);

	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,seek file failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	offset = aui_fs_tell(fp);

	if(offset != 0)
	{
		AUI_PRINTF("%s line %d,seek and tell postion are not match,offset = %d\n",
			__func__,__LINE__,offset);
		return -1;
	}

	memset(buf, 1, 512);

	fs_ret = aui_fs_write(fp, buf, 256);

	if(fs_ret != 256)
	{
		AUI_PRINTF("%s line %d, write file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}


	offset = aui_fs_tell(fp);
	if(offset != 256)
	{
		AUI_PRINTF("%s line %d,seek and tell postion are not match,offset should be %d,but actual is %d\n",
			__func__,__LINE__,256,offset);
		return -1;
	}

	fs_ret = aui_fs_flush(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, flush file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	

	fp = aui_fs_open("/mnt/uda1/test_fs/test1.txt", "r");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(r)file failed!\n",
			__func__,__LINE__);
		return -1;
	}

	aui_fs_file_stat_t psfilestat;
	memset(&psfilestat,0x00,sizeof(aui_fs_file_stat_t));
	fs_ret = aui_fs_fsfstate(fp,  &psfilestat);

	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,read file status failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fs_ret = aui_fs_seek(fp, 0, SEEK_SET);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,seek file failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	
	offset = aui_fs_tell(fp);
	if(offset != 0)
	{
		AUI_PRINTF("%s line %d,seek and tell postion are not match,offset should be %d,but actual is %d\n",
			__func__,__LINE__,0,offset);
		return -1;
	}

	fs_ret = aui_fs_read(fp, buf, 256);
	if(fs_ret < 0)
	{
		AUI_PRINTF("%s line %d,read file failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	AUI_PRINTF("%s line %d print out the reading bytes,count:%d\n",__func__,__LINE__,fs_ret);
	for(iter = 0; iter < fs_ret; iter ++)
	{
		if((iter%16) == 0)
			AUI_PRINTF("    \n0x%04x:",iter);
		AUI_PRINTF("  0x%02x",buf[iter]);
		
	}
	AUI_PRINTF("\n");

	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	

	fp = aui_fs_open("/mnt/uda1/test_fs/test1.txt", "w");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(w)file failed!\n",
			__func__,__LINE__);
		return -1;
	}

	fs_ret = aui_fs_seek(fp, 0, SEEK_END);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,seek file failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	offset = aui_fs_tell(fp);
	if(offset != 0)
	{
		AUI_PRINTF("%s line %d,seek and tell postion are not match,offset should be %d,but actual is %d\n",
			__func__,__LINE__,0,offset);
		return -1;
	}

	fs_ret = aui_fs_fsfstate(fp,  &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,read file status failed!\n",
			__func__,__LINE__);
		return -1;
	}

	fs_ret = aui_fs_write(fp, buf, 128);
	if(fs_ret != 128)
	{
		AUI_PRINTF("%s line %d, write file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fs_ret = aui_fs_fsfstate(fp,  &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,read file status failed!\n",
			__func__,__LINE__);
		return -1;
	}

	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fs_ret = aui_fs_remove("/mnt/uda1/test_fs/test1.txt");

	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, delete file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
/****************************************************************************************************/
	AUI_PRINTF("%s line %d, test rename.\n",__func__,__LINE__);
	fp = aui_fs_open("/mnt/uda1/test_fs/test2.txt", "w+");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(w+)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fs_ret = aui_fs_rename("/mnt/uda1/test_fs/test2.txt", "/mnt/uda1/test_fs/testx.c");
	if(fs_ret < 0)
	{
		if(-EEXIST != fs_ret)
		{
		AUI_PRINTF("%s line %d, rename file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
			return -1;
		}
		else
		{
			AUI_PRINTF("%s line %d, WARNING -> exist new name\n", __func__,__LINE__);
		}
	}

	fs_ret = aui_fs_rename("/mnt/uda1/test_fs", "/mnt/uda1/test_fs123");
	if(fs_ret < 0)
	{
		if(-EEXIST != fs_ret)
		{
		AUI_PRINTF("%s line %d, rename file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
			return -1;
		}
		else
		{
			AUI_PRINTF("%s line %d, WARNING -> exist new name\n", __func__,__LINE__);
		}
	}
	
/****************************************************************************************************/
	AUI_PRINTF("%s line %d, test truncated!\n",__func__,__LINE__);
	fs_ret = aui_fs_mkdir("/mnt/uda1/test_fs_for_truncate");
	if(fs_ret < 0)
	{
		if(-EEXIST != fs_ret)
		{
		AUI_PRINTF("%s line %d,mkdir failed!fs_ret = %d.\n",__func__,__LINE__,fs_ret);
			return -1;
		}
		else
		{
			AUI_PRINTF("%s line %d, TIP -> dir already exists\n",__func__,__LINE__);
		}
	}

	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "w+");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(w+)file failed!\n",
			__func__,__LINE__);
		return -1;
	}

	fs_ret = aui_fs_seek(fp, 0, SEEK_SET);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,seek file failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fs_ret = aui_fs_write(fp, buf, 512);
	if(fs_ret != 512)
	{
		AUI_PRINTF("%s line %d, write file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "r");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(r)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_fsfstate(fp,  &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,read file status failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fs_ret = aui_fs_truncated("/mnt/uda1/test_fs_for_truncate/test.txt", 256);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, truncated file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "r");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(r)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_fsfstate(fp,  &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,read file status failed!\n",
			__func__,__LINE__);
		return -1;
	}

	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "w");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(w)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_ftruncated(fp, 1024);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, truncated file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "r");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(r)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_fsfstate(fp,  &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,read file status failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
/************************************************************************************************/
	AUI_PRINTF("%s line %d, fs status test.\n",__func__,__LINE__);
	fs_ret = aui_fs_fsstate("/mnt/uda1/test_fs_for_truncate", &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,fsstate file status failed!fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}

	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "r");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(r)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_fsfstate(fp, &psfilestat);
	if(fs_ret != 0)
	{
		AUI_PRINTF("%s line %d,fsstate file status failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
/*****************************************************************************************************/
	AUI_PRINTF("%s line %d,testfs.\n",__func__,__LINE__);

	aui_fs_fs_status_t psfsstat;
	memset(&psfsstat,0x00,sizeof(aui_fs_fs_status_t));
	fs_ret = aui_fs_statfs("/mnt/uda1/test_fs_for_truncate/test.txt", &psfsstat);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, statfs failed!fs_ret = %d.\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	fs_ret = aui_fs_statfs("/mnt/uda1/test_fs_for_truncate", &psfsstat);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, statfs failed!fs_ret = %d.\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fp = aui_fs_open("/mnt/uda1/test_fs_for_truncate/test.txt", "r");
	if(!fp)
	{
		AUI_PRINTF("%s line %d, open file(r)  failed!\n",
			__func__,__LINE__);
		return -1;
	}
	
	fs_ret = aui_fs_fstatfs(fp, &psfsstat);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, statfs failed!fs_ret = %d.\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	fs_ret = aui_fs_close(fp);
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, close failed!fs_ret = %d.\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
/********************************************************************************************************/
	AUI_PRINTF("%s line %d, test rmdir.\n",__func__,__LINE__);
	fs_ret = aui_fs_mkdir("/mnt/uda1/test_fs_for_rmdir");
	if(fs_ret < 0)
	{
		if(-EEXIST != fs_ret)
		{
		AUI_PRINTF("%s line %d,mkdir failed!fs_ret = %d.\n",__func__,__LINE__,fs_ret);
			return -1;
		}
		else
		{
			AUI_PRINTF("%s line %d, TIP -> dir already exists\n",__func__,__LINE__);
		}
	}
	
	fp = aui_fs_open("/mnt/uda1/test_fs_for_rmdir/test_rm.txt", "w");
	if(!fp)
	{
		AUI_PRINTF("%s line %d,open(w)file failed!\n",
			__func__,__LINE__);
		return -1;
	}
	fs_ret = aui_fs_remove("/mnt/uda1/test_fs_for_rmdir/test_rm.txt");
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, delete file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
	
	fs_ret = aui_fs_rmdir("/mnt/uda1/test_fs_for_rmdir");
	if(fs_ret)
	{
		AUI_PRINTF("%s line %d, rmdir file failed!,fs_ret = %d\n",
			__func__,__LINE__,fs_ret);
		return -1;
	}
/********************************************************************************************************/


	return 0;
}

