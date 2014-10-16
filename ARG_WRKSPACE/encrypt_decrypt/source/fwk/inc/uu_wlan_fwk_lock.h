/*************************************************************************
**                                                                      **
** File name :  uu_wlan_fwk_lock.h                                       **
**                                                                      **
** Copyright © 2013, Uurmi Systems                                     **
** All rights reserved.                                                 **
** http://www.uurmi.com                                                 **
**                                                                      **
** All information contained herein is property of Uurmi Systems        **
** unless otherwise explicitly mentioned.                               **
**                                                                      **
** The intellectual and technical concepts in this file are proprietary **
** to Uurmi Systems and may be covered by granted or in process national**
** and international patents and are protect by trade secrets and       **
** copyright law.                                                       **
**                                                                      **
** Redistribution and use in source and binary forms of the content in  **
** this file, with or without modification are not permitted unless     **
** permission is explicitly granted by Uurmi Systems.                   **
**                                                                      **
*************************************************************************/
#ifndef __UU_WLAN_FWK_LOCK_H__
#define __UU_WLAN_FWK_LOCK_H__

#ifdef __KERNEL__
#include <linux/spinlock.h>

typedef spinlock_t uu_wlan_spin_lock_type_t;

#define UU_WLAN_SPIN_LOCK_INIT(x)   spin_lock_init(x)
#define UU_WLAN_SPIN_LOCK(x)        spin_lock(x)
#define UU_WLAN_SPIN_UNLOCK(x)      spin_unlock(x)
#else
typedef int uu_wlan_spin_lock_type_t;
#define UU_WLAN_SPIN_LOCK_INIT(x)   *x = 1
#define UU_WLAN_SPIN_LOCK(x)        *x = 0 /* TODO: do something here */
#define UU_WLAN_SPIN_UNLOCK(x)      *x = 1
#define EXPORT_SYMBOL(x)    
#endif


#endif /* __UU_WLAN_FWK_LOCK_H__ */

