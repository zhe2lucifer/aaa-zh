/**  @file		 aui_fs.c
*	 @brief 	A middle layer about ali AUI fs system.
*	 @author	 seiya.cao
*	 @date		   2013-8-22
*	 @version	  1.0.0
*	 @note		   ali corp. all rights reserved. 2013-2999 copyright (C)
*				 for many reasons,ali native fs system	is not POSIX compatiable.this file offer a middle layer implention oriented POSIX semantic.
*				 not completed.
*/
#include <osal/osal_timer.h>
#include <api/libmp/pe.h>
#include <api/libfs2/statvfs.h>
#include <api/libfs2/fs_main.h>
#include <api/libvfs/vfs.h>
#include <api/libvfs/device.h>
#include <bus/usb2/usb.h>
#include "aui_common_priv.h"
#include <aui_fs.h>
/**
*	 @brief 		definition about  globe variable and types.
*	 @author		seiya.cao
*	 @date		2013-8-22
*/
AUI_MODULE(FS)
/**major number definition of AUI removeable device*/
enum
{
    /** major device id of storage device.*/
    AUI_RDI_STORAGE_MAJOR_DEVICEID = 1 << 16,
    /** major device id of volume device.*/
    AUI_RDI_VOLUME_MAJOR_DEVICEID = 2 << 16
};
#define NAME_BUF_LEN (16)
struct dev_info  
{
	unsigned int	 volume_id;
	char		 dev_name[NAME_BUF_LEN];
	unsigned long	 part_start; //in sectors
	unsigned long	 part_size;  //in sectors
	unsigned long part_sector_size; //in bytes
	char		 mnt_path[NAME_BUF_LEN];
	char		 fs_name[NAME_BUF_LEN];
	char 		vol_name[NAME_BUF_LEN];
	unsigned long long	  total_size; //in bytes
	unsigned long long	  free_size;  //in bytes
};


struct aui_fs_dev
{
	long	 valid;
	char	 name[16];
	unsigned char	  fs_num;
	unsigned char	  focus;
	unsigned long	  dev_id;
	unsigned long	  ali_dev_id;
	unsigned long long		  total_size;
	unsigned long long		   free_size;
	struct dev_info 		part[16];
};

typedef enum
{
	AUI_MSG_TYPE_USB_IN = 1,
	AUI_MSG_TYPE_USB_OUT,
}aui_fs_msg_type_t;

typedef struct
{
	aui_fs_msg_type_t msg_type;
	unsigned long msg_code;
	void *callback;
	unsigned long param1;
	unsigned long param2;
}aui_fs_msg_t;

typedef struct
{
	aui_fs_event_callback	  m_fn_callback;
	void *				   m_usercb_data;
}aui_fs_callback_t;

typedef struct aui_file_manager
{
	struct list_head file_list;
	aui_f_hdl pfd;
	int is_dir;
	char mode[4];
	char path[1024];
}aui_ofile_manage_t;
typedef struct aui_fs_manager		
{
	UINT32 dev_id;
	UINT32 vol_id;
	UINT32 dev_num;

	ID mnt_timer_id;
	OSAL_ID aui_fs_mutex;

	char filesys_type[2][16];			   
	struct list_head listpointer;
	struct aui_fs_dev dev[AUI_MAX_DEVICE_NS];
	aui_fs_callback_t callback[AUI_MAX_DEVICE_NS];
}aui_fs_info_t;

struct mbr_partition_record
{
	unsigned char	   status;
	unsigned char	   first_head;
	unsigned short	  first_cyl;
	unsigned char	   type;
	unsigned char	   last_head;
	unsigned short	   last_cyl;
	unsigned int		start;
	unsigned int		 size;
};

typedef void(*aui_fsmount_func)(unsigned long type, unsigned long param);
static aui_fs_info_t *fs_info = NULL;


/**
*	 @brief 		   aui_get_mount_path
*	 @author		   seiya.cao
*	 @date		  2013-8-22
*	 @param[in]    path,file path.
*	 @return		 return mount path of mounted filesystem.
*	 @note		  if input /mnt/uda1/xxxx/xxxx,you will get /mnt/uda1 output.
*
*/

static const char * aui_get_mount_path(char* path)
{
	unsigned char c;
	unsigned char slash_cnt = 0;
	while(*path =='/')
		path ++;

	if(!*path)
		return NULL;

	char * name = path;
	do
	{
		name++;
		c = *(unsigned char *)name;
		if (c == '/')
			slash_cnt ++;
	} while (c && (slash_cnt < 2));

	if(!c)
		return NULL;
	name[0] = '\0';

	return --path;
}


/**
*	 @brief 		   _get_volume_name
*	 @author		   seiya.cao
*	 @date		  2013-8-22
*	 @param[in]    path,file path.
*	 @return		 return mount path of mounted filesystem.
*	 @note		  if input /mnt/uda1/xxxx/xxxx,you will get /mnt/uda1 output.
*						   aui_get_mount_path is a better choice than this.
*/

static void _get_volume_name(const char *path, char *vol_name)
{
	UINT32 i, j = 0;

	for(i = 0; i < STRLEN(path); i++)
	{
		vol_name[i] = path[i];
		if('/' == path[i]) j++;

		if(3 == j) break;
	}

	vol_name[i] = '\0';
}

static void auifs_tmo_handler(UINT32 param)
{
	if(param)
	{
		aui_fs_msg_t msg = *(aui_fs_msg_t *) param;
        AUI_INFO("type: %u, param: 0x%x\n", msg.param1, msg.param2);
		if(msg.callback)
			((aui_fsmount_func)msg.callback)(msg.param1,msg.param2);
	}
}

static void  stop_auifs_timer()
{
	if(OSAL_INVALID_ID != fs_info->mnt_timer_id)
		osal_timer_delete(fs_info->mnt_timer_id);
	fs_info->mnt_timer_id = OSAL_INVALID_ID;
}
static void start_auifs_timer(UINT32 param)
{
	OSAL_T_CTIM 	t_dalm;
	t_dalm.callback = auifs_tmo_handler;
	t_dalm.param = (UINT)param;
	t_dalm.type = TIMER_ALARM;
	t_dalm.time = 300;
	fs_info->mnt_timer_id = osal_timer_create(&t_dalm);
}

static int _update_dev_info(void)
{
	struct dev_info *pinfo = NULL;
	struct device_geometry geo;
	struct statvfs	  stvfs;
	int fd, fdir;
	int i = 0, j = 0;
	int ret;
	char direbuffer[sizeof(struct dirent)+32];
	struct dirent *dire = (struct dirent *)direbuffer;
	int dev_idx;
	int part_idx;
	char dev_path[16]={0};
	int max_dev_num;

	fs_info->dev_num = 0;

	//MEMSET((void*)fs_info->dev, 0x00, AUI_MAX_DEVICE_NS*sizeof(struct aui_fs_dev));

	dev_idx = 0;
	for (i = 0; i < AUI_DEVICE_TYPE_SUPPORT; i++)
	{
		switch (i)
		{
			case 0:
				STRCPY(dev_path, "/dev/hda");
				max_dev_num = AUI_MAX_IDE_DISK_NUM;
				break;
			case 1:
				STRCPY(dev_path, "/dev/sda");
				max_dev_num = AUI_MAX_SD_DISK_NUM;
				break;
			case 2:
				STRCPY(dev_path, "/dev/uda");
				max_dev_num = AUI_MAX_USB_DISK_NUM;
				break;
			default:
				STRCPY(dev_path, "");
				max_dev_num = 0;
				break;
		}

		for (j = 0; j < max_dev_num; j++)
		{
			STRCPY(fs_info->dev[dev_idx].name, dev_path);
			fs_info->dev[dev_idx].name[7] += j;

			fd = fs_open(fs_info->dev[dev_idx].name, 0/*O_RDONLY*/, 0777);
			if(fd < 0)
			{
				continue;
			}

			ret = fs_ioctl(fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry));
			fs_close(fd);
			if(ret < 0)
			{
				MEMSET(fs_info->dev[dev_idx].name,0x00,16);
				continue;
			}

			fs_info->dev[dev_idx].valid = TRUE;
			fs_info->dev[dev_idx].total_size = 0;
			fs_info->dev[dev_idx].free_size = 0;
			STRCPY(fs_info->dev[dev_idx].part[0].dev_name, fs_info->dev[dev_idx].name);
			fs_info->dev[dev_idx].part[0].part_start = 0;
			fs_info->dev[dev_idx].part[0].part_size = geo.sector_count;
			fs_info->dev[dev_idx].part[0].part_sector_size = geo.bytes_per_sector;
			fs_info->dev[dev_idx].part[0].total_size = geo.bytes_per_sector * geo.sector_count;
			fs_info->dev[dev_idx].part[0].free_size = -1LL;
			STRCPY(fs_info->dev[dev_idx].part[0].mnt_path,fs_info->dev[dev_idx].name);
			STRCPY(fs_info->dev[dev_idx].part[0].fs_name,"devfs");
			STRCPY(fs_info->dev[dev_idx].part[0].vol_name,"devfs");
			part_idx = 0; // partitions info start from index 1
			//loop all the /dev to find the partition
			fdir = fs_opendir("/dev");
			while(fs_readdir(fdir, dire) > 0)
			{
				if((MEMCMP(&fs_info->dev[dev_idx].name[5], dire->d_name, 3) == 0) && (STRLEN(dire->d_name) > 3))
				{
					pinfo = &fs_info->dev[dev_idx].part[part_idx+1];// 1st part is total info
					//MEMSET(pinfo, 0, sizeof(struct dev_info));
					STRCPY(pinfo->dev_name, "/dev/");
					STRCPY(pinfo->mnt_path, "/mnt/");
					strncat(pinfo->mnt_path, dire->d_name, NAME_BUF_LEN-1);
					strncat(pinfo->dev_name, dire->d_name, NAME_BUF_LEN-1);

					fd = fs_open(pinfo->dev_name, 0/*O_RDONLY*/, 0777);
					ret = fs_ioctl(fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry));
					fs_close(fd);

					pinfo->part_start = geo.start_sector;
					//pinfo->part_size = geo.sector_count;
					pinfo->part_sector_size = geo.bytes_per_sector;

					ret = fs_statvfs(pinfo->mnt_path, &stvfs);
					if(ret < 0)
					{
						pinfo->mnt_path[0] = 0;
					}
					else
					{
						if(STRCMP(pinfo->dev_name, stvfs.f_device_name) != 0)
						{
							pinfo->mnt_path[0] = 0;
						}
						else
						{
							STRCPY(pinfo->fs_name, stvfs.f_fsh_name);
							STRCPY(pinfo->vol_name,stvfs.f_volume_name);
							pinfo->total_size = (unsigned long long)stvfs.f_blocks * stvfs.f_frsize;
							pinfo->free_size  = (unsigned long long)stvfs.f_bfree * stvfs.f_frsize;
							pinfo->part_size = pinfo->total_size/pinfo->part_sector_size;
							fs_info->dev[dev_idx].total_size += pinfo->total_size;
							fs_info->dev[dev_idx].free_size += pinfo->free_size;
						}
					}
					part_idx++;
				}
			}
			fs_closedir(fdir);
			fs_info->dev[dev_idx].fs_num = part_idx;
			fs_info->dev[dev_idx].focus = 1;
			dev_idx++;
		};
	}
	fs_info->dev_num = dev_idx;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
			list_del(ptr);
			osal_mutex_unlock(fs_info->aui_fs_mutex);
			FREE(file_info);
			file_info = NULL;
		}
	}

	return 0;
}

static void aui_update_fs_msg(UINT32 event_type, UINT32 param)		   // callback
{
	unsigned int i = 0, j = 0, k = 0;
    UINT32 dev_no = 0;
//	UINT32 dev_type 	= MNT_DEV_TYPE(param);
	UINT32 dev_id		= MNT_DEV_ID(param);
//	UINT32 mount_status = MNT_STATUS(param);

    AUI_INFO("type: %u, param: 0x%x, ali_dev_id: %u\n", event_type, param, dev_id);
	switch (event_type)
	{
		case MP_FS_MOUNT:
			_update_dev_info();
            dev_no=dev_id;
			fs_info->dev[dev_no].dev_id = ++fs_info->dev_id;
			fs_info->dev[dev_no].ali_dev_id = dev_id;

			for( j = 0; j < fs_info->dev[dev_no].fs_num; j++)
			{
				fs_info->dev[dev_no].part[j+1].volume_id = ++fs_info->vol_id;
			}

			for(i = 0; i < AUI_MAX_DEVICE_NS; i++)
			{
				if(NULL != fs_info->callback[i].m_fn_callback)
				{
					fs_info->callback[i].m_fn_callback(AUI_FS_EVENT_FOUND, \
					fs_info->dev[dev_no].dev_id, NULL, fs_info->callback[i].m_usercb_data);
					fs_info->callback[i].m_fn_callback(AUI_FS_EVENT_READY, \
					fs_info->dev[dev_no].dev_id, NULL, fs_info->callback[i].m_usercb_data);
					
					for(k = 0; k<fs_info->dev[dev_no].fs_num; k++)
					{
						fs_info->callback[i].m_fn_callback(AUI_FS_EVENT_FOUND, \
						fs_info->dev[dev_no].part[k+1].volume_id, NULL, fs_info->callback[i].m_usercb_data);
						fs_info->callback[i].m_fn_callback(AUI_FS_EVENT_READY, \
						fs_info->dev[dev_no].part[k+1].volume_id, NULL, fs_info->callback[i].m_usercb_data);
					}
				}
			}
			break;

		case MP_FS_UNMOUNT:
			for( i = 0; i < fs_info->dev_num; i++ )
			{
				if(dev_id == fs_info->dev[i].ali_dev_id)
					break;
			}
			for(j = 0; j < AUI_MAX_DEVICE_NS; j++)
			{
				if(NULL != fs_info->callback[j].m_fn_callback)
				{
					for(k = 0; k<fs_info->dev[i].fs_num; k++)
					{
						fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_PLUGOUT, fs_info->dev[i].part[k+1].volume_id,\
							NULL, fs_info->callback[j].m_usercb_data);
					}
					
					fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_PLUGOUT, \
					fs_info->dev[i].dev_id, NULL, fs_info->callback[j].m_usercb_data);
				}
			}
			
			if( i < fs_info->dev_num)
				_update_dev_info();
			break;
		default:
			break;
	}
}
static void aui_fs_callback(UINT32 event_type, UINT32 param)
{
	static aui_fs_msg_t msg;
    AUI_INFO("type: %u, param: 0x%x\n", event_type, param);
	switch (event_type)
	{
		case MP_FS_MOUNT:
		case MP_FS_UNMOUNT:
			msg.msg_type = event_type;
			msg.callback = (void *)aui_update_fs_msg;
			msg.param1 = event_type;
			msg.param2 = param;
			stop_auifs_timer();
			start_auifs_timer((UINT32)&msg);
			break;
		default:
			break;
	}
    AUI_INFO("out -> type: %u, param: 0x%x\n", msg.param1, msg.param2);
}


AUI_RTN_CODE aui_fs_init(void)
{
	int ret = 0;

	if(fs_info)
	{
		AUI_ERR("invalid arg\n");
		ret = 0;
		goto  RETURN;
	}
	if(( fs_info = MALLOC(sizeof(aui_fs_info_t)))  ==NULL)
	{
		AUI_ERR("malloc fail\n");
		ret = -1;
		goto RETURN;
	}

	MEMSET((void*)fs_info,0x00,sizeof(aui_fs_info_t));

	fs_info->dev_id = AUI_RDI_STORAGE_MAJOR_DEVICEID;
	fs_info->vol_id = AUI_RDI_VOLUME_MAJOR_DEVICEID;
	fs_info->mnt_timer_id = OSAL_INVALID_ID;
	fs_info->aui_fs_mutex = osal_mutex_create();

	if(fs_info->aui_fs_mutex == OSAL_INVALID_ID )
	{
		AUI_ERR("create mutex fail\n");
		ret = -1;
		FREE(fs_info);
		fs_info = NULL;
		goto RETURN;
	}
	STRCPY(fs_info->filesys_type[0], "FAT");
	STRCPY(fs_info->filesys_type[1], "NTFS");
	INIT_LIST_HEAD(&fs_info->listpointer);

	fs_init(aui_fs_callback);
RETURN:
	return ret;
}

aui_f_hdl aui_fs_open(const char * aui_file_name, const char * aui_file_mode)
{
	FILE *fp = NULL;
	aui_ofile_manage_t *file_info = NULL;
	aui_f_hdl ret = NULL;
	int fs_ret = 0;
	struct stat st;
	
	if((!aui_file_name) || (!aui_file_mode))
	{
		AUI_ERR("invalid arg\n");
		return NULL;
	}
	if(STRLEN(aui_file_name) > 512)
	{
		AUI_ERR("name longer 512\n");
		return NULL;
	}

	fs_ret = fs_stat(aui_file_name, &st);

	if(fs_ret < 0)
	{
		st.st_mode = 0;
	}

	if(S_ISDIR(st.st_mode))
	{
		return ret;
	}

	fp = fopen(aui_file_name, aui_file_mode);
	if(!fp)
	{
		AUI_ERR("invalid arg\n");
		return  NULL;
	}

	file_info = MALLOC(sizeof(aui_ofile_manage_t));
	if(!file_info)
	{
		AUI_ERR("malloc fail\n");
        fclose(fp);
		return NULL;
	}

	file_info->is_dir = 0;
	file_info->pfd = (aui_f_hdl)fp;
	STRCPY(file_info->mode, aui_file_mode);
	STRCPY(file_info->path, aui_file_name);
	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	list_add_tail(&file_info->file_list, &fs_info->listpointer);
	osal_mutex_unlock(fs_info->aui_fs_mutex);
	
	ret = (aui_f_hdl)fp;
	return ret;
}

AUI_RTN_CODE aui_fs_write(aui_f_hdl aui_h_file,char * pc_buf, unsigned int u_count)
{
	int ret = 0;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;

	if((!pc_buf) || (!aui_h_file))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}


	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);
	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	ret = fwrite((const void*)pc_buf, 1, u_count, (FILE*)aui_h_file);
	if(ret <= 0)
	{
		AUI_ERR("write fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_read(aui_f_hdl aui_h_file,char * pc_buf, unsigned int u_count)
{
	int ret = 0;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;
	FILE *fp;

	if((!pc_buf) || (!aui_h_file))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	fp = (FILE*)aui_h_file;
	if( (fp->_flags & __SRD) == 0)
	{
		if((fp->_flags & __SRW) == 0)
		{
			AUI_ERR("not permit\n");
			ret = -1;
			goto RETURN;
		}
	}

	ret = fread((void*)pc_buf, 1, u_count, (FILE*)aui_h_file);

	if(0 == ret)
	{
		if(0 == STRCMP(file_info->mode, "a+"))
		{
			ret = -1;
		}
	}

RETURN:
	return ret;
}
 AUI_RTN_CODE aui_fs_close(aui_f_hdl aui_h_file)
{
	int ret = 0;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				list_del(ptr);
				FREE(file_info);
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);
	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	ret = fclose((FILE*)aui_h_file);
RETURN:
	return ret;
}


AUI_RTN_CODE aui_fs_flush(aui_f_hdl aui_h_file)
{
	int ret = 0;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	ret = fflush((FILE*)aui_h_file);
	if(ret < 0)
	{
		AUI_ERR("flush fail\n");
		ret = -1;
		goto RETURN;
	}

	int sync_ret = fsync(file_info->path);
	if(sync_ret < 0)
	{
		AUI_ERR("sync fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}

aui_f_hdl aui_fs_open_dir(const char * aui_path_name)
{
	DIR *ret = NULL;
	aui_ofile_manage_t *file_info = NULL;

	if(!aui_path_name)
	{
		AUI_ERR("invalid arg\n");
		ret = NULL;
		goto RETURN;
	}

	ret = fopendir(aui_path_name);
	if(!ret)
	{
		AUI_ERR("open fail\n");
		ret = NULL;
		goto RETURN;
	}

	file_info = MALLOC(sizeof(aui_ofile_manage_t));
	if(NULL==file_info)
	{
	    AUI_ERR("malloc fail\n");
		ret = NULL;
		goto RETURN;
	}
	
	file_info->is_dir = 1;
	file_info->pfd = (aui_f_hdl)ret;
	STRCPY(file_info->path, aui_path_name);
	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	list_add_tail(&file_info->file_list, &fs_info->listpointer);
	osal_mutex_unlock(fs_info->aui_fs_mutex);

RETURN:
	return (aui_f_hdl)ret;
}


AUI_RTN_CODE aui_fs_rewind_dir(aui_f_hdl aui_hdir_file)
{
	int ret = 0;
	struct list_head *ptr, *ptn;
	BOOL dir_exist = FALSE;
	aui_ofile_manage_t *file_info = NULL;

	if(!aui_hdir_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_hdir_file) && (file_info->is_dir == 1))
			{
				dir_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!dir_exist)
	{
		AUI_ERR("dir not exist\n");
		ret = -1;
		goto RETURN;
	}

	DIR *dir = (DIR*)aui_hdir_file;
	ret = frewinddir(dir);
	if(ret < 0)
	{
		AUI_ERR("rewind fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}

static BOOL _dir_skip_system_file(char *dirname)
{
	if( (!STRCMP(dirname, "recycled")) ||
			(!STRCMP(dirname, "system volume information")) ||
			(!STRCMP(dirname, "FOUND.000")) ||
			(!STRCMP(dirname, "$RECYCLE.BIN")) ||
			(!STRCMP(dirname, "RECYCLER")))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


aui_fs_dirent_t * aui_fs_read_dir(aui_f_hdl aui_h_dir)
{
	static aui_fs_dirent_t dir_info;
	f_dirent dent;
	aui_fs_dirent_t *ret = NULL;
	int fs_ret = 0;
	struct list_head *ptr, *ptn;
	BOOL dir_exist = FALSE;
	aui_ofile_manage_t *file_info = NULL;

	if(!aui_h_dir)
	{
		AUI_ERR("invalid arg\n");
		ret = NULL;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_dir) && (file_info->is_dir == 1))
			{
				dir_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!dir_exist)
	{
		AUI_ERR("dir not exit\n");
		ret = NULL;
		goto RETURN;
	}

	DIR *dir = (DIR*)aui_h_dir;

	do{
		fs_ret = freaddir(dir, &dent);
	}while((fs_ret > 0) && (_dir_skip_system_file(dent.name)));  // skip system file

	if(fs_ret <= 0)
	{
	    //AUI_ERR("readdir fail\n");
		ret = NULL;
		goto RETURN;
	}

	dir_info.m_uc_dir_type = dent.is_dir;
	//STRCPY(dir_info.m_c_dir_name, dent.name);
	strncpy(dir_info.m_c_dir_name, dent.name, sizeof(dir_info.m_c_dir_name)-1);
	ret = &dir_info;

RETURN:
	return ret;
}
AUI_RTN_CODE aui_fs_close_dir(aui_f_hdl aui_h_dir)
{
	int ret = 0;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL dir_exist = FALSE;

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_dir) && (file_info->is_dir == 1))
			{
				dir_exist = TRUE;
				list_del(ptr);
				FREE(file_info);
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!dir_exist)
	{
		AUI_ERR("dir not exist\n");
		ret = -1;
		goto RETURN;
	}

	ret = fclosedir((DIR*)aui_h_dir);

	if(ret < 0)
	{
		AUI_ERR("close dir fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}


AUI_RTN_CODE aui_fs_rmdir(const char * aui_path_name)
{
	int ret = 0, fs_ret = 0;

	fs_ret = frmdir(aui_path_name);
	if (fs_ret < 0)
	{
	    AUI_ERR("rm dir fail\n");
		ret = -1;
		goto RETURN;
	}
    char * mnt_path = MALLOC(STRLEN(aui_path_name)+1);
    if(!mnt_path)
	{
	    AUI_ERR("malloc fail\n");
		ret = -1;
		goto RETURN;
	}
	MEMSET(mnt_path,0x00,STRLEN(aui_path_name)+1);
	STRCPY(mnt_path,aui_path_name);
	const char *mnt_name =  aui_get_mount_path(mnt_path);
	if(!mnt_name)
	{
	    AUI_ERR("unknown fail\n");
		ret = -1;
		FREE(mnt_path);
		goto RETURN;
	}

	int sync_ret =  fsync(mnt_name);
	if(sync_ret < 0)
	{
	    AUI_ERR("sync fail\n");
		ret = -1;
		FREE(mnt_path);
		goto RETURN;
	}
    FREE(mnt_path);
RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_mkdir(const char * aui_path_name)
{
	int ret = 0, fs_ret = 0;

	if(!aui_path_name)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}
	fs_ret = fmkdir(aui_path_name);
	if(fs_ret < 0)
	{
        if(-EEXIST == fs_ret)
        {           
            return fs_ret;
        }
		AUI_ERR("mkdir fail\n");
		ret = -1;
		goto RETURN;
	}

	int sync_ret = fsync(aui_path_name);

	if(sync_ret < 0)
	{
		AUI_ERR("sync fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_seek(aui_f_hdl aui_h_file, long l_offset, unsigned int u_origin)
{
	int ret;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;
	FILE *fp;
	long long offset = (long long)l_offset;

	if(!aui_h_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	fp = (FILE*)aui_h_file;

	ret = fseek(fp, offset, (int)u_origin);

	if(ret < 0)
	{
	    AUI_ERR("seek fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_lseek(aui_f_hdl aui_h_file,signed long long i64offset,unsigned int u_origin)
{
	int ret;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;
	FILE *fp;
	long long offset = (long long)i64offset;

	if(!aui_h_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	fp = (FILE*)aui_h_file;

	ret = fseek(fp, offset, (int)u_origin);

	if(ret < 0)
	{
	    AUI_ERR("seek fail\n");
		ret = -1;
		goto RETURN;
	}

RETURN:
	return ret;
}

signed long aui_fs_tell(aui_f_hdl aui_h_file)
{
	long long pos;
	long ret;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;

	if(!aui_h_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	pos = ftell((FILE*)aui_h_file);

	if(pos > (long long)0x7FFFFFFF)
	{
		AUI_ERR("unknown fail\n");
		ret = -1;
		goto RETURN;
	}
	ret = (long)(pos&0x00000000FFFFFFFF);

RETURN:
	return ret;
}

long long aui_fs_ltell(aui_f_hdl aui_h_file)
{
	long long ret = 0;
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL file_exist = FALSE;

	if(!aui_h_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	ret = ftell((FILE*)aui_h_file);
RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_remove(const char * aui_file_name)
{
	int ret = 0;
	if (! aui_file_name)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}
	ret = fs_remove(aui_file_name);

	if (ret < 0)
	{
		AUI_ERR("remove fail\n");
		ret = -1;
		goto RETURN;
	}
	char * mnt_path = MALLOC(STRLEN(aui_file_name)+1);

	if(!mnt_path)
	{
		AUI_ERR("malloc fail\n");
		ret = -1;
		goto RETURN;
	}
	MEMSET(mnt_path,0x00,STRLEN(aui_file_name)+1);
	STRCPY(mnt_path,aui_file_name);
	const char *mnt_name =  aui_get_mount_path(mnt_path);
	
	if(!mnt_name)
	{
		AUI_ERR("unknown error\n");
		ret = -1;
		FREE(mnt_path);
		goto RETURN;
	}

	int sync_ret =  fsync(mnt_name);

	if(sync_ret < 0)
	{
		AUI_ERR("sync fail\n");
		ret = -1;
		FREE(mnt_path);
		goto RETURN;
	}
	FREE(mnt_path);
	
RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_fsstate(const char * aui_file_name, aui_fs_file_stat_t * aui_file_stat)
{
	struct stat st;
	int ret = 0, fs_ret = 0;
	BOOL dir_flag;

	if((!aui_file_name) || (!aui_file_stat))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	MEMSET(aui_file_stat, 0, sizeof(aui_fs_file_stat_t));

	fs_ret = fs_stat(aui_file_name, &st);

	if(fs_ret < 0)
	{
		AUI_ERR("stat fail\n");
		ret = -1;
		goto RETURN;
	}

	dir_flag = S_ISDIR(st.st_mode);
	if(dir_flag)
		aui_file_stat->m_isdir = 1;
	else
		aui_file_stat->m_isdir = 0;

	aui_file_stat->m_l_size = st.st_size;
	aui_file_stat->m_l_blk_size = st.st_blksize;
	aui_file_stat->m_l_blk_count = st.st_blocks;
	aui_file_stat->m_l_atime = st.st_atime;
	aui_file_stat->m_l_mtime = st.st_mtime;
	aui_file_stat->m_l_ctime = st.st_ctime;


RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_fsfstate(aui_f_hdl aui_h_file, aui_fs_file_stat_t * aui_ps_file_stat)
{
	struct stat st;
	int ret = 0, fs_ret = 0;
	FILE *fp;
	struct list_head *ptr, *ptn;
	aui_ofile_manage_t *file_info = NULL;
	BOOL file_exist = FALSE;

	if((!aui_h_file)||(!aui_ps_file_stat))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	MEMSET(aui_ps_file_stat, 0, sizeof(aui_fs_file_stat_t));

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if( !file_exist)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	fp = (FILE*)aui_h_file;
	fs_ret = fs_fstat(fp->_file, &st);

	if(fs_ret < 0)
	{
		AUI_ERR("fstat fail\n");
		ret = -1;
		goto RETURN;
	}

	aui_ps_file_stat->m_isdir = 0;
	aui_ps_file_stat->m_l_size = st.st_size;
	aui_ps_file_stat->m_l_blk_size = st.st_blksize;
	aui_ps_file_stat->m_l_blk_count = st.st_blocks;
	aui_ps_file_stat->m_l_atime = st.st_atime;
	aui_ps_file_stat->m_l_mtime = st.st_mtime;
	aui_ps_file_stat->m_l_ctime = st.st_ctime;

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_rename(const char * aui_old_path, const char * aui_new_path)
{
	int ret = 0;
	int fs_ret = 0;
	struct stat st;

	if((!aui_old_path)||(!aui_new_path))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	fs_ret = fs_stat(aui_old_path, &st);
	if(fs_ret < 0)
	{
	    AUI_ERR("stat 1 fail\n");
		ret = -1;
		goto RETURN;
	}

	fs_ret = fs_stat(aui_new_path, &st);

	if(fs_ret >= 0)
	{
	    AUI_ERR("stat 2 fail\n");
		ret = -EEXIST;
		goto RETURN;
	}

	ret = frename(aui_old_path, aui_new_path);

	if(ret < 0)
	{
	    AUI_ERR("rename fail\n");
		ret = -1;
		goto RETURN;
	}
    char * old_mnt_path = MALLOC(STRLEN(aui_old_path)+1);
	if(!old_mnt_path)
	{
	    AUI_ERR("malloc fail\n");
		ret = -1;
		goto RETURN;
	}
	MEMSET(old_mnt_path,0x00,STRLEN(aui_old_path)+1);
	STRCPY(old_mnt_path,aui_old_path);
	const char *old_mnt_name =  aui_get_mount_path(old_mnt_path);
	if(!old_mnt_name)
	{
	    AUI_ERR("unknown fail\n");
		ret = -1;
		FREE(old_mnt_path);
		goto RETURN;
	}
	fsync(old_mnt_name);
    FREE(old_mnt_path);
    char * new_mnt_path = MALLOC(STRLEN(aui_new_path)+1);
	if(!new_mnt_path)
	{
	    AUI_ERR("malloc fail\n");
		ret = -1;
		goto RETURN;
	}
	MEMSET(new_mnt_path,0x00,STRLEN(aui_new_path)+1);
	STRCPY(new_mnt_path,aui_new_path);
	const char *new_mnt_name =  aui_get_mount_path(new_mnt_path);
	if(!new_mnt_name)
	{
	    AUI_ERR("unknown fail\n");
		ret = -1;
		FREE(new_mnt_path);
		goto RETURN;
	}
	fsync(new_mnt_name);
    FREE(new_mnt_path);

RETURN:
	return ret;
}

#define WRITE_BUF_SIZE	  0x10000
AUI_RTN_CODE aui_fs_truncated(const char * aui_path_name, unsigned long aui_ul_length)
{
	struct stat st;
	FILE *fp =NULL;
	char *buf;
	long write_size;
	int ret = 0;
	int fs_ret = 0;

	if(!aui_path_name)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	fs_ret = fs_stat(aui_path_name, &st);

	if(fs_ret < 0)
	{
		AUI_ERR("stat fail\n");
		ret = -1;
		goto RETURN;
	}


	if(S_ISDIR(st.st_mode))
	{
		AUI_ERR("not permit\n");
		ret = -1;
		goto RETURN;
	}

	if(st.st_size >= aui_ul_length)
	{
		st.st_size = (long long)aui_ul_length;
		fs_wstat(aui_path_name, &st, WSTAT_SIZE);
	}
	else
	{

		write_size = aui_ul_length -(unsigned long)st.st_size;
		buf = MALLOC(WRITE_BUF_SIZE);
		if(!buf)
		{
			AUI_ERR("malloc fail\n");
			ret = -1;
			goto RETURN;
		}

		MEMSET(buf, 0, WRITE_BUF_SIZE);

		fp = fopen(aui_path_name, "a+");
		if(!fp)
		{
			AUI_ERR("open fail\n");
			ret = -1;
			goto RETURN;
		}

		fs_ret = fseek(fp, st.st_size, SEEK_SET);

		if(fs_ret < 0)
		{
			AUI_ERR("seek fail\n");
			ret = -1;
			goto RETURN;
		}


		while(write_size > 0)
		{
			if(write_size < WRITE_BUF_SIZE)
			{
				fs_ret = fwrite(buf, 1, write_size, fp);
				break;
			}
			else
			{
				fwrite(buf, 1, WRITE_BUF_SIZE, fp);
				write_size -= WRITE_BUF_SIZE;
			}
		}

		int flush_ret = fflush((FILE*)fp);
		if(flush_ret < 0)
		{
			AUI_ERR("flush fail\n");
			ret = -1;
			goto RETURN;
		}

		int fsync_ret = fsync(aui_path_name);
		if(fsync_ret < 0)
		{
			AUI_ERR("sync fail\n");
			ret = -1;
			goto RETURN;
		}

		if(NULL != buf)
		{
			FREE(buf);
			buf = NULL;
		}
	}

RETURN:
	if(fp !=NULL)
	{
		fclose(fp);
	}
	return ret;
}

AUI_RTN_CODE aui_fs_ftruncated(aui_f_hdl aui_h_file, unsigned long aui_ul_length)
{
	struct stat st;
	FILE *fp;
	char *buf;
	long write_size;
	struct list_head *ptr, *ptn;
	aui_ofile_manage_t *file_info = NULL;
	int ret = 0;
	BOOL file_exist = FALSE;

	if(!aui_h_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	fp = (FILE*)aui_h_file;

	int flush_ret = fflush(fp);

	if(flush_ret < 0)
	{
		AUI_ERR("flush fail\n");
		ret = -1;
		goto RETURN;
	}

	int fstat_ret = fs_stat(file_info->path, &st);

	if(fstat_ret < 0)
	{
		AUI_ERR("stat fail\n");
		ret = -1;
		goto RETURN;
	}

	if(st.st_size >= aui_ul_length)
	{
		st.st_size = (off_t)aui_ul_length;
		fs_wstat(file_info->path, &st, WSTAT_SIZE);
	}
	else
	{
		write_size = aui_ul_length -(unsigned long)st.st_size;
		buf = (char*)MALLOC(WRITE_BUF_SIZE);

		if(!buf)
		{
			AUI_ERR("malloc fail\n");
			ret = -1;
			goto RETURN;
		}

		MEMSET(buf, 0, WRITE_BUF_SIZE);
		fseek(fp, st.st_size, SEEK_SET);
		while(write_size > 0)
		{
			if(write_size < WRITE_BUF_SIZE)
			{
				fwrite(buf, 1, write_size, fp);
				break;
			}
			else
			{
				fwrite(buf, 1, WRITE_BUF_SIZE, fp);
				write_size -= WRITE_BUF_SIZE;
			}
		}

		int flush_ret = fflush((FILE*)fp);
		if(flush_ret < 0)
		{
			AUI_ERR("fflush fail\n");
			ret = -1;
			goto RETURN;
		}

		int fsync_ret = fsync(file_info->path);
		if(fsync_ret < 0)
		{
			AUI_ERR("sync fail\n");
			ret = -1;
			goto RETURN;
		}

		if(NULL != buf)
		{
			FREE(buf);
			buf = NULL;
		}
	}

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_ltruncated(const char * aui_path_name,unsigned long long u64length)
{
	struct stat st;
	FILE *fp = NULL;
	char *buf;
	long write_size;
	int ret = 0;
	int fs_ret = 0;

	if(!aui_path_name)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	fs_ret = fs_stat(aui_path_name, &st);

	if(fs_ret < 0)
	{
		AUI_ERR("stat fail\n");
		ret = -1;
		goto RETURN;
	}

	if(S_ISDIR(st.st_mode))
	{
		AUI_ERR("not permit\n");
		ret = -1;
		goto RETURN;
	}

	if(st.st_size >= (long long)u64length)
	{
		st.st_size = (long long)u64length;
		fs_wstat(aui_path_name, &st, WSTAT_SIZE);
	}
	else
	{
		write_size = u64length -(unsigned long long)st.st_size;
		buf = (char*)MALLOC(WRITE_BUF_SIZE);
		if(!buf)
		{
			AUI_ERR("malloc fail\n");
			ret = -1;
			goto RETURN;
		}

		MEMSET(buf, 0, WRITE_BUF_SIZE);
		fp = fopen(aui_path_name, "a+");

		if(!fp)
		{
			AUI_ERR("open fail\n");
			ret = -1;
			goto RETURN;
		}

		fs_ret = fseek(fp, st.st_size, SEEK_SET);

		if(fs_ret < 0)
		{
			AUI_ERR("seek fail\n");
			ret = -1;
			fclose(fp);
			goto RETURN;
		}
		while(write_size > 0)
		{
			if(write_size < WRITE_BUF_SIZE)
			{
				fwrite(buf, 1, write_size, fp);
				break;
			}
			else
			{
				fwrite(buf, 1, WRITE_BUF_SIZE, fp);
				write_size -= WRITE_BUF_SIZE;
			}
		}

		int flush_ret = fflush((FILE*)fp);
		if(flush_ret < 0)
		{
			AUI_ERR("flush fail\n");
			ret = -1;
			fclose(fp);
			goto RETURN;
		}

		int fsync_ret = fsync(aui_path_name);
		if(fsync_ret < 0)
		{
			AUI_ERR("sync fail\n");
			ret = -1;
			fclose(fp);
			goto RETURN;
		}

		if(NULL != buf)
		{
			FREE(buf);
			buf = NULL;
		}
	}
	if(NULL != fp)
	{
		fclose(fp);
	}

RETURN:

	return ret;
}


AUI_RTN_CODE aui_fs_fltruncated(aui_f_hdl aui_h_file,unsigned long long u64length)
{
	struct stat st;
	FILE *fp;
	char *buf;
	long write_size;
	struct list_head *ptr, *ptn;
	aui_ofile_manage_t *file_info = NULL;
	int ret  = 0;
	BOOL file_exist = FALSE;

	if(!aui_h_file)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	fp = (FILE*)aui_h_file;
	int fs_ret = fflush(fp);

	if(fs_ret < 0)
	{
		AUI_ERR("flush fail\n");
		ret = -1;
		goto RETURN;
	}

	fs_ret = fs_stat(file_info->path, &st);

	if(fs_ret < 0)
	{
		AUI_ERR("stat fail\n");
		ret = -1;
		goto RETURN;
	}

	if(st.st_size >= (long long)u64length)
	{
		st.st_size = (long long)u64length;
		fs_wstat(file_info->path, &st, WSTAT_SIZE);
	}
	else
	{
		write_size = u64length -(unsigned long)st.st_size;
		buf = (char*)MALLOC(WRITE_BUF_SIZE);
		if(!buf)
		{
			AUI_ERR("malloc fail\n");
			ret = -1;
			goto RETURN;
		}

		MEMSET(buf, 0, WRITE_BUF_SIZE);
		fs_ret = fseek(fp, st.st_size, SEEK_SET);

		if(fs_ret < 0)
		{
			AUI_ERR("seek fail\n");
			ret = -1;
			goto RETURN;
		}

		while(write_size > 0)
		{
			if(write_size < WRITE_BUF_SIZE)
			{
				fwrite(buf, 1, write_size, fp);
				break;
			}
			else
			{
				fwrite(buf, 1, WRITE_BUF_SIZE, fp);
				write_size -= WRITE_BUF_SIZE;
			}
		}

		fs_ret = fflush(fp);
		if(fs_ret < 0)
		{
			AUI_ERR("flush fail\n");
			ret = -1;
			goto RETURN;
		}

		fs_ret = fsync(file_info->path);
		if(fs_ret < 0)
		{
			AUI_ERR("sync fail\n");
			ret = -1;
			goto RETURN;
		}

		if(NULL != buf)
		{
			FREE(buf);
			buf = NULL;
		}
	}

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_statfs(const char * aui_path_name, aui_fs_fs_status_t * ps_fs_stat)
{
	char vol_name[16];
	struct statvfs stfs;
	int ret = 0;
	int fs_ret = 0;
	struct stat st;

	if((!aui_path_name)||(!ps_fs_stat))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	fs_ret = fs_stat(aui_path_name, &st);
	if(fs_ret < 0)
	{
		AUI_ERR("stat fail\n");
		ret = -1;
		goto RETURN;
	}

	MEMSET(ps_fs_stat, 0, sizeof(aui_fs_fs_status_t));

	_get_volume_name(aui_path_name, vol_name);

	ret = fs_statvfs(vol_name, &stfs);

	if(ret < 0)
	{
		AUI_ERR("stat vfs fail\n");
		ret = -1;
		goto RETURN;
	}

	if (strncmp (stfs.f_fsh_name,"FAT32",5) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_FAT32;
	else if(strncmp (stfs.f_fsh_name,"FAT16",5) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_FAT16;
	else if(strncmp (stfs.f_fsh_name,"FAT12",5) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_FAT12;	
	else if(strncmp (stfs.f_fsh_name,"NTFS",4) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_NTFS;

	ps_fs_stat->m_l_bsize = stfs.f_frsize;
	ps_fs_stat->m_l_blocks = stfs.f_blocks;
	ps_fs_stat->m_l_bfree = stfs.f_bfree;
	ps_fs_stat->m_l_bavail = stfs.f_favail;
	ps_fs_stat->m_l_files = stfs.f_files;
	ps_fs_stat->m_l_ffree = stfs.f_ffree;
	ps_fs_stat->m_l_namelen = stfs.f_namemax;

RETURN:
	return ret;
}
AUI_RTN_CODE aui_fs_fstatfs(aui_f_hdl aui_h_file, aui_fs_fs_status_t * ps_fs_stat)
{
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	char vol_path[16];
	struct statvfs stfs;
	int ret = 0;
	BOOL file_exist = FALSE;

	if((!aui_h_file)||(!ps_fs_stat))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	MEMSET(ps_fs_stat, 0, sizeof(aui_fs_fs_status_t));

	osal_mutex_lock(fs_info->aui_fs_mutex, OSAL_WAIT_FOREVER_TIME);
	if(!list_empty(&fs_info->listpointer))
	{
		list_for_each_safe(ptr, ptn, &fs_info->listpointer)
		{
			file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
			if((file_info->pfd == aui_h_file) && (file_info->is_dir == 0))
			{
				_get_volume_name(file_info->path, vol_path);
				file_exist = TRUE;
				break;
			}
		}
	}
	osal_mutex_unlock(fs_info->aui_fs_mutex);

	if(!file_exist)
	{
		AUI_ERR("file not exist\n");
		ret = -1;
		goto RETURN;
	}

	ret = fs_statvfs(vol_path, &stfs);
	if(ret < 0)
	{
		AUI_ERR("stat vfs fail\n");
		ret = -1;
		goto RETURN;
	}

	if (strncmp (stfs.f_fsh_name,"FAT32",5) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_FAT32;
	else if(strncmp (stfs.f_fsh_name,"FAT16",5) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_FAT16;
	else if(strncmp (stfs.f_fsh_name,"FAT12",5) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_FAT12;	
	else if(strncmp (stfs.f_fsh_name,"NTFS",4) == 0)
		ps_fs_stat->m_e_fs_type = AUI_FS_PARTITION_NTFS;
	
	ps_fs_stat->m_l_bsize = stfs.f_frsize;
	ps_fs_stat->m_l_blocks = stfs.f_blocks;
	ps_fs_stat->m_l_bfree = stfs.f_bfree;
	ps_fs_stat->m_l_bavail = stfs.f_favail;
	ps_fs_stat->m_l_files = stfs.f_files;
	ps_fs_stat->m_l_ffree = stfs.f_ffree;
	ps_fs_stat->m_l_namelen = stfs.f_namemax;

RETURN:
	return ret;
}

AUI_RTN_CODE aui_fs_de_init(void)
{
	if(!fs_info)
	{
	   AUI_ERR("invalid arg\n");
	   return -1;
	}
	else
	{
		MEMSET((void*)fs_info,0x00,sizeof(aui_fs_info_t));
		FREE(fs_info);
	}

	return 0;
}

static int _format_volume(struct dev_info *focus_vol, aui_fs_fs_type_t e_type)
{
	BOOL unmount_dev = FALSE;
	struct statvfs stfs;
	int  ret = 0;
	int fs_ret	= 0;

	// step1: unmount
	if(0 == STRLEN(focus_vol->mnt_path))
	{
		unmount_dev = TRUE;
	}

	if(!unmount_dev)
	{
		fs_statvfs(focus_vol->mnt_path, &stfs);
		int ret_val = fs_unmount(focus_vol->mnt_path, 0);

		if(ret_val < 0)
		{
			AUI_ERR("umount fail\n");
			ret = -1;
			goto RETURN;
		}
	}

	// step2: format
	char fs_name[16];
	MEMSET(fs_name,0,sizeof(fs_name));
	if(AUI_FS_PARTITION_FAT32 == e_type)
	{
		STRCPY(fs_name, fs_info->filesys_type[0]);
	}
	else if(AUI_FS_PARTITION_NTFS == e_type)
	{
		STRCPY(fs_name, fs_info->filesys_type[1]);
	}

	fs_ret = fs_mkfs(focus_vol->dev_name, fs_name);
	if(fs_ret < 0)
	{
		if (!unmount_dev)
			fs_rmdir(focus_vol->mnt_path);

		AUI_ERR("mkfs fail\n");
		ret = -1;
		goto RETURN;
	}

	// step3: mount
	STRCPY(focus_vol->mnt_path, focus_vol->dev_name);
	focus_vol->mnt_path[1] = 'm';
	focus_vol->mnt_path[2] = 'n';
	focus_vol->mnt_path[3] = 't';

	if(unmount_dev)
	{
		fs_mkdir(focus_vol->mnt_path, 0);
	}

	fs_ret = fs_mount(focus_vol->mnt_path, focus_vol->dev_name, fs_name, 0, NULL);
	if(fs_ret < 0)
	{
		fs_rmdir(focus_vol->mnt_path);

		AUI_ERR("mnt fail\n");
		ret = -1;
		goto RETURN;
	}

	if(!unmount_dev)
	{
		fs_write_statvfs(focus_vol->mnt_path, &stfs, WFSSTAT_NAME);
		fs_sync(focus_vol->mnt_path);
	}

	_update_dev_info();
RETURN:
	return ret;
}


static void backup_volume_id(int devid,unsigned int *to)
{
	int i = 0;
	int j = 0;

	for(j = 0; j < fs_info->dev[devid].fs_num; j++)
	{
		to[i++] = fs_info->dev[devid].part[j+1].volume_id;
	}

	return;
}

static void restore_volume_id(int devid,unsigned int *from)
{
	int i = 0;
	int j = 0;

	for(j = 0; j < fs_info->dev[devid].fs_num; j++)
	{
		fs_info->dev[devid].part[j+1].volume_id = from[i++];
	}

	return;
}

/**
*	 @brief 		 mkfs on a volume or device.
*	 @author		 seiya.cao
*	 @date		   2013-8-23
*	 @param[in]    device id,represet type of device.
*	 @param[out]  fs type formate to.
*	 @return		   return 0 when success.others return -1.
*	 @note
*
*/

 AUI_RTN_CODE aui_fs_format(unsigned int dw_device_id,aui_fs_fs_type_t e_type)
{
	UINT8 i = 0, j = 0, k = 0;
	struct dev_info *focus_vol;
	BOOL unmount_dev = FALSE;
	struct statvfs stfs;
	FILE *fp = NULL;
	int ret = 0;
	int fs_ret = 0;

	if(AUI_ISREMOVEABLE(dw_device_id) == 0)
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}


	if((e_type != AUI_FS_PARTITION_FAT32) && (e_type != AUI_FS_PARTITION_NTFS))
	{
		AUI_ERR("invalid arg\n");
		ret = -1;
		goto RETURN;
	}

	if(0x20000 == AUI_ISREMOVEABLE(dw_device_id))		   // volume device
	{
		for(i = 0; i < fs_info->dev_num; i++)
		{
			for(j = 0; j < fs_info->dev[i].fs_num; j++)
			{
				if(dw_device_id == fs_info->dev[i].part[j+1].volume_id)
				{
					focus_vol = &fs_info->dev[i].part[j+1];
					UINT32	dev_id_before_format = fs_info->dev[i].dev_id;
					unsigned int	vol_id_before_format[32];
					memset(vol_id_before_format,0x00,32*4);

					backup_volume_id(i,vol_id_before_format);
					UINT32	ali_dev_id_bf = fs_info->dev[i].ali_dev_id;
	
					if(_format_volume(focus_vol, e_type) != 0)
					{
						AUI_ERR("format fail\n");
						ret = -1;
						goto RETURN;
					}
					restore_volume_id(i,vol_id_before_format);
					fs_info->dev[i].ali_dev_id = ali_dev_id_bf;
					fs_info->dev[i].dev_id = dev_id_before_format;
					goto RETURN;
				}
			}
		}

	}
	else if(0x10000 == AUI_ISREMOVEABLE(dw_device_id))	 // storage device
	{
		int bytes_each_sector = 512;
		char buf[512];
		UINT32 mbr_buf[16];
		char dev_name[16];
		struct mbr_partition_record* p_table = (struct mbr_partition_record*)mbr_buf;

		for(i = 0; i < fs_info->dev_num; i++)
		{
			if(dw_device_id == fs_info->dev[i].dev_id)
			{
				break;
			}
		}
			unsigned int ali_dev_id_bf = fs_info->dev[i].ali_dev_id;

		if(1 == fs_info->dev[i].fs_num)
		{
			focus_vol = &fs_info->dev[i].part[1];

			if(_format_volume(focus_vol, e_type) != 0)
			{
				AUI_ERR("fmt vol fail\n");
				ret = -1;
				goto RETURN;
			}
			int pv_eventdata = AUI_FS_PLUGOUT_SAFE;
			for(j = 0; j < 10; j++)
			{
				if(NULL != fs_info->callback[j].m_fn_callback)
				{
					fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_PLUGOUT, fs_info->dev[i].part[1].volume_id,\
						 (const void*)pv_eventdata, fs_info->callback[j].m_usercb_data);
				}
			}

			for(j = 0; j < 10; j++)
			{
				if(NULL != fs_info->callback[j].m_fn_callback)
				{
					fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_FOUND, fs_info->dev[i].part[1].volume_id, \
						(const void*)pv_eventdata, fs_info->callback[j].m_usercb_data);
					fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_READY, fs_info->dev[i].part[1].volume_id, \
						(const void*)pv_eventdata, fs_info->callback[j].m_usercb_data);
				}
			}
			fs_info->dev[i].dev_id = ++fs_info->dev_id;
			fs_info->dev[i].ali_dev_id = ali_dev_id_bf;

			for( j = 0; j < fs_info->dev[i].fs_num; j++)
			{
				fs_info->dev[i].part[j+1].volume_id = ++fs_info->vol_id;
			}

			goto RETURN;
		}

		// step1: unmount
		for(j = 0; j < fs_info->dev[i].fs_num; j++)
		{
			focus_vol = &fs_info->dev[i].part[j+1];
			if(0 == STRLEN(focus_vol->mnt_path))
			{
				unmount_dev = TRUE;
			}

			if(0 == j)
			{
				STRCPY(dev_name, focus_vol->dev_name);
			}

			if(!unmount_dev)
			{
				if(0 == j)
					fs_statvfs(focus_vol->mnt_path, &stfs);

				int ret_val = fs_unmount(focus_vol->mnt_path, 0);
				if(ret_val < 0)
				{
					AUI_ERR("umnt fail\n");
					ret = -1;
					goto RETURN;
				}

				fs_ret = fs_rmdir(focus_vol->mnt_path);
			}
		}

		//step 2: rewrite mbr
		fp = fopen(fs_info->dev[i].name, "r+");
		if(!fp)
		{
			AUI_ERR("open fail\n");
			ret = -1;
			goto RETURN;
		}

		fs_lseek(fp->_file, 0, SEEK_SET);

		MEMSET(buf, 0, bytes_each_sector);
		fs_ret = fs_read(fp->_file, buf, bytes_each_sector);
		if(fs_ret < 0)
		{
			AUI_ERR("read fail\n");
			ret = -1;
			goto RETURN;
		}

		if(*((unsigned short*)(buf + 0x1fe)) !=0xAA55)
		{
			AUI_ERR("not mbr\n");
			ret = -1;
			goto RETURN;
		}

		MEMCPY(mbr_buf, buf + 0x1be, 64);

		p_table[0].size = fs_info->dev[i].part[0].part_size;
		MEMSET(&p_table[1], 0, 48);

		MEMCPY(buf + 0x1be, mbr_buf, 64);
		fs_lseek(fp->_file, 0, SEEK_SET);
		fs_ret = fs_write(fp->_file, buf, bytes_each_sector);
		fclose(fp);
        fp = NULL;
		if(fs_ret < 0)
		{
			AUI_ERR("write fail\n");
			ret = -1;
			goto RETURN;
		}

//			clear dbr.
		char volume_name[16];
		int fd;
		STRCPY(volume_name, fs_info->dev[i].name);
		volume_name[8] = '1';
		volume_name[9] = '\0';
		fd = fs_open(volume_name, O_RDWR | 0x8000, S_IREAD);

		if(fd < 0)
		{
			AUI_ERR("open fail\n");
			ret = -1;
			goto RETURN;
		}

		fs_lseek(fd, 0, SEEK_SET);

		MEMSET(buf, 0, bytes_each_sector);
		fs_ret = fs_write(fd, buf, bytes_each_sector);
		fs_close(fd);
		if(fs_ret != bytes_each_sector)
		{
			AUI_ERR("w2 fail\n");
			ret = -1;
			goto RETURN;
		}

		int node_id, dev_fd;
		dev_fd = fs_open(fs_info->dev[i].name, O_RDONLY, 0);
		fs_ret = fs_ioctl(dev_fd, IOCTL_GET_NODEID, &node_id, sizeof(int));
		fs_close(dev_fd);

		_update_dev_info();

		if(_format_volume(&fs_info->dev[i].part[1], e_type) != 0)
		{
			AUI_ERR("fmt vol fail\n");
			ret = -1;
			goto RETURN;
		}

		int pv_eventdata = AUI_FS_PLUGOUT_SAFE;
		for(j = 0; j < 10; j++)
		{
			if(NULL != fs_info->callback[j].m_fn_callback)
			{
				for(k = 0; k<fs_info->dev[i].fs_num; k++)
				{
					fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_PLUGOUT, fs_info->dev[i].part[k+1].volume_id, \
						(const void*)pv_eventdata, fs_info->callback[j].m_usercb_data);
				}
			}
		}

		fs_info->dev[i].dev_id = ++fs_info->dev_id;
		fs_info->dev[i].ali_dev_id = ali_dev_id_bf;

		for( j = 0; j < fs_info->dev[i].fs_num; j++)
		{
			fs_info->dev[i].part[j+1].volume_id = ++fs_info->vol_id;
		}
		for(j = 0; j < 10; j++)
		{
			if(NULL != fs_info->callback[j].m_fn_callback)
			{
				fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_FOUND, fs_info->dev[i].part[1].volume_id, NULL, fs_info->callback[j].m_usercb_data);
				fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_READY, fs_info->dev[i].part[1].volume_id, NULL, fs_info->callback[j].m_usercb_data);
			}
		}
	}
RETURN:
    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }
	return ret;
}

 AUI_RTN_CODE aui_fs_addcallback(aui_fs_event_callback m_fn_callback,const void * m_usercb_data)
{
	UINT8 i = 0;
	BOOL cmp1, cmp2;
	int ret = AUI_RTN_SUCCESS;

	if(!m_fn_callback)
	{
		AUI_ERR("invalid arg\n");
		ret = AUI_RTN_FAIL;
		goto RETURN;
	}

	for(i = 0; i < AUI_MAX_DEVICE_NS; i++)
	{
		if(NULL != fs_info->callback[i].m_fn_callback)
		{
			cmp1 = (fs_info->callback[i].m_fn_callback == m_fn_callback);
			cmp2 = (fs_info->callback[i].m_usercb_data == m_usercb_data);

			if(cmp1&&cmp2)
			{
				AUI_ERR("user param error\n");
				ret = FS_ERR_CBARD_ADDED;
				goto RETURN;
			}
		}
		else
		{
			fs_info->callback[i].m_fn_callback = m_fn_callback;
			fs_info->callback[i].m_usercb_data = (void*)m_usercb_data;
			break;
		}
	}

	if(i == AUI_MAX_DEVICE_NS)
	{
		AUI_ERR("not found\n");
		ret = AUI_RTN_FAIL;
		goto RETURN;
	}

RETURN:
	return ret;
}

 AUI_RTN_CODE  aui_fs_remove_callback(aui_fs_event_callback m_fn_callback,const void * m_usercb_data)
{
	int ret = AUI_RTN_SUCCESS;
	UINT8 i = 0;

	if(!m_fn_callback)
	{
		AUI_ERR("invalid arg\n");
		ret = AUI_RTN_FAIL;
		goto RETURN;
	}

	for(i = 0; i < AUI_MAX_DEVICE_NS; i++)
	{
		if(NULL != fs_info->callback[i].m_fn_callback)
		{
			if((fs_info->callback[i].m_fn_callback == m_fn_callback) && (fs_info->callback[i].m_usercb_data == m_usercb_data))
			{
				fs_info->callback[i].m_fn_callback = NULL;
				fs_info->callback[i].m_usercb_data = NULL;
				break;
			}
		}
	}

	if(i == AUI_MAX_DEVICE_NS)
	{
		AUI_ERR("not found\n");
		ret = AUI_RTN_FAIL;
		goto RETURN;
	}

RETURN:
	return ret;
}

 AUI_RTN_CODE  aui_fs_remove_dev(unsigned int dw_device_id)
{
	UINT8 i = 0, j = 0, k = 0;
	int ret = AUI_RTN_SUCCESS;
	int fs_ret = 0;
	char dev_name[16];
	aui_ofile_manage_t *file_info = NULL;
	struct list_head *ptr, *ptn;
	BOOL open_file = FALSE;

	for(i = 0; i < AUI_MAX_DEVICE_NS; i++)
	{
		if(dw_device_id == fs_info->dev[i].dev_id)
		{
			// check if file is opened
			if(!list_empty(&fs_info->listpointer))
			{
				list_for_each_safe(ptr, ptn, &fs_info->listpointer)
				{
					file_info = list_entry(ptr, aui_ofile_manage_t, file_list);
					for(i = 0; i < fs_info->dev_num; i++)
					{
						STRCPY(dev_name, fs_info->dev[i].name);
						if(file_info->path[7] == dev_name[7])
						{
							open_file = TRUE;
							break;
						}
					}

					if(open_file)
					{
						AUI_ERR("busy\n");
						ret = AUI_RTN_FAIL;
						goto RETURN;
					}
				}
			}

			UINT32 node_id;
			int fd;
			fd = fs_open(fs_info->dev[i].name, O_RDONLY, 0);
			fs_ret = fs_ioctl(fd, IOCTL_GET_NODEID, &node_id, sizeof(int));
			fs_close(fd);

			if(fs_ret != 0)
			{
				AUI_ERR("ioctl fail\n");
				ret = AUI_RTN_FAIL;
				goto RETURN;
			}

			if((ALI_C3701==sys_ic_get_chip_id())||(ALI_S3503==sys_ic_get_chip_id()))
			{
				usb_fs_unmount(node_id);
			}
			else
				usbd_safely_remove_ex((USBD_NODE_ID)node_id);

			int pv_eventdata = AUI_FS_PLUGOUT_SAFE;
			for(j = 0; j < AUI_MAX_DEVICE_NS; j++)
			{
				if(NULL != fs_info->callback[j].m_fn_callback)
				{
					for(k = 0; k<fs_info->dev[i].fs_num; k++)
					{
						fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_PLUGOUT, fs_info->dev[i].part[k+1].volume_id, \
							(const void*)&pv_eventdata, fs_info->callback[j].m_usercb_data);
					}
					fs_info->callback[j].m_fn_callback(AUI_FS_EVENT_PLUGOUT, fs_info->dev[i].dev_id, \
						(const void*)&pv_eventdata, fs_info->callback[j].m_usercb_data);
				}
			}

			fs_info->dev[i].valid = 0;
			MEMSET(fs_info->dev[i].name, 0, 16);
			fs_info->dev[i].fs_num = 0;
			fs_info->dev[i].dev_id = 0;
			fs_info->dev[i].ali_dev_id = 0;
			fs_info->dev[i].total_size = 0;
			fs_info->dev[i].free_size = 0;
			if(fs_info->dev_num > 0)
				fs_info->dev_num--;
			MEMSET(fs_info->dev[i].part, 0, sizeof(struct dev_info)*16);
			break;
		}
	}

	if(i == AUI_MAX_DEVICE_NS)
	{
		AUI_ERR("not found\n");
		ret = AUI_RTN_FAIL;
		goto RETURN;
	}
	_update_dev_info();

RETURN:
	return ret;
}


 AUI_RTN_CODE aui_fs_get_alldevid(unsigned int * pdw_ids,int n_max_id_cnt, int * pn_act_id_cnt)
{
	int dev_num = 0;
	int i = 0, j = 0, k = 0;
	int ret = AUI_RTN_SUCCESS;


	if((!pn_act_id_cnt)||(n_max_id_cnt < 0))
	{
		AUI_ERR("invalid arg\n");
		ret = AUI_RTN_FAIL;
		goto RETURN;
	}
	if(NULL != pdw_ids)
		MEMSET(pdw_ids, 0, n_max_id_cnt*sizeof(unsigned int));

	dev_num = fs_info->dev_num;

	if(dev_num > 0)
	{
		for(i = 0; i < dev_num; i++)
		{
			if( j<n_max_id_cnt )
			{
				if(NULL != pdw_ids)
				{
					pdw_ids[j++] = fs_info->dev[i].dev_id;
				}
				else
				{
					j++;
				}
			}
			else
				break;

			for(k = 0; k < fs_info->dev[i].fs_num; k++)
			{
				if( j<n_max_id_cnt )
				{
					if(NULL != pdw_ids)
					{
						pdw_ids[j++] = fs_info->dev[i].part[k+1].volume_id;
					}
					else
					{
						j++;
					}
				}
				else
					break;
			}
		}
	}

	*pn_act_id_cnt = j;

RETURN:
	return ret;
}

 AUI_RTN_CODE aui_fs_get_device_info(unsigned int device_id,aui_fsdev_info * ps_device_info)
{
	UINT8 i = 0, j = 0, k = 0;
	int ret = AUI_RTN_FAIL;
	BOOL hit = FALSE;

	MEMSET(ps_device_info, 0, sizeof(aui_fsdev_info));
	if(0x10000 == AUI_ISREMOVEABLE(device_id))
	{
		for(i = 0; i < fs_info->dev_num; i++)
		{
			if(device_id == fs_info->dev[i].dev_id)
			{
				ps_device_info->m_dev_id = device_id;
				ps_device_info->m_dev_type = AUI_FS_DEV_TYPE_STORAGE;
				STRCPY(ps_device_info->m_dev_name, fs_info->dev[i].name);
				ps_device_info->m_dev_size = (unsigned long long)fs_info->dev[i].total_size;
				hit = TRUE;
				k = i;
				break;
			}
		}

	}
	else if(0x20000 == AUI_ISREMOVEABLE(device_id))
	{
		for(i = 0; i < fs_info->dev_num; i++)
		{
			if(hit)
				break;

			for(j = 0; j < fs_info->dev[i].fs_num; j++)
			{
				if(fs_info->dev[i].part[j+1].volume_id == device_id)
				{
					ps_device_info->m_dev_id = device_id;
					ps_device_info->m_dev_type = AUI_FS_DEV_TYPE_VOLUME;
					STRCPY(ps_device_info->m_dev_name, fs_info->dev[i].part[j+1].dev_name);
					ps_device_info->m_dev_size = (unsigned long long)fs_info->dev[i].part[j+1].total_size;
					ps_device_info->m_parentdev_id = fs_info->dev[i].dev_id;
					STRCPY(ps_device_info->m_mount_point, fs_info->dev[i].part[j+1].mnt_path);
					hit = TRUE;
					k = i;
					break;
				}
			}
		}

	}

	if(hit == TRUE)
	{
		int dev_fd = -1;
		struct device_geometry geo;

		dev_fd = fs_open(fs_info->dev[k].name, O_RDONLY, 0);
        memset(&geo, 0, sizeof(geo));
		if(fs_ioctl(dev_fd, IOCTL_GET_DEVICE_GEOMETRY, &geo, sizeof(struct device_geometry)))  // libo
		{
			AUI_ERR("ioctl fail\n");
			ret = AUI_RTN_FAIL;
		}
		fs_close(dev_fd);

		memcpy(ps_device_info->product_name, geo.product, 256);
		memcpy(ps_device_info->serial_number_name, geo.serialid, 256);
		memcpy(ps_device_info->manufacturer_name, geo.manufact, 256);

		ret = AUI_RTN_SUCCESS;
	}

	return ret;
}


