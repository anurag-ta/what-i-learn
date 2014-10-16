/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ba_sb_tx.c                                  **
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

/* headers */
#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#endif


#include "uu_datatypes.h" 

#include "uu_wlan_tx_if.h"

#include "uu_wlan_cp_ba_session.h" 
#include "uu_wlan_cp_ba_sb.h" 
#include "uu_wlan_cp_ba_sb_tx.h" 


#if 0 /* Same as 'uu_wlan_sb_init' */
/*
****** Func header *******
*/
uu_int32 uu_wlan_sb_tx_init(ba_sb_t *sb_p, unsigned short buffsz, unsigned short ssn)
{
   assert(buffsz <= BA_BASIC_BITMAP_BUF_SIZE);

   memset(sb_p->bitmap, 0, sizeof(sb_p->bitmap));
   sb_p->winstart = (ssn & MPDU_SEQN_MASK);
   sb_p->winsize = buffsz ;
   /* wrap around is considered only when WinStart exceeds 4095 */
   sb_p->winend =  (sb_p->winstart + sb_p->winsize - 1); 

   sb_p->buf_winstart = 0;
   return 0;
}
#endif


/*
****** Func header *******
** Caller should decide whether to handle basic BA or compressed BA 
*/

uu_int32 uu_wlan_handle_basic_ba(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uint16 *ba_bitmap)
{
   uu_uint16 cnt;

   assert(NULL != ba_bitmap);

   /* Storing the Seq number received in BA frame */
   sb_p->ba_frame_seq_no = sn;

   /* Storing the bitmap received in BA frame */
   for (cnt  = 0; cnt < BA_BASIC_BITMAP_BUF_SIZE ; cnt++)
   {
      sb_p->ba_frame_bitmap[cnt] = *ba_bitmap++;
   }
   return 0;
}

uu_int32 uu_wlan_cp_tx_compare_bitmap(uu_wlan_ba_sb_t *sb_p)
{
   uu_uint16  cnt;

   /* Compare the bit map received with tx bitmap */
   for (cnt = 0; cnt < sb_p->winsize; cnt ++)
   {
      if (sb_p->ba_frame_bitmap[cnt] != (sb_p->bitmap[(cnt + sb_p->buf_winstart) & BA_BASIC_BITMAP_BUF_MASK]))
      {  
         return UU_FAILURE;
      }
   }
   return UU_SUCCESS;
}

/*
****** Func header *******
** Caller should decide whether to handle basic BA or compressed BA 
*/
uu_int32 uu_wlan_handle_comp_ba(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uint8 *ba_bitmap, uu_uchar ac)
{
    uu_uint16  cnt;

    /* Storing the Seq number received in BA frame */
    sb_p->ba_frame_seq_no = sn;

    /* 
    ** Storing the bitmap received in BA frame 
    ** Though the ba_frame_bitmap[] array contains 128 bytes only the first 8 bytes are considered
    ** in compressed BA case
    */
    for (cnt = 0; cnt < BA_BASIC_BITMAP_BUF_SIZE;)
    {
       sb_p->ba_frame_bitmap[cnt] = ((*ba_bitmap >> (cnt & 0x07)) & 0x01);

        /* Giving bitmap status to Tx-handler without checking the status(success or failure), for RTL consideration */
       uu_wlan_tx_update_mpdu_status_for_reagg((sn+cnt)& MPDU_SEQN_MASK, ac, sb_p->ba_frame_bitmap[cnt]); 
       cnt++;
       /* increment tmp_ptr only after filling 8 bits of *tmp_ptr */
       if ((cnt != 0) && ((cnt & 0x07) == 0))
       {
            ba_bitmap++;
       }
    }
    return 0;
}


/*
****** Func header *******
** Called when a packet is transmitted
** The below logic is implementation specific and not described by standard
** The coding as been done with the initial understanding of reorder buffer
** functionality, BA scorebard of peer STA as per 80211 spec
*/
/* NOTE: Functionality is similar to '_sb_update'. Only stats (counters) will change */
static uu_int32 _sb_update_tx(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uint16 fn)
{
   uu_uint16 clear_start;
   uu_uint16 clear_end;

   assert(NULL != sb_p);


   /* sn is within window */
   if ((sb_p->winstart <= sn) && (sn <= sb_p->winend))
   {
      sb_p->bitmap[(sb_p->buf_winstart+sn-sb_p->winstart) & BA_BASIC_BITMAP_BUF_MASK] |= (1 << fn);
   }
   /* sn is beyond window but within valid range */
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
   /* transmitting old packet (this is not supposed to happen) or could be wrap-around also */
   else 
   {
      return -1;
   }
   return 0;
} /* _sb_update_tx */


uu_int32 uu_wlan_sb_update_tx(uu_wlan_ba_sb_t *sb_p, uu_uint16 scf)
{
   uu_uint16 sn = MPDU_SN_FROM_SCF(scf);
   uu_uint16 fn = MPDU_FN_FROM_SCF(scf);

   if (_sb_update_tx(sb_p, sn, fn) != 0)
   {
      /* Try update again, considering wrap-around */
      if (_sb_update_tx(sb_p, BA_ELEVATE_SN_FOR_WRAPAROUND(sn), fn) != 0)
      {
         /* Received old packet. Update the error counter */
         return -1;
      }
   }
   return 0;
} /* uu_wlan_sb_update_tx */


/* EOF */


