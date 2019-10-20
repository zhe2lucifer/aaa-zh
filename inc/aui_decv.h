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
Current ALi author: Wendy.He
Last update:        2017.05.08
-->

@file  aui_decv.h

@brief Video Decoder (DECV) Module

       <b> Video Decoder (DECV) Module </b> is used to process <b> ES data </b>
       from De-Multiplexing (DMX) Module or memory.\n
       When the data source come from <b> DMX Module </b>, that one will
       - Receive TS packets from
       - Transport Stream Switch Interface (TSI) Module
         (which gets data from either NET Interface (NIM) Module or
         Transport Stream Generator (TSG) Module)
       - Local disk by injecting directly
       - Parse PES data to ES data
       - Send the ES data to DECV Module

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Video Decoder (DECV) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_DECV_H

#define _AUI_DECV_H

/**************************Included Header File List***************************/

#include "aui_common.h"

#include "aui_dis.h"

/******************************Global Macro List*******************************/

/**
Macro to denote that presently the structs contained in this header file are
designed to support up to
- One (1) Video Decoder on TDS System
- Two (2) Video Decoder on Linux System
*/
#define AUI_DECV_DEV_CNT_MAX (2)

/*******************************Global Type List*******************************/

/**
Function pointer to specify the type of callback function registered with the
function #aui_decv_reg_time_code_callback to get video time code notifications.\n
The input parameter is @b u_param1, more information about it can be found in the
enum #aui_decv_time_code_report_type.

@note This function pointer can be used @a only in projects based on <b> TDS
      OS </b>
*/
typedef void (*aui_time_code_cbfunc) (

    unsigned long ul_param1

    );

/**
Function pointer to specify the type of callback function filled in the struct
#aui_decv_callback_node and registered for various event as present in the enum
#aui_en_decv_callback_type.\n
The callback function receives @b p_user_hld as user parameter which is given
when registering, please refer to the struct #aui_decv_callback_node to get
more information.\n
The input parameters @b para1 and @b para2 are used in different callback
notification, more information about them can be found in the enum
#aui_en_decv_callback_type.
*/
typedef void (*aui_decv_callback) (

    void *pv_user_hld,

    unsigned int para1,

    unsigned int para2

    );

/**
Function pointer to specify the type of callback function registered with the
functions #aui_decv_init and #aui_decv_de_init, and got called during <b>
Init/De-Init stage </b> of DECV Module.
*/
typedef void (*aui_func_decv_init) (

    void

    );

/**
Enum to specify the channel change mode. User can opt for selecting what should
appear during channel change, it can be
- Either a <b> Free Frame </b>
- Or <b> Black Screen </b>

The desired mode can be set by
 - Either the function #aui_decv_open with the input parameter #aui_attr_decv
 - Or the function #aui_decv_set with the input parameter #AUI_DECV_SET_CHGMODE
*/
typedef enum aui_en_decv_chg_mode {

    /**
    In this mode a black screen will appear during the channel transition.
    */
    AUI_DECV_CHG_BLACK,

    /**
    In this mode the last frame of the program of the previous channel will be
    kept as still during the channel transition.
    */
    AUI_DECV_CHG_STILL,

    /**
    This mode is similar like #AUI_CHG_STILL mode, but is intended @a only for
    <b> OTV PFM </b> (Play From Memory).

    @note This mode can be used @a only in projects based on <b> TDS OS </b>.
    */
    AUI_DECV_CHG_OTV_PFM

} aui_decv_chg_mode;

/**
Enum to specify all possible error type returned by a callback function,
please refer to callback type #AUI_DECV_CB_ERROR.
*/
typedef enum aui_decv_error_type {

    /**
    This error type indicates that the video input data buffer of the video
    decoder is underflow (i.e. the remaining data in the buffer is less than
    one frame), then the <b> Display Engine (DIS) Module </b> cannot get any
    frame from the video decoder to play.\n
    If this error type lasts up to <b> 500 ms </b>, the video status will
    change into the value #AUI_DECV_STATE_NODATA (please refer to the enum
    #aui_decv_state_type), which can be retrieved by the callback function
    #AUI_DECV_CB_STATE_CHANGE then this error type will not be returned any
    longer.
    */
    AUI_DECV_ERROR_NO_DATA = 0x1,

    /**
    This error type indicates that the hardware related to the video decoder
    has failed to decode a frame, then that frame will be lost but the video
    flow will not be stopped (i.e. the video just lost a frame).
    */
    AUI_DECV_ERROR_HARDWARE = 0x2,

    /**
    This error type indicates that a frame lost the synchronization with
    - Either the audio
    - Or the <b> PCR (Program Clock Reference) </b>

    and it will be held until it can be synchronized again.\n
    This error type will be kept raising until the synchronization of that frame
    is successfully achieved. However, when this error type occurs the audio and
    video flow will not be stopped.

    @note  Supposed that the video is synchronized with the audio or PCR, this
           error type might occur during playtime.
    */
    AUI_DECV_ERROR_SYNC = 0x4,

    /**
    This error type indicates that a frame dropped because it lost the
    synchronization
    - Either with the audio
    - Or with the <b> PCR (Program Clock Reference) </b>

    This error type will continue to occur (i.e. other consecutive and contiguous
    frames may continue to be dropped cause synchronization problems) until the
    synchronization of a subsequent frame is successfully achieved. However, when
    this error type occur the audio and video flow will not be stopped.

    @note  Supposed that the video is synchronized with the audio or PCR, this
           error type might occur during playtime.
    */
    AUI_DECV_ERROR_FRAME_DROP = 0x8

} aui_decv_error_type;

/**
Enum to specify all possible video status returned by a callback function, please
refer to callback type #AUI_DECV_CB_STATE_CHANGE.
*/
typedef enum aui_decv_state_type {

    /**
    This video status indicates that video decoder is not getting data to decode
    by at least <b> 500 ms </b>, then the <b> Display Engine (DIS) Module </b>
    cannot get any frame from the video decoder to play.\n
    In particular, since the video decoder started to wait for new data, the last
    showed frame will be frozen on the screen and this video status will occur
    from the time <b> t=500 ms </b>.Then this video status will last until the
    video decoder starts getting data to decode, and the video status will go
    into the decoding status (i.e. the video status will change into the value
    #AUI_DECV_STATE_DECODING).
    */
    AUI_DECV_STATE_NODATA = 0x1,

    /**
    This video status indicates that video decoder is decoding then the Display
    Engine can get frame from the video decoder to play.

    @pre Precondition to have this video status is that the video decoder is
         not getting data to decode, i.e. the video decoder will go into this video
         status from the video status with the value #AUI_DECV_STATE_NODATA.
    */
    AUI_DECV_STATE_DECODING = 0x2

} aui_decv_state_type;

/**
Enum to specify the different callback types for all possible video events.\n
The @a dummy identifiers @b para1 and @b para2 appearing in the description of
each callback type are the return value of callback function, and can represent:
- Either the input parameters @b param1 and @b param2 of the callback function
  @b common_callback (which is no longer supported then is @a deprecated)
- Or the input parameter @b para1 and @b para2 of the callback function
  #aui_decv_callback

@note  The value @b NULL appearing for those parameters in some callback types
       just means no need to care about them, please refer to the struct
       #aui_decv_callback_node
*/
typedef enum aui_en_decv_callback_type {

    /**
    This callback type is for the event
    <b><em> "First frame of a program showed on the screen" </em></b>.\n
    The callback function will be called once while changing the channel, and
    the input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL
    */
    AUI_DECV_CB_FIRST_SHOWED,

    /**
    This callback type is for the event
    <b><em> "Switching mode done after setting display mode" </em></b>.\n
    The callback function will be called once after switching mode done, and
    the input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL

    @note  This callback type is intended @a only for TS playback <b> right now
           
    @note  This callback type is intended @a only for projects based on <b> Linux
           OS </b>
    */
    AUI_DECV_CB_MODE_SWITCH_OK,

    /**
    @warning  This callback type is no longer supported then is @a deprecated
    */
    AUI_DECV_CB_BACKWARD_RESTART_GOP,

    /**
    This callback type is for the event
    <b><em> "Head of the first I-frame parsed" </em></b>.\n
    The callback function will be called once while changing the channel, and
    the input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL
    */
    AUI_DECV_CB_FIRST_HEAD_PARSED,

    /**
    This callback type is for the event
    <b><em> "First I-frame decoded" </em></b>.\n
    The callback function will be called once while changing the channel, and
    the input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL
    */
    AUI_DECV_CB_FIRST_I_FRAME_DECODED,

    /**
    This callback type is for the event
    <b><em> "User data for closed captions in video stream parsed" </em></b>.\n
    The input parameters @b para1 and @b para2 of the callback function which
    belong to this callback type will take the following values:
    - @b para1 = The length of the memory block where the user data are stored
    - @b para2 = Pointer to the first memory location from which the user data
                 are stored
    */
    AUI_DECV_CB_USER_DATA_PARSED,

    /**
    The callback function that belong to this callback type will return all
    Video information of the pictures on the screen changed during playtime, e.g.
    - Height and/or Width
    - Frame Rate
    - Active Format Description (AFD)
    - Sample Aspect Ratio

    and the input parameters @b para1 and @b para2 will get different values
    depends of the underlying operating system as below:
    - For projects based on <b> Linux OS </b>:
      - @b para1 = Pointer to the struct #aui_decv_info_cb,
                   i.e. new (changed) video information
      - @b para2 = NULL
    - For projects based on <b> TDS OS </b>:
      - @b para1 = Flag to indicate that the video information changed (the integer
                   values and related meaning for this flag are defined in the enum
                   #aui_decv_change_flag)
      - @b para2 = Pointer to the new information struct #aui_decv_change_info
    */
    AUI_DECV_CB_INFO_CHANGED,

    /**
    This callback type is for the event
    <b><em> "Error occurred" </em></b>.\n
    The callback function will return an error type as defined in the enum
    #aui_decv_error_type during playtime. The input parameters @b para1 and @b para2
    will get the following values:
    - @b para1 = Flag to indicate the error type occurred (the integer values
                 and related
                 meaning of this flag are defined in the enum #aui_decv_error_type)
    - @b para2 = NULL
    */
    AUI_DECV_CB_ERROR,

    /**
    This callback type is for the event
    <b><em> "Video decoding process started" </em></b>.\n
    The callback function will be called when the video decoding process started.
    The input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL

    @note  This callback type is intended @a only for projects based on <b> TDS
           OS </b>
    */
    AUI_DECV_CB_MONITOR_START,

    /**
    This callback type is for the event
    <b><em> "Video decoding process stopped" </em></b>.\n

    @pre Precondition of this event is that the video decoding process already
         started

    The callback function will be called when the video decoding process stopped,
    and the input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL

    @note This callback type is intend @a only for projects based on <b> TDS OS </b>
    */
    AUI_DECV_CB_MONITOR_STOP,

    /**
    This callback type is for the event
    <b><em> "Video frames decoding" </em></b>\n
    The callback function will used whenever <b> 50 frames </b> are decoded,
    and the input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL

    @note This callback type is intended @a only for projects based on <b> TDS
          OS </b>
    */
    AUI_DECV_CB_MONITOR_STATE_CHANGE,

    /**
    This callback type is for the event
    <b><em> "Video state changed" </em></b>.\n
    The callback function is called when the state of video decoder changed from
    @b Decoding to <b> No data Status </b> or @a vice-versa. The input parameters
    @b para1 and @b para2 will get the following values:
    - @b para1 = Flag to indicate the current state (the integer values and related
                 meaning of this flag are defined in the enum #aui_decv_state_type)
    - @b para2 = NULL
    */
    AUI_DECV_CB_STATE_CHANGE,

    /**
    This callback type is for the event
    <b><em> "Group of Pictures" </em></b>\n
    The callback function is called when the group of pictures starts to be
    decoded.\n
    The input parameters @b para1 and @b para2 will get the following values:
    - @b para1 = NULL
    - @b para2 = NULL

    @note This callback type is intended for mornitoring I-Frame events
    */
    AUI_DECV_CB_MONITOR_GOP,

    /**
    This enum value indicates the total number of maximum callback types available
    */
    AUI_DECV_CB_MAX

} aui_decv_callback_type;

/**
Enum to specify different modes to manage the
<b> VBV (Video Buffering Verifier) buffer overflow </b>.
The desired mode can be set by
- Either by the function #aui_decv_open with the input parameter #aui_attr_decv
- Or by the function #aui_decv_set with the input parameter #AUI_DECV_SET_VBV_BUF_MODE
*/
typedef enum aui_decv_vbv_buf_mode {

    /**
    In this mode, when the VBV buffer overflow occurs it @a needs to be reset.\n
    This mode should be selected to make sure that the <b> De-Multiplexing (DMX)
    Module </b>
    will not be blocked when the VBV buffer overflow occurs.

    @note  This mode @a needs to be set for <b> Tuner Live Play </b>.
    */
    AUI_DECV_VBV_BUF_OVERFLOW_RST_MODE = 0,

    /**
    In this mode, when the VBV buffer overflows occurs, it <em> does not need </em>
    to be reset.\n
    This mode should be selected to make sure that DMX Module will wait till video
    decoder extracts data from the VBV buffer before to put new data into it,
    in order to prevent data loss.

    @note  This mode @a needs to be set for <b> TS Playback </b>
    */
    AUI_DECV_VBV_BUF_BLOCK_FULL_MODE = 1

} aui_decv_vbv_buf_mode;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Video Decoder (DECV) </b> Module to notify the video
        information changed
        </div> @endhtmlonly

        Struct used to notify the video information changed using callback notification
        #AUI_DECV_CB_INFO_CHANGED. It consists of a @a flag and some @a fields, where:
        - The @a flag denotes which fields (i.e. video information) changed
        - The @a fields contain the video information changed as per the indication
          of the flag

@note @a Only in projects based on <b> Linux OS </b>, the fields changed as per
      the indication of the flag take a suitable value, instead the others will
      take the value <b> zero (0) </b> as default.
*/
typedef struct aui_decv_info_cb {

    /**
    Member as the flag to denotes which fields containing the
    <b> Changed video information </b>.
    The different integer values of this flag are listed below:
    - @b 0 = Dimensions of the video pictures changed
             (i.e. the field #pic_width and/or #pic_height of this struct changed)
    - @b 1 = Frame rate of the video pictures changed
             (i.e. the field #frame_rate of this struct changed, where the unit
             is @a fps*1000)
    - @b 2 = Active Format Description (AFD) of the video picture changed
             ( i.e. the field #active_format of this struct changed)
    - @b 3 = Sample Aspect Ratio (SAR) of the video pictures changed
             (i.e. the field #sar_width and/or #sar_height of this struct changed)
    */
    unsigned int flag;

    /**
    Member as the field related to the
    <b> Width of the video pictures </b>.
    The value of this field makes sense then is suitable when:
    - Either the flag takes the value <b> zero (0) </b>
    - Or this field takes the value <b> zero (0) </b> as default

    @note  This field is paired with the field #pic_height
    */
    unsigned int pic_width;

    /**
    Member as the field related to the
    <b> Height of the video pictures </b>.
    The value of this field makes sense then is suitable when:
    - Either the flag takes the value <b> zero (0) </b>
    - Or this field takes the value <b> zero (0) </b> as default

    @note  This field is paired with the field #pic_width
    */
    unsigned int pic_height;

    /**
    Member as the field related to the
    <b> Frame Rate of the video pictures </b>.
    The value of this field makes sense then is suitable when:
    - Either the flag takes the value <b> one (1) </b>
    - Or this field takes the value <b> zero (0) </b> as default

    @note The unit of this field is @a fps*1000
    */
    unsigned int frame_rate;

    /**
    Member as the field related to the
    <b> Active Format Description (AFD) of the video pictures </b>.
    The value of this field makes sense then is suitable when:
    - Either the flag takes the value <b> two (2) </b>
    - Or this field takes the value <b> zero (0) </b> as default
    */
    unsigned char active_format;

    /**
    Member as the field related to the
    <b> Sample Aspect Ratio Width of the video pictures </b>.
    The value of this field makes sense then is suitable when:
    - Either the flag takes the value <b> three (3) </b>
    - Or this field takes tha value <b> zero (0) </b> as default
    */
    unsigned int sar_width;

    /**
    Member as the field related to the
    <b> Sample Aspect Ratio Height of the video pictures </b>.
    The value of this field makes sense then is suitable when:
    - Either the flag takes the value <b> three (3) </b>
    - Or this field takes the value <b> zero (0) </b> as default
    */
    unsigned int sar_height;

} aui_decv_info_cb;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Video Decoder (DECV) Module </b> to specify
        <em> Callback Type, Callback Function, User Data </em> to be used
        </div> @endhtmlonly

        Struct to specify:
        - A member as <em> Callback Type </em>
        - A member as <em> Callback Function With/Without User Data </em>
        - A member a pointer to <em> User Data </em> for the callback function
          (when expected)

        This struct is used by the function #aui_decv_set with the input
        parameter #AUI_DECV_SET_REG_CALLBACK
*/
typedef struct aui_decv_callback_node {

    /**
    Member as <em> Callback Type </em>
    (i.e. as the enum #aui_en_decv_callback_type)
    */
    aui_decv_callback_type  type;

    /**
    Member as <em> Callback Function with user data </em>
    (i.e. as the callback function #aui_decv_callback)

    @note It is used as default in projects based on <b> Linux OS </b>
    */
    aui_decv_callback callback;

    /**
    Member as pointer to <em> User Data </em> for the callback function
    with the identifier #callback
    */
    void *puser_data;

} aui_decv_callback_node;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Video Decoder (DECV) Module </b> to specify the
       attributes available to be configured for DECV Module
       </div> @endhtmlonly

       Struct to specify the attributes available to be configured for DECV
       Module by the function #aui_decv_open.
*/
typedef struct aui_attr_decv {

    /**
    Member as @a Index which integer values (from the value <b> zero (0) </b>)
    refer to different supported DECV devices.

    @note At the moment @a only one (1) DECV device is supported, then that
          @a Index can take @a only the value <b> zero (0) </b>.
    */
    unsigned char uc_dev_idx;

    /**
    Member as <em> Change Mode </em> (i.e. as the enum #aui_en_decv_chg_mode)
    used for the pre-configuration of the change mode when the video decoder
    opening.
    */
    aui_decv_chg_mode en_chg_mode;

    /**
    Member as <em> VBV Buffer Mode </em> (i.e. as the enum #aui_decv_vbv_buf_mode)
    used for setting the mode to manage the VBV buffer overflow after the video
    decoder opening.
    */
    aui_decv_vbv_buf_mode en_vbv_buf_mode;

    /**
    Member as an array of struct as defined in #aui_decv_callback_node with size
    #AUI_DECV_CB_MAX, as defined in the enum #aui_en_decv_callback_type.
    This array is used to store callback info (i.e. <em> Callback Type </em>
    and <em> Callback Function </em>) which will be registered after the
    video decoder opening.
    */
    aui_decv_callback_node callback_nodes[AUI_DECV_CB_MAX];

    /**
    Member as <em> display layer </em> (i.e. as the enum #aui_dis_layer)
    used for the pre-configuration of the display layer when the video decoder
    opening.
    */
    aui_dis_layer dis_layer;

} aui_attr_decv, *aui_p_attr_decv;

/**
Enum to specify different available video frame types for decoding.
The desired video frame type can be configured by calling the function
#aui_decv_set with the input parameter #AUI_DECV_SET_FRAME_TYPE.

@note Those video frame type are available @a only for projects based on <b>
      Linux OS </b>
**/
typedef enum aui_decv_frame_type {

    /**
    With this video frame type, the video decoder will decode all kind of frames
    (e.g. <b> I-frame, P-frame, B-frame </b>) of the video stream.
    */
    AUI_DECV_FRAME_TYPE_ALL,

    /**
    With this video frame type, the video decoder will decode @a only the @b
    I-frames

    @note @b 1. If all the video frames in the stream are @b I-frames, this video
                frame type needs to be set.

    @note @b 2. This video frame type can be chosen also by the video format
                @b MPEG2 and @b H.264
    */
    AUI_DECV_FRAME_TYPE_I

} aui_decv_frame_type;

/**
Enum to specify miscellaneous settings to be set for a video decoder, and is
used by the function #aui_decv_set to perform a specific setting where:
- The second parameter @b ul_item takes the item related to the specific setting
  to perform
- The third parameter @b pv_param takes the pointer as per the description of the
  specific setting to perform
*/
typedef enum aui_en_decv_item_set {

    /**
    This value is to set the <b> channel change mode </b>.\n
    The parameter @b pv_param takes the pointer to a channel change mode defined
    in the enum #aui_en_decv_chg_mode.
    */
    AUI_DECV_SET_CHGMODE,

    /**
    This value is to register a <b> callback function </b> for a video event.\n
    The parameter @b pv_param takes the pointer to a callback node information
    defined in the struct #aui_decv_callback_node
    */
    AUI_DECV_SET_REG_CALLBACK,

    /**
    This value is to de-register a <b> callback function </b> for a video event.\n
    The parameter @b pv_param takes the pointer to a callback type defined in the
    enum #aui_en_decv_callback_type.

    */
    AUI_DECV_SET_UNREG_CALLBACK,

    /**
    This value is to set the <b> delay time </b> for every video frame to be
    shown on the screen.\n
    This setting is intended to solve the synchronization problems of video
    frames with audio by the needed delay time (in @a msec unit) for the frames
    to be shown on the screen.\n
    The parameter @b pv_param takes the pointer to an integer variable containing
    the needed delay time.

    @note This setting is performed @a only in projects based on <b> TDS OS </b>.
    */
    AUI_DECV_SYNC_DELAY_TIME,

    /**
    This value is to set the <b> color bar </b> to perform factory test.\n
    The parameter @b pv_param takes:
    - The pointer to the integer value @b AVC_VBV_ADDR defined in the operating
      system define files (@a only in projects based on <b> TDS OS</b>)
    - The pointer to an integer variable with the value <b> zero (0) </b>
      (@a only in projects based on <b> Linux OS</b>)
    */
    AUI_DECV_SET_COLOR_BAR,

    /**
    @warning  The setting related to this item is no longer supported then is
              @a deprecated
    **/
    AUI_DECV_DROP_FREERUN_PIC,

    /**
    This value is to set the mode to manage the <b> VBV buffer overflow </b>.\n
    The parameter @b pv_param takes the pointer to a VBV buffer overflow mode
    defined in the enum #aui_en_vbv_buf_mode.

    @note @b 1. This setting is the input parameter of the function #aui_decv_set,
                which needs to be called after calling the function #aui_decv_start.\n
                If this setting is performed before calling the function #aui_decv_start,
                that setting will not be effect then need to set again.\n
                That's a special variant of the general working flow mentioned
                in ALi Document <b><em> "ALi_AUI_API_Porting_Guide.pdf" </em></b>

    @note @b 2. This setting is performed @a only in project based on <b> Linux
                OS </b>
    */
    AUI_DECV_SET_VBV_BUF_MODE,

    /**
    This value is to set the synchronization <b> free-run mode </b> for the
    first @a I-frame of the video stream.\n
    The parameter @b pv_param takes the pointer of an integer variable which
    value can be:
    - @b 0 = Video will try to sync with PCR or audio at the first @a I-frame
    - @b 1 = The first @a I-frame will free-run and video will try to sync with
             PCR or audio at the second frame after the first @a I-frame
    */
    AUI_DECV_SET_FIRST_I_FREERUN,

    /**
    This value is to set/unset the support to play the <b> variable resolution
    </b> of the video pictures properly.\n
    The parameter @b pv_param takes the pointer to an integer variable which
    value can be:
    - @b 0 = The video stream cannot be played properly cause of the change of
             width and/or height of the video pictures (i.e. playing the variable
             resolution is not supported)
    - @b 1 = The video stream can be played properly even the change of width
             and/or height of the video pictures (i.e. playing the variable
             resolution is supported)

    @note @b 1. If user does not perform any setting, by default playing the
                variable resolution is not supported.
    @note @b 2. When the video decoder is closed, this setting will go into the
                state not supported again
    @note @b 3. This setting is @a only for projects based on <b> Linux OS </b>.
                For project based on <b> TDS OS </b>, please refer to the function
                @b aui_decv_io_sameless_switch (which is no longer supported then
                is @a deprecated)
    @note @b 4. This setting is @a only supported by the video format @b H.264
    */
    AUI_DECV_SET_VARIABLE_RESOLUTION,

    /**
    This value is to set the <b> video frame type </b> to be decoded.\n
    The parameter @b pv_param takes the pointer to a video frame type defined
    in the enum #aui_decv_frame_type.

    @note @b 1. This setting is the input parameter of the function #aui_decv_set,
                which needs to be called after calling the function #aui_decv_start.\n
                If this setting is performed before calling the function #aui_decv_start,
                that setting will not be effect then need to set again.\n
                That's a special variant of the general working flow mentioned in
                ALi Document <b><em> "ALi_AUI_API_Porting_Guide.pdf" </em></b>
    @note @b 2. This setting is @a only supported by the video format @b MPEG2 and @b H.264
    */
    AUI_DECV_SET_FRAME_TYPE,

    /**
    This value is to set/unset the support to perform the <b> video synchronization </b>
    with audio or PCR (Program Clock Reference) properly.\n
    The parameter @b pv_param takes the pointer to an integer variable which integer
    value (N) can be:
    - @b N>0 = There are <b> N free run frames </b> between <b> two (2) hold frames </b>
               (the first video frame of the stream is always a <b> free run frame </b>).\n
               In general, @b N=2 or @b N=3 should be enough to achieve the needed video
               synchronization.
    - @b N=0 = This setting is disabled then the video synchronization can be
               achieve by others ways (e.g. by the setting related to the item
               #AUI_DECV_SET_FIRST_I_FREERUN defined in the enum #aui_en_decv_item_set)

    @note @b 1. If @b N is getting larger, the video synchronization will take
                longer time to be achieved but the video will be shown on the
                screen more smoothly.

    @note @b 2. This setting is @a only for projects based on <b> Linux OS </b>
    */
    AUI_DECV_SET_AVSYNC_SMOOTH,

    /**
    This value is to handle the decoding when an hardware decoding error occured,
    in particular:

    - @b 0 = Drop the sequence of the video frames (default value)
    - @b 1 = Continue decoding without any handling
    */
    AUI_DECV_SET_CONTINUE_ON_DECODER_ERROR,

    /**
    This value specifies the total number of items available in this enum
    */
    AUI_DECV_SET_LAST

} aui_decv_item_set;

/**
Enum to specify miscellaneous information to be gotten from the video decoder,
and is used by the function #aui_decv_get to get a specific information where:
- The second parameter @b ul_item takes the item related to the specific
  information to get
- The third parameter @b pv_param takes the pointer as per the description of
  the specific information to get
*/
typedef enum aui_en_decv_item_get {

    /**
    This value is to get the whole collection of video information available.\n
    The parameter @b pv_param takes the pointer to the video information status
    defined in the struct #aui_decv_info

    @note Getting video information from the video decoder with this item is
          supported only when using the video format:
          - @b MPEG2
          - @b H.264
          - @b AVS (@a only in project based on <b> Linux OS </b>)

          and this item can be applied when playing <b> TS Stream </b>.\n
          About how to get video information with <b> AV-Inject Module </b>
          (@a only for projects based on <b> Linux OS</b>), please refer to the
          related header file
    */
    AUI_DECV_GET_INFO,

    /**
    This value indicates the total number of items available in this enum
    */
    AUI_DECV_GET_LAST

} aui_decv_item_get;

/**
Enum to specify different video decoder status. To know the video decoder status,
please use the function #aui_decv_get with the input parameter #AUI_DECV_GET_INFO.
*/
typedef enum aui_decv_status {

    /**
    This status indicates the video decoder is ready to decode
    */
    AUI_DECV_ATTACHED,

    /**
    This status indicates the video decoder is into the decoding stage.
    The video decoder will go into this status after using the function
    #aui_decv_start to start the decoding stage.
    */
    AUI_DECV_DECODING,

    /**
    This status indicates the video decoder has been stopped by using the function
    #aui_decv_stop. Depending of the channel change mode as defined in the enum
    #aui_en_decv_chg_mode, in this status:
    - Either the screen can show as black
    - Or the screen can freeze the last frame shown before this status occurred.

    After the video decoder stopped, the decoding stage can start again by using
    the function #aui_decv_start
    */
    AUI_DECV_STOPPED,

    /**
    This status indicates the video decoder has been paused by using the function
    #aui_decv_pause. In this status, the last frame before this status occurred
    will be freeze on the screen.\n
    To resume the video decoding stage, just use the function #aui_decv_resume.
    When resuming, two (2) situation can occur:
    - In case of <b> TS Live Stream </b>, the video data belonging to the interval
      time <b> [pause,resume] </b>  <em> will be lost </em> then the video data after
      resume occurred will start to be decoded.
    - In case of <b> Local TS Stream </b>, the video data in the interval time
      <b> [pause,resume] </b> <em> will not be lost </em> then the video data after
      pause occurred will start to be decoded.
    */
    AUI_DECV_PAUSED

} aui_decv_status;

/**
Enum to specify different modes of showing the video frame on the screen.
To configure the desired mode, please use the function #aui_dis_mode_set.
To know which mode is running, please use the function #aui_decv_get with the
input parameter #AUI_DECV_GET_INFO.
*/
typedef enum aui_decv_output_mode {

    /**
    In this mode, the video will be shown on full screen.
    */
    AUI_DECV_MP_MODE,

    /**
    @attention This mode is @a deprecated, so user can ignore this value
    */
    AUI_DECV_PIP_MODE,

    /**
    In this mode, the video will be shown in one small window on the screen
    as preview. About the setting of that small window (e.g. width, height,
    position on the screen), please refer to the function #aui_dis_mode_set.
    */
    AUI_DECV_PREVIEW_MODE,

} aui_decv_output_mode;

/**
Enum to specify different <b> Display Aspect Ratio (DAR) </b> of the video stream.
To know that information, please use the function #aui_decv_get with the input
parameter #AUI_DECV_GET_INFO.
*/
typedef enum aui_decv_aspect_ratio {

    /**
    @warning    The information related to this value is no longer supported
                then is @a deprecated.
    */
    AUI_DECV_DAR_FORBIDDEN = 0,

    /**
    This value refers to the <b> Sample Aspect Ratio (SAR) </b> which has
    effect on the value of the @b DAR.
    */
    AUI_DECV_SAR,

    /**
    This value indicates the value of the @b DAR is <b> 4:3 </b>
    */
    AUI_DECV_DAR_4_3,

    /**
    This value indicates the value of the @b DAR is <b> 16:9 </b>
    */
    AUI_DECV_DAR_16_9,

    /**
    This value indicates the value of the @b DAR is <b> 2.21:1 </b>
    */
    AUI_DECV_DAR_221_1

} aui_decv_aspect_ratio;

/**
Enum to specify different playback speeds of the video frame. To set the desired
playback speed, please use the function #aui_decv_playmode_set.

@note @b 1. Usage of that function is supported @a only in projects based on
            <b> Linux OS </b>. For projects based on <b> TDS OS </b>, needs to
            use another function as per reference of the header file related to
            <b> Personal Video Recoder (PVR) Module </b>.\n
            To know what is the current playback speed, please use the function
            #aui_decv_get with the input parameter #AUI_DECV_GET_INFO.

@note @b 2. The above ways to set and know the playback speed are supported @a only
            by the video format @b MPEG2 and @b H.264 for <b> TS Stream </b>
@note @b 3. Cause the video decoder cannot play properly the video frames if the
            playback speed is eight times faster (i.e. @b x8) than the normal speed,
            user needs to control the writing speed of video data by:
            - Either developing an own function
            - Or using the function #aui_av_trickmode_set
*/
typedef enum aui_playback_speed {

    /**
    This value indicates the <b> playback speed </b> is half times slower
    (i.e. @b x1/2) than the normal value.

    @note The backward direction is not supported.
    */
    AUI_PLAYBACK_SPEED_1_2,

    /**
    This value indicates the <b> playback speed </b> is four times slower
    (i.e. @b x1/4) than the normal speed value.

    @note The backward direction is not supported.
    */
    AUI_PLAYBACK_SPEED_1_4,

    /**
    This value indicates the <b> playback speed </b> is eight times slower
    (i.e. @b x1/8) than the normal speed value.

    @note The backward direction is not supported.
    */
    AUI_PLAYBACK_SPEED_1_8,

    /**
    This value indicates the playback is performed <b> frame by frame </b>
    according to the repeating use of the function #aui_decv_playmode_set.

    @note The backward direction is not supported.
    */
    AUI_PLAYBACK_SPEED_STEP,

    /**
    This value indicates the <b> playback speed </b> is @b normal.

    @note The backward direction is not supported.
    */
    AUI_PLAYBACK_SPEED_1,

    /**
    This value indicates the <b> playback speed </b> is two times faster
    (i.e. @b x2) than the normal speed value.
    */
    AUI_PLAYBACK_SPEED_2,

    /**
    This value indicates the <b> playback speed </b> is four times faster
    (i.e. @b x4) than the normal speed
    */
    AUI_PLAYBACK_SPEED_4,

    /**
    This value indicates the <b> playback speed </b> is eight times faster
    (i.e. @b x8) than the normal speed
    */
    AUI_PLAYBACK_SPEED_8,

    /**
    This value indicates the <b> playback speed </b> is sixteen times faster
    (i.e. @b x16) than the normal speed.

    @note  Please refer to the notes in the brief section of this enum.
    */
    AUI_PLAYBACK_SPEED_16,

    /**
    This value indicates the <b> playback speed </b> is thirty-two times faster
    (i.e. @b x32) than the normal speed.

    @note  Please refer to the notes in the brief section of this enum.
    */
    AUI_PLAYBACK_SPEED_32,

    /**
    This value indicates the <b> playback speed </b> is sixty-four times faster
    (i.e. @b x64) than the normal speed.

    @note  Please refer to the notes in the brief section of this enum.
    */
    AUI_PLAYBACK_SPEED_64,

    /**
    This value indicates the <b> playback speed </b> is one hundred twenty-eight
    times faster (i.e. @b x128) than the normal speed.
    */
    AUI_PLAYBACK_SPEED_128

} aui_playback_speed;

/**
Enum to specify different playback direction of the video frame. To set the
desired playback direction, please use the function #aui_decv_playmode_set.

@note @b 1. Usage of that function is supported @a only in projects based on
            <b> Linux OS </b>.For projects based on <b> TDS OS </b>, instead,
            @a needs to use another function as per reference to the header
            file related to <b> Personal Video Recorder (PVR) Module </b>.\n
            To find out the current playback direction, please use the function
            #aui_decv_get with the input parameter #AUI_DECV_GET_INFO.

@note @b 2. The above ways to set and know the playback direction are supported
            @a only by the video format @b MPEG2 and @b H.264 for <b> TS Stream </b>.
*/
typedef enum aui_playback_direction {

    /**
    This value indicates the playback direction is forward.
    */
    AUI_PLAYBACK_FORWARD,

    /**
    This value indicates the playback direction is backward.

    @note @a Only fast backward is supported.
    */
    AUI_PLAYBACK_BACKWARD

} aui_playback_direction;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Video Decoder (DECV) Module </b> to specify a miscellaneous
        of video information available to get
        </div> @endhtmlonly

        Struct to specify a list of miscellaneous information about the video.
        To get the whole collection of the video information available, please use
        the function #aui_decv_get with the input parameter #AUI_DECV_GET_INFO.

@note Usage of that function is supported @a only with the video format:
      - @b MPEG2
      - @b H.264
      - @b AVS (@a only in project based on <b> Linux OS</b>)

      for <b> TS Stream </b>.
*/
typedef struct aui_decv_status_info {

    /**
    Member as <em> Video Decoder Status </em> (i.e. as the enum #aui_decv_status)
    to contain information related to the <b> current status of the video decoder </b>
    */
    aui_decv_status u_cur_status;

    /**
    Member as a @a Flag to denote if the <b> first picture of the new video
    channel </b> has been shown on the screen, in particular:
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned int    u_first_pic_showed;

    /**
    Member as a @a Flag to denote if the header of the <b> first frame of the
    new channel </b> has been gotten, in particular:
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned int    b_first_header_got;

    /**
    Member to contain the @b width of the video stream
    */
    unsigned short  pic_width;

    /**
    Member to contain the @b height of the video stream
    */
    unsigned short  pic_height;

    /**
    Member as a @a Flag to denote the <b> internal status of the DECV Module </b>
    (e.g. first picture showed).

    @attention This member is @a reserved to ALi R&D Dept. then users can
               @a ignore it

    @note Usage of this member is relevant for debugging @a only in project based
          on <b> TDS OS </b>.
    */
    unsigned short  status_flag;

    /**
    Member to contain the <b> reading position of the VBV buffer </b>.

    @attention This member is @a reserved to ALi R&D Dept. then user can @a ignore it

    @note Usage of this member is relevant for debugging @a only in project based on
          <b> TDS OS </b>.
    */
    unsigned short  read_p_offset;

    /**
    Member to contain the <b> writing position of the VBV buffer </b>.

    @warning  This member is @a reserved to ALi R&D Dept. then user can @a ignore it.

    @note Usage of this member is relevant for debugging @a only in project based on
          <b> TDS OS </b>.
    */
    unsigned short  write_p_offset;

    /**
    Member to contain the <b> amount of frames of the new channel </b> have already
    been shown on the screen, from the first frame showed till when the user
    decides to perform that checking.
    */
    unsigned short  display_idx;

    /**
    Member to specify the number of frames that have been decoded

    @note Member available @a only for projects based on <b> Linux OS </b>
    */
    unsigned int    frames_decoded;

    /**
    Member to specify the number of <b> video frames decoding errors </b>
    */
    unsigned int decode_error_cnt;

    /**
    Member to specify the number of <b> dropped video frames </b>
    */
    unsigned int drop_cnt;

    /**
    Member to specify the PTS of the current displayed frame

    @note Available @a only for projects based on <b> Linux OS </b>
    */
    unsigned int    last_pts;

    /**
    @warning    Member no longer supported then is @a deprecated
    */
    unsigned int    use_sml_buf;

    /**
    Member as <em> Video Decoder Output </em> (i.e. as the enum #aui_decv_output_mode)
    to contain information related to the <b> output mode of the video frame </b>
    */
    aui_decv_output_mode output_mode;

    /**
    Member to contain the <b> amount of free space </b> still available in the
    <b> VBV buffer </b>.
    */
    unsigned int    valid_size;

    /**
    @warning    Member no longer supported then is @a deprecated
    */
    unsigned int    mpeg_format;

    /**
    Member as <em> Display Aspect Ratio </em> (i.e. as the enum #aui_decv_aspect_ratio)
    to contain information related to the <b> Display Aspect Ratio (DAR) </b>
    of the video stream
    */
    aui_decv_aspect_ratio aspect_ratio;

    /**
    Member to contain the <b> frame rate </b> (in @a fps*1000 unit) of the video stream
    */
    unsigned short  frame_rate;

    /**
    @warning Member no longer supported then is @a deprecated
    */
    unsigned int    bit_rate;

    /**
    Member as a @a flag to denote if a <b> hardware error occurred </b> when
    decoding the video stream, in particular:
    - @b 0 = No
    - @b 1 = Yes

    @note This member is used @a only for the video format @b MPEG2
    */
    unsigned int    hw_dec_error;

    /**
    Member as a @a flag to denote the <b> status of displaying frames</b>,
    in particular:
    - @b 0 = Displaying frames is in pause
    - @b 1 = Frames are under displaying stage

    @note This member is used @a only in projects based on <b> TDS OS</b>.
    */
    unsigned int    display_frm;

    /**
    Member to contain how many <b> frames already decoded and already in queue </b>
    are still waiting to be displayed.

    @attention This member is @a reserved to ALi R&D Dept. then user can @a ignore it

    @note Usage of this member is relevant for debugging @a only in project
          based on <b> TDS OS </b>.
    */
    unsigned char   top_cnt;

    /**
    Member as a @a flag to denote the <b> current playback direction </b> of the
    video. Please refer to the enum #aui_playback_direction to know the different
    values of that flag.

    @note This member is <em> not used </em> for live stream.
    */
    unsigned char   play_direction;

    /**
    Member as a @a flag to denote the <b> current playback speed </b> of the
    video. Please refer to the enum #aui_playback_speed to know the different
    values of that flag.

    @note This member is <em> not used </em> for live stream.
    */
    unsigned char   play_speed;

    /**
    Member as @a flag to denote the <b> current playback direction </b> which
    has been set by the function #aui_decv_playmode_set. Please refer to the
    enum #aui_playback_direction to know the different values of that flag.

    @note This member is <em> not used </em> for live stream.
    */
    unsigned char   api_play_direction;

    /**
    Member to contain the <b> play speed </b> which has been set by the function
    #aui_decv_playmode_set. Please refer to the enum #aui_playback_speed to know
    the different values of that flag.

    @note This member is <em> not used </em> for live stream.
    */
    unsigned char   api_play_speed;

    /**
    Member as @a flag to denote if the <b> video format or video resolution </b>
    is supported by the video decoder, in particular:
    - @b 0 = One of three (3) cases can occur:
             - The video format is supported and the video resolution is not supported
             - The video format is not supported and the video resolution is supported
             - Both the video format and the video resolution are not supported
    - @b 1 = Both the video format and video resolution are supported
    */
    unsigned int    is_support;

    /**
    Member to contain the <b> total size of the VBV buffer </b> for the video decoder
    */
    unsigned int    vbv_size;

    /**
    Member to contain the <b> Direct Memory Access (DMA) Channel ID </b>, used
    by the video decoder to get video data.

    @attention This member is @a reserved to ALi R&D Dept. then user can @a ignore it.

    @note Usage of this member is relevant for debugging @a only in projects
          based on <b> TDS OS </b>.
    */
    unsigned char   cur_dma_ch;

    /**
    Member to contain the <b> Sample Aspect Ratio Width </b> of the video stream.\n
    */
    unsigned int    sar_width;

    /**
    Member to contain the <b> Sample Aspect Ratio Height </b> of the video stream.
    */
    unsigned int    sar_height;

    /**
    Member to contain the <b> Active Format Description (AFD) </b> of the video stream.\n
    */
    unsigned char   active_format;

    /**
    Member to contain the <b> display layer </b> of the video decoder.\n

    @note This member is used @a only in projects based on <b> Linux OS </b>
    */
    aui_dis_layer display_layer;

    /**
    Member as a @a Flag to denote if the <b> display rectangle switching </b> 
    has been done, in particular:
    - @b 0 = No
    - @b 1 = Yes
    */
    unsigned char rect_switch_done;
	
} aui_decv_status_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Video Decoder (DECV) Module </b> to specify video
        descriptions of video information
        </div> @endhtmlonly

        Struct to specify a list of descriptions related to the video information,
        and it is used by the function #aui_decv_get with the input parameter
        #AUI_DECV_GET_INFO.
*/
typedef struct aui_decv_info {

    /**
    Member as defined in the struct #aui_decv_status_info to specify
    <b> miscellaneous video information </b>.
    */
    aui_decv_status_info st_info;

    /**
    Member as <em> TV System </em> ( i.e. as the enum #aui_dis_tvsys) to contain
    <b> TV system information </b> for the Display Engine
    */
    aui_dis_tvsys en_tv_mode;

    /**
    Member as @a flag to denote which <b> scan mode </b> is running, in particular:
    - @b 0 = Interlaced scanning
    - @b 1 = Progressive scanning
    */
    unsigned int b_progressive;

} aui_decv_info;

/**
Enum to specify different <em> flag masks </em> to denote the new (changed)
video information as the input parameter @b para1 of the callback function
#aui_decv_callback, and returned together with the struct #aui_decv_change_info
by a callback function that belong to the callback type #AUI_DECV_CB_INFO_CHANGED
afterwards.\n Please refer to the struct #aui_decv_callback_node to know about
new (changed) video information, as per indication of the <em> flag masks </em>
stored in the struct #aui_decv_change_info.

@note This enum is used @a only in projects based on <b> TDS OS </b>.
*/
typedef enum aui_decv_change_flag {

    /**
    This <em> flag mask </em> denotes that the dimensions (i.e. height and/or width)
    of the video picture changed.
    */
    AUI_DECV_CHANGE_DIMENSIONS = 0x1,

    /**
    This <em> flag mask </em> denotes that the frame rate (in @a fps*1000 unit)
    of the video stream changed.
    */
    AUI_DECV_CHANGE_FRAME_RATE = 0x2,

    /**
    This <em> flag mask </em> denotes that the Active Format Description (AFD)
    of the video stream changed.
    */
    AUI_DECV_CHANGE_AFD = 0x4

} aui_decv_change_flag;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Video Decoder (DECV) Module </b> to specify the (new)
       changed video information
       </div> @endhtmlonly

       Struct to specify about new (changed) video information such as the input
       parameter @b para2 of the callback function #aui_decv_callback, and returned
       together with the enum #aui_decv_change_flag by a callback function that
       belong to the callback type #AUI_DECV_CB_INFO_CHANGED afterwards.\n\n

       Please refer to the struct #aui_decv_callback_node in order to know
       different variables, which contains the specific new (changed) video
       information, as per the indication of the <em> flag masks </em> defined
       in the enum #aui_decv_change_flag.

@note This struct is used @a only in projects based on <b> TDS OS </b>.
*/
typedef struct aui_decv_change_info {

    /**
    Member to contain the <b> new width </b> of the video picture.
    */
    unsigned int pic_width;

    /**
    Member to contain the <b> new height </b> of the video picture.
    */
    unsigned int pic_height;

    /**
    Member to contain the <b> new frame rate </b> (in @a fps*1000 unit)
    of the video stream.
    */
    unsigned int frame_rate;

    /**
    Member to contain the <b> new Active Format Description (AFD) </b>
    of the video stream.
    */
    unsigned char  active_format;

} aui_decv_change_info;

/**
Enum to specify available different video time code modes to be set. This enum
is used by the struct #aui_decv_time_code_info and the function #aui_decv_cancel_time_code

@note Even the video format @b MPEG2 is supported under both <b> Linux OS </b>
      and <b> TDS OS </b>, the video time code mode can be managed at the moment
      @a only under <b> TDS OS </b>.
*/
typedef enum aui_decv_time_code_mode {

    /**
    This video time code mode is useful when is necessary a video time code report,
    and involves two (2) function as below:
    - The function #aui_decv_set_time_code, which is to set the video time code
      report at the new value based on the value provided by the function
      #aui_decv_get_time_code.\n
      When the video time code report will be raised, the video will not be stopped.
    - The function #aui_decv_cancel_time_code, which is to cancel the video time
      code report.
    */
    AUI_DECV_REPORT_TIME_CODE = 1,

    /**
    This video time code mode is useful when is necessary a video time code report
    as well as stop the video, and involves two (2) function as below:
    - The function #aui_decv_set_time_code, which is to set the freeze video time
      code at the new value based on the value provided by the function
      #aui_decv_get_time_code.\n
      When the video time code occurs, a video time code report will be raise,
      and the video will be stopped with the last video frame on the screen.
    - The function #aui_decv_cancel_time_code, which is to cancel the video time
      code report.\n
      After cancel the video time code report, the video will continue to be
      display on the screen.
    */
    AUI_DECV_FREEZE_FRAME_AT_TIME_CODE

} aui_decv_time_code_mode;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Video Decoder (DECV) Module </b>
       to specify the video time code components
       </div> @endhtmlonly

       Struct to specify different time components that make the value of the
       video time code, which can be managed by two (2) functions as below:
       - The function #aui_decv_set_time_code, which is to set the value.
       - The function #aui_decv_get_time_code, which is to get the value.

@note Even the video format @b MPEG2 is supported under both <b> Linux OS </b>
      and <b> TDS OS </b>, the time components of the video time code can be
      managed at the moment @a only under <b> TDS OS </b>.
*/
typedef struct aui_decv_time_code {

    /**
    Member to specify the time component @b Hours of the video time code, which
    value belong to the range <b> [0,23] </b>.
    */
    unsigned char hour;

    /**
    Member to specify the time component @b Minutes of the video time code, which
    value belong to the range <b> [0,59] </b>.
    */
    unsigned char minute;

    /**
    Member to specify the time component @b Seconds of the video time code, which
    value belong to the range <b> [0,59] </b>.
    */
    unsigned char second;

    /**
    Member to specify the time component @b Frames of the video time code
    (i.e. the frame index within one second), which value belong to range:
    - <b> [0,29] </b> for the @b NTSC System
    - <b> [0,24] </b> for the @b PAL System
    */
    unsigned char frame;

} aui_decv_time_code;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Video Decoder (DECV) Module </b> to specify the video
       time code information
       </div> @endhtmlonly

       Struct to specify different video time code information available to be
       set by the function #aui_decv_set_time_code.

@note Even the video format @b MPEG2 is supported under both <b> Linux OS </b>
      and <b> TDS OS </b>, the different video time information can be managed
      at the moment @a only under <b> TDS OS </b>.
*/
typedef struct aui_decv_time_code_info {

    /**
    Member to specify the <b> different time components </b> that make the value
    of the video time code, as defined in the struct #aui_time_code.
    */
    aui_decv_time_code time_code;

    /**
    Member to provide the <b> available video time code mode to be set </b>, as
    defined in the enum #aui_decv_time_code_mode.
    */
    aui_decv_time_code_mode mode;

} aui_decv_time_code_info;

/**
Enum to specify different report types returned by a callback function registered
by the function #aui_decv_reg_time_code_callback.
*/
typedef enum aui_decv_time_code_report_type {

    /**
    This report type indicates that the value of the video time code has been
    set by the function #aui_decv_set_time_code has been reached or passed under
    the video time code mode #AUI_DECV_REPORT_TIME_CODE, as defined in the enum
    #aui_decv_time_code_mode.
    */
    AUI_DECV_TIME_CODE_TIME_OUT_REPORT = 0,

    /**
    This report type indicates that the value of the video time code set by the
    function #aui_decv_set_time_code is reached or passed under the video time
    code mode #AUI_DECV_FREEZE_FRAME_AT_TIME_CODE, as defined in the enum
    #aui_decv_time_code_mode.
    With this report type the video will be stopped and the last video frame
    will be frozen on the screen.
    */
    AUI_DECV_TIME_CODE_FREEZE_FRAME,

    /**
    This report type indicates that the function #aui_decv_cancel_time_code has
    been called under the video time code mode #AUI_DECV_FREEZE_FRAME_AT_TIME_CODE
    as defined in the enum #aui_decv_time_code_mode.
    After cancelled the video time code report, the video will continue to be
    displayed on the screen.
    */
    AUI_DECV_TIME_CODE_CANCEL_FREEZE_FRAME,

    /**
    This report type indicates that the function #aui_decv_cancel_time_code has
    been called under the video time code mode #AUI_DECV_REPORT_TIME_CODE, as
    defined in the enum #aui_decv_time_code_mode,
    */
    AUI_DECV_TIME_CODE_CANCEL_REPORT

} aui_decv_time_code_report_type;

/**
@brief @htmlonly <div class="details">
       Struct of the <b> Video Decoder (DECV) Module </b> to specify the
       extension info of user data which are passed to a proper callback function
       </div> @endhtmlonly

       Struct to specify the extension info of user data which are passed to a
       proper callback function. In particular, the callback type is the one
       specified with the enum value #AUI_DECV_CB_USER_DATA_PARSED
*/
typedef struct aui_decv_user_data_ext_info {

    /**
    Member to specify that the top field in the picture coding extension defined
    in the MPEG2 specifications comes first
    */
    unsigned int top_field_first     :1;

    /**
    Member to specify that the first field in the picture coding extension
    defined in the MPEG2 specification is repeated
    */
    unsigned int repeat_first_field  :1;

    /**
    Member to specify the reserved extension info of user data
    */
    unsigned int reserved             :30;

} aui_decv_user_data_ext_info;



/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to open the DECV Module and configure the desired
                attributes then get the related handle.

@warning        This function can @a only be used in the <b> Pre-Run stage </b>
                of the DECV Module, in particular:
                - Either after performing the initialization of the DECV Module
                  by the function #aui_decv_init for the first opening of the
                  DECV Module
                - Or after closing the DECV Module by the function #aui_decv_close,
                  considering the initialization of the DECV Module has previously
                  been performed by the function #aui_decv_init

@param[in]      *p_attr_decv        = Pointer to a struct #aui_attr_decv, which
                                      collects the desired attributes for the
                                      DECV Module.
@param[out]     *ppv_handle_decv    = #aui_hdl pointer to the handle of the
                                      DECV Module just opened.

@return         @b AUI_RTN_SUCCESS  = DECV Module opened successfully then user
                                      can start to configure the video decoder.
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameter
                                      (i.e. [in], [out]) are invalid.
@return         @b Other_Values     = Opening of the DECV Module failed for
                                      some reasons
*/
AUI_RTN_CODE aui_decv_open (

    aui_attr_decv *p_attr_decv,

    aui_hdl *ppv_handle_decv

    );

/**
@brief          Function used to close the DECV Module already opened by the
                function #aui_decv_open then the related handle will be released
                (i.e. the related resources such as memory, device).

@warning        This function can @a only be used in the <b> Post-Run Stage </b>
                of the DECV Module in pair with its opening by the function
                #aui_decv_open. After closing the DECV Module, user can:
                - Either perform the De-Initialization of the DECV Module by the
                  function #aui_decv_de_init
                - Or open again the DECV Module by the function #aui_decv_open,
                  considering the initialization of the DECV Module has been
                  performed previously by the function #aui_decv_init

@param[in]      *p_attr_decv        = Pointer to a struct #aui_attr_decv, which
                                      collects the desired attributes for the
                                      DECV Module to be closed
                                      - @b Caution: For the DECV Module this
                                           value is suggested to be set as @b NULL,
                                           cause the struct #aui_attr_decv does not
                                           store any memory pointer in heap.
@param[in]      *ppv_handle_decv    = #aui_hdl pointer to the handle of the
                                      DECV Module already opened and to be closed

@return         @b AUI_RTN_SUCCESS  = DECV Module closed successfully
@return         @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                      (i.e. [in]) are invalid.
@return         @b Other_Values     = Closing of the DECV Module failed for
                                      same reasons

@note           @a Only in projects based on <b> TDS OS </b>, it is suggested to
                @a only use this function before to go into standby mode.
*/
AUI_RTN_CODE aui_decv_close (

    aui_attr_decv *p_attr_decv,

    aui_hdl *ppv_handle_decv

    );

/**
@brief          Function used to start the DECV Module already opened by the
                function #aui_decv_open. In particular, this function is used
                after some supposed/suggested configurations such as:
                - Synchronization (by the function #aui_decv_sync_mode)
                - Video format (by the function #aui_decv_decode_format_set)

                have already been performed after opening the DECV Module. After
                starting the DECV Module, the video decoder can start to decode
                the incoming video data.

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened and to be started.

@return         @b AUI_RTN_SUCCESS  = DECV Module started successfully,
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Starting of the DECV Module failed
                                      for some reasons.
*/
AUI_RTN_CODE aui_decv_start (

    aui_hdl pv_handle_decv

    );

/**
@brief          Function used to stop the DECV Module already opened and started
                by, respectively, the functions #aui_decv_open and #aui_decv_start.\n
                After stopping the DECV Module, the video decoder will clear the
                decoded ES data and the screen will show:
                - As black
                - The last picture of the video just stopped

                depends of the channel change mode that is running, as defined
                in the enum #aui_en_decv_chg_mode. After stopping the DECV Module,
                user can:
                - Either close the DECV Module by the function #aui_decv_close
                  if no need to use the DECV Module any more
                - Or start again the DECV Module by the function #aui_decv_start,
                  considering the supposed/suggested configurations of the DECV
                  Module such as
                  - Synchronization (by the function #aui_decv_sync_mode)
                  - Video format (by the function #aui_decv_decode_format_set)

                  have been performed again before starting the DECV Module, if
                  user wants to change the video channel.

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened and started then to be stopped.

@return         @b AUI_RTN_SUCCESS  = DECV Module stopped successfully.
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid.
@return         @b Other_Values     = Stopping of the DECV Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_decv_stop (

    aui_hdl pv_handle_decv

    );

/**
@brief          Function used to pause the DECV Module already opened and started
                by, respectively, the functions #aui_decv_open and #aui_decv_start.\n
                After pausing the DECV Module, the last showed frame of the video
                will be frozen on the screen, and two (2) actions can be performed
                as below:
                - Resuming the video by the function #aui_decv_resume
                - Stopping the video by the function #aui_decv_stop

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened and started then to be paused.

@return         @b AUI_RTN_SUCCESS  = DECV Module paused successfully.
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid.
@return         @b Other_Values     = Pausing of the DECV Module failed
                                      for some reasons
*/
AUI_RTN_CODE aui_decv_pause (

    aui_hdl pv_handle_decv

    );

/**
@brief          Function used to resume the DECV Module already opened, started
                and paused by, respectively, the functions #aui_decv_open,
                #aui_decv_start and #aui_decv_pause.\n
                After resuming the DECV Module, the video will play again, and
                two (2) actions can be performed as below:
                - Pausing again by the function #aui_decv_pause
                - Stopping the video by the function #aui_decv_stop

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened,started and paused then to be resumed.

@return         @b AUI_RTN_SUCCESS  = DECV Module resumed successfully.
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid.
@return         @b Other_Values     = Resuming of the DECV Module failed
                                      for some reasons.
*/
AUI_RTN_CODE aui_decv_resume (

    aui_hdl pv_handle_decv

    );

/**
@brief          Function used to perform the initialization of the DECV Module
                before its opening by the function #aui_decv_open.

@warning        This function can be used @a only in the <b> Pre-Run Stage </b>
                of the DECV Module.

@param[in]      fn_decv_init        = Callback function used for the initialization
                                      of the DECV Module, as per comment for the
                                      function pointer #aui_func_decv_init.

@return         @b AUI_RTN_SUCCESS  = DECV Module initialized successfully,
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid.
@return         @b Other_Values     = Initializing of the DECV Module failed
                                      for some reasons.

@note           About the callback function @b fn_decv_init as input parameter:
                - In projects based on <b> Linux OS </b>, it is suggested to
                  set it as @b NULL
                - In projects based on <b> TDS OS </b>, it will perform some
                  video device attachments and configurations, please refer to
                  the sample code related to the <b> Init Stage </b> for more
                  clarifications.
*/
AUI_RTN_CODE aui_decv_init (

    aui_func_decv_init fn_decv_init

    );

/**
@brief          Function used to perform the de-initialization of the DECV Module
                after its closing by the function #aui_decv_close.

@param[in]      fn_decv_de_init     = Callback function used to de-initialize
                                      of the DECV Module, as per comment for the
                                      function pointer #aui_func_decv_init.

@return         @b AUI_RTN_SUCCESS  = DECV Module de-initialized successfully.
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid.
@return         @b Other_Values     = De-initializing of the DECV Module failed
                                      for some reasons.

@note           @a Only in projects based on <b> TDS OS </b>, it is suggested to
                @a only use this function before to go into standby mode.
*/
AUI_RTN_CODE aui_decv_de_init (

    aui_func_decv_init fn_decv_de_init

    );

/**
@brief          Function used to set the desired video decoding format to the
                DECV Module between its opening and starting by, respectively,
                the functions #aui_decv_open and #aui_decv_start.
                In particular, this function will change the handle attribution
                to the desired video format.

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened and to be set for the desired video
                                      decoding format
@param[in]      en_format           = Desired video format as defined in the enum
                                      #aui_en_dec_format and to be set to the
                                      DECV Module, i.e.
                                      - @b MPEG2
                                      - @b H.264
                                      - @b AVS (@a only supported in projects
                                        based on <em> Linux OS </em>)

@return         @b AUI_RTN_SUCCESS  = Desired video decoding format set successfully
                                      to the DECV Module.
@return         @b AUI_RTN_EINVAL   = Either one or both input parameters
                                      (i.e. [in]) are invalid.
@return         @b Other_Values     = Setting of the desired video decoding format
                                      to the DECV Module failed for some reasons

@note           The video formats defined in the enum #aui_en_dec_format and valid
                for the <b> AV-Inject Module </b> cannot be set in this way, then
                to know how to play these video formats please refer to the related
                header file.
*/
AUI_RTN_CODE aui_decv_decode_format_set (

    aui_hdl pv_handle_decv,

    aui_decv_format en_format

    );

/**
@brief          Function used to get the current video format set to the DECV
                Module by the function #aui_decv_decode_format_set.

@param[in]      pv_handle_decv      = #aui_hdl handle of DECV Module already
                                      opened and to be checked for the video
                                      format currently set.

@param[out]     p_en_format         = The video format already set as the current
                                      one to the DECV Module, as defined in the
                                      enum #aui_en_dec_format.

@return         @b AUI_RTN_SUCCESS  = Checking for the current video format set
                                      to the DECV Module performed successfully,
                                      then the output parameter @b en_format
                                      reports the video format currently set to
                                      the DECV Module
@return         @b AUI_RTN_EINVAL   = Either one or both of the parameters
                                      (i.e. [in], [out]) are invalid.
@return         @b Other_Values     = Getting of the current video format set
                                      to the DECV Module failed for some reasons.
*/
AUI_RTN_CODE aui_decv_decode_format_get (

    aui_hdl pv_handle_decv,

    aui_decv_format *p_en_format

    );

/**
@brief          Function used to change the desired video playback speed and
                direction parameters of the DECV Module between its opening
                and starting by, respectively, the functions #aui_decv_open
                and #aui_decv_start.

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened and to be changed for the desired
                                      video playback speed and direction
                                      parameters
@param[in]      speed               = Desired video playback speed parameter to
                                      be changed for the DECV Module.
@param[in]      direction           = Desired video playback direction parameter
                                      to be changed for the DECV Module.

@return         @b AUI_RTN_SUCCESS  = Changing to the desired video playback
                                      speed and direction parameters of the
                                      DECV Module performed successfully.
@return         @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                      (i.e. [in]) are invalid.
@return         @b Other_Values     = Changing to the desired playback speed and
                                      direction parameters of the DECV Module
                                      failed for some reasons.

@note           @b 1. A High level interface for changing the video playback
                      speed and direction parameters is available also by other
                      functions mention in the header file related to <b> AV
                      Module </b>\n

@note           @b 2. This function is used @a only for projects based on <b>
                      Linux OS </b>.
*/
AUI_RTN_CODE aui_decv_playmode_set (

    aui_hdl pv_handle_decv,

    aui_playback_speed speed,

    aui_playback_direction direction

    );

/**
@brief          Function used to set the desired synchronization mode of the
                DECV Module between its opening and starting by, respectively,
                the functions #aui_decv_open and #aui_decv_start.

@param[in]      pv_handle_decv      = #aui_hdl handle of the DECV Module already
                                      opened and to be set for the desired
                                      synchronization mode.
@param[in]      en_sync_mode        = Desired synchronization mode to be set
                                      for the DECV Module. For more information
                                      about the different synchronization mode,
                                      please refer to the enum #aui_stc_avsync_mode.

@return         @b AUI_RTN_SUCCESS  = Setting of the desired synchronization mode
                                      of the DECV Module performed successfully,
@return         @b AUI_RTN_EINVAL   = Either one or both of the input parameters
                                      (i.e. [in]) are invalid.
@return         @b Other_Values     = Setting of the desired synchronization mode
                                      of the DECV Module failed for some reasons.
*/
AUI_RTN_CODE aui_decv_sync_mode (

    aui_hdl pv_handle_decv,

    aui_stc_avsync_mode en_sync_mode

    );

/**
@brief          Function used to set the desired channel change mode of the
                DECV Module between its opening and starting by, respectively,
                the functions #aui_decv_open and #aui_decv_start.\n
                Three (3) channel change modes are available, please refer to
                the enum #aui_en_decv_chg_mode for more clarifications.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be set for the desired channel
                                       change mode.
@param[in]      en_chg_mode         =  Desired channel change mode to be set for
                                       the DECV Module.

@return         @b AUI_RTN_SUCCESS  =  Setting of the desired channel change mode
                                       of the DECV Module performed successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid.
@return         @b Other_Values     =  Setting of the desired channel change mode
                                       of the DECV Module failed for some reasons.
*/
AUI_RTN_CODE aui_decv_chg_mode_set (

    aui_hdl pv_handle_decv,

    aui_decv_chg_mode en_chg_mode

    );

/**
@brief          Function used to get the buffer of the DECV Module with the size
                still available to inject new video data, after its opening and
                starting by, respectively, the functions #aui_decv_open and
                #aui_decv_start.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be checked for the buffer
                                       with the size still available to inject
                                       new video data.
@param[in]      ul_request_size     =  Total size of the buffer needed to inject
                                       new video data.

@param[out]     pp_inject_buf       =  Getting of the pointer to the buffer to be
                                       injected of new video data.
@param[out]     p_size_got          =  Getting of the available size of the buffer
                                       for injecting new video data.

@return         @b AUI_RTN_SUCCESS  =  Checking for the available size of the buffer
                                       of the DECV Module performed successfully,
                                       then the buffer can be injected of new video
                                       data by, for example, memory copy.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the parameters
                                       (i.e. [in], param[out]) are invalid.
@return         @b Other_Values     =  Checking for the available size of the buffer
                                       of the DECV Module failed for some reasons.

@note           @b 1. This function is used @a only to show the @b I-frame on the
                      screen under the video format @b MPEG2, in particular to show
                      the customized logo.
@note           @b 2. In projects based on <b> TDS OS </b>, this function is used
                      to get the buffer from VBV.\n
                      In projects based on <b> Linux OS </b>. please use the function
                      #aui_decv_copy_data to inject new video data into the video
                      decoder instead using this function cause already @a deprecated.
*/
AUI_RTN_CODE aui_decv_inject_buf_get (

    aui_hdl pv_handle_decv,

    unsigned long ul_request_size,

    void **pp_inject_buf,

    int *p_size_got

    );

/**
@brief          Function used to inject the new video data of the buffer of the
                DECV Module (which has been gotten by the function #aui_decv_inject_buf_get)
                into the video decoder.

@warning        User @a must use this function after the function #aui_decv_inject_buf_get
                in order to get enough information related to the buffer to perform
                the right video data injection, then avoid unexpected system
                behaviours (e.g. crash system).

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be injected of the new video
                                       data into the buffer.
@param[in]      size_got            =  The size of the new video data of the buffer
                                       to be injected in the video decoder.

@return         @b AUI_RTN_SUCCESS  =  The new video data of the buffer has been
                                       injected into the video decoder successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid.
@return         @b Other_Values     =  The injection of the new video data of the
                                       buffer into the video decoder failed
                                       for some reasons.

@note           For projects based on <b> Linux OS </b>, please use the function
                #aui_decv_copy_data to inject the new video data into the video
                decoder instead using this function cause already @a deprecated.
*/
AUI_RTN_CODE aui_decv_inject_update (

    aui_hdl pv_handle_decv,

    int size_got

    );

/**
@brief          Function used to perform a specific setting of the DECV Module
                (as defined in the enum #aui_en_decv_item_set) between its
                opening and starting by, respectively, the functions #aui_decv_open
                and #aui_decv_start.

@param[in]      pv_hdl_decv         =  #aui_hdl handle of the DECV Module already
                                       opened and to be managed to perform a
                                       specific setting
@param[in]      ul_item             =  The item related to the specific setting
                                       of the DECV Module to be performed, as
                                       defined in the enum #aui_en_decv_item_set.
@param[in]      *pv_param           =  The pointer as per the description of the
                                       specific setting of the DECV Module to be
                                       performed, as defined in the enum
                                       #aui_en_decv_item_set

@return         @b AUI_RTN_SUCCESS  =  Specific setting of the DECV Module
                                       performed successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid.
@return         @b Other_Values     =  Specific setting of the DECV Module
                                       failed for some reasons.
*/
AUI_RTN_CODE aui_decv_set (

    aui_hdl pv_hdl_decv,

    aui_decv_item_set ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific information of the DECV Module
                (as defined in the enum #aui_en_decv_item_get) after its opening
                and starting by, respectively, the functions #aui_decv_open
                and #aui_decv_start.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be managed to get a specific
                                       information.
@param[in]      ul_item             =  The item related to the specific information
                                       of the DECV Module to be gotten, as defined
                                       in the enum #aui_en_decv_item_get.

@param[out]     *pv_param           =  The pointer as per the description of the
                                       specific information of the DECV Module
                                       to be gotten, as defined in the enum
                                       #aui_en_decv_item_get.

@return         @b AUI_RTN_SUCCESS  =  Getting of the specific information of the
                                       DECV Module performed successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the parameters
                                       (i.e. [in], [out]) are invalid.
@return         @b Other_Values     =  Getting of the specific information of
                                       the DECV Module failed for some reasons.
*/
AUI_RTN_CODE aui_decv_get (

    aui_hdl pv_handle_decv,

    aui_decv_item_get ul_item,

    void *pv_param

    );

/**
@brief          Function used to set the time code information to display module

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be manged to set the time code
@param[in]      aui_time_code_info  =  Collection of time code information as per
                                       the struct #aui_decv_time_code_info

@return         @b AUI_RTN_SUCCESS  =  Setting of the time code information to
                                       display module performed succesfully
@return         @b AUI_RTN_EINVAL   =  At least one input parameter (i.e. [in])
                                       is invalid
@return         @b Other_Values     =  Setting of the time code information to
                                       display module failed for some reasons

@note           This function is used @a only in projects based on <b> TDS OS </b>,
                especially for <b> OTV PFM </b> (Play From Memory)
*/
AUI_RTN_CODE aui_decv_set_time_code (

    aui_hdl pv_handle_decv,

    aui_decv_time_code_info aui_time_code_info

    );

/**
@brief          Function used to cancel the video time code previously set to the
                DECV Module by the function #aui_decv_set_time_code.\n
                For more details about using this function, please refer to the
                enum #aui_decv_time_code_mode.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be managed to cancel the video
                     time code.
@param[in]      mode                =  The video time code mode for the DECV Module
                                       to be cancelled, as per the enum
                                       #aui_decv_time_code_mode

@return         @b AUI_RTN_SUCCESS  =  Cancelling of the video time code set to
                                       the DECV Module performed successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid.
@return         @b Other_Values     =  Cancelling of the video time code set to
                                       the DECV Module failed for some reasons.

@note           This function is used @a only in projects based on <b> TDS OS </b>,
                especially for <b> OTV PFM </b> (Play From Memory).
*/
AUI_RTN_CODE aui_decv_cancel_time_code (

    aui_hdl pv_handle_decv,

    aui_decv_time_code_mode mode

    );

/**
@brief          Function used to get the video time code value from the DECV Module
                after its opening and starting by, respectively, the functions
                #aui_decv_open and #aui_decv_start.

@pre            Usage of this function is the second precondition for using the
                function #aui_decv_set_time_code to set the new video time code
                information.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be managed to get the video
                                       time code

@param[out]     p_time_code         =  Pointer to the basic video time code value,
                                       as per the struct #aui_time_code

@return         @b AUI_RTN_SUCCESS  =  Getting of the video time code value
                                       performed successfully
@return         @b AUI_RTN_EINVAL   =  Either one or both of the parameters
                                       (i.e. [in], [out]) are invalid
@return         @b Other_Values     =  Getting of the video time code value
                                       failed for some reasons

@note           This function is used @a only for projects based on <b> TDS OS </b>,
                especially for <b> OTV PFM </b> (Play For Memory).
*/
AUI_RTN_CODE aui_decv_get_time_code (

    aui_hdl pv_handle_decv,

    aui_decv_time_code *p_time_code

    );

/**
@brief          Function used to register the callback function for the video
                time code between its opening and starting by, respectively,
                the functions #aui_decv_open and #aui_decv_start.\n
                If the time reaches the target time code value set by the function
                #aui_decv_set_time_code, the registered callback function will be
                actioned.\n
                For more information about that registered callback function,
                please refer to the comment for the function pointer
                #aui_time_code_cbfunc

@pre            Usage of this function is the first precondition for using the
                function #aui_decv_set_time_code to set the new video time code
                information.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be managed to register the
                                       callback function for the video time code.
@param[in]      callback            =  The callback function for the video time
                                       code to be register.

@return         @b AUI_RTN_SUCCESS  =  Registering of the callback function for
                                       the video time code performed successfully
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid
@return         @b Other_Values     =  Registering of the callback function for
                                       the video time code failed for some reasons

@note           This function is used @a only for projects based on <b> TDS OS </b>,
                especially for <b> OTV PFM </b> (Play From Memory).
*/
AUI_RTN_CODE aui_decv_reg_time_code_callback (

    aui_hdl pv_handle_decv,

    aui_time_code_cbfunc callback

    );

/**
@brief          This function is used to inject the video data directly into the
                DECV Module

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be directly injected of the
                                       video data
@param[in]      src_addr            =  The memory address of the pointer to the
                                       video data
@param[in]      req_data            =  Size of the video data to be injected into
                                       the DECV Module

@param[out]     p_got_size          =  Size of the video data already injected.

@return         @b AUI_RTN_SUCCESS  =  Injection of the video data directly into
                                       the DECV Module performed successfully
@return         @b AUI_RTN_EINVAL   =  Either one or both of the parameters
                                       (i.e. [in] and param[out]) are invalid
@return         @b Other_Values     =  Injection of the video data directly into
                                       the DECV Module failed for some reasons

@note           This function is used @a only to show the @b I-frame on the screen
                under the video format @b MPEG2, in particular to show the
                customized logo.\n
                For more clarification about showing I-frame, please refer to
                the ALi Document
                <b><em>
                ALI_AUI_Porting_Guide_Modules - Chapter "Video Decoder (DECV) Module"
                </em></b>
@attention      For MPEG2 m2v data, the application needs to send two (2) I frames
                to the DECV in order to make the I frame decoded and displayed on the
                screen.\n
                For AVC annex.b bitstream, the application needs to send three (3) I
                frames to the decv in order to make the first I frame decoded and
                displayed on the screen
*/
AUI_RTN_CODE aui_decv_copy_data (

    aui_hdl pv_handle_decv,

    unsigned int src_addr,

    unsigned int req_data,

    unsigned int *p_got_size

    );

/**
@brief          Function used to flush the video frames of the display queue into
                the <b> Display Engine (DIS) Module </b>.

@pre            Precondition for using this function is no video data is coming,
                however the last video frame in the display queue cannot be
                flushed then displayed on the screen till using this function.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV module already
                                       opened and to be managed to flush the
                                       video frames of the display queue into
                                       the Display Engine (DIS) Module.
@param[in]      enable              =  Flag to indicate which flush mode need to
                                       be performed, in particular:
                                       - @b 1 = Flush immediately
                                                - @b Caution: Make sure all the
                                                     video frames have been
                                                     decoded before to perform
                                                     that flush mode
                                       - @b 0 = Flush when all the video frames
                                                have been decoded

@return         @b AUI_RTN_SUCCESS  =  Flushing the video frames performed
                                       successfully
@return         @b AUI_RTN_EINVAL   =  Either one or both of the parameters
                                       (i.e. [in], [out]) are invalid.
@return         @b Other_Values     =  Flushing the video frames failed for
                                       some reasons

@note           This function is @a only used in project based on <b> TDS OS
                </b>, especially @a only for <b> OTV PFM </b> (Play From
                Memory).
*/
AUI_RTN_CODE aui_decv_io_flush (

    aui_hdl pv_handle_decv,

    unsigned int  enable

    );

/**
@brief          Function used to set the type of user data which are passed to
                a proper callback function. In particular, the callback type is
                the one specified with the enum value #AUI_DECV_CB_USER_DATA_PARSED

@warning        This function needs to be called after calling the function
                #aui_decv_start

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV module
                                       already opened
@param[in]      type                =  User data type to be set as defined in
                                       the enum #aui_decv_user_data_type

@return         @b AUI_RTN_SUCCESS  =  Setting of the user data type performed
                                       successfully
@return         @b AUI_RTN_EINVAL   =  At least one input parameters (i.e. [in])
                                       is invalid
@return         @b Other_Values     =  Setting of user data type failed for some
                                       reasons

@note           This function supports @a only the video format @b MPEG2

*/
AUI_RTN_CODE aui_decv_user_data_type_set (

    aui_hdl pv_handle_decv,

    aui_decv_user_data_type  type

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
Function pointer to specify the type of callback function
- Registered with the function #aui_decv_common_callback_register for video
  decoder events
- Filled in the the struct #aui_decv_callback_node and registered for various
  event as present in the enum #aui_en_decv_callback_type

The input parameters @b param1 and @b param2 are used in different callback
notification, more information about them can be found in the
enum #aui_en_decv_callback_type.

@note This function pointer can be used @a only in projects based on <b> TDS
      OS </b>
*/
typedef void (*common_callback) (

    unsigned int param1,

    unsigned int param2

    );

/**
Function pointer to specify the type of callback function registered with the
function #aui_decv_first_showed_set to get the first video frame notification
*/
typedef void (*aui_fun_first_showed) (

    void

    );

/**
Below the renaming of some items
START LIST
*/

#define aui_en_dec_format  aui_decv_format

#define AUI_CHG_BLACK AUI_DECV_CHG_BLACK

#define AUI_CHG_STILL AUI_DECV_CHG_STILL

#define AUI_CHG_OTV_PFM AUI_DECV_CHG_OTV_PFM

#define AUI_CB_USER_DATA_PARSED AUI_DECV_CB_USER_DATA_PARSED

#define AUI_CB_INFO_CHANGE AUI_DECV_CB_INFO_CHANGED

#define AUI_CB_ERROR AUI_DECV_CB_ERROR

#define AUI_CB_MONITOR_START AUI_DECV_CB_MONITOR_START

#define AUI_CB_MONITOR_STOP AUI_DECV_CB_MONITOR_STOP

#define AUI_CB_MONITOR_STATE_CHANGE AUI_DECV_CB_MONITOR_STATE_CHANGE

#define AUI_CB_STATE_CHANGE AUI_DECV_CB_STATE_CHANGE

#define aui_en_vbv_buf_mode aui_decv_vbv_buf_mode

#define AUI_VBV_BUF_OVERFLOW_RST_MODE AUI_DECV_VBV_BUF_OVERFLOW_RST_MODE

#define AUI_VBV_BUF_BLOCK_FULL_MODE AUI_DECV_VBV_BUF_BLOCK_FULL_MODE

#define aui_vdec_status aui_decv_status

#define AUI_VDEC_ATTACHED AUI_DECV_ATTACHED

#define AUI_VDEC_DECODING AUI_DECV_DECODING

#define AUI_VDEC_PAUSED AUI_DECV_PAUSED

#define AUI_VDEC_STOPPED AUI_DECV_STOPPED

#define aui_en_sync_mode aui_stc_avsync_mode

//#define error_type AUI_DECV_ERROR_TYPE

//#define AUI_ERROR_NO_DATA AUI_DECV_ERROR_NO_DATA

//#define AUI_ERROR_HARDWARE AUI_DECV_ERROR_HARDWARE

//#define AUI_ERROR_SYNC AUI_DECV_ERROR_SYNC

//#define AUI_ERROR_FRAME_DROP AUI_DECV_ERROR_FRAME_DROP

#define aui_time_code_mode aui_decv_time_code_mode

#define AUI_REPORT_TIME_CODE AUI_DECV_REPORT_TIME_CODE

#define AUI_FREEZE_FRAME_AT_TIME_CODE AUI_DECV_FREEZE_FRAME_AT_TIME_CODE

#define aui_time_code_t aui_decv_time_code

#define aui_set_time_code_info aui_decv_time_code_info

#define AUI_DECV_SYNC_PTS AUI_STC_AVSYNC_PCR

#define AUI_DECV_SYNC_AUDIO AUI_STC_AVSYNC_AUDIO

#define AUI_DECV_SYNC_FREERUN AUI_STC_AVSYNC_FREERUN

#define aui_vdec_status_info aui_decv_status_info

/**
Renaming of some items
END LIST
*/

/**
Enum to specify different error type
*/
enum error_type {

    /**
    Value to specify that the decoder is not getting data
    */
    AUI_ERROR_NO_DATA = 0x1,

    /**
    Value to specify an hardware error
    */
    AUI_ERROR_HARDWARE = 0x2,

    /**
    Value to specify a synchronization error
    */
    AUI_ERROR_SYNC = 0x4,

    /**
    Frame dropped
    */
    AUI_ERROR_FRAME_DROP = 0x8

};

/**
Enum to specify different output mode
*/
enum aui_vdec_output_mode {

    /**
    In this mode, the video will be shown on full screen
    */
    AUI_MP_MODE,

    /**
    @attention This mode will be available in the future when ALi chipset will
               support multiple video decoder, so user can ignore this value
    */
    AUI_PIP_MODE,

    /**
    In this mode, the video will be shown in one small window on the screen as
    preview. About the setting of that small window (e.g. width, height, position
    on the screen), please refer to the function #aui_dis_mode_set.
    */
    AUI_PREVIEW_MODE,

    /**
    Below output modes @a only used in the old ALi chipsets then not valid in
    the new ones
    START LIST
    */

    AUI_HSCALE_MODE,

    AUI_DVIEW_MODE,

    AUI_MP_DVIEW_MODE,

    AUI_HSCALE_DVIEW_MODE,

    AUI_AUTO_MODE,

    AUI_DUAL_MODE,

    AUI_DUAL_PREVIEW_MODE,

    AUI_IND_PIP_MODE,

    AUI_SW_PASS_MODE,

    AUI_HW_DEC_ONLY_MODE,

    AUI_MULTI_VIEW_MODE,

    AUI_RESERVE_MODE

    /**
    Modes @a only used in the old ALi chipsets then not valid in the new ones
    END LIST
    */

};

/**
Enum to specify different aspect ratio
*/
enum aui_asp_ratio {

    /**
    @warning    Value no longer supported then is @a deprecated
    */
    AUI_DAR_FORBIDDEN = 0,

    /**
    This value refers to the <b> Sample Aspect Ratio (SAR) </b> which has effect
    on the value of the @b DAR
    */
    AUI_SAR,

    /**
    This value indicates the value of the @b DAR is <b> 4:3 </b>
    */
    AUI_DAR_4_3,

    /**
    This value indicates the value of the @b DAR is <b> 16:9 </b>
    */
    AUI_DAR_16_9,

    /**
    This value indicates the value of the @b DAR is <b> 2.21:1 </b>
    */
    AUI_DAR_221_1

};

/**
Enum to specify different information changing
*/
enum msg_info_change {

    /**
    <em> Flag mask </em> to denote that the dimensions of the video picture
    changed (i.e. height and/or width)
    */
    AUI_CHANGE_DIMENSIONS = 0x1,

    /**
    <em> Flag mask </em> to denote that the frame rate of the video stream
    changed (in @a fps*1000 unit)
    */
    AUI_CHANGE_FRAME_RATE = 0x2,

    /**
    <em> Flag mask </em> to denote that the Active Format Description (AFD)
    of the video stream changed
    */
    AUI_CHANGE_AFD = 0x4

};

/**
Enum to specify different report types returned by a callback function registered
by the function #aui_decv_reg_time_code_callback.
*/
typedef enum {

    /**
    This report type indicates that the value of the video time code set by the
    function #aui_decv_set_time_code has been reached or passed under the video
    time code mode #AUI_DECV_REPORT_TIME_CODE, as defined in the enum
    #aui_decv_time_code_mode.
    */
    AUI_TIME_OUT_REPORT = 0,

    /**
    This report type indicates that the value of the video time code set by the
    function #aui_decv_set_time_code is reached or passed under the video time
    code mode #AUI_DECV_FREEZE_FRAME_AT_TIME_CODE, as defined in the enum
    #aui_decv_time_code_mode. With this report type the video will be stopped
    and the last video frame will be frozen on the screen.
    */
    AUI_FREEZE_FRAME,

    /**
    This report type indicates that the function #aui_decv_cancel_time_code has
    been called under the video time code mode #AUI_DECV_FREEZE_FRAME_AT_TIME_CODE
    as defined in the enum #aui_decv_time_code_mode. After cancelled the video
    time code report, the video will continue to be displayed on the screen.
    */
    AUI_CANCEL_FREEZE_FRAME,

    /**
    This report type indicates that the function #aui_decv_cancel_time_code has
    been called under the video time code mode #AUI_DECV_REPORT_TIME_CODE, as
    defined in the enum #aui_decv_time_code_mode,
    */
    AUI_CANCEL_REPORT

} aui_report_type;

/**
Struct of the DECV Module to contain the changed information
*/
struct change_info {

    /**
    Member to contain the <b> new width </b> of the video picture
    */
    unsigned int pic_width;

    /**
    Member to contain the <b> new height </b> of the video picture
    */
    unsigned int pic_height;

    /**
    Member to contain the <b> new frame rate </b> of the video stream
    (in @a fps*1000 unit)
    */
    unsigned int frame_rate;

    /**
    Member to contain the <b> new Active Format Description (AFD) </b>
    of the video stream.
    */
    unsigned char  active_format;

};

/**
@brief         This function is used to register a callback function

@warning       Since this function is @a deprecated, please use the function
               #aui_decv_set with input parameter #AUI_DECV_SET_REG_CALLBACK
               as defined in the enum #aui_decv_item_set to register a
               callback function

@param[in]     pv_handle_decv       #aui_hdl pointer to the handle of the DECV
                                    module already opened and to be managed to
                                    register a callback function
@param[in]     callback_type        Callback type to register as per the enum
                                    #aui_en_decv_callback_type
@param[in]     com_cb               Callback funtion pointer as per comment for
                                    the function pointer #common_callback

@return        @b AUI_RTN_SUCCESS   Registering of the callback function performed
                                    successfully
@return        @b AUI_RTN_EINVAL    At least on input parameter (i.e. [in])
                                    is invalid
@return        @b Other_Values      Registering of the callback funtion failed
                                    for soem reasons

*/
AUI_RTN_CODE aui_decv_common_callback_register (

  void *pv_handle_decv,

  int callback_type,

  common_callback com_cb

  );

/**
@brief          Function used to set the callback function to the DECV Module
                to be called when the first video frame of the new channel is
                shown on the screen (as per comment for the function pointer
                #aui_fun_first_showed).

@warning        This function will be @a deprecated soon then it suggested to
                ignore it

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV Module already
                                       opened and to be managed to set the callback
                                       function.
@param[in]      first_showed        =  The callback function to be set, as per
                                       comment for the function pointer
                                       #aui_fun_first_showed.

@return         @b AUI_RTN_SUCCESS  =  Setting of the callback function to the
                                       DECV Module performed successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid.
@return         @b Other_Values     =  Setting of the callback function to the
                                       DECV Module failed for some reasons.
*/
AUI_RTN_CODE aui_decv_first_showed_set (

  aui_hdl pv_handle_decv,

  aui_fun_first_showed first_showed

  );

/**
@brief          Function used to set/unset the support of the DECV Module to
                play the variable resolution of the video pictures properly
                between its opening and starting by, respectively, the functions
                #aui_decv_open and #aui_decv_start.

@param[in]      pv_handle_decv      =  #aui_hdl handle of the DECV module already
                                       opened and to be set/unset for the support
                                       to play the variable resolution of the
                                       video pictures properly.
@param[in]      enable              =  Flag to indicate if the support of the DECV
                                       Module to play the variable resolution of
                                       the video pictures properly has to be
                                       enabled or disabled, in particular:
                                       - @b 1 = Support enabled
                                       - @b 0 = Support disabled (that is the
                                                default value if either this
                                                function is not used or the flag
                                                is not set by the user)

@return         @b AUI_RTN_SUCCESS  =  Setting/Resetting of the support of the
                                       DECV Module to play the variable resolution
                                       performed successfully.
@return         @b AUI_RTN_EINVAL   =  Either one or both of the input parameters
                                       (i.e. [in]) are invalid.
@return         @b Other_Values     =  Setting/Resetting of the support of the
                                       DECV Module to play variable resolution
                     failed for some reasons.

@note           This function is @a only used for projects based on <b> TDS
                OS </b> and is @a only supported by the video format @b H.264.\n
                For projects based on <b> Linux OS </b>,instead, please refer
                to the description of the item #AUI_DECV_SET_VARIABLE_RESOLUTION
                of the enum #aui_en_decv_item_set.
*/
AUI_RTN_CODE aui_decv_io_sameless_switch (

  aui_hdl pv_handle_decv,

  unsigned int enable

  );

AUI_RTN_CODE aui_decv_trickmode_set(void *pv_handle_decv, enum aui_playback_speed speed, 
	enum aui_playback_direction direction, int mode);

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


