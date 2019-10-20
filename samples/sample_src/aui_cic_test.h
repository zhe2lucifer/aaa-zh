/**@file
*   @brief 		common interface controller(CIC) module sample code
*   @author		Jason.Chen
*   @date       2014-11-28
*   @version	1.0.0
*   @note       ali corp. all rights reserved. 2013~2020 copyright (C)
*/
#ifndef _AUI_CIC_TEST_H
#define _AUI_CIC_TEST_H
/****************************INCLUDE FILE************************************/
#include <aui_cic.h>
#include "aui_test_app.h"
//#include "unity_fixture.h"

#ifdef __cplusplus
extern "C" {
#endif

struct aui_cis_tuple
{
    unsigned char        tuple_code;
    unsigned char        tuple_link;
    unsigned char        tuple_data_max;
    unsigned char        tuple_data_len;
    unsigned char        *tuple_data;
};

#define AUI_CIS_MAX_DEVICES            4

struct aui_cis_device
{
    unsigned char    ndev;
    struct
    {
        unsigned char                type;
        unsigned char                wp;
        unsigned int                speed;
        unsigned int                size;
    } dev[AUI_CIS_MAX_DEVICES];
};

struct aui_cis_device_o
{
    unsigned char                mwait;
    unsigned char                vcc_used;
    struct aui_cis_device        device;
};

/* Structure for Level 1 Version Information */
#define AUI_CIS_VERS_1_MAX_PROD_STRINGS    4

struct aui_cis_vers_1
{
    unsigned char    major;
    unsigned char    minor;
    unsigned char    ns;
    unsigned char    ofs[AUI_CIS_VERS_1_MAX_PROD_STRINGS];
    char        str[254];
    unsigned char    compatible;
};

/* Structure for Manufactory ID Information */
struct aui_cis_manfid
{
    unsigned short    manf;
    unsigned short    card;
};

/* Structure for Config Information */
enum stci_ifn_code
{
    IFN_ZOOMVIDEO        = 0x0141,
    IFN_DVBCI            = 0x0241,
    IFN_OPENCABLEPOD    = 0x0341
};

#define AUI_CIS_CIF_MAX_PROD_STRINGS        8

struct ccstpl_cif
{
    unsigned int    ifn;
    unsigned char    ns;
    unsigned char    ofs[AUI_CIS_CIF_MAX_PROD_STRINGS];
    char        str[128];
};

struct aui_cis_config
{
    unsigned char                last_idx;
    unsigned int                base;
    unsigned int                rmask[4];
    unsigned char                subtuples;
    struct ccstpl_cif        cif_infor;
};

/* Structure for Config Table Entry Information */
#define AUI_CIS_POWER_HIGHZ_OK        0x01
#define AUI_CIS_POWER_HIGHZ_REQ    0x02

struct aui_cis_power
{
    unsigned char    present;
    unsigned char    flags;
    unsigned int    param[7];
};

struct aui_cis_timing
{
    unsigned int    wait, waitscale;
    unsigned int    ready, rdyscale;
    unsigned int    reserved, rsvscale;
};

#define AUI_CIS_IO_LINES_MASK    0x1f
#define AUI_CIS_IO_MAX_WIN        16

struct aui_cis_io
{
    unsigned char    flags;
    unsigned char    nwin;
    struct
    {
        unsigned int    base;
        unsigned int    len;
    } win[AUI_CIS_IO_MAX_WIN];
};

struct aui_cis_irq
{
    unsigned int    irqinfo1;
    unsigned int    irqinfo2;
};

#define AUI_CIS_MEM_MAX_WIN    8

struct aui_cis_mem
{
    unsigned char    flags;
    unsigned char    nwin;
    struct
    {
        unsigned int    len;
        unsigned int    card_addr;
        unsigned int    host_addr;
    } win[AUI_CIS_MEM_MAX_WIN];
};

#define AUI_CIS_CFTABLE_DEFAULT        0x0001
#define AUI_CIS_CFTABLE_BVDS        0x0002
#define AUI_CIS_CFTABLE_WP            0x0004
#define AUI_CIS_CFTABLE_RDYBSY        0x0008
#define AUI_CIS_CFTABLE_MWAIT        0x0010
#define AUI_CIS_CFTABLE_AUDIO        0x0800
#define AUI_CIS_CFTABLE_READONLY    0x1000
#define AUI_CIS_CFTABLE_PWRDOWN    0x2000

struct aui_cis_cftable_entry
{
    unsigned char                    index;
    unsigned short                    flags;
    unsigned char                    interface;
    struct aui_cis_power        vcc, vpp1, vpp2;
    struct aui_cis_timing        timing;
    struct aui_cis_io            io;
    struct aui_cis_irq            irq;
    struct aui_cis_mem            mem;
    unsigned char                    subtuples;
};

struct aui_cis_slot
{
    /* Card Information Structure */
    struct aui_cis_device_o        device_infor;
    struct aui_cis_vers_1        version_infor;
    struct aui_cis_manfid        manid_infor;
    struct aui_cis_config        config_infor;
    struct aui_cis_cftable_entry    cfgtable_infor;
};

typedef enum 
{
    CIC_CLR               = 0x00,
    CIC_RE                = 0x01,
    CIC_WE                = 0x02,
    CIC_IIR                = 0x10, /* CI+ */
    CIC_FR                = 0x40,
    CIC_DA                = 0x80,
    CIC_HC                = 0x01,
    CIC_SW                = 0x02,
    CIC_SR                = 0x04,
    CIC_RS                = 0x08
}aui_cic_cmd;

#ifdef __cplusplus
}
#endif
#endif
