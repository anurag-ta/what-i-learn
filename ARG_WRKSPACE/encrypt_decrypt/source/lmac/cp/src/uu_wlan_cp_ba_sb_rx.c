/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ba_sb_rx.c                                  **
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

#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#endif

#include "uu_datatypes.h" 
#include "uu_wlan_cp_ba_session.h" 
#include "uu_wlan_cp_ba_sb.h"
#include "uu_wlan_cp_ba_sb_rx.h"


/*
****** Func header *******
** Handles Basic BA bitmap preparation
*/

static uu_int32 _sb_fill_basic_ba(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uint16  *ba_bitmap, uu_uint16 *seq_no_bitmap)
{
   uu_uint16  clear_start;
   uu_uint16  clear_end,cnt;

   /* SN is within our existing window */
   if ((sb_p->winstart <= sn) && (sn <= sb_p->winend))
   {
      /*
      ** Set to 0 the bits corresponding to MPDUs with Sequence Number subfield values from
      ** WinEndR + 1 through WinStartR + WinSizeR - 1.
      */

      clear_start = sb_p->buf_winstart + (sb_p->winend-sb_p->winstart) + 1;
      clear_end = sb_p->buf_winstart + (sn - sb_p->winstart + sb_p->winsize - 1);

      sb_p->buf_winstart = (sb_p->buf_winstart + sn - sb_p->winstart) & BA_BASIC_BITMAP_BUF_MASK;

      /* WinStartR = SSN */
      sb_p->winstart = sn;

      /* WinEndR = WinStartR + WinSizeR - 1 */
      sb_p->winend = sn + sb_p->winsize - 1;

   }
   /*
   ** The SN is beyond winend, but is NEW (within half of SN range).
   ** Move the window, according to SN.
    */
   else if ((sb_p->winend < sn) && (sn < sb_p->winstart + BA_SEQN_RANGE_BY2))
   {

      sb_p->buf_winstart = (sb_p->buf_winstart + sn ) & BA_BASIC_BITMAP_BUF_MASK;

      /* WinStartR = SSN */
      sb_p->winstart = sn;

      /* WinEndR = WinStartR + WinSizeR - 1 */
      sb_p->winend = sn + sb_p->winsize - 1;

      /*
      ** Set to 0 bits the corresponding to MPDU with Sequence Number subfield values from
      ** WinStartR to WinEndR
      */
      clear_start = sb_p->buf_winstart; 
      clear_end = sb_p->buf_winstart + sb_p->winsize;

   }
   else /* Received BAR for old packet (could be wrap-around also) */
   {

      /* 
      ** Preparing bitmap(with all zero's) for BA frame though the received BAR contains
      ** SN of an old packet
      */
      *seq_no_bitmap = (sn & MPDU_SEQN_MASK); 
      for (cnt  = 0; cnt < BA_BASIC_BITMAP_BUF_SIZE ; cnt++)
      {
         (*ba_bitmap) = 0; 
         ba_bitmap++;
      }
      return -1;
   }

   /* Clear all the bits in buffer, from WinEnd+1 to new-WinEnd, including new WinEnd */
   while (clear_start <= clear_end)
   {
      sb_p->bitmap[clear_start & BA_BASIC_BITMAP_BUF_MASK] = 0;
      clear_start++;
   }

   /* get the seq no of BA frame */
   *seq_no_bitmap = sb_p->winstart & MPDU_SEQN_MASK;

   /* Populating BA bitmap */
   for (cnt  = 0; cnt < BA_BASIC_BITMAP_BUF_SIZE ; cnt++)
   {
      (*ba_bitmap) = sb_p->bitmap[(cnt + sb_p->buf_winstart) & BA_BASIC_BITMAP_BUF_MASK];
      ba_bitmap++;
   }

   /* Wrap around */
   if (sb_p->winstart > BA_MPDU_SEQN_MAX)
   {
      sb_p->winstart = sb_p->winstart & MPDU_SEQN_MASK;
      sb_p->winend = sb_p->winend & MPDU_SEQN_MASK;
   }

   return 0;

} /* _sb_fill_ba */


/*
****** Func header *******
** Handles compressed BA bitmap preparation
** Make sure ba_bitap memory locations are cleared
*/

static uu_int32 _sb_fill_comp_ba(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uchar *ba_bitmap, uu_uint16 *seq_no_bitmap)
{
   uu_uint16  clear_start;
   uu_uint16  clear_end, cnt;

   /* SN is within our existing window */
   if ((sb_p->winstart <= sn) && (sn <= sb_p->winend))
   {
      /*
      ** Set to 0 the bits corresponding to MPDUs with Sequence Number subfield values from
      ** WinEndR + 1 through WinStartR + WinSizeR - 1.
      */

      clear_start = sb_p->buf_winstart + (sb_p->winend-sb_p->winstart) + 1;
      clear_end = sb_p->buf_winstart + (sn - sb_p->winstart + sb_p->winsize - 1);

      sb_p->buf_winstart = (sb_p->buf_winstart + sn - sb_p->winstart) & BA_BASIC_BITMAP_BUF_MASK;

      /* WinStartR = SSN */
      sb_p->winstart = sn;

      /* WinEndR = WinStartR + WinSizeR - 1 */
      sb_p->winend = sn + sb_p->winsize - 1;

   }
   /*
   ** The SN is beyond winend, but is NEW (within half of SN range).
   ** Move the window, according to SN.
   */
   else if ((sb_p->winend < sn) && (sn < sb_p->winstart + BA_SEQN_RANGE_BY2))
   {

      sb_p->buf_winstart = (sb_p->buf_winstart + sn ) & BA_BASIC_BITMAP_BUF_MASK;

      /* WinStartR = SSN */
      sb_p->winstart = sn;

      /* WinEndR = WinStartR + WinSizeR - 1 */
      sb_p->winend = sn + sb_p->winsize - 1;

      /*
      ** Set to 0 bits the corresponding to MPDU with Sequence Number subfield values from
      ** WinStartR to WinEndR
      */
      clear_start = sb_p->buf_winstart; 
      clear_end = sb_p->buf_winstart + sb_p->winsize;

   }
   else /* Received BAR for old packet (could be wrap-around also) */
   {

      /* 
      ** Preparing bitmap(with all zero's) for BA frame as the received BAR contains
      ** SN of old packet
      */
      *seq_no_bitmap = (sn & MPDU_SEQN_MASK); 
      for (cnt  = 0; cnt < BA_COMP_BITMAP_SIZE; cnt++)
      {
         (*ba_bitmap) = 0; 
         ba_bitmap++;
      }
      return -1;
   }

   /* Clear all the bits in buffer, from WinEnd+1 to new-WinEnd, including new WinEnd */
   while (clear_start <= clear_end)
   {
      sb_p->bitmap[clear_start & BA_BASIC_BITMAP_BUF_MASK] = 0;
      clear_start++;
   }

#if 0
   /* get the seq no of BA frame */
   *seq_no_bitmap = sb_p->winstart & MPDU_SEQN_MASK;

   /* Populating BA bitmap */
   for (cnt = 0; cnt < BA_BASIC_BITMAP_BUF_SIZE;)
   {
      (*ba_bitmap) |= ((sb_p->bitmap[(cnt + sb_p->buf_winstart) & BA_BASIC_BITMAP_BUF_MASK]) << cnt);  /* Is this correct? Look for other similar equations */
      cnt++;
      /* increment tmp_ptr only after filling 8 bits of *tmp_ptr */
      if ((cnt & 0x07) == 0)
      {
         ba_bitmap++;
      }
   }
#else
   uu_wlan_sb_fill_implicit_comp_ba(sb_p, ba_bitmap, seq_no_bitmap);
#endif

   /* Wrap around */
   if (sb_p->winstart > BA_MPDU_SEQN_MAX)
   {
      sb_p->winstart = sb_p->winstart & MPDU_SEQN_MASK;
      sb_p->winend = sb_p->winend & MPDU_SEQN_MASK;
   }

   return 0;
} /* _sb_fill_comp_ba */


uu_int32 uu_wlan_sb_fill_implicit_comp_ba(uu_wlan_ba_sb_t *sb_p, uu_uint8 *ba_bitmap, uu_uint16 *seq_no_bitmap)
{
   uu_uint16  cnt;

   *seq_no_bitmap = (sb_p->winstart & MPDU_SEQN_MASK);

   /* Populating BA bitmap */
   for (cnt = 0; cnt < BA_BASIC_BITMAP_BUF_SIZE;)
   {
      (*ba_bitmap) |= ((sb_p->bitmap[(cnt + sb_p->buf_winstart) & BA_BASIC_BITMAP_BUF_MASK]) << (cnt & 0x7)) ;
      cnt++;
      /* increment tmp_ptr only after filling 8 bits of *tmp_ptr */
      if ((cnt & 0x07) == 0)
      {
         ba_bitmap++;
      }
   }

   return 0;
}


/*
****** Func header *******
** Caller should decide whether to call compressed or Basic BA and accordingly allocate mem for BA bitmap
*/
uu_int32 uu_wlan_sb_fill_basic_ba(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uint16 *ba_bitmap, uu_uint16 *seq_no_bitmap)
{
   uu_int32 ret;

   assert(NULL != ba_bitmap);
   assert(NULL != seq_no_bitmap);

// Caller will extract/provide seq no of BAR frame
//   unsigned short sn = MPDU_SN_FROM_SCF(scf);

   if ((ret = _sb_fill_basic_ba(sb_p, sn, ba_bitmap, seq_no_bitmap)) != 0)
   {
      /* Try update again, considering wrap-around */
      if ((ret = _sb_fill_basic_ba(sb_p, BA_ELEVATE_SN_FOR_WRAPAROUND(sn), ba_bitmap, seq_no_bitmap)) != 0)
      {
         /* Received old packet. Update the error counter */
      }
   }

   return ret;
} /* uu_wlan_sb_fill_ba */


/*
****** Func header *******
** Caller should decide whether to call compressed or Basic BA and accordingly allocate mem for BA bitmap
*/
uu_int32 uu_wlan_sb_fill_comp_ba(uu_wlan_ba_sb_t *sb_p, uu_uint16 sn, uu_uchar *ba_bitmap, uu_uint16 *seq_no_bitmap)
{
   uu_int32 ret;

   assert(NULL != ba_bitmap);
   assert(NULL != seq_no_bitmap);

//   unsigned short sn = MPDU_SN_FROM_SCF(scf);

   if ((ret = _sb_fill_comp_ba(sb_p, sn, ba_bitmap, seq_no_bitmap)) != 0)
   {
      /* Try update again, considering wrap-around */
      if ((ret = _sb_fill_comp_ba(sb_p, BA_ELEVATE_SN_FOR_WRAPAROUND(sn), ba_bitmap, seq_no_bitmap)) != 0)
      {
         /* Received old packet. Update the error counter */
      }
   }

   return ret;
} /* uu_wlan_sb_fill_comp_ba */


/* EOF */


