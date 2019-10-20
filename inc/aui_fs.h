/**
<!--
Notice:             This file mainly contains Doxygen-style comments to generate
                    the HTML help file (.chm) provided by ALi Corporation to
                    give ALi Customers a quick support for using ALi AUI API
                    Interface.\n
                    Furthermore, the comments have been wrapped at about 80 bytes
                    in order to avoid the horizontal scrolling when splitting
                    the display in two part for coding, testing, debugging
                    convenience
Current Author:     Amu.Tu
Last update:        2017.02.13
-->

@file   aui_fs.h

@brief  File system (FS) Module

        Generally, before using a physical storage device, the storage device
        must be partitioned and the partitions should be formatted by any one
        kind of file system.\n
        <b> File System (FS) Module </b> is used for that scope as well as for
        reading/writing files from/to the storage device.\n
        FS Module @a only support two (2) type of file system as below.
        - FAT
        - NTFS (which activation requires third-party license)

@note   FS Module is @a only applicable for TDS OS platforms

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_FS_H

#define _AUI_FS_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*****************************Global Macro List*******************************/

/**
Macro to specify the maximum number of <b> USB Disk </b> supported by the
platform
*/
#define AUI_MAX_USB_DISK_NUM    16

/**
Macro to specify the maximum number of <b> SD Disk </b> supported by the platform
*/
#define AUI_MAX_SD_DISK_NUM     1

/**
Macro to specify maximum number of <b> IDE Disk </b> supported by the platform
*/
#define AUI_MAX_IDE_DISK_NUM    1

/**
Macro to specify the number of <b> Physical Device Type </b> supported by the
platform

@note  <b> TDS OS </b> platform can support three (3) kinds of physical device
       which are including
       - HDD
       - USB
       - SD
*/
#define AUI_DEVICE_TYPE_SUPPORT 3

/**
Macro to specify the maximum number of <b> Physical Storage Device </b>
supported by FS Module
*/
#define AUI_MAX_DEVICE_NS       10

/**
Macro to specify the <b> Major Device ID Mask </b> which is used for the macro
#AUI_ISREMOVEABLE to get the major device ID
*/
#define AUI_RDI_MAJOR_DEVICEID_MASK    (0xFFFF0000)

/**
@warning  This macro is currently no used then can be ignored
*/
#define AUI_RDI_MINOR_DEVICEID_MASK    (0x0000FFFF)

/**
Macro used to judge whether a device is a <b> Removable Device </b> or not
according to the device ID. In particular, if the major device ID is zero then
the device is a no-removable device otherwise it is
*/
#define AUI_ISREMOVEABLE(dw_device_id) ((dw_device_id) & AUI_RDI_MAJOR_DEVICEID_MASK)

/*******************************Global Type List*******************************/

/**
Pointer to the file handle.

@note   At first, one handle should be gotten when opening FS Module, then the
        other functions can be used to access the file with that handle
*/
typedef void* aui_f_hdl;

/**
Enum to specify the plug-out event type of the storage device.\n
*/
typedef enum aui_fs_plugout_type {

    /**
    @warning   This value is not used currently then user can ignore it
    */
    AUI_FS_PLUGOUT_UNKNOWN,

    /**
    Value to specify the device has been plug-out by user safely
    */
    AUI_FS_PLUGOUT_SAFE,

    /**
    @warning   This value is not used currently then user can ignore it
    */
    AUI_FS_PLUGOUT_UNSAFE,

    /**
    @warning   This value is not used currently then user can ignore it
    */
    AUI_FS_PLUGOUT_OVERFLOW

} aui_fs_plugout_type, aui_plugout_type_t;

/**
Enum to specify an event when the storage device is working.\n
*/
typedef enum aui_fs_event {

    /**
    @warning   This item is not used currently then user can ignore it
    */
    AUI_FS_EVENT_UNKNOWN,

    /**
    Value to specify the platform detected a storage device
    */
    AUI_FS_EVENT_FOUND,

    /**
    @warning   This item is not used currently then user can ignore it.
    */
    AUI_FS_EVENT_CHECKING,

    /**
    Value to specify a storage device is mounted successfully and is ready
    for read/write operation
    */
    AUI_FS_EVENT_READY,

    /**
    @warning   This item is not used currently then user can ignore it
    */
    AUI_FS_EVENT_ERROR,

    /**
    Value to specify a a storage device is plugged out
    */
    AUI_FS_EVENT_PLUGOUT

} aui_fs_event, aui_fs_event_t;

/**
Function pointer used to notify when storage device event happened, where:
- @b type           = Output parameter to specify which event happened, as
                      defined in the enum #aui_fs_event_t
- @b device_id      = Output parameter to specify the ID of the device which
                      generated the event
- @b pv_event_data  = Output parameter to specify detailed information about
                      the event. For example, the detailed information about
                      the #AUI_FS_EVENT_PLUGOUT event type is defined in the
                      enum #aui_plugout_type_t
- @b pv_usercb_data = Input parameter to specify the user data passed by the
                      function #aui_fs_addcallback for a specific event
*/
typedef void (*aui_fs_event_callback) (

    aui_fs_event type,

    unsigned int device_id,

    const void *pv_event_data,

    void *pv_usercb_data

    );

/**
Enum to specify the device type
*/
typedef enum aui_fs_dev_type {

    /**
    Value to specify the device is not recognized by the platform
    */
    AUI_FS_DEV_TYPE_UNKNOWN,

    /**
    Value to specify the device is a whole storage device.

    @note   On platform, this kind of device may be named as <b> /dev/uda </b>
    */
    AUI_FS_DEV_TYPE_STORAGE,

    /**
    Value to specify the device is just one partition of a storage device.

    @note   On platform, this kind of device may be named as <b> dev/uda </b>
    */
    AUI_FS_DEV_TYPE_VOLUME

} aui_fs_dev_type, aui_fsdev_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> File System (FS) Module </b> to specify the attributes
        related to the storage device
        </div> @endhtmlonly

        Struct to specify the attributes related to the storage device
*/
typedef struct aui_fsdev_info {

    /**
    Member to specify the <b> Device ID </b>
    */
    unsigned long m_dev_id;

    /**
    Member to specify the <b> Device Type </b>, as defined in the enum
    #aui_fs_dev_type.
    */
    aui_fs_dev_type m_dev_type;

    /**
    Member to specify the <b> Device Name </b>
    */
    char m_dev_name[128];

    /**
    Member to specify the <b> Device Size </b> (in @a byte unit)
    */
    unsigned long long m_dev_size;

    /**
    Member to specify the <b> Parent Device ID Value </b>.\n

    @note   If the device is a partition, its parent device ID value is the
            storage device which partitions contain the partition device.
    */
    unsigned long m_parentdev_id;

    /**
    Member to specify the <b> Mount Path </b> in the file system
    */
    char m_mount_point[256];

    /**
    Member to specify the <b> Product Name </b> of the storage device
    */
    char product_name[256];

    /**
    Member to specify the <b> Manufacturer Name </b> of the storage device
    */
    char manufacturer_name[256];

    /**
    Member to specify the <b> Serial Number </b> of the storage device
    */
    char serial_number_name[256];

} aui_fsdev_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> File System (FS) Module </b> to specify the attributes
        related to the files </div> @endhtmlonly

        Struct to specify the attributes related to the files
*/
typedef struct aui_fs_file_stat {

    /**
    Member as a @a flag to specify the <b> file type </b>, in particular \n
    - @b 1 = The file attribute descriptor is mapped to a directory
    - @b 0 = The file attribute descriptor is mapped to a common file
    */
    unsigned long m_isdir;

    /**
    Member to specify the <b> Actual Size </b> of the file (in @a byte unit)
    */
    long long m_l_size;

    /**
    Member to specify the <b> Block Size </b> of the file system
    */
    unsigned long m_l_blk_size;

    /**
    Member to specify the <b> Block Number </b> taken by the file, so the
    allocated file size is \n\n

    <b> m_l_blk_size x m_l_blk_count </b>\n\n

    (in @a byte unit)
    */
    unsigned long m_l_blk_count;

    /**
    Member to specify the <b> Last Visit Time </b> from <b> 1970-01-01:0:0:0
    </b>, i.e. <b> Unix Time </b> (in @a second unit)

    @note   This time is not reliable unless operating on the file after
            initializing the platform time with a real and right one
    */
    unsigned long m_l_atime;

    /**
    Member to specify the <b> Last Modification Time </b> from
    <b> 1970-01-01:0:0:0 </b> , i.e. Unix time (in @a second unit)

    @note   This time is not reliable unless operating on the file after
    initializing the platform time with a real and right one
    */
    unsigned long m_l_mtime;

    /**
    Member to specify the <b> Last Creation Time </b> from 1970-01-01:0:0:0
    <b> 1970-01-01:0:0:0 </b> , i.e. Unix time (in @a second unit)

    @note   This time is not reliable unless operating on the file after
    initializing the platform time with a real and right one
    */
    unsigned long m_l_ctime;

} aui_fs_file_stat, aui_fs_file_stat_t;

/**
Enum to specify the <b> File System Type </b> for partition
*/
typedef enum aui_fs_fs_type {

    /**
    Value to specify the file system type is <b> FAT12 </b>
    */
    AUI_FS_PARTITION_FAT12,

    /**
    Value to specify the file system type is <b> FAT16 </b>
    */
    AUI_FS_PARTITION_FAT16,

    /**
    Value to specify the file system type is <b> FAT32 </b>
    */
    AUI_FS_PARTITION_FAT32,

    /**
    Value to specify the file system type is <b> NTFS </b>
    */
    AUI_FS_PARTITION_NTFS,

} aui_fs_fs_type, aui_fs_fs_type_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> File System (FS) Module </b> to specify the attributes
        related to the file system type
        </div> @endhtmlonly

        Struct to specify the attributes related to the file system type
*/
typedef struct aui_fs_fs_status {

    /**
    Member to specify the <b> File System Type </b>, as defined in the enum
    #aui_fs_fs_type
    */
    aui_fs_fs_type m_e_fs_type;

    /**
    Member to specify the <b> Block Size </b> of the file system
    (in unit @a byte unit)
    */
    long m_l_bsize;

    /**
    Member to specify the <b> Total blocks Allocated </b> for the file system
    */
    long m_l_blocks;

    /**
    Member to specify the <b> Free Blocks </b> of the file system
    */
    long m_l_bfree;

    /**
    Member to specify the <b> Free Blocks Available </b> for a
    <b> non-superuser </b>

    @warning   This member is not used currently then user can ignore it
    */
    long m_l_bavail;

    /**
    Member to specify <b> Total File Nodes </b> in the file system

    @warning   This member is not used currently then user can ignore it
    */
    long m_l_files;

    /**
    Member to specify the <b> Free File Nodes </b> in the file system

    @warning   This member is not used currently then user can ignore it
    */
    long m_l_ffree;

    /**
    Member to specify the <b> Maximum Length </b> of file name

    @warning   This member is not used currently then user can ignore it
    */
    long m_l_namelen;

} aui_fs_fs_status, aui_fs_fs_status_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> File System (FS) Module </b> to specify the
        information about each file entry within a file folder
        </div> @endhtmlonly

        Struct to specify the information about each file entry within a file
        folder
*/
typedef struct aui_fs_dirent {

    /**
    Member as a @a flag to specify the file type, in particular:
    - @b 1 = The file is a directory
    - @b 0 = The file is common file
    */
    unsigned char m_uc_dir_type;

    /**
    Member to specify the file name
    */
    char m_c_dir_name[550];

} aui_fs_dirent, aui_fs_dirent_t;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to perform the initialization of the FS Module
                and build the environment for mounting the physical storage
                device

@return         @b AUI_RTN_SUCCESS  = Initializing of the FS Module performed
                                      successfully
@return         @b AUI_RTN_FAIL     = Initializing of the FS Module failed for
                                      some reason

@note           This function should be called only once while initializing the
                platform.\n
                It is not advised to call any other function before calling
                #aui_fs_init
*/
AUI_RTN_CODE aui_fs_init (

    void

    );

/**
@brief          Function used to perform the de-initialization of the FS Module

@return         @b AUI_RTN_SUCCESS = De-initializing of the FS Module performed
                                     successfully
@return         @b AUI_RTN_FAIL    = De-initializing of the FS Module failed for
                                     some reason

@note           User must call this function when system is going to power down
                or standby mode.\n
                After this function is called, user cannot call any other function
*/
AUI_RTN_CODE aui_fs_de_init (

    void

    );
/**
@brief          Function used to open the file with one full path file name

@param[in]      pc_file_name   = Path of the file needed to open
@param[in]      pc_file_mode   = Ponter to a string which starts with one of the
                                 following sequences of characters:
                                 - @b r  = Open text file for reading
                                 - @b r  = Open for reading and writing
                                 - @b w  = Truncate file to zero length or create
                                           text file for writing.
                                 - @b w+ = Open for reading and writing, the file
                                           is created if it does not exist
                                           otherwise it is truncated)
                                 - @b a  = Open for appending (writing at the end
                                           of the file), the file is created if
                                           it does not exist
                                 - @b a+ = Open for reading and appending
                                           (writing at the end of the file), the
                                           file is created if it does not exist

@return          Return a valid file handle if opening file successfully, and a
                 read/write position indicator associated to this handle will
                 point at the beginning of file.\n
@return          Return a NULL file handle if opening file fails for some reasons
*/
aui_f_hdl aui_fs_open (

    const char *pc_file_name,

    const char *pc_file_mode

    );

/**
@brief          Function used to close the file handle opened by the function
                #aui_fs_open

@param[in]      aui_h_file          = File handle that needed to be closed

@return         @b AUI_RTN_SUCCESS  = Closing of the file handler performed
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
                                      successfully
@return         @b AUI_RTN_FAIL     = Closing of the file handler failed for
                                      some some reasons
*/
AUI_RTN_CODE aui_fs_close (

    aui_f_hdl aui_h_file

    );

/**
@brief          Function used to read data from a file and store it in a buffer.\n\n

                After reading the data from the file succesfully, the read/write
                position indicator value will be increased by the number of byte
                actually read

@param[in]      aui_h_file           = The file handle already opened and
                                       available to be read
@param[in]      u_count              = The data size needed to be read from the
                                       file

@param[out]     pc_buf               = Buffer used to store the data just read
                                       from the file

@return         @b AUI_RTN_SUCCESS   = Reading of the data from the file performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Reading of the data fron the fail failed
                                       for some reasons
*/
AUI_RTN_CODE aui_fs_read (

    aui_f_hdl aui_h_file,

    char *pc_buf,

    unsigned int u_count

    );

/**
@brief          Function used to write data stored in a buffer to a file

@param[in]      aui_h_file           = The file handle already opened and
                                       available to be written
@param[in]      u_count              = The data size needed to be written to the
                                       file
@param[in]      pc_buf               = Buffer containg the data to be written
                                       to a file

@return         @b AUI_RTN_SUCCESS   = Writing of the data to the file performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Writing of the data to the file failed
                                       for some reasons
*/
AUI_RTN_CODE aui_fs_write (

    aui_f_hdl aui_h_file,

    char *pc_buf,

    unsigned int u_count

    );

/**
@brief          Function used to set the read/write position indicator value of
                the file handle.\n\n

                The new file position (in @a bytes unit) is obtained by adding
                an @a offset (in @a byte unit) to the original position

@param[in]      aui_h_file           = The file handle already opened and
                                       available to be read/written
@param[in]      l_offset             = Offset to be added to the original
                                       position to set the new read/write position
                                       indicator value
@param[in]      u_origin             = The original read/write position indicator
                                       value, which can be:
                                       - @b SEEK_SET = Begining of the file
                                       - @b SEEK_CUR = Current position in the
                                                       file
                                       - @b SEEK_END = End of the file

@return         @b AUI_RTN_SUCCESS   = Setting of the new read/write position
                                       indicator value performed successfully
@return         @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Setting of the new read/write position
                                       indicator value failed for some reasons
*/
AUI_RTN_CODE aui_fs_seek (

    aui_f_hdl aui_h_file,

    long l_offset,

    unsigned int u_origin

    );

/**
@brief          Function used to set the read/write position indicator value of
                the file handle.\n\n

                The new file position (in @a bytes unit) is obtained by adding
                an @a offset (in @a byte unit) to the original position

@note           The difference between this function and #aui_fs_lseek is
                summarized below:
                - #aui_fs_lseek supports 64 bits file position indicator
                - #aui_fs_seek supports 32 bits file position indicator

@param[in]      aui_h_file           = The file handle already opened and
                                       available to be read/written
@param[in]      i64offset            = Offset to be added to the original
                                       position to set the new read/write position
                                       indicator value
@param[in]      u_origin             = The original read/write position indicator
                                       value, which can be:
                                       - @b SEEK_SET = Begining of the file
                                       - @b SEEK_CUR = Current position in the file
                                       - @b SEEK_END = End of the file

@return         @b AUI_RTN_SUCCESS   = Setting of the new read/write position
                                       indicator value performed successfully
@return         @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Setting of the new read/write position
                                       indicator value failed for some reasons
*/

AUI_RTN_CODE aui_fs_lseek (

    aui_f_hdl aui_h_file,

    signed long long i64offset,

    unsigned int u_origin

    );

/// @coding

/**
@brief          Function used to get the current value of the read/write position
                indicator on the opened file (<b> 32 bits </b> length, size in
                @a bytes unit)

@param[in]      aui_h_file           = The file handle already opened

@return         @b AUI_RTN_SUCCESS   = Getting of the current read/write position
                                       indicator value performed successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b AUI_RTN_FAIL      = Getting of the current read/write position
                                       indicator value failed for some reasons
*/
signed long aui_fs_tell (

    aui_f_hdl aui_h_file

    );

/**
@brief          Function used to get the current value of the read/write position
                indicator on the opened file (<b> 64 bits </b> length, size in
                @a bytes unit)

@param[in]      aui_h_file           = The file handle already opened

@return         @b AUI_RTN_SUCCESS   = Getting of the current read/write position
                                       indicator value performed successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b AUI_RTN_FAIL      = Getting of the current read/write position
                                       indicator value failed for some reasons
*/
long long aui_fs_ltell (

    aui_f_hdl aui_h_file

    );

/// @endcoding

/**
@brief          Function used to write the file memory data back to the physical
                file of the storage device

@param[in]      aui_h_file           = The file handle already opened

@return         @b AUI_RTN_SUCCESS   = Writing of the file memory data back to
                                       the physical file of the storage devide
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b AUI_RTN_FAIL      = Writing of the file memory data back to
                                       the physical file of the storage devide
                                       failed for some reasons

@note           It is strongly recommended that user should call this function
                before the function #aui_fs_close, otherwise some data belonging
                to the file may be lost
*/
AUI_RTN_CODE aui_fs_flush (

    aui_f_hdl aui_h_file

    );

/**
@brief          Function used to delete a file from the file system

@param[in]      pc_file_name         = Path of the file to be deleted

@return         @b AUI_RTN_SUCCESS   = Deleting of the file performed successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b AUI_RTN_FAIL      = Deleting of the file failed for some reasons
*/
AUI_RTN_CODE aui_fs_remove (

    const char *pc_file_name

    );

/**
@brief          Function used to rename a file

@param[in]      pc_old_path          = Path of file to be renamed
@param[in]      pc_new_path          = Path of file after renaming

@return         @b AUI_RTN_SUCCESS   = Renaming of the file performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in]) is
                                       invalid
@return         @b AUI_RTN_FAIL      = Renaming of the file failed for some reasons
*/
AUI_RTN_CODE aui_fs_rename (

    const char *pc_old_path,

    const char *pc_new_path

    );

/**
@brief          Function used to truncate a file size to precisely at 32 bits

@note           The difference of this function with #aui_fs_ftruncated is the
                accessing to the file directly by a path file instead of a handle

@param[in]      pc_path_name         = Path of the file to be truncated
@param[in]      ul_length            = The new length of the file to be obtained

@return         @b AUI_RTN_SUCCESS   = Truncating of the file performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Truncating of the file failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_truncated (

    const char *pc_path_name,

    unsigned long ul_length

    );

/**
@brief          Function used to truncate the file size to precisely at 32 bits

@note           The difference of this function with #aui_fs_truncated is the
                accessing to the file by a handle instead directly of a path file

@param[in]      aui_h_file           = Handle of the file to be truncated
@param[in]      ul_length            = The new length of the file to be obtained

@return         @b AUI_RTN_SUCCESS   = Truncating of the file performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Truncating of the file failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_ftruncated (

    aui_f_hdl aui_h_file,

    unsigned long ul_length

    );

/// @coding

/**
@brief          Function used to truncate the file size to precisely at 64 bits

@note           The difference of this function with #aui_fs_fltruncated is the
                accessing to the file directly by a path file instead of a handle

@param[in]      pc_path_name         = Path of file to be truncated
@param[in]      ull_length           = The new length of the file to be obtained

@return         @b AUI_RTN_SUCCESS   = Truncating of the file performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Truncating of the file failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_ltruncated (

    const char *pc_path_name,

    unsigned long long ull_length

    );

/**
@brief          Function used to truncate the file size to precisely 64 bits

@note           The difference of this function with #aui_fs_ltruncated is the
                accessing to the file by a handle instead directly of a path file

@param[in]      aui_h_file           = Handle of file to be truncated
@param[in]      ull_length           = The new length of the file to be obtained

@return         @b AUI_RTN_SUCCESS   = Truncating of the file performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Truncating of the file failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_fltruncated (

    aui_f_hdl aui_h_file,

    unsigned long long ull_length

    );

/// @endcoding

/**
@brief          Function used to get a file attribute from the file system

@note           The difference of this function with #aui_fs_fsfstate is the
                accessing to the file by directly a path file instead of a handle

@param[in]      pc_file_name         = The path of the file for which to get an
                                       attribute
@param[in]      p_file_stat          = Buffer to store the file attribute just
                                       read from the file system, as defined in
                                       the struct #aui_fs_file_stat

@return         @b AUI_RTN_SUCCESS   = Getting of the file attribute performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Getting of the file attributes failed for
                                       some reasons
*/
AUI_RTN_CODE aui_fs_fsstate (

    const char *pc_file_name,

    aui_fs_file_stat *p_file_stat

    );

/**
@brief          Function used to get a file attribute from the file system

@note           The difference of this function with #aui_fs_fsstate is the
                accessing to the file by a handle instead directly of a path file

@param[in]      aui_h_file           = The handle of file for which to get a
                                       attribute
@param[in]      p_file_stat          = Buffer to store the file attribute jsut
                                       read from the file system, as defined in
                                       the struct #aui_fs_file_stat.

@return         @b AUI_RTN_SUCCESS   = Getting of the file attribute performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Getting of the file attributes failed for
                                       some reasons
*/
AUI_RTN_CODE aui_fs_fsfstate (

    aui_f_hdl aui_h_file,

    aui_fs_file_stat *p_file_stat

    );

/**
@brief          Function used to create a directory

@param[in]      pc_path_name         = The full path of directory to be created

@return         @b AUI_RTN_SUCCESS   = Creating of the directory performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Creating of the directory failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_mkdir (

    const char *pc_path_name

    );

/**
@brief          Function used to delete a directory

@param[in]      pc_path_name         = The full path of directory to be deleted

@return         @b AUI_RTN_SUCCESS   = Deleting of the directory performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Deleting of the directory failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_rmdir (

    const char *pc_path_name

    );

/**
@brief          Function used to open a directory

@note           After calling this function successfully, the file handle will
                point to the first entry in the directory just opened

@param[in]      pc_path_name         = The full path of directory to be opened

@return         This function returns a valid directory file handle if opening
                directory is performed successfully\n
@return         This function returns a NULL directory file handle if opening
                directory failed for some reasons
*/
aui_f_hdl aui_fs_open_dir (

    const char *pc_path_name

    );

/**
@brief          Function used to close a directory.

@param[in]      aui_h_dir            = The handle of directory opened by the
                                       function #aui_fs_open_dir

@return         @b AUI_RTN_SUCCESS   = Closing of the directory performed
                                       successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Closing of the directory failed for some
                                       reasons
*/
AUI_RTN_CODE aui_fs_close_dir (

    aui_f_hdl aui_h_dir

    );

/**
@brief          Function used to read a directory

@note           After calling this function succesfully, the file entry indicator
                will refer to a file entry of the directory

@param[in]      aui_h_dir            = The handle of directory to be read

@return         This function returns a pointer to the struct #aui_fs_dirent,
                which is to represent the next file entry in the directory, if
                reading directory is performed succesfully\n
@return         This function returns NULL when reaching the end of the directory
*/
aui_fs_dirent* aui_fs_read_dir (

    aui_f_hdl aui_h_dir

    );

/**
@brief          Function used to get information about the file system of which
                a file belongs to

@note           The difference of this function with #aui_fs_fstatfs is the
                accessing to the file by directly a path file instead of a file
                handle

@param[in]      pc_path_name         = The full file path belonging to a file
                                       system for which to get information
@param[out]     p_fs_stat            = Pointer to a struct #aui_fs_fs_status
                                       intended to store information about the
                                       file system

@return         @b AUI_RTN_SUCCESS   = Getting of the file system information
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least a parameter (i.e. [in], [out])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Getting of the file system information
                                       failed for some reasons
*/
AUI_RTN_CODE aui_fs_statfs (

    const char *pc_path_name,

    aui_fs_fs_status *p_fs_stat

    );

/**
@brief          Function used to get information about the file system of which
                a file belongs to

@note           The difference of this function with #aui_fs_statfs is the
                accessing to the file by a file handle instead directly of a
                path file

@param[in]      aui_h_file           = The handle of a file belonging to a file
                                       system for which to get information.
@param[out]     p_fs_stat            = Pointer to a struct #aui_fs_fs_status
                                       intended to store information about the
                                       file system.

@return         @b AUI_RTN_SUCCESS   = Getting of the file system information
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least a parameter (i.e. [in], [out])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Getting of the file system information
                                       failed for some reasons
*/
AUI_RTN_CODE aui_fs_fstatfs (

    aui_f_hdl aui_h_file,

    aui_fs_fs_status *p_fs_stat

    );

/**
@brief          Function used to reset the file entry indicator of a directory
                to the beginning of it

@param[in]      aui_hdir_file        = The handle of directory opened by the
                                       function #aui_fs_open_dir

@return         @b AUI_RTN_SUCCESS   = Resetting of the file entry of a directory
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = The input parameter (i.e. [in]) is invalid
@return         @b AUI_RTN_FAIL      = Resetting of the file entry of a directory
                                       failed for some reasons
*/
AUI_RTN_CODE aui_fs_rewind_dir (

    aui_f_hdl aui_hdir_file

    );

/**
@brief          Function used to format a partition device

@param[in]      dw_device_id         = The ID of the partition device to be
                                       formated
@param[in]      e_type               = Specific file system that the partition
                                       device will be formatted with

@return         @b AUI_RTN_SUCCESS   = Formatting of the device partition
                                       performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b AUI_RTN_FAIL      = Formatting of the device partition
                                       failed for some reasons

@note           When formatting the partition device, the event specified by the
                macro #EM_AUIRDI_EVENT_PLUGOUT will be generated.\n
                When formatting successfully, the events specified by the macro
                #EM_AUIRDI_EVENT_FOUND and #EM_AUIRDI_EVENT_READY will be generate
*/
AUI_RTN_CODE aui_fs_format (

    unsigned int dw_device_id,

    aui_fs_fs_type e_type

    );

/**
@brief          Function used to register a callback function intended to receive
                events defined in the enum #aui_fs_event_t

@param[in]      fn_callback          = The callback function to be registered,
                                       as per comment for the function pointer
                                       #aui_fs_event_callback
@param[in]      pv_usercb_data       = Pointer to the user data to be passed
                                       to the callback funtion for a specific
                                       event, as per comment of the function
                                       pointer #aui_fs_event_callback

@return         This function returns @b 0 if the registering of the callback
                function has been performed successfully
@return         This function returns the value #FS_ERR_CBARD_ADDED if the
                registering of the callback function has been already registered
@return         This function returns @b -1 if the registering of the callback
                function is failed for some reasons
*/
AUI_RTN_CODE aui_fs_addcallback (

    aui_fs_event_callback fn_callback,

    const void *pv_usercb_data

    );

/**
@brief          Function used to un-register a callback function

@param[in]      fn_callback          = The callback funtion to be un-registered,
                                       as per comment for the function pointer
                                       #aui_fs_event_callback
@param[in]      pv_usercb_data       = The user data which will not to be passed
                                       to the callback funtion for a specific
                                       event anymore, as per comment of the
                                       function pointer #aui_fs_event_callback

@return         @b AUI_RTN_SUCCESS   = Un-registering of the callback function
                                       performed successfully
@return         @b AUI_RTN_FAIL      = Un-registering of the callback function
                                       failed for some reasons

@note           @b 1.   After calling this function, any events defined in the
                        enum #aui_fs_event_t wil not be received anymore
                @b 2.   The callback function and user data as the parameter
                        @b fn_callback and pv_usercb_data, respectively, must be the
                        same with the namesake in the function #aui_fs_addcallback,
                        otherwise the un-registering will fail
*/
AUI_RTN_CODE aui_fs_remove_callback (

    aui_fs_event_callback fn_callback,

    const void *pv_usercb_data

    );

/**
@brief          Function used to get all the ID of storage devices that have
                been mounted on the system

@param[in]      n_max_id_cnt         = Maximum number of device IDs to be gotten

@param[out]     pu_ids               = Pointer to a buffer intended to store
                                       device IDs
@param[out]     pn_act_id_cnt        = Actual number of devices IDs gotten from
                                       the buffer

@return         @b AUI_RTN_SUCCESS   = Getting of the device IDs performed
                                       successfully
@return         @b AUI_RTN_FAIL      = Getting of the device IDs failed for
                                       some reasons
*/
AUI_RTN_CODE aui_fs_get_alldevid (

    unsigned int *pu_ids,

    int n_max_id_cnt,

    int *pn_act_id_cnt

    );

/**
@brief          Function used to get information about a storage device known
                the related ID

@param[in]      device_id            = The ID of the storage device for which
                                       to get information
@param[out]     p_device_info        = Pointer to a struct #aui_fsdev_info which
                                       is intended to store the device storage
                                       information

@return         @b AUI_RTN_SUCCESS   = Getting of the device storage information
                                       performed successfully
@return         @b AUI_RTN_FAIL      = Getting of the device storage information
                                       failed for some reasons
*/

AUI_RTN_CODE aui_fs_get_device_info (

    unsigned int device_id,

    aui_fsdev_info *p_device_info

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                 START                                     */
/*****************************************************************************/

///@cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_fs_FSstate aui_fs_fsstate

#define aui_fs_FSfstate aui_fs_fsfstate

#define EM_AUIFS_DEVTYPE_UNKNOWN AUI_FS_DEV_TYPE_UNKNOWN

#define EM_AUIFS_DEVTYPE_STORAGE AUI_FS_DEV_TYPE_STORAGE

#define EM_AUIFS_DEVTYPE_VOLUME AUI_FS_DEV_TYPE_VOLUME

#define EM_AUIFS_PARTITION_FAT12 AUI_FS_PARTITION_FAT12

#define EM_AUIFS_PARTITION_FAT16 AUI_FS_PARTITION_FAT16

#define EM_AUIFS_PARTITION_FAT32 AUI_FS_PARTITION_FAT32

#define EM_AUIFS_PARTITION_NTFS AUI_FS_PARTITION_NTFS

#define EM_AUIRDI_EVENT_UNKNOWN AUI_FS_EVENT_UNKNOWN

#define EM_AUIRDI_EVENT_FOUND AUI_FS_EVENT_FOUND

#define EM_AUIRDI_EVENT_CHECKING AUI_FS_EVENT_CHECKING

#define EM_AUIRDI_EVENT_READY AUI_FS_EVENT_READY

#define EM_AUIRDI_EVENT_ERROR AUI_FS_EVENT_ERROR

#define EM_AUIRDI_EVENT_PLUGOUT AUI_FS_EVENT_PLUGOUT

#define EM_AUIRDI_PLUGOUT_UNKNOWN AUI_FS_PLUGOUT_UNKNOWN

#define EM_AUIRDI_PLUGOUT_SAFE AUI_FS_PLUGOUT_SAFE

#define EM_AUIRDI_PLUGOUT_UNSAFE AUI_FS_PLUGOUT_UNSAFE

#define EM_AUIRDI_PLUGOUT_OVERFLOW AUI_FS_PLUGOUT_OVERFLOW

/**
Volume FS type Definition
*/
typedef enum {

    /**
    Auto FS Type which is decided by the AUI layer, usually is the best
    FS type supported by the platform
    */
    EM_AUIFS_PARTITION_AUTO = 0,

    /**
    Start border definition of RO filesystem, which can not be considered as a file
    system type
    */
    EM_AUIFS_PARTITION_READONLY_START,

    /**
    ROMFS
    */
    EM_AUIFS_PARTITION_ROMFS,

    /**
    End border definition of RO filesystem which can not be considered as a file
    system type
    */
    EM_AUIFS_PARTITION_READONLY_END = 0x100,

    /**
    Order definiton of RW filesystem which can not be considered as a file
    system type
    */
    EM_AUIFS_PARTITION_RW_START,

    EM_AUIFS_PARTITION_FAT12_,

    EM_AUIFS_PARTITION_FAT16_,

    /**
    FAT32
    */
    EM_AUIFS_PARTITION_FAT32_,

    /**
    E2FS
    */
    EM_AUIFS_PARTITION_EXT2,

    /**
    EXT3
    */
    EM_AUIFS_PARTITION_EXT3,

    /**
    JFFS2
    */
    EM_AUIFS_PARTITION_JFFS2,

    /**
    NTFS
    */
    EM_AUIFS_PARTITION_NTFS_,

    /**
    UBIFS
    */
    EM_AUIFS_PARTITION_UBIFS,

    /**
    YAFFS2
    */
    EM_AUIFS_PARTITION_YAFFS2,

    /**
    Border definiton of RW filesystem, which can not be considered as a file
    system type
    */
    EM_AUIFS_PARTITION_RW_END = 0x200

} aui_fs_fs_type_t_;

#endif

///@endcond

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */


