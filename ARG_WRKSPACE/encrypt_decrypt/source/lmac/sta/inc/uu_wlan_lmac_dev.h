/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_lmac_dev.h                                     **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                          **
 ** All rights reserved.                                                      **
 ** http://www.uurmi.com                                                      **
 **                                                                           **
 ** All information contained herein is property of Uurmi Systems             **
 ** unless otherwise explicitly mentioned.                                    **
 **                                                                           **
 ** The intellectual and technical concepts in this file are proprietary      **
 ** to Uurmi Systems and may be covered by granted or in process national     **
 ** and international patents and are protect by trade secrets and            **
 ** copyright law.                                                            **
 **                                                                           **
 ** Redistribution and use in source and binary forms of the content in       **
 ** this file, with or without modification are not permitted unless          **
 ** permission is explicitly granted by Uurmi Systems.                        **
 **                                                                           **
 ******************************************************************************/

#ifndef __UU_WLAN_LMAC_DEV_H__
#define __UU_WLAN_LMAC_DEV_H__

#include "uu_wlan_frame.h"


typedef struct uu_wlan_config_thrd_info
{
    struct task_struct *thread;
    int running;
    int stop;
} uu_wlan_tx_thrd_info_t, uu_wlan_config_thrd_info_t;


typedef struct uu_wlan_lmac_device 
{
    /*  Tx thread information */
    uu_wlan_tx_thrd_info_t           *tx_thread;

} uu_wlan_lmac_device_t;


extern uu_wlan_lmac_device_t      uu_wlan_lmac_device_g;
extern uu_wlan_config_thrd_info_t *config_thrd_gp;


#endif /* __UU_WLAN_LMAC_DEV_H__ */

