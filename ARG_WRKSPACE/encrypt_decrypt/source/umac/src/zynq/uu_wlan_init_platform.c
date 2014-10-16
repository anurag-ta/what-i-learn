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

#define DRIVER_NAME         "uurmi"
#define DRIVER_DESCRIPTION      "UURMI WiFi TEST driver"
#define DRIVER_VERSION          "1.00a"

#define UU_PM  NULL

void __iomem *baseaddr_config;
void __iomem *baseaddr_status;
void __iomem *baseaddr_txbuf;
void __iomem *baseaddr_rxbuf;
void __iomem *baseaddr_stamgmt;



uu_int32 uu_getReg(uu_uint32 offset)
{
    return readl(baseaddr_config+offset);
}

uu_int32 uu_getstatusReg(uu_uint32 offset)
{
    return readl(baseaddr_status+offset);
}

uu_int32 uu_gettxReg(uu_uint32 offset)
{
    return readl(baseaddr_txbuf+offset);
}

uu_int32 uu_getrxReg(uu_uint32 offset)
{
    return readl(baseaddr_rxbuf+offset);
}

uu_int32 uu_getstamgmtReg(uu_uint32 offset)
{
    return readl(baseaddr_stamgmt+offset);
}


uu_void uu_setReg(uu_uint32 val, uu_uint32 offset)
{
    writel(val, baseaddr_config+offset);
}

uu_void uu_setstatusReg(uu_uint32 val, uu_uint32 offset)
{
    writel(val, baseaddr_status+offset);
}


uu_void uu_settxReg(uu_uint32 val, uu_uint32 offset)
{
    writel(val, baseaddr_txbuf+offset);
}


uu_void uu_setrxReg(uu_uint32 val, uu_uint32 offset)
{
    writel(val, baseaddr_rxbuf+offset);
}

uu_void uu_setstamgmtReg(uu_uint32 val, uu_uint32 offset)
{
    writel(val, baseaddr_stamgmt+offset);
}


static int uu_probe(struct platform_device *pdev)
{

    struct resource *r_mem = NULL;
#if 0
    struct resource *r_irq = NULL;
    struct net_device *ndev;
    uu_uint32 regval = 0,i;
#endif  
    int rc = -ENXIO;
    unsigned int        irq;
    uu_int32 error;
    int r;

    UU_WLAN_LOG_DEBUG(("PROBE \n"));
    if(pdev==NULL)
    {
        UU_WLAN_LOG_DEBUG(("NULL Platform device returned ......\n"));
        goto err_out;
    }
    UU_WLAN_LOG_DEBUG(("UMAC : dev id is %d and name is %s\n", pdev->id, pdev->name));

    irq = platform_get_irq(pdev, 0);
    UU_WLAN_LOG_DEBUG(("UMAC : IRQ is %d \n", irq));

    for(r=0;r<pdev->num_resources;r++)
    {
        r_mem = platform_get_resource(pdev, IORESOURCE_MEM, r);
        if(!r_mem)
        {
            UU_WLAN_LOG_DEBUG(("UMAC : No IO resource defined... Null r_mem \n"));
            goto err_out;
        }

        switch(r)
        {
            case 0:
                baseaddr_config = ioremap(r_mem->start, (r_mem->end - r_mem->start -1)); //ioremap(0x40000000, (0x0fff-1));
                UU_WLAN_LOG_DEBUG(("UMAC Config : r_mem start is %x and r_mem end is %x \n", r_mem->start, r_mem->end));
                break;

            case 1:
                baseaddr_status = ioremap(r_mem->start, (r_mem->end - r_mem->start -1)); //ioremap(0x40000000, (0x0fff-1));
                UU_WLAN_LOG_DEBUG(("UMAC Status : r_mem start is %x and r_mem end is %x \n", r_mem->start, r_mem->end));
                break;

            case 2:
                baseaddr_txbuf = ioremap(r_mem->start, (r_mem->end - r_mem->start -1)); //ioremap(0x40000000, (0x0fff-1));
                UU_WLAN_LOG_DEBUG(("TX Buffer : r_mem start is %x and r_mem end is %x \n", r_mem->start, r_mem->end));
                break;

            case 3:
                baseaddr_rxbuf = ioremap(r_mem->start, (r_mem->end - r_mem->start -1)); //ioremap(0x40000000, (0x0fff-1));
                UU_WLAN_LOG_DEBUG(("RX Buffer : r_mem start is %x and r_mem end is %x \n", r_mem->start, r_mem->end));
                break;

            case 4:
                baseaddr_stamgmt = ioremap(r_mem->start, (r_mem->end - r_mem->start -1)); //ioremap(0x40000000, (0x0fff-1));
                UU_WLAN_LOG_DEBUG(("Station management : r_mem start is %x and r_mem end is %x \n", r_mem->start, r_mem->end));
                break;

            default:
                return -1;
                break;
        }
    }

    if ((!baseaddr_config) || (!baseaddr_status) || (!baseaddr_txbuf) || (!baseaddr_rxbuf))
    {
        UU_WLAN_LOG_DEBUG(("TESTDRIVER : failed to map baseaddress.\n"));
        rc = -ENOMEM;
        goto err_out;
    }

    /** Setting default values to registers before LMAC init */
    UU_WLAN_LOG_DEBUG(("config baseaddr is %x\n",(int)baseaddr_config));
    uu_wlan_set_default_reg_values();
    UU_WLAN_LOG_DEBUG(("AFter set def reg \n"));
#if 1
    error = uu_wlan_umac_init();
    if (error < 0)
    {
        UU_WLAN_LOG_ERROR(("UU_WLAN: uu_init init failed. E:%x\n", error));
        error = -ENODEV;
        goto err_out;
    }
#endif
    UU_WLAN_LOG_DEBUG(("Module init ..... \n"));


err_out:
    platform_set_drvdata(pdev, NULL);
    return rc;
} /* uu_probe */


static int uu_remove(struct platform_device *pdev)
{
    uu_wlan_umac_exit();
    UU_WLAN_LOG_DEBUG((" Driver unloaded\n"));

    return 0;
}

static struct of_device_id uu_drv_of_match[] = {
    { .compatible = "xlnx,bramreg-1.00.a", },
    { /* end of table */}
};
MODULE_DEVICE_TABLE(of, uu_drv_of_match);

static struct platform_driver uu_driver = {
    .probe   = uu_probe,
    .remove  = uu_remove,
    .driver  = {
        .name  = DRIVER_NAME,
        .owner = THIS_MODULE,
        .of_match_table = uu_drv_of_match,
        .pm = UU_PM,
    },
};

module_platform_driver(uu_driver);

MODULE_AUTHOR("UURMI Systems");
MODULE_DESCRIPTION("UURMI MAC driver");

