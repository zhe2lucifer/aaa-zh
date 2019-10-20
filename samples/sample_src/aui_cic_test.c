#include "aui_cic_test.h"

enum aui_ci_transport_tags        /* From CI Spec. */
{
    TRANS_TAG_SB                = 0x80,
    TRANS_TAG_RCV                = 0x81,
    TRANS_TAG_CREAT_TC        = 0x82,
    TRANS_TAG_CTC_REPLY        = 0x83,
    TRANS_TAG_DELETE_TC        = 0x84,
    TRANS_TAG_DTC_REPLY        = 0x85,
    TRANS_TAG_REQUEST_TC        = 0x86,
    TRANS_TAG_NEW_TC            = 0x87,
    TRANS_TAG_TC_ERROR        = 0x88,

    TRANS_TAG_DATA_LAST        = 0xa0,
    TRANS_TAG_DATA_MORE        = 0xa1
};


/**********************************  For cis parsing **************************/
enum aui_pcmcia_cis_tuple_code
{
    AUI_CIS_NULL                 = 0x00,
    AUI_CIS_DEVICE                 = 0x01,
    AUI_CIS_LONGLINK_CB         = 0x02,
    AUI_CIS_INDIRECT             = 0x03,
    AUI_CIS_CONFIG_CB            = 0x04,
    AUI_CIS_CFTABLE_ENTRY_CB     = 0x05,
    AUI_CIS_LONGLINK_MFC         = 0x06,
    AUI_CIS_BAR                          = 0x07,
    AUI_CIS_PWR_MGMNT            = 0x08,
    AUI_CIS_EXTDEVICE                = 0x09,
    /* 0ah - 0fh reseved */
    AUI_CIS_CHECKSUM                 = 0x10,
    AUI_CIS_LONGLINK_A               = 0x11,
    AUI_CIS_LONGLINK_C            = 0x12,
    AUI_CIS_LINKTARGET               = 0x13,
    AUI_CIS_NO_LINK                  = 0x14,
    AUI_CIS_VERS_1                   = 0x15,
    AUI_CIS_ALTSTR                = 0x16,
    AUI_CIS_DEVICE_A                 = 0x17,
    AUI_CIS_JEDEC_C                  = 0x18,
    AUI_CIS_JEDEC_A                  = 0x19,
    AUI_CIS_CONFIG                = 0x1a,
    AUI_CIS_CFTABLE_ENTRY        = 0x1b,
    AUI_CIS_DEVICE_OC                = 0x1c,
    AUI_CIS_DEVICE_OA                = 0x1d,
    AUI_CIS_DEVICEGEO            = 0x1e,
    AUI_CIS_DEVICEGEO_A              = 0x1f,
    AUI_CIS_MANFID                   = 0x20,
    AUI_CIS_FUNCID                   = 0x21,
    AUI_CIS_FUNCE                = 0x22,
    AUI_CIS_SWIL                         = 0x23,
    /* 24 - 3fh reseved */
    AUI_CIS_VERS_2                   = 0x40,
    AUI_CIS_FORMAT                   = 0x41,
    AUI_CIS_GEOMETRY            = 0x42,
    AUI_CIS_BYTEORDER                = 0x43,
    AUI_CIS_DATE                         = 0x44,
    AUI_CIS_BATTERY                  = 0x45,
    AUI_CIS_ORG                    = 0x46,
    AUI_CIS_FORMAT_A                 = 0x47,
    /* 80h - 8fh Vendor unique tuples */
    AUI_CIS_CPCL                         = 0x90,
    /* 90h - feh Reseved */
    AUI_CIS_END                          = 0xff
};

enum aui_pcmcia_cis_sub_tuple_code
{
    AUI_CIS_CIF                = 0xc0
};

#undef le16_to_cpu
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
#define le16_to_cpu(a)        ((((a) >> 8) & 0xff) | (((a) & 0xff) << 8)))
#else
#define le16_to_cpu(a)        (a)
#endif

static const unsigned char mantissa[] = {
    10, 12, 13, 15, 20, 25, 30, 35,
    40, 45, 50, 55, 60, 70, 80, 90
};

static const unsigned int exponent[] = {
    1, 10, 100, 1000, 10000, 100000, 1000000, 10000000
};

#define abs(a)            ((a)>0 ? (a) : -(a))

/* Convert an Extended Speed Byte to a Time in Nanoseconds */
#define SPEED_CVT(v)        (mantissa[(((v) >> 3) & 15) - 1] * exponent[(v) & 7] / 10)
/* Convert a Power Byte to a Current in 0.1 Microamps */
#define POWER_CVT(v)    (mantissa[((v) >> 3) & 15] * exponent[(v) & 7] / 10)
#define POWER_SCALE(v)    (exponent[(v) & 7])

 /* Parse Device Other Information */
static int parse_device_oa(struct aui_cis_tuple *tuple, struct aui_cis_device_o *device_o)
{
    struct aui_cis_device *device = &device_o->device;
    int i;
    unsigned char *p, *q, scale;

    p = (unsigned char *)tuple->tuple_data;
    q = p + tuple->tuple_data_len;

    /* Other Conditions Info */
    device_o->mwait = *p & 1;            /* Support WAIT Signal or not */
    device_o->vcc_used = (*p >> 1) & 3;    /* Vcc to be Used */

    /* Jump Off Addition Bytes */
    while (*p & 0x80)
    {
        if (++p == q)
        {
            return AUI_RTN_FAIL;
        }
    }
    p++;

    /* Parse Device Informatiom */
    device->ndev = 0;
    for (i = 0; i < AUI_CIS_MAX_DEVICES; i++)
    {
    if (*p == 0xff)
    {
        break;
    }
    device->dev[i].type = (*p >> 4) & 0xf;        /* Card Device Type */
    device->dev[i].wp = (*p & 0x08) ? 1 : 0;        /* Write Protect Switch */
    switch (*p & 0x07)                        /* Card Device Speed */
    {
    case 0: device->dev[i].speed = 0; break;
    case 1: device->dev[i].speed = 250; break;
    case 2: device->dev[i].speed = 200; break;
    case 3: device->dev[i].speed = 150; break;
    case 4: device->dev[i].speed = 100; break;
    case 7:
            if (++p == q)
            {
                return AUI_RTN_FAIL;
            }
            device->dev[i].speed = SPEED_CVT(*p);
            while (*p & 0x80)
            {
            if (++p == q)
            {
                return AUI_RTN_FAIL;
            }
            }
        break;
    default:
            return AUI_RTN_FAIL;
    }

    if (++p == q)
    {
        return AUI_RTN_FAIL;
    }

    if (*p == 0xff)
    {
        break;
    }

    scale = *p & 7;    /* Device Size: Device Memory Address Range */
    if (scale == 7)
    {
        return AUI_RTN_FAIL;
    }
    device->dev[i].size = ((*p >> 3) + 1) * (512 << (scale * 2));
    device->ndev++;
    if (++p == q)
    {
        break;
    }
    }

    return AUI_RTN_SUCCESS;
}

/* Parse Strings */
static int parse_strings(unsigned char *p, unsigned char *q, int max, char *s, unsigned char *ofs, unsigned char *found)
{
    int i, j, ns;

    if (p == q)
    {
        return AUI_RTN_FAIL;
    }

    ns = 0; j = 0;
    for (i = 0; i < max; i++)
    {
        if (*p == 0xff)
        {
            break;
        }
        ofs[i] = j;    /* Starting Point of Every String */
        ns++;
        for (;;)
        {
                s[j++] = (*p == 0xff) ? '\0' : *p;    /* Return Strings */
                if ((*p == '\0') || (*p == 0xff))
            {
                break;
                }
                if (++p == q)
            {
                return AUI_RTN_FAIL;
                }
        }
        if ((*p == 0xff) || (++p == q))
        {
            s[j] = '\0';
            break;
        }
        }
    if (found)
    {
        *found = ns;        /* Number of Strings */
        return AUI_RTN_SUCCESS;
    }
    else
    {
        return (ns == max) ? AUI_RTN_SUCCESS : AUI_RTN_FAIL;
    }
}

/* Parse Version Level 1 Information */
static int parse_vers1(struct aui_cis_tuple *tuple, struct aui_cis_vers_1 *vers_1)
{
    unsigned char *p, *q;

    p = (unsigned char *)tuple->tuple_data;
    q = p + tuple->tuple_data_len;

    vers_1->major = *p; p++;
    vers_1->minor = *p; p++;
    if (p >= q)
    {
        return AUI_RTN_FAIL;
    }

    return parse_strings(p, q, AUI_CIS_VERS_1_MAX_PROD_STRINGS, vers_1->str, vers_1->ofs, &vers_1->ns);
}

/* Parse Manufacturer ID Information */
static int parse_manfid(struct aui_cis_tuple *tuple, struct aui_cis_manfid *manfid)
{
    unsigned short *p;

    if (tuple->tuple_data_len < 4)
    {
    return AUI_RTN_FAIL;
    }
    p = (unsigned short *)tuple->tuple_data;
    manfid->manf = le16_to_cpu(p[0]);    /* PC Card's Manufacturer */
    manfid->card = le16_to_cpu(p[1]);        /* Card Identifier and Revision Information */
    return AUI_RTN_SUCCESS;
}

/* Parse Config Information */
static int parse_config(struct aui_cis_tuple *tuple, struct aui_cis_config *config)
{
    int rasz, rmsz, i, ifnsz;
    unsigned char *p, *q;

    p = (unsigned char *)tuple->tuple_data;
    q = p + tuple->tuple_data_len;
    rasz = *p & 0x03;            /* Number of Bytes of TPCC_RADR - 1 */
    rmsz = (*p & 0x3c) >> 2;    /* Number of Bytes TPCC_RMSK field - 1 */
    if (tuple->tuple_data_len < rasz + rmsz + 4)
    {
    return AUI_RTN_FAIL;
    }
    config->last_idx = *(++p);    /* Index Number of Final Entry in the Card Configuration Table */
    p++;
    config->base = 0;        /* TPCC_RADR: Configuration Registers Base Address */
    for (i = 0; i <= rasz; i++)
    {
    config->base += p[i] << (8 * i);
    }
    p += rasz + 1;
    for (i = 0; i < 4; i++)        /* TPCC_RMSK: Configuration Registers Present Mask */
    {
        config->rmask[i] = 0;
    }
        for (i = 0; i <= rmsz; i++)
    {
        config->rmask[i >> 2] += p[i] << (8 * (i % 4));
    }
    config->subtuples = tuple->tuple_data_len - (rasz + rmsz + 4);    /* Data Length of Sub-Tuples */

    /* Note: Add Code for Sub-Tuple of AUI_CIS_CIF in DVB CAM Applicatoins */
    if (config->subtuples != 0)
    {
        p += (rmsz + 1);
        if (*p == AUI_CIS_CIF)
        {
            p += 2;
            ifnsz = (*p >> 6) & 3;        /* STCI_IFN_SIZE: Number of Bytes in Custom Interface ID Number */
            config->cif_infor.ifn = 0;    /* Interface ID Number */
            for (i = 0; i <= ifnsz; i++)
            {
                config->cif_infor.ifn += p[i] << (8 * i);
            }
            p += (ifnsz + 1);
            /* Interface Description Strings */
            parse_strings(p, q, AUI_CIS_CIF_MAX_PROD_STRINGS, config->cif_infor.str, config->cif_infor.ofs, &(config->cif_infor.ns));
            config->cif_infor.ns++;    /* Interface Description String Number */
        }
    }

        return AUI_RTN_SUCCESS;
}

/* Parse Power Description Structure of Config Table Entry Information */
static unsigned char * parse_power(unsigned char *p, unsigned char *q, struct aui_cis_power *pwr)
{
    int i;
    unsigned int scale;

    if (p == q)
    {
        return NULL;
    }
    pwr->present = *p;
    pwr->flags = 0;
    p++;

    /* Param Definitions */
    for (i = 0; i < 7; i++)
    {
    if (pwr->present & (1 << i))
    {
            if (p == q)
        {
            return NULL;
        }
            pwr->param[i] = POWER_CVT(*p);
            scale = POWER_SCALE(*p);
            while (*p & 0x80)
        {
            if (++p == q)
            {
                return NULL;
            }
            if ((*p & 0x7f) < 100)    /* Binary Value for Next Two Decimal Digits to Right of Current Value */
                {
                    pwr->param[i] += (*p & 0x7f) * scale / 100;
            }
            else if (*p == 0x7d)    /* High Impedance OK During Sleep or Power-Down only */
                {
                    pwr->flags |= AUI_CIS_POWER_HIGHZ_OK;
            }
            else if (*p == 0x7e)    /* Zero Value */
                {
                    pwr->param[i] = 0;
            }
            else if (*p == 0x7f)    /* High Impedance Required */
                {
                    pwr->flags |= AUI_CIS_POWER_HIGHZ_REQ;
            }
            else
                    return NULL;
            }
            p++;
    }
    }
    return p;
}

/* Parse Configuration Timing Information of Config Table Entry Information */
static unsigned char * parse_timing(unsigned char *p, unsigned char *q, struct aui_cis_timing *timing)
{
    unsigned char scale;

    if (p == q)
    {
        return NULL;
    }

    scale = *p;
    if ((scale & 3) != 3)    /* MAX Wait Time and Wait Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->wait = SPEED_CVT(*p);
        timing->waitscale = exponent[scale & 3];
    }
    else
    {
        timing->wait = 0;
    }

    scale >>= 2;
    if ((scale & 7) != 7)    /* MAX Ready Time and Ready Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
            timing->ready = SPEED_CVT(*p);
            timing->rdyscale = exponent[scale & 7];
        }
    else
    {
        timing->ready = 0;
    }

    scale >>= 3;
    if (scale != 7)            /* Reserved Time Definition and Reserved Scale */
    {
        if (++p == q)
        {
            return NULL;
        }
        timing->reserved = SPEED_CVT(*p);
        timing->rsvscale = exponent[scale];
    }
    else
    {
        timing->reserved = 0;
    }

    p++;
    return p;
}

/* Parse I/O Space Addresses Required For This Configuration of Config Table Entry Information */
static unsigned char * parse_io(unsigned char *p, unsigned char *q, struct aui_cis_io *io)
{
    int i, j, bsz, lsz;

    if (p == q)
    {
        return NULL;
    }

    io->flags = *p;
    if (!(*p & 0x80))    /* Only I/O Space Definition Byte */
    {
        io->nwin = 1;
        io->win[0].base = 0;
        io->win[0].len = (1 << (io->flags & AUI_CIS_IO_LINES_MASK));
        return p+1;
    }

    if (++p == q)
    {
        return NULL;
    }

    /* Range Descriptor Byte follows I/O Space Definition Byte */
    io->nwin = (*p & 0x0f) + 1;    /* Number of I/O Address Ranges */
    bsz = (*p & 0x30) >> 4;        /* Number of  Bytes which are Used to Encode Start of I/O Address Block */
    if (bsz == 3)
    {
        bsz++;
    }
    lsz = (*p & 0xc0) >> 6;            /* Number of Bytes which are Used to Encode Length of I/O Address Block */
    if (lsz == 3)
    {
        lsz++;
    }
    p++;

    /* I/O Address Range Description */
    for (i = 0; i < io->nwin; i++)
    {
        io->win[i].base = 0;
        io->win[i].len = 1;
        /* Start of I/O Address Block */
        for (j = 0; j < bsz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
            }
            io->win[i].base += *p << (j * 8);
        }
        /* Length of I/O Address Block */
        for (j = 0; j < lsz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
        }
            io->win[i].len += *p << (j * 8);
        }
    }
    return p;
}

/* Parse Memory Space Description Structure of Config Table Entry Information */
static unsigned char * parse_mem(unsigned char *p, unsigned char *q, struct aui_cis_mem *mem)
{
    int i, j, asz, lsz, has_ha;
    unsigned int len, ca, ha;

    if (p == q)
    {
        return NULL;
    }

    mem->nwin = (*p & 0x07) + 1;        /* Number of Window Descriptor */
    lsz = (*p & 0x18) >> 3;                /* Length Field Size */
    asz = (*p & 0x60) >> 5;            /* Card Address Field Size */
    has_ha = (*p & 0x80);                /* Host Address is Present or not */
    if (++p == q)
    {
        return NULL;
    }

    for (i = 0; i < mem->nwin; i++)
    {
        // Length
        len = ca = ha = 0;
        for (j = 0; j < lsz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
            }
            len += *p << (j * 8);
        }
        // Card Address
        for (j = 0; j < asz; j++, p++)
        {
            if (p == q)
            {
                return NULL;
            }
            ca += *p << (j * 8);
        }
        // Host Address
        if (has_ha)
        {
            for (j = 0; j < asz; j++, p++)
            {
                if (p == q)
                {
                    return NULL;
                }
                ha += *p << (j * 8);
            }
        }
        mem->win[i].len = len << 8;
        mem->win[i].card_addr = ca << 8;
        mem->win[i].host_addr = ha << 8;
    }
    return p;
}

#define IRQ_INFO2_VALID        0x10        /* MASK bit */

/* Parse Interrupt Request Description Structure of Config Table Entry Information */
static unsigned char * parse_irq(unsigned char *p, unsigned char *q, struct aui_cis_irq *irq)
{
    if (p == q)
    {
        return NULL;
    }
    irq->irqinfo1 = *p;                        /* Interrupt Request Byte 0 */
    p++;
    if (irq->irqinfo1 & IRQ_INFO2_VALID)
    {
        if (p + 2 > q)
        {
            return NULL;
        }
        irq->irqinfo2 = (p[1] << 8) + p[0];        /* Interrupt Request Byte 1 and 2 */
        p += 2;
    }
    return p;
}

/* Parse Config Table Entry Information */
static int parse_cftable_entry(struct aui_cis_tuple *tuple, struct aui_cis_cftable_entry *entry)
{
    unsigned char *p, *q, features;

    p = tuple->tuple_data;
    q = p + tuple->tuple_data_len;
    entry->index = *p & 0x3f;        /* Configuration-Entry-Number */
    entry->flags = 0;
    if (*p & 0x40)                    /* Default Value for Entry */
    {
    entry->flags |= AUI_CIS_CFTABLE_DEFAULT;
    }
    if (*p & 0x80)                    /* Interface Configuration Present or not */
    {
        if (++p == q)
        {
            return AUI_RTN_FAIL;
        }
        if (*p & 0x10)                /* BVDs Active */
            {
                entry->flags |= AUI_CIS_CFTABLE_BVDS;
        }
        if (*p & 0x20)                /* Write Protect (WP) Status Active*/
            {
                entry->flags |= AUI_CIS_CFTABLE_WP;
        }
        if (*p & 0x40)                /* READY Status Active */
            {
                entry->flags |= AUI_CIS_CFTABLE_RDYBSY;
        }
        if (*p & 0x80)                /* M Wait Required */
            {
                entry->flags |= AUI_CIS_CFTABLE_MWAIT;
        }
        entry->interface = *p & 0x0f;        /* Interface Type */
    }
    else
    {
        entry->interface = 0;
    }

    /* Process Optional Features */
    if (++p == q)
    {
        return AUI_RTN_FAIL;
    }
    features = *p;                    /* Feature Selection Byte */
    p++;

    /* Power Options */
    if ((features & 3) > 0)
    {
        p = parse_power(p, q, &entry->vcc);        /* VCC Power-Description-Structure */
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
    }
    else
    {
        entry->vcc.present = 0;
    }
    if ((features & 3) > 1)
    {
        p = parse_power(p, q, &entry->vpp1);        /* Vpp1 Power-Description-Structure */
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
    }
    else
    {
        entry->vpp1.present = 0;
    }
    if ((features & 3) > 2)
    {
        p = parse_power(p, q, &entry->vpp2);        /* Vpp2 Power-Description-Structure */
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
    }
    else
    {
        entry->vpp2.present = 0;
    }

    /* Timing Options */
    if (features & 0x04)
    {
        p = parse_timing(p, q, &entry->timing);
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
    }
    else
    {
        entry->timing.wait = 0;
        entry->timing.ready = 0;
        entry->timing.reserved = 0;
    }

    /* I/O Space Options */
    if (features & 0x08)
    {
        p = parse_io(p, q, &entry->io);
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
    }
    else
    {
        entry->io.nwin = 0;
    }

    /* Interrupt Options */
    if (features & 0x10)
    {
        p = parse_irq(p, q, &entry->irq);
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
    }
    else
    {
        entry->irq.irqinfo1 = 0;
    }

    /* Memory Space Option */
    switch (features & 0x60)
    {
        case 0x00:
        entry->mem.nwin = 0;
        break;
        case 0x20:
        /* the address must be Even, because (unsigned short *)p
         * otherwise, STB will reboot
         */
        if (((unsigned int)p) & 0x01)
            return AUI_RTN_FAIL;

        entry->mem.nwin = 1;
        entry->mem.win[0].len = le16_to_cpu(*(unsigned short *)p) << 8;
        entry->mem.win[0].card_addr = 0;
        entry->mem.win[0].host_addr = 0;
        p += 2;
        if (p > q)
        {
            return AUI_RTN_FAIL;
        }
        break;
        case 0x40:
        /* the address must be Even, because (unsigned short *)p
         * otherwise, STB will reboot
         */
        if (((unsigned int)p) & 0x01)
            return AUI_RTN_FAIL;
        entry->mem.nwin = 1;
        entry->mem.win[0].len = le16_to_cpu(*(unsigned short *)p) << 8;
        entry->mem.win[0].card_addr = le16_to_cpu(*(unsigned short *)(p + 2)) << 8;
        entry->mem.win[0].host_addr = 0;
        p += 4;
        if (p > q)
        {
            return AUI_RTN_FAIL;
        }
        break;
        case 0x60:
        p = parse_mem(p, q, &entry->mem);
        if (p == NULL)
        {
            return AUI_RTN_FAIL;
        }
        break;
    }

    /* Misc Option */
    if (features & 0x80)
    {
        if (p == q)
        {
            return AUI_RTN_FAIL;
        }
        entry->flags |= (*p << 8);
        while (*p & 0x80)
        {
            if (++p == q)
            {
                return AUI_RTN_FAIL;
            }
        }
        p++;
    }

    entry->subtuples = q - p;

    return AUI_RTN_SUCCESS;
}

static unsigned char ali_tolower(char *str_dst, char *str_src, unsigned int str_len)
{
    unsigned int i = 0;
    if((NULL == str_dst) || (NULL == str_src) || (0 == str_len))
        return 1;

    if(abs(str_dst - str_src) < str_len)
    {
        // TO DO .....
    }

    for(i = 0;i < str_len;i++)
    {
        if(str_src[i] >= 'A' && str_src[i] <= 'Z')
            str_dst[i] = str_src[i] + 0x20;
        else
            str_dst[i] = str_src[i];
    }

    return 0;
}

/******************************************************************************/
static  aui_hdl cic_hdl;
static  struct aui_cis_slot  cis_slot_info;
static  int read_cis_pass = 0;
static  int config_cam_pass = 0;

static  int test_read_cis( int slot)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;    
    int ready = 0;
    struct aui_cis_slot  *ci_slot = &cis_slot_info;
    struct aui_cis_tuple tuple;
    unsigned char   buf[255] = {0};
    unsigned int    cis_addr = 0;        
    char *str_dst = NULL;
    unsigned char   str_conv = 1;
    int  i,count=0;    
    
    MEMSET(&tuple, 0, sizeof(struct aui_cis_tuple));
    MEMSET(ci_slot, 0, sizeof(struct aui_cis_slot));
    tuple.tuple_data = buf;
    tuple.tuple_data_max = sizeof(buf);

    while(1)
    {
        ret = aui_cic_cam_is_ready(cic_hdl,slot,&ready);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("check cam fail!\n");
        if(ready == 0)
        {
            if(count == 3)
            {
                libc_printf("cam %d is not ready, please pass [cic prepare cam] first !\n",slot);
                return  AUI_RTN_FAIL;
            }
            count ++;
            aui_os_task_sleep(100);
            continue;
        }

        //Read Tuple Code
        ret = aui_cic_read_mem(cic_hdl,slot,1,cis_addr,&(tuple.tuple_code));
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("read attribute memory fail!\n");
        /* If it is End of Tuple Chain, then Return */
        if (tuple.tuple_code == AUI_CIS_END)
        {
            libc_printf("parse cis finished!\n");
            return  AUI_RTN_SUCCESS;
        }
        //Read Tuple Length
        cis_addr++;
        ret = aui_cic_read_mem(cic_hdl,slot,1,cis_addr,&(tuple.tuple_link));
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("read attribute memory fail!\n");

        tuple.tuple_data_len = tuple.tuple_link;
        if (tuple.tuple_data_len > tuple.tuple_data_max)
        {
            libc_printf("read_cis:[slot %d] CAM too long tuple\n", slot);
            return AUI_RTN_FAIL;
        }
        //Read Tuple Data Out to Buffer
        cis_addr++;
        ret = aui_cic_read_mem(cic_hdl,slot,tuple.tuple_data_len,cis_addr,tuple.tuple_data);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("read attribute memory fail!\n");        

        cis_addr += tuple.tuple_data_len;

        switch (tuple.tuple_code)
        {
        case AUI_CIS_DEVICE_OA :
        case AUI_CIS_DEVICE_OC :
            libc_printf("read_cis: Found a AUI_CIS_DEVICE_OA/OC=0x%02x\n", tuple.tuple_code);
            parse_device_oa(&tuple, &ci_slot->device_infor);
            break;
        case AUI_CIS_VERS_1 :
            libc_printf("read_cis: Found a AUI_CIS_VERS_1=0x%02x\n", tuple.tuple_code);
            parse_vers1(&tuple, &ci_slot->version_infor);

            str_dst = NULL;
            if(tuple.tuple_data_len)
                str_dst = (char *)MALLOC(254); // Same as version info str len

            if(NULL != str_dst)
                str_conv = ali_tolower(str_dst, ci_slot->version_infor.str, tuple.tuple_data_len);

            if(0 == str_conv)
            {
                for(i=0; i < tuple.tuple_data_len - 8; i++)
                {
                    if(0 == MEMCMP(str_dst + i, "ciplus=1", 8))
                    {
                        ci_slot->version_infor.compatible = 1;
                        libc_printf("read_cis:[slot %d] CICAM compatible --> ciplus=1\n", slot);
                        break;
                    }
                }
            }

            if (str_dst != NULL) {
                FREE(str_dst);
                str_dst = NULL;
            }
            break;
        case AUI_CIS_MANFID :
            libc_printf("read_cis: Found a AUI_CIS_MANFID=0x%02x\n", tuple.tuple_code);
            parse_manfid(&tuple, &ci_slot->manid_infor);
            break;
        case AUI_CIS_CONFIG :
            libc_printf("read_cis: Found a AUI_CIS_CONFIG=0x%02x\n", tuple.tuple_code);
            parse_config(&tuple, &ci_slot->config_infor);
            break;
        case AUI_CIS_CFTABLE_ENTRY:
            libc_printf("read_cis: Found a AUI_CIS_CFTABLE_ENTRY=0x%02x\n", tuple.tuple_code);
            if (tuple.tuple_data_len > 2 + 11 + 17)        /* Magic Number (???) */
            {
                parse_cftable_entry(&tuple, &ci_slot->cfgtable_infor);
            }
            break;
        case AUI_CIS_END:
            libc_printf("read_cis: Found a AUI_CIS_END=0x%02x\n", tuple.tuple_code);
            return AUI_RTN_SUCCESS;
        case AUI_CIS_NO_LINK:
            libc_printf("read_cis: Found a AUI_CIS_NO_LINK=0x%02x\n", tuple.tuple_code);
            break;
        default:
            libc_printf("read_cis: Found a un-process tuple=0x%02x\n", tuple.tuple_code);
            break;
        }
        /* Dump Tuple */
        libc_printf("Tuple Code: %02x, Tuple Link: %02x\n", tuple.tuple_code, tuple.tuple_link);
        for (i = 0; i < tuple.tuple_data_len; i++)
        {
            libc_printf("%02x ", tuple.tuple_data[i]);
        }
        libc_printf("\n\n");
    }  
}

static int test_config_cam(int slot)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;     
    struct aui_cis_slot  *ci_slot = &cis_slot_info;
    struct aui_cis_vers_1        *version = &(ci_slot->version_infor);
    struct aui_cis_manfid        *manid = &(ci_slot->manid_infor);
    struct aui_cis_config        *config = &(ci_slot->config_infor);
    struct aui_cis_cftable_entry    *cfgtable = &(ci_slot->cfgtable_infor);
    int i;
    unsigned char  tmp = 0;

    /* Is DVB CI Card? If not, then Return with Error */
    switch (config->cif_infor.ifn)
    {
    case IFN_DVBCI:
        libc_printf("config_card:[slot %d] CAM This is a DVB CI card!\n", slot);
        break;
    case IFN_OPENCABLEPOD:
        libc_printf("config_card:[slot %d] CAM This is a OpenCable POD card!\n", slot);
        return AUI_RTN_FAIL;
    case IFN_ZOOMVIDEO:
        libc_printf("config_card:[slot %d] CAM This is a Zoom Video card!\n", slot);
        return AUI_RTN_FAIL;
    default:
        libc_printf("config_card:[slot %d] CAM This is a unknow card!\n", slot);
        return AUI_RTN_FAIL;
    }

    for (i = 0; i < config->cif_infor.ns; i++)
    {
        libc_printf("config_card:[slot %d] CAM Inferface[%d]:%s\n", slot, i,
          &(config->cif_infor.str[config->cif_infor.ofs[i]]));
    }

    /* DVB CI PC Card Version should be 5.0. If not, then Return with Error */
    if (version->major != 5 || version->minor != 0)
    {
        libc_printf("config_card:[slot %d] CAM card version error!\n", slot);
        return AUI_RTN_FAIL;
    }
    for (i = 0; i < version->ns; i++)
    {
        libc_printf("config_card:[slot %d] CAM version[%d]:%s\n", slot, i,
          &(version->str[version->ofs[i]]));
    }

    if((manid->manf) || (manid->card))
    {
        libc_printf("config_card:[slot %d] CAM Manufacturer ID: 0x%02x, Card S/N: 0x%02x\n",
                slot, manid->manf, manid->card);
    }

    /* Setup PCMCIA Configuration Registers */
    if (config->base > 0x0fee)
    {
        libc_printf("config_card: PCMCIA config register base invailid!\n");
        return AUI_RTN_FAIL;
    }
    libc_printf("config_card:[slot %d] CI config register address is 0x%04x\n", slot, config->base);

    /* Cfg Optoin Register: Enable the CAM Function */
    if (config->rmask[0] & 1)
    {
        ret = aui_cic_read_mem(cic_hdl,slot,1,(config->base >> 1),&tmp);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("read attribute memory fail!\n");
        
        /* Enable CAM */
        libc_printf("config_card:[slot %d] config index value 0x%04x\n", slot, cfgtable->index);
        libc_printf("config_card:[slot %d] config register value 0x%04x\n", slot, tmp);

        tmp = (cfgtable->index | (tmp & 0x3f));
        ret = aui_cic_write_mem(cic_hdl,slot,1,(config->base >> 1),&tmp);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("write attribute memory fail!\n");        
        libc_printf("config_card:[slot %d] config register setup 0x%04x\n", slot, tmp);
    }

    return AUI_RTN_SUCCESS;
}

static int test_negotiation(int slot)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;    
    int ready = 0;
    int count=0;
    unsigned char   data[2] = {0};
    int bsize;

    while(1)
    {    
        ret = aui_cic_cam_is_ready(cic_hdl,slot,&ready);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("check cam fail!\n");
        if(ready == 0)
        {
            if(count == 3)
            {
                libc_printf("cam %d is not ready, please pass [cic prepare cam] first !\n",slot);
                return  AUI_RTN_FAIL;
            }
            count ++;
            aui_os_task_sleep(100);
            continue;
        }
        else
            break;
    }

    // Step 1: Reset the CAM Card Interface
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_RS);
    libc_printf("write io reg:addr=0x1, value=0x08\n");
    aui_os_task_sleep(200);
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);
    libc_printf("write io reg:addr=0x1, value=0x00\n");
    // Step 2: Wait Interface Ready
    count = 0;
    while(1)
    {
        aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
        libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);
        if (data[0] & CIC_FR)
        {
            libc_printf("cic_negotiation:[slot %d] Reset IFX wait FREE %dmS!\n", slot, count * 100);
            // Notify CAM Card of Provide its Maximum Buffer Size
            aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_SR);
            libc_printf("write io reg:addr=0x1, value=0x04\n");
            break;
        }
        if(count == 100)
        {
            libc_printf("cic_negotiation:[slot %d] Reset wait FREE timeout!\n", slot);
            return  AUI_RTN_FAIL;
        }
        count ++;
        aui_os_task_sleep(100);        
    }

    // Step 3: Wait for CAM Card Send Max Buffer Size
    count = 0;
    while(1)
    {
        aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
        libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);        
        if (data[0] & CIC_DA)
        {
            libc_printf("cic_negotiation:[slot %d] Get size wait DATA %dmS!\n", slot, count * 100);
            // Read Max Buffer Size of CAM Card
            // Get Data Size
            aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_SIZEMS,data);
            libc_printf("read io reg:addr=0x3, value=0x%x\n",data[0]);
            bsize = (data[0] << 8);
            aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_SIZELS,data);
            libc_printf("read io reg:addr=0x2, value=0x%x\n",data[0]);            
            bsize |= data[0];

            libc_printf("read:[slot %d] size %d <-\n", slot, bsize);
            
            if(bsize != 2)
            {
                libc_printf("cic_negotiation:[slot %d] Buffer data size is not 2: %d!\n", slot, bsize);
                return AUI_RTN_FAIL;
            }
            
            aui_cic_read_io_data(cic_hdl,slot,bsize,data);
            libc_printf("read data:%02x,%02x\n",data[0],data[1]);

            // Select Minimum Buffer Size to be Used
            bsize = (data[0] << 8) | data[1];
            
            // Clear Command Register
            aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);
            libc_printf("write io reg:addr=0x1, value=0x00\n");
            
            // Set the CAM Card Buffer Size to be Used
            aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_SW);
            libc_printf("write io reg:addr=0x1, value=0x02\n");
            break;
        }    
        if(count == 100)
        {
            return  AUI_RTN_FAIL;
        }
        count ++;
        aui_os_task_sleep(100);   
    }
    // Step 4: Wait CAM Card Ready
    count = 0;
    while(1)
    {
        aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
        libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);    
        if (data[0] & CIC_FR)
        {
            libc_printf("cic_negotiation:[slot %d] Set size wait FREE %dmS!\n", slot, count * 100);

            /* Keep Few Time for Some Card(board) Need Some Gap between Read Size and Write Size */
            aui_os_task_sleep(100);
            break;
        }
        if(count == 100)
        {
            return  AUI_RTN_FAIL;
        }
        count ++;
        aui_os_task_sleep(100);           
    }
    // Step 5: Send Buffer Size to be Used to CAM Card
    aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
    libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);    
    if (data[0] & CIC_DA)
    {
        libc_printf("write:[slot %d] Collision with read when write!\n", slot);
        return  AUI_RTN_FAIL;        
    }    
    count = 0;
    while(1)
    {
        // Host Control before Start Data Write Sequence
        aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_HC|CIC_SW);
        libc_printf("write io reg:addr=0x1, value=0x03\n");

        // Wait Module Free to Accept Data from Host
        aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
        libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);          
        if ((data[0] & CIC_FR) == CIC_FR)
        {
            break;
        }
        // Clear Host Control after Wait Fail
        aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);
        libc_printf("write io reg:addr=0x1, value=0x00\n");
        
        if(count == 100)
        {
            return  AUI_RTN_FAIL;
        }
        count ++;
        aui_os_task_sleep(100);   
    }
    // Set Data Size   
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_SIZEMS,0);
    libc_printf("write io reg:addr=0x3, value=0x00\n");    
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_SIZELS,2);
    libc_printf("write io reg:addr=0x2, value=0x02\n");
    
    // Write Data to CI card
    data[0] = ((bsize >> 8) & 0xff);
    data[1] = (bsize & 0xff);     
    aui_cic_write_io_data(cic_hdl,slot,2,data);
    libc_printf("write data:%02x,%02x\n",data[0],data[1]);    
    
    // Clear Command Register
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);  
    libc_printf("write io reg:addr=0x1, value=0x00\n");
    
    libc_printf("cic_negotiation:[slot %d] Shakhands OK!\n", slot);
    
    return AUI_RTN_SUCCESS;    
    
}

static void aui_cic_callback(int slot)
{
    int detect = 0;
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;
    
    ret = aui_cic_detect_cam(cic_hdl,slot,&detect);
    if(ret != AUI_RTN_SUCCESS)
        libc_printf("cam %d is not inserted well !\n",slot);
    
    if(detect == 0)
        libc_printf("callback:slot %d removed\n",slot);
    else
        libc_printf("callback:slot %d inserted\n",slot);
}

unsigned long cic_test_prepare_cam(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;    
    int detected = 0;
    int ready = 0;
    int count = 0;
    int slot =0;
    int timeout = 0;
    
    libc_printf("enter %s\n",__FUNCTION__);
    
    ret = aui_cic_open(aui_cic_callback,&cic_hdl);
    if(ret != AUI_RTN_SUCCESS)
        libc_printf("cic open fail!\n");

    ret = aui_cic_detect_cam(cic_hdl,slot,&detected);
    if(ret != AUI_RTN_SUCCESS)
        libc_printf("cam %d is not inserted well !\n",slot);    

    if(detected == 0)
        libc_printf("No cam in slot %d,please insert cam ,will wait 30 seconds...\n",slot);

    while(1)
    {
        ret = aui_cic_detect_cam(cic_hdl,slot,&detected);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("cam %d is not inserted well !\n",slot);
        if(detected == 0)
            count = 0;
        else
            count ++;
        if(count > 3)
            break;
        aui_os_task_sleep(100);
        timeout ++;
        if(timeout > 300)
        {
            libc_printf("wait too long, time out!\n");
            return AUI_RTN_FAIL;
        }
    }
    
    ret = aui_cic_enable_cam(cic_hdl,slot);
    
    if(ret != AUI_RTN_SUCCESS)
        libc_printf("enable cam fail!\n");
    else
        libc_printf("enable cam success!\n");

    // keep 300ms
    aui_os_task_sleep(300);

    ret = aui_cic_reset_cam(cic_hdl,slot);

    if(ret != AUI_RTN_SUCCESS)
        libc_printf("reset cam fail!\n");
    else
        libc_printf("reset cam success!\n");

    // after reset ,delay 100ms
    aui_os_task_sleep(100);

    count = 0;
    while(1)
    {
        ret = aui_cic_cam_is_ready(cic_hdl,slot,&ready);
        if(ret != AUI_RTN_SUCCESS)
            libc_printf("check cam fail!\n");
        if(ready == 1)
        {
            libc_printf("\n\n>>> cam %d is ready for operation!\n\n",slot);
            break;
        }
        else
        {
            libc_printf("cam %d is NOT ready !\n",slot);
            count ++;
        }
        if(count == 50)
        {
            libc_printf("times out for cam to ready, test failed!\n");
            return  1;
        }
        aui_os_task_sleep(200);
    }

    return  AUI_RTN_SUCCESS;

}

unsigned long cic_test_read_cis(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;    
    int slot =0;
    read_cis_pass = 0;
    ret = test_read_cis(slot);
    if ( ret != AUI_RTN_SUCCESS)
    {
        libc_printf("read cam cis fail!\n");
        return  AUI_RTN_FAIL;
    }
    else
    {
        libc_printf("read cam cis success!\n");
        read_cis_pass = 1;
        return  AUI_RTN_SUCCESS;
    }
}

unsigned long cic_test_config_cam(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;    
    int slot =0;

    if(read_cis_pass == 0)
    {
        libc_printf("please pass [cic read cis] first !\n");
        return  AUI_RTN_FAIL;
    }
    config_cam_pass = 0;
    ret = test_config_cam(slot);
    if (ret != AUI_RTN_SUCCESS)
    {
        libc_printf("config cam card fail!\n");
        return  AUI_RTN_FAIL;
    }
    else
    {
        libc_printf("config cam card success!\n");
        config_cam_pass = 1;
        return  AUI_RTN_SUCCESS;
    } 
}

unsigned long cic_test_negotiation(unsigned long *argc,char **argv,char *sz_out_put)
{
    AUI_RTN_CODE ret = AUI_RTN_SUCCESS;    
    int slot =0;

    if(config_cam_pass == 0)
    {
        libc_printf("please pass [cic config cam] first !\n");
        return  AUI_RTN_FAIL;
    }
    ret = test_negotiation(slot);
    if (ret != AUI_RTN_SUCCESS)
    {
        libc_printf("negotiation fail!\n");
        return  AUI_RTN_FAIL;
    }
    else
    {
        libc_printf("negotiation success!\n");
        return  AUI_RTN_SUCCESS;
    } 
}

static unsigned int is_status_fr(int slot)
{
    unsigned char data[1];
    aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
    libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);          
    if ((data[0] & CIC_FR) == CIC_FR)
        return 1;
    else
        return 0;
}
static unsigned int is_status_da(int slot)
{
    unsigned char data[1];    
    aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_CSR,data);
    libc_printf("read io reg:addr=0x1, value=0x%x\n",data[0]);        
    if (data[0] & CIC_DA)
        return  1;
    else
        return  0;
}
static int  write_tpdu(int slot, unsigned short len, unsigned char * buf)
{
    int count;
    
    if(is_status_da(slot))
        return AUI_RTN_FAIL;
    count = 0;
    while(1)
    {
        // Host Control before Start Data Write Sequence
        aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_HC);
        //libc_printf("write io reg:addr=0x1, value=0x03\n");

        // Wait Module Free to Accept Data from Host     
        if (is_status_fr(slot))
        {
            break;
        }
        // Clear Host Control after Wait Fail
        aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);
        //libc_printf("write io reg:addr=0x1, value=0x00\n");
        
        if(count == 100)
        {
            return  AUI_RTN_FAIL;
        }
        count ++;
        aui_os_task_sleep(100);   
    }
    // Set Data Size   
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_SIZEMS,0);
    //libc_printf("write io reg:addr=0x3, value=0x00\n");    
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_SIZELS,len);
    //libc_printf("write io reg:addr=0x2, value=0x%x\n",len);
    
    // Write Data to CI card
    aui_cic_write_io_data(cic_hdl,slot,len,buf);
    libc_printf("write data buffer -> size %d\n",len);
    libc_printf(":---w-->");
    for(count=0;count<len;count++)
        libc_printf("%x,",buf[count]); 
    libc_printf("\n");
    // Clear Command Register
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);  

    return  AUI_RTN_SUCCESS;
}

static int read_tpdu(int slot, int* len, unsigned char * buf)
{
    int bsize;    
    int count =0;
    unsigned char   data[2] = {0};    
    while(1)
    {     
        if (is_status_da(slot))
        {
            break;
        }    
        if(count == 100)
        {
            return  AUI_RTN_FAIL;
        }
        count ++;
        aui_os_task_sleep(100);   
    }
    // Read Max Buffer Size of CAM Card
    // Get Data Size
    aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_SIZEMS,data);
    //libc_printf("read io reg:addr=0x3, value=0x%x\n",data[0]);
    bsize = (data[0] << 8);
    aui_cic_read_io_reg(cic_hdl,slot,AUI_CIC_SIZELS,data);
    //libc_printf("read io reg:addr=0x2, value=0x%x\n",data[0]);            
    bsize |= data[0];

    * len = bsize;
    libc_printf("read:[slot %d] size %d <-\n", slot, bsize);
    aui_cic_read_io_data(cic_hdl,slot,bsize,buf);
    libc_printf("<--r---:");
    for(count=0;count<bsize;count++)
        libc_printf("%x,",buf[count]); 
    libc_printf("\n");    
    // Clear Command Register
    aui_cic_write_io_reg(cic_hdl,slot,AUI_CIC_CSR,CIC_CLR);
    //libc_printf("write io reg:addr=0x1, value=0x00\n");

	return AUI_RTN_SUCCESS;
}

//only simulate the interaction between host stb and VIACCESS-SMIT cam.
unsigned long cic_test_viaccess_tpdu(unsigned long *argc,char **argv,char *sz_out_put)
{
    int slot = 0;
    unsigned char sbuf1[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x01,0x00,0x41,0x00,0x01};
    unsigned char sbuf2[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x01,0x9f,0x80,0x10,0x00};
    unsigned char sbuf3[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x01,0x9f,0x80,0x12,0x00};
    unsigned char sbuf4[37]={0x01,0x00,0xa0,0x21,0x01,0x90,0x02,0x00,0x01,0x9f,0x80,0x11,0x18,0,1,0,0x41,0,2,0,0x41,0,3,0,0x41,0,0x20,0,0x41,0,0x24,0,0x41,0,0x40,0,0x41};
    unsigned char sbuf5[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x02,0x00,0x41,0x00,0x02};
    unsigned char sbuf6[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x02,0x9f,0x80,0x20,0x00};
    unsigned char sbuf7[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x03,0x00,0x41,0x00,0x03};
    unsigned char sbuf8[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x03,0x9f,0x80,0x30,0x00};
    unsigned char sbuf9[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x02,0x9f,0x80,0x22,0x00};
    unsigned char sbufa[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x40,0x00,0x41,0x00,0x04};
    unsigned char sbufb[15]={0x01,0x00,0xa0,0x0b,0x01,0x90,0x02,0x00,0x04,0x9f,0x88,0x02,0x2,0x01,0x01};
    unsigned char sbufc[14]={0x01,0x00,0xa0,0x0a,0x01,0x90,0x02,0x00,0x04,0x9f,0x88,0x0b,0x01,0x01};

    unsigned char rbuf[256];
    unsigned char tcmd[5] ={0x1,0x0,0xa0,0x1,0x1};
    int rlen,i;
    //Creating connection: slot 0, tcid 1
    tcmd[2] = TRANS_TAG_CREAT_TC;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);
    //Create connection finished    
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbuf1);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf2);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf3);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,37,sbuf4);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbuf5);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf6);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    tcmd[2] = TRANS_TAG_DATA_LAST;    
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbuf7);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf8);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==10)
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }    

    write_tpdu(slot,13,sbuf9);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbufa);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,15,sbufb);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==10)
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }  


    write_tpdu(slot,14,sbufc);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);
    
    return  AUI_RTN_SUCCESS;
}

unsigned long cic_test_irdeto_tpdu(unsigned long *argc,char **argv,char *sz_out_put)
{
    int slot = 0;
    unsigned char sbuf1[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x01,0x00,0x41,0x00,0x01};
    unsigned char sbuf2[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x01,0x9f,0x80,0x10,0x00};
    unsigned char sbuf3[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x01,0x9f,0x80,0x12,0x00};
    unsigned char sbuf4[33]={0x01,0x00,0xa0,0x1d,0x01,0x90,0x02,0x00,0x01,0x9f,0x80,0x11,0x14,0,1,0,0x41,0,2,0,0x41,0,3,0,0x41,0,0x24,0,0x41,0,0x40,0,0x41};
    unsigned char sbuf5[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x02,0x00,0x41,0x00,0x03};
    unsigned char sbuf6[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x03,0x9f,0x80,0x20,0x00};
    unsigned char sbuf7[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x03,0x00,0x41,0x00,0x05};
    unsigned char sbuf8[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x05,0x9f,0x80,0x30,0x00};
    unsigned char sbuf9[13]={0x01,0x00,0xa0,0x09,0x01,0x90,0x02,0x00,0x03,0x9f,0x80,0x22,0x00};
    unsigned char sbufa[14]={0x01,0x00,0xa0,0x0a,0x01,0x92,0x07,0x00,0x00,0x40,0x00,0x41,0x00,0x07};
    unsigned char sbufb[15]={0x01,0x00,0xa0,0x0b,0x01,0x90,0x02,0x00,0x07,0x9f,0x88,0x02,0x2,0x01,0x01};
    unsigned char sbufc[14]={0x01,0x00,0xa0,0x0a,0x01,0x90,0x02,0x00,0x07,0x9f,0x88,0x0b,0x01,0x01};
    unsigned char sbufd[14]={0x01,0x00,0xa0,0x0a,0x01,0x90,0x02,0x00,0x07,0x9f,0x88,0x0b,0x01,0x01};
    unsigned char sbufe[14]={0x01,0x00,0xa0,0x0a,0x01,0x90,0x02,0x00,0x07,0x9f,0x88,0x0b,0x01,0x02};

    unsigned char rbuf[256];
    unsigned char tcmd[5] ={0x1,0x0,0xa0,0x1,0x1};
    int rlen,i;
    //Creating connection: slot 0, tcid 1
    tcmd[2] = TRANS_TAG_CREAT_TC;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);
    //Create connection finished    
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbuf1);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf2);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf3);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,33,sbuf4);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbuf5);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf6);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    tcmd[2] = TRANS_TAG_DATA_LAST;    
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,14,sbuf7);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,13,sbuf8);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==1)//set loop times you want
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }    

    libc_printf("======================= Enter CI Slot ===========================\n");

    write_tpdu(slot,13,sbuf9);    
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);
    
    write_tpdu(slot,14,sbufa);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    write_tpdu(slot,15,sbufb);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==2)
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }  


    write_tpdu(slot,14,sbufc);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==2)
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }  

    write_tpdu(slot,14,sbufd);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==5)
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }  

    write_tpdu(slot,14,sbufe);
    read_tpdu(slot,&rlen,rbuf);
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(rbuf[5] & 0x80)
        {
            break;
        }
        aui_os_task_sleep(100);
    }
    //recieve data...
    tcmd[2] = TRANS_TAG_RCV;
    write_tpdu(slot,5,tcmd);
    read_tpdu(slot,&rlen,rbuf); 
    //read more
    read_tpdu(slot,&rlen,rbuf);

    //polling
    i = 0;
    tcmd[2] = TRANS_TAG_DATA_LAST;
    while(1)
    {
        write_tpdu(slot,5,tcmd);
        aui_os_task_sleep(10);
        read_tpdu(slot,&rlen,rbuf);
        if(i==5)
        {
            break;
        }
        i++;
        aui_os_task_sleep(100);
    }  
    
    return  AUI_RTN_SUCCESS;
}

void aui_load_tu_cic()
{
	aui_tu_reg_group("cic", "cic tests");
	aui_tu_reg_item(2, "1", AUI_CMD_TYPE_SYS, cic_test_prepare_cam, "cic prepare cam");
	aui_tu_reg_item(2, "2", AUI_CMD_TYPE_UNIT, cic_test_read_cis, "cic read cis");  
	aui_tu_reg_item(2, "3", AUI_CMD_TYPE_UNIT, cic_test_config_cam, "cic config cam");
 	aui_tu_reg_item(2, "4", AUI_CMD_TYPE_UNIT, cic_test_negotiation, "cic negotiation");
    aui_tu_reg_item(2, "5", AUI_CMD_TYPE_UNIT, cic_test_irdeto_tpdu, "cic simulate test irdeto cam");
    aui_tu_reg_item(2, "6", AUI_CMD_TYPE_UNIT, cic_test_viaccess_tpdu, "cic simulate test viaccess cam");
}
