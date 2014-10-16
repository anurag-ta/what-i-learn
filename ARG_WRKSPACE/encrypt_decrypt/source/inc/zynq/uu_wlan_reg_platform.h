/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_reg_platform.h                                  **
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

#ifndef __UU_WLAN_REG_PLATFORM_H__
#define __UU_WLAN_REG_PLATFORM_H__
#include "uu_datatypes.h"
#include <linux/io.h>
#include <linux/interrupt.h>

#if 0 /* Implemented as functions */
#define uu_setReg(reg_name, val)  __raw_writel(val, reg_name)
#define uu_getReg(reg_name)       __raw_readl(reg_name)
#endif
#define uu_wlan_lmac_start_platform()     0
#define uu_wlan_lmac_mode_switch_platform() 0
#define uu_wlan_lmac_stop_platform()      0


extern void __iomem *baseaddr_config;
extern void __iomem *baseaddr_status;
extern void __iomem *baseaddr_txbuf;
extern void __iomem *baseaddr_rxbuf;
extern void __iomem *baseaddr_stamgmt;


extern uu_int32 uu_getReg(uu_uint32  offset);
extern uu_int32 uu_getstatusReg(uu_uint32 offset);
extern uu_int32 uu_gettxReg(uu_uint32 offset);
extern uu_int32 uu_getrxReg(uu_uint32 offset);
extern uu_int32 uu_getstamgmtReg(uu_uint32 offset);

extern uu_void uu_setReg(uu_uint32 val,uu_uint32 offset);
extern uu_void uu_setstatusReg(uu_uint32 val,uu_uint32 offset);
extern uu_void uu_settxReg(uu_uint32 val,uu_uint32 offset);
extern uu_void uu_setrxReg(uu_uint32 val,uu_uint32 offset);
extern uu_void uu_setstamgmtReg(uu_uint32 val,uu_uint32 offset);

#define uu_getBits(value,start,num) (value>>start)&(~(~0>>num))

/** BRAM Offset Address for Station */
#define UU_BRAM_OFFSET_STA_BASE_ADDR	0x14
#define UU_STA_OFFSET			0x0030
#define UU_BRAM_OFFSET_STA(i)		(UU_BRAM_OFFSET_STA_BASE_ADDR + (i * UU_STA_OFFSET))

/** BRAM Offset Address for BA bitmap */
#define UU_BRAM_OFFSET_BA_BASE_ADDR	0x15
#define UU_BA_OFFSET			0x0010
#define UU_BRAM_OFFSET_BA(i)		(UU_BRAM_OFFSET_BA_BASE_ADDR + (i * UU_BA_OFFSET))
/** Registers list with corresponding memory map for RTL use */

#define UU_WLAN_AC0_BUF_START	0x4AC00000
#define UU_WLAN_AC0_BUF_STOP	0x4AC03FFF
#define UU_WLAN_AC1_BUF_START	0x4AC20000
#define UU_WLAN_AC1_BUF_STOP	0x4AC23FFF
#define UU_WLAN_AC2_BUF_START	0x4AC40000
#define UU_WLAN_AC2_BUF_STOP	0x4AC43FFF
#define UU_WLAN_AC3_BUF_START	0x4AC60000
#define UU_WLAN_AC3_BUF_STOP	0x4AC63FFF
#define UU_WLAN_RX_BUF_DETAIL_START	0x4ACA0000
#define UU_WLAN_RX_BUF_DETAIL_STOP	0x4ACFAFFF

#define UU_WLAN_CONFIG_START    0x4AC84000

#define UU_WLAN_AC0_BUF_START_OFFSET   0x0000
#define UU_WLAN_AC1_BUF_START_OFFSET   0x4000
#define UU_WLAN_AC2_BUF_START_OFFSET   0x8000
#define UU_WLAN_AC3_BUF_START_OFFSET   0xc000
#define UU_WLAN_RX_BUF_DETAIL_START_OFFSET 0x20000

#define UU_SLOT_TIMER_VALUE_R             0x0000
#define UU_WLAN_SIFS_TIMER_VALUE_R        0x0004
#define UU_WLAN_DIFS_VALUE_R              0x0008
#define UU_PHY_RX_START_DELAY_R           0x000C
#define UU_WLAN_ACK_TIMER_VALUE_R         0x0010
#define UU_WLAN_CTS_TIMER_VALUE_R         0x0014
#define UU_WLAN_DOT11_SHORT_RETRY_COUNT_R 0x0018
#define UU_WLAN_DOT11_LONG_RETRY_COUNT_R  0x001C
#define UU_WLAN_RTS_THRESHOLD_R           0x0020
#define UU_REG_LMAC_BEACON_INRVL          0x0024
#define UU_WLAN_CWMIN_VALUE_R             0x0028
#define UU_WLAN_CWMAX_VALUE_R             0x002C

/** i value varies as per the AC */
#define UU_WLAN_CW_MIN_VALUE_R(i)   (0x0030+(i*10))
#define UU_WLAN_CW_MAX_VALUE_R(i)   (0x0034+(i*10))
#define UU_WLAN_TXOP_LIMIT_R(i)     (0x0038+(i*10))
#define UU_WLAN_AIFS_VALUE_R(i)     (0x003C+(i*10))

#define UU_WLAN_SIGNAL_EXTENSION_R   0x0070

#define UU_WLAN_IEEE80211_QOS_MODE_R             0x0074
#define UU_WLAN_IEEE80211_OP_MODE_R              0x0078


#define UU_WLAN_IEEE80211_STA_MAC_ADDR_R    0x007C
#define UU_WLAN_IEEE80211_STA_MAC_ADDR_R_1  0x0080

#define UU_WLAN_SELF_CTS_R                  0x0084
#define UU_REG_DOT11_ACK_FAILURE_COUNT      0x0088
#define UU_REG_DOT11_RTS_FAILURE_COUNT      0x008C
#define UU_REG_DOT11_RTS_SUCCESS_COUNT      0x0090

#define UU_REG_DOT11_FCS_ERROR_COUNT        0x0094
#define UU_WLAN_RX_FRAMES                   0x0098
#define UU_WLAN_RX_MULTICAST_CNT            0x009C
#define UU_WLAN_RX_BROADCAST_CNT            0x00A0
#define UU_WLAN_RX_FRAME_FOR_US             0x00A4
#define UU_WLAN_RX_AMPDU_FRAMES             0x00A8
#define UU_WLAN_RX_AMPDU_SUBFRAMES          0x00AC
#define UU_WLAN_RX_ERR_FRAMES               0x00B0

/** 16-bits are used ie from 0 to 15.
 * and from 16 to 31 reserved.*/
#define UU_WLAN_RX_STATUS_REG               0x00B4

/* TODO: What is the difference between these 2 registers? */
#define UU_REG_LMAC_FILTER_FLAG             0x00B8
#define UU_WLAN_RX_FILTER_FLAG_R            0x01B8


/** Reserved 66 registers of 32-bit size.
 * reserved address map is 0x00BC to 0x41BF */
#define Operating_chbndwdth                 0x00BC
#define Channel_hwvalue                     0x00C0
#define Center_frequency                    0x00C4


#define Sta_is_used                         0x018C
#define ba_is_used_w0                       0x0190
#define ba_is_used_w1                       0X0194

#define BSSBasicRateSet                     0x015d

#define UU_ISR_OFFSET                       0x0070
#define UU_ISR_RX_MASK                      0x0000001
#define UU_ISR_TXSTATUS_MASK                0x0000002
#define UU_WLAN_TXREADY                     0x0074


#define UU_WLAN_LMAC_IN_AC_SELECT_INFO      0x01A4
#define UU_WLAN_LMAC_IN_EV_REG              0x01A8

#define LMAC_IN_TXFRAMEINFO_VALID_R         0x01AC
#define LMAC_IN_AGGR_COUNT                  0x01B4

/* PHY Configuration and Status Registers */
#define PHY_COND_REGISTER0                  0x019c  /* Configuration Reg */
#define PHY_COND_REGISTER1                  0x01A0  /* Configuration Reg */ 
#define PHY_COND_REGISTER2                  0x0538  /* Status Reg */ 
#define PHY_COND_REGISTER3                  0x053c  /* Status Reg */ 


extern uu_wlan_op_mode_t uu_dot11_op_mode_r;

/** Reserved from 0x41CC to 0x 41CF */

/** Station MAC Address */

#define UU_STA_MAC_ADDR_BASE_ADDR	(0xD0)
#define UU_STA_MAC_ADDR_R_W0(i)		( UU_STA_MAC_ADDR_BASE_ADDR + (i*8) ) /*TODO Check value*/ 

/** TX STATUS MEMORY MAP */
#define UU_WLAN_TX_STATUS_INFO_BASE_ADDR	(0x10)
#define UU_WLAN_TX_STATUS_INFO_R(i,j)		((UU_WLAN_TX_STATUS_INFO_BASE_ADDR+(i*0x80)) + (j*8))

/** RX STATUS MEMORY MAP */
#define UU_WLAN_RX_STATUS_INFO_BASE_ADDR	0x20
#define UU_WLAN_RX_STATUS_INFO(i)		(UU_WLAN_RX_STATUS_INFO_BASE_ADDR + ( (i-1) * 4 ) )


/** Reserved 36 registers of 32-bit size.
 * reserved address map is 0x4270 to ox42FF */
#define UU_WLAN_TX_STATUS_FLAGS_REG(i)       (0x00+i*4)

#define UU_WLAN_TXSTATUS_SEQNO_MASK 0xFFF

/** Interrupt Status Register.
 * Bit 0 is for Tx status change notification,
 * Bit 1 is for Rx frame notification
 */
#define UU_WLAN_INT_STATUS_REG_R               0X124


/* Debug Registers */
#define UU_WLAN_DEBUG_STATUS_REG_START         0x131
#define UU_WLAN_DEBUG_STATUS_REG_END           0x14F
#define UU_WLAN_DEBUG_STATUS_INFO(i)           (i * 4)


/*
** Platform specific functions
*/
extern irqreturn_t uu_wlan_interrupt(int irq, void *dev_id);

extern void uu_wlan_rx_tasklet_zynq(unsigned long data);

extern void uu_wlan_txstatus_tasklet(unsigned long data);


#endif /* __UU_WLAN_REG_PLATFORM_H__ */

