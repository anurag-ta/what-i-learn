
/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_reg.h                                           **
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

#ifndef __UU_WLAN_REG_H__
#define __UU_WLAN_REG_H__

#include "uu_datatypes.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_lmac_if.h"


/** Enumeration of operational modes.
 * UMAC set the default operational mode as Managed ie STA.
 * Based on the userspace application running on top of driver, umac updates the
 * operational mode.
 * Ex : Sets to MASTER mode if hostapd application is running.
 * If the operational mode is MASTER, then lmac starts the beacon timer.
 */
typedef enum uu_wlan_op_mode
{
    /** STA, defatult */
    UU_WLAN_MODE_MANAGED  = 0,
    /** AP mode */
    UU_WLAN_MODE_MASTER   = 1,
    /** Monitor mode */
    UU_WLAN_MODE_MONITOR  = 2,
    UU_WLAN_MODE_ADHOC    = 3,
    UU_WLAN_MODE_MESH     = 4,
    UU_WLAN_MODE_REPEATER = 5
} uu_wlan_op_mode_t;


/* Filters */
typedef enum uu_wlan_lmac_filter_flags
{
    UU_WLAN_FILTR_NO_FILTER        = BIT(0),
    UU_WLAN_FILTR_CTL_FRAMES       = BIT(1),
    UU_WLAN_FILTR_ALLOW_RTS_CTS    = BIT(2),

    UU_WLAN_FILTR_BROAD_CAST       = BIT(3),
    UU_WLAN_FILTR_MULTI_CAST       = BIT(4),

    UU_WLAN_FILTR_PROBE_REQ       = BIT(5),
    UU_WLAN_FILTR_BEACON          = BIT(6),
    UU_WLAN_FILTR_PROBE_RESP      = BIT(7),
    UU_WLAN_FILTR_PSPOLL          = BIT(8),

    /* LAST Flag */
    UU_WLAN_FILTR_DROP_ALL         = BIT(9)
} uu_wlan_lmac_filter_flags_t;


/* TODO: Verify whether these comments are still valid */
/* Mail Box Registers */
/*
 *  UU_REG_LMAC_CONF_BUSY:
 *      LMAC sets this register while processing the update request.
 *      After processing completed this register will be cleared
 *
 *   NOTE: UMAC should check this before updating the new request.
 *         If this SET, UMAC should wait untill it clears by LMAC
 *
 *  UU_REG_LMAC_CONF_UPDATE_PENDING:
 *      UMAC updates the respective registers and SETS this value for updating the request in LMAC.
 *
 *   NOTE: UMAC should check this before updating the new request.
 *         If this SET, UMAC should wait until it clears by LMAC.
 *
 * For UU_REG_CONF_BEACON_INFO_UPDATED:
 *    Update the Register values directly before giving the request
 *
 * For UU_REG_CONF_CH_CHANGE:
 *    UU_REG_LMAC_CONF_RQ_8BIT_V1  = new channel number
 *
 * For UU_REG_CONF_BA_SES_ADD:
 *    UU_REG_LMAC_CONF_RQ_8BIT_V1  = Tid
 *    UU_REG_LMAC_CONF_RQ_32BIT_V1 = BA session type "uu_wlan_ba_ses_type_t"
 *    UU_REG_LMAC_CONF_RQ_32BIT_V2 = Buffer Size
 *    UU_REG_LMAC_CONF_RQ_32BIT_V3 = STA(for which ba established) mac address of first 4 bytes
 *    UU_REG_LMAC_CONF_RQ_32BIT_V4 = STA(for which ba established) mac address of last 2 bytes at lower order
 *    UU_REG_LMAC_CONF_RQ_32BIT_V4  = 16bit Starting sequence number at higher order
 *
 *
 * For UU_REG_CONF_BA_SES_DEL
 *     UU_REG_LMAC_CONF_RQ_8BIT_V1  = Tid
 *     UU_REG_LMAC_CONF_RQ_32BIT_V1 = STA(for which ba established) mac address of first 4 bytes
 *     UU_REG_LMAC_CONF_RQ_32BIT_V2 = STA(for which ba established) mac address of last 2 bytes at lower order
 *
 */
/* TODO: Combine this, with station information config */
typedef enum uu_wlan_conf_reg_update_flags
{
    UU_REG_CONF_BEACON_INFO_UPDATED = 1,
    //UU_REG_CONF_CH_CHANGE,
    UU_REG_CONF_BA_SES_ADD,
    UU_REG_CONF_BA_SES_DEL

    //UU_REG_CONF_NO_CHANGE /* This should be last */
} uu_wlan_conf_reg_update_flags_t;


/* TODO: Combine with with Interrupt Status Register. Applicable for both C & RTL models */
typedef enum uu_wlan_ind
{   
    /** Indication for tx status */
    UU_IND_TX_STATUS = 1,
    /** Indication for rx frame ready */
    UU_IND_RX_FRAME
} uu_wlan_ind_t;


/*
** Function Declarations
*/

extern uu_void uu_wlan_tx_write_buffer_platform(uu_uchar ac, uu_uchar *adr[], uu_uint32 len[]);

extern uu_uint32 uu_wlan_tx_get_available_buffer_platform(uu_uint8 ac);

extern uu_void uu_wlan_clear_rx_status_flag_platform(uu_uint8 index);

extern uu_uint32 uu_wlan_get_rx_status_platform(uu_uint8 index);

extern uu_void uu_wlan_get_rx_status_info_platform(uu_wlan_rx_status_info_t *rsi, uu_uint8 index);

extern uu_void uu_wlan_get_data_platform(uu_char *dest ,uu_uchar *src ,uu_uint32 len);

extern uu_void uu_wlan_tx_status_info_platform(uu_uint32 i,uu_uint32 j);

extern uu_uint32 uu_wlan_tx_get_status_seq_no_platform(uu_uint32 i, uu_uint32 j);

extern uu_uint32 uu_wlan_tx_get_status_platform(uu_uint8 qid, uu_uint8 index);

extern uu_void uu_wlan_tx_reset_status_flags_platform(uu_uint32 i,uu_uint32 j);

#if 1 /* TODO: Find out the purpose of these. Probably not required */
extern uu_void uu_wlan_umac_send_addba_info_platform(uu_uint8 tid, uu_char* addr, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_bool dir);

extern uu_void uu_wlan_umac_send_delba_info_platform(uu_uint8 tid, uu_char* addr, uu_uint8 dir);

extern uu_void uu_wlan_change_defaults(uu_void);
#endif


/* Contains platform-specific expansion for macros */
#include "uu_wlan_reg_platform.h"

#endif /* __UU_WLAN_REG_H__ */

