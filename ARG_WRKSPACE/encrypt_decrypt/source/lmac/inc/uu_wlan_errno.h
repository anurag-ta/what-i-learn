/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_errno.h                                         **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                           **
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

#ifndef __UU_WLAN_ERRNO_H__
#define __UU_WLAN_ERRNO_H__

#include "uu_errno.h"

#define WLAN_GEN_ERROR_BASE    (0x00F01000)
#define WLAN_UTILS_ERROR_BASE  (0x00F02000)
#define WLAN_MEM_ERROR_BASE    (0x00F03000)
#define WLAN_FRAME_ERROR_BASE  (0x00F04000)


/* Error code to indicate Queue Full */
#define UU_ERR_MQ_FULL         -(WLAN_UTILS_ERROR_BASE + 1)
/* Error code to indicate Queue Empty */
#define UU_ERR_MQ_EMPTY        -(WLAN_UTILS_ERROR_BASE + 2)


/* Memory related errors */
#define  UU_ERR_NULL_ADDRESS     -(WLAN_MEM_ERROR_BASE+ 0x01)



/* Frame Errors */
//#define  UU_SUBTYPE_ERROR         -(WLAN_FRAME_ERROR_BASE+ 0x01)
//#define  UU_FRAME_ERROR           -(WLAN_FRAME_ERROR_BASE+ 0x02)
/* Version is not Zero (or) Type is Reserved */
//#define  UU_UNKNOWN_FRAME         -(WLAN_FRAME_ERROR_BASE+ 0x03)
//#define  UU_RXEND_ERROR           -(WLAN_FRAME_ERROR_BASE+ 0x04) /* Relook ?? */
/* FCS Invalid */
//#define  UU_FRAME_FCS_ERROR       -(WLAN_FRAME_ERROR_BASE+ 0x05)
//#define  UU_FRAME_AMPDU_DEL_FCS_ERROR   -(WLAN_FRAME_ERROR_BASE+ 0x06)


#define  UU_BA_SESSION_INVALID    -(WLAN_GEN_ERROR_BASE+ 0x01)
//#define  UU_ADDRESS_INVALID       -(WLAN_GEN_ERROR_BASE+ 0x02)

#endif /* __UU_WLAN_ERRNO_H__ */

/* EOF */


