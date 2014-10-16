/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_eeprom.h                                        **
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

#ifndef __UU_WLAN_EEPROM_H__
#define __UU_WLAN_EEPROM_H__

UU_BEGIN_DECLARATIONS

/* EEPROM GET and SET functions for MAC address handling */

/** Setting mac address to eeprom module.
 * Read the last byte of the mac address from the command line and 
 * set the mac address
 */
extern uu_void uu_wlan_set_macaddr_to_eeprom(int mac_addr);

/** Getting mac address from EEPROM module.
 * @param[out] new_mac_address get from eeprom module
 */
extern uu_void uu_wlan_get_macaddr_from_eeprom(uu_uchar new_mac_address[]);

UU_END_DECLARATIONS
#endif /*__UU_WLAN_EEPROM_H__ */
