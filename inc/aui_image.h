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
Current Author:     Alfa.Shang
Last update:        2017.04.01
-->

@file   aui_image.h

@brief  Image Module

        Image Module is a sub-module of the Media Player (MP) Module and is used
        to play image file on the screen. It must cooperate with its own file
        system and the only input is the file path of the image to play

@note   For further details, please refer to ALi document
        <b><em>
        ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Media Player Module"
        </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_IMAGE_H

#define _AUI_IMAGE_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List******************************/

/**
Macro to define the maximum size of image file can be supported
*/

#define MAX_FILENAME_LEN 128

/**
Macro to define the maximum number of image files can be supported
*/

#define MAX_MULTI_FILE_NUM 20

/*******************************Global Type List*******************************/

/// @cond

/**
Enum to specify the messages for Image Module
*/
typedef enum aui_image_message {

    /**
    Value to specify the message which indicates the driver completed decoding
    and the picture has already been showed on the screen successfully.

    @note For this message, the parameter @b pv_data of the function pointer
          #aui_image_message_callback is zero (0)
    */
    AUI_IMAGE_SHOW_COMPLETE,

    /**
    Value to specify the maximum number of messages available for Image Module
    */
    AUI_IMAGE_MESSAGE_MAX

} aui_image_message;

/**
Function pointer to specify the type of callback function for the enum
#aui_image_message, where the output parameters are explained below:
- @b msg           = The type of message from the player, as defined in the enum
                     #aui_image_message
- @b pv_data       = Data corresponding to a message which tells user the detailed
                     information about either decoding or playing
- @b pv_user_data  = User data given from the member @b user_data of the struct
                     #aui_attr_image
*/
typedef void (*aui_image_message_callback) (

    aui_image_message msg,

    void *pv_data,

    void *pv_user_data

    );

/**
Enum to specify the <b> Image Display Modes <\b> on the TV screen
*/
typedef enum aui_image_mode {

    /**
    Value to specify the <b> Fullview Mode </b>
    */
    AUI_MODE_FULL,

    /**
    Value to specify the <b> Preview Mode </b>
    */
    AUI_MODE_PREVIEW,

    /**
    Value to specify the <b> Multiview Mode </b>

    @warning  This mode is not supported currently
    */
    AUI_MODE_MULTIIEW

} aui_image_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Image Module </b> to specify the source/destination
        picture size
        </div> @endhtmlonly

        Struct to specify the source/destination picture size, i.e. the image
        rectangle below:

                 |(0,0)- - - - - - - - - - - - - - (x,0)|
                 |                                      |
                 |                                      H
                 |                                      E
                 |               Display                I
                 |                Image                 G
                 |                                      H
                 |                                      T
                 |                                      |
                 |(0,y)- - - - - -WIDTH- - - - - - (x,y)|

*/
typedef struct aui_image_rect{

    /**
    Member to specify the <b> X-axis coordinate </b> of the <b> top left corner </b>
    (which value start from zero (0)) of the image rectangle
    */
    int start_x;

    /**
    Member to specify the <b> Y-axis coordinate </b> of the <b> top left corner </b>
    (which value start from zero (0)) of the image rectangle
    */
    int start_y;

    /**
    Member to specify the @b width of the image rectangle
    */
    int width;

    /**
    Member to specify the @b height of the image rectangle
    */
    int height;

} aui_image_rect;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Image Module </b> to specify the image attributes
        available to be configured
        </div> @endhtmlonly

        Struct to specify the image attributes available to be configured
*/
typedef struct aui_attr_image {

    /**
    Member to specify the image file path
    */
    unsigned char uc_file_name[MAX_FILENAME_LEN];

    /**
    Member to specify all the image file paths for multiview mode
    */
    unsigned char uc_multi_file_name[MAX_MULTI_FILE_NUM][MAX_FILENAME_LEN];

    /**
    Member to specify the image display mode on the TV screen, as defined in
    the enum #aui_image_mode
    */
    aui_image_mode en_mode;

    /**
    Member to specify the callback function related to an image message,
    as per comment of the function pointer #aui_image_message_callback
    */
    aui_image_message_callback aui_image_cb;

    /**
    Member to specify the user data to be passed to the callback function related
    to an image message mentioned in the member @b aui_image_cb of this struct
    */
    void *user_data;

} aui_attr_image, *aui_p_attr_image;

/**
Function pointer to specify the callback function used during the initialization/
de-initialization of Image Module
*/
typedef void (*aui_func_image_init)(

void

);

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Image Module </b> to specify the location of image files
        for multiview mode
        </div> @endhtmlonly

        Struct to specify the location of image files for multiview mode
*/
typedef struct aui_multi_name_set {

    /**
    Member to specify all the iamge file path for multiview mode
    */
    unsigned char uc_multi_file_name[MAX_MULTI_FILE_NUM][MAX_FILENAME_LEN];

    /**
    Member to specify the number of image file path for multiview mode
    */
    unsigned int ui_file_num;

} aui_multi_name_set;

/**
Enum to specify miscellaneous settings available to be performed for Image Module
*/
typedef enum aui_image_item_set {

    /**
    Value to specify the image decoding has been done
    */
    AUI_IMAGE_SET_DECDONE_FLAG,

    /**
    Value to specify the setting of the image file path for multiview mode
    */
    AUI_IMAGE_SET_MULTI_FILENAME,

    /**
    Value to specify the setting of the image file path for fullview mode
    */
    AUI_IMAGE_SET_FILE_NAME

} aui_image_item_set;

/**
Enum to specify miscellaneous information available to be gotten from the Image
Module
*/
typedef enum aui_image_item_get {

    /**
    Value to specify the getting of the image information available
    */
    AUI_IMAGE_GET_INFO

} aui_image_item_get;

/**
Enum to specify the rotation angles of the image available to be set
*/
typedef enum aui_image_angle {

    /**
    Value to specify the <b> original angle </b> of the image
    */
    AUI_ANGLE_ORG,

    /**
    Value to specify a rotation of <b> 90 degrees clockwise </b> from the original
    angle of the image
    */
    AUI_ANGLE_ALONG_90,

    /**
    Value to specify a rotation of <b> 180 degrees clockwise </b> from the original
    angle of the image
    */
    AUI_ANGLE_180,

    /**
    Value to specify a rotation of <b> 90 degrees counterclockwise </b> from the
    original angle of the image
    */
    AUI_ANGLE_CONTER_90

} aui_image_angle;

/**
Enum to specify the shifting & moving direction of the image available to be set
*/
typedef enum aui_image_direction {

    /**
    Value to specify <b> left shifting </b> of the image
    */
    AUI_DIR_LEFT,

    /**
    Value to specify <b> right shifting </b> of the image
    */
    AUI_DIR_RIGHT,

    /**
    Value to specify <b> moving up </b> of the image
    */
    AUI_DIR_UP,

    /**
    Value to specify <b> moving down </b> of the iamge
    */
    AUI_DIR_DOWN

} aui_image_direction;

/// @cond

/**
Enum to specify the enlargement options of the image available to be set
*/
enum aui_image_enlarge {

    /**
    Value to specify the <b> original shape </b> of the image
    */
    AUI_IMAGE_ORG,

    /**
    Value to specify <b> Two (2) times enlargement </b> of the original shape
    */
    AUI_IMAGE_2x,

    /**
    Value to specify <b> Four (4) times enlargement </b> of the original shape
    */
    AUI_IMAGE_4x,

    /**
    Value to specify <b> Eight (8) times enlargement </b> of the original shape
    */
    AUI_IMAGE_8x,

    /**
    Value to specify <b> Sixteen (16) times enlargement </b> of the original shape
    */
    AUI_IMAGE_16x

};

/// @endcond

/**
@brief  @htmlonly <div class="details"> Struct of the <b> Image Module </b> to
        specify the image information available to be gotten
        </div> @endhtmlonly

        Struct to specify the image information available to be gotten

@warning  This struct is no longer supported then is @a deprecated
*/
typedef struct aui_image_info {

  /**
  Member to specify the @b size of the file picture
  */
  unsigned long fsize;

  /**
  Member to specify the @b width of the picture
  */
  unsigned long width;

  /**
  Member to specify the @b height of the picture
  */
  unsigned long height;

  /**
  Member to specify the <b> total number of pixels </b> of the picture
  */
  unsigned long bbp;

} aui_image_info;

/// @endcond

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief        Function used to open the Image Module and configure the desired
              attributes, then get the related handle

@warning      This function can @a only be used in the <b> Pre-Run Stage </b>
              of the Image Module, in particular:
              - Either after performing the initialization of the Image Module
                by the function #aui_image_init for the first opening of the
                Image Module
              - Or after closing the Image Module by the function #aui_image_close,
                considering the initialization of the Image Module has been
                performed previously by the function #aui_image_init

@param[in]    p_image_attr           = Pointer to a struct #aui_attr_image,
                                       which collects the desired attributes
                                       for the Image Module.

@param[out]   pp_handle_image        = Pointer to the handle of the image Module
                                       just opened

@return       @b AUI_RTN_SUCCESS     = Opening of the Image Module performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one parameter (i.e. [in],[out])
                                       is invalid
@return       @b Other_Values        = Opening of the Image Module failed for
                                       some reasons
*/
AUI_RTN_CODE aui_image_open (

    aui_attr_image *p_image_attr,

    void **pp_handle_image

    );

/**
@brief        Function used to close the Image Module already opened by the
              function #aui_image_open, then the related handle (i.e. the related
              resources such as memory, device) will be released

@warning      This function can @a only be used in the <b> Post-Run Stage </b>
              of the Image Module in pair with its the opening by the function
              #aui_image_open. After closing the Image Module, user can
              - Either perform the de-initialization of the Image Module by the
                function #aui_image_de_init
              - Or open again the image Module by the function #aui_image_open,
                considering the initialization of the Image Module has been
                performed previously by the function #aui_image_init

@param[in]    p_image_attr           = Pointer to a struct #aui_attr_image,
                                       which collects the desired attributes
                                       for the Image Module to be closed.
                                       - @b Caution: It is suggested to set as
                                            @b NULL cause the struct #aui_attr_image
                                            doesn't store any memory pointer in heap.

@param[out]   pp_handle_image        = Pointer to the handle of the Image Module
                                       already opened

@return       @b AUI_RTN_SUCCESS     = Closing of the image Module performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one parameter (i.e. [in], [out])
                                       is invalid
@return       @b Other_Values        = Closing of the Image Module failed for
                                       some reasons
*/
AUI_RTN_CODE aui_image_close (

    aui_attr_image *p_image_attr,

    void **pp_handle_image

    );

/**
@brief        Function used to start playing one or multiple image files

@param[in]    pv_handle_image        = Pointer to the handle of Image module
                                       already opened
@param[in]    p_ary_rect             = The position for playing the image files
                                       at the first time
@param[in]    i_num                  = The image file numbers which need to be
                                       showed in Multiview Mode
                                       - @b Caution: This parameter should be
                                            set to as @b 1 if the image display
                                            mode is not multiview mode


@return       @b AUI_RTN_SUCCESS     = Starting to play image files performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in])
                                       is invalid
@return       @b Other_Values        = Starting to play image files failed for
                                       some reasons

@note         It supposed/suggested to set the parameters as following:

              - For <b> Fullview Mode </b>:
                - @b p_ary_rect = NULL
                - @b i_num = 1
              - For <b> Preview Mode </b>:
                - Set the position for the piture with @b p_ary_rect
                - @b i_num = 1
              - For <b> Multiview Mode </b>:
                - @b p_ary_rect should be an array of the positions
                - @b i_num should be the image file numbers
*/
AUI_RTN_CODE aui_image_start (

    void *pv_handle_image,

    aui_image_rect *p_ary_rect,

    int i_num

    );

/**
@brief        Function used to perform the initialization of the Image Module
              before its opening by the function #aui_image_open

@pre          Important preconditon for using this function is the
              initialization of Media Player Module: Image Module will perform
              some audio decoder and play engine configuration i.e. it will affect
              Media Player Module, so it is necessary initialize Media Player
              Module before initialize Image Module.


@warning      This function can be used only in the <b> Pre-Run Stage </b> of
              the Image Module

@param[in]    fn_image_init          = Callback function used for the
                                       initialization of the Image Module, as
                                       per comment for the function pointer
                                       #aui_func_image_init

@return       @b AUI_RTN_SUCCESS     = Initializing of the Image Module perfomed
                                       successfully
@return       @b AUI_RTN_EINVAL      = The input parameter (i.e. [in]) is invalid
@return       @b Other_Values        = Initializing of the Image Module failed
                                       for some reasons
*/
AUI_RTN_CODE aui_image_init (

    aui_func_image_init fn_image_init

    );

/**
@brief        Function used to perform the de-initialization of the Image Module
              after its closing by the function #aui_image_close

@param[in]    fn_image_de_init       = Callback function used for the
                                       de-initialization of the Image Module,
                                       as per comment for the function pointer
                                       #aui_func_image_init

@return       @b AUI_RTN_SUCCESS     = De-initializing of the Image Module
                                       performed successfully
@return       @b AUI_RTN_EINVAL      = The input parameter (i.e. [in]) is invalid
@return       @b Other_Values        = De-initializing of the Image Module
                                       failed for some reasons

@note         This function is @a only used in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_image_de_init (

    aui_func_image_init fn_image_de_init

    );

/**
@brief        Function used to perform a specific setting for Image Module,
              between its opening and starting by respectively the functions
              #aui_image_open and #aui_image_start

@param[in]    pv_hdl_image           = The handle of Image Module already opened
@param[in]    ul_item                = The item related to the specific setting
                                       to be performed, as defined in the enum
                                       #aui_image_item_set
@param[in]    pv_param               = The pointer as per the description of the
                                       specific setting to be performed

@return       @b AUI_RTN_SUCCESS     = Setting of the Image Module performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in])
                                       is invalid
@return       @b Other_Values        = Specific setting for Image Module failed
                                       for some reasons
*/
AUI_RTN_CODE aui_image_set (

    void *pv_hdl_image,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief        Function used to get a specific information from the Image Module,
              after its opening and starting by respectively the functions
              #aui_image_open and #aui_image_start

@param[in]    pv_hdl_image           = The handle of Image Module already opened
@param[in]    ul_item                = The item related to the specific information
                                       of the Image Module to be gotten, as defined
                                       in the enum #aui_image_item_get
@param[out]   pv_param               = The pointer as per the description of the
                                       specific information to be gotten

@return       @b AUI_RTN_SUCCESS     = Getting of the specific information of the
                                       Image Module performed successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in]) is
                                       invalid
@return       @b Other_Values        = Getting of the specific information of the
                                       Image Module failed for some reasons
*/
AUI_RTN_CODE aui_image_get (

    void *pv_hdl_image,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief        Function used to zoom the display images in Fullview Mode

@param[in]    pv_handle_image        = Pointer to the handle of the Image Module
                                       already opened
@param[in]    p_src_rect             = Source picture size, as defined in the
                                       struct #aui_image_rect
@param[in]    p_dst_rect             = Destination picture size, as defined in the
                                       struct #aui_image_rect

@return       @b AUI_RTN_SUCCESS     = Zooming of the display image performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in])
                                       is invalid
@return       @b Other_Values        = Zooming of the display image failed for
                                       some reasons
*/
AUI_RTN_CODE aui_image_zoom (

    void *pv_handle_image,

    aui_image_rect *p_src_rect,

    aui_image_rect *p_dst_rect

    );

/**
@brief        Function used to rotate the display image

@param[in]    pv_handle_image        = Pointer to the handle of the Image Module
                                       already opened
@param[in]    en_angle               = Rotation angles, as defined in the enum
                                       #aui_image_angle

@return       @b AUI_RTN_SUCCESS     = Rotating of the display image performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in])
                                       is invalid
@return       @b Other_Values        = Rotating of the display image failed for
                                       some reasons
*/
AUI_RTN_CODE aui_image_rotate (

    void *pv_handle_image,

    aui_image_angle en_angle

    );

/**
@brief        Function used to set the image display mode

@param[in]    pv_handle_image        = Pointer to the handle of the Image Module
                                       already opened
@param[in]    en_mode                = Display modes, as defined in the enum
                                       #aui_image_mode

@return       @b AUI_RTN_SUCCESS     = Setting of the image display mode performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = The input parameter (i.e. [in]) is
                                       invalid
@return       @b Other_Values        = Setting of the image display mode failed
                                       for some reasons
*/
AUI_RTN_CODE aui_image_display_mode_set (

    void *pv_handle_image,

    aui_image_mode en_mode

    );

/**
@brief        Function used to move an image to a desired direction

@param[in]    pv_handle_image        = Pointer to the handle of the Image Module
                                       already opened
@param[in]    en_dir                 = Moving directions, as defined in the enum
                                       #aui_image_direction
@param[in]    step                   = Number of steps for moving the image

@return       @b AUI_RTN_SUCCESS     = Moving of the image to the desired direction
                                       performed successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in])
                                       is invalid
@return       @b Other_Values        = Moving of the image to the desired direction
                                       failed for some reasons
*/
AUI_RTN_CODE aui_image_move (

    void *pv_handle_image,

    aui_image_direction en_dir,

    int step

    );

/// @cond

/**
@brief        Function used to enlarge an image

@param[in]    pv_handle_image        = Pointer to the handle of the Image Module
                                       already opened
@param[in]    en_times               = The size of enlargement, as defined in
                                       the enum #aui_image_enlarge

@return       @b AUI_RTN_SUCCESS     = Enlarging of the image performed
                                       successfully
@return       @b AUI_RTN_EINVAL      = At least one input parameter (i.e. [in])
                                       is invalid
@return       @b others              = Enlarging of the image failed for some
                                       reasons
*/
AUI_RTN_CODE aui_image_enlarge (

    void *pv_handle_image,

    enum aui_image_enlarge en_times

    );

/// @endcond

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */



