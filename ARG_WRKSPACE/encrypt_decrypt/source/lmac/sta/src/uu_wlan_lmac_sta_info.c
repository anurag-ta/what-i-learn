/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_lmac_sta_info.c                                **
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

#include "uu_wlan_utils.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_lmac_dev.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_cp_if.h"


uu_uint32 uu_wlan_sta_info_is_used_g;

/* Station slots */
uu_uint64 uu_wlan_ba_info_is_used_g;

/* Station address table, gives the station index(address) in the Station management table */
uu_wlan_sta_addr_index_t     uu_sta_addr_index_g[UU_WLAN_MAX_ASSOCIATIONS];
/* Station management table */
/* Station management table */
uu_wlan_asso_sta_info_t      uu_sta_info_table_g[UU_WLAN_MAX_ASSOCIATIONS];

/* BA management table */
uu_wlan_ba_ses_context_t     uu_ba_table_g[UU_WLAN_MAX_BA_SESSIONS];


uu_wlan_asso_sta_info_t* uu_wlan_lmac_get_sta(const uu_uint8 *addr_p)
{
    uu_uint8   i;

    /* Search for the station entry */
    for (i = 0; i<UU_WLAN_MAX_ASSOCIATIONS; i++)
    {
        if ((uu_wlan_sta_info_is_used_g >> i & 0x01) &&
                isaddrsame(uu_sta_addr_index_g[i].addr, addr_p))
        {
            /* Found a valid station entry for this station address */
            return  &(uu_sta_info_table_g[i]);
        }
    }
    return UU_NULL;

} /* uu_wlan_lmac_del_ba */


/* NOTE: This function should be called under LOCK of uu_wlan_ba_ses_context_lock_g */
uu_wlan_ba_ses_context_t* uu_wlan_get_ba_contxt_of_sta(uu_uchar *addr, uu_uchar tid, uu_bool dir)
{
   uu_wlan_asso_sta_info_t*  sta = uu_wlan_lmac_get_sta(addr);
 
   if (sta == NULL)
   { 
      return NULL;
   } 
   else
   {
        if (sta->ba_cnxt[dir][tid] != UU_WLAN_INVALID_BA_SESSION)
        {
            return &(uu_ba_table_g[sta->ba_cnxt[dir][tid]]);
        }
   }
   return NULL;
} /* uu_wlan_get_ba_contxt_of_sta */


/* EOF */

