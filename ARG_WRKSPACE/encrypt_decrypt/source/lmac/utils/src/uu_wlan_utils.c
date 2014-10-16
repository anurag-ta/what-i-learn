/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_utils.c                                        **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                          **
 ** All rights reserved.                                                      **
 ** http://www.uurmi.com                                                      **
 **                                                                           **
 ** All information contained herein is property of Uurmi Systems             **
 ** unless otherwise explicitly mentioned.                                    **
 **                                                                           **
 ** The intellectual and technical concepts in this file are proprietary      **
 ** to Uurmi Systems and may be covered by granted or in process national     **
 ** and international patents and are protect by trade secrets and            **
 ** copyright law.                                                            **
 **                                                                           **
 ** Redistribution and use in source and binary forms of the content in       **
 ** this file, with or without modification are not permitted unless          **
 ** permission is explicitly granted by Uurmi Systems.                        **
 **                                                                           **
 ******************************************************************************/

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_fwk_log.h"


uu_uint8 uu_wlan_get_ac_from_tid(uu_uchar tid)
{
    if((tid == 6) || (tid == 7))
        return UU_WLAN_AC_VO;
    else if((tid == 1) || (tid == 2))
        return UU_WLAN_AC_BK;
    else if((tid == 4) || (tid == 5))
        return UU_WLAN_AC_VI;
    else if((tid == 0) || (tid == 3))
        return UU_WLAN_AC_BE;
    else
        return UU_WLAN_AC_BE;
} /* uu_wlan_get_ac_from_tid */


uu_void uu_wlan_log_frame_type(uu_uchar fc0, uu_bool  is_tx)
{
    uu_char *dir_tag;

    if (is_tx)
        dir_tag = "****Transmitted frame";
    else
        dir_tag = "****Received frame";

    /* Logging frame type */
    switch(fc0)
    {
        /* Control frame types */
        case 0x74:
            UU_WLAN_LOG_DEBUG(("%s: CONTROL-WRAPPER", dir_tag));
            break;
        case 0x84:
            UU_WLAN_LOG_DEBUG(("%s: BLOCK-ACK-REQ", dir_tag));
            break;
        case 0x94:
            UU_WLAN_LOG_DEBUG(("%s: BLOCK-ACK", dir_tag));
            break;
        case 0xA4:
            UU_WLAN_LOG_DEBUG(("%s: PS-POLL", dir_tag));
            break;
        case 0xB4:
            UU_WLAN_LOG_DEBUG(("%s: RTS", dir_tag));
            break;
        case 0xC4:
            UU_WLAN_LOG_DEBUG(("%s: CTS", dir_tag));
            break;
        case 0xD4:
            UU_WLAN_LOG_DEBUG(("%s: ACK", dir_tag));
            break;
        case 0xE4:
            UU_WLAN_LOG_DEBUG(("%s: CF-END", dir_tag));
            break;
        case 0xF4:
            UU_WLAN_LOG_DEBUG(("%s: CF-END+CF-ACK", dir_tag));
            break;

        /* Data frame types */
        case 0x08:
            UU_WLAN_LOG_DEBUG(("%s: Non-QOS DATA", dir_tag));
            break;
        case 0x48:
            UU_WLAN_LOG_DEBUG(("%s: NULL DATA", dir_tag));
            break;
        case 0x88:
            UU_WLAN_LOG_DEBUG(("%s: QOS DATA", dir_tag));
            break;
        case 0xC8:
            UU_WLAN_LOG_DEBUG(("%s: QOS NULL DATA", dir_tag));
            break;
        case 0xD8:
            UU_WLAN_LOG_DEBUG(("%s: RESERVED Data Frame (%x)", dir_tag, fc0));
            break;

        /* Management frame types */
        case 0x00:
            UU_WLAN_LOG_DEBUG(("%s: ASSOCIATION-REQUEST", dir_tag));
            break;
        case 0x10:
            UU_WLAN_LOG_DEBUG(("%s: ASSOCIATION-RESPONSE", dir_tag));
            break;
        case 0x20:
            UU_WLAN_LOG_DEBUG(("%s: REASSOCIATION-REQUEST", dir_tag));
            break;
        case 0x30:
            UU_WLAN_LOG_DEBUG(("%s: REASSOCIATION-RESPONSE", dir_tag));
            break;
        case 0x40:
            UU_WLAN_LOG_DEBUG(("%s: PROBE-REQUEST", dir_tag));
            break;
        case 0x50:
            UU_WLAN_LOG_DEBUG(("%s: PROBE-RESPONSE", dir_tag));
            break;
        case 0x60:
            UU_WLAN_LOG_DEBUG(("%s: TIMING-ADVERT", dir_tag));
            break;
        case 0x70:
        case 0xF0:
            UU_WLAN_LOG_DEBUG(("%s: RESERVED Management Frame (%x)", dir_tag, fc0));
            break;
        case 0x80:
            UU_WLAN_LOG_DEBUG(("%s: BEACON", dir_tag));
            break;
        case 0x90:
            UU_WLAN_LOG_DEBUG(("%s: ATIM", dir_tag));
            break;
        case 0xa0:
            UU_WLAN_LOG_DEBUG(("%s: DISASSOCIATION", dir_tag));
            break;
        case 0xb0:
            UU_WLAN_LOG_DEBUG(("%s: AUTHENTICATION", dir_tag));
            break;
        case 0xc0:
            UU_WLAN_LOG_DEBUG(("%s: DEAUTH", dir_tag));
            break;
        case 0xd0:
            UU_WLAN_LOG_DEBUG(("%s: ACTION", dir_tag));
            break;
        case 0xe0:
            UU_WLAN_LOG_DEBUG(("%s: ACTION-no-ACK", dir_tag));
            break;

        default:
            UU_WLAN_LOG_DEBUG(("%s frame: (%x)", dir_tag, fc0));
            break;
    }
} /* uu_wlan_log_frame_type */


/* EOF */


