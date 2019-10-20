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

@file   aui_music.h

@brief  Music Module

    Music Module is a sub-module of the Media Player Module and is used to
    play audio files. It @a must cooperate with its own file system.\n
    To play the stream file, user needs to have the path of the music file
    as input.

@note  For further details, please refer to ALi document
       <b><em>
     ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Media Player Module"
     </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_MUSIC_H

#define _AUI_MUSIC_H

/*************************Included Header File List***************************/

#include "aui_common.h"

/*******************************Global Type List******************************/

/**
Function pointer used to specify the type of callback function registered with
the functions #aui_music_init and #aui_music_de_init,and to be called during the
<b> Initialization/De-Initialization Stage </b> of Music Module.\n
*/
typedef void (*aui_func_music_init) (

    void

    );

/**
Enum used to perform miscellaneous settings on Music Module

@note   This enum is used by the function #aui_music_set to perform a specific
        setting where
        - The parameter @b ul_item takes the item related to the specific setting
          to perform
        - The parameter @b pv_param takes the pointer as per the description of
          the specific setting to perform

@warning  This enum is @a deprecated then can be ignored
*/
typedef enum aui_music_item_set {

    /**
  Value used to set the file path to be played by the Music Module
    */
    AUI_MUSIC_SET_FILE_NAME,

} aui_music_item_set;

/**
Enum used to get information about miscellaneous settings on Music Module

@note   This enum is used by function #aui_music_get to get information about
        a specific setting where
        - The parameter @b ul_item takes the item related to the specific setting
          to perform
        - The parameter @b pv_param takes the pointer as per the description of
          the specific setting
*/
typedef enum aui_music_item_get {

    /**
    Value used to get music information such as
    - Title
    - Artist
    - Album
    - Year
    - etc.

    @note   This value is available @a only in projects based on <b> TDS OS </b>
    */
    AUI_MUSIC_GET_MUSIC_INFO,

    /**
    Value used to get information about the <b> Audio decoder </b>

    @note   This value is available @a only in projects based on <b> TDS OS </b>
    */
    AUI_MUSIC_GET_DECODER_INFO,

} aui_music_item_get;

/**
Enum used to get extra information about miscellaneous settings on Music Module

@note This enum is used @a only in projects based on <b> Linux OS </b>
*/
typedef enum aui_music_ext_item_get {

    /**
    Value used to get information about the <b> Audio </b>
    */
    AUI_MUSIC_GET_AUDIO,

    /**
    Value used to get information about the <b> Subtitle </b>
    */
    AUI_MUSIC_GET_SUBTITLE,

    /**
    Value used to get information about the <b> Program </b>
    */
    AUI_MUSIC_GET_PROGRAM,

    /**
    Value used to get information about the <b> Media Size </b>
    */
    AUI_MUSIC_GET_MEDIA_SIZE,

    /**
    Value used to get the whole collection of music information available
    */
    AUI_MUSIC_GET_MUSIC_EXT_INFO = 136,

} aui_music_ext_item_get;

/**
Enum used to specify all possible messages for a callback function.

@note   All messages are sent to an application by the function
        #aui_music_set_playend_callback, then application can perform the
        corresponding action
*/
typedef enum aui_music_message {

    /**
    Value to specify a message which indicates the music file is playing over
    */
    AUI_MUSIC_PLAY_END,

    /**
    Value to specify a message which indicates the number of messages
    */
    AUI_MUSIC_MESSAGE_MAX,

} aui_music_message;

/**
Function pointer to specify the type of callback function for the enum
#aui_music_message, where the output parameters are explained below:
- @b msg           = The type of message from the player, as defined in the enum
                     #aui_music_message
- @b pv_data       = Data corresponding to a message which tells user the detailed
                     information about either decoding or playing
- @b pv_user_data  = User data given from the member @b user_data of the struct
                     #aui_attr_music
*/
typedef void (*aui_music_message_callback) (

    aui_music_message msg,

    void *pv_data,

    void *pv_user_data

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Music Module </b> to specify its attributes available
        to be configured
        </div> @endhtmlonly

        Struct to specify the attributes of Music Module available to be
        configured
*/
typedef struct aui_attr_music {

    /**
    Member to specify the path of the file to be played by the Music Module.
    */
    unsigned char uc_file_name[128];

    /**
    Member to specify the callback function related to a music callback message,
    as per comment of the function pointer #aui_music_message_callback
    */
    aui_music_message_callback aui_music_cb;

    /**
    Member to specify the user data to be passed to the callback function related
    to a music callback message mentioned in the member @b aui_music_cb of this struct
    */
    void *user_data;

} aui_attr_music, *aui_p_attr_music;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Music Module </b> to specify miscellaneous information
        about the Music Decoder </div> @endhtmlonly

        Struct to specify miscellaneous information about the music decoder

@warning  This struct is @a deprecated then can be ignored

@note   This struct is available @a only in projects based on <b> TDS OS </b>
*/
typedef struct aui_decoder_info {

    /**
    Member to specify the <b> bit rate </b> of the decoder
    */
    unsigned long   bit_rate;

    /**
    Member to sepcify the <b> sample rate </b> of the audio stream
    */
    unsigned long   sample_rate;

    /**
    Member to specify the  <b> channel mode </b>
    */
    unsigned long   channel_mode;

} aui_decoder_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Music Module </b> to specify information related to
        the current playing music </div> @endhtmlonly

        Struct to specify information related to the current playing music

@warning  This struct is @a deprecated then can be ignored

@note   This struct is available @a only in projects based on <b> TDS OS </b>
*/
typedef struct aui_music_info {

    /**
    Member to specify the @b title of the music
    */
    char title[30];

    /**
    Member to specify the @b artist of the music
    */
    char artist[30];

    /**
    Member to specify the @b album of the music
    */
    char album[30];

    /**
    Member to specify the <b> production year </b> of the music
    */
    char year[4];

    /**
    Member to specify the <b> written comment </b> on the music
    */
    char comment[30];

    /**
    Member to specify the @b type of the music
    */
    char genre;

    /**
    Member to specify the <b> track number </b> for the music
    */
    char track;

    /**
    Member to specify the <b> total time </b> of the music
    */
    unsigned long time;

    /**
    Member to specify the <b> file size </b> of the music
    */
    unsigned long file_length;

} aui_music_info;

/// @cond

/**
Function pointer used to specify the type of <b> callback function </b>
registered by the value #AUI_MUSIC_PLAY_END of the enum #aui_music_message

*/
typedef void (*fn_music_end_callback) (

    void

    );

/// @endcond

/*****************************Global Function List*****************************/

#ifdef __cplusplus

extern "C" {

#endif

/**
@brief          Function used to open the Music Module and configure the desired
                attributes, then get the related handle

@note           This function can @a only be used in the <b> Pre-Run Stage </b>
                of the music Module, in particular:
                - Either after performing the initialization of the Music Module
                  by the function #aui_music_init for the first opening of the
                  Music Module
                - Or after closing the Music Module by the function
                  #aui_music_close, considering the initialization of the Music
                  Module has been performed previously by the function
                  #aui_music_init

@param[in]      p_music_attr        Pointer to a struct #aui_attr_music, which
                                    collects the desired attributes for the Music
                                    Module

@param[out]     p_handle_music      Pointer to the handle of the Music Module
                                    just opened

@return         @b AUI_RTN_SUCCESS  Music Module opened successfully then user
                                    can start to configure the Music module
@return         @b AUI_RTN_EINVAL   At least one parameter (i.e. [in], [out])
                                    is invalid
@return         @b others           Opening of the Music Module failed for some
                                    reasons
*/
AUI_RTN_CODE aui_music_open (

    aui_attr_music *p_music_attr,

    aui_hdl *p_handle_music

    );

/**
@brief          Function used to close the Music Module already opened by the
                function #aui_music_open, then the related handle (i.e. the
                related resources such as memory, device) will be released

@note           This function can only be used in the <b> Post-Run Stage </b>
                of the Music Module in pair with its the opening by the
                function #aui_music_open. After closing the Music Module,
                user can
                - Either perform the de-initialization of the Music Module
                  by the function #aui_music_de_init
                - Or open again the Music Module by the function
                  #aui_music_open, considering the initialization of the
                  music Module has been performed previously by the function
                  #aui_music_init

@param[in]      p_music_attr        Pointer to a struct #aui_attr_music, which
                                    collects the desired attributes for the
                                    Music Module to be closed
                                    - @b Caution: For the music Module this value
                                      is suggested to be set as "NULL" since
                                      the struct #aui_attr_music doesn't store
                                      any memory pointer in heap

@param[in]     p_handle_music       Pointer to the handle of the Music Module
                                    already opened and to be closed

@return        @b AUI_RTN_SUCCESS   Closing of the Music Module performed
                                    successfully
@return        @b AUI_RTN_EINVAL    At least one parameter (i.e. [in], [out])
                                    is invalid
@return        @b others            Closing of the Music Module failed for same
                                    reasons
*/
AUI_RTN_CODE aui_music_close (

    aui_attr_music *p_music_attr,

    aui_hdl *p_handle_music

    );

/**
@brief          Function used to @b start the Music Module already opened by
                the function #aui_music_open

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened

@return         @b AUI_RTN_SUCCESS  Starting of the Music Module  performed
                                    successfully
@return         @b AUI_RTN_EINVAL   The input parameter (i.e.[in]) is invalid
@return         @b others           Starting of the Music Module failed for
                                    some reasons
*/
AUI_RTN_CODE aui_music_start (

    aui_hdl handle

    );

/**
@brief          Function used to @b stop the Music Module already started by
                the function #aui_music_start

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened

@return         @b AUI_RTN_SUCCESS  Stopping of the Music Module performed
                                    successfully
@return         @b AUI_RTN_EINVAL   The input parameter (i.e.[in]) is invalid
@return         @b others           Stopping of the Music Module failed for
                                    some reasons
*/
AUI_RTN_CODE aui_music_stop (

    aui_hdl handle

    );

/**
@brief          Function used to @b pause the Music Module already started by
                the function #aui_music_start

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened

@return         @b AUI_RTN_SUCCESS  Pausing of the Music Module performed
                                    successfully
@return         @b AUI_RTN_EINVAL   The input parameter (i.e.[in]) is invalid
@return         @b others           Pausing of the Music Module failed for
                                    some reasons
*/
AUI_RTN_CODE aui_music_pause (

    aui_hdl handle

    );

/**
@brief          Function used to @b resume the Music Module already paused by
                the function #aui_music_pause

@param[in]      handle              Pointer to the handle of the Music Module
                                    already paused and to be resumed later

@return         @b AUI_RTN_SUCCESS  Resuming of the Music Module performed
                                    successfully
@return         @b AUI_RTN_EINVAL   The input parameter (i.e.[in]) is invalid
@return         @b others           Resuming of the Music Module failed for
                                    some reasons
*/
AUI_RTN_CODE aui_music_resume (

    aui_hdl handle

    );

/**
@brief          Function used to perform the @b initialization of the Music
                Module before its opening by the function #aui_music_open

@param[in]      fn_music_init       Callback function used for the initialization
                                    of the Music Module, as per comment for the
                                    function pointer #aui_func_music_init

@return         @b AUI_RTN_SUCCESS  Initializing of the Music Module performed
                                    successfully
@return         @b AUI_RTN_EINVAL   The input parameter (i.e.[in]) is invalid
@return         @b others           Initializing of the Music Module failed for
                                    some reasons
*/
AUI_RTN_CODE aui_music_init (

    aui_func_music_init fn_music_init

    );

/**
@brief          Function used to perform the @b De-Initialization of the Music
                Module, after its closing by the function #aui_music_close

@param[in]      fn_music_de_init    Callback function used to de-initialize the
                                    Music Module, as per comment of the function
                                    pointer #aui_func_music_init

@return         @b AUI_RTN_SUCCESS  De-initializing of the Music Module performed
                                    successfully
@return         @b AUI_RTN_EINVAL   The input parameter (i.e.[in]) is invalid
@return         @b others           De-initializing of the Music Module failed
                                    for some reasons

@note     This function is @a only used in projects based on <b> TDS OS </b>
*/
AUI_RTN_CODE aui_music_de_init (

    aui_func_music_init fn_music_de_init

    );

/**
@brief          Function used to @b seek the specific time of playing music with
                the Music Module

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened
@param[in]      ul_time_in_ms       Desired specific time (in @a millisecond
                                    (ms) unit)

@return         @b AUI_RTN_SUCCESS  Seeking of the specific time of playing music
                                    performed successfully
@return         @b AUI_RTN_EINVAL   At least one input parameters (i.e. [in]) is
                                    invalid
@return         @b others           Seeking of the specific time of playing music
                                    failed for some reasons
*/
AUI_RTN_CODE aui_music_seek (

    aui_hdl handle,

    unsigned long ul_time_in_ms

    );

/**
@brief          Function used to @b get the <b> total time </b> of the music file

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened

@param[out]     pui_total_time      Point to the total time of the music file
                                    (in @a second unit)

@return         @b AUI_RTN_SUCCESS  Getting of the total time of the music file
                                    performed successfully,
@return         @b AUI_RTN_EINVAL   At least one parameter (i.e. [in], [out])
                                    is invalid
@return         @b others           Getting of the total time of the music file
                                    failed for some reasons
*/
AUI_RTN_CODE aui_music_total_time_get (

    aui_hdl handle,

    unsigned int *pui_total_time

    );

/**
@brief          Function used to @b get the <b> current playing time </b> of
                the music file

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened

@param[out]     pui_cur_time        Pointer to the current playing time of the
                                    Music Module (in @a millisecond (ms) unit)

@return         @b AUI_RTN_SUCCESS  Getting of the current playing time of the
                                    music file performed successfully
@return         @b AUI_RTN_EINVAL   At least one parameter (i.e. [in], [out])
                                    is invalid
@return         @b others           Getting of the current playing time of the
                                    music file failed for some reasons
*/
AUI_RTN_CODE aui_music_cur_time_get (

    aui_hdl handle,

    unsigned int *pui_cur_time

    );

/**
@brief          Function used to perform a specific setting for the callback
                function to be called when the music file is at the end

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened
@param[in]      msg                 The callback function to be called, as per
                                    comment for the function pointer
                                    #fn_music_end_callback

@param[in]      mc_cb               The message for the callback function, as
                                    defined in the enum #aui_music_message

@return         @b AUI_RTN_SUCCESS  Setting performed successfully
@return         @b AUI_RTN_EINVAL   At least one parameter (i.e. [in]) is invalid
@return         @b others           Setting failed for some reasons
*/
AUI_RTN_CODE aui_music_set_playend_callback (

    aui_hdl handle,

    aui_music_message msg,

    fn_music_end_callback mc_cb

    );

/**
@warning        This function is currently @ a reserved to ALi R&D Dept.,
                please ignore it.

@brief          Function used to perform a specific setting to Music Module
                after its opening by the functions #aui_music_open

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened
@param[in]      ul_item             The item related to the specific setting to
                                    be performed to Music Module, as defined in
                                    the enum #aui_music_item_set
@param[in]      pv_param            The pointer as per the description of the
                                    specific setting to be performed to Music
                                    Module, as defined in the enum #aui_music_item_set

@return         @b AUI_RTN_SUCCESS  Setting performed successfully
@return         @b AUI_RTN_EINVAL   At least one input parameters (i.e. [in])
                                    is invalid
@return         @b others           Setting failed for some reasons
*/
AUI_RTN_CODE aui_music_set (

    aui_hdl handle,

    unsigned long ul_item,

    void *pv_param

    );

/**
@brief          Function used to get a specific setting information of the Music
                Module, as defined in the enum #aui_music_item_get, after its
                starting by function #aui_music_start

@param[in]      handle              Pointer to the handle of the Music Module
                                    already opened
@param[in]      ul_item             The item related to the specific setting to
                                    be gotten from Music Module, as defined in
                                    the enum #aui_music_item_get
@param[out]     pv_param            The pointer as per the description of the
                                    specific setting to be gotten from Music Module,
                                    as defined in the enum #aui_music_item_get

@return         @b AUI_RTN_SUCCESS  Getting of the specific setting of the Music
                                    Module performed successfully
@return         @b AUI_RTN_EINVAL   At least one input parameters (i.e. [in])
                                    is invalid
@return         @b others           Getting of the specific setting of the Music
                                    Module failed for some reasons
*/
AUI_RTN_CODE aui_music_get (

    aui_hdl handle,

    unsigned long ul_item,

    void *pv_param

    );

#ifdef __cplusplus

}

#endif

#endif

/* END OF FILE */


