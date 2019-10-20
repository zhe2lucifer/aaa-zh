
#ifndef _AUI_AV_INJECTER_PRIV_H

#define _AUI_AV_INJECTER_PRIV_H

/*****************************Global Function List*****************************/
#include <aui_common.h>
#ifdef __cplusplus
extern "C"
{
#endif

#define AUI_ES_TAG (('A'<<24)|('L'<<16)|('I'<<8))

// Used to idendify AV INJECT ES packet content all bytes are clear data
#define AUI_ES_PKT_TYPE_ALL_CLEAR               (AUI_ES_TAG + 1)  // 0xALI1

// Used to identify AV INJECT ES packet content all bytes are encrypted data
#define AUI_ES_PKT_TYPE_ALL_ENCRYPTED           (AUI_ES_TAG + 2)  // 0xALI2

// Used to identify AV INJECT ES packet content, partially encrypted
#define AUI_ES_PKT_TYPE_PART_ENCRYPTED          (AUI_ES_TAG + 3)  // 0xALI3

/**
    When sbm data path is set to: REE -> TEE -> SEE(stream crypto module) -> VBV
    ES packet need add ES data decription header, which will be used by stream crypto module,
    to determine whether need to decrypt, and how to decrypt es data
*/
typedef struct aui_es_data_desc {
    /**
        Specify the ES packet is clear or encrypted or partially encrypted
    */
    unsigned long pkt_enc_type;   // e.g. AUI_ES_PKT_TYPE_PART_ENCRYPTED

    /**
        Specify the description header length
        = sizeof(aui_es_data_desc) + sub_pkt_count * sizeof(aui_es_sub_pkt_info)
    */
    unsigned long desc_header_length;

    /**
        Specify the data length of ES packet
    */
    unsigned long es_data_length;

    /**
        Specify whether has sub packet, and sub packet count. Only sub sample case will has sub packet.
    */
    unsigned long sub_pkt_count;
} aui_es_data_desc ;

typedef struct aui_es_sub_pkt_info {

    /**
    Member to specify the sub packet is encrypted or not
    */
    unsigned long encrypted_flag;

    /**
    Member to specify the offset of sub packet data
    */
    unsigned long sub_pkt_offset;

    /**
    Member to specify the length of sub packet data
    */
    unsigned long  sub_pkt_length;

} aui_es_sub_pkt_info ;

AUI_RTN_CODE aui_audio_decoder_open_internal(aui_audio_info_init* audio_info,
        aui_hdl* decoder_out, unsigned char enc_flag); 

AUI_RTN_CODE aui_video_decoder_open_internal(aui_video_decoder_init *video_info,
        aui_hdl *decoder_out, unsigned char enc_flag);

AUI_RTN_CODE aui_video_decoder_write_nal_header(aui_hdl decoder,
        const unsigned char *buf, unsigned long size);

#ifdef __cplusplus
}
#endif

#endif

/* END OF FILE */

