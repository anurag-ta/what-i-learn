/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_eeprom.c                                              **
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
/* $Revision: 1.5 $ */

#include "uu_wlan_main.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_eeprom.h"

/* For MAC Address */

/** Note: For testing the 3-byte OUI (company id) is taken as 0xFCF8B0. And of
 *  the three-byte device id, in current implementation first two bytes are
 *  fixed and the last byte is read from the command line during module load
*/
uu_uchar macaddr[UU_WLAN_MAC_ADDR_LEN]={0xFC,0xF8,0xB0,0x10,0x20,0x02};


uu_void uu_wlan_set_macaddr_to_eeprom(uu_int32 mac_addr)
{
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : mac address is %x:%x:%x:%x:%x:%x\n",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]));

    macaddr[UU_WLAN_MAC_ADDR_LEN-1]=(uu_uchar)mac_addr;

    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : mac address from cmdline is %x:%x:%x:%x:%x:%x\n",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]));
} /* uu_wlan_set_macaddr_to_eeprom */


uu_void uu_wlan_get_macaddr_from_eeprom(uu_uchar new_mac_address[])
{
   uu_uint16 i;

   UU_WLAN_LOG_DEBUG(("UURMIDRIVER : mac address in get mac address fun  %x:%x:%x:%x:%x:%x\n",macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]));
 
   for(i=0; i < UU_WLAN_MAC_ADDR_LEN; i++) 
   {
       new_mac_address[i] = macaddr[i];
   }
} /* uu_wlan_get_macaddr_from_eeprom */


/* EOF */

