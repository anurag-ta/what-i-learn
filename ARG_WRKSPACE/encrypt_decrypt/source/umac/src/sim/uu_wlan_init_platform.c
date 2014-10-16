/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_init_platform.c                                       **
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
/* $Revision: 1.2 $ */

/** 
 * @uu_wlan_init_platform.c
 * Contains main Module hooks like module init and module exit.
 */

#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


#include "uu_datatypes.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_main.h"
#include "uu_wlan_init.h"
#include "uu_wlan_reg_init.h"
#include "uu_wlan_eeprom.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_reg_platform.h"


MODULE_AUTHOR("UURMI Systems");
MODULE_DESCRIPTION("Support for uurmi 802.11ac wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("uurmi 802.11ac WLAN cards");
MODULE_LICENSE("Dual BSD/GPL");


/** Value passed from the command line argument for variable MAC address.
 * Change the last byte of the hardcoded MAC address with this value.
 */
int mac_addr = 2;

module_param(mac_addr, int, 0);

uu_lmac_ops_t lmac_ops_g=
{
    .umacCBK = uu_wlan_umac_cbk
};

/************************/
/*     Module Hooks     */
/************************/

/** Module init function.
 * Intialize lmac and umac. Set the mac address to EEPROM.
 * @return -ENODEV, failed to intialization or umac.
 * @return 0 for successful registration with lmac and umac.
 */


static uu_int32 __init uu_wlan_init(uu_void)
{
    uu_int32 error;
    unsigned char macaddr[6];

    uu_wlan_set_macaddr_to_eeprom(mac_addr);

    /** Setting default values to registers before LMAC init */
    uu_wlan_set_default_reg_values();
    /* Platform-specific change to default values */
    uu_wlan_change_defaults();

    /** Set MAC address to LMAC */
    uu_wlan_get_macaddr_from_eeprom(macaddr);
    UU_COPY_MAC_ADDR(UU_WLAN_IEEE80211_STA_MAC_ADDR_R, macaddr);
    uu_lmac_reg_ops(&lmac_ops_g);
    error = uu_wlan_lmac_init();
    //uu_setReg(UU_WLAN_IEEE80211_STA_MAC_ADDR_R,  0xFCF8B010);
    //uu_setReg(UU_WLAN_IEEE80211_STA_MAC_ADDR_R_1,  0x2002);

    if (error < 0)
    {
        UU_WLAN_LOG_ERROR(("UU_WLAN: uu_wlan_lmac_init failed. E:%x \n\n", error));
        error = -ENODEV;
        goto err_out;
    }

    error = uu_wlan_umac_init();
    if (error < 0)
    {
        UU_WLAN_LOG_ERROR(("UU_WLAN: uu_init init failed. E:%x\n", error));
        error = -ENODEV;
        goto err_out;
    }

    UU_WLAN_LOG_DEBUG(("UU_WLAN: init success \n"));
    return 0;

err_out:
    return error;
} /* uu_wlan_init */
module_init(uu_wlan_init);


/** Module exit function. */
static uu_void __exit uu_wlan_exit(uu_void)
{
    uu_wlan_umac_exit();
    UU_WLAN_LOG_DEBUG((" Driver unloaded\n"));
    uu_wlan_lmac_shutdown();
} /* uu_wlan_exit */
module_exit(uu_wlan_exit);

