/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_buf_desc.c                                     **
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
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"


/* DESC state - 0 indicates free, non-zero indicates busy */
static uu_bool  uu_wlan_desc_tx_busy_state_g[UU_WLAN_MAX_QID][UU_WLAN_TX_BUF_MAX_DESC_PER_AC];
static uu_bool  uu_wlan_desc_rx_busy_state_g[UU_WLAN_RX_BUF_MAX_DESC];

/** Tx Buffers.
** This is treated as 4 circular buffers of equal size, one per AC.
** The buffer descriptor number may wrap around, when using consecutive buffers for aggregation.
*/
static uu_wlan_tx_frame_info_t  uu_wlan_desc_tx_data_g[UU_WLAN_MAX_QID][UU_WLAN_TX_BUF_MAX_DESC_PER_AC];

/*
** Tx descriptor, that was most recently marked as busy.
** Note: UMAC may free this descriptor (in some conditions), without sending buffer to LMAC.
**       If that happens, there could be unused buffer in-between used buffers.
*/
static uu_int16  uu_wlan_desc_tx_index_last_returned_g[UU_WLAN_MAX_QID] = {UU_WLAN_TX_BUF_MAX_DESC_PER_AC,
                                                    UU_WLAN_TX_BUF_MAX_DESC_PER_AC, UU_WLAN_TX_BUF_MAX_DESC_PER_AC,
                                                    UU_WLAN_TX_BUF_MAX_DESC_PER_AC
#ifdef UU_WLAN_BQID
                                                    , UU_WLAN_TX_BUF_MAX_DESC_PER_AC
#endif
                                                    };

/** Rx Buffers. */
static uu_wlan_rx_frame_info_t  uu_wlan_desc_rx_data_g[UU_WLAN_RX_BUF_MAX_DESC];


uu_int16  uu_wlan_desc_get_tx_buf(uu_uint8 ac)
{
    uu_int16  index;
    uu_int16  desc_no = -1;

#if 0 /* No longer required, as we are maintaining write index */
    for (i = 0; i < UU_WLAN_TX_BUF_MAX_DESC_PER_AC; i++)
#endif
    {
        index = (uu_wlan_desc_tx_index_last_returned_g[ac] +1) % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;

        if (uu_wlan_desc_tx_busy_state_g[ac][index] == 0)
        {
            uu_wlan_desc_tx_busy_state_g[ac][index] = 1;
            uu_wlan_desc_tx_index_last_returned_g[ac] = index;
            desc_no = UU_WLAN_BUF_DESC_TX_FLAG | (ac << UU_WLAN_BUF_DESC_TX_AC_SHIFT) | index;
            printk("Allocating tx buffer: (ac:%d) %d\n", ac, index);
            return desc_no;
        }
    }
    UU_WLAN_LOG_ERROR(("UURMI:  WARNING: Tx Buffer Descriptors are full. \n"));
    return -1;
}


uu_int16  uu_wlan_desc_get_tx_buf_aggr(uu_uint8 ac, uu_int16 count)
{
    uu_int16  i, j;
    uu_int16  found = 0;
    uu_int16  index;
    uu_int16  desc_no = -1;

    /* Find whether there are 'count' number of consecutive free buffers, from current position */
    for (i = 0; i < UU_WLAN_TX_BUF_MAX_DESC_PER_AC; i++)
    {
        index = (uu_wlan_desc_tx_index_last_returned_g[ac]+i +1) % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;

        if (uu_wlan_desc_tx_busy_state_g[ac][index] == 0)
        {
            found++;

            /* If got enough buffers, starting from write-index */
            if (found == count)
            {
                index = (uu_wlan_desc_tx_index_last_returned_g[ac] +1) % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;
                desc_no = UU_WLAN_BUF_DESC_TX_FLAG | (ac << UU_WLAN_BUF_DESC_TX_AC_SHIFT) | index;

                printk("Allocating tx buffer: (ac:%d) %d, count:%d\n", ac, index, count);
                /* Mark all buffers till now, as busy */
                for (j = 0; j < count; j++)
                {
                    uu_wlan_desc_tx_busy_state_g[ac][index] = 1;
                    index = (index +1) % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;
                }

                /* We have marked 'count' descriptors as busy */
                uu_wlan_desc_tx_index_last_returned_g[ac] = (uu_wlan_desc_tx_index_last_returned_g[ac] + count) % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;
                return desc_no;
            }
        }
        else /* Encountered a busy buffer, after some free buffers */
        {
            break;
        }
    }

    UU_WLAN_LOG_ERROR(("UURMI: WARNING: Tx Buffer Descriptors are full. Needed: %d, free: %d\n", count, found));
    return -1;
} /* uu_wlan_desc_get_tx_buf_aggr */


uu_int16  uu_wlan_desc_get_rx_buf(uu_void)
{
    uu_int16 i;

    for (i = 0; i < UU_WLAN_RX_BUF_MAX_DESC; i++)
    {
        if (uu_wlan_desc_rx_busy_state_g[i] == 0)
        {
            uu_wlan_desc_rx_busy_state_g[i] = 1;
            return i;
        }
    }
    UU_WLAN_LOG_ERROR(("UURMI:  WARNING: Rx Buffer Descriptors are full. \n"));
    return -1;
}


uu_void  uu_wlan_desc_tx_free(uu_int16 desc_no)
{
    uu_bool  is_tx_buf = desc_no >> UU_WLAN_BUF_DESC_TX_FLAG_SHIFT;
    uu_uint8 ac = (((uu_uint16)desc_no) & UU_WLAN_BUF_DESC_TX_AC_MASK) >> UU_WLAN_BUF_DESC_TX_AC_SHIFT;
    uu_int16 index = desc_no & UU_WLAN_BUF_DESC_TX_INDEX_MASK;

    /* Gracefully handle the buffer descriptor wrap-around.
    ** In aggregation case, consecutive buffer descriptros are allocated.
    ** But as we use circular buffer, the buffer descriptor number has to be suitably-handled.
    */
    index = index % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;

    if ((is_tx_buf) && (index >= 0))
    {
        printk("Freeing tx buffer: (ac:%d) %d\n", ac, index);
        uu_wlan_desc_tx_busy_state_g[ac][index] = 0;
    }
    else
    {
        UU_WLAN_LOG_ERROR(("UURMI: Wrong buffer desc given for Tx-free\n"));
    }
    return;
}


#if 0 /* Not used anywhere */
uu_void  uu_wlan_desc_tx_free_aggr(uu_int16 desc_no, uu_int16 count)
{
    uu_bool  is_tx_buf = desc_no >> UU_WLAN_BUF_DESC_TX_FLAG_SHIFT;
    uu_uint8 ac = (((uu_uint16)desc_no) & UU_WLAN_BUF_DESC_TX_AC_MASK) >> UU_WLAN_BUF_DESC_TX_AC_SHIFT;
    uu_int16 index = desc_no & UU_WLAN_BUF_DESC_TX_INDEX_MASK;
    uu_int16 i;

    /* Gracefully handle the buffer descriptor wrap-around.
    ** In aggregation case, consecutive buffer descriptros are allocated.
    ** But as we use circular buffer, the buffer descriptor number has to be suitably-handled.
    */
    if ((is_tx_buf) && (index >= 0))
    {
        for (i = 0; i < count; i++)
        {
            printk("Freeing tx buffer: (ac:%d) %d\n", ac, index);
            uu_wlan_desc_tx_busy_state_g[ac][index] = 0;
            index = (index + 1) % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;
        }
    }
    else
    {
        UU_WLAN_LOG_ERROR(("UURMI: Wrong buffer desc given for Tx-free\n"));
    }
    return;
} /* uu_wlan_desc_tx_free_aggr */
#endif


uu_void  uu_wlan_desc_rx_free(uu_int16 desc_no)
{
    if ((desc_no < UU_WLAN_RX_BUF_MAX_DESC) && (desc_no >= 0))
    {
        printk("Freeing rx buffer: %d\n", desc_no);
        uu_wlan_desc_rx_busy_state_g[desc_no] = 0;
    }
    else
    {
        UU_WLAN_LOG_ERROR(("UURMI: Wrong buffer desc given for Rx-free\n"));
    }
    return;
}


uu_uchar *uu_wlan_desc_get_tx_data_ptr(uu_int16 desc_no)
{
    uu_bool  is_tx_buf = desc_no >> UU_WLAN_BUF_DESC_TX_FLAG_SHIFT;
    uu_uint8 ac = (((uu_uint16)desc_no) & UU_WLAN_BUF_DESC_TX_AC_MASK) >> UU_WLAN_BUF_DESC_TX_AC_SHIFT;
    uu_int16 index = desc_no & UU_WLAN_BUF_DESC_TX_INDEX_MASK;

    /* Gracefully handle the buffer descriptor wrap-around.
    ** In aggregation case, consecutive buffer descriptros are allocated.
    ** But as we use circular buffer, the buffer descriptor number has to be suitably-handled.
    */
    if ((is_tx_buf) && (index >= 0))
    {
        index = index % UU_WLAN_TX_BUF_MAX_DESC_PER_AC;
        return (uu_uchar*)&uu_wlan_desc_tx_data_g[ac][index];
    }
    else
    {
        UU_WLAN_LOG_ERROR(("UURMI: Wrong buffer desc: %d given for get_tx_data_ptr\n", desc_no));
        return NULL;
    }
}


uu_uchar *uu_wlan_desc_get_rx_data_ptr(uu_int16 desc_no)
{
    if ((desc_no >= 0) && (desc_no < UU_WLAN_RX_BUF_MAX_DESC))
    {
        return (uu_uchar*)&uu_wlan_desc_rx_data_g[desc_no];
    }
    else
    {
        UU_WLAN_LOG_ERROR(("UURMI: Wrong buffer desc: %d given for get_rx_data_ptr\n", desc_no));
        return NULL;
    }
}


/* EOF */


