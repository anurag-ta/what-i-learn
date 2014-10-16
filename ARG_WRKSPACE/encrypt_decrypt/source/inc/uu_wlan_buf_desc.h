/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_buf_desc.h                                     **
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

#ifndef __UU_WLAN_BUF_DESC_H__
#define __UU_WLAN_BUF_DESC_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"


/*
** Tx buffers are sub-divided into 4 groups (ACs). Each AC gets 16-buffers.
**
** Descriptor id is assumed to be signed 16-bit value.  -ve value is used to indicate error.
** The bit 14 (MSB, after sign bit) is used to indicate direction (tx=1, rx=0).
** The bit 12 & 13 are used to indicate ac, for the Tx buffers.
*/
#define UU_WLAN_TX_BUF_MAX_DESC_PER_AC       16
#define UU_WLAN_RX_BUF_MAX_DESC              16


/* The 15-bit Tx descriptor ID contains multiple fields in it */
#define UU_WLAN_BUF_DESC_TX_FLAG        0x4000
#define UU_WLAN_BUF_DESC_TX_FLAG_SHIFT     14
#define UU_WLAN_BUF_DESC_TX_AC_MASK     0x3000
#define UU_WLAN_BUF_DESC_TX_AC_SHIFT       12
/* This mask excludes other fields in the Tx descriptor ID.
** Number of bits here could be more than actually required.
** Consider descriptor ID wrap-around (aggregation case), before touching this.
*/
#define UU_WLAN_BUF_DESC_TX_INDEX_MASK  0x0FFF


extern uu_uchar *uu_wlan_desc_get_tx_data_ptr(uu_int16 desc_no);

extern uu_uchar *uu_wlan_desc_get_rx_data_ptr(uu_int16 desc_no);

/** Function to get descriptor of a free Tx buffer. Function takes AC as input.
** The AC will be used, in case the buffers are maintained on per-AC basis.
*/
extern uu_int16 uu_wlan_desc_get_tx_buf(uu_uint8 ac);

/** Function to get a bunch of consecutive Tx buffers, from circular buffer of an AC.
** Returns the first buffer desciptor, from which 'count' consecutive descriptors are usable.
** All Tx functions in this file can handle Tx descriptor wrap-around (aggregation case).
** As allocation is done as a bunch, freeing all buffers should be done at one go.
*/
extern uu_int16  uu_wlan_desc_get_tx_buf_aggr(uu_uint8 ac, uu_int16 count);

extern uu_int16 uu_wlan_desc_get_rx_buf(uu_void);

extern uu_void  uu_wlan_desc_tx_free(uu_int16 id);

/** Function to free a bunch of consecutive Tx buffers, from circular buffer of an AC.
** This is complementary function to 'uu_wlan_desc_get_tx_buf_aggr'.
*/
extern uu_void  uu_wlan_desc_tx_free_aggr(uu_int16 id, uu_int16 count);

extern uu_void  uu_wlan_desc_rx_free(uu_int16 id);


#endif /* __UU_WLAN_BUF_DESC_H__ */

/* EOF */


