/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_rx_q.c                                         **
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
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_buf_desc.h"


#define UU_BUF_SIZE_32K_MASK   0X00007FFF 
#define UU_BUF_SIZE_32K        0X00008000

#define UU_BUF_C_MODEL_MIN_SIZE       0X00001000
#define UU_BUF_C_MODEL_MIN_SIZE_MASK  0X00000FFF

/** Receive circular buffer of size 32K
 * RX handler fills the buffer with data bytes received from physical layer. 
 * Write index will be updated while filling the data.
 * Buffer contains the individual packets, each packet address and length gives to UMAC.
 * UMAC reads the buffer using receive status information.
 */
static uu_uchar uu_rx_q_g[UU_BUF_SIZE_32K];

/** Contains write index, at which next byte has to be written. Need to do +1, on write.
 * On writing complete frame, adjustment will be done for alignment (both in C & RTL models).
 * If the frame need not be given to UMAC, it will be reset to 'uu_rx_q_curr_frame_info_wr_indx_g'.
*/
static uu_int32 uu_rx_q_wr_indx_g = 0;

/** Contains current frame info starting location in the circular buffer.
 * Updated to 'uu_rx_q_wr_indx_g', after writing complete frame for giving to UMAC.
*/
static uu_int32 uu_rx_q_curr_frame_info_wr_indx_g = 0;

/*
** Contains the Rx buffer's base address. Intialized with Rx buffer starting address.
** This is exposed to UMAC, as a register.
** For giving Rx frame to UMAC, the offset is provided in Rx-Status.
*/
uu_uchar* uu_wlan_rx_buff_base_addr_reg_g = uu_rx_q_g;
EXPORT_SYMBOL(uu_wlan_rx_buff_base_addr_reg_g);


/** Writes the data into the LMAC circular buffer
 * param[in] adr contains the address of the data
 * param[in] len contains the length of the data
 * param[in] rx_end_ind_success contains the rx end indication status
 */
uu_void uu_wlan_rx_write_buffer(uu_uchar *adr, uu_uint16 len, uu_bool rx_end_ind_success)
{
    uu_uint32 temp_len;
    
    /* writing into the circular buffer of particular RX */
    if ((adr != UU_NULL) && (len != 0))
    {
        temp_len = UU_BUF_SIZE_32K - uu_rx_q_wr_indx_g;
        /* len not causing the wrap-around */ 
        if (temp_len >= len)
        {
            UU_WLAN_COPY_BYTES(&uu_rx_q_g[uu_rx_q_wr_indx_g], adr, len);
            uu_rx_q_wr_indx_g = (uu_rx_q_wr_indx_g + len) & UU_BUF_SIZE_32K_MASK;
        }
        /* Wrap-around for circular buffer */ 
        else
        {
            UU_WLAN_COPY_BYTES(&uu_rx_q_g[uu_rx_q_wr_indx_g], adr, temp_len);
            uu_rx_q_wr_indx_g = 0;
            UU_WLAN_COPY_BYTES(&uu_rx_q_g[uu_rx_q_wr_indx_g], adr+temp_len, len - temp_len);
            uu_rx_q_wr_indx_g = len - temp_len;
        }
    } /* while */

#if 1 /* C model only. Avoid problems with wrap-around */
    if (rx_end_ind_success)
    {
        if (uu_rx_q_wr_indx_g & UU_BUF_C_MODEL_MIN_SIZE_MASK)
        {
            uu_rx_q_wr_indx_g += UU_BUF_C_MODEL_MIN_SIZE - (uu_rx_q_wr_indx_g & UU_BUF_C_MODEL_MIN_SIZE_MASK);
            uu_rx_q_wr_indx_g = uu_rx_q_wr_indx_g & UU_BUF_SIZE_32K_MASK;
        }
        uu_rx_q_curr_frame_info_wr_indx_g = uu_rx_q_wr_indx_g; 
    }
#endif
} /* uu_wlan_rx_write_buffer */


/* Resets the current frame write index
 * For failed packets, reset the write index to current frame_info write index 
 */
uu_void uu_wlan_reset_curr_frame_wrindx(uu_void)
{
    uu_rx_q_wr_indx_g = uu_rx_q_curr_frame_info_wr_indx_g;
} /* uu_wlan_reset_curr_frame_wrindx */


/* Gets the reference of the current frame info from the RX Q
 * WARNING: In RTL model, should not read the buffer after data writing into circular buffer.  
 */
uu_wlan_rx_frame_info_t* uu_wlan_rx_get_frame_info(uu_void)
{
    return ((uu_wlan_rx_frame_info_t *)(&uu_rx_q_g[uu_rx_q_curr_frame_info_wr_indx_g & UU_BUF_SIZE_32K_MASK]));
}


/** Rx status updation indication from Rx-handler to UMAC.
 */
uu_void uu_wlan_update_rx_status(uu_void)
{
    uu_wlan_rx_frame_info_t *frame_info = uu_wlan_rx_get_frame_info();

    uu_wlan_rx_update_status_for_umac(uu_rx_q_curr_frame_info_wr_indx_g,
        (frame_info->frameInfo.framelen + sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t)
#ifdef UU_WLAN_TSF
        + sizeof(frame_info->timestamp_msb) + sizeof(frame_info->timestamp_lsb)
        + sizeof(frame_info->becn_timestamp_msb_tsf) + sizeof(frame_info->becn_timestamp_lsb_tsf)
#endif
        ));
}


/* EOF */

