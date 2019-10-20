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
Current ALi Author: Davy.Wu
Last update:        2017.02.10
-->

@file aui_errno_stb.h

@brief  Set-Top Box Errors (ERRNO_STB) Module

        <b> Set-Top Box Errors (ERRNO_STB) Module </b> is used to define error
        code for each AUI API module.

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
            List of Included Files
            </h2></td> @endhtmlonly
            No AUI API Header File is included.
*/

#ifndef _AUI_ERRNO_STB_H

#define _AUI_ERRNO_STB_H


/*************************Included Header File List***************************/

/**
Macro to specify a shift of N=16 bits in an AUI STB error code in order to make
it different with the return codes defined in @b aui_errno_sys.h
*/

#define AUI_ERR_SHIFT 16

/// @cond

/**
Enum to specify all STB features related to the error codes which might be returned
by AUI APIs
*/
enum {

    /**
    Value to specify a <b> ring buffer writing operation error </b>, in
    particular the buffer space is not enough
    */
    AUI_RTN_RING_WT_NO_BUF = (1 << AUI_ERR_SHIFT),

    /**
    Value to specify a <b> ring buffer reading operation error </b>, in particular
    the data to read out are not enough
    */
    AUI_RTN_RING_RD_NO_BUF,

    /**
    Value to specify a <b> timeout event </b>
    */
    AUI_RTN_OS_EVENT_TIMEOUT,

    /**
    Value to specify a <b> MSGQ timeout </b>
    */
    AUI_RTN_OS_MSGQ_TIMEOUT,

    /**
    Value to specify a <b> SEM timeout </b>
    */
    AUI_RTN_OS_SEM_TIMEOUT,

    /**
    Value to specify a <b> Mutex timeout </b>
    */
    AUI_RTN_OS_MUTEX_TIMEOUT,

    /**
    Value to specify <b> Mutex invalid </b>
    */
    AUI_RTN_OS_MUTEX_EINVAL,

    /**
    Value to specify a <b> Mutex Resource Deadlock </b> occured
    */
    AUI_RTN_OS_MUTEX_EDEADLK,

    /**
    Value to specify a <b> Mutex operation not permitted </b>
    */
    AUI_RTN_OS_MUTEX_EPERM,

    /**
    Value to specify <b> Mutex busy </b>
    */
    AUI_RTN_OS_MUTEX_BUSY,

    /**
    Value to specify that the Mutex is not used in the current AUI Task because
    the current task is not created by AUI
    */
    AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM,

    /**
    Value to specify that the Smart Card has plugged in
    */
    AUI_RTN_SMC_PLUGIN,

    /**
    Value to specify that the Smart Card has plugged out
    */
    AUI_RTN_SMC_PLUGOUT,

    /**
    Value to specify that the Smart Card is in lock status then cannot execute
    other commands
    */
    AUI_RTN_SMC_NOT_LOCK_OWNER,

    /**
    Value to specify that the Smart Card is busy for processing data, then needs
    to wait a moment before sending new data to it
    */
    AUI_RTN_SMC_CARD_BUSY,

    /**
    Value to specify an operation timeout error
    */
    AUI_RTN_SMC_TIMEOUT,

    /**
    Value to specify that the answer is not correct
    */
    AUI_RTN_SMC_ERROR_ANSWER,

    /**
    Value to specify a transfer data validation error
    */
    AUI_RTN_SMC_ERROR_PARITY,

    /**
    Value to specify that the Smart Card is ready to execute command or receive
    data
    */
    AUI_RTN_SMC_READY,

    /**
    Value to specify that the Smart Card is an invalid card because, for example:
    - Plug backward
    - Wrong card type which cannot be recognized
    - etc.
    */
    AUI_RTN_SMC_MUTE,

    /**
    Value to specify that the Smart Card is not correct because, for example:
    - No match the current using CA-system
    - The card is badly worn
    - etc.
    */
    AUI_RTN_SMC_ERROR_CARD,

    /**
    Value to specify that the protocol is invalid
    */
    AUI_RTN_SMC_INVALID_PROTOCOL,

    /**
    Value to specify that the Smart Card does not reset after powering on
    */
    AUI_RTN_SMC_NOT_RESET,

    /**
    Value to specify that an error occurred in the underlying driver when calling
    the function.

    @note Please check the error log for further details.
    */
    AUI_RTN_DRIVER_ERROR,
    /**
    Value to specify no initialization
    */
    AUI_RTN_NOT_INIT,

    /**
    Value to specify no resources available
    */
    AUI_RTN_NO_RESOURCE,

    /**
    Value to specify that a callback already added
    */
    AUI_RTN_FS_CBARD_ADDED,

    /**
    Value to specify a data type error
    */
    AUI_RTN_ERR_DATA_TYPE,

    /**
    Value to specify that a callback already exists
    */
    AUI_RTN_CALLBACK_EXIST,

    /**
    Value to specify that a callback doesn't exist
    */
    AUI_RTN_CALLBACK_UNEXIST,

    /**
    Value to specify that URI verifing failed
    */
    AUI_RTN_URI_VERIFY_FAILED,

    /**
    Value to specify a not supported feature
    */
    AUI_RTN_FEATURE_NOT_SUPPORTED,
};

#ifndef DOXYGEN_SKIP_DEPRECATED_AUI_API

#define OS_ERR                      AUI_RTN_FAIL
#define OS_EVENT_TIMEOUT            AUI_RTN_OS_EVENT_TIMEOUT
#define OS_MSGQ_TIMEOUT             AUI_RTN_OS_MSGQ_TIMEOUT
#define OS_SEM_TIMEOUT              AUI_RTN_OS_SEM_TIMEOUT
#define OS_MUTEX_TIMEOUT            AUI_RTN_OS_MUTEX_TIMEOUT
#define OS_MUTEX_EINVAL             AUI_RTN_EINVAL
#define OS_MUTEX_EDEADLK            AUI_RTN_OS_MUTEX_EDEADLK
#define OS_MUTEX_EPERM              AUI_RTN_OS_MUTEX_EPERM
#define OS_MUTEX_BUSY               AUI_RTN_EBUSY
#define OS_MUTEX_TASK_ID_SYSTEM     AUI_RTN_OS_MUTEX_TASK_ID_SYSTEM
#define OS_MUTEX_NOT_SUPPORT        AUI_RTN_FEATURE_NOT_SUPPORTED
#define KL_ERR                      AUI_RTN_FAIL
#define DSC_ERR                     AUI_RTN_FAIL
#define DSC_ERR_NO_RES_STREAM_ID    AUI_RTN_NO_RESOURCE
#define DSC_ERR_NO_RES_SUB_DEV      AUI_RTN_NO_RESOURCE
#define DSC_ERR_NO_INIT             AUI_RTN_NOT_INIT
#define DSC_ERR_DATA_TYPE_ERR       AUI_RTN_ERR_DATA_TYPE
#define RTC_ERR                     AUI_RTN_FAIL
#define UART_ERR_NO_RESOURCE        AUI_RTN_NO_RESOURCE
#define PANEL_ERR_NO_RESOURCE       AUI_RTN_NO_RESOURCE
#define DMX_ERR                     AUI_RTN_FAIL
#define DMX_NO_INIT                 AUI_RTN_NOT_INIT
#define DIS_ERR                     AUI_RTN_FAIL
#define NIM_ERR                     AUI_RTN_FAIL
#define FS_ERR_CBARD_ADDED          AUI_RTN_FS_CBARD_ADDED
#define AUI_PVR_ERR                 AUI_RTN_FAIL
#define AUI_PVR_BUSY                AUI_RTN_EBUSY
#define AUI_PVR_CALLBACK_EXIST      AUI_RTN_CALLBACK_EXIST
#define AUI_PVR_CALLBACK_UNEXIST    AUI_RTN_CALLBACK_UNEXIST
#define AUI_PVR_URI_VERIFY_FAILED   AUI_RTN_URI_VERIFY_FAILED
#define MP_ERR                      AUI_RTN_FAIL
#define IMAGE_ERR                   AUI_RTN_FAIL
#define MUSIC_ERR                   AUI_RTN_FAIL
#define AUI_I2C_ERROR               AUI_RTN_FAIL
#define AUI_I2C_BAD_PARAMETER       AUI_RTN_EINVAL
#define AUI_SMC_ERROR_BAD_PARAMETER AUI_RTN_EINVAL
#define AUI_SMC_ERROR_ERROR_PARITY  AUI_RTN_SMC_ERROR_PARITY
#define AUI_SMC_ERROR_ERROR_ANSWER  AUI_RTN_SMC_ERROR_ANSWER
#define AUI_SMC_ERROR_TIMEOUT       AUI_RTN_SMC_TIMEOUT
#define AUI_SMC_ERROR_OUT           AUI_RTN_SMC_PLUGOUT
#define AUI_SMC_ERROR_IN            AUI_RTN_SMC_PLUGIN
#define AUI_SMC_ERROR_CARD_BUSY     AUI_RTN_SMC_CARD_BUSY
#define AUI_SMC_ERROR_NOT_LOCK_OWNER    AUI_RTN_SMC_NOT_LOCK_OWNER
#define AUI_SMC_ERROR_READY         AUI_RTN_SMC_READY
#define AUI_SMC_ERROR_MUTE          AUI_RTN_SMC_MUTE
#define AUI_SMC_ERROR_ERROR_CARD    AUI_RTN_SMC_ERROR_CARD
#define AUI_SMC_ERROR_INVALID_PROTOCOL  AUI_RTN_SMC_INVALID_PROTOCOL
#define AUI_SMC_ERROR_NOT_RESET     AUI_RTN_SMC_NOT_RESET
#define AUI_SMC_ERROR_FAILURE       AUI_RTN_FAIL
#define AUI_SMC_ERROR_NOT_SUPPORT   AUI_RTN_FEATURE_NOT_SUPPORTED
#define AUI_FLASH_SUCCESS           AUI_RTN_SUCCESS
#define AUI_FLASH_PARAM_INVALID     AUI_RTN_EINVAL
#define AUI_FLASH_NO_MEMORY         AUI_RTN_ENOMEM
#define AUI_FLASH_FEATURE_NOT_SUPPORTED AUI_RTN_FEATURE_NOT_SUPPORTED
#define AUI_FLASH_DRIVER_ERROR      AUI_RTN_DRIVER_ERROR
#define aui_flash_errno             AUI_RTN_CODE
#define AUI_GFX_SUCCESS             AUI_RTN_SUCCESS
#define AUI_GFX_PARAM_INVALID       AUI_RTN_EINVAL
#define AUI_GFX_NO_MEMORY           AUI_RTN_ENOMEM
#define AUI_GFX_FEATURE_NOT_SUPPORTED   AUI_RTN_FEATURE_NOT_SUPPORTED
#define AUI_GFX_DRIVER_ERROR        AUI_RTN_DRIVER_ERROR
#define AUI_GFX_OTHTER_ERROR        AUI_RTN_FAIL

#endif

/// @endcond

#endif

/* END OF FILE */


