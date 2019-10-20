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
Current Author:     Lisa.Liu
Last update:        2017.05.08
-->

@file   aui_pvr.h

@brief  Personal Video Recorder (PVR) Module

        <b> Personal Video Recorder (PVR) Module </b> is used to record and play
        back TV/Radio programs.\n\n

        This module support three (3) functions:
         - Recording
         - Playback
         - Time Shift

        About the <b> Recording Process </b>:
        - The PVR engine set a callback to demux device and the program data will
          be copied once and sent to PVR engine (the program data is stored in a
          storage disk (USB disk))
        - If the program is FTA, the program data could be stored as plain text
          in a storage disk
        - If the program is scrambled, the program data could be decrypted and
          then encrypted with specific algorithms and stored in a storage device

        About the <b> Playback Process </b>:
        - PVR engine read the program data from a storage device and send it to
          demux device then, the demux device sends data to A/V decoder for
          displaying
        - If the program data is in plain text form, it will be sent to demux
          device directly
        - If the program data is scrambled, the data will be sent into
          descrambler device by the demux device for decryption. After that it
          will be sent to A/V decoder for displaying.\n\n

        @b Caution: For the hard resources saving, the software demux device
                    (i.e. <b> dmx_id = 2 </b>) is recommended for playback

        About the <b> Time Shift Process </b>, user can play back the program
        while the program is recording and
        - Firstly, PVR engine records the data to a storage disk as the record
          process
        - Secondly, PVR play the data as the playback process

@note For further details, please refer to ALi document
      <b><em>
      ALi_AUI_Porting_Guide_Modules.pdf - Chapter "Personal Video Recorder
      (PVR) Module"
      </em></b>

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_PVR_H

#define _AUI_PVR_H

#ifdef __cplusplus

extern "C" {

#endif

/*************************Included Header File List***************************/

#include "aui_common.h"

#include "aui_deca.h"

#include <time.h>

/*******************************Global Macro List*****************************/

/**
Macro to indicate the <b> version number </b> of PVR Module, which is a
hexadecimal number that consist of three (3) reading parts (sorted from left to
right) as exemplified below:\n\n

Version number 0x00000011, where
- 0x0000 = <b> main version </b>
- 00 =  <b> 1st level sub-version </b>
- 11 = <b> 2nd level sub-version </b>
*/
#define AUI_PVR_VERSION_    (0X00000011)

/**
Macro to specify the maximum name lenght of the PVR disk partition to be mounted
*/
#define AUI_PVR_MOUNT_NAME_LEN_MAX   (16)

/**
Macro to specify the <b> maximum name length </b> of a <b> PVR recording file </b>

@note   The actual file name length needs to be less than this value otherwise
        the recording could not start.\n
        The file name can be set by the user and will be used when recording
        starts.
*/
#define AUI_PVR_FILE_NAME_LEN_MAX   (256)

/**
Macro to specify the maximum <b> folder path length </b> of a <b> PVR recording
file </b>

@note   The actual folder path length needs to be less than this value otherwise
        the recording could not start.\n
        PVR Engine will calculate that length as length of the PVR recording
        file name plus the length of the attached PVR storage device path. \n
        The folder path will be used when a recording starts.
*/
#define AUI_PVR_FILE_PATH_LEN_MAX   (1024)

/**
Macro to specify the <b> maximum PVR channel name length </b>

@note   The actual channel name length needs to be less than this value otherwise
        the recording could not start.\n
        The channel name can be set by user and it will be saved into the meta
        data. As suggestion, the channel name can be set to either the channel
        or program.
*/
#define AUI_PVR_SERVICE_NAME_LEN_MAX    (36)

/**
Macro to specify the <b> maximum PVR event name length
*/
#define AUI_PVR_EVENT_NAME_LEN_MAX    (70)
/**
Macro to specify <b> FTA Stream </b> as recording type

@note   This macro is used for setting the record special mode so that it can
        fully fill the requirement of the @b CAS
*/
#define AUI_PVR_NONE                    0

/**
Macro to specify <b> CONAX AS </b> re-encryption mode as @b single recording
type

@note   This macro is used for setting the record special mode so that it can
        fully fill the requirement of the @b CAS
*/
#define AUI_PVR_CAS9_RE_ENCRYPTION      1

/**
Macro to specify @b CIPLUS re-encryption mode as recording type

@note   This macro is used for setting the record special mode so that it can
        fully fill the requirement of the @b CAS
*/
#define AUI_PVR_CIPLUS_RE_ENCRYPTION    2

/**
Macro to specify <b> Software Security </b> re-encrypt mode as recording type

@note   This macro is used for setting the record special mode so that it can
        fully fill the requirement of the @b CAS
*/
#define AUI_PVR_COMMON_RE_ENCRYPTION    3

/**
Macro to specify <b> Only Fingerprint </b> re-encryption mode as recording type

@note   This macro is used for setting the record special mode so that it can
        fully fill the requirement of the @b CAS
*/
#define AUI_PVR_FINGERP_RE_ENCRYPTION   4

/**
Macro to specify <b> CONAX AS </b> re-encryption mode as @b multiple recording
type

@note   @b 1. At present, @a multiple means @a only two (2) channel
@note   @b 2. This macro is used for setting the record special mode so that
              it can fully fill the requirement of the @b CAS
*/
#define AUI_PVR_CAS9_MULTI_RE_ENCRYPTION 5

/**
Macro to specify <b> VERMATRIX AS </b> re-encryption mode as @b multiple
recording type

@note   @b 1. At present, @a multiple means @a only two (2) channel
@note   @b 2. This macro is used for setting the record special mode so that
              it can fully fill the requirement of the @b CAS.
*/
#define AUI_PVR_VMX_MULTI_RE_ENCRYPTION 6

/**
Macro to specify <b> NAGRA AS </b> re-encryption mode as @b multiple recording
type

@note   @b 1. At present, @a multiple means @a only two (2) channel
@note   @b 2. This macro is used for setting the record special mode so that
              it can fully fill the requirement of the @b CAS.
*/
#define AUI_RSM_C0200A_MULTI_RE_ENCRYPTION 7

/**
Macro to specify <b> General AS </b> re-encryption mode as @b multiple recording
type

@note   @b 1. At present, @a multiple means @a only two (2) channel
@note   @b 2. This macro is used for setting the record special mode so that it
              can fully fill the requirement of the @b CAS
*/
#define AUI_RSM_GEN_CA_MULTI_RE_ENCRYPTION 8
/**
Macro to specify the @b Index of the <b> Time Shift Record </b>

@note   While the storage device is attached for the PVR process, the record
        items in the PVR folder will be traversed.\n
        At the same time, the indexes of the record items will be assigned for
        getting record information conveniently.\n
        For differing from the normal record, the index of the Time Shift
        Record is a fixed one
*/
#define AUI_TMS_INDEX           1

/**
Macro to specify the <b> maximum number of bookmarks </b> in a record item

@note   During playback a record, a bookmarks can be set in the record item for
        marking specific time then the PVR can jump to play between bookmarks.\n
        When the total number of bookmark in a record item reaches the maximum
        allowed, further bookmarks can not be added
*/
#define AUI_MAX_BOOKMARK_NUM    20

/**
Macro to specify the <b> maximum number of Audio PIDs </b>

@note   Since the DMX filter is limited, the number of Audio PIDs needs to be
        limited as well.\n
        When the total number of Audio PIDs reaches the maximum allowed, further
        Audio PIDs will be cut out.
*/
#define AUI_MAX_PVR_AUDIO_PID   10

/**
Macro to specify the <b> maximum number of ECM PIDs </b>
.
@note   Since the DMX filter is limited, the number of ECM PIDs needs to be
        limited as well. \n
        When the total number of ECM PIDs reaches the maximum allowed, further
        ECM PIDs will be cut out.
*/
#define AUI_MAX_PVR_ECM_PID     16

/**
Macro to specify the <b> maximum number of EMM PIDs </b>
.
@note   Since the DMX filter is limited, the number of ECM PIDs needs to be
        limited as well. \n
        When the total number of EMM PIDs reaches the maximum allowed, further
        EMM PIDs will be cut out.
*/
#define AUI_MAX_PVR_EMM_PID     16

/**
Macro to specify the <b> maximum number of Subtitle PIDs </b>
.
@note   Since the DMX filter is limited, the number of Subtitles PIDs needs to
        be limited as well. \n
        When the total number of Subtitles PIDs reaches the maximum allowed,
        further Subtitles PIDs will be cut out.
*/
#define AUI_MAX_PVR_SUBT_PID        11

/**
Macro to specify the <b> maximum number of TTX PIDs </b>
.
@note   Since the DMX filter is limited, the number of TTX PIDs needs to be
        limited as well. \n
        When the total number of TTX PIDs reaches the maximum allowed, further
        TTX PIDs will be cut out.
*/
#define AUI_MAX_PVR_TTX_PID     11

/**
Macro to specify the <b> maximum number of TTX Subtitle PIDs </b>
.
@note   Since the DMX filter is limited, the number of TTX subtitle PIDs needs
        to be limited as well. \n
        When the total number of TTX Subtitle PIDs reaches the maximum allowed,
        further TTX Subtitle PIDs will be cut out.
*/
#define AUI_MAX_PVR_TTX_SUBT_PID    11

/**
Macro to specify the <b> maximum number of ISDBTCC PIDs </b>
.
@note   Since the DMX filter is limited, the number of ISDBTCC PIDs needs
        to be limited as well. \n
        When the total number of ISDBTCC PIDs reaches the maximum allowed,
        further ISDBTCC PIDs will be cut out.
*/
#define AUI_MAX_PVR_ISDBTCC_PID    8
/**
Macro to specify the <b> maximum number of expired items </b>
.
@note   Since the age limit of an item is a feature not implemented in ALI AUI,
        - When recording some expired channels, the age limit information can
          be saved into the PVR meta data
        - During playback an expired PVR item, the age limit information can be
          gotten out by the play time
@note   then the UI layer can do some process, such as close the screen
*/
#define AUI_MAX_AGELIMIT_NUM    512

/**
Macro to specify the <b> maximum number of rating control information </b>,
in particular
- When recording some rating control channel, the rating control information
  can be saved into the PVR meta data.\n
- During playback a rating control PVR item, the rating control information can
  be gotten out by the play time

then the UI layer can do some process, such as close the screen, show password
input frame, etc.
*/
#define AUI_MAX_RATINGCTL_NUM       256

/**
Macro to specify the <b> maximum number of disk supported by PVR </b>

@note   Storage devices are used in a PVR application, and their usage may be
        for TMS or recording.\n
        Although many storage devices can be mounted in the OS layer, @a only a
        certain number of them can be used in PVR.
*/
#define AUI_MAX_PVR_DISK_NUM        2

/**
Macro to specify that the <b> disk usage mode </b> is <b> normal recording <b>
and </b> time shifting </b>

@note   The disk usage mode needs to be specified when attaching a storage device
        to PVR layer.\n
        In this mode, the free disk size will be split into
        - Record part
        - Time shift part.
        directly by PVR.
*/
#define AUI_PVR_REC_AND_TMS_DISK    0

/**
Macro to specify that the <b> disk usage mode </b> is <b> normal recording only </b>

@note   The disk usage mode needs to be specified when attaching a storage device
        to PVR layer.\n
        In this mode, all the free disk size will be used for normal recording only
        by PVR
*/
#define AUI_PVR_REC_ONLY_DISK       1

/**
Macro to specify that the <b> disk usage mode </b> is <b> TMS only </b>

@note   The disk usage mode needs to be specified when attaching a storage device
        to PVR layer.\n
        In this mode, all the free disk size will be used for normal TMS only by PVR
*/
#define AUI_PVR_TMS_ONLY_DISK       2

/**
Macro to specify the size of the header information to store

@note   PVR can store some information generated by CAS library, the maximum length
        of header information is limited to the value #AUI_PVR_INFO_HEADER_SIZE
*/
#define AUI_PVR_INFO_HEADER_SIZE 248

/**
Macro to specify the size of the data information to store

@note   PVR can store some information generated by CAS library, the maximum length
        of data information is limited to the value #AUI_PVR_STORE_INFO_DATA_SIZE_MAX
*/
#define AUI_PVR_STORE_INFO_DATA_SIZE_MAX 64*1024

/**
Macro to specify the size of a quantum, which is the minimum length of the TS
chunk processed by PVR
*/
#define AUI_PVR_QUANTUM_SIZE             (47*1024)

/**
Macro to specify whether or not the notification messages
- #AUI_EVNT_PVR_END_DATAEND
- #AUI_EVNT_PVR_END_REVS
defined in the enum #aui_evnt_pvr will be processed by AUI PVR internally.\n
That depends on whether or not the member @b start_mode of the struct
#aui_ply_param will be masked to this macro by the user. Therefore:
- @b start_mode masked to this macro --> The two notification messages above
  will NOT be processed by AUI PVR Module
- @b start_mode NOT masked to this macro --> The two notification messages above
  will be processed by AUI PVR Module
*/
#define AUI_PVR_PB_STOP_MODE             (1<<7)

/*******************************Global Type List*******************************/

/**
Enum to specify different <b> Stream Type </b> available to be recorded
*/
typedef enum aui_pvr_rec_type {

    /**
    Value to specify <b> Transport Stream </b> type
    */
    AUI_PVR_REC_TYPE_TS = 0,

    /**
    Value to specify <b> Program Stream </b> type

    @note   At present, this stream type is not supported
    */
    AUI_PVR_REC_TYPE_PS = 1

} aui_pvr_rec_type;

/**
Enum to specify different <b> Project Mode </b> available to be managed
*/
typedef enum aui_pvr_project_mode {

    /**
    Value to specify @b DVBS-2 standard project mode
    */
    AUI_PVR_DVBS2   = (1<<0),

    /**
    Value to specify @b DVBT standard project mode
    */
    AUI_PVR_DVBT    = (1<<1),

    /**
    Value to specify @b DVBS-2M standard project mode
    */
    AUI_PVR_DVBT_2M = (1<<2),

    /**
    Value to specify @b DVBC standard project mode
    */
    AUI_PVR_ATSC    = (1<<3)

} aui_pvr_project_mode;

/**
Enum to specify different <b> PVR Recording Mode </b>
*/
typedef enum aui_pvr_rec_mode {

    /**
    Value to specify @b Normal recording mode
    */
    AUI_PVR_REC_MODE_NORMAL = 0,

    /**
    Value to specify <b> TMS Time Shift </b> recording mode
    */
    AUI_PVR_REC_MODE_TMS,

    /**
    Value to specify that all data in the current Transport Stream will be
    write to the local file

    @note   At the moment, this mode is not supported
    */
    AUI_PVR_REC_MODE_TP

} aui_pvr_rec_mode;

/**
Enum to specify different <b> Content Playback View Mode </b> available to be
set
*/
typedef enum aui_pvr_play_mode {

    /**
    Value to specify <b> Full Screen </b> view mode
    */
    AUI_PVR_PLAY_MODE_FULL_SCREEN = 0,

    /**
    Value to specify @b Preview view mode
    */
    AUI_PVR_PLAY_MODE_PREVIEW

} aui_pvr_play_mode;

/**
Enum to specify different <b> mode to stop playing </b> a recorded content

@note   This enum is used to play a recorded content time to time repeatedly
        (e.g fast backward to the head then restart to play)
*/
typedef enum aui_pvr_stop_mode {

    /**
    Deprecadted, User should always use #AUI_PVR_STOPPED_ONLY
    */
    AUI_PVR_STOP_AND_REOPEN   = 0,

    /**
    Value to specify that the PVR device will be stopped only
    */
    AUI_PVR_STOPPED_ONLY      = 1

} aui_pvr_stop_mode;

/**
Enum to specify different <b> content recording status </b>
*/
typedef enum aui_pvr_rec_state {

    /**
    Value to specify that the content recording is in @b progress
    */
    AUI_PVR_REC_STATE_RECORDING =0,

    /**
    Value to specify that the content recording is @b paused
    **/
    AUI_PVR_REC_STATE_PAUSE,

    /**
    Value to specify that the content recording is @b stopped
    */
    AUI_PVR_REC_STATE_STOP

} aui_pvr_rec_state;

/**
Enum to specify different <b> player states </b>
*/
typedef enum aui_pvr_play_state {

    /**
    Value to specify that the player is @b stopped
    */
    AUI_PVR_PLAY_STATE_STOP          = 0,

    /**
    Value to specify that the player is <b> working normally </b>
    */
    AUI_PVR_PLAY_STATE_PLAY          = 1,

    /**
    Value to specify that the player is @b paused.
    */
    AUI_PVR_PLAY_STATE_PAUSE         = 2,

    /**
    Value to specify that the player is in the <b> fast forward </b> state
    */
    AUI_PVR_PLAY_STATE_FF            = 3,

    /**
    Value to specify that the player is in the <b> step-by-step </b> state
    */
    AUI_PVR_PLAY_STATE_STEP          = 4,

    /**
    Value to specify that the player is in the <b> fast backward </b> state
    */
    AUI_PVR_PLAY_STATE_FB            = 5,

    /**
    Value to specify that the player is in the <b> slow </b> state
    */
    AUI_PVR_PLAY_STATE_SLOW          = 6,

    /**
    Value to specify that the player is in the <b> reverse slow </b> state
    */
    AUI_PVR_PLAY_STATE_REVSLOW       = 7,

    /**
    Value to specify the setting of a proper speed for the player, as per values
    defined in the enum #AUI_PlayerSpeed_E
    */
    AUI_PVR_PLAY_STATE_SPEED          =8,

} aui_pvr_play_state;

/**
Enum to specify different <b> speeds for the player </b>
*/
typedef enum aui_player_speed {

    /**
    Value to specify <b> normal speed </b> (i.e. @b 1x)
    */
    AUI_PLAYER_SPEED_NORMAL= 0,

    /**
    Value to specify <b> fast rewind </b> with speed @b 64x
    */
    AUI_PLAYER_SPEED_FASTREWIND_64,

    /**
    Value to specify <b> fast rewind </b> with speed @b 32x
    */
    AUI_PLAYER_SPEED_FASTREWIND_32,

    /**
    Value to specify <b> fast rewind </b> with speed @b 16x
    */
    AUI_PLAYER_SPEED_FASTREWIND_16,

    /**
    Value to specify <b> fast rewind </b> with speed @b 8x
    */
    AUI_PLAYER_SPEED_FASTREWIND_8,

    /**
    Value to specify <b> fast rewind </b>  with speed @b 4x
    */
    AUI_PLAYER_SPEED_FASTREWIND_4,

    /**
    Value to specify <b> fast rewind </b> with speed @b 2x
    */
    AUI_PLAYER_SPEED_FASTREWIND_2,

    /**
    Value to specify <b> fast rewind </b> with speed @b 1x
    */
    AUI_PLAYER_SPEED_FASTREWIND_1,	

    /**
    Value to specify <b> slow rewind </b> wind speed @b 1/2
    */
    AUI_PLAYER_SPEED_SLOWREWIND_2,

    /**
    Value to specify <b> slow rewind </b> with speed @b 1/4
    */
    AUI_PLAYER_SPEED_SLOWREWIND_4,

    /**
    Value to specify <b> slow rewind </b> with speed @b 1/8
    */
    AUI_PLAYER_SPEED_SLOWREWIND_8,

    /**
    Value to specify <b> slow forward </b> with speed @b 1/2
    */
    AUI_PLAYER_SPEED_SLOWFORWARD_2,

    /**
    Value to specify <b> slow forward </b> with speed @b 1/4
    */
    AUI_PLAYER_SPEED_SLOWFORWARD_4,

    /**
    Value to specify <b> slow forward </b> with speed @b 1/8
    */
    AUI_PLAYER_SPEED_SLOWFORWARD_8,

    /**
    Value to specify <b> fast forward </b> with speed @b 2x
    */
    AUI_PLAYER_SPEED_FASTFORWARD_2,

    /**
    Value to specify <b> fast forward </b> with speed @b 4x
    */
    AUI_PLAYER_SPEED_FASTFORWARD_4,

    /**
    Value to specify <b> fast forward </b> with speed @b 8x
    */
    AUI_PLAYER_SPEED_FASTFORWARD_8,

    /**
    Value to specify <b> fast forward </b> with speed @b 16x
    */
    AUI_PLAYER_SPEED_FASTFORWARD_16,

    /**
    Value to specify <b> fast forward </b> with speed @b 32x
    */
    AUI_PLAYER_SPEED_FASTFORWARD_32,

    /**
    Value to specify <b> fast forward </b> with speed @b 64x
    */
    AUI_PLAYER_SPEED_FASTFORWARD_64,

    /**
    Value to specify the total number of items in this enum,
    i.e. speeds available to be set
    */
    AUI_PLAYER_SPEED_MAX

} aui_player_speed;

/**
Enum to specify the <b> PID type </b>
*/
typedef enum aui_pvr_pid_type {

    /**
    Value to specify @b default PID type when it is not neither Video nor Audio
    */
    AUI_PVR_PID_DEFUALT_TYPE = 0,

    /**
    Value to specify @b MPEG video format PID type
    */
    AUI_PVR_VIDEO_MPEG = 1,

    /**
    Value to specify @b H264 video format PID type
    */
    AUI_PVR_VIDEO_AVC,

    /**
    Value to specify @b MPEG1 audio format PID type
    */
    AUI_PVR_AUDIO_MPEG1,

    /**
    Value to specify @b MPEG2 audio format PID type
    */
    AUI_PVR_AUDIO_MPEG2,

    /**
    Value to specify @b AAC audio format PID type
    */
    AUI_PVR_AUDIO_MPEG_AAC,

    /**
    Value to specify @b AC-3 audio format PID type
    */
    AUI_PVR_AUDIO_AC3,

    /**
    Value to specify <b> DTS (for DVD-video) </b> audio format PID type
    */
    AUI_PVR_AUDIO_DTS,

    /**
    Value to specify <b> Packet PCM (for DVD-audio) </b> audio format PID type
    */
    AUI_PVR_AUDIO_PPCM,

    /**
    Value to specify <b> Liner PCM (for DVD-video) </b> audio format PID type
    */
    AUI_PVR_AUDIO_LPCM_V,

    /**
    Value to specify <b> Linear PCM (for DVD-audio) </b> audio format PID type
    */
    AUI_PVR_AUDIO_LPCM_A,

    /**
    Value to specify @b PCM audio format PID type
    */
    AUI_PVR_AUDIO_PCM,

    /**
    Value to specify @b BYE1 audio format PID type
    */
    AUI_PVR_AUDIO_BYE1,

    /**
    Value to specify <b> Real Audio 8 </b> audio format PID type
    */
    AUI_PVR_AUDIO_RA8,

    /**
    Value to specify @b MP3 audio format PID type
    */
    AUI_PVR_AUDIO_MP3,

    /**
    Value to specify @b MPEG_ADTS_AAC audio format PID type
    */
    AUI_PVR_AUDIO_MPEG_ADTS_AAC,

    /**
    Value to specify @b OGG audio format PID type
    */
    AUI_PVR_AUDIO_OGG,

    /**
    Value to specify @b EC3 audio format PID type
    */
    AUI_PVR_AUDIO_EC3,

    /**
    Value to specify <b> MP3 layer 3 </b> audio format PID type
    */
    AUI_PVR_AUDIO_MP3_L3,

    /**
    Value to specify <b> RAW PCM </b> audio format PID type
    */
    AUI_PVR_AUDIO_RAW_PCM,

    /**
    Value to specify @b MP3_2 audio format PID type
    */
    AUI_PVR_AUDIO_MP3_2,

    /**
    Value to specify <b> audio TeleText </b> PID type
    */
    AUI_PVR_TTX_PID,

    /**
    Value to specify <b> audio Subtitle </b> PID type
    */
    AUI_PVR_SUBTITLE_PID,

    /**
    Value to specify @b invalid audio format PID Type
    */
    AUI_PVR_AUDIO_INVALID,

    /**
    Value to specify other PID type not mentioned in this enum

    @note   This value specify also the end of this enum
    */
    AUI_PVR_PID_TYPE_UNKONW

} aui_pvr_pid_type;

/**
Enum to specify the information related to
- PVR recorder
- PVR recording file
- PVR player

available to be set/gotten

@note   This enum is used by the functions
        - #aui_pvr_set
        - #aui_pvr_get

@note   where their input parameters
        - @b p_param_1
        - @b p_param_2
        - @b p_param_3

@note   take the value as per the description of each specific value of this enum

@note   @b Caution: Usually, @b p_param_2 and @b p_param_3 are reserved to ALi
           R&D Dept. then they take NULL as value unless otherwise notice
*/
typedef enum aui_pvr_cmd {

    /**
    Value used to get the <b> content recording state </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_STATES =0,

    /**
    Value used to get the <b> content recording DMX ID </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_DMX_ID,

    /**
    Value used to get the <b> recording time </b> (in @b second unit)

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_TIME_S,

    /**
    Value used to get the <b> recording time </b> (in @a millisecond (ms))

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_TIME_MS,

    /**
    Value used to get the <b> recording time shift </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_TIME_TMS,

    /**
    Value used to get the <b> recording (TMS) size </b> (in @a kB unit)

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_SIZE_KB,

    /**
    Value used to get the <b> recording (TMS) capability </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_CAPBILITY,

    /**
    Value used to get the <b> player state </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_STATES,

    /**
    Value used to get the <b> player speed </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_SPEED,

    /**
    Value used to get the <b> play direction </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_DIRECTION,

    /**
    Value used to get the <b> play time </b> (in @a second unit)

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_TIME_S,

    /**
    Value used to get the <b> play time </b> (in @a millisecond (ms))

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_TIME_MS,

    /**
    Value used to get the <b> bit rate </b> (in @a bits per second (bps)) of
    the playing file

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_BITRATE,

    /**
    Value used to get the <b> play position </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_POS,

    /**
    Value used to get the <b> play item path </b>
    (i.e. path of the PVR recording file)

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_PATH,

    /**
    Value used to get the <b> play item index </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_INDEX,

    /**
    Value used to get the <b> disk configuration </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_DISK_CONFIG,

    /**
    Value used to get the <b> PVR version </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_VERSION,

    /**
    Value used to get the <b> current recording item number </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEMS_NUM,

    /**
    Value used to get the <b> recording item duration </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEM_DURATION,

    /**
    Value used to set the <b> Maximum Time Shift Length </b>

    @note   @b p_param_1 takes that information

    @attention    About the maximum value of timeshiting lenght:
                  - It is @b 3600 seconds in Advanced Security (AS) projects
                  - It is limited to the available space on the disk in other
                    projects
    */
    AUI_PVR_REC_MAX_TMS_LENGTH,

    /**
    Value used to get the <b> total number </b> of <b> Audio PIDs </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_AUDIO_PID_COUNT,

    /**
    Value used to get the <b> Audio PID list </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_AUDIO_PID_LIST,

    /**
    Value used to set (change) the <b> audio PID </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_AUDIO_PID_CHANGE,

    /**
    Value used to set (change) the <b> audio channel </b>

    @warning  This value refers to a deprecated command, please use function
              #aui_snd_channel_set instead
    */
    AUI_PVR_AUDIO_CHANNEL_CHANGE,

    /**
    Value used to set/get the <b> PID information </b>

    @note   @b p_param_1 takes the pointer to the struct #aui_pvr_pid_info
    */
    AUI_PVR_PID_INFO,

    /**
    Value used to set (replace) the <b> PVR PID </b>

    @note   @b 1. @b p_param_1 takes the PID length
    @note   @b 2. @b p_param_2 takes the new PID list
    @note   @b 3. @b p_param_3 takes the old PID list
    */
    AUI_PVR_PID_REPLACE,

    /**
    Value used to set (change) the <b> PVR PID </b>

    @note   @b 1. @b p_param_1 takes the PID length
    @note   @b 2. @b p_param_2 takes the new PID list
    @note   @b 3. @b p_param_3 takes the old PID list
    */
    AUI_PVR_PID_CHANGE,

    /**
    Value used to set (add) the <b> PVR PID </b>

    @note   @b 1. @b p_param_1 takes the PID length
    @note   @b 2. @b p_param_2 takes the new PID list
    @note   @b 3. @b p_param_3 takes the old PID list
    */
    AUI_PVR_PID_ADD,

    /**
    Value used to set the <b> minimum disk free size </b> for the disk usage as
    per the macro #AUI_PVR_REC_AND_TMS_DISK

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_FREE_SIZE_LOW_THRESHOLD,

    /**
    Value used to get the <b> recording item duration </b> by the <b> recording
    index </b>\n

    @note   @b 1. @b p_param_1 takes the address of the recording index
    @note   @b 2. @b p_param_2 takes the address of the recording duration
    */
    AUI_PVR_ITEM_DURATION_BY_INDEX,

    /**
    Value used to get the <b> recording (TMS) size </b> by the recording
    index \n

    @note   @b 1. @b p_param_1 takes the address of the recording index
    @note   @b 2. @b p_param_2 takes the address of the recording (TMS) size
                  (in @a kB unit)
    */
    AUI_PVR_REC_SIZE_KB_BY_INDEX,

    /**
    Value used to to set (delete) the <b> PVR record </b> by the <b> recording
    index </b> \n

    @note   @b p_param_1 takes the index of the PVR record to be deleted
    */
    AUI_PVR_DELETE_RECORD_BY_INDEX,

    /**
    Value to specify the option <b> Header of stored information </b>
    */
    AUI_PVR_INFO_HEADER,

    /**
    Value to specify the option <b> Header of stored information by index </b>
    */
    AUI_PVR_INFO_HEADER_BY_INDEX,

    /**
    Value to specify the option <b> Main data of stored information </b>
    */
    AUI_PVR_INFO_DATA,

    /**
    Value to specify the <b> PVR Record Index </b> by the <b> Position Order
    Number </b> of multiple record files in the USB device.

    @note To be highlighted:
          - The <b> PVR Record Index </b> is an internal identifier of a record
            file and takes a random value
          - The <b> Position Order Number </b> of a record file start from zero (0)
            and takes continuous.
          - The value of PVR Record Index is paired with the value of Position Order
            Number for each record file
          - If a record file is cancelled, the PVR Record Index doesn't change
            contrarily the value of Position Order Number changes in order to be
            always continuous.

    @note @b Example. \n
          File_1   --> Index = 4, Position order number = 0 \n
          File_2   --> Index = 7, Position order number = 1 \n
          File_3   --> Index = 2, Position order number = 2 \n
          File_4   --> Index = 6, Position order number = 3 \n
          File_5   --> Index = 5, Position order number = 4 \n

    @note If File_3 is cancelled then:\n
          File_1   --> Index = 4, Position order number = 0 \n
          File_2   --> Index = 7, Position order number = 1 \n
          File_4   --> Index = 6, Position order number = 2 \n
          File_5   --> Index = 5, Position order number = 3 \n

    @note So, with this enum value (command) is possible to get the PVR Record
          Index by the Position Order Number of a record file, and is passed to
          @b p_param_1
    */
    AUI_PVR_RECORD_INDEX_BY_POS,

    /**
    Value to specify the option <b> Main data of stored information by index </b>
    */
    AUI_PVR_INFO_DATA_BY_INDEX,

    /**
    Value used to get/set the <b> PVR date block mode </b>
    */
    AUI_PVR_BLOCK_MODE,

    /**
    Value used to get/set the <b> PVR date block size </b>
    */
    AUI_PVR_BLOCK_SIZE,

    /**
    Value used to get/set the <b> URI item information </b>
    */
    AUI_PVR_NV_URI,

    /**
    Value used to get the <b> URI item number </b>
    */
    AUI_PVR_NV_URI_COUNT,

    /**
    Value used to get the <b> URI item information by index </b>
    */
    AUI_PVR_NV_URI_SET,

    /**
    Value used to get/set the <b> URI external item information </b>
    */
    AUI_PVR_NV_URI_EXT,

    /**
    Value used to get the <b> URI External item number </b>
    */
    AUI_PVR_NV_URI_COUNT_EXT,

    /**
    Value used to get the <b> URI external item information by index </b>
    */
    AUI_PVR_NV_URI_SET_EXT,

    /**
    Value used to get/set the <b> credential data </b>
    */
    AUI_PVR_NV_CREDENTIAL_DATA,

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

    /**
    Value used to get the <b> TS route </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_TS_ROUTE,

    /**
    Value used to get the <b> PVR Debug mode </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_DEBUG_MODE,

    /**
    Value used to set/get the <b> PVR item information </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEM_INFO,

    /**
    Value used to set/get the <b> re-encryption random key </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEM_KEY_INFO,

    /**
    Value used to set/get the <b> re-encryption fingerprint </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEM_FINGER_INFO,

    /**
    Value used to set/get the <b> re-encryption rating lock information </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEM_RATING_INFO,

    /**
    Value used to set/get the <b> re-encryption general manager information </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_ITEM_GM_INFO,

    /**
    Value used to get the <b> play time DMX ID </b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_PLAY_DMX_ID,

    /**
    Value used to get the <b> recording mode <b>

    @note   @b p_param_1 takes that information
    */
    AUI_PVR_REC_MODE_T,

#endif

/// @endcond

    /**
    Value used to set the <b> time shift record mode </b> to <b> normal </b> value

    @note   @b p_param_1 returns the recorded valid time (in @a second (s) unit)
    */
    AUI_PVR_REC_TMS_TO_REC,

    /**
    Value as a @a flag used to set the <b> PVR information lock </b>, in particular:
    - @b 0 = Not locked
    - @b 1 = Locked
    */
    AUI_PVR_SET_LOCK_FLAG,

    /**
    Value to specify the <b> record item information </b> as per the member
    @b index of the struct #aui_pvr_rec_item_info
    */
    AUI_PVR_REC_ITEM_INFO

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                   END                                     */
/*****************************************************************************/

} aui_pvr_cmd;

/**
Enum to specify different <b> notification messages </b> from @b PVR Module to
@b Application/User Interface
*/
typedef enum aui_evnt_pvr {

    /**
    Value to specify that the player cannot playback more data from the recorded
    file cause the end-of-file condition occurred (i.e. @b EOF)
    */
    AUI_EVNT_PVR_END_DATAEND                =1,

    /**
    Value to specify that the HDD is @b full
    */
    AUI_EVNT_PVR_END_DISKFULL               =2,

    /**
    Value to specify that the player is catching up the recorder,
    i.e. the player is working in the <b> Time Shift Mode </b>
    */
    AUI_EVNT_PVR_END_TMS                    =3,

    /**
    Value to specify that the player is getting the beginning of the recorder
    file, i.e. the player is working in the <b> Backward Mode </b>
    */
    AUI_EVNT_PVR_END_REVS                   =4,

    /**
    Value to specify that the @b writing of the record file @b failed for some
    reasons
    */
    AUI_EVNT_PVR_END_WRITEFAIL              =9 ,

    /**
    Value to specify that the @b reading of the recorded file failed for some
    reasons
    */
    AUI_EVNT_PVR_END_READFAIL               =10 ,

    /**
    Value to specify <b> time-shifting overlapped </b>
    */
    AUI_EVNT_PVR_TMS_OVERLAP                =12,

    /**
    Value to specify that the @b status of the @b recorder/player has been
    updated
    */
    AUI_EVNT_PVR_STATUS_UPDATE              =13 ,

    /**
    Value to specify that the status of the <b> recording/playing file changed </b>
    */
    AUI_EVNT_PVR_STATUS_FILE_CHG            =14,

    /**
    Value to specify that the status of the @b PID changed when playing file

    @note   This notification message can occur when recording/playback a Dynamic
            PID channel
    */
    AUI_EVNT_PVR_STATUS_PID_CHG             =15,

    /**
    Value to specify that the @b speed of the PVR is <b> too low </b>
    */
    AUI_EVNT_PVR_SPEED_LOW                  =16,

    /**
    Value to specify that the <b> channel changed </b> when playing one record

    @note   When recording in append mode, the different channel Transport Stream
            packages can be record in one record item.\n
            In this case, the channel may change during playback then this
            notification message will be sent to the UI layer
    */
    AUI_EVNT_PVR_STATUS_CHN_CHG             =17,

    /**
    Value to specify that the <b> recorder started </b> to work
    */
    AUI_EVNT_PVR_MSG_REC_START              =18,

    /**
    Value to specify that the <b> recorder stopped </b> to work
    */
    AUI_EVNT_PVR_MSG_REC_STOP               =19,

    /**
    Value to specify that the <b> player started </b> to work
    */
    AUI_EVNT_PVR_MSG_PLAY_START             =20,

    /**
    Value to specify that the <b> player stopped </b> to work
    */
    AUI_EVNT_PVR_MSG_PLAY_STOP              =21,

    /**
    Value to specify that the <b> PVR updated </b> the @b key
    */
    AUI_EVNT_PVR_MSG_UPDATE_KEY             =22,

    /**
    Value to specify that the <b> PVR changed </b> the <b> control word (CW) </b>
    */
    AUI_EVNT_PVR_MSG_UPDATE_CW              =23,

    /**
    Value to specify <b> time shift capability updated </b>
    */
    AUI_EVNT_PVR_MSG_TMS_CAP_UPDATE         =24,

    /**
    Value to specify that the <b> recorder started </b> to work and got the related
    @b handle
    */
    AUI_EVNT_PVR_MSG_REC_START_GET_HANDLE   =25,

    /**
    Value to specify that the <b> recorder started a DMX device </b>
    */
    AUI_EVNT_PVR_MSG_REC_START_OP_STARTDMX  =26,

    /**
    Value to specify that the <b> player started </b> to work and got the related
    handle
    */
    AUI_EVNT_PVR_MSG_PLAY_START_GET_HANDLE  =27,

    /**
    Value to specify that the <b> player started a DMX device </b>
    */
    AUI_EVNT_PVR_MSG_PLAY_START_OP_STARTDMX =28,

    /**
    Value to specify that the <b> player stopped a DMX device </b>
    */
    AUI_EVNT_PVR_MSG_PLAY_STOP_OP_STOPDMX   =29,

    /**
    Value to specify <b> play time (PTM) updated </b>

    @note   This notification message will be sent to the UI layer then the PTM
            could be shown on the screen, such as time bar
    */
    AUI_EVNT_PVR_MSG_PLAY_PTM_UPDATE        =30,

    /**
    Value to specify that the player got an <b> Uniform Resource Identifier
    (URI) information data </b>
    */
    AUI_EVNT_PVR_MSG_PLAY_URI_NOTIFY        =31,

    /**
    Value to specify that RL (Retention Limited) need to be shifted.\n
    While playing the RL (Retention Limited) type one item, the RL info will
    be detected by the PVR Engine.\n
    When the following RL is updated and the player can be set to a new play
    time, this value (message) will be sent out by PVR Engine, and the new time
    information will be sent as parameter of this message

    @note   This message is used @a only for <b> CI Plus projects </b>
    */
    AUI_EVNT_PVR_MSG_PLAY_RL_SHIFT          =32,

    /**
    Value to specify that RL (Retention Limited) need to be adjusted.\n
    While playing the RL (Retention Limited) type one item, the RL info will
    be detected by the PVR Engine.\n
    If the RL is invalid and the player can be set to a new play time, this
    value (message) will be sent out by PVR Engine, and the new time information
    will be sent as parameter of this message.

    @note   This message is used @a only for <b> CI Plus projects </b>
    */
    AUI_EVNT_PVR_MSG_PLAY_RL_RESET          =33,

    /**
    Value to specify that RL (Retention Limited) is invalid.\n
    While playing the RL (Retention Limited) type one item, the RL info will
    be detected by the PVR Engine.\n
    If the RL is invalid, this value (message) will be sent out by PVR Engine,
    and the RL time information will be sent as parameter of this message.

    @note   This message is used @a only for <b> CI Plus projects </b>
    */
    AUI_EVNT_PVR_MSG_PLAY_RL_INVALID        =34,

    /**
    Value to specify that the <b> reading speed </b> of PVR is <b> too low </b>

    @note   During playback, the reading speed changes dynamically and it will
            be detected by the PVR engine.\n
            If the reading speed is too low to playback, this message will be
            send to the UI layer.
    */
    AUI_EVNT_PVR_READ_SPEED_LOW             =35,

    /**
    Value to specify that the <b> writing speed </b> of PVR is <b> too low </b>

    @note   When recording, the speed for saving data changes dynamically and
            it will be detected by the PVR engine.\n
            If the saving speed is too low to record data, this message will
            be send to the UI layer.

    */
    AUI_EVNT_PVR_WRITE_SPEED_LOW            =36,

    /**
    Value to specify that the <b> recorder stopped a DMX device </b>
    */
    AUI_EVNT_PVR_MSG_REC_STOP_OP_STOPDMX    =37,

    /**
    Value to specify that the <b> recorder got the encrypted key </b>
    */
    AUI_EVNT_PVR_MSG_REC_GET_KREC           =38,

    /**
    Value to specify <b> other data information (except audio/video) </b> to
    be encrypted

    @note   Example of other data information can be non-Transport Stream data
            to describe the Transport Stream data
    */
    AUI_EVNT_PVR_MSG_CRYPTO_DATA            =39,

    /**
    Value to specify the <b> application which PIDs </b> to be @b encrypted
    before saving to the disk

    @note   The PIDs information for the re-encryption needs to be set to the
            DSC device otherwise the Transport Stream packages will not be
            re-encrypted.\n
            When start recording, this notification message will be sent to
            the UI layer.
    */
    AUI_EVNT_PVR_MSG_REC_SET_REENCRYPT_PIDS =40,

    /**
    Value to specify the <b> application which PIDs </b> to be @b de_crypted
    before playback the content from the disk
    */
    AUI_EVNT_PVR_MSG_PLAY_SET_REENCRYPT_PIDS=41,

    /**
    Value to specify the @b verification of <b> TS and information data </b>

    @note   This notification message is used to calculate the hash value for
            the chunk data and return it to the PVR
    */
    AUI_EVNT_PVR_MSG_CAL_CHUNK_HASH         =42,

    /**
    Value to specify that the <b> URI stop </b> changed
    */
    AUI_EVNT_PVR_MSG_REC_STOP_URICHANGE     =43,

    /**
    Value to specify that the <b> TMS URI stop </b> changed
    */
    AUI_EVNT_PVR_MSG_TMS_STOP_URICHANGE     =44,

    /**
    Value to specify that the <b> TMS URI pause </b> changed
    */
    AUI_EVNT_PVR_MSG_TMS_PAUSE_URICHANGE    =45,

    /**
    Value to specify that the <b> TMS URI resume </b> changed
    */
    AUI_EVNT_PVR_MSG_TMS_RESUME_URICHANGE   =46,

    /**
    Value to specify that the PVR recording file will be delete if its time
    length is shorter than the member @b record_min_len of the struct
    #aui_pvr_init_param
    */
    AUI_EVNT_PVR_MSG_REC_TOO_SHORT          =47,

    /**
    Value to specify the PVR recording block mode
    */
    AUI_EVNT_PVR_MSG_BLOCK_MODE_DECRYPT     =52,

    /**
    Value to specify the end of the notification messages available in this enum
    */
    AUI_EVNT_END

} aui_evnt_pvr;

/**
Enum to specify the <b> PVR State </b>

@note   This enum is @a reserved to ALi R&D Dept. then user can ignore it
*/
typedef enum aui_pvr_state {

    /**
    Value to specify that PVR is in @b Idle state, which means <b> recording/
    playback pause </b>
    */
    AUI_PVR_STATE_IDLE = 0,

    /**
    Value to specify that PVR is in @b time-shifting state
    */
    AUI_PVR_STATE_TMS,

    /**
    This enum indicates PVR is in play time-shifting program

    @note   It means that the player is working in time shift mode.
    */
    AUI_PVR_STATE_TMS_PLAY,

    /**
    Value to specify that PVR is in <b> recording state </b>
    */
    AUI_PVR_STATE_REC,

    /**
    Value to specify that PVR is <b> recording program </b> and <b> playing
    recording program </b>
    */
    AUI_PVR_STATE_REC_PLAY,

    /**
    Value to specify that PVR is <b> recording program </b> and <b> playing
    HDD program </b>
    */
    AUI_PVR_STATE_REC_PVR_PLAY,

    /**
    Value to specify that PVR is <b> playing HDD program </b>
    */
    AUI_PVR_STATE_UNREC_PVR_PLAY,

    /**
    Value to specify that PVR is <b> time-shifting program </b> within the
    same Transport Stream <b> without playing </b>

    @note   It means that the player is working in time shift mode
    */
    AUI_PVR_STATE_REC_TMS,

    /**
    Value to specify that PVR is <b> time-shifting program </b> within the
    same Transport Stream and <b> playing </b>

    @note   It means that the player and recorder are working in time shift mode
    */
    AUI_PVR_STATE_REC_TMS_PLAY,

    /**
    Value to specify that PVR is <b> playing another program </b> from @b HDD.
    */
    AUI_PVR_STATE_REC_TMS_PLAY_HDD,

    /**
    Value to specify that PVR is <b> recording two (2) programs </b> within the
    same Transport Stream
    */
    AUI_PVR_STATE_REC_REC,

    /**
    Value to specify that PVR is <b> recording two (2) programs </b> within the
    same Transport Stream and <b> playing one (1) recording program </b>
    */
    AUI_PVR_STATE_REC_REC_PLAY,

    /**
    Value to specify that PVR is recording two programs within the same Transport
    Stream and <b> playing HDD program </b>.
    */
    AUI_PVR_STATE_REC_REC_PVR_PLAY

} aui_pvr_state;

/**
Enum to specify different <b> player seeking mode </b>
*/
typedef enum aui_pvr_play_position_flag {

    /**
    Value to specify the @b offset from the @b head
    */
    AUI_PVR_PLAY_POSITION_FROM_HEAD = 0,

    /**
    Value to specify the @b offset from the <b> current position </b>
    */
    AUI_PVR_PLAY_POSITION_FROM_CURRENT,

    /**
    Value to specify the @b offset from the @b end
    */
    AUI_PVR_PLAY_POSITION_FROM_END

} aui_pvr_play_position_flag;

/**
Function pointer to specify a callback function used by the upper layer to deal
with event message sent by the PVR Engine
*/
typedef void (*aui_pvr_callback) (

    aui_hdl handle,

    aui_evnt_pvr event,

    unsigned int msg_code,

    void *pv_user_data

    );

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        all the parameters available to be initialized
        </div> @endhtmlonly

        Struct to specify all the parameters available to be initialized
*/
typedef struct aui_pvr_init_param {

    /**
    Member to specify the <b> Project Mode </b> available to be managed, as
    defined in the enum #aui_pvr_project_mode
    */
    unsigned short int prj_mode;

    /**
    Member to specify the <b> Maximum number of supported recorder </b>

    @note   At the moment, only two (2) recorder are supported
    */
    unsigned char max_rec_number;

    /**
    Member to specify the <b> Maximum number of supported player </b>

    @note   At the moment, only one (1) player is supported
    */
    unsigned char max_play_number;

    /**
    Member to specify the <b> Video Object (VOB) cache address </b>

    @note   Please use the VOB cache address from a continuous buffer
    */
    unsigned int cache_addr;

    /**
    Member to specify the <b> Video Object (VOB) cache size </b>
    (in @b bytes unit)

    @note   The size is determined by the recording and playing capability,
            and it is different depending on SD/HD solution as below:
            - For @b HD solution:
              @b cache_size = (@b max_rec_number + @b max_play_number) * 250 * 47K
            - For @b SD solution:
              @b cache_size = (@b max_rec_number + @b max_play_number) * 30 * 47K

    where @b max_rec_number and @b max_play_number are member of this struct
    */
    unsigned int cache_size;

    /**
    Member to specify the <b> PVR debug level </b>, as defined in the enum
    #aui_pvr_debug_level)

    @note   This member is used to set the sub modules to print debug messages
    */
    unsigned char debug_level;

    /**
    Member to specify the desired <b> path prefix </b> to add to the attached
    PVR storage device path to get the full path where saving the recording file

    @note   Example: If the
            - Desired path prefix is @b ali_dvr
            - And the attached PVR storage device path is @b /mnt/usb1

    @note   then the full path where saving the recording file will be
            @b /mnt/usb1/ali_dvr
    */
    char dvr_path_prefix[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the <b> information file name </b> which contains information
    related to the PVR recording file
    */
    char info_file_name[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the <b> new information file name </b> which contains @a new
    information related to the PVR recording file
    */
    char info_file_name_new[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the name of the file used as extension of TS recording mode

    @note   Usually, this member is used for saving the TS package and time
            information during recording before PVR Ver.2 ( @a only for
            compatibility)
    */
    char ts_file_format[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the name of the file used as extension of the TS recording
    mode

    @note   Usually, this member is used for saving the TS package during
            recording since PVR Version 3
    */
    char ts_file_format_new[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the name of the file which is used as extension of the PS
    recording mode record

    @note   Usually, this member is used for saving the PES data during recording
            since PVR Ver.3
    */
    char ps_file_format[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the name of the first file which is used to test device
    speed after mounting file system
    */
    char test_file1[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the name of the second file which is used to test device
    speed after mounting file system
    */
    char test_file2[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    Member to specify the name of the file which is used as extension of the TS
    recording mode

    @note   Usually, this member is used for saving the CAS return values during
            recording since PVR Ver.3
    */
    char storeinfo_file_name[256];

    /**
    Member to specify how frequently (in @a millisecond (ms) unit) the stream
    data will be saved to the disk when recording H.264 stream

    @note   The valid value belong to the range [500,2000], it is recommended
            to set 600
    */
    unsigned int h264_vobu_time_len;

    /**
    Member to specify the support of @b AC3 decoding
    */
    unsigned char ac3_decode_support;

    /**
    Member to specify <b> continuous time-shifting enabled </b>

    @note   This setting is not used in the current PVR version PVR (i.e. V.3)
            then it can be considered @a only for compatibility
    */
    unsigned char continuous_tms_en;

    /**
    Member to specify how frequently (in @a millisecond (ms) unit) the stream
    data will be saved to the disk when recording scramble stream

    @note   The valid value belong to the range [500,2000], it is recommended
            to set 600
    */
    unsigned int scramble_vobu_time_len;

    /**
    Member to specify the <b> refresh time </b> (in @a second unit) of the
    header file

    @note   Minimum 15 seconds, recommended 30 seconds
    */
    unsigned int file_header_save_dur;

    /**
    Member to specify the <b> minimum recording time </b> (in @a second unit)

    @note   @b 1. Recommended 15 seconds
    @note   @b 2. The recording will be cancelled if its time length is shorter
                  than this value
    */
    unsigned int record_min_len;

    /**
    Member to specify the <b> maximum recording time shift </b>
    (in @a second unit)

    @note   Recommended 7200 seconds
    */
    unsigned int tms_time_max_len;

    /**
    Member to specify the <b> minimum size </b> of the <b> time shift recording
    file </b> (in @a Mbytes unit)

    @note   Recommend 10 Mbyte
    */
    unsigned int tms_file_min_size;

    /**
    Member  to specify whether the record duration should include the pause time
    or not. In particular:
      - @b 0 = no
      - @b 1 = yes
    @note  The default value is @b 0
    */
    unsigned int trim_record_ptm;

} aui_pvr_init_param, *p_aui_pvr_init_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the PID information of a program
        </div> @endhtmlonly

        Struct to specify the PID information of a program
*/
typedef struct aui_pvr_pid_info {

    /**
    Member to specify total number of @b PIDs

    @note   It should be less than @b 32 (@b 27 for PIP projects) where
            redundant PIDs will be ignored by PVR and DMX
    */
    unsigned char   total_num;

    /**
    Member to specify the total number of <b> Audio PID </b>
    */
    unsigned char   audio_count;

    /**
    Member to specify the total number of <b> Entitled Control Message (ECM)
    PIDs </b>
    */
    unsigned char   ecm_pid_count;

    /**
    Member to specify the total number of <b> Entitled Management Message (EMM)
    PIDs </b>
    */
    unsigned char   emm_pid_count;

    /**
    Member to specify the total number of <b> TeleText PIDs </b>
    */
    unsigned char   ttx_pid_count;

    /**
    Member to specify the total number of <b> Subtitle PIDs </b>
    */
    unsigned char   subt_pid_count;

    /**
    Member to specify the total number of <b> TeleText Subtitle  PIDs </b>
    */
    unsigned char   ttx_subt_pid_count;

    /**
    Member to specify the current selected <b> audio channel PID </b>
    */
    unsigned char   cur_audio_pid_sel;

    /**
    Member to specify the total number of <b> ISDBTCC PID </b>
    */
    unsigned char   isdbtcc_pid_count;

    /**
    @attention  This member is @a reserved to ALi r&D Dept. then user can
                ignore it
    */
    unsigned char   resv0[3];

    /**
    Member to specify the <b> Video PIDs </b>
    */
    unsigned  short int video_pid;

    /**
    Member to specify the <b> Audio PIDs </b>
    */
    unsigned  short int audio_pid[AUI_MAX_PVR_AUDIO_PID];

    /**
    Member to specify the <b> audio type </b>, as defined in the enum
    #aui_deca_stream_type, for each Audio PID

    @note   PVR will convert an audio type to the corresponding ALi's
            internal audio type
    */
    aui_deca_stream_type audio_type[AUI_MAX_PVR_AUDIO_PID];

    /**
    Member to specify the <b> Audio language </b>
    */
    unsigned  short int audio_lang[AUI_MAX_PVR_AUDIO_PID];

    /**
    Member to specify the <b> Program Clock Reference (PCR) PID </b>
    */
    unsigned  short int pcr_pid;

    /**
    Member to specify the <b> Program Association Table (PAT) PID </b>
    */
    unsigned  short int pat_pid;

    /**
    Member to specify the <b> Program Map Table (PMT) PID </b>
    */
    unsigned  short int pmt_pid;

    /**
    Member to specify the <b> Service Description Table (SDT) PID </b>
    */
    unsigned  short int sdt_pid;

    /**
    Member to specify the <b> Event Information Table (EIT) PID </b>
    */
    unsigned  short int eit_pid;

    /**
    Member to specify the <b> Conditional Access Table (CAT) PID </b>
    */
    unsigned  short int cat_pid;

    /**
    Member to specify the <b> Network Information Table (NIT) PID </b>
    */
    unsigned  short int nit_pid;

    /**
    Member to specify the <b> Entitled Control Message (ECM) PID </b>
    */
    unsigned  short int ecm_pids[AUI_MAX_PVR_ECM_PID];

    /**
    Member to specify the <b> Entitled Management Message (EMM) PID </b>
    */
    unsigned  short int emm_pids[AUI_MAX_PVR_EMM_PID];

    /**
    Member to specify the <b> TeleText PID </b>
    */
    unsigned  short int ttx_pids[AUI_MAX_PVR_TTX_PID];

    /**
    Member to specify the <b> Subtitle PID </b>
    */
    unsigned  short int subt_pids[AUI_MAX_PVR_SUBT_PID];

    /**
    Member to specify the <b> TeleText Subtitle PID </b>
    */
    unsigned  short int ttx_subt_pids[AUI_MAX_PVR_TTX_SUBT_PID];

    /**
    Member to specify the <b> ISDBTCC PID </b>
    */
    unsigned  short int isdbtcc_pids[AUI_MAX_PVR_ISDBTCC_PID];
    /**
    @attention  This member is @a reserved to ALi R&D Dept. then user can
                ignore it
    */
    unsigned  short int resv1[3];

} aui_pvr_pid_info, *p_aui_pvr_pid_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the record item information
        </div> @endhtmlonly

        Struct to specify the record item information
*/
typedef struct aui_pvr_rec_item_info {

    /**
    Member to specify the @b index of the record item
    */
    unsigned int        index;

    /**
    Member as a @a flag to specify the <b> recording mode </b> of the record item,
    in particular:
    - @b 0 = Normal mode
    - @b 1 = Time-shift mode
    */
    unsigned int        rec_mode;

    /**
    Member as @a flag to specify the <b> stream type </b> of the record item,
    in particular:
    - @b 0 = Transport Stream (TS)
    - @b 1 = Program Stream (PS)

    @note   This member can be used @a only for FTA Streams
    */
    unsigned char rec_type;

    /**
    Member as a @a flag to specify the <b> media type </b> of the record item,
    in particular:
    - @b 0 = Radio
    - @b 1 = TV

    @note  The default value is @b 1
    */
    unsigned char av_type:1;

    /**
    Member to specify the <b> Program ID </b> of the record item
    */
    unsigned short prog_number;

    /**
    Member to specify the @b path of the record item
    */
    char                path[AUI_PVR_FILE_PATH_LEN_MAX];

    /**
    Member to specify the <b> program name </b> of the record item
    */
    char                service_name[AUI_PVR_SERVICE_NAME_LEN_MAX];

    /// @cond

    /**
    Member to specify the <b> information start time </b> of the record item
    */
    struct tm        start_time;

    /// @endcond

    /**
    Member as a @a flag to specify the <b> lock status </b> of the record item,
    in particular:
    - @b 0 = Not locked
    - @b 1 = Locked
    */
    unsigned int        lock_flag;

    /**
    Member as a @a flag to specify the <b> channel type </b> of the record item,
    in particular:
    - @b 0 = Radio
    - @b 1 = TV
    */
    unsigned int        channel_type;

    /**
    Member to specify the <b> video type </b> of the record item, as defined
    in the enum #aui_decv_format
    */
    aui_decv_format     video_type;

    /**
    Member as a @a flag to specify the <b> scramble status </b> of the @a original
    record item (i.e. <b> CI/CI+/CA </b>) </b>, in particular:
    - @b 1 = Scrambled
    - @b 0 = Not scrambled
    */
    unsigned char scrambled_rec;

    /**
    Member to specify the <b> re-encryption mode </b> of the @b CA record item,
    as defined in the enum #S_PVR_REENCRYPT_MODE
    */
    unsigned char rec_special_mode;

    /**
    Member to specify the <b> playback/recording duration </b> (in @a second (s) unit)
    of the record item
    */
    unsigned int        duration;

    /**
    Member to specify the @b size (in @a KB unit) of the record item
    */
    unsigned int        size_in_KB;

    /**
    Member as a @a flag to specify the <b> recording status </b> of the @b current
    record item, in particular:
    - @b 0 = Not recording
    - @b 1 = Recording
    */
    unsigned int        is_recording;

    /**
    Member to specify the @b index of the <b> current selected audio </b>
    */
    unsigned int        cur_audio_sel_idx;

    /**
    Member to specify the <b> event name </b> of the record item

    @warning  Set the event name at the beginning of the recording
    */
    char                event_name[AUI_PVR_EVENT_NAME_LEN_MAX];

} aui_pvr_rec_item_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the program record parameters
        </div> @endhtmlonly

        Struct to specify the program record parameters
*/
typedef struct aui_record_prog_param {

    /**
    Member as @a flag to specify the <b> recording type </b>, in particular
    - @b 0 = Transport stream (TS)
    - @b 1 = Program stream (PS)

    @note   This member is used @a only for FTA Streams
    */
    unsigned char rec_type;

    /**
    Member to specify <b> channel ID </b>
    */
    unsigned int channel_id;

    /**
    Member to specify the <b> audio channel </b>
    */
    unsigned char audio_channel;

    /**
    Member to specify the <b> Program ID </b>
    */
    unsigned short prog_number;

    /**
    Member as a @a flag to specify the <b> recording mode </b>, in particular
    - @b 1 = Time shift mode
    - @b 0 = Normal recording
    */
    unsigned char is_tms_record:1;

    /**
    Member as a @a flag to specify whether the member @b folder_name of the
    struct #aui_record_prog_param represent the <b> full path </b> to locate
    the recorded file or not, in particular
    - @b 1 = Yes
    - @b 0 = No
    */
    unsigned char full_path:1;

    /**
    Member as a @a flag to specify @b Radio or @b TV, in particular
    - @b 0 = Radio
    - @b 1 = TV
    */
    unsigned char av_flag:1;

    /**
    Member as a @a flag to specify a lock, in particular
    - @b 1 = Lock
    - @b 0 = Unlock
    */
    //unsigned char lock_flag:1;

    /**
    Member as a @a flag to specify whether the stream is scrambled when
    recording or not, in particular
    - @b 1 = Yes
    - @b 0 = No (i.e. FTA Stream)
    */
    //unsigned char is_scrambled:1;

    /**
    Member as a @a flag to enable/disable the adding of the current recording
    file to the end of the existing file, in particular
    - @b 1 = Enable
    - @b 0 = Disable

    @note   @a Only enable when using the time shift recording mode
    */
    unsigned char continuous_tms;

    /**
    Member as a @a flag to specify whether the data is re-encrypted before
    storing into USB disk or not, in particular
    - @b 1 = Yes
    - @b 0 = No (i.e. FTA Stream)
    */
    unsigned char is_reencrypt:1;

    /**
    Member as a @ flag to specify whether the @b ECM and @b EMM data are
    recorded or not, in particular
    - @b 1 = Yes
    - @b 0 = No (i.e. FTA Stream)
    */
    unsigned char ca_mode:1;

    /**
    Member as a @a flag to specify the current program video type, in particular
    - @b 0 = @b MPEG2
    - @b 1 = @b H264
    - @b 2 = @b AVS
    - @b 3 = @b H265
    */
    unsigned char h264_flag:6;

    /**
    Member to specify the <b> DMX ID </b> used when recording
    */
    unsigned char dmx_id:4;

    /**
    Member to specify the <b> DMX ID </b> when play live
    */
    unsigned char live_dmx_id:4;

    /**
    Member to specify the <b> folder name </b> used to store the current
    recording data in U-disk
    */
    char folder_name[AUI_PVR_FILE_PATH_LEN_MAX];

    /**
    Member to specify the <b> recording item name </b>, i.e. the <b> program
    (service) name </b> which is defined by DVB specification
    */
    char service_name[AUI_PVR_SERVICE_NAME_LEN_MAX];

    /**
    Member as a @a flag to specify whether the stream data is recorded and
    stored as raw data or not, in particular
    - @b 1 = Yes
    - @b 0 = No
    */
    unsigned char is_scrambled;

    /**
    Member to specify the <b> recording mode </b> as per definition of the
    macro below:
    - #AUI_PVR_NONE
    - #AUI_PVR_CAS9_RE_ENCRYPTION
    - #AUI_PVR_CIPLUS_RE_ENCRYPTION
    - #AUI_PVR_COMMON_RE_ENCRYPTION
    - #AUI_PVR_FINGERP_RE_ENCRYPTION
    - #AUI_PVR_CAS9_MULTI_RE_ENCRYPTION
    - #AUI_PVR_VMX_MULTI_RE_ENCRYPTION
    - #AUI_RSM_C0200A_MULTI_RE_ENCRYPTION
    */
    unsigned char rec_special_mode;

    /**
    Member to specify the <b> PID Channel Information </b>,as defined in the
    struct #aui_pvr_pid_info
    */
    aui_pvr_pid_info pid_info;

    /**
    Member to specify the setting of user data when registering the callback
    function specified in the member @b fn_callback of this struct
    */
    void* user_data;

    /**
    Member used to register a callback function for an event message, as per
    comment for the function pointer #aui_pvr_callback
    */
    aui_pvr_callback fn_callback;

    /**
    Member to specify that the curent record will be appended to the existing
    file
    */
    unsigned char append_to_exist_file;

    /**
    Member as a flag to specify whether take record of all TP data or not,
    in particular
    - @b 0 = Don't take record of all TP data
    - @b 1 = take record of all TP data
    */
    unsigned char record_whole_tp_data;

} aui_record_prog_param, *p_aui_record_prog_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the parameters for stopping playback
        </div> @endhtmlonly

        Struct to specify the parameters for stopping playback
*/
typedef struct aui_pvr_stop_ply_param {

    /**
    Member to specify the mode to stop playing, as defined in the struct
    #aui_pvr_stop_mode
    */
    unsigned char stop_mode;

    /**
    Member as a @a flag to specify the mode to close the screen when stopping,
    in particular
    - @b 0 = Close video output and the screen will show as black immediately
    - @b 1 = Do not close video output then the last frame will be kept on the screen
    */
    unsigned char vpo_mode;

    /**
    @attention  This member is no longer supported then is @a deprecated
    */
    unsigned char sync;

} aui_pvr_stop_ply_param, *p_aui_pvr_stop_ply_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the parameters for playback
        </div> @endhtmlonly

        Struct to specify the parameters for playback
*/
typedef struct aui_ply_param {

    /**
    Member to specify a <b> Playback Index </b>
    */
    unsigned short int index;

    /**
    Member to specify the <b> Playback Recording Item </b>
    */
    char path[AUI_PVR_FILE_PATH_LEN_MAX];

    /**
    Member to specify the <b> Playback state </b>, as defined in the enum
    #aui_pvr_play_state
    */
    aui_pvr_play_state  state;

    /**
    Member to specify the <b> starting point to play </b>
    */
    unsigned int start_time;

    /**
    Member to specify the <b> starting position to play </b>
    */
    unsigned int start_pos;

    /**
    Member to specify the <b> playback speed </b>, as defined in the enum
    #AUI_PVR_PLY_SPEED
    */
    unsigned char speed;

    /**
    Member to specify the <b> configuration for playing </b>, as defined in
    the enum #AUI_PVR_OPEN_CONFIG
    */
    unsigned char start_mode;

    /**
    Member to specify the <b> playback DMX ID </b>
    */
    unsigned char dmx_id;

    /**
    Member to specify the <b> live play DMX ID </b>
    */
    unsigned char live_dmx_id;

    /**
    Member as a @a flag to specify the <b> content playback view mode </b>,
    in particular
    - @b 1 = Preview_mode
    - @b 0 = Full screen
    */
    unsigned char preview_mode;

    /**
    Member to specify the setting of user data when registering the callback
    function specified in the member @b fn_callback of this struct
    */
    void* user_data;

    /**
    Member used to register a callback function for an event message, as per
    comment for the function pointer #aui_pvr_callback
    */
    aui_pvr_callback fn_callback;

} aui_ply_param, *p_aui_ply_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to
        specify the parameters for attaching and using a disk
        </div> @endhtmlonly

        Struct to specify the parameters for attaching and using a disk
*/
typedef struct aui_pvr_disk_attach_info {

    /**
    Member as a @a flag to specify the disk mode, in particular
    - @b 0 = USB Disk
    - @b 1 = IDE Disk
    */
    unsigned char   disk_mode;

    /**
    Member to specify how to use the disk

    @note   This member can take the macro below:
            - @b #AUI_PVR_REC_AND_TMS_DISK for normal record & TMS record
            - @b #AUI_PVR_REC_ONLY_DISK for @a only normal recording
            - @b #AUI_PVR_TMS_ONLY_DISK for @a only TMS recording
    */
    unsigned char   disk_usage;

    /**
    Member to specify the name of the mounted USB disk, such as <b> /mnt/uda1
    </b>
    */
    char    mount_name[AUI_PVR_FILE_NAME_LEN_MAX];

    /**
    @attention  This member is no longer supported then is @a deprecated
    */
    unsigned int    vbh_addr;

    /**
    @attention  This member is no longer supported then is @a deprecated
    */
    unsigned int    vbh_len;

    /**
    Member as a @a flag to specify whether the PVR recording list from disk
    needs to be initialized cause it is the first recording or not, in
    particular
    - @b 1 = It is the first recording then the initialization is necessary
    - @b 0 = It is not the first recording then the initialization is not
             necessary
    */
    unsigned char   init_list;

    /**
    Member as a @a flag to specify whether the current action is returned
    immediately or not, in particular
    - @b 1 = Yes
    - @b 0 = No
    */
    unsigned char sync;

    /**
    Member as a @a flag to specify whether checking the speed of the disk or
    not,in particular
    - @b 1 = Yes
    - @b 0 = No

    @note   Recommended to set as @b 1
    */
    unsigned char   check_speed;

} aui_pvr_disk_attach_info, *p_aui_pvr_disk_attach_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the parameters for de-attaching a disk
        </div> @endhtmlonly

        Struct to specify the parameters for de-attaching a disk
*/
typedef struct aui_pvr_disk_detach_info {

    /**
    Member to specify the <b> disk mode </b> as defined in the member @b disk_mode
    of the struct #aui_pvr_disk_attach_info
    */
    unsigned char   disk_mode;

    /**
    Member to specify the name of the mounted USB disk, such as <b> /mnt/uda1 </b>,
    as defined in the member #mount_name of the struct #aui_pvr_disk_attach_info
    */
    char    mount_name[AUI_PVR_FILE_NAME_LEN_MAX];

} aui_pvr_disk_detach_info, *p_aui_pvr_disk_detach_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the URI information as per CONAX Specifications
        </div> @endhtmlonly

        Struct to specify the URI information according to CONAX Specifications

@note   @b 1. This struct is 32-byte aligned for HASH calculation
@note   @b 2. This struct is used @a only for Advanced Security (AS)
*/
typedef struct aui_pvr_uri {

    /// @coding

    /**
    Member as a struct of the C-standard library to specify a calendar date and
    time broken down
    */
    struct tm dt;

    /// @endcoding

    /**
    Member to specify the playtime (in @a millisecond (ms) unit) to apply the URI
    */
    unsigned int ptm;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bap_default   :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bap_ecm_mat   :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bap_res1      :2;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bap_pvr_mat   :4;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bex_res1      :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bex_res2      :3;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bex_ciplus    :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bex_pbda      :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bex_dtcp      :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned char bex_hndrm     :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_aps :2;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_emi :2;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_ict :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_rct :1;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_retlimit    :3;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_trickplay   :3;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_mat :3;

    /**
    @warning    Member to specify ab URI property as per CONAX Specifications,
                please refer to them for more clarifications
    */
    unsigned short int buri_da  :1;

} aui_pvr_uri, aui_pvr_uri_t, *p_aui_pvr_uri; //32-byte alignment for Hash calculation

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        the fingerprint information according to CONAX Specifications
        </div> @endhtmlonly

        Struct to specify the fingerprint information according to CONAX
        Specifications

@note   This struct is used @a only for Advanced Security (AS)
*/
typedef struct aui_pvr_finger_info {

    /**
    Member to specify the playtime (in @a millisecond (ms) unit) to show the
    fingerprint on the screen
    */
    unsigned int ptm;

    /**
    Member to specify the height of the fingerprint message (in @a pixel unit)
    to be shown on the screen
    */
    unsigned char  fp_height;

    /**
    Member to specify the priority to show the fingerprint message on the screen

    @note   This member is used when more than two (2) fingerprint are available
            to be shown on the screen
    */
    unsigned char  fp_priority;

    /**
    Member to specify how long (in <em> 10 milliseconds (ms) </em> unit) the
    fingerprint will be shown on the screen
    */
    unsigned short fp_duration;

    /**
    Member to specify the X-coordinate of the fingerprint message to be shown
    from the top-left corner of the screen
    */
    unsigned short fp_pos_x;

    /**
    Member to specify the Y-coordinate of the fingerprint message to be shown
    from the top-left corner of the screen
    */
    unsigned short fp_pos_y;

    /**
    Member to specify the fingerprint message to be shown on the screen
    */
    unsigned char  fp_id[64];

} aui_pvr_finger_info;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        which PIDs need to be re-encrypted
        </div> @endhtmlonly

        Struct to specify which PIDs need to be re-encrypted by the PVR Module

@note   @b 1. While recording TS stream, many TS streams with different PIDs
              will be recorded to the same file but only a few of them need to
              be re-encrypted.\n
              The application can handle the event
              #AUI_EVNT_PVR_MSG_REC_SET_REENCRYPT_PIDS in the callback funtion
              #aui_pvr_callback to tell PVR Module which PIDs need to be
              re-encrypted. \n
              When the event in the callback function #aui_pvr_callback is
              #AUI_EVNT_PVR_MSG_REC_SET_REENCRYPT_PIDS, the parameter @b msg_code
              of that callback function will the pointer to the struct
              #aui_pvr_crypto_pids_param

@note   @b 2. This struct is used @a only for <b> VMX Advanced Security (AS) </b>
*/
typedef struct aui_pvr_crypto_pids_param {

    /**
    In
    */
    aui_pvr_pid_info *pid_info;

    /**
    Out
    */
    unsigned short   *pid_list;

    /**
    In/Out
    */
    unsigned short    pid_num;

} aui_pvr_crypto_pids_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        general parameters to be re-encrypted
        </div> @endhtmlonly

        Struct to specify general parameters to be re-encrypted
*/
typedef struct aui_pvr_crypto_general_param {

    /**
    Out
    */
    unsigned short     *pid_list;

    /**
    Out
    */
    unsigned short      pid_num;

    /**
    Out
    */
    unsigned char       dmx_id;

} aui_pvr_crypto_general_param;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> for CAS
        to specify header information
        </div> @endhtmlonly

        Struct for CAS to specify header information
*/
typedef struct aui_pvr_store_info_header {

    /**
    N/A
    */
    unsigned int block_size;

    /**
    N/A
    */
    unsigned int storeinfoheader_len;

    /**
    N/A
    */
    unsigned char storeinfoheader[AUI_PVR_INFO_HEADER_SIZE];

} aui_pvr_store_info_header;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> (used for CAS)
        to specify data information
        </div> @endhtmlonly

        Struct used for CAS to specify data information
*/
typedef struct aui_pvr_store_info_data_single {

    /**
    Member as an index to specify the stored information element,
    which values can be:
    - @b time, for TS mode
    - @b block count, for block mode
    */
    unsigned int index;

    /**
    N/A
    */
    unsigned int encrypt_mode;

    /**
    N/A
    */
    unsigned short storeinfodata_len;

    /**
    N/A
    */
    unsigned char storeinfodata[AUI_PVR_STORE_INFO_DATA_SIZE_MAX];

} aui_pvr_store_info_data_single;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        data information to be stored
        </div> @endhtmlonly

        Struct used to specify data information to be stored
*/
typedef struct aui_pvr_store_info_data {

    /**
    N/A
    */
    aui_pvr_store_info_data_single    store_info_data_pre;

    /**
    N/A
    */
    aui_pvr_store_info_data_single    store_info_data_nex;

} aui_pvr_store_info_data;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        parameters for decrypting a block
        </div> @endhtmlonly

        Struct to specify parameters for decrypting a block
*/
typedef struct aui_pvr_block_decrypt_param {

    /**
    N/A
    */
    unsigned int  pvr_hnd;

    /**
    N/A
    */
    unsigned char   *input;

    /**
    N/A
    */
    unsigned int    length;

    /**
    N/A
    */
    unsigned int    block_idx;

    /*
    Member used to indicate the @b H.264 or @b MPEG2 Video TS,
    in particular:
    - @b 0 = VOB frame data start
    - @b 1 = VOB frame normal data
    - @b 2 = VOB frame end data
    */
    unsigned int     indicator;

} aui_pvr_block_decrypt_param;

/**
Enum to specify the <b> PVR disk partition status </b>
*/
typedef enum aui_pvr_partition_status {

    /**
    Value to specify that the PVR disk partition is available to be read
    and/or written
    */
    AUI_PVR_PARTITION_OK              = 0,

    /**
    Value to specify that the PVR disk partition doesn't exist
    */
    AUI_PVR_PARTITION_NOT_EXIST       = (1<<5),

    /**
    Value to specify that the PVR disk partition is full so no space available
    to be written but only to be read
    */
    AUI_PVR_PARTITION_SPACE_FULL      = (1<<6),

    /**
    Value to specify that an error occurred when creating a root directory
    so the PVR disk partition is disabled to be written and/or read
    */
    AUI_PVR_PARTITION_CREATE_DIR_ERR  = (1<<7),

    /**
    Value to specify that an error occurred when writing to the PVR disk partition
    */
    AUI_PVR_PARTITION_WRITE_ERR       = (1<<8),

    /**
    Value to specify that an error occurred when reading from the PVR disk partition
    */
    AUI_PVR_PARTITION_READ_ERR        = (1<<9)

} aui_pvr_partition_status;

/**
@brief  @htmlonly <div class="details">
        Struct of the <b> Personal Video Recorder (PVR) Module </b> to specify
        information related to the mounted PVR disk partiton
        </div> @endhtmlonly

        Struct to specify information related to the mounted PVR disk partition
*/
typedef struct aui_pvr_partition_info {

    /**
    Member to specify the @b name of the mounted PVR disk partition
    */
    char mount_name[AUI_PVR_MOUNT_NAME_LEN_MAX];

    /**
    Member to specify the <b> file system type </b> for the mounted PVR disk
    partition, in particular:
    - @b 0 = Unknown type
    - @b 1 = FAT type
    - @b 2 = NTFS type
    */
    unsigned char type;

    /**
    Member to specify what the mounted PVR disk partition is used for.
    In particular, the specific use is indicated by a proper MACRO value as
    below:
    - #AUI_PVR_REC_AND_TMS_DISK for <b> Normal & TMS Recording </b>
    - #AUI_PVR_REC_ONLY_DISK for @a only <b> Normal Recording </b>
    - @b #AUI_PVR_TMS_ONLY_DISK for @a only <b> TMS Recording </b>
    */
    unsigned char disk_usage;

    /**
    Member as a @a flag to specify whether the <b> record list </b> of the
    mounted PVR disk partition will be scanned and initialized or not,
    in particular:
    - @b 1 = Scanned and initialized
    - @b 0 = Not scanned and initialized
    */
    unsigned char init_list;

    /**
    Member as a @a flag to specify whether the <b> disk speed check </b>
    of the mounted PVR disk partition will be processed or not, in particular:
    - @b 1 = Processed
    - @b 0 = Not processed
    */
    unsigned char check_speed;

    /**
    Member to specify the total size (in @b KB  unit) of the mounted PVR disk
    partition
    */
    unsigned int total_size_in_KB;

    /**
    Member to specify the <b> free size </b> (i.e. space not available to be used,
    in @b KB unit) of the mounted PVR disk partition
    */
    unsigned int free_size_in_KB;

    /**
    Member to specify the <b> record free size </b> (i.e. space still available
    for recording, in @a KB unit) of the PVR disk partition
    */
    unsigned int rec_free_size_in_KB;

    /**
    Member to specify the <b> time shift size </b> (in @a KB unit) of the
    mounted PVR disk partition
    */
    unsigned int tms_size_in_KB;

    /**
    Member to specify the @b status of the mounted PVR disk partition,
    as per the enum #aui_pvr_partition_status
    */
    unsigned int status;

} aui_pvr_partition_info;

/****************************Global Function List ****************************/

/**
@brief          Function used to initialize the PVR Module

@param[in]      p_init_cfg          = Pointer to a struct #aui_pvr_init_param
                                      which collects the initialization
                                      parameters

@return         @b AUI_RTN_SUCCESS  = Initializing of the PVR Module performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = Initializing of the PVR Module failed for
                                      some reasons

@note           For @b DVBS projects, please refer to the sample function
                @b test_pvr_init for more clarifications.\n
                For other projects, please refer to the enum #aui_pvr_project_mode
                and choose the right project mode.\n
                This function should be called @a only once through out the
                application
*/
AUI_RTN_CODE aui_pvr_init (

    const aui_pvr_init_param *p_init_cfg

    );

/**
@brief          Function is used to enable the VSC feature (CONAX card less CA)
                for PVR Module

@return         @b AUI_RTN_SUCCESS  = Enabling of the VSC feature performed
                                      successfully
@return         @b Other_Values     = Enabling of the VSC feature failed for
                                      some reasons

@note           User needs to enable the VSC feature before recording program
                with VSC platform otherwise either the function #aui_pvr_rec_open
                or #aui_pvr_ply_open will return an error.\n
                Furthermore, user @a must not use this function if the VSC feature
                is not supported by the platform otherwise either the function
                #aui_pvr_rec_open or #aui_pvr_ply_open will return an error.
*/
AUI_RTN_CODE aui_pvr_vsc_enable (

    void

    );

/**
@brief          Function used to de-initialize the PVR Module and release the
                related

@return         @b AUI_RTN_SUCCESS  = De-initializing of the PVR Module performed
                                      successfully
@return         @b Other_Values     = De-initializing of the PVR Module failed
                                      for some reasons

@note           This function should be called @b only once through out the
                application
*/
AUI_RTN_CODE aui_pvr_deinit (

    void

    );

/**
@brief          Function used to attach a disk to PVR Module and prepare it for
                recording and playback

@param[in]      p_apart_param       = Pointer to a struct #aui_pvr_disk_attach_info
                                      which collects the parameters for attaching
                                      and using a disk

@return         @b AUI_RTN_SUCCESS  = Attaching of a the disk performed successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid.
@return         @b Other_Values     = Attaching of the disk failed for some reasons
*/
AUI_RTN_CODE aui_pvr_disk_attach (

    const aui_pvr_disk_attach_info *p_apart_param

    );

/**
@brief          Function used to de-attach a disk from the PVR Module

@param[in]      p_depart_param      = Pointer to a struct #aui_pvr_disk_detach_info
                                      which collects the parameters for de-attaching
                                      a disk

@return         @b AUI_RTN_SUCCESS  = De-attaching of the disk performed successfully
@return         @b AUI_RTN_EINVAL   = The input parameter (i.e. [in]) is invalid
@return         @b Other_Values     = De-attaching of the disk failed for some
                                      reasons
*/
AUI_RTN_CODE aui_pvr_disk_detach (

    const aui_pvr_disk_detach_info *p_depart_param

    );

/**
@brief          Function used for registering a callback function used by the
                upper layer to deal with the message sent by the PVR ENG

@param[in]      handle              = #aui_hdl handle of the PVR Module opened
                                      by either the function #aui_pvr_rec_open or
                                      #aui_pvr_ply_open then initialized by the
                                      function #aui_pvr_init
@param[in]      fn_callback         = Pointer to the callback function to be
                                      registered, as per comment for the function
                                      pointer #aui_pvr_callback

@return         @b AUI_RTN_SUCCESS  = Registering of the callback function to
                                      the PVR Module performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Registering of the callback function to
                                      the PVR Module failed for some reasons
*/
AUI_RTN_CODE aui_pvr_callback_register (

    aui_hdl handle,

    aui_pvr_callback fn_callback

    );

/**
@brief          Function used to un-register the callback function used by the
                upper layer to deal with the message sent by the PVR ENG

@warning        After using this function, plese delete the callback function
                related to PVR handle

@param[in]      handle              = #aui_hdl handle of the PVR Module opened
                                      by either the function #aui_pvr_rec_open
                                      or #aui_pvr_ply_open then initialized by
                                      the function #aui_pvr_init
@param[in]      fn_callback         = Pointer to the callback function to be
                                      un-registered, as per comment for the
                                      function pointer #aui_pvr_callback

@return         @b AUI_RTN_SUCCESS  = Un-registering of the callback function
                                      from the PVR Module performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Un-registering of the callback function
                                      from the PVR Module failed for some reasons
*/
AUI_RTN_CODE aui_pvr_callback_unregister (

    aui_hdl handle,

    aui_pvr_callback fn_callback

    );

/**
@brief          Function used to open the recorder and start recording

@param[in]      p_rec_param         = Pointer to a struct #aui_record_prog_param,
                                      which collects the parameters for opening
                                      a recorder

@param[out]     p_handle            = #aui_hdl pointer to the handle of PVR
                                      Module (recorder) just opened

@return         @b AUI_RTN_SUCCESS  = Opening and starting of the recorder
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Opening and starting of the recorder failed
                                      for some reasons

@note           If the demux is free then this function will create a recorder
                otherwise will fail.\n
                If either Video or Audio PID is wrong then the recording file
                may be empty
*/
AUI_RTN_CODE aui_pvr_rec_open (

    const aui_record_prog_param *p_rec_param,

    aui_hdl *p_handle

    );

/**
@brief          Function used to change the state of the recorder

@param[in]      pvr_handle          = #aui_hdl handle of the PVR Module (recorder)
                                      opened by either the function #aui_pvr_rec_open
                                      or #aui_pvr_ply_open then initialized by
                                      the function #aui_pvr_init
@param[in]      new_state           = The desired recorder state to be set as
                                      defined in the enum #aui_pvr_rec_state, i,e.
                                      - @b Record
                                      - @b Pause
                                      - @b Stop
@return         @b AUI_RTN_SUCCESS  = Changing of the recorder state performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Changing of the recorder state failed for
                                      some reasons
*/
AUI_RTN_CODE aui_pvr_rec_state_change (

    aui_hdl pvr_handle,

    aui_pvr_rec_state new_state

    );

/**
@brief          Function used to stop recording and close the recorder

@param[in]      handle              = #aui_hdl handle of the PVR Module (recorder)
                                      already opened by the function
                                      #aui_pvr_rec_open or #aui_pvr_ply_open
                                      then initialized by the function #aui_pvr_init
@param[in]      sync                = Flag to specify the synchronization type,
                                      in particular
                                      - @b 0 = Asynchronous
                                      - @b 1 = Synchronization

@return         @b AUI_RTN_SUCCESS  = Stopping and closing of the recorder
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Stopping and closing of the recorder
                                      failed for some reasons

@note           If @b sync = 1 then the recorder will be stopped and the function
                #aui_pvr_rec_close will return.\n
                If @b sync = 0 then the function #aui_pvr_rec_close will return
                immediately and the recorder will be stopped asynchronously
*/
AUI_RTN_CODE aui_pvr_rec_close (

    aui_hdl handle,

    int sync

    );

/**
@brief          Function used to start the player to playback the recorded content

@warning        It is very important to note that the live stream decoding from
                the front-end @a must be stopped (stop the DMX, DECV and DECA of
                live decoding) before calling this function, otherwise the video
                decoder will not be able to decode correctly

@param[out]     p_handle            = #aui_hdl pointer to the handle of the
                                      PVR Module (player) just opened

@param[in]      p_ply_param         = Pointer to a struct #aui_ply_param, which
                                      collects the parameters for playback

@return         @b AUI_RTN_SUCCESS  = Starting of the player to playback the
                                      recorded content performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Starting of the player to playback the
                                      recorded content failed for some reasons

@note           The full path of the recording file is required, and a re-encrypted
                recording file requires a CA card inserted
*/
AUI_RTN_CODE aui_pvr_ply_open (

    aui_ply_param *p_ply_param,

    aui_hdl *p_handle

    );

/**
@brief          Function used to change the player state.

@param[in]      handler             = #aui_hdl handle of the PVR Module (player)
                                      already opened by the function
                                      #aui_pvr_rec_open or #aui_pvr_ply_open
                                      then initialized by the function
                                      #aui_pvr_init
@param[in]      new_state           = The desired player state to be set as
                                      defined in the enum #aui_pvr_play_state,
                                      i.e.
                                      - @b Stop
                                      - @b Play
                                      - @b Pause
                                      - <b> Fast forward </b>
                                      - @b Step
                                      - <b> Fast backward / Fast rewind </b>
                                      - @b Slow
                                      - <b> Rewind slow </b>
                                      - @b Speed
                                      - @b Recording
@param[in]      state_param         = This parameter is @a only used when setting
                                      the speed value as per definition in the
                                      enum #AUI_PlayerSpeed_E


@return         @b AUI_RTN_SUCCESS  = Changing of the player state performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Changing of the player state failed for
                                      some reasons
*/
AUI_RTN_CODE aui_pvr_ply_state_change (

    aui_hdl handler,

    aui_pvr_play_state new_state,

    int state_param

    );

/**
@brief          Function used to seek a particular point of the recorded content

@param[in]      handler             = #aui_hdl handle of the PVR Module (player)
                                      already opened by the function
                                      #aui_pvr_rec_open or #aui_pvr_ply_open
                                      then initialized by the function
                                      #aui_pvr_init
@param[in]      pos_in_sec          = The position to be sought
@param[in]      play_pos_flag       = Desired player seeking mode to be set as
                                      defined in the enum #aui_pvr_play_position_flag

@return         @b AUI_RTN_SUCCESS  = Seeking of the particular point of the
                                      recorded content performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Seeking of the particular point of the
                                      recorded content failed for some reasons
*/
AUI_RTN_CODE aui_pvr_ply_seek (

    aui_hdl handler,

    const int pos_in_sec,

    const aui_pvr_play_position_flag play_pos_flag

    );

/**
@brief          Function used to stop the player

@attention      The function #aui_dmx_data_path_set must be used after finishing
                the AUI PVR playback (i.e. after using the function #aui_pvr_ply_close)
                as it may change the current DMX data path.

@param[in]      handle              = #aui_hdl handle of the PVR Module (player)
                                      already opened by the function #aui_pvr_rec_open
                                      or #aui_pvr_ply_open then initialized by the
                                      function #aui_pvr_init
@param[in]      p_stop_param        = Pointer to a struct #aui_pvr_stop_ply_param,
                                      which collects the parameters to stop the
                                      player

@return         @b AUI_RTN_SUCCESS  = Stopping of the player performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in]) is
                                      invalid
@return         @b Other_Values     = Stopping of the player failed for some reasons
*/
AUI_RTN_CODE aui_pvr_ply_close (

    aui_hdl handle,

    const aui_pvr_stop_ply_param *p_stop_param

    );

/**
@brief          Function used to get PVR parameters from the recorder or player

@param[in]      handle              = #aui_hdl handle of the PVR device (recorder
                                      or player) already opened by the function
                                      #aui_pvr_rec_open or #aui_pvr_ply_open
                                      then initialized by the function
                                      #aui_pvr_init
@param[in]      cmd                 = The specific PVR command to get PVR
                                      parameters from the recorder or player as
                                      defined in the enum #aui_pvr_cmd

@param[out]     p_param_1           = Pointer to the first PVR parameter to be
                                      gotten as per the description of the
                                      specific PVR command defined in the enum
                                      #aui_pvr_cmd
@param[out]     p_param_2           = Pointer to the second PVR parameter to be
                                      gotten as per the description of the
                                      specific PVR command defined in the enum
                                      #aui_pvr_cmd
@param[out]     p_param_3           = Pointer to the second PVR parameter to be
                                      gotten as per the description of the
                                      specific PVR command defined in the enum
                                      #aui_pvr_cmd

@return         @b AUI_RTN_SUCCESS  = Getting of the PVR parameter from the
                                      recorder or player performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting ot the PVR parameter from the
                                      recorder or player failed for some reasons

@note           Usually, the parameter @b p_parma_1 is the @a only one to be
                returned while the parameters @b p_param_2 and @b p_param_3 are
                @a reserved to ALi R&D Dept then they take NULL as value unless
                otherwise notice.\n
                At present, the following commands in #aui_pvr_cmd are supported
                to get PVR parameters:
                - #AUI_PVR_REC_STATES
                - #AUI_PVR_REC_DMX_ID
                - #AUI_PVR_REC_TIME_S
                - #AUI_PVR_REC_TIME_TMS
                - #AUI_PVR_REC_CAPBILITY
                - #AUI_PVR_PLAY_SPEED
                - #AUI_PVR_PLAY_DIRECTION
                - #AUI_PVR_PLAY_TIME_S

@note           The others commands are not supported.
*/
AUI_RTN_CODE aui_pvr_get (

    aui_hdl handle,

    aui_pvr_cmd cmd,

    unsigned int *p_param_1,

    unsigned int *p_param_2,

    unsigned int *p_param_3

    );

/**
@brief          Function used to set PVR parameters to the recorder or player

@param[in]      handle              = #aui_hdl handle of the PVR device (recorder
                                      or player) already opened by the function
                                      #aui_pvr_rec_open or #aui_pvr_ply_open
                                      then initialized by the function
                                      #aui_pvr_init
@param[in]      cmd                 = The specific PVR command to set the
                                      parameters to the recorder or player as
                                      defined in the enum #aui_pvr_cmd
@param[in]      param_1             = Pointer to the first PVR parameter to be
                                      set as per the description of the specific
                                      PVR command defined in the enum #aui_pvr_cmd
@param[in]      param_2             = Pointer to the second PVR parameter to be
                                      set as per the description of the specific
                                      PVR command defined in the enum #aui_pvr_cmd
@param[in]      param_3             = Pointer to the second PVR parameter to be
                                      set as per the description of the specific
                                      PVR command defined in the enum #AUI_PVR_CM

@return         @b AUI_RTN_SUCCESS  = Setting of the PVR parameter to the
                                      recorder or player performed successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the PVR parameter to the
                                      recorder or player failed for some reasons

@note           Usually, the parameter @b p_parma_1 is the @a only one to be
                used for setting PVR parameters while the parameters @b p_param_2
                and @b p_param_3 are @a reserved to  ALi R&D Dept. then they
                take NULL as value unless otherwise notice.\n
                At present, the following commands in #aui_pvr_cmd are supported
                to set PVR parameters:
                - #AUI_PVR_REC_CAPBILITY
@note           The others commands are not supported.
*/
AUI_RTN_CODE aui_pvr_set (

    aui_hdl handle,

    aui_pvr_cmd cmd,

    unsigned int param_1,

    unsigned int param_2,

    unsigned int param_3

    );

/**
@brief          Function used to set Uniform Resource Identifier (URI) information
                to the recording file

@param[in]      handle              = #aui_hdl handle of the PVR device already
                                      opened by the function #aui_pvr_rec_open
                                      or #aui_pvr_ply_open then initialized by
                                      the function #aui_pvr_init
@param[in]      p_uri               = Pointer to a struct #aui_pvr_uri which
                                      collects the URI information as per CONAX
                                      Specifications to be set to the recording
                                      file

@return         @b AUI_RTN_SUCCESS  = Setting of the URI information performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Setting of the URI information failed for
                                      some reasons
*/
AUI_RTN_CODE aui_pvr_set_uri (

    aui_hdl handle,

    const aui_pvr_uri *p_uri

    );

/**
@brief          Function used to get Uniform Resource Identifier (URI) information
                from the recording file

@param[in]      handle              = #aui_hdl handle of the PVR already opened
                                      by the function #aui_pvr_rec_open or
                                      #aui_pvr_ply_open then initialized by the
                                      function #aui_pvr_init

@param[out]     p_uri               = Pointer to a struct #aui_pvr_uri which
                                      collects the URI information as per CONAX
                                      Specifications to be gotten from the
                                      recording file

@return         @b AUI_RTN_SUCCESS  = Getting of the URI information performed
                                      successfully
@return         @b AUI_RTN_EINVAL   = At least one input parameter (i.e. [in])
                                      is invalid
@return         @b Other_Values     = Getting of the URI information failed for
                                      some reasons
*/
AUI_RTN_CODE aui_pvr_get_uri (

    aui_hdl handle,

    aui_pvr_uri *p_uri

    );

/**
@brief          Function used to get the PVR recording index by the path of the
                PVR recording file

@param[in]      pc_path             = Path of the PVR recording file to be used
                                      to get the PVR recording index

@param[out]     p_index             = PVR recording index just gotten by the
                                      path of the PVR recoding file

@return         @b AUI_RTN_SUCCESS  = Getting of the PVR recording index
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in],
                                      [out]) is invalid
@return         @b Other_Values     = Getting of the PVR recording index failed
                                      for some reasons
*/
AUI_RTN_CODE aui_pvr_get_index_by_path (

    char *pc_path,

    unsigned int *p_index

    );

/**
@brief          Function used to get the total number of Uniform Resource
                Identifier (URI) information from the recording file

@param[in]      index               = PVR recording index

@param[out]     p_cnt               = The total number of URI information

@return         @b AUI_RTN_SUCCESS  = Getting of the total number of URI
                                      information performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the total number of URI
                                      information failed for some reasons
*/
AUI_RTN_CODE aui_pvr_get_uri_cnt (

    unsigned int index,

    unsigned int *p_cnt

    );

/**
@brief          Function used to get a set of Uniform Resource Identifier (URI)
                information from the recording file

@param[in]      index               = PVR recording index.
@param[in]      base                = Number of desired URI data to be gotten

@param[out]     cnt                 = Number of URI data actually returned
@param[out]     p_uri               = Pointer to a struct #aui_pvr_uri which
                                      collects the URI data just gotten

@return         @b AUI_RTN_SUCCESS  = Getting of the desired set of URI information
                                      performed successfully
@return         @b AUI_RTN_EINVAL   = At least one parameter (i.e. [in], [out])
                                      is invalid
@return         @b Other_Values     = Getting of the desired set of URI information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_get_uri_sets (

    unsigned int index,

    unsigned int base,

    unsigned int cnt,

    aui_pvr_uri *p_uri

    );

/**
@brief         Function used to get the fingerprint information from the recording
               file

@param[in]     handle               = #aui_hdl handle of the PVR already opened
                                      by the function #aui_pvr_rec_open or
                                      #aui_pvr_ply_open then initialized by the
                                      function #aui_pvr_init

@param[out]    p_fingerprint        = Pointer to a struct #aui_pvr_finger_info
                                      which collects the fingerprint information
                                      just gotten

@return        @b AUI_RTN_SUCCESS   = Getting of the fingerprint information
                                      performed successfully
@return        @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in], [out])
                                      is invalid
@return        @b Other_Values      = Getting of the fingerprint information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_get_finger_print (

    aui_hdl handle,

    aui_pvr_finger_info *p_fingerprint

    );

/**
@brief         Function used to set fingerprint information to the recording
               file

@param[in]     handle               = #aui_hdl handle of the PVR already opened
                                      by the function #aui_pvr_rec_open or
                                      #aui_pvr_ply_open) then initialized by the
                                      function #aui_pvr_init
@param[in]     p_fingerprint        = Pointer to a struct #aui_pvr_finger_info
                                      which collects the fingerprint information
                                      to be set

@return        @b AUI_RTN_SUCCESS   = Setting of the fingerprint information
                                      performed successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                      is invalid
@return        @b Other_Values      = Setting of the fingerprint information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_set_finger_print (

    aui_hdl handle,

    aui_pvr_finger_info *p_fingerprint

    );

/**
@brief         Function used to set maturity rating information to the recording
               file

@param[in]     handle               = #aui_hdl handle of the PVR already opened
                                      by the function #aui_pvr_rec_open or
                                      #aui_pvr_ply_open then initialized by the
                                      function #aui_pvr_init
@param[in]     play_time            = Time stamp (in @a millisecond unit) of
                                      the maturity rating information to be set
@param[in]     rating               = Maturity rating information to be set

@return        @b AUI_RTN_SUCCESS   = Setting of the maturity rating information
                                      performed successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                      is invalid
@return        @b Other_Values      = Setting of the maturity rating information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_set_mat_rating (

    aui_hdl handle,

    unsigned int play_time,

    unsigned int rating

    );

/**
@brief         Function used to get maturity rating information from the recording
               file by PVR handler

@param[in]     handle               = #aui_hdl handle of the PVR already opened
                                      and to be used to get maturity rating
                                      information
@param[in]     play_time            = Time stamp (in @a millisecond unit) of the
                                      maturity rating information to be gotten
@param[in]     p_rating             = The maturity rating information just gotten

@return        @b AUI_RTN_SUCCESS   = Getting of the maturity rating information
                                      performed successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                      is invalid
@return        @b Other_Values      = Getting of the maturity rating information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_get_mat_rating (

    aui_hdl handle,

    unsigned int play_time,

    unsigned int *p_rating

    );

/**
@brief         Function used to get maturity rating information from the recording
               file by PVR recording index

@param[in]     index                = PVR recording index to be used
@param[in]     play_time            = Time stamp (in @a millisecond unit) of the
                                      maturity rating information to be gotten
@param[in]     p_rating             = The maturity rating information just gotten

@return        @b AUI_RTN_SUCCESS   = Getting of the maturity rating information
                                      performed successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                      is invalid
@return        @b Other_Values      = Getting of the maturity rating information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_get_mat_rating_by_index (

    unsigned short index,

    unsigned int play_time,

    unsigned int *p_rating

    );

/**
@brief         Function used to rename the record item by the PVR recording index

@param[in]     index                = The PVR recording index to be used for
                                      renaming the record item
@param[in]     p_new_name           = The new name of the record item
@param[in]     len                  = The length of the new name of the record
                                      item

@return        @b AUI_RTN_SUCCESS   = Renaming of the record item performed
                                      successfully
@return        @b AUI_RTN_EINVAL    = At least one input parameter (i.e. [in])
                                      is invalid
@return        @b Other_Values      = Renaming of the record item failed for
                                      some reasons
*/
AUI_RTN_CODE aui_pvr_rename_rec_item (

    unsigned short index,

    char *p_new_name,

    int len

    );

/**
@brief         Function used to get information about a PVR disk partition
               by the related mount name

@param[in]     pc_mount_name        = Pointer to the mount name of the targeted
                                      PVR disk partition

@param[out]    p_pvr_partition_info = Pointer to a struct #aui_pvr_partition_info
                                      which collects the PVR disk partition
                                      information just gotten

@return        @b AUI_RTN_SUCCESS   = Getting of the PVR disk partition information
                                      performed successfully
@return        @b AUI_RTN_EINVAL    = At least one parameter (i.e. [in],  [out])
                                      is invalid
@return        @b Other_Values      = Getting of the PVR disk partition information
                                      failed for some reasons
*/
AUI_RTN_CODE aui_pvr_get_parition_info (

    char *pc_mount_name,

    aui_pvr_partition_info *p_pvr_partition_info

    );

/*****************************************************************************/
/*           List of definitions which are for deprecated items only,        */
/*                  please keep here for compatibility reasons               */
/*                                                                           */
/*                                  START                                    */
/*****************************************************************************/

/// @cond

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define AUI_PVR_MSG_REC_STOP_URICHANGE  AUI_EVNT_PVR_MSG_REC_STOP_URICHANGE

#define AUI_PVR_MSG_TMS_STOP_URICHANGE  AUI_EVNT_PVR_MSG_TMS_STOP_URICHANGE

#define AUI_PVR_MSG_TMS_PAUSE_URICHANGE AUI_EVNT_PVR_MSG_TMS_PAUSE_URICHANGE

#define AUI_PVR_MSG_TMS_RESUME_URICHANGE    AUI_EVNT_PVR_MSG_TMS_RESUME_URICHANGE

#define AUI_PVR_MSG_REC_TOO_SHORT       AUI_EVNT_PVR_MSG_REC_TOO_SHORT

#define AUI_PVR_DEBUG_LEVEL aui_pvr_debug_level

#define AUI_PVR_REC_TYPE aui_pvr_rec_type

/**
@note PVR_PROJECT_MODE is already defined in libpvr
*/
// #define PVR_PROJECT_MODE aui_pvr_project_mode

#define AUI_EVNT_PVR aui_evnt_pvr

#define AUI_PVR_CMD aui_pvr_cmd

#define AUI_PVR_PID_TYPE aui_pvr_pid_type

#define fn_aui_pvr_callback aui_pvr_callback

/**
Enum to specify different <b> Audio Channel </b>
*/
typedef enum {

    /**
    Value to specify the @b Left Audio Channel
    */
    AUI_AUDIO_DUP_L =0,

    /**
    Value to specify the @b Right Audio Channel
    */
    AUI_AUDIO_DUP_R =1,

    /**
    Value to specify @b Mono Audio Channel
    */
    AUI_AUDIO_DUP_MONO =3,

    /**
    Value to specify @b Stereo Audio Channel
    */
    AUI_AUDIO_CH_STEREO =2

} AUI_PVR_AUDIO_CHANNEL;

#define AUI_REC_MODE_NORMAL AUI_PVR_REC_MODE_NORMAL

#define AUI_REC_MODE_TMS AUI_PVR_REC_MODE_TMS

#define AUI_REC_MODE_TP AUI_PVR_REC_MODE_TP

#define AUI_PVR_REC_MODE aui_pvr_rec_mode

#define AUI_REC_MODE_FULL_SCREEN AUI_PVR_PLAY_MODE_FULL_SCREEN

#define AUI_REC_MODE_PRE_VIEW AUI_PVR_PLAY_MODE_PREVIEW

#define AUI_PVR_PLAY_MODE aui_pvr_play_mode

#define AUI_PVR_STATE aui_pvr_state

#define AUI_PVR_STATE_IDEL AUI_PVR_STATE_IDLE

#define AUI_REC_STATE_RECORDING AUI_PVR_REC_STATE_RECORDING

#define AUI_REC_STATE_PAUSE AUI_PVR_REC_STATE_PAUSE

#define AUI_REC_STATE_STOP AUI_PVR_REC_STATE_STOP

#define AUI_PVR_REC_STATE aui_pvr_rec_state

#define AUI_P_STOP_AND_REOPEN AUI_PVR_STOP_AND_REOPEN

#define AUI_P_STOPPED_ONLY AUI_PVR_STOPPED_ONLY

#define AUI_PVR_STOP_MODE aui_pvr_stop_mode

/**
Enum to specify different <b> playback status </b>

@note   This enum is @a reserved to ALi R&D Dept. then user can ignore it
*/
typedef enum {
    /**
    Value to specify that the playback is @b stopped
    */
    AUI_NV_STOP         = 0,

    /**
    Value to specify that the playback is <b> in progress normally </b>
    */
    AUI_NV_PLAY         = 1,

    /**
    Value to specify that the playback is @b paused
    */
    AUI_NV_PAUSE        = 2,

    /**
    Value to specify the <b> fast forward </b> playback status
    */
    AUI_NV_FF           = 3,

    /**
    Value to specify the <b> step-by-step (i.e. frame-by-frame) </b> playback
    status

    @note   That refers only to the video
    */
    AUI_NV_STEP         = 4,

    /**
    Value to specify the <b> backward/fast rewind </b> playback status
    */
    AUI_NV_FB           = 5,

    /**
    Value to specify the <b> slow </b> playback status
    */
    AUI_NV_SLOW         = 6,

    /**
    Value to specify the <b> reverse slow </b> playback status
    */
    AUI_NV_REVSLOW      = 7,

    /**
    Value to specify that PVR is recording a certain program
    */
    AUI_NV_RECORDING    = 11

} AUI_PVR_PLAYBACK_STATE;

/**
Enum to specify different <b> speeds for playing </b> recorded content
*/
typedef enum {

    /**
    Value to specify @b 1x as play speed
    */
    AUI_PVR_PLAY_SPEED_1X=1,

    /**
    Value to specify @b 2x play speed
    */
    AUI_PVR_PLAY_SPEED_2X,

    /**
    Value to specify @b 4x play speed
    */
    AUI_PVR_PLAY_SPEED_4X,

    /**
    Value to specify @b 8x as play speed
    */
    AUI_PVR_PLAY_SPEED_8X,

    /**
    Value to specify @b 16x as play speed
    */
    AUI_PVR_PLAY_SPEED_16X,

    /**
    Value to specify @b 32x as play speed
    */
    AUI_PVR_PLAY_SPEED_32X

} AUI_PVR_PLY_SPEED;

/**
Enum to specify different <b> configurations for playing </b> a recorded content
*/
typedef enum {

    /**
    Value to specify playing a recorded content with <b> default configuration </b>
    */
    AUI_P_OPEN_DEFAULT          = 0,

    /**
    Value to specify playing a recorded content <b> without closing/opening VPO
    (VIdeo Post Process) and showing logo </b>

    @note   @b VPO is an internal feature of the display engine (DIS) used to
            optimize the displaying on the screen
    */
    AUI_P_OPEN_VPO_NO_CTRL      = (1<<0),

    /**
    Value to specify playing recorded content as @b preview

    @note   This value is for the recording manager
    */
    AUI_P_OPEN_PREVIEW          = (1<<1),

    /**
    Value to specify playing a recorded content from the @b head
    */
    AUI_P_OPEN_FROM_HEAD        = (1<<2),

    /**
    Value to specify playing a recorded content from the @b tail

    @note   That is used in Time Shift mode
    */
    AUI_P_OPEN_FROM_TAIL        = (1<<3),

    /**
    Value to specify playing a recorded content from a <b> certain time </b>
    */
    AUI_P_OPEN_FROM_PTM         = (1<<4),

    /**
    Value to specify playing a recorded content from a <b> certain position </b>
    */
    AUI_P_OPEN_FROM_POS         = (1<<5),

    /**

    Value to specify playing a recorded content from the last played position
    saved after playback
    */
    AUI_P_OPEN_FROM_LAST_POS    = (1<<6)

} AUI_PVR_OPEN_CONFIG;

/**
Enum to specify different <b> PVR debug levels </b> available to be set
*/
typedef enum aui_pvr_debug_level {

    /**
    Value to specify </b> no debugging </b>

    @note   That is the default setting
    */
    AUI_PVR_DEBUG_NONE      = 0,

    /**
    Value to specify @b Player debugging
    */
    AUI_PVR_DEBUG_PLAYER    = (1<<0),

    /**
    Value to specify @b Recorder debugging
    */
    AUI_PVR_DEBUG_RECORDER  = (1<<1),

    /**
    Value to specify <b> PVR Data Information </b> debugging
    */
    AUI_PVR_DEBUG_DATA      = (1<<2),

    /**
    Value to specify <b> PVR Cache Information </b> debugging
    */
    AUI_PVR_DEBUG_CACHE     = (1<<3),

    /**
    Value to specify <b> PVR Device Information </b> debugging
    */
    AUI_PVR_DEBUG_DEVICE    = (1<<4),

    /**
    Value to specify <b> Other PVR Information </b> debugging, such as
    - Task information
    - Callback Route information
    - etc.
    */
    AUI_PVR_DEBUG_OTHER     = (1<<5),

    /**
    Value to specify <b> TS Parser Information </b> debugging

    @note   This debug level can be set when PVR is used as TS parser
    */
    AUI_PVR_DEBUG_TS        = (1<<6),

    /**
    Value to specify <b> All debug level </b>
    */
    AUI_PVR_DEBUG_ALL       = 0xff

} aui_pvr_debug_level;

#define AUI_PLAYER_POSITION_FROM_HEAD AUI_PVR_PLAY_POSITION_FROM_HEAD

#define AUI_PLAYER_POSITION_FROM_CURRENT AUI_PVR_PLAY_POSITION_FROM_CURRENT

#define AUI_PLAYER_POSITION_FROM_END AUI_PVR_PLAY_POSITION_FROM_END

#define AUI_PlayPosition_E aui_pvr_play_position_flag

#define AUI_PLY_STATE_STOP  AUI_PVR_PLAY_STATE_STOP

#define AUI_PLY_STATE_PLAY  AUI_PVR_PLAY_STATE_PLAY

#define AUI_PLY_STATE_PAUSE AUI_PVR_PLAY_STATE_PAUSE

#define AUI_PLY_STATE_FF    AUI_PVR_PLAY_STATE_FF

#define AUI_PLY_STATE_STEP  AUI_PVR_PLAY_STATE_STEP

#define AUI_PLY_STATE_FB    AUI_PVR_PLAY_STATE_FB

#define AUI_PLY_STATE_SLOW  AUI_PVR_PLAY_STATE_SLOW

#define AUI_PLY_STATE_REVSLOW  AUI_PVR_PLAY_STATE_REVSLOW

#define AUI_PLY_STATE_SPEED AUI_PVR_PLAY_STATE_SPEED

#define AUI_PVR_PLAY_STATE aui_pvr_play_state

#define AUI_PlayerSpeed_E aui_player_speed;

#define AUI_PVR_AUDIO_TTX_PID  AUI_PVR_TTX_PID

#define AUI_PVR_AUDIO_SUBTITLE_PID  AUI_PVR_SUBTITLE_PID

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

/* END OF FIlE */


