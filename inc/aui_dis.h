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
Current Author:     Adolph.Liu
Last update:        2017.02.13
-->

@file       aui_dis.h

@brief      Display Engine (DIS) Module

            <b> Display Engine (DIS) Module </b> consists of two (2) hardware
            modules:
            - DE (Display Engine)
            - TV Encoder

            About @b DE, it is used for post-processing video data and output
            it through the TV Encoder. The following features related to the
            video output are supported by DE:
            - Turn on/off video layer
            - Capture screen
            - Video zoom-in/out
            - Image sharpening
            - Switch auto display mode
            - Match mode
            - Aspect ratio adjustment
            - Interlace/Progressive scan

            About <b> TV Encoder </b>, it is used to encode the video data and
            output it via DACs. The following features related to video output
            are supported by TV Encoder:
            - CVBS/SVideo/RGB/YCbCr output
            - TV system

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Display Engine (DIS) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_DIS_H

#define _AUI_DIS_H

/**************************Included Header File List***************************/

#include "aui_common.h"

/******************************Global Macro List*******************************/

/**
Macro to specify that presently the structs contained in this header file are
designed to support up to one (4) DACs
*/
#define AUI_DIS_MAX_DAC_NUM (4)

/**
Macro to specify that presently the structs contained in this header file are
designed to support up to two (2) display devices to output
- Standard Definition Television (SDTV)
- High Definition Television (HDTV)
*/
#define AUI_DIS_DEV_CNT_MAX (2)

/**
Macro to specify the limit on X-axis as the maximum value of an user coordinate
*/
#define AUI_PICTURE_WIDTH 720

/**
Macro to specify the limit on Y-axis as the maximum value of an user coordinate
*/
#define AUI_PICTURE_HEIGHT 2880

/*******************************Global Type List*******************************/

/**
Enum to specify the available different <b> video display formats </b> supported
by DIS Module
*/
typedef enum aui_dis_tvsys {

    /**
    Value to specify the <b> PAL4.43(==PAL_BDGHI), fh=15.625 and fv=50 </b>
    video display format, in particular:
    - @b 576I video display format for interlaced display
    - @b 576P video display format for progressive display
    */
    AUI_DIS_TVSYS_PAL = 0,

    /**
    Value to specify the <b> NTSC3.58, fh=15.734 and fv=59.94 </b> video display
    format, in particular
    - @b 480I video display format for interlaced display
    - @b 480P video display format for progressive display
    */
    AUI_DIS_TVSYS_NTSC,

    /**
    Value to specify the <b> PAL3.58, fh=15.734 and fv=59.94 </b> video display
    format
    */
    AUI_DIS_TVSYS_PAL_M,

    /**
    Value to specify the <b> PAL4.43(changed PAL mode), fh=15.625 and fv=50 </b>
    video display format
    */
    AUI_DIS_TVSYS_PAL_N,

    /**
    Value to specify the <b> PAL, fh=15.734 and fv=59.94 </b> video display format
    */
    AUI_DIS_TVSYS_PAL_60,

    /**
    Value to specify the <b> NTSC4.43, fh=15.734 and fv=59.94 </b> video display
    format
    */
    AUI_DIS_TVSYS_NTSC_443,

    /**
    @attention  This video display format is @a reserved to ALi R&D Dept. then
    user can ignore it
    */
    AUI_DIS_TVSYS_MAC,

    /**
    Value to specify the <b> 720p50 </b> video display format, which has
    720 lines per frame and 50 frames per second

    @note   ALi chipsets don't support the video display format <b> 720i50 </b>
    */
    AUI_DIS_TVSYS_LINE_720_50,

    /**
    Value to specify the <b> 720p60 </b> video display format, which has
    720 lines per frame and 60 frames per second

    @note   ALi chipsets don't support the video display format <b> 720i60 </b>
    */
    AUI_DIS_TVSYS_LINE_720_60,

    /**
    Value to specify the
    - @b 1080p25 video display format for progressive display
    - @b 1080i50 video display format for interlaced  display

    which has 1080 lines per frame and 25 frames per second.

    User can select the desired video display format by the function
    #aui_dis_tv_system_set with the parameter #uc_b_progressive
    */
    AUI_DIS_TVSYS_LINE_1080_25,

    /**
    Value to specify the
    - @b 1080p30 video display format for progressive display
    - @b 1080i60 video display format for interlaced  display

    which has 1080 lines per frame and 30 frames per second.

    User can select the desired video display format by the function
    #aui_dis_tv_system_set with the parameter #uc_b_progressive
    */
    AUI_DIS_TVSYS_LINE_1080_30,

    /**
    Value to specify the <b> 1080p50 </b> video display format, which has
    1080 lines per frame and 50 frames per second.

    @note   This value is only for the @b 1080p50 video display format, for the
            @b 1080i50 video display format please refer to the enum value
            #AUI_DIS_TVSYS_LINE_1080_25
    */
    AUI_DIS_TVSYS_LINE_1080_50,

    /**
    Value to specify the <b> 1080p60 </b> video display format, which has
    1080 lines per frame and 60 frames per second.

    @note   This value is only for the @b 1080p60 video display format, for the
            @b 1080i60 video display format please refer to the enum value
            #AUI_DIS_TVSYS_LINE_1080_30
    */
    AUI_DIS_TVSYS_LINE_1080_60,

    /**
    Value to specify the <b> 1080p24 </b> video display format, which has
    1080 lines per frame and 24 frames per second.

    @note   The @b 1080i24 video display format is not supported
    */
    AUI_DIS_TVSYS_LINE_1080_24,

    /**
    @attention  This video display format is @a reserved to ALi R&D Dept. then
                user can ignore it
    */
    AUI_DIS_TVSYS_LINE_1152_ASS,

    /**
    @attention  This video display format is @a reserved to ALi R&D Dept. then
                user can ignore it
    */
    AUI_DIS_TVSYS_LINE_1080_ASS,

    /**
    Value to specify the <b> PAL3.58, fh=15.625 and fv=50 </b> video display
    format
    */
    AUI_DIS_TVSYS_PAL_NC,

    /**
    Value to specify the <b> 3840x2160 @24Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_3840X2160_24,

    /**
    Value to specify the <b> 3840x2160 @25Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_3840X2160_25,

    /**
    Value to specify the <b> 3840x2160 @30Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_3840X2160_30,

    /**
    Value to specify the <b> 3840x2160 @50Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_3840X2160_50,

    /**
    Value to specify the <b> 3840x2160 @60Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_3840X2160_60,

    /**
    Value to specify the <b> 4096x2160 @24Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_4096X2160_24,

    /**
    Value to specify the <b> 4096x2160 @25Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_4096X2160_25,

    /**
    Value to specify the <b> 4096x2160 @30Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_4096X2160_30,

    /**
    Value to specify the <b> 4096x2160 @50Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_4096X2160_50,

    /**
    Value to specify the <b> 4096x2160 @60Hz </b> video display format
    */
    AUI_DIS_TVSYS_LINE_4096X2160_60


} aui_dis_tvsys;

/**
Enum to specify the <b> Picture Layer Display Order </b>

@note   Quick understanding by the enumerator names:
        - About the <b> Picture Layer </b>
          - @b MP represents @b fb1 as the @b Video layer
          - @b GMAS represents @b fb2 as the @b Subtitle layer
          - @b GMAF represents @b fb0 as the @b GMA layer
          - @b AUXP is not currently used
        - About the <b> Display Order </b>:
          - The extreme left side is the lowest layer
          - The extreme right side is the uppermost layer

@note   @b Example \n
        The enumerator name #AUI_DIS_LAYER_ORDER_MP_GMAS_GMAF_AUXP represents
        the picture layer display order <b> fb1 -> fb2 -> fb0 </b> where
        - @b fb0 is the uppermost layer
        - @b fb1 is the lowest layer
*/
typedef enum aui_dis_layer_blend_order {

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb2 -> fb0 </b>
    where
    - @b fb0 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_MP_GMAS_GMAF_AUXP = 0,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb2 -> fb0 </b>
    where
    - @b fb0 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_MP_GMAS_AUXP_GMAF,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb0 -> fb2 </b>
    where
    - @b fb2 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_MP_GMAF_GMAS_AUXP,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb0 -> fb2 </b>
    where
    - @b fb2 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_MP_GMAF_AUXP_GMAS,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb2 -> fb0 </b>
    where
    - @b fb0 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_MP_AUXP_GMAS_GMAF,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb0 -> fb2 </b>
    where
    - @b fb2 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_MP_AUXP_GMAF_GMAS,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb2 -> fb0 </b>
    where
    - @b fb0 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_AUXP_MP_GMAS_GMAF,

    /**
    Value to specify the picture layer display order
    <b> fb1 -> fb0 -> fb2 </b>
    where
    - @b fb2 is the uppermost layer
    - @b fb1 is the lowest layer
    */
    AUI_DIS_LAYER_ORDER_AUXP_MP_GMAF_GMAS

} aui_dis_layer_blend_order;

/**
Enum to specify the <b> DAC output </b> with signal component in either @b YUV,
@b RGB, @b SVIDEO or @b CVBS output type

@note   At present, ALi DAC supports the following output type
        - @b YUV
        - @b RGB
        - @b SVIDEO
        - @b CVBS

@note   Obviously each output type needs a different number of DACs, in
        particular:
        - @b YUV uses 3 DACs
        - @b RGB uses 3 DACs
        - @b SVIDEO uses 2 DACs
        - @b CVBS uses 1 DAC
*/
typedef enum aui_dis_output_type {

    /**
    Value to specify the DAC output with @b Y signal component in @b YUV output
    type
    */
    AUI_DIS_TYPE_Y = 0,

    /**
    Value to specify the DAC output with @b U signal component in @b YUV output
    type
    */
    AUI_DIS_TYPE_U,

    /**
    Value to specify the DAC output with @b V signal component in @b YUV output
    type
    */
    AUI_DIS_TYPE_V,

    /**
    Value to specify the DAC output in @b CVBS output type
    */
    AUI_DIS_TYPE_CVBS,

    /**
    Value to specify the DAC output with @b Y signal component in @b SVIDEO output
    type
    */
    AUI_DIS_TYPE_SVIDEO_Y,

    /**
    Value to specify the DAC output with @b C signal component in @b SVIDEO output
    type
    */
    AUI_DIS_TYPE_SVIDEO_C,

    /**
    Value to specify the DAC output with @b R signal component in @b RGB output
    type
    */
    AUI_DIS_TYPE_R,

    /**
    Value to specify the DAC output with @b G signal component in @b RGB output
    type
    */
    AUI_DIS_TYPE_G,

    /**
    Value to specify the DAC output with @b B signal component in @b RGB output
    type
    */
    AUI_DIS_TYPE_B,

    /**
    Value to specify the DAC output is not registered
    */
    AUI_DIS_TYPE_NONE = 0xFF
    
} aui_dis_output_type;

/**
Enum to specify the <b> View Mode </b> of the Display Device
*/
typedef enum aui_view_mode {

    /**
    Value to specify the <b> Preview Mode </b> with which the video decoder will
    resize the video frames for the display engine with scale rate 1/2, 1/4, 1/8,
    then the display engine will zoom in/out the video frame to the rectangle
    specified

    @warning    This mode can be used @a only for H.264 and MPEG2 Video Format
    */
    AUI_VIEW_MODE_PREVIEW = 0,

    /*
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_VIEW_MODE_MULTI,

    /**
    Value to specify the <b> Full Mode </b> with which the video frames will be
    shown fully in the display device
    */
    AUI_VIEW_MODE_FULL

} aui_view_mode;

/**
Enum to specify miscellaneous information available to be gotten from the DIS
Module

@note   This enum is used by the function #aui_dis_get to get a specific
        information where:
        - The parameter @b ul_item takes the items related to the specific
          information to get
        - The parameter @b pv_param takes the pointer as per the description
          of the specific information to get
*/
typedef enum aui_dis_item_get {

    /**
    Value used to get all the available display information such as
    - TV system
    - Source picture height and width
    - Progressive and interlaced scan
    - etc.

    @note   The parameter @b pv_param takes the pointer to the current display
            information as defined in the struct #aui_dis_info
    */
    AUI_DIS_GET_INFO,

    /**
    Value used to get the status of the boot media

    @warning    DIS Module will not be available to AUI layer if it is showing
                the boot media

    @note   @b 1. The parameter @ pv_param is a @a flag which can take the
                  following values
                  - @b 0 = The boot media is still running then DIS Module is
                           not available
                  - @b 1 = The boot media has ended then DIS Module is available

    @note   @b 2. This value is available @a only in projects based on <b> Linux
                  OS </b>
    */
    AUI_DIS_GET_BOOT_MEDIA_STATUS,

    /**
    Value to spcify the total number of items available in this enum
    */
    AUI_DIS_GET_LAST

} aui_dis_item_get;

/**
Enum to specify miscellaneous settings available to be performed for DIS Module

@note   This enum is used by the function #aui_dis_set to perform a specific
        setting where
        - The parameter @b ul_item takes the item related to the specific
          setting to perform
        - The parameter @b pv_param takes the pointer as per the description
          of the specific setting to perform
*/
typedef enum aui_dis_item_set {

    /**
    Value used to enable/disable the DE to open the video layer automatically

    @note   The parameter @b pv_param is a @a flag which can take the following
            value:
            - @b 0 = This is the default value when system is up. It allows the
                     Display Engine (DE) to automatically open the video layer
                     when VE sends the first picture.
            - @b 1 = Disable the video layer auto-on feature. The DE will NOT
                     automatically open the video layer when VE sends the first
                     picture. The application need to manually control the video
                     layer by the function #aui_dis_video_enable.
    */
    AUI_DIS_SET_AUTO_WINONOFF,

    /**
    @attention  This value is @a reserved to aLi R&D Dept. then user can ignore it
    */
    AUI_DIS_SET_APS,

    /**
    Value used to set the displaying of the CGMS-A output signal
    @note   The parameter @b pv_param is a @a value which can take the following
            value:
            - @b 0 = Copy freely
            - @b 1 = Copy no more
            - @b 2 = Copy once
            - @b 3 = Copy never
    @note   ALi only support value 3 right now
    */
    AUI_DIS_SET_CGMS,

    /**
    Value used to enable/disable showing the auxiliary layer

    @note   The parameter @b pv_param is a @a flag which take the following
            values:
            - @b 1 = Enabled then the DE will show the auxiliary layer
            - @b 0 = Disabled then the DE will not show the auxiliary layer
    */
    AUI_DIS_SET_AUXP_ENABLE,

    /**
    Value used to set HD TV encode signal source

    @note   The parameter @b pv_param takes a value of the enum #aui_dis_format
            as follows
            - @b AUI_DIS_HD then the HD TV encoder will be attached to DEN with
              the signal source from DEN.
            - @b AUI_DIS_SD then the HD TV encoder will be attached to DEO with
              the signal source from DEO
    */
    AUI_DIS_SET_TVESDHD_SOURCE,

    /**
    Value to specify the total number of items available in this enum
    */
    AUI_DIS_SET_LAST

} aui_dis_item_set;

/**
Enum used to de-register a DAC with a specific signal output type

@note   This enum is used by the function #aui_dis_dac_unreg to perform a
        specific de-registering where a parameter takes the item related to the
        specific de-registering to perform
*/
typedef enum aui_dis_unreg_type {

    /**
    Value used to de-register the DAC with the YUV output type
    */
    AUI_DIS_TYPE_UNREG_YUV = 0,

    /**
    Value to de-register the DAC with the RGB output type
    */
    AUI_DIS_TYPE_UNREG_RGB,

    /**
    Value to de-register the DAC with the SVIDEO output type
    */
    AUI_DIS_TYPE_UNREG_SVIDEO,

    /**
    Value to de-register the DAC with the CVBS output type
    */
    AUI_DIS_TYPE_UNREG_CVBS

} aui_dis_unreg_type;

/**
Function pointer to specify the callback function used to get the configuration
parameters of DIS Module

@note   This Function pointer can be used @a only in projects based on <b> TDS
        OS </b>
*/
typedef void (*aui_dis_open_param_callback) (

    void *p_param

    );

/**
Enum to specify HD/SD display device
*/
typedef enum aui_dis_format {

    /**
    Value to specify HD display device
    */
    AUI_DIS_HD,

    /**
    Value to specify SD display device
    */
    AUI_DIS_SD

} aui_dis_format;

/**

@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify the video
        background color
        </div> @endhtmlonly

        Struct to specify the <b> Video Background Color </b> which is set by
        the @b YCbCr color space
*/
typedef struct aui_st_dis_color {

    /**
    Member to specify the video background color component @b Y, which is
    defined to have a nominal 8-bit representation with a range of [16,235]
    */
    unsigned char y;

    /**
    Member to specify the video background color component @b Cb, which is
    defined to have a nominal 8-bit representation with a range of [16,235]
    */
    unsigned char cb;

    /**
    Member to specify the video background color component @b Cr, which is
    defined to have a nominal 8-bit representation with a range of [16,235]
    */
    unsigned char cr;

} aui_dis_color;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify the
        attributes available to be configured
        </div> @endhtmlonly

        Struct to specify the attributes of DIS Module available to be configured
*/
typedef struct aui_attr_dis {

    /**
    Member to specify HD/SD display device as defined in the enum #aui_dis_format
    */
    aui_dis_format uc_dev_idx;

    /**
    @attention  This member is @a reserved to ALi R&D Dept. then users can ignore it

    @note   This member is used as default in projects based on <b> TDS OS </b>
    */
    aui_dis_open_param_callback get_param;

    /**
    Member as a @a flag to set progressive/interlaced DAC output, in particular
    - @b 1 = The DAC output is interlaced
    - @b 0 = The DAC output is progressive
    */
    unsigned int b_dac_progressive;

} aui_attr_dis, *aui_p_attr_dis;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify the source/
        destination picture rectangular size
        </div> @endhtmlonly

        Struct to specify the source/destination picture rectangular size

@note   This struct is used by the functions
        - #aui_dis_zoom
        - #aui_dis_mode_set
        - #aui_dis_zoom_ext

@note   to zoom in/out the video. About the user coordinate system to be used
        when zooming the video, it is <b> 720*2880 </b> where
        <b> x &isin; [0,720] </b> and <b> y &isin; [0;2880] </b> as shown below

                 |(0,0)- - - - - - - - - - - - - (720,0)|
                 |                                      |
                 |                                      |
                 |              TV display              |
                 |                                      |
                 |                                      |
                 |(0,2880)- - - - - - - - - - (720,2880)|

@note   This user coordinate system is the same for any resolutions, such as
        - 1280*720
        - 1920*1080
        - etc.

@note   The conversion between the user coordinate system and TV coordinate
        system is explained below:
        - TV_coordinate_x(unit:pixel) = (x*resolution_x)/720
        - TV_coordinate_y(unit:pixel) = (y*resolution_y)/2880

@note   @b Example \n
        The resolution 1280*720,(x,y) = (360,1440) follows the calculation below:
        - TV_coordinate_x = (360*1280)/720 = 640(pixel)
        - TV_coordinate_y = (1440*720)/2880 = 360(pixel)

@note   @b Caution: The above calculation method is @a only used to calculate
                    @a approximately the TV_coordinate, i.e. with a tolerance
                    less than 1 pixel
*/
typedef struct aui_dis_zoom_rect {

    /**
    Member as @a Index (which values are integer from 0) to refer to the top
    left corner X-axis

    @note   The variable range is [0,720].
    */
    unsigned int ui_startX;

    /**
    A variable (integer from "0") as an index which refers to the top left
    corner Y-axis.

    @note   The variable range is <b> [0,2880] </b>.
    */
    unsigned int ui_startY;

    /**
    A variable (integer from "0") as an index which refers to the width of
    the source picture.

    @note   The value as result of the sum <b> ui_startX + ui_width </b>
            needs to belong to the range <b> [0,720] </b>
    */
    unsigned int ui_width;

    /**
    A variable (integer from "0") as an index which refers to the height of the
    source picture.

    @note   The value as result of the sum <b> ui_startY + ui_height </b>
            needs to belong to the range <b> [0,2880] </b>
    */
    unsigned int ui_height;

} aui_dis_zoom_rect, aui_p_zoom_rect_dis;

/**
Enum to specify the available attributes to enhance the video, i.e.
- Brightness
- Contrast
- Saturation
- Sharpness
- Hue

@note   This enum is used by the function #aui_dis_enhance_set to perform a
        specific setting where
        - The parameter @b ul_item takes the item related to the specific
          setting to perform
        - The parameter @b ui_value takes the attribute value as per the
          description of the specific setting to perform
*/
typedef enum aui_dis_video_enhance {

    /**
    Value used to enhance the <b> brightness </b> of the video.

    @note   The parameter @b ui_value can takes a value which belongs to the
            range @b [0,100], however it has been set to @b 50 as default
    */
    AUI_DIS_ENHANCE_BRIGHTNESS = 0,

    /**
    Value used to enhance the <b> contrast </b> of the video,

    @note   The parameter @b ui_value can takes a value which belongs to the
            range @b [0,100], however it has been set to @b 50 as default
    */
    AUI_DIS_ENHANCE_CONTRAST,

    /**
    Value used to enhance the <b> saturation </b> of the video,

    @note   The parameter @b ui_value can takes a value which belongs to the
            range @b [0,100], however it has been set to @b 50 as default
    */
    AUI_DIS_ENHANCE_SATURATION,

    /**
    Value used to enhance the <b> sharpness </b> of the video.

    @note   The parameter @b ui_value can takes a value which belongs to the
            range @b [0,10], however it has been set to @b 5 as default
    */
    AUI_DIS_ENHANCE_SHARPNESS,

    /**
    Value used to enhance the <b> hue </b> of the video,

    @note   The parameter @b ui_value can takes a value which belongs to the
            range @b [0,100], however it has been set to @b 50 as default
    */
    AUI_DIS_ENHANCE_HUE

} aui_dis_video_enhance;

/**
Enum to specify different <b> display aspect ratio </b> available to be set

@note   This enum is used by the function #aui_dis_aspect_ratio_set_ext to
        perform a specific setting where the parameter @b en_asp_ratio takes
        the item related to the specific setting to perform
*/
typedef enum aui_dis_aspect_ratio {

    /**
    Value to specify the display aspect ratio as <b> 16:9 </b>
    */
    AUI_DIS_AP_16_9 = 0,

    /**
    Value to specify the display aspect ratio <b> 4:3 </b>
    */
    AUI_DIS_AP_4_3,

    /**
    Value to specify that the display aspect ratio will be set by drivers
    automatically
    */
    AUI_DIS_AP_AUTO

} aui_dis_aspect_ratio;

/**
Enum used to set the <b> display match mode </b>

@note   This enum is used by the function #aui_dis_aspect_ratio_set_ext to
        perform a specific setting where the parameter @b match_mode takes the
        item related to the specific setting to perform
*/
typedef enum aui_dis_match_mode {

    /**
    In this display match mode
    when
            - The aspect ratio of source video frame is <b> 16:9 </b>
            - The aspect ratio of TV set is <b> 4:3 </b>
    then
            - The vertical part of the picture will be not changed
            - The left and right side of the picture will be cut
    others no effect
    */
    AUI_DIS_MM_PANSCAN = 0,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_PANSCAN_NOLINEAR,

    /**
    In this display match mode
    when
        - The aspect ratio of source video frame is <b> 16:9 </b>
        - The aspect ratio of TV set is <b> 4:3 </b>
    then
        - The horizontal part of the picture will be not changed
        - The top and bottom of the picture will be filled in black color
    others no effect
    */
    AUI_DIS_MM_LETTERBOX,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_TWOSPEED,

    /**
    In this display match mode
    when
        - The aspect ratio of source video frame is <b> 4:3 </b>
        - The aspect ratio of TV set is <b> 16:9 </b>
    then
        - The vertical part of the picture will be not changed
        - The left and right side of the picture will be filled in black color
    others no effect
    */
    AUI_DIS_MM_PILLBOX,

    /**
    In this display match mode
    when
        - The aspect ratio of source video frame is <b> 4:3 </b>
        - The aspect ratio of TV set is <b> 16:9 </b>
    then
        - The horizontal part of the picture will be not changed
        - The top and bottom of the picture will be cut
    others no effect
    */
    AUI_DIS_MM_VERTICALCUT,

    /*
    Value to specify that DIS Module will not manage the aspect ratio of the
    video, i.e. any aspect ratio change will be not performed for the destination
    picture on the TV then the aspect ratio of the source picture will be kept
    */
    AUI_DIS_MM_NORMAL_SCALE,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_LETTERBOX149,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_AFDZOOM,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_PANSCAN43ON169,

    /**
    In this display match mode
    when
        - The aspect ratio of source video frame is <b> 4:3 </b>
        - The aspect ratio of TV set is <b> 16:9 </b>
    then
        - The left and right side of the picture will be filled in black color
        - The top and bottom of the picture will be cut

    when
        - The aspect ratio of source video frame is <b> 16:9 </b>
        - The aspect ratio of TV set is <b> 4:3 </b>
    then
        - The left and right side of the picture will be cut
        - The top and bottom of the picture will be filled in black color
    others no effect
    */
    AUI_DIS_MM_COMBINED_SCALE,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_IGNORE,

    /**
    @attention  This value is @a reserved to ALi R&D Dept. then user can ignore it
    */
    AUI_DIS_MM_VERTICALCUT_149

} aui_dis_match_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify a list of
        parameters related to captured pictures
        </div> @endhtmlonly

        Struct to specify a list of parameters related to captured pictures
*/
typedef struct aui_st_capture_pic {

    /**
    Member as a pointer to the @b buffer intended to save the captured pictures
    */
    unsigned char *puc_out_data_buf;

    /**
    Member to specify the @b size (in @a byte unit) of the @b buffer intended to
    save the captured pictures, as mentioned in the member @b puc_out_data_buf
    of this struct
    */
    unsigned int ui_out_data_buf_size;

    /**
    Member to specify the @b size (in @a byte unit) of <b> valid data </b> in
    the @b buffer intended to save the captured pictures, as mentioned in the
    member @b puc_out_data_buf of this struct

    @attention  This member is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    unsigned int ui_out_size_valid;

    /**
    Member to specify the @b width of the captured pictures

    @attention  This member is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    unsigned int ui_pic_width;

    /**
    Member to specify the @b height of the captured pictures

    @attention  This member is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    unsigned int ui_pic_height;

} aui_capture_pic;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify the display
        order of layers
        </div> @endhtmlonly

        Struct to specify the display order of layers

@attention  This struct is @a reserved to ALi R&D Dept. the users can ignore it
*/
typedef struct aui_layer_order {

    /**
    Member to specify which layers are available to be displayed, as defined in
    the enum #aui_dis_layer
    */
    aui_dis_layer en_ly;

    /**
    Member to specify the layers to be displayed, as defined in the enum
    #aui_dis_layer
    */
    unsigned int ui_num;

} aui_layer_order;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify the
        current display informations
        </div> @endhtmlonly

        Struct to specify the current display informations
*/
typedef struct aui_dis_info {

    /**
    Member as a @a flag to specify whether the current DAC output is progressive
    or interlaced, in particular
    - @b 1 = The DAC output is progressive
    - @b 0 = The DAC output is interlaced
    */
    unsigned char   bprogressive;

    /**
    Member to specify the current display video format, as defined in the enum
    #aui_dis_tvsys
    */
    aui_dis_tvsys   tvsys;

    /**
    Member to specify the width (in pixels) of the source video picture
    */
    unsigned short  source_width;

    /**
    Member to specify the height (in pixels) of the source video picture
    */
    unsigned short  source_height;

    /**
    Member to specify the width (in pixels) of the display region in TV set

    @note The value of this member depends on the resolution of current video
          display formats. For example, it will be 1920 pixels if the display
          format is 1080P
    */
    unsigned short    des_width;

    /**
    Member to specify the height (in pixels) of the display region in TV set

    @note The value of this member depends on the resolution of current video
          display formats. For example, it will be 1080 pixels if the display
          format is 1080P
    */
    unsigned short  des_height;

    /**
    Member to specify whether the OSD layer is on or off, in particular
    - @b 1 = The OSD layer is on
    - @b 0 = the OSD layer is off
    */
    unsigned char    gma1_onoff;

    /**
    Member to specify the source video size, as defined in the enum
    #aui_dis_zoom_rect
    **/
    aui_dis_zoom_rect src_rect;

    /**
    Member to specify the destination video size, as defined in the
    #aui_dis_zoom_rect.
    */
    aui_dis_zoom_rect dst_rect;

    /**
    Member to specify the current display match mode,
    as defined in #aui_dis_match_mode
    */
    aui_dis_match_mode dis_match_mode;

    /**
    Member to specify the current display aspect ratio,
    as defined in #aui_dis_aspect_ratio
    */
    aui_dis_aspect_ratio dis_aspect_ratio;

} aui_dis_info;

/**
Enum to specify a list of parameters related to 3D display mode

@note   This enum is available @a only for projects based on <b> Linux OS </b>
*/
typedef enum aui_dis_3D_mode {

    /**
    Value to specify that the 3D display mode is @b disabled

    @note   The 2D display mode is set by default
    */
    AUI_DIS_3D_MODE_NONE,

    /**
    Value to specify the <b> side by side </b>
    */
    AUI_DIS_3D_MODE_SIDE_BY_SIDE,

    /**
    Value to specify the <b> Top and Bottom </b>

    */
    AUI_DIS_3D_MODE_TOP_AND_BOTTOM,

    /**
    Value to specify the conversion <b> from 2D to 3D </b> display mode
    */
    AUI_DIS_3D_MODE_2D_TO_3D,

    /**
    Value to specify the conversion <b> from 2D to 3D red blue feature </b>
    display mode
    */
    AUI_DIS_3D_MODE_2D_TO_RED_BLUE,

    /**
    Value to specify the total number of items available in this enum
    */
    AUI_DIS_3D_MODE_MAX

} aui_dis_3D_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify the Active
        Format Description (AFD) display mode
        </div> @endhtmlonly

        Struct to specify the Active Format Description (AFD) display mode
*/
typedef struct aui_st_dis_afd_attr {

    /**
    Member as a @a flag to specify the working mode of AFD, in particular
    - @b 0 = AFD is implemented by the co-work of DIS and TV set
             - @b Caution: In this case, TV set needs to support AFD
             - @b Caution: In this case, only support DTG specification
    - @b 1 = AFD is implemented by DIS scaling or clipping
             - @b Caution: That is required to support MinDig function
    - @b 2 = AFD is disabled, the AFD information in the video bitstream will
             be ignored. The final match mode will be controlled by the enums
             #aui_dis_match_mode and #aui_dis_aspect_ratio.

    @note    If AFD is enabled, it will effect the
             display match mode as the match mode displaying depends on the AFD.
             When AFD mode is enabled, SAR scale mode will be disabled automatically
    */
    unsigned char afd_mode;

    /**
    Member as @a flag to indicate the supported AFD specification, in particular
    - @b 0 = DTG specification
    - @b 1 = MinDig specification
    */
    unsigned char afd_spec;

    /**
    Member as a @a flag to specify whether AFD protect mode is enabled or
    disabled, in particular
    - @b 0 = AFD protect mode disabled
    - @b 1 = AFD protect mode enabled
    */
    unsigned char afd_protect_enable;

} aui_dis_afd_attr;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Display Engine (DIS) Module </b> to specify video
        attributes for the display layer
        </div> @endhtmlonly

        Struct to specify the source of video and the dispaly position for a
        display layer.
*/
typedef struct aui_dis_video_attr {

    /**
    Member to specify the handle of video decoder to be used as the source of
    current display layer.
    */
    aui_hdl             video_source;

    /**
    Member to specify the rectangle of source video to be shown on the current
    display layer. The rectangle of video frame is always relative to a virtual
    rectangle as (0,0,720,2880).\n
    In most of the case, this member should be the pointer to a rectangle as
    (0,0,720,2880), which means full frame size then no cutting will be performed.

    @note If this member is set to NULL, full frame size will be used.
    */
    aui_dis_zoom_rect  *p_src_rect;

    /**
    Member to specify the destination display rectangle for the video decoder
    on current display layer. The rectangle is always relative to a virtual
    rectangle as (0,0,720,2880).\n
    If the rectangle size defined with this member is different from the size of
    the rectangle defined in the member @b p_src_rect, the scaling will be performed.

    @note: This member must not be set to NULL.
    */
    aui_dis_zoom_rect   *p_dst_rect;

} aui_dis_video_attr;

/**
Function pointer to specify the callback function used during the initialization
of DIS Module
*/
typedef void (*aui_func_dis_init) (

    void

    );

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to perform the initialization of the DIS Module
                before its opening by the function #aui_dis_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the DIS Module

@param[in]      fn_dis_init             = Callback function used to initialize
                                          the DIS Module,as per comment for the
                                          function pointer #aui_func_dis_init

@return         @b AUI_RTN_SUCCESS      = Initializing of the DIS Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in])
                                          is invalid
@return         @b DIS_ERR              = Getting of the mutex failed for
                                          some reasons
@return         @b Other_Values         = Initializing of the DIS Module failed
                                          for some reasons

@note           About the callback function @b fn_dis_init as input parameter:
                - In projects based on <b> Linux OS </b>, it is suggested to
                  set as NULL
                - In projects based on <b> TDS OS </b>, it needs to perform some
                  display device attachments and configurations. Please refer
                  to the sample code of DIS Module initialization for more
                  clarifications
*/
AUI_RTN_CODE aui_dis_init (

    aui_func_dis_init fn_dis_init

    );

/**
@brief          Function used to open the DIS Module and configure the desired
                attributes, then get the related handle

@warning        This function can @a only be used in the <b> Pre-Run Stage </b>
                of the DIS Module, in particular:
                 - Either after performing the initialization of the DIS Module
                   by the function #aui_dis_init for the first opening of the
                   DIS Module
                 - Or after closing the DIS Module by the function #aui_dis_close,
                   considering the initialization of the DIS Module has been
                   performed previously by the function #aui_dis_init

@param[in]      *p_attr_dis             = Pointer to a struct #aui_attr_dis,
                                          which collects the desired attributes
                                          for the DIS Module

@param[out]     *ppv_hdldis             = Pointer to the handle of the DIS Module
                                          just opened

@return         @b AUI_RTN_SUCCESS      = Opening of the DIS Module performed
                                          successfully then user can start to
                                          configure the DE and TV encoder
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Opening of the DIS Module failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dis_open (

    aui_attr_dis *p_attr_dis,

    void **ppv_hdldis

    );

/**
@brief          Function used to register the DAC output with signal component
                in either YUV, RGB, SVIDEO or CVBS output type

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened and to be managed to
                                          register DAC
@param[in]      *pui_dac_attr           = DAC output type to be registered, such
                                          as YUV, RGB, SVIDEO, CVBS as defined
                                          in the enum #aui_dis_output_type
@param[in]      ui_ary_num              = The amount of DAC needed by the output
                                          type to be set

@return         @b AUI_RTN_SUCCESS      = Registering of the DAC output type
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Registering of the DAC output type
                                          failed for some reasons

@note           Registered DACs need to be configured and connected to HD/SD TV
                encoder by using the related handle. Currently
                - ALi chipsets have 4 DACs
                - HD handle can be used for YUV output type
                - SD handle can be used for CVBS, SVIDEO, RGB output type

@note           Obviously each output type needs a different number of DACs, in
                particular
                - YUV needs 3 DACs
                - CVBS needs 1 DAC
                - SVIDEO needs 2 DACs
                - RGB needs 3 DACs

@note           Below an example of settings:
                - Set the first DAC to be YUV_Y,
                  i.e. @b pui_dac_attr = #AUI_DIS_TYPE_Y
                - Set the second DAC to be YUV_U,
                  i.e. @b pui_dac_attr = #AUI_DIS_TYPE_U
                - Set the third DAC to be YUV_V,
                  i.e. @b pui_dac_attr = #AUI_DIS_TYPE_V
                - Set the fourth DAC to be CVBS,
                  i.e. @b pui_dac_attr = #AUI_DIS_TYPE_CVBS
                - Set @b ui_ary_num = 4

@note           To change the output type of a DAC already registered, firstly
                de-register the DAC by the function #aui_dis_dac_unreg then use
                this function again to register the DAC with the new output type
*/
AUI_RTN_CODE aui_dis_dac_reg (

    aui_hdl handle,

    unsigned int *pui_dac_attr,

    unsigned int ui_ary_num

    );

/**
@brief          Function used to get DAC output register information with signal 
                component in either YUV, RGB, SVIDEO or CVBS output type

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened
                                          
@param[in]      *pui_dac_attr           = Pointer to of an array which stores 
                                          DAC output register information 
										  as YUV, RGB, SVIDEO, CVBS as defined
                                          in the enum #aui_dis_output_type
@param[in]      ui_ary_num              = The amount of DAC needed by the output
                                          type to be get, and must take one of 
										  the values of 1,2,3,4

@return         @b AUI_RTN_SUCCESS      = Geting of the DAC output register information 
                                                          performed successfully                                               
                                          
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Geting of the DAC output type
                                          failed for some reasons

@note           Get DACs registered indformation need to be connected to HD/SD TV
                encoder by using the related handle. Currently
                - ALi chipsets have 4 DACs
                - HD handle can be used for YUV output type
                - SD handle can be used for CVBS, SVIDEO, RGB output type
*/
AUI_RTN_CODE aui_dis_dac_reg_get (

    aui_hdl handle,

    unsigned int *pui_dac_attr,

    unsigned int ui_ary_num

);

/**
@brief          Function used to set the attributes to enhance the video

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened
@param[in]      ul_item                 = The item related to the specific
                                          attribute to be set for enhancing the
                                          video, as defined in the enum
                                          #aui_dis_video_enhance
@param[in]      ui_value                = The value of the specific attribute to
                                          enhance the video, as defined in the
                                          enum #aui_dis_video_enhance

@return         @b AUI_RTN_SUCCESS      = Setting of the specific attribute to
                                          enhance the video performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameters (i.e.[in]) is
                                          invalid
@return         @b Other_Values         = Setting of the specific attribute to
                                          enhance the video failed for some
                                          reasons
*/
AUI_RTN_CODE aui_dis_enhance_set (

    aui_hdl handle,

    aui_dis_video_enhance ul_item,

    unsigned int ui_value

    );

/**
@brief          Function used to set the parameters for zooming in/out the main
                picture layer shown on the TV set screen.

@warning        This function is used for scaling <b> SD Video </b> @a only.\n
                For <b> HD Video </b> instead, please use the function
                #aui_dis_mode_set

@note           This function and the function #aui_dis_mode_set are different
                when the video is in preview mode, in fact this function is used
                to make the video display window slightly smaller than the full
                screen view. If the output rectangle is near to or smaller than
                1/2 of the full screen view, please use the function
                #aui_dis_mode_set instead.

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened
@param[in]      *p_src_rect             = Source picture size, as defined in the
                                          struct #aui_dis_zoom_rect
@param[in]      *p_dst_rect             = Destination picture size, as defined
                                          in the struct #aui_dis_zoom_rect

@return         @b AUI_RTN_SUCCESS      = Setting of the zoom parameters
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the zoom parameters failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dis_zoom (

    aui_hdl handle,

    aui_dis_zoom_rect *p_src_rect,

    aui_dis_zoom_rect *p_dst_rect

    );

/**
@brief          Function used to enable/disable the main picture layer output
                of the TV Encoder

@param[in]      handle                  = Pointer to the handle of the DIS
                                          Module already opened
@param[in]      uc_b_enable             = Flag to enable/disable the main picture
                                          layer output of the TV Encoder, in
                                          particular
                                          - @b 1 = Enabled
                                          - @b 0 = Disabled

@return         @b AUI_RTN_SUCCESS      = Enabling/Disabling of the main picture
                                          layer performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Enabling/Disabling of the main picture
                                          layer failed for some reasons

@note           DE can open video layer automatically, please refer to the enum
                #aui_dis_item_set in particular to the value
                @b AUI_DIS_SET_AUTO_WINONOFF for more details
*/
AUI_RTN_CODE aui_dis_video_enable (

    aui_hdl handle,

    unsigned char uc_b_enable

    );

/**
@brief          Function used to set the display match mode and display output
                aspect ratio

@note           This function is the extension of the function
                #aui_dis_match_mode_set and #aui_dis_aspect_ratio_set

@param[in]      handle                  = Pointer to the handle of the DIS
                                          Module already opened
@param[in]      en_asp_ratio            = The display output aspect ratio to be
                                          set, as defined in the enum
                                          #aui_dis_aspect_ratio
@param[in]      en_match_mode           = The display match mode to be set, as
                                          defined in the enum #aui_dis_match_mode

@return         @b AUI_RTN_SUCCESS      = Setting of the display match mode and
                                          display output aspect ratio performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the display match mode and
                                          display output aspect ratio failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dis_aspect_ratio_set_ext (

    aui_hdl handle,

    aui_dis_aspect_ratio en_asp_ratio,

    aui_dis_match_mode en_match_mode

    );

/**
@brief          Function used to set @a only the display output aspect ratio

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened
@param[in]      en_asp_ratio            = The display output aspect ratio to be
                                          set, as defined in the enum
                                          #aui_dis_aspect_ratio

@return         @b AUI_RTN_SUCCESS      = Setting of the display output aspect
                                          ratio performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the display output aspect
                                          ratio failed for some reasons

@note           When using HD applications, the corresponding HD handle @a must
                be used
*/
AUI_RTN_CODE aui_dis_aspect_ratio_set (

    aui_hdl handle,

    aui_dis_aspect_ratio en_asp_ratio

    );

/**
@brief          Function used to set @a only the display match mode

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened
@param[in]      en_match_mode           = The display match mode to be set, as
                                          defined in the enum #aui_dis_match_mode

@return         @b AUI_RTN_SUCCESS      = Setting of the display match mode
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the display match mode
                                          failed for some reasons

@note           If AFD is enabled (AFD is enabled by default), AFD will effect
                the display match mode. If the application doen't allow the AFD
                information in video bitstream to affect the final match mode, the
                application should disable AFD by the function #aui_dis_afd_set.

@note           Match modes work only when the video is in full view mode. In
                preview mode, the video is always stretched to the preview window.

*/
AUI_RTN_CODE aui_dis_match_mode_set (

    aui_hdl handle,

    aui_dis_match_mode en_match_mode

    );

/**
@brief          Function used to set the display video format

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened
@param[in]      en_tv_sys               = The display video format to be set,
                                          as defined in the enum #aui_dis_tvsys

@param[in]      uc_b_progressive        = Flag to specify whether the video
                                          output is progressive or interlaced,
                                          in particular
                                          - @b 1 = Video output is progressive
                                          - @b 0 = Video output is is interlaced

@return         @b AUI_RTN_SUCCESS      = Setting of the display video format
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the display video format
                                          failed for some reasons

@warning        Really important, the function #aui_dis_dac_unreg @b must be
                called before setting a new display video format by this function:
                the CVBS signal will be abnormal if the DAC is not disabled by
                #aui_dis_dac_unreg first.

*/
AUI_RTN_CODE aui_dis_tv_system_set (

    aui_hdl handle,

    aui_dis_tvsys en_tv_sys,

    unsigned char uc_b_progressive

    );

/**
@brief          Function used to close the DIS Module already opened by the
                function #aui_dis_open, then the related handle will be released
                (i.e. the related resources such as memory, device)

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of the DIS Module in pair with its the opening by the function
                #aui_dis_open.\n
                Furthermore after closing the DIS Module, user can open it again
                by the function #aui_dis_open considering its initialization
                has been performed previously by the function #aui_dis_init

@param[in]      *p_attr_dis             = Pointer to a struct #aui_attr_dis,
                                          which collects the attributes for the
                                          DIS Module to be closed
                                          - @b Caution: It is suggested to set
                                               as NULL cause the struct
                                               #aui_attr_dis does not store
                                               any memory pointer in heap
@param[in]      *ppv_handle_dis         = Pointer to the handle of the DIS
                                          Module already opened and to be closed.

@return         @b AUI_RTN_SUCCESS      = Closing of the DIS Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Closing of the DIS Module failed for
                                          same reasons

@note           Only in projects based on <b> TDS OS </b>, it is suggested to
                @a only use this function before to go into standby mode
*/
AUI_RTN_CODE aui_dis_close (

    aui_attr_dis *p_attr_dis,

    void **ppv_handle_dis

    );

/**
brief           Function used to perform a specific setting of the DIS Module
                after its opening by the functions #aui_dis_open

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened and to be managed to
                                          perform a specific setting
@param[in]      ul_item                 = The items related to the specific
                                          setting of the DIS Module to be
                                          performed, as defined in the enum
                                          #aui_dis_item_set
@param[in]      *pv_param               = The pointer as per the description of
                                          the specific setting of the DIS Module
                                          to be performed, as defined in the
                                          enum #aui_dis_item_set

@return         @b AUI_RTN_SUCCESS      = Specific setting of the DIS Module
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Specific setting of the DIS Module
                                          failed for some reasons

@note           This function can be used to add some extra new features, such as
                - Auto display of the valid signal in input
                - Attaching of the HDTV encode to DEO
                - etc.
*/
AUI_RTN_CODE aui_dis_set (

    aui_hdl handle,

    aui_dis_item_set ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific information from the DIS Module
                after its opening by the functions #aui_dis_open

@param[in]      handle                  = Pointer to the handle of the DIS Module
                                          already opened and to be managed to
                                          get a specific information
@param[in]      ul_item                 = The items related to the specific
                                          information of the DIS Module to be
                                          gotten, as defined in the enum
                                          #aui_dis_item_get

@param[out]     *pv_param               = The pointer as per the description of
                                          the specific information of the DIS
                                          Module to be gotten, as defined in
                                          the enum #aui_dis_item_get

@return         @b AUI_RTN_SUCCESS      = Getting of the specific information
                                          of the DIS Module performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the specific information of
                                          the DIS Module failed for some reasons
*/
AUI_RTN_CODE aui_dis_get (

    aui_hdl handle,

    aui_dis_item_get ul_item,

    void *pv_param

    );

/**
@brief          Function used to capture the picture in the main picture layer

@param[in]      handle                  = Pointer to the handle of the DIS
                                          Module already opened

@param[out]     *p_cap_param            = Pointer to the buffer intended to
                                          save the capture picture, as defined
                                          in the struct #aui_st_capture_pic

@return         @b AUI_RTN_SUCCESS      = Getting of the specific information
                                          of the DIS Module performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameters (i.e. [in],
                                          [out]) is invalid
@return         @b Other_Values         = Getting of the specific information
                                          of the DIS Module failed for some
                                          reasons

@note           @b 1. Currently, capturing of the MPEG2 video format is
                      supported, and the captured pictures are in YUV format

@note           @b 2. To use the captured pictures in OSD layer, needs to change
                      YUV to RGB
*/
AUI_RTN_CODE aui_dis_capture_pic (

    aui_hdl handle,

    aui_capture_pic *p_cap_param

    );

/**
@brief          Function used to set the display device view mode, i.e. either
                full mode or preview mode

                This function and the functions #aui_dis_zoom_ext and
                #aui_dis_zoom are different when the video is in preview mode.\n
                Indeed, the video decoder will resize the video frames for the
                display engine with scale rate 1/2, 1/4, 1/8, then the display
                engine will zoom in/out the video frame to the rectangle specified
                for HD

@param[in]      handle                  = Pointer to the handle of the DIS
                                          Module already opened
@param[in]      mode                    = The display device view mode to be set,
                                          as defined in the enum #aui_dis_mode
@param[in]      *p_src_rect             = Source picture size, as defined in the
                                          struct #aui_dis_zoom_rect
                                          - @b Caution: This parameter is @a only
                                               used in the preview mode
@param[in]      *p_dst_rect             = Destination picture size, as defined
                                          in the struct #aui_dis_zoom_rect
                                          - @b Caution: This parameter is @a only
                                               used in the preview mode

@return         @b AUI_RTN_SUCCESS      = Setting of the display device view
                                          mode performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameters (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the display device view
                                          mode failed for some reasons
@note           @b 1. For SD only project, please use #aui_dis_zoom or #aui_dis_zoom_ext
                      instead
@note           @b 2. The implement cannot finish in time if there is no enough video data
                      to switch.
                      There is no enough video data in following conditions:
                      (a) the video decoder is in stopped status 
                      (b) the video decoder is in paused status 
                      (c) there is no video data coming when video decorder is in decoding 
                          status
                      although the display mode and rect will change to the desire value when there
                      is enough data for video decorder to decoder new frame.

                      If you want to make sure the swiching is done, there two ways:
                      (a) you can try to use
                          #rect_switch_done in #aui_decv_status_info to check whether the 
                          switching done or not 
                      (b) when it switchs done, the callback function will be called
                          which is resigsterd with decv callback type 
                          #AUI_DECV_CB_MODE_SWITCH_OK, 
                          only video formats mpeg2,avc,hevc support this callback
                          right now, please refer to #AUI_DECV_CB_MODE_SWITCH_OK
                          get more information

                      The way to check switching done or not is available @a only for 
                      projects based on <b> Linux OS </b> right now
*/
AUI_RTN_CODE aui_dis_mode_set (

    aui_hdl handle,

    aui_view_mode mode,

    aui_dis_zoom_rect *p_src_rect,

    aui_dis_zoom_rect *p_dst_rect

    );

/**
@brief          Function used to de-register DACs output with signal component
                in either YUV, RGB, SVIDEO or CVBS output type, already
                registered by the function #aui_dis_dac_reg

@param[in]      handle                  = Pointer to the handle of the DIS
                                          Module already opened
@param[in]      type                    = DAC output type to be de-registered,
                                          such as YUV, RGB, SVIDEO or CVBS as
                                          defined in the enum #aui_dis_unreg_type

@return         @b AUI_RTN_SUCCESS      = De-Registering of the DACs output
                                          type performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameters (i.e. [in])
                                          is invalid
@return         @b Other_Values         = De-Registering of the DACs output
                                          type failed for same reasons

@warning        Really important, this function @b must be called before setting
                a new display video format by the function #aui_dis_tv_system_set:
                the CVBS signal will be abnormal if the DAC is not disabled
                by this function first.
*/
AUI_RTN_CODE aui_dis_dac_unreg (

    aui_hdl handle,

    aui_dis_unreg_type type

    );

/**
@brief          Function used to set the parameters for zooming in/out any
                layers shown on the TV set screen.

@warning        This Function is used for scaling <b> SD Video </b> @a only.\n
                For <b> HD Video </b> instead, please use the function
                #aui_dis_mode_set

@note           This function and the function #aui_dis_mode_set are different
                when the video is in preview mode, in fact this function is used
                to make the video display window slightly smaller than the full
                screen view. If the output rectangle is near to or smaller than
                1/2 of the full screen view, please use the function
                #aui_dis_mode_set instead.

@param[in]      handle                  = Pointer to the handle of the DIS
                                          Module already opened
@param[in]      *p_src_rect             = Source picture size, as defined in
                                          the struct #aui_dis_zoom_rect
@param[in]      *p_dst_rect             = Destination picture size, as defined
                                          in the struct #aui_dis_zoom_rect
@param[in]      layer                   = Layer to be displayed, as defined in
                                          the enum #aui_dis_layer

@return         @b AUI_RTN_SUCCESS      = Setting of the zoom parameters
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the zoom parameters
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dis_zoom_ext (

    aui_hdl handle,

    aui_dis_zoom_rect *p_src_rect,

    aui_dis_zoom_rect *p_dst_rect,

    int layer

    );

/**
@brief          Function used to start displaying pictures on the CVBS output

@warning        This function should be used @a only after CVBS is disabled by
                the function #aui_dis_cvbs_stop

@param[in]      pv_hdl_dis              = Pointer to the handle of the DIS
                                          Module already opened

@return         @b AUI_RTN_SUCCESS      = Displaying of the CVBS output type
                                          started successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e.[in]) is
                                          invalid
@return         @b Other_Values         = Displaying of the CVBS output type
                                          not started for some reasons

@note           This function needs to be used with the function
                #aui_dis_cvbs_stop, and it is @a only used in dual output mode
*/
AUI_RTN_CODE aui_dis_cvbs_start (

    void *pv_hdl_dis

    );

/**
@brief          Function used to stop displaying pictures on the CVBS output,
                then CVBS will have black signals.

@note           This function is different from the function #aui_dis_dac_unreg:
                if the application unregister the CVBS DAC, there will not be TV
                signal on CVBS output.

@param[in]      pv_hdl_dis              = Pointer to the handle of the DIS
                                          Module already opened

@return         @b AUI_RTN_SUCCESS      = Displaying of the CVBS output type
                                          stopped successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e.[in]) is
                                          invalid
@return         @b Other_Values         = Displaying of the CVBS output type
                                          not stopped for some reasons

@note           This function needs to be used with the function
                #aui_dis_cvbs_start, and it is @a only used in dual output mode
*/
AUI_RTN_CODE aui_dis_cvbs_stop (

    void *pv_hdl_dis

    );

/**
@brief          Function used to inject a black picture to be shown on the
                main display layer as per the enum value #AUI_DIS_LAYER_VIDEO
                layer for both CVBS and HDMI.\n
                The OSD layers are not filled with a black screen by this
                function.

@param[in]      pv_hdl_dis              = Pointer to the handle of the DIS
                                          Module already opened

@return         @b AUI_RTN_SUCCESS      = Injecting of the black picture
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = The input parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Injecting of the black picture failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dis_fill_black_screen (

    void *pv_hdl_dis

    );

/**
@brief          Function used to set the picture layer display order

@param[in]      pv_hdl_dis              = #aui_hld handle of the DIS Module
                                          already opened
@param[in]      order                   = Picture layer display order to be set,
                                          as defined in the enum
                                          #aui_dis_layer_blend_order

@return         @b AUI_RTN_SUCCESS      = Setting of the picture layer display
                                          order performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the picture layer display
                                          order failed for some reasons
*/
AUI_RTN_CODE aui_dis_layer_order (

    aui_hdl pv_hdl_dis,

    aui_dis_layer_blend_order order

    );

/**
@brief          Function used to set the video background color in YCbCr
                color space

@param[in]      pv_hdl_dis              = #aui_hdl handle of the DIS Module
                                          already opened
@param[in]      *p_color                = YCbCr color space components to be
                                          set, as defined in the struct
                                          #aui_dis_color

@return         @b AUI_RTN_SUCCESS      = Setting of the video background color
                                          in YCbCr color space performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the video background color
                                          in YCbCr color space failed for some
                                          reasons
*/
AUI_RTN_CODE aui_dis_set_background_color (

    aui_hdl pv_hdl_dis,

    aui_dis_color *p_color

    );

/**
@brief          Function used to set the 3D video display mode


@param[in]      pv_hdl_dis              = #aui_hdl handle of the DIS Module
                                          already opened
@param[in]      dis_3d_mode             = 3D video display mode parameters to
                                          be set, as defined in the enum
                                          #aui_dis_3D_mode

@return         @b AUI_RTN_SUCCESS      = Setting of the 3D video display mode
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the 3D video display mode
                                          failed for some reasons
*/
AUI_RTN_CODE aui_dis_3d_set (

    aui_hdl pv_hdl_dis,

    aui_dis_3D_mode dis_3d_mode

    );

/**
@brief          Function used to set the Active Format Description (AFD)
                display mode.
                This function needs to be called before video playing.
                When AFD mode is enable, the SAR scale mode will be disabled automatically.
                When SAR scale mode is enable, the afd information will be ignored automatically.
                please refer to function aui_dis_sar_scale_mode_enable to get more
                information about SAR scale mode
                The last setting of SAR scale mode and afd will affect actually

@param[in]      pv_hdl_dis              = #aui_hdl handle of the DIS Module
                                          already opened
@param[in]      *p_dis_afd_para         = AFD mode parameters to be set, as
                                          defined in the enum #aui_dis_afd_attr

@return         @b AUI_RTN_SUCCESS      = Setting of the AFD mode performed
                                          successfully
@return         @b AUI_RTN_EINVAL       = At least one parameter (i.e. [in]) is
                                          invalid
@return         @b Other_Values         = Setting of the AFD mode failed for
                                          some reasons
*/
AUI_RTN_CODE aui_dis_afd_set (

    aui_hdl pv_hdl_dis,

    aui_dis_afd_attr *p_dis_afd_para

    );

/**
@brief          Function used to enable/disable the main/auxiliary picture
                layer output of the TV Encoder

@param[in]      pv_hdl_dis              = Pointer to the handle of the DIS Module
                                          already opened
@param[in]      layer                   = Specify the display layer
@param[in]      uc_enable               = Flag to enable/disable the main picture
                                          layer output of the TV Encoder, in
                                          particular:
                                          - @b 1 = Enabled
                                          - @b 0 = Disabled

@return         @b AUI_RTN_SUCCESS      = Enabling/Disabling of the main picture
                                          layer performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Enabling/Disabling of the main picture
                                          layer failed for some reasons

@note           DE can open video layer automatically, please refer to the enum
                #aui_dis_item_set in particular to the value
                @b AUI_DIS_SET_AUTO_WINONOFF for more details
*/
AUI_RTN_CODE aui_dis_layer_enable (

    aui_hdl pv_hdl_dis,

    aui_dis_layer layer,

    unsigned char uc_enable

    );

/**
@brief          Function used to set the display device view mode by layer,
                i.e. either full mode or preview mode

@param[in]      pv_hdl_dis              = Pointer to the handle of the DIS
                                          Module already opened
@param[in]      layer                   = specify the display layer
@param[in]      *p_src_rect             = Source picture size, as defined in the
                                          struct #aui_dis_zoom_rect
                                          - @b Caution: The default value is
                                               {0,0,720,2880}
@param[in]      *p_dst_rect             = Destination picture size, as defined
                                          in the struct #aui_dis_zoom_rect

@return         @b AUI_RTN_SUCCESS      = Setting of the display device view
                                          mode performed successfully
@return         @b AUI_RTN_EINVAL       = At least one parameters (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the display device view
                                          mode failed for some reasons
*/
AUI_RTN_CODE aui_dis_video_display_rect_set (

    aui_hdl pv_hdl_dis,

    aui_dis_layer layer,

    aui_dis_zoom_rect *p_src_rect,

    aui_dis_zoom_rect *p_dst_rect

    );

/**
@brief          Function used to set video attributes for two display layer.
                In the case of PiP, two video decoders are outputing video
                frames to two display layer. This function will enable the
                application to change the source of two display layers and
                change the display rectangle at the same time.

@param[in]      pv_hdl_dis              = #aui_hdl handle of the DIS Module
                                          already opened
@param[in]      p_main_video_attr       = Pointer to the struct
                                          #aui_dis_video_attr to specify the
                                          video attributes of the main display
                                          layer as per the enum value
                                          #AUI_DIS_LAYER_VIDEO

@param[in]      p_pip_video_attr        = Pointer to the struct #aui_dis_video_attr
                                          to specify the video attributes
                                          (video source and position) of the PiP
                                          display layer as per the enum value
                                          #AUI_DIS_LAYER_AUXP

@return         @b AUI_RTN_SUCCESS      = Setting of the video attributes
                                          performed successfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the video attributes failed
                                          for some reasons
*/
AUI_RTN_CODE aui_dis_video_attr_set (

    aui_hdl pv_hdl_dis,

    aui_dis_video_attr *p_main_video_attr,

    aui_dis_video_attr *p_pip_video_attr

    );

/*
@brief          Function used to set the maximum limit value (MAX) of the range
                of video enhancements such as:
                - Brightness
                - Contrast
                - Saturation
                - Hue

                In particular, the range is [0, MAX] where MAX > 0
                (the default value is 100) which affects only the function
                #aui_dis_enhance_set

@param[in]      pv_hdl_dis              = #aui_hdl handle of the DIS Module
                                          already opened
@param[in]      max                     = Maximum limit value of the range of
                                          video enhancements to be set, which should
                                          be a value belonging to the range [1, 65536]

@return         @b AUI_RTN_SUCCESS      = Setting of the maximum limit value for video
                                          enhancements performed succesfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the maximum limit value for video
                                          enhancements failed for some reasons
*/
AUI_RTN_CODE aui_dis_enhance_max_value_set (

    aui_hdl         pv_hdl_dis,

    unsigned int    max

    );

/*
@brief          Function used to enable/disable display match mode with SAR scaling or not.
                This function needs to be called before video playing.
                When AFD mode is enable, the SAR scale mode will be disabled automatically.
                When SAR scale mode is enable, the afd information will be ignored automatically.
                The afd and SAR information will both affect the match mode, please
                refer to function aui_dis_afd_set to get more information about afd
                The last setting of SAR scale mode and afd will affect actually

@param[in]      pv_hdl_dis              = #aui_hdl handle of the DIS Module
                                          already opened
@param[in]      uc_enable               = 1: the match mode displaying depends
                                             on picutre width, height and SAR information (default value)

                                          0: the match mode displaying generally or
                                             depends on afd information
                                          general scale:
                                             (1) disable SAR scale mode
                                             (2) disable afd
                                          afd scale: please refer to function aui_dis_afd_set
                                                     to get more information


@return         @b AUI_RTN_SUCCESS      = Setting of the SAR scaling mode performed succesfully
@return         @b AUI_RTN_EINVAL       = At least one input parameter (i.e. [in])
                                          is invalid
@return         @b Other_Values         = Setting of the SAR scaling mode failed for some reasons
*/

AUI_RTN_CODE aui_dis_sar_scale_enable (

    aui_hdl         pv_hdl_dis,

    unsigned char   uc_enable

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

///@cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define aui_en_tv_system_dis      aui_dis_tvsys

#define AUI_DIS_LAYER_BLEND_ORDER aui_dis_layer_blend_order

#define AUI_MP_GMAS_GMAF_AUXP AUI_DIS_LAYER_ORDER_MP_GMAS_GMAF_AUXP

#define AUI_MP_GMAS_AUXP_GMAF AUI_DIS_LAYER_ORDER_MP_GMAS_AUXP_GMAF

#define AUI_MP_GMAF_GMAS_AUXP AUI_DIS_LAYER_ORDER_MP_GMAF_GMAS_AUXP

#define AUI_MP_GMAF_AUXP_GMAS AUI_DIS_LAYER_ORDER_MP_GMAF_AUXP_GMAS

#define AUI_MP_AUXP_GMAS_GMAF AUI_DIS_LAYER_ORDER_MP_AUXP_GMAS_GMAF

#define AUI_MP_AUXP_GMAF_GMAS AUI_DIS_LAYER_ORDER_MP_AUXP_GMAF_GMAS

#define AUI_AUXP_MP_GMAS_GMAF AUI_DIS_LAYER_ORDER_AUXP_MP_GMAS_GMAF

#define AUI_AUXP_MP_GMAF_GMAS AUI_DIS_LAYER_ORDER_AUXP_MP_GMAF_GMAS

#define aui_en_output_type_dis aui_dis_output_type

#define aui_en_mode aui_view_mode

#define aui_en_item_get_dis aui_dis_item_get

#define aui_en_item_set_dis aui_dis_item_set

#define aui_en_unreg_type aui_dis_unreg_type

#define aui_fun_get_open_param aui_dis_open_param_callback

#define aui_zoom_rect_dis aui_dis_zoom_rect

#define aui_en_video_enhance_dis aui_dis_video_enhance

#define aui_en_aspect_ratio_dis aui_dis_aspect_ratio

#define aui_en_match_mode_dis aui_dis_match_mode

#define aui_en_layer aui_dis_layer

#define aui_dis_set_3d aui_dis_3d_set

#define aui_dis_set_afd aui_dis_afd_set

#define AUI_VERTICALCUT_149 AUI_DIS_MM_VERTICALCUT_149

#define AUI_COMAND_GET_NULL AUI_DIS_GET_LAST

#define AUI_DIS_TVSYS_LINE_720_25 AUI_DIS_TVSYS_LINE_720_50

#define AUI_DIS_TVSYS_LINE_720_30 AUI_DIS_TVSYS_LINE_720_60

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


