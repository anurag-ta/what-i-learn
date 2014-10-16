/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_init.h                                          **
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

#ifndef __UU_WLAN_INIT_H__
#define __UU_WLAN_INIT_H__

UU_BEGIN_DECLARATIONS

/** UMAC intialization function.
 * Device creation and Main Hardware device allocation done here.
 * Sets the created device for 802.11 hardware and Sets mac address.
 * Intialization of Queues and hardware capabilities done here.
 * If device creation failed, Free the allocated memory.
 * Return following error codes
 * @return -ENOMEM - Failed to allocated requested memory.
 * @return 0 for successful registration with mac framework.
 */
extern uu_int32 uu_wlan_umac_init(uu_void);

/** Unregister the hardware using ieee80211_unregister_hw,
 * unregister the created device using device_unregister and free the allocated memory.
 */
extern uu_void uu_wlan_umac_exit(uu_void);

extern uu_void uu_wlan_umac_cbk(uu_int8 ind);
UU_END_DECLARATIONS

#endif /*__UU_WLAN_INIT_H__ */
