/*******************************************************************************
**                                                                            **
** File name :  uu_mac_drv_txstatus.c                                         **
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
/* $Revision: 1.4 $ */

#include "uu_wlan_main.h"
#include "uu_mac_txstatus.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_reg.h"


uu_void uu_mac_drv_get_ac_txstatus(uu_void)
{
    uu_uint8 ac, indx;
    uu_uint32 status;
    uu_uint32 txstatus;

    /* Check status registers */
    for(ac=0; ac< UU_WLAN_MAX_QID; ac++)
    {
        txstatus = uu_getReg (uu_wlan_tx_status_flags_g[ac]);
        for(indx=0; indx<UU_WLAN_MAX_TX_STATUS_TO_UMAC; indx++)
        {
            status = txstatus & BIT(indx);
            if(status)
            {
                uu_wlan_send_tx_status(ac, indx);
            }
        }
    }
} /* uu_mac_drv_get_ac_txstatus */

