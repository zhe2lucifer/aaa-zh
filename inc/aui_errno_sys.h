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
Last update:        2017.02.25
-->

@file aui_errno_sys.h

@brief  System Errors (ERRNO_SYS) Module

        <b> System Errors (ERRNO_SYS) Module </b> is used to define the common
        different return values of different functions, i.e. the different base
        system error numbers which will arise a specific action.\n

@copyright Copyright &copy; 2015 ALi &reg; Corporation. All Rights Reserved

@htmlonly   <td colspan="2"><h2 class="groupheader">
List of Included Files
</h2></td> @endhtmlonly
*/

#ifndef _AUI_ERRNO_SYS_H

#define _AUI_ERRNO_SYS_H

/*******************************Global Macro List*****************************/

/**
Macro to specify that the function has performed successfully the expected
action
*/
#define AUI_RTN_SUCCESS   0

/**
Macro to specify that the function has failed the expected action
*/
#define AUI_RTN_FAIL    (-1)

/**
Macro used by ALi sample code to indicate the result of a function is showing help
information

@warning  This macro is used @a only by ALi R&D Dept. then user can ignore it
*/
#define AUI_RTN_HELP        3

/**
Macro to specify that the function cannot find such file or directory
*/
#define AUI_RTN_ENOENT    2

/**
Macro to specify that the function has met an I/O error
*/
#define AUI_RTN_EIO     5

/**
Macro to specify that the function is out of memory when executing
*/
#define AUI_RTN_ENOMEM    12

/**
Macro to specify that the resource is busy
*/
#define AUI_RTN_EBUSY     16

/**
Macro to specify that the function has at least an invalid argument
*/
#define AUI_RTN_EINVAL    22

#endif

/* END OF FILE */

