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
Current ALi author: Alfa.Shang
Last update:        2017.02.10
-->

@file     aui_i2c.h

@brief    Inter-Integrated Circuit (I2C) Module

          <b> Inter-Integrated Circuit (I2C) Module </b> is conveniently used
          to let a host read/write data from/to a slave device.

          Below the essential working flow:
          1. Pre-Run Stage
             - Initialize an I2C bus device
             - Open an I2C bus device
          2. Run Stage
             - Read/Write from/to a slave device mounted on an opened I2C bus
              device
          3. Post-Run Stage
             - Close an opened I2C bus device
             - De-initialize an I2C bus device

@copyright Copyright &copy; 2016 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_I2C_H_

#define _AUI_I2C_H_

/*************************Included Header File List***************************/

#include <aui_common.h>

/******************************Global Type List*******************************/

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Inter-Integrated Circuit (I2C) Module </b> to specify
       the user configuration attributes of an opened I2C bus device
       </div> @endhtmlonly

       Struct to specify the user configuration attributes of the opened I2C bus
       device
*/
typedef struct aui_attr_i2c {

    /**
    Member to specify the
    1. Type of I2C bus devices available to be opened, i.e.
       - <b> Hardware I2C device </b>
       - <b> GPIO simulated I2C device </b>
    2. Maximum number of I2C bus devices for each type, i.e.
       - @b n.4 of Hardware I2C devices
       - @b n.2 of GPIO simulated I2C devices
    3. ID for the chosen I2C bus device, i.e.
       - @b 0-3 for Hardware I2C device
       - @b 4-5 for GPIO simulated I2C device
    */
    unsigned int uc_dev_idx;

} aui_attr_i2c;

/*****************************Global Function List*****************************/

/**
@brief       Function used to perform the initialization of an I2C bus device
             before its opening by the function #aui_i2c_open

@warning     This function can be used @a only in the <b> Pre-Run Stage </b> of
             I2C Module

@param[in]   p_init_callback     = Callback function used to initialize an I2C
                                   bus device, as per comment for the function
                                   pointer @b p_fun_cb
@param[in]   pv_param            = The input parameter of the callback function
                                   @b p_init_callback which is the first input
                                   parameter of this function

@return      @b AUI_RTN_SUCCESS  = I2C bus device initialized successfully
@return      @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in]) is
                                   invalid
@return      @b Other_Values     = Initializing of the I2C bus device failed for
                                   some reasons
*/

AUI_RTN_CODE aui_i2c_init (

    p_fun_cb p_init_callback,

    void *pv_param

    );

/**
@brief       Function used to perform the de-initialization of an I2C bus device
             after its closing by the function #aui_i2c_close

@warning     This function can be used @a only in the <b> Post-Run Stage </b> of
             I2C Module

@param[in]   p_deinit_callback   = Callback function used to de-initialize an
                                   I2C bus device, as per comment for the
                                   function pointer @b p_fun_cb

@return      @b AUI_RTN_SUCCESS  = I2C bus device de-initialized successfully
@return      @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return      @b Other_Values     = De-initializing of the I2C bus device failed
                                   for some reasons
*/
AUI_RTN_CODE aui_i2c_deinit (

    p_fun_cb p_deinit_callback

    );

/**
@brief       Function used to open, register and configure an I2C bus device
             then get the related handle

@warning     This function can @a only be used in the <b> Pre-Run Stage </b>
             of I2C Module

@param[in]   p_attr              = Pointer to a struct #aui_attr_i2c which
                                   collects the user configuration attributes
                                   of the I2C bus device to be opened

@param[out]  *p_i2c_handle       = #aui_hdl pointer to the handle of the
                                   I2C bus device just opened for mounting
                                   different slave devices to be read/written

@return      @b AUI_RTN_SUCCESS  = I2C bus device opened, registered and
                                   configured successfully
@return      @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                   is invalid
@return      @b Other_Values     = Either opening or registering or
                                   configuring of the I2C bus device failed for
                                   some reasons
*/
AUI_RTN_CODE aui_i2c_open (

    aui_attr_i2c *p_attr,

    aui_hdl *p_i2c_handle

    );

/**
@brief       Function used to close an I2C bus device already opened by the
             function #aui_i2c_open, then the related handle will be released
             (i.e. the related resources such as memory, device)

@warning     This function can @a only be used in the <b> Post-Run Stage </b>
             of I2C Module

@param[in]   i2c_handle          = #aui_hdl pointer to the handle of the
                                   I2C bus device already opened and to be closed

@return      @b AUI_RTN_SUCCESS  = I2C bus device closed successfully
@return      @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return      @b Other_Values     = Closing of the I2C bus device failed for
                                   some reasons
*/
AUI_RTN_CODE aui_i2c_close (

    aui_hdl i2c_handle

    );

/**
@brief         Function used to read data from a slave device mounted on the
               I2C bus device opened by the function #aui_i2c_open

@warning       This function can be used when the targeted slave device really
               has a data register to be read, otherwise the function
               #aui_i2c_read_no_subaddr @a must be used

@param[in]     i2c_handle          = #aui_hdl pointer to the handle of the
                                     I2C bus device already opened
@param[in]     ul_chip_addr        = The address of the slave device mounted on
                                     the targeted I2C bus device and to be read
                                     - @b Caution: This address value is based
                                          on the targeted chipset specifications
                                          (the least significant 7 bits of the byte
                                          used to store the address of the chip)\n
                                     - @b Warning: Make sure the slave device
                                          referred by this address value is
                                          actually mounted on the targeted I2C
                                          bus device
@param[in]     ul_sub_addr         = The address of the data register in the slave
                                     device to be read
                                     - @b Caution: This address value is based on
                                       the targeted chipset specifications (the
                                       number of bits is in turn based on the
                                       slave device settings)
                                     - @b Warning: Make sure the data register
                                       referred by this address value really
                                       exists as well as is enabled to be read
                                       for the number of bytes specified in the
                                       input parameter @b ul_buf_len of this
                                       function
@param[in,out] *puc_buff           = Pointer to a proper buffer intended to store
                                     the byte read from the data register in the
                                     slave device
@param[in]     ul_buf_len          = The number of bytes to be read from the data
                                     register in the slave device
                                     - @b Caution: Because the current
                                          specifications, this value should be no
                                          more than 16 (bytes). The latest kernel
                                          version doesn't have this limit.

@return        @b AUI_RTN_SUCCESS  = Reading data from the slave device performed
                                     successfully
@return        @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                     is invalid
@return        @b Other_Values     = Reading data from the slave device failed for
                                     some reasons
*/
AUI_RTN_CODE aui_i2c_read (

    aui_hdl i2c_handle,

    unsigned long ul_chip_addr,

    unsigned long ul_sub_addr,

    unsigned char *puc_buff,

    unsigned long  ul_buf_len

    );

/**
@brief       Function used to write data in the slave device mounted on the
             I2C bus device opened by the function #aui_i2c_open

@warning     This function can be used when the targeted slave device really
             has a data register to be written, otherwise the function
             #aui_i2c_write_no_subaddr @a must be used

@param[in]   i2c_handle          = #aui_hdl pointer to the handle of the I2C
                                   bus device already opened
@param[in]   ul_chip_addr        = The address of the slave device mounted on
                                   the targeted I2C bus device and to be written
                                   - @b Caution: This address value is based on
                                        the targeted chipset specifications
                                        (the least significant 7 bits of the byte
                                        used to store the address of the chip)
                                   - @b Warning: Make sure the slave device
                                        referred by this address value is
                                        actually mounted on the targeted I2C
                                        bus device
@param[in]   ul_sub_addr         = The address of the data register in the slave
                                   device to be written
                                   - @b Caution: This address value is based on
                                        the targeted chipset specifications (the
                                        number of bits is in turn based on the
                                        slave device settings)
                                   - @b Warning: Make sure the data register
                                        referred by this address value really
                                        exists as well as is enabled to be
                                        written for the number of bytes
                                        specified in the input parameter
                                        @b ul_buf_len of this function
@param[in]   *puc_buff           = Pointer to a proper buffer containing the
                                   bytes to be written to the data register in
                                   the slave device
@param[in]   ul_buf_len          = The number of bytes to be written to the data
                                   register in the slave device
                                   - @b Caution: Because the current
                                        specifications, this value should be no
                                        more than 16 (bytes). The latest kernel
                                        version doesn't have this limit

@return      @b AUI_RTN_SUCCESS  = Writing data to the slave device performed
                                   successfully
@return      @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                   is invalid
@return      @b Other_Values     = Writing data to the slave device failed for
                                   some reasons
*/
AUI_RTN_CODE aui_i2c_write (

    aui_hdl i2c_handle,

    unsigned long ul_chip_addr,

    unsigned long ul_sub_addr,

    unsigned char *puc_buff,

    unsigned long  ul_buf_len

    );

/**
@brief          Function used to write and read data to and from a same slave
                device mounted on the I2C bus device opened by the function
                #aui_i2c_open

@warning        This function can be used when the targeted slave device really
                has a data register to be written and read, otherwise the function
                #aui_i2c_write_read_no_subaddr @a must be used

@param[in]      i2c_handle          = #aui_hdl pointer to the handle of the I2C
                                      bus device already opened
@param[in]      ul_chip_addr        = The address of the slave device mounted on
                                      the targeted I2C bus device and to be written
                                      and read
                                      - @b Caution: This address value is based on
                                           the targeted chipset specifications
                                           (the least significant 7 bits of the byte
                                           used to store the address of the chip)
                                      - @b Warning: Make sure the slave device
                                           referred by this address value is
                                           actually mounted on the targeted I2C
                                           bus device
@param[in]      ul_sub_addr         = The address of the data register in the slave
                                      device to be written and read
                                      - @b Caution: This address value is based on
                                           the targeted chipset specifications (the
                                           number of bits is in turn based on the
                                           slave device settings)
                                      - @b Warning: Make sure the data register
                                           referred by this address value really
                                           exists as well as is enabled to be
                                           written and read for the number of
                                           bytes specified in the input parameter
                                           @b ul_buf_len of this function

@param[in]      *puc_write_buff     = Pointer to a proper buffer containing the
                                      bytes to be written to the data register in
                                      the slave device
@param[in]      ul_write_buf_len    = The number of bytes to be written to the data
                                      register in the slave device
                                      - @b Caution: Because the current
                                           specifications, this value should be no
                                           more than 16 (bytes). The latest kernel
                                           version doesn't have this limit.
@param[in,out]  *puc_read_buff      = Pointer to a proper buffer intended to store
                                      the byte read from the data register in the
                                      slave device
@param[in]      ul_read_buf_len     = The number of bytes to be read from the data
                                      register in the slave device
                                      - @b Caution: Because the current
                                           specifications, this value should be no
                                           more than 16 (bytes). The latest kernel
                                           version doesn't have this limit.

@return         @b AUI_RTN_SUCCESS  = Writing and reading data to and from the
                                      slave device performed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                      (i.e. [in], [out]) is invalid
@return         @b Other_Values     = Writing and reading data to and from the
                                      slave device failed for some reasons
*/
AUI_RTN_CODE aui_i2c_write_read (

    aui_hdl i2c_handle,

    unsigned long ul_chip_addr,

    unsigned long ul_sub_addr,

    unsigned char *puc_write_buff,

    unsigned long ul_write_buf_len,

    unsigned char *puc_read_buff,

    unsigned long ul_read_buf_len

    );

/**
@brief         Function used to read data from a slave device mounted on the I2C
               bus device opened by the function #aui_i2c_open

@warning       This function is the alternative to the function #aui_i2c_read, and
               is just used to read data from the targeted slave device which
               doesn't have other data register to be read

@param[in]     i2c_handle          = #aui_hdl pointer to the handle of the
                                     I2C bus device already opened
@param[in]     ul_chip_addr        = The address of the slave device mounted on
                                     the targeted I2C bus device and to be read
                                     - @b Caution: This address value is based
                                          on the targeted chipset specifications
                                          (the least significant 7 bits of the byte
                                          used to store the address of the chip)
                                     - @b Warning: Make sure the slave device
                                          referred by this address value is
                                          actually mounted on the targeted I2C
                                          bus device
@param[in,out] *puc_buff           = Pointer to a proper buffer intended to store
                                     the byte read from the slave device
@param[in]     ul_buf_len          = The number of bytes to be read from the
                                     slave device
                                     - @b Caution: Because the current
                                          specifications, this value should be no
                                          more than 16 (bytes). The latest kernel
                                          version doesn't have this limit.

@return        @b AUI_RTN_SUCCESS  = Reading data from the slave device performed
                                     successfully
@return        @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                     is invalid
@return        @b Other_Values     = Reading data from the slave device failed for
                                     some reasons
*/
AUI_RTN_CODE aui_i2c_read_no_subaddr (

    aui_hdl i2c_handle,

    unsigned long ul_chip_addr,

    unsigned char *puc_buff,

    unsigned long  ul_buf_len

    );

/**
@brief       Function used to write data to a slave device mounted on the I2C
             bus device opened by the function #aui_i2c_open

@warning     This function is the alternative to the function #aui_i2c_write, and
             is just used to write data to the targeted slave device which
             doesn't have other data register to be written

@param[in]   i2c_handle          = #aui_hdl pointer to the handle of the I2C
                                   bus device already opened
@param[in]   ul_chip_addr        = The address of the slave device mounted on
                                   the targeted I2C bus device and to be written
                                   - @b Caution: This address value is based on
                                        the targeted chipset specifications
                                        (the least significant 7 bits of the byte
                                        used to store the address of the chip)
                                   - @b Warning: Make sure the slave device
                                        referred by this address value is
                                        actually mounted on the targeted I2C
                                        bus device
@param[in]   *puc_buff           = Pointer to a proper buffer containing the
                                   bytes to be written to the slave device
@param[in]   ul_buf_len          = The number of bytes to be written to the slave
                                   device
                                   - @b Caution: Because the current
                                        specifications, this value should be no
                                        more than 16 (bytes). The latest kernel
                                        version doesn't have this limit.

@return      @b AUI_RTN_SUCCESS  = Writing data to the slave device performed
                                   successfully
@return      @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                   is invalid
@return      @b Other_Values     = Writing data to the slave device failed for
                                   some reasons
*/
AUI_RTN_CODE aui_i2c_write_no_subaddr (

    aui_hdl i2c_handle,

    unsigned long ul_chip_addr,

    unsigned char *puc_buff,

    unsigned long  ul_buf_len

    );

/**
@brief          Function used to write and read data to and from a same slave device
                mounted on the I2C bus device opened by the function #aui_i2c_open

@note           This function is the alternative to the function #aui_i2c_write_read,
                is just used to write and read data to and from the targeted slave
                device which doesn't have other register to be written and read

@param[in]      i2c_handle          = #aui_hdl point to the handle of the I2C
                                      Module already opened
@param[in]      ul_chip_addr        = The address of the slave device mounted
                                      on the targeted I2C bus device and to be
                                      written and read
                                      - @b Caution: This address value is based on
                                           the targeted chipset specifications
                                           (the least significant 7 bits of the byte
                                           used to store the address of the chip)
                                      - @b Warning: Make sure the slave device
                                           referred by this address value is
                                           actually mounted on the targeted I2C
                                           bus device
@param[in]      *puc_write_buff     = Pointer to a proper buffer containing the
                                      bytes to be written to the data register in
                                      the slave device
@param[in]      ul_write_buf_len    = The number of bytes to be written to the slave
                                      device
                                      - @b Caution: Because the current
                                           specifications, this value should be no
                                           more than 16 (bytes). The latest kernel
                                           version doesn't have this limit.
@param[in,out]  *puc_read_buff      = Pointer to a proper buffer intended to store
                                      the byte read from the slave device
@param[in]      ul_read_buf_len     = The number of bytes to be read from the
                                      slave device
                                      - @b Caution: Because the current
                                           specifications, this value should be no
                                           more than 16 (bytes). The latest kernel
                                           version doesn't have this limit.

@return         @b AUI_RTN_SUCCESS  = Writing data to the slave device performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Writing data to the slave device failed for
                                      some reasons
*/
AUI_RTN_CODE aui_i2c_write_read_no_subaddr (

    aui_hdl i2c_handle,

    unsigned long ul_chip_addr,

    unsigned char *puc_write_buff,

    unsigned long ul_write_buf_len,

    unsigned char *puc_read_buff,

    unsigned long ul_read_buf_len

    );

#endif

/* END OF FILE */


