/*******************************************************************************
 **                                                                            **
 ** File name :  uu_wlan_platform.c                                            **
 **                                                                            **
 ** Copyright © 2013, Uurmi Systems                                            **
 ** All rights reserved.                                                       **
 ** http://www.uurmi.com                                                       **
 **                                                                            **
 ** All information contained herein is property of Uurmi Systems              **
 ** unless otherwise explicitly mentioned.                                     **
 **                                                                            **
 ** The intellectual and technical concepts in this file are proprietary       **
 ** to Uurmi Systems and may be covered by granted or in process national      **
 ** and international patents and are protect by trade secrets and             **
 ** copyright law.                                                             **
 **                                                                            **
 ** Redistribution and use in source and binary forms of the content in        **
 ** this file, with or without modification are not permitted unless           **
 ** permission is explicitly granted by Uurmi Systems.                         **
 **                                                                            **
 *******************************************************************************/
/* $Revision: 1.2 $ */

/** 
 * @uu_wlan_platform.c
 * Contains all the platform dependent fuctionality.
 */
#include "uu_wlan_main.h"
#include "uu_mac_drv.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_frame.h"
#include "uu_mac_txstatus.h"
#include "uu_wlan_recv.h"
#include "uu_wlan_init.h"
#include "uu_wlan_reg.h"
#include "uu_mac_scheduling.h"


uu_wlan_tx_status_info_t uu_wlan_tx_status_info_g[UU_WLAN_AC_MAX_AC][UU_WLAN_MAX_TX_STATUS_TO_UMAC];


uu_void uu_wlan_tx_write_buffer_platform(uu_uchar ac, uu_uchar *adr[], uu_uint32 len[])
{

}


uu_void uu_wlan_set_mode_platform(uu_uint8 mode)
{
    uu_setReg(Uu_operational_settings_r, mode);


}


uu_void uu_wlan_set_qos_platform(uu_uint8 mode)
{

}


uu_void uu_wlan_change_bss_rate_platform(uu_uint16 rate)
{
}





uu_uint32 uu_wlan_tx_get_available_buffer_platform(uu_uint8 ac)
{
    /* TODO: Implement this */
    return 1000;
}


/** Interrupt handler */
irqreturn_t uu_wlan_interrupt(int irq, void *dev_id)
{
    uu_uint32 regval;
    printk("Interrupt triggered -------------------\n");
    /** Read ISR register */


    regval = uu_getReg(UU_ISR_OFFSET);

    if(regval&UU_ISR_TXSTATUS_MASK)
    {
        /**Schedule task to handle tx status */
        printk("TX STATUS is received from LMAC \n");
        /** schedule rx_tasklet */
        tasklet_schedule(&uu_wlan_context_gp->rx_task);
    }
    else if(regval&UU_ISR_RX_MASK)
    {
        /** Schedule task to handle rx packet */
        printk("RX packet is received from LMAC \n");
        tasklet_schedule(&uu_wlan_context_gp->txstatus_task);

    }

    return IRQ_HANDLED;
}


void uu_wlan_rx_tasklet(unsigned long data)
{
    uu_wlan_umac_context_t *mac_context = (uu_wlan_umac_context_t *)data;

    printk("Task is scheduled and calling main rx function \n");
    uu_wlan_rx(mac_context->hw);


}
void uu_wlan_txstatus_tasklet(unsigned long data)
{
    uu_wlan_umac_context_t *mac_context = (uu_wlan_umac_context_t *)data;

    uu_mac_drv_get_ac_txstatus();
}


uu_void uu_wlan_fill_rx_status(uu_wlan_rx_frame_info_t *recv_fi, uu_wlan_rx_status_t *rs)
{

}


uu_int32 uu_wlan_rx_setfilters_platform(uu_uint32 total_flags)
{

    return 0;
}


uu_uint32 uu_wlan_rx_get_status_flags_platform(uu_uint32 i)
{

    return 0;

}


uu_void uu_wlan_rx_reset_status_flags_platform(uu_uint32 i)
{

    return;

}


uu_void uu_wlan_rx_get_frame_addr_platform(uu_uint8 jj,uu_wlan_rx_frame_info_t *recv_fi,uu_uchar** mpdu)
{


}


uu_void uu_wlan_tx_status_info_platform(uu_uint32 i,uu_uint32 j)
{
    uu_wlan_tx_status_info_g[i][j].seqNo = uu_getReg(UU_WLAN_TX_STATUS_INFO_R(i,j)) & UU_WLAN_TXSTATUS_SEQNO_MASK;
    uu_wlan_tx_status_info_g[i][j].retry_count =getbit(uu_getReg(UU_WLAN_TX_STATUS_INFO_R(i,j)),16,3);
    uu_wlan_tx_status_info_g[i][j].status = getbit(uu_getReg(UU_WLAN_TX_STATUS_INFO_R(i,j)),24,1)

}



uu_uint32 uu_wlan_tx_get_status_seq_no_platform(uu_uint32 i, uu_uint32 j)
{
    return  uu_wlan_tx_status_info_g[i][j].seqNo;
}


uu_uint32 uu_wlan_tx_get_status_platform(uu_uint8 qid, uu_uint8 index)
{
    return uu_getReg(uu_wlan_tx_status_flags_g(qid)) & (1 << index);
}

uu_void uu_wlan_tx_reset_status_flags_platform(uu_uint32 qid, uu_uint32 index)
{
    //TODO: THis is wrong.
    uu_setReg(uu_wlan_tx_status_flags_g(qid), (uu_getReg(uu_wlan_tx_status_flags_g(qid)) & index);
}

