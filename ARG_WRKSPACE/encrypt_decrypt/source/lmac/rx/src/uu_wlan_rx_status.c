/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_rx_status.c                                    **
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
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_rx_if.h"


uu_wlan_rx_status_info_t uu_wlan_rx_status_info_g[UU_WLAN_MAX_RX_STATUS_TO_UMAC];
EXPORT_SYMBOL(uu_wlan_rx_status_info_g);
uu_uint32 uu_wlan_rx_status_flags_g;
EXPORT_SYMBOL(uu_wlan_rx_status_flags_g);

/** Write index to fill the uu_wlan_rx_status_info_g structure, and for updating uu_wlan_rx_status_flags_g flag.
 */
static uu_uint8 uu_wlan_rx_status_write_index_g;


uu_void uu_wlan_rx_update_status_for_umac(uu_uint32 offset, uu_uint16 len)
{
    /* Filling rx status structure information */
    uu_wlan_rx_status_info_g[uu_wlan_rx_status_write_index_g].offset = offset;
    uu_wlan_rx_status_info_g[uu_wlan_rx_status_write_index_g].len = len;

    /* updating corresponding bit rx status flag */
    uu_wlan_rx_status_flags_g|= (1 << uu_wlan_rx_status_write_index_g);

    /* Moving write index to next level */
    uu_wlan_rx_status_write_index_g = ((uu_wlan_rx_status_write_index_g + 1) & UU_WLAN_MAX_RX_STATUS_TO_UMAC_MASK);

} /* uu_wlan_tx_update_status_for_umac */


/* EOF */

