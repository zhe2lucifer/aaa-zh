/**@file
*    @brief         UDI osd module public data struct and interface declear
*    @author        andy.yu
*    @date          2013-7-18
*    @version       1.0.0
*    @note          ali corp. all rights reserved. 2013~2999 copyright (C)
*/
#ifndef _AUI_OSD_TEST_H
#define _AUI_OSD_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_osd.h>
#include "aui_test_app.h"

/****************************GLOBAL MACRO************************************/

/****************************GLOBAL TYPE************************************/

/****************************GLOBAL FUNC DECLEAR*****************************/
#ifdef __cplusplus
extern "C" {
#endif

#define RED_4444 0xFF00
#define RED_565 0xF100
#define RED_1555 0xFC00
#define RED_8888 0xFFFF0000
#define RED_CLUT8 1
#define RED_555 0xfa00

#define GREEN_4444 0xF0F0
#define GREEN_565 0x07E0
#define GREEN_1555 0x83E0
#define GREEN_8888 0xFF00FF00
#define GREEN_CLUT8 6

#define BLUE_4444 0xF00F
#define BLUE_565 0x001F
#define BLUE_1555 0x801F
#define BLUE_8888 0xFF0000FF
#define BLUE_CLUT8 5

#define WHITE_4444 0xFFFF
#define WHITE_565 0xFFFF
#define WHITE_1555 0xFFFF
#define WHITE_8888 0xFFFFFFFF
#define WHITE_CLUT8 3

#define MIX_4444 0xAAAA
#define MIX_4444 0xAAAA
#define MIX_1555 0xAAAA
#define MIX_8888 0xAAAAAAAA
#define MIX_CLUT8 8

#define TRANSPARENT_COLOR 0x0

void osd_test_reg(void);

#ifdef __cplusplus
}
#endif

#endif
