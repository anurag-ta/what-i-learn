/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ba_session.c                                **
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
#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_lmac_sta_info.h" 
#include "uu_wlan_cp_ba_session.h" 

/* Global decl */

/* Static decl */

/* Local macros */


/*
****** Func header *******
*/
uu_wlan_spin_lock_type_t uu_wlan_ba_ses_context_lock_g;

/*
****** Func header *******
*/
/* NOTE: Functionality is similar to '_sb_update_tx'. Only stats (counters) will change */
static uu_int32  _sb_update(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uint16 fn)
{
   uu_uint16  clear_start;
   uu_uint16  clear_end;

   /* SN is within our existing window */
   if ((sb_p->winstart <= sn) && (sn <= sb_p->winend))
   {
      sb_p->bitmap[(sb_p->buf_winstart+sn-sb_p->winstart) & BA_BASIC_BITMAP_BUF_MASK] |= (1 << fn);
   }
   /*
   ** The SN is beyond winend, but is NEW (within half of SN range).
   ** Move the window, according to SN.
   */
   else if ((sb_p->winend < sn) && (sn < sb_p->winstart + BA_SEQN_RANGE_BY2))
   {
      /* Start & End of the area to be cleared (Status starting from WinEnd+1 to new SN) */
      clear_start = sb_p->buf_winstart + (sb_p->winend-sb_p->winstart) + 1;
      clear_end = sb_p->buf_winstart + (sn - sb_p->winstart);

      sb_p->buf_winstart = (sb_p->buf_winstart + sn - sb_p->winend) & BA_BASIC_BITMAP_BUF_MASK;
      sb_p->winstart = sn - sb_p->winsize + 1;
      sb_p->winend = sn;

      /* Clear all the bits in buffer, from WinEnd+1 to new-WinEnd, including new WinEnd */
      while (clear_start <= clear_end)
      {
         sb_p->bitmap[clear_start & BA_BASIC_BITMAP_BUF_MASK] = 0;
         clear_start++;
      }

      /* Set the bit for received packet. All other bits are 0 */
      sb_p->bitmap[(sb_p->buf_winstart+sn-sb_p->winstart) & BA_BASIC_BITMAP_BUF_MASK] = (1 << fn);

      /* If 'winstart' is above the range, wraparound */
      if (sb_p->winstart > BA_MPDU_SEQN_MAX)
      {
         sb_p->winstart = sb_p->winstart & MPDU_SEQN_MASK;
         sb_p->winend = sb_p->winend & MPDU_SEQN_MASK;
      }
   }
   else /* Received old packet (could be wrap-around also) */
   {
      //printk("received frame sn %d ba context winstart %d winend %d\n", sn, sb_p->winstart, sb_p->winend);
      return UU_FAILURE;
   }
   return UU_SUCCESS;
} /* _sb_update */


uu_int32 uu_wlan_ba_sb_update_rx(uu_wlan_ba_sb_t *sb_p, uu_uint16 scf)
{
   uu_uint16 sn = MPDU_SN_FROM_SCF(scf);
   uu_uint16 fn = MPDU_FN_FROM_SCF(scf);
   uu_int32  ret;

   if ((ret = _sb_update(sb_p, sn, fn)) != 0)
   {
      /* Try update again, considering wrap-around */
      if ((ret = _sb_update(sb_p, BA_ELEVATE_SN_FOR_WRAPAROUND(sn), fn)) != UU_SUCCESS)
      {
         /* Received old packet. Update the error counter */
      }
   }
   return ret;
} /* uu_wlan_ba_sb_update_rx */


/* EOF */

