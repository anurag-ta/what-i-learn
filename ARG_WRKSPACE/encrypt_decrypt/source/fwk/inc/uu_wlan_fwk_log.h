/*************************************************************************
**                                                                      **
** File name :  uu_wlan_fwk_log.h                                       **
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

#ifndef __UU_WLAN_FWK_LOG_H__
#define __UU_WLAN_FWK_LOG_H__

#ifndef __KERNEL__
#include <stdio.h>
#endif

#ifdef __KERNEL__
#define UU_LOG_MSG(x)    printk x
#else
#define UU_LOG_MSG(x)    printf x
#define KERN_CRIT
#define KERN_INFO
#define printk printf
#endif
 
#ifdef LOG_DEBUG 

#define UU_WLAN_LOG_DEBUG(x)    UU_LOG_MSG(x)
#define UU_WLAN_LOG_INFO(x)     UU_LOG_MSG(x)
#define UU_WLAN_LOG_EVENT(x)    UU_LOG_MSG(x)

#else

#define UU_WLAN_LOG_DEBUG(x)    
#define UU_WLAN_LOG_INFO(x)
/* Enable this, to get only the important events & state-transitions log */
#define UU_WLAN_LOG_EVENT(x)    UU_LOG_MSG(x)

#endif

#ifndef UU_WLAN_LOG_EVENT
/* Empty macro expansion, to satisfy compiler */
#define UU_WLAN_LOG_EVENT(x)
#endif

#define UU_WLAN_LOG_ERROR(x)    UU_LOG_MSG(x)


#endif /* __UU_WLAN_FWK_LOG_H__ */

/* EOF */


