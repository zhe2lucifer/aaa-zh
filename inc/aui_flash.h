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
Current Author:     Fawn.Fu, Niker.Li
Last update:        2017.02.25
-->

@file       aui_flash.h

@brief      Flash Memory Module

            <b> Flash Module </b> is used to perform basic operations on NOR/NAND
            Flash Memory mounted on the targeted ALi board, such as:
            - Read
            - Write
            - Erase
            - Auto Erase
            - Seek
            - Seek Read
            - Seek Write

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Flash Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_FLASH_H

#define _AUI_FLASH_H

/**************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Type List*******************************/

/**
Macro to specify the <b> maximum number of partitions </b> that can be created
in Flash Memory
*/
#define AUI_FLASH_PARTITION_NUM_MAX 32

/**
Enum to specify all possible <b> base address </b> as value of the input parameter
@b seek_type of the function #aui_flash_seek for seeking in <b> Flash Memory </b>
*/
typedef enum aui_en_flash_seek_type {

    /**
    This value is to define that the base address is the @b beginning of the
    Flash Memory
    */
    AUI_FLASH_LSEEK_SET = 0,

    /**
    This value is to define that the base address is the <b> current address </b>
    of the Flash Memory
    */
    AUI_FLASH_LSEEK_CUR,

    /**
    This value is to define that the base address is the @b end of the Flash Memory
    */
    AUI_FLASH_LSEEK_END,

} aui_flash_seek_type;

/**
Enum to specify the different kinds of Flash Memory (i.e. @b NAND and @b NOR)
available to be operated, and is used as:
- A variable of the struct #aui_st_flash_open_param
- An input parameter of the function #aui_flash_open
*/
typedef enum aui_en_flash_type {

    /**
    This value is to specify @b NOR Flash Memory
    */
    AUI_FLASH_TYPE_NOR = 0,

    /**
    This value is to specify @b NAND Flash Memory
    */
    AUI_FLASH_TYPE_NAND,

} aui_flash_type;

/**
Enum to specify the erase unit can be @b block or @b sector, and is used as:
- A variable of the struct #aui_st_flash_open_param
- An input parameter of the function #aui_flash_open
*/
typedef enum aui_en_flash_erase_type {

    /**
    Value to specify the erase unit is @b block (usually 64K for NOR Flash
    Memory), both in the function #aui_flash_erase and #aui_flash_auo_erase_write
    */
    AUI_FLASH_ERASE_TYPE_BLOCK = 0,

    /**
    Value to specify the erase unit is @b sector which size is 4K. \n
    Some Flash Memories support the erasing of small sectors (i.e. 4096 B).
    Depending on the usage, this feature may provide a performance gain in
    comparison to the erasing of whole blocks (i.e. 32/64 KB).
    Changing a small part of the Flash Memory contents is usually faster with
    small sectors. On the other hand, erasing should be faster when using
    64 KB block instead of 16 x 4 KB sectors.

    Please note that some tools/drivers/filesystems may not work with 4096 B
    erase size (e.g. UBIFS requires 15 KB as minimum).

    @note  Please set this enum value only when the SPI NOR Flash on board
           supports 4K sectors
    */
    AUI_FLASH_ERASE_TYPE_SECTOR,

} aui_flash_erase_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Flash Module </b> to collect Flash Memory Information
        </div> @endhtmlonly

        Struct to collect the <b> Flash Memory Information </b> gotten by the
        function #aui_flash_info_get and useful to perform the operations of
        the whole set of functions available for Flash Memory Module
*/
typedef struct aui_st_flash_info {

    /**
    Member to contain the total number of blocks in the Flash Memory
    */
    unsigned long block_cnt;

    /**
    Member to contain the size (in @a bytes unit) of a block of Flash Memory
    */
    unsigned long block_size;

    /**
    Member to contain the total size (in @a bytes unit) of the Flash Memory
    */
    unsigned long flash_size;

    /**
    Member to contain the starting address to operate on Flash Memory
    */
    unsigned char *star_address;

    /**
    Member to contain the Minimum write unit size (in @a bytes unit) of Flash Memory
    */
    unsigned long write_size;

} aui_flash_info;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Flash Module </b> to collect some information to be
       used for opening a Flash Memory Device
       </div> @endhtmlonly

       Struct to collect some information to be used for opening a Flash
       Memory Devices, in particular
       - <b> Flash Memory Type </b>
       - <b> Flash Memory Device Index </b>

       and is used by the function #aui_flash_open
*/
typedef struct aui_st_flash_open_param {

    /**
    Member as an @a Index which refers to different supported Flash Memory
    Devices. About the values of that index:
    1. For <b> TDS OS </b>:
       - If the partition table <b> is not set </b> by the function
         #aui_flash_init_partition_table, the index values can be
         - @b 0 for NOR Flash Memory
         - @b 1 for NAND Flash Memory (and need to define @b _NAND_ENABLE_
           Macro in the building configuration file)
       - If the partition table <b> is set </b> by the function
         #aui_flash_init_partition_table, the index is the element index
         of the partitions array defined in the structure #aui_flash_partition_table
    2. For <b> Linux OS </b> the index is the MTD device index, and it can be
       - Either a value that belongs to the range <b> [0;8] </b>
         (i.e. <b> [mtd0;mtd8] </b>) for @b NAND Flash Memory
       - Or @a only @b 9 (i.e. @b mtd9) for @b NOR Flash Memory.
    */
    unsigned long flash_id;

    /**
    Member to indicates the Flash Memory type (i.e. @b NAND or @b NOR), as per
    the enum #aui_en_flash_type
    */
    aui_flash_type flash_type;

    /**
    Member to select different erase type for the Flash Memory device, as defined
    in the enum #aui_flash_erase_type

    @note  @b 1. This member is @a only used in projects based on <b> TDS OS </b>.\n

    @note  @b 2. In projects based on <b> Linux OS </b>, instead, this member
                 is ignored, then the erase type for Flash Memory device will
                 be set to the value #AUI_FLASH_ERASE_TYPE_SECTOR as default if
                 supported.\n
                 Furthermore, the driver used for Flash Memory device is the
                 standard Linux Flash Memory driver. The change of the erase
                 type for Flash Memory device needs to be done in the Flash
                 Memory driver as well.
    */
    aui_flash_erase_type flash_erase_type;

} aui_flash_open_param;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Flash Module </b> to collect information related to
       a Flash Memory partition
       </div> @endhtmlonly

       Struct to collect information related to a Flash Memory partition,
       in particular
       - @b Type of the Flash Memory
       - <b> Start offset </b> of the Flash Memory partition
       - @b Size of the Flash Memory partition

@note  This struct is used in project based on <b> TDS OS </b>
*/
typedef struct aui_flash_partition {

    /**
    Member to specify the @b type of the Flash Memory (i.e. @b NAND or @b NOR),
    as defined in the enum #aui_en_flash_type
    */
    aui_flash_type type;

    /**
    Member to specify the <b> start offset </b> (in @a bytes unit) of the
    Flash Memory partition
    */
    unsigned long ul_offset;

    /**
    Member to specify the @b size (in @a bytes unit) of the Flash Memory partition
    */
    unsigned long ul_size;

} aui_flash_partition;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Flash Module </b> as a quick reference table of Flash
       Memory partitions information
       </div> @endhtmlonly

       Struct as a quick reference table of Flash Memory partitions information

@note  This struct is used in project based on <b> TDS OS </b>
*/
typedef struct aui_flash_partition_table {

    /**
    Member to specify the total number of Flash Memory partitions
    */
    unsigned long ul_partition_num;

    /**
    Member as an array of Flash Memory partitions information, as defined in
    the struct #aui_flash_partition
    */
    aui_flash_partition partitions[AUI_FLASH_PARTITION_NUM_MAX];

} aui_flash_partition_table;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief              Function used to perform the initialization of the Flash
                    Memory Module before its opening by the function
                    #aui_flash_open

@warning            This function can be used @a only in the <b> Pre-Run Stage
                    </b> of the Flash Memory Module

@param[in]          p_call_back_init        = Callback function used for the
                                              initialization of Flash Memory
                                              Module, as per comment for the
                                              function pointer #p_fun_cb
@param[in]          pv_param                = Input parameter of the callback
                                              function @b p_call_back_init
                                               - @b Caution: The callback function
                                                    @b p_call_back_init @a must
                                                    take a input parameter, anyway
                                                    it may take @b NULL value as
                                                    well

@return             @b AUI_RTN_SUCCESS      = Flash Memory Module initialized
                                              successfully
@return             @b AUI_FLASH_NO_MEMORY  = There is not memory available for
                                              performing memory allocation
*/
AUI_RTN_CODE aui_flash_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief              Function used to perform the de-initialization of the Flash
                    Memory Module after its closing by the function #aui_flash_close

@param[in]          p_call_back_de_init =   Callback function used to de-initialize
                                            the Flash Memory Module, as per
                                            comment for the function pointer
                                            #p_fun_cb
@param[in]          pv_param            =   Input parameter of the callback
                                            function @b p_call_back_de_init
                                            - @b Caution: The callback function
                                                 @b p_call_back_de_init @a must
                                                 take a input parameter, anyway
                                                 it may take @b NULL value as
                                                 well

@return             @b AUI_RTN_SUCCESS  =   Flash Memory Module de-initialized
                                            successfully
*/
AUI_RTN_CODE aui_flash_de_init (

    p_fun_cb p_call_back_de_init,

    void *pv_param

    );

/**
@brief              Function used to open the Flash Memory Module after its
                    initialization by the function #aui_flash_init

@param[in]          p_open_param        =   Pointer to a struct
                                            #aui_flash_open_param, which
                                            collects the Flash Memory Device
                                            Index and Flash Memory Type to be
                                            used for opening a Flash Memory
                                            Device

@param[out]         p_flash_handle      =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module just opened

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
*/
AUI_RTN_CODE aui_flash_open (

    aui_flash_open_param *p_open_param,

    aui_hdl *p_flash_handle

    );

/**
@brief              Function used to close the Flash Memory Module already
                    opened by the function #aui_flash_open then the related
                    handle will be released.\n

@warning            This function can be used before the de-initialization
                    of the Flash Memory Module

@param[in]          flash_handle        =   #aui_hdl pointer to the handle
                                            of the Flash Memory Module
                                            already opened and to be closed

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID
*/
AUI_RTN_CODE aui_flash_close (

    aui_hdl flash_handle

    );

/**
@brief              Function used to get the Flash Memory information which will
                    be stored in the struct #aui_flash_info.

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory module already
                                            opened and to be checked for the
                                            Flash Memory information to collect

@param[out]         p_flash_info        =   Pointer to the struct #aui_flash_info
                                            to store the Flash Memory information
                                            just collected

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_info_get (

    aui_hdl flash_handle,

    aui_flash_info *p_flash_info

    );

/**
@brief              Function used to erase an area of Flash Memory from a start
                    address

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of the
                                            Flash Memory Module already opened
                                            and to be managed to erase the
                                            desired size of Flash Memory
@param[in]          ul_address          =   Start address from which the Flash
                                            Memory will be erased.
@param[in]          ul_erase_size       =   Area of Flash Memory (in @a byte
                                            unit) to be erased from the start
                                            address

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_erase (

    aui_hdl flash_handle,

    unsigned long ul_address,

    unsigned long ul_erase_size

    );

/**
@brief              Function used to read an area of FLash Memory from a start
                    address

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened and to be managed to read
                                            the desired size of Flash Memory
@param[in]          ul_address          =   Start address from which the Flash
                                            Memory will be read
@param[in]          ul_read_size        =   Area of Flash Memory (in @a byte
                                            unit) of Flash Memory to be read
                                            from the start address

@param[out]         pul_return_size     =   Actual number of data (in @a byte
                                            unit) read from the Flash Memory \n
                                            (e.g. if @b read_size = 1024 (bytes)
                                            but actually the data read are just
                                            1000 (bytes), then
                                            @b p_return_size = 1000 (bytes))
@param[out]         puc_buf             =   Buffer used by Flash Memory Module
                                            to hold the data just read from
                                            Flash Memory

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_read (

    aui_hdl flash_handle,

    unsigned long ul_address,

    unsigned long ul_read_size,

    unsigned long *pul_return_size,

    unsigned char *puc_buf

    );

/**
@brief              Function used to write data into an area of Flash Memory

@warning            Writing data into Flash Memory is allow if the desired area
                    is free otherwise needs to erase it by the function
                    #aui_flash_erase before using this function

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened and to be managed to write
                                            data into the desired area
@param[in]          ul_start_addr       =   Start address from which the Flash
                                            Memory will be auto erased then
                                            written
@param[in]          ul_write_size       =   Area of Flash Memory (in @a byte
                                            unit) of Flash Memory to be written
                                            from the start address

@param[out]         pul_return_size     =   Actual number of data (in @a bytes
                                            unit) written into the Flash Memory
                                            (e.g. If @b write_size = 1024 (bytes)
                                            but actually the data written are
                                            just 1000 (bytes), then
                                            @b p_return_size = 1000 (bytes))
@param[out]         puc_buf             =   Buffer used by Flash Memory Module
                                            to hold the data to be written into
                                            Flash Memory

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_write (

    aui_hdl flash_handle,

    unsigned long ul_start_addr,

    unsigned long ul_write_size,

    unsigned long *pul_return_size,

    unsigned char *puc_buf

    );

/**
@brief              Function used to auto-erase an area of Flash Memory then
                    write data into it (i.e. the desired area to be written
                    will be firstly erased for any data already stored then
                    it will be written of new data)

@warning            User must make sure that no need the data stored in the
                    area to be auto-erased then written any more
@param[in]          flash_handle        =   #aui_hdl pointer to the handle
                                            of the Flash Memory Module already
                                            opened and to be managed to auto-erase
                                            then write data into the desired area
@param[in]          ul_start_addr       =   Start address from which the Flash
                                            Memory will be auto-erased then written
@param[in]          ul_write_size       =   Area of Flash Memory (in @a byte unit)
                                            to be auto-erase then written from
                                            the start address
@param[in]          pul_return_size     =   Actual number of data (in @a bytes
                                            unit) written into the Flash Memory
                                            (e.g. If @b write_size = 1024 (bytes)
                                            but actually the data written are
                                            just 1000 (bytes), then
                                            @b p_return_size = 1000 (bytes))

@param[out]         puc_buf             =   Buffer used by Flash Memory Module
                                            to hold the data to be written into
                                            Flash Memory

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value

@note               This function is lower effective than the function
                    #aui_flash_write which is @a recommended to be used
*/
AUI_RTN_CODE aui_flash_auto_erase_write (

    aui_hdl flash_handle,

    unsigned long ul_start_addr,

    unsigned long ul_write_size,

    unsigned long *pul_return_size,

    unsigned char *puc_buf

    );

/**
@brief              Function used to relocate the address for reading/writing
                    into Flash Memory

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened and to be managed to relocate
                                            the address for reading/writing into
                                            the Flash Memory
@param[in]          l_seek_offset       =   Offset (in @a byte unit) added to
                                            the base address @b seek_type in
                                            order to get the <em> absolute
                                            (specific) address </em> from which
                                            read/write into Flash Memory
@param[in]          seek_type           =   Base address to be relocate by
                                            adding the offset @b seek-offset
                                            in order to get the <em> absolute
                                            (specific) address </em> from which
                                            read/write into Flash Memory, as
                                            defined in the enum
                                            #aui_en_flash_seek_type

@param[out]         p_cur_addr          =   Absolute (specific) address (in
                                            @a byte unit) gotten after the
                                            relocation (i.e. the current address)
                                            and from which read/write into Flash
                                            Memory

@return             @b ERROR_CODE       =   As per the enum enum
                                            #aui_en_flash_errno, this error code
                                            can be
                                            - Either @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_seek (

    aui_hdl flash_handle,

    long l_seek_offset,

    aui_flash_seek_type seek_type,

    unsigned int *p_cur_addr

    );

/**
@brief              Function used to write data into Flash Memory from the current
                    file pointer position. The file pointer can be relocated by
                    the function #aui_flash_seek

@attention          About the difference between the functions
                    #aui_flash_seek_write and #aui_flash_write functions:
                    - #aui_flash_write needs a start address for writing into
                      Flash Memory, instead #aui_flash_seek_write no need since
                      it uses the current address as start address for writing
                      into Flash Memory
                    - #aui_flash_seek_write will auto-update the current address,
                      instead #aui_flash_write will not do any auto-update

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already opened
                                            and to be managed to write into Flash
                                            Memory from the relocated address
@param[in]          ul_write_len        =   Area of Flash Memory (in @a byte
                                            unit) to be written from the
                                            relocated address
@param[in]          puc_buf             =   Buffer used by Flash Memory Module
                                            to hold the data to be written into
                                            Flash Memory

@param[out]         pul_actual_len      =   Actual number of bytes written into
                                            Flash Memory
                                            (e.g. If @b write_len = 1024 (bytes)
                                            but actually the data written are
                                            just 1000 (bytes), then
                                            @b actual_len = 1000 (bytes))

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_seek_write (

    aui_hdl flash_handle,

    unsigned long ul_write_len,

    unsigned char *puc_buf,

    unsigned long *pul_actual_len

    );

/**
@brief              Function used to read data from Flash Memory from the current
                    file pointer position. The file pointer can be relocated by
                    the function #aui_flash_seek

@note               About the difference between the functions
                    #aui_flash_seek_read and #aui_flash_read:
                    - #aui_flash_read needs a start address for reading from
                      Flash Memory, instead #aui_flash_seek_read no need since
                      it uses the current address as start address for reading
                      from Flash Memory
                    - #aui_flash_seek_read will auto-update the current address,
                      instead #aui_flash_read will not do any auto-update

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened and to be managed to read
                                            into Flash Memory from the
                                            relocated address
@param[in]          ul_read_len         =   Area of Flash Memory (in @a byte
                                            unit) to be read from the relocated
                                            address

@param[out]         pul_actual_len      =   Actual number of bytes read into
                                            Flash Memory
                                            (e.g. If @b write_len = 1024 (bytes)
                                            but actually the data written are
                                            just 1000 (bytes), then
                                            @b actual_len = 1000 (bytes))
@param[out]         puc_buf             =   Buffer used by Flash Memory Module
                                            to hold the data just read from
                                            Flash Memory

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_seek_read (

    aui_hdl flash_handle,

    unsigned long ul_read_len,

    unsigned char *puc_buf,

    unsigned long *pul_actual_len

    );

/**
@brief              Function used to check whether the specified NOR Flash
                    Memory area is locked (i.e. write-protected) or not.
                    If positive, it cannot be written without calling
                    the function #aui_flash_unlock first.

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened
@param[in]          ul_address          =   The start address (in @a byte unit)
                                            of the Flash Memory area to be checked
                                            for the lock status
@param[in]          ul_size             =   Number of bytes of the Flash Memory
                                            area to be checked for the lock status

@param[out]         pul_lock            =   The lock status of the checked Flash
                                            Memory area, in particular:
                                            - @b 0 = Unlock, then can be written directly
                                            - @b 1 = Locked, then needs to be unlocked
                                                     before writing

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_is_lock (

    aui_hdl flash_handle,

    unsigned long ul_address,

    unsigned long ul_size,

    unsigned long *pul_lock

    );

/**
@brief              Function used to lock (i.e. write-protect) the specified NOR
                    Flash Memory area, then it cannot be written without calling
                    the function #aui_flash_unlock first.

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened
@param[in]          ul_address          =   The start address (in @a byte unit)
                                            of the Flash Memory area to be locked
@param[in]          ul_size             =   Number of bytes of the Flash Memory
                                            area to be locked

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_lock (

    aui_hdl flash_handle,

    unsigned long ul_address,

    unsigned long ul_size

    );

/**
@brief              Function used to unlock the specified NOR Flash Memory area,
                    then it can be written.

@param[in]          flash_handle        =   #aui_hdl pointer to the handle of
                                            the Flash Memory Module already
                                            opened
@param[in]          ul_address          =   The start address (in @a byte unit)
                                            of the Flash Memory area to be unlocked
@param[out]         ul_size             =   Number of bytes of the Flash Memory
                                            area to be unlocked

@return             @b ERROR_CODE       =   As per the enum #aui_en_flash_errno,
                                            this error code can be
                                            - Either @b AUI_FLASH_DRIVER_ERROR value
                                            - Or @b AUI_FLASH_SUCCESS value
                                            - Or @b AUI_FLASH_PARAM_INVALID value
*/
AUI_RTN_CODE aui_flash_unlock (

    aui_hdl flash_handle,

    unsigned long ul_address,

    unsigned long ul_size

    );

/**
@brief              Function used to set a quick reference table of Flash
                    Memory partitions information.

@note               Once this table has been set, each Flash Memory partition
                    can be opened calling the function #aui_flash_open with
                    input parameter the structure #aui_flash_open_param, which
                    contains
                    - A member to specify the Flash Memory @b type
                    - A member as a partition information @b index, which is in turn
                      the element index of the partitions array defined in the
                      structure #aui_flash_partition_table

@param[in]          p_partition_table   =   Pointer to a struct #aui_flash_partition_table
                                            as quick reference table of Flash
                                            Memory partitions information to be set

@return             @b AUI_RTN_SUCCESS  =   Setting of the partitions information
                                            table performed successfully
@return             @b Other_Values     =   Setting of the partition information
                                            table failed forsome reasons

@note   @b 1. This function is @a only available in projects based on <b> TDS OS </b>,
              and @b must be called @a only one time before calling the function
              #aui_flash_open.\n
              If this function is not called, the default partitions information table
              will be used, where:
              - For @b NOR Flash Devices, there will be <b> only one </b> partition which
                index value is 0
              - For @b NAND Flash Devices, there will be <b> only one </b> partition which
                index value is 1

@note   @b 2. In projects based on <b> Linux OS </b>, instead, the Flash Memory partitions
              information are read from the MTD device when calling the function #aui_flash_open
*/
AUI_RTN_CODE aui_flash_init_partition_table (

    aui_flash_partition_table *p_partition_table

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define FLASH_SUCCESS AUI_FLASH_SUCCESS

#define FLASH_PARAM_INVALID AUI_FLASH_PARAM_INVALID

#define FLASH_NO_MEMORY AUI_FLASH_NO_MEMORY

#define FLASH_FEATURE_NOT_SUPPORTED AUI_FLASH_FEATURE_NOT_SUPPORTED

#define FLASH_LSEEK_SET AUI_FLASH_LSEEK_SET

#define FLASH_LSEEK_CUR AUI_FLASH_LSEEK_CUR

#define FLASH_LSEEK_END AUI_FLASH_LSEEK_END

#define FLASH_TYPE_NOR AUI_FLASH_TYPE_NOR

#define FLASH_TYPE_NAND AUI_FLASH_TYPE_NAND

#define FLASH_SUCCESS AUI_FLASH_SUCCESS

#define FLASH_PARAM_INVALID AUI_FLASH_PARAM_INVALID

#define FLASH_NO_MEMORY AUI_FLASH_NO_MEMORY

#define FLASH_FEATURE_NOT_SUPPORTED AUI_FLASH_FEATURE_NOT_SUPPORTED

#define FLASH_DRIVER_ERROR AUI_FLASH_DRIVER_ERROR

#endif

/// @endcond

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

