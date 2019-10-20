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
Current ALi Author: Fawn.Wu
Last update:        2017.02.25
-->

@file       aui_osd.h

@brief      On-Screen Display (OSD) & Graphic Module

            <b> On-Screen Display (OSD) & Graphic Module </b> is used to draw
            and display
            - The menu of the User Interface (UI)
            - Subtitle & Teletext on screen

            This module contains
            - @b GE (Graphic Engine)
            - @b GMA (Graphic Memory Access)

            About @b GE, it is a Hardware GFX accelerator which can perform
            - BitBLT (Bit Block Transfer)
            - Pixel Rasterization
            - Primitive Scan

            to input graphics data and finally outputs the result to the
            destination buffer.\n
            The main features supported by GE are listed below:
            - Alpha Blending
            - Color Space Conversion
            - Color Keying
            - Clipping
            - Solid Color Fill
            - Picture Rectangle Copy
            - Draw Line Mode
            - Scaling
            - Mirror

            About @b GMA it is used to operate the memory buffer intended to
            display and perform global alpha operation. Furthermore it transports
            the data in the graphic memory to the Display Engine (DIS).\n
            GMA contains two (2) layers
            - @b G0 Layer, which is usually used for displaying the Menu of the
              User Interface (UI) on the screen, and support the following input
              data format:
              - CLUT2/4/8
              - ACLUT88
              - RGB888/555/565/444
              - ARGB8888/4444/1555
            - @b G1 Layer, which is usually used for displaying the Subtitle and
              Teletext on the screen, and support the following input data format:
              - CLUT2/4/8

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "On-Screen Display (OSD) &
      Graphics Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_OSD_H

#define _AUI_OSD_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Type List*******************************/

/**
Enum to specify the different type of <b> GMA layers </b> available to be opened
*/
typedef enum aui_osd_layer {

    /**
    Value to specify the layer @b G0 which is mainly used for showing the Menu
    of the User Interface (UI) on the screen
    */
    AUI_OSD_LAYER_GMA0 = 0,

    /**
    Value to specify the layer @b G1 which is mainly used for showing the Subtitle
    and Teletext on the screen
    */
    AUI_OSD_LAYER_GMA1 = 1,

    /**
    Value to specify the total number of layer available to be opened
    */
    AUI_OSD_LAYER_MAX = 2

} aui_osd_layer;

/**
Enum to specify the different type of <b> color space </b> available to be set
for the palette
*/
typedef enum aui_pallette_color_type {

    /**
    Value to specify the color space @b RGB for the palette
    */
    AUI_PALLETTE_COLOR_TYPE_RGB = 0,

    /**
    Value to specify the color space @b YUV for the palette
    */
    AUI_PALLETTE_COLOR_TYPE_YUV

} aui_pallette_color_type;

/**
Enum to specify all supported <b> Mirror Modes </b> to perform <b> blit
operations </b> for displaying pictures on the screen
*/
typedef enum aui_gfx_mirror_type {

    /**
    Value to specify that the pictures data will be displayed
    - From @b top to @b bottom in the @b vertical direction
    - From @b left to @b right in the @b horizontal direction
    */
    AUI_GFX_MIRROR_NORMAL = 0,

    /**
    Value to specify that the pictures data will be displayed
    - From @b top to @b bottom in the @b vertical direction
    - From @b right to @b left in the @b horizontal direction
    */
    AUI_GFX_MIRROR_RIGHT_LEFT,

    /**
    Value to specify that the pictures data will be displayed
    - From @b bottom to @b top in the @b vertical direction
    - From @b left to @b right in the @b horizontal direction
    */
    AUI_GFX_MIRROR_BOTTON_TOP,

    /**
    Value to specify that the pictures data will be displayed
    - From @b bottom to @b top in the @b vertical direction
    - From @b right to @b left in the @b horizontal direction

    @note   This mirror mode is available @a only in projects based on
            <b> TDS OS </b>
    */
    AUI_GFX_MIRROR_BOTTON_TOP_RIGHT_LEFT

} aui_gfx_mirror_type;

/**
Enum to specify all supported <b> Raster Modes </b> to perform <b> blit
operations </b> for displaying pictures on the screen

@note   If user wants to blit or capture memory buffer to display, then one of
        the raster mode listed in this enum needs to be chosen
*/
typedef enum aui_gfx_rop_operation_type {

    /**
    Value to specify that the buffer data in foreground covers the destination
    directly, then the content in foreground is displayed
    */
    AUI_GFX_ROP_DERECT_COVER = 0,

    /**
    Value to specify that the buffer data in foreground and the data in background
    perform the alpha blend operations, then the content displayed is the result of
    them \n\n

    In this mode, an alpha blend mode needs to be set (choosing between twelve
    (12) different kinds totally) by the struct #aui_blit_operation. Please
    refer to the enum #aui_ge_alpha_blend_mode for more detail.
    */
    AUI_GFX_ROP_ALPHA_BLENDING,

    /**
    Value to specify that the buffer data in foreground and the data in background
    do the boolean operations, then the content displayed is the result of them\n\n

    In this mode, a boolean mode needs to be set (choosing between sixteen (16)
    different kinds totally) by the struct #aui_blit_operation. Please refer to
    the enum #aui_ge_bop_mode for more details.

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_GFX_ROP_BOOLEAN,

    /**
    Value to specify that the buffer data in foreground and the data in the
    background do the boolean and alpha blend operations, then the
    content displayed is the result of them.\n\n

    In this mode, a boolean and alpha blend mode need to be set (where the
    boolean mode can be chosen between sixteen (16) different kind totally and
    the alpha blend mode between twelve (12) instead.\n\n

    The boolean and alpha blend mode can be set by the structure # aui_blit_operation,
    please refer to the enums @b aui_ge_bop_mode and #aui_ge_alpha_blend_mode for
    more details.

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_GFX_ROP_BOOL_ALPHA_BLENDIN

} aui_gfx_rop_operation_type;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used to
        locate a point on the screen
        </div> @endhtmlonly

        Struct to specify horizontal/vertical coordinate values of a point on
        the screen, i.e. (x,y) \n\n

        The points location gotten by this struct lets drawing on the screen,
        please refer to the function #aui_gfx_surface_draw_line for its usage.
*/
typedef struct aui_coordinate {

    /**
    Member to specify the horizontal coordinate (i.e. X axis) of the point on
    the screen
    */
    unsigned long X;

    /**
    Member to specify the vertical coordinate (i.e. Y axis) of the point on
    the screen
    */
    unsigned long Y;

} aui_coordinate;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used to
        delimit a rectangular area on the screen
        </div> @endhtmlonly

        Struct used to delimit a rectangular area on the screen by the two (2)
        essential information below:
        - The upper left corner as start vertex
        - The width and height drawn from the start vertex

        That rectangular area is intended to perform some operations within it,
        e.g.
        - Blit or draw line
        - Draw color or draw bitmap
        - Fill color
        - etc.
*/
typedef struct aui_osd_rect {

    /**
    Member to specify the <b> horizontal coordinate (i.e. X axis) </b> of the
    upper left corner as the of the start vertex of the rectangular area to
    be drawn
    */
    unsigned short  uLeft;

    /**
    Member to specify the <b> vertical coordinate (i.e. Y axis) </b> of the
    upper left corner as the of the start vertex of the rectangular area
    to be drawn
    */
    unsigned short  uTop;

    /**
    Member to specify the @b width of the rectangle to be drawn from the
    start vertex
    */
    unsigned short  uWidth;

    /**
    Member to specify the @b height of the rectangle to be drawn from the
    start vertex
    */
    unsigned short  uHeight;

} aui_osd_rect;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> to
        specify the rectangular area on the screeen for blit operations
        </div> @endhtmlonly

        Struct to specify the rectangular area on the screeen for blit
        operations, please refer to the function #aui_gfx_rop_operation_type
        for its usage
*/
typedef struct aui_st_blit_rect {

    /**
    Member to specify the rectangular area of destination for performing blit
    operations, as defined in the struct #aui_osd_rect
    */
    aui_osd_rect dst_rect;

    /**
    Member to specify the rectangular area of buffer data in foreground
    for performing blit operations, as defined in  struct #aui_osd_rect
    */
    aui_osd_rect fg_rect;

    /**
    Member to specify the rectangular area of background data for performing
    blit operations,  as defined in struct #aui_osd_rect

    @note  If background data is not used then that member cannot be set
    */
    aui_osd_rect bg_rect;

} aui_blit_rect;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> to
        specify the range for the color key
        </div> @endhtmlonly

        Struct to specify the range for the color key.

@note   When the color key is enabled, the maximum and minimum value for color
        key need to be set according to the current color format, by using the
        function #aui_gfx_surface_colorkey_set
*/
typedef struct aui_st_color_key {

    /**
    Member to  specify the @b maximum color key value according the color format
    below:
    - [31:24] = Alpha value
    - [23:16] = Red channel value
    - [15:08] = Green channel value
    - [07:00] = Blue channel value
    */
    unsigned long color_key_max;

    /**
    Member to specify the @b minimum color key value according the color format
    below:
    - [31:24] = Alpha channel value
    - [23:16] = Red channel value
    - [15:08] = Green channel value
    - [07:00] = Blue channel value

    @note  @b 1. In general, the minimum color key value can be either the same
                 or different as the maximum value
    @note  @b 2. In projects based on <b> Linux OS </b>, the minimum color key
                 value @a must be the same as the maximun value in Linux platform.
    */
    unsigned long color_key_min;

} aui_color_key;

/**
Enum to specify the output mode type in the alpha channel of destination
surface, which is either a display buffer or a copy of display buffer.\n\n

@note  When the raster mode for blit operations is set as the enum vale
       - Either #AUI_GFX_ROP_ALPHA_BLENDING
       - Or @b AUI_GFX_ROP_BOOL_ALPHA_BLENDIN,

@note  the alpha output mode needs to be set by
       - Either the function #aui_gfx_data_blit
       - Or the function #aui_gfx_surface_blit

@note  During blit operations, the value in
       - Alpha channel
       - Red channel
       - Green channel
       - Blue channel

@note  will be given priority. Each output mode has a different calculation
       method as described in this enum, where the abbreviations used have
       the meaning listed below:
       - @a da  = Alpha value of destination surface that is displayed at last
       - @b fc  = Color value of foreground surface
       - @b bc  = Color value of background surface
       - @b fa  = (pixel alpha of foreground surface) / max_alpha
       - @b ba  = (pixel alpha of background surface) / max_alpha
       - @b Ga  = Coefficient @b Galpha, which can be:
                  - Either (foreground global alpha) / max_alpha
                  - Or (background global alpha) / max_alpha
                  - Or 1
@note  according the alpha output mode
       - @b fc' = Color value associated with @b Ga and @b fc
                  (Each output mode has a different calculation method as
                  described in this enum)
       - @b bc' = Color value associated with @b Ga and @b bc
                  (Each output mode has a different calculation method as
                  described in this enum)
       - @b fa' = Alpha value associated with @b Ga and @b fa
                  (Each output mode has a different calculation method as
                  described in this enum)
       - @b ba' = Alpha value associated with @b Ga and @b ba
                  (Each output mode has a different calculation method as
                  described in this enum)

@note  @b Caution: About the maximum value of Alpha (indicated as
       @b max_alpha):
       - If it is represented by 8 bit, its maximum value is @b 256
       - If it is represented by 1 bit, its maximum value is @b 1
*/
typedef enum aui_alpha_out_mode {

    /**
    Value to specify that
    - If the raster mode is set as the enum value #AUI_GFX_ROP_DERECT_COVER then
      the output will be pixel alpha of foreground surface directly
    - If the raster mode is set as the enum value #AUI_GFX_ROP_DERECT_COVER then
      the output will be the result of alpha blending between foreground and
      background, where:
      - @b fa'  = @b fa
      - @b ba'  = @b ba
      - @b fc'  = <b> fc x fa' </b>
      - @b bc'  = @b bc
      - @b Ga   = @b 1
      - @b da   = Result of alpha blending which usual follows the formula
                  <b> fa' + ba' x (1-fa') </b>
    */
    AUI_ALPHA_OUT_NORMAL = 0,

    /**
    Value to specify that:
    - @b fa' = <b> fa x Ga </b>
    - @b ba' = @b ba
    - @b fc' = <b> fc x fa' </b>
    - @b bc' = @b bc
    - @b Ga  = (foreground global alpha) / @b max_alpha
    - @b da  = @b fa'

    In this mode, foreground global alpha and foreground pixel alpha will work

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_ALPHA_OUT_FG_GLOBAL_ALPHA,

    /**
    Value to specify that:
    - @b fa' = <b> fa x Ga </b>
    - @b ba' = @b ba
    - @b fc' = <b> fc x fa' </b>
    - @b bc' = @b bc
    - @b Ga  = (background global alpha) / @b max_alpha
    - @b da  = @b fa'

    In this mode, background global alpha will work

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_ALPHA_OUT_BG_GLOBAL_ALPHA,

    /**
    Value to specify that:
    - @b fa' = @b fa
    - @b ba' = @b ba
    - @b fc' = <b> fc x fa' </b>
    - @b bc' = @b bc
    - @b Ga  = @b 1
    - @b da  = @b fa'

    In this mode, foreground pixel alpha will work

    @note  This mode is @a only available in projects based on <b> TDS OS </b>>
    */
    AUI_ALPHA_OUT_FG_PIXEL_ALPHA,

    /**
    Value to specify that:
    - @b fa' = @b fa
    - @b ba' = @b ba
    - @b fc' = <b> fc x fa' </b>
    - @b bc' = @b bc
    - @b Ga  = @b 1
    - @b da  = @b ba'

    In this mode, background pixel alpha will work

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_ALPHA_OUT_BG_PIXEL_ALPHA,

    /**
    Value to specify that:
    - @b fa' = <b> fa x Ga </b>
    - @b ba' = @b ba
    - @b fc' = <b> fc  x fa' </b>
    - @b bc' = @b bc
    - @b Ga  = (foreground global alpha) / @b max_alpha
    - @b da  = Result of alpha blending which usual follows the formula
               <b> fa' + ba' x (1-fa') </b>

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_ALPHA_OUT_FG_GLOBAL_ALPHA_BLENDING,

    /**
    Value to specify that:
    - @b fa' = @b fa
    - @b ba' = <b> ba x Ga </b>
    - @b fc' = <b> fc x fa' </b>
    - @b bc' = @b bc
    - @b Ga  = (background global alpha) / @b max_alpha
    - @b da  = Result of alpha blending which usual follows the formula
               <b> fa' + ba' x (1-fa') </b>

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_ALPHA_OUT_BG_GLOBAL_ALPHA_BLENDING,

    /**
    Value to specify that:
    - @b fa' = <b> fa x Ga </b>
    - @b ba' = @b ba
    - @b fc' = @b fc
    - @b bc' = @b bc
    - @b Ga  = (foreground global alpha) / @b max_alpha
    - @b da  = Result of alpha blending which usual follow the formula
               <b> da = fa' </b>
    */
    AUI_ALPHA_OUT_BLENDING_FG_GLOBAL_ONLY

} aui_alpha_out_mode;

/**
    Enum to specify the color space mode to be used when creating a surface

    @note @b 1. The color space mode can be specified by
                - Either the function #aui_gfx_sw_surface_create
                - Or the function #aui_gfx_hw_surface_create

    @note @b 2. When calling the function of @b aui_gfx_hw_surface_create, if
                the GMA layer type is set as the enum value #AUI_OSD_LAYER_GMA1
                then the supported color space mode will be @b CLUT4/8
  */
typedef enum aui_osd_pixel_format {

    /**
    Value to specify @b CLUT4 as color space mode

    @warning  GMA layer #AUI_OSD_LAYER_GMA0 can not support this pixel format
    */
    AUI_OSD_4_COLOR =   0,

    /**
    Value to specify @b CLUT8 as color space mode

    @warning  GMA layer #AUI_OSD_LAYER_GMA0 specified in the enum #aui_osd_layer
            can not support this pixel format
    */
    AUI_OSD_256_COLOR,

    /**
    Value to specify @b CLUT4 as color space mode

    @warning  Presently this value is not used than can be ignored
    */
    AUI_OSD_16_COLOR_PIXEL_ALPHA,

    /**
    Value to specify @b ACLUT88 as color space mode, where:
    - 8 bit for Alpha
    - 8 bit for CLUT

    @warning  Presently this value is not used than can be ignored
    */
    AUI_OSD_HD_ACLUT88,

    /**
    Value to specify @b RGB565 for High Definition as color space mode
    */
    AUI_OSD_HD_RGB565,

    /**
    Value to specify @b RGB888 for High Definition as color space mode
    */
    AUI_OSD_HD_RGB888,

    /**
    Value to specify @b RGB555 for High Definition as color space mode
    */
    AUI_OSD_HD_RGB555,

    /**
    Value to specify @b RGB444 as color space mode

    @warning  Presently this value is not used than can be ignored
    */
    AUI_OSD_HD_RGB444,

    /**
    Value to specify @b ARGB8888 for High Definition as color space mode
    */
    AUI_OSD_HD_ARGB8888,

    /**
    Value to specify @b ARGB1555 for High Definition as color space mode
    */
    AUI_OSD_HD_ARGB1555,

    /**
    Value to specify @b ARGB4444 for High Definition as color space mode
    */
    AUI_OSD_HD_ARGB4444,

    /**
    Value to specify @b AYCbCr8888 for High Definition as color space mode
    */
    AUI_OSD_HD_AYCbCr8888,

    /**
    Value to specify @ YCBCR888 as color space mode

    @warning  Presently this value is not used than can be ignored
    */
    AUI_OSD_HD_YCBCR888,

    /**
    Value to specify @b YCBCR420MB as color space mode

    @warning  Presently this value is not used than can be ignored
    */
    AUI_OSD_HD_YCBCR420MB,

    /**
    Value to specify the masximum number of color space mode available in ths enum
    */
    AUI_OSD_COLOR_MODE_MAX

} aui_osd_pixel_format;

/**
    Enum to specify different boolean operation type.

    @note  @b 1. When the raster mode is set as the enum value
                 - Either #AUI_GFX_ROP_BOOLEAN
                 - Or #AUI_GFX_ROP_BOOL_ALPHA_BLENDIN
                 the boolean operation type between the foreground and background
                 data needs to be set, considering that the targeted display buffer
                 causes a different boolean operation to be displayed on the screen
                 as final result. The boolean operation type can be set by the
                 function #aui_gfx_surface_blit

    @note  This enum is @a only available in project based on <b> TDS OS </b>
*/
typedef enum aui_ge_bop_mode {

    /**
    Value to specify that the targeted display buffer is set to @b 0 in order to
    display <b> black screen </b>
    */
    AUI_GE_BOP_SETBLACK     = 0x00,
    /**

    Value to specify that the screen displaying result is the @b AND boolean
    operation between foreground and background data,
    i.e. <b> foreground & background </b>
    */
    AUI_GE_BOP_AND          = 0x01,

    /**
    Value to specify that the screen displaying result is the @b ANDINV boolean
    operation between foreground and background data
    i.e. <b> foreground & (~ background) </b>
    */
    AUI_GE_BOP_ANDINV       = 0x02,

    /**
    Value to specify that the screen displays @b foreground data
    */
    AUI_GE_BOP_UPDATE       = 0x03,

    /**
    Value to specify that the screen displaying result is the @b INVAND boolean
    operation between foreground and background data
    i.e. <b> (~ foreground) & background </b>
    */
    AUI_GE_BOP_INVAND       = 0x04,

    /**
    Value to specify that the screen displays @b background data
    */
    AUI_GE_BOP_REMAIN       = 0x05,

    /**
    Value to specify that the screen displaying result is the @b XOR boolean
    operation between foreground and background data
    i.e. <b> foreground ^ background </b>
    */
    AUI_GE_BOP_XOR          = 0x06,

    /**
    Value to specify that the screen displaying result is the @b OR boolean
    operation between foreground and background data
    i.e. <b> foreground | background </b>
    */
    AUI_GE_BOP_OR           = 0x07,

    /**
    Value to specify that the screen displaying result is the @b NOR boolean
    operation between foreground and background data
    i.e. <b> ~ (foreground | background) </b>
    */
    AUI_GE_BOP_NOR          = 0x08,

    /**
    Value to specify that the screen displaying result is the @b INVXOR boolean
    operation between foreground and background data
    i.e. <b> (~ foreground) ^ background </b>
    */
    AUI_GE_BOP_INVXOR       = 0x09,

    /**
    Value to specify that the screen displaying result is the @b NOT boolean
    operation of the background data
    i.e. <b> ~ background </b>
    */
    AUI_GE_BOP_INVERT       = 0x0A,

    /**
    Value to specify that the screen displaying result is the @b ORINV boolean
    operation between the foreground and background data
    i.e. <b> foreground | (~background) </b>
    */
    AUI_GE_BOP_ORINV        = 0x0B,

    /**
    Value to specify that the screen displaying result is the @b NOT boolean
    operation of the foreground data
    i.e. <b> ~ foreground </b>
    */
    AUI_GE_BOP_UPDATEINV    = 0x0C,

    /**
    Value to specify that the screen displaying result is the @b INVOR boolean
    operation between the foreground and background data
    i.e. <b> (~ foreground) | background </b>
    */
    AUI_GE_BOP_INVOR        = 0x0D,

    /**
    Value to specify that the screen displaying result is the @b NAND boolean
    operation between the foreground and background data
    i.e. <b> ~ (foreground & background) </b>
    */
    AUI_GE_BOP_NAND         = 0x0E,

    /**
    Value to specify that the targeted display buffer is set to @b 0xFFFFFFFF
    in order to <b> white screen </b>
    */
    AUI_GE_BOP_SETWHITE     = 0x0F

} aui_ge_bop_mode;

/**
Enum to specify different modes of alpha blending to be set for performing
boolean operation.

@note  When the raster mode is set as the enum value
       - Either #AUI_GFX_ROP_ALPHA_BLENDING
       - Or #AUI_GFX_ROP_BOOL_ALPHA_BLENDIN

       the mode of alpha blending need to be set, considering that the
       targeted display buffer causes a different boolean operation to be
       displayed on the screen as final result. The boolean operation type
       can be by the function
       - #aui_gfx_surface_blit
       - #aui_gfx_data_blit
@note  Each output mode has different calculation method as described in this
       enum, where the abbreviations used have the meaning listed below:
       - @b dc = Color value of the destination surface
       - @b fc  = Color value of foreground surface
       - @b bc  = Color value of background surface
       - @b fa  = (pixel alpha of foreground surface) / max_alpha
       - @b ba  = (pixel alpha of background surface) / max_alpha
       - @b Ga  = Coefficient @b Galpha, which can be:
                  - Either (foreground global alpha) / max_alpha
                  - Or (background global alpha) / max_alpha
                  - Or 1
@note  according the alpha output mode
       - @b fc' = Color value associated with @b fa, @b Ga and @b fc
                  (Each output mode has a different calculation method as
                  described in this enum)
       - @b bc' = Color value associated with @b ba, @b Ga and @b bc
                  (Each output mode has a different calculation method as
                  described in this enum)
       - @b fa' = Alpha value associated with @b Ga and @b fa
       - @b ba' = Alpha value associated with @b Ga and @b ba


@note  @b Caution: About the maximum value of Alpha (indicated as
       @b max_alpha):
       - If it is represented by 8 bit, its maximum value is @b 256
       - If it is represented by 1 bit, its maximum value is @b 1

@note  Please refer also to the enum #aui_alpha_out_mode for further details.\n

@note  This enum is @a only available in projects based on <b> TDS OS </b>
*/
typedef enum aui_ge_alpha_blend_mode {

    /**
    Value to specify that
    <b> dc = fc' + bc' x (1 - fa') </b>
    */
    AUI_GE_ALPHA_BLEND_SRC_OVER,

    /**
    Value to specify that
    <b> dc = fc'*(1 - ba') + bc' </b>
    */
    AUI_GE_ALPHA_BLEND_DST_OVER,

    /**
    Value to specify that
    <b> dc = fc' </b>
    */
    AUI_GE_ALPHA_BLEND_SRC,

    /**
    Value to specify that
    <b> dc = bc' </b>
    */
    AUI_GE_ALPHA_BLEND_DST,

    /**
    Value to specify that
    <b> dc = fc' x ba' </b>
    */
    AUI_GE_ALPHA_BLEND_SRC_IN,

    /**
    Value to specify that
    <b> dc = bc' x fa' </b>
    */
    AUI_GE_ALPHA_BLEND_DST_IN,

    /**
    Value to specify that
    <b> dc = fc' x (1 - ba') </b>
    */
    AUI_GE_ALPHA_BLEND_SRC_OUT,

    /**
    Value to specify that
    <b> dc = bc' x (1 - fa') </b>
    */
    AUI_GE_ALPHA_BLEND_DST_OUT,

    /**
    Value to specify that
    <b> dc = fc'*ba' + bc' x (1 - fa') </b>
    */
    AUI_GE_ALPHA_BLEND_SRC_ATOP,

    /**
    Value to specify that
    <b> dc = fc'*(1 - ba') + bc' x fa' </b>
    */
    AUI_GE_ALPHA_BLEND_DST_ATOP,

    /**
    Value to specify that
    <b> dc = fc' x (1 - ba') + bc' x (1 - fa') </b>
    */
    AUI_GE_ALPHA_BLEND_XOR,

    /**
    Value to specify that
    <b> dc = 0 </b>
    */
    AUI_GE_ALPHA_BLEND_CLEAR

} aui_ge_alpha_blend_mode;

/**
Enum to specify the <b> color key source mode </b> to be set for enabling/disabling
color key feature
*/
typedef enum aui_ge_color_key_mode {

    /**
    Value to specify that the color key mode is @b disabled
    */
    AUI_GE_CKEY_DISABLE,

    /**
    Value to specify that the color key mode is @b enabled then
    - The color key of @b background surface will take effect
    - The pixels of @b background surface will be filtered during blit operation
    */
    AUI_GE_CKEY_DST,

    /**
    Value to specify that the color key mode is @b enabled then
    - The color key of @b foreground surface will take effect
    - The pixels of @b foreground surface will be filtered during blit operation
    */
    AUI_GE_CKEY_PTN_POST_CLUT,

    /**
    Value to specify that the color key mode is @b enabled then
    - The color key of @b foreground surface will take effect
    - The specified value is the one of the index of palette, and the pixels of
      @b foreground surface will be filtered during blit operation
    */
    AUI_GE_CKEY_PTN_PRE_CLUT

} aui_ge_color_key_mode;

/**
Enum to specify the <b> color key match mode </b>

@note  This enum is used when the <b> color key source mode </b> is not set as
       the value @b AUI_GE_CKEY_DISABLE as defined in the enum #aui_ge_color_key_mode
       (i.e. when the color key is enabled), then the color to be filtered can
       be set by the function
       - Either #aui_gfx_surface_blit
       - Or #aui_gfx_data_blit
*/
typedef enum aui_ge_color_key_match_mode {

    /**
    Value to specify that all values in the RGB color channel are matched and
    will be filtered, then the screen will display black
    */
    AUI_GE_CKEY_MATCH_ALWAYS,

    /**
    Value to specify that the value in range will be filtered in the RGB color
    channel as defined in the struct #aui_color_key

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_GE_CKEY_MATCH_IN_RANGE,

    /**
    Value to specify that the value out of range will be filtered in the RGB
    color channel as defined in the struct #aui_color_key

    @note  This mode is @a only available in projects based on <b> TDS OS </b>
    */
    AUI_GE_CKEY_MATCH_OUT_RANGE

} aui_ge_color_key_match_mode;

/**
Enum to specify the <b> clipping mode </b>

@note  When the clipping mode is enabled, either the outer or inner rectangular
       area can be clipped. The clipping mode can be set by the function
       #aui_gfx_surface_clip_rect_set
*/
typedef enum aui_ge_clip_mode {

    /**
    Value to specify that the clipping feature is @b disabled
    */
    AUI_GE_CLIP_DISABLE,

    /**
    Value to specify that the @b inner rectangular area will be clipped, i.e.
    @a only the inner rectangular area can be updated
    */
    AUI_GE_CLIP_INSIDE,

    /**
    Value to specify that the @b outer rectangular area will be clipped, i.e.
    @a only the outer rectangular area can be updated.
    */
    AUI_GE_CLIP_OUTSIDE

} aui_ge_clip_mode;

/**
Enum to specify the <b> mask filter mode </b>

@note  The mask filter mode can be set by the function #aui_gfx_surface_mask_filter
*/
typedef enum aui_gfx_mask_format {

    /**
    Value to specify @b 1 bit mask filter mode
    */
    AUI_GFX_MASK_1BIT = 0,

    /**
    Value to specify @b 8 bit mask filter mode
    */
    AUI_GFX_MASK_8BIT,

    /**
    Value to specify the maximum mask filter mode available in this enum
    */
    AUI_GFX_MASK_MAX

} aui_gfx_mask_format;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used
        for configuring blit operations
        </div> @endhtmlonly

        Struct used for configuring blit operations

@note   This struct @a must be set when the surface has to perform the blit
        operation by using the functions
        - Either #aui_gfx_surface_blit
        - Or #aui_gfx_data_blit
*/
typedef struct aui_st_blit_operation {

    /**
    Member to specify the <b> Raster Operation Mode </b> as defined in the enum
    #aui_gfx_rop_operation_type
    */
    aui_gfx_rop_operation_type rop_operation;

    /**
    Member to specify the <b> Boolean Operation Mode </b> as defined in the enum
    #aui_ge_bop_mode.

    @note  @1. This member is valid when the <b> Raster Operation Mode </b> is
               set as the enum value
               - Eitner #AUI_GFX_ROP_BOOLEAN
               - Or #AUI_GFX_ROP_BOOL_ALPHA_BLENDIN

    @note  @2. This member is @a only available in projects based on <b> TDS OS </b>
    */
    aui_ge_bop_mode bop_mode;

    /**
    Member to specify the <b> Alpha Blending Mode </b> as defined in the enum
    #aui_ge_alpha_blend_mode

    @note  This member is valid when the <b> Raster Operation Mode </b> is set
           as the enum value
           - Either #AUI_GFX_ROP_ALPHA_BLENDING
           - Or #AUI_GFX_ROP_BOOL_ALPHA_BLENDIN
    */
    aui_ge_alpha_blend_mode alpha_blend_mode;

    /**
    Member to specify the <b> Color Key Mode </b> as defined in the enum
    #aui_ge_color_key_mode
    */
    aui_ge_color_key_mode color_key_source;

    /**
    Member to specify the <b> Color Key Match Mode </b> in @b Red Channel as
    defined in the enum #aui_ge_color_key_match_mode

    @note  @b 1. This member is valid when the <b> Color Key Source Mode </b>
                 is enabled

    @note  @b 2. This member is @a only available in projects based on <b> TDS
                 OS </b>
    */
    aui_ge_color_key_match_mode color_key_red_match_mode;

    /**
    Member to specify the <b> Color Key Match Mode </b> in @b Green Channel as
    defined in the enum #aui_ge_color_key_match_mode

    @note  @b 1. This member is valid when the <b> Color Key Source Mode </b>
                 is enabled

    @note  @b 2. This member is @a only available in projects based on <b> TDS
                 OS </b>
    */
    aui_ge_color_key_match_mode color_key_green_match_mode;

    /**
    Member to specify the <b> Color Key Match Mode </b> in @b Blue Channel as
    defined in the enum #aui_ge_color_key_match_mode

    @note  @b 1. This member is valid when the <b> Color Key Source Mode </b>
                 is enabled

    @note  @b 2. This member is @a only available in projects based on <b> TDS
                 OS </b>
    */
    aui_ge_color_key_match_mode color_key_blue_match_mode;

    /**
    Member to specify the <b> Alpha Output Mode </b> as defined in the enum
    #aui_alpha_out_mode

    @note  This member is valid when the <b> Raster Operation Mode </b> is set
           as the enum value
           - Either #AUI_GFX_ROP_ALPHA_BLENDING
           - Or #AUI_GFX_ROP_BOOL_ALPHA_BLENDIN
    */
    aui_alpha_out_mode alpha_out_mode;

    /**
    Member to specify the <b> Rotation Angle </b> value

    @warning  Presently this member is not used and its value is set to @b 0 as
              default
    */
    unsigned long  rotate_degree;

    /**
    Member to specify the <b> Mirror Operation Mode </b>  as defined in the enum
    #aui_gfx_mirror_type
    */
    aui_gfx_mirror_type mirror_type;

} aui_blit_operation;

/// @cond

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used
        for setting/getting different hardware surface information
        </div> @endhtmlonly

        Struct used for setting/getting different hardware surface information
*/
typedef struct st_hw_surface_private_info_ {

    /**
    Member to specify the <b> OSD Layer ID </b> used by the current surface as
    defined in the enum #aui_osd_layer
    */
    unsigned long layer_id;

    /**
    Member to specify the <b> OSD Layer Handle </b> used by the current surface

    @warning  This handle cannot be @b NULL
    */
    aui_hdl layer_handle;

    /**
    Member to specify the <b> Hardware Surface ID </b>
    */
    unsigned long hw_surface_id;

    /**
    Member to specify the <b> Horizontal coordinate </b> (i.e. X axis) of the
    upper left corner of the hardware surface.

    @note  It must belong to the range
           <b> [0, maximum width value supported by the system] </b>
    */
    unsigned long left_off_set;

    /**
    Member to specify the <b> Vertical Coordinate </b> (i.e. Y axis) of the upper
    left corner of the hardware surface

    @note  It must belong to the range
           <b> [0, maximum height value supported by the system] </b>
    */
    unsigned long top_off_set;

    /**
    Member as a @a flag to specify the usage or not of the <b> Double Buffer </b>
    valid @a only for hardware surface, in  particular:
    - @b 0 = The hardware surface does not use double buffer, then the hardware
             buffer is the display buffer which is directly shown on the screen
    - @b 1 = The hardware surface use double buffer, then the hardware
             - Firstly use one buffer to store the display data which have not be shown
             - Secondly use another buffer as display buffer

    To display the hardware buffer on the screen then the function #aui_gfx_surface_flush
    @a must be called

    */
    unsigned long isdouble_buf;

    /**
    Member to specify the <b> Internal Buffer Pointer </b> which is used to store
    not displayed data.

    @note  @b 1. That internal buffer pointer is 16 bytes aligned

    @note  @b 2. This member is valid when the member @b isdouble_buf of this
                 struct is set as @b 1
    */
    unsigned char *second_buf;

    /**
    Member to specify the <b> Internal Backup Buffer Pointer </b> which is used
    to store not displayed data

    @note  It is the backup of the <b> internal buffer pointer </b> specified
           in the member @b seconf_buf of this struct

    @note  That internal buffer backup pointer is 16 byte aligned (as the
           internal buffer pointer)
    */
    unsigned char *second_buf_backup;

} hw_surface_private_info;

/// @endcond

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used for
        storing the palette information
        </div> @endhtmlonly

        Struct used for storing the palette information

@note   This struct is used by the functions
        - #aui_gfx_surface_pallette_get
        - #aui_gfx_surface_pallette_set
*/
typedef struct aui_st_pallette_info {

    /**
    Member to specify the value of the buffer pointer in the palette lookup table

    @note  This pallette table can be set by the function #aui_gfx_surface_pallette_set
    */
    unsigned char *p_pallette;

    /**
    Member to specify the number of color in the pallette table
    */
    unsigned long color_cnt;

    /**
    Member to specify the color space type in pallette lookup table as defined
    in the enum #aui_pallette_color_type
    */
    aui_pallette_color_type en_pallette_color_type;

} aui_pallette_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used for
        storing surface information
        </div> @endhtmlonly

        Struct used for storing surface information

@note   This struct is used by the function #aui_gfx_surface_info_get
*/
typedef struct aui_st_surface_info {

    /**
    Member as a @a flag to specify the <b> Hardware Surface </b>, in particular:
    - @b 1 = The surface is the hardware surface and it can be displayed on the
             screen
    - @b 0 = The surface is the software surface and it cannot be displayed on
             the screen, but it has memory buffer to store data
    */
    unsigned long is_hw_surface;

    /**
    Member to specify the @b width of the surface, which belongs to the
    range <b> [0, maximum width value supported by the system] </b>
    */
    unsigned long width;

    /**
    Member to specify the @b height of the surface, which belongs to the
    range <b> [0, maximum height value supported by the system] </b>
    */
    unsigned long height;

    /**
    Member to specify the <b> color space </b> used by the current surface, as
    defined in the enum #aui_osd_pixel_format
    */
    aui_osd_pixel_format en_color_mode;

    /**
    Member to specify the number of bytes of one row in the memory

    @warning  The pitch is aligned to internal hardware requirement.\n
              The application can not calculate the pitch by the formula\n\n

              @b width x (bytes per pixel) \n \n

              but @a must use the @b pitch provided in the struct #aui_surface_info
              when copying data from/to the pixel buffer @b p_surface_buf as
              member of the same structure
    */
    unsigned long pitch;

    /**
    Member as a @a flag to specify the status of displaying surface, in
    particular:
    - @b 0 = The surface is not currently displayed
    - @b 1 = The surface is currently displayed
    */
    unsigned long show_onoff;

    /**
    Member to specify the maximum and minimum color key of the surface as defined
    in the struct #aui_color_key
    */
    aui_color_key color_key;

    /**
    Member to specify the <b> Global Alpha Value </b>
    */
    unsigned long golbal_alpha;

    /**
    Member to specify the <b> surface buffer pointer </b>

    @note  It is the pointer to the member @b p_surface_buf_backup defined
           in this struct, and it is 16 bytes aligned

    @warning  The pitch is aligned to internal hardware requirement. \n
              The application can not caculate the @b pitch by the formula \n \n

              width x (bytes per pixel) \n \n

              but @a must use the @b pitch provided in the struct #aui_surface_info
              when copying data from/to the pixel buffer p_surface_buf as
              member of the same structure
    */
    unsigned char * p_surface_buf;

    /**
    Member to specify the <b> surface backup buffer pointer </b>

    @note  @b 1. It is a real malloc address and maybe not 16 bytes aligned

    @note  @b 2. This member is @a only used by ALi R&D Dept. then user can
                 ignore it
    */
    unsigned char * p_surface_buf_backup;

    /**
    Member to specify the <b> buffer size </b> (in @a byte unit) of the surface
    data
    */
    unsigned long buf_size;

    /**
    Member to specify the <b> palette pointer </b> used by the surface

    @note  When creating a surface by bitmap this palette pointer needs to be
           set or it is used for color expansion
    */
    unsigned char *palette;

    /**
    Member to specify the clipping rectangular area, as defined in the struct
    #aui_osd_rect, where performing blit operations

    @note  This member is valid when the clipping mode is enabled, as defined
           in the enum #aui_ge_clip_mode
    */
    aui_osd_rect clip_rect;

    /**
    Member to specify palette information as defined in the struct #aui_pallette_info
    */
    aui_pallette_info pallettte_info;

    /**
    Member to specify the clipping mode as defined in the enum #aui_ge_clip_mode
    */
    aui_ge_clip_mode clip_mode;

    /**
    Member to specify the hardware surface private information as defined in the
    struct #hw_surface_private_info

    @note  This member is valid when the member @b is_hw_surface of this struct
           is set as @b 1
    */
    hw_surface_private_info hw_surface_info;

} aui_surface_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> used for
        configuring GMA scaling parameters
        </div> @endhtmlonly

        Struct used for configuring GMA scaling parameters.\n
        GMA does the scaling and displaying of the content on the screen according
        to proper parameters, and the scale ratio is the result of the
        calculation \n\n

        <b> output &divide; input </b> \n\n

        This struct is used when the size of the hardware surface is different
        from the size of the screen, and that scaling is a kind of standard change.\n
        Before displaying, if the size of the hardware surface is different from
        the size of the screen, proper parameters need to be set.
*/
typedef struct aui_st_scale_param {

    /**
    Member to specify the original width of hardware surface, which belongs to
    the range <b> [1, maximum value supported by the system] </b>
    */
    unsigned long input_width;

    /**
    Member to specify the original height of hardware surface which belongs to
    the range <b> [1, maximum value supported by the system] </b>
    */
    unsigned long input_height;

    /**
    Member to specify the width of hardware surface when is displayed on the
    screen which depends on the Aspect Ratio Mode
    */
    unsigned long output_width;

    /**
    Member to specify the height of hardware surface when is displayed on the
    screen which depends on the Aspect Ratio Mode
    */
    unsigned long output_height;

} aui_scale_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> to
        specify bitmap information/attributes
        </div> @endhtmlonly

        Struct to specify bitmap information/attributes.\n
        This struct is used to perfom the operation of bitmap, i.e.
        - Decoding a specified image file
        - Releasing a bitmap data or creating a surface with a bitmap
*/
typedef struct aui_gfx_bitmap_info {

    /**
    Member to specify the <b> color mode </b> used by bitmap
    */
    aui_osd_pixel_format color_type;

    /**
    Member to specify the @b width of the bitmap which value should be within
    the maximum value supported by the system
    */
    unsigned int width;

    /**
    Member to specify the @b height of the bitmap which value should be within
    the maximum value supported by the system
    */
    unsigned int height;

    /**
    Member to specify the number of bytes of one row in the memory
    */
    unsigned int pitch;

    /**
    Member to specify the bitmap data buffer pointer
    */
    void *p_data;

} aui_gfx_bitmap_info, aui_gfx_bitmap_info_t;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> to
        define affine matrices
        </div> @endhtmlonly

        Struct to define affine matrices.\n
        This struct is used for applying a linear combination of
        - Translation
        - Rotation
        - Scaling and/or Shearing

        which is performed by setting a proper map (matrix) of parameters.


@note   An example of a proper map (matrix) of parameters to define affine
        matrices is shown below:

        |   a_0_0   a_0_1   a_0_2   |
        |   a_1_0   a_1_1   a_1_2   |
        |     0       0       1     |

@note   \n To @b translate the surface, the parameters should be as below:

        |   1       0       a_0_2   |
        |   0       1       a_1_2   |
        |   0       0         1     |

@note   where
        - @b a_0_2 = Offset in the horizontal direction
        - @b a_1_2 = Offset in the vertical direction

@note   \n To @b rotate the surface, the parameters should be as below:

        |   cos(A)    -sin(A)   0   |
        |   sin(A)     cos(A)   0   |
        |     0          0      1   |

@note   where
        - @b A = Angle of rotation (in @b radiant unit)

@note   \n To @b shear the surface, the parameter should be as below:

        |     1       a_0_1     0   |
        |   a_1_0       1       0   |
        |     0         0       1   |

@note   where
        - @b a_0_1 = Width for shearing the surface
        - @b a_1_0 = Height for shearing the surface

@note   \n To @b scale the surface, the parameters should be as below:

        |   a_0_0       0       0   |
        |     0       a_1_1     0   |
        |     0         0       1   |

@note   where
        - @b a_0_0 = Zoom ratio in the vertical direction
        - @b a_1_1 = Zoom ratio in the horizontal direction
*/
typedef struct aui_gfx_affine_matrix {

    /**
    Member to specify the element at
    - @b First Row
    - @b First Column

    of the affine matrix
    */
    double a_0_0;

   /**
    Member to specify the element at
    - @b First Row
    - @b Second Column

    of the affine matrix
    */
    double a_0_1;

    /**
    Member to specify the element at
    - @b First Row
    - @b Third Column

    of the affine matrix
    */
    double a_0_2;

    /**
    Member to specify the element at
    - @b Second Row
    - @b First Column

    of the affine matrix
    */
    double a_1_0;

    /**
    Member to specify the element at
    - @b Second Row
    - @b Second Column

    of the affine matrix
    */
    double a_1_1;

    /**
    Member to specify the element at
    - @b Second Row
    - @b Third Column

    of the affine matrix
    */
    double a_1_2;

} aui_gfx_affine_matrix;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> On-Screen Display (OSD) & Graphic Module </b> to
        specify a mask filter for the screen
        </div> @endhtmlonly

        Struct to specify a mask filter to be used to mask an area of the screen
        to be not displayed
*/
typedef struct aui_gfx_mask_filter {

    /**
    Member to specify the pointer to the buffer to stores the mask data.\n

    @note  The mask data is usually a color data for masking the area where
           user wants to either display clearly or display translucently
           the data
    */
    unsigned char *mask_buf;

    /**
    Member to specify the number of pixels in a line
    */
    unsigned long pixel_per_line;

    /**
    Member to specify the current type of mask format as defined in the enum
    #aui_gfx_mask_format
    */
    aui_gfx_mask_format mask_format;

} aui_gfx_mask_filter;

/**
Enum used for setting
- Brightness
- Contrast
- Saturation
- Sharpness
- Hue

in order to enhance the OSD layer

@note  This enum is used @a only in projects based on <b> Linux OS </b>
*/
typedef enum aui_osd_enhance {

    /**
    Value to specify the @b brightness which value belongs to the range
    <b> [0, 100] </b>

    @note The default value is @b 50
    */
    AUI_OSD_ENHANCE_BRIGHTNESS = 0,

    /**
    Value to specify the @b contrast which value belongs to the range
    <b> [0, 100] </b>

    @note The default value is @b 50
    */
    AUI_OSD_ENHANCE_CONTRAST,

    /**
    Value to specify the @b saturation which value belongs to the range
    <b> [0, 100] </b>

    @note The default value is @b 50
    */
    AUI_OSD_ENHANCE_SATURATION,

    /**
    Value to specify the @b sharpness which value belongs to the range
    <b> [0, 100] </b>

    @note The default value is @b 50
    */
    AUI_OSD_ENHANCE_SHARPNESS,

    /**
    Value to specify the @b hue which value belongs to the range
    <b> [0, 100] </b>

    @note The default value is @b 50
    */
    AUI_OSD_ENHANCE_HUE

} aui_osd_enhance;

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to perform the initialization of the OSD Module,
                before its opening by the function #aui_gfx_layer_open

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the OSD Module

@param[in]      p_call_back_init    = Callback function used for the initialization
                                      of the OSD Module, as per comment for the
                                      function pointer #p_fun_cb
@param[in]      pv_param            = Callback function parameter list, as per
                                      comment for the function pointer #p_fun_cb

@return         @b AUI_RTN_SUCCESS  = OSD Module initialized successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Initializing of the OSD Module failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           About the input parameter @b p_call_back_init, i.e. the callback
                function:
                - In projects based on <b> Linux OS </b>, it is suggested to set
                  as NULL
                - In projects based on <b> TDS OS </b>, it will perform some OSD
                  device attachments and configurations.

@note           Please refer to the sample code of the initialization for more
                clarifications
*/
AUI_RTN_CODE aui_gfx_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to perform the de-initialization of the OSD Module,
                after its closing by the function #aui_gfx_layer_close

@warning        When the system is going to shut down or reboot, this function
                should be called to free the resources used by the OSD module

@param[in]      p_call_back_init    = Callback function used for the de-initialization
                                      of the OSD Module, as per comment for the
                                      function pointer #p_fun_cb
@param[in]      pv_param            = Callback function parameter list, as per
                                      comment for the function pointer #p_fun_cb

@return         @b AUI_RTN_SUCCESS  = OSD Module de-initialized successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = De-initializing of the OSD Module failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           @a Only in projects based on <b> TDS OS </b>, it is suggested
                to use this function before going into standby mode
*/
AUI_RTN_CODE aui_gfx_de_init (

    p_fun_cb p_call_back_init,

    void *pv_param

    );

/**
@brief          Function used to open a hardware layer, initialize the related
                resource (i.e. open GE and GMA) for OSD Module and returns the
                OSD layer handle

@param[in]      layer_id            = ID of the OSD hardware layer used by the
                                      driver to perform the realated opening, as
                                      defined in the enum #aui_osd_layer

@param[out]     p_gfx_layer_handle  = Handle of the OSD layer used to perform
                                      some operations in the OSD layer, e.g.
                                      - Turn on/off the OSD layer display
                                      - Set layer alpha
                                      - Create the corresponding hardware surface
                                      - etc.

@return         @b AUI_RTN_SUCCESS  = Opening of the hardware layer performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b other_Values     = Opening of the hardware layer failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           Different ICs have different layer ID which can be supported by
                the system
*/
AUI_RTN_CODE aui_gfx_layer_open (

    aui_osd_layer layer_id,

    aui_hdl *p_gfx_layer_handle

    );

/**
@brief          Function used to close the hardware layer and de-initialize the
                related resource (i.e. close GE and GMA) for OSD module

@warning        Before calling this function, please make sure that all hardware
                surfaces created in the OSD layers are closed

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open

@return         @b AUI_RTN_SUCCESS  = Closing of the OSD layer performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Closing of the OSD layer failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_layer_close (

    aui_hdl gfx_layer_handle

    );

/**
@brief          Function used to turn on/off the anti-flicker attribute

@note           If the OSD displayed on the screen looks flickering, this
                attribute can be changed then see the related affect.\n

@warning        Before calling this function, please make sure that the
                corresponding OSD layer (i.e. specified by the handle)
                has already been opened by the function #aui_gfx_layer_open

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open
@param[in]      ul_on_off           = Flag to set on/off the anti-flicker attribute,
                                      in paticular:
                                      - @b 0 = Off
                                      - @b 1 = On

@return         AUI_RTN_SUCCESS     = Setting of the anti-flicker attribute
                                      performed succesfully
@return         AUI_RTN_EINVAL      = A least one input parameter (i.e. [in])
                                      is invalid
@return         Other_Values        = Setting of the anti-flicker attribute failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_layer_antifliker_on_off (

    aui_hdl gfx_layer_handle,

    unsigned long ul_on_off

    );

/**
@brief          Function used to turn on/off the OSD layer specified by the
                handle, then let the OSD layer show itself on the screen or not

@warning        Before calling this function, please make sure that the
                corresponding OSD layer (i.e. specified by the handle) has
                already been opened by the function #aui_gfx_layer_open

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open
@param[in]      ul_on_off           = Flag to set on/off the showing of OSD on
                                      the screen, in particular
                                      - @b 0 = Off
                                      - @b 1 = On

@return         @b AUI_RTN_SUCCESS  = Setting of the showing of the OSD layer
                                      on the screen or not performed succesfully
@return         @b AUI_RTN_EINVAL   = A least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the showing of the OSD layer
                                      on the screen or not failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_layer_show_on_off (

    aui_hdl gfx_layer_handle,

    unsigned long ul_on_off

    );

/**
@brief          Function used to configure the Global Alpha value for OSD layer

@note           The Global Alpha value is covered on the OSD UI, then is mixed
                with OSD UI on the screen

@warning        Before calling this function, please make sure that the
                corresponding OSD layer (i.e. specified by the handle)
                has already been opened by the function #aui_gfx_layer_open

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open
@param[in]      ul_alpha            = The Global Alpha value for OSD layer to
                                      be set

@return         @b AUI_RTN_SUCCESS  = Global Alpha value configured successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Global Alpha value failed to be configured
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_layer_alpha_set (

    aui_hdl gfx_layer_handle,

    unsigned long ul_alpha

    );

/**
@brief          Function used to get the Global Alpha value of OSD layer

@note           The Global Alpha value is covered on the OSD UI, then is mixed
                with OSD UI on the screen

@warning        Before calling this function, please make sure that the
                corresponding OSD layer (i.e. specified by the handle)
                has already been opened by the function #aui_gfx_layer_open

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open

@param[out]     pul_alpha           = The Global Alpha value just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the Global Alpha value performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the Global Alpha value failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_layer_alpha_get (

    aui_hdl gfx_layer_handle,

    unsigned long *pul_alpha

    );

/**
@brief          Function used to scale the surface size same with TV system to
                be displayed on the screen

@note           This function is useful when changing the TV system and is valid
                for hardware surface.

@warning        User can call this function to decided in which mode the hardware
                surface is displayed on the screen.\n
                When the surface for displaying UI does not fit the screen,
                the application can call this function to scale the surface.\n

                As example,
                - Surface size is <b> 1920 x 1080 </b>
                - TV system is <b> 720 x 576 (PAL) </b>

                then the application needs to scale OSD output to <b> 720 x 576 </b>
                by performing the below configuration of the proper members of
                the struct #aui_scale_param as the parameter @b p_scale_param:
                - Input width   = 1920
                - Input height  = 1080
                - Output width  = 720
                - Output height = 576

                User can get the current TV system by the function @b aui_dis_get

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open
@param[in]      p_scale_param       = The scaling value for the surface size as
                                      defined in the struct @ #aui_scale_param

@return         @b AUI_RTN_SUCCESS  = Scaling of the surface size performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Scaling of the surface size failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_layer_scale (

    aui_hdl gfx_layer_handle,

    const aui_scale_param *p_scale_param

    );

/**
@brief          Function used to perform specific enhancement of the OSD layer

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open
@param[in]      en_type             = Specific enhancement of the OSD layer to
                                      be set, as defined in the enum
                                      #aui_osd_enhance
@param[in]      en_value            = Desired value to be set for the specific
                                      enhancement of the OSD layer

@return         @b AUI_RTN_SUCCESS  = Setting of the specific enhancement of
                                      the OSD layer performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Setting of the specific enhancement of
                                      the OSD layer failed for some reasons,
                                      as defined in the enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux Os </b>
*/
AUI_RTN_CODE aui_gfx_layer_enhance_set (

    aui_hdl gfx_layer_handle,

    aui_osd_enhance en_type,

    unsigned int en_value

    );

/**
@brief          Function used to create a software surface

@note           The software surface actually stores OSD data or UI material to
                memory buffer, then uses GE to do drawing, filling, etc.

@warning        Before destroying the surface create by this function, user needs
                to call the function #aui_gfx_surface_delete

@param[in]      pixel_format        = Pixel format used for the software surface
                                      which is used to manage GE drawing
                                      (i.e. the the application can store the UI
                                      material or the result of GE drawing in the
                                      software surface; after GE drawing is
                                      completed user can call the function
                                      #aui_gfx_surface_blit to perform blit
                                      operation with the software surface to
                                      display buffer for showing on the screen)
@param[in]      ul_width            = Width of the software surface
@param[in]      ul_height           = Height of the software surface

@param[out]     p_surface_handle    = Software Surface handle which will be used
                                      to do blitting, filling, capture, etc.

@return         @b AUI_RTN_SUCCESS  = Creating of the software surface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out]) is
                                      invalid
@return         @b Other_Values     = Creating of the software surface failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_sw_surface_create (

    aui_osd_pixel_format pixel_format,

    unsigned long ul_width,

    unsigned long ul_height,

    aui_hdl *p_surface_handle

    );

/**
@brief          Function used to create a software surface by using the
                preallocated bitmap source.

@note           Since the software surface is created with bitmap source, OSD
                Module can do graphic operations directly in the specified bitmap
                buffer, and does not need to malloc another surface buffer used
                to copy data between the specified bitmap buffer and surface buffer.

@warning        When destroying this created software surface by the function
                #aui_gfx_surface_delete, the specified bitmap buffer will not be
                released then the image buffer needs to be freed by the user
                specifically

@param[in]      p_bitmap_info       = Detailed bitmap information such as
                                      - Width
                                      - Height
                                      - Pitch
                                      - Color Mode
                                      - etc.

                                      as defined in the struct #aui_gfx_bitmap_info,
                                      the software surface will be created according
                                      to those values

@param[out]     p_surface_handle    = Software surface handle which will be used
                                      for monitoring the status

@return         @b AUI_RTN_SUCCESS  = Creating of the software surface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Creating of the software surface failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_sw_surface_create_by_bitmap (

    aui_hdl *p_surface_handle,

    const aui_gfx_bitmap_info *p_bitmap_info

    );

/**
@brief          Function used to create a new hardware surface

@note           Hardware surface is used to manage the OSD buffer which will be
                directly used in the OSD layer for display purpose.\n
                The application can create several hardware surface, but cannot
                be overlapped in row. \n \n

                @b Example: \n \n

                The size of the layer @b G1 as per the enum value
                #AUI_OSD_LAYER_GMA1 is 1920 x 1080 bytes for both Linux and TDS
                OS platform.\n
                Then the maximun sizes of the hardware surface that can be created
                on this layer with different pixel format are listed below:
                -  8 bits per pixel -> 1920 x 1080
                - 16 bits per pixel -> 1371 x 771
                - 32 bits per pixel ->  960 x 540

@param[in]      gfx_layer_handle    = Handle of the OSD layer returned by the
                                      function #aui_gfx_layer_open
@param[in]      pixel_format        = Pixel format for the hardware surface
@param[in]      p_rect              = Visible area of the rectangle for the
                                      hardware surface, specified by
                                      - Width
                                      - Height
                                      - Upper left corner coordinates
                                      as defined in struct #aui_osd_rect
@param[in]      ul_is_double_buf    = Flag to indicate whether the hardware
                                      surface uses double buffer for displaying
                                      or not, in particular:
                                      - @b 0 = Not used, then the display buffer
                                               used by the hardware surface will
                                               directly be shown on the screen
                                      - @b 1 = Used, then the function
                                               #aui_gfx_surface_flush need to be
                                               called to show the display buffer
                                               on the screen

@param[out]     p_hw_surface_handle = Handle of the hardware surface just created

@return         @b AUI_RTN_SUCCESS  = Creating of the new hardware surface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out]) is
                                      invalid
@return         @b Other_Values     = Creating of the new hardware surface failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@warning  The GMA layer #AUI_OSD_LAYER_GMA0 defined in the enum #aui_osd_layer
          can not support the color space mode #AUI_OSD_4_COLOR and
          #AUI_OSD_256_COLOR defined in the enum #aui_osd_pixel_format
*/
AUI_RTN_CODE aui_gfx_hw_surface_create (

    aui_hdl gfx_layer_handle,

    aui_osd_pixel_format pixel_format,

    aui_osd_rect *p_rect,

    aui_hdl *p_hw_surface_handle,

    unsigned long ul_is_double_buf

    );

/**
@brief          Function used to delete surfaces, including hardware or software,
                then release all the related resource.

@warning        After a surface the corrisponding handle cannot be used anymore

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create

@return         @b AUI_RTN_SUCCESS  = Deleting of the urface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Deleting of the surface failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_delete (

    aui_hdl p_surface_handle

    );

/**
@brief          Function used for performing blit operations between different
                types of surface on a rectangular area on the scree such as:
                - Scale
                - Alpha blend
                - Boolean
                - Color key
                - Clip
                - Mirror
                - etc.

@note           If the sizes of the rectangular area are different between
                foreground, background and destination, then scaling will be
                executed firstly to make the size of the rectangle in
                foreground and background the same.\n
                This scaling operation will be performed by GE automatically,
                the application does not need to take care of this.\n
                But this scaling operation can @a only be supported when the
                color space mode is set as the enum value #AUI_OSD_HD_ARGB8888

@param[in]      dst_surface_handle  = Destination surface handle which is returned
                                      by the function #aui_gfx_hw_surface_create
                                      or #aui_gfx_sw_surface_create
@param[in]      fg_surface_handle   = Foreground surface handle which is returned
                                      by the function #aui_gfx_hw_surface_create
                                      or #aui_gfx_sw_surface_create
@param[in]      bg_surface_handle   = Background surface handle which is valid
                                      when the operation is boolean or alpha blend,
                                      and is returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
                                      - @b Caution: Background surface can be
                                           the same surface as the destination
                                           surface.\n
                                           If background surface is NULL, when the
                                           operation is boolean or alpha blend,
                                           the destination surface will be used
                                           as background surface.

@param[in]      p_blit_operation    = Blit operation configuration as defined in
                                      the struct #aui_blit_operation
@param[in]      p_blit_rect         = Specific rectangular area on the screen for
                                      performing blit operations as defined in the
                                      struct #aui_blit_rect

@return         @b AUI_RTN_SUCCESS  = Surface blit operations performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Surface blit operations failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_blit (

    aui_hdl dst_surface_handle,

    aui_hdl fg_surface_handle,

    aui_hdl bg_surface_handle,

    aui_blit_operation *p_blit_operation,

    aui_blit_rect *p_blit_rect

    );

/**
@brief          Function used to blit pure data to the destination surface,

@note           Pure data maybe:
                - Pictures
                - Colors
                - UI
                - etc.

                for which no need to create any source surfaces since.\n
                This function can perform blit operations between pure data and
                surfaces such as
                - Scale
                - Global Alpha blend
                - Boolean
                - Color key
                - Clip
                - etc.

@param[in]      dst_surface_handle  = Destination surface handle which is returned
                                      by the function #aui_gfx_hw_surface_create
                                      or #aui_gfx_sw_surface_create
@param[in]      puc_data            = Data buffer pointer
                                      - @b Caution: That pointer cannot be @b NULL
@param[in]      p_blit_rect         = Rectangular area for the destination surface,
                                      as defined in the struct #aui_blit_rect
@param[in]      p_blit_operation    = Configurations for blit operations as
                                      defined in the struct #aui_blit_operation
@param[in]      ul_pitch            = Number of bytes in one row of the source
                                      image data
@param[in]      pixel_format        = Pixel format of the pure data as defined
                                      in the enum #aui_osd_pixel_format

@return         @b AUI_RTN_SUCCESS  = Blitting of the pure data to the destination
                                      surface performed successfully
@return         @b AUI_RTN_EINVAL   = A least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Blitting of the pure data to the destination
                                      surface failed for some reasons, as defined
                                      in the enum #ui_gfx_errno_type_em
*/
AUI_RTN_CODE aui_gfx_data_blit (

    aui_hdl dst_surface_handle,

    unsigned char *puc_data,

    aui_blit_rect *p_blit_rect,

    aui_blit_operation *p_blit_operation,

    unsigned long ul_pitch,

    aui_osd_pixel_format pixel_format

    );

/**
@brief          Function used to capture the data on to the surface

@note           The range of the rectangle cannot be out of the rectangle of
                the surface

@param[in]      surface_handle      = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      p_rect              = Rectangular area for capturing data as
                                      defined in the struct #aui_osd_rect

@param[out]     pv_data             = Pointer to the data just captured in the
                                      rectangle

@return         @b AUI_RTN_SUCCESS  = Capturing of the data on to the surface
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Capturing of the data on to the surface
                                      failed for some reasons, as defined in
                                      the enum #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_capture (

    aui_hdl surface_handle,

    aui_osd_rect *p_rect,

    void *pv_data

    );

/**
@brief          Function used to fill a surface with solid colors

@note           User can call this function to show an area with the desired
                color on the screen

@param[in]      surface_handle      = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      ul_color            = Desired color of the area to be shown which
                                      depends on the surface color space mode, as
                                      defined in the enum #aui_osd_pixel_format
@param[in]      p_rect              = Rectangular area to be filled, as defined
                                      in the struct #aui_osd_rect

@return         @b AUI_RTN_SUCCESS  = Filling of the surface with the desired
                                      color performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Filling of the surface with the desired
                                      color failed for some reasons, as defined
                                      in the enum #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_fill (

    aui_hdl surface_handle,

    unsigned long ul_color,

    aui_osd_rect *p_rect

    );

/**
@brief          Function used to draw a line in the horizontal and vertical
                direction with a specified colors

@param[in]      surface_handle      = Surface handle returned by the fucntion
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      ul_color            = Color of the line to draw which depends on
                                      the surface color space mode, as defined
                                      in the enum #aui_osd_pixel_forma
@param[in]      p_start_coordinate  = Starting point coordinate of the colored
                                      line to draw
@param[in]      p_end_coordinate    = Ending point coordinate of the colored
                                      line to draw

@return         @b AUI_RTN_SUCCESS  = Drawing of the colored line performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Drawing of the colored line failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_draw_line (

    aui_hdl surface_handle,

    unsigned long ul_color,

    aui_coordinate *p_start_coordinate,

    aui_coordinate *p_end_coordinate

    );

/**
@brief          Function used to draw a rectangle that has outline with a
                specified color

@param[in]      surface_handle      = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      ul_fore_color       = Foreground color as the outline color of
                                      the rectangle
@param[in]      ul_back_color       = Background color as the fill color of the
                                      rectangle
                                      - @b Caution: The value of this parameter
                                                    is valid if the flag
                                                    @b ul_fill_background is set
                                                    as @b 1
@param[in]      p_rect              = Rectangular area to be drawn as defined
                                      in the struct #aui_osd_rect
@param[in]      ul_fill_background  = Flag to indicate whether filling the
                                      rectangle with a color or not, in particular
                                      - @b 1 = Fill
                                      - @b 0 = Not Fill

@return         @b AUI_RTN_SUCCESS  = Drawing of the rectangle performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Drawing of th rectangle failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_draw_outline (

    aui_hdl surface_handle,

    unsigned long ul_fore_color,

    unsigned long ul_back_color,

    aui_osd_rect *p_rect,

    unsigned long ul_fill_background

    );

/**
@brief          Function used to flush the data in the backup OSD buffer to OSD
                display buffer.

@note           To display hardware surface buffer on the screen, please call
                this function since the hardware surface is using double buffer.\n

@note           To perform blit/fill operation when the hardware is using double
                buffer, please call this function which result is the displaying
                of the buffer content on to the display screen.

@param[in]      dst_surface_handle  = Surface handle which is actually related
                                      to the hardware surface handle
@param[in]      p_dst_rect          = Rectangular area to be displayed or flushed,
                                      which should be set within the range of the
                                      surface

@return         @b AUI_RTN_SUCCESS  = Flushing of the data performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values
= Flushing of the data failed for some reasons,
                                      as defined in the enum #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_flush (

    aui_hdl dst_surface_handle,

    aui_osd_rect *p_dst_rect

    );

/**
@brief          Function used to get surface information

@note           With this function, user can get information about the surface
                such as
                - Width
                - Height
                - Color mode
                - etc.

@note           as defined in the struct #aui_surface_info

@param[in]      p_surface_handle    = Surface handle which actually is related
                                      to the hardware surface handle and returned
                                      by the function #aui_gfx_hw_surface_create
                                      or #aui_gfx_sw_surface_create

@param[out]     p_surface_info      = Pointer to a struct #aui_surface_info which
                                      collect the surface information just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the surface information performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Getting of the surface information failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_info_get (

    aui_hdl p_surface_handle,

    aui_surface_info *p_surface_info

    );

/**
@brief          Function used to turn on/off the showing of the hardware surface

@note           This function is valid for hardware surface, user can call it
                to decided whether the hardware surface is displayed on the screen
                or not

@param[in]      p_hw_surface_handle = Hardware surface handle, which is returned
                                      by the function #aui_gfx_hw_surface_create
@param[in]      ul_on_off           = Flag to indicate whether the hardware
                                      surface is displayed on the screen or not,
                                      in particular:
                                      - @b 0 = Off
                                      - @b 1 = On

@return         @b AUI_RTN_SUCCESS  = Turning on/off of the showing performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Turning on/off of the showing failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_show_on_off (

    aui_hdl p_hw_surface_handle,

    unsigned long ul_on_off

    );

/**
@brief          Function used to configure the surface color key

@note           When the color key is enabled, the color key will play a role
                according to the color mode and the color key range, and the
                color key will be filtered according the color key mode.\n
                Please refer to the struct #aui_color_key and the enum
                #aui_ge_color_key_match_mode for more details

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      p_color_key         = Pointer to the struct #aui_color_key
                                      intended to collect the color key format
                                      to be set as per below ranges:
                                      - <b> [31, 24] </b> = @b Alpha channel range
                                      - <b> [23, 16] </b> = @b Red   channel range
                                      - <b> [15, 08] </b> = @b Green channel range
                                      - <b> [07, 00] </b> = @b Blue  channel range

@return         @b AUI_RTN_SUCCESS  = Configuring of the surface color key
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Configuring of the surface color key failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_colorkey_set (

    aui_hdl p_surface_handle,

    aui_color_key *p_color_key

    );

/**
@brief          Function used to get the current color key of the surface

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[out]     p_colorkey          = Pointer to the struct #aui_color_key
                                      intended to store the color key just
                                      gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the color key of the surface
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At leat one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the color key of the surface
                                      failed for some reasons, as defined in
                                      the enum #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_colorkey_get (

    aui_hdl p_surface_handle,

    aui_color_key *p_colorkey

    );

/**
@brief          Function used to set the clipping rectangle of the surface

@note           This function @a only set the clipping parameters for future
                blit operations.\n
                After calling the function #aui_gfx_surface_blit or
                #aui_gfx_data_blit", the clipping result will be displayed
                on the screen and @a only the clipped area will be updated.

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      p_rect              = Rectangular area for clipping as defined
                                      in the enum #aui_osd_rect
@param[in]      clip_mode           = Clipping mode as defined in the enum
                                      #aui_ge_clip_mode

@return         @b AUI_RTN_SUCCESS  = Setting of the clipping rectangle
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the clipping rectangle
                                      failed for some reasons, as defined in
                                      the enum #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_clip_rect_set (

    aui_hdl p_surface_handle,

    aui_osd_rect *p_rect,

    aui_ge_clip_mode clip_mode

    );

/**
@brief          Function used to clear the clipping rectangle of the surface.

@note           This function is also used to cancel the clipping parameters and
                disable the clip mode\n
                When calling the function #aui_gfx_surface_blit or
                #aui_gfx_data_blit, the clear operation will be triggered

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create

@return         @b AUI_RTN_SUCCESS  = Clearing of the clipping rectangle
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Clearing of the clipping rectangle failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_clip_rect_clear (

    aui_hdl p_surface_handle

    );

/**
@brief          Function used to configure the palette

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      p_pallette_info     = Palette information to be set as defined
                                      in the struct #aui_pallette_info

@return         @b AUI_RTN_SUCCESS  = Configuring of the palette performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Configuring of the palette failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           In all chipset without GE, the palette is supported @a only
                when the surface is a hardware surface
*/

AUI_RTN_CODE aui_gfx_surface_pallette_set (

    aui_hdl p_surface_handle,

    aui_pallette_info *p_pallette_info

    );

/**
@brief          Function used to get the surface palette information

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[out]     p_pallette_info     = Pointer to the struct #aui_pallette_info
                                      intended to store the palette information
                                      just gotten such as:
                                      - Palette color mode
                                      - Palette table address
                                      - Color number look up

@return         @b AUI_RTN_SUCCESS  = Getting of the surface palette information
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Getting of the surface palette information
                                      failed for some reasons, as defined in the
                                      enum #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_pallette_get (

    aui_hdl p_surface_handle,

    aui_pallette_info *p_pallette_info

    );

/**
@brief          Function used to set the surface global alpha value when call
                blit operations

@note           When the blit operation is set as the enum value
                #AUI_GFX_ROP_ALPHA_BLENDING or #AUI_GFX_ROP_BOOL_ALPHA_BLENDIN,
                the surface global alpha value may be used in blit operations
                according to the alpha output mode as defined in the enum
                #aui_alpha_out_mode

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[in]      ul_global_alpha     = Global alpha value to be set which range
                                      is <b> [0, 255] </b>

@return         @b AUI_RTN_SUCCESS  = Setting of the surface global alpha value
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = A least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the surface global alpha value
                                      failed for some reasons, as defined in the
                                      enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> TDS OS </b>
*/
AUI_RTN_CODE aui_gfx_surface_galpha_set (

    aui_hdl p_surface_handle,

    unsigned long ul_global_alpha

    );

/**
@brief          Function used to get the current surface global alpha value
                which is usually used for surface alpha blending operations

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[out]     pul_global_alpha    = Surface global alpha value jsut gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the current surface global
                                      alpha value performed successfully
@return         @b AUI_RTN_EINVAL   = A least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Getting of the current surface global
                                      alpha value failed for some reasons, as
                                      defined in the enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> TDS OS </b>
*/
AUI_RTN_CODE aui_gfx_surface_galpha_get (

    aui_hdl p_surface_handle,

    unsigned long *pul_global_alpha

    );

/**
@brief          Function used to render an image to a surface

@note           In <b> Linux OS </b> platform, this function supports the image
                file format below:
                - @b BMP
                - @b JPEG
                - @b PNG
                - @b GIF

@param[in]      surface_handle      = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create
@param[out]     pc_image_path       = Absolute path of the input image to be
                                      rendered

@return         @b AUI_RTN_SUCCESS  = Rendering of the image to a surface
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in]) is
                                      invalid
@return         @ b Other_Values    = Rendering of the image to a surface
                                      failed for some reasons, as defined in
                                      the enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_gfx_render_image_to_surface (

    aui_hdl surface_handle,

    const char *pc_image_path

    );

/**
@brief          Function used to get the width/height of a specified image

@note           In <b> Linux OS </b> platform, this function supports the image
                file format below:
                - @b BMP
                - @b JPEG
                - @b PNG
                - @b GIF

@param[in]      pc_image_path       = Absolute path of the input image

@param[out]     p_width             = Width of the input image
@param[out]     p_height            = Height of the input iamge

@return         @b AUI_RTN_SUCCESS  = Getting of the width/height of a specified
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Getting of the the width/height of a
                                      specified image failed for some reasons,
                                      as defined in the enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_gfx_get_image_info (

    const char *pc_image_path,

    int *p_width,

    int *p_height

    );

/**
@brief          Function used to decode and output a specified image data

@param[out]     pp_bitmap           = Pointer to a struct #aui_gfx_bitmap_info
                                      intended to store the the output decoding
                                      image parameters

@param[in]      dst_format          = Pixel format of the output bitmap as
                                      defined in the enum #aui_osd_pixel_format
@param[in]      pc_image_buf        = Pointer to the input image data buffer
@param[in]      buf_size            = Size of the input image data

@return         @b AUI_RTN_SUCCESS  = Decoding of the specified image file
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Decoding of the specified image file
                                      failed for some reasons, as defined in
                                      the enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_gfx_image_decode (

    aui_gfx_bitmap_info **pp_bitmap,

    aui_osd_pixel_format dst_format,

    const char *pc_image_buf,

    int buf_size

    );

 /**
@brief          Function used to release a bitmap data

@note           This function usually frees the memory allocated for bitmap.\n
                The bitmap data are usually created by the function #aui_gfx_image_decode

@param[in]      p_bitmap            = Pointer to the struct #aui_gfx_bitmap_info
                                      which collect the bitmap data to be released.
                                      - @b Caution: This pointer is returned by
                                           the function #aui_gfx_image_decode
                                           and cannot be a @b NULL value

@return         @b AUI_RTN_SUCCESS  = Releasing of the bitmap data performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Releasing of the bitmap data failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_gfx_image_release (

    aui_gfx_bitmap_info *p_bitmap

    );

/**
@brief          Function used to lock a surface

@note           Locking a surface @a must be done before starting to read/write
                the surface buffer

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create

@return         @b AUI_RTN_SUCCESS  = Locking of the surface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Locking of the surface failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_gfx_surface_lock (

    aui_hdl p_surface_handle

    );

/**
@brief          Function used to unlock a surface unlocked by the function
                #aui_gfx_surface_lock

@note           Unlocking a surface @a must be done after completing to read/write
                the surface buffer

@param[in]      p_surface_handle    = Surface handle returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create

@return         @b AUI_RTN_SUCCESS  = Unlocking of the surface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Unlocking of the surface failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> Linux OS </b>
*/
AUI_RTN_CODE aui_gfx_surface_unlock (

    aui_hdl p_surface_handle

    );

/**
@brief          Function used to perform a transformation of an affine matrix

@note           This function is used for applying a linear combination of
                - Translation
                - Rotation
                - Scaling and/or shearing

@note           According to the different elements value of an affine matrix,
                the linear combination will be different and thereby the
                displaying will also be different

@param[in]      p_src_surface_handle= Source surface handle which cannot be
                                      @b NULL
@param[in]      p_dst_surface_handle= Destination surface handle which cannot
                                      be @b NULL
@param[in]      p_affine_matrix     = Affine matrix as defined in the struct
                                      #aui_gfx_affine_matrix
@param[in]      p_tran_rect         = Pointer to a struct #aui_blit_rect
                                      which specifies the source and destination
                                      rectangular area

@return         @b AUI_RTN_SUCCESS  = Affine transformation performed successfully

@return         @b AUI_RTN_EINVAL   = At leat one input parameter (i.e. [in])
                                      is invalid

@return         @b Other_Values     = Affine transformation failed for some
                                      reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> TDS OS </b>
*/
AUI_RTN_CODE aui_gfx_surface_affine_matrix_transform (

    aui_hdl p_src_surface_handle,

    aui_hdl p_dst_surface_handle,

    aui_gfx_affine_matrix *p_affine_matrix,

    aui_blit_rect *p_tran_rect

    );

/**
@brief          Function used for filtering with a specified mask

@note           This function makes clipping mask between the source and destination
                surface by the rectangular mask and mask filter parameters.\n
                The mask area will be filtered to show the destination area, then
                the source surface area will be shown on the screen or not

@param[in]      p_src_surface_handle = Source surface handle which cannot be
                                       @b NULL
@param[in]      p_dst_surface_handle = Destination surface handle which cannot
                                       be @b NULL
@param[in]      p_mask_filter        = Pointer to a struct #aui_gfx_mask_filter
                                       which specifies the mask filter
@param[in]      p_tran_rect          = Pointer to a struct #aui_blit_rect which
                                       specifies the rectangular area for blit
                                       operations
                                       - @b Caution: It's better that the mask
                                            filter area and source area are not
                                            beyond the destination data

@return         @b AUI_RTN_SUCCESS   = Filtering performed successfully
@return         @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                       is invalid
@return         @b Other_Values      = Filtering failed for some reasons, as
                                       defined in the enum #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> TDS OS </b>
*/
AUI_RTN_CODE aui_gfx_surface_mask_filter (

    aui_hdl p_src_surface_handle,

    aui_hdl p_dst_surface_handle,

    aui_gfx_mask_filter *p_mask_filter,

    aui_blit_rect *p_tran_rect

    );

/**
@brief          Function used to create the software surface by a RLE
                encoded bitmap buffer

@note           This function is used to decode the RLE bitmap buffer to the
                specified color mode, and create a surface to store the decoded
                bitmap data

@param[out]     p_surface_handle    = Software surface handle which is used for
                                      oeprations such as
                                      - Blit
                                      - Fill
                                      - Capture
                                      - etc.

@param[in]      pixel_format        = Color space mode of the bitmap to be
                                      decoded, as defined in the enum
                                      #aui_osd_pixel_format
@param[in]      ul_width            = Surface width (in @a pixels unit) which
                                      actually is the bitmap width
@param[in]      ul_height           = Surface height (in @a pixels unit) which
                                      actually is the bitmap height
@param[in]      puc_data            = Pointer to the buffere intended to store
                                      the RLE encoded bitmap data which cannot
                                      be @b NULL
@param[in]      ul_data_length      = Bitmap data buffer size which is used
                                      @a only for flushing data memory

@return         @b AUI_RTN_SUCCESS  = Creating of the software surface
                                      peformed successfully
@return         @b AUI_RTN_EINVAL   = A least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Creating of the software surface failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           This function can be used @a only in projects based on
                <b> TDS OS </b>
*/
AUI_RTN_CODE aui_gfx_create_surface_by_rle_pixmap (

    aui_hdl *p_surface_handle,

    aui_osd_pixel_format pixel_format,

    unsigned long ul_width,

    unsigned long ul_height,

    unsigned char *puc_data,

    unsigned long ul_data_length

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

/**
@brief          Function used to configure the scaling of the hardware surface

@warning        User can call this function to decided in which mode the hardware
                surface is displayed on the screen.\n
                When the surface for displaying UI does not fit the screen,
                the application can call this function to scale the surface.\n

                As example,
                - Surface size is <b> 1920 x 1080 </b>
                - TV system is <b> 720 x 576 (PAL) </b>

                then the application needs to scale OSD output to <b> 720 x 576 </b>
                by performing the below configuration of the proper members of
                the struct #aui_scale_param as the parameter @b p_scale_param:
                - Input width   = 1920
                - Input height  = 1080
                - Output width  = 720
                - Output height = 576

                User can get the current TV system by the function @b aui_dis_get

@param[in]      p_hw_surface_handle = Hardware surface handle which is returned
                                      by the function #aui_gfx_hw_surface_create
@param[in]      scale_param         = Hardware scaling value as defined in the
                                      struct #aui_scale_param

@return         @b AUI_RTN_SUCCESS  = Scaling of the hardware surface performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Scaling of the hardware surface failed for
                                      some reasons, as defined in the enum
                                      #aui_gfx_errno_type

@note           This function is @a deprecated then can be ignored, please use
                the function #aui_gfx_layer_scale instead

*/
AUI_RTN_CODE aui_gfx_surface_scale_param_set (

    aui_hdl p_hw_surface_handle,

    aui_scale_param *p_scale_param

    );

/**
@brief          Function used to perform the synchronization of the destination
                surface

@warning        Currently this function is @a reserved to ALi R&D Dept. so user
                can ignore it

@param[in]      dst_surface_handle  = Destination surface handle (which is
                                      actually related to the hardware's surface
                                      handle) returned by the function
                                      #aui_gfx_hw_surface_create or
                                      #aui_gfx_sw_surface_create

@return         @b AUI_RTN_SUCCESS  = Destination surface synchronized successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Destination surface synchronization failed
                                      for some reasons, as defined in the enum
                                      #aui_gfx_errno_type
*/
AUI_RTN_CODE aui_gfx_surface_sync (

    aui_hdl dst_surface_handle

    );

#define aui_osd_color_mode_type_em aui_osd_pixel_format

#define aui_ge_bop_mode_type_em aui_ge_bop_mode

#define aui_ge_clip_mode_type_em aui_ge_clip_mode

#define aui_osd_enhance_em aui_osd_enhance

#define _aui_gfx_mask_filter aui_gfx_mask_filter

#define _aui_gfx_affine_matrix aui_gfx_affine_matrix

#define aui_pallette_color_type_em aui_pallette_color_type

#define aui_gfx_errno_type_em aui_gfx_errno_type

#define aui_gfx_mirror_type_em aui_gfx_mirror_type

#define aui_gfx_rop_operation_type_em aui_gfx_rop_operation_type

#define aui_alpha_out_mode_type_em aui_alpha_out_mode

#define aui_ge_color_key_mode_type_em aui_ge_color_key_mode

#define aui_ge_color_key_match_mode_type_em aui_ge_color_key_match_mode

#define aui_gfx_mask_format_em aui_gfx_mask_format

#define aui_ge_alpha_blend_mode_type_em aui_ge_alpha_blend_mode

#define osd_layer_e aui_osd_layer

#define eAUI_OSD_LAYER_GMA1 AUI_OSD_LAYER_GMA0

#define eAUI_OSD_LAYER_GMA2 AUI_OSD_LAYER_GMA1

#define aui_st_coordinate_ aui_coordinate

#define aui_st_coordinate aui_coordinate

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


