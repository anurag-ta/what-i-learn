/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_tx_handler.c                                   **
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
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_if.h"


uu_wlan_tx_status_info_t uu_wlan_tx_status_info_g[UU_WLAN_MAX_QID][UU_WLAN_MAX_TX_STATUS_TO_UMAC];
EXPORT_SYMBOL(uu_wlan_tx_status_info_g);
uu_uint32 uu_wlan_tx_status_flags_g[UU_WLAN_MAX_QID];
EXPORT_SYMBOL(uu_wlan_tx_status_flags_g);
static uu_uint8 uu_wlan_tx_status_write_index_g[UU_WLAN_MAX_QID];

#ifdef UU_WLAN_TPC
uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status, uu_uint8 bandwidth)
#else
uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status)
#endif
{
    /* Filling tx status structure information */
    uu_wlan_tx_status_info_g[ac][uu_wlan_tx_status_write_index_g[ac]].seqno = (seqno >> 4);//Discarding fragment number from scf.
    uu_wlan_tx_status_info_g[ac][uu_wlan_tx_status_write_index_g[ac]].retry_count = retries;
    uu_wlan_tx_status_info_g[ac][uu_wlan_tx_status_write_index_g[ac]].status = status;
#ifdef UU_WLAN_TPC
    uu_wlan_tx_status_info_g[ac][uu_wlan_tx_status_write_index_g[ac]].bandwidth = bandwidth;
#endif
    /* updating corresponding bit tx status flag */
    uu_wlan_tx_status_flags_g[ac] |= (1 << uu_wlan_tx_status_write_index_g[ac]);

    /* Moving write index to next level */
    uu_wlan_tx_status_write_index_g[ac] = ((uu_wlan_tx_status_write_index_g[ac] + 1) & UU_WLAN_MAX_TX_STATUS_TO_UMAC_MASK);

    /* umac cbk for tx status */
    lmac_ops_gp->umacCBK(UU_LMAC_IND_TX_STATUS);
    uu_wlan_int_status_reg_g |= 0x01;
} /* uu_wlan_tx_update_status_for_umac */


/* EOF */

